#!perl

use strict;
use warnings;

require "adder.pl";

# For BBO bug: qx|o1,BOARD1|

if ($#ARGV < 0)
{
  print "Usage: perl qxer.pl 07\n";
  exit;
}

# PC
my $DIR = "../../../bridgedata/hands/BBOVG";

# Laptop
# my $DIR = "../../../bridgedata/BBOVG";

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
  my $first = 1;
  while (my $line = <$fh>)
  {
    chomp $line;
    $line =~ s///g;
    $lno++;

    if ($line =~ s/(qx\|[oc]\d+),BOARD \d+/$1/)
    {
      $line =~ s/rh\|\|ah\|Board \d+/st|/;
      my $ref = "$lno replace \"$line\"";
      if ($first)
      {
        $first = 0;
	print "$fname\n";
      }
      addref($refname, $ref);
    }
  }
}

