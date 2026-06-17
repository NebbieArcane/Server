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
    (CRYSTAL, PROFANED, DROW_DEEP),  # 24 regina rova nera
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
           "Il Drago soffia un inferno.", "Il suolo trema sotto le fiamme.",
           themes=[FIRE, ARENA, CANYON, GORGOROTH, DRAGON_LAIR]),
        _A("RACE_DRAGON_BLACK", "drago nero corrotto", "Drago Nero Corrotto",
           "Un drago nero corrotto dalla bruma striscia fuori dall'oscurita'.",
           "Alito acido corrode la pietra circostante.",
           "Il Drago Nero attacca.", "Ruggito che squarcia la roccia.",
           themes=[SWAMP, ABYSS, PROFANED, MIRKWOOD]),
        _A("RACE_DRAGON_WHITE", "drago bianco gelido", "Drago Bianco Gelido",
           "Un drago bianco scende da vortici di neve eterna.",
           "Ghiaccio si forma su ogni superficie al suo passaggio.",
           "Il Drago Bianco congela l'aria.", "Vento gelido tagliente.",
           themes=[ICE, MOON, NEVERWINTER]),
        _A("RACE_DRAGON_GOLD", "drago dorato caduto", "Drago Dorato Caduto",
           "Un drago dorado decaduto troneggia su tesori illusori.",
           "Scaglie opache nascondono ancora potere immenso.",
           "Il Drago Dorado attacca.", "Ruggito metallico.",
           themes=[TEMPLE, LIBRARY, CRYSTAL, SMAUG, EREBOR]),
        _A("RACE_DRAGON_GREEN", "signore drago verde", "Signore Drago Verde",
           "Un signore drago verde avvolge la caverna di gas velenoso.",
           "Ali spalancate oscurano torce e speranza.",
           "Il Signore Drago piomba.", "Alito corrosivo.",
           themes=[SWAMP, STONEGARDEN, ABYSS, DRAGON_LAIR, MIRKWOOD]),
        _A("RACE_UNDEAD_SPECTRE", "re nazgul bruma", "Re Nazgul della Bruma",
           "Un re spettrale in armatura nera alza una lama senza luce.",
           "Corona spezzata e terrore antico precedono ogni passo.",
           "Il Re Nazgul ordina la morte.", "Un grido che gela il sangue.",
           themes=[MIST, PROFANED, TEMPLE, DEAD_PATHS, GORGOROTH]),
        _A("RACE_UNDEAD_SPECTRE", "signore anello spezzato", "Signore dell'Anello Spezzato",
           "Uno spettro corazzato porta un anello spezzato al dito.",
           "La sua sola presenza spegne le fiamme.",
           "Il Signore avanza implacabile.", "Sussurro di obbedienza.",
           themes=[MIST, CRYPT, TOWER, DEAD_PATHS]),
        _A("RACE_UNDEAD_LICH", "lich re ossario", "Lich Re dell'Ossuario",
           "Un lich reale troneggia su un trono di teschi impilati.",
           "Corona di ossa e staffa runica pulsano di oscurita'.",
           "Il Lich Re scaglia morte.", "Incantesimi che gelano l'anima.",
           themes=[OSSUARY, CRYPT, LIBRARY, BARROW, BALDUR]),
        _A("RACE_DEMON", "principe demone abisso", "Principe Demone dell'Abisso",
           "Un principe demone emerge da una fessura ardente.",
           "Ali carbonizzate e zolfo riempiono la stanza.",
           "Il Principe flagella con fuoco infernale.", "Odore di zolfo e paura.",
           themes=[ABYSS, PROFANED, FIRE, NINE_HELLS, GORGOROTH]),
        _A("RACE_DEVIL", "arconte infernale", "Arconte Infernale",
           "Un arconte infernale conduce un coro di dannati.",
           "Contratti scritti con sangue fluttuano attorno a lui.",
           "L'Arconte invoca catene infuocate.", "Canti profani.",
           themes=[PROFANED, TEMPLE, FIRE, NINE_HELLS]),
        _A("RACE_GIANT_STORM", "gigante tempesta", "Gigante della Tempesta",
           "Un gigante della tempesta scuote la volta con ogni passo.",
           "Fulmini danzano tra le sue dita massicce.",
           "Il Gigante scaglia saette.", "Tuono sordo continuo.",
           themes=[CANYON, TOWER, MOON, HELM]),
        _A("RACE_GIANT_FIRE", "gigante fuoco", "Gigante del Fuoco",
           "Un gigante del fuoco arde come un vulcano ambulante.",
           "Pietra fusa gocciola dai suoi pugni.",
           "Il Gigante incendia la sala.", "Calore insopportabile.",
           themes=[FIRE, FORGE, ARENA, GORGOROTH, ISENGARD]),
        _A("RACE_GIANT_FROST", "gigante gelo", "Gigante del Gelo",
           "Un gigante del gelo congela l'aria con un solo sguardo.",
           "Barba di ghiaccio e occhi come iceberg.",
           "Il Gigante colpisce con pugni gelidi.", "Ghiaccio che scricchiola.",
           themes=[ICE, MOON, NEVERWINTER]),
        _A("RACE_MFLAYER", "arconte illithid", "Arconte Illithid",
           "Un arconte illithid fluttua con tentacoli regali.",
           "Menti deboli collassano al suo sguardo.",
           "L'Arconte attacca la mente.", "Pulsazioni psichiche.",
           themes=[LIBRARY, ABYSS, CRYSTAL, BEHOLDER, SIGIL]),
        _A("RACE_PLANAR", "signore abisso stellare", "Signore dell'Abisso Stellare",
           "Una creatura planare distorce lo spazio attorno a se'.",
           "Geometrie impossibili orbitano nel suo velo.",
           "Il Signore piega la realta'.", "Echi non euclidei.",
           themes=[MOON, CRYSTAL, ABYSS, SIGIL]),
        _A("RACE_UNDEAD_VAMPIRE", "gran signore vampiro", "Gran Signore Vampiro",
           "Un gran signore vampiro sorge da un sarcofago di marmo nero.",
           "Potere antico e sete insaziabile lo avvolgono.",
           "Il Gran Signore attacca.", "Sussurro che ipnotizza.",
           themes=[CRYPT, TEMPLE, TOWER, BALDUR, RAVENLOFT]),
        _A("RACE_ORC", "signore uruk nero", "Signore Uruk-Nero",
           "Un signore uruk-nero brandisce una spada enorme.",
           "Stendardo nero e occhi come brace.",
           "Il Signore ordina la strage.", "Cori di guerra.",
           themes=[FORGE, MINE, ARENA, URUK_LAIR, ISENGARD]),
        _A("RACE_TROLL", "signore troll montagna", "Signore Troll di Montagna",
           "Un signore troll delle montagne riempie l'intera sala.",
           "Ogni passo fa crollare stalattiti.",
           "Il Signore Troll schianta.", "Ruggito tellurico.",
           themes=[CANYON, ABYSS, MINE, MORIA, CHASM]),
        _A("RACE_ARACHNID", "regina ragni antica", "Regina Ragni Antica",
           "Una regina ragni antica tesse un trono di seta e ossa.",
           "Figliame infinito si muove nelle ombre.",
           "La Regina convoca ragni.", "Ragnatele ovunque.",
           themes=[SPIDER, UNGOL]),
        _A("RACE_TREE", "signore ent corrotto", "Signore Ent Corrotto",
           "Un signore ent corrotto torce rami come torri.",
           "Linfa nera stilla dal tronco.",
           "Il Signore Ent schianta.", "Legno che urla.",
           themes=[WILD, STONEGARDEN, THORNS, FANGORN]),
        _A("RACE_ELEMENT", "signore elementale tempesta", "Signore Elementale della Tempesta",
           "Un signore elementale della tempesta riempie la sala di vento.",
           "Fulmini e pioggia obbediscono al suo gesto.",
           "Il Signore scatena la tempesta.", "Tuono continuo.",
           themes=[CANYON, DROWNED, MOON]),
        _A("RACE_DARK_ELF", "matrona drow", "Matrona Drow",
           "Una matrona drow troneggia su un altare di ossidiana.",
           "Ragni sacri le tessono il manto.",
           "La Matrona ordina la morte.", "Incantesimi sibilati.",
           themes=[CRYSTAL, PROFANED, SPIDER, DROW_DEEP, UNGOL]),
        _A("RACE_GHOST", "re spettro corone", "Re Spettro delle Corone",
           "Un re spettro delle corone perdute fluttua sopra un trono.",
           "Corone spezzate orbitano come lune morte.",
           "Il Re Spettro invoca il gelo.", "Lamenti regali.",
           themes=[CRYPT, TEMPLE, TOWER, BALDUR, RAVENLOFT]),
        _A("RACE_DEMON", "balrog antico", "Balrog Antico",
           "Un balrog antico sorge tra fiamme e ombra.",
           "Frusta infuocata e spada di fuoco squarciando il buio.",
           "Il Balrog flagella.", "Ruggito di magma.",
           themes=[FIRE, ABYSS, MINE, MORIA, GORGOROTH, NINE_HELLS]),
        _A("RACE_DRAGON_SILVER", "drago argento lunare", "Drago d'Argento Lunare",
           "Un drago d'argento riflette luce lunare impossibile.",
           "Ali argentee e occhi pieni di maree.",
           "Il Drago d'Argento attacca.", "Luce accecante.",
           themes=[MOON, CRYSTAL, DROWNED, LORIEN]),
    ]


def get_traps() -> list[dict[str, Any]]:
    """20 trappole-mob (stanze ProcArchetype::Trap)."""
    return [
        _A("RACE_SNAKE", "trappola anguilla fango", "Anguilla del Fango",
           "Un serpente viscido emerge dal fango con bocca spalancata.",
           "La pelle oleosa rende difficile colpirla.",
           "L'anguilla si avvolge verso di te.", "Schizzi di fango ovunque.",
           themes=[SWAMP, DROWNED, WATERDEEP]),
        _A("RACE_SPECIAL", "trappola mimica pavimento", "Mimica del Pavimento",
           "Una piastrella si apre a fauci: era una mimica.",
           "Denti d'acciaio nascondevano sotto la polvere.",
           "La mimica morde.", "Scricchiolio metallico.",
           themes=[LIBRARY, CRYSTAL, TEMPLE, SIGIL, BEHOLDER]),
        _A("RACE_SLIME", "trappola gelatina assassina", "Gelatina Assassina",
           "Una gelatina assassina cade dal soffitto.",
           "Corpo viscido inghiotte luce e speranza.",
           "La gelatina avvolge.", "Schizzi viscidi.",
           themes=[FUNGUS, SWAMP, DROWNED, MIRKWOOD]),
        _A("RACE_INSECT", "trappola ragnatela vivente", "Ragnatela Vivente",
           "Filamenti di ragnatela si tendono all'improvviso.",
           "Qualcosa tira verso l'alto con forza innaturale.",
           "La ragnatela strangola.", "Ragnatele che vibrano.",
           themes=[SPIDER, UNGOL]),
        _A("RACE_SNAKE", "trappola vipera nascosta", "Vipera Nascosta",
           "Una vipera scatta da una fessura nel muro.",
           "Veleno gocciola dalle zanne appena visibili.",
           "La vipera morde.", "Sibilo secco.",
           themes=[SWAMP, THORNS, WILD, MIRKWOOD]),
        _A("RACE_ELEMENT", "trappola gas velenoso", "Nube di Gas Velenoso",
           "Una nube di gas velenoso si alza dal pavimento.",
           "Occhi gialli pulsano nel vapore.",
           "Il gas avvelena.", "Tossire violento."),
        _A("RACE_GOLEM", "trappola porta punta", "Porta a Punta",
           "La porta si chiude rivelando punte di pietra.",
           "Runa attivata scorrono lungo il telaio.",
           "Le punte trafiggono.", "Pietre che scivolano.",
           themes=[CRYPT, MINE, TOWER, MORIA, EREBOR, CHASM]),
        _A("RACE_ELEMENT", "trappola ghiaccio sottile", "Ghiaccio Sottile",
           "Il pavimento e' un sottile strato di ghiaccio nascosto.",
           "Crepe si aprono sotto i tuoi passi.",
           "Il ghiaccio cede.", "Scricchiolio gelido.",
           themes=[ICE, MOON, NEVERWINTER]),
        _A("RACE_ELEMENT", "trappola fiamma improvvisa", "Trappola di Fiamma",
           "Getti di fiamma erompono da ugelli nel pavimento.",
           "Runa bruciate si accendono in sequenza.",
           "Le fiamme avvolgono.", "Esplosione di fuoco.",
           themes=[FIRE, FORGE, GORGOROTH, ISENGARD, NINE_HELLS]),
        _A("RACE_ARACHNID", "trappola ragno agguato", "Ragno d'Agguato",
           "Un ragno d'agguato precipita dal soffitto.",
           "Veleno paralizzante nelle zanne.",
           "Il ragno morde.", "Ronzio acuto.",
           themes=[SPIDER, FUNGUS, UNGOL]),
        _A("RACE_ELEMENT", "trappola cristallo tagliente", "Cristallo Tagliente",
           "Cristalli affilati sporgono dal pavimento.",
           "Luce riflessa nasconde i fili taglienti.",
           "I cristalli tagliano.", "Scheggia di vetro.",
           themes=[CRYSTAL, SALT]),
        _A("RACE_ELEMENT", "trappola maree improvvisa", "Marea Improvvisa",
           "Acqua salata irrompe da fessure laterali.",
           "Corrente spinge verso pareti taglienti.",
           "La marea travolge.", "Onde improvvise.",
           themes=[DROWNED, MOON]),
        _A("RACE_ELEMENT", "trappola polvere sale", "Polvere di Sale Tagliente",
           "Polvere di sale tagliente si alza in vortice.",
           "Cristalli bianchi graffiano pelle e occhi.",
           "Il sale taglia.", "Polvere acida.",
           themes=[SALT,]),
        _A("RACE_UNDEAD_SKELETON", "trappola scheletri sepolcro", "Scheletri del Sepolcro",
           "Scheletri cadono dal soffitto in una pioggia d'ossa.",
           "Lame arrugginite pronte a colpire.",
           "Gli scheletri attaccano.", "Ossa che sbattono.",
           themes=[CRYPT, OSSUARY, BARROW, BALDUR]),
        _A("RACE_DEMON", "trappola fessura infernale", "Fessura Infernale",
           "Una fessura infernale si apre sotto i piedi.",
           "Zolfo e artigli emergono dal baratro.",
           "La fessura brucia.", "Odore di zolfo.",
           themes=[PROFANED, ABYSS, FIRE, NINE_HELLS, GORGOROTH]),
        _A("RACE_INSECT", "trappola vespa caverna", "Vespa di Caverna",
           "Vespe di caverna escono da un nido nascosto.",
           "Pungiglioni lunghi gocciolano veleno.",
           "Le vespe attaccano.", "Ronzio furioso.",
           themes=[FUNGUS, CANYON]),
        _A("RACE_PREDATOR", "trappola worg fossa", "Worg della Fossa",
           "Un worg balza da una fossa coperta.",
           "Zanne pronte a strappare la gola.",
           "Il worg piomba.", "Ringhio famelico.",
           themes=[WILD, ABYSS, CANYON, CHASM, HELM]),
        _A("RACE_GHOST", "trappola ombra maledetta", "Ombra Maledetta",
           "Un'ombra maledetta si stacca dal pavimento.",
           "Freddo non appartiene a questo mondo.",
           "L'ombra avvolge.", "Sussurro gelido.",
           themes=[MIST, CRYPT, PROFANED, DEAD_PATHS, RAVENLOFT]),
        _A("RACE_ELEMENT", "trappola fulmine runico", "Fulmine Runico",
           "Runa sul pavimento scatenano fulmini a catena.",
           "Odore di ozono riempie la stanza.",
           "Il fulmine colpisce.", "Tuono improvviso.",
           themes=[CANYON, TOWER, FORGE, ISENGARD, HELM]),
        _A("RACE_PARASITE", "trappola parassita mimetico", "Parassita Mimetico",
           "Un parassito mimetico si stacca dal soffitto come stalattite.",
           "Ventose si attaccano prima che tu reagisca.",
           "Il parassito succhia.", "Schizzi viscidi.",
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
