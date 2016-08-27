#!perl

use strict;
use warnings;

my %dict;
set_constants();

open my $FH, "<", "b.txt" or die "Can't open file";
while (<$FH>)
{
  my $line = $_;
  chomp $line;
  $line =~ s///g;

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
  next unless $line =~ tr/ ,\.\-//; # No single words

  print "$line\n";
}

exit;


sub set_constants
{
  my @domains = qw(
    at ca ch cn dk es eu fi fr gr hk
    ie il in it jp kr nl no pl pt tr tw uk
    com org net int edu gov mil
  );

  $dict{domains}{$_} = 1 for (@domains);

  my @van_den = qw(
    berg bosch enk nieuwenhof
  );

  $dict{part_van_den}{$_} = 1 for (@van_den);

  my @van_de = qw(
    dool heuvel konijnenberg reijt scheur tillaar wetering
  );

  $dict{part_van_de}{$_} = 1 for (@van_de);

  # Rest of v.d. are "van der".

  my @van = qw(
    amerongen amsterdam bussel cranenbroek diemen erp
    kappel leeuwen luyk nieuwkerk oijen paassen rochow rooijen 
    straten tartwijk thiel velthoven woerden
  );

  $dict{part_van}{$_} = 1 for (@van);

  my @von = qw(
    heuzen lowzow werthern zarzycki
  );

  $dict{part_von}{$_} = 1 for (@von);

  my @scots = qw(
    briain brien callaghan carroll connell connor
    donnell donoghue donovan fagan gorman halloran
    hara keeffe lubaigh mahony mearain neill
    regan riordan shaunessy sullivan toole
  );

  $dict{part_scots}{$_} = 1 for (@scots);

  my @macs = qw(
    allan allister auliffe brien cann carthy
    donald evoy ewan gowan gregor innis
    kenna laughlin namara quilkin
  );

  $dict{part_macs}{$_} = 1 for (@macs);

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
    if (defined $dict->{part_macs}{$trail})
    {
      $line =~ s/\b[mM][cC][ \.]+(\w+)/Mc/;
      my $w = $1;
      $line .= uc(substr($w, 0, 1)) . lc(substr($w, 1));
    }
  }

  if ($line =~ /\b[mM][aA][cC][ \.]+(\w+)/)
  {
    # Mc Scot.
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

