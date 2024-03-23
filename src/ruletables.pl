#!perl

use strict;
use warnings;
use Scalar::Util 'looks_like_number';
use v5.10;

use lib '.';
use Distributions;

# Throw-away script that parses the output from reader with
# -Q 9=4=0=0 (say) and -v 63, particularly the lines for each rule.
# Keeps "Any" separate.
# Outputs:
# 1. Text suitable for putting in the .txt files.
# 2a. Hard error per dist and pos/vul, with marginal sums.
# 2b. Passing mass, ditto, with marginal sums.
# 2c. 2a/2b in percent, also for the marginal sums.

if ($#ARGV < 0)
{
  print "Usage: perl rules.pl file\n";
  exit;
}

set_distributions();

my @POS = qw(first second third fourth Any);
my @VUL = qw(None Both We They Any);

my $file = $ARGV[0];

my (%file_text, %ft_passes, %ft_passouts, %ft_hard_error);
my (%ft_possibles, %ft_marg_dist, %ft_marg_pv);
my (%hard_errors, %he_marg_dist, %he_marg_pv);
my (%pass_mass, %pm_marg_dist, %pm_marg_pv);
my (%perc, %perc_marg_dist, %perc_marg_pv);

open my $fr, '<', $file or die "Can't open $file $!";
while (my $line = <$fr>)
{
  chomp $line;
  $line =~ s///g;

  next if ($line =~ /^Rules/);

  my @a = split /\s+/, $line;
  my $hits =$a[1];
  my $passes = $a[2];
  my $prob = $a[3];
  my $hand_hits = $a[5];
  my $hand_passouts = $a[6];
  my $cum_mass = $a[7];
  my $cum_HE = $a[8];

  my @criterion = split /\./, $a[0];
  my $dist = $criterion[0];
  my $pos_name = $criterion[1];
  my $vul_name = $criterion[2];
  my $condition = $criterion[3];

  $condition =~ /(\d+)/;
  my $first_number = $1;

  $file_text{$dist}{$pos_name}{$vul_name}{$condition}[0] += $hits;
  $file_text{$dist}{$pos_name}{$vul_name}{$condition}[1] += $passes;
  $file_text{$dist}{$pos_name}{$vul_name}{$condition}[2] += $prob;
  $file_text{$dist}{$pos_name}{$vul_name}{$condition}[3] += $hand_hits;
  $file_text{$dist}{$pos_name}{$vul_name}{$condition}[4] += $hand_passouts;
  $file_text{$dist}{$pos_name}{$vul_name}{$condition}[5] += $cum_mass;

  # my $he = abs($hand_passouts - $cum_mass);
  my $he = $cum_HE;

  $ft_passes{$dist}{$pos_name}{$vul_name} += $passes;
  $ft_passouts{$dist}{$pos_name}{$vul_name} += 
    $hand_passouts;
  $ft_hard_error{$dist}{$pos_name}{$vul_name} += $he;


  $hard_errors{$dist}{$pos_name}{$vul_name} += $he;

  $he_marg_dist{$dist} += $he;

  $he_marg_pv{$pos_name}{$vul_name} += $he;


  $ft_possibles{$dist}{$pos_name}{$vul_name} += 
    $hand_hits;

  $ft_marg_dist{$dist} += $hand_hits;

  $ft_marg_pv{$pos_name}{$vul_name} += $hand_hits;


  $pass_mass{$dist}{$pos_name}{$vul_name} += $cum_mass;

  $pm_marg_dist{$dist} += $cum_mass;

  $pm_marg_pv{$pos_name}{$vul_name} += $cum_mass;


  # TODO This part is nonsense and doesn't work.
  # You can't add up percentages like that.

  my $rel = 0.;
  if ($cum_mass > 0.)
  {
    $rel = $he / $cum_mass;
  }

  $perc{$dist}{$pos_name}{$vul_name} += $rel;
  
  $perc_marg_dist{$dist} += $rel;

  $perc_marg_pv{$pos_name}{$vul_name} += $rel;
}

close $fr;

for my $dist (@DISTRIBUTIONS)
{
  next unless defined $file_text{$dist};
  for my $pos (@POS)
  {
    next unless defined $file_text{$dist}{$pos};
    for my $vul (@VUL)
    {
      next unless defined $file_text{$dist}{$pos}{$vul};
      print "$dist, $pos, $vul:\n\n";

      my @condition_map;
      my $n = 0;
      for my $c (keys %{$file_text{$dist}{$pos}{$vul}})
      {
        $condition_map[$n]{key} = $c;
        $c =~ /(\d+)/;
        $condition_map[$n]{val} = $1;
        $n++;
      }

      my @sorted = sort {$a->{val} <=> $b->{val}} @condition_map;
      my $allhands = 0;

      for my $cc (@sorted)
      {
        next unless defined $file_text{$dist}{$pos}{$vul}{$cc->{key}};

        printf("# %-20s%6d%6d%8.3f%6d%6d%8.3f\n",
          $cc->{key},
          $file_text{$dist}{$pos}{$vul}{$cc->{key}}[0],
          $file_text{$dist}{$pos}{$vul}{$cc->{key}}[1],
          ($file_text{$dist}{$pos}{$vul}{$cc->{key}}[0] ?
          $file_text{$dist}{$pos}{$vul}{$cc->{key}}[1] /
            $file_text{$dist}{$pos}{$vul}{$cc->{key}}[0] : 0.),
          $file_text{$dist}{$pos}{$vul}{$cc->{key}}[3],
          $file_text{$dist}{$pos}{$vul}{$cc->{key}}[4],
          $file_text{$dist}{$pos}{$vul}{$cc->{key}}[5]);
        $allhands += $file_text{$dist}{$pos}{$vul}{$cc->{key}}[3];
      }
      print "# \n";
      printf("# Passes: %d, passouts %d of %d, hard error %.3f\n\n",
        $ft_passes{$dist}{$pos}{$vul}, 
        $ft_passouts{$dist}{$pos}{$vul}, 
        $allhands,
        $ft_hard_error{$dist}{$pos}{$vul});
    }
  }
}


print_int_table(\%ft_possibles, \%ft_marg_dist, \%ft_marg_pv, 
  "Possibles");

print_table(\%hard_errors, \%he_marg_dist, \%he_marg_pv, 
  "Hard errors");

print_table(\%pass_mass, \%pm_marg_dist, \%pm_marg_pv, 
  "Passing mass derived from tables");

# print_table(\%perc, \%perc_marg_dist, \%perc_marg_pv, 
  # "Fraction hard error");


sub print_table
{
  my ($main_ref, $marg_dist_ref, $marg_pv_ref, $name) = @_;

  print("$name:\n\n");

  printf("%-12s%11s", "Dist.", "Sum");
  for my $pos (@POS)
  {
    next unless defined $pm_marg_pv{$pos};
    for my $vul (@VUL)
    {
      next unless defined $pm_marg_pv{$pos}{$vul};
      my $pn;
      if ($pos eq "first")
      {
        $pn = "P1";
      }
      elsif ($pos eq "second")
      {
        $pn = "P2";
      }
      elsif ($pos eq "third")
      {
        $pn = "P3";
      }
      elsif ($pos eq "fourth")
      {
        $pn = "P4";
      }
      else
      {
        $pn = "Any";
      }

      printf("%8s", $pn . "-" . $vul);
    }
  }
  print "\n";

  for my $dist (@DISTRIBUTIONS)
  {
    next unless defined $marg_dist_ref->{$dist};

  printf("%-12s%11.3f", $dist, $marg_dist_ref->{$dist});

    for my $pos (@POS)
    {
      for my $vul (@VUL)
      {
        if (defined $main_ref->{$dist}{$pos}{$vul})
        {
          printf("%8.3f", $main_ref->{$dist}{$pos}{$vul});
        }
        elsif (defined $marg_pv_ref->{$pos}{$vul})
        {
          printf("%8s", "-");
        }
      }
    }
    print "\n";
  }

  printf("%-12s%11s", "Sum", "");

  for my $pos (@POS)
  {
    next unless defined $marg_pv_ref->{$pos};
    for my $vul (@VUL)
    {
      next unless defined $marg_pv_ref->{$pos}{$vul};
      printf("%8.3f", $marg_pv_ref->{$pos}{$vul});
    }
  }
  print "\n\n";
}

sub print_int_table
{
  my ($main_ref, $marg_dist_ref, $marg_pv_ref, $name) = @_;

  print("$name:\n\n");

  printf("%-12s%11s", "Dist.", "Sum");
  for my $pos (@POS)
  {
    next unless defined $pm_marg_pv{$pos};
    for my $vul (@VUL)
    {
      next unless defined $pm_marg_pv{$pos}{$vul};
      my $pn;
      if ($pos eq "first")
      {
        $pn = "P1";
      }
      elsif ($pos eq "second")
      {
        $pn = "P2";
      }
      elsif ($pos eq "third")
      {
        $pn = "P3";
      }
      elsif ($pos eq "fourth")
      {
        $pn = "P4";
      }
      else
      {
        $pn = "Any";
      }

      printf("%8s", $pn . "-" . $vul);
    }
  }
  print "\n";

  for my $dist (@DISTRIBUTIONS)
  {
    next unless defined $marg_dist_ref->{$dist};

  printf("%-12s%11d", $dist, $marg_dist_ref->{$dist});

    for my $pos (@POS)
    {
      for my $vul (@VUL)
      {
        if (defined $main_ref->{$dist}{$pos}{$vul})
        {
          printf("%8d", $main_ref->{$dist}{$pos}{$vul});
        }
        elsif (defined $marg_pv_ref->{$pos}{$vul})
        {
          printf("%8s", "-");
        }
      }
    }
    print "\n";
  }

  printf("%-12s%11s", "Sum", "");

  for my $pos (@POS)
  {
    next unless defined $marg_pv_ref->{$pos};
    for my $vul (@VUL)
    {
      next unless defined $marg_pv_ref->{$pos}{$vul};
      printf("%8d", $marg_pv_ref->{$pos}{$vul});
    }
  }
  print "\n\n";
}

