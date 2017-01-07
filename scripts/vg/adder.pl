#!perl

use strict;
use warnings;

# General function to add a line to a ref file.

sub addref
{
  my ($fname, $line) = @_;

  my ($cmd, $start, $end);
  parseline($line, \$cmd, \$start, \$end);
  my $delete = ($cmd eq 'delete' ? 1 : 0);

  my @llines;

  if (-e $fname)
  {
    open my $fh, '<', $fname or die "Can't open $fname: $!";
    while (my $lline = <$fh>)
    {
      chomp $lline;
      $lline =~ s///g;
      push @llines, $lline;
    }
    close $fh;
  }

  open my $fr, '>', $fname or die "Can't open $fname: $!";
  my $used = 0;
  for my $lline (@llines)
  {
    my ($lcmd, $lstart, $lend);
    parseline($lline, \$lcmd, \$lstart, \$lend);
    my $ldelete = ($lcmd eq 'delete' ? 1 : 0);

    if ($used)
    {
      print $fr "$lline\n";
    }
    elsif ($lend < $start)
    {
      print $fr "$lline\n";
    }
    elsif ($lstart > $end)
    {
      print $fr "$line\n";
      print $fr "$lline\n";
      $used = 1;
    }
    elsif ($lstart >= $start && $lend <= $end)
    {
      if ($delete)
      {
        print $fr "$line\n";
        $used = 1;
      }
      elsif ($start != $end)
      {
        print "line : $line\n";
        print "lline: $lline\n";
        die "Cannot reconcile";
      }
      elsif ($ldelete)
      {
        print $fr "$lline\n";
	$used = 1;
      }
      elsif ($cmd ne $lcmd)
      {
        print "line : $line\n";
        print "lline: $lline\n";
        die "Cannot reconcile";
      }
      else
      {
        print $fr "$line\n";
        $used = 1;
      }
    }
    elsif ($lstart <= $start && $lend >= $end)
    {
      if ($ldelete)
      {
        print $fr "$lline\n";
	$used = 1;
      }
      else
      {
        print "line : $line\n";
        print "lline: $lline\n";
        die "Cannot reconcile";
      }
    }
    else
    {
      print "line : $line\n";
      print "lline: $lline\n";
      die "Cannot reconcile";
    }
  }

  if (! $used)
  {
    print $fr "$line\n";
  }

  close $fr;
}


sub parseline
{
  my ($line, $cmdref, $startref, $endref) = @_;

  die "Bad line: $line" unless $line =~ /^(\d+)\s+(\w+)/;
  $$startref = $1;
  $$cmdref = $2;
  $$endref = $$startref;

  if ($$cmdref ne "insert" &&
      $$cmdref ne "replace" &&
      $$cmdref ne "delete")
  {
    die "Bad line: $line";
  }

  if ($$cmdref eq "delete" && $line =~ /(\d+)$/)
  {
    $$endref = $1 + $$startref - 1;
  }
}

1;
