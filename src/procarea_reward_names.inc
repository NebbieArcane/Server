/* Nomi unici premio istanza: keyword, short_descr, description per variante. */
struct ProcRewardNameEntry {
	const char* keywords;
	const char* short_descr;
	const char* description;
};

static constexpr ProcRewardNameEntry kProcRewardLightNames[PROCAREA_TEMPLATE_BANDS] = {
	{
		"candela velo bruma sottile",
		"una candela del velo brumoso",
		"La fiamma e' piccola e incerta, come accesa dentro una bolla di nebbia:\n"
		"illumina senza scaldare, e la cera non gocciola mai.\n",
	},
	{
		"lanterna runa sussurrante",
		"una lanterna dalle rune sussurranti",
		"I simboli sul vetro mutano mentre la guardi;\n"
		"ogni runa sembra voler dirti qualcosa, poi dimentica di averlo fatto.\n",
	},
	{
		"lanterna runa sussurrante",
		"una lanterna dalle rune sussurranti",
		"I simboli sul vetro mutano mentre la guardi;\n"
		"ogni runa sembra voler dirti qualcosa, poi dimentica di averlo fatto.\n",
	},
	{
		"torcia nebbia argentata",
		"una torcia di nebbia argentata",
		"Il fuoco e' bianco e freddo al tatto, come la Piazza delle Nebbie\n"
		"riflessa in una fiamma che non consuma legna.\n",
	},
	{
		"torcia nebbia argentata",
		"una torcia di nebbia argentata",
		"Il fuoco e' bianco e freddo al tatto, come la Piazza delle Nebbie\n"
		"riflessa in una fiamma che non consuma legna.\n",
	},
	{
		"globo luce varco oscuro",
		"un globo di luce del varco oscuro",
		"Una sfera nera trattiene al centro un nucleo violaceo:\n"
		"irradia luce come se il buio stesso si rifiutasse di vincere del tutto.\n",
	},
	{
		"globo luce varco oscuro",
		"un globo di luce del varco oscuro",
		"Una sfera nera trattiene al centro un nucleo violaceo:\n"
		"irradia luce come se il buio stesso si rifiutasse di vincere del tutto.\n",
	},
	{
		"fiaccola cenere custode",
		"una fiaccola di cenere del custode",
		"Braci grigie si rigenerano da sole sul bastone di ossidiana;\n"
		"profuma di sala finale ancora calda.\n",
	},
	{
		"fiaccola cenere custode",
		"una fiaccola di cenere del custode",
		"Braci grigie si rigenerano da sole sul bastone di ossidiana;\n"
		"profuma di sala finale ancora calda.\n",
	},
	{
		"stella eclisse morente",
		"una stella dell'eclisse morente",
		"Una scheggia di cielo spento orbita lentamente attorno al tuo polso:\n"
		"non e' calda, ma la stanza sembra piu' vicina al varco quando la accendi.\n",
	},
};

static constexpr ProcRewardNameEntry kProcRewardFingerNames[PROCAREA_TEMPLATE_BANDS * 2] = {
	{ "anello bruma sigillo infranto", "l'anello del sigillo infranto",
	  "Metallo opaco, inciso con una runa spezzata a meta':\n"
	  "chi lo indossa al dito sinistro sente il velo cedere appena.\n" },
	{ "anello bruma filo speranza", "l'anello del filo di speranza",
	  "Un filo d'argento avvolge il dito destro come nebbia solida;\n"
	  "e' leggero, quasi invisibile, ma non scivola mai.\n" },
	{ "anello runa eco svanita", "l'anello dell'eco svanita",
	  "La pietra al centro pulsa una volta, poi tace:\n"
	  "al dito sinistro ricorda un incantesimo interrotto a meta'.\n" },
	{ "anello runa verso dimenticato", "l'anello del verso dimenticato",
	  "Runi minuscole corrono sul bordo esterno;\n"
	  "al dito destro sembrano scritte per chi ha gia' lasciato la dimensione.\n" },
	{ "anello runa eco svanita", "l'anello dell'eco svanita",
	  "La pietra al centro pulsa una volta, poi tace:\n"
	  "al dito sinistro ricorda un incantesimo interrotto a meta'.\n" },
	{ "anello runa verso dimenticato", "l'anello del verso dimenticato",
	  "Runi minuscole corrono sul bordo esterno;\n"
	  "al dito destro sembrano scritte per chi ha gia' lasciato la dimensione.\n" },
	{ "anello nebbia luna specchiata", "l'anello della luna specchiata",
	  "Il gioiello riflette la nebbia anche al coperto:\n"
	  "al dito sinistro e' freddo come vetro appena lavato dalla bruma.\n" },
	{ "anello nebbia stella caduta", "l'anello della stella caduta",
	  "Un puntino argenteo brilla al centro del cerchio;\n"
	  "al dito destro pare una stella caduta e poi recuperata dal velo.\n" },
	{ "anello nebbia luna specchiata", "l'anello della luna specchiata",
	  "Il gioiello riflette la nebbia anche al coperto:\n"
	  "al dito sinistro e' freddo come vetro appena lavato dalla bruma.\n" },
	{ "anello nebbia stella caduta", "l'anello della stella caduta",
	  "Un puntino argenteo brilla al centro del cerchio;\n"
	  "al dito destro pare una stella caduta e poi recuperata dal velo.\n" },
	{ "anello varco catena spezzata", "l'anello della catena spezzata",
	  "Due anelli intrecciati si aprono come una serratura rotta:\n"
	  "al dito sinistro porta ancora l'odore del tempio nella foresta.\n" },
	{ "anello varco chiave senza porta", "l'anello della chiave senza porta",
	  "La gemma e' nera e assorbe la luce circostante;\n"
	  "al dito destro sembra aprire porte che non esistono piu'.\n" },
	{ "anello varco catena spezzata", "l'anello della catena spezzata",
	  "Due anelli intrecciati si aprono come una serratura rotta:\n"
	  "al dito sinistro porta ancora l'odore del tempio nella foresta.\n" },
	{ "anello varco chiave senza porta", "l'anello della chiave senza porta",
	  "La gemma e' nera e assorbe la luce circostante;\n"
	  "al dito destro sembra aprire porte che non esistono piu'.\n" },
	{ "anello custode giuramento antico", "l'anello del giuramento antico",
	  "Inciso dentro c'e' una croce obliqua consumata dal tempo;\n"
	  "al dito sinistro pesa come una promessa fatta al custode.\n" },
	{ "anello custode ultimo respiro", "l'anello dell'ultimo respiro",
	  "Il metallo e' scuro e caldo, come appena tolto da una mano caduta;\n"
	  "al dito destro vibra quando la sala finale e' vicina.\n" },
	{ "anello custode giuramento antico", "l'anello del giuramento antico",
	  "Inciso dentro c'e' una croce obliqua consumata dal tempo;\n"
	  "al dito sinistro pesa come una promessa fatta al custode.\n" },
	{ "anello custode ultimo respiro", "l'anello dell'ultimo respiro",
	  "Il metallo e' scuro e caldo, come appena tolto da una mano caduta;\n"
	  "al dito destro vibra quando la sala finale e' vicina.\n" },
	{ "anello eclisse orbita nera", "l'anello dell'orbita nera",
	  "Un cerchio di ossidiana orbita intorno al dito sinistro;\n"
	  "non tocca la pelle, ma non si stacca neanche.\n" },
	{ "anello eclisse sole ingoiato", "l'anello del sole ingoiato",
	  "Al centro brilla un alone dorato intrappolato nel nero;\n"
	  "al dito destro ricorda l'istante in cui la dimensione si chiude.\n" },
};

static constexpr ProcRewardNameEntry kProcRewardNeckNames[PROCAREA_TEMPLATE_BANDS * 2] = {
	{ "collare bruma filo nebbia", "il collare a filo di nebbia",
	  "Un filo di perle opache cinge il collo corto:\n"
	  "ogni perla e' una goccia di bruma cristallizzata.\n" },
	{ "amuleto bruma sussurro lontano", "l'amuleto del sussurro lontano",
	  "Pende basso sul petto e tintinna senza vento;\n"
	  "a volte sembra ripetere frasi dette da qualcuno gia' uscito dal velo.\n" },
	{ "collare runa verso spezzato", "il collare al verso spezzato",
	  "Rune incise su piastre sottili si scontrano al movimento;\n"
	  "corto, aderente, come un sigillo che non vuole cadere.\n" },
	{ "amuleto runa eco dimenticata", "l'amuleto dell'eco dimenticata",
	  "Un cristallo opaco pende lungo fino allo sterno;\n"
	  "dentro si muove una luce che nessuno ricorda aver acceso.\n" },
	{ "collare runa verso spezzato", "il collare al verso spezzato",
	  "Rune incise su piastre sottili si scontrano al movimento;\n"
	  "corto, aderente, come un sigillo che non vuole cadere.\n" },
	{ "amuleto runa eco dimenticata", "l'amuleto dell'eco dimenticata",
	  "Un cristallo opaco pende lungo fino allo sterno;\n"
	  "dentro si muove una luce che nessuno ricorda aver acceso.\n" },
	{ "collare nebbia argento gelido", "il collare d'argento gelido",
	  "Maglie sottili riflettono la Piazza anche lontano da Myst;\n"
	  "corto, elegante, freddo come la nebbia all'alba.\n" },
	{ "amuleto nebbia stella darkstar", "l'amuleto della stella DarkStar",
	  "Una stella a otto punte di nebbia solida pende sul cuore;\n"
	  "pulsa lentamente, come un faro nella dimensione.\n" },
	{ "collare nebbia argento gelido", "il collare d'argento gelido",
	  "Maglie sottili riflettono la Piazza anche lontano da Myst;\n"
	  "corto, elegante, freddo come la nebbia all'alba.\n" },
	{ "amuleto nebbia stella darkstar", "l'amuleto della stella DarkStar",
	  "Una stella a otto punte di nebbia solida pende sul cuore;\n"
	  "pulsa lentamente, come un faro nella dimensione.\n" },
	{ "collare varco catena oscura", "il collare a catena oscura",
	  "Anelli neri si intrecciano al collo senza peso;\n"
	  "corto, severo, profuma di muschio e pietra bagnata.\n" },
	{ "amuleto varco croce obliqua", "l'amuleto della croce obliqua",
	  "La croce di DarkStar e' incisa su una piastra consumata;\n"
	  "lungo, penzolante, scalda appena quando guardi verso l'uscita.\n" },
	{ "collare varco catena oscura", "il collare a catena oscura",
	  "Anelli neri si intrecciano al collo senza peso;\n"
	  "corto, severo, profuma di muschio e pietra bagnata.\n" },
	{ "amuleto varco croce obliqua", "l'amuleto della croce obliqua",
	  "La croce di DarkStar e' incisa su una piastra consumata;\n"
	  "lungo, penzolante, scalda appena quando guardi verso l'uscita.\n" },
	{ "collare custode cenere viva", "il collare di cenere viva",
	  "Ceneri rosse si rigenerano tra gli anelli del metallo;\n"
	  "corto, austero, odora di battaglia appena conclusa.\n" },
	{ "amuleto custode cuore spezzato", "l'amuleto del cuore spezzato",
	  "Due meta' di gemma si cercano senza mai unirsi del tutto;\n"
	  "pende fino al petto e vibra quando il custode cade.\n" },
	{ "collare custode cenere viva", "il collare di cenere viva",
	  "Ceneri rosse si rigenerano tra gli anelli del metallo;\n"
	  "corto, austero, odora di battaglia appena conclusa.\n" },
	{ "amuleto custode cuore spezzato", "l'amuleto del cuore spezzato",
	  "Due meta' di gemma si cercano senza mai unirsi del tutto;\n"
	  "pende fino al petto e vibra quando il custode cade.\n" },
	{ "collare eclisse anello nero", "il collare all'anello nero",
	  "Un disco eclittico cinge il collo come un'eclisse fermata nel tempo;\n"
	  "corto, opprimente, assorbe i riflessi della stanza.\n" },
	{ "amuleto eclisse sole prigioniero", "l'amuleto del sole prigioniero",
	  "Un globo dorato e' intrappolato in vetro nero;\n"
	  "lungo, maestoso, sembra contenere l'ultimo giorno della dimensione.\n" },
};

static constexpr ProcRewardNameEntry kProcRewardBodyNames[PROCAREA_TEMPLATE_BANDS] = {
	{ "corazza bruma velo cristallino", "la corazza del velo cristallino",
	  "Piastre sottili di bruma indurita scivolano l'una sull'altra;\n"
	  "proteggono senza rumore, come nebbia trasformata in seta.\n" },
	{ "giubba runa tessuto instabile", "la giubba dal tessuto instabile",
	  "Runi ricamate mutano disposizione ad ogni passo;\n"
	  "il tessuto sembra ricordare colpi che non hai ancora ricevuto.\n" },
	{ "giubba runa tessuto instabile", "la giubba dal tessuto instabile",
	  "Runi ricamate mutano disposizione ad ogni passo;\n"
	  "il tessuto sembra ricordare colpi che non hai ancora ricevuto.\n" },
	{ "brune nebbia argento tessuto", "le brune nebbiose d'argento",
	  "Filamenti argentei attraversano la stoffa come rivoli di nebbia;\n"
	  "al tatto e' freddo, ma sul corpo diventa seconda pelle.\n" },
	{ "brune nebbia argento tessuto", "le brune nebbiose d'argento",
	  "Filamenti argentei attraversano la stoffa come rivoli di nebbia;\n"
	  "al tatto e' freddo, ma sul corpo diventa seconda pelle.\n" },
	{ "corazza varco piastra oscura", "la corazza dalla piastra oscura",
	  "Lamiera nera incisa con la croce obliqua di DarkStar;\n"
	  "assorbe la luce e restituisce solo il tuo riflesso stanco.\n" },
	{ "corazza varco piastra oscura", "la corazza dalla piastra oscura",
	  "Lamiera nera incisa con la croce obliqua di DarkStar;\n"
	  "assorbe la luce e restituisce solo il tuo riflesso stanco.\n" },
	{ "usbergo custode sala finale", "l'usbergo della sala finale",
	  "Frammenti di sigilli fusi nella maglia ricordano l'ultimo guardiano;\n"
	  "e' pesante, ma sembra conoscere gia' dove andrai.\n" },
	{ "usbergo custode sala finale", "l'usbergo della sala finale",
	  "Frammenti di sigilli fusi nella maglia ricordano l'ultimo guardiano;\n"
	  "e' pesante, ma sembra conoscere gia' dove andrai.\n" },
	{ "corazza eclisse carapace vuoto", "la corazza dal carapace vuoto",
	  "Il petto e' protetto da una scocca che non riflette nulla;\n"
	  "dentro, il vuoto dimensionale e' piu' denso dell'aria.\n" },
};

static constexpr ProcRewardNameEntry kProcRewardHeadNames[PROCAREA_TEMPLATE_BANDS] = {
	{ "elmo bruma corona nebbia", "l'elmo a corona di nebbia",
	  "Una corona di vapori solidi cinge la fronte;\n"
	  "non offusca la vista, anzi: la nebbia sembra partire da te.\n" },
	{ "celata runa visiera sussurrante", "la celata dalla visiera sussurrante",
	  "Sulla visiera corrono rune che cambiano ogni volta che distogli lo sguardo;\n"
	  "proteggono gli occhi da luci che non dovrebbero esistere.\n" },
	{ "celata runa visiera sussurrante", "la celata dalla visiera sussurrante",
	  "Sulla visiera corrono rune che cambiano ogni volta che distogli lo sguardo;\n"
	  "proteggono gli occhi da luci che non dovrebbero esistere.\n" },
	{ "bacinetto nebbia cresta argentata", "il bacinetto dalla cresta argentata",
	  "Una cresta di nebbia solida corre lungo il cranio;\n"
	  "brilla come la Piazza delle Nebbie sotto la luna.\n" },
	{ "bacinetto nebbia cresta argentata", "il bacinetto dalla cresta argentata",
	  "Una cresta di nebbia solida corre lungo il cranio;\n"
	  "brilla come la Piazza delle Nebbie sotto la luna.\n" },
	{ "elmo varco maschera oscura", "l'elmo con maschera oscura",
	  "La maschera integrata e' liscia e nera come il varco stesso;\n"
	  "fa sembrare piu' lontano il rumore del mondo reale.\n" },
	{ "elmo varco maschera oscura", "l'elmo con maschera oscura",
	  "La maschera integrata e' liscia e nera come il varco stesso;\n"
	  "fa sembrare piu' lontano il rumore del mondo reale.\n" },
	{ "elmo custode visiera spezzata", "l'elmo dalla visiera spezzata",
	  "Una crepa obliqua attraversa il metallo come un colpo ricordato;\n"
	  "sotto, gli occhi del portatore sembrano piu' antichi.\n" },
	{ "elmo custode visiera spezzata", "l'elmo dalla visiera spezzata",
	  "Una crepa obliqua attraversa il metallo come un colpo ricordato;\n"
	  "sotto, gli occhi del portatore sembrano piu' antichi.\n" },
	{ "corona eclisse sole negato", "la corona del sole negato",
	  "Punte nere orbitano la testa come un'eclisse permanente;\n"
	  "chi la indossa sente il peso del cielo chiuso.\n" },
};

static constexpr ProcRewardNameEntry kProcRewardLegsNames[PROCAREA_TEMPLATE_BANDS] = {
	{ "gambali bruma passo leggero", "i gambali dal passo leggero",
	  "La stoffa e' cosi' leggera che ogni passo sembra sul velo;\n"
	  "non lascia impronte, neanche nella nebbia piu' fitta.\n" },
	{ "schinieri runa passo sigillato", "gli schinieri al passo sigillato",
	  "Rune sui ginocchi si accendono quando corri;\n"
	  "il passo e' muto, come se la dimensione approvasse la tua fretta.\n" },
	{ "schinieri runa passo sigillato", "gli schinieri al passo sigillato",
	  "Rune sui ginocchi si accendono quando corri;\n"
	  "il passo e' muto, come se la dimensione approvasse la tua fretta.\n" },
	{ "gambali nebbia striscia argentata", "i gambali dalla striscia argentata",
	  "Strisce d'argento corrono lungo le gambe come rivoli di bruma;\n"
	  "freddi al primo contatto, poi perfettamente aderenti.\n" },
	{ "gambali nebbia striscia argentata", "i gambali dalla striscia argentata",
	  "Strisce d'argento corrono lungo le gambe come rivoli di bruma;\n"
	  "freddi al primo contatto, poi perfettamente aderenti.\n" },
	{ "schinieri varco passo profondo", "gli schinieri al passo profondo",
	  "Piastre scure coprono ginocchia e stinchi;\n"
	  "ogni passo sembra scendere di un piano in piu' nel varco.\n" },
	{ "schinieri varco passo profondo", "gli schinieri al passo profondo",
	  "Piastre scure coprono ginocchia e stinchi;\n"
	  "ogni passo sembra scendere di un piano in piu' nel varco.\n" },
	{ "gambali custode passo ultimo", "i gambali dell'ultimo passo",
	  "Consumati ai bordi, rinforzati dove serviva al custode;\n"
	  "portano ancora polvere di sala finale.\n" },
	{ "gambali custode passo ultimo", "i gambali dell'ultimo passo",
	  "Consumati ai bordi, rinforzati dove serviva al custode;\n"
	  "portano ancora polvere di sala finale.\n" },
	{ "schinieri eclisse passo negato", "gli schinieri al passo negato",
	  "Il metallo nero frena ogni movimento brusco;\n"
	  "chi li indossa cammina come se il tempo stesso esitasse.\n" },
};

static constexpr ProcRewardNameEntry kProcRewardFeetNames[PROCAREA_TEMPLATE_BANDS] = {
	{ "calzari bruma passo silente", "i calzari dal passo silente",
	  "Suole di nebbia compressa non fanno rumore;\n"
	  "a volte lasciano una scia che svanisce in un battito di ciglia.\n" },
	{ "stivali runa impronta cancellata", "gli stivali dall'impronta cancellata",
	  "Rune sotto la suola cancellano le tracce appena posate;\n"
	  "il cuoio e' morbido come pergamena appena scritta.\n" },
	{ "stivali runa impronta cancellata", "gli stivali dall'impronta cancellata",
	  "Rune sotto la suola cancellano le tracce appena posate;\n"
	  "il cuoio e' morbido come pergamena appena scritta.\n" },
	{ "calzari nebbia suola argentata", "i calzari dalla suola argentata",
	  "L'argento sotto i piedi riflette stanze che non stai ancora vedendo;\n"
	  "leggeri, aderenti, profumano di pioggia lontana.\n" },
	{ "calzari nebbia suola argentata", "i calzari dalla suola argentata",
	  "L'argento sotto i piedi riflette stanze che non stai ancora vedendo;\n"
	  "leggeri, aderenti, profumano di pioggia lontana.\n" },
	{ "stivali varco passo profondo", "gli stivali dal passo profondo",
	  "Talloni rinforzati e suola nera assorbono ogni urto;\n"
	  "camminare con essi sembra scendere scale invisibili.\n" },
	{ "stivali varco passo profondo", "gli stivali dal passo profondo",
	  "Talloni rinforzati e suola nera assorbono ogni urto;\n"
	  "camminare con essi sembra scendere scale invisibili.\n" },
	{ "calzari custode impronta finale", "i calzari dell'impronta finale",
	  "Consumati davanti, intatti dietro: la traccia di chi ha fatto un ultimo giro;\n"
	  "dentro c'e' ancora cenere fine.\n" },
	{ "calzari custode impronta finale", "i calzari dell'impronta finale",
	  "Consumati davanti, intatti dietro: la traccia di chi ha fatto un ultimo giro;\n"
	  "dentro c'e' ancora cenere fine.\n" },
	{ "stivali eclisse passo sospeso", "gli stivali dal passo sospeso",
	  "Per un istante dopo ogni passo il piede sembra non toccare terra;\n"
	  "come se la dimensione non avesse deciso se lasciarti andare.\n" },
};

static constexpr ProcRewardNameEntry kProcRewardHandsNames[PROCAREA_TEMPLATE_BANDS] = {
	{ "guanti bruma presa leggera", "i guanti dalla presa leggera",
	  "Dita avvolte in velo solido: afferrano senza stringere;\n"
	  "perfetti per chi deve aprire sigilli senza romperli.\n" },
	{ "guanti runa dita incise", "i guanti dalle dita incise",
	  "Ogni falange porta un simbolo diverso che si accende al contatto;\n"
	  "la pelle sotto resta al sicuro da rune ostili.\n" },
	{ "guanti runa dita incise", "i guanti dalle dita incise",
	  "Ogni falange porta un simbolo diverso che si accende al contatto;\n"
	  "la pelle sotto resta al sicuro da rune ostili.\n" },
	{ "guanti nebbia palmo argentato", "i guanti dal palmo argentato",
	  "Palmi riflettenti come specchi di nebbia;\n"
	  "le dita scivolano facilmente su metallo bagnato.\n" },
	{ "guanti nebbia palmo argentato", "i guanti dal palmo argentato",
	  "Palmi riflettenti come specchi di nebbia;\n"
	  "le dita scivolano facilmente su metallo bagnato.\n" },
	{ "guanti varco presa oscura", "i guanti dalla presa oscura",
	  "Cuoio nero e rinforzi ai nodi delle dita;\n"
	  "stringono con la fermezza di chi ha gia' aperto un varco.\n" },
	{ "guanti varco presa oscura", "i guanti dalla presa oscura",
	  "Cuoio nero e rinforzi ai nodi delle dita;\n"
	  "stringono con la fermezza di chi ha gia' aperto un varco.\n" },
	{ "guanti custode presa spezzata", "i guanti dalla presa spezzata",
	  "Rinforzi consumati sulle nocche, intatti sul palmo;\n"
	  "hanno retto fino all'ultimo colpo del custode.\n" },
	{ "guanti custode presa spezzata", "i guanti dalla presa spezzata",
	  "Rinforzi consumati sulle nocche, intatti sul palmo;\n"
	  "hanno retto fino all'ultimo colpo del custode.\n" },
	{ "guanti eclisse presa negata", "i guanti dalla presa negata",
	  "Le dita sembrano avvolgere il vuoto piu' che la lama;\n"
	  "chi li indossa afferra con la certezza di chi sfida l'eclisse.\n" },
};

static constexpr ProcRewardNameEntry kProcRewardArmsNames[PROCAREA_TEMPLATE_BANDS] = {
	{ "avambracci bruma rivo gelido", "gli avambracci al rivo gelido",
	  "Strisce di bruma corrono lungo gli avambracci come acqua immobile;\n"
	  "deviano i colpi con un sussurro, non con un clangore.\n" },
	{ "parabracci runa arco inciso", "i parabracci all'arco inciso",
	  "Rune disegnano archi concentrici che pulsano al parare;\n"
	  "il metallo e' sottile ma ostinato come un verso dimenticato.\n" },
	{ "parabracci runa arco inciso", "i parabracci all'arco inciso",
	  "Rune disegnano archi concentrici che pulsano al parare;\n"
	  "il metallo e' sottile ma ostinato come un verso dimenticato.\n" },
	{ "avambracci nebbia lamina argentata", "gli avambracci alla lamina argentata",
	  "Lamine argentee si sovrappongono come nebbia stratificata;\n"
	  "riflettono lame nemiche per un istante fuori tempo.\n" },
	{ "avambracci nebbia lamina argentata", "gli avambracci alla lamina argentata",
	  "Lamine argentee si sovrappongono come nebbia stratificata;\n"
	  "riflettono lame nemiche per un istante fuori tempo.\n" },
	{ "parabracci varco piastra nera", "i parabracci alla piastra nera",
	  "Piastre scure coprono avambraccio e gomito;\n"
	  "profumano di muschio e pietra del tempio nella foresta.\n" },
	{ "parabracci varco piastra nera", "i parabracci alla piastra nera",
	  "Piastre scure coprono avambraccio e gomito;\n"
	  "profumano di muschio e pietra del tempio nella foresta.\n" },
	{ "avambracci custode arco spezzato", "gli avambracci all'arco spezzato",
	  "Una crepa obliqua attraversa il metallo come memoria di un colpo reale;\n"
	  "tengono ancora, per pura testardaggine dimensionale.\n" },
	{ "avambracci custode arco spezzato", "gli avambracci all'arco spezzato",
	  "Una crepa obliqua attraversa il metallo come memoria di un colpo reale;\n"
	  "tengono ancora, per pura testardaggine dimensionale.\n" },
	{ "parabracci eclisse arco negato", "i parabracci all'arco negato",
	  "Il metallo non brilla e non arrugginisce: e' semplicemente assente alla luce;\n"
	  "protegge come un'ombra che ha scelto il tuo braccio.\n" },
};

static constexpr ProcRewardNameEntry kProcRewardAboutNames[PROCAREA_TEMPLATE_BANDS] = {
	{ "mantello bruma velo errante", "il mantello del velo errante",
	  "La stoffa e' nebbia tessuta: si muove anche senza vento;\n"
	  "avvolge le spalle come un ricordo della Piazza lontana.\n" },
	{ "mantello runa bordo sussurrante", "il mantello dal bordo sussurrante",
	  "Il bordo e' ricamato di rune che mutano ad ogni passo;\n"
	  "a volte sembra sussurrare il nome di chi entra dopo di te.\n" },
	{ "mantello runa bordo sussurrante", "il mantello dal bordo sussurrante",
	  "Il bordo e' ricamato di rune che mutano ad ogni passo;\n"
	  "a volte sembra sussurrare il nome di chi entra dopo di te.\n" },
	{ "mantello nebbia coda argentata", "il mantello dalla coda argentata",
	  "Strisce d'argento corrono sulla stoffa come rivoli di bruma;\n"
	  "col movimento lascia scie che svaniscono subito.\n" },
	{ "mantello nebbia coda argentata", "il mantello dalla coda argentata",
	  "Strisce d'argento corrono sulla stoffa come rivoli di bruma;\n"
	  "col movimento lascia scie che svaniscono subito.\n" },
	{ "mantello varco ombra profonda", "il mantello dall'ombra profonda",
	  "Nero assoluto, pesante solo quando qualcuno ti guarda;\n"
	  "nasconde la sagoma come il varco nasconde la strada.\n" },
	{ "mantello varco ombra profonda", "il mantello dall'ombra profonda",
	  "Nero assoluto, pesante solo quando qualcuno ti guarda;\n"
	  "nasconde la sagoma come il varco nasconde la strada.\n" },
	{ "mantello custode coda spezzata", "il mantello dalla coda spezzata",
	  "L'orlo e' bruciato e rammendato con filo di sigillo;\n"
	  "porta ancora l'odore della caduta del custode.\n" },
	{ "mantello custode coda spezzata", "il mantello dalla coda spezzata",
	  "L'orlo e' bruciato e rammendato con filo di sigillo;\n"
	  "porta ancora l'odore della caduta del custode.\n" },
	{ "mantello eclisse coda negata", "il mantello dalla coda negata",
	  "Dietro di te non proietta ombra: la assorbe;\n"
	  "camminare con esso e' camminare dentro un'ombra piu' grande.\n" },
};

static constexpr ProcRewardNameEntry kProcRewardWaistNames[PROCAREA_TEMPLATE_BANDS] = {
	{ "cintura bruma nodo nebbia", "la cintura al nodo di nebbia",
	  "Un nodo di vapori solidi tiene su la cintura;\n"
	  "non si allenta, neanche quando corri nel velo.\n" },
	{ "cintura runa fibbia incisa", "la cintura alla fibbia incisa",
	  "La fibbia e' una runa tridimensionale che gira lentamente;\n"
	  "tiene stretto senza mai strangolare.\n" },
	{ "cintura runa fibbia incisa", "la cintura alla fibbia incisa",
	  "La fibbia e' una runa tridimensionale che gira lentamente;\n"
	  "tiene stretto senza mai strangolare.\n" },
	{ "cintura nebbia catena argentata", "la cintura alla catena argentata",
	  "Maglie sottili d'argento scintillano come bruma al sole;\n"
	  "fredda, precisa, perfetta per chi ama ordine nel caos.\n" },
	{ "cintura nebbia catena argentata", "la cintura alla catena argentata",
	  "Maglie sottili d'argento scintillano come bruma al sole;\n"
	  "fredda, precisa, perfetta per chi ama ordine nel caos.\n" },
	{ "cintura varco fibbia oscura", "la cintura alla fibbia oscura",
	  "Fibbia nera a forma di varco obliquo;\n"
	  "stringe con la fermezza di chi conosce l'uscita e la teme.\n" },
	{ "cintura varco fibbia oscura", "la cintura alla fibbia oscura",
	  "Fibbia nera a forma di varco obliquo;\n"
	  "stringe con la fermezza di chi conosce l'uscita e la teme.\n" },
	{ "cintura custode nodo spezzato", "la cintura al nodo spezzato",
	  "Rammendata piu' volte dove il custode cadde in ginocchio;\n"
	  "regge ancora, per pura abitudine dimensionale.\n" },
	{ "cintura custode nodo spezzato", "la cintura al nodo spezzato",
	  "Rammendata piu' volte dove il custode cadde in ginocchio;\n"
	  "regge ancora, per pura abitudine dimensionale.\n" },
	{ "cintura eclisse fibbia negata", "la cintura alla fibbia negata",
	  "La fibbia non ha buco ne' perno: si chiude da sola;\n"
	  "come se l'eclisse avesse deciso di tenerti dentro un po' piu' a lungo.\n" },
};

static constexpr ProcRewardNameEntry kProcRewardWristNames[PROCAREA_TEMPLATE_BANDS * 2] = {
	{ "bracciale bruma cerchio sinistro", "il bracciale al cerchio sinistro",
	  "Un cerchio opaco al polso sinistro: dentro, nebbia che non esce mai.\n" },
	{ "bracciale bruma spirale destro", "il bracciale a spirale destro",
	  "Una spirale d'argento al polso destro si allarga ad ogni gesto;\n"
	  "poi torna stretta, come se non volesse lasciarti andare.\n" },
	{ "bracciale runa arco sinistro", "il bracciale ad arco sinistro",
	  "Rune minuscole formano un arco sul polso sinistro;\n"
	  "si accendono quando pronunci un incantesimo.\n" },
	{ "bracciale runa spirale destro", "il bracciale a spirale destro",
	  "La spirale runica al polso destro corre verso l'interno;\n"
	  "sembra scritta da chi e' uscito prima di finire la frase.\n" },
	{ "bracciale runa arco sinistro", "il bracciale ad arco sinistro",
	  "Rune minuscole formano un arco sul polso sinistro;\n"
	  "si accendono quando pronunci un incantesimo.\n" },
	{ "bracciale runa spirale destro", "il bracciale a spirale destro",
	  "La spirale runica al polso destro corre verso l'interno;\n"
	  "sembra scritta da chi e' uscito prima di finire la frase.\n" },
	{ "bracciale nebbia luna sinistro", "il bracciale alla luna sinistro",
	  "Una mezzaluna di nebbia solida cinge il polso sinistro;\n"
	  "brilla come la Piazza all'alba.\n" },
	{ "bracciale nebbia stella destro", "il bracciale alla stella destro",
	  "Una stella a otto punte di bruma al polso destro;\n"
	  "non punge, ma guida lo sguardo verso l'uscita.\n" },
	{ "bracciale nebbia luna sinistro", "il bracciale alla luna sinistro",
	  "Una mezzaluna di nebbia solida cinge il polso sinistro;\n"
	  "brilla come la Piazza all'alba.\n" },
	{ "bracciale nebbia stella destro", "il bracciale alla stella destro",
	  "Una stella a otto punte di bruma al polso destro;\n"
	  "non punge, ma guida lo sguardo verso l'uscita.\n" },
	{ "bracciale varco catena sinistro", "il bracciale a catena sinistro",
	  "Tre anelli neri al polso sinistro tintinnano senza suono;\n"
	  "profumano di pietra bagnata e muschio.\n" },
	{ "bracciale varco sigillo destro", "il bracciale al sigillo destro",
	  "Un sigillo obliquo al polso destro si chiude da solo;\n"
	  "come se DarkStar avesse firmato il tuo passaggio.\n" },
	{ "bracciale varco catena sinistro", "il bracciale a catena sinistro",
	  "Tre anelli neri al polso sinistro tintinnano senza suono;\n"
	  "profumano di pietra bagnata e muschio.\n" },
	{ "bracciale varco sigillo destro", "il bracciale al sigillo destro",
	  "Un sigillo obliquo al polso destro si chiude da solo;\n"
	  "come se DarkStar avesse firmato il tuo passaggio.\n" },
	{ "bracciale custode ferro sinistro", "il bracciale di ferro sinistro",
	  "Ferro consumato al polso sinistro, ancora caldo al tatto;\n"
	  "porta la curva dell'ultimo parare del custode.\n" },
	{ "bracciale custode cenere destro", "il bracciale di cenere destro",
	  "Cenere rossa si rigenera sul metallo al polso destro;\n"
	  "non brucia la pelle, ma ricorda la caduta.\n" },
	{ "bracciale custode ferro sinistro", "il bracciale di ferro sinistro",
	  "Ferro consumato al polso sinistro, ancora caldo al tatto;\n"
	  "porta la curva dell'ultimo parare del custode.\n" },
	{ "bracciale custode cenere destro", "il bracciale di cenere destro",
	  "Cenere rossa si rigenera sul metallo al polso destro;\n"
	  "non brucia la pelle, ma ricorda la caduta.\n" },
	{ "bracciale eclisse orbita sinistro", "il bracciale all'orbita sinistro",
	  "Un anello nero orbita il polso sinistro senza toccarlo;\n"
	  "l'eclisse sembra aver scelto quel braccio.\n" },
	{ "bracciale eclisse sole destro", "il bracciale al sole destro",
	  "Un sole miniatura e' intrappolato nel metallo al polso destro;\n"
	  "non scalda, ma rifiuta di spegnersi.\n" },
};

static constexpr ProcRewardNameEntry kProcRewardWeaponNames[PROCAREA_TEMPLATE_BANDS * 3] = {
	{ "lama velo bruma spezzato", "la lama del velo spezzato",
	  "Il filo e' sottile e opaco come nebbia congelata;\n"
	  "taglia senza luce, lasciando solo il sussurro del taglio.\n" },
	{ "stiletto bruma ago nebbia", "lo stiletto ago di nebbia",
	  "Sottile come un ago di bruma: entra dove l'armatura esita;\n"
	  "il manico e' freddo e leggero come un sospiro.\n" },
	{ "mazza bruma nodo contuso", "la mazza al nodo contuso",
	  "Un nodo di nebbia solida capovolge l'equilibrio del colpo;\n"
	  "non lascia tagli, solo il ricordo di essere stato colpito.\n" },
	{ "lama runa verso interrotto", "la lama al verso interrotto",
	  "Rune sul pomo si spengono ad ogni fendente;\n"
	  "la lama sembra tagliare anche cio' che non e' ancora accaduto.\n" },
	{ "stiletto runa punto dimenticato", "lo stiletto al punto dimenticato",
	  "La punta e' un simbolo runico piegato in acciaio;\n"
	  "perfora come un segreto che nessuno vuole udire.\n" },
	{ "mazza runa testa sigillata", "la mazza alla testa sigillata",
	  "La testa e' un sigillo rotondo che non si apre mai;\n"
	  "colpisce con la forza di una runa caduta a terra.\n" },
	{ "lama runa verso interrotto", "la lama al verso interrotto",
	  "Rune sul pomo si spengono ad ogni fendente;\n"
	  "la lama sembra tagliare anche cio' che non e' ancora accaduto.\n" },
	{ "stiletto runa punto dimenticato", "lo stiletto al punto dimenticato",
	  "La punta e' un simbolo runico piegato in acciaio;\n"
	  "perfora come un segreto che nessuno vuole udire.\n" },
	{ "mazza runa testa sigillata", "la mazza alla testa sigillata",
	  "La testa e' un sigillo rotondo che non si apre mai;\n"
	  "colpisce con la forza di una runa caduta a terra.\n" },
	{ "lama nebbia filo argentato", "la lama dal filo argentato",
	  "Il filo riflette la Piazza anche lontano da Myst;\n"
	  "taglia lasciando una scia argentata che svanisce subito.\n" },
	{ "stiletto nebbia stella darkstar", "lo stiletto a stella DarkStar",
	  "La guardia e' una stella a otto punte di nebbia solida;\n"
	  "la punta cerca il cuore con fredda pazienza.\n" },
	{ "mazza nebbia testa lunare", "la mazza dalla testa lunare",
	  "Una mezzaluna di metallo nebbioso capovolge il peso;\n"
	  "colpisce come la marea dentro la dimensione.\n" },
	{ "lama nebbia filo argentato", "la lama dal filo argentato",
	  "Il filo riflette la Piazza anche lontano da Myst;\n"
	  "taglia lasciando una scia argentata che svanisce subito.\n" },
	{ "stiletto nebbia stella darkstar", "lo stiletto a stella DarkStar",
	  "La guardia e' una stella a otto punte di nebbia solida;\n"
	  "la punta cerca il cuore con fredda pazienza.\n" },
	{ "mazza nebbia testa lunare", "la mazza dalla testa lunare",
	  "Una mezzaluna di metallo nebbioso capovolge il peso;\n"
	  "colpisce come la marea dentro la dimensione.\n" },
	{ "lama varco filo oscuro", "la lama dal filo oscuro",
	  "Nera, senza riflesso, incisa con la croce obliqua;\n"
	  "taglia come se aprisse una fessura nel mondo.\n" },
	{ "stiletto varco ago profondo", "lo stiletto ago profondo",
	  "L'ago e' piu' lungo del manico: entra e non chiede permesso;\n"
	  "profuma di tempio nella foresta e pioggia antica.\n" },
	{ "mazza varco testa caduta", "la mazza dalla testa caduta",
	  "Testa squadrata e nera, pesante come una pietra del varco;\n"
	  "ogni colpo sembra chiudere una porta dietro di te.\n" },
	{ "lama varco filo oscuro", "la lama dal filo oscuro",
	  "Nera, senza riflesso, incisa con la croce obliqua;\n"
	  "taglia come se aprisse una fessura nel mondo.\n" },
	{ "stiletto varco ago profondo", "lo stiletto ago profondo",
	  "L'ago e' piu' lungo del manico: entra e non chiede permesso;\n"
	  "profuma di tempio nella foresta e pioggia antica.\n" },
	{ "mazza varco testa caduta", "la mazza dalla testa caduta",
	  "Testa squadrata e nera, pesante come una pietra del varco;\n"
	  "ogni colpo sembra chiudere una porta dietro di te.\n" },
	{ "lama custode filo spezzato", "la lama dal filo spezzato",
	  "Una crepa corre lungo il filo senza renderlo meno tagliente;\n"
	  "e' il ricordo dell'ultima lama del custode.\n" },
	{ "stiletto custode ago finale", "lo stiletto ago finale",
	  "Sottile, consumato, ancora affilato dove serviva al guardiano;\n"
	  "perfora con la precisione di chi non aveva piu' diritto all'errore.\n" },
	{ "mazza custode testa scheggiata", "la mazza dalla testa scheggiata",
	  "La testa e' scheggiata ma bilanciata sul manico;\n"
	  "colpisce con il peso di una sentenza gia' pronunciata.\n" },
	{ "lama custode filo spezzato", "la lama dal filo spezzato",
	  "Una crepa corre lungo il filo senza renderlo meno tagliente;\n"
	  "e' il ricordo dell'ultima lama del custode.\n" },
	{ "stiletto custode ago finale", "lo stiletto ago finale",
	  "Sottile, consumato, ancora affilato dove serviva al guardiano;\n"
	  "perfora con la precisione di chi non aveva piu' diritto all'errore.\n" },
	{ "mazza custode testa scheggiata", "la mazza dalla testa scheggiata",
	  "La testa e' scheggiata ma bilanciata sul manico;\n"
	  "colpisce con il peso di una sentenza gia' pronunciata.\n" },
	{ "lama eclisse filo negato", "la lama dal filo negato",
	  "Il filo non brilla e non arrugginisce: e' assenza affilata;\n"
	  "taglia come l'eclisse taglia il giorno.\n" },
	{ "stiletto eclisse ago prigioniero", "lo stiletto ago prigioniero",
	  "Un sole miniatura e' intrappolato nella guardia;\n"
	  "la punta cerca la carne con fredda determinazione dimensionale.\n" },
	{ "mazza eclisse testa ingoiata", "la mazza dalla testa ingoiata",
	  "La testa e' un disco nero che assorbe l'impatto visivo;\n"
	  "colpisce come la chiusura della dimensione stessa.\n" },
};

static constexpr ProcRewardNameEntry kProcRewardHoldNames[PROCAREA_TEMPLATE_BANDS] = {
	{ "focus bruma sfera opaca", "il focus a sfera opaca",
	  "Una sfera di nebbia compressa vibra nel palmo;\n"
	  "amplifica gli incantesimi come il velo amplifica i sussurri.\n" },
	{ "focus runa prisma instabile", "il focus a prisma instabile",
	  "Facce runiche ruotano senza motore visibile;\n"
	  "ogni faccia ricorda un incantesimo diverso, poi lo dimentica.\n" },
	{ "focus runa prisma instabile", "il focus a prisma instabile",
	  "Facce runiche ruotano senza motore visibile;\n"
	  "ogni faccia ricorda un incantesimo diverso, poi lo dimentica.\n" },
	{ "focus nebbia cristallo darkstar", "il focus a cristallo DarkStar",
	  "Un cristallo a stella cattura la luce e la restituisce argentata;\n"
	  "nel palmo e' freddo come la Piazza all'alba.\n" },
	{ "focus nebbia cristallo darkstar", "il focus a cristallo DarkStar",
	  "Un cristallo a stella cattura la luce e la restituisce argentata;\n"
	  "nel palmo e' freddo come la Piazza all'alba.\n" },
	{ "focus varco sfera profonda", "il focus a sfera profonda",
	  "Nera, lucida, profonda oltre il diametro visibile;\n"
	  "chi la stringe sente il varco tirare leggermente.\n" },
	{ "focus varco sfera profonda", "il focus a sfera profonda",
	  "Nera, lucida, profonda oltre il diametro visibile;\n"
	  "chi la stringe sente il varco tirare leggermente.\n" },
	{ "focus custode scheggia sigillo", "il focus con la scheggia del sigillo",
	  "Un frammento di sigillo della sala finale pulsa nel palmo;\n"
	  "e' caldo, ostinato, rifiuta di spegnersi.\n" },
	{ "focus custode scheggia sigillo", "il focus con la scheggia del sigillo",
	  "Un frammento di sigillo della sala finale pulsa nel palmo;\n"
	  "e' caldo, ostinato, rifiuta di spegnersi.\n" },
	{ "focus eclisse sole catturato", "il focus al sole catturato",
	  "Un sole miniatura e' intrappolato nel vetro nero;\n"
	  "irradia potere come l'ultimo istante prima che la dimensione ceda.\n" },
};

static constexpr ProcRewardNameEntry kProcRewardEarNames[PROCAREA_TEMPLATE_BANDS * 2] = {
	{ "orecchino bruma campana sinistro", "l'orecchino a campana sinistro",
	  "Una campanella di nebbia all'orecchio sinistro tintinna senza vento;\n"
	  "a volte sembra richiamare chi e' ancora nel velo.\n" },
	{ "orecchino bruma goccia destro", "l'orecchino a goccia destro",
	  "Una goccia opaca all'orecchio destro scivola e torna al suo posto;\n"
	  "come bruma che non vuole cadere.\n" },
	{ "orecchino runa spirale sinistro", "l'orecchino a spirale sinistro",
	  "Una spirale runica all'orecchio sinistro ruota lentamente;\n"
	  "sussurra versi che non hai ancora imparato.\n" },
	{ "orecchino runa stella destro", "l'orecchino a stella destro",
	  "Una stella runica all'orecchio destro brilla ad ogni incantesimo lanciato;\n"
	  "poi si spegne, timida.\n" },
	{ "orecchino runa spirale sinistro", "l'orecchino a spirale sinistro",
	  "Una spirale runica all'orecchio sinistro ruota lentamente;\n"
	  "sussurra versi che non hai ancora imparato.\n" },
	{ "orecchino runa stella destro", "l'orecchino a stella destro",
	  "Una stella runica all'orecchio destro brilla ad ogni incantesimo lanciato;\n"
	  "poi si spegne, timida.\n" },
	{ "orecchino nebbia luna sinistro", "l'orecchino alla luna sinistro",
	  "Una mezzaluna d'argento all'orecchio sinistro cattura suoni lontani;\n"
	  "a volte senti la Piazza anche da qui.\n" },
	{ "orecchino nebbia stella destro", "l'orecchino alla stella destro",
	  "Una stella a otto punte all'orecchio destro scintilla come bruma al sole;\n"
	  "leggera, fredda, perfetta.\n" },
	{ "orecchino nebbia luna sinistro", "l'orecchino alla luna sinistro",
	  "Una mezzaluna d'argento all'orecchio sinistro cattura suoni lontani;\n"
	  "a volte senti la Piazza anche da qui.\n" },
	{ "orecchino nebbia stella destro", "l'orecchino alla stella destro",
	  "Una stella a otto punte all'orecchio destro scintilla come bruma al sole;\n"
	  "leggera, fredda, perfetta.\n" },
	{ "orecchino varco uncino sinistro", "l'orecchino all'uncino sinistro",
	  "Un uncino nero all'orecchio sinistro non graffia mai;\n"
	  "profuma di pietra bagnata e muschio antico.\n" },
	{ "orecchino varco sigillo destro", "l'orecchino al sigillo destro",
	  "Un sigillo obliquo all'orecchio destro si chiude da solo;\n"
	  "come se DarkStar avesse firmato anche il tuo ascolto.\n" },
	{ "orecchino varco uncino sinistro", "l'orecchino all'uncino sinistro",
	  "Un uncino nero all'orecchio sinistro non graffia mai;\n"
	  "profuma di pietra bagnata e muschio antico.\n" },
	{ "orecchino varco sigillo destro", "l'orecchino al sigillo destro",
	  "Un sigillo obliquo all'orecchio destro si chiude da solo;\n"
	  "come se DarkStar avesse firmato anche il tuo ascolto.\n" },
	{ "orecchino custode piastra sinistro", "l'orecchino alla piastra sinistro",
	  "Una piastra consumata all'orecchio sinistro vibra quando il custode cade;\n"
	  "porta ancora polvere di sala finale.\n" },
	{ "orecchino custode cenere destro", "l'orecchino di cenere destro",
	  "Cenere rossa si rigenera sul gioiello all'orecchio destro;\n"
	  "non brucia, ma ricorda la caduta.\n" },
	{ "orecchino custode piastra sinistro", "l'orecchino alla piastra sinistro",
	  "Una piastra consumata all'orecchio sinistro vibra quando il custode cade;\n"
	  "porta ancora polvere di sala finale.\n" },
	{ "orecchino custode cenere destro", "l'orecchino di cenere destro",
	  "Cenere rossa si rigenera sul gioiello all'orecchio destro;\n"
	  "non brucia, ma ricorda la caduta.\n" },
	{ "orecchino eclisse disco sinistro", "l'orecchino al disco sinistro",
	  "Un disco nero all'orecchio sinistro assorbe rumori superflui;\n"
	  "ti fa sentire solo cio' che conta nella dimensione.\n" },
	{ "orecchino eclisse sole destro", "l'orecchino al sole destro",
	  "Un sole miniatura all'orecchio destro e' intrappolato nel metallo;\n"
	  "non scalda, ma rifiuta di spegnersi.\n" },
};

static constexpr ProcRewardNameEntry kProcRewardEyesNames[PROCAREA_TEMPLATE_BANDS] = {
	{ "lente bruma velo sottile", "le lenti del velo sottile",
	  "Cristalli opachi adattano la vista alla nebbia fitta;\n"
	  "le stanze sembrano piu' vicine, i pericoli piu' chiari.\n" },
	{ "lente runa doppio verso", "le lenti al doppio verso",
	  "Rune sui vetri si sovrappongono creando doppie immagini utili;\n"
	  "a volte vedi il corridoio che non hai ancora preso.\n" },
	{ "lente runa doppio verso", "le lenti al doppio verso",
	  "Rune sui vetri si sovrappongono creando doppie immagini utili;\n"
	  "a volte vedi il corridoio che non hai ancora preso.\n" },
	{ "lente nebbia stella darkstar", "le lenti a stella DarkStar",
	  "Una stella a otto punte e' incisa su ogni vetro;\n"
	  "la Piazza delle Nebbie appare anche quando chiudi gli occhi.\n" },
	{ "lente nebbia stella darkstar", "le lenti a stella DarkStar",
	  "Una stella a otto punte e' incisa su ogni vetro;\n"
	  "la Piazza delle Nebbie appare anche quando chiudi gli occhi.\n" },
	{ "lente varco doppio buio", "le lenti al doppio buio",
	  "Vetri neri filtrano la luce fino al varco minimo necessario;\n"
	  "il mondo reale sembra piu' lontano del solito.\n" },
	{ "lente varco doppio buio", "le lenti al doppio buio",
	  "Vetri neri filtrano la luce fino al varco minimo necessario;\n"
	  "il mondo reale sembra piu' lontano del solito.\n" },
	{ "lente custode scheggia visus", "le lenti con la scheggia visus",
	  "Una scheggia del sigillo del custode e' incastonata nella montatura;\n"
	  "mostra crepe nell'aria dove passano i colpi.\n" },
	{ "lente custode scheggia visus", "le lenti con la scheggia visus",
	  "Una scheggia del sigillo del custode e' incastonata nella montatura;\n"
	  "mostra crepe nell'aria dove passano i colpi.\n" },
	{ "lente eclisse sole negato", "le lenti al sole negato",
	  "I vetri assorbono ogni bagliore fino a lasciare solo contorni;\n"
	  "come guardare la dimensione durante l'eclisse permanente.\n" },
};

[[nodiscard]] static const ProcRewardNameEntry*
procarea_reward_name_for(const ProcRewardGearSlotDef& def, int variant) {
	switch(def.slot) {
	case ProcRewardGearSlot::Light:
		return &kProcRewardLightNames[variant];
	case ProcRewardGearSlot::Finger:
		return &kProcRewardFingerNames[variant];
	case ProcRewardGearSlot::Neck:
		return &kProcRewardNeckNames[variant];
	case ProcRewardGearSlot::Body:
		return &kProcRewardBodyNames[variant];
	case ProcRewardGearSlot::Head:
		return &kProcRewardHeadNames[variant];
	case ProcRewardGearSlot::Legs:
		return &kProcRewardLegsNames[variant];
	case ProcRewardGearSlot::Feet:
		return &kProcRewardFeetNames[variant];
	case ProcRewardGearSlot::Hands:
		return &kProcRewardHandsNames[variant];
	case ProcRewardGearSlot::Arms:
		return &kProcRewardArmsNames[variant];
	case ProcRewardGearSlot::About:
		return &kProcRewardAboutNames[variant];
	case ProcRewardGearSlot::Waist:
		return &kProcRewardWaistNames[variant];
	case ProcRewardGearSlot::Wrist:
		return &kProcRewardWristNames[variant];
	case ProcRewardGearSlot::Wield:
		return &kProcRewardWeaponNames[variant];
	case ProcRewardGearSlot::Hold:
		return &kProcRewardHoldNames[variant];
	case ProcRewardGearSlot::Ear:
		return &kProcRewardEarNames[variant];
	case ProcRewardGearSlot::Eyes:
		return &kProcRewardEyesNames[variant];
	default:
		return nullptr;
	}
}
