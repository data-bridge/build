#!perl

use strict;
use warnings;

# Looks at .ref files with an overall replace and suggests replaceLIN.

if ($#ARGV < 0)
{
  print "Usage: perl genmod.pl number|file.ref|dir|30|all\n";
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
elsif ($ARGV[0] =~ /^\d+$/ && length($ARGV[0]) > 3)
{
  my $base = $ARGV[0];
  if ($base < 1000)
  {
    push @files, "$DIR/000000/$base.ref";
  }
  elsif ($base < 10000)
  {
    my $t = int($base/1000);
    push @files, "$DIR/00${t}000/$base.ref";
  }
  else
  {
    my $t = int($base/1000);
    push @files, "$DIR/0${t}000/$base.ref";
  }
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

    if ($line =~ /^(\d+)\s+replace\s+\"(\w\w)\|(.*\|)\"/)
    {
      my $lno = $1;
      my $tag = $2;
      my $arg = $tag . "|" . $3;

      my $linline = getLINline($linfile, $lno);
      if ($linline !~ /^$tag\|/)
      {
        warn "$linfile: Expected $tag and no pipes in $linline";
        next;
      }

      my @slines;
      next unless getdiffs($lno, $arg, $linline, \@slines);

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
next;

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

  my @reflist = split /\|/, $argref, -1;
  my @linlist = split /\|/, $arglin, -1;

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
      if ($i % 2 == 0)
      {
        next;
      }
      my $j = ($i+1) / 2;
      my $tag = $linlist[$i-1];
      my $line = "$lno replaceLIN \"$j,$tag,$linlist[$i],$reflist[$i]\"";
      push @$listref, $line;
      $flag = 1;
    }
  }
  return $flag;
}

