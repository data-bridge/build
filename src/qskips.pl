#!perl

use strict;
use warnings;

require "addskip.pl";

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
      write_skip_file($skipfile, "ERR_LIN_QX_MISSING",
        $countlin, $countqx, $countbd);
      # print "Add\n";
      $num_empty++;
    }
    else
    {
      # Assuming file already has sensible content
      add_skip_file($skipfile, "ERR_LIN_QX_MISSING", 
        $countlin, $countqx, $countbd);
      # print "Modify\n";
      $num_annot++;
    }
  }
  else
  {
    # Making new file
    write_skip_file($skipfile, "ERR_LIN_QX_MISSING",
      $countlin, $countqx, $countbd);
    # print "Make\n";
    $num_none++;
  }
  print "\n";
}

print "\n";
print "Empty : $num_empty\n";
print "Marked: $num_annot\n";
print "None  : $num_none\n";

