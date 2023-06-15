#!perl

use strict;
use warnings;

# Checks ref files for ERR_DUPLICATE and ERR_SUBSET (lin only).
# May have to use linmerge.pl on files afterwards.

my $OVERLAP = "overlap2.txt";

if ($#ARGV != 0)
{
  print "Usage: perl checkdupl.pl dir\n";
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

my @overlaps;
read_overlaps($OVERLAP);

my $dir = $ARGV[0];
my @reffiles = glob("$dir/*.ref $dir/*/*.ref");

for my $reffile (@reffiles)
{
  open my $fr, '<', $reffile or die "Can't open $reffile $!";
  my $first = <$fr>;
  close $fr;

  my $flag;
  if ($first =~ /ERR_LIN_DUPLICATE/)
  {
    $flag = 1;
  }
  elsif ($first =~ /ERR_LIN_SUBSET/)
  {
    $flag = 2;
  }
  else
  {
    next;
  }

  if ($reffile !~ /(\d+)\.ref$/)
  {
    print "$reffile: Can't get base.\n";
    next;
  }
  my $base1 = $1;

  if (! defined $overlaps[$base1])
  {
    print "$reffile: No overlaps for $base1\n";
    next;
  }

  my (%list1, @md1);
  my $file1 = no2file($base1);
  file2list($file1, \%list1, \@md1);

  my $seen = 0;
  my $found = 0;
  my $str = "";
  for my $base2 (@{$overlaps[$base1]})
  {
    next if ($base2 > $base1+500 || $base2 < $base1-500);
    $seen++;

    my (%list2, @md2);
    my $file2 = no2file($base2);
    file2list($file2, \%list2, \@md2);

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

    # Check the md entries as well.
    @md1 = sort @md1;
    @md2 = sort @md2;
    dedup(\@md1);
    dedup(\@md2);

    my $l1 = $#md1;
    my $l2 = $#md2;
    my $md_overlap = find_overlap(\@md1, \@md2, $l1, $l2);

    if ($#only1 < 0)
    {
      if ($md_overlap > 0)
      {
        $found = 1;
        # print "$base1 ($overlap1, $md_overlap): Hit $base2\n\n";
        last;
      }
      else
      {
        $str .= "$base1, $base2: No md overlap\n";
      }
    }
    elsif ($md_overlap > 0)
    {
      $str .= "$base1 ($overlap1, $md_overlap):";
      $str .= " $_" for (sort @only1);
      $str .= "\n";
      $str .= "$base2 ($overlap2):";
      $str .= " $_" for (sort @only2);
      $str .= "\n";
    }
    else
    {
      $str .= "$base1, $base2: Spurious overlap\n";
    }
  }

  if (! $seen)
  {
    print "$base1: No candidates in";
    print " $_" for (sort @{$overlaps[$base1]});
    print "\n";
  }
  elsif (! $found)
  {
    print "$str\n";
  }
  else
  {
    # print "$base1: Hit\n";
  }
}


sub read_overlaps
{
  my $file = pop;
  die "No overlap file $file" unless -e $file;
  open my $fo, '<', $file or die "Can't open $file $!";
  while (my $line = <$fo>)
  {
    if ($line !~ /^Overlap files (\d+) and (\d+): (\d+) \(out of (\d+), (\d+)\)/)
    {
      die "Bad line $line in $file";
    }

    my ($base1, $base2, $overlap, $len1, $len2) = ($1, $2, $3, $4, $5);
    my $min = ($len1 < $len2 ? $len1 : $len2);
    next if ($overlap != $min);

    if ($len1 == $len2)
    {
      push @{$overlaps[$base1]}, $base2;
      push @{$overlaps[$base2]}, $base1;
    }
    elsif ($len1 < $len2)
    {
      push @{$overlaps[$base1]}, $base2;
    }
    else
    {
      push @{$overlaps[$base2]}, $base1;
    }
  }
  close $fo;
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
  my ($file, $list_ref, $md_ref) = @_;

  open my $fr, '<', $file or die "Can't open $file $!";
  my $prev = "";
  while (my $line = <$fr>)
  {
    if ($line =~ /^qx\|([^,\|]+)/ || $line =~ /\|qx\|([^,\|]+)/)
    {
      $list_ref->{$1} = 1;
    }
    if ($line =~ /^md\|([^|]*)/ || $line =~ /\|md\|([^|]*)/)
    {
      my $cand = substr $1, 0, 54;
      push @$md_ref, $cand unless ($cand eq $prev);
      $prev = $cand;
    }
  }
  close $fr;
}


sub dedup
{
  my ($aref) = @_;
  for (my $i = $#$aref-1; $i >= 0; $i--)
  {
    if ($aref->[$i] eq $aref->[$i+1])
    {
      splice @$aref, $i+1, 1;
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
    if ($li_ref->[$i] eq $lj_ref->[$j])
    {
      $i++;
      $j++;
      $overlap++;
    }
    elsif ($li_ref->[$i] le $lj_ref->[$j])
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

