#!perl

use strict;
use warnings;
use Scalar::Util 'looks_like_number';
use v5.10;

use lib '.';
use Distributions;

# Throw-away script that parses the output of ./reader (or the
# part that contains distributions and tables).
# By default summarizes each component (HCP, CCCC etc.) for each
# position separately, adding up the vulnerabilities.  If
# called with a second argument "all", also sums up the positions.
# Useful for an overview when the data material is sparse.

if ($#ARGV != 0 && $#ARGV != 1)
{
  print "Usage: perl sumcomp.pl file [all]\n";
  exit;
}

my $file = $ARGV[0];
my $all_flag = ($#ARGV == 1 ? 1 : 0);

my (%store, %sumtotal, %passtotal);
my ($dist, $pos, $vul, $component);

my @fields = ("HCP", "CCCC", "ZP", "Spades", "Controls", "Short HCP",
  "Long HCP", "Long12 HCP");

set_distributions();

open my $fr, '<', $file or die "Can't open $file $!";
while (my $line = <$fr>)
{
  chomp $line;
  $line =~ s///g;

  if ($line =~ /^DISTRIBUTION+/)
  {
    $dist = $line;
  }
  elsif ($line =~ /^Player pos/)
  {
    $pos = $line;
  }
  elsif ($line =~ /^Vulnerability/)
  {
    $vul = $line;
  }
  elsif ($line =~ "HCP" ||
      $line =~ "CCCC" ||
      $line =~ "ZP" ||
      $line =~ "Spades" ||
      $line =~ "Controls" ||
      $line =~ "Short HCP" ||
      $line =~ "Long HCP" ||
      $line =~ "Long12 HCP")
  {
    $component = $line;
  }
  elsif ($line =~ /^ Value Count/)
  {
    while ($line = <$fr>)
    {
      chomp $line;
      $line =~ s///g;
      last if $line =~ /----/;
      my @a = split /\s+/, $line;

      my $value = $a[1];
      my $total = $a[2];
      my $passes = $a[4];

      if ($all_flag)
      {
        $store{$dist}{$component}{$value}{total} += $total;
        $store{$dist}{$component}{$value}{passes} += $passes;
        $sumtotal{$dist}{$component} += $total;
        $passtotal{$dist}{$component} += $passes;
      }
      else
      {
        $store{$dist}{$pos}{$component}{$value}{total} += $total;
        $store{$dist}{$pos}{$component}{$value}{passes} += $passes;
        $sumtotal{$dist}{$pos}{$component} += $total;
        $passtotal{$dist}{$pos}{$component} += $passes;
      }
    }
  }
}
close $fr;

for my $dist_short (@DISTRIBUTIONS)
{
  my $dist = "DISTRIBUTION $dist_short";
  next unless defined $store{$dist};
  print "$dist\n";

  if ($all_flag)
  {
    print "\n";
    for my $component (@fields)
    {
      next unless defined $store{$dist}{$component};
      print "$component\n\n";

      print_table(\%{$store{$dist}{$component}}, 
        $sumtotal{$dist}{$component}, 
        $passtotal{$dist}{$component},
        $component eq "CCCC");
      print "\n";
    }
  }
  else
  {
    for my $pos (sort keys %{$store{$dist}})
    {
      print "$pos\n\n";
      for my $component (@fields)
      {
        next unless defined $store{$dist}{$pos}{$component};
        print "$component\n\n";

        print_table(\%{$store{$dist}{$pos}{$component}},
          $sumtotal{$dist}{$pos}{$component},
          $passtotal{$dist}{$pos}{$component},
          $component eq "CCCC");
        print "\n";
      }
    }
  }
}


sub print_table
{
  my ($tref, $sumtotal, $passtotal, $float_flag) = @_;

  print " Value Count       %  Hits       %\n";
  for my $value (sort keys %$tref)
  {
    next unless defined $tref->{$value};
    next unless $tref->{$value}{total} > 0;
  
    if ($float_flag)
    {
      printf("%6.2f%6d%7.2f%%%6d%7.2f%%\n",
        $value, 
        $tref->{$value}{total},
        100. * $tref->{$value}{total} / $sumtotal,
        $tref->{$value}{passes},
        100. * $tref->{$value}{passes} / $tref->{$value}{total});
    }
    else
    {
      printf("%6d%6d%7.2f%%%6d%7.2f%%\n",
        $value, 
        $tref->{$value}{total},
        100. * $tref->{$value}{total} / $sumtotal,
        $tref->{$value}{passes},
        100. * $tref->{$value}{passes} / $tref->{$value}{total});
    }
  }

  print "-" x 34, "\n";
  printf("%6s%6d%8s%6d\n","", $sumtotal, "", $passtotal);
}

