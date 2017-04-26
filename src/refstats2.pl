#!perl

use strict;
use warnings;

# Reads all files in directory argument that end on .ref,
# generates statistics in refstats.txt, and tests content.

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

my $ASSOC_TAG_UNKNOWN = 0;
my $ASSOC_ASSOC_UNKNOWN = 1;
my $ASSOC_OK_GLOBAL = 2;
my $ASSOC_OK_LOCAL = 3;


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

my (%tag_lists, %tag_global, %tag_no_global_count, %tag_global_count);
set_tag_assocs();

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


sub set_tag_assocs
{
  $tag_global{vg} = 1;
  $tag_global{pn} = 1;

  $tag_no_global_count{ERR_LIN_VG_FIRST} = 1;
  $tag_no_global_count{ERR_LIN_VG_LAST} = 1;
  $tag_no_global_count{ERR_LIN_VG_SYNTAX} = 1;
  $tag_no_global_count{ERR_LIN_PN_DELETE} = 1;

  $tag_no_global_count{ERR_LIN_VHEADER_INSERT} = 1;
  $tag_no_global_count{ERR_LIN_VHEADER_SYNTAX} = 1;
  $tag_no_global_count{ERR_LIN_RESULTS_INSERT} = 1;
  $tag_no_global_count{ERR_LIN_RESULTS_DELETE} = 1;
  $tag_no_global_count{ERR_LIN_PLAYERS_DELETE} = 1;

  $tag_global_count{ERR_LIN_MD_MISSING} = 1;
  $tag_global_count{ERR_LIN_HAND_OUT_OF_RANGE} = 1;
  $tag_global_count{ERR_LIN_HAND_DUPLICATE} = 1;
  $tag_global_count{ERR_LIN_HAND_AUCTION_NONE} = 1;
  $tag_global_count{ERR_LIN_HAND_AUCTION_WRONG} = 1;
  $tag_global_count{ERR_LIN_HAND_AUCTION_LIVE} = 1;
  $tag_global_count{ERR_LIN_HAND_AUCTION_ABBR} = 1;
  $tag_global_count{ERR_LIN_HAND_CARDS_MISSING} = 1;
  $tag_global_count{ERR_LIN_HAND_CARDS_WRONG} = 1;
  $tag_global_count{ERR_LIN_HAND_PLAY_MISSING} = 1;
  $tag_global_count{ERR_LIN_HAND_PLAY_WRONG} = 1;
  $tag_global_count{ERR_LIN_HAND_DIRECTOR} = 1;

  @{$tag_lists{vg}} = qw(
    ERR_LIN_VG_FIRST
    ERR_LIN_VG_LAST
    ERR_LIN_VG_REPLACE
    ERR_LIN_VG_SYNTAX
  );

  @{$tag_lists{rs}} = qw(
    ERR_LIN_RS_REPLACE
    ERR_LIN_RS_INSERT 
    ERR_LIN_RS_DELETE
    ERR_LIN_RS_DECL_PARD
    ERR_LIN_RS_DECL_OPP
    ERR_LIN_RS_DENOM
    ERR_LIN_RS_LEVEL
    ERR_LIN_RS_MULT
    ERR_LIN_RS_TRICKS
    ERR_LIN_RS_EMPTY
    ERR_LIN_RS_INCOMPLETE
    ERR_LIN_RS_SYNTAX
  );

  @{$tag_lists{pn}} = qw(
    ERR_LIN_PN_REPLACE
    ERR_LIN_PN_INSERT
    ERR_LIN_PN_DELETE
  );

  @{$tag_lists{qx}} = qw(
    ERR_LIN_QX_REPLACE
    ERR_LIN_QX_UNORDERED
  );

  @{$tag_lists{md}} = qw(
    ERR_LIN_MD_REPLACE
    ERR_LIN_MD_MISSING
    ERR_LIN_MD_SYNTAX
  );

  @{$tag_lists{nt}} = qw(
    ERR_LIN_NT_SYNTAX
  );

  @{$tag_lists{sv}} = qw(
    ERR_LIN_SV_REPLACE
    ERR_LIN_SV_INSERT
    ERR_LIN_SV_DELETE
    ERR_LIN_SV_SYNTAX
  );

  @{$tag_lists{mb}} = qw(
    ERR_LIN_MB_TRAILING
    ERR_LIN_MB_REPLACE
    ERR_LIN_MB_INSERT
    ERR_LIN_MB_DELETE
    ERR_LIN_MB_SYNTAX
  );

  @{$tag_lists{an}} = qw(
    ERR_LIN_AN_DELETE
  );

  @{$tag_lists{pc}} = qw(
    ERR_LIN_PC_REPLACE
    ERR_LIN_PC_INSERT
    ERR_LIN_PC_DELETE
    ERR_LIN_PC_SYNTAX
    ERR_LIN_TRICK_DELETE
  );

  @{$tag_lists{mc}} = qw(
    ERR_LIN_MC_REPLACE
    ERR_LIN_MC_INSERT
    ERR_LIN_MC_DELETE
    ERR_LIN_MC_SYNTAX
  );
}


sub assoc_ok
{
  my ($tag, $assoc) = @_;

  return $ASSOC_TAG_UNKNOWN unless defined $tag_lists{$tag};

  for my $a (@{$tag_lists{$tag}})
  {
    return (defined $tag_global{$tag} ? $ASSOC_OK_GLOBAL : $ASSOC_OK_LOCAL) 
      if $a eq $assoc;
  }
  return $ASSOC_ASSOC_UNKNOWN;
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

      my ($refNo, $noLIN, $action, $tagNo, $tagLIN, $repeat);
      $tagLIN = "";
      $repeat = 0;
      $refNo = 0;
      if ($line =~ /^(\d+)\s+(\w+)\s+(\d+)/)
      {
        $noLIN = $3;
        $action = $2;
        $refNo = $1;
      }
      elsif ($line =~ /^(\d+)\s+(\w+)/)
      {
        $noLIN = 1;
        $action = $2;
        $refNo = $1;
        if ($action =~ /LIN/ && $line =~ /^\d+\s+\w+\s+\"([^"]*)\"/)
        {
          my $quotes = $1;
          quotes_to_content($file, $action, $quotes, \$tagNo, \$tagLIN, \$repeat);
        }
      }
      elsif (($file =~ /skip/ || $file =~ /noval/) && $line =~ /^(\d+)\s+/)
      {
        $noLIN = $1;
        $action = "";
      }
      else
      {
        warn "Bad line: File '$file', line '$line'";
        next;
      }

      if ($line !~ /\{(.*)\}\s*$/)
      {
        log_entry($file, $accum_rem_ref, $rem_seen{ERR_SIZE}, 
          $noLIN, "ERR_SIZE", 0, 0, 0);
        $rem_seen{ERR_SIZE} = 1;
        print "File $file: '$line'\n";
        next;
      }

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
        log_entry($file, $accum_expl_ref, $expl_seen{$tag},
          $noLIN, $tag, $c1, $c2, $c3);
        $expl_seen{$tag} = 1;

        if ($tagLIN eq "" && $action !~ /LIN/)
        {
          if ($action eq "")
          {
            if ($file =~ /skip/ || $file =~ /noval/)
            {
# check_whole_file_count($file, $line, $c1, $c2, $c3);

              my %count;
              my $filelin = $file;
              $filelin =~ s/skip$/lin/;
              $filelin =~ s/noval$/lin/;
              count_tags($filelin, 0, 999999, \%count);

              if ($noLIN != $count{lines})
              {
                warn "$file: Bad line $noLIN vs. count $count{lines}";
                next;
              }
          
              if ($c1 != 0)
              {
                warn "$file: $c1 is 0 by convention";
                next;
              }

              if ($c2 != $count{qx} || $c3 != $count{bd})
              {
                warn "Bad skip count $file,\n$line, $count{qx}, $count{bd}\n\n";
                next;
              }
            }
            else
            {
              warn "$file: $line\n";
            }
          }
          elsif (defined $tag_global_count{$tag})
          {
            my %count;
            my $filelin = $file;
            $filelin =~ s/skip$/lin/;
            $filelin =~ s/ref$/lin/;
            $filelin =~ s/noval$/lin/;
            count_tags($filelin, $refNo, $refNo+$noLIN-1, \%count);
          
            if ($c1 != 0 || $c2 != $count{qx} || $c3 != $count{bd})
            {
              next if defined $tag_no_global_count{$tag};

              warn "Bad non-LIN count $file,\n$line, $count{qx}, $count{bd}\n\n";
              next;
            }
          }
          elsif ($action eq "replace")
          {
            # TODO
          }
          elsif ($action eq "insert")
          {
            # TODO
          }
          elsif ($action eq "delete")
          {
            # TODO
# print "$file: $line\n";
          }
          else
          {
            warn "$file, $line: Bad action $action";
          }
        }
        else
        {
          # Check replaceLIN, insertLIN, deleteLIN.

          my $assoc = assoc_ok($tagLIN, $tag);
          if ($assoc == $ASSOC_TAG_UNKNOWN)
          {
            next if ($tag eq "ERR_LIN_SYNTAX"); # Accept any tag
            next if ($tagLIN eq "" && $tag eq "ERR_LIN_PC_SYNTAX"); 
            # Accept empty tag in ERR_LIN_PC_SYNTAX

            $assoc = assoc_ok(lc($tagLIN), $tag);
            next if ($assoc == $ASSOC_OK_GLOBAL || 
                $assoc == $ASSOC_OK_LOCAL);

            warn "Bad tag $file,\n$e, $tagLIN, $tag\n\n";
            next;
          }
          elsif ($assoc == $ASSOC_ASSOC_UNKNOWN)
          {
            next if ($tag eq "ERR_LIN_SYNTAX"); # Accept any tag

            warn "Bad association $file,\n$e, $tagLIN, $tag\n\n";
            next;
          }
          elsif ($assoc == $ASSOC_OK_GLOBAL)
          {
            if ($c1 != $repeat && ! defined $tag_no_global_count{$tag})
            {
              warn "Bad global repeat $file,\n$e, $c1, $repeat\n\n";
              next;
            }

            my %count;
            my $filelin = $file;
            $filelin =~ s/skip$/lin/;
            $filelin =~ s/ref$/lin/;
            $filelin =~ s/noval$/lin/;
            count_tags($filelin, 0, 999999, \%count);
          
            if ($c2 != $count{qx} || $c3 != $count{bd})
            {
              next if defined $tag_no_global_count{$tag};

              warn "Bad global count $file,\n$e, $c2, $c3\n\n";
              next;
            }
          }
          elsif ($assoc == $ASSOC_OK_LOCAL)
          {
            if ($repeat == 1)
            {
              if ($c1 != 1)
              {
                my $numpipes = ($line =~ tr/\|//);
                if (($numpipes % 2 == 0) &&
                    $c1 == (1 + $numpipes/2))
                {
                  next;
                }
                warn "Bad local repeat $file,\n$e, $c1, $repeat\n\n";
                next;
              }
            }
            elsif ($c1 != $repeat)
            {
              if ($tag eq "ERR_LIN_RS_DELETE")
              {
                if ($c1 == 1 && $c2 == $repeat && $c3 == $repeat/2)
                {
                  next;
                }

                warn "Accepting repeat $file,\n$e, $tagLIN, $tag\n\n";
                next;
              }

              my $filelin = $file;
              $filelin =~ s/skip$/lin/;
              $filelin =~ s/ref$/lin/;
              $filelin =~ s/noval$/lin/;
              my $fline = getline($filelin, $refNo);

              my %count;
              incr_tag_count($fline, $tagNo-1, $repeat, \%count);
              if ($c1 != $count{realtags})
              {
                warn "Bad local repeat $file,\n$e, stated $c1, counted $count{realtags}\n\n";
                next;
              }
            }
          
            if ($c2 != 1 || $c3 != 1)
            {
              warn "Bad local count $file,\n$e, $c2, $c3\n\n";
              next;
            }
          }
          else
          {
            die "Bad assoc return $assoc";
          }
        }
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
  my ($file, $action, $str, $tagnoref, $tagref, $repref) = @_;

  my @list = split ',', $str, -1;
  my $pos = -1;
  $$tagnoref = $list[0];
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


sub count_tags
{
  my ($file, $first, $last, $cref) = @_;

  $cref->{lines} = 0;
  $cref->{qx} = 0;
  $cref->{bd} = 0;
  $cref->{tags} = 0;
  $cref->{realtags} = 0;

  my $prev = "";
  my $curr;
  my $lno = 0;

  my @seen;
  open my $fr, '<', $file or die "Can't open $file $!";
  while (my $line = <$fr>)
  {
    $lno++;
    next unless $lno >= $first;
    last unless $lno <= $last;
    $cref->{lines}++;

    if ($line =~ /^qx\|([^,\|]+)/ || $line =~ /\|qx\|([^,\|]+)/)
    {
      $curr = $1;
      $curr = substr $curr, 1;
      if (! defined $seen[$curr])
      {
        $seen[$curr] = 1;
        $cref->{bd}++;
      }

      $cref->{qx}++;
      # $cref->{bd}++ if ($curr ne $prev);
      $prev = $curr;
    }
    next if $line !~ /\|/;
    incr_tag_count($line, 0, 0, $cref);
  }
  close $fr;
}


sub incr_tag_count
{
  my ($line, $start, $count, $cref) = @_;

  my @list = split /\|/, $line, -1;
  my $l = $#list;
  $l-- unless ($l % 2);
  my $p;
  if ($start >= 0)
  {
    $p = 2*$start;
  }
  else
  {
    $p = $l + 3 + 2*$start;
  }

  if ($p > $l)
  {
    die "Start $start is too large ($l)\n";
  }

  if ($count != 0)
  {
    if ($p + 2*$count -1 > $l)
    {
      die "Huh?";
    }
    $l = $p + 2*$count - 1;
  }

  my $i;
  for ($i = $p; $i <= $l; $i += 2)
  {
    $cref->{tags}++;
    if ($list[$i] ne 'pg')
    {
      $cref->{realtags}++;
    }
  }
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

