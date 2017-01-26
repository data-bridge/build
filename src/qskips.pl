#!perl

use strict;
use warnings;

if ($#ARGV < 0)
{
  print "Usage: perl qskips.pl 31\n";
  exit;
}

# PC
my $DIR = "../../../bridgedata/hands/BBOVG";

# Laptop
# my $DIR = "../../../bridgedata/BBOVG";

my $chapter = $ARGV[0];
if ($chapter < 0 || $chapter > 99)
{
  print "$chapter is probably out of range\n";
  exit;
}

my $out = "qskip$chapter.txt";

my @files = glob("$DIR/0${chapter}000/*.lin");

my $num_empty = 0;
my $num_annot = 0;
my $num_none = 0;

for my $file (@files)
{
  open my $fh, '<', $file or die "Can't open $file";
  my $found = 0;
  while (my $line = <$fh>)
  {
    if ($line =~ /qx\|/)
    {
      $found = 1;
      last;
    }
  }

  next if $found;

  print "$file:\n";

  my ($countlin, $countqx, $countbd);
  count_file($file, \$countlin, \$countqx, \$countbd);

  my $skipfile = $file;
  $skipfile =~ s/lin$/skip/;
  if (-e $skipfile)
  {
    if (-z $skipfile)
    {
      # Adding line to empty file
      write_skip_file($skipfile, $countlin, $countqx, $countbd);
      # print "Add\n";
      $num_empty++;
    }
    else
    {
      # Assuming file already has sensible content
      add_skip_file($skipfile, $countlin, $countqx, $countbd);
      # print "Modify\n";
      $num_annot++;
    }
  }
  else
  {
    # Making new file
    write_skip_file($skipfile, $countlin, $countqx, $countbd);
    # print "Make\n";
    $num_none++;
  }
  print "\n";
}

print "\n";
print "Empty : $num_empty\n";
print "Marked: $num_annot\n";
print "None  : $num_none\n";


sub count_file
{
  my ($file, $countlin, $countqx, $countbd) = @_;

  my $wc = `wc -l $file`;
  my $count;
  if ($wc =~ /^(\d)+/)
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
  my ($skipfile, $countlin, $countqx, $countbd) = @_;
  open my $fs, '<', $skipfile or die "Can't open $skipfile: $!";
  my $line = <$fs>;
  close $fs;

  chomp $line;
  $line =~ s///g;

  die "$skipfile: Got $line" unless ($line =~ /^(\d+)\s+\{(.*)\}$/);

  my $lcount = $1;
  die "New line count: $lcount vs $countlin" if ($lcount != $countlin);

  my $newline = $lcount . " {";

  my $entry = $2;
  my @entries = split ';', $entry;

  my $seen = 0;
  my @components;
  for my $e (@entries)
  {
    die "$skipfile: Bad entry '$e' in $line" unless
      $e =~ /(.+)\((\d+),(\d+),(\d+)\)/;

    my ($tag, $c1, $c2, $c3) = ($1, $2, $3, $4);
    if ($tag eq "ERR_LIN_QX_MISSING")
    {
      die "$skipfile Expected != 0 in '$line'" if ($c1 != 0);
      die "$skipfile Expected != $countqx in '$line'" if ($c2 != 0);
      die "$skipfile Expected != $countbd in '$line'" if ($c3 != 0);
      $seen = 1;
    }
    push @components, $e;
  }

  if (! $seen)
  {
    push @components, "ERR_LIN_QX_MISSING(0,$countqx,$countbd)";
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
  my ($skipfile, $countlin, $countqx, $countbd) = @_;
  open my $fs, '>', $skipfile or die "Can't open $skipfile: $!";
  print $fs "$countlin {ERR_LIN_QX_MISSING(0,$countqx,$countbd)}\n";
  close $fs;
}
