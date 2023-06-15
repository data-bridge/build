#!perl

use strict;
use warnings;

# Parses output from reader with -e flag (possible equals).
# Flags those that might have overlaps of hands for further checks.

if ($#ARGV < 0)
{
  print "Usage: perl equals.pl file.txt\n";
  exit;
}

my $file = $ARGV[0];
die "No file '$file'" unless -e $file;

my @data;
my $no = 0;

open my $fh, '<', $file or die "Can't open '$file': $!";
while (my $line = <$fh>)
{
  chomp $line;
  $line =~ s///g;

  $line =~ /([^\/]+)\....: (.*)$/;
  my ($fname, $rest) = ($1, $2);
  if (! defined $rest)
  {
    print "Problem $fname, $line\n";
  }

  my @list = sort {$a <=> $b} (split /\s+/, $rest);
  $data[$no]{name} = $fname;
  push @{$data[$no]{list}}, @list;
  $no++;
}
close $fh;

for my $i (0 .. $no-2)
{
  my $li = $#{$data[$i]{list}};
  for my $j ($i+1 .. $no-1)
  {
    my $lj = $#{$data[$j]{list}};
    my $min = ($li < $lj ? $li : $lj);
    my $overlap = find_overlap(\@{$data[$i]{list}}, \@{$data[$j]{list}},
      $li, $lj);

    my $a = $li+1;
    my $b = $lj+1;
    my $m = $min+1;

    if ($overlap == $m || ($overlap > 2 && $overlap >= 0.5 * $m))
    {
      print "Overlap files $data[$i]{name} and $data[$j]{name}: $overlap (out of $a, $b)\n";
    }
  }
}


sub find_overlap
{
  my ($li_ref, $lj_ref, $li, $lj) = @_;
  my $overlap = 0;
  my $i = 0;
  my $j = 0;

  while ($i <= $li && $j <= $lj)
  {
    if ($li_ref->[$i] == $lj_ref->[$j])
    {
      $i++;
      $j++;
      $overlap++;
    }
    elsif ($li_ref->[$i] < $lj_ref->[$j])
    {
      $i++;
    }
    else
    {
      $j++;
    }
  }
  return $overlap;
}

