#!perl

# Part of BridgeData.
#
# Copyright (c) 2016 by Soren Hein.
#
# See LICENSE and README.

package Name;
use strict;
use warnings;


use constant
{
  # value: Original value.

  # lower: All lower-case version of word
  TOLOWER => "Lower-case version",

  # type: Token type
  WORD => "Word",
  SPACE => "Space",
  COMMA => "Comma",
  DASH => "Dash",
  PERIOD => "Period",
  UNKNOWN => "Unknown",

  # cap: Capitalization
  UPPER => "Upper",
  LOWER => "Lower",
  WELLFORMED => "Well-formed",
  ILLFORMED => "Ill-formed",

  # letters: Content
  CONSONLY => "Consonants only",
  VOWELONLY => "Vowels only",
  MIXED => "Consonants and vowels",

  # meaning: Type of word
  FIRST => "First name",
  AMBIGUOUS => "Ambiguous name",
  LAST => "Last name",
  INITIAL => "Single letter",
  PARTICLE => "Particle",
  PRE_LOWER => "Lower-case prefix",
  PRE_UPPER => "Upper-case prefix",
  POST_LOWER => "Lower-case postfix",
  OTHER => "Other word",

  # length
};

my %token_type = (
  ' ' => SPACE,
  ',' => COMMA,
  '-' => DASH,
  '.' => PERIOD
);


sub new
{
  my $class = shift;
  my $self = { };
  bless $self, $class;
  $self->{num} = 0;
  $self->{addition} = "";
  @{$self->{list}} = ( );
  return $self;
}


sub reset
{
  my ($self) = @_;

  $self->{num} = 0;
  $self->{addition} = "";
  @{$self->{list}} = ( );
}


sub set_string
{
  my ($self, $line) = @_;
  $self->reset();

  my @list = split /( |,|\.|\-)/, $line;
  for my $token (@list)
  {
    next if ($token eq '');
    my $n = $self->{num}++;
    my $r = \%{$self->{list}[$n]};

    $r->{value} = $token;
    $r->{lower} = $token;
    $r->{cap} = UNKNOWN;
    $r->{letters} = UNKNOWN;
    $r->{meaning} = UNKNOWN;
    $r->{length} = length $token;

    if (defined $token_type{$token})
    {
      $r->{type} = $token_type{$token};
      next;
    }

    $r->{type} = WORD;

    $r->{lower} =~ tr/A-Z\x8e\xc0-\xcf\xd1-\xd6\xd8-\xdd/a-z\x9e\xe0-\xef\xf1-\xf6\xf8-\xfd/;

    $r->{cap} = _set_cap($token);

    $r->{letters} = _set_letters($token);

  }
}


sub _set_cap
{
  my $token = pop;

  my $has_u = $token =~ tr/[A-Z]//;
  my $has_l = $token =~ tr/[a-z]//;

  return UPPER if ($has_u && ! $has_l);
  return LOWER if (! $has_u && $has_l);
  if (! $has_l && ! $has_u)
  {
    $has_u = $token =~ tr/\x8a\x8c\x8e\x9f[\xc0-\xdd]//;
    $has_l = $token =~ tr/\x9a\x9c\x9e[\xdf-\xff]//;
    return UPPER if ($has_u && ! $has_l);
    return LOWER if (! $has_u && $has_l);
    return ILLFORMED if (! $has_l && ! $has_u);
  }

  # So now there are both types.
  my $first_letter = substr($token, 0, 1);
  my $rest = substr($token, 1);

  if ($first_letter =~ tr/[A-Z]//)
  {
    return WELLFORMED unless ($rest =~ tr/[A-Z]//);

    # De'Vand
    if ($token =~ /^Mc[A-Z](.*)/)
    {
      # McNie
      return ILLFORMED if ($1 =~ /[A-Z]/);
      return WELLFORMED;
    }
    elsif ($token =~ /^Le[A-Z](.*)/)
    {
      # LeCompte
      return ILLFORMED if ($1 =~ /[A-Z]/);
      return WELLFORMED;
    }
    elsif ($token =~ /^De'[A-Z](.*)/)
    {
      # De'Vand
      return ILLFORMED if ($1 =~ /[A-Z]/);
      return WELLFORMED;
    }
    else
    {
      return ILLFORMED;
    }
  }
  elsif ($token =~ /d'[A-Z](.*)/)
  {
    # d'Ambleteuse
    return ILLFORMED if ($1 =~ /[A-Z]/);
    return WELLFORMED;
  }
  else
  {
    return ILLFORMED;
  }
}


sub _set_letters
{
  my $token = pop;

  my $has_v = $token =~ tr/aeiouyAEIOUY//;
  my $has_c = $token =~ tr/bcdfghjklmnpqrstuvwxzBCDFGHJKLMNPQRSTVWXZ//;

  return VOWELONLY if ($has_v && ! $has_c);
  return CONSONLY if (! $has_v && $has_c);
  return ILLFORMED if (! $has_v && ! $has_c);
  return MIXED;
}


sub delete_from_dict
{
  my ($self, $dict, $start_flag) = @_;
  my $start = 0;
  if ($start_flag == 1)
  {
    # Skip until we get to a comma.
    $start++ while ($start < $self->{num} &&
        $self->{list}[$start]{type} ne COMMA);
  }
  elsif ($start_flag == 2)
  {
    # Skip until we get to a dash.
    $start++ while ($start < $self->{num} &&
        $self->{list}[$start]{type} ne DASH);
  }

  for my $i ($start .. $self->{num}-1)
  {
    my $r = \%{$self->{list}[$i]};
    if ($r->{type} eq WORD && 
        (defined $dict->{$r->{lower}} || $r->{lower} =~ /bri\xe7$/))
    {
      # Somehow I can't get the Turkish bric into the dictionary...

      $i-- while ($i > 0 && 
         ($self->{list}[$i-1]{type} eq SPACE ||
          $self->{list}[$i-1]{type} eq COMMA ||
          $self->{list}[$i-1]{type} eq DASH));

      $self->{num} = $i;
      # $dict->{$r->{lower}}++;
      return;
    }
  }
}


sub delete_titles
{
  my ($self, $dict1, $dict2) = @_;
  return if $self->{num} == 0;

  # Leading single titles
  if (defined $dict1->{$self->{list}[0]{lower}})
  {
    # Find next word.
    my $start = 1;
    $start++ while ($start < $self->{num} &&
        $self->{list}[$start]{type} ne WORD);

    if ($start >= $self->{num})
    {
      $self->{num} = 0;
      return;
    }
    splice @{$self->{list}}, 0, $start;
    $self->{num} -= $start;
  }

  # Trailing single titles.
  if (defined $dict1->{$self->{list}[$self->{num}-1]{lower}})
  {
    splice @{$self->{list}}, $self->{num}-2, 2;
    $self->{num} -= 2;
  }

  # Double titles
  for (my $i = $self->{num}-2; $i >= 0; $i--)
  {
    next unless defined $dict2->{$self->{list}[$i]{lower}};

    # Find next word;
    my $j = $i+1;
    $j++ while ($j < $self->{num} &&
      $self->{list}[$j]{type} ne WORD);
    return if ($j >= $self->{num});

    my $l = $#{$dict2->{$self->{list}[$i]{lower}}};
    for my $k (0 .. $l)
    {
      if ($self->{list}[$j]{lower} eq 
          $dict2->{$self->{list}[$i]{lower}}[$k])
      {
        # Go to the next word after
        $j++;
        $j++ while ($j < $self->{num} &&
          $self->{list}[$j]{type} ne WORD);

        if ($j == $self->{num} &&
            $i > 0 &&
            ($self->{list}[$i-1]{type} eq SPACE ||
             $self->{list}[$i-1]{type} eq COMMA))
        {
          $i--;
        }

        splice @{$self->{list}}, $i, $j-$i;
        $self->{num} -= $j-$i;
	last;
      }
    }
  }
}


sub fix_periods
{
  my $self = pop;

  for (my $i = $self->{num}-1; $i >= 2; $i--)
  {
    if ($self->{list}[$i]{type} eq SPACE &&
        $self->{list}[$i-1]{type} eq PERIOD &&
        $self->{list}[$i-2]{type} eq SPACE)
    {
      # ' . '
      splice @{$self->{list}}, $i-1, 2;
      $self->{num} -= 2;
    }
    elsif ($self->{list}[$i]{type} eq PERIOD &&
        $self->{list}[$i-1]{type} eq SPACE)
    {
      #  ' .'
      splice @{$self->{list}}, $i, 1;
      $self->{num}--;
    }
    elsif ($self->{list}[$i]{type} eq SPACE &&
        $self->{list}[$i-1]{type} eq PERIOD)
    {
      # ' ..
      splice @{$self->{list}}, $i, 1;
      $self->{num}--;
    }
    elsif ($self->{list}[$i]{type} eq WORD &&
        $self->{list}[$i-1]{type} eq PERIOD &&
        $self->{list}[$i-2]{type} eq SPACE)
    {
      # 'word ..
      splice @{$self->{list}}, $i-1, 1;
      $self->{num}--;
    }
  }

  return if $self->{num} <= 1;

  if ($self->{list}[$self->{num}-1]{type} eq PERIOD &&
      $self->{list}[$self->{num}-2]{type} eq SPACE)
  {
    $self->{num} -= 2;
  }
}


sub fix_dashes
{
  my ($self, $dict) = @_;

  # Trailing dash
  $self->{num}-- 
    while ($self->{num} > 0 && 
           $self->{list}[ $self->{num}-1 ]{type} eq DASH);

  my $end = $self->{num};
  for (my $i = $self->{num}-1; $i > 0; $i--)
  {
    next unless $self->{list}[$i]{type} eq DASH;
    if ($self->{list}[$i+1]{type} eq SPACE &&
        $self->{list}[$i-1]{type} eq SPACE)
    {
      # ' - '
      $end = $i-1;
    }
    elsif ($self->{list}[$i-1]{type} eq DASH)
    {
     # '--'
      $end = $i-1;
    }
    elsif ($self->{list}[$i-1]{type} eq PERIOD)
    {
      # E.-A. Krüger.  For consistency (alternating words)
      # we delete the period.
      splice @{$self->{list}}, $i-1, 1;
      $end--;
    }
    elsif ($self->{list}[$i+1]{type} eq SPACE ||
           $self->{list}[$i-1]{type} eq SPACE)
    {
      # '- ', ' -'
      my $prev = $i;
      $prev-- while ($prev >= 0 && $self->{list}[$prev]{type} ne WORD);
      if ($prev < 0)
      {
        # Shouldn't happen.
        $self->{num} = 0;
	return;
      }
      if (defined $dict->{$self->{list}[$prev]{lower}})
      {
        if ($self->{list}[$i+1]{type} eq SPACE)
	{
          splice @{$self->{list}}, $i+1, 1;
	  $end--;
	}
	else
	{
          splice @{$self->{list}}, $i-1, 1;
	  $end--;
	}
      }
      else
      {
        $end = $prev+1;
      }
    }
    elsif ($self->{list}[$i+1]{type} eq PERIOD)
    {
      $end = $i;
    }
    elsif ($self->{list}[$i+1]{type} eq WORD ||
           $self->{list}[$i-1]{type} eq WORD)
    {
      if ($self->{list}[$i-1]{length} > 1 &&
          $self->{list}[$i+1]{length} > 1 &&
          $self->{list}[$i-1]{cap} ne UPPER &&
          $self->{list}[$i+1]{cap} eq UPPER)
      {
        $end = $i;
      }
    }
  }

  $self->{num} = $end;
}


sub fix_additions
{
  my ($self, $dict) = @_;

  my $last_word = $self->{num}-1;
  $last_word-- 
    while ($last_word >= 0 && $self->{list}[$last_word]{type} ne WORD);
  return if $last_word <= 0;

  if (defined $dict->{$self->{list}[$last_word]{lower}})
  {
    if ($self->{list}[$last_word]{lower} =~ /ii/)
    {
      $self->{addition} = uc $self->{list}[$last_word]{lower};
    }
    else
    {
      $self->{addition} = $self->{list}[$last_word]{lower} . ".";
      $self->{addition} =~ s/^([a-z])/uc($1)/e;
    }

    $self->{num} = $last_word-1;
  }
}


sub check_tightness
{
  my $self = pop;
  my $ok = 1;
  for my $i (0 .. $self->{num}-1)
  {
    if ($i % 2 == 0)
    {
      if ($self->{list}[$i]{type} ne WORD)
      {
        $ok = 0;
	last;
      }
    }
    elsif ($self->{list}[$i]{type} eq WORD)
    {
      $ok = 0;
      last;
    }
  }

  if (! $ok)
  {
    print "XX ", $self->as_string(), "\n";
  }
}


sub fix_commas
{
  my ($self, $dict) = @_;

  my $num_commas = 0;
  my $first_comma = 0;
  for my $n (0 .. $self->{num}-1)
  {
    $num_commas++ if $self->{list}[$n]{type} eq COMMA;
    $first_comma++ if ($num_commas == 0);
  }

  return if $num_commas == 0;

  my $num_spaces = 0;
  for my $n (0 .. $first_comma-1)
  {
    $num_spaces++ 
      if ($self->{list}[$n]{type} eq SPACE ||
          $self->{list}[$n]{type} eq PERIOD);
  }

  if ($num_commas >= 3)
  {
    $self->{num} = ($num_spaces < 2 ? 0 : $first_comma);
    return;
  }
  elsif ($num_commas == 2)
  {
    $self->{num} = ($num_spaces == 0 ? 0 : $first_comma);
    return;
  }

  my $post_comma = $first_comma;
  $post_comma++ while ($post_comma < $self->{num} &&
    $self->{list}[$post_comma]{type} ne WORD);

  return if ($post_comma >= $self->{num}); # Should not happen

  my $pre_comma = $first_comma;
  $pre_comma-- while ($pre_comma >= 0 &&
    $self->{list}[$pre_comma]{type} ne WORD);

  return if ($pre_comma >= $self->{num});

  if (defined $dict->{$self->{list}[$pre_comma]{lower}})
  {
    $pre_comma--;
    $pre_comma-- while ($pre_comma >= 0 &&
      $self->{list}[$pre_comma]{type} ne WORD);
    $self->{num} = $pre_comma+1;
    return;
  }
  elsif (defined $dict->{$self->{list}[$post_comma]{lower}})
  {
    $self->{num} = $pre_comma+1;
    return;
  }

  my $post_all_lower = 1;
  for my $n ($post_comma .. $self->{num}-1)
  {
    if ($self->{list}[$n]{type} eq WORD &&
        $self->{list}[$n]{cap} ne LOWER)
    {
      $post_all_lower = 0;
      last;
    }
  }

  if ($pre_comma == 0)
  {
    if ($self->{list}[0]{cap} eq WELLFORMED && $post_all_lower)
    {
      $self->{num} = 1;
    }
    # Otherwise no change
  }
  elsif ($post_all_lower)
  {
    $self->{num} = 0;
  }
  else
  {
    # No change.
  }
}


sub fix_place_pairs
{
  my ($self, $dict) = @_;
  return if $self->{num} <= 1;

  # Double places
  for (my $i = $self->{num}-2; $i >= 0; $i--)
  {
    next unless defined $dict->{$self->{list}[$i]{lower}};

    # Find next word;
    my $j = $i+1;
    $j++ while ($j < $self->{num} &&
      $self->{list}[$j]{type} ne WORD);
    return if ($j >= $self->{num});

    if ($self->{list}[$j]{lower} eq $dict->{$self->{list}[$i]{lower}})
    {
      $i--
        if ($self->{list}[$i-1]{type} eq COMMA ||
            $self->{list}[$i-1]{type} eq SPACE);
      $self->{num} = $i;
      return;
    }
  }
}


sub fix_trailing_initials
{
  my $self = pop;
  my $c = 0;
  my $f = $self->{num}-1;
  for (my $i = $self->{num}-1; $i >= 0; $i--)
  {
    next unless $self->{list}[$i]{type} eq WORD;
    last unless $self->{list}[$i]{length} == 1;
    $c++;
    $f = $i;
  }

  if ($c >= 3)
  {
    $self->{num} = ($f == 0 ? 0 : $f-1);
  }

  if ($self->{num} >= 2 &&
      $self->{list}[$self->{num}-1]{type} eq PERIOD &&
     ($self->{list}[$self->{num}-2]{type} eq WORD ||
      $self->{list}[$self->{num}-2]{type} eq SPACE))
  {
    $self->{num}--;
  }
  elsif ($self->{num} >= 2 &&
      $self->{list}[$self->{num}-1]{type} eq SPACE &&
      $self->{list}[$self->{num}-2]{type} eq WORD)
  {
    $self->{num}--;
  }
}


sub fix_many_tokens
{
  my ($self, $dict_pos) = @_;

  return if $self->{num} <= 1;

  # Anything long with a period is considered a name by now
  for my $i (0 .. $self->{num}-1)
  {
    return if $self->{list}[$i]{type} eq PERIOD;
  }
  
  my $has_u = 0;
  my $has_l = 0;
  my $has_w = 0;
  my $has_i = 0;
  for my $i (0 .. $self->{num}-1)
  {
    next unless $self->{list}[$i]{type} eq WORD;
    my $c =$self->{list}[$i]{cap};
    if ($c eq UPPER)
    {
      $has_u = 1;
    }
    elsif ($c eq ILLFORMED)
    {
      $has_i = 1;
    }
    elsif ($c eq LOWER || ($i == 0 && $c eq WELLFORMED))
    {
      $has_l = 1;
    }
    else
    {
      $has_w = 1;
    }
  }

  if ($has_u && ! $has_l && ! $has_w && ! $has_i)
  {
    # All-caps are considered text by now.
    $self->{num} = 0;
    return;
  }

  if (! $has_u && $has_l && ! $has_w && ! $has_i)
  {
    # If first "real" word is in dict_pos, it's a name.
    # Otherwise delete all.
    my $f = 0;
    $f++ while ($f < $self->{num} &&
        ($self->{list}[$f]{type} ne WORD ||
	 $self->{list}[$f]{length} <= 1));

    if (! defined $dict_pos->{$self->{list}[$f]{lower}})
    {
      $self->{num} = 0;
      return;
    }
  }
}


sub fix_only_short
{
  my $self = pop;

  # Eliminate names with no word of at least 3 characters.
  for my $n (0 .. $self->{num}-1)
  {
    return if ($self->{list}[$n]{type} eq WORD &&
        (($self->{list}[$n]{length} > 2 ||
         ($self->{list}[$n]{length} == 2 &&
	  $self->{list}[$n]{lower} eq "lo"))));
  }
  $self->{num} = 0;
}


sub annotate
{
  my ($self, 
      $dict_first, 
      $dict_poor_last, 
      $dict_good_last, 
      $dict_ambig, 
      $dict_particles) = @_;

  for my $n (0 .. $self->{num}-1)
  {
    next unless $self->{list}[$n]{type} eq WORD;
    if ($self->{list}[$n]{length} == 1)
    {
      $self->{list}[$n]{meaning} = INITIAL;
    }
    elsif (defined $dict_first->{$self->{list}[$n]{lower}})
    {
      if (defined $dict_ambig->{$self->{list}[$n]{lower}})
      {
        $self->{list}[$n]{meaning} = AMBIGUOUS;
      }
      else
      {
        $self->{list}[$n]{meaning} = FIRST;
      }
    }
    elsif (defined $dict_particles->{$self->{list}[$n]{lower}})
    {
      $self->{list}[$n]{meaning} = PARTICLE;
    }
    else
    {
      $self->{list}[$n]{meaning} = OTHER;
    }
  }

  if ($self->{num} == 3)
  {
    _annotate3($self, $dict_first, $dict_poor_last, 
      $dict_good_last, $dict_ambig, $dict_particles);
  }
  elsif ($self->{num} == 5)
  {
    _annotate5($self, $dict_first, $dict_poor_last, 
      $dict_good_last, $dict_ambig, $dict_particles);
  }
  elsif ($self->{num} >= 7)
  {
    _annotate_long($self, $dict_particles);
  }
}


sub _annotate3
{
  my ($self, 
      $dict_first, 
      $dict_poor_last, 
      $dict_good_last, 
      $dict_ambig, 
      $dict_particles) = @_;

  if ($self->{list}[1]{type} eq SPACE)
  {
    if ($self->{list}[0]{meaning} eq FIRST)
    {
      if ($self->{list}[2]{meaning} eq FIRST)
      {
        # Lose about 900 double-first names.
        $self->{num} = 0;
      }
      elsif ($self->{list}[2]{meaning} eq AMBIGUOUS)
      {
	# Keep about 750 names: First, last
	$self->{list}[2]{meaning} = LAST;
      }
      elsif ($self->{list}[2]{meaning} eq INITIAL)
      {
        # Keep about 950 names: First, last initial
        $self->{list}[2]{meaning} = LAST;
      }
      elsif ($self->{list}[2]{meaning} eq PARTICLE)
      {
        # Lose about 10 odd names.
        $self->{num} = 0;
      }
      elsif ($self->{list}[2]{meaning} eq OTHER)
      {
	# Keep about 29,000 names.
	$self->{list}[2]{meaning} = LAST;
      }
      else
      {
        die "Unknown meaning";
      }
    }
    elsif ($self->{list}[0]{meaning} eq AMBIGUOUS)
    {
      if ($self->{list}[2]{meaning} eq FIRST)
      {
	if ($self->{list}[0]{cap} eq UPPER &&
	    $self->{list}[2]{cap} ne UPPER)
	{
	  # Strong reason to swap.
	  $self->{list}[0]{meaning} = LAST;
	}
	elsif ($self->{list}[0]{cap} ne UPPER &&
	    $self->{list}[2]{cap} eq UPPER)
	{
	  # Strong reason not to swap.
	  $self->{list}[0]{meaning} = FIRST;
	  $self->{list}[2]{meaning} = LAST;
	}
	elsif (defined $dict_poor_last->{$self->{list}[0]{lower}})
	{
	  # Drop entirely, as poor last name.
	  $self->{num} = 0;
	}
	elsif (defined $dict_good_last->{$self->{list}[0]{lower}})
	{
	  # Swap, as good last name.
	  $self->{list}[0]{meaning} = LAST;
	}
	else
	{
	  # Keep in order for lack of a better reason.
	  $self->{list}[0]{meaning} = FIRST;
	  $self->{list}[2]{meaning} = LAST;
	}
      }
      elsif ($self->{list}[2]{meaning} eq AMBIGUOUS)
      {
	# Keep in order for lack of a better reason.
	$self->{list}[0]{meaning} = FIRST;
	$self->{list}[2]{meaning} = LAST;
      }
      elsif ($self->{list}[2]{meaning} eq INITIAL)
      {
	$self->{list}[0]{meaning} = FIRST;
	$self->{list}[2]{meaning} = LAST;
      }
      elsif ($self->{list}[2]{meaning} eq PARTICLE)
      {
        $self->{num} = 0;
      }
      elsif ($self->{list}[2]{meaning} eq OTHER)
      {
        # About 10,000 cases.
	$self->{list}[0]{meaning} = FIRST;
	$self->{list}[2]{meaning} = LAST;
      }
      else
      {
        die "Unknown meaning";
      }
    }
    elsif ($self->{list}[0]{meaning} eq INITIAL)
    {
      # About 1,000 cases.
      $self->{list}[0]{meaning} = FIRST;
      $self->{list}[2]{meaning} = LAST;
    }
    elsif ($self->{list}[0]{meaning} eq PARTICLE)
    {
      # About 200 cases.
      $self->{num} = 0;
    }
    elsif ($self->{list}[0]{meaning} eq OTHER)
    {
      if ($self->{list}[0]{cap} eq UPPER &&
          $self->{list}[2]{cap} eq UPPER)
      {
        if ($self->{list}[2]{meaning} eq FIRST)
	{
          # About 60 cases.
          $self->{list}[0]{meaning} = LAST;
	}
	else
	{
          # About 900 cases, not great quality.
          $self->{list}[0]{meaning} = FIRST;
          $self->{list}[2]{meaning} = LAST;
	}
      }
      elsif ($self->{list}[0]{cap} eq UPPER)
      {
        if ($self->{list}[0]{length} >= 3 &&
	    $self->{list}[0]{letters} ne CONSONLY)
	{
	  if (1 >= $self->{list}[2]{value} =~ tr/[a-z]// &&
	      $self->{list}[2]{length} >= 4)
	  {
	    # Second word is "almost" upper case too.
            $self->{list}[0]{meaning} = FIRST;
            $self->{list}[2]{meaning} = LAST;
	  }
	  else
	  {
	    # About 140 cases.
            $self->{list}[0]{meaning} = LAST;
            $self->{list}[2]{meaning} = FIRST;
	  }
	}
	else
	{
          # About 200 cases.
          $self->{list}[0]{meaning} = LAST;
          $self->{list}[2]{meaning} = FIRST;
	}
      }
      elsif ($self->{list}[2]{cap} eq UPPER)
      {
	# About 1,400 cases.
        $self->{list}[0]{meaning} = FIRST;
        $self->{list}[2]{meaning} = LAST;
      }
      else
      {
	# About 34,400 cases.
        $self->{list}[0]{meaning} = FIRST;
        $self->{list}[2]{meaning} = LAST;
      }
    }
    else
    {
      die "Unknown meaning";
    }
  }
  elsif ($self->{list}[1]{type} eq COMMA)
  {
    # About 80 cases.
    $self->{list}[0]{meaning} = LAST;
    $self->{list}[2]{meaning} = FIRST;
  }
  elsif ($self->{list}[1]{type} eq DASH)
  {
    if ($self->{list}[2]{meaning} eq FIRST ||
        $self->{list}[2]{meaning} eq AMBIGUOUS)
    {
      # Less than 10 cases.
      $self->{num} = 0;
    }
    else
    {
      # About 60 cases.  Pretty mixed quality.
      $self->{list}[0]{meaning} = LAST;
      $self->{list}[2]{meaning} = FIRST;
    }
  }
  elsif ($self->{list}[1]{type} eq PERIOD)
  {
    # About 1,600 cases.
    $self->{list}[0]{meaning} = FIRST;
    $self->{list}[2]{meaning} = LAST;
  }
}


sub _annotate5
{
  my ($self, 
      $dict_first, 
      $dict_poor_last, 
      $dict_good_last, 
      $dict_ambig, 
      $dict_particles) = @_;

  if ($self->{list}[1]{type} eq SPACE &&
      $self->{list}[3]{type} eq SPACE)
  {
    if ($self->{list}[2]{meaning} eq INITIAL &&
        $self->{list}[4]{meaning} eq INITIAL)
    {
      if ($self->{list}[0]{meaning} eq FIRST ||
         ($self->{list}[0]{meaning} eq AMBIGUOUS &&
	 defined $dict_poor_last->{$self->{list}[0]{lower}}))
      {
        # About 20 cases of first initial initial -- drop.
	$self->{num} = 0;
      }
      else
      {
        # About 35 cases.  The Western ones lose...
	$self->{list}[0]{meaning} = LAST;
	$self->{list}[2]{meaning} = FIRST;
	$self->{list}[4]{meaning} = FIRST;
      }
    }
    elsif ($self->{list}[4]{meaning} eq INITIAL)
    {
      if ($self->{list}[2]{meaning} eq FIRST ||
         ($self->{list}[2]{meaning} eq AMBIGUOUS &&
	 defined $dict_poor_last->{$self->{list}[2]{lower}}))
      {
        # About 20 cases of first first initial -- drop.
	$self->{num} = 0;
      }
      elsif ($self->{list}[2]{meaning} eq PARTICLE)
      {
	# About 5 cases.
	$self->{list}[0]{meaning} = FIRST;
	$self->{list}[4]{meaning} = LAST;
      }
      else
      {
        # About 75 cases. Not high quality.
	$self->{list}[0]{meaning} = FIRST;
	$self->{list}[2]{meaning} = LAST;
	$self->{num} = 3;
      }
    }
    elsif ($self->{list}[2]{meaning} eq INITIAL ||
           $self->{list}[2]{meaning} eq PARTICLE)
    {
      # About 2000 cases, pretty good quality.
      $self->{list}[0]{meaning} = FIRST;
      $self->{list}[2]{meaning} = FIRST;
      $self->{list}[4]{meaning} = LAST;
    }
    elsif ($self->{list}[0]{cap} eq UPPER &&
           $self->{list}[2]{cap} eq UPPER)
    {
      if ($self->{list}[4]{cap} eq WELLFORMED)
      {
	# Very few cases.
        $self->{list}[0]{meaning} = LAST;
        $self->{list}[2]{meaning} = LAST;
        $self->{list}[4]{meaning} = FIRST;
      }
      else
      {
        # About 150 cases.
        $self->{list}[0]{meaning} = FIRST;
        $self->{list}[2]{meaning} = FIRST;
        $self->{list}[4]{meaning} = LAST;
      }
    }
    elsif ($self->{list}[4]{meaning} eq PARTICLE)
    {
      # About 10 cases, very different.
      $self->{num} = 0;
    }
    elsif ($self->{list}[0]{meaning} eq PARTICLE)
    {
      # About 30 cases.
      $self->{list}[0]{meaning} = LAST;
      $self->{list}[2]{meaning} = LAST;
      $self->{list}[4]{meaning} = FIRST;
    }
    elsif ($self->{list}[0]{cap} eq UPPER &&
        $self->{list}[0]{meaning} ne INITIAL)
    {
      # About 25 cases.
      $self->{list}[0]{meaning} = FIRST;
      $self->{list}[2]{meaning} = FIRST;
      $self->{list}[4]{meaning} = LAST;
    }
    elsif ($self->{list}[4]{length} == 2 &&
        $self->{list}[4]{cap} eq UPPER)
    {
      # About 30 cases.
      $self->{list}[0]{meaning} = FIRST;
      $self->{list}[2]{meaning} = LAST;
      $self->{num} = 3;
    }
    else
    {
      # About 3,600 cases.
      $self->{list}[0]{meaning} = FIRST;
      $self->{list}[2]{meaning} = FIRST;
      $self->{list}[4]{meaning} = LAST;
    }
  }
  elsif ($self->{list}[1]{type} eq SPACE &&
      $self->{list}[3]{type} eq DASH)
  {
    if (defined $dict_first->{$self->{list}[2]{lower}} &&
        defined $dict_first->{$self->{list}[4]{lower}})
    {
      # About 25 cases.
      $self->{list}[0]{meaning} = LAST;
      $self->{list}[2]{meaning} = FIRST;
      $self->{list}[4]{meaning} = FIRST;
    }
    else
    {
      # About 400 cases.
      $self->{list}[0]{meaning} = FIRST;
      $self->{list}[2]{meaning} = LAST;
      $self->{list}[4]{meaning} = LAST;
    }
  }
  elsif ($self->{list}[1]{type} eq DASH &&
      $self->{list}[3]{type} eq SPACE)
  {
    if (defined $dict_first->{$self->{list}[4]{lower}} &&
       $self->{list}[4]{cap} ne UPPER &&
       ! defined $dict_first->{$self->{list}[0]{lower}} &&
       ! defined $dict_first->{$self->{list}[2]{lower}})
    {
      # About 10 cases.
      $self->{list}[0]{meaning} = LAST;
      $self->{list}[2]{meaning} = LAST;
      $self->{list}[4]{meaning} = FIRST;
    }
    else
    {
      # About 550 cases.
      $self->{list}[0]{meaning} = FIRST;
      $self->{list}[2]{meaning} = FIRST;
      $self->{list}[4]{meaning} = LAST;
    }
  }
  elsif ($self->{list}[1]{type} ne COMMA &&
      $self->{list}[3]{type} eq COMMA)
  {
    if ($self->{list}[2]{meaning} eq INITIAL)
    {
      # About 4 cases.
      $self->{list}[0]{meaning} = FIRST;
      $self->{list}[2]{meaning} = FIRST;
      $self->{list}[4]{meaning} = LAST;
    }
    else
    {
      # About 10 cases.
      $self->{list}[0]{meaning} = LAST;
      $self->{list}[2]{meaning} = LAST;
      $self->{list}[4]{meaning} = FIRST;
    }
  }
  elsif ($self->{list}[1]{type} eq COMMA &&
      $self->{list}[3]{type} ne COMMA)
  {
    if ($self->{list}[0]{meaning} eq INITIAL)
    {
      # About 2 cases.
      $self->{list}[2]{meaning} = FIRST;
      $self->{list}[4]{meaning} = LAST;
      splice @{$self->{list}}, 0, 2;
      $self->{num} = 3;
    }
    else
    {
      # About 30 cases.
      $self->{list}[0]{meaning} = LAST;
      $self->{list}[2]{meaning} = FIRST;
      $self->{list}[4]{meaning} = FIRST;
    }
  }
  elsif ($self->{list}[4]{meaning} eq INITIAL)
  {
    if ($self->{list}[2]{meaning} eq INITIAL)
    {
      # About 50 cases.
      $self->{num} = 0;
    }
    else
    {
      # About 10 cases.
      $self->{list}[0]{meaning} = FIRST;
      $self->{list}[2]{meaning} = LAST;
      $self->{num} = 3;
    }
  }
  else
  {
    # About 1,500 cases.
    $self->{list}[0]{meaning} = FIRST;
    $self->{list}[2]{meaning} = FIRST;
    $self->{list}[4]{meaning} = LAST;
  }
}


sub _annotate_long
{
  my ($self, $dict_particles) = @_;

  my $num_commas = 0;
  my $n_comma = 0;
  for my $n (0 .. $self->{num}-1)
  {
    next unless $self->{list}[$n]{type} eq COMMA;
    $num_commas++;
    $n_comma = $n;
  }

  if ($num_commas >= 2)
  {
    die "Unknown situation (multiple commas left)";
  }
  elsif ($num_commas == 1)
  {
    # About 8 pretty messy situations.
    for (my $n = 0; $n < $n_comma; $n += 2)
    {
      $self->{list}[$n]{type} = LAST;
    }
    for (my $n = $n_comma+1; $n < $self->{num}; $n += 2)
    {
      $self->{list}[$n]{type} = FIRST;
    }
  }
  else
  {
    my $n = 0;
    $n++ while ($n < $self->{num} && 
          $self->{list}[$n]{meaning} ne PARTICLE);

    if ($n < $self->{num})
    {
      # About 400 cases.
      # Found a particle.  Everything before then is a first name,
      # after that come last names (other than more particles).
      for my $i (0 .. $n-1)
      {
        $self->{list}[$i]{meaning} = FIRST
          if ($self->{list}[$i]{type} eq WORD);
      }
      for my $i ($n+1 .. $self->{num}-1)
      {
        $self->{list}[$i]{meaning} = LAST
          if ($self->{list}[$i]{type} eq WORD &&
	      $self->{list}[$i]{meaning} ne PARTICLE);
      }
      return;
    }

    # Drop trailing initials under the circumstances.
    $n = $self->{num}-1;
    $n-- while ($n >= 0 &&
        ($self->{list}[$n]{type} ne WORD ||
	 $self->{list}[$n]{length} == 1));

    if ($n >= 0 && $n < $self->{num}-1)
    {
      $self->{num} = $n+1;
    }
    
    # Drop random trailing consonant pair.
    if ($self->{list}[$self->{num}-1]{length} >= 2 &&
        $self->{list}[$self->{num}-1]{letters} eq CONSONLY &&
	$self->{num} >= 3)
    {
      $self->{num} -= 2;
    }

    # About 300 cases, including some low-quality ones.
    # Just define as first+ last.
    for $n (0 .. $self->{num}-2)
    {
      $self->{list}[$n]{meaning} = FIRST
        if ($self->{list}[$n]{type} eq WORD);
    }
    $self->{list}[$self->{num}-1]{meaning} = LAST;
  }
}


sub check_annotations
{
  my $self = pop;
  return if $self->{num} <= 2;

  for my $n (0 .. $self->{num}-1)
  {
    next unless $self->{list}[$n]{type} eq WORD;
    if ($self->{list}[$n]{meaning} ne FIRST &&
        $self->{list}[$n]{meaning} ne LAST &&
        $self->{list}[$n]{meaning} ne PARTICLE)
    {
      print "XX ", $self->as_string, "\n";
    }
  }
}


sub capitalize
{
  my ($self, $fields) = @_;

  $fields->{first} = ();
  $fields->{particle} = ();
  $fields->{last} = ();
  $fields->{addition} = ();
  return if $self->{num} <= 1;

  for my $n (0 .. $self->{num}-1)
  {
    next unless $self->{list}[$n]{type} eq WORD;
    my $m = $self->{list}[$n]{meaning};
    my $r = \%{$self->{list}[$n]};
    if ($m eq FIRST)
    {
      push @{$fields->{first}}, _capitalize_first($r);
    }
    elsif ($m eq LAST)
    {
      push @{$fields->{last}}, _capitalize_last($r);
    }
    elsif ($m eq PARTICLE)
    {
      push @{$fields->{particle}}, _capitalize_particle($r);
    }
    else
    {
      die "Unknown capitalization state";
    }
  }

  if ($self->{addition} ne "")
  {
    my $a = $self->{addition};
    $a =~ s/\.$// if $a =~ /^ii/;
    push @{$fields->{addition}}, $self->{addition};
  }
}

sub _upchar
{
  my $c = pop;
  $c =~ tr/a-z\x9e\xe0-\xef\xf1-\xf6\xf8-\xfd/A-Z\x8e\xc0-\xcf\xd1-\xd6\xd8-\xdd/;
  return $c;
}


sub _capitalize_first
{
  my $r = pop;
  if ($r->{length} == 1)
  {
    return _upchar($r->{value}) . "." ;
  }
  elsif ($r->{length} == 2 &&
      $r->{cap} eq UPPER &&
      $r->{letters} eq CONSONLY)
  {
    if ($r->{lower} eq "th")
    {
      return "Th.";
    }
    else
    {
      return _upchar(substr($r->{value}, 0, 1)) . ". " .
             _upchar(substr($r->{value}, 1, 1)) . ".";
    }
  }
  else
  {
    return _upchar(substr($r->{value}, 0, 1)) . 
           substr($r->{value}, 1);
  }
}


sub _capitalize_last
{
  my $r = pop;
  if ($r->{length} == 1)
  {
    return _upchar($r->{value}) . "." ;
  }

  my $l = $r->{lower};
  if ($l =~ /^d'/ || $l =~ /^l'/)
  {
    if ($r->{length} < 4)
    {
      return substr($l, 0, 2) .
        _upchar(substr($r->{value}, 2, 1));
    }
    return substr($l, 0, 2) .
        _upchar(substr($r->{value}, 2, 1)) .
        substr($r->{lower}, 3);
  }
  elsif ($l =~ /^mc/)
  {
    if ($r->{length} < 4)
    {
      return _upchar(substr($r->{value}, 0, 1)) .
          substr($r->{lower}, 1);
    }
    return "Mc" . 
        _upchar(substr($r->{value}, 2, 1)) .
        substr($r->{lower}, 3);
  }
  elsif ($l =~ /^mac/)
  {
    if ($r->{length} < 5)
    {
      return _upchar(substr($r->{value}, 0, 1)) .
          substr($r->{lower}, 1);
    }
    return "Mac" . 
        _upchar(substr($r->{value}, 3, 1)) .
        substr($r->{lower}, 4);
  }
  elsif ($l =~ /^o'/)
  {
    if ($r->{length} < 4)
    {
      return _upchar(substr($r->{value}, 0, 1)) .
          substr($r->{lower}, 1);
    }
    return "O'" .
        _upchar(substr($r->{value}, 2, 1)) .
        substr($r->{lower}, 3);
  }
  else
  {
    return _upchar(substr($r->{value}, 0, 1)) .
        substr($r->{lower}, 1);
  }
}


sub _capitalize_particle
{
  my $r = pop;
  return $r->{lower};
}


sub is_short
{
  my $self = pop;
  return ($self->{num} <= 1 ? 1 : 0);
}


sub get_length
{
  my $self = pop;
  $self->{num}
}


sub as_string
{
  my $self = pop;
  my $line = "";
  for my $n (0 .. $self->{num}-1)
  {
    $line .= $self->{list}[$n]{value};
  }

  if ($self->{addition} ne "")
  {
    $line .= ", $self->{addition}";
  }

  return $line;
}


1;
