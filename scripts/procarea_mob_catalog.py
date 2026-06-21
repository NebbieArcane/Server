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
            "L'Alfa ringhia e la nebbia si addensa.",
            "Un ringhio squarcia il silenzio.",
        ),
        _A(
            "RACE_ARACHNID",
            "regina tarantole",
            "Regina delle Tarantole",
            "Una tarantola colossale domina un nido di seta e ossa.",
            "Setole urticanti coprono zampe spesse come tronchi.",
            "La Regina scatta lateralmente.",
            "Ragnatele vibrano.",
        ),
        _A(
            "RACE_TREE",
            "guardiano muschio antico",
            "Guardiano di Muschio Antico",
            "Un colosso vegetale blocca la sala con radici animate.",
            "Muschio vivo pulsa come un secondo cuore.",
            "Il Guardiano schianta i pugni a terra.",
            "Radici scoppiano fuori.",
        ),
        _A(
            "RACE_PREDATOR",
            "predatore cieco",
            "Predatore Cieco delle Profondita'",
            "Una bestia senza occhi percepisce ogni tuo movimento.",
            "Artigli lunghi segnano le pareti.",
            "Il predatore si lancia d'istinto.",
            "Un urlo cavernoso.",
        ),
        _A(
            "RACE_INSECT",
            "matriarca vermi",
            "Matriarca dei Vermi Bianchetti",
            "Una montagna gelatinosa regge un trono di larve.",
            "Vermi obbedienti si gettano contro gli intrusi.",
            "La Matriarca convoca vermi.",
            "Schizzi viscidi.",
        ),
        _A(
            "RACE_BIRD",
            "signore pipistrelli",
            "Signore dei Pipistrelli Profondi",
            "Una creatura alata incornata di ossa volteggia basso.",
            "Canini sporgono da un muso allungato e famelico.",
            "Il Signore piomba dal soffitto.",
            "Ali battono ovunque.",
        ),
        _A(
            "RACE_INSECT",
            "capo saprofagi",
            "Capo dei Saprofagi",
            "Un insettoide putrefatto comanda sciami affamati.",
            "Mandibole gocciolano liquido nero corrosivo.",
            "Il Capo spruzza acido.",
            "Odore di carogne.",
        ),
        _A(
            "RACE_ORC",
            "custode fosso",
            "Custode del Fosso",
            "Un guerriero rannicchiato sorveglia un baratro laterale.",
            "Armatura e' incrostata di fango secco e sangue.",
            "Il Custode spinge verso il vuoto.",
            "Pietre cadono nel baratro.",
        ),
        _A(
            "RACE_INSECT",
            "re larve",
            "Re delle Larve",
            "Un sovrano viscido e' nato dal cumulo di generazioni.",
            "Occhi multipli si aprono e chiudono a ondate.",
            "Il Re convoca larve reali.",
            "Brusio innaturale.",
        ),
        _A(
            "RACE_GHOST",
            "ombra cunicolo",
            "Ombra del Primo Cunicolo",
            "La prima ombra nata nella dimensione si stacca dal muro.",
            "Non ha volto: solo fame antica.",
            "L'Ombra ti avvolge.",
            "Il freddo morde le ossa.",
        ),
        _A(
            "RACE_UNDEAD_LICH",
            "arconte catacombe",
            "Arconte delle Catacombe",
            "Un lich avvolto in fasce polverose troneggia su un sepolcro aperto.",
            "Rune necromantiche bruciano come brace nel vuoto degli occhi.",
            "L'Arconte scaglia un incantesimo gelido.",
            "Sussurri in lingue morte.",
        ),
        _A(
            "RACE_UNDEAD_SPECTRE",
            "signore nazgul caduto",
            "Signore dei Nazgul Caduti",
            "Uno spettro corazzato di nero emerge dalla bruma come un re senza corona.",
            "Un anello spezzato gli pende dal collo spettrale.",
            "Il Signore urla un comando senza voce.",
            "Un fischio gelido tra le pietre.",
        ),
        _A(
            "RACE_ARACHNID",
            "madre ragni ungol",
            "Madre dei Ragni di Ungol",
            "Un'aracnide titanica attende nel buio filando seta nera.",
            "Occhi multipli brillano come stelle malevole.",
            "La Madre scaglia filamenti appiccicosi.",
            "Ragnatele si tendono all'improvviso.",
        ),
        _A(
            "RACE_GOBLIN",
            "capobanda goblin profondita",
            "Capobanda Goblin delle Profondita'",
            "Un goblin bardato di ferro di recupero brandisce un'ascia dentata.",
            "Trofei di orecchie pendono dalla sua cintura.",
            "Il Capobanda fischia ai suoi.",
            "Risate acute e feroci.",
        ),
        _A(
            "RACE_TROLL",
            "signore troll pietra",
            "Signore Troll di Pietra",
            "Un troll massiccio si rialza da una pozza di sangue coagulato.",
            "La pelle sembra granito vivo; le ferite si richiudono da sole.",
            "Il Signore Troll schianta il pugno a terra.",
            "Pietre rotolano nel buio.",
        ),
        _A(
            "RACE_UNDEAD_WIGHT",
            "custode barrow wight",
            "Custode Barrow-Wight",
            "Un guerriero sepolcrale sorge da un tumulo di ossa antiche.",
            "Spada arrugginita e corona spezzata lo contraddistinguono.",
            "Il Custode invoca il gelo del sepolcro.",
            "Un lamento di tombe profonde.",
        ),
        _A(
            "RACE_DRAGON_GREEN",
            "drago caverne giade",
            "Drago delle Caverne Giade",
            "Un drago giovane ma feroce riempie la caverna di vapore acido.",
            "Scaglie verdastre raccolgono condensa velenosa.",
            "Il Drago soffia un alito corrosivo.",
            "Un ruggito fa tremare la volta.",
        ),
        _A(
            "RACE_BIRD",
            "matriarca arpie rupi",
            "Matriarca Arpia delle Rupi",
            "Un'arpia regale plana su artigli insanguinati.",
            "Piume nere e occhi d'ambra la rendono un incubo alato.",
            "La Matriarca piomba urlando.",
            "Un grido straziante echeggia.",
        ),
        _A(
            "RACE_ORC",
            "signore uruk profondi",
            "Signore degli Uruk Profondi",
            "Un capitano uruk-corvo brandisce una lama larga come un'ascia.",
            "Rune di guerra bruciano sulla sua pelle grigio-ferrea.",
            "Il Signore ordina la carica.",
            "Cori di guerra sotterranei.",
        ),
        _A(
            "RACE_TREE",
            "ent corrotto antico",
            "Antico Ent Corrotto",
            "Un ent contorto dalla bruma torce rami come mazze.",
            "Corteccia nera stilla linfa velenosa.",
            "L'Ent schianta i rami verso di te.",
            "Legno che scricchiola in modo minaccioso.",
        ),
        _A(
            "RACE_DEMON",
            "demone fessure",
            "Demone delle Fessure",
            "Un demone cornuto striscia fuori da una crepa nel muro.",
            "Zolfo e cenere aleggiano intorno a lui.",
            "Il Demone scaglia fiamme infernali.",
            "Odore di zolfo bruciacchiato.",
        ),
        _A(
            "RACE_UNDEAD_VAMPIRE",
            "signore vampiro bruma",
            "Signore Vampiro della Bruma",
            "Un vampiro aristocratico emerge dalla nebbia con mantello lacero.",
            "Canini affilati e occhi cremisi non tradiscono l'eta'.",
            "Il Signore si scaglia famelicamente.",
            "Un sussurro seducente e mortale.",
        ),
        _A(
            "RACE_GIANT_HILL",
            "gigante radici",
            "Gigante delle Radici",
            "Un gigante delle colline blocca il passaggio con un tronco nodoso.",
            "Radici gli crescono dalle caviglie fino al ginocchio.",
            "Il Gigante schianta il tronco.",
            "Passi che fanno tremare la sala.",
        ),
        _A(
            "RACE_ELEMENT",
            "signore elementale fango",
            "Signore Elementale del Fango",
            "Una massa di fango senziente assume forma umanoide.",
            "Pietre e ossa galleggiano nella sua corrente viscida.",
            "Il Signore inghiotte il suolo sotto i tuoi piedi.",
            "Schizzi di fango ovunque.",
        ),
        _A(
            "RACE_DARK_ELF",
            "regina rova nera",
            "Regina della Rova Nera",
            "Una drow incappucciata troneggia su un trono di stalattiti.",
            "Lame di ossidiana le scintillano attorno come aureola.",
            "La Regina ordina l'esecuzione.",
            "Incantesimi sibilati nel buio.",
        ),
        _A(
            "RACE_DEVIL",
            "signore culto abissale",
            "Signore del Culto Abissale",
            "Un diavolo cornuto conduce un coro di fedeli mutati.",
            "Simboli infernali bruciano sulla pietra circostante.",
            "Il Signore invoca fiamme profane.",
            "Canti distorti e irreali.",
        ),
        _A(
            "RACE_GHOST",
            "oracolo corrotto moria",
            "Oracolo Corrotto di Moria",
            "Lo spirito di un saggio nano urla profezie nel buio.",
            "Libri fluttuano attorno a lui come lune orbitanti.",
            "L'Oracolo scaglia rune spettrali.",
            "Echi di martelli lontani.",
        ),
        _A(
            "RACE_SPECIAL",
            "signore mimiche",
            "Signore delle Mimiche",
            "Una creatura che imita un trono e un tesoro attende immobile.",
            "La pelle sembra metallo finche' non si apre a bocca piena.",
            "Il Signore si slancia con fauci d'acciaio.",
            "Un tonfo sordo di carne che diventa pietra.",
        ),
        _A(
            "RACE_DEMON",
            "custode balrog minore",
            "Custode del Balrog Minore",
            "Una creatura di fuoco e ombra brandisce una frusta di fiamme.",
            "Ali carbonizzate spalancano calore insopportabile.",
            "Il Custode flagella l'aria ardente.",
            "Un ruggito di magma sotterraneo.",
        ),
        _A(
            "RACE_UNDEAD_GHAST",
            "sovrano ghoul ossa",
            "Sovrano Ghoul delle Ossa",
            "Un ghoul reale si rialza da un trono di femori intrecciati.",
            "Artigli neri e lingua biforcuta gocciolano ichor.",
            "Il Sovrano convoca ghoul affamati.",
            "Masticare ossa nel buio.",
        ),
    ]


def _mob_categories() -> list[list[Archetype]]:
    """150 mob in 15 categorie da 10."""
    cats: list[list[Archetype]] = []

    cats.append(
        [
            _A("RACE_GOBLIN", "goblin esploratore", "Goblin Esploratore", "Un goblin magro sbuca da un crepaccio con pugnale arrugginito.", "Orecchie tagliate pendono al collo come trofei.", "Il goblin si lancia urlando.", "Un urlo acuto."),
            _A("RACE_GOBLIN", "goblin sciamano", "Goblin Sciamano", "Un goblin coperto di rune brandisce un bastone di ossa.", "Occhi gialli brillano di follia sciamanesca.", "Lo sciamano invoca fulmini verdi.", "Mormori gutturali."),
            _A("RACE_GOBLIN", "goblin arcere", "Goblin Arciere", "Un goblin armato di arco corto mira dall'ombra.", "Frecce avvelenate spuntano dalla faretra.", "L'arciere scocca una freccia.", "Frecce che fischiano."),
            _A("RACE_GOBLIN", "goblin speleologo", "Goblin Speleologo", "Un goblin con lanterna di tartaruga esplora i cunicoli.", "Corde e uncini gli pendono dalla cintura.", "Lo speleologo attacca con uncino.", "Metallo che striscia sulla roccia."),
            _A("RACE_GOBLIN", "goblin ladro", "Goblin Ladro", "Un goblin furtivo fruga tra i detriti con occhi furbi.", "Dita nere e veloci non lasciano traccia.", "Il ladro pugnala alle spalle.", "Risate soffocate."),
            _A("RACE_GOBLIN", "hobgoblin soldato", "Hobgoblin Soldato", "Un hobgoblin in armatura di cuoio brandisce spada e scudo.", "Disciplina crudele si legge nei suoi occhi rossi.", "Il soldato carica in formazione.", "Passi marziali."),
            _A("RACE_GOBLIN", "hobgoblin capitano", "Hobgoblin Capitano", "Un hobgoblin bardato conduce un manipolo inferiore.", "Stendardo strappato ancora gli cinge il petto.", "Il capitano ordina l'assalto.", "Comandi ruvidi."),
            _A("RACE_ORC", "orc guerriero", "Orco Guerriero", "Un orco muscoloso ringhia mostrando zanne incarnate.", "Cicatrici rituali solcano il petto nudo.", "L'orco si scaglia con ascia.", "Grugniti di guerra."),
            _A("RACE_ORC", "orc berserker", "Orco Berserker", "Un orco impazzito da battaglia schiuma alla bocca.", "Ascia a due mani sollevata, occhi fuori orbita.", "Il berserker entra in furia.", "Urla di sangue."),
            _A("RACE_HALF_ORC", "uruk corsiero", "Uruk Corsiero", "Un uruk-corvo corre senza stancarsi lungo i corridoi.", "Elmo nero e lama larga lo rendono un'ombra letale.", "L'uruk piomba con lama bassa.", "Passi pesanti e rapidi."),
        ]
    )

    cats.append(
        [
            _A("RACE_ORC", "orc sciamano", "Orco Sciamano", "Un orco coperto di ossa invoca spiriti della caverna.", "Tatuaggi neri pulsano quando canta.", "Lo sciamano scaglia maledizioni.", "Canti gutturali."),
            _A("RACE_ORC", "orc guardia", "Orco Guardia", "Un orco armato sorveglia un varco stretto.", "Lancia arrugginita pronta a trafiggere.", "La guardia blocca il passaggio.", "Grugnito di sfida."),
            _A("RACE_ORC", "orc spezzaneve", "Orco Spezzaneve", "Un orco delle montagne emerge dal ghiaccio sotterraneo.", "Pelliccia imbrattata di sangue e brina.", "Lo spezzaneve colpisce con mazza.", "Vento gelido."),
            _A("RACE_HALF_ORC", "mezzorco reietto", "Mezzorco Reietto", "Un mezzorco emarginato combatte con disperazione.", "Occhi tristi e pugno di ferro.", "Il reietto attacca senza paura.", "Ringhio sommesso."),
            _A("RACE_GOBLIN", "goblin furtivo", "Goblin Furtivo", "Un goblin si mimetizza tra le stalattiti.", "Pelle grigia lo confonde con la roccia.", "Il furtivo colpisce dall'alto.", "Pietre che cadono."),
            _A("RACE_TROGMAN", "troglodita guerriero", "Troglodita Guerriero", "Un uomo-delle-caverne brandisce una clava d'osso.", "Pelle pallida e occhi ciechi al sole.", "Il troglodita colpisce alla cieca.", "Grugniti cavernosi."),
            _A("RACE_TROGMAN", "troglodita cacciatore", "Troglodita Cacciatore", "Un cacciatore sotterraneo trascina una lancia di selce.", "Odore di muschio e carne cruda.", "Il cacciatore trafigge con lancia.", "Passi silenziosi."),
            _A("RACE_ORC", "orc flagellatore", "Orco Flagellatore", "Un orco sadico brandisce frusta chiodata.", "Sorriso crudele sotto elmo deformato.", "Il flagellatore frusta l'aria.", "Scroscio di frusta."),
            _A("RACE_GOBLIN", "goblin alchimista", "Goblin Alchimista", "Un goblin maneggia fiale fumanti con cautela.", "Meta' del volto e' bruciata da esplosioni recenti.", "L'alchimista lancia acido.", "Vetro che si rompe."),
            _A("RACE_HALF_ORC", "uruk sentinella", "Uruk Sentinella", "Un uruk immobile veglia su un ponte di pietra.", "Lancia lunga e scudo nero coprono ogni angolo.", "La sentinella carica.", "Armatura che stride."),
        ]
    )

    cats.append(
        [
            _A("RACE_UNDEAD_SKELETON", "scheletro guerriero", "Scheletro Guerriero", "Uno scheletro armato si rialza dal cumulo di ossa.", "Costole vuote battono come tamburi.", "Lo scheletro falcia con spada.", "Ossa che sbattono."),
            _A("RACE_UNDEAD_SKELETON", "scheletro arciere", "Scheletro Arciere", "Uno scheletro tende un arco di ossa e tendini.", "Frecce di metallo arrugginito puntano verso di te.", "L'arciere scocca.", "Frecce nel buio."),
            _A("RACE_UNDEAD_ZOMBIE", "zombie contadino", "Zombie Contadino", "Un cadavere rigido avanza con braccia tese.", "Vestiti laceri e occhi vitrei.", "Lo zombie cerca di afferrarti.", "Gemiti soffocati."),
            _A("RACE_UNDEAD_ZOMBIE", "zombie soldato", "Zombie Soldato", "Un soldato morto marcia ancora con spada in mano.", "Armatura ammaccata copre carne putrefatta.", "Lo zombie colpisce goffamente.", "Armatura che stride."),
            _A("RACE_UNDEAD_GHOUL", "ghoul famelico", "Ghoul Famelico", "Un ghoul striscia a quattro zampe verso di te.", "Artigli neri e lingua biforcuta.", "Il ghoul morde famelicamente.", "Masticare carne."),
            _A("RACE_UNDEAD_GHOUL", "ghoul necrofago", "Ghoul Necrofago", "Un ghoul rovista tra tombe aperte.", "Denti affilati gocciolano ichor nero.", "Il necrofago si slancia.", "Odore di sepolcro."),
            _A("RACE_UNDEAD_WIGHT", "wight guerriero", "Wight Guerriero", "Un wight spettrale brandisce spada gelida.", "Aura di freddo mortale lo circonda.", "Il wight colpisce con spada.", "Freddo che penetra le ossa."),
            _A("RACE_UNDEAD_SPECTRE", "spettro errante", "Spettro Errante", "Uno spettro fluttua attraverso le pareti.", "Voce sussurrante promette morte.", "Lo spettro attraversa la tua difesa.", "Sussurri gelidi."),
            _A("RACE_GHOST", "fantasma prigioniero", "Fantasma Prigioniero", "Un fantasma incatenato alla caverna urla silenziosamente.", "Catene spettrali gli legano polsi e caviglie.", "Il fantasma stringe con catene.", "Lamenti ovattati."),
            _A("RACE_UNDEAD_LICH", "lich minore", "Lich Minore", "Un lich incompleto alza mani scheletriche.", "Pergamene bruciano attorno a lui.", "Il lich scaglia dardi oscuri.", "Incantesimi sibilati."),
        ]
    )

    cats.append(
        [
            _A("RACE_UNDEAD_SKELETON", "scheletro guardia", "Scheletro Guardia", "Uno scheletro in armatura sorveglia un arco.", "Elmo vuoto fissa il vuoto con occhi di brace.", "La guardia blocca il passaggio.", "Spade che si incrociano."),
            _A("RACE_UNDEAD_ZOMBIE", "zombie ogre", "Zombie Ogre", "Un ogre morto si trascina con mazza imponente.", "Carne verde e gonfia colma le ferite.", "Lo zombie ogre schianta la mazza.", "Passi pesanti e morti."),
            _A("RACE_UNDEAD_GHOUL", "ghoul delle profondita", "Ghoul delle Profondita'", "Un ghoul emaciato striscia dalle fessure.", "Occhi gialli brillano nel buio.", "Il ghoul attacca in branco.", "Ringhi famelici."),
            _A("RACE_UNDEAD_WIGHT", "wight sepolcrale", "Wight Sepolcrale", "Un wight sorge da un sarcofago rotto.", "Polsi scheletrici stringono una corona.", "Il wight invoca gelo.", "Vento gelido dal sepolcro."),
            _A("RACE_UNDEAD_SPECTRE", "spettro guerriero", "Spettro Guerriero", "Uno spettro in armatura antica fluttua in modo minaccioso.", "Spada spettrale attraversa la roccia.", "Lo spettro falcia.", "Lame che fischiano nel vuoto."),
            _A("RACE_GHOST", "anima perduta", "Anima Perduta", "Un'anima perduta vaga senza meta tra i corridoi.", "Luce pallida la rende quasi trasparente.", "L'anima ti attraversa gelida.", "Pianto lontano."),
            _A("RACE_UNDEAD_VAMPIRE", "vampiro servitore", "Vampiro Servitore", "Un vampiro giovane serve un padrone piu' antico.", "Mantello nero e occhi affamati.", "Il servitore morde rapidamente.", "Sussurro seducente."),
            _A("RACE_UNDEAD_SKELETON", "scheletro chierico", "Scheletro Chierico", "Uno scheletro in tonaca invoca l'oscurita'.", "Simboli sacri corrotti brillano sul petto.", "Lo scheletro scaglia maledizioni.", "Preghiere invertite."),
            _A("RACE_UNDEAD_ZOMBIE", "zombie paludoso", "Zombie Paludoso", "Un cadavere paludoso emerge da pozze viscide.", "Alghe gli pendono dalle ferite.", "Lo zombie afferra con mani viscide.", "Schizzi di acqua fetida."),
            _A("RACE_UNDEAD_GHAST", "ghast affamato", "Ghast Affamato", "Un ghast piu' potente di un ghoul striscia verso di te.", "Alito putrefatto precede l'attacco.", "Il ghast paralizza con lo sguardo.", "Odore insopportabile."),
        ]
    )

    cats.append(
        [
            _A("RACE_ARACHNID", "ragno caverna", "Ragno di Caverna", "Un ragno grosso come un cane attende nella ragnatela.", "Occhi neri multipli ti scrutano.", "Il ragno scatta.", "Ragnatele che si tendono."),
            _A("RACE_ARACHNID", "ragno veleno", "Ragno Velenoso", "Un ragno dalle zampe striate di giallo si muove rapido.", "Gocce di veleno brillano alle zanne.", "Il ragno morde.", "Sibilo acuto."),
            _A("RACE_ARACHNID", "ragno gigante", "Ragno Gigante", "Un ragno gigante copre meta' del soffitto.", "Seta fresca ancora umida di prede.", "Il ragno avvolge in seta.", "Ragnatele che vibrano."),
            _A("RACE_INSECT", "scarabeo corazza", "Scarabeo Corazzato", "Uno scarabeo grande come un vassoio avanza.", "Carapace lucido devia le lame.", "Lo scarabeo carica.", "Ronzio profondo."),
            _A("RACE_INSECT", "centopiedi caverna", "Centopiedi di Caverna", "Un centopiedi lungo come un braccio striscia sulle pareti.", "Mandibole gocciolano veleno.", "Il centopiedi morde.", "Scricchiolio di chitin."),
            _A("RACE_INSECT", "formica gigante", "Formica Gigante", "Una formica gigante trasporta un pezzo di armatura.", "Mandibole possono tranciare ossa.", "La formica morde.", "Ronzio di sciame."),
            _A("RACE_INSECT", "mosca necrofaga", "Mosca Necrofaga", "Una mosca grande come un pugno ronzia attorno.", "Occhi composti riflettono ogni movimento.", "La mosca punge.", "Ronzio insistente."),
            _A("RACE_INSECT", "grillo caverna", "Grillo di Caverna", "Un grillo nero grande come un gatto salta.", "Antenne tese captano ogni suono.", "Il grillo morde.", "Stridio acuto."),
            _A("RACE_ARACHNID", "vedova nera", "Vedova Nera", "Una vedova nera grossa attende immobile.", "Segno rosso sul dorso avverte pericolo.", "La vedova scatta.", "Silenzio prima del morso."),
            _A("RACE_INSECT", "sciamatore caverna", "Sciamatore di Caverna", "Un insetto corazzato fruga tra le pietre con antenne tese.", "Mandibole lucide portano tracce di ossa recenti.", "Lo sciamatore batte le ali.", "Un ronzio acuto riempie la stanza."),
        ]
    )

    cats.append(
        [
            _A("RACE_INSECT", "larva nebbia effimera", "Larva della Nebbia Effimera", "Una larva traslucida striscia lasciando una scia di vapore gelido.", "La pelle viscida pulsa come se respirasse nebbia compressa.", "La larva si inarca minacciosa.", "Senti un sibilo umido."),
            _A("RACE_INSECT", "verme bianchetto", "Verme Bianchetto", "Un verme bianco grosso come un braccio emerge dal fango.", "Pelle gelatinosa brilla di umidita'.", "Il verme si avvolge.", "Schizzi viscidi."),
            _A("RACE_INSECT", "mosca stirge", "Stirge delle Profondita'", "Una stirge alata plana con proboscide sanguinante.", "Ali sottili e occhi rossi.", "La stirge si attacca.", "Ronzio famelico."),
            _A("RACE_ARACHNID", "tick gigante", "Tick Gigante", "Un tick grosso come un piatto attende su una roccia.", "Corpo gonfio di sangue altrui.", "Il tick morde.", "Schiocco umido."),
            _A("RACE_INSECT", "mantide caverna", "Mantide di Caverna", "Una mantide verde scuro ferma le zampe a forbice.", "Sguardo alieno non tradisce alcuna emozione.", "La mantide falcia.", "Fruscio secco."),
            _A("RACE_INSECT", "ape caverna", "Ape di Caverna", "Un'ape grossa come un pugno ronzia minacciosa.", "Pungiglione lungo gocciola veleno.", "L'ape punge.", "Ronzio furioso."),
            _A("RACE_INSECT", "locusta bruna", "Locusta Bruna", "Una locusta grossa come un falco plana bassa.", "Mandibole possono tagliare cuoio.", "La locusta morde.", "Ali che frusciano."),
            _A("RACE_ARACHNID", "ragno cacciatore", "Ragno Cacciatore", "Un ragno veloce corre sulle pareti.", "Veleno paralizzante nelle zanne.", "Il cacciatore salta.", "Passi leggeri sulla roccia."),
            _A("RACE_INSECT", "coleottero nero", "Coleottero Nero", "Un coleottero nero lucido grande come un gatto.", "Corna possono sollevare un uomo.", "Il coleottero carica.", "Ronzio metallico."),
            _A("RACE_INSECT", "scarafaggio caverna", "Scarafaggio di Caverna", "Uno scarafaggio immenso striscia dal buio.", "Le antenne captano ogni odore.", "Lo scarafaggio attacca.", "Fruscio viscido."),
        ]
    )

    cats.append(
        [
            _A("RACE_SNAKE", "vipera caverna", "Vipera di Caverna", "Una vipera striata si avvolge su una roccia calda.", "Zanne gocciolano veleno giallo.", "La vipera morde.", "Sibilo secco."),
            _A("RACE_SNAKE", "serpente ossa", "Serpente delle Ossa", "Un serpente bianco striscia tra teschi.", "Occhi rosa privi di pupilla.", "Il serpente si avvolge.", "Ossa che rotolano."),
            _A("RACE_REPTILE", "lucertola gigante", "Lucertola Gigante", "Una lucertola grossa come un cane corre sulle pareti.", "Coda spessa colpisce come frusta.", "La lucertola morde.", "Artigli sulla roccia."),
            _A("RACE_REPTILE", "rana pietra", "Rana di Pietra", "Una rana massiccia mimetizzata tra i massi attende immobile.", "La pelle sembra pietra viva; solo gli occhi si muovono.", "La rana si scaglia con la lingua.", "Un colpo secco sulle pietre."),
            _A("RACE_REPTILE", "geco caverna", "Geco di Caverna", "Un geco enorme mimetizzato attende sul soffitto.", "La pelle cambia colore con la roccia.", "Il geco cade addosso.", "Fruscio improvviso."),
            _A("RACE_SNAKE", "boa caverna", "Boa di Caverna", "Un boa grosso come un tronco blocca il corridoio.", "Muscoli poderosi si contraggono.", "Il boa si avvolge.", "Respiro sibilante."),
            _A("RACE_REPTILE", "coccodrillo sotterraneo", "Coccodrillo Sotterraneo", "Un coccodrillo albino emerge da una pozza.", "Pelle traslucida e occhi rossi.", "Il coccodrillo morde.", "Schizzi d'acqua."),
            _A("RACE_SNAKE", "aspide nero", "Aspide Nero", "Un serpente nero lucido si muove silenziosamente.", "Veleno letale in gocce alle zanne.", "L'aspide morde rapidamente.", "Sibilo quasi impercettibile."),
            _A("RACE_REPTILE", "basilisco giovane", "Basilisco Giovane", "Un basilisco immaturo evita il tuo sguardo.", "Pelle grigia simula pietra.", "Il basilisco morde.", "Sibilo roco."),
            _A("RACE_SNAKE", "serpente nebbia", "Serpente della Nebbia", "Un serpente traslucido emerge dal vapore.", "Il corpo sembra fatto di bruma solidificata.", "Il serpente morde gelidamente.", "Vapore che si condensa."),
        ]
    )

    cats.append(
        [
            _A("RACE_PREDATOR", "predatore giovane", "Predatore delle Profondita'", "Un giovane predatore delle caverne ti fissa senza paura.", "Zanne non ancora mature, ma gia' affilate.", "Il predatore si prepara al balzo.", "Un ululato lontano."),
            _A("RACE_PREDATOR", "cane radici", "Cane delle Radici", "Un cane magro dalle zampe simili a radici ringhia basso.", "Pelliccia impregnata di terra e sangue secco.", "Il cane si lancia avanti.", "Un ringhio profondo."),
            _A("RACE_PREDATOR", "warg caverna", "Warg di Caverna", "Un warg grosso come un cavallo ringhia nel buio.", "Pelliccia nera e zanne da lupo infernale.", "Il warg carica.", "Ululato cavernoso."),
            _A("RACE_PREDATOR", "lupo nebbia", "Lupo della Nebbia", "Un lupo spettrale emerge dal vapore.", "Occhi argentati non tradiscono paura.", "Il lupo attacca in branco.", "Ululato nella bruma."),
            _A("RACE_PREDATOR", "pantera caverna", "Pantera di Caverna", "Una pantera nera scivola senza rumore.", "Occhi verdi brillano nel buio.", "La pantera piomba.", "Ringhio basso."),
            _A("RACE_PREDATOR", "ibrido worg", "Worg Ibrido", "Un worg corrotto dalla dimensione ringhia in modo distorto.", "Zanne irregolari e saliva nera.", "Il worg si scaglia.", "Ringhio metallico."),
            _A("RACE_PREDATOR", "sciacallo profondita", "Sciacallo delle Profondita'", "Uno sciacallo emaciato segue il gruppo.", "Occhi furbi attendono la distrazione.", "Lo sciacallo morde alle caviglie.", "Guaito acuto."),
            _A("RACE_PREDATOR", "ibrido ratto gigante", "Ratto Gigante Corrotto", "Un ratto grosso come un cane mostra zanne innaturali.", "Pelliccia rada e occhi rossi.", "Il ratto attacca in branco.", "Squitti minacciosi."),
            _A("RACE_PREDATOR", "tigre caverna", "Tigre di Caverna", "Una tigre dalle strisce pallide attende.", "Muscoli tesi pronti al balzo.", "La tigre attacca.", "Ruggito sordo."),
            _A("RACE_PREDATOR", "ibrido iena", "Iena delle Profondita'", "Una iena macilenta ride mostrando fauci.", "Risata disturbante riempie la caverna.", "L'iena morde.", "Risata isterica."),
        ]
    )

    cats.append(
        [
            _A("RACE_BIRD", "pipistrello vorace", "Pipistrello Vorace", "Un pipistrello grosso come un cane volteggia sopra la tua testa.", "Canini sporgono da un muso allungato e famelico.", "Il pipistrello piomba giu'.", "Ali scalciano l'aria."),
            _A("RACE_BIRD", "pipistrello vampiro", "Pipistrello Vampiro", "Un pipistrello enorme plana con muso porcino.", "Lingua sanguinante lecca l'aria.", "Il pipistrello morde.", "Ronzio famelico."),
            _A("RACE_BIRD", "corvo caverna", "Corvo di Caverna", "Un corvo grosso osserva dall'ombra.", "Piume nere assorbono la luce.", "Il corvo becca violentemente.", "Gracchio sinistro."),
            _A("RACE_BIRD", "caveira alata", "Caveira Alata", "Un uccello carogna grosso come un falco plana.", "Becco ad uncino pronto a squartare.", "La caveira attacca.", "Ali che battono."),
            _A("RACE_BIRD", "gufo caverna", "Gufo di Caverna", "Un gufo gigante fissa con occhi dorati.", "Ali silenziose planano nel buio.", "Il gufo artiglia.", "Verso stridulo."),
            _A("RACE_BIRD", "arpia giovane", "Arpia Giovane", "Un'arpia immatura plana goffamente.", "Artigli non ancora maturi ma affilati.", "L'arpia graffia.", "Grido acuto."),
            _A("RACE_BIRD", "picchio ossidiana", "Picchio d'Ossidiana", "Un picchio dalle piume nere picchietta la roccia.", "Becco duro come acciaio.", "Il picchio becca.", "Picchiettio ossessionante."),
            _A("RACE_BIRD", "condor profondita", "Condor delle Profondita'", "Un condor macilento scende dal soffitto.", "Ali ampie oscurano la luce.", "Il condor artiglia.", "Vento d'ali."),
            _A("RACE_BIRD", "rondine caverna", "Rondine di Caverna", "Rondini giganti volteggiano minacciose.", "Becco corto ma veloce.", "Le rondini attaccano in sciame.", "Cinguetti cavernosi."),
            _A("RACE_BIRD", "fenice corrotta", "Fenice Corrotta", "Un uccello fiammeggiante corrotto dalla bruma volteggia.", "Piume nere e braci verdi.", "La fenice scaglia braci.", "Crepitio di fuoco."),
        ]
    )

    cats.append(
        [
            _A("RACE_DWARF", "nano maledetto", "Nano Maledetto", "Un nano morto-vivente brandisce ascia runica.", "Barba incrostata di polvere e sangue.", "Il nano maledetto colpisce.", "Imprecazioni nane."),
            _A("RACE_DWARF", "nano guardia", "Nano Guardia", "Un nano armato sorveglia un corridoio stretto.", "Elmo con visiera ribassata.", "La guardia blocca.", "Armatura che stride."),
            _A("RACE_DWARF", "nano scassinatore", "Nano Scassinatore", "Un nano con piccone e corda esplora.", "Occhi abituati al buio.", "Lo scassinatore colpisce.", "Piccone sulla roccia."),
            _A("RACE_DRAAGDIM", "duergar guerriero", "Duergar Guerriero", "Un duergar grigio brandisce martello da guerra.", "Barba d'acciaio e sguardo ostile.", "Il duergar colpisce.", "Grugnito duergar."),
            _A("RACE_DRAAGDIM", "duergar chierico", "Duergar Chierico", "Un duergar chierico invoca l'oscurita'.", "Simboli profani brillano sull'armatura.", "Il chierico maledice.", "Preghiere profane."),
            _A("RACE_GOLEM", "golem pietra minore", "Golem di Pietra Minore", "Un golem di pietra animato avanza goffamente.", "Rune incise pulsano debolmente.", "Il golem colpisce.", "Pietre che stridono."),
            _A("RACE_ENFAN", "enfan errante", "Enfan Errante", "Un enfan dalle orecchie lunghe brandisce una lama curva.", "Occhi antichi in volto giovane.", "L'enfan attacca.", "Passi leggeri."),
            _A("RACE_GNOME", "gnomo fessura", "Gnomo delle Fessure", "Un piccolo umanoide dalle mani artigliate ti osserva dall'ombra.", "Occhi troppo grandi brillano di malizia famelosa.", "Lo gnomo sibila un'imprecazione.", "Senti un riso stridulo."),
            _A("RACE_GNOME", "gnomo illusionista", "Gnomo Illusionista", "Un gnomo sotterraneo crea miraggi attorno a se'.", "Sorriso furbo e dita veloci.", "L'illusionista confonde.", "Risate distorte."),
            _A("RACE_SKEXIE", "skexie furtivo", "Skexie Furtivo", "Un skexie dalle movenze rapide brandisce daghe.", "Pelliccia scura lo mimetizza.", "Lo skexie colpisce due volte.", "Fruscio rapido."),
        ]
    )

    cats.append(
        [
            _A("RACE_DARK_ELF", "drow esploratore", "Drow Esploratore", "Un drow incappucciato emerge dall'ombra.", "Lame d'ossidiana pronte a colpire.", "Il drow attacca rapidamente.", "Sussurri elfici."),
            _A("RACE_DARK_ELF", "drow mago", "Drow Mago", "Un mago drow traccia rune nel buio.", "Energia viola danza sulle dita.", "Il mago scaglia dardi.", "Incantesimi sibilati."),
            _A("RACE_DARK_ELF", "drow guerriero", "Drow Guerriero", "Un guerriero drow in armatura di mithril scuro.", "Scudo e spada coordinati.", "Il guerriero falcia.", "Armatura elfica."),
            _A("RACE_ELVEN", "elfo ombra", "Elfo Ombra", "Un elfo corrotto dalla bruma brandisce arco lungo.", "Occhi vuoti e movenze fluide.", "L'elfo scocca.", "Frecce nel buio."),
            _A("RACE_ELVEN", "elfo esiliato", "Elfo Esiliato", "Un elfo esiliato combatte con spada sottile.", "Mantello strappato e sguardo amaro.", "L'esiliato attacca.", "Passi silenziosi."),
            _A("RACE_PLANAR", "fey corrotto", "Fey Corrotto", "Una creatura fey distorta danza nel buio.", "Ali spezzate e sorriso crudele.", "Il fey confonde.", "Risate argentine e false."),
            _A("RACE_GHOST", "elfo spettrale", "Elfo Spettrale", "Un elfo morto da secoli vaga senza riposo.", "Armatura elfica ancora lucente.", "L'elfo spettrale colpisce.", "Lamento elfico."),
            _A("RACE_DARK_ELF", "drow assassino", "Drow Assassino", "Un assassino drow attende immobile.", "Veleno gocciola dalla lama.", "L'assassino colpisce alle spalle.", "Silenzio mortale."),
            _A("RACE_ELVEN", "elfo custode", "Elfo Custode", "Un custode elfico protegge un passaggio antico.", "Rune elfiche brillano sull'arco.", "Il custode scocca.", "Canti elfici lontani."),
            _A("RACE_PLANAR", "ombra fey", "Ombra Fey", "Un'ombra fey si stacca dal muro danzando.", "Forma umanoide indistinta.", "L'ombra avvolge.", "Sussurri musicali."),
        ]
    )

    cats.append(
        [
            _A("RACE_TROLL", "troll caverna", "Troll di Caverna", "Un troll goffo si rialza sbavando.", "Pelle verde e rigenerazione visibile.", "Il troll colpisce con clava.", "Grugnito troll."),
            _A("RACE_TROLL", "troll ricucito", "Troll Ricucito", "Un troll ricucito grossolanamente avanza.", "Cicatrici e punti visibili ovunque.", "Il troll attacca senza dolore.", "Risate gutturali."),
            _A("RACE_GIANT", "gigante caverna", "Gigante di Caverna", "Un gigante curvo si muove nel corridoio.", "Clava di tronco sollevata.", "Il gigante schianta.", "Passi tremendi."),
            _A("RACE_GIANT_HILL", "gigante colline", "Gigante delle Colline", "Un gigante delle colline blocca la volta.", "Pelle coriacea e odore di muschio.", "Il gigante colpisce.", "Rumore di massi."),
            _A("RACE_GIANT_STONE", "gigante pietra", "Gigante di Pietra", "Un gigante di pietra si muove lentamente.", "Pelle simile a granito.", "Il gigante schianta.", "Pietre che cadono."),
            _A("RACE_HALF_OGRE", "mezzogre guardia", "Mezzogre Guardia", "Un mezzogre armato sorveglia un varco.", "Testa piccola e braccia enormi.", "La guardia colpisce.", "Grugnito ogre."),
            _A("RACE_HALF_OGRE", "mezzogre berserker", "Mezzogre Berserker", "Un mezzogre impazzito brandisce mazza.", "Schiuma alla bocca e occhi rossi.", "Il berserker colpisce.", "Urla di furia."),
            _A("RACE_TYTAN", "titano caduto", "Titano Caduto", "Un titano decaduto si trascina inginocchiato.", "Corpo enorme ma curvo dal tempo.", "Il titano schianta il pugno.", "Tremore tellurico."),
            _A("RACE_GIANT", "gigante mutilato", "Gigante Mutilato", "Un gigante con arti mancanti combatte comunque.", "Ferite antiche non guariscono.", "Il gigante colpisce.", "Lamento profondo."),
            _A("RACE_TROLL", "troll nebbia", "Troll della Nebbia", "Un troll avvolto in bruma attacca goffamente.", "Pelle grigia e occhi vitrei.", "Il troll colpisce.", "Grugnito ovattato."),
        ]
    )

    cats.append(
        [
            _A("RACE_DEMON", "imp caverna", "Imp di Caverna", "Un imp alato volteggia lanciando scintille.", "Coda punge come una vespa.", "L'imp morde.", "Risate acute."),
            _A("RACE_DEMON", "demone minor", "Demone Minore", "Un demone cornuto giovane striscia dal buio.", "Zolfo e cenere lo circondano.", "Il demone attacca.", "Odore di zolfo."),
            _A("RACE_DEVIL", "diavolo imps", "Diavolo delle Fessure", "Un diavolo piccolo brandisce tridente.", "Pelle rossa e occhi gialli.", "Il diavolo punge.", "Risate infernali."),
            _A("RACE_ELEMENT", "elementale terra", "Elementale di Terra", "Un elementale di terra si rialza dal pavimento.", "Pietre e polvere compongono il corpo.", "L'elementale colpisce.", "Pietre che cadono."),
            _A("RACE_ELEMENT", "elementale acqua", "Elementale d'Acqua", "Un elementale d'acqua scorre come marea.", "Corpo trasparente e gelido.", "L'elementale annega.", "Schizzi d'acqua."),
            _A("RACE_ELEMENT", "elementale fuoco", "Elementale di Fuoco", "Un elementale di fuoco danza nel buio.", "Braci verdi corrompono la fiamma.", "L'elementale brucia.", "Crepitio di fiamme."),
            _A("RACE_SLIME", "gelatina caverna", "Gelatina di Caverna", "Una gelatina traslucida striscia sul pavimento.", "Ossa galleggiano nel corpo viscido.", "La gelatina inghiotte.", "Schizzi viscidi."),
            _A("RACE_SLIME", "muffa vivente", "Muffa Vivente", "Una muffa senziente si espande sulle pareti.", "Spore verdi riempiono l'aria.", "La muffa attacca.", "Tossite e starnuti."),
            _A("RACE_DEMON", "manes caverna", "Manes di Caverna", "Un manes distorto striscia come fumo solido.", "Il volto muta continuamente.", "Il manes attacca.", "Lamenti distorti."),
            _A("RACE_DEVIL", "diavolo catena", "Diavolo a Catena", "Un diavolo trascina catene infuocate.", "Ferite bruciano al contatto.", "Il diavolo flagella.", "Scroscio di catene."),
        ]
    )

    cats.append(
        [
            _A("RACE_MFLAYER", "mind flayer giovane", "Mind Flayer Giovane", "Un illithid immaturo tenta di entrare nella tua mente.", "Tentacoli facciali si contorcono.", "L'illithid attacca.", "Sussurri psichici."),
            _A("RACE_SPECIAL", "mimica giovane", "Mimica Giovane", "Una cassa si apre mostrando le fauci.", "Lingua viscida lecca l'aria.", "La mimica morde.", "Tonfo di legno."),
            _A("RACE_SPECIAL", "mimica scrigno", "Mimica Scrigno", "Uno scrigno dorato attende troppo comodo per essere vero.", "La serratura sembra un occhio vivo.", "Lo scrigno morde.", "Scricchiolio metallico."),
            _A("RACE_PRIMATE", "grimlock guerriero", "Grimlock Guerriero", "Un grimlock cieco percepisce il tuo odore.", "Pelle grigia e denti affilati.", "Il grimlock attacca.", "Grugniti cavernosi."),
            _A("RACE_PRIMATE", "grimlock cacciatore", "Grimlock Cacciatore", "Un cacciatore grimlock trascina lancia di osso.", "Percezione acustica terrificante.", "Il cacciatore trafigge.", "Passi silenziosi."),
            _A("RACE_PARASITE", "parassita caverna", "Parassita di Caverna", "Un parassito grosso come un gatto si attacca.", "Corpo viscido e ventose.", "Il parassita succhia.", "Schizzi viscidi."),
            _A("RACE_ASTRAL", "creatura astrale minore", "Creatura Astrale Minore", "Una creatura planare fluttua in modo instabile.", "La forma cambia continuamente.", "La creatura attacca.", "Echi lontani."),
            _A("RACE_PLANAR", "ombra planare", "Ombra Planare", "Un'ombra planare si stacca dal muro.", "Un freddo che non appartiene a questo mondo.", "L'ombra avvolge.", "Silenzio innaturale."),
            _A("RACE_SPECIAL", "aberrazione caverna", "Aberrazione di Caverna", "Una creatura innominabile striscia dal buio.", "Troppi occhi e troppa bocca.", "L'aberrazione attacca.", "Suoni disturbanti."),
            _A("RACE_MFLAYER", "illithid servitore", "Illithid Servitore", "Un servitore illithid obbedisce a un padrone lontano.", "Tentacoli tengono una lama psionica.", "Il servitore attacca.", "Pulsazioni mentali."),
        ]
    )

    cats.append(
        [
            _A("RACE_VEGMAN", "servitore muschio", "Servitore di Muschio", "Un umanoide ricoperto di muschio blocca il passaggio.", "Radici sottili serpeggiano lungo le sue gambe.", "Il servitore alza braccia nodose.", "La terra scricchiola sotto di lui."),
            _A("RACE_VEGMAN", "guardiano radici", "Guardiano delle Radici", "Un umanoide vegetale sorveglia un passaggio.", "Radici emergono dal pavimento.", "Il guardiano colpisce.", "Fruscio di foglie."),
            _A("RACE_TREE", "treant giovane", "Treant Giovane", "Un treant giovane si muove goffamente.", "Rami sfiorano il soffitto.", "Il treant schianta.", "Legno che scricchiola."),
            _A("RACE_VEGGIE", "pianta carnivora", "Pianta Carnivora", "Una pianta carnivora attende con fauci aperte.", "Polline dolce maschera pericolo.", "La pianta morde.", "Fruscio di foglie."),
            _A("RACE_VEGGIE", "liana strangolatrice", "Liana Strangolatrice", "Liane animate pendono dal soffitto.", "Steli si avvolgono come serpenti.", "La liana strangola.", "Fruscio secco."),
            _A("RACE_GHOST", "eco nebbia", "Eco della Nebbia", "Un'eco spettrale della dimensione vaga, confusa.", "Forma muta come vapore.", "L'eco ti attraversa.", "Sussurri ovattati."),
            _A("RACE_ELEMENT", "nebbia vivente", "Nebbia Vivente", "Una nube di bruma assume forma umanoide.", "Occhi argentati nel vapore.", "La nebbia avvolge.", "Silenzio gelido."),
            _A("RACE_GHOST", "spettro nebbia", "Spettro della Nebbia", "Uno spettro nato dalla bruma fluttua.", "Corpo semitrasparente.", "Lo spettro attacca.", "Vento gelido."),
            _A("RACE_PREDATOR", "predatore bruma", "Predatore della Bruma", "Un predatore nato nel velo caccia in silenzio.", "Pelliccia grigia e occhi argentati.", "Il predatore piomba.", "Ringhio ovattato."),
            _A("RACE_INSECT", "libellula nebbia", "Libellula della Nebbia", "Una libellula traslucida volteggia nel vapore.", "Ali sottili tagliano l'aria.", "La libellula morde.", "Ronzio sottile."),
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
