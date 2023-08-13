#!perl

use strict;
use warnings;

# Throw-away script that parses the output from reader with
# -Q 4-3-3-3 (say) and -v 63.

if ($#ARGV < 0)
{
  print "Usage: perl cards.pl file\n";
  exit;
}

my $file = $ARGV[0];

open my $fr, '<', $file or die "Can't open $file $!";
my @store;
while (my $line = <$fr>)
{
  chomp $line;
  $line =~ s///g;

  if ($line =~ /^Input/)
  {
    @store = ();
    push @store, $line;
  }
  elsif ($line =~ /^Made/ || $line =~ /^Down/)
  {
    push @store, $line;
    print "\n";
    for my $l (@store)
    {
      print "$l\n";
    }
    @store = ();
  }
  else
  {
    push @store, $line;
  }
}
close $fr;

