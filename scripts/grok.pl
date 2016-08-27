#!perl

# Part of BridgeData.
#
# Copyright (c) 2016 by Soren Hein.
#
# See LICENSE and README.

use strict;
use warnings;
use Name;

my %dict;
set_dictionaries();

my $name = Name->new();
my @hist;

open my $FH, "<", "c.txt" or die "Can't open file";
while (<$FH>)
{
  my $line = $_;
  chomp $line;
  $line =~ s///g;

  $name->set_string($line);

  $name->delete_from_dict(\%{$dict{words}}, 0);
  $name->delete_from_dict(\%{$dict{after_comma}}, 1);
  $name->delete_from_dict(\%{$dict{after_dash}}, 2);

  $name->delete_titles(\%{$dict{titles}}, \%{$dict{title_pairs}});

  $name->fix_periods();

  $name->fix_dashes(\%{$dict{first_names}});

  $name->fix_additions(\%{$dict{additions}});

  $name->fix_commas(\%{$dict{places}});

  $name->fix_place_pairs(\%{$dict{place_pairs}});

  # $name->check_tightness();

  $name->fix_trailing_initials();

  if ($name->get_length() >= 7)
  {
    $name->fix_many_tokens(\%{$dict{first_names}});
  }

  $name->fix_only_short();

  # if ($line =~ /Starre/)
  # {
    # my $x;
  # }

  $name->annotate(
    \%{$dict{first_names}}, 
    \%{$dict{poor_last_names}}, 
    \%{$dict{good_last_names}}, 
    \%{$dict{ambig_names}}, 
    \%{$dict{particles}});

  # $name->check_annotations();

  my %fields;
  $name->capitalize(\%fields);

  $hist[$name->get_length()]++;
  if ($name->get_length() >= 3)
  {
    print $name->as_string(), "\n";
    print print_field($fields{first}, "First");
    print print_field($fields{particle}, "Particle");
    print print_field($fields{last}, "Last");
    print print_field($fields{addition}, "Addition");
    print "\n";
  }
}

# exit;
for (my $i = 0; $i <= $#hist; $i++)
{
  next unless defined $hist[$i];
  printf "%3d  %6d\n", $i, $hist[$i];
}

exit;
for my $k (sort keys %{$dict{after_dash}})
{
  printf "%3d  %6s\n", $dict{after_dash}{$k}-1, $k;
}

exit;


sub set_dictionaries
{
  my @aw = qw( 
    acbl acm addicted advanced afl after again against agence ah 
    aka aka aksa aksaray all always amateur amici 
    amicidiitalo any appel appelasd area as ask att ave
    baby bacon bad bbk bbo bcsh bco being benji better bfans 
    bg bid bidding big bil bjk blkw blkwd blm blw bonjour born 
    bottom boule branche bric bridge bsc bsk bueno by
    call calling capp captain card cardinal carding cards
    carreau catastrofic cbfriends ceg centrum cheerful chinese 
    citizen cmk code collante come common competent conv country 
    course crazy cue cuebid cz
    dbl demande dessus dfs dft diem differenza diren discard discards 
    dit dlm doch dont dream dreams droit dsi
    easy ein electron elm en enc enchere enjoy ent epi erdemdir 
    error esk et ex excel excellent exert exp expat exper expert
    fans fast feda festival ffb figb fj flight fly for forc forcing 
    forever forum freude friends from fun
    galaxyclub game gamle garozzo give gl glm godt good
    happy have hazard heart hellas hep her here hey home homo horse
    iac ich idiot iit iitk iitm il ilacy imp inter intermediate 
    intl inv inverted is isik italian italiano italo itbhu
    je joue journey judge jump just
    kamera klb kleppe ksk kunta kurd
    lav lavinthal lead leave leb lebenshol lebensohl level liberalizm 
    lido life like lives lm losing loue love loves lucky
    made make mani marlboro master maybe meine memb member 
    memento memory merci mind minor misunderstood misura mnit 
    modest multi my
    natural nbo ncis ne need neg negative nein new nice nihil 
    niit nit nmf no non noob not now nr nt
    often ogust old online only open ortum over
    parle partner peace petite pga pl plan planet play player players 
    playing pleasant please pls plse plus point polite poor pour 
    precision prefer pro profesional profil profile psu ptit puppet 
    pzbs
    queen qf
    raise raises relax remove respect rev reverse right rivate rlm rta
    sakarya 
    sayc sef simply sinner sistema slam slm slow smile solo 
    sometimes sono sorry spasso speak specific splinter spog 
    sposata spreche sry stand standard stayman std stym suis 
    suit suop supp svp sympa system
    table take talk tambien td teaching team teams tempo test the
    think this time tmt tolclub tourn town transf transfer transfers 
    tres trial trumps try trying turkish two
    udca un undo undos unibridge united unusual upon upside usbf
    vdm very village vive
    want warning wbf weak wer what when where who why with
    wjs world 
    xfer 
    years yes you your
    );

  $dict{words}{$_} = 1 for (@aw);

  my @cw = qw(
    allsuits bbofan capeletti comments cugino dare de dds duke 
    equality est emerald flannery german gwl i iii iitb irsee iitm
    keep karo l lvnt le la
    ms membre mentor mon meet more mystery mel
    ni parle per pd pid roudi ruby short spd
    truscott transf unt uom use will zine zero
  );

  $dict{after_comma}{$_} = 1 for (@cw);

  my @dw = qw(
    antalya bilgrad cogito club ege fair group
    hard hoch in izmir la low maior mtv uludag xxx
  );

  $dict{after_dash}{$_} = 1 for (@dw);

  my @tw = qw(adv afm av capt col doc dr dt engin engr fr ir 
    md mr mrs ms mgs opr presidente prof sir tc);

  $dict{titles}{$_} = 1 for (@tw);

  # Title pairs
  my @tw2a = qw(dr  ph m m  t op mgr);
  my @tw2b = qw(med d  d mt c dr inz);
  for my $i (0 .. $#tw2a)
  {
    push @{$dict{title_pairs}{$tw2a[$i]}}, $tw2b[$i];
  }

  my @pw = qw(jr jre sr sen ii iii);

  $dict{additions}{$_} = 1 for (@pw);

  my @cl = qw(
    aalesund airdrie alanya alberta aligarh ankara arcadia barcelona
    bochum boston botan calgary dubrovnik fbg fethiye firenze 
    göcek guadalope hajdu houston hyderabad ingolstadt istanbul
    izmir kona lincoln lofoten livorno lyngby mesa milan montrose 
    oslo riviera rossbach mumbai rognan steinkjer tours turgay 
    victoria);
  
  my @sl = qw(
    alaska angola az bc ca colorado fl florida hawaii in iowa
    jabalpur jodh maine mich motherland ny ottawa qc québec 
    sc tasmania tex wa wi yulin
  );
  
  my @nl = qw(denmark finland hungary indonesia 
    malta norway taiwan world);

  $dict{places}{$_} = 1 for (@cl);
  $dict{places}{$_} = 1 for (@sl);
  $dict{places}{$_} = 1 for (@nl);

  # Place pairs
  my @pw2a = qw(alta  bluff biarritz boca  las    las    santa  green 
                new   san   st);
  my @pw2b = qw(baja  city  bayonne  raton vegas  palmas clara  bay   
                delhi ant   petersburg);
  for my $i (0 .. $#pw2a)
  {
    $dict{place_pairs}{$pw2a[$i]} = $pw2b[$i];
  }

  my @fn = qw(
    abdullah ad ada adam adnan adrian ahmed ahmet
    al alain alan albert alberto aleksandar alessandro alex
    alfred ali alice alicia allan allen amit ana
    anders andre andrea andrew andrzej andy ane angela
    angelo anil anita ann anna anne annette annie
    anthony anton antonia antonio arif arlene arne arnold
    art arthur ashok aydin ayhan barb barbara barry
    beaufrere becky ben bengt bente berit bernard bernice
    bert bertil beth betsy betty beverly bill birgit
    birthe bjorn bo bob bobby bogdan bonnie boris
    brenda brian brigitte bruce bruno bryan burak burhan
    can carl carla carlo carlos carmen carol carole
    carolyn catherine cecilia cem cemal cemil cengiz chantal 
    charles charlie charlotte chen cheryl chris christian christina
    christine christopher chuck claire clara clare claude claudia
    claudio colin connie coral craig cristina cun cynthia
    dai dale dan daniel daniela danker danny dave
    david dee delia denis denise dennis derek diana
    diane dick diego dirk don donald donna doris
    dorothy doug douglas ed eddie edith edna eduardo
    edward eileen einar eivind elaine eleanor elena eli
    elisabeth elizabeth ella ellen elly els else emilio 
    emily emin erdal erdogan eric erik ester esther
    ethel eva evelien evelyn fatih fernando finn florence
    fran frances francesco francis franco frank frans fred
    fuat gail gary geir gene geoff george georgi
    gerald gerard gerd gerda gerry gianni gilles giordano
    giorgio giovanni giuseppe glenn gloria gordon grace graham
    greg grethe grzegorz gun gunilla gunnar guy hakan
    hal halil hanne hanny hans harald harold harry
    harvey hasan helen helene helge helmi henk henrik
    henry henryk herman heysch howard hugh hugo huseyin
    ian ibrahim ida ihsan ina inge inger ingrid
    irene iris isabel ismail ivan jacek jack jackie
    jacob jacqueline jacques jaime james jan jane janet
    janice jany janusz jarandyr jason jay jean jeanne
    jeff jennifer jenny jens jerry jerzy jette jill
    jim jimmy jo joan joann joanna joanne joao
    joe joel joep johan john johnny joke jon
    jons joop jorge jos jose joseph jozeph joyce
    juan jonathan joy judi judith judy julia julian
    julie june jurek jytte kaj kamil karen karin
    karl kate katherine kathleen kathy kawbab kay kees
    keith kemal ken kenneth kent kerstin kevin khaled 
    kim kirilsa kirsten kjell klaus knud knut koos 
    kostas krzysztof kunwar kwok kurt larry lars laura
    lea lee leif lena lennart leo leon les
    leslie leszek levent lila lillian linda ling lisa
    lise liz lois lorraine louis louise luc lucia
    lucy luigi luis lynn lynne maartje madeleine maggie
    magnus mahmoud maillet malcolm manuel mar marc marcel
    marcia marcin marco marek margaret margie maria marian
    marianne marie marilyn marina mario marion marius marjorie
    mark marlene marsha marta martha martien martin mary
    massimo maureen maurice max maxine may mehmet mel
    metin michael michal michel michele micheline mike mirek
    miriam mohamed mona monica monika monique morten moshe
    murat mustafa nancy neil nel nelly nick nico
    nicole niek niels nils nina norma norman odd
    ola ole olivier om orhan oscar osman otto
    ove pablo pam pamela paolo pat patricia patrick
    paul paula pauline paulo pedro peg peggy penny
    per perry pete peter phil philip philippe phyllis
    pierre piet pietro pilar piotr piotrus pkees poul
    rachel radu raj ralph ramazan raul ray raymond
    reidar renate rene renee ria rich richard rick
    riet rita rob robert roberta roberto robin rod
    roger roland rolf roman ron ronald rosa rose
    rosemary roy ruby rumen rune ruth ryszard salih
    sally sam samuel sandra sandy sanjay sara sarah
    scott sean serdar serge sergio sharon sheila shirley
    silvia simon simone sinan sonia sonja stan stanislaw
    stanley stefan stephen steve steven sue susan susanne
    suzanne suze svein sven svend sylvia tadeusz tahir
    ted teresa terje terry theo thomas tim tina
    tom tomasz tomek tommy ton tony tor tore
    tove trond trudy ugur ulf ulla ursula valentiner 
    vicki victor vincent virginia vroeg walter wayne wendy
    werner wieslaw wil will willem william willie willy
    wouter wim yasar yasasin yi yilmaz yusuf yvon
    yvonne zafer zbigniew zbyszek zygmunt
  );

  $dict{first_names}{$_} = 1 for (@fn);

  my @alson = qw(
    adam adrian albert allen anders anthony arnold barb
    barry bente bernard betty brian bruce bruno charles
    chen chris christopher colin craig dale daniel david
    dennis dick diego donald douglas fernando finn francis
    franco frank gary george georgi gerard giordano giovanni
    glenn gloria gordon grace graham guy hans harry
    harvey henny henry herman howard hugo ibrahim jacob
    jacques james jason jay jean john joseph joyce
    kamil karl kay keith kent kim kostas kurt
    kwok lee leon leslie ling louis lynn malcolm
    manuel mark martin may michael norman odd olivier
    patrick paul penny peter perry philip pierre pilar
    ray raymond rich richard rob robert robin roger
    rose roy ruby samuel sara scott simon stanley
    terry thomas victor walter wayne werner yi
  );

  $dict{ambig_names}{$_} = 1 for (@alson);

  my @pn = qw(
    bin bir da de del della den der des di du 
    el ibn in la le op st 't te ten ter van von zu
  );

  $dict{particles}{$_} = 1 for (@pn);

  my @poor = qw(
    betty finn francis giovanni jacques james jean john joseph
    hans karl kurt martin paul pierre rose sara victor
  );

  $dict{poor_last_names}{$_} = 1 for (@poor);

  my @good = qw(
    albert anders anthony bernard bruce david simon terry thomas
  );

  $dict{good_last_names}{$_} = 1 for (@good);
}


sub print_field
{
  my ($a, $title) = @_;
  return "" unless defined $a;
  my $s = sprintf "%-8s %s\n", $title,  join(" ", @$a);
  return $s;
}

