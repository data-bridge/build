#!perl

use strict;
use warnings;
use File::Basename;

# Reads all files in directory argument that end on .ref,
# generates statistics in refstats.txt, and tests content.
# Directory may be absolute or relative to the central hands directory.

if ($#ARGV != 0 && $#ARGV != 2)
{
  print "Usage:\n";
  print "perl refstats.pl dir\n";
  print "perl refstats.pl dir year 2009\n";
  exit;
}

# On laptop (CDD), no "hands" in between.
my $HANDDIR = "../../../bridgedata";
$HANDDIR .= "/hands" unless (`uname -a` =~ /CDD/);

my $indir = get_dir_name($ARGV[0]) or die "Bad directory '$ARGV[0]'";

my $DATES_FILE = "dates.log";

my ($out_file, $year, @year_index);
if ($#ARGV == 2)
{
  die "Not 'year'" unless $ARGV[1] eq 'year';
  $year = $ARGV[2];
  die "Not a year: $year" unless $year =~ /^\d+$/;
  read_year_index($indir, \@year_index);

  $out_file = "refstats$year.txt";
}
else
{
  $year = 0;
  $out_file = "refstats.txt";
}

my (@codes, %code_hash);
my $CODE_DIR = glob("~/GitHub/Build/src");
my $CODE_FILE = "refcodes.h";
read_error_codes("$CODE_DIR/$CODE_FILE", \@codes, \%code_hash);

my @INPUTS = qw(lin pbn rbn rbx eml txt rec);
my (%input_list, @ref_list);
make_input_list($indir, \%input_list, \@ref_list);
my @CATEGORIES = qw(ref skip noval order);

my $ASSOC_TAG_UNKNOWN = 0;
my $ASSOC_ASSOC_UNKNOWN = 1;
my $ASSOC_OK_GLOBAL = 2;
my $ASSOC_OK_LOCAL = 3;

my (%tag_lists, %tag_global, %tag_no_global_count, %tag_global_count);
my (%tag_globalRBN);
set_tag_assocs();

# {lin .. rec}{ref,skip,noval,order}
my (%accum, %file_seen, @year_histo);
make_stats(\@ref_list, \%accum, \@year_histo);

open my $fo, '>', $out_file or die "Can't open $out_file: $!";
write_ref_stats($fo, \@codes, \%accum);
write_file_numbers($fo, \%input_list, \@ref_list);
write_year_histo($fo, \@year_histo) if $year > 0;
close $fo;


sub get_dir_name
{
  my $arg = pop;
  return $arg if -e $arg; # Absolute
  return "$HANDDIR/$arg" if -e "$HANDDIR/$arg"; # Relative

  # Try inside the central hand directory as wlel.
  opendir(DIR, "$HANDDIR") or die "Cannot open $HANDDIR\n";
  my @files = readdir(DIR);
  closedir(DIR);

  foreach my $file (@files) 
  {
    next unless -d "$HANDDIR/$file";
    return "$HANDDIR/$file/$arg" if -e "$HANDDIR/$file/$arg";
  }

  return "";
}


sub read_year_index
{
  my ($indir, $year_list_ref) = @_;
  open my $fy, '<', "$indir/$DATES_FILE" or die "Can't open $DATES_FILE: $!";
  while (my $line = <$fy>)
  {
    die "Bad date line: $line" unless
      $line =~ /^(\d+).lin\s+(\d+)-(\d+)-(\d+)\s*$/;
    my ($fn, $yyyy, $mm, $dd) = ($1, $2, $3, $4);
    $year_list_ref->[$fn] = $yyyy;
  }
  close $fy;
}


sub read_error_codes
{
  my ($fname, $codes_ref, $code_hash_ref) = @_;

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
      $line = <$ff>; # Skip leading curly bracket
      $seen = 1;
    }
  }
  close $ff;

  foreach my $i (0 .. $#$codes_ref)
  {
    $code_hash_ref->{$codes_ref->[$i]} = $i;
  }
}


sub make_input_list
{
  my ($indir, $input_list_ref, $ref_list_ref) = @_;

  my @dirs = ($indir);
  my %seen;

  my %extensions;
  $extensions{$_} = 1 for @INPUTS;

  while (my $pwd = shift @dirs) 
  {
    opendir(DIR, "$pwd") or die "Cannot open $pwd\n";
    my @files = sort readdir(DIR);
    closedir(DIR);

    foreach my $file (sort @files) 
    {
      if (-d "$pwd/$file" and ($file !~ /^\.\.?$/) and ! $seen{$file}) 
      {
        $seen{$file} = 1;
        push @dirs, "$pwd/$file";
      }

      next unless $file =~ /\.(\w+)$/;
      my $ext = lc $1;

      if (defined $extensions{$ext})
      {
        push @{$input_list_ref->{$ext}}, "$pwd/$file";
      }
      elsif ($ext eq 'ref')
      {
        push @$ref_list_ref, "$pwd/$file";
      }
    }
  }
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

  $tag_global_count{ERR_LIN_VHEADER_INSERT} = 1;
  $tag_global_count{ERR_LIN_RESULTS_INSERT} = 1;
  $tag_global_count{ERR_LIN_MD_MISSING} = 1;
  $tag_global_count{ERR_LIN_HAND_OUT_OF_RANGE} = 1;
  $tag_global_count{ERR_LIN_HAND_DUPLICATED} = 1;
  $tag_global_count{ERR_LIN_HAND_AUCTION_NONE} = 1;
  $tag_global_count{ERR_LIN_HAND_AUCTION_WRONG} = 1;
  $tag_global_count{ERR_LIN_HAND_AUCTION_LIVE} = 1;
  $tag_global_count{ERR_LIN_HAND_AUCTION_ABBR} = 1;
  $tag_global_count{ERR_LIN_HAND_CARDS_MISSING} = 1;
  $tag_global_count{ERR_LIN_HAND_CARDS_WRONG} = 1;
  $tag_global_count{ERR_LIN_HAND_PLAY_MISSING} = 1;
  $tag_global_count{ERR_LIN_HAND_PLAY_WRONG} = 1;
  $tag_global_count{ERR_LIN_HAND_DIRECTOR} = 1;
  $tag_global_count{ERR_LIN_SYNTAX} = 1;

  $tag_globalRBN{ERR_RBN_N_REPLACE} = 1;

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
    ERR_LIN_AN_REPLACE
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


sub make_stats
{
  my ($files_ref, $accum_ref, $year_histo_ref) = @_;

  foreach my $file (@$files_ref)
  {
    if ($year > 0)
    {
      my $b = basename($file, ".ref");
      warn "Bad file $file" unless $b =~ /^(\d+)$/;
      my $no = $1;
      next unless defined $year_index[$no] && $year_index[$no] == $year;
      $year_histo_ref->[$no / 1000]++;
    }

    my $src_file = get_source_file($file) or die "No source for $file: $!";
    $src_file =~ /\.([^.]+)$/;
    my $src_ext = $1;

    open my $fr, '<', $file or die "Can't open $file: $!";
    while (my $line = <$fr>)
    {
      chomp $line;
      $line =~ s///g;

      my %counts;
      if ($line =~ /^(\w+)\s+\{(.*)\((\d+),(\d+),(\d+)\)\}$/)
      {
        # skip, noval, order
        $counts{action} = $1;
        $counts{tag_ref} = $2;
        $counts{lines} = $3;
        $counts{qxs} = $4;
        $counts{bds} = $5;
        $counts{tags} = 0;
        $counts{repeat_lines} = 1;

        log_entry($file, $src_ext, $counts{action}, $accum_ref, \%counts);

        my %file_counts;
        count_file($src_file, $src_ext, \%file_counts);
        compare_file_counts($file, \%counts, \%file_counts);
        next;
      }


      if ($src_ext eq 'lin')
      {
        if (! parse_refLIN($file, $line, \%counts))
        {
          warn "$file, $line: Could not parse";
          next;
        }

        check_refLIN($file, $src_file, $line, \%counts);
      }
      elsif ($src_ext eq 'pbn')
      {
        if (! parse_refPBN($file, $line, \%counts))
        {
          warn "$file, $line: Could not parse";
          next;
        }

        check_refPBN($file, $src_file, $line, \%counts);
      }
      elsif ($src_ext eq 'rbn')
      {
        if (! parse_refRBN($file, $line, \%counts))
        {
          warn "$file, $line: Could not parse";
          next;
        }

        check_refRBN($file, $src_file, $line, \%counts);
      }
      else
      {
        warn "$file: $src_ext not yet implemented";
      }

      log_entry($file, $src_ext, 'ref', $accum_ref, \%counts);
    }
    close $fr;
  }
}


sub get_source_file
{
  my $ref_file = pop;
  for my $ext (@INPUTS)
  {
    my $src = $ref_file;
    $src =~ s/.ref$/.$ext/;
    return $src if -e $src;
  }
  return "";
}


sub log_entry
{
  my ($file, $ext, $type, $accum_ref, $counts_ref) = @_;

  if (! defined $code_hash{$counts_ref->{tag_ref}})
  {
    warn "Undefined code in $file: $counts_ref->{tag_ref}";
    return;
  }
  my $c = $code_hash{$counts_ref->{tag_ref}};

  my $t;
if (! defined $type)
{
  print "File $file\n";
}
  if ($type eq 'ref')
  {
    $t = 'ref';
  }
  elsif ($type =~ /^skip/)
  {
    $t = 'skip';
  }
  elsif ($type =~ /^noval/)
  {
    $t = 'noval';
  }
  elsif ($type =~ /^order/)
  {
    $t = 'order';
  }
  else
  {
    die "Bad type '$type'";
  }

  $accum_ref->{$ext}{$t}[$c]{files}++ 
    unless defined $file_seen{$ext}{$t}[$c]{$file};
  $accum_ref->{$ext}{$t}[$c]{refs}++;
  $accum_ref->{$ext}{$t}[$c]{lines} += $counts_ref->{repeat_lines};
  $accum_ref->{$ext}{$t}[$c]{tags} += $counts_ref->{tags};
  $accum_ref->{$ext}{$t}[$c]{qxs} += $counts_ref->{qxs};
  $accum_ref->{$ext}{$t}[$c]{boards} += $counts_ref->{bds};

  $file_seen{$ext}{$t}[$c]{$file} = 1;
}


sub parse_refLIN
{
  my ($file, $line, $counts_ref) = @_;

  $counts_ref->{lno} = 0;
  $counts_ref->{action} = "";
  $counts_ref->{repeat_lines} = 0;

  if ($line !~ /\{(.+)\((\d+),(\d+),(\d+)\)\}/)
  {
    warn "Bad entry: $file, $line";
    return 0;
  }
  $counts_ref->{tag_ref} = $1;
  $counts_ref->{tags} = $2;
  $counts_ref->{qxs} = $3;
  $counts_ref->{bds} = $4;

  if ($line =~ /^(\d+)-(\d+)\s+(\w+)/)
  {
    $counts_ref->{lno} = $1;
    $counts_ref->{action} = $3;
    $counts_ref->{repeat_lines} = $2 - $counts_ref->{lno} + 1;
    return 1;
  }
  elsif ($line =~ /^(\d+)\s+(\w+)/)
  {
    $counts_ref->{lno} = $1;
    $counts_ref->{action} = $2;
    $counts_ref->{repeat_lines} = 1;

    if ($counts_ref->{action} =~ /LIN/ && $line =~ /^\d+\s+\w+\s+\"([^"]*)\"/)
    {
      my $quoted = $1;
      quotes_to_contentLIN($file, $quoted, $counts_ref);
      if (! defined $counts_ref->{tag_source} &&
          $counts_ref->{tag_ref} eq 'ERR_LIN_PC_SYNTAX')
      {
        $counts_ref->{tag_source} = 'pc';
        $counts_ref->{repeat_tags} = 1;
      }
    }
    return 1;
  }
  else
  {
    return 0;
  }
}


sub parse_refPBN
{
  my ($file, $line, $counts_ref) = @_;

  $counts_ref->{lno} = 0;
  $counts_ref->{action} = "";
  $counts_ref->{repeat_lines} = 0;

  if ($line !~ /\{(.+)\((\d+),(\d+),(\d+)\)\}/)
  {
    warn "Bad entry: $file, $line";
    return 0;
  }
  $counts_ref->{tag_ref} = $1;
  $counts_ref->{tags} = $2;
  $counts_ref->{qxs} = $3;
  $counts_ref->{bds} = $4;

  if ($line =~ /^(\d+)\s+(\w+)\s+(\d+)/)
  {
    $counts_ref->{lno} = $1;
    $counts_ref->{action} = $2;
    $counts_ref->{repeat_lines} = $3;
    return 1;
  }
  elsif ($line =~ /^(\d+)\s+(\w+)/)
  {
    $counts_ref->{lno} = $1;
    $counts_ref->{action} = $2;
    $counts_ref->{repeat_lines} = 1;

    if ($counts_ref->{action} =~ /PBN/ && $line =~ /^\d+\s+\w+\s+\"(.*)\" \{/)
    {
      my $quoted = $1;
      quotes_to_contentPBN($file, $quoted, $counts_ref);
    }
    return 1;
  }
  else
  {
    return 0;
  }

}


sub parse_refRBN
{
  my ($file, $line, $counts_ref) = @_;

  $counts_ref->{lno} = 0;
  $counts_ref->{action} = "";
  $counts_ref->{repeat_lines} = 0;

  if ($line !~ /\{(.+)\((\d+),(\d+),(\d+)\)\}/)
  {
    warn "Bad entry: $file, $line";
    return 0;
  }
  $counts_ref->{tag_ref} = $1;
  $counts_ref->{tags} = $2;
  $counts_ref->{qxs} = $3;
  $counts_ref->{bds} = $4;

  if ($line =~ /^(\d+)\s+(\w+)\s+(\d+)/)
  {
    $counts_ref->{lno} = $1;
    $counts_ref->{action} = $2;
    $counts_ref->{repeat_lines} = $3;
    return 1;
  }
  elsif ($line =~ /^(\d+)\s+(\w+)/)
  {
    $counts_ref->{lno} = $1;
    $counts_ref->{action} = $2;
    $counts_ref->{repeat_lines} = 1;

    if ($counts_ref->{action} =~ /RBN/ && $line =~ /^\d+\s+\w+\s+\"([^"]*)\"/)
    {
      my $quoted = $1;
      quotes_to_contentRBN($file, $quoted, $counts_ref);
    }
    return 1;
  }
  else
  {
    return 0;
  }

}


sub count_file
{
  my ($src_file, $ext, $counts_ref) = @_;
  if ($ext eq 'lin')
  {
    countLIN($src_file, 0, 999999, $counts_ref);
  }
  elsif ($ext eq 'pbn')
  {
    countPBN($src_file, 0, 999999, $counts_ref);
  }
  elsif ($ext eq 'rbn')
  {
    countRBN($src_file, 0, 999999, $counts_ref);
  }
  elsif ($ext eq 'rbx')
  {
    die "TODO extension 'ext'";
  }
  elsif ($ext eq 'txt')
  {
    die "TODO extension 'ext'";
  }
  elsif ($ext eq 'eml')
  {
    die "TODO extension 'ext'";
  }
  elsif ($ext eq 'rec')
  {
    die "TODO extension 'ext'";
  }
  else
  {
    die "Bad extension 'ext'";
  }
}


sub countLIN
{
  my ($file, $first, $last, $cref) = @_;

  $cref->{lines} = 0;
  $cref->{qxs} = 0;
  $cref->{bds} = 0;
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
        $cref->{bds}++;
      }

      $cref->{qxs}++;
      # $cref->{bd}++ if ($curr ne $prev);
      $prev = $curr;
    }
    next if $line !~ /\|/;
    incr_tag_count($line, 0, 0, $cref);
  }
  close $fr;
}


sub countPBN
{
  my ($file, $first, $last, $cref) = @_;

  $cref->{lines} = 0;
  $cref->{qxs} = 0;
  $cref->{bds} = 0;

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

    if ($line =~ /^\[Board \"(.*)\"\]/)
    {
      $curr = $1;
      $cref->{qxs}++;
      if ($curr ne '#' && ! defined $seen[$curr])
      {
        $seen[$curr] = 1;
        $cref->{bds}++;
      }
    }
  }
  close $fr;

  if ($cref->{qxs} == 1 && $cref->{bds} == 0)
  {
    $cref->{bds} = 1;
  }
}


sub countRBN
{
  my ($file, $first, $last, $cref) = @_;

  $cref->{lines} = 0;
  $cref->{qxs} = 0;
  $cref->{bds} = 0;

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

    chomp $line;
    $cref->{bds}++ if ($line =~ /^B (\d+)/);
    $cref->{qxs}++ if ($line =~ /^R /);
  }
  close $fr;

  if ($cref->{qxs} == 1 && $cref->{bds} == 0)
  {
    $cref->{bds} = 1;
  }
}


sub write_ref_stats
{
  my ($fo, $codes_ref, $accum_ref) = @_;

  my $FORMAT = "%-28s %5s %6s %5s %5s %5s %5s\n";

  for my $ext (@INPUTS)
  {
    for my $cat (@CATEGORIES)
    {
      next unless defined $accum_ref->{$ext}{$cat};

      print $fo "$ext, $cat\n\n";
      printf $fo $FORMAT,
        "Reference", "#file", "#lin", "#ref", "#tags", "#qx", "#bds";

      my @sum;
      for (my $i = 0; $i <= $#$codes_ref; $i++)
      {
        next unless defined $accum_ref->{$ext}{$cat}[$i];
        my $code = $codes[$i];

        printf $fo $FORMAT,
          # $codes_ref->{$ext}{$cat}[$i],
          $code,
          $accum_ref->{$ext}{$cat}[$i]{files},
          $accum_ref->{$ext}{$cat}[$i]{lines},
          $accum_ref->{$ext}{$cat}[$i]{refs},
          $accum_ref->{$ext}{$cat}[$i]{tags},
          $accum_ref->{$ext}{$cat}[$i]{qxs},
          $accum_ref->{$ext}{$cat}[$i]{boards};

          $sum[0] += $accum_ref->{$ext}{$cat}[$i]{lines};
          $sum[1] += $accum_ref->{$ext}{$cat}[$i]{refs};
          $sum[2] += $accum_ref->{$ext}{$cat}[$i]{tags};
          $sum[3] += $accum_ref->{$ext}{$cat}[$i]{qxs};
          $sum[4] += $accum_ref->{$ext}{$cat}[$i]{boards};
      }

      next unless $#sum >= 0;
      print $fo ("-" x 65) . "\n";
      printf $fo $FORMAT,
        "Sum", "N/A", @sum;

      printf $fo "\n\n";
    }
  }
}


sub assoc_ok
{
  my ($tag, $assoc) = @_;

if (! defined $tag)
{
  print "HERE\n";
}
  return $ASSOC_TAG_UNKNOWN unless defined $tag_lists{$tag};

  for my $a (@{$tag_lists{$tag}})
  {
    return (defined $tag_global{$tag} ? $ASSOC_OK_GLOBAL : $ASSOC_OK_LOCAL) 
      if $a eq $assoc;
  }
  return $ASSOC_ASSOC_UNKNOWN;
}


sub compare_file_counts
{
  my ($file, $counts_ref, $file_counts_ref) = @_;
  for my $k (qw(lines qxs bds))
  {
    if ($counts_ref->{$k} != $file_counts_ref->{$k})
    {
      print "$file, $k: $counts_ref->{$k} vs $file_counts_ref->{$k}\n";
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


sub quotes_to_contentLIN
{
  my ($file, $str, $counts_ref) = @_;

  my @list = split ',', $str, -1;
  my $pos = -1;
  for my $a (0 .. $#list)
  {
    my $t = $list[$a];
    next if $t =~ /^\d+$/ || $t =~/^-\d+$/;
    $pos = $a;
    $counts_ref->{tag_source} = $t;
    last;
  }
  return if $pos == -1;

  $counts_ref->{tag_start} = $list[0];

  my $action = $counts_ref->{action};
  if ($action eq 'insert')
  {
    # OK
  }
  elsif ($action eq 'delete')
  {
    # OK
  }
  elsif ($action eq 'replace')
  {
    # OK
  }
  elsif ($action eq "insertLIN")
  {
    if ($pos < $#list-1)
    {
      warn "$file, insertLIN has extras: $str";
      return;
    }
    $counts_ref->{repeat_tags} = 1;
  }
  elsif ($action eq "deleteLIN")
  {
    if ($pos == $#list-1)
    {
      $counts_ref->{repeat_tags} = 1;
    }
    elsif ($pos == $#list-2)
    {
      if ($list[$#list] !~ /^\d+$/)
      {
        warn "$file, deleteLIN count is non-numeric: $str";
        return;
      }
      $counts_ref->{repeat_tags} = $list[$#list];
    }
    else
    {
      $counts_ref->{repeat_tags} = 1;
    }
  }
  elsif ($action eq "replaceLIN")
  {
    if ($pos != $#list-2)
    {
      warn "$file, replaceLIN has extras: $str";
    }
    $counts_ref->{repeat_tags} = 1;
  }
  else
  {
    warn "$file, bad action: $action in '$str'";
  }
}


sub quotes_to_contentPBN
{
  my ($file, $str, $counts_ref) = @_;

  my @list = split ',', $str, -1;

  die "TODO";
}


sub incr_tag_count
{
  my ($line, $start, $count, $cref) = @_;

  my @list = split /\|/, $line, -1;
  my $l = $#list;
  $l-- unless ($l % 2);
  my $p;
  if ($start == 0)
  {
    $p = 0;
  }
  elsif ($start > 0)
  {
    $p = 2*($start-1);
  }
  else
  {
    $p = $l + 1 + 2*$start;
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
    if ($list[$i] ne 'pg' && $list[$i] ne 'nt')
    {
      $cref->{realtags}++;
    }
  }
}


sub check_warn
{
  my ($file, $line, $reason, $counts_ref, $file_counts_ref) = @_;

  print "File '$file'\n";
  print "Line '$line'\n";
  print "Reason: $reason\n";
  for my $k (sort keys %$counts_ref)
  {
    printf "%-15s %-15s\n", $k, $counts_ref->{$k};
  }
  print "\n";
  for my $k (sort keys %$file_counts_ref)
  {
    printf "%-15s %-15s\n", $k, $file_counts_ref->{$k};
  }
  print "---\n";
}


sub check_refLIN
{
  my ($file, $src_file, $line, $counts_ref) = @_;

  my %file_counts;
  countLIN($src_file, $counts_ref->{lno}, 
    $counts_ref->{lno} + $counts_ref->{repeat_lines} - 1, \%file_counts);

  if (defined $counts_ref->{tag_ref} &&
      defined $tag_global_count{$counts_ref->{tag_ref}})
  {
    return if defined $tag_no_global_count{$counts_ref->{tag_ref}};

    if (($counts_ref->{tag_ref} eq 'ERR_LIN_SYNTAX' || 
         $counts_ref->{tag_ref} eq 'ERR_LIN_HAND_DUPLICATED') &&
        $file_counts{qxs} == 0 && 
        $file_counts{bds} == 0 && 
        $counts_ref->{qxs} == 1 && 
        $counts_ref->{bds} == 1)
    {
      # OK
      return;
    }
          
    check_warn($file, $line, "Bad count", $counts_ref, \%file_counts)
      unless (($counts_ref->{tags} == 0 ||
            ($counts_ref->{tags} == 1 && $counts_ref->{action} =~ /LIN/)) && 
          $counts_ref->{qxs} == $file_counts{qxs} && 
          $counts_ref->{bds} == $file_counts{bds});

    return;
  }

  if ($counts_ref->{action} eq "insert" || 
      $counts_ref->{tag_ref} eq 'ERR_LIN_TRICK_DELETE')
  {
    my $fline = getline($src_file, $counts_ref->{lno});

    check_warn($file, $line, "Bad repeat", $counts_ref, \%file_counts)
      unless ($counts_ref->{tags} == $file_counts{realtags} ||
        ($counts_ref->{tag_ref} eq 'ERR_LIN_TRICK_DELETE' && 
         $counts_ref->{tags} < 3*$file_counts{realtags}));
    check_warn($file, $line, "Not (a,1,1)", $counts_ref, \%file_counts)
      unless ($counts_ref->{qxs} == 1 && $counts_ref->{bds} == 1);
    return;
  }

  if ($counts_ref->{action} !~ /LIN/)
  {
    if (defined $tag_global_count{$counts_ref->{tag_ref}})
    {
      check_whole_file_count($file, $line, 1, 1, $counts_ref, \%file_counts);
    }
    else
    {
      my %whole_counts;
      countLIN($src_file, 0, 999999, \%whole_counts);
      check_whole_file_count($file, $line, 1, 1, $counts_ref, \%whole_counts);
    }
    return;
  }

  # Check replaceLIN, insertLIN, deleteLIN.
  my $assoc = assoc_ok($counts_ref->{tag_source}, $counts_ref->{tag_ref});

  if ($assoc == $ASSOC_TAG_UNKNOWN)
  {
    return if ($counts_ref->{tag_ref} eq "ERR_LIN_SYNTAX"); # Accept any tag
    return if ($counts_ref->{tag_source} eq "" && 
        $counts_ref->{tag_ref} eq "ERR_LIN_PC_SYNTAX"); 
    # Accept empty tag in ERR_LIN_PC_SYNTAX

    $assoc = assoc_ok(lc($counts_ref->{tag_source}), $counts_ref->{tag_ref});
    return if ($assoc == $ASSOC_OK_GLOBAL || $assoc == $ASSOC_OK_LOCAL);

    check_warn($file, $line, "Bad tag", $counts_ref, \%file_counts);
    return;
  }
  elsif ($assoc == $ASSOC_ASSOC_UNKNOWN)
  {
    return if ($counts_ref->{tag_ref} eq "ERR_LIN_SYNTAX"); # Accept any tag

    check_warn($file, $line, "Bad association", $counts_ref, \%file_counts);
    return;
  }
  elsif ($assoc == $ASSOC_OK_GLOBAL)
  {
    if ($counts_ref->{tags} != $counts_ref->{repeat_tags} && 
        ! defined $tag_no_global_count{$counts_ref->{tag_ref}})
    {
      check_warn($file, $line, "Bad repeat", $counts_ref, \%file_counts);
      return;
    }

    my %whole_counts;
    countLIN($src_file, 0, 999999, \%whole_counts);
    check_whole_file_count($file, $line, 0, 0, $counts_ref, \%whole_counts);
  }
  elsif ($assoc == $ASSOC_OK_LOCAL)
  {
    if ($counts_ref->{repeat_tags} == 1)
    {
      if ($counts_ref->{tags} != 1)
      {
        my $numpipes = ($line =~ tr/\|//);
        if (($numpipes % 2 == 0) &&
            $counts_ref->{tags} == (1 + $numpipes/2))
        {
          return;
        }
        check_warn($file, $line, "Bad repeat", $counts_ref, \%file_counts);
        return;
      }
    }
    elsif ($counts_ref->{tags} != $counts_ref->{repeat_tags})
    {
      if ($counts_ref->{tag_ref} eq "ERR_LIN_RS_DELETE")
      {
        if ($counts_ref->{tags} == 1 && 
            $counts_ref->{qxs} == $counts_ref->{repeat_tags} && 
            $counts_ref->{bds} == $counts_ref->{repeat_tags}/2)
        {
          return;
        }

        check_warn($file, $line, "Accept repeat", $counts_ref, \%file_counts);
        return;
      }

      if ($counts_ref->{tags} != $file_counts{realtags})
      {
        my $fline = getline($src_file, $counts_ref->{lno});
        my %line_counts;
        incr_tag_count($fline, $counts_ref->{tag_start},
          $counts_ref->{repeat_tags}, \%line_counts);

        if ($counts_ref->{tags} != $line_counts{realtags})
        {
          check_warn($file, $line, "Bad repeat", $counts_ref, \%line_counts);
        }
        return;
      }
    }
          
    if ($counts_ref->{qxs} != 1 || 
        $counts_ref->{bds} != 1)
    {
      check_warn($file, $line, "Bad count", $counts_ref, \%file_counts);
      return;
    }
  }
  else
  {
    die "Bad assoc return $assoc";
  }
}


sub check_refPBN
{
  my ($file, $src_file, $line, $counts_ref) = @_;

  my %file_counts;
  countPBN($src_file, $counts_ref->{lno}, 
    $counts_ref->{lno} + $counts_ref->{repeat_lines} - 1, \%file_counts);

  if ($counts_ref->{repeat_lines} != $file_counts{lines})
  {
    check_warn($file, $line, "Line count", $counts_ref, \%file_counts);
    return;
  }

  if ($counts_ref->{qxs} == 1 &&
      $counts_ref->{bds} == 1 &&
      $file_counts{qxs} == 0 &&
      $file_counts{bds} == 0)
  {
    # OK
  }
  elsif ($counts_ref->{qxs} != $file_counts{qxs})
  {
    check_warn($file, $line, "qx count", $counts_ref, \%file_counts);
    return;
  }
  elsif ($counts_ref->{bds} != $file_counts{bds})
  {
    check_warn($file, $line, "bd count", $counts_ref, \%file_counts);
    return;
  }
  elsif ($counts_ref->{repeat_lines} != $file_counts{lines})
  {
    check_warn($file, $line, "lines count", $counts_ref, \%file_counts);
    return;
  }
}


sub check_refRBN
{
  my ($file, $src_file, $line, $counts_ref) = @_;

  my %file_counts;
  countRBN($src_file, $counts_ref->{lno}, 
    $counts_ref->{lno} + $counts_ref->{repeat_lines} - 1, \%file_counts);

  if ($counts_ref->{repeat_lines} != $file_counts{lines})
  {
    check_warn($file, $line, "Line count", $counts_ref, \%file_counts);
    return;
  }

  if ($counts_ref->{qxs} == 1 &&
      $counts_ref->{bds} == 1 &&
      $file_counts{qxs} == 0 &&
      $file_counts{bds} == 0)
  {
    # OK
  }
  elsif (defined $counts_ref->{tag_ref} &&
      defined $tag_globalRBN{$counts_ref->{tag_ref}})
  {
    my %whole_counts;
    countRBN($src_file, 0, 999999, \%whole_counts);

    if ($counts_ref->{qxs} != $whole_counts{qxs})
    {
      check_warn($file, $line, "qx count", $counts_ref, \%whole_counts);
      return;
    }
    elsif ($counts_ref->{bds} != $whole_counts{bds})
    {
      check_warn($file, $line, "bd count", $counts_ref, \%whole_counts);
      return;
    }
  }
  elsif ($counts_ref->{qxs} != $file_counts{qxs})
  {
    check_warn($file, $line, "qx count", $counts_ref, \%file_counts);
    return;
  }
  elsif ($counts_ref->{bds} != $file_counts{bds})
  {
    check_warn($file, $line, "bd count", $counts_ref, \%file_counts);
    return;
  }
  elsif ($counts_ref->{repeat_lines} != $file_counts{lines})
  {
    check_warn($file, $line, "bd count", $counts_ref, \%file_counts);
    return;
  }
}


sub check_whole_file_count
{
  my ($file, $line, $checkc1, $c1val, $counts_ref, $whole_counts_ref) = @_;
  if ($checkc1 && $counts_ref->{tags} != $c1val)
  {
    check_warn($file, $line, "Line count", $counts_ref, $whole_counts_ref);
  }

  if (! defined $whole_counts_ref->{qxs})
  {
    print "$file: $line\n";
  }

  if (($counts_ref->{qxs} != $whole_counts_ref->{qxs} || 
       $counts_ref->{bds} != $whole_counts_ref->{bds}) && 
       $counts_ref->{tag_ref} !~ /_VG_/ && 
       $counts_ref->{tag_ref} !~ /_PN_/)
  {
    check_warn($file, $line, "Board count", $counts_ref, $whole_counts_ref);
  }
}


sub write_file_numbers
{
  my ($fo, $input_list_ref, $ref_list_ref) = @_;

  for my $ext (@INPUTS)
  {
    next unless defined $input_list_ref->{$ext};
    printf $fo "%-28s %5s\n",
      "Number of $ext files", $#{$input_list_ref->{$ext}}+1;
  }

  printf $fo "%-28s %5s\n", 
    "Number of ref files", $#$ref_list_ref+1;
}


sub write_year_histo
{
  my ($fo, $href) = @_;
  print $fo "\nDirectories:\n";
  for my $i (0 .. $#$href)
  {
    next unless defined $href->[$i];
    printf $fo "%-28d %5d\n", $i, $href->[$i];
  }
}


