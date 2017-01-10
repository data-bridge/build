#!perl

# Part of BridgeDate.
#
# Copyright (c) 2016-17 by Soren Hein.
#
# See LICENSE and README.
#
# Quick hack to compare two LIN files *almost* verbatim:
# - Line breaks are ignored
# - Case is ignored in values
# - Alert markers (!) in bids are ignored
# - All pg||'s are ignored

use strict;
use warnings;

die "No two input files" if ($#ARGV <= 0);
my $fn1 = $ARGV[0];
my $fn2 = $ARGV[1];

open my $fh1, '<', $fn1 or die "Can't < $fn1 $!";
open my $fh2, '<', $fn2 or die "Can't < $fn2 $!";

my $line1 = slurp_lin($fh1);
my $line2 = slurp_lin($fh2);

close $fh1;
close $fh2;

$line1 =~ s/nt\|[^|]*\|//g; # Remove chat
$line2 =~ s/nt\|[^|]*\|//g;
$line1 =~ s/pg\|\|//g; # Too many versions of page changes
$line2 =~ s/pg\|\|//g; # Too many versions of page changes

my @a1 = split /\|/, $line1;
my @a2 = split /\|/, $line2;

my $num_diffs = 0;

for (my $i = 0; $i <= $#a1; $i += 2)
{
  if ($i > $#a2)
  {
    print "Extra lines in first file\n";
    last;
  }

  if ($a1[$i] ne $a2[$i])
  {
    print "tag number $i: tag $a1[$i] vs $a2[$i]\n";
    $num_diffs++;
  }

  if (lc $a1[$i+1] ne lc $a2[$i+1])
  {
    if (length $a1[$i+1] > 20)
    {
      print "tag value  $i: tag $a1[$i+1]\n";
      print "               vs $a2[$i+1]\n";
    }
    elsif (length($a1[$i+1])+1 == length($a2[$i+1]))
    {
      if (! fixed_by_alert($a1[$i+1], $a2[$i+1]))
      {
        print "tag value  $i: tag $a1[$i+1] vs $a2[$i+1]\n";
      }
    }
    elsif (length($a1[$i+1]) == length($a2[$i+1])+1)
    {
      if (! fixed_by_alert($a2[$i+1], $a1[$i+1]))
      {
        print "tag value  $i: tag $a1[$i+1] vs $a2[$i+1]\n";
      }
    }
    else
    {
      print "tag value  $i: tag $a1[$i+1] vs $a2[$i+1]\n";
    }
    $num_diffs++;
  }

  last if ($num_diffs > 10);
}


sub slurp_lin
{
  my ($fh) = @_;

  my $res;
  while (my $line = <$fh>)
  {
    $line =~ s///g;
    $line =~ s/\n//g;
    $res .= $line;
  }
  $res
}


sub fixed_by_alert
{
  my ($shorter, $longer) = @_;

  my $shortened = substr $longer, 0, length $shorter;
  return (lc($shorter) eq lc($shortened));
}
