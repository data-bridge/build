#!perl

# Part of BridgeData.
#
# Copyright (c) 2016 by Soren Hein.
#
# See LICENSE and README.

# Does a decent job of parsing a BBO database file of players.
# It is effectively an NER (Named Entity Recognizer), specialized
# for the multi-language BBO name field.
# Takes about 35 seconds on my laptop with the 60 MB db file
# and about 90,000 "real" names.
# perl split db


use strict;
use warnings;
use Fcntl;
use List::Util qw(min);
use Name;

require 'dict.pl';


my $DEFAULT_DB = "~/bridgedata/BBODB/db";
my $db = ($#ARGV == -1 ? glob($DEFAULT_DB) : $ARGV[0]);


# File containing a structured list of countries.
# Scraped from www.worldatlas.com/aatlas/ctycodes.htm
my $DEFAULT_COUNTRIES = "~/bridgedata/countries/countries.txt";
my $countries = ($#ARGV <= 0 ? glob($DEFAULT_COUNTRIES) : $ARGV[1]);


# The NER destroys a few good names.
my %correct_names;
set_correct_names(\%correct_names);


# Used ASCII characters in the range 0x80 to 0xff.
my (@special_chars, %unicodes);
set_valid_special_characters(\@special_chars, \%unicodes);

# Set dictionaries.
my %dict;
set_dictionaries(\%dict);

my %country_list;
read_countries(\%country_list, $countries);

# BBO database.
my $FIELD_FULL_NAME = 0;
my $FIELD_COUNTRY = 2;

# Split database and skip leading entry.
my $slurp = slurp_file($db);
my @list = split /\x00\x50([A-Z0-9_]+)\x00[\x01-\x1f]/, $slurp;
shift @list;

my $c = 0;
my (@entries, %fields);
my $name = Name->new();

while ($#list >= 0)
{
  my $handle = lc shift @list;

  # Skip handles with special characters.
  next if ($handle =~ /[\x00-\x1f\x80-\xff]/);

  my $entry = shift @list;

  # Strip trailing null bytes.
  $entry =~ s/\x00*$//;

  # Compress consecutive null bytes.
  $entry =~ s/\x00+/\x00/g;

  # Lose a few valid entries this way.
  my @fields = split /\x01/, join '', $entry;
  next if ($#fields < 2);

  my $full_name = $fields[$FIELD_FULL_NAME];
  my $clean_name = "";
  if (length $full_name > 0)
  {
    # Skip full names with low special characters.
    next if ($full_name =~ /[\x00-\x1f]/);
    next if ($full_name eq "");

    $clean_name = cleanup_full_name($full_name);
    $clean_name = fix_special_characters($clean_name);

    $clean_name = "" if (name_is_odd($clean_name) ||
        has_other_special_characters($clean_name));
  }

  my $country = $fields[$FIELD_COUNTRY];
  next if ($country =~ /[\x00-\x1f]/);

  my $cmatch = (defined $country_list{$country} ? 
    $country : fuzzy_match($country));

if ($clean_name =~ /Adam Hey/)
{
  my $x;
}

  $clean_name = sanitize_name($clean_name);
  if ($clean_name ne "")
  {
    $name->set_string($clean_name);
    grok($name, \%dict, \%fields);

    if ($name->get_length() >= 3)
    {
      # print $name->as_string(), "\n";
      # print print_field($fields{first}, "First");
      # print print_field($fields{particle}, "Particle");
      # print print_field($fields{last}, "Last");
      # print print_field($fields{addition}, "Addition");

      $clean_name = print_fields_compact(\%fields);
      # printf "%-30s", $name->as_string();
      # print print_fields(\%fields);
      # print "\n";
    }
    else
    {
      $clean_name = "";
    }
  }

  $entries[$c][0] = $handle;
  $entries[$c][1] = $clean_name;
  $entries[$c][2] = ($cmatch eq "" ? "" : $country_list{$cmatch});
  $c++;
}

# Deal with collisions (multiple entries for the same player).
my %record;
for my $e (@entries)
{
  my $k = $e->[0];
  if (defined $record{$k})
  {
    $record{$k}[0] = $e->[1] if ($e->[1] ne "");
    $record{$k}[1] = $e->[2] if ($e->[2] ne "");
  }
  else
  {
    $record{$k}[0] = $e->[1];
    $record{$k}[1] = $e->[2];
  }
}

# Manually fix some names that collide with the dictionaries.
for my $k (keys %correct_names)
{
  next unless defined $record{$k};
  $record{$k}[0] = print_fields_compact(\%{$correct_names{$k}});
}

for my $k (sort keys %record)
{
  printf "%-16s %-6s %-40s\n", $k, $record{$k}[1], $record{$k}[0];
}

exit();


sub read_countries
{
  my ($list, $countries) = @_;
  open my $fh, '<', $countries or die "Can't < $countries: $!";
  while (my $line = <$fh>)
  {
    chomp $line;
    $line =~ s/$//;
    my @a = split /\|/, $line;
    
    die "Bad line $line" if ($#a != 2);
    my @ct;
    if ($a[2] =~ /;/)
    {
      @ct = split /;/, $a[2];
    }
    else
    {
      push @ct, $a[2];
    }
    for my $cc (@ct)
    {
      $list->{$cc} = $a[1];
    }
  }
}


sub set_correct_names
{
  my $correct_names = pop;

  my @hlist = qw( 
    aplo12
    marychang  englefjes  frb123      patybridge mobrignow
    stuart76   bones36    smxfoxterr  kyumikr    sillafu
    benitozzo  sercowy    reginaheart oizyz      adelba
    ralc8      justmatt17 kcsuez      ma57er     agnes1903
    shannin    jimqueen   tkqueen     pludog     150honours
    futurerist msuit
  );
  my @flist = qw( 
    Lieselotte
    Mary       Bente      Roy         Pat        Mike
    Stuart     Charlotte  Lou         Emerald    Benito
    Silvana    Jack       Regina      Adam       Tony
    Matt       Matthew    Sue         Christian  Mary
    Mary       James      Kevin       Frank      Kyle
    Mary       Michael    
  );

  my @llist = qw( 
    Appel
    Appel      Ask        Bridge      Bridge     Bridge
    Bridge     Bridge     Capp        Cho        Garozzo
    Garozzo    Heart      Heart       Hey        Jump
    Just       Just       Minor       Peace      Queen
    Queen      Queen      Queen       Queen      Splinter
    Splinter   Suit
  );

  $flist[1] = "Mary Y."; # Can't get this into qw()...
  $flist[2] = "Bente M.";
  $flist[24] = "Frank C.";
  $flist[25] = "Kyle R.";

  for my $i (0 .. $#hlist)
  {
    push @{$correct_names->{$hlist[$i]}{first}}, $flist[$i];
    push @{$correct_names->{$hlist[$i]}{particle}}, "";
    push @{$correct_names->{$hlist[$i]}{last}}, $llist[$i];
    push @{$correct_names->{$hlist[$i]}{addition}}, "";
  }
}


sub slurp_file 
{
  # www.perl.com/pub/2003/11/21/slurp.html, modified
  my($file_name) = @_;

  my $buf = "";
  my $buf_ref = \$buf;
  my $mode = O_RDONLY | O_BINARY;

  local(*FH);
  sysopen(FH, $file_name, $mode) or die "Can't open $file_name: $!";

  my $size_left = -s FH;
  while ($size_left > 0) 
  {
    my $read_cnt = sysread(FH, ${$buf_ref},
      $size_left, length ${$buf_ref});

    die "read error in file $file_name: $!" unless ($read_cnt);
    $size_left -= $read_cnt;
  }
  return ${$buf_ref};
}


sub cleanup_full_name
{
  my $name = pop;

  # Remove (...)
  $name =~ s/\([^()]*\)/ /g;

  # Remove [...]
  $name =~ s/\[[^\[\]]*\]/ /g;

  # Remove from first " - " onward.
  $name =~ s/ \- .*$/ /g;

  # Remove leading and trailing space, combine spaces.
  # Remove space before comma.
  $name =~ s/^\s+//;
  $name =~ s/\s+$//;
  $name =~ s/\s+/ /g;
  $name =~ s/\s+,/,/g;

  return $name;
}


sub fix_special_characters
{
  my $line = pop;

  if ($line =~ /\xc2/)
  {
    # No particular system.
    # All other \xc2's are considered real Â's.
    $line =~ s/\xc2\x8c/S/g;            # U+015a, ´S
    $line =~ s/\xc2\x9a/s/g;            # U+0161, s with caron
    $line =~ s/\xc2\x9c/s/g;            # U+015b, ´s
    $line =~ s/\xc2\x9f/z/g;            # U+017a, ´z
    $line =~ s/\xc2\xa3/L/g;            # U+0141, L with stroke
    $line =~ s/([Mm])\xc2\xaa/$1\xaa/g; # Maria, let through
    $line =~ s/\xc2\xb0/\xb0/g;         # °, let through
    $line =~ s/\xc2\xb3/l/g;            # U+0142, l with stroke
    $line =~ s/\xc2\xb9/a/g;            # U+0105, a with ogonek
    $line =~ s/\xc2\xbf/\xbf/g;         # z, let through
  }

  if ($line =~ /\xc3/)
  {
    # Regular escapes with respect to high ASCII codes.
    # All other \xc3's are considered real A-tildes.
    my @a = split //, $line;
    for (my $i = $#a-1; $i >= 0; $i--)
    {
      if (ord($a[$i]) == 195 &&
          ord($a[$i+1]) >= 128 && 
	  ord($a[$i+1]) < 192)
      {
        $a[$i] = chr(ord($a[$i+1]) + 64);
        splice @a, $i+1, 1;
      }
    }
    $line = join '', @a;
  }

  if ($line =~ /\xc4/)
  {
    # Regular escapes with respect to Unicodes (add 0x80).
    # But we only use ASCII here, so it doesn't help.
    # All other \xc3's are considered real Ä's.
    $line =~ s/\xc4\x85/a/g;   # U+0105, a with ogonek
    $line =~ s/\xc4\x86/C/g;   # U+0106, ´C
    $line =~ s/\xc4\x87/c/g;   # U+0107, ´c
    $line =~ s/\xc4\x8c/C/g;   # U+010c, C with caron
    $line =~ s/\xc4\x8d/c/g;   # U+010d, c with caron
    $line =~ s/\xc4\x97/e/g;   # U+0117, e with dot above
    $line =~ s/\xc4\x99/e/g;   # U+0119, e with ogonek
    $line =~ s/\xc4\xb0/I/g;   # U+0130, I with dot above
    $line =~ s/\xc4\xb1/i/g;   # U+0131, i without dot above
    $line =~ s/\xc4\x9e/G/g;   # U+011c, G with breve
    $line =~ s/\xc4\x9f/g/g;   # U+011f, g with breve
  }

  if ($line =~ /\xc5/)
  {
    # Regular escapes with respect to Unicodes (add 0xc0).
    # But we only use ASCII here, so it doesn't help.
    # All other \xc5's are considered regular (Danish) AA's.
    $line =~ s/\xc5\x81/L/g;   # U+0141, L with stroke
    $line =~ s/\xc5\x82/l/g;   # U+0142, l with stroke
    $line =~ s/\xc5\x83/N/g;   # U+0143, ´N
    $line =~ s/\xc5\x84/n/g;   # U+0144, ´n
    $line =~ s/\xc5\x91/ö/g;   # U+0151, o with double acute
    $line =~ s/\xc5\x98/R/g;   # U+0158, R with caron
    $line =~ s/\xc5\x99/r/g;   # U+0159, r with caron
    $line =~ s/\xc5\x9a/S/g;   # U+015a, ´S
    $line =~ s/\xc5\x9b/s/g;   # U+015b, ´s
    $line =~ s/\xc5\x9e/S/g;   # U+015e, S-cedilla
    $line =~ s/\xc5\x9f/s/g;   # U+015f, s-cedilla
    $line =~ s/\xc5\xa1/s/g;   # U+0161, s with caron
    $line =~ s/\xc5\xba/z/g;   # U+017a, ´z
    $line =~ s/\xc5\xbb/Z/g;   # U+017b, Z with dot above
    $line =~ s/\xc5\xbc/z/g;   # U+017c, z with dot above
    $line =~ s/\xc5\xbe/z/g;   # U+017e, z with caron
  }

  # Other substitutions without escapes.

  $line =~ s/\x80/i/g;         # Unclear, occurs in Niresh
  $line =~ s/\x8c/S/g;         # U+015a, ´S
  $line =~ s/\x8e/Z/g;         # U+017d, Z with caron

  $line =~ s/\x92/'/g;         # Typo, probably
  $line =~ s/\x9a/s/g;         # U+0161, s with caron
  $line =~ s/\x9c/s/g;         # U+015b, ´s
  $line =~ s/\x9f/z/g;         # U+017a, ´z

  $line =~ s/\xa3/L/g;         # U+0141, L with stroke
  $line =~ s/\xaf/Z/g;         # U+017b, Z with dot

  $line =~ s/\xb3/l/g;         # U+0142, l with stroke
  $line =~ s/\xb4/'/g;         # Typo (or taste)
  $line =~ s/\xb6/ö/g;         # U+0151, o with double acute
  $line =~ s/\xb9/a/g;         # U+0105, a with ogonek
  $line =~ s/\xbf/z/g;         # U+017c, z with dot

  $line =~ s/\xd0/G/g;         # U+011e, G with breve
  $line =~ s/\xdd/I/g;         # U+0130, I with dot above
  $line =~ s/\xde/S/g;         # U+015e, S-cedilla

  $line =~ s/\xfd/i/g;         # U+0131, i without dot above
  $line =~ s/\xfe/s/g;         # U+015f, s-cedilla

  # \xf0 can be both a Turkish g with breve (U+011f) and
  # an Icelandic soft d (the ASCII \xf0).  These are tricky to
  # tell apart.  Here we say it's Turkish if:
  # - followed by ac, an, az, ai (without dot) or a (end of word)
  # - followed by d, e, l, t
  # - followed by il, im, in (not edin), is, it, i (end of word)
  # - followed by Turkish i without dot, or s-cedilla
  # - followed by me, mu (not mun)
  # - followed by ne
  # - followed by ra, re, ri (not rik), rt, ru
  # - followed by uz, or in the form [Uu]du
  # - end of word, preceded by a, e, u, or i (not vid, though)

  $line =~ s/\xf0(a[cnz,\xe7\W])/g$1/g;
  $line =~ s/\xf0([delt])/g$1/g;
  $line =~ s/\xf0(i[lmst\W])/g$1/g; # Also space, end
  $line =~ s/\xf0([\xfd\xfe])/g$1/g;
  $line =~ s/[^e][^d]\xf0(in)/g$1/g;
  $line =~ s/\xf0(mu[^n])/g$1/g;
  $line =~ s/\xf0([mn]e)/g$1/g;
  $line =~ s/\xf0(r[aetu])/g$1/g;
  $line =~ s/\xf0(ri[^k])/g$1/g;
  $line =~ s/\xf0(uz)/g$1/g;
  $line =~ s/([Uu])\xf0u/$1gu/g;
  $line =~ s/([^v][aeiu\xfd])\xf0(\W)/$1g$2/g;

  # A lesser ambiguity: \xd1 can be N-tilde or 'N (also lower-case).
  $line =~ s/\xd1([^aeiou])/N$1/g; # U+0143, Polish ´N (no vowel)
  $line =~ s/\xf1([^aeiou])/n$1/g; # U+0144, Polish ´n (no vowel)

  # Spanish writing for Maria.
  $line =~ s/([Mm])\xaa/$1aria/g;

  # Raised a.
  $line =~ s/\xaa/a/g;

  # Turn ° to . if there is a digit before it.
  $line =~ s/([1-9])\xb0/$1./g;  # °

  # Some Unicodes are embedded without escapes to identify them.
  while ($line =~ /(01[0-9A-F][0-9A-F])/)
  {
if (! defined $unicodes{$1})
{
  print "UU $1\n";
}
    my $s = $1;
    $line =~ s/$s/$unicodes{$s}/g;
  }

  return $line;
}


sub name_is_odd
{
  my $name = lc pop;

  # Looks private
  return 1 if ($name =~ /privat/);
  
  # Looks like an e-mail
  return 1 if ($name !~ / / && $name =~ /@/);

  my $l = length $name;
  my $normal = $name =~ tr/[a-z]//;
  my $excl = $name =~ tr/!//;
  my $dig = $name =~ tr/[0-9]//;
  my $space = $name =~ tr/ //;
  my $point = $name =~ tr/\.//;
  my $rest = $l - ($normal + $excl + $dig + $space + $point);

  # At least 4 normal letters
  return 1 if ($normal < 4);

  # No more special characters than rest
  return 1 if ($rest >= $normal);

  # No more digits than normal characters
  return 1 if ($dig >= $normal);

  # At most one exclamation mark
  return 1 if ($excl > 1);

  # No single words
  return 1 if ($space == 0 && $point == 0);

  return 0;
}



sub has_other_special_characters
{
  my $line = pop;
  return 0 unless $line =~ /[\x80-\xff]/;
  for my $i (split //, $line)
  {
    my $x = ord($i);
    next if $x <= 0x80;
    return 1 unless defined $special_chars[ord($i)];
  }
  return 0;
}


sub levenshtein
{
  # https://en.wikibooks.org/wiki/Algorithm_Implementation/Strings/Levenshtein_distance#Perl
  my ($str1, $str2) = @_;
  my @ar1 = split //, $str1;
  my @ar2 = split //, $str2;

  my @dist;
  $dist[$_][0] = $_ foreach (0 .. @ar1);
  $dist[0][$_] = $_ foreach (0 .. @ar2);

  foreach my $i (1 .. @ar1)
  {
    foreach my $j (1 .. @ar2)
    {
      my $cost = $ar1[$i-1] eq $ar2[$j-1] ? 0 : 1;
      $dist[$i][$j] = min(
        $dist[$i-1][$j] + 1,
	$dist[$i][$j-1] +1,
	$dist[$i-1][$j-1] + $cost);
    }
  }
  return $dist[@ar1][@ar2];
}


sub fuzzy_match
{
  my $s = pop;
  return "" if (
    $s eq 'Other' || 
    $s eq '' || 
    $s eq 'null' || 
    $s =~ /^Priv/);

  my @matches;
  my $best = 999;
  for my $k (keys %country_list)
  {
    my $d = levenshtein($s, $k);
    if ($d < $best)
    {
      @matches = ();
      push @matches, $k;
      $best = $d;
    }
    elsif ($d == $best)
    {
      push @matches, $k;
    }
  }

  # Too far off
  return "" if ($best >= 3); 

  # In case of confusion, give up.
  return ($#matches > 0 ? "" : $matches[0]);
}



sub sanitize_name
{
  my $line = pop;

  $line = fix_brackets($line);
  $line = fix_symbols($line);
  $line = fix_digits($line) if ($line =~ /[0-9]/);
  $line = fix_particles(\%dict, $line);
  $line = fix_apostrophes($line) if ($line =~ /'/);
  $line = fix_web_stuff(\%{$dict{domains}}, $line) if ($line =~ /\./);

  # General clean-up
  $line =~s/^[\s,\.\-]+//; # Leading junk
  $line =~s/[\s,\-]+$//;   # Trailing junk
  $line =~ s/\s+/ /g;      # Multiple spaces
  $line =~ s/\s+,/,/g;     # Spaces before comma
  $line =~ s/,\s+/,/g;     # Spaces after comma
  return "" unless $line =~ tr/ ,\.\-//; # No single words
  return $line;
}


sub fix_brackets
{
  # After this function there are no more ( ) { } [ ] < > "

  my $line = pop;
  if ($line =~ /\(/)
  {
    # Take out ( } "pairs".
    $line =~ s/\([^)}]*\}//;

    # Take out from ( on out.
    $line =~ s/\(.*$//;
  }

  if ($line =~ /\{/)
  {
    # Take out { } pairs.
    $line =~ s/\{[^}]*\}//;

    # Take out from { on out.
    $line =~ s/\{.*$//;
  }

  # Take out from [ on out.
  $line =~ s/\[.*$//;

  if ($line =~ /</)
  {
    # Take out leading <'s.
    $line =~ s/^<+//;

    # Take out paired ones.
    $line =~ s/<[^>]*\>//;

    # Take out from < on out.
    $line =~ s/<.*//;
  }

  if ($line =~ /"/)
  {
    # Take out "" "pairs".
    $line =~ s/"[^"]*"//;

    # Turn O"Name into O'Name.
    $line =~ s/O"([A-Z])/O'$1/;

    # Take out from ( on out.
    $line =~ s/".*$//;
  }

  # Skip smiley-like leading and trailing characters.
  $line =~ s/^[:\);-]+//;
  $line =~ s/[:\);-]+$//;
  $line =~ s/\S*:\)//;

  # Take out from word with ) on out.
  $line =~ s/\S*\).*//;

  # Take out from word with } on out.
  $line =~ s/\S*\).*//;

  # Take out from word with ] on out.
  $line =~ s/\S*\].*//;

  # Take out from word with > on out.
  $line =~ s/\S*>.*//;

  return $line;
}


sub fix_symbols
{
  # After this function there are no more 
  # ~ \ / | ^ ` & $ % # * @ ? = ! + _ : ; 'and'

  my $line = pop;

  # Leading ~ and \
  $line =~ s/^~*//;
  $line =~ s/^\\*//;

  # Any line with & in it.
  return "" if ($line =~ /&/);
  return "" if ($line =~ / and /);
  return "" if ($line =~ / or /);
  return "" if ($line =~ / ou /);
  return "" if ($line =~ / en /);
  return "" if ($line =~ / och /);
  return "" if ($line =~ / og /);
  return "" if ($line =~ / fra /);
  return "" if ($line =~ /^d\-r /);

  $line =~ s/ on .*$//;
  $line =~ s/ of .*$//;
  $line =~ s/\blll\b/III/;

  if ($line =~ / in /)
  {
    if ($line !~ / in de / && $line !~ / in 't / && $line !~ / in t /)
    {
      $line =~ s/ in .*$//;
    }
  }

  # Fix apostrophe.
  $line =~ s/`/'/g;

  # From ~ \ / | ^ ; : on out (from that character).
  $line =~ s/~.*//;
  $line =~ s/\\.*//;
  $line =~ s/\/.*//;
  $line =~ s/\|.*//;
  $line =~ s/\^.*//;
  $line =~ s/;.*//;
  $line =~ s/:.*//;

  # From ., in out
  $line =~ s/\.,.*$//;

  my $line1 = $line;

  # Trailing +
  $line =~ s/\++$//g;

  # Leading and trailing *
  $line =~ s/^\*+//g;
  $line =~ s/\*+$//g;

  # From word containing special character on out.
  $line =~ s/[^\s,\.;-]*[\$\%\#\*@?=!+\xb0].*$//;

  # Turn a_b into a-b, otherwise drop _
  $line =~ s/(\w)_(\w)/$1-$2/g;
  $line =~ s/_//g;

  # From multiple periods on out
  $line =~ s/\.\..*$//;

  # Delete some websites.
  $line =~ s/\bhttp.*//g;
  $line =~ s/\bwww.*//g;


  return $line;
}


sub fix_digits
{
  # Delete from the word with a digit on out.
  my $line = pop;
  $line =~ s/^[^\s,.\-]*[0-9].*$//;
  $line =~ s/[\s,\.\-][^\s,.\-]*[0-9].*$//;
  return $line;
}



sub fix_particles
{
  my ($dict, $line) = @_;

  $line =~ s/in't/in 't/;
  $line =~ s/in t /in 't /;
  $line =~ s/ d' / d'/;

  # Turn O' Brien into O'Brien
  $line =~ s/ O' / O'/g;

  # O'Brien and d'Arcy are OK.
  return $line if ($line =~ /\b[oOdD]'\w/);

  if ($line =~ /\b[vV][ \.]?[dD][ \.]+(\w+)/)
  {
    # v.d.
    my $trail = lc $1;
    if (defined $dict->{part_van_den}{$trail})
    {
      $line =~ s/\b([vV][ \.]?[dD][ \.]+)\b/van den /;
    }
    elsif (defined $dict->{part_van_de}{$trail})
    {
      $line =~ s/\b([vV][ \.]?[dD][ \.]+)\b/van de /;
    }
    else
    {
      $line =~ s/\b([vV][ \.]?[dD][ \.]+)\b/van der /;
    }
  }

  if ($line =~ /\b[vV][ \.]?\b(\w+)/)
  {
    # v.
    my $trail = lc $1;

    if (defined $dict->{part_van}{$trail})
    {
      $line =~ s/\b(v[ \.]?)\b/van /;
    }
    elsif (defined $dict->{part_von}{$trail})
    {
      $line =~ s/\b(v[ \.]?)\b/von /;
    }
  }

  if ($line =~ /\b[oO][ \.]+(\w+)/)
  {
    # O'Scot.
    my $trail = lc $1;
    if (defined $dict->{part_scots}{$trail})
    {
      $line =~ s/\b([oO][ \.]+)/O'/;
    }
  }

  if ($line =~ /\b[mM][cC][ \.]+(\w+)/)
  {
    # Mc Scot.
    my $trail = lc $1;
    if (defined $dict->{part_mcs}{$trail})
    {
      $line =~ s/\b[mM][cC][ \.]+(\w+)/Mc/;
      my $w = $1;
      $line .= uc(substr($w, 0, 1)) . lc(substr($w, 1));
    }
  }

  if ($line =~ /\b[mM][aA][cC][ \.]+(\w+)/)
  {
    # Mac Scot.
    my $trail = lc $1;
    if (defined $dict->{part_macs}{$trail})
    {
      $line =~ s/\b[mM][aA][cC][ \.]+(\w+)/Mac/;
      my $w = $1;
      $line .= uc(substr($w, 0, 1)) . lc(substr($w, 1));
    }
  }

  return $line;
}


sub fix_apostrophes
{
  # This function is a bit tricky.
  my $line = pop;

  # Simple fixes.
  $line =~ s/^'+//;    # Leading '
  $line =~ s/'+$//;    # Trailing '
  $line =~ s/'\w*'//g; # Paired '', leaving only single ticks
  return $line if ($line !~ /'/);

  # Fix some likely typos.
  $line =~ s/' s /'s /g;
  $line =~ s/ 's /'s /g;
  $line =~ s/s' /'s /g;

  # Delete from the word with the genitive on out.
  $line =~ s/\b\w+'s\W.*//;
  return $line if ($line !~ /'/);

  # Delete from embedded 't on out
  $line =~ s/\b\w+'t .*//;

  if ($line =~ / qu'/)
  {
    # Delete from qu' onwards.
    $line =~ s/ qu'.*//;
  }

  if ($line =~ /you'll/)
  {
    # Delete from you'll onwards.
    $line =~ s/ you'll.*//;
  }

  # Note the letter just before.
  $line =~ /(\w)'/;
  return $line unless defined $1;

  my $letter = $1;
  if ($letter =~ /[ADHNOVabdhlmtuy]/)
  {
    # Consider it a name
    return $line;
  }
  elsif ($letter =~ /[EIJKTcjkpv]/)
  {
    # Delete from word on out
    $line =~ s/\b\S*$letter'.*//;
    return $line;
  }
  elsif ($letter eq 'C' && $line =~ /\w\w$letter'/)
  {
    # C not alone.
    return $line;
  }
  elsif ($letter eq 'L' && $line =~ / $letter'/)
  {
    # L alone but not at beginning of line.
    return $line;
  }
  elsif ($letter eq 'M' && $line =~ /\b$letter'/)
  {
    # M alone.
    return $line;
  }
  elsif ($letter eq 'e' && $line =~ /\w$letter'/)
  {
    # e not alone.
    return $line;
  }
  elsif ($letter eq 'o' && $line =~ /\b$letter'/)
  {
    # o alone.
    return $line;
  }
  elsif ($line =~ /n't/)
  {
    # n't
    return $line;
  }
  else
  {
    # Delete from word on out
    $line =~ s/\b\w*$letter'.*//;
    return $line;
  }
}


sub fix_web_stuff
{
  my ($dict, $line) = @_;

  # Delete websites.
  for my $d (keys %$dict)
  {
    $line =~ s/\b[^ ,\-]+\.$d\b.*//;
  }

  return $line;
}


sub grok
{
  my ($name, $dict, $fields) = @_;

  $name->delete_from_dict(\%{$dict->{words}}, 0);
  $name->delete_from_dict(\%{$dict->{after_comma}}, 1);
  $name->delete_from_dict(\%{$dict->{after_dash}}, 2);

  $name->delete_titles(\%{$dict->{titles}}, \%{$dict->{title_pairs}});

  $name->fix_periods();

  $name->fix_dashes(\%{$dict->{first_names}});

  $name->fix_additions(\%{$dict->{additions}});

  $name->fix_commas(\%{$dict->{places}});

  $name->fix_place_pairs(\%{$dict->{place_pairs}});

  # $name->check_tightness();

  $name->fix_trailing_initials();

  if ($name->get_length() >= 7)
  {
    $name->fix_many_tokens(\%{$dict->{first_names}});
  }

  $name->fix_only_short();

  $name->annotate(
    \%{$dict->{first_names}}, 
    \%{$dict->{poor_last_names}}, 
    \%{$dict->{good_last_names}}, 
    \%{$dict->{ambig_names}}, 
    \%{$dict->{particles}});

  # $name->check_annotations();

  $name->capitalize($fields);
}


sub print_field
{
  my ($a, $title) = @_;
  return "" unless defined $a;
  my $s = sprintf "%-8s %s\n", $title,  join(" ", @$a);
  return $s;
}


sub print_fields
{
  my $a = pop;
  return "" unless defined $a;
  my $s = sprintf "%-20s", 
    (defined $a->{first} ? join(" ", @{$a->{first}}) : "");
  $s .= sprintf "%-10s", 
    (defined $a->{particle} ? join(" ", @{$a->{particle}}) : "");
  $s .= sprintf "%-20s", 
    (defined $a->{last} ? join(" ", @{$a->{last}}) : "");
  $s .= sprintf "%-8s", 
    (defined $a->{addition} ? join(" ", @{$a->{addition}}) : "");
}


sub print_fields_compact
{
  my $a = pop;
  return "" unless defined $a;
  my @s;
  push @s, join(" ", @{$a->{first}}) if 
    (defined $a->{first} && $a->{first}[0] ne "");
  push @s, join(" ", @{$a->{particle}}) if 
    (defined $a->{particle} && $a->{particle}[0] ne "");
  push @s, join(" ", @{$a->{last}}) if
    (defined $a->{last} && $a->{last}[0] ne "");
  push @s, join(" ", @{$a->{addition}}) if 
    (defined $a->{addition} && $a->{addition}[0] ne "");

  return join " ", @s;
}

