#!perl

use strict;
use warnings;

if ($#ARGV < 0)
{
  print "Usage: perl shorts.pl readerout.txt\n";
  exit;
}

get_files($ARGV[0]);
exit;


sub get_files
{
  my ($fname) = @_;

  open my $fh, '<', $fname or die "Can't open $fname: $!";

  my $fullname = '';
  my $lno = 0;
  my $bad = 0;

  while (my $line = <$fh>)
  {
    $lno++;
    chomp $line;
    $line =~ s///g;

    if ($line =~ /^Out-short\s+1$/)
    {
      $fullname = '';
      $bad = 0;
    }
    elsif ($line =~ /^Out \((\d+)\)/ && $1 > 10000)
    {
      $bad = 1;
    }
    elsif ($line =~ /^Error came from\s+(..)\s+(.+)$/)
    {
      $fullname = $2;
      print "$fullname\nskip\n\n";
    }
  }

  close $fh;
}

