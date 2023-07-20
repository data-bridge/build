#!perl

use strict;
use warnings;

# Throw-away script that parses the output from reader with
# -Q 4-3-3-3 (say) and -v 63.

my $DIR = "4432";

if ($#ARGV < 0)
{
  print "Usage: perl tabul.pl file\n";
  exit;
}

my $file = $ARGV[0];

my @store;
my %files;
my %summary;

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
  elsif ($line =~ /^Made / || $line =~ /^Down /)
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
      elsif ($l =~ /Tag: (.*)/)
      {
        $data{tag} = $1;
      }
    }

    for my $l (@store)
    {
      push @{$files{$data{tag}}}, "$l\n";
    }

    push @{$summary{$data{tag}}}, 
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
  else
  {
    push @store, $line;
  }
}
close $fr;


for my $tag (sort keys %summary)
{
  my $file = "$DIR/$tag";
  open my $fw, '>', $file or die "Can't open $file: $!";

  for my $l (@{$summary{$tag}})
  {
    print $fw $l;
  }

  close $fw;
}


for my $tag (sort keys %files)
{
  my $file = "$DIR/T$tag";
  open my $fw, '>', $file or die "Can't open $file: $!";

  for my $l (@{$files{$tag}})
  {
    print $fw $l;
  }

  close $fw;
}
