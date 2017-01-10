#!perl

# Part of BridgeData.
#
# Copyright (C) 2016-17 by Soren Hein.
#
# See LICENSE and README.

use strict;
use warnings;

require "adder.pl";

# For BBO bug: Comment line ending on ...rs| becomes
# ...
# rs|

if ($#ARGV < 0)
{
  print "Usage: perl rser.pl 07\n";
  exit;
}

# PC
# my $DIR = "../../../bridgedata/hands/BBOVG";

# Laptop
my $DIR = "../../../bridgedata/BBOVG";

my $chapter = $ARGV[0];
if ($chapter < 0 || $chapter > 99)
{
  print "$chapter is probably out of range\n";
  exit;
}

my $indir = "$DIR/0${chapter}000";
# $indir = "ttest";
my $lintext = `ls $indir/*.lin`;
my @lins = split ' ', $lintext;

for my $fname (@lins)
{
  my $skipname = $fname;
  $skipname =~ s/\.lin/.skip/;
  next if (-e $skipname);

  my $refname = $fname;
  $refname =~ s/\.lin/.ref/;

  open my $fh, '<', $fname or die "Can't open $fname: $!";
  my $lno = 0;
  my $prevline = "";
  while (my $line = <$fh>)
  {
    chomp $line;
    $line =~ s///g;
    $lno++;

    if ($line =~ /^rs\|/ && $prevline =~ /[^|]$/)
    {
      my $p = $lno-1;
      my $ref1 = "$p replace \"" . $prevline . $line . "\"";
      my $ref2 = "$lno delete";
      print "$fname\n";
      addref($refname, $ref1);
      addref($refname, $ref2);
    }
    $prevline = $line;
  }
}

