#!perl

use strict;
use warnings;

# Parses output from reader with -e flag (possible equals).
# Fixes ref files.

if ($#ARGV < 0)
{
  print "Usage: perl eqskip.pl file.txt\n";
  exit;
}

my $file = $ARGV[0];
die "No file '$file'" unless -e $file;

my @data;
my $no = 0;

open my $fh, '<', $file or die "Can't open '$file': $!";
while (my $line = <$fh>)
{
  chomp $line;
  $line =~ s///g;
  $no++;

  if ($line =~ /^([^\s]+)\.(...), ref /)
  {
    push @data, $1, $2;
  }
  elsif ($line =~ /^skip /)
  {
    if ($#data != 3)
    {
      warn "line $line, no $no: data length $#data\n";
      next;
    }

    my $reffile = $data[0] . ".ref";

    # Will overwrite reffile, in general with wrong counts.
    if (-e $reffile)
    {
      if ($data[1] eq "lin" || $data[1] eq "LIN")
      {
        my %count;
        countLIN($data[0] . "." . $data[1], \%count);
        # print "line was '$line'\n";
        my $n = "(" . $count{lines} . "," . 
          $count{qxs} . "," . 
          $count{bds} . ")";
        $line =~ s/\(.*\)/$n/;
        # print "line is '$line'\n";
      }
      else
      {
        warn "$reffile already exists";
        @data = ();
        next;
      }
    }

    @data = ();

    open my $fr, '>', $reffile or die "Can't open '$reffile': $!";
    print $fr "$line\n";
    close $fr;
    # print "file $reffile: line '$line'\n";
  }

}
close $fh;


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

