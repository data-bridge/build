#!perl

use strict;
use warnings;

require "adder.pl";

if ($#ARGV < 0)
{
  print "Usage: perl pbncount.pl cvg20xx.txt\n";
  exit;
}
my $fname = $ARGV[0];

my %counts;

open my $fh, '<', $fname or die "Can't open $fname: $!";
while (my $line = <$fh>)
{
  chomp $line;
  $line =~ s///g;

  if ($line =~ /:$/)
  {
    next if $line =~ /short:/ || $line =~ /Error:/;
    $counts{$line}++;
  }
}
close $fh;

my $sum = 0;
for my $k (sort keys %counts)
{
  printf "%4d  %s\n", $counts{$k}, $k;
  $sum += $counts{$k};
}
print "----\n$sum\n";
