#!perl

use strict;
use warnings;

if ($#ARGV < 0)
{
  print "Usage: perl multicom.pl 12345.lin\n";
  exit;
}

my $fname = $ARGV[0];

open my $fh, '<', $fname or die "Can't open zskip.txt: $!";

my $mopen = 0;
my $lno = 0;
while (my $line = <$fh>)
{
  $lno++;
  chomp $line;
  $line =~ s///g;

  if ($line =~ /^nt\|[^|]*$/)
  {
    # Starting on multi-line nt.
    die "Already open nt: $lno" if ($mopen);
    $mopen = 1;
  }
  elsif ($mopen)
  {
    if ($line =~ /\|$/)
    {
      $mopen = 0;
    }
  }
  else
  {
    print "$line\n";
  }
}

close $fh;

exit;

