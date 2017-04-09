#!perl

use strict;
use warnings;

# Modify .ref files to create multiple segments (player change).
# Detect these by lines of "n delete m" in ref files.

if ($#ARGV < 0)
{
  print "Usage: perl multiseg.pl number\n";
  exit;
}

# PC
my $DIR = "../../../bridgedata/hands/BBOVG";

# Laptop
# my $DIR = "../../../bridgedata/BBOVG";

my $indir = $ARGV[0];
if ($indir =~ /^\d+$/)
{
  $indir = "$DIR/0${indir}000";
}

$indir = "$DIR/*" if ($indir eq "all");

# $indir = "ttt";
my @files = glob "$indir/*.ref";

for my $file (@files)
{
  open my $fr, '<', $file or die "Can't open $file $!";
  my @lines;
  my $changeFlag = 0;
  while (my $line = <$fr>)
  {
    chomp $line;
    $line =~ s///g;

    my ($lno, $arg);
    if ($line =~ /LIN/ || $line !~ /^(\d+)\s+delete\s+(\d+)$/)
    {
      push @lines, $line;
      next;
    }

    $lno = $1;
    $arg = $2;

    my $linfile = $file;
    $linfile =~ s/ref$/lin/;
    die "$file: no $linfile" unless -e $linfile;

    my $pnNew = getline($linfile, $lno);
    if ($pnNew !~ /^pn/)
    {
      warn "$file, $pnNew: Not a pn line";
      push @lines, $line;
      next;
    }

    my (@vgList, @rsList, $lastQx);
    if (! getTop($linfile, $lno, \@vgList, \@rsList, \$lastQx))
    {
      warn "$file, $pnNew: Odd file";
      push @lines, $line;
      next;
    }

    my $rsStartNew = 2 * ($lastQx + 1 - $vgList[3]);
    my $rsStartCount = $rsStartNew + 1;
    my $rsDeletions = $#rsList + 1 - $rsStartNew;

    my ($rsNewline, $vgNewline);
    $vgList[3] = $lastQx+1;
    vgConcat(\@vgList, \$vgNewline);
    rsConcat(\@rsList, $rsStartNew, \$rsNewline);

    push @lines, "1 replaceLIN \"1,5,vg,$vgList[4],$lastQx\" {ERR_LIN_VG_WRONG(1,0,0)}";

    push @lines, "2 deleteLIN \"1,$rsStartCount,rs,$rsList[$rsStartNew],$rsDeletions\" {ERR_LIN_VG_WRONG(1,0,0)}";

    push @lines, "$lno insert \"$vgNewline\" {ERR_LIN_VG_LINE(1,0,0)}";
    push @lines, "$lno insert \"$rsNewline\" {ERR_LIN_RS_LINE(1,0,0)}";
    $changeFlag = 1;
  }
  close $fr;

  next unless $changeFlag;

  open my $fo, '>', $file or die "Can't open $file $!";
  for my $ll (@lines)
  {
    print $fo "$ll\n";
    # print "$file: $ll\n";
  }
  close $fo;
}


sub getline
{
  my ($fn, $lno) = @_;
  open my $fr, '<', $fn or die "Can't open $fn $!";

  my $running = 0;
  while (my $line = <$fr>)
  {
    chomp $line;
    $line =~ s///g;
    $running++;

    if ($running == $lno)
    {
      close $fr;
      return $line;
    }
  }
  close $fr;
  die "$fn, $lno: Not found";
}


sub getTop
{
  my ($fn, $lno, $vgRef, $rsRef, $lastRef) = @_;

  open my $fr, '<', $fn or die "Can't open $fn $!";
  my $running = 0;
  my $line;
  while (++$running < $lno && ($line = <$fr>))
  {
    chomp $line;
    $line =~ s///g;

    if ($line =~ /^vg\|([^|]*)\|$/)
    {
      @$vgRef = split ',', $1;
    }
    elsif ($line =~ /^rs\|([^|]*)\|$/)
    {
      @$rsRef = split ',', $1;
    }
    elsif ($line =~ /qx\|([^|]*)\|/)
    {
      $$lastRef = $1;
      $$lastRef =~ s/^[oc]//;
    }
  }
  close $fr;
}


sub vgConcat
{
  my ($vgRef, $lineRef) = @_;

  $$lineRef = "vg|" . join(',', @$vgRef) . "|";
}

sub rsConcat
{
  my ($rsRef, $first, $lineRef) = @_;

  $$lineRef = "rs|" . join(',', @$rsRef[$first .. $#$rsRef]) . "|";
}

