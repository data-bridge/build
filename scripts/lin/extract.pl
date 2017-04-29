#!perl

use strict;
use warnings;

# Extracts fixes of sht files (from ref files) and 
# stores them in zfix.txt

if ($#ARGV < 0)
{
  print "Usage: perl extract.pl dir\n";
  exit;
}

# PC
my $DIR = "../../../bridgedata/hands/BBOVG";

# Laptop
# my $DIR = "../../../bridgedata/BBOVG";

my $HOMEDIR = glob("~/GitHub/Build/src");

my $indir = $ARGV[0];

my $ref = "referr.h";
my $out = "zfix.txt";
my @files;
@files = glob("$indir/*.sht");

my @codes;
read_error_codes("$HOMEDIR/$ref", \@codes);

open my $fo, '>', $out or die "Can't open $out $!";

foreach my $file (@files)
{
  open my $fr, '<', $file or die "Can't open $file: $!";

  my $outfile = `basename $file`;
  chomp $outfile;
  $outfile =~ s/\.sht$/.lin/;
  $outfile =~ s///g;
  my $dirint = "";
  if ($outfile =~ /(\d+)/)
  {
    my $dirno = int($1/1000);
    if ($dirno < 10)
    {
      $dirint = "00${dirno}000/";
    }
    else
    {
      $dirint = "0${dirno}000/";
    }
  }

  my @accum;
  while (my $line = <$fr>)
  {
    chomp $line;
    $line =~ s///g;

    next unless $line =~ /^Active ref lines: (\w+)$/;

    my $qx = $1;
    my $bno;
    if ($qx =~ /^([oc])(\d+)$/)
    {
      $bno = $2;
    }
    else
    {
      die "File $file: Bad line $line";
    }

    my %entry;
    my $expl;
    while (1)
    {
      last unless defined($line = <$fr>);
      last unless $line =~ /ERR_LIN/;

      if ($line =~ /^(.+)\s+\{(.*)\}\s*$/)
      {
        $entry{text} = $1;
        $expl = $2;
      }
      else
      {
        # Skip unexplained lines
        next;
      }

      my $lineno;
      if ($line =~ /^(\d+)/)
      {
        $lineno = $1;
      }
      else
      {
        die "Bad line number: $file, $line";
      }

      my @entries = split ';', $expl;
      for my $e (@entries)
      {
        if ($e =~ /^(.+)\((\d+),(\d+),(\d+)\)$/)
        {
          my $code = $1;
          $entry{expl}{$code}{tags} = $2;
          $entry{expl}{$code}{qxs} = $3;
          $entry{expl}{$code}{boards} = $4;
        }
        else
        {
          die "Bad line within {}: $file, $line, $e";
        }
      }

      if (! defined $accum[$lineno])
      {
        %{$accum[$lineno]} = %entry;
        push @{$accum[$lineno]{number}}, $bno;
      }
      else
      {
        if ($accum[$lineno]{text} ne $entry{text})
        {
          die "Texts differ";
        }

        my $found = 0;
        for my $i (0 .. $#{$accum[$lineno]{number}})
        {
          if ($accum[$lineno]{number}[$i] == $bno)
          {
            $found = 1;
            last;
          }
        }

        if (! $found)
        {
          push @{$accum[$lineno]{number}}, $bno;
        }

        for my $code (keys %{$entry{expl}})
        {
          $accum[$lineno]{expl}{$code}{tags} += $entry{expl}{$code}{tags};
          $accum[$lineno]{expl}{$code}{qxs} += $entry{expl}{$code}{qxs};
          $accum[$lineno]{expl}{$code}{boards} += $entry{expl}{$code}{boards}
            unless $found;
        }
      }
    }
  }
  close $fr;

  for my $l (0 .. $#accum)
  {
    next unless defined $accum[$l];

    my $outline = $accum[$l]{text} . " {";
    my @components;
    for my $code (@codes)
    {
      next unless defined $accum[$l]{expl}{$code};
      next if $code eq "ERR_SIZE";
next unless $code =~ /OVERLONG/;
      push @components, $code . "(" .
        $accum[$l]{expl}{$code}{tags} . "," .
        $accum[$l]{expl}{$code}{qxs} . "," .
        $accum[$l]{expl}{$code}{boards} . ")";
    }

    next if ($#components == -1);

    $outline .= join ',', @components;
    $outline .= "\}\n";
    
    print $fo "$DIR/$dirint$outfile\n";
    print $fo $outline . "\n";
  }
}

close $fo;


sub read_error_codes
{
  my ($fname, $codes_ref) = @_;

  open my $ff, '<', $fname or die "Can't open $fname: $!";

  my $seen = 0;
  while (my $line = <$ff>)
  {
    chomp $line;
    $line =~ s///g;
    next if ($line =~ /^\s*$/);

    if ($seen)
    {
      $line =~ s/,\s*$//;
      last if ($line =~ /^\}/);
      $line =~ s/\s//g;
      push @$codes_ref, $line;
    }
    elsif ($line =~ /^enum RefErrorsType/)
    {
      $line = <$ff>; # Skip leading {
      $seen = 1;
    }
  }

  close $ff;
}

