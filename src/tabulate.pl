#!perl

use strict;
use warnings;

# Throw-away script taking output of cards.pl, turning it into
# a tabular summary.

if ($#ARGV < 0)
{
  print "Usage: perl tabulate.pl file\n";
  exit;
}

my $file = $ARGV[0];

open my $fr, '<', $file or die "Can't open $file $!";
my @store;
while (my $line = <$fr>)
{
  chomp $line;
  $line =~ s///g;

  if ($line =~ /^Input/)
  {
    @store = ();
    push @store, $line;
  }
  elsif ($line =~ /^Made/ || $line =~ /^Down/)
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
    }

    printf "%6s %20s %6s %6s %6s %-40s\n",
      $data{lin},
      $data{hand},
      $data{hcp},
      $data{vul},
      $data{bid},
      $data{players},

    @store = ();
    push @store, $linline;
  }
  else
  {
    push @store, $line;
  }
}
close $fr;

