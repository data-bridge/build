#!perl

use strict;
use warnings;

if ($#ARGV < 0)
{
  print "Usage: perl hinter.pl readerout.txt\n";
  exit;
}

my $FILENO = 99999;

my $ERROR_UNKNOWN = 0;
my $ERROR_CALL_EXTRA = 1;
my $ERROR_MD_NO_CARDS = 2;
my $ERROR_CARD_NOT_HELD = 3;
my $ERROR_DECL_DENOM = 4;
my $ERROR_CONTRACT_SET = 5;
my $ERROR_NUM_TRICKS = 6;
my $ERROR_PLAYERS = 7;
my $ERROR_COMMENT = 8;

my @ERROR_NAMES;
push @ERROR_NAMES, "Unknown";
push @ERROR_NAMES, "Extra call"; # Done
push @ERROR_NAMES, "No cards"; # Done
push @ERROR_NAMES, "Card not held";
push @ERROR_NAMES, "Decl/denom";
push @ERROR_NAMES, "Contract set";
push @ERROR_NAMES, "Number of tricks";
push @ERROR_NAMES, "Players";
push @ERROR_NAMES, "Comment"; # Done


my @files;
get_files(\@files, $ARGV[0], 1);

for my $eref (@files)
{
  my (@lines, @count, $vg, $rs, $pn, @blist);
  $vg = "";
  $pn = "";
  slurp_file($eref->{fullname}, \@lines, \@count,
    \$vg, \$rs, \$pn, \@blist);

  if ($eref->{error} == $ERROR_UNKNOWN)
  {
    die "Unknown error should not arise: $eref->{fullname}";
  }
  elsif ($eref->{error} == $ERROR_CALL_EXTRA)
  {
    print_entry($eref);
    hint_call_extra($eref, \@blist, \@lines, \@count);
  }
  elsif ($eref->{error} == $ERROR_MD_NO_CARDS)
  {
    print_entry($eref);
    hint_md_no_cards($eref, \@blist, \@lines, \@count);
  }
  elsif ($eref->{error} == $ERROR_CARD_NOT_HELD)
  {
    print_entry($eref);
    print_summary($vg, $rs, $pn, \@blist);
  }
  elsif ($eref->{error} == $ERROR_DECL_DENOM)
  {
    print_entry($eref);
    print_summary($vg, $rs, $pn, \@blist);
  }
  elsif ($eref->{error} == $ERROR_CONTRACT_SET)
  {
    print_entry($eref);
    print_summary($vg, $rs, $pn, \@blist);
  }
  elsif ($eref->{error} == $ERROR_NUM_TRICKS)
  {
    print_entry($eref);
    print_summary($vg, $rs, $pn, \@blist);
  }
  elsif ($eref->{error} == $ERROR_PLAYERS)
  {
    print_entry($eref);
    hint_bad_players($pn, \@lines, \@count);
  }
  elsif ($eref->{error} == $ERROR_COMMENT)
  {
    print_entry($eref);
    hint_bad_comment($eref, \@lines, \@count);
  }
  else
  {
    die "Error $eref->{error} should not arise";
  }
}
exit;


my $n = 0;
my (@lines, $vg0, $vgs, $vg1, @rs, $pn, @blist);
for my $entry (@files)
{
  undef @lines;
  undef @blist;
  slurp_file($entry->{fullname}, \@lines, 
    \$vg0, \$vgs, \$vg1, \@rs, \$pn, \@blist);
  print $entry->{fullname}, "\n";

  my $fixfile = $entry->{fullname};
  $fixfile =~ s/lin$/fix/;
  my $reffile = $entry->{fullname};
  $reffile =~ s/lin$/ref/;

  if (-e $fixfile)
  {
    system("cp $fixfile $FILENO.fix");
  }
  else
  {
    unlink "$FILENO.fix";
  }
  if (-e $reffile)
  {
    system("cp $reffile $FILENO.ref");
  }
  else
  {
    unlink "$FILENO.ref";
  }

  my $blast = $#blist;
  my $b = 0;
  my $vgmid = $vgs;
  while ($b <= $blast)
  {
    dump_subfile("$FILENO.lin", \@lines, 
      $vg0, $vgmid, $vg1, \@rs, $pn, $blist[$b]{no}, $vgmid-$vgs);

    system("./reader -i $FILENO.lin -r $FILENO.lin -c -v 30 > y.lin");

    my @revfiles;
    get_files(\@revfiles, "y.lin", 0);

    if ($#revfiles >= 0)
    {
      print "Failed entry $b: $blist[$b]{name}, $blist[$b]{no}";
    }
    else
    {
      print "Passed entry $b: $blist[$b]{name}, $blist[$b]{no}";
    }

    if ($b == $blast)
    {
      $b++;
    }
    else
    {
      my $n = substr $blist[$b]{name}, 1;
      my $m = substr $blist[$b+1]{name}, 1;
      $b += ($n eq $m ? 2 : 1);
    }
    $vgmid++;
  }

  $n++;
  print "\n";
}



sub get_files
{
  my ($fref, $fname) = @_;

  open my $fh, '<', $fname or die "Can't open $fname: $!";

  my %entry;
  my $lno = 0;
  my @count;

  while (my $line = <$fh>)
  {
    $lno++;
    chomp $line;
    $line =~ s///g;

    if ($line =~ /^Input file:\s+(.*)$/)
    {
      if (%entry)
      {
        push @$fref, {%entry};
        $count[$entry{error}]++;
        undef %entry;
      }

      $entry{fullname} = $1;
      $entry{error} = $ERROR_UNKNOWN;
      $entry{start} = '';
      $entry{end} = '';
      $entry{board} = '';
      $entry{auction} = '';
      $entry{play} = '';
      $entry{lno} = '';
    }
    elsif ($line =~ /^Line numbers: (\d+) to (\d+)/)
    {
      $entry{start} = $1;
      $entry{end} = $2;
    }
    elsif ($line =~ /^Line number:\s+(\d+)/)
    {
      $entry{start} = $1;
      $entry{end} = $entry{start};
    }
    elsif ($line eq "Call after auction is over")
    {
      print "ERROR1 $lno $line\n" unless defined $entry{start};
      $entry{error} = $ERROR_CALL_EXTRA;
    }
    elsif ($line eq "No LIN cards found (md)")
    {
      print "ERROR2 $lno $line\n" unless defined $entry{start};
      $entry{error} = $ERROR_MD_NO_CARDS;
    }
    elsif ($line =~ / not held /)
    {
      print "ERROR3 $lno $line\n" unless defined $entry{start};
      $entry{error} = $ERROR_CARD_NOT_HELD;
    }
    elsif ($line =~ /YY/)
    {
      print "ERROR4 $lno $line\n" unless defined $entry{start};
      $entry{error} = $ERROR_DECL_DENOM;
    }
    elsif ($line =~ /Declarer and denomination reset/)
    {
      # Could be a separate error, perhaps.
      print "ERROR4 $lno $line\n" unless defined $entry{start};
      $entry{error} = $ERROR_DECL_DENOM;
    }
    elsif ($line eq "Contract already set")
    {
      print "ERROR5 $lno $line\n" unless defined $entry{start};
      $entry{error} = $ERROR_CONTRACT_SET;
    }
    elsif ($line =~ /XX/)
    {
      print "ERROR6 $lno $line\n" unless defined $entry{start};
      $entry{error} = $ERROR_NUM_TRICKS;
    }
    elsif ($line =~ /Tricks already set/)
    {
      # Could be a separate error, perhaps.
      print "ERROR6 $lno $line\n" unless defined $entry{start};
      $entry{error} = $ERROR_NUM_TRICKS;
    }
    elsif ($line eq "Bad number of fields")
    {
      print "ERROR7 $lno $line\n" unless defined $entry{start};
      $entry{error} = $ERROR_PLAYERS;
    }
    elsif ($line =~ /Bad LIN line: .*, (\d+)$/)
    {
      print "ERROR7 $lno $line\n" unless defined $entry{start};
      $entry{error} = $ERROR_COMMENT;
      $entry{lno} = $1+1;
    }
    elsif ($line =~ /Board number \(18\), '(.*)'$/)
    {
      $entry{board} = $1;
    }
    elsif ($line =~ /Auction \(24\), '(.*)'$/ ||
           $line =~ /Auction \(24\), '(.*)$/)
    {
      $entry{auction} = $1;
    }
    elsif ($line =~ /Play \(27\), '(.*)'$/)
    {
      $entry{play} = $1;
    }
  }

  for my $i (0 .. $#count)
  {
    next unless defined ($count[$i]);
    printf "%-16s %3d\n", $ERROR_NAMES[$i], $count[$i]
  }
  print "\n";

  close $fh;
}


sub print_entry
{
  my $eref = pop;
  print "File    $eref->{fullname}\n";
  print "Error   $ERROR_NAMES[$eref->{error}]\n";
  print "Range   $eref->{start} .. $eref->{end}\n";
  print "Board   $eref->{board}\n";
  print "Auction $eref->{auction}\n";
  print "Play    $eref->{play}\n";
  print "Line no $eref->{lno}\n";
  print "\n";
}


sub slurp_file
{
  my ($fname, $linesref, $countref, $vgref, $rsref, $pnref, $blistref) = @_;

  # Read the unadulterated file.
  open my $fh, '<', $fname or die "Can't open $fname: $!";
  my $lno = 0;
  my $bno = 0;
  while (my $line = <$fh>)
  {
    $lno++;
    chomp $line;
    $line =~ s///g;
    push @$linesref, $line;
    push @$countref, $lno;
  }
  close $fh;

  # Apply the ref fixes.
  my $refname = $fname;
  $refname =~ s/\.lin$/.ref/;
  if (-e $refname)
  {
    open my $fr, '<', $refname or die "Can't open ref $fname: $!";
    while (my $line = <$fr>)
    {
      chomp $line;
      $line =~ s///g;
      next if ($line eq '');

      if ($line !~ /^(\w+)\s+(\w+)\s+\"(.*)\"$/ &&
          $line !~ /^(\w+)\s+(\w+)\s+(.*)$/)
      {
        die "$refname syntax error: $line";
      }

      my $lr = $1;
      my $lt = $2;
      my $lv = $3;

      if ($lt eq "replace")
      {
        if ($lr !~ /^\d+$/)
        {
          die "$refname bad line number: $line";
        }
        my $ix = get_index($lr, $countref);
        die "Could not find index $ix: $line" if ($ix == -1);
        $linesref->[$ix] = $lv;
      }
      elsif ($lt eq "insert")
      {
        if ($lr !~ /^\d+$/)
        {
          die "$refname bad line number: $line";
        }
        my $ix = get_index($lr, $countref);
        die "Could not find index $ix: $line" if ($ix == -1);
        splice @$linesref, $ix, 0, $lv;
        splice @$countref, $ix, 0, -1;
      }
      elsif ($lt eq "delete")
      {
        my $c;
        my $ll;
        if ($lr =~ /^\d+$/)
        {
          $c = 1;
          $ll = $lr;
        }
        elsif ($lr =~ /^(\d+)\-(\d+)$/)
        {
          $c = $2 - $1 + 1;
          $ll = $1;
        }
        else
        {
          die "$refname bad keyword: $line";
        }

        my $ix = get_index($ll, $countref);
        die "Could not find index $ix: $line" if ($ix == -1);
        splice @$linesref, $ix, $c;
        splice @$countref, $ix, $c;
      }
      else
      {
        die "$refname bad keyword: $line";
      }
    }
    close $fr;
  }

  # Parse out some relevant data.
  for my $i (0 .. $#$countref)
  {
    my $line = $linesref->[$i];

    if ($line =~ /^vg\|/)
    {
      $$vgref = $countref->[$i];
    }
    elsif ($line =~ /^rs\|/)
    {
      $$rsref = $countref->[$i];
    }
    elsif ($line =~ /^pn\|/)
    {
      $$pnref = $countref->[$i];
    }
    elsif ($line =~ /^qx\|([^|]+)\|/)
    {
      $blistref->[$bno]{name} = $1;
      $blistref->[$bno]{no} = $countref->[$i];
      $bno++;
    }
  }
}


sub get_index
{
  my ($lno, $lref) = @_;

  my $start = ($lno >= $#$lref ? $#$lref : $lno);
  if ($lref->[$start] == $start)
  {
    return $start;
  }
  elsif ($lref->[$start] > $start)
  {
    my $s = $start;
    while ($s >= 0 && $lref->[$s] != $start)
    {
      $s--;
    }
    return $s;
  }
  else
  {
    my $s = $start;
    while ($s <= $#$lref && $lref->[$s] != $start)
    {
      $s++;
    }
    return $s;
  }
}


sub print_summary
{
  my ($vg, $rs, $pn, $blistref) = @_;

  print "vg $vg\n";
  print "rs $rs\n";
  print "pn $pn\n";
  for my $i (0 .. $#$blistref)
  {
    printf "%-4d %4d %s\n", $i, $blistref->[$i]{no}, $blistref->[$i]{name};
  }
  print "\n";
}


sub btag_to_bno
{
  my ($btag, $listref) = @_;
  for my $i (0 .. $#$listref)
  {
    return $i if ($listref->[$i]{name} eq $btag);
  }
  return -1;
}


sub find_last_bid_line
{
  my ($boardtag, $listref, $lineref, $countref) = @_;

  my $bno = btag_to_bno($boardtag, $listref);
  if ($bno == -1)
  {
    die "Could not find $boardtag";
  }

  my $lno;
  if ($bno == $#$listref)
  {
    $lno = $#$lineref;
  }
  else
  {
    $lno = get_index($listref->[$bno+1]{no}-1, $countref);
    die "Can't find $bno+1" if ($lno == -1);
  }

  my $t = $listref->[$bno]{no};
  die "Can't find $bno" if ($t == -1);
  my $l0 = get_index($t, $countref);

  while ($lno >= $l0)
  {
    return $lno if ($lineref->[$lno] =~ /mb\|/);
    $lno--;
  }
  return -1;
}


sub count_trailing_passes
{
  my ($line) = @_;

  my $c = 0;
  if ($line =~ /\n/)
  {
    my @a = split "\n", $line;
    $line = $a[0];
  }

  while ($line =~ s/[Pp]$//)
  {
    $c++;
  }
  return $c;
}


sub modify_bid_line
{
  my ($line, $excess) = @_;

  my $e = $excess;
  while ($e > 0)
  {
    $line =~ s/(.*)mb\|[Pp]\|/$1/;
    $e--;
  }
  return $line;
}



sub find_line_range
{
  my ($boardtag, $listref, $lineref, $countref, $l0ref, $l1ref) = @_;

  my $bno = btag_to_bno($boardtag, $listref);
  if ($bno == -1)
  {
    die "Could not find $boardtag";
  }

  $$l0ref = $listref->[$bno]{no};
  if ($bno == $#$listref)
  {
    $$l1ref = $countref->[$#$lineref];
  }
  else
  {
    $$l1ref = $listref->[$bno+1]{no}-1;
  }
}


sub hint_call_extra
{
  my ($eref, $blistref, $linesref, $countref) = @_;

  my $pcount = count_trailing_passes($eref->{auction});
  if ($pcount <= 3)
  {
    die "Not enough trailing passes";
  }

  my $mno = find_last_bid_line($eref->{board}, $blistref, 
    $linesref, $countref);
  die "No bid line" if ($mno == -1);
  my $modif = modify_bid_line($linesref->[$mno], $pcount-3);

  print "$countref->[$mno] replace \"$modif\"\n";
  print "\n";
}


sub hint_md_no_cards
{
  my ($eref, $blistref, $linesref, $countref) = @_;

  if ($#$blistref == 0)
  {
    print "skip\n";
  }
  else
  {
    my ($l0, $l1);
    find_line_range($eref->{board}, $blistref, $linesref, $countref, 
      \$l0, \$l1);
    if ($l0 == $l1)
    {
      print "$l0 delete\n";
    }
    else
    {
      print "$l0 delete " . ($l1-$l0+1) . "\n";
    }
  }
  print "\n";
}


sub hint_bad_players
{
  my ($pn, $linesref, $countref) = @_;

  my $lno = get_index($pn, $countref);
  if ($lno == -1)
  {
    die "Cannot find $lno";
  }

  my $line = $linesref->[$lno];
  print "EDIT BY HAND\n";
  print "$pn replace \"" . $line . "\"\n";
  print "\n";
}


sub hint_bad_comment
{
  my ($eref, $linesref, $countref) = @_;

  my $lno = get_index($eref->{lno}, $countref);
  if ($lno == -1)
  {
    die "Cannot find $lno";
  }

  my $line = $linesref->[$lno];
  my @a = split /\|/, $line, -1;
  for (my $i = $#a; $i >= 0; $i--)
  {
    # Delete all comments.
    if ($a[$i] eq 'nt')
    {
      if ($i == $#a)
      {
        die "Unmatched trailing nt";
      }
      my $j = $i+1;
      while ($j <= $#a && $a[$j] !~ /^\w\w$/)
      {
        $j++;
      }
      if ($j > $#a)
      {
        splice @a, $i;
      }
      else
      {
        splice @a, $i, $j-$i;
      }
    }
  }

  $line = join '|', @a;

  print $eref->{lno} . " replace \"" . $line . "\"\n";
  print "\n";
}


sub dump_subfile
{
  my ($fname, $linesref, 
    $vg0, $vgmid, $vg1, $rsref, $pn, $fromline, $numskips) = @_;

  die "Not enough contracts" unless 2*$numskips <= $#$rsref;

  open my $fh, '>', $fname or die "Can't open $fname: $!";
  print $fh "$vg0,$vgmid,$vg1\n";

  my $rs = "rs|";
  for (my $i = 2*$numskips; $i <= $#$rsref; $i++)
  {
    $rs .= $rsref->[$i] . ",";
  }
  $rs =~ s/,$//;
  $rs .= "|";
  print $fh "$rs\n";

  print $fh "$pn\n";

  for (my $i = $fromline; $i <= $#$linesref; $i++)
  {
    print $fh "$linesref->[$i]\n";
  }

  close $fh;
}

