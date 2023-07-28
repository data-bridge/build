#!perl

use strict;
use warnings;

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
    my $sum = $1;
    $all_ones{$distr}[$ppos]{$vul} += $sum;
  }
  else
  {
    push @store, $line;
  }
}
close $fr;


for my $dist (sort keys %summary)
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


for my $dist (sort keys %files)
{
  for my $tag (sort keys %{$files{$dist}})
  {
    my $file = "$DIR/$dist/T$tag";
    open my $fw, '>', $file or die "Can't open $file: $!";

    for my $l (@{$files{$dist}{$tag}})
    {
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

for my $dist (sort keys %all_ones)
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

  for my $dist (sort keys %all_ones)
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
