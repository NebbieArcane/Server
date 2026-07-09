#!/usr/bin/env python3
"""Riscrive ogni description premio come oggetto a terra, in italiano corretto."""

from __future__ import annotations

import re
import subprocess
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
NAMES_INC = ROOT / "src/procarea_reward_names.inc"
MAX_LEN = 125

ENTRY_RE = re.compile(
    r"\{\s*"
    r'"(?P<kw>(?:\\.|[^"\\])*)"\s*,\s*'
    r'"(?P<short>(?:\\.|[^"\\])*)"\s*,\s*'
    r'"(?P<desc>(?:\\.|[^"\\])*)"\s*,?\s*'
    r"\}",
    re.MULTILINE,
)

ORIGINAL_ENTRY_RE = re.compile(
    r'\{\s*"(?P<kw>(?:\\.|[^"\\])*)"\s*,\s*'
    r'"(?P<short>(?:\\.|[^"\\])*)"\s*,\s*'
    r'"(?P<desc>(?:\\.|[^"\\])*)"\s*,?\s*\}',
    re.MULTILINE,
)

PREFIX_BY_INDEX: list[dict[tuple[str, str], str]] = [
    {
        ("m", "sg"): "{noun} giace abbandonato qui, {body}",
        ("f", "sg"): "{noun} giace abbandonata qui, {body}",
        ("m", "pl"): "{noun} giacciono abbandonati qui, {body}",
        ("f", "pl"): "{noun} giacciono abbandonate qui, {body}",
    },
    {
        ("m", "sg"): "{noun} e' stato lasciato cadere, {body}",
        ("f", "sg"): "{noun} e' stata lasciata cadere, {body}",
        ("m", "pl"): "{noun} sono stati lasciati cadere, {body}",
        ("f", "pl"): "{noun} sono state lasciate cadere, {body}",
    },
    {
        ("m", "sg"): "{noun} riposa sul pavimento, {body}",
        ("f", "sg"): "{noun} riposa sul pavimento, {body}",
        ("m", "pl"): "{noun} riposano sul pavimento, {body}",
        ("f", "pl"): "{noun} riposano sul pavimento, {body}",
    },
    {
        ("m", "sg"): "{noun} e' stato dimenticato in un angolo, {body}",
        ("f", "sg"): "{noun} e' stata dimenticata in un angolo, {body}",
        ("m", "pl"): "{noun} sono stati dimenticati in un angolo, {body}",
        ("f", "pl"): "{noun} sono state dimenticate in un angolo, {body}",
    },
    {
        ("m", "sg"): "{noun} giace smarrito sulla pietra, {body}",
        ("f", "sg"): "{noun} giace smarrita sulla pietra, {body}",
        ("m", "pl"): "{noun} giacciono smarriti sulla pietra, {body}",
        ("f", "pl"): "{noun} giacciono smarrite sulla pietra, {body}",
    },
    {
        ("m", "sg"): "{noun} e' caduto qui, semidimenticato, {body}",
        ("f", "sg"): "{noun} e' caduta qui, semidimenticata, {body}",
        ("m", "pl"): "{noun} sono caduti qui, semidimenticati, {body}",
        ("f", "pl"): "{noun} sono cadute qui, semidimenticate, {body}",
    },
]


def lcfirst(text: str) -> str:
    return text[0].lower() + text[1:] if text else text


def classify_short(short: str) -> tuple[str, str]:
    s = short.strip().lower()
    if s.startswith(("i ", "gli ")):
        return "m", "pl"
    if s.startswith("le "):
        return "f", "pl"
    if s.startswith(("una ", "la ")):
        return "f", "sg"
    return "m", "sg"


def generic_noun(short: str) -> tuple[str, str, str]:
    """Ritorna (nome generico con articolo, genere, numero)."""
    raw = short.strip()
    s = raw.lower()
    if s.startswith("l'"):
        noun = raw[2:].split()[0]
        return f"un {noun.lower()}", "m", "sg"
    if s.startswith("il "):
        noun = raw[3:].split()[0]
        return f"un {noun.lower()}", "m", "sg"
    if s.startswith("lo "):
        noun = raw[3:].split()[0]
        return f"uno {noun.lower()}", "m", "sg"
    if s.startswith("la "):
        noun = raw[3:].split()[0]
        return f"una {noun.lower()}", "f", "sg"
    if s.startswith("una "):
        noun = raw[4:].split()[0]
        return f"una {noun.lower()}", "f", "sg"
    if s.startswith("un "):
        noun = raw[3:].split()[0]
        return f"un {noun.lower()}", "m", "sg"
    if s.startswith("uno "):
        noun = raw[4:].split()[0]
        return f"uno {noun.lower()}", "m", "sg"
    if s.startswith("i "):
        noun = raw[2:].split()[0]
        return f"i {noun.lower()}", "m", "pl"
    if s.startswith("gli "):
        noun = raw[4:].split()[0]
        return f"gli {noun.lower()}", "m", "pl"
    if s.startswith("le "):
        noun = raw[3:].split()[0]
        return f"le {noun.lower()}", "f", "pl"
    return "un oggetto", "m", "sg"


def load_git_originals() -> dict[tuple[str, str], str]:
    raw = subprocess.check_output(
        ["git", "show", "HEAD:src/procarea_reward_names.inc"],
        cwd=ROOT,
        text=True,
    )
    originals: dict[tuple[str, str], str] = {}
    for match in ORIGINAL_ENTRY_RE.finditer(raw):
        originals[(match.group("kw"), match.group("short"))] = match.group("desc")
    return originals


def simplify_body(body: str) -> str:
    body = body.strip().rstrip(".")
    body = re.sub(r":\s*", ", ", body)
    replacements = (
        (r"\bavvolge il dito destro\b", "avvolge l'anello come nebbia solida"),
        (r"\bavvolge il dito sinistro\b", "avvolge l'anello come nebbia solida"),
        (r"\bpende basso sul petto e tintinna\b", "una catena lunga tintinna"),
        (r"\bpende basso sul petto\b", "una catena lunga pende"),
        (r"\bpende lungo fino allo sterno\b", "pende lungo"),
        (r"\bpende sul cuore\b", "pende come nebbia solida"),
        (r"\bpende come un cuore di nebbia\b", "pende come nebbia solida"),
        (r"\bcinge il collo corto\b", "forma un cerchio stretto"),
        (r"\bcinge il collo come\b", "forma un cerchio come"),
        (r"\bcinge la fronte\b", "corre intorno alla calotta"),
        (r"\bsi intrecciano al collo\b", "si intrecciano nella catena"),
        (r"\bmutano disposizione ad ogni passo\b", "mutano al minimo movimento"),
        (r"\bmutano ad ogni passo\b", "mutano al minimo movimento"),
        (r"\bmutano ogni volta che distogli lo sguardo\b", "mutano al minimo soffio d'aria"),
        (r"\bmutano ogni volta che\b", "mutano al minimo soffio d'aria"),
        (r"\bcorre lungo il cranio\b", "corre lungo la calotta"),
        (r"\borbitano la testa come\b", "orbitano il cerchio come"),
        (r"\bRune sui ginocchi si accendono quando corri\b", "rune sulle ginocchiere brillano"),
        (r"\blungo le gambe come rivoli di bruma\b", "lungo i gambali come rivoli di bruma"),
        (r"\bogni passo sembra sul velo\b", "sembra fatta di velo"),
        (r"\bPer un istante dopo ogni passo il piede sembra non toccare terra\b", "le suole non sembrano lasciare impronta"),
        (r"\bmentre la guardi\b", "al minimo soffio d'aria"),
        (r"\bquando qualcuno ti guarda\b", "sotto uno sguardo fisso"),
        (r"\bDietro di te non proietta ombra\b", "disteso non proietta ombra"),
        (r"\bdietro di te non proietta ombra\b", "disteso non proietta ombra"),
        (r"\bnon proietta ombra e la assorbe\b", "non proietta ombra e la assorbe"),
        (r"\bnon proietta ombra: la assorbe\b", "non proietta ombra e la assorbe"),
        (r"\bogni falange porta un simbolo diverso che si accende al contatto\b", "ogni dito porta un simbolo che si accende al tocco"),
        (r"\bDita avvolte in velo solido: afferrano senza stringere\b", "le dita sono avvolte in velo solido"),
        (r"\ble dita sembrano avvolgere il vuoto\b", "le dita sembrano chiuse sul vuoto"),
        (r"\bcorrono lungo gli avambracci\b", "corrono lungo le piastre"),
        (r"\bpulsano al parare\b", "pulsano debolmente"),
        (r"\bformano un arco sul polso sinistro\b", "formano un arco sulla fascia sinistra"),
        (r"\bformano un arco sul polso destro\b", "formano un arco sulla fascia destra"),
        (r"\bcinge il polso sinistro\b", "chiude un cerchio stretto"),
        (r"\borbita il polso sinistro senza toccarlo\b", "orbita il cerchio senza toccarlo"),
        (r"\borbita il polso sinistro\b", "orbita intorno al cerchio"),
        (r"\bsi allarga ad ogni gesto\b", "sembra allargarsi al minimo scossone"),
        (r"\bpulsa nel palmo\b", "pulsa debolmente"),
        (r"\bcome rivoli di nebbia\b", "come filamenti di nebbia"),
        (r"\bfilamenti argentei attraversano la stoffa come rivoli di nebbia\b", "filamenti argentei attraversano la stoffa"),
        (r"\bfilamenti argentei attraversano la stoffa come filamenti di nebbia\b", "filamenti argentei attraversano la stoffa"),
        (r"\bcome memoria di un colpo reale\b", "in obliquo sul metallo"),
        (r"\buna crepa obliqua attraversa il metallo come memoria di un colpo reale\b", "una crepa obliqua attraversa il metallo"),
        (r"\bun frammento di sigillo della sala finale pulsa nel palmo\b", "un frammento di sigillo della sala finale pulsa debolmente"),
        (r"\bun sole miniatura e' intrappolato nel metallo al polso destro\b", "un sole miniatura e' intrappolato nel metallo"),
        (r"\bferro consumato al polso sinistro, ancora caldo al tatto\b", "ferro consumato, ancora caldo al tatto"),
        (r"\bcenere rossa si rigenera sul metallo al polso destro\b", "cenere rossa si rigenera sul metallo"),
        (r"\bun cerchio opaco al polso sinistro, dentro, nebbia che non esce mai\b", "nebbia chiusa in un cerchio opaco"),
        (r"\bun cerchio opaco sulla fascia sinistra, dentro, nebbia che non esce mai\b", "nebbia chiusa in un cerchio opaco"),
        (r"\buna spirale d'argento al polso destro si allarga ad ogni gesto\b", "una spirale d'argento sembra allargarsi"),
        (r"\bla spirale runica al polso destro corre verso l'interno\b", "la spirale runica corre verso l'interno"),
        (r"\buna stella a otto punte di bruma al polso destro\b", "una stella a otto punte di bruma"),
        (r"\btre anelli neri al polso sinistro tintinnano senza suono\b", "tre anelli neri tintinnano senza suono"),
        (r"\bun sigillo obliquo al polso destro si chiude da solo\b", "un sigillo obliquo si chiude da solo"),
        (r"\bal polso sinistro\b", "sulla fascia sinistra"),
        (r"\bal polso destro\b", "sulla fascia destra"),
        (r"\borbita il polso sinistro\b", "orbita intorno al bracciale sinistro"),
        (r"\borbita intorno al dito sinistro\b", "orbita intorno all'anello"),
        (r"\bvibra nel palmo\b", "vibra sul pavimento"),
        (r"\battorno al tuo polso\b", "intorno al gioiello"),
        (r"\badattano la vista alla nebbia fitta\b", "filtrano la nebbia fitta"),
        (r"\bfiltrano la nebbia nei cristalli\b", "filtrano la nebbia fitta"),
        (r"\bun cerchio opaco, dentro nebbia che non esce mai\b", "nebbia chiusa in un cerchio opaco"),
        (r"\bun cerchio opaco sulla fascia sinistra, dentro, nebbia che non esce mai\b", "nebbia chiusa in un cerchio opaco"),
        (r"\bscivola e torna al suo posto\b", "scivola e torna al centro"),
        (r"\buna stella a otto punte all'orecchio destro scintilla come bruma al sole\b", "una stella a otto punte scintilla come bruma al sole"),
        (r"\bcenere rossa si rigenera sul gioiello all'orecchio destro\b", "cenere rossa si rigenera sul gioiello"),
        (r"\bun sole miniatura all'orecchio destro e' intrappolato nel metallo\b", "un sole miniatura e' intrappolato nel metallo"),
        (r"\ball'orecchio sinistro\b", ""),
        (r"\ball'orecchio destro\b", ""),
        (r"\bIl petto e' protetto da una scocca\b", "la parte del petto e' protetta da una scocca"),
        (r"\bcome accesa dentro una bolla di nebbia\b", "come chiusa in una bolla di nebbia"),
        (r"\bcome la Piazza delle Nebbie\b", "come ghiaccio antico"),
        (r"\bL'argento sotto i piedi riflette stanze che non stai ancora vedendo\b", "l'argento sotto le suole riflette stanze lontane"),
        (r"\bvibra quando il custode cade\b", "vibra al minimo scossone"),
        (r"\bbrilla ad ogni incantesimo\b", "brilla quando la magia passa vicino"),
        (r"\bnebbia tessuta: si muove\b", "nebbia tessuta, si muove anche senza vento"),
        (r"\bne' assenza affilata\b", "e' assenza affilata"),
        (r"\bpiastre sottili di bruma indurita scivolano l'una sull'altra\b", "piastre di bruma indurita scivolano l'una sull'altra"),
        (r"\bframmenti di sigilli fusi nella maglia ricordano l'ultimo guardiano\b", "sigilli fusi nella maglia ricordano l'ultimo guardiano"),
        (r"\binciso con la croce obliqua di DarkStar\b", "incisa con la croce obliqua di DarkStar"),
        (r"\bcome appena tolto da una mano caduta\b", "come appena caduto da una mano"),
        (r"\bstrisce d'argento corrono lungo i gambali come rivoli di bruma\b", "strisce d'argento corrono sui gambali"),
        (r"\bstrisce d'argento corrono sulla stoffa come rivoli di bruma\b", "strisce d'argento corrono sulla stoffa"),
        (r"\bconsumati sulle nocche, intatti sul palmo\b", "consumati sulle nocche, intatti sul palmo interno"),
        (r"\bpalmi riflettenti come specchi di nebbia\b", "i palmi sono riflettenti come specchi di nebbia"),
        (r"\bun cerchio opaco del bracciale sinistro, dentro, nebbia che non esce mai\b", "un cerchio opaco, dentro nebbia che non esce mai"),
        (r"\bun cerchio opaco del bracciale sinistro: dentro\b", "un cerchio opaco, dentro nebbia che non esce mai"),
        (r"\bla spirale runica del bracciale destro corre verso l'interno\b", "la spirale runica corre verso l'interno"),
        (r"\buna spirale d'argento del bracciale destro si allarga\b", "una spirale d'argento sembra allargarsi"),
        (r"\bchiude un cerchio sul bracciale sinistro\b", "chiude un cerchio di nebbia"),
        (r"\buna mezzaluna di nebbia solida cinge il polso sinistro\b", "una mezzaluna di nebbia solida chiude un cerchio"),
        (r"\buna stella a otto punte di bruma del bracciale destro\b", "una stella a otto punte di bruma"),
        (r"\btre anelli neri del bracciale sinistro tintinnano senza suono\b", "tre anelli neri tintinnano senza suono"),
        (r"\bun sigillo obliquo del bracciale destro si chiude da solo\b", "un sigillo obliquo si chiude da solo"),
        (r"\bferro consumato del bracciale sinistro, ancora caldo al tatto\b", "ferro consumato, ancora caldo al tatto"),
        (r"\bcenere rossa si rigenera sul metallo del bracciale destro\b", "cenere rossa si rigenera sul metallo"),
        (r"\buna campanella di nebbia dell'orecchino sinistro tintinna senza vento\b", "una campanella di nebbia tintinna senza vento"),
        (r"\buna goccia opaca dell'orecchino destro scivola e torna al suo posto\b", "una goccia opaca scivola e torna al suo posto"),
        (r"\buna spirale runica dell'orecchino sinistro ruota lentamente\b", "una spirale runica ruota lentamente"),
        (r"\buna stella runica dell'orecchino destro brilla ad ogni incantesimo\b", "una stella runica brilla quando la magia passa vicino"),
        (r"\buna mezzaluna d'argento dell'orecchino sinistro cattura suoni lontani\b", "una mezzaluna d'argento cattura suoni lontani"),
        (r"\buna stella a otto punte dell'orecchino destro scintilla come bruma al sole\b", "una stella a otto punte scintilla come bruma al sole"),
        (r"\bun uncino nero dell'orecchino sinistro non graffia mai\b", "un uncino nero non graffia"),
        (r"\bun sigillo obliquo dell'orecchino destro si chiude da solo\b", "un sigillo obliquo si chiude da solo"),
        (r"\buna piastra consumata dell'orecchino sinistro vibra quando il custode cade\b", "una piastra consumata vibra al minimo scossone"),
        (r"\bun disco nero dell'orecchino sinistro assorbe rumori superflui\b", "un disco nero assorbe rumori superflui"),
        (r"\bun sole miniatura dell'orecchino destro e' intrappolato nel metallo\b", "un sole miniatura e' intrappolato nel metallo"),
        (r"\buna scheggia del sigillo del custode e' incastonata tra i vetri\b", "una scheggia del sigillo e' incastonata tra i vetri"),
        (r"\brune sui vetri si sovrappongono creando doppie immagini\b", "rune sui vetri creano doppie immagini"),
        (r"\bvetri neri filtrano la luce fino al varco minimo\b", "vetri neri filtrano la luce fino al minimo"),
        (r"\bi vetri assorbono ogni bagliore fino a lasciare solo contorni\b", "i vetri assorbono il bagliore e lasciano solo contorni"),
        (r"\bsottile come un ago di bruma: entra dove l'armatura esita\b", "sottile come un ago di bruma, entra dove l'armatura esita"),
        (r"\bl'ago e' piu' lungo del manico: entra e non chiede permesso\b", "l'ago e' piu' lungo del manico e entra senza chiedere permesso"),
        (r"\bancora affilato dove serviva al guardiano\b", "ancora affilato dove serviva"),
        (r"\bun cristallo a stella cattura la luce e la restituisce come bruma\b", "un cristallo a stella cattura e restituisce la luce"),
        (r"\bun frammento di sigillo della sala finale pulsa\b", "un frammento di sigillo della sala finale pulsa"),
        (r"\buna sfera nera trattiene al centro un nucleo violaceo\b", "una sfera nera trattiene un nucleo violaceo"),
        (r"\bbraci grigie si rigenerano da sole sul bastone di ossidiana\b", "braci grigie si rigenerano sul bastone"),
        (r"\bMetallo opaco, inciso con una runa spezzata a meta'\b", "metallo opaco, inciso con una runa spezzata"),
        (r"\binciso dentro c'e' una croce obliqua consumata dal tempo\b", "inciso con una croce obliqua consumata"),
        (r"\bDue meta' di gemma si cercano senza mai unirsi del tutto\b", "due meta' di gemma si cercano senza unirsi"),
        (r"\bun disco eclittico cinge il collo come un'eclisse fermata nel tempo\b", "un disco eclittico forma un cerchio come un'eclisse ferma"),
        (r"\bcome un'eclisse permanente\b", "come un'eclisse ferma"),
        (r"\bpiastre scure coprono ginocchia e stinchi\b", "piastre scure coprono ginocchia e stinchi"),
        (r"\bconsumati davanti e intatti dietro, con cenere dentro\b", "consumati davanti e intatti dietro, la cenere e' ancora dentro"),
        (r"\brammendata piu' volte dove il custode cadde in ginocchio\b", "rammendata dove il custode cadde in ginocchio"),
        (r"\bla fibbia non ha buco ne' perno, si chiude da sola\b", "la fibbia non ha buco ne' perno e si chiude da sola"),
        (r"\bintorno all'anello\b", "intorno ad esso"),
        (r"\bintorno al gioiello\b", "intorno ad esso"),
        (r"\bil metallo e' scuro e caldo\b", "il suo metallo e' scuro e caldo"),
        (r"\bil metallo e' opaco\b", "il suo metallo e' opaco"),
        (r"\bla gemma e' nera\b", "la sua gemma e' nera"),
        (r"\bla fiamma e' piccola\b", "la sua fiamma e' piccola"),
        (r"\bil fuoco e' bianco\b", "il suo fuoco e' bianco"),
        (r"\bla maschera integrata\b", "la sua maschera integrata"),
    )
    for pattern, repl in replacements:
        body = re.sub(pattern, repl, body, flags=re.I)
    body = re.sub(r"\s+", " ", body).strip(" ,;:")
    body = re.sub(r":\s*", ", ", body)
    if body and body[0].islower():
        body = body[0].upper() + body[1:]
    return body


def fit_text(text: str, max_len: int) -> str:
    cleaned = re.sub(r"\s+", " ", text).strip().rstrip(".")
    if len(cleaned) <= max_len:
        return cleaned + "."
    bad_endings = re.compile(
        r"\b(un|una|il|lo|la|i|gli|le|al|allo|alla|ai|agli|alle|"
        r"sul|sullo|sulla|sui|sugli|sulle|nel|nello|nella|nei|negli|nelle|"
        r"del|dello|della|dei|degli|delle|dal|dallo|dalla|dai|dagli|dalle|"
        r"di|da|in|a|e|o|che|come|con|per|senza|tra|fra|su|ne|se)$",
        re.I,
    )
    clauses = [part.strip() for part in cleaned.split(",")]
    while len(clauses) > 1:
        clauses.pop()
        candidate = ", ".join(clauses).rstrip(".,;:")
        if len(candidate) <= max_len and not bad_endings.search(candidate):
            return candidate + "."
    words = cleaned.split()
    while words and (
        len(" ".join(words)) > max_len or bad_endings.search(" ".join(words))
    ):
        words.pop()
    return " ".join(words).rstrip(".,;:") + "."


def to_ground(body: str, short: str, index: int) -> str:
    body = simplify_body(body)
    noun, gender, number = generic_noun(short)
    template = PREFIX_BY_INDEX[index % len(PREFIX_BY_INDEX)][(gender, number)]
    prefix_len = len(template.format(noun=noun, body=""))
    max_body = max(24, MAX_LEN - prefix_len)
    body = fit_text(lcfirst(body), max_body)
    filled = template.format(noun=noun, body=body.rstrip("."))
    if filled and filled[0].islower():
        filled = filled[0].upper() + filled[1:]
    return fit_text(filled, MAX_LEN)


def rewrite_file(text: str, originals: dict[tuple[str, str], str]) -> tuple[str, int]:
    changes = 0
    counter = 0

    def repl(match: re.Match[str]) -> str:
        nonlocal changes, counter
        kw = match.group("kw")
        short = match.group("short")
        key = (kw, short)
        source_body = originals.get(key, match.group("desc"))
        new_desc = to_ground(source_body, short, counter)
        counter += 1
        if new_desc != match.group("desc"):
            changes += 1
        if "\n" in match.group(0):
            return f'\t{{\n\t\t"{kw}",\n\t\t"{short}",\n\t\t"{new_desc}",\n\t}}'
        indent = "\t" if match.group(0).startswith("\t") else ""
        return f'{indent}{{ "{kw}", "{short}", "{new_desc}" }}'

    return ENTRY_RE.sub(repl, text), changes


def main() -> None:
    originals = load_git_originals()
    original = NAMES_INC.read_text(encoding="utf-8")
    updated, count = rewrite_file(original, originals)
    NAMES_INC.write_text(updated, encoding="utf-8")
    print(f"Rewrote {count} ground descriptions in {NAMES_INC}")


if __name__ == "__main__":
    main()
