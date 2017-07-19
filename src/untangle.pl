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
  warn "Titles differ -- fix first?";
}

my @reflines;
if ($resultsL ne $resultsO)
{
  my (@reslistL, @reslistO);
  res2list($resultsL, \@reslistL);
  res2list($resultsO, \@reslistO);

  reslist2ref(\@reslistL, \@reslistO, \@reflines);

  if ($#reflines > 23)
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

my @qxlines;
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

    if (! $file_cached)
    {
      die "Could not find source file for $qlistL[$nL]{qx}";
    }

    if (! defined $mdlistS{$md})
    {
      print "nL $nL, nO $nO\n";
      print "md $md\n";
      die "Could not find md in $rno";
    }

    my $qfound = 0;
    my $chunkno;
    find_in($qlistL[$nL]{qx}, \@qlistS, \@{$mdlistS{$md}},
      \$qfound, \$chunkno);

    if (! $qfound)
    {
      die "Couldn't find $qlistL[$nL]{qx} ($rno)\n";
    }
    elsif (! same_lines($listL[$nL], $listS[$chunkno]))
    {
      print "Different $qlistL[$nL]{qx}:\n";
      my $n1 = $#{$listL[$nL]};
      my $n2 = $#{$listS[$chunkno]};
      my $m = ($n1 < $n2 ? $n1 : $n2);
      for my $n (0 .. $m)
      {
        if ($listL[$nL][$n] ne $listS[$chunkno][$n])
        {
          print "lin $listL[$nL][$n]\n";
          print "src $listS[$chunkno][$n]\n";
          my $lno = $qlistO[$nO]{lno0} + $n;
          print "$lno replaceLIN ... {ERR_LIN_MC_REPLACE(1,1,1)}\n";
          # TODO: Propose a fix
          die "Fatal error";
        }
      }
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

if (! $file_cached)
{
  die "Didn't need a source file!";
}


my $srcreffile = "$DIR/orig/$rno.ref";
my @srcreflines;
if (-e $srcreffile)
{
  read_ref($srcreffile, \@srcreflines);
  die "Want exactly one source ref line" unless ($#srcreflines == 0);
  die "First source ref line must be skip" 
    unless $srcreflines[0] =~ /^skip/;
}
else
{
  die "No ref file for $rno";
}


my @existreflines;
if (-e $reffile)
{
  read_ref($reffile, \@existreflines);
  die "$reffile is a skip" if ($existreflines[0] =~ /skip/);

  fix_exist(\@existreflines, \@srcreflines,
    \@qlistL, \@qlistO, \%mdlistO, \@qlistS, \%mdlistS,
    $rno);
}



if ($fix_flag)
{
  # The source lin file just survives unchanged
  rename "$DIR/orig/$rno.lin", "$DIR/untangle/$rno.lin";

  my $oldsrcref = "$DIR/orig/$rno.ref";
  my $newsrcref = "$DIR/untangle/$rno.ref";
  if ($#srcreflines >= 1)
  {
    unlink $oldsrcref; 
    
    open my $fs, '>', $newsrcref or die "Can't write $newsrcref: $!";
    print $fs "$_\n" for (@srcreflines);
    close $fs;
  }
  else
  {
    rename $oldsrcref, $newsrcref;
  }

  # The orig lin file becomes the lin file again.
  rename $origfile, "$DIR/untangle/$linno.lin";

  # The .orig file goes away
  unlink $linfile;

  # merge reflines and existreflines
  push @reflines, @existreflines;
  my @final = sort reflex @reflines;

  my $newref = "$DIR/untangle/$linno.ref";
  open my $fn, '>', $newref or die "Can't write $newref: $!";
  print $fn "$_\n" for (@final);
  close $fn;
}
else
{
  push @reflines, @existreflines;
  my @final = sort reflex @reflines;

  print "$reffile:\n";
  print "$_\n" for (@final);
  print "$_ " for (@qxlines);
  print "\n";

  if ($#srcreflines >= 1)
  {
    print "\nSource ref file:\n";
    print "$_\n" for (@srcreflines);
  }
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


sub read_ref
{
  my ($reffile, $lines_ref) = @_;
  open my $fr, '<', $reffile or die "Can't open $reffile $!";
  while (my $line = <$fr>)
  {
    chomp $line;
    $line =~ s///g;
    push @$lines_ref, $line;
  }
  close $fr;
}


sub fix_exist
{
  my ($reflinesref, $srclinesref,
      $qlistLref, $qlistOref, $mdlistOref, $qlistSref, $mdlistSref,
      $rno) = @_;

  my @refcopy = @$reflinesref;
  @$reflinesref = ();

  for my $n (0 .. $#refcopy)
  {
    my $line = $refcopy[$n];

    my ($l0, $l1, $rest);
    if ($line =~ /^(\d+) (.*)$/)
    {
      $l0 = $1;
      $l1 = $1;
      $rest = $2;
    }
    elsif ($line =~ /^(\d+)-(\d+) (.*)$/)
    {
      $l0 = $1;
      $l1 = $2;
      $rest = $3;
    }
    else
    {
      die "Haven't learned ref line $line";
    }

    if ($l0 < $qlistLref->[0]{lno0})
    {
      push @$reflinesref, $line;
      next;
    }

    my $found = 0;
    for my $m (0 .. $#$qlistLref)
    {
      next unless ($l0 >= $qlistLref->[$m]{lno0});
      next unless ($l1 <= $qlistLref->[$m]{lno1});

      my $qx = $qlistLref->[$m]{qx};
      my $md = $qlistLref->[$m]{md};

      my $qfound = 0;
      my $chunkno;
      find_in($qx, $qlistOref, \@{$mdlistOref->{$md}},
        \$qfound, \$chunkno);


      if ($qfound)
      {
        $found = 1;
        next if ($qlistLref->[$n]{lno0} == $qlistOref->[$chunkno]{lno0});

        $line = shift_ref($l0, $l1, $rest,
          $qlistOref->[$chunkno]{lno0} - $qlistLref->[$m]{lno0});
        push @$reflinesref, $line;
        last;
      }

      find_in($qx, $qlistSref, \@{$mdlistSref->{$md}},
        \$qfound, \$chunkno);

      if (! $qfound)
      {
        print "The ref line for $qx is nowhere to be found ($rno)\n";
        die "Fatal error";
      }

      $found = 1;
      $line = shift_ref($l0, $l1, $rest,
        $qlistSref->[$chunkno]{lno0} - $qlistLref->[$m]{lno0});
      push @$srclinesref, $line;
      last;
    }

    if (! $found)
    {
      die "Couldn't find $l0";
    }
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
    die "Different lengths: $#$rlistLref vs $#$rlistOref";
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


sub find_in
{
  my ($qx, $qlistref, $mdlistref, $foundref, $cnoref) = @_;
  for my $cno (@$mdlistref)
  {
    if ($qx eq $qlistref->[$cno]{qx})
    {
      $$foundref = 1;
      $$cnoref = $cno;
      return;
    }
  }
}


sub shift_ref
{
  my ($l0, $l1, $rest, $delta) = @_;
  my $line;
  if ($l0 == $l1)
  {
    $line = $l0 + $delta;
  }
  else
  {
    $line = ($l0 + $delta) . "-" . ($l1 + $delta);
  }
  $line .= " $rest";
  return $line;
}


sub ref_parse
{
  my ($line, $lnoref, $fnoref) = @_;
  $$fnoref = 0;

  if ($line !~ /^(\d+)/)
  {
    $$lnoref = 0;
    return;
  }

  $$lnoref = $1;

  if ($line =~ /\"\d+,(\d+),/)
  {
    $$fnoref = $1;
  }
}


sub reflex
{
  my ($a1, $a2, $b1, $b2);
  ref_parse($a, \$a1, \$a2);
  ref_parse($b, \$b1, \$b2);

  return -1 if $a1 < $b1;
  return 1 if $a1 > $b1;
  return ($a2 <=> $b2);
}

