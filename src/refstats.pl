#!perl

use strict;
use warnings;

# Reads all files in directory argument that end on .ref,
# and generates statistics.

if ($#ARGV < 0)
{
  print "Usage: perl refstats.pl tmp\n";
  exit;
}

my $HOMEDIR = glob("~/GitHub/Build/src");

# PC
my $DIR = "../../../bridgedata/hands/BBOVG";

# Laptop
# my $DIR = "../../../bridgedata/BBOVG";

my $indir = $ARGV[0];
if ($indir =~ /^\d+$/)
{
  $indir = "$DIR/0${indir}000";
}

my $ref = "referr.h";
my $out = "zfix.txt";
my @files;
$indir = "$DIR/*" if ($indir eq "all");
@files = glob("$indir/*.ref");

my (@codes, @accum_expl, @accum_rem);
read_error_codes("$HOMEDIR/$ref", \@codes);
my %codehash;
for (my $i = 0; $i <= $#codes; $i++)
{
  $codehash{$codes[$i]} = $i;
  reset_accum(\%{$accum_expl[$i]});
  reset_accum(\%{$accum_rem[$i]});
}

my $num_lin_files = count_files($indir, "lin");
my $num_skip_files = count_files($indir, "skip");
my $num_orig_files = count_files($indir, "orig");

my $num_ref_files = 0;
foreach my $file (@files)
{
  $num_ref_files++;
  open my $fr, '<', $file or die "Can't open $file: $!";
  my $expl_seen = 0;
  my $rem_seen = 0;

  while (my $line = <$fr>)
  {
    chomp $line;
    $line =~ s///g;

    my $noLIN;
    if ($line =~ /^(\d+)\s+(\w+)\s+(\d+)/)
    {
      $noLIN = $3;
    }
    elsif ($line =~ /^(\d+)\s+(\w+)/)
    {
      $noLIN = 1;
    }
    else
    {
      die "Bad line: $line";
    }

    if ($line =~ /\{(.*)\}\s*$/)
    {
      my $text = $1;
      my @entries = split ',', $text;
      for my $e (@entries)
      {
        if ($e !~ /(.+)\((\d+),(\d+),(\d+)\)/)
        {
          die "Bad entry: $e";
        }
        log_entry(\@accum_expl, $expl_seen, $noLIN, $1, $2, $3, $4);
        $expl_seen = 1;
      }
    }
    else
    {
      log_entry(\@accum_rem, $rem_seen, $noLIN, "ERR_SIZE", 0, 0, 0);
      $rem_seen = 1;
    }
  }
  close $fr;
}

write_stats($out, \@codes, \@accum_expl, \@accum_rem, 
  $num_lin_files, $num_ref_files, $num_skip_files, $num_orig_files);


sub reset_accum
{
  my ($accum_ref) = @_;
  $accum_ref->{files} = 0;
  $accum_ref->{lins} = 0;
  $accum_ref->{refs} = 0;
  $accum_ref->{tags} = 0;
  $accum_ref->{qxs} = 0;
  $accum_ref->{boards} = 0;
}


sub read_error_codes
{
  my ($fname, $codes_ref) = @_;

  open my $ff, '<', $fname or die "Can't open $fname: $!";

  my $seen = 0;
  while (my $line = <$ff>)
  {
    chomp $line;
    $line =~ s///g;
    next if ($line =~ /^\s*$/);

    if ($seen)
    {
      $line =~ s/,\s*$//;
      last if ($line =~ /^\}/);
      $line =~ s/\s//g;
      push @$codes_ref, $line;
    }
    elsif ($line =~ /^enum RefErrorsType/)
    {
      $line = <$ff>; # Skip leading {
      $seen = 1;
    }
  }

  close $ff;
}


sub log_entry
{
  my ($accum_ref, $seen_flag, $noLIN,
    $code, $count_tag, $count_qx, $count_board) = @_;

  if (! defined $codehash{$code})
  {
    die "Undefined code: $code";
  }

  my $i = $codehash{$code};
  $accum_ref->[$i]{files}++ if ! $seen_flag;
  $accum_ref->[$i]{refs}++;
  $accum_ref->[$i]{lins} += $noLIN;
  $accum_ref->[$i]{tags} += $count_tag;
  $accum_ref->[$i]{qxs} += $count_qx;
  $accum_ref->[$i]{boards} += $count_board;
}


sub count_files
{
  my ($indir, $ext) = @_;
  my @files = glob("$indir/*.$ext");
  if (! @files)
  {
    return 0;
  }
  else
  {
    return $#files+1;
  }
}


sub write_stats
{
  my ($fout, $codes_ref, $accum_expl_ref, $accum_rem_ref,
    $num_lin_files, $num_ref_files, $num_skip_files, $num_orig_files) = @_;

  open my $fo, '>', $fout or die "Can't open $fout $!";

  printf $fo "%-28s %23s %11s | %12s\n", 
    "", "Explained", "", "Remaining";
  my $FORMAT = "%-28s %5s %5s %5s %5s %5s %5s | %6s %5s\n";

  printf $fo $FORMAT,
    "Reference", "#file", "#lin", "#ref", "#tags", "#qx", "#bds",
    "#lin", "#ref";

  my @sum;
  for (my $i = 0; $i <= $#$codes_ref; $i++)
  {
    printf $fo $FORMAT,
      $codes_ref->[$i],
      $accum_expl_ref->[$i]{files},
      $accum_expl_ref->[$i]{lins},
      $accum_expl_ref->[$i]{refs},
      $accum_expl_ref->[$i]{tags},
      $accum_expl_ref->[$i]{qxs},
      $accum_expl_ref->[$i]{boards},
      $accum_rem_ref->[$i]{lins},
      $accum_rem_ref->[$i]{refs};

      $sum[0] += $accum_expl_ref->[$i]{lins};
      $sum[1] += $accum_expl_ref->[$i]{refs};
      $sum[2] += $accum_expl_ref->[$i]{tags};
      $sum[3] += $accum_expl_ref->[$i]{qxs};
      $sum[4] += $accum_expl_ref->[$i]{boards};
      $sum[5] += $accum_rem_ref->[$i]{lins};
      $sum[6] += $accum_rem_ref->[$i]{refs};
  }

  print $fo ("-" x 79) . "\n";
  printf $fo $FORMAT,
    "Sum", "N/A", @sum;

  printf $fo "\n\n%-28s %5s\n",
    "Number of lin files", $num_lin_files;
  printf $fo "%-28s %5s\n",
    "Number of ref files", $num_ref_files;
  printf $fo "%-28s %5s\n",
    "Number of skip files", $num_skip_files;
  printf $fo "%-28s %5s\n",
    "Number of orig files", $num_orig_files;
}

