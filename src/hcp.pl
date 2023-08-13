#!perl

use strict;
use warnings;
use Scalar::Util 'looks_like_number';
use v5.10;

use lib '.';
use Distributions;

# Throw-away script that parses the output from reader with
# -Q 4-3-3-3 (say, but the distribution filter flag may be turned
# to false) and -v 63, getting the histograms for the HCP.

if ($#ARGV < 0)
{
  print "Usage: perl hcp.pl file\n";
  exit;
}

my @VUL_LIST = ( "None", "Both", "We", "They" );
my @RANGE_LIST = ( "weak", "mid", "str" );
my @PLAYER_POS = ( "First", "Second", "Third", "Fourth" );

set_distributions();

my $file = $ARGV[0];

my @store = ();

my ($dist, $ppos, $vul);

open my $fr, '<', $file or die "Can't open $file $!";
while (my $line = <$fr>)
{
  chomp $line;
  $line =~ s///g;

  if ($line =~ /^DISTRIBUTION (.+)/)
  {
    $dist = $1;
  }
  elsif ($line =~ /^Player pos.  : (\d)/)
  {
    $ppos = $PLAYER_POS[$1];
  }
  elsif ($line =~ /^Vulnerability: (\d)/)
  {
    $vul = $VUL_LIST[$1];
  }
  elsif ($line =~ /^HCP$/)
  {
    while ($line = <$fr>)
    {
      chomp $line;
      $line =~ s///g;
      last if $line =~ /^CCCC/;
      push @store, $line;
    }

    printf("%-14s%s\n", "Distribution", $dist);
    printf("%-14s%s\n", "Position", $ppos);
    printf("%-14s%s\n", "Vulnerability", $vul);

    for my $l (@store)
    {
      print "$l\n";
    }

    @store = ();
  }
}
close $fr;

