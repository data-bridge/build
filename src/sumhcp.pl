#!perl

use strict;
use warnings;
use Scalar::Util 'looks_like_number';

# Throw-away script that parses the output from hcp.pl and
# summarizes all positions and vulnerabilities.  Useful for an
# overview when the data material is sparse.

if ($#ARGV != 1)
{
  print "Usage: perl dist sumhcp.pl file\n";
  exit;
}

my $DIST = $ARGV[0];
my $file = $ARGV[1];

my $flag = 0;

my @store;
my $sumtotal = 0;
my $passtotal = 0;

open my $fr, '<', $file or die "Can't open $file $!";
while (my $line = <$fr>)
{
  chomp $line;
  $line =~ s///g;

  if ($line =~ /^Distribution\s+$DIST/)
  {
    $flag = 1;
  }
  elsif ($line =~ /^Distribution\s+/)
  {
    $flag = 0;
  }
  elsif ($flag && $line =~ /^ Value Count/)
  {
    while ($line = <$fr>)
    {
      chomp $line;
      $line =~ s///g;
      last if $line =~ /----/;
      my @a = split /\s+/, $line;

      my $hcp = $a[1];
      my $total = $a[2];
      my $passes = $a[4];

      $store[$hcp]{total} += $total;
      $store[$hcp]{passes} += $passes;

      $sumtotal += $total;
      $passtotal += $passes;
    }
  }
}
close $fr;

for my $hcp (0 .. 40)
{
  next unless defined $store[$hcp];
  next unless $store[$hcp]{total};

  printf("%6d%6d%7.2f%%%6d%7.2f%%\n",
    $hcp, 
    $store[$hcp]{total},
    100. * $store[$hcp]{total} / $sumtotal,
    $store[$hcp]{passes},
    100. * $store[$hcp]{passes} / $store[$hcp]{total});
}

print "-" x 34, "\n";
printf("%6s%6d%8s%6d\n","", $sumtotal, "", $passtotal);
