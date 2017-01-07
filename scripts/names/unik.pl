#!perl

use strict;
use warnings;

my $prev = "";
while (my $line = <>)
{
  print $line if ($line ne $prev);
  $prev = $line;
}
