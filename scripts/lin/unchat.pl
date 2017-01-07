#!perl

# Part of BridgeDate.
#
# Copyright (c) 2016 by Soren Hein.
#
# See LICENSE and README.
#
# Quick hack to remove comments from a BBO Vugraph file.

use strict;
use warnings;

die "No input file" if ($#ARGV == -1);
my $fname = $ARGV[0];

open my $fh, '<', $fname or die "Can't < $fname: $!";

while (my $line = <$fh>)
{
  $line =~ s/nt\|[^|]*\|pg\|\|//g;
  print $line unless $line =~ /^\s*$/;
}

close $fh;
