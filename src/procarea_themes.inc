/* Tematiche procedurali antro effimero (25 set) */

static const ProcRoomTemplate kTheme0Entrances[] = {
	{ "Ingresso dell'Antro Effimero",
	  "Una fessura nel terreno rivela una discesa verso l'oscurita'.\n"
	  "L'aria e' fredda e odora di terra bagnata e muschio.\n"
	  "Passaggi instabili sembrano richiamarti piu' in profondita'.\n",
	  SECT_HILLS, static_cast<long>(INDOORS | DARK) },
	{ "Varco di Pietra Grezza",
	  "Due massi inclinati formano un varco stretto verso il basso.\n"
	  "Il terreno e' scivoloso e l'eco dei tuoi passi rimbomba lontano.\n",
	  SECT_HILLS, static_cast<long>(INDOORS | DARK) },
};
static const ProcRoomTemplate kTheme0Corridors[] = {
	{ "Corridoio Stretto",
	  "Un passaggio stretto costringe a procedere a singolo file.\n"
	  "Gocce d'acqua cadono ritmicamente da qualche parte del soffitto.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
	{ "Galleria Rocciosa",
	  "La roccia grezza forma pareti irregolari e un soffitto basso.\n"
	  "Ogni passo fa scricchiolare ghiaia sotto i piedi.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
	{ "Cunicolo Umido",
	  "Il pavimento e' coperto da una sottile patina d'acqua.\n"
	  "Senti odore di muffa e un debole fruscio da qualche parte.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
	{ "Passaggio Curvo",
	  "Il tunnel curva verso destra e perde subito la luce esterna.\n"
	  "Incisioni primitive segnano alcune pietre lungo il cammino.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
	{ "Atrio di Pietra",
	  "Una piccola camera naturale si apre davanti a te.\n"
	  "Stalattiti sottili pendono dal soffitto come zanne di pietra.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
};
static const ProcRoomTemplate kTheme0Treasures[] = {
	{ "Nicchia Abbandonata",
	  "Una nicchia nel muro sembra essere stata usata come nascondiglio.\n"
	  "Resti di stoffa marcia e ossicini piccoli giacciono sparsi.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
	{ "Cella del Tesoro",
	  "Questa stanza e' curiosamente piu' asciutta delle altre.\n"
	  "Un cumulo di detriti nasconde qualcosa che luccica debolmente.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
};
static const ProcRoomTemplate kTheme0Traps[] = {
	{ "Sala Trabocchettata",
	  "Il pavimento presenta piastrelle leggermente piu' basse delle altre.\n"
	  "Qualcosa non quadra: l'aria qui e' troppo immobile.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
	{ "Passaggio Inquietante",
	  "Graffi recenti segnano le pareti, come se qualcuno fosse fuggito in fretta.\n"
	  "Un odore acre ti punge le narici.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
};
static const ProcRoomTemplate kTheme0Boss = {
	"Caverna del Predatore",
	"Una caverna piu' ampia si apre all'improvviso.\n"
	"Ossa consumate formano un macabro semicerchio al centro.\n"
	"Un ringhio basso e' l'unico avvertimento prima dell'attacco.\n",
	SECT_INSIDE, static_cast<long>(INDOORS | DARK)
};

static const ProcRoomTemplate kTheme1Entrances[] = {
	{ "Bocca della Cripta Effimera",
	  "Radici contorte incorniciano l'accesso a una cripta che non sembra "
	  "esistere da sempre.\n"
	  "Ogni tanto un soffio di vento sale dal basso, portando echi lontani.\n",
	  SECT_FOREST, static_cast<long>(INDOORS | DARK) },
	{ "Scalinata Sepolta",
	  "Gradini consumati scendono in una fossa di pietra.\n"
	  "L'odore di terra secca e rame vecchio aleggia nell'aria.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
};
static const ProcRoomTemplate kTheme1Corridors[] = {
	{ "Corridoio delle Ossa",
	  "Piccole ossa polverose formano strani cumuli lungo le pareti.\n"
	  "Il silenzio e' rotto solo dal tuo respiro.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
	{ "Volta Crollata",
	  "Macigni caduti costringono a passare curvandosi.\n"
	  "La polvere sollevata ti punge gli occhi.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
	{ "Galleria delle Lapidi",
	  "Lastre di pietra inclinate segnano passaggi gia' dimenticati.\n"
	  "Runa sbiadite si intravedono nella penombra.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
	{ "Atrio Sepolcrale",
	  "Una nicchia centrale domina la stanza come un altare vuoto.\n"
	  "Senti un freddo che non appartiene a questo mondo.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
};
static const ProcRoomTemplate kTheme1Treasures[] = {
	{ "Tomba Saccheggiata",
	  "Un sarcofago vuoto giace rovesciato al centro.\n"
	  "Qualcosa e' stato lasciato indietro nella furia del saccheggio.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
	{ "Reliquiario Dimenticato",
	  "Una nicchia murata e' stata forzata di recente.\n"
	  "Resti di cera e stoffa pregiata giacciono sparsi.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
};
static const ProcRoomTemplate kTheme1Traps[] = {
	{ "Camera a Punta",
	  "Il pavimento e' coperto di buche poco profonde e sospette.\n"
	  "Segni di lama segnano la pietra circostante.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
	{ "Passaggio Maledetto",
	  "Simboli arsi a fuoco ricoprono l'architrave dell'uscita.\n"
	  "Un brivido ti percorre la schiena senza ragione apparente.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
};
static const ProcRoomTemplate kTheme1Boss = {
	"Sanctum del Guardiano",
	"Una sala circolare si apre davanti a te, dominata da una statua spezzata.\n"
	"Due fuochi fatui danzano sopra resti di armature antiche.\n"
	"Qualcosa si stacca dall'ombra con intento assassino.\n",
	SECT_INSIDE, static_cast<long>(INDOORS | DARK)
};

static const ProcRoomTemplate kTheme2Entrances[] = {
	{ "Varco Nebbioso",
	  "Una coltre di nebbia bassa avvolge l'entrata di un antro poco profondo.\n"
	  "Le pareti sono stillanti e il pavimento e' coperto di ghiaia scivolosa.\n",
	  SECT_FIELD, static_cast<long>(INDOORS | DARK) },
	{ "Breccia nella Nebbia",
	  "La nebbia si condensa in gocce sulle pareti invisibili.\n"
	  "Ogni passo sembra muffato, come se il suono venisse inghiottito.\n",
	  SECT_FIELD, static_cast<long>(INDOORS | DARK) },
};
static const ProcRoomTemplate kTheme2Corridors[] = {
	{ "Passaggio Evanescente",
	  "La nebbia e' cosi' fitta che non vedi il soffitto.\n"
	  "Per un istante credi di sentire passi accanto a te.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
	{ "Galleria Sbiadita",
	  "Contorni incerti si dissolvono appena li guardi.\n"
	  "Il pavimento sembra cedere sotto il peso del tuo corpo.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
	{ "Curva di Vapore",
	  "Una curva improvvisa nasconde cio' che viene dopo.\n"
	  "Sussurri lontani si confondono con il fruscio del tuo equipaggiamento.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
	{ "Atrio di Bruma",
	  "La stanza sembra piu' ampia di quanto le pareti lascino supporre.\n"
	  "L'aria vibra di una tensione silenziosa.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
};
static const ProcRoomTemplate kTheme2Treasures[] = {
	{ "Nicchia nella Bruma",
	  "La nebbia si dirada un istante su un incavo nel muro.\n"
	  "Qualcosa luccica e scompare subito dopo.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
	{ "Cumulo Illusorio",
	  "Detriti e riflessi si mescolano in un mucchio confuso.\n"
	  "Non sei sicuro di cio' che e' reale e cio' che e' soltanto nebbia.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
};
static const ProcRoomTemplate kTheme2Traps[] = {
	{ "Sala dei Specchi",
	  "Superfici lucide suggeriscono passaggi che non esistono.\n"
	  "Il disorientamento ti fa esitare un passo di troppo.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
	{ "Passaggio Instabile",
	  "Il suolo cede leggermente sotto di te, poi torna solido.\n"
	  "L'antro sembra reagire alla tua presenza.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
};
static const ProcRoomTemplate kTheme2Boss = {
	"Cuore della Nebbia",
	"La nebbia si dirada in un'arena silenziosa.\n"
	"Un'ombra massiccia prende forma al centro, alimentata da vortici grigi.\n"
	"Non hai molto tempo per capire cosa sia.\n",
	SECT_INSIDE, static_cast<long>(INDOORS | DARK)
};

static const ProcRoomTemplate kTheme3Entrances[] = {
	{ "Crepaccio Inundato",
	  "Acqua fredda ti arriva alle caviglie appena oltrepassi la soglia.\n"
	  "Pareti coperte di muschio pulsano debolmente alla luce del tuo passaggio.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
	{ "Breccia Sommersa",
	  "Una frattura nel terreno rivela cunicoli parzialmente allagati.\n"
	  "Senti odore di alghe e pietra bagnata.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
};
static const ProcRoomTemplate kTheme3Corridors[] = {
	{ "Canale Roccioso",
	  "Un rivolo d'acqua scorre lungo il centro del passaggio.\n"
	  "Le pareti gocciolano in un ritmo costante.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
	{ "Galleria Allagata",
	  "Chiazze d'acqua stagnante rendono il cammino incerto.\n"
	  "Ogni passo solleva un'eco smorzata.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
	{ "Passaggio di Rovine",
	  "Blocchi di pietra lavorata sporgono dalle pareti naturali.\n"
	  "Resti di mosaico si intravedono sotto il muschio.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
	{ "Atrio delle Colonne",
	  "Mezzecolonne spezzate delimitano uno spazio un tempo piu' nobile.\n"
	  "L'acqua gocciola dall'alto come pioggia lenta.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
};
static const ProcRoomTemplate kTheme3Treasures[] = {
	{ "Camera del Tesoro Sommerso",
	  "Una volta asciutta, ora invasa da acqua bassa.\n"
	  "Qualcosa luccica sotto la superficie.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
	{ "Relitto di Pietra",
	  "Frammenti di statue e vasi rotti giacciono ammassati.\n"
	  "Un oggetto intatto sembra fuori posto tra le macerie.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
};
static const ProcRoomTemplate kTheme3Traps[] = {
	{ "Sala del Ponte Spezzato",
	  "Resti di un ponte sospeso oscillano sopra un vuoto d'acqua.\n"
	  "Il legno marcio scricchiola minacciosamente.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
	{ "Passaggio Sifonante",
	  "L'acqua sale e scende in cicli imprevedibili.\n"
	  "Senti correnti nascoste tirare verso il basso.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
};
static const ProcRoomTemplate kTheme3Boss = {
	"Arena delle Rovine Profonde",
	"Una vasta sala sommersa fino alle ginocchia si apre davanti a te.\n"
	"Colonne spezzate incorniciano un trono di pietra vuoto.\n"
	"Qualcosa si risveglia dal fango al centro.\n",
	SECT_INSIDE, static_cast<long>(INDOORS | DARK)
};

static const ProcRoomTemplate kTheme4Entrances[] = {
	{ "Ingresso della Forgia",
	  "Un arco di pietra nera conduce in un antro caldo e fumoso.\n"
	  "Senti odore di ferro bruciato e carbone spento da giorni.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK | HOT) },
	{ "Crepaccio Rovente",
	  "Vapori caldi salgono da fessure nel pavimento.\n"
	  "La roccia e' scura e lucida come se fosse stata vitrificata.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK | HOT) },
};
static const ProcRoomTemplate kTheme4Corridors[] = {
	{ "Passaggio delle Incudini",
	  "Incudini rovesciate e martelli arrugginiti bloccano parzialmente il cammino.\n"
	  "Scintille spente macchiano le pareti.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK | HOT) },
	{ "Galleria delle Catene",
	  "Catene pendono dal soffitto come liane di metallo.\n"
	  "Ogni passo fa vibrare un debole rintocco.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK | HOT) },
	{ "Corridoio delle Braci",
	  "Carboni ardenti illuminano debolmente nicchie vuote.\n"
	  "L'aria e' secca e punge la gola.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK | HOT) },
	{ "Atrio della Fornace",
	  "Una fornace spenta domina la stanza come un occhio cavo.\n"
	  "Ceneri sottili coprono ogni superficie.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK | HOT) },
};
static const ProcRoomTemplate kTheme4Treasures[] = {
	{ "Deposito di Lingotti",
	  "Lingotti deformi giacciono ammassati in un angolo.\n"
	  "Qualcosa di piu' piccolo luccica tra il metallo spento.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK | HOT) },
	{ "Scrigno del Fabbro",
	  "Un banco da lavoro crollato nasconde cassetti intrappolati.\n"
	  "Attrezzi fini sono ancora impigliati nel legno carbonizzato.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK | HOT) },
};
static const ProcRoomTemplate kTheme4Traps[] = {
	{ "Sala delle Lamine",
	  "Lame a molla sporgono appena dal pavimento polveroso.\n"
	  "Segni di tagli recenti incrociano le piastrelle.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK | HOT) },
	{ "Passaggio Rovente",
	  "Il pavimento e' insolitamente caldo in punti precisi.\n"
	  "Vapor acido sfiata da fessure sottili.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK | HOT) },
};
static const ProcRoomTemplate kTheme4Boss = {
	"Forgia del Dominatore",
	"Una vasta sala riecheggia del calore residuo di mille martellate.\n"
	"Un'incudine enorme troneggia al centro, circondata da armi incomplete.\n"
	"Qualcosa si stacca dall'ombra con passi pesanti.\n",
	SECT_INSIDE, static_cast<long>(INDOORS | DARK | HOT)
};

static const ProcRoomTemplate kTheme5Entrances[] = {
	{ "Ghiaccio Sottile",
	  "Una fessura rivestita di brina conduce verso il freddo profondo.\n"
	  "Il tuo alito si condensa all'istante.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK | COLD) },
	{ "Bocca del Gelo",
	  "Cristalli di ghiaccio incorniciano un passaggio stretto.\n"
	  "Scricchiolii lontani echeggiano come passi su neve dura.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK | COLD) },
};
static const ProcRoomTemplate kTheme5Corridors[] = {
	{ "Galleria Ghiacciata",
	  "Stalattiti di ghiaccio pendono minacciosamente dal soffitto basso.\n"
	  "Il pavimento e' scivoloso e coperto da una crosta trasparente.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK | COLD) },
	{ "Passaggio Sibilante",
	  "Venti gelidi attraversano fessure invisibili nelle pareti.\n"
	  "La luce sembra assottigliarsi ad ogni passo.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK | COLD) },
	{ "Corridoio di Brina",
	  "Ogni superficie e' ricoperta da un velo bianco e granuloso.\n"
	  "Senti le dita intorpidite nonostante il movimento.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK | COLD) },
	{ "Atrio del Gelo Eterno",
	  "Una colonna di ghiaccio al centro riflette la tua figura distorta.\n"
	  "Il silenzio qui e' cosi' denso da far male alle orecchie.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK | COLD) },
};
static const ProcRoomTemplate kTheme5Treasures[] = {
	{ "Cavità Congelata",
	  "Oggetti intrappolati nel ghiaccio luccicano debolmente.\n"
	  "Uno strato superficiale sembra piu' fragile del resto.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK | COLD) },
	{ "Tesoro del Bianco",
	  "Un mucchio di oggetti congelati forma una scultura caotica.\n"
	  "Qualcosa spicca ancora riconoscibile in mezzo al ghiaccio.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK | COLD) },
};
static const ProcRoomTemplate kTheme5Traps[] = {
	{ "Sala delle Lame di Ghiaccio",
	  "Schegge affilate sporgono dal pavimento come zanne.\n"
	  "Il freddo qui e' piu' intenso che altrove.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK | COLD) },
	{ "Passaggio Instabile",
	  "Il ghiaccio sotto i tuoi piedi scricchiola in modo inquietante.\n"
	  "Crepe sottili si propagano dove calpesti.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK | COLD) },
};
static const ProcRoomTemplate kTheme5Boss = {
	"Trono di Ghiaccio Nero",
	"Una caverna aperta culmina in un trono scolpito nel ghiaccio.\n"
	"Fratture nere attraversano la parete come vene gelide.\n"
	"Una figura immobile si scolla dal trono.\n",
	SECT_INSIDE, static_cast<long>(INDOORS | DARK | COLD)
};

static const ProcRoomTemplate kTheme6Entrances[] = {
	{ "Tela dell'Ingresso",
	  "Ragnatele dense ostruiscono parzialmente l'accesso.\n"
	  "Piccoli corpi essiccati pendono come macabri trofei.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
	{ "Fessura Infestata",
	  "Un intreccio di fili appiccicosi ricopre ogni sporgenza rocciosa.\n"
	  "Senti un fruscio secco provenire da piu' direzioni.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
};
static const ProcRoomTemplate kTheme6Corridors[] = {
	{ "Galleria delle Tele",
	  "Tele spesse riducono lo spazio a un corridoio claustrofobico.\n"
	  "Qualcosa si muove appena fuori dal tuo campo visivo.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
	{ "Passaggio Appiccicoso",
	  "I fili si attaccano alle tue spalle ad ogni passo.\n"
	  "Resti di prede giacciono avvolti in bozzoli grigi.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
	{ "Corridoio dei Bozzoli",
	  "Sacche ovattate pendono dal soffitto come frutti immaturi.\n"
	  "Alcuni sembrano ancora pulsare debolmente.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
	{ "Atrio del Ragnatelaio",
	  "Una volta naturale e' stata completamente rivestita di seta.\n"
	  "Ossa piccole formano un tappeto scricchiolante.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
};
static const ProcRoomTemplate kTheme6Treasures[] = {
	{ "Deposito di Prede",
	  "Equipaggiamenti strappati giacciono ammucchiati in un angolo.\n"
	  "Qualcosa di valore e' sopravvissuto alla digestione.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
	{ "Nicchia del Tesoro Avvolto",
	  "Un bozzolo parzialmente aperto rivela oggetti lucidi all'interno.\n"
	  "Filamenti appiccicosi rendono difficile avvicinarsi.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
};
static const ProcRoomTemplate kTheme6Traps[] = {
	{ "Sala delle Trappole di Seta",
	  "Fili tesi attraversano la stanza all'altezza del collo.\n"
	  "Non li vedi finche' non sfiori qualcosa di appiccicoso.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
	{ "Passaggio Collante",
	  "Il pavimento e' coperto da un velo viscoso che frena ogni passo.\n"
	  "Vibrazioni leggere corrono lungo le pareti.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
};
static const ProcRoomTemplate kTheme6Boss = {
	"Nido della Regina",
	"Una cupola naturale e' stata trasformata in un nido enorme.\n"
	  "Tele fresche brillano debolmente al centro.\n"
	  "Occhi multipli si aprono nell'oscurita' sopra di te.\n",
	SECT_INSIDE, static_cast<long>(INDOORS | DARK)
};

static const ProcRoomTemplate kTheme7Entrances[] = {
	{ "Cenere e Vento",
	  "Un crollo parziale rivela un cunicolo pieno di cenere soffocante.\n"
	  "Ogni respiro ti riempie la bocca di polvere amara.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK | HOT) },
	{ "Varco Bruciato",
	  "Pareti annerite segnano il confine di un antro devastato dal fuoco.\n"
	  "Calore residuo aleggia ancora nell'aria stagnante.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK | HOT) },
};
static const ProcRoomTemplate kTheme7Corridors[] = {
	{ "Galleria delle Ceneri",
	  "Ceneri profonde rendono incerto ogni appoggio.\n"
	  "Resti carbonizzati sporgono come arbusti neri.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK | HOT) },
	{ "Passaggio Fumoso",
	  "Fumo basso aleggia a mezz'altezza offuscando la vista.\n"
	  "Tossi secche echeggiano da qualche parte davanti a te.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK | HOT) },
	{ "Corridoio delle Scintille",
	  "Schegge ardenti cadono occasionalmente dal soffitto instabile.\n"
	  "Odore di legno e carne bruciata si mescolano.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK | HOT) },
	{ "Atrio del Fuoco Spento",
	  "Un antico rogo al centro e' ridotto a un cumulo di braci morte.\n"
	  "Simboli scrostati circondano il perimetro.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK | HOT) },
};
static const ProcRoomTemplate kTheme7Treasures[] = {
	{ "Reliquia Carbonizzata",
	  "Oggetti mezzo fusi giacciono in una nicchia riparata dal fuoco.\n"
	  "Qualcosa ha resistito alla temperatura infernale.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK | HOT) },
	{ "Forziere delle Braci",
	  "Un contenitore di metallo scuro e' ancora tiepido al tatto.\n"
	  "Ceneri soffocanti si sollevano quando ti avvicini.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK | HOT) },
};
static const ProcRoomTemplate kTheme7Traps[] = {
	{ "Sala del Fuoco Rinascente",
	  "Piastrelle incandescenti si accendono a intermittenza sotto i piedi.\n"
	  "L'aria vibra di calore improvviso.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK | HOT) },
	{ "Passaggio Instabile",
	  "Il soffitto gocciola resina bollente in punti imprevedibili.\n"
	  "Segni di ustioni fresche segnano il cammino.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK | HOT) },
};
static const ProcRoomTemplate kTheme7Boss = {
	"Altare delle Fiamme Morte",
	"Una piattaforma rialzata domina la sala devastata.\n"
	  "Fiamme spettrali danzano senza calore attorno a un trono vuoto.\n"
	  "Una figura brucia ancora pur essendo viva.\n",
	SECT_INSIDE, static_cast<long>(INDOORS | DARK | HOT)
};

static const ProcRoomTemplate kTheme8Entrances[] = {
	{ "Frattura di Cristallo",
	  "Pareti luccicanti conducono in un antro di riflessi distorti.\n"
	  "Ogni suono si moltiplica in echi acuti.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK | BRIGHT) },
	{ "Varco Prismatico",
	  "Cristalli affilati incorniciano l'ingresso come zanne di vetro.\n"
	  "La luce sembra piegarsi attraverso le superfici taglienti.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK | BRIGHT) },
};
static const ProcRoomTemplate kTheme8Corridors[] = {
	{ "Galleria Riflettente",
	  "Mille immagini deformate del tuo passaggio ti accompagnano.\n"
	  "E' difficile distinguere cio' che e' reale da cio' che e' riflesso.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK | BRIGHT) },
	{ "Passaggio Tagliente",
	  "Schegge di cristallo sporgono da ogni angolo.\n"
	  "Un passo incerto potrebbe costare caro.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK | BRIGHT) },
	{ "Corridoio delle Facette",
	  "Pareti multifaccette scintillano con luce interna debole.\n"
	  "Il pavimento e' coperto da polvere di vetro sottile.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK | BRIGHT) },
	{ "Atrio del Prisma",
	  "Un cristallo enorme sospeso al centro divide la luce in arcobaleni spenti.\n"
	  "L'aria sa di ozono e pietra fredda.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK | BRIGHT) },
};
static const ProcRoomTemplate kTheme8Treasures[] = {
	{ "Geode Spaccato",
	  "Una geode aperta rivela minerali grezzi di valore incerto.\n"
	  "Schegge affilate rendono difficile raccogliere il contenuto.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK | BRIGHT) },
	{ "Cumulo di Gemme",
	  "Pietre lucenti giacciono ammucchiate come sabbia grossa.\n"
	  "Alcune sembrano troppo perfette per essere naturali.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK | BRIGHT) },
};
static const ProcRoomTemplate kTheme8Traps[] = {
	{ "Sala delle Schegge",
	  "Cristalli affilati sporgono dal pavimento in pattern irregolari.\n"
	  "Riflessi ingannevoli nascondono i punti piu' pericolosi.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK | BRIGHT) },
	{ "Passaggio Fratturato",
	  "Il pavimento di cristallo cede con suoni di vetro rotto.\n"
	  "Fessure sottili si aprono dove meno te lo aspetti.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK | BRIGHT) },
};
static const ProcRoomTemplate kTheme8Boss = {
	"Cuore di Cristallo",
	"Una caverna interamente rivestita di facce lucenti pulsa di luce interna.\n"
	  "Al centro, un monolito scintillante domina la sala.\n"
	  "Qualcosa si muove dentro la pietra vivente.\n",
	SECT_INSIDE, static_cast<long>(INDOORS | DARK | BRIGHT)
};

static const ProcRoomTemplate kTheme9Entrances[] = {
	{ "Bocca Fungina",
	  "Giganteschi funghi luminescenti fiancheggiano l'ingresso.\n"
	  "Un odore dolciastro ti invade le narici all'istante.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK | UNDERGROUND) },
	{ "Discesa Sporigena",
	  "Spore dense fluttuano nell'aria come nebbia verdastra.\n"
	  "Il terreno e' molle e elastico sotto i piedi.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK | UNDERGROUND) },
};
static const ProcRoomTemplate kTheme9Corridors[] = {
	{ "Galleria dei Funghi",
	  "Colonne fungine formano un bosco sotterraneo innaturale.\n"
	  "Gocce vischiose cadono da cappelli enormi.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK | UNDERGROUND) },
	{ "Passaggio Micelio",
	  "Radici biancastre intrecciate coprono pareti e soffitto.\n"
	  "Un fruscio costante sembra provenire dal terreno stesso.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK | UNDERGROUND) },
	{ "Corridoio delle Spore",
	  "Nuvole di spore pulsano lentamente nell'aria immobile.\n"
	  "Tossire qui sembra attirare qualcosa.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK | UNDERGROUND) },
	{ "Atrio del Cappello Rosso",
	  "Un fungo rosso enorme domina la stanza come un baldacchino.\n"
	  "Luci biancastre pulsano ritmicamente sotto di esso.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK | UNDERGROUND) },
};
static const ProcRoomTemplate kTheme9Treasures[] = {
	{ "Culla di Spore",
	  "Sacche di spore dorate giacciono protette da anelli di micelio.\n"
	  "Qualcosa di solido e' cresciuto al loro interno.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK | UNDERGROUND) },
	{ "Deposito Organico",
	  "Oggetti avvolti in tessuto fungino attendono di essere scoperti.\n"
	  "La materia viscosa si contrae al tuo avvicinarti.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK | UNDERGROUND) },
};
static const ProcRoomTemplate kTheme9Traps[] = {
	{ "Sala delle Spore Acide",
	  "Nuvole verdi si concentrano vicino al pavimento.\n"
	  "La pelle prude appena vi entri.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK | UNDERGROUND) },
	{ "Passaggio Elastico",
	  "Il terreno cede e ti riporta indietro con un tonfo sordo.\n"
	  "Filamenti viscidi tentano di avvolgere le caviglie.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK | UNDERGROUND) },
};
static const ProcRoomTemplate kTheme9Boss = {
	"Santuario del Mycelium",
	"Una cupola naturale e' completamente rivestita da funghi giganti.\n"
	  "Al centro, un trono organico pulsa come un cuore lento.\n"
	  "Una forma massiccia si stacca dalla parete vivente.\n",
	SECT_INSIDE, static_cast<long>(INDOORS | DARK | UNDERGROUND)
};

static const ProcRoomTemplate kTheme10Entrances[] = {
	{ "Portico Rovinato",
	  "Colonne spezzate segnano l'ingresso di un tempio dimenticato.\n"
	  "Sabbia fine ha invaso ogni gradino.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK | DESERTIC) },
	{ "Scalinata Sepolta",
	  "Gradini consumati scendono verso sale sotterranee di culto.\n"
	  "Idoli senza volto guardano il tuo passaggio.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK | DESERTIC) },
};
static const ProcRoomTemplate kTheme10Corridors[] = {
	{ "Galleria delle Colonne",
	  "File di colonne spezzate creano un labirinto di ombre.\n"
	  "Iscrizioni sbiadite raccontano riti dimenticati.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK | DESERTIC) },
	{ "Passaggio delle Arene",
	  "Sabbia fine copre il pavimento rendendo faticoso ogni passo.\n"
	  "Venti secchi sibilano tra le fessure.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK | DESERTIC) },
	{ "Corridoio degli Idoli",
	  "Statue mutilate fiancheggiano il cammino con sguardo vuoto.\n"
	  "Offerte fossilizzate giacciono ai loro piedi.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK | DESERTIC) },
	{ "Atrio del Deserto",
	  "Una volta parzialmente crollata lascia filtrare sabbia dal mondo esterno.\n"
	  "L'eco dei tuoi passi sembra ripetere preghiere antiche.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK | DESERTIC) },
};
static const ProcRoomTemplate kTheme10Treasures[] = {
	{ "Camera delle Offerte",
	  "Vasi di terracotta e monete antiche giacciono sparsi.\n"
	  "Un oggetto piu' recente sembra fuori posto tra le reliquie.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK | DESERTIC) },
	{ "Santuario del Tesoro",
	  "Un altare basso sorregge oggetti avvolti in stoffa secca.\n"
	  "Simboli arcaici ne incorniciano il perimetro.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK | DESERTIC) },
};
static const ProcRoomTemplate kTheme10Traps[] = {
	{ "Sala delle Frecce",
	  "Fori sottili nelle pareti suggeriscono meccanismi ancora attivi.\n"
	  "Segni sul pavimento indicano dove non calpestare.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK | DESERTIC) },
	{ "Passaggio del Crollo",
	  "Macigni instabili bloccano parzialmente il cammino.\n"
	  "Polvere fresca indica movimenti recenti.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK | DESERTIC) },
};
static const ProcRoomTemplate kTheme10Boss = {
	"Sanctum del Profeta Morto",
	"Una sala vasta culmina in un trono di pietra deserta.\n"
	  "Una figura scheletrica incoronata di sabbia si alza lentamente.\n"
	  "L'aria sa di incenso rancido e tempo infinito.\n",
	SECT_INSIDE, static_cast<long>(INDOORS | DARK | DESERTIC)
};

static const ProcRoomTemplate kTheme11Entrances[] = {
	{ "Bordo del Baratro",
	  "Un passaggio stretto costeggia un vuoto senza fondo.\n"
	  "Venti freddi salgono dal basso strappando via polvere e sassolini.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
	{ "Crepaccio Verticale",
	  "Piattaforme naturali scendono a zig-zag verso l'oscurita'.\n"
	  "Ogni passo fa eco lontano nel vuoto.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
};
static const ProcRoomTemplate kTheme11Corridors[] = {
	{ "Passaggio Sospeso",
	  "Un sentiero stretto costeggia un abisso senza ringhiere.\n"
	  "Pietre cadono nel vuoto con ritardo spaventoso.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
	{ "Galleria del Vuoto",
	  "Il corridoio e' scavato nel fianco di una parete verticale.\n"
	  "L'oscurita' sotto di te sembra muoversi.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
	{ "Corridoio Echeggiante",
	  "Ogni suono rimbalza dal baratro rendendo difficile orientarsi.\n"
	  "Venti improvvisi spingono con forza inaspettata.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
	{ "Atrio del Precipizio",
	  "Una sporgenza naturale domina la vista su un abisso immenso.\n"
	  "Luci lontane sembrano danzare sul fondo invisibile.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
};
static const ProcRoomTemplate kTheme11Treasures[] = {
	{ "Riparo sul Vuoto",
	  "Una nicchia protetta contiene oggetti lasciati da esploratori precedenti.\n"
	  "Alcuni sembrano troppo vicini al bordo.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
	{ "Tesoro del Precipitato",
	  "Oggetti caduti da chissa' dove giacciono ammucchiati su una sporgenza.\n"
	  "Qualcosa luccica tra le pietre instabili.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
};
static const ProcRoomTemplate kTheme11Traps[] = {
	{ "Sala del Vuoto",
	  "Piastrelle cedevoli nascondono fessure verso l'abisso.\n"
	  "Un passo falso potrebbe essere l'ultimo.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
	{ "Passaggio del Vento",
	  "Raffiche improvvise spingono verso il baratro laterale.\n"
	  "Non c'e' nulla a cui aggrapparsi.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
};
static const ProcRoomTemplate kTheme11Boss = {
	"Abisso Coronato",
	"Una piattaforma naturale si affaccia su un vuoto infinito.\n"
	  "Al centro, una figura massiccia sembra ancorata alla roccia stessa.\n"
	  "Il vento urla mentre qualcosa si scolla dall'ombra.\n",
	SECT_INSIDE, static_cast<long>(INDOORS | DARK)
};

static const ProcRoomTemplate kTheme12Entrances[] = {
	{ "Arco dell'Ossuario",
	  "Cranii impilati formano un arco macabro verso l'interno.\n"
	  "Un silenzio pesante aleggia non appena oltrepassi.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK | UNDERGROUND) },
	{ "Discesa delle Ossa",
	  "Gradini di pietra sono sparsi di resti umani fossilizzati.\n"
	  "Ogni passo scricchiola in modo inquietante.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK | UNDERGROUND) },
};
static const ProcRoomTemplate kTheme12Corridors[] = {
	{ "Galleria delle Costole",
	  "Arcate ossee ricostruite sostengono il soffitto basso.\n"
	  "Polvere di osso copre ogni superficie.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK | UNDERGROUND) },
	{ "Passaggio dei Teschi",
	  "Muri di teschi ti osservano da entrambi i lati.\n"
	  "Alcune orbite sembrano seguirti mentre passi.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK | UNDERGROUND) },
	{ "Corridoio del Sacro Resto",
	  "Resti impilati con cura formano decorazioni disturbanti.\n"
	  "Simboli ossari segnano il cammino.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK | UNDERGROUND) },
	{ "Atrio del Re delle Ossa",
	  "Un trono di teschi domina la stanza come trofeo eterno.\n"
	  "L'aria e' fredda e immobile.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK | UNDERGROUND) },
};
static const ProcRoomTemplate kTheme12Treasures[] = {
	{ "Camera del Tesoro Osseo",
	  "Gioielli antichi giacciono tra resti umani dimenticati.\n"
	  "Qualcosa di recente brilla fuori contesto.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK | UNDERGROUND) },
	{ "Reliquiario Macabro",
	  "Una nicchia murata e' stata forzata di recente.\n"
	  "Resti avvolti in stoffa nascondono qualcosa di solido.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK | UNDERGROUND) },
};
static const ProcRoomTemplate kTheme12Traps[] = {
	{ "Sala delle Lame Ossee",
	  "Schegge d'osso affilate sporgono dal pavimento.\n"
	  "Segni di lacerazioni recenti segnano le pareti.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK | UNDERGROUND) },
	{ "Passaggio Instabile",
	  "Pile di ossa cedono sotto il peso del corpo.\n"
	  "Rumori di crollo risuonano da piu' direzioni.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK | UNDERGROUND) },
};
static const ProcRoomTemplate kTheme12Boss = {
	"Trono di Cranii",
	"Una sala circolare e' rivestita interamente di teschi umani.\n"
	  "Al centro, una figura imponente e' coronata di ossa.\n"
	  "Scheletri incompleti si agitano ai suoi piedi.\n",
	SECT_INSIDE, static_cast<long>(INDOORS | DARK | UNDERGROUND)
};

static const ProcRoomTemplate kTheme13Entrances[] = {
	{ "Porta del Culto",
	  "Due statue acefale fiancheggiano un arco di pietra consacrato.\n"
	  "Cera rancida e incenso stantio impregnano l'aria.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
	{ "Scalinata Consacrata",
	  "Gradini consumati da secoli di pellegrinaggi conducono al basso.\n"
	  "Simboli sacri coprono ogni superficie disponibile.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
};
static const ProcRoomTemplate kTheme13Corridors[] = {
	{ "Galleria delle Candele",
	  "Candele spente da tempo riempiono nicchie su entrambi i lati.\n"
	  "Gocce di cera formano stalattiti grottesche.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
	{ "Passaggio del Rito",
	  "Cerchi di sale e cenere segnano il pavimento a intervalli regolari.\n"
	  "Sussurri sembrano provenire da pareti spesse.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
	{ "Corridoio delle Icone",
	  "Icone scrostate osservano il cammino con occhi vuoti.\n"
	  "Qualcuna e' stata deliberatamente mutilata.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
	{ "Atrio del Culto Dimenticato",
	  "Una cupola bassa amplifica ogni respiro in un sussurro.\n"
	  "Banchi di pietra attendono fedeli che non arriveranno.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
};
static const ProcRoomTemplate kTheme13Treasures[] = {
	{ "Camera delle Offerte Sacre",
	  "Calici d'argento e pergamene sigillate giacciono su un altare.\n"
	  "Qualcosa e' stato lasciato indietro nel frettoloso abbandono.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
	{ "Reliquiario Profanato",
	  "Una cassa sacra e' stata forzata con violenza.\n"
	  "Resti di cera e stoffa profumata riempiono l'apertura.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
};
static const ProcRoomTemplate kTheme13Traps[] = {
	{ "Sala del Giudizio",
	  "Piastrelle con simboli di condanna coprono il pavimento.\n"
	  "Un brivido sacro ti percorre la schiena senza motivo.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
	{ "Passaggio Maledetto",
	  "Runa bruciate a fuoco segnano l'architrave dell'uscita.\n"
	  "Senti un peso inspiegabile sulle spalle.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
};
static const ProcRoomTemplate kTheme13Boss = {
	"Altare del Falso Dio",
	"Una navata sotterranea culmina in un altare macchiato di secoli.\n"
	  "Una figura incoronata di luce spenta si alza dal pulpito di pietra.\n"
	  "Cori invisibili sembrano intonare preghiere distorte.\n",
	SECT_INSIDE, static_cast<long>(INDOORS | DARK)
};

static const ProcRoomTemplate kTheme14Entrances[] = {
	{ "Sentiero di Pietra Viva",
	  "Alberi e cespugli sono stati trasformati in pietra grigia.\n"
	  "Il silenzio e' assoluto: nemmeno il vento osa muoversi.\n",
	  SECT_FOREST, static_cast<long>(INDOORS | DARK) },
	{ "Varco del Giardino Morto",
	  "Un arco di rovine ricopre l'ingresso di un giardino congelato nel tempo.\n"
	  "Statue vegetali sembrano osservarti senza occhi.\n",
	  SECT_FOREST, static_cast<long>(INDOORS | DARK) },
};
static const ProcRoomTemplate kTheme14Corridors[] = {
	{ "Viale delle Statue",
	  "Figure arboree pietrificate fiancheggiano un sentiero stretto.\n"
	  "Muschio secco ricopre ogni superficie immobile.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
	{ "Passaggio di Quarzo",
	  "Cristalli opachi sporgono dalle pareti come spine di pietra.\n"
	  "I tuoi passi echeggiano in modo innaturale.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
	{ "Corridoio delle Radici",
	  "Radici fossilizzate formano un soffitto a volta irregolare.\n"
	  "Polline di pietra ricopre il pavimento.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
	{ "Atrio del Giardiniere",
	  "Attrezzi arrugginiti giacciono accanto a vasi di pietra vuoti.\n"
	  "Una sensazione di attesa permane nell'aria.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
};
static const ProcRoomTemplate kTheme14Treasures[] = {
	{ "Serra Pietrificata",
	  "Piante rare congelate in minerali luccicano debolmente.\n"
	  "Qualcosa e' stato nascosto tra i vasi rotti.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
	{ "Nicchia del Seme",
	  "Una capsula di pietra e' stata scalfita da mani recenti.\n"
	  "Frammenti di gemme verdi giacciono sparsi.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
};
static const ProcRoomTemplate kTheme14Traps[] = {
	{ "Sala delle Statue Cadenti",
	  "Figure di pietra sono inclinate in equilibrio precario.\n"
	  "Un urto potrebbe innescare un crollo a catena.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
	{ "Passaggio Instabile",
	  "Piastrelle sottili scricchiolano sotto il peso del corpo.\n"
	  "Crepe sottili si propagano ad ogni passo.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
};
static const ProcRoomTemplate kTheme14Boss = {
	"Cuore del Giardino Pietrificato",
	"Un albero colossale di pietra domina la sala circolare.\n"
	  "Figure immobili sembrano pronte a scollarsi dalla roccia.\n"
	  "Una presenza antica pulsa al centro del tronco mineralizzato.\n",
	SECT_INSIDE, static_cast<long>(INDOORS | DARK)
};

static const ProcRoomTemplate kTheme15Entrances[] = {
	{ "Pozzo della Miniera",
	  "Una scala di ferro arrugginito scende in profondita'.\n"
	  "Odore di polvere e metallo stagnante riempie l'aria.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK | UNDERGROUND) },
	{ "Galleria di Servizio",
	  "Carrelli abbandonati bloccano parzialmente l'ingresso.\n"
	  "Piccoli cristalli brillano nelle pareti umide.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK | UNDERGROUND) },
};
static const ProcRoomTemplate kTheme15Corridors[] = {
	{ "Tunnel di Ardesia",
	  "Travi di legno marcio sorreggono un soffitto instabile.\n"
	  "Gocce nere cadono ritmicamente da qualche parte.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK | UNDERGROUND) },
	{ "Passaggio dei Picconi",
	  "Attrezzi da miniera giacciono abbandonati lungo il muro.\n"
	  "Segni di scavo recente contraddicono l'abbandono.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK | UNDERGROUND) },
	{ "Corridoio del Carbone",
	  "Pareti nere di fuliggine rendono difficile respirare.\n"
	  "Impronte di stivali si perdono in piu' direzioni.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK | UNDERGROUND) },
	{ "Atrio del Capo Miniera",
	  "Una lampada spenta pende da un gancio arrugginito.\n"
	  "Piani polverosi coprono un tavolo di legno.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK | UNDERGROUND) },
};
static const ProcRoomTemplate kTheme15Treasures[] = {
	{ "Camera del Vezzo",
	  "Filoni di minerali grezzi affiorano dalle pareti.\n"
	  "Resti di sacchi strappati indicano un furto interrotto.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK | UNDERGROUND) },
	{ "Cassaforte del Taglio",
	  "Una porta di ferro e' stata forzata con violenza.\n"
	  "Monete coperte di polvere giacciono in cumuli disordinati.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK | UNDERGROUND) },
};
static const ProcRoomTemplate kTheme15Traps[] = {
	{ "Sala del Crollo",
	  "Macigni sospesi minacciano di cadere da supporti marci.\n"
	  "Vibrazioni leggere fanno tremare la polvere.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK | UNDERGROUND) },
	{ "Passaggio dei Carrelli",
	  "Binari arrugginiti attraversano il pavimento a sorpresa.\n"
	  "Un carrello inclinato potrebbe rotolare da un lato.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK | UNDERGROUND) },
};
static const ProcRoomTemplate kTheme15Boss = {
	"Camera del Filone Nero",
	"Una vena di minerali oscuri pulsa debolmente al centro della caverna.\n"
	  "Resti di equipaggiamento da minatore circondano un trono di roccia.\n"
	  "Qualcosa si muove tra le ombre del filone.\n",
	SECT_INSIDE, static_cast<long>(INDOORS | DARK | UNDERGROUND)
};

static const ProcRoomTemplate kTheme16Entrances[] = {
	{ "Riva della Palude Nera",
	  "Acqua stagnante e fango nero coprono il terreno.\n"
	  "Vapori velenosi alzano spirali lente sopra la superficie.\n",
	  SECT_WATER_NOSWIM, static_cast<long>(INDOORS | DARK) },
	{ "Passaggio dei Cipressi",
	  "Alberi morti emergono dall'acqua come dita scheletriche.\n"
	  "Rane e insetti tacciono all'improvviso al tuo passaggio.\n",
	  SECT_WATER_NOSWIM, static_cast<long>(INDOORS | DARK) },
};
static const ProcRoomTemplate kTheme16Corridors[] = {
	{ "Sentiero di Radici",
	  "Radici semi sommerse formano un ponte instabile.\n"
	  "Bollicine di gas salgono dal fango sotto i piedi.\n",
	  SECT_WATER_NOSWIM, static_cast<long>(INDOORS | DARK) },
	{ "Galleria Umida",
	  "Muschio viscoso ricopre ogni superficie.\n"
	  "Luci fatue sembrano muoversi nell'acqua laterale.\n",
	  SECT_WATER_NOSWIM, static_cast<long>(INDOORS | DARK) },
	{ "Corridoio delle Canne",
	  "Canne alte nascondono passaggi laterali stretti.\n"
	  "Zanzare ronzano in nugoli fitii.\n",
	  SECT_WATER_NOSWIM, static_cast<long>(INDOORS | DARK) },
	{ "Atrio del Pantano",
	  "Un cumulo di detriti forma un isolotto artificiale.\n"
	  "Ossa di animali sporgono dal fango consolidato.\n",
	  SECT_WATER_NOSWIM, static_cast<long>(INDOORS | DARK) },
};
static const ProcRoomTemplate kTheme16Treasures[] = {
	{ "Isola del Tesoro",
	  "Una piattaforma di legno marcio sostiene casse parzialmente affondate.\n"
	  "Metallo arrugginito luccica tra le alghe.\n",
	  SECT_WATER_NOSWIM, static_cast<long>(INDOORS | DARK) },
	{ "Reliquia del Paludario",
	  "Un idolo corrosi giace semisommerso nel fango.\n"
	  "Offerte recenti contraddicono l'abbandono del luogo.\n",
	  SECT_WATER_NOSWIM, static_cast<long>(INDOORS | DARK) },
};
static const ProcRoomTemplate kTheme16Traps[] = {
	{ "Sala del Fango Profondo",
	  "Il pavimento cede improvvisamente in pozze nascoste.\n"
	  "L'acqua fredda arriva rapidamente alle ginocchia.\n",
	  SECT_WATER_NOSWIM, static_cast<long>(INDOORS | DARK) },
	{ "Passaggio dei Gas",
	  "Vapori verdognoli fuoriescono da crepe nel terreno.\n"
	  "La testa gira appena respiri piu' a fondo.\n",
	  SECT_WATER_NOSWIM, static_cast<long>(INDOORS | DARK) },
};
static const ProcRoomTemplate kTheme16Boss = {
	"Cuore della Palude Nera",
	"Un albero colossale morto domina una radura fangosa.\n"
	  "Luci biancastre danzano tra i rami nodosi.\n"
	  "Una figura emerge dal fango ai piedi del tronco.\n",
	SECT_WATER_NOSWIM, static_cast<long>(INDOORS | DARK)
};

static const ProcRoomTemplate kTheme17Entrances[] = {
	{ "Base della Torre Spezzata",
	  "Macigni caduti bloccano parzialmente l'ingresso crollato.\n"
	  "Venti alti ululano tra le crepe della struttura.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
	{ "Scalinata Fratturata",
	  "Gradini spezzati salgono verso l'oscurita' in curva stretta.\n"
	  "Polvere di pietra ricopre ogni gradino.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
};
static const ProcRoomTemplate kTheme17Corridors[] = {
	{ "Passaggio Instabile",
	  "Pareti inclinate rendono difficile mantenere l'equilibrio.\n"
	  "Sassi cadono occasionalmente dal soffitto.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
	{ "Galleria delle Finestre Rotte",
	  "Vetri spezzati scintillano sul pavimento coperto di detriti.\n"
	  "Il vento entra da aperture irregolari.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
	{ "Corridoio del Crollo",
	  "Travi spezzate formano un labirinto di legno e pietra.\n"
	  "Luce debole filtra da fessure imprevedibili.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
	{ "Atrio del Campanile",
	  "Corde arrugginite pendono da un arco senza campana.\n"
	  "Echo lontani sembrano provenire dai piani superiori.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
};
static const ProcRoomTemplate kTheme17Treasures[] = {
	{ "Camera del Tesoro Caduto",
	  "Forzieri schiacciati hanno sparso monete sotto le macerie.\n"
	  "Qualcosa di intatto brilla tra le pietre.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
	{ "Archivio Sepolto",
	  "Pergamene bagnate giacciono in armadi rovesciati.\n"
	  "Sigilli di cera si sciolgono ancora sul pavimento.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
};
static const ProcRoomTemplate kTheme17Traps[] = {
	{ "Sala del Vuoto",
	  "Una sezione del pavimento e' completamente crollata.\n"
	  "Solo travi sospette collegano i lati opposti.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
	{ "Passaggio del Ventaglio",
	  "Raffiche improvvise spingono verso il baratro laterale.\n"
	  "Non c'e' parapetto a proteggerti.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
};
static const ProcRoomTemplate kTheme17Boss = {
	"Cima Spezzata",
	"Il cielo aperto inquadra una piattaforma circolare esposta.\n"
	  "Fulmini lontani illuminano una figura immobile al centro.\n"
	  "La torre geme sotto ogni passo come se volesse crollare.\n",
	SECT_INSIDE, static_cast<long>(INDOORS | DARK)
};

static const ProcRoomTemplate kTheme18Entrances[] = {
	{ "Fauci del Canyon",
	  "Pareti rocciose si ergono a tagliare il cielo in una striscia.\n"
	  "Tuoni lontani rimbombano in modo continuo.\n",
	  SECT_MOUNTAIN, static_cast<long>(INDOORS | DARK) },
	{ "Discesa del Tuono",
	  "Un sentiero stretto serpeggia verso il basso tra pareti verticali.\n"
	  "Echi multipli confondono ogni suono.\n",
	  SECT_MOUNTAIN, static_cast<long>(INDOORS | DARK) },
};
static const ProcRoomTemplate kTheme18Corridors[] = {
	{ "Passaggio del Eco",
	  "Ogni parola rimbalta sette volte prima di dissolversi.\n"
	  "Pietre cadono occasionalmente da invisibili sporgenze.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
	{ "Galleria del Fulmine",
	  "Segni bruciati segnano pareti e pavimento.\n"
	  "Odore di ozono e' persistente.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
	{ "Corridoio del Vento",
	  "Raffiche laterali spingono con forza imprevedibile.\n"
	  "Sabbia e ghiaia graffiano la pelle esposta.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
	{ "Atrio del Tuono",
	  "Una conca naturale amplifica ogni rumore in un boato.\n"
	  "Luce intermittente filtra da crepe in alto.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
};
static const ProcRoomTemplate kTheme18Treasures[] = {
	{ "Cavità del Fulmine",
	  "Metallo fuso forma strane sculture sul pavimento.\n"
	  "Oggetti calcinati giacciono in un cumulo centrale.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
	{ "Tesoro del Tuono",
	  "Una cassa di pietra e' stata aperta da scariche ripetute.\n"
	  "Gemme opache resistono al calore passato.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
};
static const ProcRoomTemplate kTheme18Traps[] = {
	{ "Sala del Tuono",
	  "Piastrelle conduttive scintillano al minimo contatto.\n"
	  "Capelli si rizzano sulla nuca senza preavviso.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
	{ "Passaggio del Baratro",
	  "Il pavimento cede verso un vuoto laterale profondo.\n"
	  "Venti violenti rendono instabile ogni passo.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
};
static const ProcRoomTemplate kTheme18Boss = {
	"Altare del Tuono",
	"Colonne naturali formano un arco sopra un altare di roccia.\n"
	  "Scariche statiche danzano tra le pareti.\n"
	  "Una figura corona di fulmini attende al centro.\n",
	SECT_INSIDE, static_cast<long>(INDOORS | DARK)
};

static const ProcRoomTemplate kTheme19Entrances[] = {
	{ "Voliera Oscura",
	  "Piume nere coprono il terreno come una neve macabra.\n"
	  "Occhi lucenti osservano dall'alto tra le travi.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
	{ "Passaggio dei Nidi",
	  "Nidi enormi bloccano parzialmente il cammino.\n"
	  "Graffi profondi segnano le pareti di pietra.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
};
static const ProcRoomTemplate kTheme19Corridors[] = {
	{ "Galleria delle Piume",
	  "Piume e viscido ricoprono ogni superficie.\n"
	  "Crepitii di becchi e artigli risuonano in lontananza.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
	{ "Passaggio del Volo",
	  "Correnti d'aria improvvise spingono verso il basso.\n"
	  "Ombre rapide attraversano la luce debole.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
	{ "Corridoio dei Corvi",
	  "Simboli graffiati da artigli segnano il percorso.\n"
	  "Ossa piccole e lucenti giacciono ammucchiate.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
	{ "Atrio del Nido",
	  "Un cumulo di detriti forma un trono di ramoscelli.\n"
	  "Odore di carogne lontane aleggia nell'aria.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
};
static const ProcRoomTemplate kTheme19Treasures[] = {
	{ "Camera del Bottino",
	  "Oggetti luccicanti sono stati ammassati con cura predatoria.\n"
	  "Anelli e monete convivono con ossa rosicchiate.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
	{ "Tesoro del Corvo Re",
	  "Gemme colorate sono incastonate tra piume e fango.\n"
	  "Qualcosa e' stato nascosto sotto le ali secche.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
};
static const ProcRoomTemplate kTheme19Traps[] = {
	{ "Sala delle Unghie",
	  "Trappole a schegge di osso coprono il pavimento.\n"
	  "Segni di sangue recente indicano vittime precedenti.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
	{ "Passaggio del Picchiata",
	  "Correnti verticali spingono verso il basso senza preavviso.\n"
	  "Artigli scattano da nicchie nascoste.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
};
static const ProcRoomTemplate kTheme19Boss = {
	"Nido del Corvo Re",
	"Un nido colossale domina la cima della caverna.\n"
	  "Piume nere come pece ricoprono ogni superficie.\n"
	  "Una figura alata si stende dal trono di ossa e ramoscelli.\n",
	SECT_INSIDE, static_cast<long>(INDOORS | DARK)
};

static const ProcRoomTemplate kTheme20Entrances[] = {
	{ "Antica Porta di Pergamena",
	  "Rotoli sigillati bloccano parzialmente l'ingresso.\n"
	  "Polvere di inchiostro ricopre ogni superficie.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
	{ "Scalinata dei Tomi",
	  "Gradini consumati scendono verso un silenzio opprimente.\n"
	  "Candele spente segnano il cammino.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
};
static const ProcRoomTemplate kTheme20Corridors[] = {
	{ "Corridoio degli Scaffali",
	  "Scaffali crollati formano un labirinto di legno e carta.\n"
	  "Pagine svolazzano senza vento.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
	{ "Passaggio dei Tomi Proibiti",
	  "Libri incatenati pendono dalle pareti.\n"
	  "Simboli arcani pulsano debolmente sulle copertine.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
	{ "Galleria del Sapere",
	  "Mappe e pergamene coprono ogni centimetro disponibile.\n"
	  "Un odore di muffa e inchiostro e' opprimente.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
	{ "Atrio del Bibliotecario",
	  "Un banco di legno regge un registro aperto da secoli.\n"
	  "Pennini arrugginiti giacciono in fila.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
};
static const ProcRoomTemplate kTheme20Treasures[] = {
	{ "Camera del Tomo Perduto",
	  "Un libro massiccio e' incastonato in un piedistallo di pietra.\n"
	  "Runa luminose proteggono la copertina.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
	{ "Reliquiario di Carta",
	  "Documenti sigillati nascondono oggetti di valore.\n"
	  "Qualcuno ha forzato la serratura di recente.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
};
static const ProcRoomTemplate kTheme20Traps[] = {
	{ "Sala delle Pagine Taglienti",
	  "Fogli affilati volano in sciami al minimo movimento.\n"
	  "Incisioni sottili segnano chi e' passato prima.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
	{ "Passaggio del Silenzio",
	  "Runa di mutismo rendono impossibile chiamare aiuto.\n"
	  "Ombre si muovono tra gli scaffali alti.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
};
static const ProcRoomTemplate kTheme20Boss = {
	"Scriptorium Proibito",
	"Una scrivania colossale domina la sala circolare.\n"
	  "Tomi fluttuanti orbitano una figura incappucciata.\n"
	  "Parole sussurrate si scrivono da sole sulle pareti.\n",
	SECT_INSIDE, static_cast<long>(INDOORS | DARK)
};

static const ProcRoomTemplate kTheme21Entrances[] = {
	{ "Arco dell'Arena",
	  "Gradinate spezzate circondano un ingresso verso l'arena.\n"
	  "Sabbia e sangue secco coprono il pavimento.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK | HOT) },
	{ "Passaggio dei Gladiatori",
	  "Armature arrugginite pendono da ganci nel muro.\n"
	  "Odore di ferro e polvere riempie l'aria.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK | HOT) },
};
static const ProcRoomTemplate kTheme21Corridors[] = {
	{ "Corridoio delle Celle",
	  "Celle vuote mostrano segni di lotta disperata.\n"
	  "Catene spezzate giacciono sul pavimento.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK | HOT) },
	{ "Galleria del Sangue",
	  "Macchie scure segnano le pareti a intervalli regolari.\n"
	  "La sabbia sotto i piedi e' ancora calda.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK | HOT) },
	{ "Passaggio dei Tormenti",
	  "Strumenti da arena giacciono pronti all'uso.\n"
	  "Risate lontane echeggiano senza fonte.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK | HOT) },
	{ "Atrio del Mastro di Arena",
	  "Un trono di legno domina una piattaforma rialzata.\n"
	  "Bandiere strappate pendono senza vento.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK | HOT) },
};
static const ProcRoomTemplate kTheme21Treasures[] = {
	{ "Camera del Bottino",
	  "Armi e trofei sono ammassati dietro una grata forzata.\n"
	  "Monete d'oro mescolano sabbia e polvere.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK | HOT) },
	{ "Forziero del Campione",
	  "Una cassa massiccia e' incisa con nomi di vincitori.\n"
	  "Qualcosa brilla ancora al suo interno.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK | HOT) },
};
static const ProcRoomTemplate kTheme21Traps[] = {
	{ "Sala delle Lame",
	  "Lame emergono dal pavimento di sabbia a intervalli irregolari.\n"
	  "Segni di calci e pugni segnano le pareti.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK | HOT) },
	{ "Passaggio del Colosseo",
	  "Trappole a fossa sono nascoste sotto strati sottili di sabbia.\n"
	  "Urla lontane sembrano provenire dal centro.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK | HOT) },
};
static const ProcRoomTemplate kTheme21Boss = {
	"Arena del Campione",
	"Un cerchio di sabbia rossa domina la sala centrale.\n"
	  "Gradinate vuote osservano da ogni lato.\n"
	  "Un guerriero corona di ferro attende al centro.\n",
	SECT_INSIDE, static_cast<long>(INDOORS | DARK | HOT)
};

static const ProcRoomTemplate kTheme22Entrances[] = {
	{ "Cancello di Spine",
	  "Rovi secchi formano un arco naturale verso l'interno.\n"
	  "Punte affilate graffiano chi passa troppo vicino.\n",
	  SECT_FOREST, static_cast<long>(INDOORS | DARK) },
	{ "Sentiero Spinato",
	  "Un sentiero stretto e' fiancheggiato da siepi impenetrabili.\n"
	  "Petali marci coprono il terreno.\n",
	  SECT_FOREST, static_cast<long>(INDOORS | DARK) },
};
static const ProcRoomTemplate kTheme22Corridors[] = {
	{ "Viale delle Spine",
	  "Rami taglienti sporgono da ogni direzione.\n"
	  "Strappi recenti su stoffa indicano passaggi pericolosi.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
	{ "Galleria dei Rovi",
	  "Pareti vegetali si chiudono dietro di te con rumori secchi.\n"
	  "Odore di linfa amara e' intenso.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
	{ "Corridoio delle Rose Nere",
	  "Rose nere crescono tra le crepe della roccia.\n"
	  "Pollini scuri rendono difficile respirare.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
	{ "Atrio del Giardiniere Spinato",
	  "Attrezzi da potatura arrugginiti giacciono in disordine.\n"
	  "Guanti strappati indicano una fuga precipitosa.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
};
static const ProcRoomTemplate kTheme22Treasures[] = {
	{ "Serra di Spine",
	  "Vasi di vetro rotti contengono piante predatrici.\n"
	  "Gemme verdi sono incastonate tra i rami.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
	{ "Tesoro del Rovo",
	  "Oggetti luccicanti sono stati trascinati in un nido di spine.\n"
	  "Sangue secco indica chi ha tentato prima di te.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
};
static const ProcRoomTemplate kTheme22Traps[] = {
	{ "Sala delle Lame Vegetali",
	  "Rami si chiudono come a forbice al passaggio.\n"
	  "Segni di tagli profondi segnano le pareti.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
	{ "Passaggio del Rovo",
	  "Spine coprono il pavimento in pattern irregolari.\n"
	  "Ogni passo falso punge senza pieta'.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK) },
};
static const ProcRoomTemplate kTheme22Boss = {
	"Cuore del Labirinto di Spine",
	"Un albero di rovi domina la sala circolare.\n"
	  "Spine lunghe quanto spade si muovono al ritmo di un respiro lontano.\n"
	  "Una figura fiorita di punte attende al centro.\n",
	SECT_INSIDE, static_cast<long>(INDOORS | DARK)
};

static const ProcRoomTemplate kTheme23Entrances[] = {
	{ "Crepaccio Lunare",
	  "Luce argentea filtra da fessure in alto nonostante l'oscurita'.\n"
	  "Cristalli bianchi crescono sulle pareti.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK | BRIGHT) },
	{ "Varco delle Maree",
	  "Segni di marea secca segnano le pareti a mezz'altezza.\n"
	  "Una brezza salata aleggia senza fonte.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK | BRIGHT) },
};
static const ProcRoomTemplate kTheme23Corridors[] = {
	{ "Galleria delle Perle",
	  "Conchiglie fossilizzate incrustano ogni superficie.\n"
	  "Riflessi argentei danzano sul pavimento bagnato.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK | BRIGHT) },
	{ "Passaggio del Marea",
	  "Pozze salate rendono scivoloso ogni passo.\n"
	  "Luce lunare pulsa in sincronia con un ritmo lontano.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK | BRIGHT) },
	{ "Corridoio delle Conchiglie",
	  "Conchiglie aperte sembrano ascoltare i tuoi passi.\n"
	  "Sussurri marini echeggiano senza origine.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK | BRIGHT) },
	{ "Atrio della Luna",
	  "Un disco di pietra rappresenta la luna al centro.\n"
	  "Cristalli crescono verso l'alto come stalagmiti.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK | BRIGHT) },
};
static const ProcRoomTemplate kTheme23Treasures[] = {
	{ "Camera delle Perle",
	  "Perle opache giacciono in cumuli iridescenti.\n"
	  "Qualcosa e' stato rimosso di recente dal centro.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK | BRIGHT) },
	{ "Tesoro del Mare Secco",
	  "Coralli fossilizzati incastonano oggetti luccicanti.\n"
	  "Sabbia bianca nasconde monete antiche.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK | BRIGHT) },
};
static const ProcRoomTemplate kTheme23Traps[] = {
	{ "Sala delle Maree",
	  "Acqua salata irrompe periodicamente da fessure laterali.\n"
	  "La corrente spinge verso pareti cristalline taglienti.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK | BRIGHT) },
	{ "Passaggio del Riflesso",
	  "Riflessi ingannevoli nascondono buchi nel pavimento.\n"
	  "Cristalli affilati sporgono dove meno te lo aspetti.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK | BRIGHT) },
};
static const ProcRoomTemplate kTheme23Boss = {
	"Tempio della Luna Effimera",
	"Una cupola di cristallo proietta luce argentea su un altare.\n"
	  "Onde fantasma si infrangono silenziosamente sulle pareti.\n"
	  "Una figura corona di marea attende immobile.\n",
	SECT_INSIDE, static_cast<long>(INDOORS | DARK | BRIGHT)
};

static const ProcRoomTemplate kTheme24Entrances[] = {
	{ "Grotta di Sale",
	  "Cristalli bianchi ricoprono ogni superficie visibile.\n"
	  "L'aria e' cosi' secca che la pelle si tende.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK | DESERTIC) },
	{ "Passaggio Salato",
	  "Pareti di sale translucido brillano debolmente.\n"
	  "Scricchiolii sottili provengono dalle pareti.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK | DESERTIC) },
};
static const ProcRoomTemplate kTheme24Corridors[] = {
	{ "Galleria Bianca",
	  "Stalattiti di sale formano colonne irregolari.\n"
	  "Ogni passo solleva polvere bianca persistente.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK | DESERTIC) },
	{ "Corridoio del Sale",
	  "Cristalli affilati sporgono dal pavimento come lame.\n"
	  "Sete improvvisa ti assale senza preavviso.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK | DESERTIC) },
	{ "Passaggio del Deserto Bianco",
	  "Illusioni di dune danzano alla periferia della vista.\n"
	  "Calore secco rende difficile pensare lucidamente.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK | DESERTIC) },
	{ "Atrio del Re di Sale",
	  "Trono di cristalli bianchi domina la stanza.\n"
	  "Statue di sale rappresentano cortigiani immobile.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK | DESERTIC) },
};
static const ProcRoomTemplate kTheme24Treasures[] = {
	{ "Camera del Cristallo",
	  "Gemme di sale puro giacciono in nicchie protette.\n"
	  "Qualcuno ha scavato di recente una cavita laterale.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK | DESERTIC) },
	{ "Tesoro del Deserto",
	  "Oggetti avvolti in stoffa secca nascondono metallo lucido.\n"
	  "Monete antiche resistono alla corrosione del sale.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK | DESERTIC) },
};
static const ProcRoomTemplate kTheme24Traps[] = {
	{ "Sala del Sale Tagliente",
	  "Cristalli affilati cadono occasionalmente dal soffitto.\n"
	  "Polvere salina irrita occhi e gola.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK | DESERTIC) },
	{ "Passaggio della Sete",
	  "Calore opprimente aumenta ad ogni passo verso il centro.\n"
	  "Miraggi distorcono la percezione delle distanze.\n",
	  SECT_INSIDE, static_cast<long>(INDOORS | DARK | DESERTIC) },
};
static const ProcRoomTemplate kTheme24Boss = {
	"Trono del Re di Sale",
	"Una montagna di cristalli bianchi culmina in un trono scolpito.\n"
	  "Statue di sale si sciolgono e riformano lentamente.\n"
	  "Una figura corona di sale attende senza battere ciglio.\n",
	SECT_INSIDE, static_cast<long>(INDOORS | DARK | DESERTIC)
};
static const ProcThemeSet kThemeSets[] = {
	{ "Antro Selvaggio",
	  theme_room_list(kTheme0Entrances),
	  theme_room_list(kTheme0Corridors),
	  theme_room_list(kTheme0Treasures),
	  theme_room_list(kTheme0Traps),
	  &kTheme0Boss },
	{ "Cripta Sepolta",
	  theme_room_list(kTheme1Entrances),
	  theme_room_list(kTheme1Corridors),
	  theme_room_list(kTheme1Treasures),
	  theme_room_list(kTheme1Traps),
	  &kTheme1Boss },
	{ "Labirinto di Nebbia",
	  theme_room_list(kTheme2Entrances),
	  theme_room_list(kTheme2Corridors),
	  theme_room_list(kTheme2Treasures),
	  theme_room_list(kTheme2Traps),
	  &kTheme2Boss },
	{ "Rovine Sommerse",
	  theme_room_list(kTheme3Entrances),
	  theme_room_list(kTheme3Corridors),
	  theme_room_list(kTheme3Treasures),
	  theme_room_list(kTheme3Traps),
	  &kTheme3Boss },
	{ "Forgia Abbandonata",
	  theme_room_list(kTheme4Entrances),
	  theme_room_list(kTheme4Corridors),
	  theme_room_list(kTheme4Treasures),
	  theme_room_list(kTheme4Traps),
	  &kTheme4Boss },
	{ "Galleria di Ghiaccio",
	  theme_room_list(kTheme5Entrances),
	  theme_room_list(kTheme5Corridors),
	  theme_room_list(kTheme5Treasures),
	  theme_room_list(kTheme5Traps),
	  &kTheme5Boss },
	{ "Tana dei Ragni",
	  theme_room_list(kTheme6Entrances),
	  theme_room_list(kTheme6Corridors),
	  theme_room_list(kTheme6Treasures),
	  theme_room_list(kTheme6Traps),
	  &kTheme6Boss },
	{ "Catacombe Bruciate",
	  theme_room_list(kTheme7Entrances),
	  theme_room_list(kTheme7Corridors),
	  theme_room_list(kTheme7Treasures),
	  theme_room_list(kTheme7Traps),
	  &kTheme7Boss },
	{ "Labirinto di Cristallo",
	  theme_room_list(kTheme8Entrances),
	  theme_room_list(kTheme8Corridors),
	  theme_room_list(kTheme8Treasures),
	  theme_room_list(kTheme8Traps),
	  &kTheme8Boss },
	{ "Antro Fungino",
	  theme_room_list(kTheme9Entrances),
	  theme_room_list(kTheme9Corridors),
	  theme_room_list(kTheme9Treasures),
	  theme_room_list(kTheme9Traps),
	  &kTheme9Boss },
	{ "Tempio Rovinato",
	  theme_room_list(kTheme10Entrances),
	  theme_room_list(kTheme10Corridors),
	  theme_room_list(kTheme10Treasures),
	  theme_room_list(kTheme10Traps),
	  &kTheme10Boss },
	{ "Voragine Profonda",
	  theme_room_list(kTheme11Entrances),
	  theme_room_list(kTheme11Corridors),
	  theme_room_list(kTheme11Treasures),
	  theme_room_list(kTheme11Traps),
	  &kTheme11Boss },
	{ "Ossuario Antico",
	  theme_room_list(kTheme12Entrances),
	  theme_room_list(kTheme12Corridors),
	  theme_room_list(kTheme12Treasures),
	  theme_room_list(kTheme12Traps),
	  &kTheme12Boss },
	{ "Santuario Profanato",
	  theme_room_list(kTheme13Entrances),
	  theme_room_list(kTheme13Corridors),
	  theme_room_list(kTheme13Treasures),
	  theme_room_list(kTheme13Traps),
	  &kTheme13Boss },
	{ "Giardino Pietrificato",
	  theme_room_list(kTheme14Entrances),
	  theme_room_list(kTheme14Corridors),
	  theme_room_list(kTheme14Treasures),
	  theme_room_list(kTheme14Traps),
	  &kTheme14Boss },
	{ "Miniera Abbandonata",
	  theme_room_list(kTheme15Entrances),
	  theme_room_list(kTheme15Corridors),
	  theme_room_list(kTheme15Treasures),
	  theme_room_list(kTheme15Traps),
	  &kTheme15Boss },
	{ "Palude Nera",
	  theme_room_list(kTheme16Entrances),
	  theme_room_list(kTheme16Corridors),
	  theme_room_list(kTheme16Treasures),
	  theme_room_list(kTheme16Traps),
	  &kTheme16Boss },
	{ "Torre Spezzata",
	  theme_room_list(kTheme17Entrances),
	  theme_room_list(kTheme17Corridors),
	  theme_room_list(kTheme17Treasures),
	  theme_room_list(kTheme17Traps),
	  &kTheme17Boss },
	{ "Canyon del Tuono",
	  theme_room_list(kTheme18Entrances),
	  theme_room_list(kTheme18Corridors),
	  theme_room_list(kTheme18Treasures),
	  theme_room_list(kTheme18Traps),
	  &kTheme18Boss },
	{ "Nido del Corvo Re",
	  theme_room_list(kTheme19Entrances),
	  theme_room_list(kTheme19Corridors),
	  theme_room_list(kTheme19Treasures),
	  theme_room_list(kTheme19Traps),
	  &kTheme19Boss },
	{ "Biblioteca Sepolta",
	  theme_room_list(kTheme20Entrances),
	  theme_room_list(kTheme20Corridors),
	  theme_room_list(kTheme20Treasures),
	  theme_room_list(kTheme20Traps),
	  &kTheme20Boss },
	{ "Arena Dimenticata",
	  theme_room_list(kTheme21Entrances),
	  theme_room_list(kTheme21Corridors),
	  theme_room_list(kTheme21Treasures),
	  theme_room_list(kTheme21Traps),
	  &kTheme21Boss },
	{ "Labirinto di Spine",
	  theme_room_list(kTheme22Entrances),
	  theme_room_list(kTheme22Corridors),
	  theme_room_list(kTheme22Treasures),
	  theme_room_list(kTheme22Traps),
	  &kTheme22Boss },
	{ "Grotta Lunare",
	  theme_room_list(kTheme23Entrances),
	  theme_room_list(kTheme23Corridors),
	  theme_room_list(kTheme23Treasures),
	  theme_room_list(kTheme23Traps),
	  &kTheme23Boss },
	{ "Caverna di Sale",
	  theme_room_list(kTheme24Entrances),
	  theme_room_list(kTheme24Corridors),
	  theme_room_list(kTheme24Treasures),
	  theme_room_list(kTheme24Traps),
	  &kTheme24Boss },
};
