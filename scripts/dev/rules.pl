#!perl

use strict;
use warnings;
use Scalar::Util 'looks_like_number';
use v5.10;

use lib '.';
use Distributions;

# Throw-away script that parses the output from reader with
# -Q 9=4=0=0 (say) and -v 63, particularly the lines for each rule.
# Combines "Any" rules into one.  Flags those rules where the
# table rule probability does not equal the occurrence.

if ($#ARGV < 0)
{
  print "Usage: perl rules.pl file\n";
  exit;
}

set_distributions();

my $file = $ARGV[0];

my %store;

open my $fr, '<', $file or die "Can't open $file $!";
while (my $line = <$fr>)
{
  chomp $line;
  $line =~ s///g;

  next if ($line =~ /^Rules/);

  my @a = split /\s+/, $line;
  $store{$a[0]}[0] += $a[1]; # Hits
  $store{$a[0]}[1] += $a[2]; # Passes
  $store{$a[0]}[2] = $a[4]; # Rule probability in text file
  $store{$a[0]}[3] += $a[7]; # Occurrence mass of four passes
}
close $fr;

for my $rule (keys %store)
{
  next if $store{$rule}[0] == 0;
  my $occurrence = $store{$rule}[1] / $store{$rule}[0];
  my $ruleprob = $store{$rule}[2];

  next if $occurrence == 0. && $ruleprob == 0.;

  if (abs($ruleprob - $occurrence) > 0.001)
  {
    printf("%-55s%8d%8d%8.3f%8.3f%8.3f\n", 
      $rule,
      $store{$rule}[0],
      $store{$rule}[1],
      $occurrence,
      $store{$rule}[2],
      $occurrence - $store{$rule}[2]);
  }
}
