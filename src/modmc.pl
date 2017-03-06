#!perl

use strict;
use warnings;

# Modify .ref files (mc line) for one specific pattern of tricks,
# n-m-n (where n wins).  Only if at least 4th trick has been started.

if ($#ARGV < 0)
{
  print "Usage: perl modmc.pl dir (with .sht files)\n";
  exit;
}

# PC
my $DIR = "../../../bridgedata/hands/BBOVG";

# Laptop
# my $DIR = "../../../bridgedata/BBOVG";

my $indir = $ARGV[0];
my @files = glob "$indir/*.sht";

my ($chances, $hits) = (0, 0);
for my $file (@files)
{
  open my $fs, '<', $file or die "Can't open $file $!";
  my @refs;
  my ($header, $claim, $dd, $fourth) = (-1, -1, -1, -1);
  while (my $line = <$fs>)
  {
    chomp $line;
    $line =~ s///g;

    if ($line =~ /^Board/)
    {
      $header = -1;
      $claim = -1;
      $dd = -1;
      $fourth = -1;
    }
    elsif ($line =~ /^Header/)
    {
      die "$file: Bad header" unless $line =~ /Header\s*:\s*(\d+)/;
      $header = $1;
    }
    elsif ($line =~ /^Claim/)
    {
      die "$file: Bad header" unless $line =~ /Claim\s*:\s*(\d+)/;
      $claim = $1;
    }
    elsif ($line =~ /^DD/)
    {
      die "$file: Bad header" unless $line =~ /DD\s*:\s*(\d+)/;
      $dd = $1;
    }
    elsif ($line =~ /^4\. /)
    {
      $fourth = 0;
    }
    elsif ($line =~ /^Active ref/)
    {
      $chances++;
      next if $header == -1;
      next if $claim == -1;
      next if $dd == -1;
      next if $fourth == -1;
      next unless $header == $dd;
      next unless $header != $claim;

      $line = <$fs>;
      chomp $line;
      $line =~ s///g;
      next unless $line =~ /^(\d+) replaceLIN/;
      my $lno = $1;
      next unless $line =~ /mc/;

      $hits++;
      push @refs, $lno;
    }
  }
  close $fs;

  my $reflen = $#refs;
  next if $reflen < 0;

  my $rfile = sht2ref($file);
  open my $fr, '<', $rfile or die "Can't open $rfile $!";
  my $refno = 0;
  my @outlines;
  while (my $line = <$fr>)
  {
    chomp $line;
    $line =~ s///g;

    die "$rfile: Bad ref syntax" unless $line =~ /^(\d+)/;
    my $reflineno = $1;
    if ($refno <= $reflen && $reflineno == $refs[$refno])
    {
      die "$file, $reflineno: Already has ERR_LIN" if ($line =~ /ERR_LIN/);
      $line .= " {ERR_LIN_MC_CLAIM_WRONG(1,1,1)}";
      $refno++;
    }
    push @outlines, $line;
  }
  close $fr;

  # print "$rfile:\n";
  # for my $i (@outlines)
  # {
    # print "$i\n";
  # }
  # print "\n";

  open my $fo, '>', $rfile or die "Can't open $rfile: $!";
  for my $i (@outlines)
  {
    print $fo "$i\n";
  }
  close $fo;
}

print "Chances: $chances\n";
print "Hits   : $hits\n";


sub sht2ref
{
  my $fns = pop;
  die "Not a numbered file" unless $fns =~ /(\d+).sht$/;
  my $base = $1;

  my $dir;
  if ($base < 1000)
  {
    $dir = "$DIR/000000";
  }
  elsif ($base < 10000)
  {
    my $t = int($base/1000);
    $dir = "$DIR/00${t}000";
  }
  else
  {
    my $t = int($base/1000);
    $dir = "$DIR/0${t}000";
  }
  return "$dir/$base.ref";
}
