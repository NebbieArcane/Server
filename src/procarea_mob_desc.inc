/* procarea_mob_desc.inc
 * Testi mob Dimensione Effimera (225 archetipi).
 * Generato da scripts/generate-procarea-mobs.py — non editare a mano.
 *
 * short_title: titolo senza articolo; procarea.cpp aggiunge Un/Una/Lo/L'.
 * long_desc: frase di presenza in stanza (con articolo).
 * look / sound: procarea.cpp aggiunge \n\r in spawn.
 * agg: messaggio aggressivita' (player.sounds).
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
	"L'Alfa ringhia e la nebbia si addensa.",
	"Un ringhio squarcia il silenzio.")

MOB_DESC(1, boss, 1, RACE_ARACHNID,
	"regina tarantole",
	"Regina delle Tarantole",
	"Una tarantola colossale domina un nido di seta e ossa.",
	"Setole urticanti coprono zampe spesse come tronchi.",
	"La Regina scatta lateralmente.",
	"Ragnatele vibrano.")

MOB_DESC(1, boss, 2, RACE_TREE,
	"guardiano muschio antico",
	"Guardiano di Muschio Antico",
	"Un colosso vegetale blocca la sala con radici animate.",
	"Muschio vivo pulsa come un secondo cuore.",
	"Il Guardiano schianta i pugni a terra.",
	"Radici scoppiano fuori.")

MOB_DESC(1, boss, 3, RACE_PREDATOR,
	"predatore cieco",
	"Predatore Cieco delle Profondita'",
	"Una bestia senza occhi percepisce ogni tuo movimento.",
	"Artigli lunghi hanno scritto segni sulle pareti.",
	"Il predatore si lancia all'istinto.",
	"Un urlo cavernoso.")

MOB_DESC(1, boss, 4, RACE_INSECT,
	"matriarca vermi",
	"Matriarca dei Vermi Bianchetti",
	"Una montagna gelatinosa regge un trono di larve.",
	"Vermi obbedienti si gettano contro gli intrusi.",
	"La Matriarca convoca vermi.",
	"Schizzi viscidi.")

MOB_DESC(1, boss, 5, RACE_BIRD,
	"signore pipistrelli",
	"Signore dei Pipistrelli Profondi",
	"Una creatura alata coronata di ossa volteggia bassa.",
	"Canini sporgono da un muso allungato e famelico.",
	"Il Signore piomba dal soffitto.",
	"Ali battono ovunque.")

MOB_DESC(1, boss, 6, RACE_INSECT,
	"capo saprofagi",
	"Capo dei Saprofagi",
	"Un insettoide putrefatto comanda sciami affamati.",
	"Mandibole gocciolano liquido nero corrosivo.",
	"Il Capo spruzza acido.",
	"Odore di carogne.")

MOB_DESC(1, boss, 7, RACE_ORC,
	"custode fosso",
	"Custode del Fosso",
	"Un guerriero rannicchiato sorveglia un baratro laterale.",
	"Armatura e' incrostata di fango secco e sangue.",
	"Il Custode spinge verso il vuoto.",
	"Pietre cadono nel baratro.")

MOB_DESC(1, boss, 8, RACE_INSECT,
	"re larve",
	"Re delle Larve",
	"Un sovrano viscido e' nato dal cumulo di generazioni.",
	"Occhi multipli si aprono e chiudono a ondate.",
	"Il Re convoca larve reali.",
	"Brusio innaturale.")

MOB_DESC(1, boss, 9, RACE_GHOST,
	"ombra cunicolo",
	"Ombra del Primo Cunicolo",
	"La prima ombra nata nella dimensione si stacca dal muro.",
	"Non ha volto: solo fame antica.",
	"L'Ombra ti avvolge.",
	"Il freddo morde le ossa.")

MOB_DESC(1, boss, 10, RACE_UNDEAD_LICH,
	"arconte catacombe",
	"Arconte delle Catacombe",
	"Un lich avvolto in fasce polverose troneggia su un sepolcro aperto.",
	"Runa necromantiche bruciano come brace nel vuoto degli occhi.",
	"L'Arconte scaglia un incantesimo gelido.",
	"Sussurri in lingue morte.")

MOB_DESC(1, boss, 11, RACE_UNDEAD_SPECTRE,
	"signore nazgul caduto",
	"Signore dei Nazgul Caduti",
	"Uno spettro corazzato di nero emerge dalla bruma come un re senza corona.",
	"Un anello spezzato gli pende dal collo spettrale.",
	"Il Signore urla un comando senza voce.",
	"Un fischio gelido tra le pietre.")

MOB_DESC(1, boss, 12, RACE_ARACHNID,
	"madre ragni ungol",
	"Madre dei Ragni di Ungol",
	"Una aracnide titanica attende nel buio filando seta nera.",
	"Occhi multipli brillano come stelle malevole.",
	"La Madre scaglia filamenti appiccicosi.",
	"Ragnatele si tendono all'improvviso.")

MOB_DESC(1, boss, 13, RACE_GOBLIN,
	"capobanda goblin profondita",
	"Capobanda Goblin delle Profondita'",
	"Un goblin bardato di ferro di recupero brandisce un'ascia dentata.",
	"Trofei di orecchie pendono dalla sua cintura.",
	"Il Capobanda fischia ai suoi.",
	"Risate acute e feroci.")

MOB_DESC(1, boss, 14, RACE_TROLL,
	"signore troll pietra",
	"Signore Troll di Pietra",
	"Un troll massiccio si rialza da una pozza di sangue coagulato.",
	"La pelle sembra granito vivo; le ferite si richiudono da sole.",
	"Il Signore Troll schianta il pugno a terra.",
	"Pietre rotolano nel buio.")

MOB_DESC(1, boss, 15, RACE_UNDEAD_WIGHT,
	"custode barrow wight",
	"Custode Barrow-Wight",
	"Un guerriero sepolcrale sorge da un tumulo di ossa antiche.",
	"Spada arrugginita e corona spezzata lo contraddistinguono.",
	"Il Custode invoca il gelo del sepolcro.",
	"Un lamento di tombe profonde.")

MOB_DESC(1, boss, 16, RACE_DRAGON_GREEN,
	"drago caverne giade",
	"Drago delle Caverne Giade",
	"Un drago giovane ma feroce riempie la caverna di vapore acido.",
	"Scaglie verdastre raccolgono condensa velenosa.",
	"Il Drago soffia un alito corrosivo.",
	"Un ruggito fa tremare la volta.")

MOB_DESC(1, boss, 17, RACE_BIRD,
	"matriarca arpie rupi",
	"Matriarca Arpia delle Rupi",
	"Una arpia regale plana su artigli insanguinati.",
	"Piume nere e occhi d'ambra la rendono un incubo alato.",
	"La Matriarca piomba urlando.",
	"Un grido straziante echeggia.")

MOB_DESC(1, boss, 18, RACE_ORC,
	"signore uruk profondi",
	"Signore degli Uruk Profondi",
	"Un capitano uruk-corvo brandisce una lama larga come un'ascia.",
	"Runa di guerra bruciano sulla sua pelle grigio-ferro.",
	"Il Signore ordina la carica.",
	"Cori di guerra sotterranei.")

MOB_DESC(1, boss, 19, RACE_TREE,
	"ent corrotto antico",
	"Antico Ent Corrotto",
	"Un ent contorto dalla bruma torce rami come mazze.",
	"Corteccia nera stilla linfa velenosa.",
	"L'Ent schianta i rami verso di te.",
	"Legno che scricchiola minaccioso.")

MOB_DESC(1, boss, 20, RACE_DEMON,
	"demone fessure",
	"Demone delle Fessure",
	"Un demone cornuto striscia fuori da una crepa nel muro.",
	"Zolfo e cenere gli aleggiano attorno.",
	"Il Demone scaglia fiamme infernali.",
	"Odore di zolfo bruciacchiato.")

MOB_DESC(1, boss, 21, RACE_UNDEAD_VAMPIRE,
	"signore vampiro bruma",
	"Signore Vampiro della Bruma",
	"Un vampiro aristocratico emerge dalla nebbia con mantello lacero.",
	"Canini affilati e occhi cremisi non tradiscono eta'.",
	"Il Signore si scaglia famelico.",
	"Un sussurro seducente e mortale.")

MOB_DESC(1, boss, 22, RACE_GIANT_HILL,
	"gigante radici",
	"Gigante delle Radici",
	"Un gigante delle colline blocca il passaggio con un tronco nodoso.",
	"Radici gli crescono dalle caviglie fino al ginocchio.",
	"Il Gigante schianta il tronco.",
	"Passi che fanno tremare la sala.")

MOB_DESC(1, boss, 23, RACE_ELEMENT,
	"signore elementale fango",
	"Signore Elementale del Fango",
	"Una massa di fango senziente assume forma umanoide.",
	"Pietre e ossa galleggiano nella sua corrente viscida.",
	"Il Signore inghiotte il suolo sotto i tuoi piedi.",
	"Schizzi di fango ovunque.")

MOB_DESC(1, boss, 24, RACE_DARK_ELF,
	"regina rova nera",
	"Regina della Rova Nera",
	"Una drow incappucciata troneggia su un trono di stalattiti.",
	"Lame di ossidiana le scintillano attorno come aureola.",
	"La Regina ordina l'esecuzione.",
	"Incantesimi sibilati nel buio.")

MOB_DESC(1, boss, 25, RACE_DEVIL,
	"signore culto abissale",
	"Signore del Culto Abissale",
	"Un diavolo cornuto conduce un coro di fedeli mutati.",
	"Simboli infernali bruciano sulla pietra circostante.",
	"Il Signore invoca fiamme profane.",
	"Canti distorti e irreali.")

MOB_DESC(1, boss, 26, RACE_GHOST,
	"oracolo corrotto moria",
	"Oracolo Corrotto di Moria",
	"Uno spirito di saggio nano urla profezie nel buio.",
	"Libri fluttuano attorno a lui come lune orbitanti.",
	"L'Oracolo scaglia rune spettrali.",
	"Echi di martelli lontani.")

MOB_DESC(1, boss, 27, RACE_SPECIAL,
	"signore mimiche",
	"Signore delle Mimiche",
	"Una creatura che imita trono e tesoro attende immobile.",
	"La pelle sembra metallo finche' non si apre a bocca piena.",
	"Il Signore si slancia con fauci d'acciaio.",
	"Un tonfo sordo di carne che diventa pietra.")

MOB_DESC(1, boss, 28, RACE_DEMON,
	"custode balrog minore",
	"Custode del Balrog Minore",
	"Una creatura di fuoco e ombra brandisce una frusta di fiamme.",
	"Ali carbonizzate spalancano calore insopportabile.",
	"Il Custode flagella l'aria ardente.",
	"Un ruggito di magma sotterraneo.")

MOB_DESC(1, boss, 29, RACE_UNDEAD_GHAST,
	"sovrano ghoul ossa",
	"Sovrano Ghoul delle Ossa",
	"Un ghoul reale si rialza da un trono di femori intrecciati.",
	"Artigli neri e lingua biforcuta gocciolano ichor.",
	"Il Sovrano convoca ghoul affamati.",
	"Masticare ossa nel buio.")

MOB_DESC(1, boss, 30, RACE_DRAGON_RED,
	"drago rosso antico",
	"Drago Rosso Antico",
	"Un drago rosso antico riempie la sala di calore soffocante.",
	"Scaglie come metallo incandescente e occhi di magma.",
	"Il Drago soffia un inferno.",
	"Il suolo trema sotto le fiamme.")

MOB_DESC(1, boss, 31, RACE_DRAGON_BLACK,
	"drago nero corrotto",
	"Drago Nero Corrotto",
	"Un drago nero corrotto dalla bruma striscia fuori dall'oscurita'.",
	"Alito acido corrode la pietra circostante.",
	"Il Drago Nero attacca.",
	"Ruggito che squarcia la roccia.")

MOB_DESC(1, boss, 32, RACE_DRAGON_WHITE,
	"drago bianco gelido",
	"Drago Bianco Gelido",
	"Un drago bianco scende da vortici di neve eterna.",
	"Ghiaccio si forma su ogni superficie al suo passaggio.",
	"Il Drago Bianco congela l'aria.",
	"Vento gelido tagliente.")

MOB_DESC(1, boss, 33, RACE_DRAGON_GOLD,
	"drago dorato caduto",
	"Drago Dorato Caduto",
	"Un drago dorado decaduto troneggia su tesori illusori.",
	"Scaglie opache nascondono ancora potere immenso.",
	"Il Drago Dorado attacca.",
	"Ruggito metallico.")

MOB_DESC(1, boss, 34, RACE_DRAGON_GREEN,
	"signore drago verde",
	"Signore Drago Verde",
	"Un signore drago verde avvolge la caverna di gas velenoso.",
	"Ali spalancate oscurano torce e speranza.",
	"Il Signore Drago piomba.",
	"Alito corrosivo.")

MOB_DESC(1, boss, 35, RACE_UNDEAD_SPECTRE,
	"re nazgul bruma",
	"Re Nazgul della Bruma",
	"Un re spettrale in armatura nera alza una lama senza luce.",
	"Corona spezzata e terrore antico precedono ogni passo.",
	"Il Re Nazgul ordina la morte.",
	"Un grido che gela il sangue.")

MOB_DESC(1, boss, 36, RACE_UNDEAD_SPECTRE,
	"signore anello spezzato",
	"Signore dell'Anello Spezzato",
	"Uno spettro corazzato porta un anello spezzato al dito.",
	"La sua sola presenza spegne le fiamme.",
	"Il Signore avanza implacabile.",
	"Sussurro di obbedienza.")

MOB_DESC(1, boss, 37, RACE_UNDEAD_LICH,
	"lich re ossario",
	"Lich Re dell'Ossuario",
	"Un lich reale troneggia su un trono di teschi impilati.",
	"Corona di ossa e staffa runica pulsano di oscurita'.",
	"Il Lich Re scaglia morte.",
	"Incantesimi che gelano l'anima.")

MOB_DESC(1, boss, 38, RACE_DEMON,
	"principe demone abisso",
	"Principe Demone dell'Abisso",
	"Un principe demone emerge da una fessura ardente.",
	"Ali carbonizzate e zolfo riempiono la stanza.",
	"Il Principe flagella con fuoco infernale.",
	"Odore di zolfo e paura.")

MOB_DESC(1, boss, 39, RACE_DEVIL,
	"arconte infernale",
	"Arconte Infernale",
	"Un arconte infernale conduce un coro di dannati.",
	"Contratti scritti con sangue fluttuano attorno a lui.",
	"L'Arconte invoca catene infuocate.",
	"Canti profani.")

MOB_DESC(1, boss, 40, RACE_GIANT_STORM,
	"gigante tempesta",
	"Gigante della Tempesta",
	"Un gigante della tempesta scuote la volta con ogni passo.",
	"Fulmini danzano tra le sue dita massicce.",
	"Il Gigante scaglia saette.",
	"Tuono sordo continuo.")

MOB_DESC(1, boss, 41, RACE_GIANT_FIRE,
	"gigante fuoco",
	"Gigante del Fuoco",
	"Un gigante del fuoco arde come un vulcano ambulante.",
	"Pietra fusa gocciola dai suoi pugni.",
	"Il Gigante incendia la sala.",
	"Calore insopportabile.")

MOB_DESC(1, boss, 42, RACE_GIANT_FROST,
	"gigante gelo",
	"Gigante del Gelo",
	"Un gigante del gelo congela l'aria con un solo sguardo.",
	"Barba di ghiaccio e occhi come iceberg.",
	"Il Gigante colpisce con pugni gelidi.",
	"Ghiaccio che scricchiola.")

MOB_DESC(1, boss, 43, RACE_MFLAYER,
	"arconte illithid",
	"Arconte Illithid",
	"Un arconte illithid fluttua con tentacoli regali.",
	"Menti deboli collassano al suo sguardo.",
	"L'Arconte attacca la mente.",
	"Pulsazioni psichiche.")

MOB_DESC(1, boss, 44, RACE_PLANAR,
	"signore abisso stellare",
	"Signore dell'Abisso Stellare",
	"Una creatura planare distorce lo spazio attorno a se'.",
	"Geometrie impossibili orbitano nel suo velo.",
	"Il Signore piega la realta'.",
	"Echi non euclidei.")

MOB_DESC(1, boss, 45, RACE_UNDEAD_VAMPIRE,
	"gran signore vampiro",
	"Gran Signore Vampiro",
	"Un gran signore vampiro sorge da un sarcofago di marmo nero.",
	"Potere antico e sete insaziabile lo avvolgono.",
	"Il Gran Signore attacca.",
	"Sussurro che ipnotizza.")

MOB_DESC(1, boss, 46, RACE_ORC,
	"signore uruk nero",
	"Signore Uruk-Nero",
	"Un signore uruk-nero brandisce una spada enorme.",
	"Stendardo nero e occhi come brace.",
	"Il Signore ordina la strage.",
	"Cori di guerra.")

MOB_DESC(1, boss, 47, RACE_TROLL,
	"signore troll montagna",
	"Signore Troll di Montagna",
	"Un signore troll delle montagne riempie l'intera sala.",
	"Ogni passo fa crollare stalattiti.",
	"Il Signore Troll schianta.",
	"Ruggito tellurico.")

MOB_DESC(1, boss, 48, RACE_ARACHNID,
	"regina ragni antica",
	"Regina Ragni Antica",
	"Una regina ragni antica tesse un trono di seta e ossa.",
	"Figliame infinito si muove nelle ombre.",
	"La Regina convoca ragni.",
	"Ragnatele ovunque.")

MOB_DESC(1, boss, 49, RACE_TREE,
	"signore ent corrotto",
	"Signore Ent Corrotto",
	"Un signore ent corrotto torce rami come torri.",
	"Linfa nera stilla dal tronco.",
	"Il Signore Ent schianta.",
	"Legno che urla.")

MOB_DESC(1, boss, 50, RACE_ELEMENT,
	"signore elementale tempesta",
	"Signore Elementale della Tempesta",
	"Un signore elementale della tempesta riempie la sala di vento.",
	"Fulmini e pioggia obbediscono al suo gesto.",
	"Il Signore scatena la tempesta.",
	"Tuono continuo.")

MOB_DESC(1, boss, 51, RACE_DARK_ELF,
	"matrona drow",
	"Matrona Drow",
	"Una matrona drow troneggia su un altare di ossidiana.",
	"Ragni sacri le tessono il manto.",
	"La Matrona ordina la morte.",
	"Incantesimi sibilati.")

MOB_DESC(1, boss, 52, RACE_GHOST,
	"re spettro corone",
	"Re Spettro delle Corone",
	"Un re spettro delle corone perdute fluttua sopra un trono.",
	"Corone spezzate orbitano come lune morte.",
	"Il Re Spettro invoca il gelo.",
	"Lamenti regali.")

MOB_DESC(1, boss, 53, RACE_DEMON,
	"balrog antico",
	"Balrog Antico",
	"Un balrog antico sorge tra fiamme e ombra.",
	"Frusta infuocata e spada di fuoco squarciando il buio.",
	"Il Balrog flagella.",
	"Ruggito di magma.")

MOB_DESC(1, boss, 54, RACE_DRAGON_SILVER,
	"drago argento lunare",
	"Drago d'Argento Lunare",
	"Un drago d'argento riflette luce lunare impossibile.",
	"Ali argentee e occhi pieni di maree.",
	"Il Drago d'Argento attacca.",
	"Luce accecante.")

MOB_DESC(1, mob, 0, RACE_GOBLIN,
	"goblin esploratore",
	"Goblin Esploratore",
	"Un goblin magro sbuca da un crepaccio con pugnale arrugginito.",
	"Orecchie tagliate pendono al collo come trofei.",
	"Il goblin si lancia urlando.",
	"Un urlo acuto.")

MOB_DESC(1, mob, 1, RACE_GOBLIN,
	"goblin sciamano",
	"Goblin Sciamano",
	"Un goblin coperto di rune brandisce un bastone di ossa.",
	"Occhi gialli brillano di follia shamanica.",
	"Lo sciamano invoca fulmini verdi.",
	"Mormorii gutturali.")

MOB_DESC(1, mob, 2, RACE_GOBLIN,
	"goblin arcere",
	"Goblin Arciere",
	"Un goblin armato di arco corto mira dall'ombra.",
	"Frecce avvelenate spuntano dal carquois.",
	"L'arcere scocca una freccia.",
	"Frecce che fischiano.")

MOB_DESC(1, mob, 3, RACE_GOBLIN,
	"goblin speleologo",
	"Goblin Speleologo",
	"Un goblin con lanterna di tartaruga esplora i cunicoli.",
	"Corde e uncini gli pendono dalla cintura.",
	"Lo speleologo attacca con uncino.",
	"Metallo che striscia sulla roccia.")

MOB_DESC(1, mob, 4, RACE_GOBLIN,
	"goblin ladro",
	"Goblin Ladro",
	"Un goblin furtivo fruga tra i detriti con occhi furbi.",
	"Dita nere e veloci non lasciano traccia.",
	"Il ladro pugnala alle spalle.",
	"Risate soffocate.")

MOB_DESC(1, mob, 5, RACE_GOBLIN,
	"hobgoblin soldato",
	"Hobgoblin Soldato",
	"Un hobgoblin in armatura di cuoio brandisce spada e scudo.",
	"Disciplina crudele si legge nei suoi occhi rossi.",
	"Il soldato carica in formazione.",
	"Passi marziali.")

MOB_DESC(1, mob, 6, RACE_GOBLIN,
	"hobgoblin capitano",
	"Hobgoblin Capitano",
	"Un hobgoblin bardato conduce un manipolo inferiore.",
	"Stendardo strappato ancora gli cinge il petto.",
	"Il capitano ordina l'assalto.",
	"Comandi ruvidi.")

MOB_DESC(1, mob, 7, RACE_ORC,
	"orc guerriero",
	"Orco Guerriero",
	"Un orco muscoloso ringhia mostrando zanne incarnate.",
	"Cicatrici rituali solcano il petto nudo.",
	"L'orco si scaglia con ascia.",
	"Grugniti di guerra.")

MOB_DESC(1, mob, 8, RACE_ORC,
	"orc berserker",
	"Orco Berserker",
	"Un orco impazzito da battaglia schiuma alla bocca.",
	"Ascia a due mani sollevata, occhi fuori orbita.",
	"Il berserker entra in furia.",
	"Urla di sangue.")

MOB_DESC(1, mob, 9, RACE_HALF_ORC,
	"uruk corsiero",
	"Uruk Corsiero",
	"Un uruk-corvo corre senza stancarsi lungo i corridoi.",
	"Elmo nero e lama larga lo rendono un'ombra letale.",
	"L'uruk piomba con lama bassa.",
	"Passi pesanti e rapidi.")

MOB_DESC(1, mob, 10, RACE_ORC,
	"orc sciamano",
	"Orco Sciamano",
	"Un orco coperto di ossa invoca spiriti della caverna.",
	"Tatuaggi neri pulsano quando canta.",
	"Lo sciamano scaglia maledizioni.",
	"Canti gutturali.")

MOB_DESC(1, mob, 11, RACE_ORC,
	"orc guardia",
	"Orco Guardia",
	"Un orco armato sorveglia un varco stretto.",
	"Lancia arrugginita pronta a trafiggere.",
	"La guardia blocca il passaggio.",
	"Grugnito di sfida.")

MOB_DESC(1, mob, 12, RACE_ORC,
	"orc spezzaneve",
	"Orco Spezzaneve",
	"Un orco delle montagne emerge dal ghiaccio sotterraneo.",
	"Pelliccia imbrattata di sangue e brina.",
	"Lo spezzaneve colpisce con mazza.",
	"Vento gelido.")

MOB_DESC(1, mob, 13, RACE_HALF_ORC,
	"mezzorco reietto",
	"Mezzorco Reietto",
	"Un mezzorco emarginato combatte con disperazione.",
	"Occhi tristi e pugno di ferro.",
	"Il reietto attacca senza paura.",
	"Ringhio sommesso.")

MOB_DESC(1, mob, 14, RACE_GOBLIN,
	"goblin furtivo",
	"Goblin Furtivo",
	"Un goblin si mimetizza tra le stalattiti.",
	"Pelle grigia lo confonde con la roccia.",
	"Il furtivo colpisce dall'alto.",
	"Pietre che cadono.")

MOB_DESC(1, mob, 15, RACE_TROGMAN,
	"troglodita guerriero",
	"Troglodita Guerriero",
	"Un uomo-delle-caverne brandisce clava di osso.",
	"Pelle pallida e occhi ciechi al sole.",
	"Il troglodita colpisce alla cieca.",
	"Grugniti cavernosi.")

MOB_DESC(1, mob, 16, RACE_TROGMAN,
	"troglodita cacciatore",
	"Troglodita Cacciatore",
	"Un cacciatore sotterraneo trascina una lancia di selce.",
	"Odore di muschio e carne cruda.",
	"Il cacciatore trafigge con lancia.",
	"Passi silenziosi.")

MOB_DESC(1, mob, 17, RACE_ORC,
	"orc flagellatore",
	"Orco Flagellatore",
	"Un orco sadico brandisce frusta chiodata.",
	"Sorriso crudele sotto elmo deformato.",
	"Il flagellatore frusta l'aria.",
	"Scroscio di frusta.")

MOB_DESC(1, mob, 18, RACE_GOBLIN,
	"goblin alchimista",
	"Goblin Alchimista",
	"Un goblin maneggia fiale fumanti con cautela.",
	"Esplosioni recenti hanno bruciato metade del volto.",
	"L'alchimista lancia acido.",
	"Vetro che si rompe.")

MOB_DESC(1, mob, 19, RACE_HALF_ORC,
	"uruk sentinella",
	"Uruk Sentinella",
	"Un uruk immobile veglia su un ponte di pietra.",
	"Lancia lunga e scudo nero coprono ogni angolo.",
	"La sentinella carica.",
	"Armatura che stride.")

MOB_DESC(1, mob, 20, RACE_UNDEAD_SKELETON,
	"scheletro guerriero",
	"Scheletro Guerriero",
	"Uno scheletro armato si rialza dal cumulo di ossa.",
	"Costole vuote battono come tamburi.",
	"Lo scheletro falcia con spada.",
	"Ossa che sbattono.")

MOB_DESC(1, mob, 21, RACE_UNDEAD_SKELETON,
	"scheletro arciere",
	"Scheletro Arciere",
	"Uno scheletro tende un arco di ossa e tendini.",
	"Frecce di metallo arrugginito puntano verso di te.",
	"L'arciere scocca.",
	"Frecce nel buio.")

MOB_DESC(1, mob, 22, RACE_UNDEAD_ZOMBIE,
	"zombie contadino",
	"Zombie Contadino",
	"Un cadavere rigido avanza con braccia tese.",
	"Vestiti lacera e occhi vitrei.",
	"Lo zombie cerca di afferrarti.",
	"Gemiti soffocati.")

MOB_DESC(1, mob, 23, RACE_UNDEAD_ZOMBIE,
	"zombie soldato",
	"Zombie Soldato",
	"Un soldato morto marcia ancora con spada in mano.",
	"Armatura ammaccata copre carne putrefatta.",
	"Lo zombie colpisce goffamente.",
	"Armatura che stride.")

MOB_DESC(1, mob, 24, RACE_UNDEAD_GHOUL,
	"ghoul famelico",
	"Ghoul Famelico",
	"Un ghoul striscia a quattro zampe verso di te.",
	"Artigli neri e lingua biforcuta.",
	"Il ghoul morde famelico.",
	"Masticare carne.")

MOB_DESC(1, mob, 25, RACE_UNDEAD_GHOUL,
	"ghoul necrofago",
	"Ghoul Necrofago",
	"Un ghoul rovista tra tombe aperte.",
	"Denti affilati gocciolano ichor nero.",
	"Il necrofago si slancia.",
	"Odore di sepolcro.")

MOB_DESC(1, mob, 26, RACE_UNDEAD_WIGHT,
	"wight guerriero",
	"Wight Guerriero",
	"Un wight spettrale brandisce spada gelida.",
	"Aura di freddo mortale lo circonda.",
	"Il wight colpisce con spada.",
	"Freddo che penetra le ossa.")

MOB_DESC(1, mob, 27, RACE_UNDEAD_SPECTRE,
	"spettro errante",
	"Spettro Errante",
	"Uno spettro fluttuata attraverso le pareti.",
	"Voce sussurrante promette morte.",
	"Lo spettro attraversa la tua difesa.",
	"Sussurri gelidi.")

MOB_DESC(1, mob, 28, RACE_GHOST,
	"fantasma prigioniero",
	"Fantasma Prigioniero",
	"Un fantasma incatenato alla caverna urla silenziosamente.",
	"Catene spettrali gli legano polsi e caviglie.",
	"Il fantasma stritola con catene.",
	"Lamentele ovattate.")

MOB_DESC(1, mob, 29, RACE_UNDEAD_LICH,
	"lich minore",
	"Lich Minore",
	"Un lich incompleto alza mani scheletriche.",
	"Pergamene bruciano attorno a lui.",
	"Il lich scaglia dardi oscuri.",
	"Incantesimi sibilati.")

MOB_DESC(1, mob, 30, RACE_UNDEAD_SKELETON,
	"scheletro guardia",
	"Scheletro Guardia",
	"Uno scheletro in armatura sorveglia un arco.",
	"Elmo vuoto fissa il vuoto con occhi di brace.",
	"La guardia blocca il passaggio.",
	"Spade che si incrociano.")

MOB_DESC(1, mob, 31, RACE_UNDEAD_ZOMBIE,
	"zombie ogre",
	"Zombie Ogre",
	"Un ogre morto si trascina con mazza imponente.",
	"Carne verde e gonfia colma le ferite.",
	"Lo zombie ogre schianta la mazza.",
	"Passi pesanti e morti.")

MOB_DESC(1, mob, 32, RACE_UNDEAD_GHOUL,
	"ghoul delle profondita",
	"Ghoul delle Profondita'",
	"Un ghoul emaciato striscia dalle fessure.",
	"Occhi gialli brillano nel buio.",
	"Il ghoul attacca in branco.",
	"Ringhi famelici.")

MOB_DESC(1, mob, 33, RACE_UNDEAD_WIGHT,
	"wight sepolcrale",
	"Wight Sepolcrale",
	"Un wight sorge da un sarcofago rotto.",
	"Polsi scheletrici stringono una corona.",
	"Il wight invoca gelo.",
	"Vento gelido dal sepolcro.")

MOB_DESC(1, mob, 34, RACE_UNDEAD_SPECTRE,
	"spettro guerriero",
	"Spettro Guerriero",
	"Uno spettro in armatura antica fluttua minaccioso.",
	"Spada spettrale attraversa la roccia.",
	"Lo spettro falcia.",
	"Lame che fischiano nel vuoto.")

MOB_DESC(1, mob, 35, RACE_GHOST,
	"anima perduta",
	"Anima Perduta",
	"Un'anima perduta vaga senza meta tra i corridoi.",
	"Luce pallida la rende quasi trasparente.",
	"L'anima ti attraversa gelida.",
	"Pianto lontano.")

MOB_DESC(1, mob, 36, RACE_UNDEAD_VAMPIRE,
	"vampiro servitore",
	"Vampiro Servitore",
	"Un vampiro giovane serve un padrone piu' antico.",
	"Mantello nero e occhi affamati.",
	"Il servitore morde rapidamente.",
	"Sussurro seducente.")

MOB_DESC(1, mob, 37, RACE_UNDEAD_SKELETON,
	"scheletro chierico",
	"Scheletro Chierico",
	"Uno scheletro in tonaca invoca l'oscurita'.",
	"Simboli sacri corrotti brillano sul petto.",
	"Lo scheletro scaglia maledizioni.",
	"Preghiere invertite.")

MOB_DESC(1, mob, 38, RACE_UNDEAD_ZOMBIE,
	"zombie paludoso",
	"Zombie Paludoso",
	"Un cadavere paludoso emerge da pozze viscide.",
	"Alghe gli pendono dalle ferite.",
	"Lo zombie afferra con mani viscide.",
	"Schizzi di acqua fetida.")

MOB_DESC(1, mob, 39, RACE_UNDEAD_GHAST,
	"ghast affamato",
	"Ghast Affamato",
	"Un ghast piu' potente di un ghoul striscia verso di te.",
	"Alito putrefatto precede l'attacco.",
	"Il ghast paralizza con lo sguardo.",
	"Odore insopportabile.")

MOB_DESC(1, mob, 40, RACE_ARACHNID,
	"ragno caverna",
	"Ragno di Caverna",
	"Un ragno grosso come un cane attende nella ragnatela.",
	"Occhi neri multipli ti scrutano.",
	"Il ragno scatta.",
	"Ragnatele che si tendono.")

MOB_DESC(1, mob, 41, RACE_ARACHNID,
	"ragno veleno",
	"Ragno Velenoso",
	"Un ragno dalle zampe striate di giallo si muove rapido.",
	"Gocce di veleno brillano alle zanne.",
	"Il ragno morde.",
	"Sibilo acuto.")

MOB_DESC(1, mob, 42, RACE_ARACHNID,
	"ragno gigante",
	"Ragno Gigante",
	"Un ragno gigante copre metà del soffitto.",
	"Seta fresca ancora umida di prede.",
	"Il ragno avvolge in seta.",
	"Ragnatele che vibrano.")

MOB_DESC(1, mob, 43, RACE_INSECT,
	"scarabeo corazza",
	"Scarabeo Corazzato",
	"Uno scarabeo grande come un vassoio avanza.",
	"Carapace lucido devia le lame.",
	"Lo scarabeo carica.",
	"Ronzio profondo.")

MOB_DESC(1, mob, 44, RACE_INSECT,
	"centopiedi caverna",
	"Centopiedi di Caverna",
	"Un centopiedi lungo come un braccio striscia sulle pareti.",
	"Mandibole gocciolano veleno.",
	"Il centopiedi morde.",
	"Scricchiolio di chitin.")

MOB_DESC(1, mob, 45, RACE_INSECT,
	"formica gigante",
	"Formica Gigante",
	"Una formica gigante trasporta un pezzo di armatura.",
	"Mandibole possono tranciare ossa.",
	"La formica morde.",
	"Ronzio di sciame.")

MOB_DESC(1, mob, 46, RACE_INSECT,
	"mosca necrofaga",
	"Mosca Necrofaga",
	"Una mosca grande come un pugno ronzia attorno.",
	"Occhi composti riflettono ogni movimento.",
	"La mosca punge.",
	"Ronzio insistente.")

MOB_DESC(1, mob, 47, RACE_INSECT,
	"grillo caverna",
	"Grillo di Caverna",
	"Un grillo nero grande come un gatto salta.",
	"Antenne tese captano ogni suono.",
	"Il grillo morde.",
	"Stridio acuto.")

MOB_DESC(1, mob, 48, RACE_ARACHNID,
	"vedova nera",
	"Vedova Nera",
	"Una vedova nera grossa attende immobile.",
	"Segno rosso sul dorso avverte pericolo.",
	"La vedova scatta.",
	"Silenzio prima del morso.")

MOB_DESC(1, mob, 49, RACE_INSECT,
	"sciamatore caverna",
	"Sciamatore di Caverna",
	"Un insetto corazzato fruga tra le pietre con antenne tese.",
	"Mandibole lucide hanno morsicato ossa recenti.",
	"Lo sciamatore batte le ali.",
	"Un ronzio acuto riempie la stanza.")

MOB_DESC(1, mob, 50, RACE_INSECT,
	"larva nebbia effimera",
	"Larva della Nebbia Effimera",
	"Una larva traslucida striscia lasciando una scia di vapore gelido.",
	"La pelle viscida pulsa come se respirasse nebbia compressa.",
	"La larva si inarca minacciosa.",
	"Senti un sibilo umido.")

MOB_DESC(1, mob, 51, RACE_INSECT,
	"verme bianchetto",
	"Verme Bianchetto",
	"Un verme bianco grosso come un braccio emerge dal fango.",
	"Pelle gelatinosa brilla di umidita'.",
	"Il verme si avvolge.",
	"Schizzi viscidi.")

MOB_DESC(1, mob, 52, RACE_INSECT,
	"mosca stirge",
	"Stirge delle Profondita'",
	"Una stirge alata plana con proboscide sanguinante.",
	"Ali sottili e occhi rossi.",
	"La stirge si attacca.",
	"Ronzio famelico.")

MOB_DESC(1, mob, 53, RACE_ARACHNID,
	"tick gigante",
	"Tick Gigante",
	"Un tick grosso come un piatto attende su una roccia.",
	"Corpo gonfio di sangue altrui.",
	"Il tick morde.",
	"Schiocco umido.")

MOB_DESC(1, mob, 54, RACE_INSECT,
	"mantide caverna",
	"Mantide di Caverna",
	"Una mantide verde scuro ferma le zampe a forbice.",
	"Sguardo alieno non tradisce emozione.",
	"La mantide falcia.",
	"Fruscio secco.")

MOB_DESC(1, mob, 55, RACE_INSECT,
	"ape caverna",
	"Ape di Caverna",
	"Un'ape grossa come un pugno ronzia minacciosa.",
	"Pungiglione lungo gocciola veleno.",
	"L'ape punge.",
	"Ronzio furioso.")

MOB_DESC(1, mob, 56, RACE_INSECT,
	"locusta bruna",
	"Locusta Bruna",
	"Una locusta grossa come un falco plana bassa.",
	"Mandibole possono tagliare cuoio.",
	"La locusta morde.",
	"Ali che frusciano.")

MOB_DESC(1, mob, 57, RACE_ARACHNID,
	"ragno cacciatore",
	"Ragno Cacciatore",
	"Un ragno veloce corre sulle pareti.",
	"Veleno paralizzante nelle zanne.",
	"Il cacciatore salta.",
	"Passi leggeri sulla roccia.")

MOB_DESC(1, mob, 58, RACE_INSECT,
	"coleottero nero",
	"Coleottero Nero",
	"Un coleottero nero lucido grande come un gatto.",
	"Corna possono sollevare un uomo.",
	"Il coleottero carica.",
	"Ronzio metallico.")

MOB_DESC(1, mob, 59, RACE_INSECT,
	"scarafaggio caverna",
	"Scarafaggio di Caverna",
	"Uno scarafaggio immenso striscia dal buio.",
	"Antenne captano ogni odore.",
	"Lo scarafaggio attacca.",
	"Fruscio viscido.")

MOB_DESC(1, mob, 60, RACE_SNAKE,
	"vipera caverna",
	"Vipera di Caverna",
	"Una vipera striata si avvolge su una roccia calda.",
	"Zanne gocciolano veleno giallo.",
	"La vipera morde.",
	"Sibilo secco.")

MOB_DESC(1, mob, 61, RACE_SNAKE,
	"serpente ossa",
	"Serpente delle Ossa",
	"Un serpente bianco striscia tra teschi.",
	"Occhi rosa privi di pupilla.",
	"Il serpente si avvolge.",
	"Ossa che rotolano.")

MOB_DESC(1, mob, 62, RACE_REPTILE,
	"lucertola gigante",
	"Lucertola Gigante",
	"Una lucertola grossa come un cane corre sulle pareti.",
	"Coda spessa colpisce come frusta.",
	"La lucertola morde.",
	"Artigli sulla roccia.")

MOB_DESC(1, mob, 63, RACE_REPTILE,
	"rana pietra",
	"Rana di Pietra",
	"Una rana massiccia mimetizzata tra i massi attende immobile.",
	"La pelle sembra pietra viva; solo gli occhi si muovono.",
	"La rana si scaglia con la lingua.",
	"Un colpo secco sulle pietre.")

MOB_DESC(1, mob, 64, RACE_REPTILE,
	"geco caverna",
	"Geco di Caverna",
	"Un geco enorme mimetizzato attende sul soffitto.",
	"Pelle cambia colore con la roccia.",
	"Il geco cade addosso.",
	"Fruscio improvviso.")

MOB_DESC(1, mob, 65, RACE_SNAKE,
	"boa caverna",
	"Boa di Caverna",
	"Un boa grosso come un tronco blocca il corridoio.",
	"Muscoli poderosi si contraggono.",
	"Il boa si avvolge.",
	"Respiro sibilante.")

MOB_DESC(1, mob, 66, RACE_REPTILE,
	"coccodrillo sotterraneo",
	"Coccodrillo Sotterraneo",
	"Un coccodrillo albino emerge da una pozza.",
	"Pelle traslucida e occhi rossi.",
	"Il coccodrillo morde.",
	"Schizzi d'acqua.")

MOB_DESC(1, mob, 67, RACE_SNAKE,
	"aspide nero",
	"Aspide Nero",
	"Un serpente nero lucido si muove silenzioso.",
	"Veleno letale in gocce alle zanne.",
	"L'aspide morde rapidamente.",
	"Sibilo quasi impercettibile.")

MOB_DESC(1, mob, 68, RACE_REPTILE,
	"basilisco giovane",
	"Basilisco Giovane",
	"Un basilisco immature evita il tuo sguardo.",
	"Pelle grigia simula pietra.",
	"Il basilisco morde.",
	"Sibilo roco.")

MOB_DESC(1, mob, 69, RACE_SNAKE,
	"serpente nebbia",
	"Serpente della Nebbia",
	"Un serpente traslucido emerge dal vapore.",
	"Corpo sembra fatto di bruma solidificata.",
	"Il serpente morde gelido.",
	"Vapore che si condensa.")

MOB_DESC(1, mob, 70, RACE_PREDATOR,
	"predatore giovane",
	"Predatore delle Profondita'",
	"Un giovane predatore delle caverne ti fissa senza paura.",
	"Zanne non ancora mature, ma gia' affilate.",
	"Il predatore si prepara al balzo.",
	"Un ululato lontano.")

MOB_DESC(1, mob, 71, RACE_PREDATOR,
	"cane radici",
	"Cane delle Radici",
	"Un cane magro dalle zampe simili a radici ringhia basso.",
	"Pelliccia impregnata di terra e sangue secco.",
	"Il cane si lancia avanti.",
	"Un ringhio profondo.")

MOB_DESC(1, mob, 72, RACE_PREDATOR,
	"warg caverna",
	"Warg di Caverna",
	"Un warg grosso come un cavallo ringhia nel buio.",
	"Pelliccia nera e zanne da lupo infernale.",
	"Il warg carica.",
	"Ululato cavernoso.")

MOB_DESC(1, mob, 73, RACE_PREDATOR,
	"lupo nebbia",
	"Lupo della Nebbia",
	"Un lupo spettrale emerge dal vapore.",
	"Occhi argentati non tradiscono paura.",
	"Il lupo attacca in branco.",
	"Ululato nella bruma.")

MOB_DESC(1, mob, 74, RACE_PREDATOR,
	"pantera caverna",
	"Pantera di Caverna",
	"Una pantera nera scivola senza rumore.",
	"Occhi verdi brillano nel buio.",
	"La pantera piomba.",
	"Ringhio basso.")

MOB_DESC(1, mob, 75, RACE_PREDATOR,
	"ibrido worg",
	"Worg Ibrido",
	"Un worg corrotto dalla dimensione ringhia distorto.",
	"Zanne irregolari e saliva nera.",
	"Il worg si scaglia.",
	"Ringhio metallico.")

MOB_DESC(1, mob, 76, RACE_PREDATOR,
	"sciacallo profondita",
	"Sciacallo delle Profondita'",
	"Uno sciacallo emaciato segue il gruppo.",
	"Occhi furbi attendono la distrazione.",
	"Lo sciacallo morde alle caviglie.",
	"Guaito acuto.")

MOB_DESC(1, mob, 77, RACE_PREDATOR,
	"ibrido ratto gigante",
	"Ratto Gigante Corrotto",
	"Un ratto grosso come un cane mostra zanne innaturali.",
	"Pelliccia rada e occhi rossi.",
	"Il ratto attacca in branco.",
	"Squittii minacciosi.")

MOB_DESC(1, mob, 78, RACE_PREDATOR,
	"tigre caverna",
	"Tigre di Caverna",
	"Una tigre dalle strisce pallide attende.",
	"Muscoli tesi pronti al balzo.",
	"La tigre attacca.",
	"Ruggito sordo.")

MOB_DESC(1, mob, 79, RACE_PREDATOR,
	"ibrido iena",
	"Iena delle Profondita'",
	"Una iena macilenta ride mostrando fauci.",
	"Risata disturbante riempie la caverna.",
	"L'iena morde.",
	"Risata isterica.")

MOB_DESC(1, mob, 80, RACE_BIRD,
	"pipistrello vorace",
	"Pipistrello Vorace",
	"Un pipistrello grosso come un cane volteggia sopra la tua testa.",
	"Canini sporgono da un muso allungato e famelico.",
	"Il pipistrello piomba giu'.",
	"Ali scalciano l'aria.")

MOB_DESC(1, mob, 81, RACE_BIRD,
	"pipistrello vampiro",
	"Pipistrello Vampiro",
	"Un pipistrello enorme plana con muso porcino.",
	"Lingua sanguinante lecca l'aria.",
	"Il pipistrello morde.",
	"Ronzio famelico.")

MOB_DESC(1, mob, 82, RACE_BIRD,
	"corvo caverna",
	"Corvo di Caverna",
	"Un corvo grosso osserva dall'ombra.",
	"Piume nere assorbono la luce.",
	"Il corvo becca violentemente.",
	"Gracchio sinistro.")

MOB_DESC(1, mob, 83, RACE_BIRD,
	"caveira alata",
	"Caveira Alata",
	"Un uccello carogna grosso come un falco plana.",
	"Becco ad uncino pronto a squartare.",
	"La caveira attacca.",
	"Ali che battono.")

MOB_DESC(1, mob, 84, RACE_BIRD,
	"gufo caverna",
	"Gufo di Caverna",
	"Un gufo gigante fissa con occhi dorati.",
	"Ali silenziose planano nel buio.",
	"Il gufo artiglia.",
	"Verso stridulo.")

MOB_DESC(1, mob, 85, RACE_BIRD,
	"arpia giovane",
	"Arpia Giovane",
	"Una arpia immatura plana goffamente.",
	"Artigli non ancora maturi ma affilati.",
	"L'arpia graffia.",
	"Grido acuto.")

MOB_DESC(1, mob, 86, RACE_BIRD,
	"picchio ossidiana",
	"Picchio d'Ossidiana",
	"Un picchio dalle piume nere picchietta la roccia.",
	"Becco duro come acciaio.",
	"Il picchio becca.",
	"Picchiettio ossessionante.")

MOB_DESC(1, mob, 87, RACE_BIRD,
	"condor profondita",
	"Condor delle Profondita'",
	"Un condor macilento scende dal soffitto.",
	"Ali spaziali oscurano la luce.",
	"Il condor artiglia.",
	"Vento d'ali.")

MOB_DESC(1, mob, 88, RACE_BIRD,
	"rondine caverna",
	"Rondine di Caverna",
	"Rondini giganti volteggiano minacciose.",
	"Becco corto ma veloce.",
	"Le rondini attaccano in sciame.",
	"Cinguettii cavernosi.")

MOB_DESC(1, mob, 89, RACE_BIRD,
	"fenice corrotta",
	"Fenice Corrotta",
	"Un uccello fiammeggiante corrotto dalla bruma volteggia.",
	"Piume nere e braci verdi.",
	"La fenice scaglia braci.",
	"Crepitio di fuoco.")

MOB_DESC(1, mob, 90, RACE_DWARF,
	"nano maledetto",
	"Nano Maledetto",
	"Un nano morto-vivente brandisce ascia runica.",
	"Barba incrostata di polvere e sangue.",
	"Il nano maledetto colpisce.",
	"Imprecazioni nane.")

MOB_DESC(1, mob, 91, RACE_DWARF,
	"nano guardia",
	"Nano Guardia",
	"Un nano armato sorveglia un corridoio stretto.",
	"Elmo con visiera ribassata.",
	"La guardia blocca.",
	"Armatura che stride.")

MOB_DESC(1, mob, 92, RACE_DWARF,
	"nano scassinatore",
	"Nano Scassinatore",
	"Un nano con piccone e corda esplora.",
	"Occhi abituati al buio.",
	"Lo scassinatore colpisce.",
	"Piccone sulla roccia.")

MOB_DESC(1, mob, 93, RACE_DRAAGDIM,
	"duergar guerriero",
	"Duergar Guerriero",
	"Un duergar grigio brandisce martello da guerra.",
	"Barba d'acciaio e sguardo ostile.",
	"Il duergar colpisce.",
	"Grugnito duergar.")

MOB_DESC(1, mob, 94, RACE_DRAAGDIM,
	"duergar chierico",
	"Duergar Chierico",
	"Un duergar chierico invoca oscurita'.",
	"Simboli profani brillano sull'armatura.",
	"Il chierico maledice.",
	"Preghiere profane.")

MOB_DESC(1, mob, 95, RACE_GOLEM,
	"golem pietra minore",
	"Golem di Pietra Minore",
	"Un golem di pietra animato avanza goffamente.",
	"Runa incise pulsano debolmente.",
	"Il golem colpisce.",
	"Pietre che stridono.")

MOB_DESC(1, mob, 96, RACE_ENFAN,
	"enfan errante",
	"Enfan Errante",
	"Un enfan dalle orecchie lunghe brandisce lama curva.",
	"Occhi antichi in volto giovane.",
	"L'enfan attacca.",
	"Passi leggeri.")

MOB_DESC(1, mob, 97, RACE_GNOME,
	"gnomo fessura",
	"Gnomo delle Fessure",
	"Un piccolo umanoide dalle mani artigliate ti osserva dall'ombra.",
	"Occhi troppo grandi brillano di malizia famelica.",
	"Lo gnomo sibila un'imprecazione.",
	"Senti un riso stridulo.")

MOB_DESC(1, mob, 98, RACE_GNOME,
	"gnomo illusionista",
	"Gnomo Illusionista",
	"Un gnomo sotterraneo crea miraggi attorno a se'.",
	"Sorriso furbo e dita veloci.",
	"L'illusionista confonde.",
	"Risate distorte.")

MOB_DESC(1, mob, 99, RACE_SKEXIE,
	"skexie furtivo",
	"Skexie Furtivo",
	"Un skexie dalle movenze rapide brandisce daghe.",
	"Pelliccia scura lo mimetizza.",
	"Lo skexie colpisce due volte.",
	"Fruscio rapido.")

MOB_DESC(1, mob, 100, RACE_DARK_ELF,
	"drow esploratore",
	"Drow Esploratore",
	"Un drow incappucciato emerge dall'ombra.",
	"Lame d'ossidiana pronte a colpire.",
	"Il drow attacca rapidamente.",
	"Sussurri elfici.")

MOB_DESC(1, mob, 101, RACE_DARK_ELF,
	"drow mago",
	"Drow Mago",
	"Un mago drow traccia rune nel buio.",
	"Energia viola danza sulle dita.",
	"Il mago scaglia dardi.",
	"Incantesimi sibilati.")

MOB_DESC(1, mob, 102, RACE_DARK_ELF,
	"drow guerriero",
	"Drow Guerriero",
	"Un guerriero drow in armatura di mithril scuro.",
	"Scudo e spada coordinati.",
	"Il guerriero falcia.",
	"Armatura elfica.")

MOB_DESC(1, mob, 103, RACE_ELVEN,
	"elfo ombra",
	"Elfo Ombra",
	"Un elfo corrotto dalla bruma brandisce arco lungo.",
	"Occhi vuoti e movenze fluide.",
	"L'elfo scocca.",
	"Frecce nel buio.")

MOB_DESC(1, mob, 104, RACE_ELVEN,
	"elfo esiliato",
	"Elfo Esiliato",
	"Un elfo esiliato combatte con spada sottile.",
	"Mantello strappato e sguardo amaro.",
	"L'esiliato attacca.",
	"Passi silenziosi.")

MOB_DESC(1, mob, 105, RACE_PLANAR,
	"fey corrotto",
	"Fey Corrotto",
	"Una creatura fey distorta danza nel buio.",
	"Ali spezzate e sorriso crudele.",
	"Il fey confonde.",
	"Risate argentine e false.")

MOB_DESC(1, mob, 106, RACE_GHOST,
	"elfo spettrale",
	"Elfo Spettrale",
	"Un elfo morto da secoli vaga senza riposo.",
	"Armatura elfica ancora lucente.",
	"L'elfo spettrale colpisce.",
	"Lamento elfico.")

MOB_DESC(1, mob, 107, RACE_DARK_ELF,
	"drow assassino",
	"Drow Assassino",
	"Un assassino drow attende immobile.",
	"Veleno gocciola dalla lama.",
	"L'assassino colpisce alle spalle.",
	"Silenzio mortale.")

MOB_DESC(1, mob, 108, RACE_ELVEN,
	"elfo custode",
	"Elfo Custode",
	"Un custode elfico protegge un passaggio antico.",
	"Runa elfiche brillano sull'arco.",
	"Il custode scocca.",
	"Canti elfici lontani.")

MOB_DESC(1, mob, 109, RACE_PLANAR,
	"ombra fey",
	"Ombra Fey",
	"Un'ombra fey si stacca dal muro danzando.",
	"Forma umanoide indistinta.",
	"L'ombra avvolge.",
	"Sussurri musicali.")

MOB_DESC(1, mob, 110, RACE_TROLL,
	"troll caverna",
	"Troll di Caverna",
	"Un troll goffo si rialza sbavando.",
	"Pelle verde e rigenerazione visibile.",
	"Il troll colpisce con clava.",
	"Grugnito troll.")

MOB_DESC(1, mob, 111, RACE_TROLL,
	"troll ricucito",
	"Troll Ricucito",
	"Un troll ricucito grossolanamente avanza.",
	"Cicatrici e punti visibili ovunque.",
	"Il troll attacca senza dolore.",
	"Risate gutturali.")

MOB_DESC(1, mob, 112, RACE_GIANT,
	"gigante caverna",
	"Gigante di Caverna",
	"Un gigante curvo si muove nel corridoio.",
	"Clava di tronco sollevata.",
	"Il gigante schianta.",
	"Passi tremendi.")

MOB_DESC(1, mob, 113, RACE_GIANT_HILL,
	"gigante colline",
	"Gigante delle Colline",
	"Un gigante delle colline blocca la volta.",
	"Pelle coriacea e odore di muschio.",
	"Il gigante colpisce.",
	"Rumore di massi.")

MOB_DESC(1, mob, 114, RACE_GIANT_STONE,
	"gigante pietra",
	"Gigante di Pietra",
	"Un gigante di pietra si muove lentamente.",
	"Pelle simile a granito.",
	"Il gigante schianta.",
	"Pietre che cadono.")

MOB_DESC(1, mob, 115, RACE_HALF_OGRE,
	"mezzogre guardia",
	"Mezzogre Guardia",
	"Un mezzogre armato sorveglia un varco.",
	"Testa piccola e braccia enormi.",
	"La guardia colpisce.",
	"Grugnito ogre.")

MOB_DESC(1, mob, 116, RACE_HALF_OGRE,
	"mezzogre berserker",
	"Mezzogre Berserker",
	"Un mezzogre impazzito brandisce mazza.",
	"Schiuma alla bocca e occhi rossi.",
	"Il berserker colpisce.",
	"Urla di furia.")

MOB_DESC(1, mob, 117, RACE_TYTAN,
	"titano caduto",
	"Titano Caduto",
	"Un titano decaduto si trascina inginocchiato.",
	"Corpo enorme ma curvo dal tempo.",
	"Il titano schianta il pugno.",
	"Tremore tellurico.")

MOB_DESC(1, mob, 118, RACE_GIANT,
	"gigante mutilato",
	"Gigante Mutilato",
	"Un gigante con arti mancanti combatte comunque.",
	"Ferite antiche non guariscono.",
	"Il gigante colpisce.",
	"Lamento profondo.")

MOB_DESC(1, mob, 119, RACE_TROLL,
	"troll nebbia",
	"Troll della Nebbia",
	"Un troll avvolto in bruma attacca goffamente.",
	"Pelle grigia e occhi vitrei.",
	"Il troll colpisce.",
	"Grugnito ovattato.")

MOB_DESC(1, mob, 120, RACE_DEMON,
	"imp caverna",
	"Imp di Caverna",
	"Un imp alato volteggia lanciando scintille.",
	"Coda punge come una vespa.",
	"L'imp morde.",
	"Risate acute.")

MOB_DESC(1, mob, 121, RACE_DEMON,
	"demone minor",
	"Demone Minore",
	"Un demone cornuto giovane striscia dal buio.",
	"Zolfo e cenere lo circondano.",
	"Il demone attacca.",
	"Odore di zolfo.")

MOB_DESC(1, mob, 122, RACE_DEVIL,
	"diavolo imps",
	"Diavolo delle Fessure",
	"Un diavolo piccolo brandisce tridente.",
	"Pelle rossa e occhi gialli.",
	"Il diavolo punge.",
	"Risate infernali.")

MOB_DESC(1, mob, 123, RACE_ELEMENT,
	"elementale terra",
	"Elementale di Terra",
	"Un elementale di terra si rialza dal pavimento.",
	"Pietre e polvere compongono il corpo.",
	"L'elementale colpisce.",
	"Pietre che cadono.")

MOB_DESC(1, mob, 124, RACE_ELEMENT,
	"elementale acqua",
	"Elementale d'Acqua",
	"Un elementale d'acqua scorre come marea.",
	"Corpo trasparente e gelido.",
	"L'elementale annega.",
	"Schizzi d'acqua.")

MOB_DESC(1, mob, 125, RACE_ELEMENT,
	"elementale fuoco",
	"Elementale di Fuoco",
	"Un elementale di fuoco danza nel buio.",
	"Braci verdi corrompono la fiamma.",
	"L'elementale brucia.",
	"Crepitio di fiamme.")

MOB_DESC(1, mob, 126, RACE_SLIME,
	"gelatina caverna",
	"Gelatina di Caverna",
	"Una gelatina traslucida striscia sul pavimento.",
	"Ossa galleggiano nel corpo viscido.",
	"La gelatina inghiotte.",
	"Schizzi viscidi.")

MOB_DESC(1, mob, 127, RACE_SLIME,
	"muffa vivente",
	"Muffa Vivente",
	"Una muffa senziente si espande sulle pareti.",
	"Spore verdi riempiono l'aria.",
	"La muffa attacca.",
	"Tossire e starnuti.")

MOB_DESC(1, mob, 128, RACE_DEMON,
	"manes caverna",
	"Manes di Caverna",
	"Un manes distorto striscia come fumo solido.",
	"Volto muta continuamente.",
	"Il manes attacca.",
	"Lamenti distorti.")

MOB_DESC(1, mob, 129, RACE_DEVIL,
	"diavolo catena",
	"Diavolo a Catena",
	"Un diavolo trascina catene infuocate.",
	"Ferite bruciano al contatto.",
	"Il diavolo flagella.",
	"Scroscio di catene.")

MOB_DESC(1, mob, 130, RACE_MFLAYER,
	"mind flayer giovane",
	"Mind Flayer Giovane",
	"Un mind flayer immature tenta di entrare nella tua mente.",
	"Tentacoli facciali si contorcono.",
	"Il mind flayer attacca.",
	"Sussurri psichici.")

MOB_DESC(1, mob, 131, RACE_SPECIAL,
	"mimica giovane",
	"Mimica Giovane",
	"Una cassa che si apre a fauci e' in realta' una mimica.",
	"Lingua viscida lecca l'aria.",
	"La mimica morde.",
	"Tonfo di legno.")

MOB_DESC(1, mob, 132, RACE_SPECIAL,
	"mimica scrigno",
	"Mimica Scrigno",
	"Uno scrigno dorato attende troppo comodo per essere vero.",
	"Serratura e' in realta' un occhio.",
	"Lo scrigno morde.",
	"Scricchiolio metallico.")

MOB_DESC(1, mob, 133, RACE_PRIMATE,
	"grimlock guerriero",
	"Grimlock Guerriero",
	"Un grimlock cieco percepisce il tuo odore.",
	"Pelle grigia e denti affilati.",
	"Il grimlock attacca.",
	"Grugniti cavernosi.")

MOB_DESC(1, mob, 134, RACE_PRIMATE,
	"grimlock cacciatore",
	"Grimlock Cacciatore",
	"Un cacciatore grimlock trascina lancia di osso.",
	"Percezione acustica terrificante.",
	"Il cacciatore trafigge.",
	"Passi silenziosi.")

MOB_DESC(1, mob, 135, RACE_PARASITE,
	"parassita caverna",
	"Parassita di Caverna",
	"Un parassito grosso come un gatto si attacca.",
	"Corpo viscido e ventose.",
	"Il parassita succhia.",
	"Schizzi viscidi.")

MOB_DESC(1, mob, 136, RACE_ASTRAL,
	"creatura astrale minore",
	"Creatura Astrale Minore",
	"Una creatura planare fluttua instabile.",
	"Forma cambia continuamente.",
	"La creatura attacca.",
	"Echi lontani.")

MOB_DESC(1, mob, 137, RACE_PLANAR,
	"ombra planare",
	"Ombra Planare",
	"Un'ombra planare si stacca dal muro.",
	"Freddo non e' di questo mondo.",
	"L'ombra avvolge.",
	"Silenzio innaturale.")

MOB_DESC(1, mob, 138, RACE_SPECIAL,
	"aberrazione caverna",
	"Aberrazione di Caverna",
	"Una creatura innominabile striscia dal buio.",
	"Troppi occhi e troppa bocca.",
	"L'aberrazione attacca.",
	"Suoni disturbanti.")

MOB_DESC(1, mob, 139, RACE_MFLAYER,
	"illithid servitore",
	"Illithid Servitore",
	"Un servitore illithid obbedisce a un padrone lontano.",
	"Tentacoli tengono lama psionica.",
	"Il servitore attacca.",
	"Pulsazioni mentali.")

MOB_DESC(1, mob, 140, RACE_VEGMAN,
	"servitore muschio",
	"Servitore di Muschio",
	"Un umanoide ricoperto di muschio blocca il passaggio.",
	"Radici sottili serpeggiano lungo le sue gambe.",
	"Il servitore alza braccia nodose.",
	"La terra scricchiola sotto di lui.")

MOB_DESC(1, mob, 141, RACE_VEGMAN,
	"guardiano radici",
	"Guardiano delle Radici",
	"Un umanoide vegetale sorveglia un passaggio.",
	"Radici emergono dal pavimento.",
	"Il guardiano colpisce.",
	"Fruscio di foglie.")

MOB_DESC(1, mob, 142, RACE_TREE,
	"treant giovane",
	"Treant Giovane",
	"Un treant giovane si muove goffamente.",
	"Rami corteggiano il soffitto.",
	"Il treant schianta.",
	"Legno che scricchiola.")

MOB_DESC(1, mob, 143, RACE_VEGGIE,
	"pianta carnivora",
	"Pianta Carnivora",
	"Una pianta carnivora attende con fauci aperte.",
	"Polline dolce maschera pericolo.",
	"La pianta morde.",
	"Fruscio di foglie.")

MOB_DESC(1, mob, 144, RACE_VEGGIE,
	"liana strangolatrice",
	"Liana Strangolatrice",
	"Liane animate pendono dal soffitto.",
	"Steli si avvolgono come serpenti.",
	"La liana strangola.",
	"Fruscio secco.")

MOB_DESC(1, mob, 145, RACE_GHOST,
	"eco nebbia",
	"Eco della Nebbia",
	"Un'eco spettrale della dimensione vaga confusa.",
	"Forma muta come vapore.",
	"L'eco ti attraversa.",
	"Sussurri ovattati.")

MOB_DESC(1, mob, 146, RACE_ELEMENT,
	"nebbia vivente",
	"Nebbia Vivente",
	"Una nube di bruma assume forma umanoide.",
	"Occhi argentati nel vapore.",
	"La nebbia avvolge.",
	"Silenzio gelido.")

MOB_DESC(1, mob, 147, RACE_GHOST,
	"spettro nebbia",
	"Spettro della Nebbia",
	"Uno spettro nato dalla bruma fluttua.",
	"Corpo semitrasparente.",
	"Lo spettro attacca.",
	"Vento gelido.")

MOB_DESC(1, mob, 148, RACE_PREDATOR,
	"predatore bruma",
	"Predatore della Bruma",
	"Un predatore nato nel velo caccia silenzioso.",
	"Pelliccia grigia e occhi argentati.",
	"Il predatore piomba.",
	"Ringhio ovattato.")

MOB_DESC(1, mob, 149, RACE_INSECT,
	"libellula nebbia",
	"Libellula della Nebbia",
	"Una libellula traslucida volteggia nel vapore.",
	"Ali sottili tagliano l'aria.",
	"La libellula morde.",
	"Ronzio sottile.")

MOB_DESC(1, trap, 0, RACE_SNAKE,
	"trappola anguilla fango",
	"Anguilla del Fango",
	"Un serpente viscido emerge dal fango con bocca spalancata.",
	"La pelle oleosa rende difficile colpirla.",
	"L'anguilla si avvolge verso di te.",
	"Schizzi di fango ovunque.")

MOB_DESC(1, trap, 1, RACE_SPECIAL,
	"trappola mimica pavimento",
	"Mimica del Pavimento",
	"Una piastrella si apre a fauci: era una mimica.",
	"Denti d'acciaio nascondevano sotto la polvere.",
	"La mimica morde.",
	"Scricchiolio metallico.")

MOB_DESC(1, trap, 2, RACE_SLIME,
	"trappola gelatina assassina",
	"Gelatina Assassina",
	"Una gelatina assassina cade dal soffitto.",
	"Corpo viscido inghiotte luce e speranza.",
	"La gelatina avvolge.",
	"Schizzi viscidi.")

MOB_DESC(1, trap, 3, RACE_INSECT,
	"trappola ragnatela vivente",
	"Ragnatela Vivente",
	"Filamenti di ragnatela si tendono all'improvviso.",
	"Qualcosa tira verso l'alto con forza innaturale.",
	"La ragnatela strangola.",
	"Ragnatele che vibrano.")

MOB_DESC(1, trap, 4, RACE_SNAKE,
	"trappola vipera nascosta",
	"Vipera Nascosta",
	"Una vipera scatta da una fessura nel muro.",
	"Veleno gocciola dalle zanne appena visibili.",
	"La vipera morde.",
	"Sibilo secco.")

MOB_DESC(1, trap, 5, RACE_ELEMENT,
	"trappola gas velenoso",
	"Nube di Gas Velenoso",
	"Una nube di gas velenoso si alza dal pavimento.",
	"Occhi gialli pulsano nel vapore.",
	"Il gas avvelena.",
	"Tossire violento.")

MOB_DESC(1, trap, 6, RACE_GOLEM,
	"trappola porta punta",
	"Porta a Punta",
	"La porta si chiude rivelando punte di pietra.",
	"Runa attivata scorrono lungo il telaio.",
	"Le punte trafiggono.",
	"Pietre che scivolano.")

MOB_DESC(1, trap, 7, RACE_ELEMENT,
	"trappola ghiaccio sottile",
	"Ghiaccio Sottile",
	"Il pavimento e' un sottile strato di ghiaccio nascosto.",
	"Crepe si aprono sotto i tuoi passi.",
	"Il ghiaccio cede.",
	"Scricchiolio gelido.")

MOB_DESC(1, trap, 8, RACE_ELEMENT,
	"trappola fiamma improvvisa",
	"Trappola di Fiamma",
	"Getti di fiamma erompono da ugelli nel pavimento.",
	"Runa bruciate si accendono in sequenza.",
	"Le fiamme avvolgono.",
	"Esplosione di fuoco.")

MOB_DESC(1, trap, 9, RACE_ARACHNID,
	"trappola ragno agguato",
	"Ragno d'Agguato",
	"Un ragno d'agguato precipita dal soffitto.",
	"Veleno paralizzante nelle zanne.",
	"Il ragno morde.",
	"Ronzio acuto.")

MOB_DESC(1, trap, 10, RACE_ELEMENT,
	"trappola cristallo tagliente",
	"Cristallo Tagliente",
	"Cristalli affilati sporgono dal pavimento.",
	"Luce riflessa nasconde i fili taglienti.",
	"I cristalli tagliano.",
	"Scheggia di vetro.")

MOB_DESC(1, trap, 11, RACE_ELEMENT,
	"trappola maree improvvisa",
	"Marea Improvvisa",
	"Acqua salata irrompe da fessure laterali.",
	"Corrente spinge verso pareti taglienti.",
	"La marea travolge.",
	"Onde improvvise.")

MOB_DESC(1, trap, 12, RACE_ELEMENT,
	"trappola polvere sale",
	"Polvere di Sale Tagliente",
	"Polvere di sale tagliente si alza in vortice.",
	"Cristalli bianchi graffiano pelle e occhi.",
	"Il sale taglia.",
	"Polvere acida.")

MOB_DESC(1, trap, 13, RACE_UNDEAD_SKELETON,
	"trappola scheletri sepolcro",
	"Scheletri del Sepolcro",
	"Scheletri cadono dal soffitto in una pioggia d'ossa.",
	"Lame arrugginite pronte a colpire.",
	"Gli scheletri attaccano.",
	"Ossa che sbattono.")

MOB_DESC(1, trap, 14, RACE_DEMON,
	"trappola fessura infernale",
	"Fessura Infernale",
	"Una fessura infernale si apre sotto i piedi.",
	"Zolfo e artigli emergono dal baratro.",
	"La fessura brucia.",
	"Odore di zolfo.")

MOB_DESC(1, trap, 15, RACE_INSECT,
	"trappola vespa caverna",
	"Vespa di Caverna",
	"Vespe di caverna escono da un nido nascosto.",
	"Pungiglioni lunghi gocciolano veleno.",
	"Le vespe attaccano.",
	"Ronzio furioso.")

MOB_DESC(1, trap, 16, RACE_PREDATOR,
	"trappola worg fossa",
	"Worg della Fossa",
	"Un worg balza da una fossa coperta.",
	"Zanne pronte a strappare la gola.",
	"Il worg piomba.",
	"Ringhio famelico.")

MOB_DESC(1, trap, 17, RACE_GHOST,
	"trappola ombra maledetta",
	"Ombra Maledetta",
	"Un'ombra maledetta si stacca dal pavimento.",
	"Freddo non appartiene a questo mondo.",
	"L'ombra avvolge.",
	"Sussurro gelido.")

MOB_DESC(1, trap, 18, RACE_ELEMENT,
	"trappola fulmine runico",
	"Fulmine Runico",
	"Runa sul pavimento scatenano fulmini a catena.",
	"Odore di ozono riempie la stanza.",
	"Il fulmine colpisce.",
	"Tuono improvviso.")

MOB_DESC(1, trap, 19, RACE_PARASITE,
	"trappola parassita mimetico",
	"Parassita Mimetico",
	"Un parassito mimetico si stacca dal soffitto come stalattite.",
	"Ventose si attaccano prima che tu reagisca.",
	"Il parassito succhia.",
	"Schizzi viscidi.")

};

#undef MOB_DESC

#endif /* __PROCAREA_MOB_DESC_INC */
