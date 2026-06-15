/* procarea_themes_ext.inc — temi 25-49 (generato, non editare a mano) */
#ifndef __PROCAREA_THEMES_EXT_INC
#define __PROCAREA_THEMES_EXT_INC

/* --- Tema 25: Moria delle Ombre --- */

static const ProcRoomTemplate kTheme25Entrances[] = {
	{ "Porta di Durin",
	  "Un arco di pietra nanica si apre su sale infinite.\n"
	  "Runa sbiadite pulsano debolmente nel buio profondo.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK | UNDERGROUND) },
	{ "Vestibolo dei Martelli",
	  "Statue di nani spezzate fiancheggiano un corridoio stretto.\n"
	  "L'eco dei passi sembra moltiplicarsi all'infinito.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK | UNDERGROUND) },
};

static const ProcRoomTemplate kTheme25Corridors[] = {
	{ "Galleria dei Pilastri",
	  "Colonne massicce sostengono una volta lontana.\n"
	  "Pozzi senza fondo si aprono ai lati del cammino.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK | UNDERGROUND) },
	{ "Passaggio del Fiume di Fuoco",
	  "Calore lontano e odore di zolfo aleggiano nell'aria.\n"
	  "Pietre nere brillano come brace spente.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK | UNDERGROUND) },
	{ "Sala delle Tombe Naniche",
	  "Sarcofagi di pietra sono stati forzati e saccheggiati.\n"
	  "Ossa naniche giacciono sparse sul pavimento.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK | UNDERGROUND) },
	{ "Atrio del Pozzo Senza Fine",
	  "Un baratro circolare taglia la sala in due.\n"
	  "Corde consumate pendono nel vuoto.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK | UNDERGROUND) },
};

static const ProcRoomTemplate kTheme25Treasures[] = {
	{ "Camera del Mithril",
	  "Filamenti d'argento raro ancora luccicano tra le rocce.\n"
	  "Picconi abbandonati raccontano una febbre dimenticata.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK | UNDERGROUND) },
	{ "Forzieri dei Re Nani",
	  "Casse sigillate con rune naniche attendono ancora.\n"
	  "Qualcuno ha tentato di forzarle di recente.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK | UNDERGROUND) },
};

static const ProcRoomTemplate kTheme25Traps[] = {
	{ "Ponte Spezzato",
	  "Lastre di pietra cedono sotto il peso al minimo passo falso.\n"
	  "Il baratro sotto richiama con vento gelido.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK | UNDERGROUND) },
	{ "Sala delle Trappole Naniche",
	  "Dardi di pietra sporgono dalle pareti a intervalli regolari.\n"
	  "Piastrelde del pavimento sono leggermente inclinate.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK | UNDERGROUND) },
};

static const ProcRoomTemplate kTheme25Boss = {
	"Sala del Balrog Dimenticato",
	  "Una caverna immensa si apre su un abisso fumante.\n"
	  "Catene spezzate pendono da un ponte di pietra crollato.\n"
	  "Qualcosa di antico e ardente attende nel cuore delle fiamme.\n",
	SECT_INSIDE, static_cast<long>(INDOORS | DARK | UNDERGROUND)
};

/* --- Tema 26: Fangorn Profondo --- */

static const ProcRoomTemplate kTheme26Entrances[] = {
	{ "Varco tra gli Ent",
	  "Due alberi colossali formano un arco naturale.\n"
	  "La corteccia sembra osservarti con pazienza antica.\n",
	  SECT_FOREST, static_cast<long>(INDOORS | DARK) },
	{ "Sentiero della Voce Verde",
	  "Muschio spesso ammortizza ogni rumore.\n"
	  "Sussurri di foglie raccontano storie millenarie.\n",
	  SECT_FOREST, static_cast<long>(INDOORS | DARK) },
};

static const ProcRoomTemplate kTheme26Corridors[] = {
	{ "Corridoio di Radici",
	  "Radici spesse come serpenti bloccano passaggi laterali.\n"
	  "Aria umida odora di terra e linfa.\n",
	  SECT_FOREST, static_cast<long>(INDOORS | DARK) },
	{ "Galleria dei Muschi",
	  "Muschio luminoso traccia sentieri nel buio.\n"
	  "Funghi alti come uomini crescono lungo le pareti.\n",
	  SECT_FOREST, static_cast<long>(INDOORS | DARK) },
	{ "Passaggio dei Sussurri",
	  "Il vento tra i rami imita voci quasi comprensibili.\n"
	  "Ogni passo fa scricchiolare legno vivo.\n",
	  SECT_FOREST, static_cast<long>(INDOORS | DARK) },
	{ "Radura degli Ent Sonnecchianti",
	  "Tronchi immobili sembrano dormire in cerchio.\n"
	  "Solo gli occhi di resina tradiscono vigilanza.\n",
	  SECT_FOREST, static_cast<long>(INDOORS | DARK) },
};

static const ProcRoomTemplate kTheme26Treasures[] = {
	{ "Cavità del Corteccia d'Oro",
	  "Schegge preziose si staccano da un tronco antico.\n"
	  "Resina ambrata racchiude insetti dimenticati.\n",
	  SECT_FOREST, static_cast<long>(INDOORS | DARK) },
	{ "Nicchia dei Semi Antichi",
	  "Semi grandi come pugni pulsano di vita sopita.\n"
	  "Qualcuno li ha raccolti con reverenza.\n",
	  SECT_FOREST, static_cast<long>(INDOORS | DARK) },
};

static const ProcRoomTemplate kTheme26Traps[] = {
	{ "Trappola di Liane",
	  "Liane si avvolgono al minimo contatto.\n"
	  "La foresta reagisce alla tua presenza.\n",
	  SECT_FOREST, static_cast<long>(INDOORS | DARK) },
	{ "Pozza di Linfa Acida",
	  "Linfa corrosiva gocciola dal soffitto di foglie.\n"
	  "Il terreno si ammorbidisce minacciosamente.\n",
	  SECT_FOREST, static_cast<long>(INDOORS | DARK) },
};

static const ProcRoomTemplate kTheme26Boss = {
	"Cuore di Fangorn",
	  "Una radura circolare accoglie un trono di rami intrecciati.\n"
	  "Ent antichi emergono dalla penombra con passi lenti.\n"
	  "La foresta stessa sembra respirare attorno a te.\n",
	SECT_FOREST, static_cast<long>(INDOORS | DARK)
};

/* --- Tema 27: Tomba dei Re Caduti --- */

static const ProcRoomTemplate kTheme27Entrances[] = {
	{ "Tumulo dei Re",
	  "Un tumulo erboso nasconde una discesa verso l'oscurita'.\n"
	  "Pietre runiche segnano confini dimenticati.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
	{ "Ingresso del Barrow",
	  "Una porta di pietra socchiusa libera aria gelida.\n"
	  "Gioielli rubati giacciono abbandonati sulla soglia.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
};

static const ProcRoomTemplate kTheme27Corridors[] = {
	{ "Corridoio delle Corone Spezzate",
	  "Corone arrugginite pendono da chiodi nel muro.\n"
	  "Telai di ragnatela coprono stemmi sbiaditi.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
	{ "Galleria dei Cavalieri",
	  "Armature vuote sorvegliano il passaggio.\n"
	  "Spade incastrate nella pietra formano una ringhiera.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
	{ "Sala delle Candele Spente",
	  "Candelabri antichi attendono un fuoco che non tornera'.\n"
	  "Ceneri di ceri secolari coprono il pavimento.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
	{ "Atrio del Re Dormiente",
	  "Un sarcofago di pietra domina la stanza.\n"
	  "Runa di protezione sono state violate.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
};

static const ProcRoomTemplate kTheme27Treasures[] = {
	{ "Camera del Tesoro Reale",
	  "Oro e argento giacciono accanto a ossa reali.\n"
	  "Una corona dimenticata brilla ancora debolmente.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
	{ "Reliquiario dei Caduti",
	  "Reliquie avvolte in seta marcia attendono un pellegrino.\n"
	  "Sigilli di cera sono stati rotti.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
};

static const ProcRoomTemplate kTheme27Traps[] = {
	{ "Passaggio Maledetto",
	  "Simboli arsi a fuoco bruciano ancora sulla soglia.\n"
	  "Un freddo innaturale ti avvolge.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
	{ "Sala delle Lame Emergenti",
	  "Lame di pietra spuntano dal pavimento a intervalli.\n"
	  "Segni di fuga disperata segnano le pareti.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
};

static const ProcRoomTemplate kTheme27Boss = {
	"Trono del Barrow-Wight",
	  "Una sala circolare ospita un trono di ossa e corone.\n"
	  "Luci verdastre danzano senza fiamma.\n"
	  "Il re dei tumuli si rialza per difendere il suo regno.\n",
	SECT_INSIDE, static_cast<long>(INDOORS | DARK)
};

/* --- Tema 28: Ponte del Baratro --- */

static const ProcRoomTemplate kTheme28Entrances[] = {
	{ "Inizio del Ponte di Pietra",
	  "Un ponte stretto attraversa un abisso senza fondo.\n"
	  "Vento gelido sale dal baratro sottostante.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK | UNDERGROUND) },
	{ "Crepaccio Laterale",
	  "Una fessura nella roccia rivela un secondo varco.\n"
	  "Corde e pioli arrugginiti pendono nel vuoto.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK | UNDERGROUND) },
};

static const ProcRoomTemplate kTheme28Corridors[] = {
	{ "Galleria del Baratro",
	  "Il cammino costeggia un precipizio senza parapetto.\n"
	  "Pietre cadute giacciono nel vuoto sotto di te.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK | UNDERGROUND) },
	{ "Passaggio Sospeso",
	  "Travi di legno marcio collegano piattaforme rocciose.\n"
	  "Ogni passo fa gemere la struttura.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK | UNDERGROUND) },
	{ "Corridoio dell'Eco Profonda",
	  "Il tuo respiro rimbomba per secondi interminabili.\n"
	  "Non vedi il fondo neppure con la torcia.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK | UNDERGROUND) },
	{ "Atrio del Vuoto",
	  "Una piattaforma circolare domina il baratro.\n"
	  "Catene spezzate oscillano nel vento.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK | UNDERGROUND) },
};

static const ProcRoomTemplate kTheme28Treasures[] = {
	{ "Nicchia del Viandante Caduto",
	  "Uno zaino logoro nasconde monete e una mappa.\n"
	  "Ossa giacciono vicino al bordo del precipizio.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK | UNDERGROUND) },
	{ "Forziere del Guardiano",
	  "Un forziere e' incatenato alla roccia.\n"
	  "La chiave giace in un cranio vicino.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK | UNDERGROUND) },
};

static const ProcRoomTemplate kTheme28Traps[] = {
	{ "Ponte Instabile",
	  "Lastre cedono una ad una al tuo passaggio.\n"
	  "Non c'e' tempo per esitare.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK | UNDERGROUND) },
	{ "Sala del Soffio Gelido",
	  "Raffiche dal baratro spingono verso il vuoto.\n"
	  "Il parapetto manca da tempo.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK | UNDERGROUND) },
};

static const ProcRoomTemplate kTheme28Boss = {
	"Cuore dell'Abisso",
	  "Una piattaforma sospesa culmina sopra il baratro.\n"
	  "Fulmini lontani illuminano una figura al centro.\n"
	  "Il ponte dietro di te trema come se volesse crollare.\n",
	SECT_INSIDE, static_cast<long>(INDOORS | DARK | UNDERGROUND)
};

/* --- Tema 29: Rovine di Osgiliath --- */

static const ProcRoomTemplate kTheme29Entrances[] = {
	{ "Arco Spezzato",
	  "Un arco monumentale e' crollato a meta'.\n"
	  "Iscrizioni in lingua antica si leggono a fatica.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
	{ "Ponte sul Anduin",
	  "Piloni di pietra emergono dall'acqua sotterranea.\n"
	  "Correnti lente portano detriti di una citta' morta.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
};

static const ProcRoomTemplate kTheme29Corridors[] = {
	{ "Via dei Re",
	  "Colonne spezzate fiancheggiano una strada deserta.\n"
	  "Stendardi a brandelli oscillano senza vento.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
	{ "Piazza del Palantir",
	  "Una sfera di cristallo frantumata giace al centro.\n"
	  "Vetri taglienti coprono il pavimento.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
	{ "Corridoio del Senato",
	  "Sedili di marmo sono stati rivoltati e bruciati.\n"
	  "Cenere antica copre mosaici preziosi.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
	{ "Atrio della Torre Bianca",
	  "Scale spezzate salgono verso il cielo aperto.\n"
	  "Corvi nidificano tra le pietre cadute.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
};

static const ProcRoomTemplate kTheme29Treasures[] = {
	{ "Tesoro del Palazzo",
	  "Monete d'argento e oro giacciono tra le macerie.\n"
	  "Qualcuno ha scavato di recente.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
	{ "Archivio delle Carte Reali",
	  "Pergamene sigillate resistono ancora all'umidita'.\n"
	  "Mappe di regni perduti attendono.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
};

static const ProcRoomTemplate kTheme29Traps[] = {
	{ "Sala del Crollo",
	  "Il soffitto minaccia di cedere ad ogni rumore.\n"
	  "Travi spezzate bloccano uscite laterali.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
	{ "Passaggio degli Orchetti",
	  "Trappole rudimentali segnano un accampamento recente.\n"
	  "Ossa e rifiuti coprono il pavimento.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
};

static const ProcRoomTemplate kTheme29Boss = {
	"Santuario del Re Pellegrino",
	  "Una cappella mezza distrutta ospita un altare ancora intatto.\n"
	  "Luce grigia filtra tra le rovine.\n"
	  "Un custode spettrale veglia su cio' che resta della citta'.\n",
	SECT_INSIDE, static_cast<long>(INDOORS | DARK)
};

/* --- Tema 30: Covo degli Uruk --- */

static const ProcRoomTemplate kTheme30Entrances[] = {
	{ "Fossa di Warcraft",
	  "Scale di legno scendono in un cantiere orco.\n"
	  "Odore di ferro e carne putrefatta riempie l'aria.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK | UNDERGROUND) },
	{ "Varco delle Gole",
	  "Pareti strette sono segnate da artigli e runa di guerra.\n"
	  "Grugniti e martelli riecheggiano da sotto.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK | UNDERGROUND) },
};

static const ProcRoomTemplate kTheme30Corridors[] = {
	{ "Galleria degli Scudi",
	  "Scudi orchi impilati formano pareti improvvisate.\n"
	  "Torce di pece fumano basso.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK | UNDERGROUND) },
	{ "Corridoio delle Lance",
	  "Lance arrugginite punteggiano il soffitto come stalattiti.\n"
	  "Ossa di prigionieri giacciono ammucchiate.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK | UNDERGROUND) },
	{ "Fucina di Guerra",
	  "Incudini ancora calde e martelli abbandonati.\n"
	  "Lame non finite pendono dai ganci.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK | UNDERGROUND) },
	{ "Atrio del Capitano",
	  "Un trono di ossa e armi domina la sala.\n"
	  "Stendardi neri segnano territorio conquistato.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK | UNDERGROUND) },
};

static const ProcRoomTemplate kTheme30Treasures[] = {
	{ "Bottino di Guerra",
	  "Armi e armature rubate sono ammucchiate senza ordine.\n"
	  "Monete sparse brillano tra il ferro.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK | UNDERGROUND) },
	{ "Camera del Signore della Guerra",
	  "Mappe graffiate sulle pareti mostrano invasioni pianificate.\n"
	  "Un forziere pesante attende.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK | UNDERGROUND) },
};

static const ProcRoomTemplate kTheme30Traps[] = {
	{ "Fossa con Pale",
	  "Pale coprono trappole nel pavimento di fango.\n"
	  "Cadere significa impalarsi.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK | UNDERGROUND) },
	{ "Sala delle Frecce",
	  "Dardi scattano dalle fessure al passaggio.\n"
	  "Segni di sangue segnano il percorso.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK | UNDERGROUND) },
};

static const ProcRoomTemplate kTheme30Boss = {
	"Trono dell'Uruk-Hai",
	  "Una sala vasta ospita un trono di ferro e ossa.\n"
	  "Cori di guerra echeggiano da tunnel laterali.\n"
	  "Il signore degli uruk si alza per schiacciare gli intrusi.\n",
	SECT_INSIDE, static_cast<long>(INDOORS | DARK | UNDERGROUND)
};

/* --- Tema 31: Galleria Elfica --- */

static const ProcRoomTemplate kTheme31Entrances[] = {
	{ "Varco di Lothlorien",
	  "Alberi argentei incorniciano un sentiero luminoso.\n"
	  "L'aria profuma di mallorn e luce antica.\n",
	  SECT_FOREST, static_cast<long>(INDOORS | BRIGHT) },
	{ "Ponte di Vine",
	  "Viti intrecciate formano un ponte sospeso.\n"
	  "Luce filtra come acqua tra le foglie.\n",
	  SECT_FOREST, static_cast<long>(INDOORS | BRIGHT) },
};

static const ProcRoomTemplate kTheme31Corridors[] = {
	{ "Corridoio dei Cantori",
	  "Melodie elfiche sembrano provenire dalle pareti stesse.\n"
	  "Intagli dorati raccontano ere dimenticate.\n",
	  SECT_FOREST, static_cast<long>(INDOORS | BRIGHT) },
	{ "Galleria delle Stelle",
	  "Cristalli incastonati imitano un cielo notturno.\n"
	  "Ogni passo risuona come una nota.\n",
	  SECT_FOREST, static_cast<long>(INDOORS | BRIGHT) },
	{ "Passaggio dei Guardiani",
	  "Statue elfiche seguono i tuoi movimenti con gli occhi.\n"
	  "Frecce incastonate decorano gli archi.\n",
	  SECT_FOREST, static_cast<long>(INDOORS | BRIGHT) },
	{ "Atrio della Regina",
	  "Colonne sottili sostengono una volta di luce.\n"
	  "Fiori eterni sbocciano senza stagione.\n",
	  SECT_FOREST, static_cast<long>(INDOORS | BRIGHT) },
};

static const ProcRoomTemplate kTheme31Treasures[] = {
	{ "Camera dei Doni",
	  "Vestiti elfici e gioielli attendono ospiti degni.\n"
	  "Pietre preziose pulsano di luce interna.\n",
	  SECT_FOREST, static_cast<long>(INDOORS | BRIGHT) },
	{ "Biblioteca dei Canti",
	  "Pergamene di canzoni antiche sono conservate in seta.\n"
	  "Una arpa d'argento giace su un piedistallo.\n",
	  SECT_FOREST, static_cast<long>(INDOORS | BRIGHT) },
};

static const ProcRoomTemplate kTheme31Traps[] = {
	{ "Sala delle Illusioni",
	  "Miraggi elfici nascondono buchi nel pavimento.\n"
	  "La bellezza distrae dal pericolo.\n",
	  SECT_FOREST, static_cast<long>(INDOORS | BRIGHT) },
	{ "Passaggio delle Frecce Silenti",
	  "Balestre nascoste puntano verso il centro.\n"
	  "Solo gli elfi conoscono il percorso sicuro.\n",
	  SECT_FOREST, static_cast<long>(INDOORS | BRIGHT) },
};

static const ProcRoomTemplate kTheme31Boss = {
	"Santuario della Luce Argentea",
	  "Una radura di luce pura accoglie un altare di cristallo.\n"
	  "Alberi mallorn formano una cupola naturale.\n"
	  "Un guardiano elfico emerge dalla luce stessa.\n",
	SECT_FOREST, static_cast<long>(INDOORS | BRIGHT)
};

/* --- Tema 32: Ceneri di Gorgoroth --- */

static const ProcRoomTemplate kTheme32Entrances[] = {
	{ "Sentiero delle Ceneri",
	  "Cenere vulcanica copre ogni superficie.\n"
	  "Il cielo e' una coltre rossastra di fumo.\n",
	  SECT_MOUNTAIN, static_cast<long>(INDOORS | DARK) },
	{ "Varco di Mordor",
	  "Rocce nere formano un arco minaccioso.\n"
	  "L'aria brucia in gola ad ogni respiro.\n",
	  SECT_MOUNTAIN, static_cast<long>(INDOORS | DARK) },
};

static const ProcRoomTemplate kTheme32Corridors[] = {
	{ "Galleria del Fuoco Lontano",
	  "Rive luminose pulsano oltre le pareti di basalto.\n"
	  "Zolfo cristallizzato ricopre il pavimento.\n",
	  SECT_MOUNTAIN, static_cast<long>(INDOORS | DARK) },
	{ "Corridoio degli Orchetti",
	  "Impronte di orchi e ferri arrugginiti segnano il cammino.\n"
	  "Urla lontane portano ordini incomprensibili.\n",
	  SECT_MOUNTAIN, static_cast<long>(INDOORS | DARK) },
	{ "Passaggio del Fumo Denso",
	  "Fumo acido riduce la visibilita' a pochi passi.\n"
	  "Occhi rossi brillano nella penombra.\n",
	  SECT_MOUNTAIN, static_cast<long>(INDOORS | DARK) },
	{ "Atrio delle Fornaci",
	  "Fornaci rudimentali fumano ancora.\n"
	  "Armi mal forgiate giacciono a raffreddare.\n",
	  SECT_MOUNTAIN, static_cast<long>(INDOORS | DARK) },
};

static const ProcRoomTemplate kTheme32Treasures[] = {
	{ "Tesoro del Signore Oscuro",
	  "Armi e trofei sono ammucchiati con brutalita'.\n"
	  "Anelli spezzati brillano tra le ceneri.\n",
	  SECT_MOUNTAIN, static_cast<long>(INDOORS | DARK) },
	{ "Camera dei Sigilli",
	  "Sigilli di ferro incisi con runa nere.\n"
	  "Un forziere e' incatenato alla roccia.\n",
	  SECT_MOUNTAIN, static_cast<long>(INDOORS | DARK) },
};

static const ProcRoomTemplate kTheme32Traps[] = {
	{ "Sala della Lava",
	  "Crepe nel pavimento mostrano lava sotto la crosta.\n"
	  "Calore insopportabile aumenta verso il centro.\n",
	  SECT_MOUNTAIN, static_cast<long>(INDOORS | DARK) },
	{ "Passaggio dei Flagelli",
	  "Fruste di ferro pendono dal soffitto.\n"
	  "Cicatrici sulle pareti raccontano torture.\n",
	  SECT_MOUNTAIN, static_cast<long>(INDOORS | DARK) },
};

static const ProcRoomTemplate kTheme32Boss = {
	"Trono delle Ceneri",
	  "Un trono di roccia vulcanica domina una pianura di cenere.\n"
	  "Il ruggito di un vulcano lontano scuote la terra.\n"
	  "Un signore delle ombre attende coronato di fumo.\n",
	SECT_MOUNTAIN, static_cast<long>(INDOORS | DARK)
};

/* --- Tema 33: Fortezza di Helm --- */

static const ProcRoomTemplate kTheme33Entrances[] = {
	{ "Porta del Fosso",
	  "Un ponte levatoio spezzato oscilla sopra un fosso profondo.\n"
	  "Mura massicce si ergono fino a perdersi nel buio.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
	{ "Ingresso della Grotta",
	  "Una grotta naturale si apre sotto le mura.\n"
	  "Echi di battaglie antiche riempiono l'aria.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
};

static const ProcRoomTemplate kTheme33Corridors[] = {
	{ "Corridoio dei Difensori",
	  "Archi per corniere si aprono sulle pareti.\n"
	  "Frecce rotte giacciono in mucchi.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
	{ "Galleria del Fosso Profondo",
	  "Acqua stagnante copre il pavimento.\n"
	  "Scheletri in armatura emergono dal fango.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
	{ "Passaggio della Sortita",
	  "Una porta segreta conduce verso l'esterno.\n"
	  "Segni di una fuga disperata segnano le pareti.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
	{ "Atrio del Re di Rohan",
	  "Stendardi sbiaditi pendono da lance spezzate.\n"
	  "Un trono di pietra e' vuoto ma imponente.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
};

static const ProcRoomTemplate kTheme33Treasures[] = {
	{ "Armeria Segreta",
	  "Armi di qualita' sono ancora impilate ordinatamente.\n"
	  "Scudi con il simbolo del cavallo bianco.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
	{ "Camera del Corno di Helm",
	  "Un corno antico giace su un piedistallo.\n"
	  "La sua eco ancora terrorizza chi la sente.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
};

static const ProcRoomTemplate kTheme33Traps[] = {
	{ "Sala del Crollo",
	  "Macigni bloccano passaggi; solo sentieri stretti restano.\n"
	  "Polvere si solleva ad ogni passo.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
	{ "Passaggio del Assedio",
	  "Dardi incastrati nella pietra segnano un assalto.\n"
	  "Olio bollente ha bruciato il pavimento.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
};

static const ProcRoomTemplate kTheme33Boss = {
	"Sala dell'Ultima Resistenza",
	  "Una grande sala ospita resti di un'ultima difesa eroica.\n"
	  "Corpi fossilizzati tengono ancora le armi.\n"
	  "Un campione spettrale sorveglia il varco finale.\n",
	SECT_INSIDE, static_cast<long>(INDOORS | DARK)
};

/* --- Tema 34: Sentieri dei Morti --- */

static const ProcRoomTemplate kTheme34Entrances[] = {
	{ "Porta dei Giuramenti",
	  "Un arco di pietra reca iscrizioni di giuramenti infranti.\n"
	  "Aria gelida esce dal tunnel sotterraneo.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
	{ "Ingresso della Montagna",
	  "Pilastri scolpiti raffigurano re spettrali.\n"
	  "Nessun suono vivo proviene dall'interno.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
};

static const ProcRoomTemplate kTheme34Corridors[] = {
	{ "Galleria dei Re Spettrali",
	  "Ombre di corone passano attraverso le pareti.\n"
	  "Passi senza corpo echeggiano accanto ai tuoi.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
	{ "Corridoio dei Giuramenti",
	  "Voci sussurrano promesse mai mantenute.\n"
	  "Il freddo aumenta ad ogni passo.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
	{ "Passaggio delle Lame Spettrali",
	  "Spade fantasma sfiorano la tua carne senza tagliare.\n"
	  "Terrore antico ti paralizza un istante.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
	{ "Atrio del Re Oathbreaker",
	  "Un trono vuoto domina una sala di ombre.\n"
	  "Stendardi polverosi cadono a pezzi.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
};

static const ProcRoomTemplate kTheme34Treasures[] = {
	{ "Reliquie dei Caduti",
	  "Armi spettrali giacciono incorrotte.\n"
	  "Medaglie di onore dimenticate brillano debolmente.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
	{ "Camera dei Sigilli Spezzati",
	  "Sigilli di cera nera sono stati infranti.\n"
	  "Pergamene di giuramento giacciono sparse.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
};

static const ProcRoomTemplate kTheme34Traps[] = {
	{ "Sala del Terrore",
	  "Visioni dei tuoi peccati appaiono sulle pareti.\n"
	  "La mente vacilla per un istante.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
	{ "Passaggio del Vuoto",
	  "Il pavimento sembra non esistere sotto i piedi.\n"
	  "Solo la volonta' ti tiene in piedi.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
};

static const ProcRoomTemplate kTheme34Boss = {
	"Trono dei Re Spettrali",
	  "Una sala infinita ospita ombre di re millenari.\n"
	  "Corone spettrali brillano nella penombra.\n"
	  "Il sovrano dei morti pronuncia il tuo destino.\n",
	SECT_INSIDE, static_cast<long>(INDOORS | DARK)
};

/* --- Tema 35: Fosse di Isengard --- */

static const ProcRoomTemplate kTheme35Entrances[] = {
	{ "Pozzo di Orthanc",
	  "Un pozzo verticale scende verso officine sotterranee.\n"
	  "Odore di carbone e metallo riempie l'aria.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK | UNDERGROUND) },
	{ "Tunnel degli Orchi",
	  "Pareti squadrate con precisione innaturale.\n"
	  "Macchine di legno e ferro bloccano passaggi.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK | UNDERGROUND) },
};

static const ProcRoomTemplate kTheme35Corridors[] = {
	{ "Galleria delle Macchine",
	  "Ingranaggi immensi sono fermi ma minacciosi.\n"
	  "Catene spesse pendono dal soffitto.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK | UNDERGROUND) },
	{ "Corridoio del Carbone",
	  "Carbone nero copre ogni superficie.\n"
	  "Polvere nera irrita occhi e gola.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK | UNDERGROUND) },
	{ "Passaggio dei Fabbri",
	  "Incudini e martelli giacciono abbandonati.\n"
	  "Lame di ferro sporgono dal pavimento.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK | UNDERGROUND) },
	{ "Atrio del Mago Traditore",
	  "Libri bruciati e strumenti spezzati coprono il pavimento.\n"
	  "Una sfera di cristallo e' incrinata.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK | UNDERGROUND) },
};

static const ProcRoomTemplate kTheme35Treasures[] = {
	{ "Armeria di Isengard",
	  "Armature uniformi sono impilate per migliaia.\n"
	  "Elmi con la S bianca dominano la sala.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK | UNDERGROUND) },
	{ "Camera dei Piani",
	  "Mappe di invasioni coprono ogni parete.\n"
	  "Un forziere contiene ordini sigillati.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK | UNDERGROUND) },
};

static const ProcRoomTemplate kTheme35Traps[] = {
	{ "Sala delle Ruote",
	  "Ruote dentate si muovono al minimo tocco.\n"
	  "Evitare i meccanismi e' questione di vita.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK | UNDERGROUND) },
	{ "Passaggio del Fuoco di Forgia",
	  "Getti di vapore bollente fuoriescono dalle pareti.\n"
	  "Il metallo rovente brucia ancora.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK | UNDERGROUND) },
};

static const ProcRoomTemplate kTheme35Boss = {
	"Cuore della Macchina",
	  "Una sala meccanica culmina in un nucleo di ingranaggi.\n"
	  "Vapore e fumo oscurano la vista.\n"
	  "Il signore delle fosse emerge tra le macchine.\n",
	SECT_INSIDE, static_cast<long>(INDOORS | DARK | UNDERGROUND)
};

/* --- Tema 36: Tunnels di Ungol --- */

static const ProcRoomTemplate kTheme36Entrances[] = {
	{ "Fessura della Ragnatela",
	  "Seta densa blocca il passaggio come nebbia solida.\n"
	  "Insetti impalati pendono ovunque.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
	{ "Bocca del Labirinto",
	  "Tunnel intrecciati perdono ogni orientamento.\n"
	  "Ossa avvolte in seta ricoprono il pavimento.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
};

static const ProcRoomTemplate kTheme36Corridors[] = {
	{ "Galleria delle Paralisi",
	  "Ragnatele appiccicose bloccano braccia e gambe.\n"
	  "Qualcosa striscia sopra la tua testa.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
	{ "Corridoio dei Sacchi",
	  "Sacchi di seta contengono prede ancora vive.\n"
	  "Movimenti deboli tradiscono vittime recenti.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
	{ "Passaggio delle Zanne",
	  "Zanne di ragni giganti decorano le pareti.\n"
	  "Liquido digestivo gocciola dal soffitto.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
	{ "Atrio della Regina",
	  "Un trono di ossa e seta domina la sala.\n"
	  "Ragni di ogni dimensione attendono ordini.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
};

static const ProcRoomTemplate kTheme36Treasures[] = {
	{ "Tesoro delle Vittime",
	  "Equipaggiamento di avventurieri e' ammucchiato con cura.\n"
	  "Anelli e monete brillano tra la seta.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
	{ "Camera delle Uova",
	  "Uova di ragno pulsano di vita imminente.\n"
	  "Larvae strisciano in cumuli viscidi.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
};

static const ProcRoomTemplate kTheme36Traps[] = {
	{ "Trappola della Ragnatela",
	  "Filamenti si tendono al passaggio tagliando la fuga.\n"
	  "Veleno paralizzante impregna ogni filo.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
	{ "Pozza di Veleno",
	  "Liquido verde corrode armatura e carne.\n"
	  "Vapori acidi rendono difficile respirare.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
};

static const ProcRoomTemplate kTheme36Boss = {
	"Nido della Regina Ragno",
	  "Una caverna immensa e' tappezzata di seta nera.\n"
	  "Occhi multipli brillano nel buio sopra di te.\n"
	  "La regina dei ragni scende dal soffitto.\n",
	SECT_INSIDE, static_cast<long>(INDOORS | DARK)
};

/* --- Tema 37: Bosco di Mirkwood --- */

static const ProcRoomTemplate kTheme37Entrances[] = {
	{ "Sentiero Oscuro",
	  "Alberi contorti chiudono il cielo a una fessura.\n"
	  "Nessun raggio di sole raggiunge il suolo.\n",
	  SECT_FOREST, static_cast<long>(INDOORS | DARK) },
	{ "Varco dei Funghi",
	  "Funghi luminosi tingono il bosco di verde pallido.\n"
	  "Spore dense rendono l'aria pesante.\n",
	  SECT_FOREST, static_cast<long>(INDOORS | DARK) },
};

static const ProcRoomTemplate kTheme37Corridors[] = {
	{ "Corridoio degli Alberi",
	  "Rami bassi costringono a chinare la testa.\n"
	  "Ragni e insetti osservano dal fogliame.\n",
	  SECT_FOREST, static_cast<long>(INDOORS | DARK) },
	{ "Galleria dei Sussurri",
	  "Voci sembrano provenire da tronchi cavi.\n"
	  "Il bosco parla in lingue dimenticate.\n",
	  SECT_FOREST, static_cast<long>(INDOORS | DARK) },
	{ "Passaggio del Fiume di Resina",
	  "Resina appiccicosa trattiene ogni passo.\n"
	  "Insetti giganti nuotano nella linfa.\n",
	  SECT_FOREST, static_cast<long>(INDOORS | DARK) },
	{ "Radura degli Elfi Prigionieri",
	  "Gabbie di legno sono vuote ma recenti.\n"
	  "Segni di lotta segnano il terreno.\n",
	  SECT_FOREST, static_cast<long>(INDOORS | DARK) },
};

static const ProcRoomTemplate kTheme37Treasures[] = {
	{ "Tesoro degli Elfi Catturati",
	  "Vesti e armi elfiche giacciono abbandonate.\n"
	  "Fiale di luce ancora brillano debolmente.\n",
	  SECT_FOREST, static_cast<long>(INDOORS | DARK) },
	{ "Camera del Re dei Funghi",
	  "Funghi giganti formano un trono naturale.\n"
	  "Spore dorate cadono come neve.\n",
	  SECT_FOREST, static_cast<long>(INDOORS | DARK) },
};

static const ProcRoomTemplate kTheme37Traps[] = {
	{ "Trappola di Resina",
	  "Resina si indurisce rapidamente attorno alle gambe.\n"
	  "Fuggire diventa quasi impossibile.\n",
	  SECT_FOREST, static_cast<long>(INDOORS | DARK) },
	{ "Sala delle Spine",
	  "Spine di legno spuntano dal suolo al passaggio.\n"
	  "Sangue secco segna il percorso.\n",
	  SECT_FOREST, static_cast<long>(INDOORS | DARK) },
};

static const ProcRoomTemplate kTheme37Boss = {
	"Trono del Bosco Nero",
	  "Un albero colossale ospita un trono di rami neri.\n"
	  "Corvi e pipistrelli volteggiano senza riposo.\n"
	  "Il signore del bosco emerge dall'ombra.\n",
	SECT_FOREST, static_cast<long>(INDOORS | DARK)
};

/* --- Tema 38: Cantine di Bree --- */

static const ProcRoomTemplate kTheme38Entrances[] = {
	{ "Botola della Locanda",
	  "Una botola nel pavimento rivela cantine umide.\n"
	  "Odore di birra e muffa si mescolano.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
	{ "Passaggio Segreto",
	  "Un corridoio nascosto dietro un barile vuoto.\n"
	  "Segni di passaggio recente segnano il fango.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
};

static const ProcRoomTemplate kTheme38Corridors[] = {
	{ "Corridoio dei Barili",
	  "Barili impilati formano pareti instabili.\n"
	  "Liquido scuro gocciola tra le assi.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
	{ "Galleria dei Contrabbandieri",
	  "Casse sigillate portano marchi di citta' lontane.\n"
	  "Mappe di rotte segrete coprono un tavolo.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
	{ "Passaggio dei Ratti",
	  "Ratti grossi come gatti osservano dall'ombra.\n"
	  "Ossa rosicchiate coprono il pavimento.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
	{ "Atrio del Capitano della Guardia",
	  "Armi confiscate sono ammucchiate in un angolo.\n"
	  "Una scrivania coperta di rapporti attende.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
};

static const ProcRoomTemplate kTheme38Treasures[] = {
	{ "Cassa del Contrabbando",
	  "Spezie rare e gioielli sono nascosti sotto fieno.\n"
	  "Monete di ogni regno riempiono sacchi.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
	{ "Forziere della Locanda",
	  "Oro della locanda e' custodito dietro serrature multiple.\n"
	  "Chiavi pendono da un chiodo vicino.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
};

static const ProcRoomTemplate kTheme38Traps[] = {
	{ "Trappola del Barile",
	  "Un barile cede rivelando punta di ferro sotto.\n"
	  "Il contrabbandiere conosceva questo trucco.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
	{ "Sala della Birra Avvelenata",
	  "Vapori alcolici nascondono gas soporiferi.\n"
	  "La testa gira dopo pochi respiri.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
};

static const ProcRoomTemplate kTheme38Boss = {
	"Cantina del Padrone Oscuro",
	  "La cantina piu' profonda ospita un tavolo rotondo.\n"
	  "Candele nere illuminano volti nascosti.\n"
	  "Il padrone dei bassifondi si alza per accoglierti.\n",
	SECT_INSIDE, static_cast<long>(INDOORS | DARK)
};

/* --- Tema 39: Tesoro di Smaug --- */

static const ProcRoomTemplate kTheme39Entrances[] = {
	{ "Fessura della Montagna",
	  "Una fessura nella roccia rivela luccichio interno.\n"
	  "Calore antico ancora aleggia nell'aria.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
	{ "Varco del Drago",
	  "Artigli profondi segnano le pareti all'ingresso.\n"
	  "Ossa calcinate giacciono ammucchiate.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
};

static const ProcRoomTemplate kTheme39Corridors[] = {
	{ "Galleria dell'Oro",
	  "Monete e gemme coprono il pavimento come ghiaia.\n"
	  "Ogni passo affonda nel tesoro.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
	{ "Corridoio delle Ossa",
	  "Cranii di nani e uomini formano cumuli macabri.\n"
	  "Armature fuse dal fuoco del drago.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
	{ "Passaggio delle Coppe",
	  "Coppe d'oro e argento sono ammucchiate a torri.\n"
	  "Pietre preziose cadono dai mucchi instabili.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
	{ "Atrio del Letto del Drago",
	  "Un'impronta immensa segna dove il drago dormiva.\n"
	  "Cenere e braci spente coprono tutto.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
};

static const ProcRoomTemplate kTheme39Treasures[] = {
	{ "Cuore del Tesoro",
	  "La pila piu' alta di oro e gemme domina la sala.\n"
	  "L'Arkenstone stessa sembra brillare da qualche parte.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
	{ "Camera delle Armi Fuse",
	  "Armi naniche fuse in lingotti d'oro impossibile.\n"
	  "Tributi di regni interi giacciono qui.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
};

static const ProcRoomTemplate kTheme39Traps[] = {
	{ "Trappola del Tesoro Instabile",
	  "Mucchi di monete cedono rivelando fosse sotto.\n"
	  "Cadere significa scomparire nell'oro.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
	{ "Sala del Fuoco Residuo",
	  "Braci ancora calde bruciano sotto il tesoro.\n"
	  "Il calore aumenta verso il centro.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
};

static const ProcRoomTemplate kTheme39Boss = {
	"Sala del Drago Caduto",
	  "Una caverna immensa ospita resti di un drago gigante.\n"
	  "Oro copre ogni centimetro come neve dorata.\n"
	  "Qualcosa ancora veglia sul tesoro con occhi avidi.\n",
	SECT_INSIDE, static_cast<long>(INDOORS | DARK)
};

/* --- Tema 40: Sale di Erebor --- */

static const ProcRoomTemplate kTheme40Entrances[] = {
	{ "Porta di Erebor",
	  "Una porta nanica massiccia reca runa di benvenuto.\n"
	  "Torce di cristallo illuminano il vestibolo.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK | UNDERGROUND) },
	{ "Scalinata dei Re",
	  "Gradini scolpiti nella roccia salgono maestosi.\n"
	  "Intagli narrano la storia dei nani.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK | UNDERGROUND) },
};

static const ProcRoomTemplate kTheme40Corridors[] = {
	{ "Galleria dei Sette Signori",
	  "Statue di sette re nanici fiancheggiano il cammino.\n"
	  "Mazze e asce sono incastonate nelle nicchie.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK | UNDERGROUND) },
	{ "Corridoio del Mithril",
	  "Filamenti d'argento corrono lungo le pareti.\n"
	  "Picconi abbandonati segnano un'era d'oro.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK | UNDERGROUND) },
	{ "Passaggio delle Fornaci",
	  "Fornaci naniche ancora tiepide.\n"
	  "Lingotti d'oro e argento attendono la forgia.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK | UNDERGROUND) },
	{ "Atrio del Trono di Pietra",
	  "Un trono scolpito nella roccia domina la sala.\n"
	  "Gemme incastonate brillano come stelle.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK | UNDERGROUND) },
};

static const ProcRoomTemplate kTheme40Treasures[] = {
	{ "Tesoro dei Re Nani",
	  "Forzieri nanici traboccano di gemme e metallo.\n"
	  "Corone dimenticate brillano sulla pietra.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK | UNDERGROUND) },
	{ "Armeria Reale",
	  "Armature naniche di qualita' leggendaria.\n"
	  "Asce e martelli runici attendono un guerriero.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK | UNDERGROUND) },
};

static const ProcRoomTemplate kTheme40Traps[] = {
	{ "Sala del Ponte Spezzato",
	  "Un ponte nanico e' crollato nel baratro.\n"
	  "Solo una trave instabile resta.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK | UNDERGROUND) },
	{ "Passaggio delle Lame Rotanti",
	  "Lame di pietra si muovono con meccanismi nanici.\n"
	  "Solo i nani conoscevano il ritmo.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK | UNDERGROUND) },
};

static const ProcRoomTemplate kTheme40Boss = {
	"Trono Sotto la Montagna",
	  "La sala del trono di Erebor risplende di gemme.\n"
	  "Corone naniche pendono dal soffitto scolpito.\n"
	  "L'ultimo guardiano delle sale si alza ad accoglierti.\n",
	SECT_INSIDE, static_cast<long>(INDOORS | DARK | UNDERGROUND)
};

/* --- Tema 41: Rova Nera Profonda --- */

static const ProcRoomTemplate kTheme41Entrances[] = {
	{ "Crepaccio della Rova",
	  "Un crepaccio nella roccia scende verso l'oscuro.\n"
	  "Funghi luminosi tingono tutto di viola.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
	{ "Ponte di Ossidiana",
	  "Un ponte sottile di ossidiana attraversa un abisso.\n"
	  "Stalattiti come lance puntano verso il basso.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
};

static const ProcRoomTemplate kTheme41Corridors[] = {
	{ "Galleria dei Drow",
	  "Intagli drow raffigurano divinita' aracnide.\n"
	  "Ragnatele decorate con fili d'argento.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
	{ "Corridoio dei Matroni",
	  "Troni vuoti di matroni drow segnano il cammino.\n"
	  "Veleno secco macchia le pareti.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
	{ "Passaggio delle Balestre",
	  "Balestre nascoste puntano verso il centro.\n"
	  "Dardi avvelenati giacciono in mucchi.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
	{ "Atrio del Tempio di Lolth",
	  "Un'ara di ossidiana domina la sala.\n"
	  "Ragni sacri tessono intorno all'altare.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
};

static const ProcRoomTemplate kTheme41Treasures[] = {
	{ "Tesoro della Matrona",
	  "Gioielli drow e armi di ossidiana brillano.\n"
	  "Schizzi di veleno proteggono i forzieri.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
	{ "Camera delle Incantesimi",
	  "Pergamene drow con incantesimi proibiti.\n"
	  "Fiale di oscurita' pulsano debolmente.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
};

static const ProcRoomTemplate kTheme41Traps[] = {
	{ "Trappola del Veleno",
	  "Gas viola fuoriesce da fessure nel pavimento.\n"
	  "La pelle brucia al contatto.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
	{ "Sala delle Ragnatele",
	  "Ragnatele bloccano ogni via di fuga.\n"
	  "Ragni avvelenati scendono dal soffitto.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
};

static const ProcRoomTemplate kTheme41Boss = {
	"Santuario della Regina Ragno",
	  "Un tempio sotterraneo ospita un trono di ragnatele.\n"
	  "Ragni giganti servono una matrona drow.\n"
	  "La regina della rova nera si alza dal trono.\n",
	SECT_INSIDE, static_cast<long>(INDOORS | DARK)
};

/* --- Tema 42: Fogne di Waterdeep --- */

static const ProcRoomTemplate kTheme42Entrances[] = {
	{ "Grata delle Fogne",
	  "Una grata arrugginita conduce a tunnel umidi.\n"
	  "Odore di rifiuti e muffa e' soffocante.\n",
	  SECT_WATER_NOSWIM, static_cast<long>(INDOORS | DARK) },
	{ "Pozzo di Scarico",
	  "Acqua nera scorre veloce sotto una passerella.\n"
	  "Ratti grossi come gatti osservano dall'ombra.\n",
	  SECT_WATER_NOSWIM, static_cast<long>(INDOORS | DARK) },
};

static const ProcRoomTemplate kTheme42Corridors[] = {
	{ "Galleria dei Rifiuti",
	  "Rifiuti di tutta la citta' si accumulano qui.\n"
	  "Gas infiammabili aleggiano bassi.\n",
	  SECT_WATER_NOSWIM, static_cast<long>(INDOORS | DARK) },
	{ "Corridoio delle Piastrelle",
	  "Piastrelle scivolose coprono il pavimento.\n"
	  "Acqua sporca arriva alle caviglie.\n",
	  SECT_WATER_NOSWIM, static_cast<long>(INDOORS | DARK) },
	{ "Passaggio dei Contrabbandieri",
	  "Casse di merci rubate bloccano il cammino.\n"
	  "Segni di Thieves' Guild sono ovunque.\n",
	  SECT_WATER_NOSWIM, static_cast<long>(INDOORS | DARK) },
	{ "Atrio del Capo Ladro",
	  "Un tavolo con mappe della citta' sopra.\n"
	  "Monete contate in mucchi ordinati.\n",
	  SECT_WATER_NOSWIM, static_cast<long>(INDOORS | DARK) },
};

static const ProcRoomTemplate kTheme42Treasures[] = {
	{ "Bottino della Gilda",
	  "Tesori rubati da tutta Waterdeep.\n"
	  "Gioielli e artefatti in casse sigillate.\n",
	  SECT_WATER_NOSWIM, static_cast<long>(INDOORS | DARK) },
	{ "Camera dei Contratti",
	  "Contratti su pergamena con sangue come inchiostro.\n"
	  "Sigilli della gilda su ogni documento.\n",
	  SECT_WATER_NOSWIM, static_cast<long>(INDOORS | DARK) },
};

static const ProcRoomTemplate kTheme42Traps[] = {
	{ "Trappola della Grata",
	  "Una grata cede rivelando acque profonde sotto.\n"
	  "Correnti trascinano verso l'ignoto.\n",
	  SECT_WATER_NOSWIM, static_cast<long>(INDOORS | DARK) },
	{ "Sala del Gas",
	  "Gas infiammabile si accumula in una nicchia.\n"
	  "Una scintilla basterebbe.\n",
	  SECT_WATER_NOSWIM, static_cast<long>(INDOORS | DARK) },
};

static const ProcRoomTemplate kTheme42Boss = {
	"Covo del Principe dei Ladri",
	  "Una sala asciutta sorprende in mezzo alle fogne.\n"
	  "Tappeti e candele contrastano con l'orrido esterno.\n"
	  "Il principe dei ladri attende con un sorriso.\n",
	SECT_WATER_NOSWIM, static_cast<long>(INDOORS | DARK)
};

/* --- Tema 43: Rovine di Neverwinter --- */

static const ProcRoomTemplate kTheme43Entrances[] = {
	{ "Porta Ghiacciata",
	  "Ghiaccio copre un arco di pietra crollato.\n"
	  "Aria gelida esce dalle rovine interne.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
	{ "Varco della Citta' Morta",
	  "Rovine di una grande citta' si estendono nel buio.\n"
	  "Neve eterna copre ogni superficie.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
};

static const ProcRoomTemplate kTheme43Corridors[] = {
	{ "Galleria del Gelo",
	  "Stalattiti di ghiaccio pendono minacciosi.\n"
	  "Il pavimento e' una lastra scivolosa.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
	{ "Corridoio dei Cavalieri di Ghiaccio",
	  "Statue di ghiaccio raffigurano cavalieri.\n"
	  "Alcune sembrano muoversi nel buio.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
	{ "Passaggio della Tempesta",
	  "Vento gelido entra da fessure nel muro.\n"
	  "Neve si accumula in cumuli alti.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
	{ "Atrio del Palazzo Congelato",
	  "Trono di ghiaccio domina una sala abbandonata.\n"
	  "Stendardi congelati cadono a pezzi.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
};

static const ProcRoomTemplate kTheme43Treasures[] = {
	{ "Tesoro del Re di Ghiaccio",
	  "Gemme congelate brillano come stelle.\n"
	  "Oro incastonato nel ghiaccio eterno.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
	{ "Camera delle Armi Gelide",
	  "Armi di ghiaccio magico ancora intatte.\n"
	  "Freddo che non scioglie mai.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
};

static const ProcRoomTemplate kTheme43Traps[] = {
	{ "Sala del Ghiaccio Sottile",
	  "Il pavimento di ghiaccio cede al peso.\n"
	  "Acqua gelida sotto minaccia di affogare.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
	{ "Passaggio degli Avalanche",
	  "Neve cade dal soffitto senza preavviso.\n"
	  "Seppellimento e' un rischio costante.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
};

static const ProcRoomTemplate kTheme43Boss = {
	"Trono del Re di Ghiaccio",
	  "Un palazzo congelato culmina in un trono di cristallo.\n"
	  "Tempesta eterna ulula fuori dalle mura.\n"
	  "Il re di ghiaccio si alza ad accoglierti.\n",
	SECT_INSIDE, static_cast<long>(INDOORS | DARK)
};

/* --- Tema 44: Cripta di Baldur --- */

static const ProcRoomTemplate kTheme44Entrances[] = {
	{ "Porta della Cripta",
	  "Un arco gotico conduce a tombe sotterranee.\n"
	  "Candele spente segnano un'abbandono recente.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
	{ "Scalinata dei Morti",
	  "Gradini consumati scendono tra lapidi inclinate.\n"
	  "Nomi cancelli dal tempo coprono le pietre.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
};

static const ProcRoomTemplate kTheme44Corridors[] = {
	{ "Galleria delle Famiglie",
	  "Tombe di famiglie nobili fiancheggiano il cammino.\n"
	  "Stemmi sbiaditi decorano ogni sarcofago.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
	{ "Corridoio dei Chierici",
	  "Simboli sacri sono stati profanati.\n"
	  "Libri di preghiera bruciati giacciono sparsi.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
	{ "Passaggio degli Scheletri",
	  "Scheletri si staccano dalle nicchie al tuo passaggio.\n"
	  "Ossa sbattono in un ritmo sinistro.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
	{ "Atrio del Vampiro",
	  "Un sarcofago di marmo nero domina la sala.\n"
	  "Rose secche e sangue secco decorano l'altare.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
};

static const ProcRoomTemplate kTheme44Treasures[] = {
	{ "Tesoro delle Casate",
	  "Gioielli funerari giacciono accanto alle tombe.\n"
	  "Oro e argento delle famiglie nobili.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
	{ "Reliquiario Profanato",
	  "Reliquie sacre rubate e contaminate.\n"
	  "Simboli oscuri sono stati aggiunti.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
};

static const ProcRoomTemplate kTheme44Traps[] = {
	{ "Sala delle Lame",
	  "Lame emergono dal pavimento a intervalli.\n"
	  "Meccanismo attivato dal peso.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
	{ "Passaggio del Gas Necrotico",
	  "Gas verdognolo fuoriesce dalle tombe aperte.\n"
	  "Carne marcia e' il prezzo di un respiro lungo.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
};

static const ProcRoomTemplate kTheme44Boss = {
	"Cripta del Signore Vampiro",
	  "La cripta piu' profonda ospita un trono di bare.\n"
	  "Pipistrelli volteggiano senza fine.\n"
	  "Il signore vampiro di Baldur si alza ad accoglierti.\n",
	SECT_INSIDE, static_cast<long>(INDOORS | DARK)
};

/* --- Tema 45: Fessura Planare --- */

static const ProcRoomTemplate kTheme45Entrances[] = {
	{ "Varco Instabile",
	  "La realta' sembra piegarsi attorno all'ingresso.\n"
	  "Colori impossibili danzano ai bordi della vista.\n",
	  SECT_AIR, static_cast<long>(INDOORS | DARK) },
	{ "Portale Fratturato",
	  "Un portale scintillante mostra mondi paralleli.\n"
	  "Vento da mille direzioni ti spinge.\n",
	  SECT_AIR, static_cast<long>(INDOORS | DARK) },
};

static const ProcRoomTemplate kTheme45Corridors[] = {
	{ "Galleria dei Frammenti",
	  "Pezzi di citta' diverse fluttuano nel vuoto.\n"
	  "Gravita' e' una suggestione qui.\n",
	  SECT_AIR, static_cast<long>(INDOORS | DARK) },
	{ "Corridoio delle Geometrie",
	  "Angoli impossibili formano passaggi.\n"
	  "Camminare in linea retta e' un'illusione.\n",
	  SECT_AIR, static_cast<long>(INDOORS | DARK) },
	{ "Passaggio dei Sussurri Planari",
	  "Voci di mille mondi parlano simultaneamente.\n"
	  "La mente fatica a restare coesa.\n",
	  SECT_AIR, static_cast<long>(INDOORS | DARK) },
	{ "Atrio del Mercante Planare",
	  "Bancarelle impossibili vendono merci di ogni realta'.\n"
	  "Il mercante non ha volto fisso.\n",
	  SECT_AIR, static_cast<long>(INDOORS | DARK) },
};

static const ProcRoomTemplate kTheme45Treasures[] = {
	{ "Tesoro Planare",
	  "Oggetti di mondi che non esistono piu'.\n"
	  "Monete con simboli sconosciuti.\n",
	  SECT_AIR, static_cast<long>(INDOORS | DARK) },
	{ "Camera delle Chiavi",
	  "Chiavi di portali per ogni piano.\n"
	  "Aprirne una senza sapere dove porta e' folle.\n",
	  SECT_AIR, static_cast<long>(INDOORS | DARK) },
};

static const ProcRoomTemplate kTheme45Traps[] = {
	{ "Trappola della Gravita'",
	  "La gravita' cambia direzione senza preavviso.\n"
	  "Cadere verso il soffitto e' possibile.\n",
	  SECT_AIR, static_cast<long>(INDOORS | DARK) },
	{ "Sala delle Illusioni Planari",
	  "Copie di te stesso attaccano da ogni angolo.\n"
	  "Quale e' reale?\n",
	  SECT_AIR, static_cast<long>(INDOORS | DARK) },
};

static const ProcRoomTemplate kTheme45Boss = {
	"Crocevia di Sigil",
	  "Una piazza fluttuante ospita un crocevia infinito.\n"
	  "Portali verso ogni piano si aprono e chiudono.\n"
	  "Un guardiano planare veglia sul varco.\n",
	SECT_AIR, static_cast<long>(INDOORS | DARK)
};

/* --- Tema 46: Castello delle Ombre --- */

static const ProcRoomTemplate kTheme46Entrances[] = {
	{ "Cancello del Castello",
	  "Cancello di ferro arrugginito scricchiola al vento.\n"
	  "Torri spezzate perforano un cielo grigio.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
	{ "Portone Principale",
	  "Un portone massiccio e' socchiuso.\n"
	  "Odore di muffa e cera vecchia esce dall'interno.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
};

static const ProcRoomTemplate kTheme46Corridors[] = {
	{ "Corridoio delle Tappezzerie",
	  "Arazzi sbiaditi raffigurano cacce macabre.\n"
	  "Occhi sembrano seguirti tra le figure.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
	{ "Galleria delle Armature",
	  "Armature vuote sorvegliano ogni passaggio.\n"
	  "Spade arrugginite giacciono ai loro piedi.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
	{ "Passaggio delle Candele",
	  "Candele nere illuminano debolmente.\n"
	  "Ombre danzano sulle pareti senza sorgente.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
	{ "Atrio della Scala",
	  "Una scala a chiocciola sale verso l'oscurita'.\n"
	  "Ragnatele coprono ogni gradino.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
};

static const ProcRoomTemplate kTheme46Treasures[] = {
	{ "Tesoro del Conte",
	  "Gioielli e argenteria di famiglie nobili.\n"
	  "Medaglioni con ritratti di defunti.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
	{ "Biblioteca Proibita",
	  "Libri di necromanzia e vampirismo.\n"
	  "Pagine che si muovono da sole.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
};

static const ProcRoomTemplate kTheme46Traps[] = {
	{ "Sala del Pavimento Cedevole",
	  "Assi marce nascondono fosse sotto.\n"
	  "Legno che scricchiola avverte troppo tardi.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
	{ "Passaggio dei Pipistrelli",
	  "Pipistrelli si lanciano in sciami al passaggio.\n"
	  "Zanne e artigli graffiano da ogni lato.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
};

static const ProcRoomTemplate kTheme46Boss = {
	"Sala del Trono Nero",
	  "Una sala gotica culmina in un trono di velluto nero.\n"
	  "Finestre alte mostrano un cielo eternamente notturno.\n"
	  "Il conte delle ombre attende con calice di sangue.\n",
	SECT_INSIDE, static_cast<long>(INDOORS | DARK)
};

/* --- Tema 47: Antro del Drago --- */

static const ProcRoomTemplate kTheme47Entrances[] = {
	{ "Fessura Fumante",
	  "Fumo esce da una fessura nella montagna.\n"
	  "Zolfo e calore avvertono di cio' che attende.\n",
	  SECT_MOUNTAIN, static_cast<long>(INDOORS | DARK) },
	{ "Covo del Predatore",
	  "Artigli e scaglie segnano l'ingresso.\n"
	  "Ossa di grandi prede giacciono ammucchiate.\n",
	  SECT_MOUNTAIN, static_cast<long>(INDOORS | DARK) },
};

static const ProcRoomTemplate kTheme47Corridors[] = {
	{ "Galleria delle Scaglie",
	  "Scaglie incastonate nella roccia brillano.\n"
	  "Calore aumenta ad ogni passo.\n",
	  SECT_MOUNTAIN, static_cast<long>(INDOORS | DARK) },
	{ "Corridoio del Tesoro Parziale",
	  "Piccoli tesori precedono il tesoro vero.\n"
	  "Monete e gemme sparsi sul pavimento.\n",
	  SECT_MOUNTAIN, static_cast<long>(INDOORS | DARK) },
	{ "Passaggio del Soffio",
	  "Braci ancora calde segnano il passaggio del drago.\n"
	  "Roccia fusa forma stalattiti bizzarre.\n",
	  SECT_MOUNTAIN, static_cast<long>(INDOORS | DARK) },
	{ "Atrio delle Ossa",
	  "Ossa di cavalli, nani e giganti formano muri.\n"
	  "Cranii impilati come decorazione.\n",
	  SECT_MOUNTAIN, static_cast<long>(INDOORS | DARK) },
};

static const ProcRoomTemplate kTheme47Treasures[] = {
	{ "Tesoro del Drago",
	  "Oro e gemme coprono ogni superficie.\n"
	  "Armi magiche di eroi caduti.\n",
	  SECT_MOUNTAIN, static_cast<long>(INDOORS | DARK) },
	{ "Camera delle Uova",
	  "Uova di drago pulsano di calore.\n"
	  "Non dovresti essere qui.\n",
	  SECT_MOUNTAIN, static_cast<long>(INDOORS | DARK) },
};

static const ProcRoomTemplate kTheme47Traps[] = {
	{ "Trappola del Soffio",
	  "Getti di fiamma erompono da fessure.\n"
	  "Il drago conosce ogni centimetro.\n",
	  SECT_MOUNTAIN, static_cast<long>(INDOORS | DARK) },
	{ "Sala del Crollo",
	  "Il soffitto e' stato indebolito dal fuoco.\n"
	  "Pietre cadono senza preavviso.\n",
	  SECT_MOUNTAIN, static_cast<long>(INDOORS | DARK) },
};

static const ProcRoomTemplate kTheme47Boss = {
	"Covo del Drago Antico",
	  "Una caverna immensa ospita un drago ancora vivo.\n"
	  "Tesoro innumerevole brilla sotto le scaglie.\n"
	  "Il drago apre un occhio e poi l'altro.\n",
	SECT_MOUNTAIN, static_cast<long>(INDOORS | DARK)
};

/* --- Tema 48: Caverna degli Occhi --- */

static const ProcRoomTemplate kTheme48Entrances[] = {
	{ "Fessura Aberrante",
	  "Pareti pulsano come carne viva.\n"
	  "Occhi crescono dalla roccia e scompaiono.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
	{ "Bocca della Caverna",
	  "Un odore mentale ti assale prima del fisico.\n"
	  "Geometrie sbagliate fanno male agli occhi.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
};

static const ProcRoomTemplate kTheme48Corridors[] = {
	{ "Galleria delle Tentacoli",
	  "Tentacoli sporgono e si ritirano nelle pareti.\n"
	  "Muco viscido copre ogni superficie.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
	{ "Corridoio della Paranoia",
	  "Sussurri nella tua mente senza fonte.\n"
	  "I compagni sembrano nemici per un istante.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
	{ "Passaggio delle Illusioni",
	  "Visioni di morte e follia ovunque.\n"
	  "La realta' e' negoziabile qui.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
	{ "Atrio del Beholder",
	  "Una sfera fluttuante osserva ogni angolo.\n"
	  "Raggi di energia hanno bruciato il pavimento.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
};

static const ProcRoomTemplate kTheme48Treasures[] = {
	{ "Tesoro della Mente",
	  "Cristalli di memoria rubate.\n"
	  "Pensieri altrui fluttuano in fiale.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
	{ "Camera degli Esperimenti",
	  "Creature fuse e mutilate in gabbie.\n"
	  "Strumenti chirurgici alieni.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
};

static const ProcRoomTemplate kTheme48Traps[] = {
	{ "Trappola del Raggio",
	  "Raggi di disintegrazione sparano dalle pareti.\n"
	  "Occhi nascosti attendono il movimento.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
	{ "Sala della Follia",
	  "La mente subisce assalti psichici.\n"
	  "Resistere e' la vera prova.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
};

static const ProcRoomTemplate kTheme48Boss = {
	"Nido del Beholder",
	  "Una caverna sferica ospita un beholder antico.\n"
	  "Occhi su tentacoli scrutano ogni difetto.\n"
	  "La creatura alza i suoi ragghi mortali.\n",
	SECT_INSIDE, static_cast<long>(INDOORS | DARK)
};

/* --- Tema 49: Fosso Infernale --- */

static const ProcRoomTemplate kTheme49Entrances[] = {
	{ "Porta Infernale",
	  "Un arco di bronzo reca runa infernali.\n"
	  "Calore e zolfo escono dal varco.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
	{ "Scala dei Dannati",
	  "Gradini di ossidiana scendono verso il calore.\n"
	  "Voci di tormento salgono dal basso.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
};

static const ProcRoomTemplate kTheme49Corridors[] = {
	{ "Galleria delle Catene",
	  "Catene infuocate pendono dal soffitto.\n"
	  "Ceneri di dannati coprono il pavimento.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
	{ "Corridoio dei Diavoli",
	  "Impronte di zoccoli bruciate nella pietra.\n"
	  "Odore di zolfo e paura.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
	{ "Passaggio del Fuoco Eterno",
	  "Fiamme che non consumano tutto illuminano il cammino.\n"
	  "Calore insopportabile verso il centro.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
	{ "Atrio del Principe",
	  "Trono di lava solidificata domina la sala.\n"
	  "Stendardi infernali segnano territorio.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
};

static const ProcRoomTemplate kTheme49Treasures[] = {
	{ "Tesoro Infernale",
	  "Oro maledetto e gemme corrotte.\n"
	  "Contratti firmati con sangue.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
	{ "Camera dei Contratti",
	  "Anime imprigionate in pergamene.\n"
	  "Firme ancora fresche su documenti antichi.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
};

static const ProcRoomTemplate kTheme49Traps[] = {
	{ "Trappola del Fuoco",
	  "Getti di fiamma infernale erompono dal pavimento.\n"
	  "Bruciare e' solo l'inizio.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
	{ "Sala delle Tentazioni",
	  "Visioni dei desideri piu' profondi.\n"
	  "Cedere significa restare per sempre.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
};

static const ProcRoomTemplate kTheme49Boss = {
	"Trono dei Nove Inferi",
	  "Una sala di fuoco e ossidiana culmina in un trono.\n"
	  "Diavoli minori servono un principe infernale.\n"
	  "Il signore del fosso pronuncia la tua condanna.\n",
	SECT_INSIDE, static_cast<long>(INDOORS | DARK)
};

#endif /* __PROCAREA_THEMES_EXT_INC */
