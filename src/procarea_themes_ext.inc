/* procarea_themes_ext.inc — temi 25-49 (generato, non editare a mano) */
#ifndef __PROCAREA_THEMES_EXT_INC
#define __PROCAREA_THEMES_EXT_INC

/* --- Tema 25: Moria delle Ombre --- */

static const ProcRoomTemplate kTheme25Entrances[] = {
	{ "Porta di Durin",
	  "Un arco di pietra nanica si apre su sale infinite.\n"
	  "Rune sbiadite pulsano debolmente nel buio profondo.\n"
	  "Odori stagnanti si mescolano a tracce di ferro e pietra.\n"
	  "Il silenzio qui ha peso, come se qualcuno ascoltasse.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK | UNDERGROUND) },
	{ "Vestibolo dei Martelli",
	  "Statue di nani spezzate fiancheggiano un corridoio stretto.\n"
	  "L'eco dei passi sembra moltiplicarsi all'infinito.\n"
	  "Ombre lunghe si allungano dove la luce finisce.\n"
	  "Odori stagnanti si mescolano a tracce di ferro e pietra.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK | UNDERGROUND) },
};

static const ProcRoomTemplate kTheme25Corridors[] = {
	{ "Galleria dei Pilastri",
	  "Colonne massicce sostengono una volta lontana.\n"
	  "Pozzi senza fondo si aprono ai lati del cammino.\n"
	  "Ombre lunghe si allungano dove la luce finisce.\n"
	  "Odori stagnanti si mescolano a tracce di ferro e pietra.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK | UNDERGROUND) },
	{ "Passaggio del Fiume di Fuoco",
	  "Calore lontano e odore di zolfo aleggiano nell'aria.\n"
	  "Pietre nere brillano come brace spente.\n"
	  "Un suono lontano si spegne appena volgi la testa.\n"
	  "La luce vacilla come se la stanza respirasse.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK | UNDERGROUND) },
	{ "Sala delle Tombe Naniche",
	  "Sarcofagi di pietra sono stati forzati e saccheggiati.\n"
	  "Ossa naniche giacciono sparse sul pavimento.\n"
	  "L'atmosfera effimera rende incerto cio' che vedi e cio' che ricordi.\n"
	  "Per un istante hai la sensazione di essere osservato.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK | UNDERGROUND) },
	{ "Atrio del Pozzo Senza Fine",
	  "Un baratro circolare taglia la sala in due.\n"
	  "Corde consumate pendono nel vuoto.\n"
	  "L'atmosfera effimera rende incerto cio' che vedi e cio' che ricordi.\n"
	  "Per un istante hai la sensazione di essere osservato.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK | UNDERGROUND) },
};

static const ProcRoomTemplate kTheme25Treasures[] = {
	{ "Camera del Mithril",
	  "Filamenti d'argento raro ancora luccicano tra le rocce.\n"
	  "Picconi abbandonati raccontano una febbre dimenticata.\n"
	  "L'eco dei passi rimbalza su pareti vicine.\n"
	  "Polvere antica si solleva a ogni movimento.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK | UNDERGROUND) },
	{ "Forzieri dei Re Nani",
	  "Casse sigillate con rune naniche attendono ancora.\n"
	  "Qualcuno ha tentato di forzarle di recente.\n"
	  "La luce vacilla come se la stanza respirasse.\n"
	  "L'eco dei passi rimbalza su pareti vicine.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK | UNDERGROUND) },
};

static const ProcRoomTemplate kTheme25Traps[] = {
	{ "Ponte Spezzato",
	  "Lastre di pietra cedono sotto il peso al minimo passo falso.\n"
	  "Il baratro sotto richiama con vento gelido.\n"
	  "Il pericolo qui non si annuncia fino a un passo di troppo.\n"
	  "L'atmosfera effimera rende incerto cio' che vedi e cio' che ricordi.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK | UNDERGROUND) },
	{ "Sala delle Trappole Naniche",
	  "Dardi di pietra sporgono dalle pareti a intervalli regolari.\n"
	  "Piastrelle del pavimento sono leggermente inclinate.\n"
	  "Per un istante hai la sensazione di essere osservato.\n"
	  "Un suono lontano si spegne appena volgi la testa.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK | UNDERGROUND) },
};

static const ProcRoomTemplate kTheme25Boss = {
	"Sala del Balrog Dimenticato",
	  "Una caverna immensa si apre su un abisso fumante.\n"
	  "Catene spezzate pendono da un ponte di pietra crollato.\n"
	  "Qualcosa di antico e ardente attende nel cuore delle fiamme.\n"
	  "Per un istante hai la sensazione di essere osservato.\n",
	SECT_INSIDE, static_cast<long>(INDOORS | DARK | UNDERGROUND)
};

/* --- Tema 26: Fangorn Profondo --- */

static const ProcRoomTemplate kTheme26Entrances[] = {
	{ "Varco tra gli Ent",
	  "Due alberi colossali formano un arco naturale.\n"
	  "La corteccia sembra osservarti con pazienza antica.\n"
	  "Un suono lontano si spegne appena volgi la testa.\n"
	  "La luce vacilla come se la stanza respirasse.\n",
	  SECT_FOREST, static_cast<long>(INDOORS | DARK) },
	{ "Sentiero della Voce Verde",
	  "Muschio spesso ammortizza ogni rumore.\n"
	  "Sussurri di foglie raccontano storie millenarie.\n"
	  "Un fruscio lontano si ferma appena ti fermi tu.\n"
	  "Qualcosa nel profondo sembra richiamarti oltre la soglia.\n",
	  SECT_FOREST, static_cast<long>(INDOORS | DARK) },
};

static const ProcRoomTemplate kTheme26Corridors[] = {
	{ "Corridoio di Radici",
	  "Radici spesse come serpenti bloccano passaggi laterali.\n"
	  "Aria umida odora di terra e linfa.\n"
	  "Luce verdastra filtra a macchia di leopardo tra le fronde.\n"
	  "Radici nodose affondano nel terreno molle.\n",
	  SECT_FOREST, static_cast<long>(INDOORS | DARK) },
	{ "Galleria dei Muschi",
	  "Muschio luminoso traccia sentieri nel buio.\n"
	  "Funghi alti come uomini crescono lungo le pareti.\n"
	  "La luce vacilla come se la stanza respirasse.\n"
	  "Resina e muschio impregnano l'aria umida.\n",
	  SECT_FOREST, static_cast<long>(INDOORS | DARK) },
	{ "Passaggio dei Sussurri",
	  "Il vento tra i rami imita voci quasi comprensibili.\n"
	  "Ogni passo fa scricchiolare legno vivo.\n"
	  "Radici nodose affondano nel terreno molle.\n"
	  "Un fruscio lontano si ferma appena ti fermi tu.\n",
	  SECT_FOREST, static_cast<long>(INDOORS | DARK) },
	{ "Radura degli Ent Sonnecchianti",
	  "Tronchi immobili sembrano dormire in cerchio.\n"
	  "Solo gli occhi di resina tradiscono vigilanza.\n"
	  "L'atmosfera effimera rende incerto cio' che vedi e cio' che ricordi.\n"
	  "Per un istante hai la sensazione di essere osservato.\n",
	  SECT_FOREST, static_cast<long>(INDOORS | DARK) },
};

static const ProcRoomTemplate kTheme26Treasures[] = {
	{ "Cavità della Corteccia d'Oro",
	  "Schegge preziose si staccano da un tronco antico.\n"
	  "Resina ambrata racchiude insetti dimenticati.\n"
	  "Un suono lontano si spegne appena volgi la testa.\n"
	  "La luce vacilla come se la stanza respirasse.\n",
	  SECT_FOREST, static_cast<long>(INDOORS | DARK) },
	{ "Nicchia dei Semi Antichi",
	  "Semi grandi come pugni pulsano di vita sopita.\n"
	  "Qualcuno li ha raccolti con reverenza.\n"
	  "L'atmosfera effimera rende incerto cio' che vedi e cio' che ricordi.\n"
	  "Per un istante hai la sensazione di essere osservato.\n",
	  SECT_FOREST, static_cast<long>(INDOORS | DARK) },
};

static const ProcRoomTemplate kTheme26Traps[] = {
	{ "Trappola di Liane",
	  "Liane si avvolgono al minimo contatto.\n"
	  "La foresta reagisce alla tua presenza.\n"
	  "Un suono lontano si spegne appena volgi la testa.\n"
	  "La luce vacilla come se la stanza respirasse.\n",
	  SECT_FOREST, static_cast<long>(INDOORS | DARK) },
	{ "Pozza di Linfa Acida",
	  "Linfa corrosiva gocciola dal soffitto di foglie.\n"
	  "Il terreno si ammorbidisce in modo minaccioso.\n"
	  "Un fruscio lontano si ferma appena ti fermi tu.\n"
	  "Il pericolo qui non si annuncia fino a un passo di troppo.\n",
	  SECT_FOREST, static_cast<long>(INDOORS | DARK) },
};

static const ProcRoomTemplate kTheme26Boss = {
	"Cuore di Fangorn",
	  "Una radura circolare accoglie un trono di rami intrecciati.\n"
	  "Ent antichi emergono dalla penombra con passi lenti.\n"
	  "La foresta stessa sembra respirare attorno a te.\n"
	  "Qualcosa di antico domina lo spazio con presenza opprimente.\n",
	SECT_FOREST, static_cast<long>(INDOORS | DARK)
};

/* --- Tema 27: Tomba dei Re Caduti --- */

static const ProcRoomTemplate kTheme27Entrances[] = {
	{ "Tumulo dei Re",
	  "Un tumulo erboso nasconde una discesa verso l'oscurita'.\n"
	  "Pietre runiche segnano confini dimenticati.\n"
	  "La luce vacilla come se la stanza respirasse.\n"
	  "L'eco dei passi rimbalza su pareti vicine.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
	{ "Ingresso del Barrow",
	  "Una porta di pietra socchiusa libera aria gelida.\n"
	  "Gioielli rubati giacciono abbandonati sulla soglia.\n"
	  "Polvere antica si solleva a ogni movimento.\n"
	  "Una corrente d'aria fredda tradisce un varco nascosto.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
};

static const ProcRoomTemplate kTheme27Corridors[] = {
	{ "Corridoio delle Corone Spezzate",
	  "Corone arrugginite pendono da chiodi nel muro.\n"
	  "Telai di ragnatela coprono stemmi sbiaditi.\n"
	  "La luce vacilla come se la stanza respirasse.\n"
	  "L'eco dei passi rimbalza su pareti vicine.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
	{ "Galleria dei Cavalieri",
	  "Armature vuote sorvegliano il passaggio.\n"
	  "Spade incastrate nella pietra formano una ringhiera.\n"
	  "Ombre lunghe si allungano dove la luce finisce.\n"
	  "Odori stagnanti si mescolano a tracce di ferro e pietra.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
	{ "Sala delle Candele Spente",
	  "Candelabri antichi attendono un fuoco che non tornera'.\n"
	  "Ceneri di ceri secolari coprono il pavimento.\n"
	  "Ombre lunghe si allungano dove la luce finisce.\n"
	  "Odori stagnanti si mescolano a tracce di ferro e pietra.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
	{ "Atrio del Re Dormiente",
	  "Un sarcofago di pietra domina la stanza.\n"
	  "Rune di protezione sono state violate.\n"
	  "Polvere antica si solleva a ogni movimento.\n"
	  "Una corrente d'aria fredda tradisce un varco nascosto.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
};

static const ProcRoomTemplate kTheme27Treasures[] = {
	{ "Camera del Tesoro Reale",
	  "Oro e argento giacciono accanto a ossa reali.\n"
	  "Una corona dimenticata brilla ancora debolmente.\n"
	  "Un luccichio falso attira lo sguardo verso gli angoli.\n"
	  "L'aria qui e' piu' ferma, come in attesa di una scelta.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
	{ "Reliquiario dei Caduti",
	  "Reliquie avvolte in seta marcia attendono un pellegrino.\n"
	  "Sigilli di cera sono stati rotti.\n"
	  "Polvere antica si solleva a ogni movimento.\n"
	  "Una corrente d'aria fredda tradisce un varco nascosto.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
};

static const ProcRoomTemplate kTheme27Traps[] = {
	{ "Passaggio Maledetto",
	  "Simboli arsi a fuoco bruciano ancora sulla soglia.\n"
	  "Un freddo innaturale ti avvolge.\n"
	  "Un suono lontano si spegne appena volgi la testa.\n"
	  "La luce vacilla come se la stanza respirasse.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
	{ "Sala delle Lame Emergenti",
	  "Lame di pietra spuntano dal pavimento a intervalli.\n"
	  "Segni di fuga disperata segnano le pareti.\n"
	  "Ombre lunghe si allungano dove la luce finisce.\n"
	  "Odori stagnanti si mescolano a tracce di ferro e pietra.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
};

static const ProcRoomTemplate kTheme27Boss = {
	"Trono del Barrow-Wight",
	  "Una sala circolare ospita un trono di ossa e corone.\n"
	  "Luci verdastre danzano senza fiamma.\n"
	  "Il re dei tumuli si rialza per difendere il suo regno.\n"
	  "La luce vacilla come se la stanza respirasse.\n",
	SECT_INSIDE, static_cast<long>(INDOORS | DARK)
};

/* --- Tema 28: Ponte del Baratro --- */

static const ProcRoomTemplate kTheme28Entrances[] = {
	{ "Inizio del Ponte di Pietra",
	  "Un ponte stretto attraversa un abisso senza fondo.\n"
	  "Vento gelido sale dal baratro sottostante.\n"
	  "Ombre lunghe si allungano dove la luce finisce.\n"
	  "Odori stagnanti si mescolano a tracce di ferro e pietra.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK | UNDERGROUND) },
	{ "Crepaccio Laterale",
	  "Una fessura nella roccia rivela un secondo varco.\n"
	  "Corde e pioli arrugginiti pendono nel vuoto.\n"
	  "Per un istante hai la sensazione di essere osservato.\n"
	  "Un suono lontano si spegne appena volgi la testa.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK | UNDERGROUND) },
};

static const ProcRoomTemplate kTheme28Corridors[] = {
	{ "Galleria del Baratro",
	  "Il cammino costeggia un precipizio senza parapetto.\n"
	  "Pietre cadute giacciono nel vuoto sotto di te.\n"
	  "Il silenzio qui ha peso, come se qualcuno ascoltasse.\n"
	  "Il passaggio si restringe e poi torna ad aprirsi senza logica.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK | UNDERGROUND) },
	{ "Passaggio Sospeso",
	  "Travi di legno marcio collegano piattaforme rocciose.\n"
	  "Ogni passo fa gemere la struttura.\n"
	  "Un suono lontano si spegne appena volgi la testa.\n"
	  "La luce vacilla come se la stanza respirasse.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK | UNDERGROUND) },
	{ "Corridoio dell'Eco Profonda",
	  "Il tuo respiro rimbomba per secondi interminabili.\n"
	  "Non vedi il fondo neppure con la torcia.\n"
	  "L'eco dei passi rimbalza su pareti vicine.\n"
	  "Polvere antica si solleva a ogni movimento.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK | UNDERGROUND) },
	{ "Atrio del Vuoto",
	  "Una piattaforma circolare domina il baratro.\n"
	  "Catene spezzate oscillano nel vento.\n"
	  "Ombre lunghe si allungano dove la luce finisce.\n"
	  "Odori stagnanti si mescolano a tracce di ferro e pietra.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK | UNDERGROUND) },
};

static const ProcRoomTemplate kTheme28Treasures[] = {
	{ "Nicchia del Viandante Caduto",
	  "Uno zaino logoro nasconde monete e una mappa.\n"
	  "Ossa giacciono vicino al bordo del precipizio.\n"
	  "Un luccichio falso attira lo sguardo verso gli angoli.\n"
	  "L'aria qui e' piu' ferma, come in attesa di una scelta.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK | UNDERGROUND) },
	{ "Forziere del Guardiano",
	  "Un forziere e' incatenato alla roccia.\n"
	  "La chiave giace in un cranio vicino.\n"
	  "L'atmosfera effimera rende incerto cio' che vedi e cio' che ricordi.\n"
	  "Per un istante hai la sensazione di essere osservato.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK | UNDERGROUND) },
};

static const ProcRoomTemplate kTheme28Traps[] = {
	{ "Ponte Instabile",
	  "Lastre cedono una a una al tuo passaggio.\n"
	  "Non c'e' tempo per esitare.\n"
	  "Polvere antica si solleva a ogni movimento.\n"
	  "Una corrente d'aria fredda tradisce un varco nascosto.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK | UNDERGROUND) },
	{ "Sala del Soffio Gelido",
	  "Raffiche dal baratro spingono verso il vuoto.\n"
	  "Il parapetto manca da tempo.\n"
	  "L'eco dei passi rimbalza su pareti vicine.\n"
	  "Polvere antica si solleva a ogni movimento.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK | UNDERGROUND) },
};

static const ProcRoomTemplate kTheme28Boss = {
	"Cuore dell'Abisso",
	  "Una piattaforma sospesa culmina sopra il baratro.\n"
	  "Fulmini lontani illuminano una figura al centro.\n"
	  "Il ponte dietro di te trema come se volesse crollare.\n"
	  "L'atmosfera effimera rende incerto cio' che vedi e cio' che ricordi.\n",
	SECT_INSIDE, static_cast<long>(INDOORS | DARK | UNDERGROUND)
};

/* --- Tema 29: Rovine di Osgiliath --- */

static const ProcRoomTemplate kTheme29Entrances[] = {
	{ "Arco Spezzato",
	  "Un arco monumentale e' crollato a meta'.\n"
	  "Iscrizioni in lingua antica si leggono a fatica.\n"
	  "Un suono lontano si spegne appena volgi la testa.\n"
	  "La luce vacilla come se la stanza respirasse.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
	{ "Ponte sull'Anduin",
	  "Piloni di pietra emergono dall'acqua sotterranea.\n"
	  "Correnti lente portano detriti di una citta' morta.\n"
	  "Il silenzio qui ha peso, come se qualcuno ascoltasse.\n"
	  "Qualcosa nel profondo sembra richiamarti oltre la soglia.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
};

static const ProcRoomTemplate kTheme29Corridors[] = {
	{ "Via dei Re",
	  "Colonne spezzate fiancheggiano una strada deserta.\n"
	  "Stendardi a brandelli oscillano senza vento.\n"
	  "L'eco dei passi rimbalza su pareti vicine.\n"
	  "Polvere antica si solleva a ogni movimento.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
	{ "Piazza del Palantir",
	  "Una sfera di cristallo frantumata giace al centro.\n"
	  "Vetri taglienti coprono il pavimento.\n"
	  "La luce vacilla come se la stanza respirasse.\n"
	  "L'eco dei passi rimbalza su pareti vicine.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
	{ "Corridoio del Senato",
	  "Sedili di marmo sono stati rivoltati e bruciati.\n"
	  "Cenere antica copre mosaici preziosi.\n"
	  "Un suono lontano si spegne appena volgi la testa.\n"
	  "La luce vacilla come se la stanza respirasse.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
	{ "Atrio della Torre Bianca",
	  "Scale spezzate salgono verso il cielo aperto.\n"
	  "Corvi nidificano tra le pietre cadute.\n"
	  "Il silenzio qui ha peso, come se qualcuno ascoltasse.\n"
	  "Il passaggio si restringe e poi torna ad aprirsi senza logica.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
};

static const ProcRoomTemplate kTheme29Treasures[] = {
	{ "Tesoro del Palazzo",
	  "Monete d'argento e oro giacciono tra le macerie.\n"
	  "Qualcuno ha scavato di recente.\n"
	  "Polvere antica si solleva a ogni movimento.\n"
	  "Una corrente d'aria fredda tradisce un varco nascosto.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
	{ "Archivio delle Carte Reali",
	  "Pergamene sigillate resistono ancora all'umidita'.\n"
	  "Mappe di regni perduti attendono.\n"
	  "L'aria qui e' piu' ferma, come in attesa di una scelta.\n"
	  "L'atmosfera effimera rende incerto cio' che vedi e cio' che ricordi.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
};

static const ProcRoomTemplate kTheme29Traps[] = {
	{ "Sala del Crollo",
	  "Il soffitto minaccia di cedere ad ogni rumore.\n"
	  "Travi spezzate bloccano uscite laterali.\n"
	  "La luce vacilla come se la stanza respirasse.\n"
	  "L'eco dei passi rimbalza su pareti vicine.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
	{ "Passaggio degli Orchetti",
	  "Trappole rudimentali segnano un accampamento recente.\n"
	  "Ossa e rifiuti coprono il pavimento.\n"
	  "La luce vacilla come se la stanza respirasse.\n"
	  "L'eco dei passi rimbalza su pareti vicine.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
};

static const ProcRoomTemplate kTheme29Boss = {
	"Santuario del Re Pellegrino",
	  "Una cappella mezza distrutta ospita un altare ancora intatto.\n"
	  "Luce grigia filtra tra le rovine.\n"
	  "Un custode spettrale veglia su cio' che resta della citta'.\n"
	  "Odori stagnanti si mescolano a tracce di ferro e pietra.\n",
	SECT_INSIDE, static_cast<long>(INDOORS | DARK)
};

/* --- Tema 30: Covo degli Uruk --- */

static const ProcRoomTemplate kTheme30Entrances[] = {
	{ "Fossa di Warcraft",
	  "Scale di legno scendono in un cantiere orco.\n"
	  "Odore di ferro e carne putrefatta riempie l'aria.\n"
	  "L'atmosfera effimera rende incerto cio' che vedi e cio' che ricordi.\n"
	  "Per un istante hai la sensazione di essere osservato.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK | UNDERGROUND) },
	{ "Varco delle Gole",
	  "Pareti strette sono segnate da artigli e rune di guerra.\n"
	  "Grugniti e martelli riecheggiano da sotto.\n"
	  "Un suono lontano si spegne appena volgi la testa.\n"
	  "La luce vacilla come se la stanza respirasse.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK | UNDERGROUND) },
};

static const ProcRoomTemplate kTheme30Corridors[] = {
	{ "Galleria degli Scudi",
	  "Scudi orchi impilati formano pareti improvvisate.\n"
	  "Torce di pece fumano basso.\n"
	  "Polvere antica si solleva a ogni movimento.\n"
	  "Una corrente d'aria fredda tradisce un varco nascosto.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK | UNDERGROUND) },
	{ "Corridoio delle Lance",
	  "Lance arrugginite punteggiano il soffitto come stalattiti.\n"
	  "Ossa di prigionieri giacciono ammucchiate.\n"
	  "L'eco dei passi rimbalza su pareti vicine.\n"
	  "Polvere antica si solleva a ogni movimento.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK | UNDERGROUND) },
	{ "Fucina di Guerra",
	  "Incudini ancora calde e martelli abbandonati.\n"
	  "Lame non finite pendono dai ganci.\n"
	  "Il passaggio si restringe e poi torna ad aprirsi senza logica.\n"
	  "Segni sulle pareti indicano che altri sono passati di recente.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK | UNDERGROUND) },
	{ "Atrio del Capitano",
	  "Un trono di ossa e armi domina la sala.\n"
	  "Stendardi neri segnano territorio conquistato.\n"
	  "Il silenzio qui ha peso, come se qualcuno ascoltasse.\n"
	  "Il passaggio si restringe e poi torna ad aprirsi senza logica.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK | UNDERGROUND) },
};

static const ProcRoomTemplate kTheme30Treasures[] = {
	{ "Bottino di Guerra",
	  "Armi e armature rubate sono ammucchiate senza ordine.\n"
	  "Monete sparse brillano tra il ferro.\n"
	  "Per un istante hai la sensazione di essere osservato.\n"
	  "Un suono lontano si spegne appena volgi la testa.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK | UNDERGROUND) },
	{ "Camera del Signore della Guerra",
	  "Mappe graffiate sulle pareti mostrano invasioni pianificate.\n"
	  "Un forziere pesante attende.\n"
	  "L'atmosfera effimera rende incerto cio' che vedi e cio' che ricordi.\n"
	  "Per un istante hai la sensazione di essere osservato.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK | UNDERGROUND) },
};

static const ProcRoomTemplate kTheme30Traps[] = {
	{ "Fossa con Pale",
	  "Pale coprono trappole nel pavimento di fango.\n"
	  "Cadere significa impalarsi.\n"
	  "Un suono lontano si spegne appena volgi la testa.\n"
	  "La luce vacilla come se la stanza respirasse.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK | UNDERGROUND) },
	{ "Sala delle Frecce",
	  "Dardi scattano dalle fessure al passaggio.\n"
	  "Segni di sangue segnano il percorso.\n"
	  "Odori stagnanti si mescolano a tracce di ferro e pietra.\n"
	  "Il silenzio qui ha peso, come se qualcuno ascoltasse.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK | UNDERGROUND) },
};

static const ProcRoomTemplate kTheme30Boss = {
	"Trono dell'Uruk-Hai",
	  "Una sala vasta ospita un trono di ferro e ossa.\n"
	  "Cori di guerra echeggiano da tunnel laterali.\n"
	  "Il signore degli uruk si alza per schiacciare gli intrusi.\n"
	  "Ombre lunghe si allungano dove la luce finisce.\n",
	SECT_INSIDE, static_cast<long>(INDOORS | DARK | UNDERGROUND)
};

/* --- Tema 31: Galleria Elfica --- */

static const ProcRoomTemplate kTheme31Entrances[] = {
	{ "Varco di Lothlorien",
	  "Alberi argentei incorniciano un sentiero luminoso.\n"
	  "L'aria profuma di mallorn e luce antica.\n"
	  "Qualcosa nel profondo sembra richiamarti oltre la soglia.\n"
	  "La transizione verso l'interno e' netta e irreversibile.\n",
	  SECT_FOREST, static_cast<long>(INDOORS | BRIGHT) },
	{ "Ponte di Vine",
	  "Viti intrecciate formano un ponte sospeso.\n"
	  "Luce filtra come acqua tra le foglie.\n"
	  "Resina e muschio impregnano l'aria umida.\n"
	  "Rami scricchiolano oltre il campo visivo.\n",
	  SECT_FOREST, static_cast<long>(INDOORS | BRIGHT) },
};

static const ProcRoomTemplate kTheme31Corridors[] = {
	{ "Corridoio dei Cantori",
	  "Melodie elfiche sembrano provenire dalle pareti stesse.\n"
	  "Intagli dorati raccontano ere dimenticate.\n"
	  "Rami scricchiolano oltre il campo visivo.\n"
	  "Luce verdastra filtra a macchia di leopardo tra le fronde.\n",
	  SECT_FOREST, static_cast<long>(INDOORS | BRIGHT) },
	{ "Galleria delle Stelle",
	  "Cristalli incastonati imitano un cielo notturno.\n"
	  "Ogni passo risuona come una nota.\n"
	  "Luce verdastra filtra a macchia di leopardo tra le fronde.\n"
	  "Radici nodose affondano nel terreno molle.\n",
	  SECT_FOREST, static_cast<long>(INDOORS | BRIGHT) },
	{ "Passaggio dei Guardiani",
	  "Statue elfiche seguono i tuoi movimenti con gli occhi.\n"
	  "Frecce incastonate decorano gli archi.\n"
	  "L'atmosfera effimera rende incerto cio' che vedi e cio' che ricordi.\n"
	  "Per un istante hai la sensazione di essere osservato.\n",
	  SECT_FOREST, static_cast<long>(INDOORS | BRIGHT) },
	{ "Atrio della Regina",
	  "Colonne sottili sostengono una volta di luce.\n"
	  "Fiori eterni sbocciano senza stagione.\n"
	  "Resina e muschio impregnano l'aria umida.\n"
	  "Rami scricchiolano oltre il campo visivo.\n",
	  SECT_FOREST, static_cast<long>(INDOORS | BRIGHT) },
};

static const ProcRoomTemplate kTheme31Treasures[] = {
	{ "Camera dei Doni",
	  "Vestiti elfici e gioielli attendono ospiti degni.\n"
	  "Pietre preziose pulsano di luce interna.\n"
	  "Un fruscio lontano si ferma appena ti fermi tu.\n"
	  "Un luccichio falso attira lo sguardo verso gli angoli.\n",
	  SECT_FOREST, static_cast<long>(INDOORS | BRIGHT) },
	{ "Biblioteca dei Canti",
	  "Pergamene di canzoni antiche sono conservate in seta.\n"
	  "Un'arpa d'argento giace su un piedistallo.\n"
	  "La luce vacilla come se la stanza respirasse.\n"
	  "Resina e muschio impregnano l'aria umida.\n",
	  SECT_FOREST, static_cast<long>(INDOORS | BRIGHT) },
};

static const ProcRoomTemplate kTheme31Traps[] = {
	{ "Sala delle Illusioni",
	  "Miraggi elfici nascondono buchi nel pavimento.\n"
	  "La bellezza distrae dal pericolo.\n"
	  "Per un istante hai la sensazione di essere osservato.\n"
	  "Un suono lontano si spegne appena volgi la testa.\n",
	  SECT_FOREST, static_cast<long>(INDOORS | BRIGHT) },
	{ "Passaggio delle Frecce Silenti",
	  "Balestre nascoste puntano verso il centro.\n"
	  "Solo gli elfi conoscono il percorso sicuro.\n"
	  "La luce vacilla come se la stanza respirasse.\n"
	  "Resina e muschio impregnano l'aria umida.\n",
	  SECT_FOREST, static_cast<long>(INDOORS | BRIGHT) },
};

static const ProcRoomTemplate kTheme31Boss = {
	"Santuario della Luce Argentea",
	  "Una radura di luce pura accoglie un altare di cristallo.\n"
	  "Alberi mallorn formano una cupola naturale.\n"
	  "Un guardiano elfico emerge dalla luce stessa.\n"
	  "Per un istante hai la sensazione di essere osservato.\n",
	SECT_FOREST, static_cast<long>(INDOORS | BRIGHT)
};

/* --- Tema 32: Ceneri di Gorgoroth --- */

static const ProcRoomTemplate kTheme32Entrances[] = {
	{ "Sentiero delle Ceneri",
	  "Cenere vulcanica copre ogni superficie.\n"
	  "Il cielo e' una coltre rossastra di fumo.\n"
	  "L'atmosfera effimera rende incerto cio' che vedi e cio' che ricordi.\n"
	  "Per un istante hai la sensazione di essere osservato.\n",
	  SECT_MOUNTAIN, static_cast<long>(INDOORS | DARK) },
	{ "Varco di Mordor",
	  "Rocce nere formano un arco minaccioso.\n"
	  "L'aria brucia in gola ad ogni respiro.\n"
	  "Per un istante hai la sensazione di essere osservato.\n"
	  "Un suono lontano si spegne appena volgi la testa.\n",
	  SECT_MOUNTAIN, static_cast<long>(INDOORS | DARK) },
};

static const ProcRoomTemplate kTheme32Corridors[] = {
	{ "Galleria del Fuoco Lontano",
	  "Rive luminose pulsano oltre le pareti di basalto.\n"
	  "Zolfo cristallizzato ricopre il pavimento.\n"
	  "Segni sulle pareti indicano che altri sono passati di recente.\n"
	  "L'atmosfera effimera rende incerto cio' che vedi e cio' che ricordi.\n",
	  SECT_MOUNTAIN, static_cast<long>(INDOORS | DARK) },
	{ "Corridoio degli Orchetti",
	  "Impronte di orchi e ferri arrugginiti segnano il cammino.\n"
	  "Urla lontane portano ordini incomprensibili.\n"
	  "La luce vacilla come se la stanza respirasse.\n"
	  "Pietra nuda e vento tagliente dominano lo spazio.\n",
	  SECT_MOUNTAIN, static_cast<long>(INDOORS | DARK) },
	{ "Passaggio del Fumo Denso",
	  "Fumo acido riduce la visibilita' a pochi passi.\n"
	  "Occhi rossi brillano nella penombra.\n"
	  "Pietra nuda e vento tagliente dominano lo spazio.\n"
	  "La roccia e' fredda al tatto e coperta da polvere sottile.\n",
	  SECT_MOUNTAIN, static_cast<long>(INDOORS | DARK) },
	{ "Atrio delle Fornaci",
	  "Fornaci rudimentali fumano ancora.\n"
	  "Armi mal forgiate giacciono a raffreddare.\n"
	  "Segni sulle pareti indicano che altri sono passati di recente.\n"
	  "L'atmosfera effimera rende incerto cio' che vedi e cio' che ricordi.\n",
	  SECT_MOUNTAIN, static_cast<long>(INDOORS | DARK) },
};

static const ProcRoomTemplate kTheme32Treasures[] = {
	{ "Tesoro del Signore Oscuro",
	  "Armi e trofei sono ammucchiati con brutalita'.\n"
	  "Anelli spezzati brillano tra le ceneri.\n"
	  "La roccia e' fredda al tatto e coperta da polvere sottile.\n"
	  "Voci lontane si perdono nel ronzio del vuoto.\n",
	  SECT_MOUNTAIN, static_cast<long>(INDOORS | DARK) },
	{ "Camera dei Sigilli",
	  "Sigilli di ferro incisi con rune nere sigillano un forziere.\n"
	  "Un forziere e' incatenato alla roccia.\n"
	  "Crepe profonde scendono verso l'oscurita'.\n"
	  "Cristalli di ghiaccio scintillano dove passa poca luce.\n",
	  SECT_MOUNTAIN, static_cast<long>(INDOORS | DARK) },
};

static const ProcRoomTemplate kTheme32Traps[] = {
	{ "Sala della Lava",
	  "Crepe nel pavimento mostrano lava sotto la crosta.\n"
	  "Calore insopportabile aumenta verso il centro.\n"
	  "Crepe profonde scendono verso l'oscurita'.\n"
	  "Cristalli di ghiaccio scintillano dove passa poca luce.\n",
	  SECT_MOUNTAIN, static_cast<long>(INDOORS | DARK) },
	{ "Passaggio dei Flagelli",
	  "Fruste di ferro pendono dal soffitto.\n"
	  "Cicatrici sulle pareti raccontano torture.\n"
	  "Cristalli di ghiaccio scintillano dove passa poca luce.\n"
	  "Il pericolo qui non si annuncia fino a un passo di troppo.\n",
	  SECT_MOUNTAIN, static_cast<long>(INDOORS | DARK) },
};

static const ProcRoomTemplate kTheme32Boss = {
	"Trono delle Ceneri",
	  "Un trono di roccia vulcanica domina una pianura di cenere.\n"
	  "Il ruggito di un vulcano lontano scuote la terra.\n"
	  "Un signore delle ombre attende incornato di fumo.\n"
	  "Un suono lontano si spegne appena volgi la testa.\n",
	SECT_MOUNTAIN, static_cast<long>(INDOORS | DARK)
};

/* --- Tema 33: Fortezza di Helm --- */

static const ProcRoomTemplate kTheme33Entrances[] = {
	{ "Porta del Fosso",
	  "Un ponte levatoio spezzato oscilla sopra un fosso profondo.\n"
	  "Mura massicce si ergono fino a perdersi nel buio.\n"
	  "Il silenzio qui ha peso, come se qualcuno ascoltasse.\n"
	  "Qualcosa nel profondo sembra richiamarti oltre la soglia.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
	{ "Ingresso della Grotta",
	  "Una grotta naturale si apre sotto le mura.\n"
	  "Echi di battaglie antiche riempiono l'aria.\n"
	  "Un suono lontano si spegne appena volgi la testa.\n"
	  "La luce vacilla come se la stanza respirasse.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
};

static const ProcRoomTemplate kTheme33Corridors[] = {
	{ "Corridoio dei Difensori",
	  "Archi per corniere si aprono sulle pareti.\n"
	  "Frecce rotte giacciono in mucchi.\n"
	  "Il passaggio si restringe e poi torna ad aprirsi senza logica.\n"
	  "Segni sulle pareti indicano che altri sono passati di recente.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
	{ "Galleria del Fosso Profondo",
	  "Acqua stagnante copre il pavimento.\n"
	  "Scheletri in armatura emergono dal fango.\n"
	  "Una corrente d'aria fredda tradisce un varco nascosto.\n"
	  "Ombre lunghe si allungano dove la luce finisce.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
	{ "Passaggio della Sortita",
	  "Una porta segreta conduce verso l'esterno.\n"
	  "Segni di una fuga disperata segnano le pareti.\n"
	  "Polvere antica si solleva a ogni movimento.\n"
	  "Una corrente d'aria fredda tradisce un varco nascosto.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
	{ "Atrio del Re di Rohan",
	  "Stendardi sbiaditi pendono da lance spezzate.\n"
	  "Un trono di pietra e' vuoto ma imponente.\n"
	  "Odori stagnanti si mescolano a tracce di ferro e pietra.\n"
	  "Il silenzio qui ha peso, come se qualcuno ascoltasse.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
};

static const ProcRoomTemplate kTheme33Treasures[] = {
	{ "Armeria Segreta",
	  "Armi di qualita' sono ancora impilate ordinatamente.\n"
	  "Scudi con il simbolo del cavallo bianco.\n"
	  "Un suono lontano si spegne appena volgi la testa.\n"
	  "La luce vacilla come se la stanza respirasse.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
	{ "Camera del Corno di Helm",
	  "Un corno antico giace su un piedistallo.\n"
	  "La sua eco terrorizza ancora chi la sente.\n"
	  "La luce vacilla come se la stanza respirasse.\n"
	  "L'eco dei passi rimbalza su pareti vicine.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
};

static const ProcRoomTemplate kTheme33Traps[] = {
	{ "Sala del Crollo",
	  "Macigni bloccano passaggi; solo sentieri stretti restano.\n"
	  "Polvere si solleva ad ogni passo.\n"
	  "La luce vacilla come se la stanza respirasse.\n"
	  "L'eco dei passi rimbalza su pareti vicine.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
	{ "Passaggio dell'Assedio",
	  "Dardi incastrati nella pietra segnano un assalto.\n"
	  "Olio bollente ha bruciato il pavimento.\n"
	  "L'eco dei passi rimbalza su pareti vicine.\n"
	  "Polvere antica si solleva a ogni movimento.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
};

static const ProcRoomTemplate kTheme33Boss = {
	"Sala dell'Ultima Resistenza",
	  "Una grande sala ospita resti di un'ultima difesa eroica.\n"
	  "Corpi fossilizzati tengono ancora le armi.\n"
	  "Un campione spettrale sorveglia il varco finale.\n"
	  "La pressione dell'aria aumenta verso il centro della sala.\n",
	SECT_INSIDE, static_cast<long>(INDOORS | DARK)
};

/* --- Tema 34: Sentieri dei Morti --- */

static const ProcRoomTemplate kTheme34Entrances[] = {
	{ "Porta dei Giuramenti",
	  "Un arco di pietra reca iscrizioni di giuramenti infranti.\n"
	  "Aria gelida esce dal tunnel sotterraneo.\n"
	  "L'eco dei passi rimbalza su pareti vicine.\n"
	  "Polvere antica si solleva a ogni movimento.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
	{ "Ingresso della Montagna",
	  "Pilastri scolpiti raffigurano re spettrali.\n"
	  "Nessun suono vivo proviene dall'interno.\n"
	  "Qualcosa nel profondo sembra richiamarti oltre la soglia.\n"
	  "La transizione verso l'interno e' netta e irreversibile.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
};

static const ProcRoomTemplate kTheme34Corridors[] = {
	{ "Galleria dei Re Spettrali",
	  "Ombre di corone passano attraverso le pareti.\n"
	  "Passi senza corpo echeggiano accanto ai tuoi.\n"
	  "Polvere antica si solleva a ogni movimento.\n"
	  "Una corrente d'aria fredda tradisce un varco nascosto.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
	{ "Corridoio dei Giuramenti",
	  "Voci sussurrano promesse mai mantenute.\n"
	  "Il freddo aumenta ad ogni passo.\n"
	  "L'eco dei passi rimbalza su pareti vicine.\n"
	  "Polvere antica si solleva a ogni movimento.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
	{ "Passaggio delle Lame Spettrali",
	  "Spade fantasma sfiorano la tua carne senza tagliare.\n"
	  "Terrore antico ti paralizza un istante.\n"
	  "Polvere antica si solleva a ogni movimento.\n"
	  "Una corrente d'aria fredda tradisce un varco nascosto.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
	{ "Atrio del Re Oathbreaker",
	  "Un trono vuoto domina una sala di ombre.\n"
	  "Stendardi polverosi cadono a pezzi.\n"
	  "Il silenzio qui ha peso, come se qualcuno ascoltasse.\n"
	  "Il passaggio si restringe e poi torna ad aprirsi senza logica.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
};

static const ProcRoomTemplate kTheme34Treasures[] = {
	{ "Reliquie dei Caduti",
	  "Armi spettrali giacciono incorrotte.\n"
	  "Medaglie di onore dimenticate brillano debolmente.\n"
	  "Un luccichio falso attira lo sguardo verso gli angoli.\n"
	  "L'aria qui e' piu' ferma, come in attesa di una scelta.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
	{ "Camera dei Sigilli Spezzati",
	  "Sigilli di cera nera sono stati infranti.\n"
	  "Pergamene di giuramento giacciono sparse.\n"
	  "Ombre lunghe si allungano dove la luce finisce.\n"
	  "Odori stagnanti si mescolano a tracce di ferro e pietra.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
};

static const ProcRoomTemplate kTheme34Traps[] = {
	{ "Sala del Terrore",
	  "Visioni dei tuoi peccati appaiono sulle pareti.\n"
	  "La mente vacilla per un istante.\n"
	  "Ombre lunghe si allungano dove la luce finisce.\n"
	  "Odori stagnanti si mescolano a tracce di ferro e pietra.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
	{ "Passaggio del Vuoto",
	  "Il pavimento sembra non esistere sotto i piedi.\n"
	  "Solo la volonta' ti tiene in piedi.\n"
	  "Una corrente d'aria fredda tradisce un varco nascosto.\n"
	  "Ombre lunghe si allungano dove la luce finisce.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
};

static const ProcRoomTemplate kTheme34Boss = {
	"Trono dei Re Spettrali",
	  "Una sala infinita ospita ombre di re millenari.\n"
	  "Corone spettrali brillano nella penombra.\n"
	  "Il sovrano dei morti pronuncia il tuo destino.\n"
	  "L'atmosfera effimera rende incerto cio' che vedi e cio' che ricordi.\n",
	SECT_INSIDE, static_cast<long>(INDOORS | DARK)
};

/* --- Tema 35: Fosse di Isengard --- */

static const ProcRoomTemplate kTheme35Entrances[] = {
	{ "Pozzo di Orthanc",
	  "Un pozzo verticale scende verso officine sotterranee.\n"
	  "Odore di carbone e metallo riempie l'aria.\n"
	  "Il silenzio qui ha peso, come se qualcuno ascoltasse.\n"
	  "Qualcosa nel profondo sembra richiamarti oltre la soglia.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK | UNDERGROUND) },
	{ "Tunnel degli Orchi",
	  "Pareti squadrate con precisione innaturale.\n"
	  "Macchine di legno e ferro bloccano passaggi.\n"
	  "Per un istante hai la sensazione di essere osservato.\n"
	  "Un suono lontano si spegne appena volgi la testa.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK | UNDERGROUND) },
};

static const ProcRoomTemplate kTheme35Corridors[] = {
	{ "Galleria delle Macchine",
	  "Ingranaggi immensi sono fermi ma minacciosi.\n"
	  "Catene spesse pendono dal soffitto.\n"
	  "Una corrente d'aria fredda tradisce un varco nascosto.\n"
	  "Ombre lunghe si allungano dove la luce finisce.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK | UNDERGROUND) },
	{ "Corridoio del Carbone",
	  "Carbone nero copre ogni superficie.\n"
	  "Polvere nera irrita occhi e gola.\n"
	  "Un suono lontano si spegne appena volgi la testa.\n"
	  "La luce vacilla come se la stanza respirasse.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK | UNDERGROUND) },
	{ "Passaggio dei Fabbri",
	  "Incudini e martelli giacciono abbandonati.\n"
	  "Lame di ferro sporgono dal pavimento.\n"
	  "L'eco dei passi rimbalza su pareti vicine.\n"
	  "Polvere antica si solleva a ogni movimento.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK | UNDERGROUND) },
	{ "Atrio del Mago Traditore",
	  "Libri bruciati e strumenti spezzati coprono il pavimento.\n"
	  "Una sfera di cristallo e' incrinata.\n"
	  "Per un istante hai la sensazione di essere osservato.\n"
	  "Un suono lontano si spegne appena volgi la testa.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK | UNDERGROUND) },
};

static const ProcRoomTemplate kTheme35Treasures[] = {
	{ "Armeria di Isengard",
	  "Armature uniformi sono impilate per migliaia.\n"
	  "Elmi con la S bianca dominano la sala.\n"
	  "Il silenzio qui ha peso, come se qualcuno ascoltasse.\n"
	  "Un luccichio falso attira lo sguardo verso gli angoli.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK | UNDERGROUND) },
	{ "Camera dei Piani",
	  "Mappe di invasioni coprono ogni parete.\n"
	  "Un forziere contiene ordini sigillati.\n"
	  "Un luccichio falso attira lo sguardo verso gli angoli.\n"
	  "L'aria qui e' piu' ferma, come in attesa di una scelta.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK | UNDERGROUND) },
};

static const ProcRoomTemplate kTheme35Traps[] = {
	{ "Sala delle Ruote",
	  "Ruote dentate si muovono al minimo tocco.\n"
	  "Evitare i meccanismi e' questione di vita.\n"
	  "Il pericolo qui non si annuncia fino a un passo di troppo.\n"
	  "L'atmosfera effimera rende incerto cio' che vedi e cio' che ricordi.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK | UNDERGROUND) },
	{ "Passaggio del Fuoco di Forgia",
	  "Getti di vapore bollente fuoriescono dalle pareti.\n"
	  "Il metallo rovente brucia ancora.\n"
	  "Il pericolo qui non si annuncia fino a un passo di troppo.\n"
	  "L'atmosfera effimera rende incerto cio' che vedi e cio' che ricordi.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK | UNDERGROUND) },
};

static const ProcRoomTemplate kTheme35Boss = {
	"Cuore della Macchina",
	  "Una sala meccanica culmina in un nucleo di ingranaggi.\n"
	  "Vapore e fumo oscurano la vista.\n"
	  "Il signore delle fosse emerge tra le macchine.\n"
	  "Odori stagnanti si mescolano a tracce di ferro e pietra.\n",
	SECT_INSIDE, static_cast<long>(INDOORS | DARK | UNDERGROUND)
};

/* --- Tema 36: Tunnels di Ungol --- */

static const ProcRoomTemplate kTheme36Entrances[] = {
	{ "Fessura della Ragnatela",
	  "Seta densa blocca il passaggio come nebbia solida.\n"
	  "Insetti impalati pendono ovunque.\n"
	  "Una corrente d'aria fredda tradisce un varco nascosto.\n"
	  "Ombre lunghe si allungano dove la luce finisce.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
	{ "Bocca del Labirinto",
	  "Tunnel intrecciati perdono ogni orientamento.\n"
	  "Ossa avvolte in seta ricoprono il pavimento.\n"
	  "Per un istante hai la sensazione di essere osservato.\n"
	  "Un suono lontano si spegne appena volgi la testa.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
};

static const ProcRoomTemplate kTheme36Corridors[] = {
	{ "Galleria delle Paralisi",
	  "Ragnatele appiccicose bloccano braccia e gambe.\n"
	  "Qualcosa striscia sopra la tua testa.\n"
	  "La luce vacilla come se la stanza respirasse.\n"
	  "L'eco dei passi rimbalza su pareti vicine.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
	{ "Corridoio dei Sacchi",
	  "Sacchi di seta contengono prede ancora vive.\n"
	  "Movimenti deboli tradiscono vittime recenti.\n"
	  "Il silenzio qui ha peso, come se qualcuno ascoltasse.\n"
	  "Il passaggio si restringe e poi torna ad aprirsi senza logica.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
	{ "Passaggio delle Zanne",
	  "Zanne di ragni giganti decorano le pareti.\n"
	  "Liquido digestivo gocciola dal soffitto.\n"
	  "Ombre lunghe si allungano dove la luce finisce.\n"
	  "Odori stagnanti si mescolano a tracce di ferro e pietra.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
	{ "Atrio della Regina",
	  "Un trono di ossa e seta domina la sala.\n"
	  "Ragni di ogni dimensione attendono ordini.\n"
	  "Per un istante hai la sensazione di essere osservato.\n"
	  "Un suono lontano si spegne appena volgi la testa.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
};

static const ProcRoomTemplate kTheme36Treasures[] = {
	{ "Tesoro delle Vittime",
	  "Equipaggiamento degli avventurieri e' ammucchiato con cura.\n"
	  "Anelli e monete brillano tra la seta.\n"
	  "Un suono lontano si spegne appena volgi la testa.\n"
	  "La luce vacilla come se la stanza respirasse.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
	{ "Camera delle Uova",
	  "Uova di ragno pulsano di vita imminente.\n"
	  "Larve strisciano in cumuli viscidi.\n"
	  "L'eco dei passi rimbalza su pareti vicine.\n"
	  "Polvere antica si solleva a ogni movimento.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
};

static const ProcRoomTemplate kTheme36Traps[] = {
	{ "Trappola della Ragnatela",
	  "Filamenti si tendono al passaggio tagliando la fuga.\n"
	  "Veleno paralizzante impregna ogni filo.\n"
	  "L'eco dei passi rimbalza su pareti vicine.\n"
	  "Polvere antica si solleva a ogni movimento.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
	{ "Pozza di Veleno",
	  "Liquido verde corrode armatura e carne.\n"
	  "Vapori acidi rendono difficile respirare.\n"
	  "Una corrente d'aria fredda tradisce un varco nascosto.\n"
	  "Ombre lunghe si allungano dove la luce finisce.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
};

static const ProcRoomTemplate kTheme36Boss = {
	"Nido della Regina Ragno",
	  "Una caverna immensa e' tappezzata di seta nera.\n"
	  "Occhi multipli brillano nel buio sopra di te.\n"
	  "La regina dei ragni scende dal soffitto.\n"
	  "Il silenzio qui ha peso, come se qualcuno ascoltasse.\n",
	SECT_INSIDE, static_cast<long>(INDOORS | DARK)
};

/* --- Tema 37: Bosco di Mirkwood --- */

static const ProcRoomTemplate kTheme37Entrances[] = {
	{ "Sentiero Oscuro",
	  "Alberi contorti chiudono il cielo a una fessura.\n"
	  "Nessun raggio di sole raggiunge il suolo.\n"
	  "Per un istante hai la sensazione di essere osservato.\n"
	  "Un suono lontano si spegne appena volgi la testa.\n",
	  SECT_FOREST, static_cast<long>(INDOORS | DARK) },
	{ "Varco dei Funghi",
	  "Funghi luminosi tingono il bosco di verde pallido.\n"
	  "Spore dense rendono l'aria pesante.\n"
	  "Luce verdastra filtra a macchia di leopardo tra le fronde.\n"
	  "Radici nodose affondano nel terreno molle.\n",
	  SECT_FOREST, static_cast<long>(INDOORS | DARK) },
};

static const ProcRoomTemplate kTheme37Corridors[] = {
	{ "Corridoio degli Alberi",
	  "Rami bassi costringono a chinare la testa.\n"
	  "Ragni e insetti osservano dal fogliame.\n"
	  "Segni sulle pareti indicano che altri sono passati di recente.\n"
	  "L'atmosfera effimera rende incerto cio' che vedi e cio' che ricordi.\n",
	  SECT_FOREST, static_cast<long>(INDOORS | DARK) },
	{ "Galleria dei Sussurri",
	  "Voci sembrano provenire da tronchi cavi.\n"
	  "Il bosco parla in lingue dimenticate.\n"
	  "Segni sulle pareti indicano che altri sono passati di recente.\n"
	  "L'atmosfera effimera rende incerto cio' che vedi e cio' che ricordi.\n",
	  SECT_FOREST, static_cast<long>(INDOORS | DARK) },
	{ "Passaggio del Fiume di Resina",
	  "Resina appiccicosa trattiene ogni passo.\n"
	  "Insetti giganti nuotano nella linfa.\n"
	  "L'atmosfera effimera rende incerto cio' che vedi e cio' che ricordi.\n"
	  "Per un istante hai la sensazione di essere osservato.\n",
	  SECT_FOREST, static_cast<long>(INDOORS | DARK) },
	{ "Radura degli Elfi Prigionieri",
	  "Gabbie di legno sono vuote ma recenti.\n"
	  "Segni di lotta segnano il terreno.\n"
	  "Un suono lontano si spegne appena volgi la testa.\n"
	  "La luce vacilla come se la stanza respirasse.\n",
	  SECT_FOREST, static_cast<long>(INDOORS | DARK) },
};

static const ProcRoomTemplate kTheme37Treasures[] = {
	{ "Tesoro degli Elfi Catturati",
	  "Vesti e armi elfiche giacciono abbandonate.\n"
	  "Fiale di luce ancora brillano debolmente.\n"
	  "Luce verdastra filtra a macchia di leopardo tra le fronde.\n"
	  "Radici nodose affondano nel terreno molle.\n",
	  SECT_FOREST, static_cast<long>(INDOORS | DARK) },
	{ "Camera del Re dei Funghi",
	  "Funghi giganti formano un trono naturale.\n"
	  "Spore dorate cadono come neve.\n"
	  "Luce verdastra filtra a macchia di leopardo tra le fronde.\n"
	  "Radici nodose affondano nel terreno molle.\n",
	  SECT_FOREST, static_cast<long>(INDOORS | DARK) },
};

static const ProcRoomTemplate kTheme37Traps[] = {
	{ "Trappola di Resina",
	  "Resina si indurisce rapidamente attorno alle gambe.\n"
	  "Fuggire diventa quasi impossibile.\n"
	  "Resina e muschio impregnano l'aria umida.\n"
	  "Rami scricchiolano oltre il campo visivo.\n",
	  SECT_FOREST, static_cast<long>(INDOORS | DARK) },
	{ "Sala delle Spine",
	  "Spine di legno spuntano dal suolo al passaggio.\n"
	  "Sangue secco segna il percorso.\n"
	  "Un fruscio lontano si ferma appena ti fermi tu.\n"
	  "Il pericolo qui non si annuncia fino a un passo di troppo.\n",
	  SECT_FOREST, static_cast<long>(INDOORS | DARK) },
};

static const ProcRoomTemplate kTheme37Boss = {
	"Trono del Bosco Nero",
	  "Un albero colossale ospita un trono di rami neri.\n"
	  "Corvi e pipistrelli volteggiano senza riposo.\n"
	  "Il signore del bosco emerge dall'ombra.\n"
	  "Luce verdastra filtra a macchia di leopardo tra le fronde.\n",
	SECT_FOREST, static_cast<long>(INDOORS | DARK)
};

/* --- Tema 38: Cantine di Bree --- */

static const ProcRoomTemplate kTheme38Entrances[] = {
	{ "Botola della Locanda",
	  "Una botola nel pavimento rivela cantine umide.\n"
	  "Odore di birra e muffa si mescolano.\n"
	  "Ombre lunghe si allungano dove la luce finisce.\n"
	  "Odori stagnanti si mescolano a tracce di ferro e pietra.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
	{ "Passaggio Segreto",
	  "Dietro un barile vuoto si apre un corridoio nascosto.\n"
	  "Segni di passaggio recente segnano il fango.\n"
	  "Un suono lontano si spegne appena volgi la testa.\n"
	  "La luce vacilla come se la stanza respirasse.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
};

static const ProcRoomTemplate kTheme38Corridors[] = {
	{ "Corridoio dei Barili",
	  "Barili impilati formano pareti instabili.\n"
	  "Liquido scuro gocciola tra le assi.\n"
	  "Il passaggio si restringe e poi torna ad aprirsi senza logica.\n"
	  "Segni sulle pareti indicano che altri sono passati di recente.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
	{ "Galleria dei Contrabbandieri",
	  "Casse sigillate portano marchi di citta' lontane.\n"
	  "Mappe di rotte segrete coprono un tavolo.\n"
	  "Per un istante hai la sensazione di essere osservato.\n"
	  "Un suono lontano si spegne appena volgi la testa.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
	{ "Passaggio dei Ratti",
	  "Ratti grossi come gatti osservano dall'ombra.\n"
	  "Ossa rosicchiate coprono il pavimento.\n"
	  "L'eco dei passi rimbalza su pareti vicine.\n"
	  "Polvere antica si solleva a ogni movimento.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
	{ "Atrio del Capitano della Guardia",
	  "Armi confiscate sono ammucchiate in un angolo.\n"
	  "Una scrivania coperta di rapporti attende.\n"
	  "La luce vacilla come se la stanza respirasse.\n"
	  "L'eco dei passi rimbalza su pareti vicine.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
};

static const ProcRoomTemplate kTheme38Treasures[] = {
	{ "Cassa del Contrabbando",
	  "Spezie rare e gioielli sono nascosti sotto fieno.\n"
	  "Monete di ogni regno riempiono sacchi.\n"
	  "Una corrente d'aria fredda tradisce un varco nascosto.\n"
	  "Ombre lunghe si allungano dove la luce finisce.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
	{ "Forziere della Locanda",
	  "Oro della locanda e' custodito dietro serrature multiple.\n"
	  "Chiavi pendono da un chiodo vicino.\n"
	  "Ombre lunghe si allungano dove la luce finisce.\n"
	  "Odori stagnanti si mescolano a tracce di ferro e pietra.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
};

static const ProcRoomTemplate kTheme38Traps[] = {
	{ "Trappola del Barile",
	  "Un barile cede rivelando punte di ferro sotto.\n"
	  "Il contrabbandiere conosceva questo trucco.\n"
	  "Il silenzio qui ha peso, come se qualcuno ascoltasse.\n"
	  "Il pericolo qui non si annuncia fino a un passo di troppo.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
	{ "Sala della Birra Avvelenata",
	  "Vapori alcolici nascondono gas soporiferi.\n"
	  "La testa gira dopo pochi respiri.\n"
	  "Una corrente d'aria fredda tradisce un varco nascosto.\n"
	  "Ombre lunghe si allungano dove la luce finisce.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
};

static const ProcRoomTemplate kTheme38Boss = {
	"Cantina del Padrone Oscuro",
	  "La cantina piu' profonda ospita un tavolo rotondo.\n"
	  "Candele nere illuminano volti nascosti.\n"
	  "Il padrone dei bassifondi si alza per accoglierti.\n"
	  "Qualcosa di antico domina lo spazio con presenza opprimente.\n",
	SECT_INSIDE, static_cast<long>(INDOORS | DARK)
};

/* --- Tema 39: Tesoro di Smaug --- */

static const ProcRoomTemplate kTheme39Entrances[] = {
	{ "Fessura della Montagna",
	  "Una fessura nella roccia rivela luccichio interno.\n"
	  "Calore antico ancora aleggia nell'aria.\n"
	  "Polvere antica si solleva a ogni movimento.\n"
	  "Una corrente d'aria fredda tradisce un varco nascosto.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
	{ "Varco del Drago",
	  "Artigli profondi segnano le pareti all'ingresso.\n"
	  "Ossa calcinate giacciono ammucchiate.\n"
	  "La transizione verso l'interno e' netta e irreversibile.\n"
	  "L'atmosfera effimera rende incerto cio' che vedi e cio' che ricordi.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
};

static const ProcRoomTemplate kTheme39Corridors[] = {
	{ "Galleria dell'Oro",
	  "Monete e gemme coprono il pavimento come ghiaia.\n"
	  "Ogni passo affonda nel tesoro.\n"
	  "Il passaggio si restringe e poi torna ad aprirsi senza logica.\n"
	  "Segni sulle pareti indicano che altri sono passati di recente.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
	{ "Corridoio delle Ossa",
	  "Crani di nani e uomini formano cumuli macabri.\n"
	  "Armature fuse dal fuoco del drago giacciono sparse.\n"
	  "Per un istante hai la sensazione di essere osservato.\n"
	  "Un suono lontano si spegne appena volgi la testa.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
	{ "Passaggio delle Coppe",
	  "Coppe d'oro e argento sono ammucchiate a torri.\n"
	  "Pietre preziose cadono dai mucchi instabili.\n"
	  "Odori stagnanti si mescolano a tracce di ferro e pietra.\n"
	  "Il silenzio qui ha peso, come se qualcuno ascoltasse.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
	{ "Atrio del Letto del Drago",
	  "Un'impronta immensa segna dove il drago dorme.\n"
	  "Cenere e braci spente coprono tutto.\n"
	  "Odori stagnanti si mescolano a tracce di ferro e pietra.\n"
	  "Il silenzio qui ha peso, come se qualcuno ascoltasse.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
};

static const ProcRoomTemplate kTheme39Treasures[] = {
	{ "Cuore del Tesoro",
	  "La pila piu' alta di oro e gemme domina la sala.\n"
	  "L'Arkenstone stessa sembra brillare da qualche parte.\n"
	  "L'eco dei passi rimbalza su pareti vicine.\n"
	  "Polvere antica si solleva a ogni movimento.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
	{ "Camera delle Armi Fuse",
	  "Armi naniche fuse in lingotti d'oro impossibili.\n"
	  "Tributi di regni interi giacciono qui.\n"
	  "Polvere antica si solleva a ogni movimento.\n"
	  "Una corrente d'aria fredda tradisce un varco nascosto.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
};

static const ProcRoomTemplate kTheme39Traps[] = {
	{ "Trappola del Tesoro Instabile",
	  "Mucchi di monete cedono rivelando fosse sotto.\n"
	  "Cadere significa scomparire nell'oro.\n"
	  "Il pericolo qui non si annuncia fino a un passo di troppo.\n"
	  "L'atmosfera effimera rende incerto cio' che vedi e cio' che ricordi.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
	{ "Sala del Fuoco Residuo",
	  "Braci ancora calde bruciano sotto il tesoro.\n"
	  "Il calore aumenta verso il centro.\n"
	  "Una corrente d'aria fredda tradisce un varco nascosto.\n"
	  "Ombre lunghe si allungano dove la luce finisce.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
};

static const ProcRoomTemplate kTheme39Boss = {
	"Sala del Drago Caduto",
	  "Una caverna immensa ospita resti di un drago gigante.\n"
	  "Oro copre ogni centimetro come neve dorata.\n"
	  "Qualcosa ancora veglia sul tesoro con occhi avidi.\n"
	  "La pressione dell'aria aumenta verso il centro della sala.\n",
	SECT_INSIDE, static_cast<long>(INDOORS | DARK)
};

/* --- Tema 40: Sale di Erebor --- */

static const ProcRoomTemplate kTheme40Entrances[] = {
	{ "Porta di Erebor",
	  "Una porta nanica massiccia reca rune di benvenuto.\n"
	  "Torce di cristallo illuminano il vestibolo.\n"
	  "Ombre lunghe si allungano dove la luce finisce.\n"
	  "Odori stagnanti si mescolano a tracce di ferro e pietra.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK | UNDERGROUND) },
	{ "Scalinata dei Re",
	  "Gradini scolpiti nella roccia salgono maestosi.\n"
	  "Intagli narrano la storia dei nani.\n"
	  "Un suono lontano si spegne appena volgi la testa.\n"
	  "La luce vacilla come se la stanza respirasse.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK | UNDERGROUND) },
};

static const ProcRoomTemplate kTheme40Corridors[] = {
	{ "Galleria dei Sette Signori",
	  "Statue di sette re nanici fiancheggiano il cammino.\n"
	  "Mazze e asce sono incastonate nelle nicchie.\n"
	  "Odori stagnanti si mescolano a tracce di ferro e pietra.\n"
	  "Il silenzio qui ha peso, come se qualcuno ascoltasse.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK | UNDERGROUND) },
	{ "Corridoio del Mithril",
	  "Filamenti d'argento corrono lungo le pareti.\n"
	  "Picconi abbandonati segnano un'era d'oro.\n"
	  "Una corrente d'aria fredda tradisce un varco nascosto.\n"
	  "Ombre lunghe si allungano dove la luce finisce.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK | UNDERGROUND) },
	{ "Passaggio delle Fornaci",
	  "Le fornaci naniche sono ancora tiepide.\n"
	  "Lingotti d'oro e argento attendono la forgia.\n"
	  "L'atmosfera effimera rende incerto cio' che vedi e cio' che ricordi.\n"
	  "Per un istante hai la sensazione di essere osservato.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK | UNDERGROUND) },
	{ "Atrio del Trono di Pietra",
	  "Un trono scolpito nella roccia domina la sala.\n"
	  "Gemme incastonate brillano come stelle.\n"
	  "La luce vacilla come se la stanza respirasse.\n"
	  "L'eco dei passi rimbalza su pareti vicine.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK | UNDERGROUND) },
};

static const ProcRoomTemplate kTheme40Treasures[] = {
	{ "Tesoro dei Re Nani",
	  "Forzieri nanici traboccano di gemme e metallo.\n"
	  "Corone dimenticate brillano sulla pietra.\n"
	  "La luce vacilla come se la stanza respirasse.\n"
	  "L'eco dei passi rimbalza su pareti vicine.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK | UNDERGROUND) },
	{ "Armeria Reale",
	  "Armature naniche di qualita' leggendaria giacciono impilate.\n"
	  "Asce e martelli runici attendono un guerriero.\n"
	  "L'eco dei passi rimbalza su pareti vicine.\n"
	  "Polvere antica si solleva a ogni movimento.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK | UNDERGROUND) },
};

static const ProcRoomTemplate kTheme40Traps[] = {
	{ "Sala del Ponte Spezzato",
	  "Un ponte nanico e' crollato nel baratro.\n"
	  "Solo una trave instabile resta.\n"
	  "Il pericolo qui non si annuncia fino a un passo di troppo.\n"
	  "L'atmosfera effimera rende incerto cio' che vedi e cio' che ricordi.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK | UNDERGROUND) },
	{ "Passaggio delle Lame Rotanti",
	  "Lame di pietra si muovono con meccanismi nanici.\n"
	  "Solo i nani conoscono ancora il ritmo.\n"
	  "L'eco dei passi rimbalza su pareti vicine.\n"
	  "Polvere antica si solleva a ogni movimento.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK | UNDERGROUND) },
};

static const ProcRoomTemplate kTheme40Boss = {
	"Trono Sotto la Montagna",
	  "La sala del trono di Erebor risplende di gemme.\n"
	  "Corone naniche pendono dal soffitto scolpito.\n"
	  "L'ultimo guardiano delle sale si alza ad accoglierti.\n"
	  "Una corrente d'aria fredda tradisce un varco nascosto.\n",
	SECT_INSIDE, static_cast<long>(INDOORS | DARK | UNDERGROUND)
};

/* --- Tema 41: Rova Nera Profonda --- */

static const ProcRoomTemplate kTheme41Entrances[] = {
	{ "Crepaccio della Rova",
	  "Un crepaccio nella roccia scende verso l'oscuro.\n"
	  "Funghi luminosi tingono tutto di viola.\n"
	  "Odori stagnanti si mescolano a tracce di ferro e pietra.\n"
	  "Il silenzio qui ha peso, come se qualcuno ascoltasse.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
	{ "Ponte di Ossidiana",
	  "Un ponte sottile di ossidiana attraversa un abisso.\n"
	  "Stalattiti come lance puntano verso il basso.\n"
	  "Ombre lunghe si allungano dove la luce finisce.\n"
	  "Odori stagnanti si mescolano a tracce di ferro e pietra.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
};

static const ProcRoomTemplate kTheme41Corridors[] = {
	{ "Galleria dei Drow",
	  "Intagli drow raffigurano divinita' aracnide.\n"
	  "Ragnatele decorate con fili d'argento ricoprono le pareti.\n"
	  "Il silenzio qui ha peso, come se qualcuno ascoltasse.\n"
	  "Il passaggio si restringe e poi torna ad aprirsi senza logica.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
	{ "Corridoio dei Matroni",
	  "Troni vuoti di matroni drow segnano il cammino.\n"
	  "Veleno secco macchia le pareti.\n"
	  "Un suono lontano si spegne appena volgi la testa.\n"
	  "La luce vacilla come se la stanza respirasse.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
	{ "Passaggio delle Balestre",
	  "Balestre nascoste puntano verso il centro.\n"
	  "Dardi avvelenati giacciono in mucchi.\n"
	  "Odori stagnanti si mescolano a tracce di ferro e pietra.\n"
	  "Il silenzio qui ha peso, come se qualcuno ascoltasse.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
	{ "Atrio del Tempio di Lolth",
	  "Un altare di ossidiana domina la sala.\n"
	  "Ragni sacri tessono intorno all'altare.\n"
	  "Odori stagnanti si mescolano a tracce di ferro e pietra.\n"
	  "Il silenzio qui ha peso, come se qualcuno ascoltasse.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
};

static const ProcRoomTemplate kTheme41Treasures[] = {
	{ "Tesoro della Matrona",
	  "Gioielli drow e armi di ossidiana brillano.\n"
	  "Schizzi di veleno proteggono i forzieri.\n"
	  "Per un istante hai la sensazione di essere osservato.\n"
	  "Un suono lontano si spegne appena volgi la testa.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
	{ "Camera degli Incantesimi",
	  "Pergamene drow recanti incantesimi proibiti giacciono impilate.\n"
	  "Fiale di oscurita' pulsano debolmente.\n"
	  "Ombre lunghe si allungano dove la luce finisce.\n"
	  "Odori stagnanti si mescolano a tracce di ferro e pietra.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
};

static const ProcRoomTemplate kTheme41Traps[] = {
	{ "Trappola del Veleno",
	  "Gas viola fuoriesce da fessure nel pavimento.\n"
	  "La pelle brucia al contatto.\n"
	  "Odori stagnanti si mescolano a tracce di ferro e pietra.\n"
	  "Il silenzio qui ha peso, come se qualcuno ascoltasse.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
	{ "Sala delle Ragnatele",
	  "Ragnatele bloccano ogni via di fuga.\n"
	  "Ragni avvelenati scendono dal soffitto.\n"
	  "Una corrente d'aria fredda tradisce un varco nascosto.\n"
	  "Ombre lunghe si allungano dove la luce finisce.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
};

static const ProcRoomTemplate kTheme41Boss = {
	"Santuario della Regina Ragno",
	  "Un tempio sotterraneo ospita un trono di ragnatele.\n"
	  "Ragni giganti servono una matrona drow.\n"
	  "La regina della rova nera si alza dal trono.\n"
	  "Una corrente d'aria fredda tradisce un varco nascosto.\n",
	SECT_INSIDE, static_cast<long>(INDOORS | DARK)
};

/* --- Tema 42: Fogne di Waterdeep --- */

static const ProcRoomTemplate kTheme42Entrances[] = {
	{ "Grata delle Fogne",
	  "Una grata arrugginita conduce a tunnel umidi.\n"
	  "Odore di rifiuti e muffa e' soffocante.\n"
	  "Qualcosa nel profondo sembra richiamarti oltre la soglia.\n"
	  "La transizione verso l'interno e' netta e irreversibile.\n",
	  SECT_WATER_NOSWIM, static_cast<long>(INDOORS | DARK) },
	{ "Pozzo di Scarico",
	  "Acqua nera scorre veloce sotto una passerella.\n"
	  "Ratti grossi come gatti osservano dall'ombra.\n"
	  "Onde lievi sfiorano le pareti come un respiro lento.\n"
	  "Schiuma verdastra si accumula negli angoli piu' bassi.\n",
	  SECT_WATER_NOSWIM, static_cast<long>(INDOORS | DARK) },
};

static const ProcRoomTemplate kTheme42Corridors[] = {
	{ "Galleria dei Rifiuti",
	  "Rifiuti di tutta la citta' si accumulano qui.\n"
	  "Gas infiammabili aleggiano bassi.\n"
	  "Schiuma verdastra si accumula negli angoli piu' bassi.\n"
	  "Il passaggio si restringe e poi torna ad aprirsi senza logica.\n",
	  SECT_WATER_NOSWIM, static_cast<long>(INDOORS | DARK) },
	{ "Corridoio delle Piastrelle",
	  "Piastrelle scivolose coprono il pavimento.\n"
	  "Acqua sporca arriva alle caviglie.\n"
	  "Schiuma verdastra si accumula negli angoli piu' bassi.\n"
	  "Il passaggio si restringe e poi torna ad aprirsi senza logica.\n",
	  SECT_WATER_NOSWIM, static_cast<long>(INDOORS | DARK) },
	{ "Passaggio dei Contrabbandieri",
	  "Casse di merci rubate bloccano il cammino.\n"
	  "Segni della Gilda dei Ladri sono ovunque.\n"
	  "La luce vacilla come se la stanza respirasse.\n"
	  "Acqua scura riflette poco e nasconde molto.\n",
	  SECT_WATER_NOSWIM, static_cast<long>(INDOORS | DARK) },
	{ "Atrio del Capo Ladro",
	  "Un tavolo coperto di mappe della citta'.\n"
	  "Monete contate in mucchi ordinati.\n"
	  "Un suono lontano si spegne appena volgi la testa.\n"
	  "La luce vacilla come se la stanza respirasse.\n",
	  SECT_WATER_NOSWIM, static_cast<long>(INDOORS | DARK) },
};

static const ProcRoomTemplate kTheme42Treasures[] = {
	{ "Bottino della Gilda",
	  "Tesori rubati da tutta Waterdeep.\n"
	  "Gioielli e artefatti in casse sigillate.\n"
	  "L'aria qui e' piu' ferma, come in attesa di una scelta.\n"
	  "L'atmosfera effimera rende incerto cio' che vedi e cio' che ricordi.\n",
	  SECT_WATER_NOSWIM, static_cast<long>(INDOORS | DARK) },
	{ "Camera dei Contratti",
	  "Contratti su pergamena con sangue come inchiostro.\n"
	  "Sigilli della gilda su ogni documento.\n"
	  "Acqua scura riflette poco e nasconde molto.\n"
	  "Un odore di alghe marcite aleggia sopra la superficie.\n",
	  SECT_WATER_NOSWIM, static_cast<long>(INDOORS | DARK) },
};

static const ProcRoomTemplate kTheme42Traps[] = {
	{ "Trappola della Grata",
	  "Una grata cede rivelando acque profonde sotto.\n"
	  "Correnti trascinano verso l'ignoto.\n"
	  "Un odore di alghe marcite aleggia sopra la superficie.\n"
	  "Onde lievi sfiorano le pareti come un respiro lento.\n",
	  SECT_WATER_NOSWIM, static_cast<long>(INDOORS | DARK) },
	{ "Sala del Gas",
	  "Gas infiammabile si accumula in una nicchia.\n"
	  "Una scintilla basterebbe.\n"
	  "Onde lievi sfiorano le pareti come un respiro lento.\n"
	  "Schiuma verdastra si accumula negli angoli piu' bassi.\n",
	  SECT_WATER_NOSWIM, static_cast<long>(INDOORS | DARK) },
};

static const ProcRoomTemplate kTheme42Boss = {
	"Covo del Principe dei Ladri",
	  "Una sala asciutta sorprende in mezzo alle fogne.\n"
	  "Tappeti e candele contrastano con l'orrido esterno.\n"
	  "Il principe dei ladri attende con un sorriso.\n"
	  "Acqua scura riflette poco e nasconde molto.\n",
	SECT_WATER_NOSWIM, static_cast<long>(INDOORS | DARK)
};

/* --- Tema 43: Rovine di Neverwinter --- */

static const ProcRoomTemplate kTheme43Entrances[] = {
	{ "Porta Ghiacciata",
	  "Ghiaccio copre un arco di pietra crollato.\n"
	  "Aria gelida esce dalle rovine interne.\n"
	  "Per un istante hai la sensazione di essere osservato.\n"
	  "Un suono lontano si spegne appena volgi la testa.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
	{ "Varco della Citta' Morta",
	  "Rovine di una grande citta' si estendono nel buio.\n"
	  "Neve eterna copre ogni superficie.\n"
	  "Il silenzio qui ha peso, come se qualcuno ascoltasse.\n"
	  "Qualcosa nel profondo sembra richiamarti oltre la soglia.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
};

static const ProcRoomTemplate kTheme43Corridors[] = {
	{ "Galleria del Gelo",
	  "Stalattiti di ghiaccio pendono minacciosamente.\n"
	  "Il pavimento e' una lastra scivolosa.\n"
	  "Il silenzio qui ha peso, come se qualcuno ascoltasse.\n"
	  "Il passaggio si restringe e poi torna ad aprirsi senza logica.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
	{ "Corridoio dei Cavalieri di Ghiaccio",
	  "Statue di ghiaccio raffigurano cavalieri.\n"
	  "Alcune sembrano muoversi nel buio.\n"
	  "Una corrente d'aria fredda tradisce un varco nascosto.\n"
	  "Ombre lunghe si allungano dove la luce finisce.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
	{ "Passaggio della Tempesta",
	  "Vento gelido entra da fessure nel muro.\n"
	  "Neve si accumula in cumuli alti.\n"
	  "Per un istante hai la sensazione di essere osservato.\n"
	  "Un suono lontano si spegne appena volgi la testa.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
	{ "Atrio del Palazzo Congelato",
	  "Un trono di ghiaccio domina una sala abbandonata.\n"
	  "Stendardi congelati cadono a pezzi.\n"
	  "Segni sulle pareti indicano che altri sono passati di recente.\n"
	  "L'atmosfera effimera rende incerto cio' che vedi e cio' che ricordi.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
};

static const ProcRoomTemplate kTheme43Treasures[] = {
	{ "Tesoro del Re di Ghiaccio",
	  "Gemme congelate brillano come stelle.\n"
	  "Oro incastonato nel ghiaccio eterno.\n"
	  "Per un istante hai la sensazione di essere osservato.\n"
	  "Un suono lontano si spegne appena volgi la testa.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
	{ "Camera delle Armi Gelide",
	  "Armi di ghiaccio magico ancora intatte.\n"
	  "Un freddo che non si scioglie mai.\n"
	  "Per un istante hai la sensazione di essere osservato.\n"
	  "Un suono lontano si spegne appena volgi la testa.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
};

static const ProcRoomTemplate kTheme43Traps[] = {
	{ "Sala del Ghiaccio Sottile",
	  "Il pavimento di ghiaccio cede al peso.\n"
	  "Acqua gelida sotto minaccia di affogare.\n"
	  "Un suono lontano si spegne appena volgi la testa.\n"
	  "La luce vacilla come se la stanza respirasse.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
	{ "Passaggio delle Valanghe",
	  "Neve cade dal soffitto senza preavviso.\n"
	  "Essere sepolti e' un rischio costante.\n"
	  "L'atmosfera effimera rende incerto cio' che vedi e cio' che ricordi.\n"
	  "Per un istante hai la sensazione di essere osservato.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
};

static const ProcRoomTemplate kTheme43Boss = {
	"Trono del Re di Ghiaccio",
	  "Un palazzo congelato culmina in un trono di cristallo.\n"
	  "Tempesta eterna ulula fuori dalle mura.\n"
	  "Il re di ghiaccio si alza ad accoglierti.\n"
	  "Per un istante hai la sensazione di essere osservato.\n",
	SECT_INSIDE, static_cast<long>(INDOORS | DARK)
};

/* --- Tema 44: Cripta di Baldur --- */

static const ProcRoomTemplate kTheme44Entrances[] = {
	{ "Porta della Cripta",
	  "Un arco gotico conduce a tombe sotterranee.\n"
	  "Candele spente segnano un abbandono recente.\n"
	  "Un suono lontano si spegne appena volgi la testa.\n"
	  "La luce vacilla come se la stanza respirasse.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
	{ "Scalinata dei Morti",
	  "Gradini consumati scendono tra lapidi inclinate.\n"
	  "Nomi cancellati dal tempo sono incisi sulle pietre.\n"
	  "L'eco dei passi rimbalza su pareti vicine.\n"
	  "Polvere antica si solleva a ogni movimento.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
};

static const ProcRoomTemplate kTheme44Corridors[] = {
	{ "Galleria delle Famiglie",
	  "Tombe di famiglie nobili fiancheggiano il cammino.\n"
	  "Stemmi sbiaditi decorano ogni sarcofago.\n"
	  "Odori stagnanti si mescolano a tracce di ferro e pietra.\n"
	  "Il silenzio qui ha peso, come se qualcuno ascoltasse.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
	{ "Corridoio dei Chierici",
	  "Simboli sacri sono stati profanati.\n"
	  "Libri di preghiera bruciati giacciono sparsi.\n"
	  "Odori stagnanti si mescolano a tracce di ferro e pietra.\n"
	  "Il silenzio qui ha peso, come se qualcuno ascoltasse.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
	{ "Passaggio degli Scheletri",
	  "Scheletri si staccano dalle nicchie al tuo passaggio.\n"
	  "Ossa sbattono in un ritmo sinistro.\n"
	  "Segni sulle pareti indicano che altri sono passati di recente.\n"
	  "L'atmosfera effimera rende incerto cio' che vedi e cio' che ricordi.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
	{ "Atrio del Vampiro",
	  "Un sarcofago di marmo nero domina la sala.\n"
	  "Rose secche e sangue secco decorano l'altare.\n"
	  "Segni sulle pareti indicano che altri sono passati di recente.\n"
	  "L'atmosfera effimera rende incerto cio' che vedi e cio' che ricordi.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
};

static const ProcRoomTemplate kTheme44Treasures[] = {
	{ "Tesoro delle Casate",
	  "Gioielli funerari giacciono accanto alle tombe.\n"
	  "Oro e argento delle famiglie nobili.\n"
	  "Polvere antica si solleva a ogni movimento.\n"
	  "Una corrente d'aria fredda tradisce un varco nascosto.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
	{ "Reliquiario Profanato",
	  "Reliquie sacre rubate e contaminate.\n"
	  "Simboli oscuri sono stati aggiunti.\n"
	  "La luce vacilla come se la stanza respirasse.\n"
	  "L'eco dei passi rimbalza su pareti vicine.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
};

static const ProcRoomTemplate kTheme44Traps[] = {
	{ "Sala delle Lame",
	  "Lame emergono dal pavimento a intervalli.\n"
	  "Un meccanismo attivato dal peso.\n"
	  "Il pericolo qui non si annuncia fino a un passo di troppo.\n"
	  "L'atmosfera effimera rende incerto cio' che vedi e cio' che ricordi.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
	{ "Passaggio del Gas Necrotico",
	  "Gas verdognolo fuoriesce dalle tombe aperte.\n"
	  "Carne marcia e' il prezzo di un respiro lungo.\n"
	  "Polvere antica si solleva a ogni movimento.\n"
	  "Una corrente d'aria fredda tradisce un varco nascosto.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
};

static const ProcRoomTemplate kTheme44Boss = {
	"Cripta del Signore Vampiro",
	  "La cripta piu' profonda ospita un trono di bare.\n"
	  "Pipistrelli volteggiano senza fine.\n"
	  "Il signore vampiro di Baldur si alza ad accoglierti.\n"
	  "Per un istante hai la sensazione di essere osservato.\n",
	SECT_INSIDE, static_cast<long>(INDOORS | DARK)
};

/* --- Tema 45: Fessura Planare --- */

static const ProcRoomTemplate kTheme45Entrances[] = {
	{ "Varco Instabile",
	  "La realta' sembra piegarsi attorno all'ingresso.\n"
	  "Colori impossibili danzano ai bordi della vista.\n"
	  "L'aria qui non obbedisce a nessun nord.\n"
	  "Qualcosa nel profondo sembra richiamarti oltre la soglia.\n",
	  SECT_AIR, static_cast<long>(INDOORS | DARK) },
	{ "Portale Fratturato",
	  "Un portale scintillante mostra mondi paralleli.\n"
	  "Venti da mille direzioni ti spingono.\n"
	  "Fratture nel vetro planare pulsano come vene di luce.\n"
	  "Per un istante vedi te stesso camminare in un mondo specchiato.\n",
	  SECT_AIR, static_cast<long>(INDOORS | DARK) },
};

static const ProcRoomTemplate kTheme45Corridors[] = {
	{ "Galleria dei Frammenti",
	  "Pezzi di citta' diverse fluttuano nel vuoto.\n"
	  "Gravita' e' una suggestione qui.\n"
	  "Il vuoto sotto i piedi non promette alcuna tenuta.\n"
	  "Frammenti di roccia fluttuano appena fuori dal sentiero.\n",
	  SECT_AIR, static_cast<long>(INDOORS | DARK) },
	{ "Corridoio delle Geometrie",
	  "Angoli impossibili formano passaggi.\n"
	  "Camminare in linea retta e' un'illusione.\n"
	  "Un suono lontano si spegne appena volgi la testa.\n"
	  "La luce vacilla come se la stanza respirasse.\n",
	  SECT_AIR, static_cast<long>(INDOORS | DARK) },
	{ "Passaggio dei Sussurri Planari",
	  "Voci di mille mondi parlano simultaneamente.\n"
	  "La mente fatica a restare coesa.\n"
	  "L'atmosfera effimera rende incerto cio' che vedi e cio' che ricordi.\n"
	  "Per un istante hai la sensazione di essere osservato.\n",
	  SECT_AIR, static_cast<long>(INDOORS | DARK) },
	{ "Atrio del Mercante Planare",
	  "Bancarelle impossibili vendono merci di ogni realta'.\n"
	  "Il mercante non ha volto fisso.\n"
	  "Luce lontana filtra da fessure che cambiano forma.\n"
	  "Ogni passo sembra spostarti senza avanzare davvero.\n",
	  SECT_AIR, static_cast<long>(INDOORS | DARK) },
};

static const ProcRoomTemplate kTheme45Treasures[] = {
	{ "Tesoro Planare",
	  "Oggetti di mondi che non esistono piu'.\n"
	  "Monete con simboli sconosciuti.\n"
	  "L'aria qui e' piu' ferma, come in attesa di una scelta.\n"
	  "L'atmosfera effimera rende incerto cio' che vedi e cio' che ricordi.\n",
	  SECT_AIR, static_cast<long>(INDOORS | DARK) },
	{ "Camera delle Chiavi",
	  "Chiavi di portali per ogni piano.\n"
	  "Aprirne una senza sapere dove porta e' folle.\n"
	  "Un luccichio falso attira lo sguardo verso gli angoli.\n"
	  "L'aria qui e' piu' ferma, come in attesa di una scelta.\n",
	  SECT_AIR, static_cast<long>(INDOORS | DARK) },
};

static const ProcRoomTemplate kTheme45Traps[] = {
	{ "Trappola della Gravita'",
	  "La gravita' cambia direzione senza preavviso.\n"
	  "Cadere verso il soffitto e' possibile.\n"
	  "Echi metallici rimbalzano da direzioni impossibili.\n"
	  "Luce lontana filtra da fessure che cambiano forma.\n",
	  SECT_AIR, static_cast<long>(INDOORS | DARK) },
	{ "Sala delle Illusioni Planari",
	  "Copie di te stesso attaccano da ogni angolo.\n"
	  "Quale e' reale?\n"
	  "Ogni passo sembra spostarti senza avanzare davvero.\n"
	  "L'aria qui non obbedisce a nessun nord.\n",
	  SECT_AIR, static_cast<long>(INDOORS | DARK) },
};

static const ProcRoomTemplate kTheme45Boss = {
	"Crocevia di Sigil",
	  "Una piazza fluttuante ospita un crocevia infinito.\n"
	  "Portali verso ogni piano si aprono e chiudono.\n"
	  "Un guardiano planare veglia sul varco.\n"
	  "L'aria qui non obbedisce a nessun nord.\n",
	SECT_AIR, static_cast<long>(INDOORS | DARK)
};

/* --- Tema 46: Castello delle Ombre --- */

static const ProcRoomTemplate kTheme46Entrances[] = {
	{ "Cancello del Castello",
	  "Un cancello di ferro arrugginito scricchiola al vento.\n"
	  "Torri spezzate perforano un cielo grigio.\n"
	  "Il silenzio qui ha peso, come se qualcuno ascoltasse.\n"
	  "Qualcosa nel profondo sembra richiamarti oltre la soglia.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
	{ "Portone Principale",
	  "Un portone massiccio e' socchiuso.\n"
	  "Odore di muffa e cera vecchia esce dall'interno.\n"
	  "Odori stagnanti si mescolano a tracce di ferro e pietra.\n"
	  "Il silenzio qui ha peso, come se qualcuno ascoltasse.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
};

static const ProcRoomTemplate kTheme46Corridors[] = {
	{ "Corridoio delle Tappezzerie",
	  "Arazzi sbiaditi raffigurano cacce macabre.\n"
	  "Occhi sembrano seguirti tra le figure.\n"
	  "La luce vacilla come se la stanza respirasse.\n"
	  "L'eco dei passi rimbalza su pareti vicine.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
	{ "Galleria delle Armature",
	  "Armature vuote sorvegliano ogni passaggio.\n"
	  "Spade arrugginite giacciono ai loro piedi.\n"
	  "Il silenzio qui ha peso, come se qualcuno ascoltasse.\n"
	  "Il passaggio si restringe e poi torna ad aprirsi senza logica.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
	{ "Passaggio delle Candele",
	  "Candele nere illuminano debolmente.\n"
	  "Ombre danzano sulle pareti senza sorgente.\n"
	  "Ombre lunghe si allungano dove la luce finisce.\n"
	  "Odori stagnanti si mescolano a tracce di ferro e pietra.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
	{ "Atrio della Scala",
	  "Una scala a chiocciola sale verso l'oscurita'.\n"
	  "Ragnatele coprono ogni gradino.\n"
	  "Polvere antica si solleva a ogni movimento.\n"
	  "Una corrente d'aria fredda tradisce un varco nascosto.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
};

static const ProcRoomTemplate kTheme46Treasures[] = {
	{ "Tesoro del Conte",
	  "Gioielli e argenteria di famiglie nobili.\n"
	  "Medaglioni con ritratti di defunti.\n"
	  "Ombre lunghe si allungano dove la luce finisce.\n"
	  "Odori stagnanti si mescolano a tracce di ferro e pietra.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
	{ "Biblioteca Proibita",
	  "Libri di necromanzia e vampirismo.\n"
	  "Pagine che si muovono da sole.\n"
	  "La luce vacilla come se la stanza respirasse.\n"
	  "L'eco dei passi rimbalza su pareti vicine.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
};

static const ProcRoomTemplate kTheme46Traps[] = {
	{ "Sala del Pavimento Cedevole",
	  "Assi marce nascondono fosse sotto.\n"
	  "Legno che scricchiola avverte troppo tardi.\n"
	  "Un suono lontano si spegne appena volgi la testa.\n"
	  "La luce vacilla come se la stanza respirasse.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
	{ "Passaggio dei Pipistrelli",
	  "Pipistrelli si lanciano in sciami al passaggio.\n"
	  "Zanne e artigli graffiano da ogni lato.\n"
	  "Una corrente d'aria fredda tradisce un varco nascosto.\n"
	  "Ombre lunghe si allungano dove la luce finisce.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
};

static const ProcRoomTemplate kTheme46Boss = {
	"Sala del Trono Nero",
	  "Una sala gotica culmina in un trono di velluto nero.\n"
	  "Finestre alte mostrano un cielo eternamente notturno.\n"
	  "Il conte delle ombre attende con calice di sangue.\n"
	  "L'eco dei passi rimbalza su pareti vicine.\n",
	SECT_INSIDE, static_cast<long>(INDOORS | DARK)
};

/* --- Tema 47: Antro del Drago --- */

static const ProcRoomTemplate kTheme47Entrances[] = {
	{ "Fessura Fumante",
	  "Fumo esce da una fessura nella montagna.\n"
	  "Zolfo e calore avvertono di cio' che attende.\n"
	  "La transizione verso l'interno e' netta e irreversibile.\n"
	  "L'atmosfera effimera rende incerto cio' che vedi e cio' che ricordi.\n",
	  SECT_MOUNTAIN, static_cast<long>(INDOORS | DARK) },
	{ "Covo del Predatore",
	  "Artigli e scaglie segnano l'ingresso.\n"
	  "Ossa di grandi prede giacciono ammucchiate.\n"
	  "L'atmosfera effimera rende incerto cio' che vedi e cio' che ricordi.\n"
	  "Per un istante hai la sensazione di essere osservato.\n",
	  SECT_MOUNTAIN, static_cast<long>(INDOORS | DARK) },
};

static const ProcRoomTemplate kTheme47Corridors[] = {
	{ "Galleria delle Scaglie",
	  "Scaglie incastonate nella roccia brillano.\n"
	  "Calore aumenta ad ogni passo.\n"
	  "Un suono lontano si spegne appena volgi la testa.\n"
	  "La luce vacilla come se la stanza respirasse.\n",
	  SECT_MOUNTAIN, static_cast<long>(INDOORS | DARK) },
	{ "Corridoio del Tesoro Parziale",
	  "Piccoli tesori precedono il tesoro vero.\n"
	  "Monete e gemme sparse sul pavimento.\n"
	  "Segni sulle pareti indicano che altri sono passati di recente.\n"
	  "L'atmosfera effimera rende incerto cio' che vedi e cio' che ricordi.\n",
	  SECT_MOUNTAIN, static_cast<long>(INDOORS | DARK) },
	{ "Passaggio del Soffio",
	  "Braci ancora calde segnano il passaggio del drago.\n"
	  "Roccia fusa forma stalattiti bizzarre.\n"
	  "Cristalli di ghiaccio scintillano dove passa poca luce.\n"
	  "Il passaggio si restringe e poi torna ad aprirsi senza logica.\n",
	  SECT_MOUNTAIN, static_cast<long>(INDOORS | DARK) },
	{ "Atrio delle Ossa",
	  "Ossa di cavalli, nani e giganti formano muri.\n"
	  "Crani impilati come decorazione.\n"
	  "Per un istante hai la sensazione di essere osservato.\n"
	  "Un suono lontano si spegne appena volgi la testa.\n",
	  SECT_MOUNTAIN, static_cast<long>(INDOORS | DARK) },
};

static const ProcRoomTemplate kTheme47Treasures[] = {
	{ "Tesoro del Drago",
	  "Oro e gemme coprono ogni superficie.\n"
	  "Armi magiche di eroi caduti.\n"
	  "Per un istante hai la sensazione di essere osservato.\n"
	  "Un suono lontano si spegne appena volgi la testa.\n",
	  SECT_MOUNTAIN, static_cast<long>(INDOORS | DARK) },
	{ "Camera delle Uova",
	  "Uova di drago pulsano di calore.\n"
	  "Non dovresti essere qui.\n"
	  "L'aria qui e' piu' ferma, come in attesa di una scelta.\n"
	  "L'atmosfera effimera rende incerto cio' che vedi e cio' che ricordi.\n",
	  SECT_MOUNTAIN, static_cast<long>(INDOORS | DARK) },
};

static const ProcRoomTemplate kTheme47Traps[] = {
	{ "Trappola del Soffio",
	  "Getti di fiamma erompono da fessure.\n"
	  "Il drago conosce ogni centimetro.\n"
	  "Pietra nuda e vento tagliente dominano lo spazio.\n"
	  "La roccia e' fredda al tatto e coperta da polvere sottile.\n",
	  SECT_MOUNTAIN, static_cast<long>(INDOORS | DARK) },
	{ "Sala del Crollo",
	  "Il soffitto e' stato indebolito dal fuoco.\n"
	  "Pietre cadono senza preavviso.\n"
	  "Il pericolo qui non si annuncia fino a un passo di troppo.\n"
	  "L'atmosfera effimera rende incerto cio' che vedi e cio' che ricordi.\n",
	  SECT_MOUNTAIN, static_cast<long>(INDOORS | DARK) },
};

static const ProcRoomTemplate kTheme47Boss = {
	"Covo del Drago Antico",
	  "Una caverna immensa ospita un drago ancora vivo.\n"
	  "Tesoro innumerevole brilla sotto le scaglie.\n"
	  "Il drago apre un occhio e poi l'altro.\n"
	  "Voci lontane si perdono nel ronzio del vuoto.\n",
	SECT_MOUNTAIN, static_cast<long>(INDOORS | DARK)
};

/* --- Tema 48: Caverna degli Occhi --- */

static const ProcRoomTemplate kTheme48Entrances[] = {
	{ "Fessura Aberrante",
	  "Pareti pulsano come carne viva.\n"
	  "Occhi crescono dalla roccia e scompaiono.\n"
	  "Per un istante hai la sensazione di essere osservato.\n"
	  "Un suono lontano si spegne appena volgi la testa.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
	{ "Bocca della Caverna",
	  "Un assalto mentale ti colpisce prima del fisico.\n"
	  "Geometrie sbagliate fanno male agli occhi.\n"
	  "Odori stagnanti si mescolano a tracce di ferro e pietra.\n"
	  "Il silenzio qui ha peso, come se qualcuno ascoltasse.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
};

static const ProcRoomTemplate kTheme48Corridors[] = {
	{ "Galleria delle Tentacoli",
	  "Tentacoli sporgono e si ritirano nelle pareti.\n"
	  "Muco viscido copre ogni superficie.\n"
	  "La luce vacilla come se la stanza respirasse.\n"
	  "L'eco dei passi rimbalza su pareti vicine.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
	{ "Corridoio della Paranoia",
	  "Sussurri nella tua mente senza fonte.\n"
	  "I compagni sembrano nemici per un istante.\n"
	  "Per un istante hai la sensazione di essere osservato.\n"
	  "Un suono lontano si spegne appena volgi la testa.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
	{ "Passaggio delle Illusioni",
	  "Visioni di morte e follia ovunque.\n"
	  "La realta' e' negoziabile qui.\n"
	  "Un suono lontano si spegne appena volgi la testa.\n"
	  "La luce vacilla come se la stanza respirasse.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
	{ "Atrio del Beholder",
	  "Una sfera fluttuante osserva ogni angolo.\n"
	  "Il pavimento e' bruciato dai raggi di energia.\n"
	  "L'atmosfera effimera rende incerto cio' che vedi e cio' che ricordi.\n"
	  "Per un istante hai la sensazione di essere osservato.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
};

static const ProcRoomTemplate kTheme48Treasures[] = {
	{ "Tesoro della Mente",
	  "Cristalli di memoria rubate.\n"
	  "Pensieri altrui fluttuano in fiale.\n"
	  "L'eco dei passi rimbalza su pareti vicine.\n"
	  "Polvere antica si solleva a ogni movimento.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
	{ "Camera degli Esperimenti",
	  "Creature fuse e mutilate in gabbie.\n"
	  "Strumenti chirurgici alieni.\n"
	  "L'atmosfera effimera rende incerto cio' che vedi e cio' che ricordi.\n"
	  "Per un istante hai la sensazione di essere osservato.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
};

static const ProcRoomTemplate kTheme48Traps[] = {
	{ "Trappola del Raggio",
	  "Raggi di disintegrazione sparano dalle pareti.\n"
	  "Occhi nascosti attendono il movimento.\n"
	  "L'eco dei passi rimbalza su pareti vicine.\n"
	  "Polvere antica si solleva a ogni movimento.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
	{ "Sala della Follia",
	  "La mente subisce assalti psichici.\n"
	  "Resistere e' la vera prova.\n"
	  "L'atmosfera effimera rende incerto cio' che vedi e cio' che ricordi.\n"
	  "Per un istante hai la sensazione di essere osservato.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
};

static const ProcRoomTemplate kTheme48Boss = {
	"Nido del Beholder",
	  "Una caverna sferica ospita un beholder antico.\n"
	  "Occhi su tentacoli scrutano ogni difetto.\n"
	  "La creatura alza i suoi raggi mortali.\n"
	  "Il silenzio qui ha peso, come se qualcuno ascoltasse.\n",
	SECT_INSIDE, static_cast<long>(INDOORS | DARK)
};

/* --- Tema 49: Fosso Infernale --- */

static const ProcRoomTemplate kTheme49Entrances[] = {
	{ "Porta Infernale",
	  "Un arco di bronzo reca rune infernali.\n"
	  "Calore e zolfo escono dal varco.\n"
	  "Ombre lunghe si allungano dove la luce finisce.\n"
	  "Odori stagnanti si mescolano a tracce di ferro e pietra.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
	{ "Scala dei Dannati",
	  "Gradini di ossidiana scendono verso il calore.\n"
	  "Voci di tormento salgono dal basso.\n"
	  "Il silenzio qui ha peso, come se qualcuno ascoltasse.\n"
	  "Qualcosa nel profondo sembra richiamarti oltre la soglia.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
};

static const ProcRoomTemplate kTheme49Corridors[] = {
	{ "Galleria delle Catene",
	  "Catene infuocate pendono dal soffitto.\n"
	  "Ceneri di dannati coprono il pavimento.\n"
	  "L'eco dei passi rimbalza su pareti vicine.\n"
	  "Polvere antica si solleva a ogni movimento.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
	{ "Corridoio dei Diavoli",
	  "Impronte di zoccoli bruciate nella pietra.\n"
	  "Odore di zolfo e paura.\n"
	  "Polvere antica si solleva a ogni movimento.\n"
	  "Una corrente d'aria fredda tradisce un varco nascosto.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
	{ "Passaggio del Fuoco Eterno",
	  "Fiamme che non consumano tutto illuminano il cammino.\n"
	  "Calore insopportabile verso il centro.\n"
	  "La luce vacilla come se la stanza respirasse.\n"
	  "L'eco dei passi rimbalza su pareti vicine.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
	{ "Atrio del Principe",
	  "Trono di lava solidificata domina la sala.\n"
	  "Stendardi infernali segnano territorio.\n"
	  "Polvere antica si solleva a ogni movimento.\n"
	  "Una corrente d'aria fredda tradisce un varco nascosto.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
};

static const ProcRoomTemplate kTheme49Treasures[] = {
	{ "Tesoro Infernale",
	  "Oro maledetto e gemme corrotte.\n"
	  "Contratti firmati con sangue.\n"
	  "L'aria qui e' piu' ferma, come in attesa di una scelta.\n"
	  "L'atmosfera effimera rende incerto cio' che vedi e cio' che ricordi.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
	{ "Camera dei Contratti",
	  "Anime imprigionate in pergamene.\n"
	  "Firme ancora fresche su documenti antichi.\n"
	  "Ombre lunghe si allungano dove la luce finisce.\n"
	  "Odori stagnanti si mescolano a tracce di ferro e pietra.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
};

static const ProcRoomTemplate kTheme49Traps[] = {
	{ "Trappola del Fuoco",
	  "Getti di fiamma infernale erompono dal pavimento.\n"
	  "Bruciare e' solo l'inizio.\n"
	  "Il silenzio qui ha peso, come se qualcuno ascoltasse.\n"
	  "Il pericolo qui non si annuncia fino a un passo di troppo.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
	{ "Sala delle Tentazioni",
	  "Visioni dei desideri piu' profondi.\n"
	  "Cedere significa restare per sempre.\n"
	  "Una corrente d'aria fredda tradisce un varco nascosto.\n"
	  "Ombre lunghe si allungano dove la luce finisce.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
};

static const ProcRoomTemplate kTheme49Boss = {
	"Trono dei Nove Inferi",
	  "Una sala di fuoco e ossidiana culmina in un trono.\n"
	  "Diavoli minori servono un principe infernale.\n"
	  "Il signore del fosso pronuncia la tua condanna.\n"
	  "L'eco dei passi rimbalza su pareti vicine.\n",
	SECT_INSIDE, static_cast<long>(INDOORS | DARK)
};

#endif /* __PROCAREA_THEMES_EXT_INC */
