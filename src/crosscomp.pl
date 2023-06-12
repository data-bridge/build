#!perl

use strict;
use warnings;

# Cross-compare a lin file with an RP TXT file.
# TXT doesn't have alerts (which are present in VG files but not
# in RP RBN files), so this is an acceptable comparison.

if ($#ARGV != 1 && $#ARGV != 2)
{
  print "Usage: perl crosscomp.pl VGno RBbase [fix]\n";
  exit;
}

my $DIR;
if (`uname -a` =~ /CDD/)
{
  # Laptop
  $DIR = "../../../bridgedata";
}
else
{
  # PC
  $DIR = "../../../bridgedata/hands";
}

my $lin = $ARGV[0];
my $txt = $ARGV[1];
my $move_flag = ($#ARGV == 2 ? 1 : 0);

my $linfile = no2linfile($lin);

if ($move_flag)
{
  for my $f (qw(LIN PBN RBN RBX TXT REC EML))
  {
    my $oldfile = str2RPfile($txt, $f);
    my $newfile = str2cpfile($txt, $f);
    rename $oldfile, $newfile;
  }
}
else
{
  my $txtfile = str2RPfile($txt, "TXT");
  system("./reader -i $linfile -o $txt.TXT");
  system("diff --ignore-all-space $txt.TXT $txtfile");
}


sub no2linfile
{
  my $no = pop;
  return "" unless $no =~ /^\d+$/;
  return "$DIR/BBOVG/000000/$no.lin" if $no < 1000;
  my $t = int($no/1000);
  return "$DIR/BBOVG/00${t}000/$no.lin" if $no < 10000;
  return "$DIR/BBOVG/0${t}000/$no.lin";
}


sub str2RPfile
{
  my ($str, $postfix) = @_;
  return "" unless $str =~ /^.(\d\d)...$/;
  my $n = $1;
  my $d = ($n > 90 ? "19$n" : "20$n");
  return "$DIR/RPmajors/$postfix/$d/$str.$postfix";
}


sub str2cpfile
{
  my ($str, $postfix) = @_;
  return "" unless $str =~ /^.(\d\d)...$/;
  my $n = $1;
  my $d = ($n > 90 ? "19$n" : "20$n");
  return "$DIR/RPmajors/duplicates/$str.$postfix";
}

