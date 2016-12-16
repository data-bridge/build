#!perl

use strict;
use warnings;

if ($#ARGV < 0)
{
  print "Usage: perl tofix.pl zfix33.txt\n";
  exit;
}

my $fname = $ARGV[0];
open my $fh, '<', $fname or die "Can't open $fname: $!";

my $lno = 0;
my $change = "";
my $pre = "";
my $skip = "";
while (my $line = <$fh>)
{
  $lno++;
  chomp $line;
  $line =~ s///g;


  if ($line =~ /EDIT/)
  {
    die "$lno $line: Should have edited";
  }
  elsif ($line =~ /^(.*)\.lin$/)
  {
    $pre = $1 . '.ref';
    $skip = $1 . '.skip';
  }
  elsif ($line eq "")
  {
    if ($change eq "skip")
    {
      `touch $skip`;
      # print "touch $skip\n\n";
      if (-e $pre)
      {
        unlink($pre);
      }
    }
    elsif (-e $pre)
    {
      # print "extend $pre\n";
      enter_line($pre, $change);
      # print "\n";
    }
    else
    {
      open my $ff, '>', $pre or die "Can't open $pre $!";
      print $ff "$change\n";
      close $ff;
    }
  }
  else
  {
    $change = $line;
  }
}

close $fh;


sub enter_line
{
  my ($fname, $line) = @_;

  # Slurp
  my @lines;
  open my $ff, '<', $fname or die "Can't open $fname: $!";
  while (my $l = <$ff>)
  {
    chomp $l;
    $l =~ s///g;
    push @lines, $l;
  }
  close $ff;

  die "Syntax error: $line" unless ($line =~ /^(\d+)/);
  my $fno = $1;

  open my $fo, '>', $fname or die "Can't open $fname: $!";

  my $seen = 0;
  for my $loop (@lines)
  {
    $loop =~ /^(\d+)/;
    my $rno = $1;
    if ($rno < $fno)
    {
      print $fo $loop, "\n";
    }
    elsif ($rno == $fno)
    {
      die "Confused: $rno same as $fno";
    }
    elsif ($seen)
    {
      print $fo $loop, "\n";
    }
    else
    {
      $seen = 1;
      print $fo $line, "\n";
      print $fo $loop, "\n";
    }
  }

  if (! $seen)
  {
    print $fo $line, "\n";
  }

  close $fo;
}
