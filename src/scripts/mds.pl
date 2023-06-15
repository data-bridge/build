#!perl

use strict;
use warnings;

# Parses output of equals.pl for more thorough comparison.

if ($#ARGV < 0)
{
  print "Usage: mds overlap.txt\n";
  exit;
}

my $file = $ARGV[0];
die "No file '$file'" unless -e $file;

my $prefix = "../../../bridgedata/hands/BBOVG";
my $no = 0;

open my $fh, '<', $file or die "Can't open '$file': $!";
while (my $line = <$fh>)
{
  chomp $line;
  $line =~ s///g;

  $line =~ /(\S+) and (\S+): (\d+) \(out of (\d+), (\d+)\)$/;
  my ($fname1, $fname2, $overlap, $count1, $count2) = ($1, $2, $3, $4, $5);

  my (@md1, @md2, $title1, $title2, $players1, $players2);
  next if (! get_mds($fname1, \@md1, \$title1, \$players1));
  next if (! get_mds($fname2, \@md2, \$title2, \$players2));

  if (! defined $title1 || ! defined $title2 ||
      ! defined $players1 || ! defined $players2)
  {
    print "Undefined fields in $fname1 or $fname2\n";
    if (defined $title1 && defined $title2)
    {
      print "$title1\n$title2\n";
    }
    if (defined $players1 && defined $players2)
    {
      print "$players1\n$players2\n";
    }
    print "\n";
    next;
  }

  @md1 = sort @md1;
  @md2 = sort @md2;
  dedup(\@md1);
  dedup(\@md2);

  my $l1 = $#md1;
  my $l2 = $#md2;
  my $md_overlap = find_overlap(\@md1, \@md2, $l1, $l2);

  if ($md_overlap > 0)
  {
    if (similar($fname1, $fname2, $title1, $title2, $players1, $players2))
    {
      print "Overlap: $fname1, $fname2, $md_overlap ($overlap) ($count1, $count2)\n";
      print "$title1\n$title2\n";
      print "$players1\n$players2\n\n";
    }
  }

  $no++;
}
close $fh;


sub get_mds
{
  my ($fname, $md_ref, $title_ref, $players_ref) = @_;

  my $full = $prefix . "/" . vgdir($fname) . "/" . $fname . ".lin";

  if (! -e $full)
  {
    $full = $prefix . "/" . vgdir($fname) . "/" . $fname . ".pbn";
    if (! -e $full)
    {
      print "$full (or .lin) doesn't exist\n";
      return 0;
    }
  }

  open my $fo, '<', $full or die "Can't open '$full': $!";
  my $prev = "";
  while (my $line = <$fo>)
  {
    chomp $line;
    $line =~ s///g;

    if ($line =~ /^vg\|([^|]*)/ || $line =~ /\|vg\|([^|]*)/)
    {
      $$title_ref = $1;
    }
    if ($line =~ /^pw\|([^|]*)/ || $line =~ /\|pw\|([^|]*)/)
    {
      $$players_ref = $1;
    }
    if ($line =~ /^pn\|([^|]*)/ || $line =~ /\|pn\|([^|]*)/)
    {
      $$players_ref = $1;
    }
    if ($line =~ /^md\|([^|]*)/ || $line =~ /\|md\|([^|]*)/)
    {
      push @$md_ref, $1 unless ($1 eq $prev);
      $prev = $1;
    }
  }
  return 1;
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


sub dename
{
  my ($pref) = @_;
  for (my $i = $#$pref; $i >= 0; $i--)
  {
    if ($pref->[$i] eq "north" ||
        $pref->[$i] eq "east" ||
        $pref->[$i] eq "south" ||
        $pref->[$i] eq "west")
    {
      splice @$pref, $i, 1;
    }
  }
}


sub vgdir
{
  my ($i) = @_;
  return "000000" if ($i < 1000);
  return "00" . int($i/1000) . "000" if ($i < 10000);
  return "0" . int($i/1000) . "000";
}


sub similar
{
  my ($fname1, $fname2, $title1, $title2, $players1, $players2) = @_;

  my @tlist1 = split ',', lc $title1;
  my @tlist2 = split ',', lc $title2;

  if ($#tlist1 < 8)
  {
    print "$fname1 has short title $title1\n";
    return 0;
  }

  if ($#tlist2 < 8)
  {
    print "$fname2 has short title $title2\n";
    return 0;
  }

  if (($tlist1[5] ne $tlist2[5] && $tlist1[5] ne $tlist2[7]) ||
      ($tlist1[7] ne $tlist2[5] && $tlist1[7] ne $tlist2[7]))
  {
    return 0;
  }

  my @plist1 = split ',', lc $players1;
  my @plist2 = split ',', lc $players2;
  dename(\@plist1);
  dename(\@plist2);

  my $ov = find_overlap(\@plist1, \@plist2, $#plist1, $#plist2);
  if ($ov > 0)
  {
    return 1;
  }
  return 0;
}

