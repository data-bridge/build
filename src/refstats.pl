#!perl

use strict;
use warnings;

# Reads all files in directory argument that end on .ref,
# and generates statistics in refstats.txt.

if ($#ARGV < 0)
{
  print "Usage: perl refstats.pl dir|30|all\n";
  exit;
}

my $HOMEDIR = glob("~/GitHub/Build/src");
my $out = "refstats.txt";

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


my $indir = $ARGV[0];
if ($indir =~ /^\d+$/)
{
  $out = "z${indir}_0.txt";
  if (-e $out)
  {
    $out = "z${indir}_1.txt";
  }
  $indir = "$DIR/0${indir}000";
}

my $ref = "referr.h";
$indir = "$DIR/*" if ($indir eq "all");
my @files = glob("$indir/*.ref");
my @files2 = glob("$indir/*.skip");
my @files3 = glob("$indir/*.noval");

my (@codes, @accum_expl, @accum_rem, @accum_expl2, @accum_rem2);
my (@accum_expl3, @accum_rem3);

read_error_codes("$HOMEDIR/$ref", \@codes);
my %codehash;
for (my $i = 0; $i <= $#codes; $i++)
{
  $codehash{$codes[$i]} = $i;
  reset_accum(\%{$accum_expl[$i]});
  reset_accum(\%{$accum_rem[$i]});
  reset_accum(\%{$accum_expl2[$i]});
  reset_accum(\%{$accum_rem2[$i]});
  reset_accum(\%{$accum_expl3[$i]});
  reset_accum(\%{$accum_rem3[$i]});
}

my $num_lin_files = count_files($indir, "lin");
my $num_skip_files = count_files($indir, "skip");
my $num_noval_files = count_files($indir, "noval");
my $num_orig_files = count_files($indir, "orig");
my $num_ref_files = count_files($indir, "ref");


make_stats(\@files, \@accum_expl, \@accum_rem);
make_stats(\@files2, \@accum_expl2, \@accum_rem2);
make_stats(\@files3, \@accum_expl3, \@accum_rem3);

open my $fo, '>', $out or die "Can't open $out $!";
write_ref_stats($fo, \@codes, \@accum_expl, \@accum_rem);
write_skip_stats($fo, \@codes, "Skip", \@accum_expl2, \@accum_rem2);
write_skip_stats($fo, \@codes, "Noval", \@accum_expl3, \@accum_rem3);
write_file_numbers($fo, $num_lin_files, $num_ref_files, $num_skip_files, 
  $num_orig_files, $num_noval_files);
close $fo;


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
  my ($file, $accum_ref, $seen_flag, $noLIN,
    $code, $count_tag, $count_qx, $count_board) = @_;

  if (! defined $codehash{$code})
  {
    warn "Undefined code in $file: $code";
    return;
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


sub make_stats
{
  my ($files_ref, $accum_expl_ref, $accum_rem_ref) = @_;

  foreach my $file (@$files_ref)
  {
    my (%expl_seen, %rem_seen);
    # my $expl_seen = 0;
    # my $rem_seen = 0;

    if ($file =~ /skip/ && -z $file)
    {
      # Zero size.
      my $filelin = $file;
      $filelin =~ s/skip$/lin/;
      my $wc = `wc -l $filelin`;
      if ($wc =~ /^(\d+)/)
      {
        my $noLIN = $1;
        log_entry($file, $accum_rem_ref, $rem_seen{ERR_SIZE}, 
          $noLIN, "ERR_SIZE", 0, 0, 0);
        $rem_seen{ERR_SIZE} = 1;
      }
      else
      {
        die "Bad wc line: $wc";
      }
    }

    open my $fr, '<', $file or die "Can't open $file: $!";

    while (my $line = <$fr>)
    {
      chomp $line;
      $line =~ s///g;

      my ($noLIN, $action, $tagLIN, $repeat);
      $tagLIN = "";
      $repeat = 0;
      if ($line =~ /^(\d+)\s+(\w+)\s+(\d+)/)
      {
        $noLIN = $3;
        $action = $2;
      }
      elsif ($line =~ /^(\d+)\s+(\w+)/)
      {
        $noLIN = 1;
        $action = $2;
        if ($action =~ /LIN/ && $line =~ /^\d+\s+\w+\s+\"([^"]*)\"/)
        {
          my $quotes = $1;
          quotes_to_content($file, $action, $quotes, \$tagLIN, \$repeat);
        }
      }
      elsif (($file =~ /skip/ || $file =~ /noval/) && $line =~ /^(\d+)\s+/)
      {
        $noLIN = $1;
      }
      else
      {
        warn "Bad line: File '$file', line '$line'";
        next;
      }

      if ($line =~ /\{(.*)\}\s*$/)
      {
        my $text = $1;
        my @entries = split ';', $text;
        for my $e (@entries)
        {
          if ($e !~ /(.+)\((\d+),(\d+),(\d+)\)/)
         {
            warn "Bad entry: $file, $e";
            next;
          }
          my ($tag, $c1, $c2, $c3) = ($1, $2, $3, $4);

          # onehand: (a,1,1)
          # onetag : (1,a,a)
          # oneline: not "delete 2"

          check_entry($file, $line, $tag, $tagLIN,
            "ERR_LIN_VG_FIRST", "vg",
            $c1, $c2, $c3, $noLIN, 0, 1, 1);
          check_entry($file, $line, $tag, $tagLIN,
            "ERR_LIN_VG_LAST", "vg",
            $c1, $c2, $c3, $noLIN, 0, 1, 1);
          check_entry($file, $line, $tag, $tagLIN,
            "ERR_LIN_VG_REPLACE", "vg",
            $c1, $c2, $c3, $noLIN, 0, 1, 1);
          check_entry($file, $line, $tag, $tagLIN,
            "ERR_LIN_VG_SYNTAX", "vg",
            $c1, $c2, $c3, $noLIN, 0, 0, 1);

          check_line($file, $line, "ERR_LIN_VHEADER_INSERT",
            $c1, $c2, $c3, $noLIN, 0, 1, 1);
          check_line($file, $line, "ERR_LIN_VHEADER_SYNTAX",
            $c1, $c2, $c3, $noLIN, 0, 1, 1);

          check_line($file, $line, "ERR_LIN_RESULTS_REPLACE", 
            $c1, $c2, $c3, $noLIN, 0, 1, 1);
          check_line($file, $line, "ERR_LIN_RESULTS_INSERT", 
            $c1, $c2, $c3, $noLIN, 0, 1, 1);
          check_line($file, $line, "ERR_LIN_RESULTS_DELETE", 
            $c1, $c2, $c3, $noLIN, 0, 1, 1);

          check_entry($file, $line, $tag, $tagLIN,
            "ERR_LIN_RS_REPLACE", "rs",
            $c1, $c2, $c3, $noLIN, 1, 1, 1);
          check_entry($file, $line, $tag, $tagLIN,
            "ERR_LIN_RS_INSERT", "rs",
            $c1, $c2, $c3, $noLIN, 1, 1, 1);
          check_entry($file, $line, $tag, $tagLIN,
            "ERR_LIN_RS_DELETE", "rs",
            $c1, $c2, $c3, $noLIN, 0, $repeat, 1);
          check_entry($file, $line, $tag, $tagLIN,
            "ERR_LIN_RS_DECL_PARD", "rs",
            $c1, $c2, $c3, $noLIN, 1, 1, 1);
          check_entry($file, $line, $tag, $tagLIN,
            "ERR_LIN_RS_DECL_OPP", "rs",
            $c1, $c2, $c3, $noLIN, 1, 1, 1);
          check_entry($file, $line, $tag, $tagLIN,
            "ERR_LIN_RS_DENOM", "rs",
            $c1, $c2, $c3, $noLIN, 1, 1, 1);
          check_entry($file, $line, $tag, $tagLIN,
            "ERR_LIN_RS_LEVEL", "rs",
            $c1, $c2, $c3, $noLIN, 1, 1, 1);
          check_entry($file, $line, $tag, $tagLIN,
            "ERR_LIN_RS_MULT", "rs",
            $c1, $c2, $c3, $noLIN, 1, 1, 1);
          check_entry($file, $line, $tag, $tagLIN,
            "ERR_LIN_RS_TRICKS", "rs",
            $c1, $c2, $c3, $noLIN, 1, 1, 1);
          check_entry($file, $line, $tag, $tagLIN,
            "ERR_LIN_RS_EMPTY", "rs",
            $c1, $c2, $c3, $noLIN, 1, 1, 1);
          check_entry($file, $line, $tag, $tagLIN,
            "ERR_LIN_RS_INCOMPLETE", "rs",
            $c1, $c2, $c3, $noLIN, 1, 1, 1);
          check_entry($file, $line, $tag, $tagLIN,
            "ERR_LIN_RS_SYNTAX", "rs",
            $c1, $c2, $c3, $noLIN, 1, 1, 1);
          
          check_line($file, $line, "ERR_LIN_PLAYERS_REPLACE",
            $c1, $c2, $c3, $noLIN, 0, 1, 1);
          check_line($file, $line, "ERR_LIN_PLAYERS_DELETE",
            $c1, $c2, $c3, $noLIN, 0, 1, 1);

          check_entry($file, $line, $tag, $tagLIN,
            "ERR_LIN_PN_REPLACE", "pn",
            $c1, $c2, $c3, $noLIN, 0, 1, 1);
          check_entry($file, $line, $tag, $tagLIN,
            "ERR_LIN_PN_INSERT", "pn",
            $c1, $c2, $c3, $noLIN, 0, 1, 1);
          check_entry($file, $line, $tag, $tagLIN,
            "ERR_LIN_PN_DELETE", "pn",
            $c1, $c2, $c3, $noLIN, 0, 1, 1);

          check_entry($file, $line, $tag, $tagLIN,
            "ERR_LIN_SV_REPLACE", "sv",
            $c1, $c2, $c3, $noLIN, 1, 1, 1);
          check_entry($file, $line, $tag, $tagLIN,
            "ERR_LIN_SV_INSERT", "sv",
            $c1, $c2, $c3, $noLIN, 1, 1, 1);
          check_entry($file, $line, $tag, $tagLIN,
            "ERR_LIN_SV_DELETE", "sv",
            $c1, $c2, $c3, $noLIN, 1, 1, 1);

          check_hand_line($file, $line, "ERR_LIN_MBIDDING_WRONG",
            $c1, $c2, $c3, 0);

          check_entry($file, $line, $tag, $tagLIN,
            "ERR_LIN_MB_TRAILING", "mb",
            $c1, $c2, $c3, $noLIN, 1, 0, 1);
          check_entry($file, $line, $tag, $tagLIN,
            "ERR_LIN_MB_REPLACE", "mb",
            $c1, $c2, $c3, $noLIN, 1, 1, 1);
          check_entry($file, $line, $tag, $tagLIN,
            "ERR_LIN_MB_INSERT", "mb",
            $c1, $c2, $c3, $noLIN, 1, 0, 1);
          check_entry($file, $line, $tag, $tagLIN,
            "ERR_LIN_MB_DELETE", "mb",
            $c1, $c2, $c3, $noLIN, 1, 0, 1);
          check_entry($file, $line, $tag, $tagLIN,
            "ERR_LIN_MB_SYNTAX", "mb",
            $c1, $c2, $c3, $noLIN, 1, 1, 1);

          check_entry($file, $line, $tag, $tagLIN,
            "ERR_LIN_AN_DELETE", "an",
            $c1, $c2, $c3, $noLIN, 1, 1, 1);

          check_entry($file, $line, $tag, $tagLIN,
            "ERR_LIN_PC_REPLACE", "pc",
            $c1, $c2, $c3, $noLIN, 1, 1, 1);
          check_entry($file, $line, $tag, $tagLIN,
            "ERR_LIN_PC_INSERT", "pc",
            $c1, $c2, $c3, $noLIN, 1, 1, 1);
          check_entry($file, $line, $tag, $tagLIN,
            "ERR_LIN_PC_DELETE", "pc",
            $c1, $c2, $c3, $noLIN, 1, $repeat, 1);
          
          check_entry($file, $line, $tag, $tagLIN,
            "ERR_LIN_MC_REPLACE", "mc",
            $c1, $c2, $c3, $noLIN, 1, 1, 1);
          check_entry($file, $line, $tag, $tagLIN,
            "ERR_LIN_MC_INSERT", "mc",
            $c1, $c2, $c3, $noLIN, 1, 1, 1);
          check_entry($file, $line, $tag, $tagLIN,
            "ERR_LIN_MC_DELETE", "mc",
            $c1, $c2, $c3, $noLIN, 1, 1, 1);

          check_line($file, $line, "ERR_LIN_TRICK_INSERT",
            $c1, $c2, $c3, $noLIN, 1, 0, 1);
          check_line($file, $line, "ERR_LIN_TRICK_DELETE", 
            $c1, $c2, $c3, $noLIN, 1, 0, 0);

          log_entry($file, $accum_expl_ref, $expl_seen{$tag}, 
            $noLIN, $tag, $c1, $c2, $c3);
          $expl_seen{$tag} = 1;
        }
      }
      else
      {
        log_entry($file, $accum_rem_ref, $rem_seen{ERR_SIZE}, 
          $noLIN, "ERR_SIZE", 0, 0, 0);
        $rem_seen{ERR_SIZE} = 1;
        print "File $file: '$line'\n";
      }
    }
    close $fr;
  }
}


sub write_ref_stats
{
  my ($fo, $codes_ref, $accum_expl_ref, $accum_rem_ref) = @_;

  printf $fo "%-28s %24s %11s | %12s\n", 
    "", "Explained", "", "Remaining";
  my $FORMAT = "%-28s %5s %6s %5s %5s %5s %5s | %6s %5s\n";

  printf $fo $FORMAT,
    "Reference", "#file", "#lin", "#ref", "#tags", "#qx", "#bds",
    "#lin", "#ref";

  my @sum;
  for (my $i = 0; $i <= $#$codes_ref; $i++)
  {
    next if ($accum_expl_ref->[$i]{lins} == 0 &&
        $accum_rem_ref->[$i]{lins} == 0);

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

  print $fo ("-" x 80) . "\n";
  printf $fo $FORMAT,
    "Sum", "N/A", @sum;

  printf $fo "\n\n";
}


sub write_skip_stats
{
  my ($fo, $codes_ref, $tag, $accum_expl_ref, $accum_rem_ref) = @_;

  printf $fo "%-28s %21s %8s | %12s\n", 
    "", "Explained", "", "Remaining";
  my $FORMAT = "%-28s %5s %6s %5s %5s %5s | %6s %5s\n";

  printf $fo $FORMAT,
    $tag, "#file", "#lin", "#ref", "#qx", "#bds",
    "#lin", "#ref";

  my @sum = qw(0 0 0 0 0 0);
  for (my $i = 0; $i <= $#$codes_ref; $i++)
  {
    next if ($accum_expl_ref->[$i]{lins} == 0 &&
        $accum_rem_ref->[$i]{lins} == 0);

    printf $fo $FORMAT,
      $codes_ref->[$i],
      $accum_expl_ref->[$i]{files},
      $accum_expl_ref->[$i]{lins},
      $accum_expl_ref->[$i]{refs},
      $accum_expl_ref->[$i]{qxs},
      $accum_expl_ref->[$i]{boards},
      $accum_rem_ref->[$i]{lins},
      $accum_rem_ref->[$i]{refs};

      $sum[0] += $accum_expl_ref->[$i]{lins};
      $sum[1] += $accum_expl_ref->[$i]{refs};
      $sum[2] += $accum_expl_ref->[$i]{qxs};
      $sum[3] += $accum_expl_ref->[$i]{boards};
      $sum[4] += $accum_rem_ref->[$i]{lins};
      $sum[5] += $accum_rem_ref->[$i]{refs};
  }

  print $fo ("-" x 74) . "\n";
  printf $fo $FORMAT,
    "Sum", "N/A", @sum;

  printf $fo "\n\n";
}


sub write_file_numbers
{
  my ($fo, $num_lin_files, $num_ref_files, $num_skip_files, 
    $num_orig_files, $num_noval_files) = @_;

  printf $fo "%-28s %5s\n",
    "Number of lin files", $num_lin_files;
  printf $fo "%-28s %5s\n",
    "Number of ref files", $num_ref_files;
  printf $fo "%-28s %5s\n",
    "Number of skip files", $num_skip_files;
  printf $fo "%-28s %5s\n",
    "Number of orig files", $num_orig_files;
  printf $fo "%-28s %5s\n",
    "Number of noval files", $num_noval_files;
}


sub quotes_to_content
{
  my ($file, $action, $str, $tagref, $repref) = @_;

  my @list = split ',', $str, -1;
  my $pos = -1;
  for my $a (0 .. $#list)
  {
    my $t = $list[$a];
    next if $t =~ /^\d+$/ || $t =~/^-\d+$/;
    $pos = $a;
    $$tagref = $t;
    last;
  }
  return if $pos == -1;

  if ($action eq "insertLIN")
  {
    if ($pos < $#list-1)
    {
      warn "$file, insertLIN has extras: $str";
      return;
    }
    $$repref = 1;
  }
  elsif ($action eq "deleteLIN")
  {
    if ($pos == $#list-1)
    {
      $$repref = 1;
    }
    elsif ($pos == $#list-2)
    {
      if ($list[$#list] !~ /^\d+$/)
      {
        warn "$file, deleteLIN count is non-numeric: $str";
        return;
      }
      $$repref = $list[$#list];
    }
    else
    {
      $$repref = 1;
    }
  }
  elsif ($action eq "replaceLIN")
  {
    if ($pos != $#list-2)
    {
      warn "$file, replaceLIN has extras: $str";
    }
    $$repref = 1;
  }
}


sub check_entry
{
  my ($file, $line, $tagERR, $tagLIN, $refERR, $refLIN, 
    $c1, $c2, $c3, $noLIN, $onehand, $onetag, $oneline) = @_;

  if ($tagERR eq $refERR)
  {
    if ($tagLIN ne $refLIN)
    {
      warn "$file, $line: Not $refLIN" unless $refERR eq "ERR_LIN_MB_INSERT";
    }
    if ($onehand && ($c2 != 1 || $c3 != 1))
    {
      warn "$file, $line: Not (a,1,1)";
    }
    if ($onetag && $c1 != 1 && $c1 != $onetag && 
        ($c1 < 0.5*$onetag || $c1 > 1.5*$onetag))
    {
      warn "$file, $line: Not (1,a,a): $onetag, $c1";
    }
    if ($c1 == 0)
    {
      warn "$file, $line: (0,a,a)";
    }
    if ($oneline && $noLIN != 1)
    {
      warn "$file, $line: NoLIN";
    }
  }
}

sub check_line
{
  my ($file, $line, $tag,
    $c1, $c2, $c3, $noLIN, $onehand, $onetag, $oneline) = @_;

  return unless $line =~ /$tag/;

  if ($onehand && ($c2 != 1 || $c3 != 1))
  {
    warn "$file, $line: Not (a,1,1)";
  }
  if ($onetag && $c1 != $onetag && ($c1 < $onetag || $c1 > 1.5*$onetag))
  {
    warn "$file, $line: Not (1,a,a)";
  }
  if ($oneline && $noLIN != 1)
  {
    warn "$file, $line: NoLIN";
  }
}

sub check_hand_line
{
  my ($file, $line, $tag, $c1, $c2, $c3, $onetag) = @_;

  return unless $line =~ /$tag/;

  if ($c2 == 0 || $c3 == 0)
  {
    warn "$file, $line: (a,0,0)";
  }

  if ($c2 < $c3 || $c2 > 2*$c3)
  {
    warn "$file, $line: Not (a,1..2,1)";
  }

  if ($onetag && $c1 != $onetag)
  {
    warn "$file, $line: Not (1,a,a)";
  }
}
