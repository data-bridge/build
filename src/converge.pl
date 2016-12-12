#!perl

use strict;
use warnings;

if ($#ARGV < 0)
{
  print "Usage: perl converge.pl 31\n";
  exit;
}

my $chapter = $ARGV[0];
if ($chapter < 0 || $chapter > 99)
{
  print "$chapter is probably out of range\n";
  exit;
}

my $out = "cvg$chapter.txt";

my $flag = 0;
my ($wc0, $n0, $w0);
if (-e $out)
{
  $wc0 = `wc $out`;
  get_nw($wc0, \$n0, \$w0);
  $flag = 1;
  printf "%6d %6d\n", $n0, $w0;
}

while (1)
{
  my $indir = "../../../bridgedata/hands/BBOVG/0${chapter}000";
  system("./reader -I $indir -R $indir -s -c -v 30 -w 1 > $out");

  die "No output?" unless (-e $out);

  my ($n1, $w1);
  my $wc1 = `wc $out`;
  get_nw($wc1, \$n1, \$w1);

  if ($flag && $n1 == $n0 && $w1 == $w0)
  {
    print "$wc0";
    print "$wc1";
    last;
  }

  printf "%6d %6d\n", $n1, $w1;

  $flag = 1;
  $n0 = $n1;
  $w0 = $w1;
  $wc0 = $wc1;
}


sub get_nw
{
  my ($wc, $n, $w) = @_;

  $wc =~ s/^\s+//;
  my @a = split /\s+/, $wc;
  die "Bad wc line: $wc" unless $#a == 3;
  $$n = $a[0];
  $$w = $a[1];
}

