"""Catalogo archetipi mob procarea — sorgente per procarea_mob_desc.inc.

55 boss (30 + 25 epici), 150 mob, 20 trap. Ispirazione LOTR + D&D.
"""

from __future__ import annotations

from typing import TypedDict


class Archetype(TypedDict):
    race: str
    keywords: str
    short: str
    long: str
    look: str
    agg: str
    sound: str


def _A(
    race: str,
    keywords: str,
    short: str,
    long: str,
    look: str,
    agg: str,
    sound: str,
    themes: list[int] | None = None,
) -> Archetype:
    entry: Archetype = {
        "race": race,
        "keywords": keywords,
        "short": short,
        "long": long,
        "look": look,
        "agg": agg,
        "sound": sound,
    }
    if themes:
        entry["themes"] = themes
    return entry


EPIC_BOSS_COUNT = 25
BASE_BOSS_COUNT = 30
BOSS_COUNT = BASE_BOSS_COUNT + EPIC_BOSS_COUNT
MOB_COUNT = 150
TRAP_COUNT = 20


def get_base_bosses() -> list[Archetype]:
    return [
        _A(
            "RACE_PREDATOR",
            "alfa nebbia effimera",
            "Alfa della Nebbia Effimera",
            "Un predatore alfa emerge dal vapore denso della dimensione.",
            "Zanne irregolari e occhi gialli non tradiscono pieta'.",
            "Senti un ringhio basso filtrare dalla nebbia.",
            "In lontananza un ringhio squarcia il silenzio.",
        ),
        _A(
            "RACE_ARACHNID",
            "regina tarantole",
            "Regina delle Tarantole",
            "Una tarantola colossale domina un nido di seta e ossa.",
            "Setole urticanti coprono zampe spesse come tronchi.",
            "Le ragnatele intorno a te vibrano come corde tese.",
            "Da qui vicino odi ragnatele che vibrano.",
        ),
        _A(
            "RACE_TREE",
            "guardiano muschio antico",
            "Guardiano di Muschio Antico",
            "Un colosso vegetale blocca la sala con radici animate.",
            "Muschio vivo pulsa come un secondo cuore.",
            "Radici vive scricchiolano sotto i tuoi piedi.",
            "Odi in lontananza radici che scoppiano fuori dalla roccia.",
        ),
        _A(
            "RACE_PREDATOR",
            "predatore cieco",
            "Predatore Cieco delle Profondita'",
            "Una bestia senza occhi percepisce ogni tuo movimento.",
            "Artigli lunghi segnano le pareti.",
            "Un urlo cavernoso ti passa a fianco senza eco.",
            "In lontananza risuona un urlo cavernoso.",
        ),
        _A(
            "RACE_INSECT",
            "matriarca larve",
            "Matriarca delle Larve",
            "Una montagna gelatinosa regge un trono di larve.",
            "Larve obbedienti si gettano contro gli intrusi.",
            "Schizzi viscidi cadono dal soffitto accanto a te.",
            "Da una sala vicina arriva lo schizzo di qualcosa di viscoso.",
        ),
        _A(
            "RACE_BIRD",
            "signore pipistrelli",
            "Signore dei Pipistrelli delle Profondita'",
            "Una creatura alata coronata di ossa volteggia in basso.",
            "Canini sporgono da un muso allungato e famelico.",
            "Ali membranose battono sopra la tua testa.",
            "Odi in lontananza un battito d'ali frenetico.",
        ),
        _A(
            "RACE_INSECT",
            "capo saprofagi",
            "Capo dei Saprofagi",
            "Un insettoide putrefatto comanda sciami affamati.",
            "Mandibole gocciolano liquido nero corrosivo.",
            "Un tanfo di carogna ti stringe la gola.",
            "Da qui vicino arriva un odore acre di carogne.",
        ),
        _A(
            "RACE_ORC",
            "custode fosso",
            "Custode del Fosso",
            "Un guerriero rannicchiato sorveglia un baratro laterale.",
            "L'armatura e' incrostata di fango secco e sangue.",
            "Pietre scivolano verso un baratro a pochi passi.",
            "In lontananza odi pietre cadere nel vuoto.",
        ),
        _A(
            "RACE_INSECT",
            "re larve",
            "Re delle Larve",
            "Un sovrano viscido e' nato dal cumulo di generazioni.",
            "Occhi multipli si aprono e chiudono a ondate.",
            "Un brusio umido e innaturale riempie la stanza.",
            "Odi in lontananza un brusio viscido e continuo.",
        ),
        _A(
            "RACE_GHOST",
            "ombra cunicolo",
            "Ombra del Primo Cunicolo",
            "La prima ombra nata nella dimensione si stacca dal muro.",
            "Non ha volto: solo fame antica.",
            "Un freddo senza fonte ti morde le ossa.",
            "Da qui vicino arriva un gelo che fa tremare.",
        ),
        _A(
            "RACE_UNDEAD_LICH",
            "arconte catacombe",
            "Arconte delle Catacombe",
            "Un lich avvolto in fasce polverose troneggia su un sepolcro aperto.",
            "Rune necromantiche bruciano come brace nel vuoto degli occhi.",
            "Qualcuno sussurra in lingue morte proprio qui.",
            "In lontananza sussurri di lingue morte si trascinano.",
        ),
        _A(
            "RACE_UNDEAD_SPECTRE",
            "signore nazgul caduto",
            "Signore dei Nazgul Caduti",
            "Uno spettro corazzato di nero emerge dalla bruma come un re senza corona.",
            "Un anello spezzato gli pende dal collo spettrale.",
            "Un fischio gelido ti sfiora tra le pietre.",
            "Odi in lontananza un fischio gelido tra le rocce.",
        ),
        _A(
            "RACE_ARACHNID",
            "madre ragni ungol",
            "Madre dei Ragni di Ungol",
            "Un'aracnide titanica attende nel buio filando seta nera.",
            "Occhi multipli brillano come stelle malevole.",
            "Ragnatele nere si tendono d'improvviso intorno a te.",
            "Da una caverna vicina arriva lo stridio di seta tesa.",
        ),
        _A(
            "RACE_GOBLIN",
            "capobanda goblin profondita",
            "Capobanda Goblin delle Profondita'",
            "Un goblin corazzato con ferri di recupero brandisce un'ascia dentata.",
            "Orecchie mozzate penzolano dalla sua cintura.",
            "Risate acute e feroci ti circondano.",
            "In lontananza echeggiano risate goblin acute.",
        ),
        _A(
            "RACE_TROLL",
            "signore troll pietra",
            "Signore dei Troll di Pietra",
            "Un troll massiccio si rialza da una pozza di sangue coagulato.",
            "La pelle sembra granito vivo; le ferite si richiudono da sole.",
            "Pietre rotolano nel buio a pochi passi da te.",
            "Odi in lontananza massi che rotolano nel buio.",
        ),
        _A(
            "RACE_UNDEAD_WIGHT",
            "custode tumuli",
            "Custode dei Tumuli",
            "Un guerriero sepolcrale sorge da un tumulo di ossa antiche.",
            "Spada arrugginita e corona spezzata lo contraddistinguono.",
            "Un lamento di tombe profonde ti passa vicino.",
            "In lontananza un lamento sepolcrale si allunga.",
        ),
        _A(
            "RACE_DRAGON_GREEN",
            "drago caverne giade",
            "Drago di Giada delle Caverne",
            "Un drago giovane ma feroce riempie la caverna di vapore acido.",
            "Scaglie verdastre raccolgono condensa velenosa.",
            "Un ruggito fa tremare la volta sopra di te.",
            "Da qui vicino arriva un ruggito che scuote la roccia.",
        ),
        _A(
            "RACE_BIRD",
            "matriarca arpie rupi",
            "Matriarca delle Arpie delle Rupi",
            "Un'arpia regale plana su artigli insanguinati.",
            "Piume nere e occhi d'ambra la rendono un incubo alato.",
            "Un grido straziante ti squarcia le orecchie.",
            "Odi in lontananza un grido d'arpia straziante.",
        ),
        _A(
            "RACE_ORC",
            "signore uruk profondi",
            "Signore degli Uruk Profondi",
            "Un capitano uruk brandisce una lama larga come un'ascia.",
            "Rune di guerra bruciano sulla sua pelle grigio-ferrea.",
            "Cori di guerra sotterranei rimbombano nella sala.",
            "In lontananza risuonano cori di guerra orcheschi.",
        ),
        _A(
            "RACE_TREE",
            "ent corrotto antico",
            "Antico Ent Corrotto",
            "Un ent contorto dalla bruma torce rami come mazze.",
            "Corteccia nera stilla linfa velenosa.",
            "Legno vivo scricchiola in modo minaccioso qui vicino.",
            "Da qui vicino arriva lo scricchiolio di rami corrotti.",
        ),
        _A(
            "RACE_DEMON",
            "demone fessure",
            "Demone delle Fessure",
            "Un demone cornuto striscia fuori da una crepa nel muro.",
            "Zolfo e cenere aleggiano intorno a lui.",
            "Un odore di zolfo bruciacchiato ti prende alla gola.",
            "In lontananza aleggia un tanfo di zolfo.",
        ),
        _A(
            "RACE_UNDEAD_VAMPIRE",
            "signore vampiro bruma",
            "Signore dei Vampiri della Bruma",
            "Un vampiro aristocratico emerge dalla nebbia con mantello lacero.",
            "Canini affilati e occhi cremisi non tradiscono l'eta'.",
            "Un sussurro seducente e mortale ti sfiora l'orecchio.",
            "Odi in lontananza un sussurro seducente e gelido.",
        ),
        _A(
            "RACE_GIANT_HILL",
            "gigante radici",
            "Gigante delle Radici",
            "Un gigante delle colline blocca il passaggio con un tronco nodoso.",
            "Radici gli crescono dalle caviglie fino al ginocchio.",
            "Passi enormi fanno tremare il pavimento sotto di te.",
            "In lontananza passi colossali scuotono la roccia.",
        ),
        _A(
            "RACE_ELEMENT",
            "signore elementale fango",
            "Signore degli Elementali del Fango",
            "Una massa di fango senziente assume forma umanoide.",
            "Pietre e ossa galleggiano nella sua corrente viscida.",
            "Schizzi di fango caldo ti schizzano addosso.",
            "Da una sala vicina arriva lo schizzo di fango vivo.",
        ),
        _A(
            "RACE_DARK_ELF",
            "regina rocca nera",
            "Regina della Rocca Nera",
            "Una drow incappucciata troneggia su un trono di stalattiti.",
            "Lame di ossidiana le scintillano attorno come aureola.",
            "Incantesimi sibilati ti girano intorno nel buio.",
            "In lontananza odi formule sibilate nel buio.",
        ),
        _A(
            "RACE_DEVIL",
            "signore culto abissale",
            "Signore del Culto Abissale",
            "Un diavolo cornuto conduce un coro di fedeli mutati.",
            "Simboli infernali bruciano sulla pietra circostante.",
            "Canti distorti e irreali riempiono l'aria qui.",
            "Odi in lontananza canti cultisti distorti.",
        ),
        _A(
            "RACE_GHOST",
            "oracolo corrotto moria",
            "Oracolo Corrotto di Moria",
            "Lo spirito di un saggio nano urla profezie nel buio.",
            "Libri fluttuano attorno a lui come lune orbitanti.",
            "Echi di martelli antichi ti vibrano nelle ossa.",
            "In lontananza echi di martelli battono nella roccia.",
        ),
        _A(
            "RACE_SPECIAL",
            "signore mimiche",
            "Signore delle Mimiche",
            "Una creatura che imita un trono e un tesoro attende immobile.",
            "La pelle sembra metallo finche' non si apre a bocca piena.",
            "Un tonfo sordo di carne che diventa pietra ti fa sussultare.",
            "Da qui vicino arriva un tonfo sordo e innaturale.",
        ),
        _A(
            "RACE_DEMON",
            "balrog minore",
            "Balrog Minore",
            "Una creatura di fuoco e ombra brandisce una frusta di fiamme.",
            "Ali carbonizzate spalancano calore insopportabile.",
            "Un ruggito di magma sotterraneo ti scuote.",
            "In lontananza un ruggito di magma ribolle.",
        ),
        _A(
            "RACE_UNDEAD_GHAST",
            "sovrano ghoul ossa",
            "Sovrano dei Ghoul delle Ossa",
            "Un ghoul reale si rialza da un trono di femori intrecciati.",
            "Artigli neri e lingua biforcuta gocciolano ichor.",
            "Ossa scricchiolano nel buio a un passo da te.",
            "Odi in lontananza ossa che scricchiolano.",
        ),
    ]


def _mob_categories() -> list[list[Archetype]]:
    """150 mob in 15 categorie da 10."""
    cats: list[list[Archetype]] = []

    cats.append(
        [
            _A("RACE_GOBLIN", "goblin esploratore", "Goblin Esploratore", "Un goblin magro sbuca da un crepaccio con pugnale arrugginito.", "Orecchie mozzate gli ornano il collo.", "Senti un goblin borbottare e grattarsi l'armatura.", "Odi in lontananza un urlo goblin acuto."),
            _A("RACE_GOBLIN", "goblin sciamano", "Goblin Sciamano", "Un goblin coperto di rune brandisce un bastone di ossa.", "Occhi gialli brillano di follia sciamanica.", "Mormori gutturali di uno sciamano ti riempiono le orecchie.", "Da qui vicino arriva un mormorio gutturale."),
            _A("RACE_GOBLIN", "goblin arciere", "Goblin Arciere", "Un goblin armato di arco corto mira dall'ombra.", "Frecce avvelenate spuntano dalla faretra.", "Una corda di arco vibra piano nella stanza.", "In lontananza odi frecce che fischiano nel buio."),
            _A("RACE_GOBLIN", "goblin speleologo", "Goblin Speleologo", "Un goblin con lanterna di tartaruga esplora i cunicoli.", "Corde e uncini gli pendono dalla cintura.", "Metallo striscia sulla roccia a un passo da te.", "Odi in lontananza uncini che graffiano la pietra."),
            _A("RACE_GOBLIN", "goblin ladro", "Goblin Ladro", "Un goblin furtivo fruga tra i detriti con occhi furbi.", "Dita nere e veloci non lasciano traccia.", "Risate soffocate ti girano intorno alle spalle.", "Da qui vicino arriva una risata goblin soffocata."),
            _A("RACE_GOBLIN", "hobgoblin soldato", "Hobgoblin Soldato", "Un hobgoblin in armatura di cuoio brandisce una spada e uno scudo.", "Disciplina crudele si legge nei suoi occhi rossi.", "Passi marziali pesanti battono il pavimento.", "In lontananza odi passi marziali hobgoblin."),
            _A("RACE_GOBLIN", "hobgoblin capitano", "Hobgoblin Capitano", "Un hobgoblin corazzato conduce una banda di sottoposti.", "Stendardo strappato ancora gli cinge il petto.", "Comandi ruvidi rimbombano nella sala.", "Odi in lontananza ordini ruvidi e secchi."),
            _A("RACE_ORC", "orco guerriero", "Orco Guerriero", "Un orco muscoloso ringhia mostrando zanne aguzze.", "Cicatrici rituali solcano il petto nudo.", "Grugniti di guerra orcheschi ti passano vicino.", "In lontananza risuonano grugniti di guerra."),
            _A("RACE_ORC", "orco berserker", "Orco Berserker", "Un orco reso folle dalla battaglia avanza ringhiando.", "Ascia a due mani sollevata, occhi fuori orbita.", "Un ansimare furioso e irrequieto riempie l'aria.", "Da qui vicino arriva un'urla di sangue orchesca."),
            _A("RACE_HALF_ORC", "uruk corridore", "Uruk Corridore", "Un uruk corre senza stancarsi lungo i corridoi.", "Elmo nero e lama larga lo rendono un'ombra letale.", "Passi pesanti e rapidi ti sfiorano di fianco.", "Odi in lontananza passi uruk pesanti e veloci."),
        ]
    )

    cats.append(
        [
            _A("RACE_ORC", "orco sciamano", "Orco Sciamano", "Un orco coperto di ossa invoca spiriti della caverna.", "Tatuaggi neri pulsano quando canta.", "Canti gutturali di uno sciamano ti circondano.", "In lontananza canti orcheschi gutturali echeggiano."),
            _A("RACE_ORC", "orco guardia", "Orco Guardia", "Un orco armato sorveglia un varco stretto.", "Lancia arrugginita pronta a trafiggere.", "Un grugnito di sfida ti arriva da vicino.", "Da qui vicino arriva un grugnito di guardia orchesca."),
            _A("RACE_ORC", "orco spezzaneve", "Orco Spezzaneve", "Un orco delle montagne emerge dal ghiaccio sotterraneo.", "Pelliccia imbrattata di sangue e brina.", "Un vento gelido ti investe dalla parte dell'orco.", "Odi in lontananza un vento gelido che fischia."),
            _A("RACE_HALF_ORC", "mezzorco reietto", "Mezzorco Reietto", "Un mezzorco emarginato combatte con disperazione.", "Occhi tristi e pugno di ferro.", "Un ringhio sommesso di reietto ti fa alzare lo sguardo.", "In lontananza un ringhio sommesso si trascina."),
            _A("RACE_GOBLIN", "goblin furtivo", "Goblin Furtivo", "Un goblin si mimetizza tra le stalattiti.", "Pelle grigia lo confonde con la roccia.", "Pietre cadono dal soffitto a pochi passi.", "Da qui vicino arriva il tonfo di pietre che cadono."),
            _A("RACE_TROGMAN", "troglodita guerriero", "Troglodita Guerriero", "Un troglodita brandisce una clava d'osso.", "Pelle pallida e occhi ciechi al sole.", "Grugniti cavernosi di un troglodita riempiono la sala.", "Odi in lontananza grugniti cavernosi."),
            _A("RACE_TROGMAN", "troglodita cacciatore", "Troglodita Cacciatore", "Un troglodita cacciatore trascina una lancia di selce.", "Odore di muschio e carne cruda.", "Passi silenziosi di cacciatore ti sfiorano.", "Ti pare di scorgere passi silenziosi oltre il buio."),
            _A("RACE_ORC", "orco flagellatore", "Orco Flagellatore", "Un orco sadico brandisce una frusta chiodata.", "Sorriso crudele sotto elmo deformato.", "Uno scroscio di frusta fende l'aria vicino a te.", "In lontananza odi lo scroscio di una frusta."),
            _A("RACE_GOBLIN", "goblin alchimista", "Goblin Alchimista", "Un goblin maneggia fiale fumanti con cautela.", "Meta' del volto e' bruciata da esplosioni recenti.", "Vetro si rompe e acido gocciola qui vicino.", "Da qui vicino arriva il fragore di vetro che si spezza."),
            _A("RACE_HALF_ORC", "uruk sentinella", "Uruk Sentinella", "Un uruk immobile veglia su un ponte di pietra.", "Lancia lunga e scudo nero coprono ogni angolo.", "L'armatura di una sentinella stride a ogni passo.", "Odi in lontananza armatura uruk che stride."),
        ]
    )

    cats.append(
        [
            _A("RACE_UNDEAD_SKELETON", "scheletro guerriero", "Scheletro Guerriero", "Uno scheletro armato si rialza dal cumulo di ossa.", "Costole vuote battono come tamburi.", "Ossa sbattono l'una contro l'altra nella stanza.", "In lontananza ossa di scheletro sbattono nel buio."),
            _A("RACE_UNDEAD_SKELETON", "scheletro arciere", "Scheletro Arciere", "Uno scheletro tende un arco di ossa e tendini.", "Frecce di metallo arrugginito puntano verso di te.", "Una corda d'osso vibra mentre una freccia viene tesa.", "Da qui vicino arriva il fischio di frecce nel buio."),
            _A("RACE_UNDEAD_ZOMBIE", "zombie contadino", "Zombie Contadino", "Un cadavere rigido avanza con braccia tese.", "Vestiti laceri e occhi vitrei.", "Senti gemiti soffocati.", "Odi in lontananza gemiti soffocati."),
            _A("RACE_UNDEAD_ZOMBIE", "zombie soldato", "Zombie Soldato", "Un soldato morto marcia ancora con spada in mano.", "Armatura ammaccata copre carne putrefatta.", "L'armatura dello zombie stride a ogni passo.", "Da qui vicino arriva lo stridio di un'armatura."),
            _A("RACE_UNDEAD_GHOUL", "ghoul famelico", "Ghoul Famelico", "Un ghoul striscia a quattro zampe verso di te.", "Artigli neri e lingua biforcuta.", "Odi una masticazione umida e famelica.", "In lontananza risuona una masticazione di carne."),
            _A("RACE_UNDEAD_GHOUL", "ghoul necrofago", "Ghoul Necrofago", "Un ghoul rovista tra tombe aperte.", "Denti affilati gocciolano ichor nero.", "Un odore di sepolcro ti prende alla gola.", "Odi in lontananza un tanfo di sepolcro."),
            _A("RACE_UNDEAD_WIGHT", "wight guerriero", "Wight Guerriero", "Un wight spettrale brandisce una spada gelida.", "Aura di freddo mortale lo circonda.", "Un freddo che penetra le ossa ti avvolge.", "Da qui vicino arriva un gelo che morde le ossa."),
            _A("RACE_UNDEAD_SPECTRE", "spettro errante", "Spettro Errante", "Uno spettro fluttua attraverso le pareti.", "Voce sussurrante promette morte.", "Sussurri gelidi ti passano attraverso.", "In lontananza sussurri gelidi si trascinano."),
            _A("RACE_GHOST", "fantasma prigioniero", "Fantasma Prigioniero", "Un fantasma incatenato alla caverna urla senza voce.", "Catene spettrali gli legano polsi e caviglie.", "Lamenti ovattati di un prigioniero ti circondano.", "Odi in lontananza lamenti ovattati."),
            _A("RACE_UNDEAD_LICH", "lich minore", "Lich Minore", "Un lich incompleto alza mani scheletriche.", "Pergamene bruciano attorno a lui.", "Incantesimi sibilati di un lich minore riempiono l'aria.", "Da qui vicino arriva lo sibilo di formule morte."),
        ]
    )

    cats.append(
        [
            _A("RACE_UNDEAD_SKELETON", "scheletro guardia", "Scheletro Guardia", "Uno scheletro in armatura sorveglia un arco.", "Elmo vuoto fissa il vuoto con occhi di brace.", "Spade d'osso si strofinano l'una contro l'altra.", "In lontananza odi lame di scheletro che sbattono."),
            _A("RACE_UNDEAD_ZOMBIE", "ogre zombie", "Ogre Zombie", "Un ogre morto si trascina con mazza imponente.", "Carne verde e gonfia sporge dalle ferite.", "Passi pesanti da morto fanno tremare la sala.", "Odi in lontananza passi pesanti di un cadavere enorme."),
            _A("RACE_UNDEAD_GHOUL", "ghoul delle profondita", "Ghoul delle Profondita'", "Un ghoul emaciato striscia dalle fessure.", "Occhi gialli brillano nel buio.", "Ringhi famelici ti arrivano da ogni lato.", "In lontananza ringhi famelici di ghoul echeggiano."),
            _A("RACE_UNDEAD_WIGHT", "wight sepolcrale", "Wight Sepolcrale", "Un wight sorge da un sarcofago rotto.", "Polsi scheletrici stringono una corona.", "Un vento gelido dal sepolcro ti investe.", "Da qui vicino arriva un vento gelido sepolcrale."),
            _A("RACE_UNDEAD_SPECTRE", "spettro guerriero", "Spettro Guerriero", "Uno spettro in armatura antica fluttua in modo minaccioso.", "Spada spettrale attraversa la roccia.", "Lame invisibili fischiano nel vuoto vicino a te.", "Odi in lontananza lame che fischiano nel vuoto."),
            _A("RACE_GHOST", "anima perduta", "Anima Perduta", "Un'anima perduta vaga senza meta tra i corridoi.", "Luce pallida la rende quasi trasparente.", "Un pianto sottile e senza corpo ti sfiora.", "In lontananza odi un pianto lontano e vuoto."),
            _A("RACE_UNDEAD_VAMPIRE", "vampiro servitore", "Vampiro Servitore", "Un vampiro giovane serve un padrone piu' antico.", "Mantello nero e occhi affamati.", "Un sussurro seducente ti sfiora l'orecchio.", "Da qui vicino arriva un sussurro vampiro seducente."),
            _A("RACE_UNDEAD_SKELETON", "scheletro chierico", "Scheletro Chierico", "Uno scheletro in tonaca invoca l'oscurita'.", "Simboli sacri corrotti brillano sul petto.", "Preghiere invertite e sacre a rovescio riempiono la stanza.", "Odi in lontananza preghiere invertite."),
            _A("RACE_UNDEAD_ZOMBIE", "zombie paludoso", "Zombie Paludoso", "Un cadavere paludoso emerge da pozze viscide.", "Alghe gli pendono dalle ferite.", "Schizzi di acqua fetida ti bagnano i piedi.", "In lontananza odi schizzi di acqua paludosa."),
            _A("RACE_UNDEAD_GHAST", "ghast affamato", "Ghast Affamato", "Un ghast piu' potente di un ghoul striscia verso di te.", "Alito putrefatto precede l'attacco.", "Un odore insopportabile ti fa venire il voltastomaco.", "Da qui vicino arriva un tanfo nauseabondo."),
        ]
    )

    cats.append(
        [
            _A("RACE_ARACHNID", "ragno caverna", "Ragno di Caverna", "Un ragno grosso come un cane attende nella ragnatela.", "Occhi neri multipli ti scrutano.", "Ragnatele si tendono piano intorno a te.", "Odi in lontananza ragnatele che si tendono."),
            _A("RACE_ARACHNID", "ragno veleno", "Ragno Velenoso", "Un ragno dalle zampe striate di giallo si muove rapido.", "Gocce di veleno brillano alle zanne.", "Un sibilo acuto di ragno velenoso ti fa alzare lo sguardo.", "In lontananza un sibilo acuto di aracnide echeggia."),
            _A("RACE_ARACHNID", "ragno gigante", "Ragno Gigante", "Un ragno gigante copre meta' del soffitto.", "Seta fresca ancora umida di prede.", "Ragnatele grosse vibrano come tamburi tesi.", "Da qui vicino arriva la vibrazione di grosse ragnatele."),
            _A("RACE_INSECT", "scarabeo corazza", "Scarabeo Corazzato", "Uno scarabeo grande come un vassoio avanza.", "Carapace lucido devia le lame.", "Un ronzio profondo di corazza di chitina riempie la sala.", "Odi in lontananza un ronzio profondo di scarabeo."),
            _A("RACE_INSECT", "centopiedi caverna", "Centopiedi di Caverna", "Un centopiedi lungo come un braccio striscia sulle pareti.", "Mandibole gocciolano veleno.", "Scricchiolio di chitina di centopiedi ti passa vicino.", "In lontananza odi chitina che scricchiola."),
            _A("RACE_INSECT", "formica gigante", "Formica Gigante", "Una formica gigante trasporta un pezzo di armatura.", "Le mandibole possono tranciare ossa.", "Un ronzio di sciame di formiche ti circonda.", "Da qui vicino arriva un ronzio di sciame."),
            _A("RACE_INSECT", "mosca necrofaga", "Mosca Necrofaga", "Una mosca grande come un pugno ronzia attorno.", "Occhi composti riflettono ogni movimento.", "Un ronzio insistente riempie la stanza.", "Odi in lontananza un ronzio insistente di mosche."),
            _A("RACE_INSECT", "grillo caverna", "Grillo di Caverna", "Un grillo nero grande come un gatto salta.", "Antenne tese captano ogni suono.", "Uno stridio acuto di grillo di caverna ti ferisce le orecchie.", "In lontananza uno stridio acuto di grillo echeggia."),
            _A("RACE_ARACHNID", "vedova nera", "Vedova Nera", "Una vedova nera grossa attende immobile.", "Segno rosso sul dorso avverte pericolo.", "Un silenzio innaturale precede qualcosa che striscia qui.", "Ti pare di scorgere un fruscio leggero di zampe."),
            _A("RACE_INSECT", "sciame caverna", "Sciame di Caverna", "Uno sciame di insetti corazzati fruga tra le pietre.", "Mandibole lucide portano tracce di ossa recenti.", "Un ronzio acuto di sciame riempie la stanza.", "Odi in lontananza un ronzio di sciame furioso."),
        ]
    )

    cats.append(
        [
            _A("RACE_INSECT", "larva nebbia effimera", "Larva della Nebbia Effimera", "Una larva traslucida striscia lasciando una scia di vapore gelido.", "La pelle viscida pulsa come se respirasse nebbia compressa.", "Senti un sibilo umido di larva nella nebbia.", "Da qui vicino arriva un sibilo umido e viscoso."),
            _A("RACE_INSECT", "verme bianco", "Verme Bianco", "Un verme bianco grosso come un braccio emerge dal fango.", "Pelle gelatinosa brilla di umidita'.", "Schizzi viscidi di verme bianco ti cadono addosso.", "In lontananza odi schizzi viscosi."),
            _A("RACE_INSECT", "stirge profondita", "Stirge delle Profondita'", "Una stirge alata plana con proboscide sanguinante.", "Ali sottili e occhi rossi.", "Un ronzio famelico di stirge ti passa sopra la testa.", "Odi in lontananza un ronzio famelico alato."),
            _A("RACE_ARACHNID", "zecca gigante", "Zecca Gigante", "Una zecca grossa come un piatto attende su una roccia.", "Corpo gonfio di sangue altrui.", "Uno schiocco umido di zecca ti fa sussultare.", "Da qui vicino arriva uno schiocco umido e nauseante."),
            _A("RACE_INSECT", "mantide caverna", "Mantide di Caverna", "Una mantide verde scuro alza le zampe a forbice.", "Sguardo alieno non tradisce alcuna emozione.", "Un fruscio secco di zampe di mantide ti circonda.", "In lontananza odi un fruscio secco di insetto."),
            _A("RACE_INSECT", "ape caverna", "Ape di Caverna", "Un'ape grossa come un pugno ronzia minacciosa.", "Pungiglione lungo gocciola veleno.", "Un ronzio furioso di ape di caverna riempie l'aria.", "Odi in lontananza un ronzio furioso di api."),
            _A("RACE_INSECT", "locusta bruna", "Locusta Bruna", "Una locusta grossa come un falco plana bassa.", "Le mandibole possono tagliare cuoio.", "Ali di locusta frusciano a un passo da te.", "In lontananza ali di locusta frusciano nel buio."),
            _A("RACE_ARACHNID", "ragno cacciatore", "Ragno Cacciatore", "Un ragno veloce corre sulle pareti.", "Veleno paralizzante nelle zanne.", "Passi leggeri di ragno sulla roccia ti sfiorano.", "Ti pare di scorgere passi leggeri sulla pietra."),
            _A("RACE_INSECT", "coleottero nero", "Coleottero Nero", "Un coleottero nero lucido grande come un gatto avanza.", "Le corna possono sollevare un uomo.", "Un ronzio metallico di coleottero vibra nella sala.", "Da qui vicino arriva un ronzio metallico."),
            _A("RACE_INSECT", "scarafaggio caverna", "Scarafaggio di Caverna", "Uno scarafaggio immenso striscia dal buio.", "Le antenne captano ogni odore.", "Un fruscio viscido di scarafaggio ti passa ai piedi.", "Odi in lontananza un fruscio viscido."),
        ]
    )

    cats.append(
        [
            _A("RACE_SNAKE", "vipera caverna", "Vipera di Caverna", "Una vipera striata si avvolge su una roccia calda.", "Zanne gocciolano veleno giallo.", "Un sibilo secco di vipera ti fa fermare.", "In lontananza un sibilo secco di serpente echeggia."),
            _A("RACE_SNAKE", "serpente ossa", "Serpente delle Ossa", "Un serpente bianco striscia tra teschi.", "Occhi rosa privi di pupilla.", "Ossa di serpente rotolano sul pavimento vicino a te.", "Da qui vicino arriva il rotolio di ossa serpentine."),
            _A("RACE_REPTILE", "lucertola gigante", "Lucertola Gigante", "Una lucertola grossa come un cane corre sulle pareti.", "Coda spessa colpisce come frusta.", "Artigli di lucertola graffiano la roccia a un passo.", "Odi in lontananza artigli che graffiano la pietra."),
            _A("RACE_REPTILE", "rana pietra", "Rana di Pietra", "Una rana massiccia mimetizzata tra i massi attende immobile.", "La pelle sembra pietra viva; solo gli occhi si muovono.", "Un colpo secco di lingua sulle pietre ti fa alzare lo sguardo.", "In lontananza odi un colpo secco sulle pietre."),
            _A("RACE_REPTILE", "geco caverna", "Geco di Caverna", "Un geco enorme mimetizzato attende sul soffitto.", "La pelle cambia colore con la roccia.", "Un fruscio improvviso di geco ti arriva da sopra.", "Da qui vicino arriva un fruscio improvviso."),
            _A("RACE_SNAKE", "boa caverna", "Boa di Caverna", "Un boa grosso come un tronco blocca il corridoio.", "Muscoli poderosi si contraggono.", "Un respiro sibilante di boa ti passa vicino.", "Odi in lontananza un respiro sibilante e pesante."),
            _A("RACE_REPTILE", "coccodrillo sotterraneo", "Coccodrillo Sotterraneo", "Un coccodrillo albino emerge da una pozza.", "Pelle traslucida e occhi rossi.", "Schizzi d'acqua di un coccodrillo ti bagnano.", "In lontananza odi schizzi d'acqua sotterranea."),
            _A("RACE_SNAKE", "aspide nero", "Aspide Nero", "Un serpente nero lucido si muove silenziosamente.", "Veleno letale in gocce alle zanne.", "Un sibilo quasi impercettibile di aspide ti sfiora.", "Ti pare di udire un sibilo sottilissimo."),
            _A("RACE_REPTILE", "basilisco giovane", "Basilisco Giovane", "Un basilisco immaturo evita il tuo sguardo.", "La pelle grigia simula pietra.", "Un sibilo roco di basilisco riempie la stanza.", "Da qui vicino arriva un sibilo roco e pesante."),
            _A("RACE_SNAKE", "serpente nebbia", "Serpente della Nebbia", "Un serpente traslucido emerge dal vapore.", "Il corpo sembra fatto di bruma solidificata.", "Vapore di nebbia si condensa e sibila intorno a te.", "Odi in lontananza vapore che si condensa con un sibilo."),
        ]
    )

    cats.append(
        [
            _A("RACE_PREDATOR", "predatore giovane", "Predatore delle Profondita'", "Un giovane predatore delle caverne ti fissa senza paura.", "Zanne non ancora mature, ma gia' affilate.", "Un ululato basso di predatore ti fa alzare lo sguardo.", "In lontananza un ululato di predatore si allunga."),
            _A("RACE_PREDATOR", "cane radici", "Cane delle Radici", "Un cane magro dalle zampe simili a radici ringhia basso.", "Pelliccia impregnata di terra e sangue secco.", "Un ringhio profondo di cane delle radici ti circonda.", "Odi in lontananza un ringhio profondo."),
            _A("RACE_PREDATOR", "warg caverna", "Warg di Caverna", "Un warg grosso come un cavallo ringhia nel buio.", "Pelliccia nera e zanne da lupo infernale.", "Un ululato cavernoso di warg riempie la sala.", "In lontananza un ululato cavernoso echeggia."),
            _A("RACE_PREDATOR", "lupo nebbia", "Lupo della Nebbia", "Un lupo spettrale emerge dal vapore.", "Occhi argentati non tradiscono paura.", "Un ululato nella bruma ti passa a fianco.", "Da qui vicino arriva un ululato nella nebbia."),
            _A("RACE_PREDATOR", "pantera caverna", "Pantera di Caverna", "Una pantera nera scivola senza rumore.", "Occhi verdi brillano nel buio.", "Un ringhio basso di pantera vibra nell'aria.", "Odi in lontananza un ringhio basso e felino."),
            _A("RACE_PREDATOR", "worg ibrido", "Worg Ibrido", "Un worg corrotto dalla dimensione ringhia in modo distorto.", "Zanne irregolari e saliva nera.", "Un ringhio metallico di worg ti fa sussultare.", "In lontananza un ringhio metallico risuona."),
            _A("RACE_PREDATOR", "sciacallo profondita", "Sciacallo delle Profondita'", "Uno sciacallo emaciato segue il gruppo.", "Occhi furbi attendono la distrazione.", "Un guaito acuto di sciacallo ti ferisce le orecchie.", "Da qui vicino arriva un guaito acuto."),
            _A("RACE_PREDATOR", "ratto gigante corrotto", "Ratto Gigante Corrotto", "Un ratto grosso come un cane mostra zanne innaturali.", "Pelliccia rada e occhi rossi.", "Squittii minacciosi di ratti giganti ti circondano.", "Odi in lontananza squittii minacciosi."),
            _A("RACE_PREDATOR", "tigre caverna", "Tigre di Caverna", "Una tigre dalle strisce pallide attende.", "Muscoli tesi pronti al balzo.", "Un ruggito sordo di tigre vibra nella stanza.", "In lontananza un ruggito sordo di grande felino echeggia."),
            _A("RACE_PREDATOR", "iena profondita", "Iena delle Profondita'", "Una iena macilenta ride mostrando fauci.", "Risata disturbante riempie la caverna.", "Una risata isterica di iena ti gela il sangue.", "Da qui vicino arriva una risata isterica di iena."),
        ]
    )

    cats.append(
        [
            _A("RACE_BIRD", "pipistrello vorace", "Pipistrello Vorace", "Un pipistrello grosso come un cane volteggia sopra la tua testa.", "Canini sporgono da un muso allungato e famelico.", "Un vortice d'aria di ali ti passa sul capo.", "Odi in lontananza un battito d'ali frenetico."),
            _A("RACE_BIRD", "pipistrello vampiro", "Pipistrello Vampiro", "Un pipistrello enorme plana con muso porcino.", "Lingua sanguinante lecca l'aria.", "Un ronzio famelico di pipistrello vampiro ti sfiora.", "In lontananza un ronzio famelico alato si avvicina."),
            _A("RACE_BIRD", "corvo caverna", "Corvo di Caverna", "Un corvo grosso osserva dall'ombra.", "Piume nere assorbono la luce.", "Un gracchio sinistro di corvo riempie la sala.", "Odi in lontananza un gracchio sinistro."),
            _A("RACE_BIRD", "avvoltoio caverne", "Avvoltoio delle Caverne", "Un avvoltoio grosso come un falco plana basso.", "Becco ad uncino pronto a squartare.", "Ali d'avvoltoio battono pesanti sopra di te.", "Da qui vicino arriva il battito di grandi ali."),
            _A("RACE_BIRD", "gufo caverna", "Gufo di Caverna", "Un gufo gigante fissa con occhi dorati.", "Ali silenziose planano nel buio.", "Un verso stridulo di gufo ti passa vicino.", "In lontananza un verso stridulo di gufo echeggia."),
            _A("RACE_BIRD", "arpia giovane", "Arpia Giovane", "Un'arpia immatura plana goffamente.", "Artigli non ancora maturi ma affilati.", "Un grido acuto di arpia giovane ti squarcia le orecchie.", "Odi in lontananza un grido d'arpia acuto."),
            _A("RACE_BIRD", "picchio ossidiana", "Picchio d'Ossidiana", "Un picchio dalle piume nere picchietta la roccia.", "Becco duro come acciaio.", "Un picchiettio ossessionante di becco sulla roccia ti segue.", "In lontananza odi un picchiettio ossessionante."),
            _A("RACE_BIRD", "condor profondita", "Condor delle Profondita'", "Un condor macilento scende dal soffitto.", "Ali ampie oscurano la luce.", "Un vento d'ali di condor ti investe il volto.", "Da qui vicino arriva un vento d'ali poderoso."),
            _A("RACE_BIRD", "rondini caverna", "Rondini di Caverna", "Rondini giganti volteggiano minacciose.", "Becchi corti ma veloci.", "Cinguettii cavernosi di rondini ti girano intorno.", "Odi in lontananza cinguettii cavernosi."),
            _A("RACE_BIRD", "fenice corrotta", "Fenice Corrotta", "Un uccello fiammeggiante corrotto dalla bruma volteggia.", "Piume nere e fiamme verdi.", "Un crepitio di fuoco di fenice corrotta riempie l'aria.", "In lontananza un crepitio di brace echeggia."),
        ]
    )

    cats.append(
        [
            _A("RACE_DWARF", "nano maledetto", "Nano Maledetto", "Un nano maledetto brandisce un'ascia runica.", "Barba incrostata di polvere e sangue.", "Imprecazioni furibonde di un nano maledetto ti circondano.", "Da qui vicino arriva un'imprecazione nanica furibonda."),
            _A("RACE_DWARF", "nano guardia", "Nano Guardia", "Un nano armato sorveglia un corridoio stretto.", "Elmo con visiera ribassata.", "L'armatura di una guardia nana stride a ogni passo.", "Odi in lontananza armatura nana che stride."),
            _A("RACE_DWARF", "nano scassinatore", "Nano Scassinatore", "Un nano con piccone e corda esplora.", "Occhi abituati al buio.", "Un piccone batte sulla roccia a un passo da te.", "In lontananza odi un piccone sulla pietra."),
            _A("RACE_DRAAGDIM", "duergar guerriero", "Duergar Guerriero", "Un duergar grigio brandisce un martello da guerra.", "Barba d'acciaio e sguardo ostile.", "Un grugnito duergar basso e duro riempie la sala.", "Da qui vicino arriva un grugnito duergar."),
            _A("RACE_DRAAGDIM", "duergar chierico", "Duergar Chierico", "Un duergar chierico invoca l'oscurita'.", "Simboli profani brillano sull'armatura.", "Preghiere profane di un chierico duergar ti passano vicino.", "Odi in lontananza preghiere duergar profane."),
            _A("RACE_GOLEM", "golem pietra minore", "Golem Minore di Pietra", "Un golem di pietra animato avanza goffamente.", "Rune incise pulsano debolmente.", "Pietre di un golem stridono l'una contro l'altra.", "In lontananza pietre vive stridono e si muovono."),
            _A("RACE_ENFAN", "enfan errante", "Enfan Errante", "Un enfan dalle orecchie lunghe brandisce una lama curva.", "Occhi antichi in volto giovane.", "Passi leggeri di un enfan errante ti sfiorano.", "Ti pare di scorgere passi leggeri oltre l'angolo."),
            _A("RACE_GNOME", "gnomo fessura", "Gnomo delle Fessure", "Un piccolo umanoide dalle mani artigliate ti osserva dall'ombra.", "Occhi troppo grandi brillano di malizia famelosa.", "Senti un riso stridulo di gnomo delle fessure.", "Da qui vicino arriva un riso stridulo."),
            _A("RACE_GNOME", "gnomo illusionista", "Gnomo Illusionista", "Un gnomo sotterraneo crea miraggi attorno a se'.", "Sorriso furbo e dita veloci.", "Risate distorte di illusionista ti girano intorno.", "Odi in lontananza risate distorte e false."),
            _A("RACE_SKEXIE", "skexie furtivo", "Skexie Furtivo", "Un skexie dalle movenze rapide brandisce delle daghe.", "Pelliccia scura lo mimetizza.", "Un fruscio rapido di skexie ti passa alle spalle.", "In lontananza un fruscio rapido si muove."),
        ]
    )

    cats.append(
        [
            _A("RACE_DARK_ELF", "drow esploratore", "Drow Esploratore", "Un drow incappucciato emerge dall'ombra.", "Lame d'ossidiana pronte a colpire.", "Sussurri elfici drow ti sfiorano l'orecchio.", "Da qui vicino arriva un sussurro elfico nero."),
            _A("RACE_DARK_ELF", "drow mago", "Drow Mago", "Un mago drow traccia rune nel buio.", "Energia viola danza sulle dita.", "Incantesimi sibilati di un mago drow riempiono l'aria.", "Odi in lontananza formule drow sibilate."),
            _A("RACE_DARK_ELF", "drow guerriero", "Drow Guerriero", "Un guerriero drow in armatura di mithril scuro avanza.", "Scudo e spada coordinati.", "Armatura elfica di un guerriero drow tintinna piano.", "In lontananza odi il tintinnio di armatura elfica."),
            _A("RACE_ELVEN", "elfo ombra", "Elfo Ombra", "Un elfo corrotto dalla bruma brandisce un arco lungo.", "Occhi vuoti e movenze fluide.", "Una corda d'arco d'ombra vibra nella stanza.", "Da qui vicino arriva il fischio di frecce nel buio."),
            _A("RACE_ELVEN", "elfo esiliato", "Elfo Esiliato", "Un elfo esiliato combatte con spada sottile.", "Mantello strappato e sguardo amaro.", "Passi silenziosi di un elfo esiliato ti sfiorano.", "Ti pare di scorgere passi silenziosi."),
            _A("RACE_PLANAR", "fey corrotta", "Fey Corrotta", "Una fey corrotta danza nel buio.", "Ali spezzate e sorriso crudele.", "Risate argentine e false di una fey ti circondano.", "Odi in lontananza risate argentine e false."),
            _A("RACE_GHOST", "elfo spettrale", "Elfo Spettrale", "Un elfo morto da secoli vaga senza riposo.", "Armatura elfica ancora lucente.", "Un lamento elfico spettrale ti passa attraverso.", "In lontananza un lamento elfico si allunga."),
            _A("RACE_DARK_ELF", "drow assassino", "Drow Assassino", "Un assassino drow attende immobile.", "Veleno gocciola dalla lama.", "Un silenzio mortale ti avverte di qualcosa alle spalle.", "Ti pare di scorgere un movimento silenzioso."),
            _A("RACE_ELVEN", "elfo custode", "Elfo Custode", "Un custode elfico protegge un passaggio antico.", "Rune elfiche brillano sull'arco.", "Canti elfici di un custode ti riempiono le orecchie.", "Odi in lontananza canti elfici."),
            _A("RACE_PLANAR", "ombra fey", "Ombra Fey", "Un'ombra fey si stacca dal muro danzando.", "Forma umanoide indistinta.", "Sussurri musicali di un'ombra fey ti avvolgono.", "Da qui vicino arriva un sussurro musicale e gelido."),
        ]
    )

    cats.append(
        [
            _A("RACE_TROLL", "troll caverna", "Troll di Caverna", "Un troll goffo si rialza sbavando.", "Pelle verde e rigenerazione visibile.", "Un grugnito di troll di caverna rimbomba nella sala.", "In lontananza un grugnito di troll echeggia."),
            _A("RACE_TROLL", "troll ricucito", "Troll Ricucito", "Un troll ricucito grossolanamente avanza.", "Cicatrici e punti visibili ovunque.", "Risate gutturali di un troll ricucito ti circondano.", "Odi in lontananza risate gutturali di troll."),
            _A("RACE_GIANT", "gigante caverna", "Gigante di Caverna", "Un gigante curvo si muove nel corridoio.", "Clava di tronco sollevata.", "Passi tremendi di un gigante fanno ballare la polvere.", "In lontananza passi tremendi scuotono la roccia."),
            _A("RACE_GIANT_HILL", "gigante colline", "Gigante delle Colline", "Un gigante delle colline blocca la volta.", "Pelle coriacea e odore di muschio.", "Rumore di massi spostati da un gigante ti arriva vicino.", "Da qui vicino arriva il rumore di massi che rotolano."),
            _A("RACE_GIANT_STONE", "gigante pietra", "Gigante di Pietra", "Un gigante di pietra si muove lentamente.", "Pelle simile a granito.", "Pietre cadono mentre un gigante di pietra si muove.", "Odi in lontananza pietre che cadono."),
            _A("RACE_HALF_OGRE", "mezzogre guardia", "Mezzogre Guardia", "Un mezzogre armato sorveglia un varco.", "Testa piccola e braccia enormi.", "Un grugnito orchesco di mezzogre riempie la stanza.", "In lontananza un grugnito orchesco risuona."),
            _A("RACE_HALF_OGRE", "mezzogre berserker", "Mezzogre Berserker", "Un mezzogre impazzito brandisce una mazza.", "Occhi rossi e muscoli tesi dalla rabbia.", "Un ansimare furioso di berserker mezzogre ti passa vicino.", "Da qui vicino arriva un'urla di furia mezzogre."),
            _A("RACE_TYTAN", "titano caduto", "Titano Caduto", "Un titano decaduto si trascina inginocchiato.", "Corpo enorme ma curvo dal tempo.", "Un tremore tellurico di passi titanici ti scuote.", "In lontananza un tremore tellurico si propaga."),
            _A("RACE_GIANT", "gigante mutilato", "Gigante Mutilato", "Un gigante con arti mancanti combatte comunque.", "Ferite antiche non guariscono.", "Un lamento profondo di gigante mutilato ti circonda.", "Odi in lontananza un lamento profondo e spezzato."),
            _A("RACE_TROLL", "troll nebbia", "Troll della Nebbia", "Un troll avvolto in bruma attacca goffamente.", "Pelle grigia e occhi vitrei.", "Un grugnito ovattato di troll della nebbia ti arriva.", "In lontananza un grugnito ovattato si trascina."),
        ]
    )

    cats.append(
        [
            _A("RACE_DEMON", "imp caverna", "Imp di Caverna", "Un imp alato volteggia lanciando scintille.", "Coda punge come una vespa.", "Risate acute di un imp di caverna ti feriscono le orecchie.", "Da qui vicino arriva una risata acuta di imp."),
            _A("RACE_DEMON", "demone minore", "Demone Minore", "Un demone cornuto giovane striscia dal buio.", "Zolfo e cenere lo circondano.", "Un odore di zolfo di demone minore ti prende alla gola.", "Odi in lontananza un tanfo di zolfo."),
            _A("RACE_DEVIL", "diavolo fessure", "Diavolo delle Fessure", "Un diavolo piccolo brandisce un tridente.", "Pelle rossa e occhi gialli.", "Risate infernali di un diavolo delle fessure ti circondano.", "In lontananza risate infernali echeggiano."),
            _A("RACE_ELEMENT", "elementale terra", "Elementale di Terra", "Un elementale di terra si rialza dal pavimento.", "Pietre e polvere compongono il corpo.", "Pietre cadono mentre un elementale di terra si muove.", "Da qui vicino arriva il tonfo di pietre vive."),
            _A("RACE_ELEMENT", "elementale acqua", "Elementale d'Acqua", "Un elementale d'acqua scorre come marea.", "Corpo trasparente e gelido.", "Schizzi d'acqua di un elementale ti bagnano.", "Odi in lontananza schizzi d'acqua elementale."),
            _A("RACE_ELEMENT", "elementale fuoco", "Elementale di Fuoco", "Un elementale di fuoco danza nel buio.", "Fiamme verdi corrompono il fuoco.", "Un crepitio di fiamme di elementale riempie la sala.", "In lontananza un crepitio di fiamme echeggia."),
            _A("RACE_SLIME", "gelatina caverna", "Gelatina di Caverna", "Una gelatina traslucida striscia sul pavimento.", "Ossa galleggiano nel corpo viscido.", "Schizzi viscidi di gelatina ti cadono ai piedi.", "Da qui vicino arriva lo schizzo di qualcosa di viscoso."),
            _A("RACE_SLIME", "muffa vivente", "Muffa Vivente", "Una muffa senziente si espande sulle pareti.", "Spore verdi riempiono l'aria.", "Una tosse e starnuti provocati da spore ti prendono.", "Odi in lontananza tosse e starnuti nel buio."),
            _A("RACE_DEMON", "manes caverna", "Manes di Caverna", "Un manes distorto striscia come fumo solido.", "Il volto muta continuamente.", "Lamenti distorti di un manes ti passano vicino.", "In lontananza lamenti distorti si trascinano."),
            _A("RACE_DEVIL", "diavolo catena", "Diavolo a Catena", "Un diavolo trascina catene infuocate.", "Ferite bruciano al contatto.", "Uno scroscio di catene di un diavolo riempie l'aria.", "Odi in lontananza catene che scrosciano."),
        ]
    )

    cats.append(
        [
            _A("RACE_MFLAYER", "illithid giovane", "Illithid Giovane", "Un illithid immaturo tenta di entrare nella tua mente.", "I tentacoli facciali si contorcono.", "Sussurri psichici di un illithid ti premono la mente.", "Da qui vicino arriva un sussurro psichico."),
            _A("RACE_SPECIAL", "mimica giovane", "Mimica Giovane", "Una cassa si apre mostrando le fauci.", "Lingua viscida lecca l'aria.", "Un tonfo di legno di mimica ti fa alzare lo sguardo.", "In lontananza odi un tonfo di legno innaturale."),
            _A("RACE_SPECIAL", "mimica scrigno", "Mimica Scrigno", "Uno scrigno dorato attende troppo comodo per essere vero.", "La serratura sembra un occhio vivo.", "Uno scricchiolio metallico di scrigno vivo vibra qui.", "Odi in lontananza uno scricchiolio metallico."),
            _A("RACE_PRIMATE", "grimlock guerriero", "Grimlock Guerriero", "Un grimlock cieco percepisce il tuo odore.", "Pelle grigia e denti affilati.", "Grugniti cavernosi di grimlock riempiono la stanza.", "In lontananza grugniti cavernosi di ciechi echeggiano."),
            _A("RACE_PRIMATE", "grimlock cacciatore", "Grimlock Cacciatore", "Un cacciatore grimlock trascina lancia di osso.", "Percezione acustica terrificante.", "Passi silenziosi di un cacciatore grimlock ti sfiorano.", "Ti pare di scorgere passi silenziosi di cacciatore."),
            _A("RACE_PARASITE", "parassita caverna", "Parassita di Caverna", "Un parassita grosso come un gatto si lancia addosso.", "Corpo viscido e ventose.", "Schizzi viscidi di un parassita ti cadono addosso.", "Da qui vicino arriva uno schizzo viscoso e nauseante."),
            _A("RACE_ASTRAL", "creatura astrale minore", "Creatura Astrale Minore", "Una creatura planare fluttua in modo instabile.", "La forma cambia continuamente.", "Echi astrali lontani e strani ti vibrano nelle ossa.", "Odi in lontananza echi astrali impossibili."),
            _A("RACE_PLANAR", "ombra planare", "Ombra Planare", "Un'ombra planare si stacca dal muro.", "Un freddo che non appartiene a questo mondo.", "Un silenzio planare innaturale ti stringe le orecchie.", "In lontananza un silenzio innaturale si allarga."),
            _A("RACE_SPECIAL", "aberrazione caverna", "Aberrazione di Caverna", "Una creatura innominabile striscia dal buio.", "Troppi occhi e troppe bocche.", "Suoni disturbanti di un'aberrazione ti fanno rabbrividire.", "Da qui vicino arriva un suono che non dovrebbe esistere."),
            _A("RACE_MFLAYER", "illithid servitore", "Illithid Servitore", "Un servitore illithid obbedisce a un padrone lontano.", "I tentacoli tengono una lama psionica.", "Pulsazioni mentali di un servitore illithid ti premono.", "Odi in lontananza pulsazioni mentali."),
        ]
    )

    cats.append(
        [
            _A("RACE_VEGMAN", "servitore muschio", "Servitore di Muschio", "Un umanoide ricoperto di muschio blocca il passaggio.", "Radici sottili serpeggiano lungo le sue gambe.", "La terra scricchiola sotto un servitore di muschio.", "In lontananza la terra scricchiola sotto passi vegetali."),
            _A("RACE_VEGMAN", "guardiano radici", "Guardiano delle Radici", "Un umanoide vegetale sorveglia un passaggio.", "Radici emergono dal pavimento.", "Un fruscio di foglie di guardiano delle radici ti circonda.", "Da qui vicino arriva un fruscio di foglie vive."),
            _A("RACE_TREE", "treant giovane", "Treant Giovane", "Un treant giovane si muove goffamente.", "Rami sfiorano il soffitto.", "Legno di treant scricchiola e geme vicino a te.", "Odi in lontananza legno vivo che scricchiola."),
            _A("RACE_VEGGIE", "pianta carnivora", "Pianta Carnivora", "Una pianta carnivora attende con fauci aperte.", "Polline dolce maschera pericolo.", "Un fruscio di foglie carnivore ti passa ai piedi.", "In lontananza un fruscio di foglie affamate echeggia."),
            _A("RACE_VEGGIE", "liane strangolatrici", "Liane Strangolatrici", "Liane animate pendono dal soffitto.", "Steli si avvolgono come serpenti.", "Un fruscio secco di liane ti sfiora le caviglie.", "Da qui vicino arriva un fruscio secco di liane."),
            _A("RACE_GHOST", "eco nebbia", "Eco della Nebbia", "Un'eco spettrale della dimensione vaga, confusa.", "Forma muta come vapore.", "Sussurri ovattati di un'eco di nebbia ti avvolgono.", "Odi in lontananza sussurri ovattati nella bruma."),
            _A("RACE_ELEMENT", "nebbia vivente", "Nebbia Vivente", "Una nube di bruma assume forma umanoide.", "Occhi argentati nel vapore.", "Un silenzio gelido di nebbia vivente ti stringe.", "In lontananza un silenzio gelido si addensa."),
            _A("RACE_GHOST", "spettro nebbia", "Spettro della Nebbia", "Uno spettro nato dalla bruma fluttua.", "Corpo semitrasparente.", "Un vento gelido di spettro della nebbia ti investe.", "Da qui vicino arriva un vento gelido nella bruma."),
            _A("RACE_PREDATOR", "predatore bruma", "Predatore della Bruma", "Un predatore nato nel velo caccia in silenzio.", "Pelliccia grigia e occhi argentati.", "Un ringhio ovattato di predatore della bruma ti passa vicino.", "Odi in lontananza un ringhio ovattato."),
            _A("RACE_INSECT", "libellula nebbia", "Libellula della Nebbia", "Una libellula traslucida volteggia nel vapore.", "Ali sottili tagliano l'aria.", "Un ronzio sottile di libellula della nebbia ti sfiora.", "In lontananza un ronzio sottile alato echeggia."),
        ]
    )

    return cats


def get_bosses() -> list[Archetype]:
    from procarea_mob_catalog_ext import (
        apply_base_boss_themes,
        get_epic_bosses,
    )

    return apply_base_boss_themes(get_base_bosses()) + get_epic_bosses()


def get_mobs() -> list[Archetype]:
    from procarea_mob_catalog_ext import apply_mob_category_themes

    mobs = apply_mob_category_themes(_mob_categories())
    if len(mobs) != MOB_COUNT:
        raise ValueError(f"Expected {MOB_COUNT} mobs, got {len(mobs)}")
    return mobs


def get_all_traps() -> list[Archetype]:
    from procarea_mob_catalog_ext import get_traps

    traps = get_traps()
    if len(traps) != TRAP_COUNT:
        raise ValueError(f"Expected {TRAP_COUNT} traps, got {len(traps)}")
    return traps


def get_all_archetypes() -> list[Archetype]:
    return get_bosses() + get_mobs() + get_all_traps()


def archetype_count() -> int:
    return len(get_all_archetypes())
