#!/usr/bin/env python3
"""Genera procarea_themes_ext.inc (temi 25-49) e aggiorna kThemeSets in procarea_themes.inc."""

from __future__ import annotations

from pathlib import Path
from textwrap import indent

from procarea_room_desc import MIN_DESC_LINES, ensure_min_lines

ROOT = Path(__file__).resolve().parents[1]
THEMES_INC = ROOT / "src/procarea_themes.inc"
EXT_INC = ROOT / "src/procarea_themes_ext.inc"
INCLUDE_MARKER = '#include "procarea_themes_ext.inc"'

# (label, sector, flags, entrances, corridors, treasures, traps, boss_name, boss_desc)
# flags: string passed to static_cast<long>(...)
THEMES_25_49: list[tuple] = [
    (
        "Moria delle Ombre",
        "SECT_INSIDE",
        "INDOORS | DARK | UNDERGROUND",
        [
            ("Porta di Durin",
             "Un arco di pietra nanica si apre su sale infinite.\n"
             "Rune sbiadite pulsano debolmente nel buio profondo.\n"),
            ("Vestibolo dei Martelli",
             "Statue di nani spezzate fiancheggiano un corridoio stretto.\n"
             "L'eco dei passi sembra moltiplicarsi all'infinito.\n"),
        ],
        [
            ("Galleria dei Pilastri",
             "Colonne massicce sostengono una volta lontana.\n"
             "Pozzi senza fondo si aprono ai lati del cammino.\n"),
            ("Passaggio del Fiume di Fuoco",
             "Calore lontano e odore di zolfo aleggiano nell'aria.\n"
             "Pietre nere brillano come brace spente.\n"),
            ("Sala delle Tombe Naniche",
             "Sarcofagi di pietra sono stati forzati e saccheggiati.\n"
             "Ossa naniche giacciono sparse sul pavimento.\n"),
            ("Atrio del Pozzo Senza Fine",
             "Un baratro circolare taglia la sala in due.\n"
             "Corde consumate pendono nel vuoto.\n"),
        ],
        [
            ("Camera del Mithril",
             "Filamenti d'argento raro ancora luccicano tra le rocce.\n"
             "Picconi abbandonati raccontano una febbre dimenticata.\n"),
            ("Forzieri dei Re Nani",
             "Casse sigillate con rune naniche attendono ancora.\n"
             "Qualcuno ha tentato di forzarle di recente.\n"),
        ],
        [
            ("Ponte Spezzato",
             "Lastre di pietra cedono sotto il peso al minimo passo falso.\n"
             "Il baratro sotto richiama con vento gelido.\n"),
            ("Sala delle Trappole Naniche",
             "Dardi di pietra sporgono dalle pareti a intervalli regolari.\n"
             "Piastrelle del pavimento sono leggermente inclinate.\n"),
        ],
        "Sala del Balrog Dimenticato",
        "Una caverna immensa si apre su un abisso fumante.\n"
        "Catene spezzate pendono da un ponte di pietra crollato.\n"
        "Qualcosa di antico e ardente attende nel cuore delle fiamme.\n",
    ),
    (
        "Fangorn Profondo",
        "SECT_FOREST",
        "INDOORS | DARK",
        [
            ("Varco tra gli Ent",
             "Due alberi colossali formano un arco naturale.\n"
             "La corteccia sembra osservarti con pazienza antica.\n"),
            ("Sentiero della Voce Verde",
             "Muschio spesso ammortizza ogni rumore.\n"
             "Sussurri di foglie raccontano storie millenarie.\n"),
        ],
        [
            ("Corridoio di Radici",
             "Radici spesse come serpenti bloccano passaggi laterali.\n"
             "Aria umida odora di terra e linfa.\n"),
            ("Galleria dei Muschi",
             "Muschio luminoso traccia sentieri nel buio.\n"
             "Funghi alti come uomini crescono lungo le pareti.\n"),
            ("Passaggio dei Sussurri",
             "Il vento tra i rami imita voci quasi comprensibili.\n"
             "Ogni passo fa scricchiolare legno vivo.\n"),
            ("Radura degli Ent Sonnecchianti",
             "Tronchi immobili sembrano dormire in cerchio.\n"
             "Solo gli occhi di resina tradiscono vigilanza.\n"),
        ],
        [
            ("Cavità della Corteccia d'Oro",
             "Schegge preziose si staccano da un tronco antico.\n"
             "Resina ambrata racchiude insetti dimenticati.\n"),
            ("Nicchia dei Semi Antichi",
             "Semi grandi come pugni pulsano di vita sopita.\n"
             "Qualcuno li ha raccolti con reverenza.\n"),
        ],
        [
            ("Trappola di Liane",
             "Liane si avvolgono al minimo contatto.\n"
             "La foresta reagisce alla tua presenza.\n"),
            ("Pozza di Linfa Acida",
             "Linfa corrosiva gocciola dal soffitto di foglie.\n"
             "Il terreno si ammorbidisce in modo minaccioso.\n"),
        ],
        "Cuore di Fangorn",
        "Una radura circolare accoglie un trono di rami intrecciati.\n"
        "Ent antichi emergono dalla penombra con passi lenti.\n"
        "La foresta stessa sembra respirare attorno a te.\n",
    ),
    (
        "Tomba dei Re Caduti",
        "SECT_INSIDE",
        "INDOORS | DARK",
        [
            ("Tumulo dei Re",
             "Un tumulo erboso nasconde una discesa verso l'oscurita'.\n"
             "Pietre runiche segnano confini dimenticati.\n"),
            ("Ingresso del Barrow",
             "Una porta di pietra socchiusa libera aria gelida.\n"
             "Gioielli rubati giacciono abbandonati sulla soglia.\n"),
        ],
        [
            ("Corridoio delle Corone Spezzate",
             "Corone arrugginite pendono da chiodi nel muro.\n"
             "Telai di ragnatela coprono stemmi sbiaditi.\n"),
            ("Galleria dei Cavalieri",
             "Armature vuote sorvegliano il passaggio.\n"
             "Spade incastrate nella pietra formano una ringhiera.\n"),
            ("Sala delle Candele Spente",
             "Candelabri antichi attendono un fuoco che non tornera'.\n"
             "Ceneri di ceri secolari coprono il pavimento.\n"),
            ("Atrio del Re Dormiente",
             "Un sarcofago di pietra domina la stanza.\n"
             "Rune di protezione sono state violate.\n"),
        ],
        [
            ("Camera del Tesoro Reale",
             "Oro e argento giacciono accanto a ossa reali.\n"
             "Una corona dimenticata brilla ancora debolmente.\n"),
            ("Reliquiario dei Caduti",
             "Reliquie avvolte in seta marcia attendono un pellegrino.\n"
             "Sigilli di cera sono stati rotti.\n"),
        ],
        [
            ("Passaggio Maledetto",
             "Simboli arsi a fuoco bruciano ancora sulla soglia.\n"
             "Un freddo innaturale ti avvolge.\n"),
            ("Sala delle Lame Emergenti",
             "Lame di pietra spuntano dal pavimento a intervalli.\n"
             "Segni di fuga disperata segnano le pareti.\n"),
        ],
        "Trono del Barrow-Wight",
        "Una sala circolare ospita un trono di ossa e corone.\n"
        "Luci verdastre danzano senza fiamma.\n"
        "Il re dei tumuli si rialza per difendere il suo regno.\n",
    ),
    (
        "Ponte del Baratro",
        "SECT_INSIDE",
        "INDOORS | DARK | UNDERGROUND",
        [
            ("Inizio del Ponte di Pietra",
             "Un ponte stretto attraversa un abisso senza fondo.\n"
             "Vento gelido sale dal baratro sottostante.\n"),
            ("Crepaccio Laterale",
             "Una fessura nella roccia rivela un secondo varco.\n"
             "Corde e pioli arrugginiti pendono nel vuoto.\n"),
        ],
        [
            ("Galleria del Baratro",
             "Il cammino costeggia un precipizio senza parapetto.\n"
             "Pietre cadute giacciono nel vuoto sotto di te.\n"),
            ("Passaggio Sospeso",
             "Travi di legno marcio collegano piattaforme rocciose.\n"
             "Ogni passo fa gemere la struttura.\n"),
            ("Corridoio dell'Eco Profonda",
             "Il tuo respiro rimbomba per secondi interminabili.\n"
             "Non vedi il fondo neppure con la torcia.\n"),
            ("Atrio del Vuoto",
             "Una piattaforma circolare domina il baratro.\n"
             "Catene spezzate oscillano nel vento.\n"),
        ],
        [
            ("Nicchia del Viandante Caduto",
             "Uno zaino logoro nasconde monete e una mappa.\n"
             "Ossa giacciono vicino al bordo del precipizio.\n"),
            ("Forziere del Guardiano",
             "Un forziere e' incatenato alla roccia.\n"
             "La chiave giace in un cranio vicino.\n"),
        ],
        [
            ("Ponte Instabile",
             "Lastre cedono una a una al tuo passaggio.\n"
             "Non c'e' tempo per esitare.\n"),
            ("Sala del Soffio Gelido",
             "Raffiche dal baratro spingono verso il vuoto.\n"
             "Il parapetto manca da tempo.\n"),
        ],
        "Cuore dell'Abisso",
        "Una piattaforma sospesa culmina sopra il baratro.\n"
        "Fulmini lontani illuminano una figura al centro.\n"
        "Il ponte dietro di te trema come se volesse crollare.\n",
    ),
    (
        "Rovine di Osgiliath",
        "SECT_INSIDE",
        "INDOORS | DARK",
        [
            ("Arco Spezzato",
             "Un arco monumentale e' crollato a meta'.\n"
             "Iscrizioni in lingua antica si leggono a fatica.\n"),
            ("Ponte sull'Anduin",
             "Piloni di pietra emergono dall'acqua sotterranea.\n"
             "Correnti lente portano detriti di una citta' morta.\n"),
        ],
        [
            ("Via dei Re",
             "Colonne spezzate fiancheggiano una strada deserta.\n"
             "Stendardi a brandelli oscillano senza vento.\n"),
            ("Piazza del Palantir",
             "Una sfera di cristallo frantumata giace al centro.\n"
             "Vetri taglienti coprono il pavimento.\n"),
            ("Corridoio del Senato",
             "Sedili di marmo sono stati rivoltati e bruciati.\n"
             "Cenere antica copre mosaici preziosi.\n"),
            ("Atrio della Torre Bianca",
             "Scale spezzate salgono verso il cielo aperto.\n"
             "Corvi nidificano tra le pietre cadute.\n"),
        ],
        [
            ("Tesoro del Palazzo",
             "Monete d'argento e oro giacciono tra le macerie.\n"
             "Qualcuno ha scavato di recente.\n"),
            ("Archivio delle Carte Reali",
             "Pergamene sigillate resistono ancora all'umidita'.\n"
             "Mappe di regni perduti attendono.\n"),
        ],
        [
            ("Sala del Crollo",
             "Il soffitto minaccia di cedere ad ogni rumore.\n"
             "Travi spezzate bloccano uscite laterali.\n"),
            ("Passaggio degli Orchetti",
             "Trappole rudimentali segnano un accampamento recente.\n"
             "Ossa e rifiuti coprono il pavimento.\n"),
        ],
        "Santuario del Re Pellegrino",
        "Una cappella mezza distrutta ospita un altare ancora intatto.\n"
        "Luce grigia filtra tra le rovine.\n"
        "Un custode spettrale veglia su cio' che resta della citta'.\n",
    ),
    (
        "Covo degli Uruk",
        "SECT_INSIDE",
        "INDOORS | DARK | UNDERGROUND",
        [
            ("Fossa di Warcraft",
             "Scale di legno scendono in un cantiere orco.\n"
             "Odore di ferro e carne putrefatta riempie l'aria.\n"),
            ("Varco delle Gole",
             "Pareti strette sono segnate da artigli e rune di guerra.\n"
             "Grugniti e martelli riecheggiano da sotto.\n"),
        ],
        [
            ("Galleria degli Scudi",
             "Scudi orchi impilati formano pareti improvvisate.\n"
             "Torce di pece fumano basso.\n"),
            ("Corridoio delle Lance",
             "Lance arrugginite punteggiano il soffitto come stalattiti.\n"
             "Ossa di prigionieri giacciono ammucchiate.\n"),
            ("Fucina di Guerra",
             "Incudini ancora calde e martelli abbandonati.\n"
             "Lame non finite pendono dai ganci.\n"),
            ("Atrio del Capitano",
             "Un trono di ossa e armi domina la sala.\n"
             "Stendardi neri segnano territorio conquistato.\n"),
        ],
        [
            ("Bottino di Guerra",
             "Armi e armature rubate sono ammucchiate senza ordine.\n"
             "Monete sparse brillano tra il ferro.\n"),
            ("Camera del Signore della Guerra",
             "Mappe graffiate sulle pareti mostrano invasioni pianificate.\n"
             "Un forziere pesante attende.\n"),
        ],
        [
            ("Fossa con Pale",
             "Pale coprono trappole nel pavimento di fango.\n"
             "Cadere significa impalarsi.\n"),
            ("Sala delle Frecce",
             "Dardi scattano dalle fessure al passaggio.\n"
             "Segni di sangue segnano il percorso.\n"),
        ],
        "Trono dell'Uruk-Hai",
        "Una sala vasta ospita un trono di ferro e ossa.\n"
        "Cori di guerra echeggiano da tunnel laterali.\n"
        "Il signore degli uruk si alza per schiacciare gli intrusi.\n",
    ),
    (
        "Galleria Elfica",
        "SECT_FOREST",
        "INDOORS | BRIGHT",
        [
            ("Varco di Lothlorien",
             "Alberi argentei incorniciano un sentiero luminoso.\n"
             "L'aria profuma di mallorn e luce antica.\n"),
            ("Ponte di Vine",
             "Viti intrecciate formano un ponte sospeso.\n"
             "Luce filtra come acqua tra le foglie.\n"),
        ],
        [
            ("Corridoio dei Cantori",
             "Melodie elfiche sembrano provenire dalle pareti stesse.\n"
             "Intagli dorati raccontano ere dimenticate.\n"),
            ("Galleria delle Stelle",
             "Cristalli incastonati imitano un cielo notturno.\n"
             "Ogni passo risuona come una nota.\n"),
            ("Passaggio dei Guardiani",
             "Statue elfiche seguono i tuoi movimenti con gli occhi.\n"
             "Frecce incastonate decorano gli archi.\n"),
            ("Atrio della Regina",
             "Colonne sottili sostengono una volta di luce.\n"
             "Fiori eterni sbocciano senza stagione.\n"),
        ],
        [
            ("Camera dei Doni",
             "Vestiti elfici e gioielli attendono ospiti degni.\n"
             "Pietre preziose pulsano di luce interna.\n"),
            ("Biblioteca dei Canti",
             "Pergamene di canzoni antiche sono conservate in seta.\n"
             "Un'arpa d'argento giace su un piedistallo.\n"),
        ],
        [
            ("Sala delle Illusioni",
             "Miraggi elfici nascondono buchi nel pavimento.\n"
             "La bellezza distrae dal pericolo.\n"),
            ("Passaggio delle Frecce Silenti",
             "Balestre nascoste puntano verso il centro.\n"
             "Solo gli elfi conoscono il percorso sicuro.\n"),
        ],
        "Santuario della Luce Argentea",
        "Una radura di luce pura accoglie un altare di cristallo.\n"
        "Alberi mallorn formano una cupola naturale.\n"
        "Un guardiano elfico emerge dalla luce stessa.\n",
    ),
    (
        "Ceneri di Gorgoroth",
        "SECT_MOUNTAIN",
        "INDOORS | DARK",
        [
            ("Sentiero delle Ceneri",
             "Cenere vulcanica copre ogni superficie.\n"
             "Il cielo e' una coltre rossastra di fumo.\n"),
            ("Varco di Mordor",
             "Rocce nere formano un arco minaccioso.\n"
             "L'aria brucia in gola ad ogni respiro.\n"),
        ],
        [
            ("Galleria del Fuoco Lontano",
             "Rive luminose pulsano oltre le pareti di basalto.\n"
             "Zolfo cristallizzato ricopre il pavimento.\n"),
            ("Corridoio degli Orchetti",
             "Impronte di orchi e ferri arrugginiti segnano il cammino.\n"
             "Urla lontane portano ordini incomprensibili.\n"),
            ("Passaggio del Fumo Denso",
             "Fumo acido riduce la visibilita' a pochi passi.\n"
             "Occhi rossi brillano nella penombra.\n"),
            ("Atrio delle Fornaci",
             "Fornaci rudimentali fumano ancora.\n"
             "Armi mal forgiate giacciono a raffreddare.\n"),
        ],
        [
            ("Tesoro del Signore Oscuro",
             "Armi e trofei sono ammucchiati con brutalita'.\n"
             "Anelli spezzati brillano tra le ceneri.\n"),
            ("Camera dei Sigilli",
             "Sigilli di ferro incisi con rune nere sigillano un forziere.\n"
             "Un forziere e' incatenato alla roccia.\n"),
        ],
        [
            ("Sala della Lava",
             "Crepe nel pavimento mostrano lava sotto la crosta.\n"
             "Calore insopportabile aumenta verso il centro.\n"),
            ("Passaggio dei Flagelli",
             "Fruste di ferro pendono dal soffitto.\n"
             "Cicatrici sulle pareti raccontano torture.\n"),
        ],
        "Trono delle Ceneri",
        "Un trono di roccia vulcanica domina una pianura di cenere.\n"
        "Il ruggito di un vulcano lontano scuote la terra.\n"
        "Un signore delle ombre attende incornato di fumo.\n",
    ),
    (
        "Fortezza di Helm",
        "SECT_INSIDE",
        "INDOORS | DARK",
        [
            ("Porta del Fosso",
             "Un ponte levatoio spezzato oscilla sopra un fosso profondo.\n"
             "Mura massicce si ergono fino a perdersi nel buio.\n"),
            ("Ingresso della Grotta",
             "Una grotta naturale si apre sotto le mura.\n"
             "Echi di battaglie antiche riempiono l'aria.\n"),
        ],
        [
            ("Corridoio dei Difensori",
             "Archi per corniere si aprono sulle pareti.\n"
             "Frecce rotte giacciono in mucchi.\n"),
            ("Galleria del Fosso Profondo",
             "Acqua stagnante copre il pavimento.\n"
             "Scheletri in armatura emergono dal fango.\n"),
            ("Passaggio della Sortita",
             "Una porta segreta conduce verso l'esterno.\n"
             "Segni di una fuga disperata segnano le pareti.\n"),
            ("Atrio del Re di Rohan",
             "Stendardi sbiaditi pendono da lance spezzate.\n"
             "Un trono di pietra e' vuoto ma imponente.\n"),
        ],
        [
            ("Armeria Segreta",
             "Armi di qualita' sono ancora impilate ordinatamente.\n"
             "Scudi con il simbolo del cavallo bianco.\n"),
            ("Camera del Corno di Helm",
             "Un corno antico giace su un piedistallo.\n"
             "La sua eco terrorizza ancora chi la sente.\n"),
        ],
        [
            ("Sala del Crollo",
             "Macigni bloccano passaggi; solo sentieri stretti restano.\n"
             "Polvere si solleva ad ogni passo.\n"),
            ("Passaggio dell'Assedio",
             "Dardi incastrati nella pietra segnano un assalto.\n"
             "Olio bollente ha bruciato il pavimento.\n"),
        ],
        "Sala dell'Ultima Resistenza",
        "Una grande sala ospita resti di un'ultima difesa eroica.\n"
        "Corpi fossilizzati tengono ancora le armi.\n"
        "Un campione spettrale sorveglia il varco finale.\n",
    ),
    (
        "Sentieri dei Morti",
        "SECT_INSIDE",
        "INDOORS | DARK",
        [
            ("Porta dei Giuramenti",
             "Un arco di pietra reca iscrizioni di giuramenti infranti.\n"
             "Aria gelida esce dal tunnel sotterraneo.\n"),
            ("Ingresso della Montagna",
             "Pilastri scolpiti raffigurano re spettrali.\n"
             "Nessun suono vivo proviene dall'interno.\n"),
        ],
        [
            ("Galleria dei Re Spettrali",
             "Ombre di corone passano attraverso le pareti.\n"
             "Passi senza corpo echeggiano accanto ai tuoi.\n"),
            ("Corridoio dei Giuramenti",
             "Voci sussurrano promesse mai mantenute.\n"
             "Il freddo aumenta ad ogni passo.\n"),
            ("Passaggio delle Lame Spettrali",
             "Spade fantasma sfiorano la tua carne senza tagliare.\n"
             "Terrore antico ti paralizza un istante.\n"),
            ("Atrio del Re Oathbreaker",
             "Un trono vuoto domina una sala di ombre.\n"
             "Stendardi polverosi cadono a pezzi.\n"),
        ],
        [
            ("Reliquie dei Caduti",
             "Armi spettrali giacciono incorrotte.\n"
             "Medaglie di onore dimenticate brillano debolmente.\n"),
            ("Camera dei Sigilli Spezzati",
             "Sigilli di cera nera sono stati infranti.\n"
             "Pergamene di giuramento giacciono sparse.\n"),
        ],
        [
            ("Sala del Terrore",
             "Visioni dei tuoi peccati appaiono sulle pareti.\n"
             "La mente vacilla per un istante.\n"),
            ("Passaggio del Vuoto",
             "Il pavimento sembra non esistere sotto i piedi.\n"
             "Solo la volonta' ti tiene in piedi.\n"),
        ],
        "Trono dei Re Spettrali",
        "Una sala infinita ospita ombre di re millenari.\n"
        "Corone spettrali brillano nella penombra.\n"
        "Il sovrano dei morti pronuncia il tuo destino.\n",
    ),
    (
        "Fosse di Isengard",
        "SECT_INSIDE",
        "INDOORS | DARK | UNDERGROUND",
        [
            ("Pozzo di Orthanc",
             "Un pozzo verticale scende verso officine sotterranee.\n"
             "Odore di carbone e metallo riempie l'aria.\n"),
            ("Tunnel degli Orchi",
             "Pareti squadrate con precisione innaturale.\n"
             "Macchine di legno e ferro bloccano passaggi.\n"),
        ],
        [
            ("Galleria delle Macchine",
             "Ingranaggi immensi sono fermi ma minacciosi.\n"
             "Catene spesse pendono dal soffitto.\n"),
            ("Corridoio del Carbone",
             "Carbone nero copre ogni superficie.\n"
             "Polvere nera irrita occhi e gola.\n"),
            ("Passaggio dei Fabbri",
             "Incudini e martelli giacciono abbandonati.\n"
             "Lame di ferro sporgono dal pavimento.\n"),
            ("Atrio del Mago Traditore",
             "Libri bruciati e strumenti spezzati coprono il pavimento.\n"
             "Una sfera di cristallo e' incrinata.\n"),
        ],
        [
            ("Armeria di Isengard",
             "Armature uniformi sono impilate per migliaia.\n"
             "Elmi con la S bianca dominano la sala.\n"),
            ("Camera dei Piani",
             "Mappe di invasioni coprono ogni parete.\n"
             "Un forziere contiene ordini sigillati.\n"),
        ],
        [
            ("Sala delle Ruote",
             "Ruote dentate si muovono al minimo tocco.\n"
             "Evitare i meccanismi e' questione di vita.\n"),
            ("Passaggio del Fuoco di Forgia",
             "Getti di vapore bollente fuoriescono dalle pareti.\n"
             "Il metallo rovente brucia ancora.\n"),
        ],
        "Cuore della Macchina",
        "Una sala meccanica culmina in un nucleo di ingranaggi.\n"
        "Vapore e fumo oscurano la vista.\n"
        "Il signore delle fosse emerge tra le macchine.\n",
    ),
    (
        "Tunnels di Ungol",
        "SECT_INSIDE",
        "INDOORS | DARK",
        [
            ("Fessura della Ragnatela",
             "Seta densa blocca il passaggio come nebbia solida.\n"
             "Insetti impalati pendono ovunque.\n"),
            ("Bocca del Labirinto",
             "Tunnel intrecciati perdono ogni orientamento.\n"
             "Ossa avvolte in seta ricoprono il pavimento.\n"),
        ],
        [
            ("Galleria delle Paralisi",
             "Ragnatele appiccicose bloccano braccia e gambe.\n"
             "Qualcosa striscia sopra la tua testa.\n"),
            ("Corridoio dei Sacchi",
             "Sacchi di seta contengono prede ancora vive.\n"
             "Movimenti deboli tradiscono vittime recenti.\n"),
            ("Passaggio delle Zanne",
             "Zanne di ragni giganti decorano le pareti.\n"
             "Liquido digestivo gocciola dal soffitto.\n"),
            ("Atrio della Regina",
             "Un trono di ossa e seta domina la sala.\n"
             "Ragni di ogni dimensione attendono ordini.\n"),
        ],
        [
            ("Tesoro delle Vittime",
             "Equipaggiamento degli avventurieri e' ammucchiato con cura.\n"
             "Anelli e monete brillano tra la seta.\n"),
            ("Camera delle Uova",
             "Uova di ragno pulsano di vita imminente.\n"
             "Larve strisciano in cumuli viscidi.\n"),
        ],
        [
            ("Trappola della Ragnatela",
             "Filamenti si tendono al passaggio tagliando la fuga.\n"
             "Veleno paralizzante impregna ogni filo.\n"),
            ("Pozza di Veleno",
             "Liquido verde corrode armatura e carne.\n"
             "Vapori acidi rendono difficile respirare.\n"),
        ],
        "Nido della Regina Ragno",
        "Una caverna immensa e' tappezzata di seta nera.\n"
        "Occhi multipli brillano nel buio sopra di te.\n"
        "La regina dei ragni scende dal soffitto.\n",
    ),
    (
        "Bosco di Mirkwood",
        "SECT_FOREST",
        "INDOORS | DARK",
        [
            ("Sentiero Oscuro",
             "Alberi contorti chiudono il cielo a una fessura.\n"
             "Nessun raggio di sole raggiunge il suolo.\n"),
            ("Varco dei Funghi",
             "Funghi luminosi tingono il bosco di verde pallido.\n"
             "Spore dense rendono l'aria pesante.\n"),
        ],
        [
            ("Corridoio degli Alberi",
             "Rami bassi costringono a chinare la testa.\n"
             "Ragni e insetti osservano dal fogliame.\n"),
            ("Galleria dei Sussurri",
             "Voci sembrano provenire da tronchi cavi.\n"
             "Il bosco parla in lingue dimenticate.\n"),
            ("Passaggio del Fiume di Resina",
             "Resina appiccicosa trattiene ogni passo.\n"
             "Insetti giganti nuotano nella linfa.\n"),
            ("Radura degli Elfi Prigionieri",
             "Gabbie di legno sono vuote ma recenti.\n"
             "Segni di lotta segnano il terreno.\n"),
        ],
        [
            ("Tesoro degli Elfi Catturati",
             "Vesti e armi elfiche giacciono abbandonate.\n"
             "Fiale di luce ancora brillano debolmente.\n"),
            ("Camera del Re dei Funghi",
             "Funghi giganti formano un trono naturale.\n"
             "Spore dorate cadono come neve.\n"),
        ],
        [
            ("Trappola di Resina",
             "Resina si indurisce rapidamente attorno alle gambe.\n"
             "Fuggire diventa quasi impossibile.\n"),
            ("Sala delle Spine",
             "Spine di legno spuntano dal suolo al passaggio.\n"
             "Sangue secco segna il percorso.\n"),
        ],
        "Trono del Bosco Nero",
        "Un albero colossale ospita un trono di rami neri.\n"
        "Corvi e pipistrelli volteggiano senza riposo.\n"
        "Il signore del bosco emerge dall'ombra.\n",
    ),
    (
        "Cantine di Bree",
        "SECT_INSIDE",
        "INDOORS | DARK",
        [
            ("Botola della Locanda",
             "Una botola nel pavimento rivela cantine umide.\n"
             "Odore di birra e muffa si mescolano.\n"),
            ("Passaggio Segreto",
             "Dietro un barile vuoto si apre un corridoio nascosto.\n"
             "Segni di passaggio recente segnano il fango.\n"),
        ],
        [
            ("Corridoio dei Barili",
             "Barili impilati formano pareti instabili.\n"
             "Liquido scuro gocciola tra le assi.\n"),
            ("Galleria dei Contrabbandieri",
             "Casse sigillate portano marchi di citta' lontane.\n"
             "Mappe di rotte segrete coprono un tavolo.\n"),
            ("Passaggio dei Ratti",
             "Ratti grossi come gatti osservano dall'ombra.\n"
             "Ossa rosicchiate coprono il pavimento.\n"),
            ("Atrio del Capitano della Guardia",
             "Armi confiscate sono ammucchiate in un angolo.\n"
             "Una scrivania coperta di rapporti attende.\n"),
        ],
        [
            ("Cassa del Contrabbando",
             "Spezie rare e gioielli sono nascosti sotto fieno.\n"
             "Monete di ogni regno riempiono sacchi.\n"),
            ("Forziere della Locanda",
             "Oro della locanda e' custodito dietro serrature multiple.\n"
             "Chiavi pendono da un chiodo vicino.\n"),
        ],
        [
            ("Trappola del Barile",
             "Un barile cede rivelando punte di ferro sotto.\n"
             "Il contrabbandiere conosceva questo trucco.\n"),
            ("Sala della Birra Avvelenata",
             "Vapori alcolici nascondono gas soporiferi.\n"
             "La testa gira dopo pochi respiri.\n"),
        ],
        "Cantina del Padrone Oscuro",
        "La cantina piu' profonda ospita un tavolo rotondo.\n"
        "Candele nere illuminano volti nascosti.\n"
        "Il padrone dei bassifondi si alza per accoglierti.\n",
    ),
    (
        "Tesoro di Smaug",
        "SECT_INSIDE",
        "INDOORS | DARK",
        [
            ("Fessura della Montagna",
             "Una fessura nella roccia rivela luccichio interno.\n"
             "Calore antico ancora aleggia nell'aria.\n"),
            ("Varco del Drago",
             "Artigli profondi segnano le pareti all'ingresso.\n"
             "Ossa calcinate giacciono ammucchiate.\n"),
        ],
        [
            ("Galleria dell'Oro",
             "Monete e gemme coprono il pavimento come ghiaia.\n"
             "Ogni passo affonda nel tesoro.\n"),
            ("Corridoio delle Ossa",
             "Crani di nani e uomini formano cumuli macabri.\n"
             "Armature fuse dal fuoco del drago giacciono sparse.\n"),
            ("Passaggio delle Coppe",
             "Coppe d'oro e argento sono ammucchiate a torri.\n"
             "Pietre preziose cadono dai mucchi instabili.\n"),
            ("Atrio del Letto del Drago",
             "Un'impronta immensa segna dove il drago dorme.\n"
             "Cenere e braci spente coprono tutto.\n"),
        ],
        [
            ("Cuore del Tesoro",
             "La pila piu' alta di oro e gemme domina la sala.\n"
             "L'Arkenstone stessa sembra brillare da qualche parte.\n"),
            ("Camera delle Armi Fuse",
             "Armi naniche fuse in lingotti d'oro impossibili.\n"
             "Tributi di regni interi giacciono qui.\n"),
        ],
        [
            ("Trappola del Tesoro Instabile",
             "Mucchi di monete cedono rivelando fosse sotto.\n"
             "Cadere significa scomparire nell'oro.\n"),
            ("Sala del Fuoco Residuo",
             "Braci ancora calde bruciano sotto il tesoro.\n"
             "Il calore aumenta verso il centro.\n"),
        ],
        "Sala del Drago Caduto",
        "Una caverna immensa ospita resti di un drago gigante.\n"
        "Oro copre ogni centimetro come neve dorata.\n"
        "Qualcosa ancora veglia sul tesoro con occhi avidi.\n",
    ),
    (
        "Sale di Erebor",
        "SECT_INSIDE",
        "INDOORS | DARK | UNDERGROUND",
        [
            ("Porta di Erebor",
             "Una porta nanica massiccia reca rune di benvenuto.\n"
             "Torce di cristallo illuminano il vestibolo.\n"),
            ("Scalinata dei Re",
             "Gradini scolpiti nella roccia salgono maestosi.\n"
             "Intagli narrano la storia dei nani.\n"),
        ],
        [
            ("Galleria dei Sette Signori",
             "Statue di sette re nanici fiancheggiano il cammino.\n"
             "Mazze e asce sono incastonate nelle nicchie.\n"),
            ("Corridoio del Mithril",
             "Filamenti d'argento corrono lungo le pareti.\n"
             "Picconi abbandonati segnano un'era d'oro.\n"),
            ("Passaggio delle Fornaci",
             "Le fornaci naniche sono ancora tiepide.\n"
             "Lingotti d'oro e argento attendono la forgia.\n"),
            ("Atrio del Trono di Pietra",
             "Un trono scolpito nella roccia domina la sala.\n"
             "Gemme incastonate brillano come stelle.\n"),
        ],
        [
            ("Tesoro dei Re Nani",
             "Forzieri nanici traboccano di gemme e metallo.\n"
             "Corone dimenticate brillano sulla pietra.\n"),
            ("Armeria Reale",
             "Armature naniche di qualita' leggendaria giacciono impilate.\n"
             "Asce e martelli runici attendono un guerriero.\n"),
        ],
        [
            ("Sala del Ponte Spezzato",
             "Un ponte nanico e' crollato nel baratro.\n"
             "Solo una trave instabile resta.\n"),
            ("Passaggio delle Lame Rotanti",
             "Lame di pietra si muovono con meccanismi nanici.\n"
             "Solo i nani conoscono ancora il ritmo.\n"),
        ],
        "Trono Sotto la Montagna",
        "La sala del trono di Erebor risplende di gemme.\n"
        "Corone naniche pendono dal soffitto scolpito.\n"
        "L'ultimo guardiano delle sale si alza ad accoglierti.\n",
    ),
    (
        "Rova Nera Profonda",
        "SECT_INSIDE",
        "INDOORS | DARK",
        [
            ("Crepaccio della Rova",
             "Un crepaccio nella roccia scende verso l'oscuro.\n"
             "Funghi luminosi tingono tutto di viola.\n"),
            ("Ponte di Ossidiana",
             "Un ponte sottile di ossidiana attraversa un abisso.\n"
             "Stalattiti come lance puntano verso il basso.\n"),
        ],
        [
            ("Galleria dei Drow",
             "Intagli drow raffigurano divinita' aracnide.\n"
             "Ragnatele decorate con fili d'argento ricoprono le pareti.\n"),
            ("Corridoio dei Matroni",
             "Troni vuoti di matroni drow segnano il cammino.\n"
             "Veleno secco macchia le pareti.\n"),
            ("Passaggio delle Balestre",
             "Balestre nascoste puntano verso il centro.\n"
             "Dardi avvelenati giacciono in mucchi.\n"),
            ("Atrio del Tempio di Lolth",
             "Un altare di ossidiana domina la sala.\n"
             "Ragni sacri tessono intorno all'altare.\n"),
        ],
        [
            ("Tesoro della Matrona",
             "Gioielli drow e armi di ossidiana brillano.\n"
             "Schizzi di veleno proteggono i forzieri.\n"),
            ("Camera degli Incantesimi",
             "Pergamene drow recanti incantesimi proibiti giacciono impilate.\n"
             "Fiale di oscurita' pulsano debolmente.\n"),
        ],
        [
            ("Trappola del Veleno",
             "Gas viola fuoriesce da fessure nel pavimento.\n"
             "La pelle brucia al contatto.\n"),
            ("Sala delle Ragnatele",
             "Ragnatele bloccano ogni via di fuga.\n"
             "Ragni avvelenati scendono dal soffitto.\n"),
        ],
        "Santuario della Regina Ragno",
        "Un tempio sotterraneo ospita un trono di ragnatele.\n"
        "Ragni giganti servono una matrona drow.\n"
        "La regina della rova nera si alza dal trono.\n",
    ),
    (
        "Fogne di Waterdeep",
        "SECT_WATER_NOSWIM",
        "INDOORS | DARK",
        [
            ("Grata delle Fogne",
             "Una grata arrugginita conduce a tunnel umidi.\n"
             "Odore di rifiuti e muffa e' soffocante.\n"),
            ("Pozzo di Scarico",
             "Acqua nera scorre veloce sotto una passerella.\n"
             "Ratti grossi come gatti osservano dall'ombra.\n"),
        ],
        [
            ("Galleria dei Rifiuti",
             "Rifiuti di tutta la citta' si accumulano qui.\n"
             "Gas infiammabili aleggiano bassi.\n"),
            ("Corridoio delle Piastrelle",
             "Piastrelle scivolose coprono il pavimento.\n"
             "Acqua sporca arriva alle caviglie.\n"),
            ("Passaggio dei Contrabbandieri",
             "Casse di merci rubate bloccano il cammino.\n"
             "Segni della Gilda dei Ladri sono ovunque.\n"),
            ("Atrio del Capo Ladro",
             "Un tavolo coperto di mappe della citta'.\n"
             "Monete contate in mucchi ordinati.\n"),
        ],
        [
            ("Bottino della Gilda",
             "Tesori rubati da tutta Waterdeep.\n"
             "Gioielli e artefatti in casse sigillate.\n"),
            ("Camera dei Contratti",
             "Contratti su pergamena con sangue come inchiostro.\n"
             "Sigilli della gilda su ogni documento.\n"),
        ],
        [
            ("Trappola della Grata",
             "Una grata cede rivelando acque profonde sotto.\n"
             "Correnti trascinano verso l'ignoto.\n"),
            ("Sala del Gas",
             "Gas infiammabile si accumula in una nicchia.\n"
             "Una scintilla basterebbe.\n"),
        ],
        "Covo del Principe dei Ladri",
        "Una sala asciutta sorprende in mezzo alle fogne.\n"
        "Tappeti e candele contrastano con l'orrido esterno.\n"
        "Il principe dei ladri attende con un sorriso.\n",
    ),
    (
        "Rovine di Neverwinter",
        "SECT_INSIDE",
        "INDOORS | DARK",
        [
            ("Porta Ghiacciata",
             "Ghiaccio copre un arco di pietra crollato.\n"
             "Aria gelida esce dalle rovine interne.\n"),
            ("Varco della Citta' Morta",
             "Rovine di una grande citta' si estendono nel buio.\n"
             "Neve eterna copre ogni superficie.\n"),
        ],
        [
            ("Galleria del Gelo",
             "Stalattiti di ghiaccio pendono minacciosamente.\n"
             "Il pavimento e' una lastra scivolosa.\n"),
            ("Corridoio dei Cavalieri di Ghiaccio",
             "Statue di ghiaccio raffigurano cavalieri.\n"
             "Alcune sembrano muoversi nel buio.\n"),
            ("Passaggio della Tempesta",
             "Vento gelido entra da fessure nel muro.\n"
             "Neve si accumula in cumuli alti.\n"),
            ("Atrio del Palazzo Congelato",
             "Un trono di ghiaccio domina una sala abbandonata.\n"
             "Stendardi congelati cadono a pezzi.\n"),
        ],
        [
            ("Tesoro del Re di Ghiaccio",
             "Gemme congelate brillano come stelle.\n"
             "Oro incastonato nel ghiaccio eterno.\n"),
            ("Camera delle Armi Gelide",
             "Armi di ghiaccio magico ancora intatte.\n"
             "Un freddo che non si scioglie mai.\n"),
        ],
        [
            ("Sala del Ghiaccio Sottile",
             "Il pavimento di ghiaccio cede al peso.\n"
             "Acqua gelida sotto minaccia di affogare.\n"),
            ("Passaggio delle Valanghe",
             "Neve cade dal soffitto senza preavviso.\n"
             "Essere sepolti e' un rischio costante.\n"),
        ],
        "Trono del Re di Ghiaccio",
        "Un palazzo congelato culmina in un trono di cristallo.\n"
        "Tempesta eterna ulula fuori dalle mura.\n"
        "Il re di ghiaccio si alza ad accoglierti.\n",
    ),
    (
        "Cripta di Baldur",
        "SECT_INSIDE",
        "INDOORS | DARK",
        [
            ("Porta della Cripta",
             "Un arco gotico conduce a tombe sotterranee.\n"
             "Candele spente segnano un abbandono recente.\n"),
            ("Scalinata dei Morti",
             "Gradini consumati scendono tra lapidi inclinate.\n"
             "Nomi cancellati dal tempo sono incisi sulle pietre.\n"),
        ],
        [
            ("Galleria delle Famiglie",
             "Tombe di famiglie nobili fiancheggiano il cammino.\n"
             "Stemmi sbiaditi decorano ogni sarcofago.\n"),
            ("Corridoio dei Chierici",
             "Simboli sacri sono stati profanati.\n"
             "Libri di preghiera bruciati giacciono sparsi.\n"),
            ("Passaggio degli Scheletri",
             "Scheletri si staccano dalle nicchie al tuo passaggio.\n"
             "Ossa sbattono in un ritmo sinistro.\n"),
            ("Atrio del Vampiro",
             "Un sarcofago di marmo nero domina la sala.\n"
             "Rose secche e sangue secco decorano l'altare.\n"),
        ],
        [
            ("Tesoro delle Casate",
             "Gioielli funerari giacciono accanto alle tombe.\n"
             "Oro e argento delle famiglie nobili.\n"),
            ("Reliquiario Profanato",
             "Reliquie sacre rubate e contaminate.\n"
             "Simboli oscuri sono stati aggiunti.\n"),
        ],
        [
            ("Sala delle Lame",
             "Lame emergono dal pavimento a intervalli.\n"
             "Un meccanismo attivato dal peso.\n"),
            ("Passaggio del Gas Necrotico",
             "Gas verdognolo fuoriesce dalle tombe aperte.\n"
             "Carne marcia e' il prezzo di un respiro lungo.\n"),
        ],
        "Cripta del Signore Vampiro",
        "La cripta piu' profonda ospita un trono di bare.\n"
        "Pipistrelli volteggiano senza fine.\n"
        "Il signore vampiro di Baldur si alza ad accoglierti.\n",
    ),
    (
        "Fessura Planare",
        "SECT_AIR",
        "INDOORS | DARK",
        [
            ("Varco Instabile",
             "La realta' sembra piegarsi attorno all'ingresso.\n"
             "Colori impossibili danzano ai bordi della vista.\n"),
            ("Portale Fratturato",
             "Un portale scintillante mostra mondi paralleli.\n"
             "Venti da mille direzioni ti spingono.\n"
             "Fratture nel vetro planare pulsano come vene di luce.\n"
             "Per un istante vedi te stesso camminare in un mondo specchiato.\n"),
        ],
        [
            ("Galleria dei Frammenti",
             "Pezzi di citta' diverse fluttuano nel vuoto.\n"
             "Gravita' e' una suggestione qui.\n"),
            ("Corridoio delle Geometrie",
             "Angoli impossibili formano passaggi.\n"
             "Camminare in linea retta e' un'illusione.\n"),
            ("Passaggio dei Sussurri Planari",
             "Voci di mille mondi parlano simultaneamente.\n"
             "La mente fatica a restare coesa.\n"),
            ("Atrio del Mercante Planare",
             "Bancarelle impossibili vendono merci di ogni realta'.\n"
             "Il mercante non ha volto fisso.\n"),
        ],
        [
            ("Tesoro Planare",
             "Oggetti di mondi che non esistono piu'.\n"
             "Monete con simboli sconosciuti.\n"),
            ("Camera delle Chiavi",
             "Chiavi di portali per ogni piano.\n"
             "Aprirne una senza sapere dove porta e' folle.\n"),
        ],
        [
            ("Trappola della Gravita'",
             "La gravita' cambia direzione senza preavviso.\n"
             "Cadere verso il soffitto e' possibile.\n"),
            ("Sala delle Illusioni Planari",
             "Copie di te stesso attaccano da ogni angolo.\n"
             "Quale e' reale?\n"),
        ],
        "Crocevia di Sigil",
        "Una piazza fluttuante ospita un crocevia infinito.\n"
        "Portali verso ogni piano si aprono e chiudono.\n"
        "Un guardiano planare veglia sul varco.\n",
    ),
    (
        "Castello delle Ombre",
        "SECT_INSIDE",
        "INDOORS | DARK",
        [
            ("Cancello del Castello",
             "Un cancello di ferro arrugginito scricchiola al vento.\n"
             "Torri spezzate perforano un cielo grigio.\n"),
            ("Portone Principale",
             "Un portone massiccio e' socchiuso.\n"
             "Odore di muffa e cera vecchia esce dall'interno.\n"),
        ],
        [
            ("Corridoio delle Tappezzerie",
             "Arazzi sbiaditi raffigurano cacce macabre.\n"
             "Occhi sembrano seguirti tra le figure.\n"),
            ("Galleria delle Armature",
             "Armature vuote sorvegliano ogni passaggio.\n"
             "Spade arrugginite giacciono ai loro piedi.\n"),
            ("Passaggio delle Candele",
             "Candele nere illuminano debolmente.\n"
             "Ombre danzano sulle pareti senza sorgente.\n"),
            ("Atrio della Scala",
             "Una scala a chiocciola sale verso l'oscurita'.\n"
             "Ragnatele coprono ogni gradino.\n"),
        ],
        [
            ("Tesoro del Conte",
             "Gioielli e argenteria di famiglie nobili.\n"
             "Medaglioni con ritratti di defunti.\n"),
            ("Biblioteca Proibita",
             "Libri di necromanzia e vampirismo.\n"
             "Pagine che si muovono da sole.\n"),
        ],
        [
            ("Sala del Pavimento Cedevole",
             "Assi marce nascondono fosse sotto.\n"
             "Legno che scricchiola avverte troppo tardi.\n"),
            ("Passaggio dei Pipistrelli",
             "Pipistrelli si lanciano in sciami al passaggio.\n"
             "Zanne e artigli graffiano da ogni lato.\n"),
        ],
        "Sala del Trono Nero",
        "Una sala gotica culmina in un trono di velluto nero.\n"
        "Finestre alte mostrano un cielo eternamente notturno.\n"
        "Il conte delle ombre attende con calice di sangue.\n",
    ),
    (
        "Antro del Drago",
        "SECT_MOUNTAIN",
        "INDOORS | DARK",
        [
            ("Fessura Fumante",
             "Fumo esce da una fessura nella montagna.\n"
             "Zolfo e calore avvertono di cio' che attende.\n"),
            ("Covo del Predatore",
             "Artigli e scaglie segnano l'ingresso.\n"
             "Ossa di grandi prede giacciono ammucchiate.\n"),
        ],
        [
            ("Galleria delle Scaglie",
             "Scaglie incastonate nella roccia brillano.\n"
             "Calore aumenta ad ogni passo.\n"),
            ("Corridoio del Tesoro Parziale",
             "Piccoli tesori precedono il tesoro vero.\n"
             "Monete e gemme sparse sul pavimento.\n"),
            ("Passaggio del Soffio",
             "Braci ancora calde segnano il passaggio del drago.\n"
             "Roccia fusa forma stalattiti bizzarre.\n"),
            ("Atrio delle Ossa",
             "Ossa di cavalli, nani e giganti formano muri.\n"
             "Crani impilati come decorazione.\n"),
        ],
        [
            ("Tesoro del Drago",
             "Oro e gemme coprono ogni superficie.\n"
             "Armi magiche di eroi caduti.\n"),
            ("Camera delle Uova",
             "Uova di drago pulsano di calore.\n"
             "Non dovresti essere qui.\n"),
        ],
        [
            ("Trappola del Soffio",
             "Getti di fiamma erompono da fessure.\n"
             "Il drago conosce ogni centimetro.\n"),
            ("Sala del Crollo",
             "Il soffitto e' stato indebolito dal fuoco.\n"
             "Pietre cadono senza preavviso.\n"),
        ],
        "Covo del Drago Antico",
        "Una caverna immensa ospita un drago ancora vivo.\n"
        "Tesoro innumerevole brilla sotto le scaglie.\n"
        "Il drago apre un occhio e poi l'altro.\n",
    ),
    (
        "Caverna degli Occhi",
        "SECT_INSIDE",
        "INDOORS | DARK",
        [
            ("Fessura Aberrante",
             "Pareti pulsano come carne viva.\n"
             "Occhi crescono dalla roccia e scompaiono.\n"),
            ("Bocca della Caverna",
             "Un assalto mentale ti colpisce prima del fisico.\n"
             "Geometrie sbagliate fanno male agli occhi.\n"),
        ],
        [
            ("Galleria delle Tentacoli",
             "Tentacoli sporgono e si ritirano nelle pareti.\n"
             "Muco viscido copre ogni superficie.\n"),
            ("Corridoio della Paranoia",
             "Sussurri nella tua mente senza fonte.\n"
             "I compagni sembrano nemici per un istante.\n"),
            ("Passaggio delle Illusioni",
             "Visioni di morte e follia ovunque.\n"
             "La realta' e' negoziabile qui.\n"),
            ("Atrio del Beholder",
             "Una sfera fluttuante osserva ogni angolo.\n"
             "Il pavimento e' bruciato dai raggi di energia.\n"),
        ],
        [
            ("Tesoro della Mente",
             "Cristalli di memoria rubate.\n"
             "Pensieri altrui fluttuano in fiale.\n"),
            ("Camera degli Esperimenti",
             "Creature fuse e mutilate in gabbie.\n"
             "Strumenti chirurgici alieni.\n"),
        ],
        [
            ("Trappola del Raggio",
             "Raggi di disintegrazione sparano dalle pareti.\n"
             "Occhi nascosti attendono il movimento.\n"),
            ("Sala della Follia",
             "La mente subisce assalti psichici.\n"
             "Resistere e' la vera prova.\n"),
        ],
        "Nido del Beholder",
        "Una caverna sferica ospita un beholder antico.\n"
        "Occhi su tentacoli scrutano ogni difetto.\n"
        "La creatura alza i suoi raggi mortali.\n",
    ),
    (
        "Fosso Infernale",
        "SECT_INSIDE",
        "INDOORS | DARK",
        [
            ("Porta Infernale",
             "Un arco di bronzo reca rune infernali.\n"
             "Calore e zolfo escono dal varco.\n"),
            ("Scala dei Dannati",
             "Gradini di ossidiana scendono verso il calore.\n"
             "Voci di tormento salgono dal basso.\n"),
        ],
        [
            ("Galleria delle Catene",
             "Catene infuocate pendono dal soffitto.\n"
             "Ceneri di dannati coprono il pavimento.\n"),
            ("Corridoio dei Diavoli",
             "Impronte di zoccoli bruciate nella pietra.\n"
             "Odore di zolfo e paura.\n"),
            ("Passaggio del Fuoco Eterno",
             "Fiamme che non consumano tutto illuminano il cammino.\n"
             "Calore insopportabile verso il centro.\n"),
            ("Atrio del Principe",
             "Trono di lava solidificata domina la sala.\n"
             "Stendardi infernali segnano territorio.\n"),
        ],
        [
            ("Tesoro Infernale",
             "Oro maledetto e gemme corrotte.\n"
             "Contratti firmati con sangue.\n"),
            ("Camera dei Contratti",
             "Anime imprigionate in pergamene.\n"
             "Firme ancora fresche su documenti antichi.\n"),
        ],
        [
            ("Trappola del Fuoco",
             "Getti di fiamma infernale erompono dal pavimento.\n"
             "Bruciare e' solo l'inizio.\n"),
            ("Sala delle Tentazioni",
             "Visioni dei desideri piu' profondi.\n"
             "Cedere significa restare per sempre.\n"),
        ],
        "Trono dei Nove Inferi",
        "Una sala di fuoco e ossidiana culmina in un trono.\n"
        "Diavoli minori servono un principe infernale.\n"
        "Il signore del fosso pronuncia la tua condanna.\n",
    ),
]


def cpp_string(text: str) -> str:
    parts = [p.strip() for p in text.strip().split("\n") if p.strip()]
    if not parts:
        return '""'
    if len(parts) == 1:
        return f'"{parts[0].replace(chr(34), chr(92)+chr(34))}\\n"'
    lines = []
    for i, part in enumerate(parts):
        escaped = part.replace('"', '\\"')
        lines.append(f'"{escaped}\\n"')
    return "\n\t  ".join(lines)


def expand_theme_rooms(
    theme: tuple,
) -> tuple:
    label, sector, flags, entrances, corridors, treasures, traps, boss_name, boss_desc = theme

    def expand_list(rooms: list[tuple[str, str]], kind: str) -> list[tuple[str, str]]:
        return [
            (name, ensure_min_lines(desc, MIN_DESC_LINES, sector, name, kind))
            for name, desc in rooms
        ]

    return (
        label,
        sector,
        flags,
        expand_list(entrances, "entrance"),
        expand_list(corridors, "corridor"),
        expand_list(treasures, "treasure"),
        expand_list(traps, "trap"),
        boss_name,
        ensure_min_lines(boss_desc, MIN_DESC_LINES, sector, boss_name, "boss"),
    )


def emit_room(name: str, desc: str, sector: str, flags: str) -> str:
    return (
        f'\t{{ "{name}",\n'
        f"\t  {cpp_string(desc)},\n"
        f"\t  {sector}, static_cast<long>({flags}) }}"
    )


def emit_theme(idx: int, theme: tuple) -> str:
    theme = expand_theme_rooms(theme)
    label, sector, flags, entrances, corridors, treasures, traps, boss_name, boss_desc = theme
    n = idx
    chunks: list[str] = [f"/* --- Tema {n}: {label} --- */"]

    def array(var: str, rooms: list[tuple[str, str]]) -> str:
        body = ",\n".join(emit_room(nm, ds, sector, flags) for nm, ds in rooms)
        return f"static const ProcRoomTemplate {var}[] = {{\n{body},\n}};"

    chunks.append(array(f"kTheme{n}Entrances", entrances))
    chunks.append(array(f"kTheme{n}Corridors", corridors))
    chunks.append(array(f"kTheme{n}Treasures", treasures))
    chunks.append(array(f"kTheme{n}Traps", traps))
    boss_body = (
        f'static const ProcRoomTemplate kTheme{n}Boss = {{\n'
        f'\t"{boss_name}",\n'
        f"\t  {cpp_string(boss_desc)},\n"
        f"\t{sector}, static_cast<long>({flags})\n"
        f"}};"
    )
    chunks.append(boss_body)
    return "\n\n".join(chunks)


def emit_kthemesets_entries(start: int, count: int, labels: list[str]) -> str:
    lines: list[str] = []
    for i in range(start, start + count):
        lines.append(
            f'\t{{ "{labels[i]}",\n'
            f"\t  theme_room_list(kTheme{i}Entrances),\n"
            f"\t  theme_room_list(kTheme{i}Corridors),\n"
            f"\t  theme_room_list(kTheme{i}Treasures),\n"
            f"\t  theme_room_list(kTheme{i}Traps),\n"
            f"\t  &kTheme{i}Boss }},"
        )
    return "\n".join(lines)


def c_unescape(text: str) -> str:
    out: list[str] = []
    i = 0
    while i < len(text):
        if text[i] == "\\" and i + 1 < len(text):
            nxt = text[i + 1]
            if nxt == "n":
                out.append("\n")
                i += 2
                continue
            if nxt == "t":
                out.append("\t")
                i += 2
                continue
            if nxt == '"':
                out.append('"')
                i += 2
                continue
            if nxt == "\\":
                out.append("\\")
                i += 2
                continue
        out.append(text[i])
        i += 1
    return "".join(out)


def expand_base_themes_inc() -> None:
    import re

    kind_suffixes = {
        "Entrances": "entrance",
        "Corridors": "corridor",
        "Treasures": "treasure",
        "Traps": "trap",
        "Boss": "boss",
    }

    text = THEMES_INC.read_text(encoding="utf-8")
    if INCLUDE_MARKER in text:
        head, tail = text.split(INCLUDE_MARKER, 1)
        tail = INCLUDE_MARKER + tail
    else:
        head, tail = text, ""

    lines = head.splitlines(keepends=True)
    out: list[str] = []
    i = 0
    current_kind = "corridor"

    while i < len(lines):
        line = lines[i]
        kind_match = re.search(r"kTheme\d+(Entrances|Corridors|Treasures|Traps|Boss)", line)
        if kind_match:
            current_kind = kind_suffixes.get(kind_match.group(1), "corridor")

        name_match = re.match(r"(\t)(\{ )?\"([^\"]+)\",\s*$", line.rstrip("\n"))
        if name_match:
            indent, brace, name = name_match.group(1), name_match.group(2), name_match.group(3)
            is_array = brace is not None
            i += 1

            string_chunk: list[str] = []
            while i < len(lines) and not re.search(r"\bSECT_[A-Z_]+\b", lines[i]):
                string_chunk.append(lines[i])
                i += 1

            sector_line = lines[i] if i < len(lines) else ""
            i += 1

            text_parts: list[str] = []
            for chunk in string_chunk:
                for sm in re.finditer(r'"((?:[^"\\]|\\.)*)"', chunk):
                    text_parts.append(c_unescape(sm.group(1)))
            existing = [part.strip() for part in "".join(text_parts).split("\n") if part.strip()]

            sector = "SECT_INSIDE"
            sector_match = re.search(r"(SECT_[A-Z_]+)", sector_line)
            if sector_match:
                sector = sector_match.group(1)

            expanded = ensure_min_lines(
                "\n".join(existing) + "\n",
                MIN_DESC_LINES,
                sector,
                name,
                current_kind,
            ).rstrip("\n")

            if is_array:
                out.append(f'{indent}{{ "{name}",\n')
                out.append(f"\t  {cpp_string(expanded)},\n")
            else:
                out.append(f'{indent}"{name}",\n')
                out.append(f"\t  {cpp_string(expanded)},\n")
            out.append(sector_line)
            continue

        out.append(line)
        i += 1

    THEMES_INC.write_text("".join(out) + tail, encoding="utf-8")


def main() -> None:
    expand_base_themes_inc()
    labels = [t[0] for t in THEMES_25_49]
    body = "\n\n".join(emit_theme(25 + i, t) for i, t in enumerate(THEMES_25_49))
    EXT_INC.write_text(
        f"""/* procarea_themes_ext.inc — temi 25-49 (generato, non editare a mano) */
#ifndef __PROCAREA_THEMES_EXT_INC
#define __PROCAREA_THEMES_EXT_INC

{body}

#endif /* __PROCAREA_THEMES_EXT_INC */
""",
        encoding="utf-8",
    )

    text = THEMES_INC.read_text(encoding="utf-8")
    if '#include "procarea_themes_ext.inc"' not in text:
        text = text.replace(
            "static const ProcThemeSet kThemeSets[] = {",
            '#include "procarea_themes_ext.inc"\n\nstatic const ProcThemeSet kThemeSets[] = {',
        )

    marker = '\t  &kTheme24Boss },\n};'
    insert_lines = []
    for i, (label, *_) in enumerate(THEMES_25_49):
        n = 25 + i
        insert_lines.append(
            f'\t{{ "{label}",\n'
            f"\t  theme_room_list(kTheme{n}Entrances),\n"
            f"\t  theme_room_list(kTheme{n}Corridors),\n"
            f"\t  theme_room_list(kTheme{n}Treasures),\n"
            f"\t  theme_room_list(kTheme{n}Traps),\n"
            f"\t  &kTheme{n}Boss }},"
        )
    insert = "\n".join(insert_lines) + "\n"
    if "kTheme25Entrances" not in text:
        if marker in text:
            text = text.replace(marker, '\t  &kTheme24Boss },\n' + insert + "};")
        else:
            raise RuntimeError("Could not find kTheme24Boss closing in procarea_themes.inc")

    THEMES_INC.write_text(text, encoding="utf-8")
    print(f"Expanded base themes in {THEMES_INC} (min {MIN_DESC_LINES} desc lines)")
    print(f"Wrote {EXT_INC} ({len(THEMES_25_49)} themes)")
    print(f"Updated {THEMES_INC} kThemeSets entries")


if __name__ == "__main__":
    main()
