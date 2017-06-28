#!perl

use strict;
use warnings;

# Merges two lin numbers if possible.

if ($#ARGV != 1 && $#ARGV != 2)
{
  print "Usage: perl mergelin.pl no1 no2 [fix]\n";
  exit;
}

my $lin1 = $ARGV[0];
my $lin2 = $ARGV[1];
my $fix_flag = ($#ARGV == 2 ? 1 : 0);

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

my $file1 = no2file($lin1);
my $file2 = no2file($lin2);

my (@list1, @qlist1, $title1, $results1, $players1);
file2list($file1, \@list1, \@qlist1, \$title1, \$results1, \$players1);

my (@list2, @qlist2, $title2, $results2, $players2);
file2list($file2, \@list2, \@qlist2, \$title2, \$results2, \$players2);

my $order1 = list2order(\@qlist1);
my $order2 = list2order(\@qlist2);
if ($order1 == 3 || $order2 == 3)
{
  die "At least one order is general";
}

die "Titles differ:\n$title1\n$title2" if ($title1 ne $title2);
die "Players differ:\n$players1\n$players2" if ($players1 ne $players2);
die "Orders differ" if ($order1 ne $order2);

my $results;
merge_results($results1, $results2, \$results);

my $strout = "$title2\n$results\n$players2\n";

my $i1 = 0;
my $i2 = 0;
my $l1 = $#qlist1;
my $l2 = $#qlist2;

my (@reflines1, $nextpos1, $nextlno1);
read_ref($file1, \@reflines1, \$nextpos1, \$nextlno1);
if ($#reflines1 >= 0)
{
  die "Old file has more than one line in ref" if ($#reflines1 > 0);
  if ($reflines1[0] !~ /ERR_LIN_SUBSET/)
  {
    warn "$lin1: $reflines1[0] should be SUBSET";
  }
}

my (@reflines2, $nextpos2, $nextlno2);
read_ref($file2, \@reflines2, \$nextpos2, \$nextlno2);

my $changed_lno_flag = 0;
my $lno2_in = 4; # After qx, rs, pn: The next line to be read
my $lno2_out = 4; # The next line to be written

while (1)
{
  if ($i1 <= $l1 && $i2 <= $l2)
  {
    if ($qlist1[$i1] eq $qlist2[$i2])
    {
      # Compare the two.
      if (! same_lines(\@{$list1[$i1]}, \@{$list2[$i2]}))
      {
        if (! same_content(\@{$list1[$i1]}, \@{$list2[$i2]},
            $qlist1[$i1]))
        {
          warn "$qlist1[$i1], $i1, $i2: Not identical (take latter)";
          $strout .= print_qx(\@{$list2[$i2]}, 1);
          $i1++, $i2++;
        }
        else
        {
          $strout .= print_qx(\@{$list2[$i2]}, 1);
          $i1++, $i2++;
        }
      }
      else
      {
        $strout .= print_qx(\@{$list2[$i2]}, 1);
        $i1++, $i2++;
      }
    }
    elsif (lex_before($qlist1[$i1], $qlist2[$i2], $order2))
    {
      $strout .= print_qx(\@{$list1[$i1]}, 0);
      $i1++;
    }
    else
    {
      $strout .= print_qx(\@{$list2[$i2]}, 1);
      $i2++;
    }
  }
  elsif ($i1 <= $l1)
  {
    $strout .= print_qx(\@{$list1[$i1]}, 0);
    $i1++;
  }
  else # $i2 <= $l2
  {
    $strout .= print_qx(\@{$list2[$i2]}, 1);
    $i2++;
  }

  last if ($i1 > $l1 && $i2 > $l2);
}

if ($nextlno2 != -1)
{
  die "Didn't use up the ref2 file";
}

if ($#reflines1 == 0)
{
  if ($fix_flag)
  {
    if ($reflines1[0] !~ s/ERR[^(]+/ERR_LIN_MERGED/)
    {
      die "$lin1 ref: $reflines1[0] does not have expected format";
    }
    else
    {
      # TODO
      my $ref1 = $file1;
      $ref1 =~ s/lin$/ref/;
      print "Rewrite $ref1:\n$reflines1[0]\n\n";
    }
  }
  else
  {
    my $ref1 = $file1;
    $ref1 =~ s/lin$/ref/;
    print "Rewrite $ref1:\n$reflines1[0]\n\n";
  }
}
elsif ($fix_flag)
{
  my %count;
  countLIN($lin1, \%count);
  my $ll = "skip {ERR_LIN_MERGED(" .
    $count{lines} . "," . $count{qxs} . "," . $count{bds} .  ")}";

  if ($fix_flag)
  {
    # TODO
    my $ref1 = $file1;
    $ref1 =~ s/lin$/ref/;
    print "Create $ref1:\n$ll\n\n";
  }
  else
  {
    my $ref1 = $file1;
    $ref1 =~ s/lin$/ref/;
    print "Create $ref1:\n$ll\n\n";
  }
}

if ($changed_lno_flag)
{
  my $ref2 = $file2;
  $ref2 =~ s/lin$/ref/;

  if ($fix_flag)
  {
    # TODO
    print "Rewrite $ref2:\n";
    print "$_\n" for (@reflines2);
    print "\n";
  }
  else
  {
    print "Rewrite $ref2:\n";
    print "$_\n" for (@reflines2);
    print "\n";
  }
}

my $orig2 = $file2;
$orig2 =~ s/lin$/orig/;
if ($fix_flag)
{
  # TODO
  print "rename $file2 $orig2\n";
  print "Write $file2: string\n";
}
else
{
  print "rename $file2 $orig2\n";
  print "Write $file2: string\n";
}


sub no2file
{
  my $no = pop;
  return "" unless $no =~ /^\d+$/;
  return "$DIR/000000/$no.lin" if $no < 1000;
  my $t = int($no/1000);
  return "$DIR/00${t}000/$no.lin" if $no < 10000;
  return "$DIR/0${t}000/$no.lin";
}


sub file2list
{
  my ($file, $list_ref, $q_ref, 
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
      push @$q_ref, $1;
      $chunkno++;
      push @{$list_ref->[$chunkno]}, $line;
    }
    elsif ($chunkno >= 0)
    {
      push @{$list_ref->[$chunkno]}, $line;
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
  my ($file, $lines_ref, $nextpos_ref, $nextlno_ref) = @_;
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
      $$nextpos_ref = 0;
      $$nextlno_ref = -1;
    }
    else
    {
      my $pos = 0;
      $pos++ if ($lines_ref->[0] !~ /^\d+ /);
      if ($pos > $#$lines_ref)
      {
        $$nextpos_ref = 0;
        $$nextlno_ref = -1;
      }
      else
      {
        $$nextpos_ref = $pos+1;
        if ($lines_ref->[$pos] !~ /^(\d+) /)
        {
          die "Haven't learned $file, $pos yet: " . $lines_ref->[$pos];
        }
        $$nextlno_ref = $1;
      }
    }
  }
  else
  {
   $$nextpos_ref = 0;
   $$nextlno_ref = -1;
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


sub list2order
{
  my ($list_ref) = @_;

  my $l = $#$list_ref;
  return 0 if ($l == 0);

  my @misfits; # 0 OCOC, 1 COCO, 2 OOCC.
  my ($onew, $nnew, $oold, $nold);
  q2value($list_ref->[0], \$onew, \$nnew);

  for my $i (1 .. $l)
  {
    $oold = $onew;
    $nold = $nnew;
    q2value($list_ref->[$i], \$onew, \$nnew);

    if ($onew eq $oold)
    {
      if ($nnew <= $nold)
      {
        $misfits[0]++;
        $misfits[1]++;
        $misfits[2]++;
      }
    }
    elsif ($onew)
    {
      $misfits[2]++;
      if ($nnew < $nold)
      {
        $misfits[0]++;
        $misfits[1]++;
      }
      elsif ($nnew == $nold)
      {
        $misfits[0]++;
      }
    }
    else
    {
      if ($nnew < $nold)
      {
        $misfits[0]++;
        $misfits[1]++;
      }
      elsif ($nnew == $nold)
      {
        $misfits[1]++;
      }
    }
  }

  if ($misfits[0] && $misfits[1])
  {
    return ($misfits[2] ? 3 : 2);
  }
  elsif ($misfits[0] && $misfits[2])
  {
    return 1;
  }
  else
  {
    return 0;
  }
}


sub res2list
{
  my ($res, $list_ref) = @_;
  if ($res !~ /^rs\|([^|]+)\|$/)
  {
    die "Bad rs line: $res";
  }
  @$list_ref = split ',', $1;
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


sub lex_before
{
  my ($q1, $q2, $order) = @_;

  # q1 and q2 guaranteed to be different.
  my ($o1, $no1, $o2, $no2);
  q2value($q1, \$o1, \$no1);
  q2value($q2, \$o2, \$no2);

  if ($order == 0) # OCOC
  {
    if ($o1 eq $o2 || $o1 eq 'c')
    {
      return ($no1 < $no2);
    }
    else
    {
      return ($no1 <= $no2);
    }
  }
  elsif ($order == 1) # COCO
  {
    if ($o1 eq $o2 || $o1 eq 'o')
    {
      return ($no1 < $no2);
    }
    else
    {
      return ($no1 <= $no2);
    }
  }
  else # OOCC
  {
    if ($o1 eq $o2)
    {
      return ($no1 < $no2);
    }
    else
    {
      return ($o1 eq 'o');
    }
  }
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
        if ($lt1[$i1+1] ne $lt2[$i2+1] && $lt1[$i1] ne 'nt')
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


sub print_qx
{
  my ($list_ref, $two_flag) = @_;
  my $str = "";
  for my $line (@$list_ref)
  {
    $str .= "$line\n";
    if ($two_flag)
    {
      if ($lno2_in == $nextlno2 && $lno2_out != $lno2_in)
      {
        my $ll = $reflines2[$nextpos2-1];
        if ($ll !~ s/^$lno2_in/$lno2_out/)
        {
          die "Couldn't replace from $lno2_in to $lno2_out in $ll";
        }
        $reflines2[$nextpos2-1] = $ll;
        $changed_lno_flag = 1;

        if ($nextpos2 >= $#reflines2)
        {
          $nextlno2 = -1;
        }
        else
        {
          if ($reflines2[$nextpos2] !~ /^(\d+) /)
          {
            die "Ref line $ll bad format";
          }
          $nextpos2++;
          $nextlno2 = $1;
        }
      }
      $lno2_in++;
    }
    $lno2_out++;
  }
  return $str;
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
