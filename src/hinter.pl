#!perl

use strict;
use warnings;

if ($#ARGV < 0)
{
  print "Usage: perl hinter.pl readerout.txt\n";
  exit;
}

my $FILENO = 99999;

my @SUITS = qw(S H D C);

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
push @ERROR_NAMES, "Players"; # Done
push @ERROR_NAMES, "Comment"; # Done


my @files;
get_files(\@files, $ARGV[0], 1);

my $numbers = `basename $ARGV[0]`;
$numbers =~ /(\d+)/;
$numbers = $1;

open my $fzfix, '>', "zfix$numbers.txt" or die "Can't open zfix.txt: $!";
open my $fzrest, '>', "zrest$numbers.txt" or die "Can't open zrest.txt: $!";

for my $eref (@files)
{
# print $eref->{fullname}, "\n";
  my (@lines, @count, $vg, $rs, $pn, @blist);
  $vg = "";
  $pn = "";
# if ($eref->{fullname} =~ /35113/)
# {
  # print "HERE\n";
# }
  slurp_file($eref->{fullname}, \@lines, \@count,
    \$vg, \$rs, \$pn, \@blist);

  if ($eref->{error} == $ERROR_UNKNOWN)
  {
    die "Unknown error should not arise: $eref->{fullname}";
  }
  elsif ($eref->{error} == $ERROR_CALL_EXTRA)
  {
    print $fzfix $eref->{fullname} . "\n";
    hint_call_extra($fzfix, $eref, \@blist, \@lines, \@count);
  }
  elsif ($eref->{error} == $ERROR_MD_NO_CARDS)
  {
    print $fzfix $eref->{fullname} . "\n";
    hint_md_no_cards($fzfix, $eref, \@blist, \@lines, \@count);
  }
  elsif ($eref->{error} == $ERROR_CARD_NOT_HELD)
  {
    if (cards_and_play_match($eref))
    {
      print_entry($fzrest, $eref);
      print $fzrest "Examine manually (no contradiction in cards)\n";
      print $fzrest "Play length " . (length $eref->{play}) . "\n";
    }
    else
    {
      print $fzfix $eref->{fullname} . "\n";
      hint_md_cards_not_held($fzfix, $eref, \@blist, \@lines, \@count);
    }
  }
  elsif ($eref->{error} == $ERROR_DECL_DENOM)
  {
    if (cards_and_play_match($eref))
    {
      print_entry($fzrest, $eref);
      print_summary($fzrest, $vg, $rs, $pn, \@blist);
    }
    else
    {
      print $fzfix $eref->{fullname} . "\n";
      hint_md_cards_not_held($fzfix, $eref, \@blist, \@lines, \@count);
    }
  }
  elsif ($eref->{error} == $ERROR_CONTRACT_SET)
  {
    print_entry($fzrest, $eref);
    print_summary($fzrest, $vg, $rs, $pn, \@blist);
  }
  elsif ($eref->{error} == $ERROR_NUM_TRICKS)
  {
    print_entry($fzrest, $eref);
    print_summary($fzrest, $vg, $rs, $pn, \@blist);
  }
  elsif ($eref->{error} == $ERROR_PLAYERS)
  {
    print $fzfix $eref->{fullname} . "\n";
    hint_bad_players($fzfix, $pn, \@lines, \@count);
  }
  elsif ($eref->{error} == $ERROR_COMMENT)
  {
    print $fzfix $eref->{fullname} . "\n";
    hint_bad_comment($fzfix, $eref, \@lines, \@count);
  }
  else
  {
    die "Error $eref->{error} should not arise";
  }
}
exit;


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
      @{$entry{holders}} = ();
    }
    elsif ($line =~ /^\s+LIN LIN-RP/)
    {
      # Last entry before results.
      if (%entry)
      {
        push @$fref, {%entry};
        $count[$entry{error}]++;
        last;
      }
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

      # Read in the deal.
      for my $i (1 .. 15)
      {
        $line = <$fh>;
        chomp $line;
        $line =~ s///g;
        push @{$entry{holders}}, $line;
      }
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

      # Read in the deal.
      for my $i (1 .. 15)
      {
        $line = <$fh>;
        chomp $line;
        $line =~ s///g;
        push @{$entry{holders}}, $line;
      }
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
    printf "%-16s %3d\n", $ERROR_NAMES[$i], $count[$i];
  }
  print "\n";

  close $fh;
}


sub print_entry
{
  my ($fzr, $eref) = @_;
  print $fzr "File    $eref->{fullname}\n";
  print $fzr "Error   $ERROR_NAMES[$eref->{error}]\n";
  print $fzr "Range   $eref->{start} .. $eref->{end}\n";
  print $fzr "Board   $eref->{board}\n";
  print $fzr "Auction $eref->{auction}\n";
  print $fzr "Play    $eref->{play}\n";
  print $fzr "Line no $eref->{lno}\n";
  print $fzr "\n";
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

      my ($lr, $lt, $lv);
      if ($line =~ /^(\w+)\s+(\w+)\s+\"(.*)\"$/ ||
          $line =~ /^(\w+)\s+(\w+)\s+(.*)$/)
      {
        $lr = $1;
        $lt = $2;
        $lv = $3;
      }
      elsif ($line =~ /^(\w+)\s+delete\s*$/)
      {
        $lr = $1;
        $lt = "delete";
        $lv = 1;
      }
      else
      {
        die "$refname syntax error: $line";
      }

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
        my $ix = get_index($lr, $countref);
	if ($ix == -1)
	{
          get_index($lr, $countref);
          die "Could not find index $ix: $line";
	}
        splice @$linesref, $ix, $lv;
        splice @$countref, $ix, $lv;
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

  # lno is a line number as seen from the outside world,
  # so 1 is the first line.
  # Returns the corresponding internal index in lref.

  my $start = ($lno >= $#$lref ? $#$lref : $lno);
  if ($lref->[$start] == $lno)
  {
    return $start;
  }
  elsif ($lref->[$start] > $lno)
  {
    my $s = $start;
    while ($s >= 0 && $lref->[$s] != $lno)
    {
      $s--;
    }
    return $s;
  }
  else
  {
    my $s = $start;
    while ($s <= $#$lref && $lref->[$s] != $lno)
    {
      $s++;
    }
    return $s;
  }
}


sub print_summary
{
  my ($fzr, $vg, $rs, $pn, $blistref) = @_;

  print $fzr "vg $vg\n";
  print $fzr "rs $rs\n";
  print $fzr "pn $pn\n";
  for my $i (0 .. $#$blistref)
  {
    printf $fzr "%-4d %4d %s\n", 
      $i, $blistref->[$i]{no}, $blistref->[$i]{name};
  }
  print $fzr "\n";
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



sub find_line_range_no_cards
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
    return;
  }

  my $b = $bno+1;
  while (1)
  {
    my $r0 = $listref->[$b]{no};
    my $r1 = ($b == $#$listref ? $countref->[$#$lineref] :
      $listref->[$b+1]{no}-1);

    my $i0 = get_index($r0, $countref);
    my $i1 = get_index($r1, $countref);

    if (check_md_has_cards($lineref, $i0, $i1))
    {
      $$l1ref = $r0-1;
      return;
    }
    elsif ($b == $#$listref)
    {
      $$l1ref = $r1;
      return;
    }
    else
    {
      $b++;
    }
  }
}


sub find_line_range_bad_cards
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
    return;
  }

  my $b = $bno+1;
  while (1)
  {
    my $r0 = $listref->[$b]{no};
    my $r1 = ($b == $#$listref ? $countref->[$#$lineref] :
      $listref->[$b+1]{no}-1);

    my %entry;
    my $i0 = get_index($r0, $countref);
    my $i1 = get_index($r1, $countref);
    my %holders;
    range_to_card_entry($lineref, $i0, $i1, \%entry, \%holders);

    if (check_holders_and_play(\%holders, $entry{play}))
    {
      $$l1ref = $r0-1;
      return;
    }
    elsif ($b == $#$listref)
    {
      $$l1ref = $r1;
      return;
    }
    else
    {
      $b++;
    }
  }
}


sub cards_player
{
  my ($line, $href, $player) = @_;

  $line =~ s/^\s+//;
  $line =~ s/\s+$//;

  my @a = split ' ', $line;
  for my $j (1 .. $#a)
  {
    my $card = ($a[$j] eq '10' ? 'T' : $a[$j]);
    $href->{$a[0] . $card} = $player;
  }
}


sub cards_and_play_match
{
  my ($eref) = @_;

  # Make a note of holder of each card in deal.
  # North 0, East 1, South 2, West 3

  my %holders;
  for my $i (1 .. 4)
  {
    # North
    my $line = $eref->{holders}[$i];
if (! defined $line)
{
  print "XX1\n";
}
    cards_player($line, \%holders, 0);
  }

  for my $i (6 .. 9)
  {
    # West and East
    my $line = $eref->{holders}[$i];
if (! defined $line)
{
  print "XX2\n";
}
    my $linewest = substr $line, 0, 24;
    my $lineeast = substr $line, 24;

    cards_player($lineeast, \%holders, 1);
    cards_player($linewest, \%holders, 3);
  }

  for my $i (11 .. 14)
  {
    # South
    my $line = $eref->{holders}[$i];
if (! defined $line)
{
  print "XX3\n";
}
    cards_player($line, \%holders, 2);
  }

  return check_holders_and_play(\%holders, $eref->{play});
}

sub check_md_has_cards
{
  my ($lineref, $i0, $i1) = @_;

  for my $i ($i0 .. $i1)
  {
    next if ($lineref->[$i] !~ /md\|([^|])*\|/);
    if (! defined $1 || $1 eq "")
    {
      return 0;
    }
    else
    {
      return 1;
    }
  }
}


sub check_holders_and_play
{
  my ($href, $play) = @_;

  # Check that each trick is possible.  (Don't check that winner
  # agrees with previous tricks etc).

  # Split play into tricks.
  my @tricks = split ':', $play;
  return 1 if ($#tricks == -1);

  for my $t (@tricks)
  {
    my $l = length $t;
    next if ($l == 2);
    my $lead = uc(substr $t, 0, 2);
    if (! defined $href->{$lead})
    {
      die "Unknown card $lead";
    }

    my $player = $href->{$lead};
    for my $i (1 .. ($l/2 - 1))
    {
      my $nextc = uc(substr $t, 2*$i, 2);
      if (! defined $href->{$lead})
      {
        die "Unknown card $lead";
      }
      my $nextp = $href->{$nextc};
      if (($nextp + 3 - $player) % 4 != 0)
      {
        return 0;
      }
      $player = $nextp;
    }
  }
  return 1;
}


sub set_holders
{
  my ($href, $p, $suit, $holding) = @_;

  my @a = split '', $holding;
  for my $c (@a)
  {
    $href->{$suit . $c} = $p;
  }
}

sub range_to_card_entry
{
  my ($lineref, $r0, $r1, $eref, $href) = @_;

  # Parse out the holding into eref->{holders}.

  if ($r0 > $#$lineref)
  {
    die "$r0 out of bounds";
  }

  my $line = $lineref->[$r0];
  if ($line !~ /md\|([^|]*)\|/)
  {
    die "Not an md line: $line";
  }
  my $deal = substr $1, 1; # Skip dealer

  for my $s (@SUITS)
  {
    for my $c (qw(2 3 4 5 6 7 8 9 T J Q K A))
    {
      $href->{$s . $c} = 1; # East
    }
  }

  my @a = split ',', $deal;
  if ($#a < 2 || $#a > 3)
  {
    die "Bad md line: $line";
  }
  for my $i (0 .. 2)
  {
    my $p = ($i + 2) % 4;
    my @b = split /[SHDC]/, $a[$i], -1;
    if ($#b != 4)
    {
      die "Bad md line $i: $line";
    }

    for my $j (0 .. 3)
    {
      set_holders($href, $p, $SUITS[$j], $b[$j+1]);
    }
  }

  # Parse out the plays.

  my $c = 0;
  $eref->{play} = "";
  for my $i ($r0+1 .. $r1)
  {
    my @a = split '\|', $lineref->[$i];
    for my $j (0 .. $#a-1)
    {
      if ($a[$j] eq 'pc')
      {
        $eref->{play} .= $a[$j+1];
	$c++;
	if ($c % 4 == 0)
	{
	  $eref->{play} .= ':';
	}
      }
    }
  }

  $eref->{play} =~ s/:$//;
}

sub hint_call_extra
{
  my ($fzf, $eref, $blistref, $linesref, $countref) = @_;

  my $pcount = count_trailing_passes($eref->{auction});
  if ($pcount <= 3)
  {
    die "Not enough trailing passes: $eref->{fullname}";
  }

  my $mno = find_last_bid_line($eref->{board}, $blistref, 
    $linesref, $countref);
  die "No bid line" if ($mno == -1);
  my $modif = modify_bid_line($linesref->[$mno], $pcount-3);

  print $fzf "$countref->[$mno] replace \"$modif\"\n";
  print $fzf "\n";
}


sub hint_md_no_cards
{
  my ($fzf, $eref, $blistref, $linesref, $countref) = @_;

  if ($#$blistref == 0)
  {
    print $fzf "skip\n";
  }
  else
  {
    my ($l0, $l1);
    find_line_range_no_cards(
      $eref->{board}, $blistref, $linesref, $countref, \$l0, \$l1);
    if ($l0 == $l1)
    {
      print $fzf "$l0 delete\n";
    }
    elsif ($l0 == $blistref->[0]{no} &&
        $l1 == $blistref->[$#$blistref]{no})
    {
      print $fzf "skip\n";
    }
    else
    {
      print $fzf "$l0 delete " . ($l1-$l0+1) . "\n";
    }
  }
  print $fzf "\n";
}


sub hint_md_cards_not_held
{
  my ($fzf, $eref, $blistref, $linesref, $countref) = @_;

  if ($#$blistref == 0)
  {
    print $fzf "skip\n";
  }
  else
  {
    my ($l0, $l1);
    find_line_range_bad_cards(
      $eref->{board}, $blistref, $linesref, $countref, \$l0, \$l1);
    if ($l0 == $l1)
    {
      print $fzf "$l0 delete\n";
    }
    elsif ($l0 == $blistref->[0]{no} &&
        $l1 == $blistref->[$#$blistref]{no})
    {
      print $fzf "skip\n";
    }
    else
    {
      print $fzf "$l0 delete " . ($l1-$l0+1) . "\n";
    }
  }
  print $fzf "\n";
}


sub hint_bad_players
{
  my ($fzf, $pn, $linesref, $countref) = @_;

  my $lno = get_index($pn, $countref);
  if ($lno == -1)
  {
    die "Cannot find $lno";
  }

  my $line = $linesref->[$lno];
  print $fzf "EDIT BY HAND\n";
  print $fzf "$pn replace \"" . $line . "\"\n";
  print $fzf "\n";
}


sub hint_bad_comment
{
  my ($fzf, $eref, $linesref, $countref) = @_;

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

  print $fzf $eref->{lno} . " replace \"" . $line . "\"\n";
  print $fzf "\n";
}

