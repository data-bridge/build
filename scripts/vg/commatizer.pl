#!perl

use strict;
use warnings;

require "adder.pl";

# For too many or too few trailing commas in rs line.

if ($#ARGV < 0)
{
  print "Usage: perl commatizer.pl 07\n";
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

  next if $fname =~ /4092.lin/;
  next if $fname =~ /4594.lin/;
  next if $fname =~ /4596.lin/;
  next if $fname =~ /5763.lin/;
  next if $fname =~ /6449.lin/;
  next if $fname =~ /6450.lin/;

  my $refname = $fname;
  $refname =~ s/\.lin/.ref/;

  open my $fh, '<', $fname or die "Can't open $fname: $!";
  my $lno = 0;
  my $want = 0;
  while (my $line = <$fh>)
  {
    chomp $line;
    $line =~ s///g;
    $lno++;

    last if ($lno > 4);

    if ($line =~ /^vg/)
    {
      my @f = split /,/, $line;
      if ($#f != 8)
      {
        die "vg commas: $line";
      }
      $want = 2 * ($f[4] - $f[3]) + 1;
    }
    elsif ($line =~ /^rs\|/)
    {
      # Could be more than one tag on line.
      my @t = split /\|/, $line, -1;
      my $tmp = $t[1];

      my $count = $tmp =~ tr/,//;
      last if ($count == $want);
      last if ($want == 0);

      # print "$fname: Wanted $want, got $count\n";
      # print "Before: $tmp\n";
      if ($want > $count)
      {
        my $add = ',' x ($want - $count);
	substr($tmp, -2 + length $tmp, 0) = $add;
	# print "Adding\n";
      }
      else
      {
        my $delta = $count - $want;
        my $subtract = ',' x $delta;
	if (substr($tmp, -$delta + length $tmp, $delta) ne
	    $subtract)
	{
	  print "Would not be removing only commas\n";
	  last;
	}
	substr($tmp, -$delta + length $tmp, $delta) = '';
	# print "Removing\n";
      }

      $t[1] = $tmp;
      $line = $lno . " replace \"" . (join '|', @t) . "\"";
      # print "After:  $line\n";
      print "$fname\n";
      addref($refname, $line);
    }
  }
}

