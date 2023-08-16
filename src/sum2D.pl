#!perl

use strict;
use warnings;
use Scalar::Util 'looks_like_number';
use v5.10;

use lib '.';
use Distributions;

# Throw-away script that parses the output of ./reader (or the
# part that contains distributions and tables).
# Unlike sumcomp, this one does 2D correlation tables.
# If called with a second argument "all", also sums up the positions.
# Useful for an overview when the data material is sparse.

if ($#ARGV != 0 && $#ARGV != 1)
{
  print "Usage: perl sum2D.pl file [all]\n";
  exit;
}

my $file = $ARGV[0];
my $all_flag = ($#ARGV == 1 ? 1 : 0);

my %totals;
my %currents;
my ($dist, $pos, $vul, $component);
my (@xvalues_cum, @yvalues_cum);

my @fields = (
  "HCP vs. Spades", 
  "HCP vs. Controls", 
  "HCP vs. Short HCP", 
  "HCP vs. Long HCP", 
  "HCP vs. Long12 HCP", 
  "Spades vs. Controls", 
  "Spades vs. Short HCP", 
  "Spades vs. Long HCP", 
  "Spades vs. Long12 HCP", 
  "Controls vs. Short HCP", 
  "Controls vs. Long HCP", 
  "Controls vs. Long12 HCP", 
  "Short HCP vs. Long HCP",
  "Short HCP vs. Long12 HCP",
  "Long HCP vs. Long12 HCP");

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
  elsif ($line =~ "HCP vs" ||
      $line =~ "Spades vs" ||
      $line =~ "Controls vs" ||
      $line =~ "Short HCP vs" ||
      $line =~ "Long HCP vs" ||
      $line =~ "Long12 HCP vs")
  {
    chomp $line;
    $line =~ s///g;
    $component = $line;

    $line = <$fr>; # Empty
    $line = <$fr>; # Header
    chomp $line;
    $line =~ s///g;

    my @yvalues = split /\s+/, $line;
    $line = <$fr>; # Not needed

    # First table: Totals.

    while ($line = <$fr>)
    {
      chomp $line;
      $line =~ s///g;
      last if $line =~ /^\s*$/;
      my @a = split /\s+/, $line;

      my $xvalue = $a[1];
      $xvalues_cum[$xvalue] = $xvalue;
      for my $y (1 .. $#yvalues)
      {
        my $yvalue = $yvalues[$y];
        $yvalues_cum[$yvalue]= $yvalue;
        my $total = $a[3+$y-1];

        if ($all_flag)
        {
          $totals{$dist}{$component}[$xvalue][$yvalue]{total} += $total;
          $currents{$dist}{$component}[$xvalue][$yvalue]{total} = $total;
        }
        else
        {
          $totals{$dist}{$pos}{$component}[$xvalue][$yvalue]{total} += 
            $total;
          $currents{$dist}{$pos}{$component}[$xvalue][$yvalue]{total} = 
            $total;
        }
      }
    }

    # Second table: Percentages.
    $line = <$fr>; # Name
    $line = <$fr>; # Empty
    $line = <$fr>; # Y values
    $line = <$fr>; # Header

    while ($line = <$fr>)
    {
      chomp $line;
      $line =~ s///g;
      last if $line =~ /^\s*$/;
      my @a = split /\s+/, $line;

      my $xvalue = $a[1];
      for my $y (1 .. $#yvalues)
      {
        my $yvalue = $yvalues[$y];
        my $perc = $a[3+$y-1];

        my $frac;
        if ($perc eq '-')
        {
          $frac = 0.;
        }
        else
        {
          $perc =~ /(\d+)%/;
          $frac = $1 / 100.;
        }

        if ($all_flag)
        {
          my $passes = $frac * 
            $currents{$dist}{$component}[$xvalue][$yvalue]{total};

          $totals{$dist}{$component}[$xvalue][$yvalue]{passes} += $passes;
        }
        else
        {
          my $passes = $frac * 
            $currents{$dist}{$pos}{$component}[$xvalue][$yvalue]{total};

          $totals{$dist}{$pos}{$component}[$xvalue][$yvalue]{passes} += 
            $passes;
        }
      }
    }
  }
}
close $fr;

for my $dist_short (@DISTRIBUTIONS)
{
  my $dist = "DISTRIBUTION $dist_short";
  next unless defined $totals{$dist};
  print "$dist\n";

  if ($all_flag)
  {
    print "\n";
    for my $component (@fields)
    {
      next unless defined $totals{$dist}{$component};
      print "$component\n\n";

      make_percentages(\@{$totals{$dist}{$component}});
      print_table(\@{$totals{$dist}{$component}});
      print "\n";
    }
  }
  else
  {
    for my $pos (sort keys %{$totals{$dist}})
    {
      print "$pos\n\n";
      for my $component (@fields)
      {
        next unless defined $totals{$dist}{$pos}{$component};
        print "$component\n\n";

        make_percentages(\@{$totals{$dist}{$pos}{$component}});
        print_table(\@{$totals{$dist}{$pos}{$component}});
        print "\n";
      }
    }
  }
}


sub make_percentages
{
  my ($tref) = @_;

  for my $xvalue (@xvalues_cum)
  {
    my $xsum = 0;
    next unless defined $xvalue;
    next unless defined $tref->[$xvalue];

    for my $yvalue (@yvalues_cum)
    {
      if (defined $yvalue &&
          defined $tref->[$xvalue][$yvalue] &&
          $tref->[$xvalue][$yvalue]{total} > 0.1)
      {
        $tref->[$xvalue][$yvalue]{perc} = int(
          100. * $tref->[$xvalue][$yvalue]{passes}  /
          $tref->[$xvalue][$yvalue]{total});
      }
      else
      {
        $tref->[$xvalue][$yvalue]{perc} = 0;
      }
    }
  }
}


sub print_table
{
  my ($tref) = @_;

  for my $field (qw(total passes perc))
  {
    # Print header
    print " Value";
    for my $yvalue (@yvalues_cum)
    {
      printf("%6d", $yvalue);
    }
    printf("%6s\n", "SUM");

    my @ysum;

    # Print lines
    for my $xvalue (@xvalues_cum)
    {
      my $xsum = 0;
      next unless defined $xvalue;
      next unless defined $tref->[$xvalue];
      printf("%6d", $xvalue);

      for my $yvalue (@yvalues_cum)
      {
        if (defined $yvalue &&
            defined $tref->[$xvalue][$yvalue]{total} &&
            $tref->[$xvalue][$yvalue]{total} > 0.1)
        {
          if ($field eq 'perc')
          {
            if ($tref->[$xvalue][$yvalue]{$field} > 0)
            {
              printf("%5d%%", $tref->[$xvalue][$yvalue]{$field});
            }
            else
            {
              printf("%6s", "-");
            }
          }
          else
          {
            printf("%6d", $tref->[$xvalue][$yvalue]{$field});
          }

          $xsum += $tref->[$xvalue][$yvalue]{$field};
          $ysum[$yvalue] += $tref->[$xvalue][$yvalue]{$field};
        }
        else
        {
          printf("%6s", "-");
        }
      }
      printf("%6d\n", $xsum);
    }

    my $dashes = 6 * (3 + $#yvalues_cum);
    print "-" x $dashes, "\n";

    printf("%6s", "SUM");
    for my $yvalue (@yvalues_cum)
    {
      if (defined $ysum[$yvalue])
      {
        printf("%6d", $ysum[$yvalue]);
      }
      else
      {
        printf("%6s", "-");
      }
    }
    print "\n\n";
  }
}

