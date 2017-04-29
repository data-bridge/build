#!perl

use strict;
use warnings;

# Modifies .ref files with an overall rs replace into individual
# contracts (replaceLIN).

if ($#ARGV < 0)
{
  print "Usage: perl rsmod.pl file.ref|dir|30|all\n";
  exit;
}

my $HOMEDIR = glob("~/GitHub/Build/src");

# PC
my $DIR = "../../../bridgedata/hands/BBOVG";

# Laptop
# my $DIR = "../../../bridgedata/BBOVG";

my @files;
if ($ARGV[0] =~ /.ref$/)
{
  push @files, $ARGV[0];
}
else
{
  my $indir = $ARGV[0];
  if ($indir =~ /^\d+$/)
  {
    $indir = "$DIR/0${indir}000";
  }

  $indir = "$DIR/*" if ($indir eq "all");

  @files = glob("$indir/*.ref");
}

for my $file (@files)
{
  my $linfile = $file;
  $linfile =~ s/.ref$/.lin/;

  my @lines;

  my $flag = 0;
  open my $fr, '<', $file or die "Can't open $file $!";
  while (my $line = <$fr>)
  {
    chomp $line;
    $line =~ s///g;
    next if ($line =~ /^\s*$/);

    if ($line =~ /^(\d+)\s+replace\s+\"rs\|(.*)\|\"/)
    {
      my $lno = $1;
      my $arg = $2;

      my $linline = getLINline($linfile, $lno);
      if ($linline !~ /^rs\|([^|]*)\|$/)
      {
        warn "$linfile: Expected rs and no pipes in $linline";
        next;
      }
      my $linarg = $1;

      my @slines;
      next unless getdiffs($lno, $arg, $linarg, \@slines);

      push @lines, @slines;

      $flag = 1;
      print "$file:\n";
      print "Old line: '$linline'\n";
      print "Ref line: '$line'\n";
      for my $sline (@slines)
      {
        print "$sline\n";
      }
      print "\n";

    }
    else
    {
      push @lines, $line;
    }
  }
  close $fr;

  next unless $flag;
# next;

  open my $fs, '>', $file or die "Can't open $file $!";
  for my $sline (@lines)
  {
    print $fs "$sline\n";
  }
  close $fs;
}


sub getLINline
{
  my ($file, $lno) = @_;
  open my $fl, '<', $file or die "Can't open $file $!";
  my $n = 1;
  while (my $line = <$fl>)
  {
    chomp $line;
    $line =~ s///g;
    if ($n == $lno)
    {
      return $line;
    }
    else
    {
      $n++;
    }
  }
  close $fl;
}


sub getdiffs
{
  my ($lno, $argref, $arglin, $listref) = @_;

  my @reflist = split ',', $argref, -1;
  my @linlist = split ',', $arglin, -1;

  my $nr = $#reflist;
  my $nl = $#linlist;
  my $nmin = ($nr < $nl ? $nr : $nl);

  if ($nr > $nl)
  {
    for my $i ($nl+1 .. $nr)
    {
      return 0 if ($reflist[$i] ne "");
    }
  }
  elsif ($nr < $nl)
  {
    for my $i ($nr+1 .. $nl)
    {
      return 0 if ($linlist[$i] ne "");
    }
  }

  my $flag = 0;
  for my $i (0 .. $nmin)
  {
    if ($reflist[$i] ne $linlist[$i])
    {
      my $j = $i+1;
      my $line = "$lno replaceLIN \"1,$j,rs,$linlist[$i],$reflist[$i]\"";
      push @$listref, $line;
      $flag = 1;
    }
  }
  return $flag;
}

