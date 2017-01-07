#!perl

# Part of BridgeData.
#
# Copyright (c) 2016 by Soren Hein.
#
# See LICENSE and README.


use strict;
use warnings;

sub set_valid_special_characters
{
  my ($special_chars, $unicodes) = @_;

  # Special characters that occur in the db file.
  my @sp = (
    #  °
    0xb0, 
    #  Á     Â    ~A     Ä    AA    AE    C,     È
    0xc1, 0xc2, 0xc3, 0xc4, 0xc5, 0xc6, 0xc7, 0xc8, 
    #  É     Í     Î    :I
    0xc9, 0xcd, 0xce, 0xcf,
    # ~N     Ò     Ó    ~O     Ö    OE     Ú    Ü      ß
    0xd1, 0xd2, 0xd3, 0xd5, 0xd6, 0xd8, 0xda, 0xdc, 0xdf,
    #  à     á     â    ~a     ä    aa    ae    c,
    0xe0, 0xe1, 0xe2, 0xe3, 0xe4, 0xe5, 0xe6, 0xe7, 
    #  è     é     ê    :e     ì     í     î    :i
    0xe8, 0xe9, 0xea, 0xeb, 0xec, 0xed, 0xee, 0xef,
    # ~d    ~n     ò     ó     ô    ~o     Ö    OE
    0xf0, 0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf8,
    #  ù     ú     û     ü    :y
    0xf9, 0xfa, 0xfb, 0xfc, 0xff);

  $special_chars->[$_] = 1 for (@sp);

  my @u0 = qw(
    0101 0102 0105 0107 010D 
    0115 0119 011B 011E 011F 
    0120 0123 012B
    0130 0131 0133 0134
    0141 0142 0143 0144 014E
    0151 0152 0156 0159 015A 015B 015E 015F 
    0160 0161 0162 0164 0166 0167 016B
    0172 0173 0175 0177 0179 017A 017B 017C 017E 017F
    0180 018D 018F
    0190 0199 019A
    01A6 01C0 01D6 01D8);

  # ASCII approximations
  my @u1 = qw(
       a    A    a    c    c    
       e    e    e    G    g    
       G    g    i
       I    i    j    J
       L    l    N    n    O
       ö    O    R    r    s    s    S    s    
       S    s    T    T    T    t    u
       U    u    w    y    Z    z    Z    y    z   s
       b    o    E
       E    k    l
       R    l    u    u);

  for my $i (0 .. $#u0)
  {
    $unicodes->{$u0[$i]} = $u1[$i];
  }
}


sub set_dictionaries
{
  my $dict = pop;

  # General dictionary of words from which on we delete.
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

  $dict->{words}{$_} = 1 for (@aw);

  # Words from which we only delete after the first comma.
  my @cw = qw(
    allsuits bbofan capeletti comments cugino dare de dds duke 
    equality est emerald flannery german gwl i iii iitb irsee iitm
    keep karo l lvnt le la
    ms membre mentor mon meet more mystery mel
    ni parle per pd pid roudi ruby short spd
    truscott transf unt uom use will zine zero
  );

  $dict->{after_comma}{$_} = 1 for (@cw);

  # Words from which we only delete after the first dash.
  my @dw = qw(
    antalya bilgrad cogito club ege fair group
    hard hoch in izmir la low maior mtv uludag xxx
  );

  $dict->{after_dash}{$_} = 1 for (@dw);

  # One-word titles that we delete (leaving the rest intact).
  my @tw = qw(adv afm av capt col doc dr dt engin engr fr ir 
    md mr mrs ms mgs opr presidente prof sir tc);

  $dict->{titles}{$_} = 1 for (@tw);

  # Title pairs, separated by space or period.
  my @tw2a = qw(dr  ph m m  t op mgr);
  my @tw2b = qw(med d  d mt c dr inz);
  for my $i (0 .. $#tw2a)
  {
    push @{$dict->{title_pairs}{$tw2a[$i]}}, $tw2b[$i];
  }

  # Name additions.
  my @pw = qw(jr jre sr sen ii iii);

  $dict->{additions}{$_} = 1 for (@pw);

  # Cities.
  my @cl = qw(
    aalesund airdrie alanya alberta aligarh ankara arcadia barcelona
    bochum boston botan calgary dubrovnik fbg fethiye firenze 
    göcek guadalope hajdu houston hyderabad ingolstadt istanbul
    izmir kona lincoln lofoten livorno lyngby mesa milan montrose 
    oslo riviera rossbach mumbai rognan steinkjer tours turgay 
    victoria);
  
  # States.
  my @sl = qw(
    alaska angola az bc ca colorado fl florida hawaii in iowa
    jabalpur jodh maine mich motherland ny ottawa qc québec 
    sc tasmania tex wa wi yulin
  );
  
  # Countries.
  my @nl = qw(denmark finland hungary indonesia 
    malta norway taiwan world);

  $dict->{places}{$_} = 1 for (@cl);
  $dict->{places}{$_} = 1 for (@sl);
  $dict->{places}{$_} = 1 for (@nl);

  # Place pairs
  my @pw2a = qw(alta  bluff biarritz boca  las    las    santa  green 
                new   san   st);
  my @pw2b = qw(baja  city  bayonne  raton vegas  palmas clara  bay   
                delhi ant   petersburg);
  for my $i (0 .. $#pw2a)
  {
    $dict->{place_pairs}{$pw2a[$i]} = $pw2b[$i];
  }

  # Frequently occurring first names (not exhaustive).
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

  $dict->{first_names}{$_} = 1 for (@fn);

  # Ambiguous first names that may also be last names.
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

  $dict->{ambig_names}{$_} = 1 for (@alson);

  # Ambiguous names that are possible last names, but poor ones.
  my @poor = qw(
    betty finn francis giovanni jacques james jean john joseph
    hans karl kurt martin paul pierre rose sara victor
  );

  $dict->{poor_last_names}{$_} = 1 for (@poor);

  # Ambiguous names that are good last names as well.
  my @good = qw(
    albert anders anthony bernard bruce david simon terry thomas
  );

  $dict->{good_last_names}{$_} = 1 for (@good);

  # Dutch last names for which "v.d." expands to "van den".
  my @van_den = qw(
    berg bosch enk nieuwenhof
  );

  $dict->{part_van_den}{$_} = 1 for (@van_den);

  # Dutch last names for which "v.d." expands to "van de".
  my @van_de = qw(
    dool heuvel konijnenberg reijt scheur tillaar wetering
  );

  $dict->{part_van_de}{$_} = 1 for (@van_de);

  # Rest of v.d. are "van der".

  # Dutch last names for which "v" expands to "van".
  my @van = qw(
    amerongen amsterdam bussel cranenbroek diemen erp
    kappel leeuwen luyk nieuwkerk oijen paassen rochow rooijen 
    straten tartwijk thiel velthoven woerden
  );

  $dict->{part_van}{$_} = 1 for (@van);

  # Other names (German, Polish) for which "v" expands to "von".
  my @von = qw(
    heuzen lowzow werthern zarzycki
  );

  $dict->{part_von}{$_} = 1 for (@von);

  # Scottish last names for which O Name becomes O'Name.
  my @scots = qw(
    briain brien callaghan carroll connell connor
    donnell donoghue donovan fagan gorman halloran
    hara keeffe lubaigh mahony mearain neill
    regan riordan shaunessy sullivan toole
  );

  $dict->{part_scots}{$_} = 1 for (@scots);

  # Scottish last names for which Mac Name becomes MacName.
  my @macs = qw(
    allan allister auliffe brien cann carthy cormac
    donald evoy ewan gowan gregor innis
    kenna laughlin namara quilkin
  );

  $dict->{part_macs}{$_} = 1 for (@macs);

  # Scottish last names for which Mc Name becomes McName.
  my @mcs = qw(
    alister allister allan annalley auliffe brien cann
    carry carthy cormack crudden donagh donald donnell
    evoy ewan fadden gann gowan gregor iver
    kanzie kecaze kenna laughlin loughlin mahon michael
    namara nulty quilkin weeney
  );

  $dict->{part_mcs}{$_} = 1 for (@mcs);

  # Name particles.
  my @pn = qw(
    bin bir da de del della den der des di du 
    el ibn in la le op st 't te ten ter van von zu
  );

  $dict->{particles}{$_} = 1 for (@pn);

  # Internet domains.
  my @domains = qw(
    at ca ch cn dk es eu fi fr gr hk
    ie il in it jp kr nl no pl pt tr tw uk
    com org net int edu gov mil
  );

  $dict->{domains}{$_} = 1 for (@domains);
}



1;
