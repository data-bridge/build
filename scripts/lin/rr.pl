#!perl 

use strict;
use warnings;

# >= 4 overlaps are extremely rare unless distributions are the same.

my $ITER = 10000;
my $MAX = 256;
my $NUMBERS = 16;

my @hist;
$hist[$_] = 0 for (0 .. $MAX-1);

for my $i (1 .. $ITER)
{
  my (@a, @b, @ha, @hb);
  $ha[$_] = 0, $hb[$_] = 0 for (0 .. $MAX-1);

  makerand(\@a);
  makerand(\@b);
  gethist(\@a, \@ha);
  gethist(\@b, \@hb);

  my $overlaps = get_overlap(\@ha, \@hb);
  $hist[$overlaps]++;
}

my $cum = 0.;
for (0 .. $MAX-1)
{
  $cum += $hist[$_];
  printf "%3d %7.3f%%\n", $_, 100. * $cum/$ITER if $hist[$_];
}
print "\n";

my $sum = 0;
$sum += $_ * $hist[$_] for (0 .. $MAX-1);
printf "Mean %6.2f\n", $sum / $ITER;


sub makerand
{
  my $vref = pop;
  for my $i (0 .. $NUMBERS)
  {
    $vref->[$i] = int rand $MAX;
  }
}


sub gethist
{
  my ($vref, $href) = @_;
  $href->[$_]++ for @$vref;
}


sub get_overlap
{
  my ($aref, $bref) = @_;

  my $overlap = 0;
  for my $o (0 .. $NUMBERS-1)
  {
    my $m = ($aref->[$o] < $bref->[$o] ? $aref->[$o] : $bref->[$o]);
    $overlap += $m;
  }
  return $overlap;
}
