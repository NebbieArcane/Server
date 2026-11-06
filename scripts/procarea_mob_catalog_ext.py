"""Boss epici, trappole e assegnazione temi al catalogo procarea."""

from __future__ import annotations

from typing import Any

from procarea_theme_tags import (
    ABYSS,
    ARENA,
    BALDUR,
    BARROW,
    BEHOLDER,
    BREE,
    CANYON,
    CHASM,
    CRYPT,
    CRYSTAL,
    CROW,
    DEAD_PATHS,
    DRAGON_LAIR,
    DROW_DEEP,
    DROWNED,
    EREBOR,
    FANGORN,
    FIRE,
    FORGE,
    FUNGUS,
    GORGOROTH,
    HELM,
    ICE,
    ISENGARD,
    LIBRARY,
    LORIEN,
    MINE,
    MIRKWOOD,
    MIST,
    MOON,
    MORIA,
    NINE_HELLS,
    NEVERWINTER,
    OSSUARY,
    OSGILIATH,
    PROFANED,
    RAVENLOFT,
    SALT,
    SIGIL,
    SMAUG,
    SPIDER,
    STONEGARDEN,
    SWAMP,
    TEMPLE,
    THORNS,
    TOWER,
    UNGOL,
    URUK_LAIR,
    WATERDEEP,
    WILD,
)


def _A(
    race: str,
    keywords: str,
    short: str,
    long: str,
    look: str,
    agg: str,
    sound: str,
    themes: list[int] | None = None,
) -> dict[str, Any]:
    entry: dict[str, Any] = {
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


# Temi per i 30 boss base (indice 0..29)
BASE_BOSS_THEMES: tuple[tuple[int, ...], ...] = (
    (MIST, WILD),           # 0 alfa nebbia
    (SPIDER,),              # 1 regina tarantole
    (WILD, STONEGARDEN),    # 2 guardiano muschio
    (ABYSS, WILD),          # 3 predatore cieco
    (FUNGUS, SWAMP),        # 4 matriarca vermi
    (ABYSS, CROW),          # 5 signore pipistrelli
    (CRYPT, OSSUARY),       # 6 capo saprofagi
    (ABYSS, CANYON),        # 7 custode fosso
    (FUNGUS, SPIDER),       # 8 re larve
    (MIST, CRYPT),          # 9 ombra cunicolo
    (CRYPT, OSSUARY),       # 10 arconte
    (MIST, PROFANED, DEAD_PATHS, GORGOROTH),  # 11 nazgul caduto
    (SPIDER, UNGOL),              # 12 madre ungol
    (MINE, FORGE, URUK_LAIR, ISENGARD),  # 13 capobanda goblin
    (ABYSS, MINE, MORIA),          # 14 signore troll
    (CRYPT, OSSUARY, BARROW, DEAD_PATHS),  # 15 barrow-wight
    (ABYSS, CANYON, DRAGON_LAIR),  # 16 drago verde
    (CANYON, CROW),         # 17 matriarca arpie
    (FORGE, ARENA, URUK_LAIR, ISENGARD),  # 18 signore uruk
    (WILD, STONEGARDEN, FANGORN, MIRKWOOD),  # 19 ent corrotto
    (PROFANED, ABYSS),      # 20 demone fessure
    (CRYPT, TEMPLE, BALDUR, RAVENLOFT),  # 21 vampiro bruma
    (WILD, CANYON),         # 22 gigante radici
    (SWAMP, DROWNED),       # 23 elementale fango
    (CRYSTAL, PROFANED, DROW_DEEP),  # 24 regina rocca nera
    (PROFANED, TEMPLE, NINE_HELLS),  # 25 culto abissale
    (MINE, LIBRARY, MORIA, EREBOR),  # 26 oracolo moria
    (LIBRARY, CRYSTAL),     # 27 signore mimiche
    (FIRE, ABYSS, MORIA, GORGOROTH, NINE_HELLS),  # 28 balrog minore
    (OSSUARY, CRYPT),       # 29 sovrano ghoul
)

# Temi per le 15 categorie mob (10 mob ciascuna)
MOB_CATEGORY_THEMES: tuple[tuple[int, ...], ...] = (
    (FORGE, MINE, ARENA, WILD, URUK_LAIR, ISENGARD, MORIA, EREBOR),  # goblinoid
    (FORGE, MINE, ARENA, ABYSS, URUK_LAIR, GORGOROTH, ISENGARD),  # orchi
    (CRYPT, OSSUARY, PROFANED, BARROW, DEAD_PATHS, BALDUR, RAVENLOFT),  # undead 1
    (CRYPT, OSSUARY, TEMPLE, BARROW, DEAD_PATHS, BALDUR),  # undead 2
    (SPIDER, FUNGUS, UNGOL),  # ragni
    (SPIDER, FUNGUS, SWAMP, MIRKWOOD, UNGOL),  # insetti
    (SWAMP, DROWNED, CANYON, WATERDEEP),  # rettili
    (WILD, MIST, ABYSS, FANGORN, MIRKWOOD),  # predatori
    (CROW, ABYSS, MIST, MIRKWOOD),  # uccelli
    (MINE, FORGE, LIBRARY, MORIA, EREBOR),  # nani/sotterranei
    (CRYSTAL, PROFANED, TEMPLE, LORIEN, DROW_DEEP),  # elfici
    (ABYSS, TOWER, CANYON, HELM, CHASM),  # giganti
    (PROFANED, FIRE, ABYSS, NINE_HELLS, GORGOROTH, SIGIL),  # planari
    (CRYSTAL, LIBRARY, ABYSS, BEHOLDER, SIGIL),  # aberrazioni
    (MIST, WILD, STONEGARDEN, FANGORN),  # nebbia/vegetale
)


def get_epic_bosses() -> list[dict[str, Any]]:
    """25 boss epici aggiuntivi (draghi, nazgul, signori)."""
    return [
        _A("RACE_DRAGON_RED", "drago rosso antico", "Drago Rosso Antico",
           "Un drago rosso antico riempie la sala di calore soffocante.",
           "Scaglie come metallo incandescente e occhi di magma.",
           "Il suolo trema sotto di te tra bagliori di fiamma.", "In lontananza il suolo trema tra echi di fuoco.",
           themes=[FIRE, ARENA, CANYON, GORGOROTH, DRAGON_LAIR]),
        _A("RACE_DRAGON_BLACK", "drago nero corrotto", "Drago Nero Corrotto",
           "Un drago nero corrotto dalla bruma striscia fuori dall'oscurita'.",
           "Il suo alito acido corrode la pietra circostante.",
           "Un ruggito nero squarcia la roccia intorno a te.", "Odi in lontananza un ruggito che spacca la pietra.",
           themes=[SWAMP, ABYSS, PROFANED, MIRKWOOD]),
        _A("RACE_DRAGON_WHITE", "drago bianco gelido", "Drago Bianco Gelido",
           "Un drago bianco scende da vortici di neve eterna.",
           "Ghiaccio si forma su ogni superficie al suo passaggio.",
           "Un vento gelido ti taglia il volto.", "Da qui vicino arriva un vento gelido tagliente.",
           themes=[ICE, MOON, NEVERWINTER]),
        _A("RACE_DRAGON_GOLD", "drago dorato caduto", "Drago Dorato Caduto",
           "Un drago dorato decaduto troneggia su tesori illusori.",
           "Scaglie opache nascondono ancora potere immenso.",
           "Un ruggito metallico vibra nella stanza.", "In lontananza risuona un ruggito metallico.",
           themes=[TEMPLE, LIBRARY, CRYSTAL, SMAUG, EREBOR]),
        _A("RACE_DRAGON_GREEN", "signore drago verde", "Signore dei Draghi Verdi",
           "Un drago verde sovrano avvolge la caverna di gas velenoso.",
           "Ali spalancate oscurano torce e speranza.",
           "Un alito acido ti brucia le narici.", "Odi in lontananza un vapore corrosivo che sibila.",
           themes=[SWAMP, STONEGARDEN, ABYSS, DRAGON_LAIR, MIRKWOOD]),
        _A("RACE_UNDEAD_SPECTRE", "re nazgul bruma", "Re dei Nazgul della Bruma",
           "Un re spettrale in armatura nera alza una lama senza luce.",
           "Corona spezzata e terrore antico precedono ogni passo.",
           "Un grido ti gela il sangue sulle spalle.", "In lontananza un grido gela l'aria.",
           themes=[MIST, PROFANED, TEMPLE, DEAD_PATHS, GORGOROTH]),
        _A("RACE_UNDEAD_SPECTRE", "signore anello spezzato", "Signore dell'Anello Spezzato",
           "Uno spettro corazzato porta un anello spezzato al dito.",
           "La sua sola presenza spegne le fiamme.",
           "Un sussurro di obbedienza ti striscia nella mente.", "Da qui vicino arriva un sussurro di cieca obbedienza.",
           themes=[MIST, CRYPT, TOWER, DEAD_PATHS]),
        _A("RACE_UNDEAD_LICH", "lich re ossario", "Re dei Lich dell'Ossuario",
           "Un lich reale troneggia su un trono di teschi impilati.",
           "La corona di ossa e il bastone runico pulsano di oscurita'.",
           "Incantesimi mortali gelano l'aria intorno a te.", "Odi in lontananza formule che gelano l'anima.",
           themes=[OSSUARY, CRYPT, LIBRARY, BARROW, BALDUR]),
        _A("RACE_DEMON", "principe demone abisso", "Principe dei Demoni dell'Abisso",
           "Un principe demone emerge da una fessura ardente.",
           "Ali carbonizzate e zolfo riempiono la stanza.",
           "Odore di zolfo e paura ti stringe il petto.", "In lontananza aleggia un tanfo di zolfo e terrore.",
           themes=[ABYSS, PROFANED, FIRE, NINE_HELLS, GORGOROTH]),
        _A("RACE_DEVIL", "arconte infernale", "Arconte degli Inferi",
           "Un arconte infernale conduce un coro di dannati.",
           "Contratti scritti con sangue fluttuano attorno a lui.",
           "Canti profani ti circondano come fumo.", "Odi in lontananza canti profani e lenti.",
           themes=[PROFANED, TEMPLE, FIRE, NINE_HELLS]),
        _A("RACE_GIANT_STORM", "gigante tempesta", "Gigante della Tempesta",
           "Un gigante della tempesta scuote la volta con ogni passo.",
           "Fulmini danzano tra le sue dita massicce.",
           "Un tuono sordo continuo vibra sotto i tuoi piedi.", "In lontananza un tuono sordo non cessa.",
           themes=[CANYON, TOWER, MOON, HELM]),
        _A("RACE_GIANT_FIRE", "gigante fuoco", "Gigante del Fuoco",
           "Un gigante del fuoco arde come un vulcano ambulante.",
           "Pietra fusa gocciola dai suoi pugni.",
           "Un calore insopportabile ti investe la pelle.", "Da qui vicino arriva un calore che soffoca.",
           themes=[FIRE, FORGE, ARENA, GORGOROTH, ISENGARD]),
        _A("RACE_GIANT_FROST", "gigante gelo", "Gigante del Gelo",
           "Un gigante del gelo congela l'aria con un solo sguardo.",
           "Barba di ghiaccio e occhi come iceberg.",
           "Ghiaccio scricchiola e si spezza vicino a te.", "Odi in lontananza ghiaccio che scricchiola.",
           themes=[ICE, MOON, NEVERWINTER]),
        _A("RACE_MFLAYER", "arconte illithid", "Arconte degli Illithid",
           "Un arconte illithid fluttua con tentacoli regali.",
           "Menti deboli collassano al suo sguardo.",
           "Pulsazioni psichiche ti premono dietro gli occhi.", "In lontananza senti pulsazioni mentali disturbanti.",
           themes=[LIBRARY, ABYSS, CRYSTAL, BEHOLDER, SIGIL]),
        _A("RACE_PLANAR", "signore abisso stellare", "Signore dell'Abisso Stellare",
           "Una creatura planare distorce lo spazio attorno a se'.",
           "Geometrie impossibili orbitano nel suo velo.",
           "Echi impossibili si spezzano e si ricompongono qui.", "Odi in lontananza echi che non dovrebbero esistere.",
           themes=[MOON, CRYSTAL, ABYSS, SIGIL]),
        _A("RACE_UNDEAD_VAMPIRE", "gran signore vampiro", "Gran Signore dei Vampiri",
           "Un antico signore dei vampiri sorge da un sarcofago di marmo nero.",
           "Potere antico e sete insaziabile lo avvolgono.",
           "Un sussurro ipnotico ti avvince la mente.", "Da qui vicino arriva un sussurro che ipnotizza.",
           themes=[CRYPT, TEMPLE, TOWER, BALDUR, RAVENLOFT]),
        _A("RACE_ORC", "signore uruk nero", "Signore degli Uruk Neri",
           "Un capitano uruk nero brandisce una spada enorme.",
           "Stendardo nero e occhi come brace.",
           "Cori di guerra nera rimbombano nella sala.", "In lontananza cori di guerra uruk echeggiano.",
           themes=[FORGE, MINE, ARENA, URUK_LAIR, ISENGARD]),
        _A("RACE_TROLL", "signore troll montagna", "Signore dei Troll di Montagna",
           "Un troll delle montagne imponente riempie l'intera sala.",
           "Ogni passo fa crollare stalattiti.",
           "Un ruggito tellurico ti scuote le ginocchia.", "Odi in lontananza un ruggito che scuote la montagna.",
           themes=[CANYON, ABYSS, MINE, MORIA, CHASM]),
        _A("RACE_ARACHNID", "regina ragni antica", "Regina dei Ragni Antichi",
           "Un'antica regina dei ragni tesse un trono di seta e ossa.",
           "Prole infinita si muove nelle ombre.",
           "Ragnatele antiche ti sfiorano il volto.", "Da una caverna vicina arriva lo stridio di ragnatele.",
           themes=[SPIDER, UNGOL]),
        _A("RACE_TREE", "signore ent corrotto", "Signore degli Ent Corrotti",
           "Un ent corrotto possente torce rami come torri.",
           "Linfa nera stilla dal tronco.",
           "Legno corrotto urla e si spezza vicino a te.", "In lontananza odi legno che urla sotto tensione.",
           themes=[WILD, STONEGARDEN, THORNS, FANGORN]),
        _A("RACE_ELEMENT", "signore elementale tempesta", "Signore degli Elementali della Tempesta",
           "Un elementale della tempesta sovrano riempie la sala di vento.",
           "Fulmini e pioggia obbediscono al suo gesto.",
           "Un tuono continuo riempie la stanza.", "Odi in lontananza un tuono di tempesta ininterrotto.",
           themes=[CANYON, DROWNED, MOON]),
        _A("RACE_DARK_ELF", "matrona drow", "Matrona Drow",
           "Una matrona drow troneggia su un altare di ossidiana.",
           "Ragni sacri le tessono il manto.",
           "Incantesimi drow sibilano nell'aria intorno a te.", "In lontananza formule elfiche nere sibilano.",
           themes=[CRYSTAL, PROFANED, SPIDER, DROW_DEEP, UNGOL]),
        _A("RACE_GHOST", "re spettro corone", "Re degli Spettri delle Corone Perdute",
           "Un re spettro delle corone perdute fluttua sopra un trono.",
           "Corone spezzate orbitano come lune morte.",
           "Lamenti regali e gelidi ti passano vicino.", "Odi in lontananza lamenti di corone perdute.",
           themes=[CRYPT, TEMPLE, TOWER, BALDUR, RAVENLOFT]),
        _A("RACE_DEMON", "balrog antico", "Balrog Antico",
           "Un balrog antico sorge tra fiamme e ombra.",
           "Frusta infuocata e spada di fuoco squarciano il buio.",
           "Un ruggito di magma antico ti riempie le orecchie.", "In lontananza un ruggito di magma ribolle.",
           themes=[FIRE, ABYSS, MINE, MORIA, GORGOROTH, NINE_HELLS]),
        _A("RACE_DRAGON_SILVER", "drago argento lunare", "Drago d'Argento Lunare",
           "Un drago d'argento riflette una luce lunare impossibile.",
           "Ali argentee e occhi pieni di marea.",
           "Una luce argentea e accecante ti ferisce gli occhi.", "Da qui vicino arriva un lampo d'argento lunare.",
           themes=[MOON, CRYSTAL, DROWNED, LORIEN]),
    ]


def get_traps() -> list[dict[str, Any]]:
    """20 trappole-mob (stanze ProcArchetype::Trap)."""
    return [
        _A("RACE_SNAKE", "trappola anguilla fango", "Anguilla del Fango",
           "Un'anguilla viscida emerge dal fango con bocca spalancata.",
           "La pelle oleosa rende difficile colpirla.",
           "Schizzi di fango gorgogliano a un passo da te.", "Da qui vicino arriva lo schizzo di fango vivo.",
           themes=[SWAMP, DROWNED, WATERDEEP]),
        _A("RACE_SPECIAL", "trappola mimica pavimento", "Mimica del Pavimento",
           "Una piastrella si apre mostrando le fauci.",
           "Denti d'acciaio, prima celati sotto la polvere, spuntano.",
           "Uno scricchiolio metallico di pavimento vivo vibra qui.", "Odi in lontananza uno scricchiolio metallico sul suolo.",
           themes=[LIBRARY, CRYSTAL, TEMPLE, SIGIL, BEHOLDER]),
        _A("RACE_SLIME", "trappola gelatina assassina", "Gelatina Assassina",
           "Una gelatina assassina cade dal soffitto.",
           "Corpo viscido inghiotte luce e speranza.",
           "Schizzi viscidi di gelatina ti cadono ai piedi.", "In lontananza odi schizzi viscosi di gelatina.",
           themes=[FUNGUS, SWAMP, DROWNED, MIRKWOOD]),
        _A("RACE_INSECT", "trappola ragnatela vivente", "Ragnatela Vivente",
           "Filamenti di ragnatela si tendono all'improvviso.",
           "Qualcosa tira verso l'alto con forza innaturale.",
           "Ragnatele vive vibrano intorno a te.", "Da qui vicino arriva la vibrazione di ragnatele vive.",
           themes=[SPIDER, UNGOL]),
        _A("RACE_SNAKE", "trappola vipera nascosta", "Vipera Nascosta",
           "Una vipera scatta da una fessura nel muro.",
           "Veleno gocciola dalle zanne appena visibili.",
           "Un sibilo secco di vipera nascosta ti fa fermare.", "Odi in lontananza un sibilo secco di vipera.",
           themes=[SWAMP, THORNS, WILD, MIRKWOOD]),
        _A("RACE_ELEMENT", "trappola gas velenoso", "Nube di Gas Velenoso",
           "Una nube di gas velenoso si alza dal pavimento.",
           "Occhi gialli pulsano nel vapore.",
           "Colpi di tosse violenti riempiono l'aria qui.", "In lontananza odi colpi di tosse violenti."),
        _A("RACE_GOLEM", "trappola porta a punte", "Porta a Punta",
           "La porta si chiude rivelando punte di pietra.",
           "Rune attivate scorrono lungo il telaio.",
           "Pietre scivolano e meccanismi scattano vicino a te.", "Da qui vicino arriva lo scatto di pietre che scivolano.",
           themes=[CRYPT, MINE, TOWER, MORIA, EREBOR, CHASM]),
        _A("RACE_ELEMENT", "trappola ghiaccio sottile", "Ghiaccio Sottile",
           "Il pavimento e' un sottile strato di ghiaccio nascosto.",
           "Crepe si aprono sotto i tuoi passi.",
           "Ghiaccio sottile scricchiola sotto i tuoi piedi.", "Odi in lontananza ghiaccio che scricchiola.",
           themes=[ICE, MOON, NEVERWINTER]),
        _A("RACE_ELEMENT", "trappola fiamma", "Trappola di Fiamma",
           "Getti di fiamma erompono da ugelli nel pavimento.",
           "Rune bruciate si accendono in sequenza.",
           "Un crepitio di fiamme nascoste ti investe il volto.", "In lontananza un crepitio di fuoco improvviso echeggia.",
           themes=[FIRE, FORGE, GORGOROTH, ISENGARD, NINE_HELLS]),
        _A("RACE_ARACHNID", "trappola ragno agguato", "Ragno d'Agguato",
           "Un ragno d'agguato precipita dal soffitto.",
           "Veleno paralizzante nelle zanne.",
           "Un fruscio di zampe di ragno d'agguato ti passa vicino.", "Da qui vicino arriva un fruscio di zampe.",
           themes=[SPIDER, FUNGUS, UNGOL]),
        _A("RACE_ELEMENT", "trappola cristallo tagliente", "Cristallo Tagliente",
           "Cristalli affilati sporgono dal pavimento.",
           "Luce riflessa nasconde i fili taglienti.",
           "Schegge di cristallo tintinnano e cadono qui.", "Odi in lontananza schegge di cristallo che cadono.",
           themes=[CRYSTAL, SALT]),
        _A("RACE_ELEMENT", "trappola marea improvvisa", "Marea Improvvisa",
           "Acqua salata irrompe da fessure laterali.",
           "Corrente spinge verso pareti taglienti.",
           "Onde improvvise sbattano contro le pareti intorno a te.", "In lontananza odi onde che si abbattono.",
           themes=[DROWNED, MOON]),
        _A("RACE_ELEMENT", "trappola polvere sale", "Polvere di Sale Tagliente",
           "Polvere di sale tagliente si alza in vortice.",
           "Cristalli bianchi graffiano pelle e occhi.",
           "Cristalli di sale tagliente scricchiolano sotto i passi.", "Da qui vicino arriva lo scricchiolio di cristalli di sale.",
           themes=[SALT,]),
        _A("RACE_UNDEAD_SKELETON", "trappola scheletri sepolcro", "Scheletri del Sepolcro",
           "Scheletri cadono dal soffitto in una pioggia d'ossa.",
           "Lame arrugginite pronte a colpire.",
           "Ossa di scheletri sepolti sbattono nella stanza.", "Odi in lontananza ossa che sbattono dal sepolcro.",
           themes=[CRYPT, OSSUARY, BARROW, BALDUR]),
        _A("RACE_DEMON", "trappola fessura infernale", "Fessura Infernale",
           "Una fessura infernale si apre sotto i piedi.",
           "Zolfo e artigli emergono dal baratro.",
           "Un odore di zolfo di fessura infernale ti prende alla gola.", "In lontananza aleggia un tanfo di zolfo.",
           themes=[PROFANED, ABYSS, FIRE, NINE_HELLS, GORGOROTH]),
        _A("RACE_INSECT", "trappola vespa caverna", "Vespe di Caverna",
           "Vespe di caverna escono da un nido nascosto.",
           "Pungiglioni lunghi gocciolano veleno.",
           "Un ronzio furioso di vespe di caverna riempie l'aria.", "Da qui vicino arriva un ronzio furioso di vespe.",
           themes=[FUNGUS, CANYON]),
        _A("RACE_PREDATOR", "trappola worg fossa", "Worg della Fossa",
           "Un worg balza da una fossa coperta.",
           "Zanne pronte a strappare la gola.",
           "Un ringhio famelico di worg di fossa ti circonda.", "Odi in lontananza un ringhio famelico di worg.",
           themes=[WILD, ABYSS, CANYON, CHASM, HELM]),
        _A("RACE_GHOST", "trappola ombra maledetta", "Ombra Maledetta",
           "Un'ombra maledetta si stacca dal pavimento.",
           "Un freddo che non appartiene a questo mondo.",
           "Un sussurro gelido di ombra maledetta ti sfiora.", "In lontananza un sussurro gelido si trascina.",
           themes=[MIST, CRYPT, PROFANED, DEAD_PATHS, RAVENLOFT]),
        _A("RACE_ELEMENT", "trappola fulmine runico", "Fulmine Runico",
           "Rune sul pavimento scatenano fulmini a catena.",
           "Odore di ozono riempie la stanza.",
           "Un tuono runico improvviso vibra nella sala.", "Da qui vicino arriva un tuono runico improvviso.",
           themes=[CANYON, TOWER, FORGE, ISENGARD, HELM]),
        _A("RACE_PARASITE", "trappola parassita mimetico", "Parassita Mimetico",
           "Un parassita mimetico si stacca dal soffitto come stalattite.",
           "Le ventose si attaccano prima che tu reagisca.",
           "Schizzi viscidi di un parassita mimetico ti cadono addosso.", "Odi in lontananza schizzi viscosi di qualcosa che striscia.",
           themes=[FUNGUS, ABYSS, SWAMP, BEHOLDER, WATERDEEP]),
    ]


def tag_entry(entry: dict[str, Any], themes: tuple[int, ...] | list[int]) -> dict[str, Any]:
    tagged = dict(entry)
    tagged["themes"] = list(themes)
    return tagged


def apply_base_boss_themes(bosses: list[dict[str, Any]]) -> list[dict[str, Any]]:
    out: list[dict[str, Any]] = []
    for idx, boss in enumerate(bosses):
        themes = BASE_BOSS_THEMES[idx] if idx < len(BASE_BOSS_THEMES) else ()
        out.append(tag_entry(boss, themes) if themes else dict(boss))
    return out


def apply_mob_category_themes(categories: list[list[dict[str, Any]]]) -> list[dict[str, Any]]:
    flat: list[dict[str, Any]] = []
    for cat_idx, cat in enumerate(categories):
        themes = MOB_CATEGORY_THEMES[cat_idx] if cat_idx < len(MOB_CATEGORY_THEMES) else ()
        for mob in cat:
            flat.append(tag_entry(mob, themes) if themes else dict(mob))
    return flat
