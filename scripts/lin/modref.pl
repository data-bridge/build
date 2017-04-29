#!perl

use strict;
use warnings;

# Modify .ref files from replace to replaceLIN.

if ($#ARGV < 0)
{
  print "Usage: perl modref.pl number\n";
  exit;
}

# PC
my $DIR = "../../../bridgedata/hands/BBOVG";

# Laptop
# my $DIR = "../../../bridgedata/BBOVG";

my $file;
if ($ARGV[0] =~ /^\d+$/ && length($ARGV[0]) > 2)
{
  my $base = $ARGV[0];
  if ($base < 1000)
  {
    $file = "$DIR/000000/$base.ref";
  }
  elsif ($base < 10000)
  {
    my $t = int($base/1000);
    $file = "$DIR/00${t}000/$base.ref";
  }
  else
  {
    my $t = int($base/1000);
    $file = "$DIR/0${t}000/$base.ref";
  }
}
else
{
  print "Usage: perl modref.pl number\n";
  exit;
}

open my $fr, '<', $file or die "Can't open $file $!";
while (my $line = <$fr>)
{
  my @lines;
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

    my $delta = getdelta($oldline, $arg);
    if ($delta ne "")
    {
      $line = "$lno replace \"$delta\"";
      print "$file:\n";
      print "$arg\n";
      print "$delta\n\n";
    }
  }
  else
  {
    push @lines, $line;
  }

}
close $fr;

# open my $fo, '>', $file or die "Can't open $file $!";


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


sub getdelta
{
  my ($oldline, $newline) = @_;

  my @oldlist = split /\|/, $oldline, -1;
  my @newlist = split /\|/, $newline, -1;

  return "" unless $#oldlist == $#newlist;

  my $hit = 0;
  my $index;
  for my $i (0 .. $#oldlist)
  {
    next if $oldlist[$i] eq $newlist[$i];
    $hit++;
    $index = $i;
  }

  return "" unless ($hit == 1 && ($index % 2 == 1));

  my $tag = $oldlist[$index-1];
  return "" if $tag eq "rs";

  my $n = ($index+1)/2;
  my $res = "$n,$tag,$oldlist[$index],$newlist[$index]";
  return $res;
}
