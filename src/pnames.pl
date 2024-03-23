#!perl

use strict;
use warnings;
use v5.10;

use lib '.';

# Throw-away script that parses the "pw|" lines and attempts to
# get the player names.

if ($#ARGV < 0)
{
  print "Usage: perl rules.pl names.txt\n";
  exit;
}

my $file = $ARGV[0];
my ($linno, $rest);
my $simple = 0;

my @default = qw(South West North East South West North East);

open my $fr, '<', $file or die "Can't open $file $!";
while (my $line = <$fr>)
{
  chomp $line;
  $line =~ s///g;

  $line =~ /^(\d+)\s+(.*)/;
  ($linno, $rest) = ($1, $2);

  $rest =~ /^pw\|(.*)\|$/;
  my @players = split/,/, $1;

  # if ($linno == 109)
  # {
    # print "HERE\n";
  # }

  # Cut off trailing empty fields.
  while (@players)
  {
    last if $players[-1] ne '';
    pop @players;
  }

  # Cut off trailing fields that match the base modulo 8.
  my $len = $#players;
  while ($len >= 8)
  {
    if ($players[$len] eq $players[$len % 8] ||
        $players[$len] eq '' ||
        $players[$len] eq $default[$len % 8])
    {
      pop @players;
      $len--;
    }
    else
    {
      last;
    }
  }

  # Bit kludgy.
  $len = $#players;
  while ($len >= 4)
  {
    if ($players[$len] eq $players[$len % 4] ||
        $players[$len] eq '' ||
        $players[$len] eq $default[$len % 4])
    {
      pop @players;
      $len--;
    }
    else
    {
      last;
    }
  }

  if ($#players == 6)
  {
    push @players, "East";
  }

  if ($#players >= 8)
  {
    # Check whether there are more than eight distinct names.
    my %seen = ();
    foreach my $item (@players)
    {
      $seen{$item} = 1 unless $item eq '';
    }
    my @unique = keys %seen;

    if ($#unique >= 8)
    {
      @players = qw(Multiple Multiple Multiple Multiple
        Multiple Multiple Multiple Multiple);
    }
  }



  if ($#players == 3 || $#players == 7)
  {
    $simple++;
    # next;
  }

  for my $i (0 .. $#players)
  {
    $players[$i] =~ s/^\s+//;
    $players[$i] =~ s/\s+$//;
  }

  print $linno, " ", join(',', @players), "\n";
}

close $fr;

