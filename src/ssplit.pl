#!perl

use strict;
use warnings;

# Throw-away script that parses the output from tabul,
# specifically the short-version output with one line per hand.

if ($#ARGV < 0)
{
  print "Usage: perl sshort.pl file\n";
  exit;
}

my $file = $ARGV[0];

my %splits;

open my $fr, '<', $file or die "Can't open $file $!";
my @store;
while (my $line = <$fr>)
{
  chomp $line;
  $line =~ s///g;
  $line =~ s/\s+$//;
  my $bid = substr $line, 41, 7;
  $bid =~ s/\s+//g;

  push @{$splits{$bid}}, $line;

}
close $fr;

for my $bid (sort keys %splits)
{
  for my $line (@{$splits{$bid}})
  {
    print "$line\n";
  }
  print "\n";
}
