#!perl

use strict;
use warnings;
use Scalar::Util 'looks_like_number';
use v5.10;

use lib '.';
use Distributions;

# Throw-away script that makes a histogram of opening categories
# from an output file otherwise suitable for Python.

if ($#ARGV != 0)
{
  print "Usage: perl openhist.pl file.txt\n";
  exit;
}

my $file = $ARGV[0];

my (@SUIT_NAMES, @SUIT_VALUES);
read_suit_names("SUITMAP");

my @DIST_NAMES;
read_dist_names("DISTMAP");

my $NUM_BIDS = 3400;
my $NUM_DIST = 560;

my $DIST_SELECT = 451;
my $POS_SELECT = 0;
my $VUL_SELECT = 0;

my ($pos, $vul, $open, $dist);
my (@store, @used);
my (@store_groups, @groups_used);
my (@dist_groups_used, @dist_used, @dist_passed, @dist_stats);

my (@select_value_hist, @select_hcp_hist, @select_groups_used,
  @select_hcp_used);

for my $open (0 .. $NUM_BIDS)
{
  $used[$open] = 0;
  $groups_used[$open] = 0;
  $select_groups_used[$open] = 0;

  for my $v (0 .. 39)
  {
    $select_value_hist[$open][$v] = 0;
    $select_hcp_hist[$open][$v] = 0;
  }
}

$select_hcp_used[$_] = 0 for (0 .. 39);

for my $d (0 .. $NUM_DIST-1)
{
  $dist_used[$d] = 0;
  $dist_passed[$d] = 0;
  for my $open (0 .. $NUM_BIDS)
  {
    $dist_groups_used[$d][$open] = 0;

    for my $pos (0 .. 3)
    {
      for my $vul (0 .. 3)
      {
        $dist_stats[$d][$open][$pos][$vul] = 0;
      }
    }
  }
}

for my $pos (0 .. 3)
{
  for my $vul (0 .. 3)
  {
    for my $open (0 .. $NUM_BIDS)
    {
      $store[$pos][$vul][$open] = 0;
      $store_groups[$pos][$vul][$open] = 0;
    }
  }
}

my $PASS = 0;
my $ONE = 1;
my $WEAK_TWO_MINOR = 2;
my $WEAK_TWO_MAJOR = 4;
my $WEAK_TWO_MAJOR_5332 = 5;
my $WEAK_TWO_BOTH_MINORS = 6;
my $WEAK_TWO_BOTH_MAJORS = 7;
my $WEAK_TWO_MAJOR_MINOR = 8;
my $WEAK_TWO_THREE_SUITER = 9;
my $WEAK_TWO_BAL = 10;
my $WEAK_THREE_MINOR = 11;
my $WEAK_THREE_MAJOR = 12;
my $WEAK_THREE_BOTH_MINORS = 13;
my $WEAK_THREE_BOTH_MAJORS = 14;
my $WEAK_THREE_MAJOR_MINOR = 15;
my $WEAK_FOUR_MINOR = 16;
my $WEAK_FOUR_MAJOR = 17;
my $WEAK_FIVE_BOTH_MINORS = 18;
my $WEAK_FIVE_MINOR = 19;

my $WEAK_TWO_OTHER = 100;

my @BID_NAMES;
$BID_NAMES[$PASS] = "Pass";
$BID_NAMES[$ONE] = "1-level";
$BID_NAMES[$WEAK_TWO_MINOR] = "2 minor";
$BID_NAMES[$WEAK_TWO_MAJOR] = "2 Major";
$BID_NAMES[$WEAK_TWO_MAJOR_5332] = "2M 5332";
$BID_NAMES[$WEAK_TWO_BOTH_MINORS] = "2 mm";
$BID_NAMES[$WEAK_TWO_BOTH_MAJORS] = "2 MM";
$BID_NAMES[$WEAK_TWO_MAJOR_MINOR] = "2 Mm";
$BID_NAMES[$WEAK_TWO_THREE_SUITER] = "2 3suit";
$BID_NAMES[$WEAK_TWO_BAL] = "2 BAL";
$BID_NAMES[$WEAK_THREE_MINOR] = "3 minor";
$BID_NAMES[$WEAK_THREE_MAJOR] = "3 Major";
$BID_NAMES[$WEAK_THREE_BOTH_MINORS] = "3 mm";
$BID_NAMES[$WEAK_THREE_BOTH_MAJORS] = "3 MM";
$BID_NAMES[$WEAK_THREE_MAJOR_MINOR] = "3 Mm";
$BID_NAMES[$WEAK_FOUR_MINOR] = "4 minor";
$BID_NAMES[$WEAK_FOUR_MAJOR] = "4 Major";
$BID_NAMES[$WEAK_FIVE_BOTH_MINORS] = "5 mm";
$BID_NAMES[$WEAK_FIVE_MINOR] = "5 minor";
$BID_NAMES[$WEAK_TWO_OTHER] = "W2 oth";

open my $fr, '<', $file or die "Can't open $file $!";
while (my $line = <$fr>)
{
  chomp $line;
  $line =~ s///g;

  my @a = split /,/, $line;
  $pos = $a[1];
  $vul = $a[2];
  $open = $a[3];
  $dist = $a[4];

  $store[$pos][$vul][$open]++;
  $used[$open]++;

  my $gused = bid_group($open);

  $store_groups[$pos][$vul][$gused]++;
  $groups_used[$gused]++;

  $dist_stats[$dist][$gused][$pos][$vul]++;
  $dist_groups_used[$dist][$gused]++;
  $dist_used[$dist]++;
  $dist_passed[$dist]++ if ($gused == $PASS);

  if ($dist == $DIST_SELECT && $pos == $POS_SELECT &&
      $vul == $VUL_SELECT)
  {
    my $value_sum = 
      $SUIT_VALUES[$a[5]] + 
      $SUIT_VALUES[$a[6]] + 
      $SUIT_VALUES[$a[7]] + 
      $SUIT_VALUES[$a[8]];
    my $hcp_sum = hcp($a[5]) + hcp($a[6]) + hcp($a[7]) + hcp($a[8]);

    $select_value_hist[$gused][int($value_sum)]++;
    $select_hcp_hist[$gused][int($hcp_sum)]++;
    $select_groups_used[$gused]++;
    $select_hcp_used[$hcp_sum]++;

    printf("%-8s  %s, %s, %s, %s: %2d, %7.2f\n",
      defined $BID_NAMES[$gused] ? $BID_NAMES[$gused] : $open,
      $SUIT_NAMES[$a[5]],
      $SUIT_NAMES[$a[6]],
      $SUIT_NAMES[$a[7]],
      $SUIT_NAMES[$a[8]], 
      $hcp_sum,
      $value_sum);
  }
}
close $fr;

print_hist(\@store, \@used, "Full");
print_hist(\@store_groups, \@groups_used, "Groups");

print_dist();

printf("%8s", "");
for my $open (0 .. $NUM_BIDS)
{
  printf("%8s", (defined $BID_NAMES[$open] ? $BID_NAMES[$open] : $open))
    if $select_groups_used[$open];
}
print "\n";

for my $v (0 .. 39)
{
  next unless $select_hcp_used[$v];
  printf("%2d%6d", $v, $select_hcp_used[$v]);
  for my $open (0 .. $NUM_BIDS)
  {
    next unless $select_groups_used[$open];
    printf("%4d%4d", 
      $select_value_hist[$open][$v],
      $select_hcp_hist[$open][$v]);
  }
  print "\n";
}
print "\n";


sub bid_group
{
  my $open = pop;

  return $PASS if ($open == 0);

  return $ONE if ($open <600);

  return $WEAK_TWO_MINOR if ($open == 600 || $open == 606 || $open == 701);

  return $WEAK_TWO_BOTH_MAJORS
    if ($open == 603 || $open == 704 || $open == 805 || $open == 903);

  return $WEAK_TWO_MAJOR_MINOR
    if ($open == 602 || $open == 703 || $open == 801 || $open == 902 ||
      $open == 803 || $open == 908 || $open == 909);

  return $WEAK_TWO_MAJOR
    if ($open == 601 || $open == 702 || $open == 800 ||
      $open == 802 || $open == 900);

  return $WEAK_TWO_BOTH_MINORS if ($open == 604);

  return $WEAK_TWO_BAL if ($open == 607 || $open == 705 || $open == 810);

  return $WEAK_TWO_THREE_SUITER if ($open == 706);

  return $WEAK_TWO_MAJOR_5332 if ($open == 808 || $open == 901);

  return $WEAK_THREE_MINOR
    if ($open == 807 || $open == 905 ||
      $open == 1001 || $open == 1100 || $open == 1101 || $open == 1200);

  return $WEAK_THREE_BOTH_MINORS
    if ($open == 809 || $open == 904 || $open == 1000 || $open == 1102);

  return $WEAK_THREE_MAJOR_MINOR if ($open == 1003 || $open == 1103);

  return $WEAK_THREE_BOTH_MAJORS if ($open == 1004);

  return $WEAK_THREE_MAJOR if ($open == 1105 || $open == 1201 ||
    $open == 1300 || $open == 1301 || $open == 1400);

  return $WEAK_FOUR_MINOR if ($open == 1504 || $open == 1505 || 
      $open == 1507 || $open == 1600 || $open == 1601 || $open == 1700);

  return $WEAK_FOUR_MAJOR if ($open == 1502 || $open == 1503 || 
      $open == 1506 || $open == 1603 || $open == 1701 || $open == 1702 ||
      $open == 1800 || $open == 1801 || $open == 1900 || $open == 1901);

  return $WEAK_TWO_OTHER if (($open >= 600 && $open < 620) ||
      ($open >= 700 && $open < 720) ||
      ($open >= 800 && $open < 820) ||
      ($open >= 900 && $open < 920));

  return $WEAK_FIVE_MINOR if ($open == 2160 || $open == 2260);

  return $WEAK_FIVE_BOTH_MINORS if ($open == 2060);

  my $base = int($open/20) * 20;
  if ($base % 100 == 40 || $base % 100 == 20)
  {
    # Strong or intermediate (where some intermediates may be weak).
    return $ONE;
  }
  else
  {
    return $base;
  }
}


sub print_hist
{
  my ($ref, $used_ref, $text) = @_;
  print "$text\n\n";

  for my $open (0 .. $NUM_BIDS)
  {
    next unless $used_ref->[$open];
    printf("%-8s ", defined $BID_NAMES[$open] ? $BID_NAMES[$open] : $open);
    for my $pos (0 .. 3)
    {
      for my $vul (0 .. 3)
      {
        printf("%8d", $ref->[$pos][$vul][$open]);
      }
    }
    print "\n";
  }
}


sub print_dist
{
  print "\nCompletely unused distributions\n\n";
  for my $d (0 .. $NUM_DIST-1)
  {
    print "$d $DIST_NAMES[$d]\n" unless $dist_used[$d];
  }

  print "\nAlways opened distributions\n\n";
  for my $d (0 .. $NUM_DIST-1)
  {
    print "$d $DIST_NAMES[$d]\n" 
      if ($dist_used[$d] && ! $dist_passed[$d]);
  }

  for my $d (0 .. $NUM_DIST-1)
  {
    next unless $dist_used[$d] && $dist_passed[$d];

    print "Dist $DIST_NAMES[$d], $d:\n\n";
    for my $open (0 .. $NUM_BIDS)
    {
      next unless $dist_groups_used[$d][$open];
      printf("%-8s ", 
        defined $BID_NAMES[$open] ? $BID_NAMES[$open] : $open);
      my $sumsum = 0;
      for my $pos (0 .. 3)
      {
        my $sum = 0;
        for my $vul (0 .. 3)
        {
          $sum += $dist_stats[$d][$open][$pos][$vul];
        }
        printf("%8d", $sum);
        $sumsum += $sum;
      }
      printf(": %8d\n", $sumsum);
    }
    print "\n";
  }
}


sub read_suit_names
{
  my $fname = pop;
  open my $fs, '<', $fname or die "Can't open $fname $!";
  while (my $line = <$fs>)
  {
    chomp $line;
    $line =~ s///g;
  
    my @a = split /\s+/, $line;
    my $sno = $a[1];
    my $sname = $a[2];
    $SUIT_NAMES[$sno] = $sname;
    $SUIT_VALUES[$sno] = $a[3];
  }
  close $fs;
}



sub read_dist_names
{
  my $fname = pop;
  open my $fd, '<', $fname or die "Can't open $fname $!";
  while (my $line = <$fd>)
  {
    chomp $line;
    $line =~ s///g;
  
    my @a = split /\s+/, $line;
    my $dno = $a[1];
    my $dname = $a[2];
    $DIST_NAMES[$dno] = $dname;
  }
  close $fd;
}


sub hcp
{
  my $sno = pop;
  my $sname = $SUIT_NAMES[$sno];
  my $p = 0;
  $p += 4 if $sname =~ /A/;
  $p += 3 if $sname =~ /K/;
  $p += 2 if $sname =~ /Q/;
  $p += 1 if $sname =~ /J/;
  return $p;
}
