#!perl

use strict;
use warnings;

if ($#ARGV < 0)
{
  print "Usage: perl miss.pl readerout.txt\n";
  exit;
}

my @files;
get_files(\@files, $ARGV[0], 1);

my (@lines, $vg0, $vgs, $vg1, @rs, $pn, @blist);

my $n = 0;
for my $entry (@files)
{
  undef @lines;
  undef @blist;
  slurp_file($entry->{fullname}, \@lines, 
    \$vg0, \$vgs, \$vg1, \@rs, \$pn, \@blist);
  print $entry->{fullname}, "\n";

  my $blast = $#blist;
  my $b = 0;
  my $vgmid = $vgs;
  while ($b <= $blast)
  {
    dump_subfile("x.lin", \@lines, 
      $vg0, $vgmid, $vg1, \@rs, $pn, $blist[$b]{no}, $vgmid-$vgs);

    system("./reader -i x.lin -r x.lin -c -v 30 > y.lin");

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
  my ($fref, $fname, $printflag) = @_;

  open my $fh, '<', $fname or die "Can't open $fname: $!";

  my %entry;
  my $lno = 0;

  while (my $line = <$fh>)
  {
    $lno++;
    chomp $line;
    $line =~ s///g;

    if ($line =~ /^Input file:\s+(.*)$/)
    {
      $entry{fullname} = $1;
    }
    elsif ($line =~ /^Line numbers: (\d+) to (\d+)/)
    {
      $entry{start} = $1;
      $entry{end} = $2;
    }
    elsif ($line =~ / not held /)
    {
      if ($entry{start} > 5)
      {
        # Just flag the ones that are not complete losses.
        print_entry(\%entry) if $printflag;
      }
      else
      {
        push @$fref, {%entry};
        undef %entry;
      }
    }
  }
  print "\n";

  close $fh;
}


sub print_entry
{
  my $eref = pop;
  print "File  $eref->{fullname}, range $eref->{start} .. $eref->{end}\n";
}


sub slurp_file
{
  my ($fname, $linesref, $vg0, $vgs, $vg1, $rsref, $pn, $blistref) = @_;

  open my $fh, '<', $fname or die "Can't open $fname: $!";

  my $lno = -1;
  my $bno = 0;
  while (my $line = <$fh>)
  {
    $lno++;
    chomp $line;
    $line =~ s///g;
    push @$linesref, $line;

    if ($line =~ /^vg\|/)
    {
      die "vg must be line 1" unless $lno == 0;
      my @vglist = split ',', $line;
      die "Bad vg line" unless $#vglist == 8;
      $$vg0 = join ',', @vglist[0..2];
      $$vgs = $vglist[3];
      $$vg1 = join ',', @vglist[4..8];
    }
    elsif ($line =~ /^rs\|(.*)\|$/)
    {
      die "rs must be line 2" unless $lno == 1;
      @$rsref = split ',', $1;
    }
    elsif ($line =~ /^pn\|/)
    {
      die "pn must be line 3" unless $lno == 2;
      $$pn = $line;
    }
    elsif ($line =~ /^qx\|([^|]+)\|/)
    {
      $blistref->[$bno]{name} = $1;
      $blistref->[$bno]{no} = $lno;
      $bno++;
    }
  }
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

