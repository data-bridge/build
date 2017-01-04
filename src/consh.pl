#!perl

use strict;
use warnings;

# PC
my $DIR = "../../../bridgedata/hands/BBOSH";

# Laptop
# my $DIR = "../../../bridgedata/BBOSH";

my $out = "cvg20xx.txt";

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
  system("./reader -I $DIR -R $DIR -s -c -v 30 -w 1 > $out");

  die "No output?" unless (-e $out);

  my ($n1, $w1);
  my $wc1 = `wc $out`;
  get_nw($wc1, \$n1, \$w1);

  skip_pbn($out);

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


sub skip_pbn
{
  my $fname = pop;
  open my $fh, '<', $fname or die "Can't open $fname: $!";
  my (@lines, @used);
  while (my $line = <$fh>)
  {
    chomp $line;
    $line =~ s///g;
    push @lines, $line;
    push @used, 1;
  }
  close $fh;

  my $pbn = 0;
  for (my $i = $#lines; $i >= 0; $i--)
  {
    my $line = $lines[$i];
    if ($line =~ /Failed to read/ && $line =~ /pbn$/)
    {
      $used[$i] = 0;
      $pbn = 1;
    }
    elsif ($line =~ /Error came from/ && $line =~ /pbn$/)
    {
      $used[$i] = 0;
      $pbn = 1;
    }
    elsif ($line =~ /Input file/ && $line =~ /pbn$/)
    {
      $used[$i] = 0;
      $pbn = 0;
    }
    elsif ($line =~ /Failed to read/)
    {
      $pbn = 0;
    }
    elsif ($line =~ /Error came from/)
    {
      $pbn = 0;
    }
    elsif ($pbn)
    {
      $used[$i] = 0;
    }
  }

  open my $fo, '>', $fname or die "Can't open $fname: $!";
  for my $i (0 .. $#lines)
  {
    if ($used[$i])
    {
      print $fo "$lines[$i]\n";
    }
  }
  close $fo;
}
