#!perl

use strict;
use warnings;

# Detects the pattern of a replace line followed by a delete line.

if ($#ARGV < 0)
{
  print "Usage: perl refpat.pl dir|30|all\n";
  exit;
}

my $HOMEDIR = glob("~/GitHub/Build/src");
my $out = "refpat.txt";

# PC
my $DIR = "../../../bridgedata/hands/BBOVG";

# Laptop
# my $DIR = "../../../bridgedata/BBOVG";

my $indir = $ARGV[0];
if ($indir =~ /^\d+$/)
{
  $out = "pat${indir}.txt";
  $indir = "$DIR/0${indir}000";
}

$indir = "$DIR/*" if ($indir eq "all");
my @files = glob("$indir/*.ref");
my @candidates;
my $cnext = 0;

foreach my $file (@files)
{
  open my $fr, '<', $file or die "Can't open $file: $!";

  my $state = 0;
  my $lno = -2;
  my $repl;
  while (my $line = <$fr>)
  {
    chomp $line;
    $line =~ s///g;
    next if ($line =~ /ERR_LIN/);

    if ($line =~ /^(\d+)\s+replace/)
    {
      $lno = $1;
      if ($line =~ /[^|"]rs\|/)
      {
        $state = 1;
        $repl = $line;
      }
    }
    elsif ($line =~ /^(\d+)\s+delete$/)
    {
      my $lno2 = $1;
      if ($state == 1 && $lno2 == $lno+1)
      {
        $candidates[$cnext]{file} = $file;
        $candidates[$cnext]{line1} = $lno;
        $candidates[$cnext]{line2} = $lno2;
        $cnext++;
      }
      $state = 0;
    }
  }
  close $fr;
}


for my $cr (@candidates)
{
  print "File $cr->{file} $cr->{line1} $cr->{line2}\n";
  
  my @lines;
  my $file = $cr->{file};
  open my $fr, '<', $file or die "Can't open $file: $!";
  while (my $line = <$fr>)
  {
    chomp $line;
    $line =~ s///g;
    push @lines, $line;
  }
  close $fr;

  open my $fs, '>', $file or die "Can't open $file: $!";
  for my $line (@lines)
  {
    die "Bad line in $file" unless ($line =~ /^(\d+)\s+/);
    my $lno = $1;
    next if ($lno == $cr->{line1});
    next if ($lno == $cr->{line2});
    print "$file\n";
    print $fs "$line\n";
  }
  close $fs;
}

