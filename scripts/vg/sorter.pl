#!perl

use strict;
use warnings;

# Sorts the qx entries in place, creating an .orig file
# and deleting the .ref file

if ($#ARGV < 0)
{
  print "Usage: perl sorter.pl 07\n";
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
my $lintext = `ls $indir/*.lin`;
my @lins = split ' ', $lintext;

for my $fname (@lins)
{
  my $skipname = $fname;
  $skipname =~ s/\.lin/.skip/;
  next if (-e $skipname);

  my (@lines, @blist);
  slurp_file($fname, \@lines, \@blist);
  next if (in_order(\@blist));

  my @ordered = sort gt_sort @blist;
  print "$fname\n";

  # A .ref file will have line numbers that may get jumbled.
  my $refname = $fname;
  $refname =~ s/\.lin/.ref/;
  if (-e $refname)
  {
    unlink $refname;
  }

  my $origname = $fname;
  $origname =~ s/\.lin/.orig/;
  rename $fname, $origname;

  print_ordered_file($fname, \@lines, \@ordered, $blist[0]{start});
}

sub slurp_file
{
  my ($fname, $linesref, $blistref) = @_;

  # Read the unadulterated file.
  open my $fh, '<', $fname or die "Can't open $fname: $!";
  my $lno = -1;
  my $bno = 0;
  while (my $line = <$fh>)
  {
    chomp $line;
    $line =~ s///g;
    push @$linesref, $line;
    $lno++;

    if ($line =~ /^qx\|([^|]+)\|/)
    {
      my $q = $1;
      $q =~ s/,BOARD \d+//;
      $blistref->[$bno]{name} = $q;
      $blistref->[$bno]{start} = $lno;
      if ($bno > 0)
      {
        $blistref->[$bno-1]{end} = $lno-1;
      }
      $bno++;
    }
  }
  close $fh;

  if ($bno > 0)
  {
    $blistref->[$bno-1]{end} = $lno;
  }
}


sub in_order
{
  my ($blistref) = @_;

  for my $i (0 .. $#$blistref-1)
  {
    if (! less_than($blistref->[$i]{name}, $blistref->[$i+1]{name}))
    {
      return 0;
    }
  }
  return 1;
}


sub less_than
{
  my ($qx1, $qx2) = @_;

  my $r1 = substr $qx1, 0, 1;
  my $r2 = substr $qx2, 0, 1;

  my $n1 = substr $qx1, 1;
  my $n2 = substr $qx2, 1;

  if ($n1 < $n2 || ($n1 == $n2 && $r1 eq 'o'))
  {
    return 1;
  }
  else
  {
    return 0;
  }
}


sub gt_sort
{
  if (less_than($b->{name}, $a->{name}))
  {
    return 1;
  }
  else
  {
    return -1;
  }
}


sub print_ordered_file
{
  my ($fname, $linesref, $blistref, $start) = @_;

  open my $fh, '>', $fname or die "Can't open $fname: $!";

  for my $i (0 .. $start-1)
  {
    print $fh "$linesref->[$i]\n";
  }

  for my $b (0 .. $#$blistref)
  {
    for my $i ($blistref->[$b]{start} .. $blistref->[$b]{end})
    {
      print $fh "$linesref->[$i]\n";
    }
  }
  close $fh;
}

