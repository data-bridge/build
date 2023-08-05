#!perl

use strict;
use warnings;
use Scalar::Util 'looks_like_number';

# Throw-away script that parses the output from reader with
# -Q 4-3-3-3 (say, but the distribution filter flag may be turned
# to false) and -v 63.

if ($#ARGV < 0)
{
  print "Usage: perl tabul.pl file\n";
  exit;
}

my @VUL_LIST = ( "None", "Both", "We", "They" );
my @RANGE_LIST = ( "weak", "mid", "str" );

my @DIST_ORDER;
set_dist_order(\@DIST_ORDER);

my $file = $ARGV[0];

my @store;
my %files;
my %summary;
my (%odd_ones, %all_ones);

my $DIR = "dist";

my ($distr, $ppos, $vul, $count);

open my $fr, '<', $file or die "Can't open $file $!";
while (my $line = <$fr>)
{
  chomp $line;
  $line =~ s///g;

  if ($line =~ /^Input/)
  {
    @store = ();
    push @store, $line;
  }
  elsif ($line =~ /^Made / || $line =~ /^Down / ||
      $line =~ /^Passed out/)
  {
    push @store, $line;

    my %data;
    my $linline = "";
    for my $l (@store)
    {
      if ($l =~ /(\d+).lin/)
      {
        $data{lin} = $1;
        $linline = $l;
      }
      elsif ($l =~ /Board: (.*)/)
      {
        $data{board} = $1;
      }
      elsif ($l =~ /Hand: (.*)/)
      {
        $data{hand} = $1;
      }
      elsif ($l =~ /HCP: (.*)/)
      {
        $data{hcp} = $1;
      }
      elsif ($l =~ /Vul: (.*)/)
      {
        $data{vul} = $1;
      }
      elsif ($l =~ /Bid: (.*)/)
      {
        $data{bid} = $1;
      }
      elsif ($l =~ /Players: (.*)/)
      {
        $data{players} = $1;
      }
      elsif ($l =~ /Tag: (.*)/)
      {
        my $tag = $1;
        $tag =~ /([^_]*)$/;
        my $dist = $1;
        $data{dist} = $dist;

        $tag =~ s/_$dist//;
        $data{tag} = $tag;

        $tag =~ /H(\d)/;
        my $hand = -1 + $1;

        my $range;
        if ($tag =~ /weak/)
        {
          $range = "weak";
        }
        elsif ($tag =~ /mid/)
        {
          $range = "mid";
        }
        elsif ($tag =~ /str/)
        {
          $range = "str";
        }
        else
        {
          die "No range: $tag";
        }

        my $v = $data{vul};
        $v = "Both" if ($v eq "both");
        $v = "None" if ($v eq "none");

        $odd_ones{$range}{$dist}[$hand]{$v}++;
      }
    }

    for my $l (@store)
    {
      push @{$files{$data{dist}}{$data{tag}}}, "$l\n";
    }

    push @{$files{$data{dist}}{$data{tag}}}, "\n";

    my $ldir;
    if ($data{lin} < 1000)
    {
      $ldir = "000000";
    }
    elsif ($data{lin} < 10000)
    {
      $ldir = "00" . int($data{lin} / 1000) . "000";
    }
    else
    {
      $ldir = "0" . int($data{lin} / 1000) . "000";
    }

    my $lin_complete = "../../../bbodata/hands/BBOVG/$ldir/$data{lin}.lin";
    my $s = make_auction_lines($lin_complete);
    push @{$files{$data{dist}}{$data{tag}}}, "$s\n";


    push @{$summary{$data{dist}}{$data{tag}}}, 
      sprintf("%6s %20s %6s %6s %6s %4s   %-40s\n",
        $data{lin},
        $data{hand},
        $data{hcp},
        $data{vul},
        $data{bid},
        "?",
        $data{players});

    @store = ();
    push @store, $linline;
  }
  elsif ($line =~ /^DISTRIBUTION (.*)/)
  {
    $distr = $1;
  }
  elsif ($line =~ /^Player pos\..*: (\d+)$/)
  {
    $ppos = $1;
  }
  elsif ($line =~ /^Vulnerability: (\d+)/)
  {
    my $v = $1;
    die unless $v >= 0 && $v <= 3;
    $vul = $VUL_LIST[$v];
  }
  elsif ($line =~ /^\s+SUM\s+(\d+)/)
  {
    # This will occur three times, but there is no harm in over-writing.
    my $sum = $1;
    $all_ones{$distr}[$ppos]{$vul} = $sum;
  }
  else
  {
    push @store, $line;
  }
}
close $fr;


for my $dist (@DIST_ORDER)
{
 if (! -e "$DIR/$dist")
 {
   mkdir "$DIR/$dist";
 }

  for my $tag (sort keys %{$summary{$dist}})
  {
    my $file = "$DIR/$dist/$tag";
    open my $fw, '>', $file or die "Can't open $file: $!";

    for my $l (@{$summary{$dist}{$tag}})
    {
      print $fw $l;
    }

    close $fw;
  }
}


for my $dist (@DIST_ORDER)
{
  for my $tag (sort keys %{$files{$dist}})
  {
    my $file = "$DIR/$dist/T$tag";
    open my $fw, '>', $file or die "Can't open $file: $!";

    for my $l (@{$files{$dist}{$tag}})
    {
      if (! defined $l)
      {
        print "ODD: $dist, $tag, $file\n";
      }
      print $fw $l;
    }

    close $fw;
  }
}


print "ALL\n\n";
printf("%-12s", "Dist");
for my $hand (0 .. 3)
{
  for my $vul (0 .. 3)
  {
    my $handno = $hand+1;
    printf("%8s", "H$handno $VUL_LIST[$vul]");
  }
}
print "\n";

for my $dist (@DIST_ORDER)
{
  printf("%12s", $dist);
  for my $hand (0 .. 3)
  {
    for my $vul (0 .. 3)
    {
      my $x = $all_ones{$dist}[$hand]{$VUL_LIST[$vul]};
      if (defined $x)
      {
        printf("%8d", $x);
      }
      else
      {
        printf("%8d", 0);
      }
    }
  }
  print "\n";
}
print "\n";

for my $range (0 .. 2)
{
  my $rangetab = $RANGE_LIST[$range];
  print $rangetab, "\n\n";

  printf("%12s", "Dist");
  for my $hand (0 .. 3)
  {
    for my $vul (0 .. 3)
    {
      my $handno = $hand+1;
      printf("%8s", "H$handno $VUL_LIST[$vul]");
    }
  }
  print "\n";

  for my $dist (@DIST_ORDER)
  {
    printf("%12s", $dist);
    for my $hand (0 .. 3)
    {
      for my $vul (0 .. 3)
      {
        my $x = $odd_ones{$rangetab}{$dist}[$hand]{$VUL_LIST[$vul]};
        if (defined $x)
        {
          printf("%8d", $x);
        }
        else
        {
          printf("%8d", 0);
        }
      }
    }
    print "\n";
  }
  print "\n";
}


sub make_auction_lines
{
  my $linfile = pop;

  my (@listL, @qlistL);
  file2list($linfile, \@listL, \@qlistL);
  my $result = "";

  for my $n (0 .. $#listL)
  {
    my @a;
    for my $line (@{$listL[$n]})
    {
      push @a, split('\|', $line);
    }

    my $num_mb = 0;
    my $num_non_pass = 0;
    my $bid = '';
    if ($#a == -1)
    {
      print $qlistL[$n]{qx}, "\n";
      my $n1 = $qlistL[$n]{lno0};
      my $n2 = $qlistL[$n]{lno1};
      if ($n1 == $n2)
      {
        $result .= "${n1} delete {ERR_LIN_HAND_AUCTION_NONE(0,1,1)}\n";
      }
      else
      {
        $result .= 
          "${n1}-${n2} delete {ERR_LIN_HAND_AUCTION_NONE(0,1,1)}\n";
      }
    }
    else
    {
      for my $i (0 .. $#a-1)
      {
        if ($a[$i] eq 'mb')
        {
          $num_mb++;
          if (lc($a[$i+1]) ne 'p' && lc($a[$i+1]) ne 'd')
          {
            $num_non_pass++;
            $bid = $a[$i+1];
          }
        }
      }
  
      if ($num_mb == 0)
      {
        $result .= $qlistL[$n]{qx} . "\n";
        my $n1 = $qlistL[$n]{lno0};
        my $n2 = $qlistL[$n]{lno1};
        if ($n1 == $n2)
        {
          $result .= "${n1} delete {ERR_LIN_HAND_AUCTION_NONE(0,1,1)}\n";
        }
        else
        {
          $result .= 
            "${n1}-${n2} delete {ERR_LIN_HAND_AUCTION_NONE(0,1,1)}\n";
        }
      }
      elsif ($num_non_pass == 1)
      {
        my $level = substr($bid, 0, 1);
        if (! looks_like_number($level))
        {
          next;
        }

        if ($level >= 1)
        {
          $result .= $qlistL[$n]{qx} . " candidate: $bid\n";
          my $n1 = $qlistL[$n]{lno0};
          my $n2 = $qlistL[$n]{lno1};
          if ($n1 == $n2)
          {
            $result .= "${n1} delete {ERR_LIN_HAND_AUCTION_ABBR(0,1,1)}\n";
          }
          else
          {
            $result .= 
              "${n1}-${n2} delete {ERR_LIN_HAND_AUCTION_ABBR(0,1,1)}\n";
          }
        }
      }
    }
  }
  return $result;
}


sub file2list
{
  my ($file, $list_ref, $q_ref) = @_;

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

    if ($line =~ /^qx\|([^,\|]+)/ || $line =~ /\|qx\|([^,\|]+)/)
    {
      my $qx = $1;
      $chunkno++;

      $q_ref->[$chunkno]{qx} = $qx;
      $q_ref->[$chunkno]{lno0} = $lno;
      $q_ref->[$chunkno]{lno1} = $lno;

      push @{$list_ref->[$chunkno]}, $line;
    }
    elsif ($line =~ /^pg\|/ || $line =~ /^nt\|/ ||
        $line =~ /^mp\|/)
    {
      # Just ignore
    }
    elsif ($chunkno >= 0)
    {
      $q_ref->[$chunkno]{lno1} = $lno;
      push @{$list_ref->[$chunkno]}, $line;
    }
  }
  close $fr;
}


sub set_dist_order
{
  @DIST_ORDER = (
  "4432",
  "5MAJ332", "5MIN332",
  "5MAJ431", "5MIN431",
  "5MAJ422", "5MIN422",
  "4333",
  "6MAJ322", "6MIN322",
  "6MAJ421", "6MIN421",
  "6MAJ331", "6MIN331",
  "55MAJ21", "5MAJ5MIN21", "55MIN21",
  "4441",
  "7MAJ321", "7MIN321",
  "6MAJ430", "6MIN430",
  "54MAJ40", "50MAJ44", "44MAJ50", "40MAJ54",
  "55MAJ30", "53MAJ50", "50MAJ53", "55MIN30",
  "65MAJ11", "61MAJ51", "51MAJ61", "65MIN11",
  "65MAJ20", "6MAJ5MIN20", "5MAJ6MIN20", "65MIN20",
  "7MAJ222", "7MIN222",
  "74MAJ11", "71MAJ41", "74MIN11", "71MIN41",
  "74MAJ20", "7MAJ4MIN20", "4MAJ7MIN20", "74MIN20",
  "7MAJ330", "7MIN330",
  "8MAJ221", "8MIN221",
  "8MAJ311", "8MIN311",
  "75MAJ10", "7MAJ5MIN10", "5MAJ7MIN10", "75MIN10",
  "8MAJ320", "8MIN320",
  "66MAJ10", "6MAJ6MIN10", "66MIN10",
  "84MAJ10", "8MAJ4MIN10", "4MAJ8MIN10", "84MIN10",
  "9MAJ211", "9MIN211",
  "9MAJ310", "9MIN310",
  "9MAJ220", "9MIN220",
  "76MAJ00", "7MAJ6MIN", "6MAJ7MIN", "76MIN00",
  "85MAJ00", "8MAJ5MIN00", "5MAJ8MIN00", "85MIN00",
  "10MAJ210", "10MIN210",
  "94MAJ00", "9MAJ4MIN00", "4MAJ9MIN00", "94MIN00",
  "10MAJ111", "10MIN111",
  "10MAJ300", "10MIN300",
  "11MAJ110", "11MIN110",
  "11MAJ200", "11MIN200",
  "12MAJ100", "12MIN100",
  "13MAJ000", "13MIN000" );
}
