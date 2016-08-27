#!perl

# Part of BridgeData.
#
# Copyright (c) 2016 by Soren Hein.
#
# See LICENSE and README.


# Does a decent job of parsing a BBO database file of players.


use strict;
use warnings;
use Fcntl;
use List::Util qw(min);

# Used ASCII characters in the range 0x80 to 0xff.
my (@special_chars, %unicodes);
set_valid_special_characters();

# File containing a structured list of countries.
# Scraped from www.worldatlas.com/aatlas/ctycodes.htm
my $COUNTRIES = "COUNTRIES";
my %country_list;
read_countries();

# BBO database.
my $DATABASE = "db";
my $FIELD_FULL_NAME = 0;
my $FIELD_COUNTRY = 2;

# Split database and skip leading entry.
my $slurp = slurp_file($DATABASE);
my @list = split /\x00\x50([A-Z0-9_]+)\x00[\x01-\x1f]/, $slurp;
shift @list;

my $c = 0;
my (@entries, @ok_names);

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

    if (name_is_odd($clean_name) ||
        has_other_special_characters($clean_name))
    {
      $clean_name = "";
    }
    else
    {
      push @ok_names, $clean_name;
    }
  }

  my $country = $fields[$FIELD_COUNTRY];
  next if ($country =~ /[\x00-\x1f]/);

  my $cmatch = (defined $country_list{$country} ? 
    $country : fuzzy_match($country));

  $entries[$c][0] = $handle;
  $entries[$c][1] = $clean_name;
  $entries[$c][2] = ($cmatch eq "" ? "" : $country_list{$cmatch});
  $c++;
}

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

for my $k (sort keys %record)
{
  printf "%-16s %-6s %-40s\n", $k, $record{$k}[1], $record{$k}[0];
}

print_list("OK names", \@ok_names);
exit();


sub set_valid_special_characters
{
  my @sp = (
    #  °
    0xb0, 
    #  Á     Â    ~A     Ä    AA    AE    C,     È
    0xc1, 0xc2, 0xc3, 0xc4, 0xc5, 0xc6, 0xc7, 0xc8, 
    #  É     Í     Î    :I
    0xc9, 0xcd, 0xce, 0xcf,
    # ~N     Ò     Ó    ~O     Ö    OE     Ú    Ü      ß
    0xd1, 0xd2, 0xd3, 0xd5, 0xd6, 0xd8, 0xda, 0xdc, 0xdf,
    #  à     á     â    ~a     ä    aa    ae    c,
    0xe0, 0xe1, 0xe2, 0xe3, 0xe4, 0xe5, 0xe6, 0xe7, 
    #  è     é     ê    :e     ì     í     î    :i
    0xe8, 0xe9, 0xea, 0xeb, 0xec, 0xed, 0xee, 0xef,
    # ~d    ~n     ò     ó     ô    ~o     Ö    OE
    0xf0, 0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf8,
    #  ù     ú     û     ü    :y
    0xf9, 0xfa, 0xfb, 0xfc, 0xff);

  $special_chars[$_] = 1 for (@sp);

  my @u0 = qw(
    0101 0102 0105 0107 010D 
    0115 0119 011B 011E 011F 
    0120 0123 012B
    0130 0131 0133 0134
    0141 0142 0143 0144 014E
    0151 0152 0156 0159 015A 015B 015E 015F 
    0160 0161 0162 0164 0166 0167 016B
    0172 0173 0175 0177 0179 017A 017B 017C 017E 017F
    0180 018D 018F
    0190 0199 019A
    01A6 01C0 01D6 01D8);

  # ASCII approximations
  my @u1 = qw(
       a    A    a    c    c    
       e    e    e    G    g    
       G    g    i
       I    i    j    J
       L    l    N    n    O
       ö    O    R    r    s    s    S    s    
       S    s    T    T    T    t    u
       U    u    w    y    Z    z    Z    y    z   s
       b    o    E
       E    k    l
       R    l    u    u);

  for my $i (0 .. $#u0)
  {
    $unicodes{$u0[$i]} = $u1[$i];
  }
}


sub read_countries
{
  open my $fh, '<', $COUNTRIES or die "Can't < $COUNTRIES: $!";
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
      $country_list{$cc} = $a[1];
    }
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


sub print_list
{
  my $r = pop;
  print "\n", pop, ":\n";
  for my $e (@$r)
  {
    print "$e\n";
  }
}

