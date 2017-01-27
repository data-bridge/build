#!perl

use strict;
use warnings;

# Helper functions for skipping files.


sub count_file
{
  my ($file, $countlin, $countqx, $countbd) = @_;

  my $wc = `wc -l $file`;
  my $count;
  if ($wc =~ /^(\d+)/)
  {
    $$countlin = $1;
  }
  else
  {
    die "$file: No wc";
  }

  my $qx = 0;
  my %seen;
  open my $fh, '<', $file or die "Can't open $file";
  while (my $line = <$fh>)
  {
    if ($line =~ /qx\|[oc](\d+)[^|]*\|/)
    {
      $qx++;
      $seen{$1} = 1;
    }
  }

  $$countbd = 0;
  for my $k (keys %seen)
  {
    $$countbd++;
  }

  $$countqx = $qx;

  close $fh;
}


sub add_skip_file
{
  my ($skipfile, $targettag, $countlin, $countqx, $countbd) = @_;
  open my $fs, '<', $skipfile or die "Can't open $skipfile: $!";
  my $line = <$fs>;
  close $fs;

  chomp $line;
  $line =~ s///g;

  die "$skipfile: Got $line" unless ($line =~ /^(\d+)\s+\{(.*)\}$/);

  my $lcount = $1;
  warn "$skipfile: New line count: $lcount vs $countlin" if ($lcount != $countlin);

  my $newline = $countlin . " {";

  my $entry = $2;
  my @entries = split ';', $entry;

  my $seen = 0;
  my @components;
  for my $e (@entries)
  {
    die "$skipfile: Bad entry '$e' in $line" unless
      $e =~ /(.+)\((\d+),(\d+),(\d+)\)/;

    my ($tag, $c1, $c2, $c3) = ($1, $2, $3, $4);
    if ($tag eq $targettag)
    {
      die "$skipfile Expected != 0 in '$line'" if ($c1 != 0);
      die "$skipfile Expected != $countqx in '$line'" if ($c2 != $countqx);
      die "$skipfile Expected != $countbd in '$line'" if ($c3 != $countbd);
      $seen = 1;
    }
    push @components, $e;
  }

  if (! $seen)
  {
    push @components, "$targettag(0,$countqx,$countbd)";
  }

  $newline .= join ';', @components;
  $newline .= "}\n";

  # print "Would like to write:\n";
  # print "$newline\n\n";
  open my $ft, '>', $skipfile or die "Can't open $skipfile: $!";
  print $ft "$newline";
  close $ft;
}


sub write_skip_file
{
  my ($skipfile, $targettag, $countlin, $countqx, $countbd) = @_;
  open my $fs, '>', $skipfile or die "Can't open $skipfile: $!";
  print $fs "$countlin {$targettag(0,$countqx,$countbd)}\n";
  close $fs;
}

1;
