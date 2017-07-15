#!perl

use strict;
use warnings;

# Takes an orig number and recovers the lin and ref files if possible.
# Reads from BBOVG/orig and writes, if possible, to BBOVG/untangle.

if ($#ARGV != 0 && $#ARGV != 1)
{
  print "Usage: perl mergelin.pl no [fix]\n";
  exit;
}

my $fix_flag = ($#ARGV == 1 ? 1 : 0);

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

die "Expected lin number as first argument" unless $ARGV[0] =~ /^\d+$/;
my $linno = $ARGV[0];

my $linfile = "$DIR/orig/$linno.lin";
die "No lin file $linfile" unless (-e $linfile);

my (@listL, @qlistL, %mdlistL, $titleL, $resultsL, $playersL);
file2list($linfile, \@listL, \@qlistL, \%mdlistL,
  \$titleL, \$resultsL, \$playersL);

my $origfile = "$DIR/orig/$linno.orig";
die "No orig file $origfile" unless (-e $origfile);

my (@listO, @qlistO, %mdlistO, $titleO, $resultsO, $playersO);
file2list($origfile, \@listO, \@qlistO, \%mdlistO,
  \$titleO, \$resultsO, \$playersO);

if ($titleL ne $titleO)
{
  print "$titleL\n";
  print "$titleO\n";
  die "Titles differ -- fix first";
}

my @resreflines;
if ($resultsL ne $resultsO)
{
  my (@reslistL, @reslistO);
  res2list($resultsL, \@reslistL);
  res2list($resultsO, \@reslistO);

  reslist2ref(\@reslistL, \@reslistO, \@resreflines);

  if ($#resreflines > 23)
  {
    print "$resultsL\n";
    print "$resultsO\n";
    die "Results differ -- fix first";
  }
}

if ($playersL ne $playersO)
{
  print "$playersL\n";
  print "$playersO\n";
  warn "Players differ -- fix first?";
}

my $reffile = "$DIR/orig/$linno.ref";
die "Can't yet do $reffile" if (-e $reffile);

# TODO
# For each ref line
#   if end-number is less than first qx pos0, then leave
#   else find qx range in lin, orig
#   correct range by the difference if diff != 0
# end
# later merge, sort by pos0
# if pos0 same
#   if above first qx pos0, die
#   sort on field number
# end

my (@reflines, @qxlines);
my $offset = 0;
my $nL = 0;
my $nO = 0;
my $file_cached = 0;
my $rno;

my (@listS, @qlistS, %mdlistS, $titleS, $resultsS, $playersS);
while ($nL <= $#qlistL)
{
  while ($nL <= $#qlistL && 
        ($nO > $#qlistO || $qlistL[$nL]{qx} ne $qlistO[$nO]{qx}))
  {
    my $md = $qlistL[$nL]{md};
    if (! $file_cached)
    {
      my @sources = glob("$DIR/orig/*.ref");
      for my $rfile (@sources)
      {
        die "Bad ref name" unless $rfile =~ /(\d+).ref$/;
        $rno = $1;
        next unless file_eligible($rfile, $rno, $linno);

        my $sfile = $rfile;
        $sfile =~ s/.ref$/.lin/;
        die "No lin source file $sfile" unless -e $sfile;

        @listS = ();
        @qlistS = ();
        %mdlistS = ();
        file2list($sfile, \@listS, \@qlistS, \%mdlistS,
          \$titleS, \$resultsS, \$playersS);

        if (defined $mdlistS{$md})
        {
          $file_cached = 1;
          last;
        }
      }
    }

    if (! defined $mdlistS{$md})
    {
      print "nL $nL, nO $nO\n";
      print "md $md\n";
      die "Could not find md";
    }

    my $qfound = 0;
    my $chunkno;
    for my $cno (@{$mdlistS{$md}})
    {
      if (same_lines($listL[$nL], $listS[$cno]))
      {
        $qfound = 1;
        $chunkno = $cno;
      }
    }

    if (! $qfound)
    {
      print "Couldn't find:\n";
      print "$qlistL[$nL]{qx}\n";
      die "Fatal error";
      # TODO: Locate the first difference
      # Look in list to find the line
      # Know the corresponding file line number in orig
      # Propose a fix
    }

    my $rline;
    if ($nO > $#qlistO)
    {
      $rline = $qlistO[$#qlistO]{lno1}+1;
    }
    else
    {
      $rline = $qlistO[$nO]{lno0};
    }
    $rline .= " insertLIN ";

    if (int($linno/1000) == int($rno/1000))
    {
      $rline .= "$rno.lin:";
    }
    else
    {
      $rline .= "../$rno.lin:";
    }

    if ($qlistS[$chunkno]{lno0} eq $qlistS[$chunkno]{lno1})
    {
      $rline .= "$qlistS[$chunkno]{lno0} ";
    }
    else
    {
      $rline .= "$qlistS[$chunkno]{lno0}-$qlistS[$chunkno]{lno1} ";
    }
    $rline .= "{ERR_LIN_HAND_INSERT(0,1,1)}";

    push @reflines, $rline;
    push @qxlines, $qlistL[$nL]{qx};

    $nL++;
  }

  $nL++;
  $nO++;
}

if ($nO <= $#qlistO)
{
  die "orig not used up";
}

if ($fix_flag)
{
  rename "$DIR/orig/$rno.lin", "$DIR/untangle/$rno.lin";

  rename "$DIR/orig/$rno.ref", "$DIR/untangle/$rno.ref";

  rename $origfile, "$DIR/untangle/$linno.lin";

  unlink $linfile;

  # TODO: Remove the old ref file

  my $newref = "$DIR/untangle/$linno.ref";
  open my $fn, '>', $newref or die "Can't write $newref: $!";
  print $fn "$_\n" for (@resreflines);
  print $fn "$_\n" for (@reflines);
  close $fn;
}
else
{
  print "$reffile:\n";
  print "$_\n" for (@resreflines);
  print "$_\n" for (@reflines);
  print "$_\n" for (@qxlines);
}


sub file2list
{
  my ($file, $list_ref, $q_ref, $md_ref,
      $title_ref, $results_ref, $players_ref) = @_;

  my $seenq = 0;
  my $chunkno = -1;
  my $lno = 0;

  open my $fr, '<', $file or die "Can't open $file $!";
  while (my $line = <$fr>)
  {
    $lno++;
    chomp $line;
    $line =~ s///g;
    next if $line eq "";

    if ($line =~ /^vg\|/)
    {
      if ($chunkno != -1)
      {
        warn "$file, $lno: vg is late";
      }
      $$title_ref = $line;
    }
    elsif ($line =~ /^rs\|/)
    {
      if ($chunkno != -1)
      {
        warn "$file, $lno: rs is late";
      }
      $$results_ref = $line;
    }
    elsif ($line =~ /^pn\|/)
    {
      if ($chunkno != -1)
      {
        warn "$file, $lno: pn is late";
      }
      $$players_ref = $line;
    }
    elsif ($line =~ /^qx\|([^,\|]+)/ || $line =~ /\|qx\|([^,\|]+)/)
    {
      my $qx = $1;
      $chunkno++;

      $q_ref->[$chunkno]{qx} = $qx;
      $q_ref->[$chunkno]{lno0} = $lno;
      $q_ref->[$chunkno]{lno1} = $lno;

      push @{$list_ref->[$chunkno]}, $line;

      if ($line =~ /^md\|\d([^\|]+)/ || $line =~ /\|md\|\d([^\|]+)/)
      {
        my $md = $1;
        push @{$md_ref->{$md}}, $chunkno;
        $q_ref->[$chunkno]{md} = $md;
      }
    }
    elsif ($chunkno >= 0)
    {
      $q_ref->[$chunkno]{lno1} = $lno;
      push @{$list_ref->[$chunkno]}, $line;

      if ($line =~ /^md\|\d([^\|]+)/ || $line =~ /\|md\|\d([^\|]+)/)
      {
        my $md = $1;
        push @{$md_ref->{$md}}, $chunkno;
        $q_ref->[$chunkno]{md} = $md;
      }
    }
    else
    {
      warn "$file, $lno: line '$line' unknown";
    }
  }
  close $fr;
}


sub parse_ref_line
{
  my ($line, $start_ref, $end_ref) = @_;

  if ($line =~ /^(\d+) /)
  {
    $$start_ref = $1;
    $$end_ref = $1;
  }
  elsif ($line =~ /^(\d+)-(\d+) /)
  {
    $$start_ref = $1;
    $$end_ref = $2;
  }
  else
  {
    die "Haven't learned ref line $line";
  }
}


sub read_ref
{
  my ($file, $lines_ref, $nextpos_ref, $nextlno_ref, $nextlno_ref_end) = @_;
  my $reffile = $file;
  die "$file should end on .lin" if ($reffile !~ /\.lin$/);
  $reffile =~ s/lin$/ref/;
  if (-e $reffile)
  {
    open my $fr, '<', $reffile or die "Can't open $reffile $!";
    while (my $line = <$fr>)
    {
      chomp $line;
      $line =~ s///g;
      push @$lines_ref, $line;
    }
    close $fr;

    if ($lines_ref->[0] =~ /skip/)
    {
      die "Extra lines in $reffile" unless $#$lines_ref == 0;
      $$nextpos_ref = 0;
      $$nextlno_ref = -1;
      $$nextlno_ref_end = -1;
    }

    my $pos = 0;
    $pos++ if ($lines_ref->[0] !~ /^\d+/); # Both 10 and 10-12

    while ($pos <= $#$lines_ref &&
        $lines_ref->[$pos] =~ /^(\d+) / &&
        $1 <= 3)
    {
      $pos++;
    }

    if ($pos > $#$lines_ref)
    {
      $$nextpos_ref = 0;
      $$nextlno_ref = -1;
      $$nextlno_ref_end = -1;
    }
    else
    {
      $$nextpos_ref = $pos+1;
      parse_ref_line($lines_ref->[$pos], $nextlno_ref, $nextlno_ref_end);
    }
  }
  else
  {
    $$nextpos_ref = 0;
    $$nextlno_ref = -1;
    $$nextlno_ref_end = -1;
  }
}


sub q2value
{
  my ($q, $open_ref, $no_ref) = @_;
  if (substr($q, 0, 1) eq 'o')
  {
    $$open_ref = 1;
  }
  elsif (substr($q, 0, 1) eq 'c')
  {
    $$open_ref = 0;
  }
  else
  {
    die "Bad qx: $q";
  }

  $$no_ref = substr($q, 1);
}


sub res2list
{
  my ($res, $list_ref) = @_;
  if ($res !~ /^rs\|([^|]+)\|$/)
  {
    die "Bad rs line: $res";
  }
  @$list_ref = split ',', $1, -1;
}


sub reslist2ref
{
  my ($rlistLref, $rlistOref, $resref) = @_;
  if ($#$rlistLref != $#$rlistOref)
  {
    die "Different lengths";
  }
  
  for my $i (0 .. $#$rlistLref)
  {
    if ($rlistLref->[$i] ne $rlistOref->[$i])
    {
      my $j = $i+1;
      push @$resref, 
        "2 replaceLIN \"1,$j,rs,$rlistOref->[$i],$rlistLref->[$i]\" {ERR_LIN_RS_REPLACE(1,1,1)}";
    }
  }
}


sub merge_results
{
  my ($res1, $res2, $res_ref) = @_;
  if ($res1 eq $res2)
  {
    $$res_ref = $res2;
    return;
  }

  my (@list1, @list2, @listc);
  res2list($res1, \@list1);
  res2list($res2, \@list2);
  my $l1 = $#list1;
  my $l2 = $#list2;
  my $l = ($l1 < $l2 ? $l1 : $l2);

  for my $i (0 .. $l)
  {
    if ($list1[$i] eq $list2[$i] || $list2[$i] eq '')
    {
      push @listc, $list1[$i];
    }
    elsif ($list1[$i] eq '')
    {
      push @listc, $list2[$i];
    }
    else
    {
      warn "rs conflict $i (take latter): $list1[$i] vs $list2[$i]";
      push @listc, $list2[$i];
    }
  }

  if ($l1 > $l2)
  {
    for my $i ($l+1 .. $l1)
    {
      push @listc, $list1[$i];
    }
  }
  elsif ($l2 > $l1)
  {
    for my $i ($l+1 .. $l2)
    {
      push @listc, $list2[$i];
    }
  }
  $$res_ref = "rs|" . join(',', @listc) . "|";
}


sub same_lines
{
  my ($list1_ref, $list2_ref) =@_;
  return 0 if ($#$list1_ref != $#$list2_ref);
  for my $i (0 .. $#$list1_ref)
  {
    return 0 if ($list1_ref->[$i] ne $list2_ref->[$i]);
  }
  return 1;
}


sub same_content
{
  my ($list1_ref, $list2_ref, $qx) =@_;
  my $str1 = "";
  $str1 .= $_ for (@$list1_ref);
  my $str2 = "";
  $str2 .= $_ for (@$list2_ref);

  my @lt1 = split /\|/, $str1;
  my @lt2 = split /\|/, $str2;
  my $i1 = 0;
  my $i2 = 0;
  my $l1 = $#lt1;
  my $l2 = $#lt2;
  if (($l1 % 2 == 0 && $lt1[$l1] ne 'pg') || 
      ($l2 % 2 == 0 && $lt2[$l2] ne 'pg'))
  {
    warn "qx $qx: Bad number of tokens";
    return 0;
  }

  while (1)
  {
    if ($i1 < $l1 && $i2 < $l2)
    {
      if ($lt1[$i1] eq $lt2[$i2])
      {
        if ($lt1[$i1+1] ne $lt2[$i2+1] && $lt1[$i1] ne 'nt' &&
            lc($lt1[$i1+1]) ne lc($lt2[$i2+1]))
        {
          warn "$qx, $i1, $lt1[$i1]: Bad values " . $lt1[$i1+1] . 
               " vs " . $lt2[$i2+1];
          return 0;
        }
        $i1 += 2;
        $i2 += 2;
      }
      elsif ($lt1[$i1] eq 'nt' || $lt1[$i1] eq 'pg')
      {
        $i1 += 2;
      }
      elsif ($lt2[$i2] eq 'nt' || $lt2[$i2] eq 'pg')
      {
        $i2 += 2;
      }
      else
      {
        die "$qx, $i1, bad tokens: $lt1[$i1] vs $lt2[$i2]";
      }
    }
    elsif ($i1 < $l1)
    {
      if ($lt1[$i1] ne 'nt' && $lt1[$i1] ne 'pg')
      {
        die "$qx, $i1, bad extra token: $lt1[$i1]";
      }
      $i1 += 2;
    }
    else # $i2 < $l2
    {
      if ($lt2[$i2] ne 'nt' && $lt2[$i2] ne 'pg')
      {
        die "$qx, $i2, bad extra token: $lt2[$i2]";
      }
      $i2 += 2;
    }

    last if ($i1 >= $l1 && $i2 >= $l2);
  }
  return 1;
}


sub countLIN
{
  my ($file, $cref) = @_;

  $cref->{lines} = 0;
  $cref->{qxs} = 0;
  $cref->{bds} = 0;

  my $prev = "";
  my $curr;
  my $lno = 0;

  my @seen;
  open my $fr, '<', $file or die "Can't open $file $!";
  while (my $line = <$fr>)
  {
    $lno++;
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
      $prev = $curr;
    }
  }
  close $fr;
}


sub file_eligible
{
  my ($rfile, $rno, $linno) = @_;
  return 0 if ($rno < $linno-100 || $rno > $linno+100);

  open my $fr, '<', $rfile or die "Can't read $rfile: $!";
  my $line = <$fr>;
  close $fr;

  return 0 unless $line =~ /^skip/;
  return 0 unless ($line =~ /LIN_MERGED/ || $line =~ /LIN_DUPLICATE/);
  return 1;
}
