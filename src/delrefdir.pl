#!perl

use strict;
use warnings;

# Modify .ref files from replace to deleteLIN.

if ($#ARGV < 0)
{
  print "Usage: perl delrefdir.pl number\n";
  exit;
}

# PC
my $DIR = "../../../bridgedata/hands/BBOVG";

# Laptop
# my $DIR = "../../../bridgedata/BBOVG";

my $indir = $ARGV[0];
if ($indir =~ /^\d+$/)
{
  $indir = "$DIR/0${indir}000";
}

$indir = "$DIR/*" if ($indir eq "all");
my @files = glob "$indir/*.ref";

for my $file (@files)
{
  open my $fr, '<', $file or die "Can't open $file $!";
  my @lines;
if ($file =~ /2019/)
{
  print "HERE\n";
}
  while (my $line = <$fr>)
  {
    chomp $line;
    $line =~ s///g;

    if ($line !~ /LIN/ && $line =~ /\sreplace\s/)
    {
      my $linfile = $file;
      $linfile =~ s/ref$/lin/;

      my ($lno, $arg);
      if ($line =~ /^(\d+)\s+replace\s+\"([^"]*)\"\s*$/)
      {
        $lno = $1;
        $arg = $2;
      }
      else
      {
        die "$file, $line: Bad syntax";
      }

      die "$file: no $linfile" unless -e $linfile;
      my $oldline = getline($linfile, $lno);

      my $delta = getins($arg, $oldline);
      if ($delta ne "")
      {
        $line = "$lno deleteLIN \"$delta\"";
        print "$file:\n";
        print "$arg\n";
        print "NEW '$line'\n";
        print "$delta\n\n";
      }
    }
    push @lines, $line;
  }
  close $fr;

  # open my $fo, '>', $file or die "Can't open $file $!";
  for my $ll (@lines)
  {
    # print $fo "$ll\n";
    print "$file: $ll\n";
  }
  # close $fo;
}


sub getline
{
  my ($fn, $lno) = @_;
  open my $fr, '<', $fn or die "Can't open $fn $!";

  my $running = 0;
  while (my $line = <$fr>)
  {
    chomp $line;
    $line =~ s///g;
    $running++;

    if ($running == $lno)
    {
      close $fr;
      return $line;
    }
  }
  close $fr;
  die "$fn, $lno: Not found";
}


sub getins
{
  my ($oldline, $newline) = @_;

  my @oldlist = split /\|/, $oldline, -1;
  my @newlist = split /\|/, $newline, -1;

  return "" unless $#oldlist+2 == $#newlist;

  my $hit = 0;
  my $indexlo;
  for my $i (0 .. $#oldlist)
  {
    next if $oldlist[$i] eq $newlist[$i];
    $hit++;
    $indexlo = $i;
    last;
  }

  return "" unless $hit == 1;

  $hit = 0;
  my $indexhi;
  for my $i (reverse 2 .. $#newlist)
  {
    next if $oldlist[$i-2] eq $newlist[$i];
    $hit++;
    $indexhi = $i;
    last;
  }

  return "" unless $hit == 1;
  my ($tag, $val);
  if ($indexhi == $indexlo+1)
  {
    $tag = $newlist[$indexlo];
    $val = $newlist[$indexhi];
  }
  elsif ($indexhi == $indexlo)
  {
    $tag = $newlist[$indexlo-1];
    $val = $newlist[$indexhi];
  }
  else
  {
    return "";
  }

  return "" if $tag eq "rs";
  return "" if $tag eq "vg";
  return "" if $tag eq "pn";
  return "" if $tag eq "pw";
  return "" if $tag eq "mp";
  return "" if $tag eq "bn";

  my $n = ($indexhi+1)/2;
  my $res = "$n,$tag,$val";
  return $res;
}
