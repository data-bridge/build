#!perl

use strict;
use warnings;

require "adder.pl";

# For qx entries with 13 tricks played, creates a .ref entry
# to delete the mc|dd| tag.

if ($#ARGV < 0)
{
  print "Usage: perl tricker.pl 07\n";
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

  next if ($fname =~ /7867.lin/); # Singular BBO_VG bug...

  my $refname = $fname;
  $refname =~ s/\.lin/.ref/;

  open my $fh, '<', $fname or die "Can't open $fname: $!";
  my $lno = 0;
  my $count = 0;
  my $looking = 0;
  while (my $line = <$fh>)
  {
    chomp $line;
    $line =~ s///g;
    $lno++;

    # Could be some stray pc's on the same line before qx...
    my $lineseg = $line;
    if ($line =~ /qx\|([^|]+)\|/)
    {
      $looking = 0;
      $count = 0;
      $lineseg =~ s/^.*qx/qx/;
    }

    my @list = split /\|/, $lineseg;
    $count += grep(/^pc$/, @list);
    die "$fname $lno: $line" if ($count > 52);

    if ($count == 52)
    {
      $looking = 1;
    }

    if ($looking && $line =~ /mc\|\d+\|/)
    {
      $line =~ s/mc\|\d+\|//;
      my $out;
      if ($line =~ /^\s*$/ || $line =~ '^pg||')
      {
        $out = "$lno delete";
        # print "$fname\n$lno delete\n\n";
      }
      else
      {
        $out = "$lno replace \"$line\"";
        # print "$fname\n$lno replace \"$line\"\n\n";
      }
      print "$refname: $out\n";
      addref($refname, $out);
    }
  }
}

