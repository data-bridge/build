#!perl

use strict;
use warnings;

# Compares qx entries.

if ($#ARGV != 1)
{
  print "Usage: perl sameq.pl no1 no2\n";
  exit;
}

my $lin1 = $ARGV[0];
my $lin2 = $ARGV[1];

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

my $file1 = no2file($lin1);
my $file2 = no2file($lin2);

my (%list1, %list2);
file2list($file1, \%list1);
file2list($file2, \%list2);

my (@only1, @only2);
my $overlap1 = 0;
my $overlap2 = 0;
for my $k (keys %list1)
{
  if (defined $list2{$k})
  {
    $overlap1++;
  }
  else
  {
    push @only1, $k;
  }
}
for my $k (keys %list2)
{
  if (defined $list1{$k})
  {
    $overlap2++;
  }
  else
  {
    push @only2, $k;
  }
}

die "Odd overlaps" unless $overlap1 == $overlap2;
if ($#only1 == -1 && $#only2 == -1)
{
  print "Complete qx overlap\n";
}
elsif ($#only1 == -1)
{
  print "$lin1 qx's are completely contained in $lin2\n";
}
elsif ($#only2 == -1)
{
  print "$lin2 qx's are completely contained in $lin1\n";
}
else
{
  print "$lin1 ($overlap1):";
  print " $_" for (sort @only1);
  print "\n";
  print "$lin2 ($overlap2):";
  print " $_" for (sort @only2);
  print "\n";
}


sub no2file
{
  my $no = pop;
  return "" unless $no =~ /^\d+$/;
  return "$DIR/000000/$no.lin" if $no < 1000;
  my $t = int($no/1000);
  return "$DIR/00${t}000/$no.lin" if $no < 10000;
  return "$DIR/0${t}000/$no.lin";
}


sub file2list
{
  my ($file, $list_ref) = @_;

  open my $fr, '<', $file or die "Can't open $file $!";
  while (my $line = <$fr>)
  {
    if ($line =~ /^qx\|([^,\|]+)/ || $line =~ /\|qx\|([^,\|]+)/)
    {
      $list_ref->{$1} = 1;
    }
  }
  close $fr;
}

