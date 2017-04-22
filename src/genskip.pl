#!perl

use strict;
use warnings;

# Looks at .lin file and suggests a skip text.

# my $reason = "ERR_LIN_PN_PLAYERS_WRONG";
my $reason = "ERR_LIN_QX_UNORDERED";

if ($#ARGV < 0)
{
  print "Usage: perl genskip.pl number\n";
  exit;
}

my $DIR;
if (`uname -a` =~ /CDD/)
{
  # Laptop
  $DIR = "../../../bridgedata/BBOVG";
}
else
{
  # PC
  $DIR = "../../../bridgedata/hands/BBOVG";
}

my $file;
if ($ARGV[0] =~ /^\d+$/ && length($ARGV[0]) > 2)
{
  my $base = $ARGV[0];
  if ($base < 1000)
  {
    $file = "$DIR/000000/$base.lin";
  }
  elsif ($base < 10000)
  {
    my $t = int($base/1000);
    $file = "$DIR/00${t}000/$base.lin";
  }
  else
  {
    my $t = int($base/1000);
    $file = "$DIR/0${t}000/$base.lin";
  }
}
else
{
  print "Usage: perl genskip.pl number\n";
  exit;
}

my $qx = 0;
my $bd = 0;
my $ln = 0;
my $prev = "";
my $curr;

open my $fr, '<', $file or die "Can't open $file $!";
while (my $line = <$fr>)
{
  $ln++;
  next unless $line =~ /qx\|([^,\|]+)/;
  $curr = $1;
  $curr = substr $curr, 1;

  $qx++;
  $bd++ if ($curr ne $prev);
  $prev = $curr;
}
close $fr;

print "$ln {$reason(0,$qx,$bd)}\n";
