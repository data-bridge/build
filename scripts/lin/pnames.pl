#!perl

use strict;
use warnings;

if ($#ARGV < 0)
{
  print "Usage: perl pnames.pl 31\n";
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

my $out = "pnames$chapter.txt";

my @files = glob("$DIR/0${chapter}000/*.lin");

my $num_empty = 0;
my $num_annot = 0;
my $num_none = 0;

my @default = qw(South West North East South West North East);

for my $file (@files)
{
  my @pn = `grep "p[nw]\|" $file`;
  my $ok = 1;
  my $example;
  for my $p (@pn)
  {
    chomp $p;
    $p =~ s///g;
    if (! is_ok($p))
    {
      $ok = 0;
      $example = $p;
      last;
    }

  }

  next if $ok;

  print "$file:\n";
  print $example, "\n";

  my ($countlin, $countqx, $countbd);
  count_file($file, \$countlin, \$countqx, \$countbd);

  my $wc = `wc -l $file`;
  my $count;
  if ($wc =~ /^(\d)+/)
  {
    $count = $1;
  }
  else
  {
    die "$file: No wc";
  }

  my $skipfile = $file;
  $skipfile =~ s/lin$/skip/;
  if (-e $skipfile)
  {
    if (-z $skipfile)
    {
      # Adding line to empty file
      write_skip_file($skipfile, $countlin, $countqx, $countbd);
      $num_empty++;
    }
    else
    {
      # Assuming file already has sensible content
      $num_annot++;
    }
  }
  else
  {
    # Making new file
    write_skip_file($skipfile, $countlin, $countqx, $countbd);
    $num_none++;
  }
  print "\n";
}

print "\n";
print "Empty : $num_empty\n";
print "Marked: $num_annot\n";
print "None  : $num_none\n";


sub is_ok
{
  my ($line) = @_;

  return 1 unless 
    ($line =~ /^p[nw]\|([^|]*)\|/ ||
     $line =~ /\|p[nw]\|([^|]*)\|/);

  my $field = $1;
  my @fields = split ",", $field;
  my $nf = $#fields;
  return 1 unless ($nf == 3 || $nf == 7);

  my $numreal = 0;
  for my $f (0 .. 3)
  {
    if ($fields[$f] ne $default[$f] && $fields[$f] ne "")
    {
      $numreal++;
    }
  }

  return 1 if ($numreal == 4);
  return 0 if ($nf == 3);

  $numreal = 0;
  for my $f (4 .. 7)
  {
    if ($fields[$f] ne $default[$f])
    {
      $numreal++;
    }
  }

  return ($numreal == 4 ? 1 : 0);
}


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


sub write_skip_file
{
  my ($skipfile, $countlin, $countqx, $countbd) = @_;
  open my $fs, '>', $skipfile or die "Can't open $skipfile: $!";
  print $fs "$countlin {ERR_LIN_PN_PLAYERS_UNKNOWN(0,$countqx,$countbd)}\n";
  close $fs;
}
