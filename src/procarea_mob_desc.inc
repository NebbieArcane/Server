/*ALARMUD* (Do not remove *ALARMUD*, used to automagically manage these lines
 *ALARMUD* AlarMUD 2.0
 *ALARMUD* See COPYING for licence information
 *ALARMUD*/
/* procarea_mob_desc.inc
 * Testi mob Dimensione Effimera (225 archetipi).
 * Generato da scripts/generate-procarea-mobs.py - non editare a mano.
 *
 * short_title: titolo senza articolo; procarea_spawn.cpp aggiunge il/la/gli/le (minuscoli).
 * long_desc: frase di presenza in stanza (con articolo).
 * look: descrizione al look.
 * agg: suono in stanza (ambient, non combattimento); spawn aggiunge \n\r.
 * sound: suono distant (stanze adiacenti, framing in lontananza); spawn aggiunge \n\r.
 * race: razza fissa per archetipo (e_races in autoenums.hpp).
 *
 * Ordine vettore = indice archetipo: boss 0-54,
 * mob 0-149 (indici 55-204),
 * trap 0-19 (indici 205-224).
 */

#ifndef __PROCAREA_MOB_DESC_INC
#define __PROCAREA_MOB_DESC_INC

struct ProcMobArchetypeText {
	const char* keywords;
	const char* short_title;
	const char* long_desc;
	const char* look;
	const char* agg;
	const char* sound;
	int race;
};

#define MOB_DESC(tier, role, slot, race, kw, st, lg, lk, ag, so) \
	{ kw, st, lg, lk, ag, so, race },

static const ProcMobArchetypeText kProcMobArchetypeTexts[] = {

MOB_DESC(1, boss, 0, RACE_PREDATOR,
	"alfa nebbia effimera",
	"Alfa della Nebbia Effimera",
	"Un predatore alfa emerge dal vapore denso della dimensione.",
	"Zanne irregolari e occhi gialli non tradiscono pieta'.",
	"Senti un ringhio basso filtrare dalla nebbia.",
	"In lontananza un ringhio squarcia il silenzio.")

MOB_DESC(1, boss, 1, RACE_ARACHNID,
	"regina tarantole",
	"Regina delle Tarantole",
	"Una tarantola colossale domina un nido di seta e ossa.",
	"Setole urticanti coprono zampe spesse come tronchi.",
	"Le ragnatele intorno a te vibrano come corde tese.",
	"Da qui vicino odi ragnatele che vibrano.")

MOB_DESC(1, boss, 2, RACE_TREE,
	"guardiano muschio antico",
	"Guardiano di Muschio Antico",
	"Un colosso vegetale blocca la sala con radici animate.",
	"Muschio vivo pulsa come un secondo cuore.",
	"Radici vive scricchiolano sotto i tuoi piedi.",
	"Odi in lontananza radici che scoppiano fuori dalla roccia.")

MOB_DESC(1, boss, 3, RACE_PREDATOR,
	"predatore cieco",
	"Predatore Cieco delle Profondita'",
	"Una bestia senza occhi percepisce ogni tuo movimento.",
	"Artigli lunghi segnano le pareti.",
	"Un urlo cavernoso ti passa a fianco senza eco.",
	"In lontananza risuona un urlo cavernoso.")

MOB_DESC(1, boss, 4, RACE_INSECT,
	"matriarca larve",
	"Matriarca delle Larve",
	"Una montagna gelatinosa regge un trono di larve.",
	"Larve obbedienti si gettano contro gli intrusi.",
	"Schizzi viscidi cadono dal soffitto accanto a te.",
	"Da una sala vicina arriva lo schizzo di qualcosa di viscoso.")

MOB_DESC(1, boss, 5, RACE_BIRD,
	"signore pipistrelli",
	"Signore dei Pipistrelli delle Profondita'",
	"Una creatura alata coronata di ossa volteggia in basso.",
	"Canini sporgono da un muso allungato e famelico.",
	"Ali membranose battono sopra la tua testa.",
	"Odi in lontananza un battito d'ali frenetico.")

MOB_DESC(1, boss, 6, RACE_INSECT,
	"capo saprofagi",
	"Capo dei Saprofagi",
	"Un insettoide putrefatto comanda sciami affamati.",
	"Mandibole gocciolano liquido nero corrosivo.",
	"Un tanfo di carogna ti stringe la gola.",
	"Da qui vicino arriva un odore acre di carogne.")

MOB_DESC(1, boss, 7, RACE_ORC,
	"custode fosso",
	"Custode del Fosso",
	"Un guerriero rannicchiato sorveglia un baratro laterale.",
	"L'armatura e' incrostata di fango secco e sangue.",
	"Pietre scivolano verso un baratro a pochi passi.",
	"In lontananza odi pietre cadere nel vuoto.")

MOB_DESC(1, boss, 8, RACE_INSECT,
	"re larve",
	"Re delle Larve",
	"Un sovrano viscido e' nato dal cumulo di generazioni.",
	"Occhi multipli si aprono e chiudono a ondate.",
	"Un brusio umido e innaturale riempie la stanza.",
	"Odi in lontananza un brusio viscido e continuo.")

MOB_DESC(1, boss, 9, RACE_GHOST,
	"ombra cunicolo",
	"Ombra del Primo Cunicolo",
	"La prima ombra nata nella dimensione si stacca dal muro.",
	"Non ha volto: solo fame antica.",
	"Un freddo senza fonte ti morde le ossa.",
	"Da qui vicino arriva un gelo che fa tremare.")

MOB_DESC(1, boss, 10, RACE_UNDEAD_LICH,
	"arconte catacombe",
	"Arconte delle Catacombe",
	"Un lich avvolto in fasce polverose troneggia su un sepolcro aperto.",
	"Rune necromantiche bruciano come brace nel vuoto degli occhi.",
	"Qualcuno sussurra in lingue morte proprio qui.",
	"In lontananza sussurri di lingue morte si trascinano.")

MOB_DESC(1, boss, 11, RACE_UNDEAD_SPECTRE,
	"signore nazgul caduto",
	"Signore dei Nazgul Caduti",
	"Uno spettro corazzato di nero emerge dalla bruma come un re senza corona.",
	"Un anello spezzato gli pende dal collo spettrale.",
	"Un fischio gelido ti sfiora tra le pietre.",
	"Odi in lontananza un fischio gelido tra le rocce.")

MOB_DESC(1, boss, 12, RACE_ARACHNID,
	"madre ragni ungol",
	"Madre dei Ragni di Ungol",
	"Un'aracnide titanica attende nel buio filando seta nera.",
	"Occhi multipli brillano come stelle malevole.",
	"Ragnatele nere si tendono d'improvviso intorno a te.",
	"Da una caverna vicina arriva lo stridio di seta tesa.")

MOB_DESC(1, boss, 13, RACE_GOBLIN,
	"capobanda goblin profondita",
	"Capobanda Goblin delle Profondita'",
	"Un goblin corazzato con ferri di recupero brandisce un'ascia dentata.",
	"Orecchie mozzate penzolano dalla sua cintura.",
	"Risate acute e feroci ti circondano.",
	"In lontananza echeggiano risate goblin acute.")

MOB_DESC(1, boss, 14, RACE_TROLL,
	"signore troll pietra",
	"Signore dei Troll di Pietra",
	"Un troll massiccio si rialza da una pozza di sangue coagulato.",
	"La pelle sembra granito vivo; le ferite si richiudono da sole.",
	"Pietre rotolano nel buio a pochi passi da te.",
	"Odi in lontananza massi che rotolano nel buio.")

MOB_DESC(1, boss, 15, RACE_UNDEAD_WIGHT,
	"custode tumuli",
	"Custode dei Tumuli",
	"Un guerriero sepolcrale sorge da un tumulo di ossa antiche.",
	"Spada arrugginita e corona spezzata lo contraddistinguono.",
	"Un lamento di tombe profonde ti passa vicino.",
	"In lontananza un lamento sepolcrale si allunga.")

MOB_DESC(1, boss, 16, RACE_DRAGON_GREEN,
	"drago caverne giade",
	"Drago di Giada delle Caverne",
	"Un drago giovane ma feroce riempie la caverna di vapore acido.",
	"Scaglie verdastre raccolgono condensa velenosa.",
	"Un ruggito fa tremare la volta sopra di te.",
	"Da qui vicino arriva un ruggito che scuote la roccia.")

MOB_DESC(1, boss, 17, RACE_BIRD,
	"matriarca arpie rupi",
	"Matriarca delle Arpie delle Rupi",
	"Un'arpia regale plana su artigli insanguinati.",
	"Piume nere e occhi d'ambra la rendono un incubo alato.",
	"Un grido straziante ti squarcia le orecchie.",
	"Odi in lontananza un grido d'arpia straziante.")

MOB_DESC(1, boss, 18, RACE_ORC,
	"signore uruk profondi",
	"Signore degli Uruk Profondi",
	"Un capitano uruk brandisce una lama larga come un'ascia.",
	"Rune di guerra bruciano sulla sua pelle grigio-ferrea.",
	"Cori di guerra sotterranei rimbombano nella sala.",
	"In lontananza risuonano cori di guerra orcheschi.")

MOB_DESC(1, boss, 19, RACE_TREE,
	"ent corrotto antico",
	"Antico Ent Corrotto",
	"Un ent contorto dalla bruma torce rami come mazze.",
	"Corteccia nera stilla linfa velenosa.",
	"Legno vivo scricchiola in modo minaccioso qui vicino.",
	"Da qui vicino arriva lo scricchiolio di rami corrotti.")

MOB_DESC(1, boss, 20, RACE_DEMON,
	"demone fessure",
	"Demone delle Fessure",
	"Un demone cornuto striscia fuori da una crepa nel muro.",
	"Zolfo e cenere aleggiano intorno a lui.",
	"Un odore di zolfo bruciacchiato ti prende alla gola.",
	"In lontananza aleggia un tanfo di zolfo.")

MOB_DESC(1, boss, 21, RACE_UNDEAD_VAMPIRE,
	"signore vampiro bruma",
	"Signore dei Vampiri della Bruma",
	"Un vampiro aristocratico emerge dalla nebbia con mantello lacero.",
	"Canini affilati e occhi cremisi non tradiscono l'eta'.",
	"Un sussurro seducente e mortale ti sfiora l'orecchio.",
	"Odi in lontananza un sussurro seducente e gelido.")

MOB_DESC(1, boss, 22, RACE_GIANT_HILL,
	"gigante radici",
	"Gigante delle Radici",
	"Un gigante delle colline blocca il passaggio con un tronco nodoso.",
	"Radici gli crescono dalle caviglie fino al ginocchio.",
	"Passi enormi fanno tremare il pavimento sotto di te.",
	"In lontananza passi colossali scuotono la roccia.")

MOB_DESC(1, boss, 23, RACE_ELEMENT,
	"signore elementale fango",
	"Signore degli Elementali del Fango",
	"Una massa di fango senziente assume forma umanoide.",
	"Pietre e ossa galleggiano nella sua corrente viscida.",
	"Schizzi di fango caldo ti schizzano addosso.",
	"Da una sala vicina arriva lo schizzo di fango vivo.")

MOB_DESC(1, boss, 24, RACE_DARK_ELF,
	"regina rocca nera",
	"Regina della Rocca Nera",
	"Una drow incappucciata troneggia su un trono di stalattiti.",
	"Lame di ossidiana le scintillano attorno come aureola.",
	"Incantesimi sibilati ti girano intorno nel buio.",
	"In lontananza odi formule sibilate nel buio.")

MOB_DESC(1, boss, 25, RACE_DEVIL,
	"signore culto abissale",
	"Signore del Culto Abissale",
	"Un diavolo cornuto conduce un coro di fedeli mutati.",
	"Simboli infernali bruciano sulla pietra circostante.",
	"Canti distorti e irreali riempiono l'aria qui.",
	"Odi in lontananza canti cultisti distorti.")

MOB_DESC(1, boss, 26, RACE_GHOST,
	"oracolo corrotto moria",
	"Oracolo Corrotto di Moria",
	"Lo spirito di un saggio nano urla profezie nel buio.",
	"Libri fluttuano attorno a lui come lune orbitanti.",
	"Echi di martelli antichi ti vibrano nelle ossa.",
	"In lontananza echi di martelli battono nella roccia.")

MOB_DESC(1, boss, 27, RACE_SPECIAL,
	"signore mimiche",
	"Signore delle Mimiche",
	"Una creatura che imita un trono e un tesoro attende immobile.",
	"La pelle sembra metallo finche' non si apre a bocca piena.",
	"Un tonfo sordo di carne che diventa pietra ti fa sussultare.",
	"Da qui vicino arriva un tonfo sordo e innaturale.")

MOB_DESC(1, boss, 28, RACE_DEMON,
	"balrog minore",
	"Balrog Minore",
	"Una creatura di fuoco e ombra brandisce una frusta di fiamme.",
	"Ali carbonizzate spalancano calore insopportabile.",
	"Un ruggito di magma sotterraneo ti scuote.",
	"In lontananza un ruggito di magma ribolle.")

MOB_DESC(1, boss, 29, RACE_UNDEAD_GHAST,
	"sovrano ghoul ossa",
	"Sovrano dei Ghoul delle Ossa",
	"Un ghoul reale si rialza da un trono di femori intrecciati.",
	"Artigli neri e lingua biforcuta gocciolano ichor.",
	"Ossa scricchiolano nel buio a un passo da te.",
	"Odi in lontananza ossa che scricchiolano.")

MOB_DESC(1, boss, 30, RACE_DRAGON_RED,
	"drago rosso antico",
	"Drago Rosso Antico",
	"Un drago rosso antico riempie la sala di calore soffocante.",
	"Scaglie come metallo incandescente e occhi di magma.",
	"Il suolo trema sotto di te tra bagliori di fiamma.",
	"In lontananza il suolo trema tra echi di fuoco.")

MOB_DESC(1, boss, 31, RACE_DRAGON_BLACK,
	"drago nero corrotto",
	"Drago Nero Corrotto",
	"Un drago nero corrotto dalla bruma striscia fuori dall'oscurita'.",
	"Il suo alito acido corrode la pietra circostante.",
	"Un ruggito nero squarcia la roccia intorno a te.",
	"Odi in lontananza un ruggito che spacca la pietra.")

MOB_DESC(1, boss, 32, RACE_DRAGON_WHITE,
	"drago bianco gelido",
	"Drago Bianco Gelido",
	"Un drago bianco scende da vortici di neve eterna.",
	"Ghiaccio si forma su ogni superficie al suo passaggio.",
	"Un vento gelido ti taglia il volto.",
	"Da qui vicino arriva un vento gelido tagliente.")

MOB_DESC(1, boss, 33, RACE_DRAGON_GOLD,
	"drago dorato caduto",
	"Drago Dorato Caduto",
	"Un drago dorato decaduto troneggia su tesori illusori.",
	"Scaglie opache nascondono ancora potere immenso.",
	"Un ruggito metallico vibra nella stanza.",
	"In lontananza risuona un ruggito metallico.")

MOB_DESC(1, boss, 34, RACE_DRAGON_GREEN,
	"signore drago verde",
	"Signore dei Draghi Verdi",
	"Un drago verde sovrano avvolge la caverna di gas velenoso.",
	"Ali spalancate oscurano torce e speranza.",
	"Un alito acido ti brucia le narici.",
	"Odi in lontananza un vapore corrosivo che sibila.")

MOB_DESC(1, boss, 35, RACE_UNDEAD_SPECTRE,
	"re nazgul bruma",
	"Re dei Nazgul della Bruma",
	"Un re spettrale in armatura nera alza una lama senza luce.",
	"Corona spezzata e terrore antico precedono ogni passo.",
	"Un grido ti gela il sangue sulle spalle.",
	"In lontananza un grido gela l'aria.")

MOB_DESC(1, boss, 36, RACE_UNDEAD_SPECTRE,
	"signore anello spezzato",
	"Signore dell'Anello Spezzato",
	"Uno spettro corazzato porta un anello spezzato al dito.",
	"La sua sola presenza spegne le fiamme.",
	"Un sussurro di obbedienza ti striscia nella mente.",
	"Da qui vicino arriva un sussurro di cieca obbedienza.")

MOB_DESC(1, boss, 37, RACE_UNDEAD_LICH,
	"lich re ossario",
	"Re dei Lich dell'Ossuario",
	"Un lich reale troneggia su un trono di teschi impilati.",
	"La corona di ossa e il bastone runico pulsano di oscurita'.",
	"Incantesimi mortali gelano l'aria intorno a te.",
	"Odi in lontananza formule che gelano l'anima.")

MOB_DESC(1, boss, 38, RACE_DEMON,
	"principe demone abisso",
	"Principe dei Demoni dell'Abisso",
	"Un principe demone emerge da una fessura ardente.",
	"Ali carbonizzate e zolfo riempiono la stanza.",
	"Odore di zolfo e paura ti stringe il petto.",
	"In lontananza aleggia un tanfo di zolfo e terrore.")

MOB_DESC(1, boss, 39, RACE_DEVIL,
	"arconte infernale",
	"Arconte degli Inferi",
	"Un arconte infernale conduce un coro di dannati.",
	"Contratti scritti con sangue fluttuano attorno a lui.",
	"Canti profani ti circondano come fumo.",
	"Odi in lontananza canti profani e lenti.")

MOB_DESC(1, boss, 40, RACE_GIANT_STORM,
	"gigante tempesta",
	"Gigante della Tempesta",
	"Un gigante della tempesta scuote la volta con ogni passo.",
	"Fulmini danzano tra le sue dita massicce.",
	"Un tuono sordo continuo vibra sotto i tuoi piedi.",
	"In lontananza un tuono sordo non cessa.")

MOB_DESC(1, boss, 41, RACE_GIANT_FIRE,
	"gigante fuoco",
	"Gigante del Fuoco",
	"Un gigante del fuoco arde come un vulcano ambulante.",
	"Pietra fusa gocciola dai suoi pugni.",
	"Un calore insopportabile ti investe la pelle.",
	"Da qui vicino arriva un calore che soffoca.")

MOB_DESC(1, boss, 42, RACE_GIANT_FROST,
	"gigante gelo",
	"Gigante del Gelo",
	"Un gigante del gelo congela l'aria con un solo sguardo.",
	"Barba di ghiaccio e occhi come iceberg.",
	"Ghiaccio scricchiola e si spezza vicino a te.",
	"Odi in lontananza ghiaccio che scricchiola.")

MOB_DESC(1, boss, 43, RACE_MFLAYER,
	"arconte illithid",
	"Arconte degli Illithid",
	"Un arconte illithid fluttua con tentacoli regali.",
	"Menti deboli collassano al suo sguardo.",
	"Pulsazioni psichiche ti premono dietro gli occhi.",
	"In lontananza senti pulsazioni mentali disturbanti.")

MOB_DESC(1, boss, 44, RACE_PLANAR,
	"signore abisso stellare",
	"Signore dell'Abisso Stellare",
	"Una creatura planare distorce lo spazio attorno a se'.",
	"Geometrie impossibili orbitano nel suo velo.",
	"Echi impossibili si spezzano e si ricompongono qui.",
	"Odi in lontananza echi che non dovrebbero esistere.")

MOB_DESC(1, boss, 45, RACE_UNDEAD_VAMPIRE,
	"gran signore vampiro",
	"Gran Signore dei Vampiri",
	"Un antico signore dei vampiri sorge da un sarcofago di marmo nero.",
	"Potere antico e sete insaziabile lo avvolgono.",
	"Un sussurro ipnotico ti avvince la mente.",
	"Da qui vicino arriva un sussurro che ipnotizza.")

MOB_DESC(1, boss, 46, RACE_ORC,
	"signore uruk nero",
	"Signore degli Uruk Neri",
	"Un capitano uruk nero brandisce una spada enorme.",
	"Stendardo nero e occhi come brace.",
	"Cori di guerra nera rimbombano nella sala.",
	"In lontananza cori di guerra uruk echeggiano.")

MOB_DESC(1, boss, 47, RACE_TROLL,
	"signore troll montagna",
	"Signore dei Troll di Montagna",
	"Un troll delle montagne imponente riempie l'intera sala.",
	"Ogni passo fa crollare stalattiti.",
	"Un ruggito tellurico ti scuote le ginocchia.",
	"Odi in lontananza un ruggito che scuote la montagna.")

MOB_DESC(1, boss, 48, RACE_ARACHNID,
	"regina ragni antica",
	"Regina dei Ragni Antichi",
	"Un'antica regina dei ragni tesse un trono di seta e ossa.",
	"Prole infinita si muove nelle ombre.",
	"Ragnatele antiche ti sfiorano il volto.",
	"Da una caverna vicina arriva lo stridio di ragnatele.")

MOB_DESC(1, boss, 49, RACE_TREE,
	"signore ent corrotto",
	"Signore degli Ent Corrotti",
	"Un ent corrotto possente torce rami come torri.",
	"Linfa nera stilla dal tronco.",
	"Legno corrotto urla e si spezza vicino a te.",
	"In lontananza odi legno che urla sotto tensione.")

MOB_DESC(1, boss, 50, RACE_ELEMENT,
	"signore elementale tempesta",
	"Signore degli Elementali della Tempesta",
	"Un elementale della tempesta sovrano riempie la sala di vento.",
	"Fulmini e pioggia obbediscono al suo gesto.",
	"Un tuono continuo riempie la stanza.",
	"Odi in lontananza un tuono di tempesta ininterrotto.")

MOB_DESC(1, boss, 51, RACE_DARK_ELF,
	"matrona drow",
	"Matrona Drow",
	"Una matrona drow troneggia su un altare di ossidiana.",
	"Ragni sacri le tessono il manto.",
	"Incantesimi drow sibilano nell'aria intorno a te.",
	"In lontananza formule elfiche nere sibilano.")

MOB_DESC(1, boss, 52, RACE_GHOST,
	"re spettro corone",
	"Re degli Spettri delle Corone Perdute",
	"Un re spettro delle corone perdute fluttua sopra un trono.",
	"Corone spezzate orbitano come lune morte.",
	"Lamenti regali e gelidi ti passano vicino.",
	"Odi in lontananza lamenti di corone perdute.")

MOB_DESC(1, boss, 53, RACE_DEMON,
	"balrog antico",
	"Balrog Antico",
	"Un balrog antico sorge tra fiamme e ombra.",
	"Frusta infuocata e spada di fuoco squarciano il buio.",
	"Un ruggito di magma antico ti riempie le orecchie.",
	"In lontananza un ruggito di magma ribolle.")

MOB_DESC(1, boss, 54, RACE_DRAGON_SILVER,
	"drago argento lunare",
	"Drago d'Argento Lunare",
	"Un drago d'argento riflette una luce lunare impossibile.",
	"Ali argentee e occhi pieni di marea.",
	"Una luce argentea e accecante ti ferisce gli occhi.",
	"Da qui vicino arriva un lampo d'argento lunare.")

MOB_DESC(1, mob, 0, RACE_GOBLIN,
	"goblin esploratore",
	"Goblin Esploratore",
	"Un goblin magro sbuca da un crepaccio con pugnale arrugginito.",
	"Orecchie mozzate gli ornano il collo.",
	"Senti un goblin borbottare e grattarsi l'armatura.",
	"Odi in lontananza un urlo goblin acuto.")

MOB_DESC(1, mob, 1, RACE_GOBLIN,
	"goblin sciamano",
	"Goblin Sciamano",
	"Un goblin coperto di rune brandisce un bastone di ossa.",
	"Occhi gialli brillano di follia sciamanica.",
	"Mormori gutturali di uno sciamano ti riempiono le orecchie.",
	"Da qui vicino arriva un mormorio gutturale.")

MOB_DESC(1, mob, 2, RACE_GOBLIN,
	"goblin arciere",
	"Goblin Arciere",
	"Un goblin armato di arco corto mira dall'ombra.",
	"Frecce avvelenate spuntano dalla faretra.",
	"Una corda di arco vibra piano nella stanza.",
	"In lontananza odi frecce che fischiano nel buio.")

MOB_DESC(1, mob, 3, RACE_GOBLIN,
	"goblin speleologo",
	"Goblin Speleologo",
	"Un goblin con lanterna di tartaruga esplora i cunicoli.",
	"Corde e uncini gli pendono dalla cintura.",
	"Metallo striscia sulla roccia a un passo da te.",
	"Odi in lontananza uncini che graffiano la pietra.")

MOB_DESC(1, mob, 4, RACE_GOBLIN,
	"goblin ladro",
	"Goblin Ladro",
	"Un goblin furtivo fruga tra i detriti con occhi furbi.",
	"Dita nere e veloci non lasciano traccia.",
	"Risate soffocate ti girano intorno alle spalle.",
	"Da qui vicino arriva una risata goblin soffocata.")

MOB_DESC(1, mob, 5, RACE_GOBLIN,
	"hobgoblin soldato",
	"Hobgoblin Soldato",
	"Un hobgoblin in armatura di cuoio brandisce una spada e uno scudo.",
	"Disciplina crudele si legge nei suoi occhi rossi.",
	"Passi marziali pesanti battono il pavimento.",
	"In lontananza odi passi marziali hobgoblin.")

MOB_DESC(1, mob, 6, RACE_GOBLIN,
	"hobgoblin capitano",
	"Hobgoblin Capitano",
	"Un hobgoblin corazzato conduce una banda di sottoposti.",
	"Stendardo strappato ancora gli cinge il petto.",
	"Comandi ruvidi rimbombano nella sala.",
	"Odi in lontananza ordini ruvidi e secchi.")

MOB_DESC(1, mob, 7, RACE_ORC,
	"orco guerriero",
	"Orco Guerriero",
	"Un orco muscoloso ringhia mostrando zanne aguzze.",
	"Cicatrici rituali solcano il petto nudo.",
	"Grugniti di guerra orcheschi ti passano vicino.",
	"In lontananza risuonano grugniti di guerra.")

MOB_DESC(1, mob, 8, RACE_ORC,
	"orco berserker",
	"Orco Berserker",
	"Un orco reso folle dalla battaglia avanza ringhiando.",
	"Ascia a due mani sollevata, occhi fuori orbita.",
	"Un ansimare furioso e irrequieto riempie l'aria.",
	"Da qui vicino arriva un'urla di sangue orchesca.")

MOB_DESC(1, mob, 9, RACE_HALF_ORC,
	"uruk corridore",
	"Uruk Corridore",
	"Un uruk corre senza stancarsi lungo i corridoi.",
	"Elmo nero e lama larga lo rendono un'ombra letale.",
	"Passi pesanti e rapidi ti sfiorano di fianco.",
	"Odi in lontananza passi uruk pesanti e veloci.")

MOB_DESC(1, mob, 10, RACE_ORC,
	"orco sciamano",
	"Orco Sciamano",
	"Un orco coperto di ossa invoca spiriti della caverna.",
	"Tatuaggi neri pulsano quando canta.",
	"Canti gutturali di uno sciamano ti circondano.",
	"In lontananza canti orcheschi gutturali echeggiano.")

MOB_DESC(1, mob, 11, RACE_ORC,
	"orco guardia",
	"Orco Guardia",
	"Un orco armato sorveglia un varco stretto.",
	"Lancia arrugginita pronta a trafiggere.",
	"Un grugnito di sfida ti arriva da vicino.",
	"Da qui vicino arriva un grugnito di guardia orchesca.")

MOB_DESC(1, mob, 12, RACE_ORC,
	"orco spezzaneve",
	"Orco Spezzaneve",
	"Un orco delle montagne emerge dal ghiaccio sotterraneo.",
	"Pelliccia imbrattata di sangue e brina.",
	"Un vento gelido ti investe dalla parte dell'orco.",
	"Odi in lontananza un vento gelido che fischia.")

MOB_DESC(1, mob, 13, RACE_HALF_ORC,
	"mezzorco reietto",
	"Mezzorco Reietto",
	"Un mezzorco emarginato combatte con disperazione.",
	"Occhi tristi e pugno di ferro.",
	"Un ringhio sommesso di reietto ti fa alzare lo sguardo.",
	"In lontananza un ringhio sommesso si trascina.")

MOB_DESC(1, mob, 14, RACE_GOBLIN,
	"goblin furtivo",
	"Goblin Furtivo",
	"Un goblin si mimetizza tra le stalattiti.",
	"Pelle grigia lo confonde con la roccia.",
	"Pietre cadono dal soffitto a pochi passi.",
	"Da qui vicino arriva il tonfo di pietre che cadono.")

MOB_DESC(1, mob, 15, RACE_TROGMAN,
	"troglodita guerriero",
	"Troglodita Guerriero",
	"Un troglodita brandisce una clava d'osso.",
	"Pelle pallida e occhi ciechi al sole.",
	"Grugniti cavernosi di un troglodita riempiono la sala.",
	"Odi in lontananza grugniti cavernosi.")

MOB_DESC(1, mob, 16, RACE_TROGMAN,
	"troglodita cacciatore",
	"Troglodita Cacciatore",
	"Un troglodita cacciatore trascina una lancia di selce.",
	"Odore di muschio e carne cruda.",
	"Passi silenziosi di cacciatore ti sfiorano.",
	"Ti pare di scorgere passi silenziosi oltre il buio.")

MOB_DESC(1, mob, 17, RACE_ORC,
	"orco flagellatore",
	"Orco Flagellatore",
	"Un orco sadico brandisce una frusta chiodata.",
	"Sorriso crudele sotto elmo deformato.",
	"Uno scroscio di frusta fende l'aria vicino a te.",
	"In lontananza odi lo scroscio di una frusta.")

MOB_DESC(1, mob, 18, RACE_GOBLIN,
	"goblin alchimista",
	"Goblin Alchimista",
	"Un goblin maneggia fiale fumanti con cautela.",
	"Meta' del volto e' bruciata da esplosioni recenti.",
	"Vetro si rompe e acido gocciola qui vicino.",
	"Da qui vicino arriva il fragore di vetro che si spezza.")

MOB_DESC(1, mob, 19, RACE_HALF_ORC,
	"uruk sentinella",
	"Uruk Sentinella",
	"Un uruk immobile veglia su un ponte di pietra.",
	"Lancia lunga e scudo nero coprono ogni angolo.",
	"L'armatura di una sentinella stride a ogni passo.",
	"Odi in lontananza armatura uruk che stride.")

MOB_DESC(1, mob, 20, RACE_UNDEAD_SKELETON,
	"scheletro guerriero",
	"Scheletro Guerriero",
	"Uno scheletro armato si rialza dal cumulo di ossa.",
	"Costole vuote battono come tamburi.",
	"Ossa sbattono l'una contro l'altra nella stanza.",
	"In lontananza ossa di scheletro sbattono nel buio.")

MOB_DESC(1, mob, 21, RACE_UNDEAD_SKELETON,
	"scheletro arciere",
	"Scheletro Arciere",
	"Uno scheletro tende un arco di ossa e tendini.",
	"Frecce di metallo arrugginito puntano verso di te.",
	"Una corda d'osso vibra mentre una freccia viene tesa.",
	"Da qui vicino arriva il fischio di frecce nel buio.")

MOB_DESC(1, mob, 22, RACE_UNDEAD_ZOMBIE,
	"zombie contadino",
	"Zombie Contadino",
	"Un cadavere rigido avanza con braccia tese.",
	"Vestiti laceri e occhi vitrei.",
	"Senti gemiti soffocati.",
	"Odi in lontananza gemiti soffocati.")

MOB_DESC(1, mob, 23, RACE_UNDEAD_ZOMBIE,
	"zombie soldato",
	"Zombie Soldato",
	"Un soldato morto marcia ancora con spada in mano.",
	"Armatura ammaccata copre carne putrefatta.",
	"L'armatura dello zombie stride a ogni passo.",
	"Da qui vicino arriva lo stridio di un'armatura.")

MOB_DESC(1, mob, 24, RACE_UNDEAD_GHOUL,
	"ghoul famelico",
	"Ghoul Famelico",
	"Un ghoul striscia a quattro zampe verso di te.",
	"Artigli neri e lingua biforcuta.",
	"Odi una masticazione umida e famelica.",
	"In lontananza risuona una masticazione di carne.")

MOB_DESC(1, mob, 25, RACE_UNDEAD_GHOUL,
	"ghoul necrofago",
	"Ghoul Necrofago",
	"Un ghoul rovista tra tombe aperte.",
	"Denti affilati gocciolano ichor nero.",
	"Un odore di sepolcro ti prende alla gola.",
	"Odi in lontananza un tanfo di sepolcro.")

MOB_DESC(1, mob, 26, RACE_UNDEAD_WIGHT,
	"wight guerriero",
	"Wight Guerriero",
	"Un wight spettrale brandisce una spada gelida.",
	"Aura di freddo mortale lo circonda.",
	"Un freddo che penetra le ossa ti avvolge.",
	"Da qui vicino arriva un gelo che morde le ossa.")

MOB_DESC(1, mob, 27, RACE_UNDEAD_SPECTRE,
	"spettro errante",
	"Spettro Errante",
	"Uno spettro fluttua attraverso le pareti.",
	"Voce sussurrante promette morte.",
	"Sussurri gelidi ti passano attraverso.",
	"In lontananza sussurri gelidi si trascinano.")

MOB_DESC(1, mob, 28, RACE_GHOST,
	"fantasma prigioniero",
	"Fantasma Prigioniero",
	"Un fantasma incatenato alla caverna urla senza voce.",
	"Catene spettrali gli legano polsi e caviglie.",
	"Lamenti ovattati di un prigioniero ti circondano.",
	"Odi in lontananza lamenti ovattati.")

MOB_DESC(1, mob, 29, RACE_UNDEAD_LICH,
	"lich minore",
	"Lich Minore",
	"Un lich incompleto alza mani scheletriche.",
	"Pergamene bruciano attorno a lui.",
	"Incantesimi sibilati di un lich minore riempiono l'aria.",
	"Da qui vicino arriva lo sibilo di formule morte.")

MOB_DESC(1, mob, 30, RACE_UNDEAD_SKELETON,
	"scheletro guardia",
	"Scheletro Guardia",
	"Uno scheletro in armatura sorveglia un arco.",
	"Elmo vuoto fissa il vuoto con occhi di brace.",
	"Spade d'osso si strofinano l'una contro l'altra.",
	"In lontananza odi lame di scheletro che sbattono.")

MOB_DESC(1, mob, 31, RACE_UNDEAD_ZOMBIE,
	"ogre zombie",
	"Ogre Zombie",
	"Un ogre morto si trascina con mazza imponente.",
	"Carne verde e gonfia sporge dalle ferite.",
	"Passi pesanti da morto fanno tremare la sala.",
	"Odi in lontananza passi pesanti di un cadavere enorme.")

MOB_DESC(1, mob, 32, RACE_UNDEAD_GHOUL,
	"ghoul delle profondita",
	"Ghoul delle Profondita'",
	"Un ghoul emaciato striscia dalle fessure.",
	"Occhi gialli brillano nel buio.",
	"Ringhi famelici ti arrivano da ogni lato.",
	"In lontananza ringhi famelici di ghoul echeggiano.")

MOB_DESC(1, mob, 33, RACE_UNDEAD_WIGHT,
	"wight sepolcrale",
	"Wight Sepolcrale",
	"Un wight sorge da un sarcofago rotto.",
	"Polsi scheletrici stringono una corona.",
	"Un vento gelido dal sepolcro ti investe.",
	"Da qui vicino arriva un vento gelido sepolcrale.")

MOB_DESC(1, mob, 34, RACE_UNDEAD_SPECTRE,
	"spettro guerriero",
	"Spettro Guerriero",
	"Uno spettro in armatura antica fluttua in modo minaccioso.",
	"Spada spettrale attraversa la roccia.",
	"Lame invisibili fischiano nel vuoto vicino a te.",
	"Odi in lontananza lame che fischiano nel vuoto.")

MOB_DESC(1, mob, 35, RACE_GHOST,
	"anima perduta",
	"Anima Perduta",
	"Un'anima perduta vaga senza meta tra i corridoi.",
	"Luce pallida la rende quasi trasparente.",
	"Un pianto sottile e senza corpo ti sfiora.",
	"In lontananza odi un pianto lontano e vuoto.")

MOB_DESC(1, mob, 36, RACE_UNDEAD_VAMPIRE,
	"vampiro servitore",
	"Vampiro Servitore",
	"Un vampiro giovane serve un padrone piu' antico.",
	"Mantello nero e occhi affamati.",
	"Un sussurro seducente ti sfiora l'orecchio.",
	"Da qui vicino arriva un sussurro vampiro seducente.")

MOB_DESC(1, mob, 37, RACE_UNDEAD_SKELETON,
	"scheletro chierico",
	"Scheletro Chierico",
	"Uno scheletro in tonaca invoca l'oscurita'.",
	"Simboli sacri corrotti brillano sul petto.",
	"Preghiere invertite e sacre a rovescio riempiono la stanza.",
	"Odi in lontananza preghiere invertite.")

MOB_DESC(1, mob, 38, RACE_UNDEAD_ZOMBIE,
	"zombie paludoso",
	"Zombie Paludoso",
	"Un cadavere paludoso emerge da pozze viscide.",
	"Alghe gli pendono dalle ferite.",
	"Schizzi di acqua fetida ti bagnano i piedi.",
	"In lontananza odi schizzi di acqua paludosa.")

MOB_DESC(1, mob, 39, RACE_UNDEAD_GHAST,
	"ghast affamato",
	"Ghast Affamato",
	"Un ghast piu' potente di un ghoul striscia verso di te.",
	"Alito putrefatto precede l'attacco.",
	"Un odore insopportabile ti fa venire il voltastomaco.",
	"Da qui vicino arriva un tanfo nauseabondo.")

MOB_DESC(1, mob, 40, RACE_ARACHNID,
	"ragno caverna",
	"Ragno di Caverna",
	"Un ragno grosso come un cane attende nella ragnatela.",
	"Occhi neri multipli ti scrutano.",
	"Ragnatele si tendono piano intorno a te.",
	"Odi in lontananza ragnatele che si tendono.")

MOB_DESC(1, mob, 41, RACE_ARACHNID,
	"ragno veleno",
	"Ragno Velenoso",
	"Un ragno dalle zampe striate di giallo si muove rapido.",
	"Gocce di veleno brillano alle zanne.",
	"Un sibilo acuto di ragno velenoso ti fa alzare lo sguardo.",
	"In lontananza un sibilo acuto di aracnide echeggia.")

MOB_DESC(1, mob, 42, RACE_ARACHNID,
	"ragno gigante",
	"Ragno Gigante",
	"Un ragno gigante copre meta' del soffitto.",
	"Seta fresca ancora umida di prede.",
	"Ragnatele grosse vibrano come tamburi tesi.",
	"Da qui vicino arriva la vibrazione di grosse ragnatele.")

MOB_DESC(1, mob, 43, RACE_INSECT,
	"scarabeo corazza",
	"Scarabeo Corazzato",
	"Uno scarabeo grande come un vassoio avanza.",
	"Carapace lucido devia le lame.",
	"Un ronzio profondo di corazza di chitina riempie la sala.",
	"Odi in lontananza un ronzio profondo di scarabeo.")

MOB_DESC(1, mob, 44, RACE_INSECT,
	"centopiedi caverna",
	"Centopiedi di Caverna",
	"Un centopiedi lungo come un braccio striscia sulle pareti.",
	"Mandibole gocciolano veleno.",
	"Scricchiolio di chitina di centopiedi ti passa vicino.",
	"In lontananza odi chitina che scricchiola.")

MOB_DESC(1, mob, 45, RACE_INSECT,
	"formica gigante",
	"Formica Gigante",
	"Una formica gigante trasporta un pezzo di armatura.",
	"Le mandibole possono tranciare ossa.",
	"Un ronzio di sciame di formiche ti circonda.",
	"Da qui vicino arriva un ronzio di sciame.")

MOB_DESC(1, mob, 46, RACE_INSECT,
	"mosca necrofaga",
	"Mosca Necrofaga",
	"Una mosca grande come un pugno ronzia attorno.",
	"Occhi composti riflettono ogni movimento.",
	"Un ronzio insistente riempie la stanza.",
	"Odi in lontananza un ronzio insistente di mosche.")

MOB_DESC(1, mob, 47, RACE_INSECT,
	"grillo caverna",
	"Grillo di Caverna",
	"Un grillo nero grande come un gatto salta.",
	"Antenne tese captano ogni suono.",
	"Uno stridio acuto di grillo di caverna ti ferisce le orecchie.",
	"In lontananza uno stridio acuto di grillo echeggia.")

MOB_DESC(1, mob, 48, RACE_ARACHNID,
	"vedova nera",
	"Vedova Nera",
	"Una vedova nera grossa attende immobile.",
	"Segno rosso sul dorso avverte pericolo.",
	"Un silenzio innaturale precede qualcosa che striscia qui.",
	"Ti pare di scorgere un fruscio leggero di zampe.")

MOB_DESC(1, mob, 49, RACE_INSECT,
	"sciame caverna",
	"Sciame di Caverna",
	"Uno sciame di insetti corazzati fruga tra le pietre.",
	"Mandibole lucide portano tracce di ossa recenti.",
	"Un ronzio acuto di sciame riempie la stanza.",
	"Odi in lontananza un ronzio di sciame furioso.")

MOB_DESC(1, mob, 50, RACE_INSECT,
	"larva nebbia effimera",
	"Larva della Nebbia Effimera",
	"Una larva traslucida striscia lasciando una scia di vapore gelido.",
	"La pelle viscida pulsa come se respirasse nebbia compressa.",
	"Senti un sibilo umido di larva nella nebbia.",
	"Da qui vicino arriva un sibilo umido e viscoso.")

MOB_DESC(1, mob, 51, RACE_INSECT,
	"verme bianco",
	"Verme Bianco",
	"Un verme bianco grosso come un braccio emerge dal fango.",
	"Pelle gelatinosa brilla di umidita'.",
	"Schizzi viscidi di verme bianco ti cadono addosso.",
	"In lontananza odi schizzi viscosi.")

MOB_DESC(1, mob, 52, RACE_INSECT,
	"stirge profondita",
	"Stirge delle Profondita'",
	"Una stirge alata plana con proboscide sanguinante.",
	"Ali sottili e occhi rossi.",
	"Un ronzio famelico di stirge ti passa sopra la testa.",
	"Odi in lontananza un ronzio famelico alato.")

MOB_DESC(1, mob, 53, RACE_ARACHNID,
	"zecca gigante",
	"Zecca Gigante",
	"Una zecca grossa come un piatto attende su una roccia.",
	"Corpo gonfio di sangue altrui.",
	"Uno schiocco umido di zecca ti fa sussultare.",
	"Da qui vicino arriva uno schiocco umido e nauseante.")

MOB_DESC(1, mob, 54, RACE_INSECT,
	"mantide caverna",
	"Mantide di Caverna",
	"Una mantide verde scuro alza le zampe a forbice.",
	"Sguardo alieno non tradisce alcuna emozione.",
	"Un fruscio secco di zampe di mantide ti circonda.",
	"In lontananza odi un fruscio secco di insetto.")

MOB_DESC(1, mob, 55, RACE_INSECT,
	"ape caverna",
	"Ape di Caverna",
	"Un'ape grossa come un pugno ronzia minacciosa.",
	"Pungiglione lungo gocciola veleno.",
	"Un ronzio furioso di ape di caverna riempie l'aria.",
	"Odi in lontananza un ronzio furioso di api.")

MOB_DESC(1, mob, 56, RACE_INSECT,
	"locusta bruna",
	"Locusta Bruna",
	"Una locusta grossa come un falco plana bassa.",
	"Le mandibole possono tagliare cuoio.",
	"Ali di locusta frusciano a un passo da te.",
	"In lontananza ali di locusta frusciano nel buio.")

MOB_DESC(1, mob, 57, RACE_ARACHNID,
	"ragno cacciatore",
	"Ragno Cacciatore",
	"Un ragno veloce corre sulle pareti.",
	"Veleno paralizzante nelle zanne.",
	"Passi leggeri di ragno sulla roccia ti sfiorano.",
	"Ti pare di scorgere passi leggeri sulla pietra.")

MOB_DESC(1, mob, 58, RACE_INSECT,
	"coleottero nero",
	"Coleottero Nero",
	"Un coleottero nero lucido grande come un gatto avanza.",
	"Le corna possono sollevare un uomo.",
	"Un ronzio metallico di coleottero vibra nella sala.",
	"Da qui vicino arriva un ronzio metallico.")

MOB_DESC(1, mob, 59, RACE_INSECT,
	"scarafaggio caverna",
	"Scarafaggio di Caverna",
	"Uno scarafaggio immenso striscia dal buio.",
	"Le antenne captano ogni odore.",
	"Un fruscio viscido di scarafaggio ti passa ai piedi.",
	"Odi in lontananza un fruscio viscido.")

MOB_DESC(1, mob, 60, RACE_SNAKE,
	"vipera caverna",
	"Vipera di Caverna",
	"Una vipera striata si avvolge su una roccia calda.",
	"Zanne gocciolano veleno giallo.",
	"Un sibilo secco di vipera ti fa fermare.",
	"In lontananza un sibilo secco di serpente echeggia.")

MOB_DESC(1, mob, 61, RACE_SNAKE,
	"serpente ossa",
	"Serpente delle Ossa",
	"Un serpente bianco striscia tra teschi.",
	"Occhi rosa privi di pupilla.",
	"Ossa di serpente rotolano sul pavimento vicino a te.",
	"Da qui vicino arriva il rotolio di ossa serpentine.")

MOB_DESC(1, mob, 62, RACE_REPTILE,
	"lucertola gigante",
	"Lucertola Gigante",
	"Una lucertola grossa come un cane corre sulle pareti.",
	"Coda spessa colpisce come frusta.",
	"Artigli di lucertola graffiano la roccia a un passo.",
	"Odi in lontananza artigli che graffiano la pietra.")

MOB_DESC(1, mob, 63, RACE_REPTILE,
	"rana pietra",
	"Rana di Pietra",
	"Una rana massiccia mimetizzata tra i massi attende immobile.",
	"La pelle sembra pietra viva; solo gli occhi si muovono.",
	"Un colpo secco di lingua sulle pietre ti fa alzare lo sguardo.",
	"In lontananza odi un colpo secco sulle pietre.")

MOB_DESC(1, mob, 64, RACE_REPTILE,
	"geco caverna",
	"Geco di Caverna",
	"Un geco enorme mimetizzato attende sul soffitto.",
	"La pelle cambia colore con la roccia.",
	"Un fruscio improvviso di geco ti arriva da sopra.",
	"Da qui vicino arriva un fruscio improvviso.")

MOB_DESC(1, mob, 65, RACE_SNAKE,
	"boa caverna",
	"Boa di Caverna",
	"Un boa grosso come un tronco blocca il corridoio.",
	"Muscoli poderosi si contraggono.",
	"Un respiro sibilante di boa ti passa vicino.",
	"Odi in lontananza un respiro sibilante e pesante.")

MOB_DESC(1, mob, 66, RACE_REPTILE,
	"coccodrillo sotterraneo",
	"Coccodrillo Sotterraneo",
	"Un coccodrillo albino emerge da una pozza.",
	"Pelle traslucida e occhi rossi.",
	"Schizzi d'acqua di un coccodrillo ti bagnano.",
	"In lontananza odi schizzi d'acqua sotterranea.")

MOB_DESC(1, mob, 67, RACE_SNAKE,
	"aspide nero",
	"Aspide Nero",
	"Un serpente nero lucido si muove silenziosamente.",
	"Veleno letale in gocce alle zanne.",
	"Un sibilo quasi impercettibile di aspide ti sfiora.",
	"Ti pare di udire un sibilo sottilissimo.")

MOB_DESC(1, mob, 68, RACE_REPTILE,
	"basilisco giovane",
	"Basilisco Giovane",
	"Un basilisco immaturo evita il tuo sguardo.",
	"La pelle grigia simula pietra.",
	"Un sibilo roco di basilisco riempie la stanza.",
	"Da qui vicino arriva un sibilo roco e pesante.")

MOB_DESC(1, mob, 69, RACE_SNAKE,
	"serpente nebbia",
	"Serpente della Nebbia",
	"Un serpente traslucido emerge dal vapore.",
	"Il corpo sembra fatto di bruma solidificata.",
	"Vapore di nebbia si condensa e sibila intorno a te.",
	"Odi in lontananza vapore che si condensa con un sibilo.")

MOB_DESC(1, mob, 70, RACE_PREDATOR,
	"predatore giovane",
	"Predatore delle Profondita'",
	"Un giovane predatore delle caverne ti fissa senza paura.",
	"Zanne non ancora mature, ma gia' affilate.",
	"Un ululato basso di predatore ti fa alzare lo sguardo.",
	"In lontananza un ululato di predatore si allunga.")

MOB_DESC(1, mob, 71, RACE_PREDATOR,
	"cane radici",
	"Cane delle Radici",
	"Un cane magro dalle zampe simili a radici ringhia basso.",
	"Pelliccia impregnata di terra e sangue secco.",
	"Un ringhio profondo di cane delle radici ti circonda.",
	"Odi in lontananza un ringhio profondo.")

MOB_DESC(1, mob, 72, RACE_PREDATOR,
	"warg caverna",
	"Warg di Caverna",
	"Un warg grosso come un cavallo ringhia nel buio.",
	"Pelliccia nera e zanne da lupo infernale.",
	"Un ululato cavernoso di warg riempie la sala.",
	"In lontananza un ululato cavernoso echeggia.")

MOB_DESC(1, mob, 73, RACE_PREDATOR,
	"lupo nebbia",
	"Lupo della Nebbia",
	"Un lupo spettrale emerge dal vapore.",
	"Occhi argentati non tradiscono paura.",
	"Un ululato nella bruma ti passa a fianco.",
	"Da qui vicino arriva un ululato nella nebbia.")

MOB_DESC(1, mob, 74, RACE_PREDATOR,
	"pantera caverna",
	"Pantera di Caverna",
	"Una pantera nera scivola senza rumore.",
	"Occhi verdi brillano nel buio.",
	"Un ringhio basso di pantera vibra nell'aria.",
	"Odi in lontananza un ringhio basso e felino.")

MOB_DESC(1, mob, 75, RACE_PREDATOR,
	"worg ibrido",
	"Worg Ibrido",
	"Un worg corrotto dalla dimensione ringhia in modo distorto.",
	"Zanne irregolari e saliva nera.",
	"Un ringhio metallico di worg ti fa sussultare.",
	"In lontananza un ringhio metallico risuona.")

MOB_DESC(1, mob, 76, RACE_PREDATOR,
	"sciacallo profondita",
	"Sciacallo delle Profondita'",
	"Uno sciacallo emaciato segue il gruppo.",
	"Occhi furbi attendono la distrazione.",
	"Un guaito acuto di sciacallo ti ferisce le orecchie.",
	"Da qui vicino arriva un guaito acuto.")

MOB_DESC(1, mob, 77, RACE_PREDATOR,
	"ratto gigante corrotto",
	"Ratto Gigante Corrotto",
	"Un ratto grosso come un cane mostra zanne innaturali.",
	"Pelliccia rada e occhi rossi.",
	"Squittii minacciosi di ratti giganti ti circondano.",
	"Odi in lontananza squittii minacciosi.")

MOB_DESC(1, mob, 78, RACE_PREDATOR,
	"tigre caverna",
	"Tigre di Caverna",
	"Una tigre dalle strisce pallide attende.",
	"Muscoli tesi pronti al balzo.",
	"Un ruggito sordo di tigre vibra nella stanza.",
	"In lontananza un ruggito sordo di grande felino echeggia.")

MOB_DESC(1, mob, 79, RACE_PREDATOR,
	"iena profondita",
	"Iena delle Profondita'",
	"Una iena macilenta ride mostrando fauci.",
	"Risata disturbante riempie la caverna.",
	"Una risata isterica di iena ti gela il sangue.",
	"Da qui vicino arriva una risata isterica di iena.")

MOB_DESC(1, mob, 80, RACE_BIRD,
	"pipistrello vorace",
	"Pipistrello Vorace",
	"Un pipistrello grosso come un cane volteggia sopra la tua testa.",
	"Canini sporgono da un muso allungato e famelico.",
	"Un vortice d'aria di ali ti passa sul capo.",
	"Odi in lontananza un battito d'ali frenetico.")

MOB_DESC(1, mob, 81, RACE_BIRD,
	"pipistrello vampiro",
	"Pipistrello Vampiro",
	"Un pipistrello enorme plana con muso porcino.",
	"Lingua sanguinante lecca l'aria.",
	"Un ronzio famelico di pipistrello vampiro ti sfiora.",
	"In lontananza un ronzio famelico alato si avvicina.")

MOB_DESC(1, mob, 82, RACE_BIRD,
	"corvo caverna",
	"Corvo di Caverna",
	"Un corvo grosso osserva dall'ombra.",
	"Piume nere assorbono la luce.",
	"Un gracchio sinistro di corvo riempie la sala.",
	"Odi in lontananza un gracchio sinistro.")

MOB_DESC(1, mob, 83, RACE_BIRD,
	"avvoltoio caverne",
	"Avvoltoio delle Caverne",
	"Un avvoltoio grosso come un falco plana basso.",
	"Becco ad uncino pronto a squartare.",
	"Ali d'avvoltoio battono pesanti sopra di te.",
	"Da qui vicino arriva il battito di grandi ali.")

MOB_DESC(1, mob, 84, RACE_BIRD,
	"gufo caverna",
	"Gufo di Caverna",
	"Un gufo gigante fissa con occhi dorati.",
	"Ali silenziose planano nel buio.",
	"Un verso stridulo di gufo ti passa vicino.",
	"In lontananza un verso stridulo di gufo echeggia.")

MOB_DESC(1, mob, 85, RACE_BIRD,
	"arpia giovane",
	"Arpia Giovane",
	"Un'arpia immatura plana goffamente.",
	"Artigli non ancora maturi ma affilati.",
	"Un grido acuto di arpia giovane ti squarcia le orecchie.",
	"Odi in lontananza un grido d'arpia acuto.")

MOB_DESC(1, mob, 86, RACE_BIRD,
	"picchio ossidiana",
	"Picchio d'Ossidiana",
	"Un picchio dalle piume nere picchietta la roccia.",
	"Becco duro come acciaio.",
	"Un picchiettio ossessionante di becco sulla roccia ti segue.",
	"In lontananza odi un picchiettio ossessionante.")

MOB_DESC(1, mob, 87, RACE_BIRD,
	"condor profondita",
	"Condor delle Profondita'",
	"Un condor macilento scende dal soffitto.",
	"Ali ampie oscurano la luce.",
	"Un vento d'ali di condor ti investe il volto.",
	"Da qui vicino arriva un vento d'ali poderoso.")

MOB_DESC(1, mob, 88, RACE_BIRD,
	"rondini caverna",
	"Rondini di Caverna",
	"Rondini giganti volteggiano minacciose.",
	"Becchi corti ma veloci.",
	"Cinguettii cavernosi di rondini ti girano intorno.",
	"Odi in lontananza cinguettii cavernosi.")

MOB_DESC(1, mob, 89, RACE_BIRD,
	"fenice corrotta",
	"Fenice Corrotta",
	"Un uccello fiammeggiante corrotto dalla bruma volteggia.",
	"Piume nere e fiamme verdi.",
	"Un crepitio di fuoco di fenice corrotta riempie l'aria.",
	"In lontananza un crepitio di brace echeggia.")

MOB_DESC(1, mob, 90, RACE_DWARF,
	"nano maledetto",
	"Nano Maledetto",
	"Un nano maledetto brandisce un'ascia runica.",
	"Barba incrostata di polvere e sangue.",
	"Imprecazioni furibonde di un nano maledetto ti circondano.",
	"Da qui vicino arriva un'imprecazione nanica furibonda.")

MOB_DESC(1, mob, 91, RACE_DWARF,
	"nano guardia",
	"Nano Guardia",
	"Un nano armato sorveglia un corridoio stretto.",
	"Elmo con visiera ribassata.",
	"L'armatura di una guardia nana stride a ogni passo.",
	"Odi in lontananza armatura nana che stride.")

MOB_DESC(1, mob, 92, RACE_DWARF,
	"nano scassinatore",
	"Nano Scassinatore",
	"Un nano con piccone e corda esplora.",
	"Occhi abituati al buio.",
	"Un piccone batte sulla roccia a un passo da te.",
	"In lontananza odi un piccone sulla pietra.")

MOB_DESC(1, mob, 93, RACE_DRAAGDIM,
	"duergar guerriero",
	"Duergar Guerriero",
	"Un duergar grigio brandisce un martello da guerra.",
	"Barba d'acciaio e sguardo ostile.",
	"Un grugnito duergar basso e duro riempie la sala.",
	"Da qui vicino arriva un grugnito duergar.")

MOB_DESC(1, mob, 94, RACE_DRAAGDIM,
	"duergar chierico",
	"Duergar Chierico",
	"Un duergar chierico invoca l'oscurita'.",
	"Simboli profani brillano sull'armatura.",
	"Preghiere profane di un chierico duergar ti passano vicino.",
	"Odi in lontananza preghiere duergar profane.")

MOB_DESC(1, mob, 95, RACE_GOLEM,
	"golem pietra minore",
	"Golem Minore di Pietra",
	"Un golem di pietra animato avanza goffamente.",
	"Rune incise pulsano debolmente.",
	"Pietre di un golem stridono l'una contro l'altra.",
	"In lontananza pietre vive stridono e si muovono.")

MOB_DESC(1, mob, 96, RACE_ENFAN,
	"enfan errante",
	"Enfan Errante",
	"Un enfan dalle orecchie lunghe brandisce una lama curva.",
	"Occhi antichi in volto giovane.",
	"Passi leggeri di un enfan errante ti sfiorano.",
	"Ti pare di scorgere passi leggeri oltre l'angolo.")

MOB_DESC(1, mob, 97, RACE_GNOME,
	"gnomo fessura",
	"Gnomo delle Fessure",
	"Un piccolo umanoide dalle mani artigliate ti osserva dall'ombra.",
	"Occhi troppo grandi brillano di malizia famelosa.",
	"Senti un riso stridulo di gnomo delle fessure.",
	"Da qui vicino arriva un riso stridulo.")

MOB_DESC(1, mob, 98, RACE_GNOME,
	"gnomo illusionista",
	"Gnomo Illusionista",
	"Un gnomo sotterraneo crea miraggi attorno a se'.",
	"Sorriso furbo e dita veloci.",
	"Risate distorte di illusionista ti girano intorno.",
	"Odi in lontananza risate distorte e false.")

MOB_DESC(1, mob, 99, RACE_SKEXIE,
	"skexie furtivo",
	"Skexie Furtivo",
	"Un skexie dalle movenze rapide brandisce delle daghe.",
	"Pelliccia scura lo mimetizza.",
	"Un fruscio rapido di skexie ti passa alle spalle.",
	"In lontananza un fruscio rapido si muove.")

MOB_DESC(1, mob, 100, RACE_DARK_ELF,
	"drow esploratore",
	"Drow Esploratore",
	"Un drow incappucciato emerge dall'ombra.",
	"Lame d'ossidiana pronte a colpire.",
	"Sussurri elfici drow ti sfiorano l'orecchio.",
	"Da qui vicino arriva un sussurro elfico nero.")

MOB_DESC(1, mob, 101, RACE_DARK_ELF,
	"drow mago",
	"Drow Mago",
	"Un mago drow traccia rune nel buio.",
	"Energia viola danza sulle dita.",
	"Incantesimi sibilati di un mago drow riempiono l'aria.",
	"Odi in lontananza formule drow sibilate.")

MOB_DESC(1, mob, 102, RACE_DARK_ELF,
	"drow guerriero",
	"Drow Guerriero",
	"Un guerriero drow in armatura di mithril scuro avanza.",
	"Scudo e spada coordinati.",
	"Armatura elfica di un guerriero drow tintinna piano.",
	"In lontananza odi il tintinnio di armatura elfica.")

MOB_DESC(1, mob, 103, RACE_ELVEN,
	"elfo ombra",
	"Elfo Ombra",
	"Un elfo corrotto dalla bruma brandisce un arco lungo.",
	"Occhi vuoti e movenze fluide.",
	"Una corda d'arco d'ombra vibra nella stanza.",
	"Da qui vicino arriva il fischio di frecce nel buio.")

MOB_DESC(1, mob, 104, RACE_ELVEN,
	"elfo esiliato",
	"Elfo Esiliato",
	"Un elfo esiliato combatte con spada sottile.",
	"Mantello strappato e sguardo amaro.",
	"Passi silenziosi di un elfo esiliato ti sfiorano.",
	"Ti pare di scorgere passi silenziosi.")

MOB_DESC(1, mob, 105, RACE_PLANAR,
	"fey corrotta",
	"Fey Corrotta",
	"Una fey corrotta danza nel buio.",
	"Ali spezzate e sorriso crudele.",
	"Risate argentine e false di una fey ti circondano.",
	"Odi in lontananza risate argentine e false.")

MOB_DESC(1, mob, 106, RACE_GHOST,
	"elfo spettrale",
	"Elfo Spettrale",
	"Un elfo morto da secoli vaga senza riposo.",
	"Armatura elfica ancora lucente.",
	"Un lamento elfico spettrale ti passa attraverso.",
	"In lontananza un lamento elfico si allunga.")

MOB_DESC(1, mob, 107, RACE_DARK_ELF,
	"drow assassino",
	"Drow Assassino",
	"Un assassino drow attende immobile.",
	"Veleno gocciola dalla lama.",
	"Un silenzio mortale ti avverte di qualcosa alle spalle.",
	"Ti pare di scorgere un movimento silenzioso.")

MOB_DESC(1, mob, 108, RACE_ELVEN,
	"elfo custode",
	"Elfo Custode",
	"Un custode elfico protegge un passaggio antico.",
	"Rune elfiche brillano sull'arco.",
	"Canti elfici di un custode ti riempiono le orecchie.",
	"Odi in lontananza canti elfici.")

MOB_DESC(1, mob, 109, RACE_PLANAR,
	"ombra fey",
	"Ombra Fey",
	"Un'ombra fey si stacca dal muro danzando.",
	"Forma umanoide indistinta.",
	"Sussurri musicali di un'ombra fey ti avvolgono.",
	"Da qui vicino arriva un sussurro musicale e gelido.")

MOB_DESC(1, mob, 110, RACE_TROLL,
	"troll caverna",
	"Troll di Caverna",
	"Un troll goffo si rialza sbavando.",
	"Pelle verde e rigenerazione visibile.",
	"Un grugnito di troll di caverna rimbomba nella sala.",
	"In lontananza un grugnito di troll echeggia.")

MOB_DESC(1, mob, 111, RACE_TROLL,
	"troll ricucito",
	"Troll Ricucito",
	"Un troll ricucito grossolanamente avanza.",
	"Cicatrici e punti visibili ovunque.",
	"Risate gutturali di un troll ricucito ti circondano.",
	"Odi in lontananza risate gutturali di troll.")

MOB_DESC(1, mob, 112, RACE_GIANT,
	"gigante caverna",
	"Gigante di Caverna",
	"Un gigante curvo si muove nel corridoio.",
	"Clava di tronco sollevata.",
	"Passi tremendi di un gigante fanno ballare la polvere.",
	"In lontananza passi tremendi scuotono la roccia.")

MOB_DESC(1, mob, 113, RACE_GIANT_HILL,
	"gigante colline",
	"Gigante delle Colline",
	"Un gigante delle colline blocca la volta.",
	"Pelle coriacea e odore di muschio.",
	"Rumore di massi spostati da un gigante ti arriva vicino.",
	"Da qui vicino arriva il rumore di massi che rotolano.")

MOB_DESC(1, mob, 114, RACE_GIANT_STONE,
	"gigante pietra",
	"Gigante di Pietra",
	"Un gigante di pietra si muove lentamente.",
	"Pelle simile a granito.",
	"Pietre cadono mentre un gigante di pietra si muove.",
	"Odi in lontananza pietre che cadono.")

MOB_DESC(1, mob, 115, RACE_HALF_OGRE,
	"mezzogre guardia",
	"Mezzogre Guardia",
	"Un mezzogre armato sorveglia un varco.",
	"Testa piccola e braccia enormi.",
	"Un grugnito orchesco di mezzogre riempie la stanza.",
	"In lontananza un grugnito orchesco risuona.")

MOB_DESC(1, mob, 116, RACE_HALF_OGRE,
	"mezzogre berserker",
	"Mezzogre Berserker",
	"Un mezzogre impazzito brandisce una mazza.",
	"Occhi rossi e muscoli tesi dalla rabbia.",
	"Un ansimare furioso di berserker mezzogre ti passa vicino.",
	"Da qui vicino arriva un'urla di furia mezzogre.")

MOB_DESC(1, mob, 117, RACE_TYTAN,
	"titano caduto",
	"Titano Caduto",
	"Un titano decaduto si trascina inginocchiato.",
	"Corpo enorme ma curvo dal tempo.",
	"Un tremore tellurico di passi titanici ti scuote.",
	"In lontananza un tremore tellurico si propaga.")

MOB_DESC(1, mob, 118, RACE_GIANT,
	"gigante mutilato",
	"Gigante Mutilato",
	"Un gigante con arti mancanti combatte comunque.",
	"Ferite antiche non guariscono.",
	"Un lamento profondo di gigante mutilato ti circonda.",
	"Odi in lontananza un lamento profondo e spezzato.")

MOB_DESC(1, mob, 119, RACE_TROLL,
	"troll nebbia",
	"Troll della Nebbia",
	"Un troll avvolto in bruma attacca goffamente.",
	"Pelle grigia e occhi vitrei.",
	"Un grugnito ovattato di troll della nebbia ti arriva.",
	"In lontananza un grugnito ovattato si trascina.")

MOB_DESC(1, mob, 120, RACE_DEMON,
	"imp caverna",
	"Imp di Caverna",
	"Un imp alato volteggia lanciando scintille.",
	"Coda punge come una vespa.",
	"Risate acute di un imp di caverna ti feriscono le orecchie.",
	"Da qui vicino arriva una risata acuta di imp.")

MOB_DESC(1, mob, 121, RACE_DEMON,
	"demone minore",
	"Demone Minore",
	"Un demone cornuto giovane striscia dal buio.",
	"Zolfo e cenere lo circondano.",
	"Un odore di zolfo di demone minore ti prende alla gola.",
	"Odi in lontananza un tanfo di zolfo.")

MOB_DESC(1, mob, 122, RACE_DEVIL,
	"diavolo fessure",
	"Diavolo delle Fessure",
	"Un diavolo piccolo brandisce un tridente.",
	"Pelle rossa e occhi gialli.",
	"Risate infernali di un diavolo delle fessure ti circondano.",
	"In lontananza risate infernali echeggiano.")

MOB_DESC(1, mob, 123, RACE_ELEMENT,
	"elementale terra",
	"Elementale di Terra",
	"Un elementale di terra si rialza dal pavimento.",
	"Pietre e polvere compongono il corpo.",
	"Pietre cadono mentre un elementale di terra si muove.",
	"Da qui vicino arriva il tonfo di pietre vive.")

MOB_DESC(1, mob, 124, RACE_ELEMENT,
	"elementale acqua",
	"Elementale d'Acqua",
	"Un elementale d'acqua scorre come marea.",
	"Corpo trasparente e gelido.",
	"Schizzi d'acqua di un elementale ti bagnano.",
	"Odi in lontananza schizzi d'acqua elementale.")

MOB_DESC(1, mob, 125, RACE_ELEMENT,
	"elementale fuoco",
	"Elementale di Fuoco",
	"Un elementale di fuoco danza nel buio.",
	"Fiamme verdi corrompono il fuoco.",
	"Un crepitio di fiamme di elementale riempie la sala.",
	"In lontananza un crepitio di fiamme echeggia.")

MOB_DESC(1, mob, 126, RACE_SLIME,
	"gelatina caverna",
	"Gelatina di Caverna",
	"Una gelatina traslucida striscia sul pavimento.",
	"Ossa galleggiano nel corpo viscido.",
	"Schizzi viscidi di gelatina ti cadono ai piedi.",
	"Da qui vicino arriva lo schizzo di qualcosa di viscoso.")

MOB_DESC(1, mob, 127, RACE_SLIME,
	"muffa vivente",
	"Muffa Vivente",
	"Una muffa senziente si espande sulle pareti.",
	"Spore verdi riempiono l'aria.",
	"Una tosse e starnuti provocati da spore ti prendono.",
	"Odi in lontananza tosse e starnuti nel buio.")

MOB_DESC(1, mob, 128, RACE_DEMON,
	"manes caverna",
	"Manes di Caverna",
	"Un manes distorto striscia come fumo solido.",
	"Il volto muta continuamente.",
	"Lamenti distorti di un manes ti passano vicino.",
	"In lontananza lamenti distorti si trascinano.")

MOB_DESC(1, mob, 129, RACE_DEVIL,
	"diavolo catena",
	"Diavolo a Catena",
	"Un diavolo trascina catene infuocate.",
	"Ferite bruciano al contatto.",
	"Uno scroscio di catene di un diavolo riempie l'aria.",
	"Odi in lontananza catene che scrosciano.")

MOB_DESC(1, mob, 130, RACE_MFLAYER,
	"illithid giovane",
	"Illithid Giovane",
	"Un illithid immaturo tenta di entrare nella tua mente.",
	"I tentacoli facciali si contorcono.",
	"Sussurri psichici di un illithid ti premono la mente.",
	"Da qui vicino arriva un sussurro psichico.")

MOB_DESC(1, mob, 131, RACE_SPECIAL,
	"mimica giovane",
	"Mimica Giovane",
	"Una cassa si apre mostrando le fauci.",
	"Lingua viscida lecca l'aria.",
	"Un tonfo di legno di mimica ti fa alzare lo sguardo.",
	"In lontananza odi un tonfo di legno innaturale.")

MOB_DESC(1, mob, 132, RACE_SPECIAL,
	"mimica scrigno",
	"Mimica Scrigno",
	"Uno scrigno dorato attende troppo comodo per essere vero.",
	"La serratura sembra un occhio vivo.",
	"Uno scricchiolio metallico di scrigno vivo vibra qui.",
	"Odi in lontananza uno scricchiolio metallico.")

MOB_DESC(1, mob, 133, RACE_PRIMATE,
	"grimlock guerriero",
	"Grimlock Guerriero",
	"Un grimlock cieco percepisce il tuo odore.",
	"Pelle grigia e denti affilati.",
	"Grugniti cavernosi di grimlock riempiono la stanza.",
	"In lontananza grugniti cavernosi di ciechi echeggiano.")

MOB_DESC(1, mob, 134, RACE_PRIMATE,
	"grimlock cacciatore",
	"Grimlock Cacciatore",
	"Un cacciatore grimlock trascina lancia di osso.",
	"Percezione acustica terrificante.",
	"Passi silenziosi di un cacciatore grimlock ti sfiorano.",
	"Ti pare di scorgere passi silenziosi di cacciatore.")

MOB_DESC(1, mob, 135, RACE_PARASITE,
	"parassita caverna",
	"Parassita di Caverna",
	"Un parassita grosso come un gatto si lancia addosso.",
	"Corpo viscido e ventose.",
	"Schizzi viscidi di un parassita ti cadono addosso.",
	"Da qui vicino arriva uno schizzo viscoso e nauseante.")

MOB_DESC(1, mob, 136, RACE_ASTRAL,
	"creatura astrale minore",
	"Creatura Astrale Minore",
	"Una creatura planare fluttua in modo instabile.",
	"La forma cambia continuamente.",
	"Echi astrali lontani e strani ti vibrano nelle ossa.",
	"Odi in lontananza echi astrali impossibili.")

MOB_DESC(1, mob, 137, RACE_PLANAR,
	"ombra planare",
	"Ombra Planare",
	"Un'ombra planare si stacca dal muro.",
	"Un freddo che non appartiene a questo mondo.",
	"Un silenzio planare innaturale ti stringe le orecchie.",
	"In lontananza un silenzio innaturale si allarga.")

MOB_DESC(1, mob, 138, RACE_SPECIAL,
	"aberrazione caverna",
	"Aberrazione di Caverna",
	"Una creatura innominabile striscia dal buio.",
	"Troppi occhi e troppe bocche.",
	"Suoni disturbanti di un'aberrazione ti fanno rabbrividire.",
	"Da qui vicino arriva un suono che non dovrebbe esistere.")

MOB_DESC(1, mob, 139, RACE_MFLAYER,
	"illithid servitore",
	"Illithid Servitore",
	"Un servitore illithid obbedisce a un padrone lontano.",
	"I tentacoli tengono una lama psionica.",
	"Pulsazioni mentali di un servitore illithid ti premono.",
	"Odi in lontananza pulsazioni mentali.")

MOB_DESC(1, mob, 140, RACE_VEGMAN,
	"servitore muschio",
	"Servitore di Muschio",
	"Un umanoide ricoperto di muschio blocca il passaggio.",
	"Radici sottili serpeggiano lungo le sue gambe.",
	"La terra scricchiola sotto un servitore di muschio.",
	"In lontananza la terra scricchiola sotto passi vegetali.")

MOB_DESC(1, mob, 141, RACE_VEGMAN,
	"guardiano radici",
	"Guardiano delle Radici",
	"Un umanoide vegetale sorveglia un passaggio.",
	"Radici emergono dal pavimento.",
	"Un fruscio di foglie di guardiano delle radici ti circonda.",
	"Da qui vicino arriva un fruscio di foglie vive.")

MOB_DESC(1, mob, 142, RACE_TREE,
	"treant giovane",
	"Treant Giovane",
	"Un treant giovane si muove goffamente.",
	"Rami sfiorano il soffitto.",
	"Legno di treant scricchiola e geme vicino a te.",
	"Odi in lontananza legno vivo che scricchiola.")

MOB_DESC(1, mob, 143, RACE_VEGGIE,
	"pianta carnivora",
	"Pianta Carnivora",
	"Una pianta carnivora attende con fauci aperte.",
	"Polline dolce maschera pericolo.",
	"Un fruscio di foglie carnivore ti passa ai piedi.",
	"In lontananza un fruscio di foglie affamate echeggia.")

MOB_DESC(1, mob, 144, RACE_VEGGIE,
	"liane strangolatrici",
	"Liane Strangolatrici",
	"Liane animate pendono dal soffitto.",
	"Steli si avvolgono come serpenti.",
	"Un fruscio secco di liane ti sfiora le caviglie.",
	"Da qui vicino arriva un fruscio secco di liane.")

MOB_DESC(1, mob, 145, RACE_GHOST,
	"eco nebbia",
	"Eco della Nebbia",
	"Un'eco spettrale della dimensione vaga, confusa.",
	"Forma muta come vapore.",
	"Sussurri ovattati di un'eco di nebbia ti avvolgono.",
	"Odi in lontananza sussurri ovattati nella bruma.")

MOB_DESC(1, mob, 146, RACE_ELEMENT,
	"nebbia vivente",
	"Nebbia Vivente",
	"Una nube di bruma assume forma umanoide.",
	"Occhi argentati nel vapore.",
	"Un silenzio gelido di nebbia vivente ti stringe.",
	"In lontananza un silenzio gelido si addensa.")

MOB_DESC(1, mob, 147, RACE_GHOST,
	"spettro nebbia",
	"Spettro della Nebbia",
	"Uno spettro nato dalla bruma fluttua.",
	"Corpo semitrasparente.",
	"Un vento gelido di spettro della nebbia ti investe.",
	"Da qui vicino arriva un vento gelido nella bruma.")

MOB_DESC(1, mob, 148, RACE_PREDATOR,
	"predatore bruma",
	"Predatore della Bruma",
	"Un predatore nato nel velo caccia in silenzio.",
	"Pelliccia grigia e occhi argentati.",
	"Un ringhio ovattato di predatore della bruma ti passa vicino.",
	"Odi in lontananza un ringhio ovattato.")

MOB_DESC(1, mob, 149, RACE_INSECT,
	"libellula nebbia",
	"Libellula della Nebbia",
	"Una libellula traslucida volteggia nel vapore.",
	"Ali sottili tagliano l'aria.",
	"Un ronzio sottile di libellula della nebbia ti sfiora.",
	"In lontananza un ronzio sottile alato echeggia.")

MOB_DESC(1, trap, 0, RACE_SNAKE,
	"trappola anguilla fango",
	"Anguilla del Fango",
	"Un'anguilla viscida emerge dal fango con bocca spalancata.",
	"La pelle oleosa rende difficile colpirla.",
	"Schizzi di fango gorgogliano a un passo da te.",
	"Da qui vicino arriva lo schizzo di fango vivo.")

MOB_DESC(1, trap, 1, RACE_SPECIAL,
	"trappola mimica pavimento",
	"Mimica del Pavimento",
	"Una piastrella si apre mostrando le fauci.",
	"Denti d'acciaio, prima celati sotto la polvere, spuntano.",
	"Uno scricchiolio metallico di pavimento vivo vibra qui.",
	"Odi in lontananza uno scricchiolio metallico sul suolo.")

MOB_DESC(1, trap, 2, RACE_SLIME,
	"trappola gelatina assassina",
	"Gelatina Assassina",
	"Una gelatina assassina cade dal soffitto.",
	"Corpo viscido inghiotte luce e speranza.",
	"Schizzi viscidi di gelatina ti cadono ai piedi.",
	"In lontananza odi schizzi viscosi di gelatina.")

MOB_DESC(1, trap, 3, RACE_INSECT,
	"trappola ragnatela vivente",
	"Ragnatela Vivente",
	"Filamenti di ragnatela si tendono all'improvviso.",
	"Qualcosa tira verso l'alto con forza innaturale.",
	"Ragnatele vive vibrano intorno a te.",
	"Da qui vicino arriva la vibrazione di ragnatele vive.")

MOB_DESC(1, trap, 4, RACE_SNAKE,
	"trappola vipera nascosta",
	"Vipera Nascosta",
	"Una vipera scatta da una fessura nel muro.",
	"Veleno gocciola dalle zanne appena visibili.",
	"Un sibilo secco di vipera nascosta ti fa fermare.",
	"Odi in lontananza un sibilo secco di vipera.")

MOB_DESC(1, trap, 5, RACE_ELEMENT,
	"trappola gas velenoso",
	"Nube di Gas Velenoso",
	"Una nube di gas velenoso si alza dal pavimento.",
	"Occhi gialli pulsano nel vapore.",
	"Colpi di tosse violenti riempiono l'aria qui.",
	"In lontananza odi colpi di tosse violenti.")

MOB_DESC(1, trap, 6, RACE_GOLEM,
	"trappola porta a punte",
	"Porta a Punta",
	"La porta si chiude rivelando punte di pietra.",
	"Rune attivate scorrono lungo il telaio.",
	"Pietre scivolano e meccanismi scattano vicino a te.",
	"Da qui vicino arriva lo scatto di pietre che scivolano.")

MOB_DESC(1, trap, 7, RACE_ELEMENT,
	"trappola ghiaccio sottile",
	"Ghiaccio Sottile",
	"Il pavimento e' un sottile strato di ghiaccio nascosto.",
	"Crepe si aprono sotto i tuoi passi.",
	"Ghiaccio sottile scricchiola sotto i tuoi piedi.",
	"Odi in lontananza ghiaccio che scricchiola.")

MOB_DESC(1, trap, 8, RACE_ELEMENT,
	"trappola fiamma",
	"Trappola di Fiamma",
	"Getti di fiamma erompono da ugelli nel pavimento.",
	"Rune bruciate si accendono in sequenza.",
	"Un crepitio di fiamme nascoste ti investe il volto.",
	"In lontananza un crepitio di fuoco improvviso echeggia.")

MOB_DESC(1, trap, 9, RACE_ARACHNID,
	"trappola ragno agguato",
	"Ragno d'Agguato",
	"Un ragno d'agguato precipita dal soffitto.",
	"Veleno paralizzante nelle zanne.",
	"Un fruscio di zampe di ragno d'agguato ti passa vicino.",
	"Da qui vicino arriva un fruscio di zampe.")

MOB_DESC(1, trap, 10, RACE_ELEMENT,
	"trappola cristallo tagliente",
	"Cristallo Tagliente",
	"Cristalli affilati sporgono dal pavimento.",
	"Luce riflessa nasconde i fili taglienti.",
	"Schegge di cristallo tintinnano e cadono qui.",
	"Odi in lontananza schegge di cristallo che cadono.")

MOB_DESC(1, trap, 11, RACE_ELEMENT,
	"trappola marea improvvisa",
	"Marea Improvvisa",
	"Acqua salata irrompe da fessure laterali.",
	"Corrente spinge verso pareti taglienti.",
	"Onde improvvise sbattano contro le pareti intorno a te.",
	"In lontananza odi onde che si abbattono.")

MOB_DESC(1, trap, 12, RACE_ELEMENT,
	"trappola polvere sale",
	"Polvere di Sale Tagliente",
	"Polvere di sale tagliente si alza in vortice.",
	"Cristalli bianchi graffiano pelle e occhi.",
	"Cristalli di sale tagliente scricchiolano sotto i passi.",
	"Da qui vicino arriva lo scricchiolio di cristalli di sale.")

MOB_DESC(1, trap, 13, RACE_UNDEAD_SKELETON,
	"trappola scheletri sepolcro",
	"Scheletri del Sepolcro",
	"Scheletri cadono dal soffitto in una pioggia d'ossa.",
	"Lame arrugginite pronte a colpire.",
	"Ossa di scheletri sepolti sbattono nella stanza.",
	"Odi in lontananza ossa che sbattono dal sepolcro.")

MOB_DESC(1, trap, 14, RACE_DEMON,
	"trappola fessura infernale",
	"Fessura Infernale",
	"Una fessura infernale si apre sotto i piedi.",
	"Zolfo e artigli emergono dal baratro.",
	"Un odore di zolfo di fessura infernale ti prende alla gola.",
	"In lontananza aleggia un tanfo di zolfo.")

MOB_DESC(1, trap, 15, RACE_INSECT,
	"trappola vespa caverna",
	"Vespe di Caverna",
	"Vespe di caverna escono da un nido nascosto.",
	"Pungiglioni lunghi gocciolano veleno.",
	"Un ronzio furioso di vespe di caverna riempie l'aria.",
	"Da qui vicino arriva un ronzio furioso di vespe.")

MOB_DESC(1, trap, 16, RACE_PREDATOR,
	"trappola worg fossa",
	"Worg della Fossa",
	"Un worg balza da una fossa coperta.",
	"Zanne pronte a strappare la gola.",
	"Un ringhio famelico di worg di fossa ti circonda.",
	"Odi in lontananza un ringhio famelico di worg.")

MOB_DESC(1, trap, 17, RACE_GHOST,
	"trappola ombra maledetta",
	"Ombra Maledetta",
	"Un'ombra maledetta si stacca dal pavimento.",
	"Un freddo che non appartiene a questo mondo.",
	"Un sussurro gelido di ombra maledetta ti sfiora.",
	"In lontananza un sussurro gelido si trascina.")

MOB_DESC(1, trap, 18, RACE_ELEMENT,
	"trappola fulmine runico",
	"Fulmine Runico",
	"Rune sul pavimento scatenano fulmini a catena.",
	"Odore di ozono riempie la stanza.",
	"Un tuono runico improvviso vibra nella sala.",
	"Da qui vicino arriva un tuono runico improvviso.")

MOB_DESC(1, trap, 19, RACE_PARASITE,
	"trappola parassita mimetico",
	"Parassita Mimetico",
	"Un parassita mimetico si stacca dal soffitto come stalattite.",
	"Le ventose si attaccano prima che tu reagisca.",
	"Schizzi viscidi di un parassita mimetico ti cadono addosso.",
	"Odi in lontananza schizzi viscosi di qualcosa che striscia.")

};

#undef MOB_DESC

#endif /* __PROCAREA_MOB_DESC_INC */
