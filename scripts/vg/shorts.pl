#!perl

# Part of BridgeData.
#
# Copyright (C) 2016-17 by Soren Hein.
#
# See LICENSE and README.

use strict;
use warnings;

if ($#ARGV < 0)
{
  print "Usage: perl shorts.pl readerout.txt\n";
  exit;
}

my $numbers = `basename $ARGV[0]`;
$numbers =~ /(\d+)/;
$numbers = $1;

open my $zout, '>', "zskip$numbers.txt" or die "Can't open zskip.txt: $!";
get_files($ARGV[0]);
close $zout;
exit;


sub get_files
{
  my ($fname) = @_;

  open my $fh, '<', $fname or die "Can't open $fname: $!";

  my $fullname = '';
  my $lno = 0;
  my $bad = 0;
  my $count = 0;

  while (my $line = <$fh>)
  {
    $lno++;
    chomp $line;
    $line =~ s///g;

    if ($line =~ /^Out-short\s+1$/)
    {
      $bad = 0;
    }
    elsif ($line =~ /^Out \((\d+)\)/ && $1 > 10000)
    {
      $bad = 1;
    }
    elsif ($line =~ /^Error came from\s+(..)\s+(.+)$/)
    {
      $fullname = $2;
      if ($bad)
      {
        print $zout "$fullname\nskip\n\n";
        $count++;
	$bad = 0;
      }
    }
  }
  close $fh;
  print "Wrote $count suggestions\n";
}

