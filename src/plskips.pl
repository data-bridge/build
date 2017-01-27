#!perl

use strict;
use warnings;

require "addskip.pl";

# PC
my $DIR = "../../../bridgedata/hands/BBOVG";

# Laptop
# my $DIR = "../../../bridgedata/BBOVG";


my @files;
if ($#ARGV == 0 && -d $ARGV[0])
{
  @files = glob("$ARGV[0]/*.sht");
}
elsif ($#ARGV >= 0)
{
  @files = @ARGV;
}
else
{
  print "Usage: perl plskips.pl dir|files (ending on sht)\n";
  exit;
}

my ($num_empty, $num_annot, $num_none) = (0, 0, 0);
for my $file (@files)
{
  my $gr = `grep -l 'Suggest skip' $file`;
  next if ($gr eq "");

  my $skipfile = $file;
  $skipfile =~ s/sht$/skip/;
  $skipfile =~ s/(.*)\///; # Basename


  if ($skipfile =~ /^(\d)\d\d\.skip/)
  {
    $skipfile = "$DIR/000000/$skipfile";
  }
  elsif ($skipfile =~ /^(\d)\d\d\d\.skip/)
  {
    $skipfile = "$DIR/00${1}000/$skipfile";
  }
  elsif ($skipfile =~ /^(\d\d)/)
  {
    $skipfile = "$DIR/0${1}000/$skipfile";
  }
  else
  {
    next;
  }

  my $linfile = $skipfile;
  $linfile =~ s/skip$/lin/;
  my ($countlin, $countqx, $countbd);
  count_file($linfile, \$countlin, \$countqx, \$countbd);

  if (-e $skipfile)
  {
    if (-z $skipfile)
    {
      # Adding line to empty file
      write_skip_file($skipfile, "ERR_LIN_DISTS_WRONG",
        $countlin, $countqx, $countbd);
      print "Add $skipfile: $countlin $countqx $countbd\n";
      $num_empty++;
    }
    else
    {
      # Assuming file already has sensible content
      add_skip_file($skipfile, "ERR_LIN_DISTS_WRONG", 
        $countlin, $countqx, $countbd);
      print "Modify $skipfile: $countlin $countqx $countbd\n";
      $num_annot++;
    }
  }
  else
  {
    # Making new file
    write_skip_file($skipfile, "ERR_LIN_QX_MISSING",
      $countlin, $countqx, $countbd);
    print "Make $skipfile: $countlin $countqx $countbd\n";
    $num_none++;
  }
}

print "\n";
print "Empty : $num_empty\n";
print "Marked: $num_annot\n";
print "None  : $num_none\n";

