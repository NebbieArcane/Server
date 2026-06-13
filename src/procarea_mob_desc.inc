/* procarea_mob_desc.inc
 * Testi mob Antro Effimero (19 archetipi). Sorgente unica per runtime e script bande.
 *
 * short_title: titolo senza articolo; procarea.cpp aggiunge Un/Una/Lo/L'.
 * long_desc: frase di presenza in stanza (con articolo).
 * look / sound: procarea.cpp aggiunge \\n\\r in spawn.
 * agg: messaggio aggressivita' (player.sounds).
 *
 * Ordine vettore = indice archetipo: boss 0-9, mob 0-7 (indici 10-17), trap 8 (18).
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
};

#define MOB_DESC(tier, role, slot, kw, st, lg, lk, ag, so) \
	{ kw, st, lg, lk, ag, so },

static const ProcMobArchetypeText kProcMobArchetypeTexts[] = {

MOB_DESC(1, boss, 0,
	"alfa nebbia effimera",
	"Alfa della Nebbia Effimera",
	"Un predatore alfa emerge dal vapore denso dell'antro.",
	"Zanne irregolari e occhi gialli non tradiscono pieta'.",
	"L'Alfa ringhia e la nebbia si addensa.",
	"Un ringhio squarcia il silenzio.")

MOB_DESC(1, boss, 1,
	"regina tarantole",
	"Regina delle Tarantole",
	"Una tarantola colossale domina un nido di seta e ossa.",
	"Setole urticanti coprono zampe spesse come tronchi.",
	"La Regina scatta lateralmente.",
	"Ragnatele vibrano.")

MOB_DESC(1, boss, 2,
	"guardiano muschio antico",
	"Guardiano di Muschio Antico",
	"Un colosso vegetale blocca la sala con radici animate.",
	"Muschio vivo pulsa come un secondo cuore.",
	"Il Guardiano schianta i pugni a terra.",
	"Radici scoppiano fuori.")

MOB_DESC(1, boss, 3,
	"predatore cieco",
	"Predatore Cieco delle Profondita'",
	"Una bestia senza occhi percepisce ogni tuo movimento.",
	"Artigli lunghi hanno scritto segni sulle pareti.",
	"Il predatore si lancia all'istinto.",
	"Un urlo cavernoso.")

MOB_DESC(1, boss, 4,
	"matriarca vermi",
	"Matriarca dei Vermi Bianchetti",
	"Una montagna gelatinosa regge un trono di larve.",
	"Vermi obbedienti si gettano contro gli intrusi.",
	"La Matriarca convoca vermi.",
	"Schizzi viscidi.")

MOB_DESC(1, boss, 5,
	"signore pipistrelli",
	"Signore dei Pipistrelli Profondi",
	"Una creatura alata coronata di ossa volteggia bassa.",
	"Canini sporgono da un muso allungato e famelico.",
	"Il Signore piomba dal soffitto.",
	"Ali battono ovunque.")

MOB_DESC(1, boss, 6,
	"capo saprofagi",
	"Capo dei Saprofagi",
	"Un insettoide putrefatto comanda sciami affamati.",
	"Mandibole gocciolano liquido nero corrosivo.",
	"Il Capo spruzza acido.",
	"Odore di carogne.")

MOB_DESC(1, boss, 7,
	"custode fosso",
	"Custode del Fosso",
	"Un guerriero rannicchiato sorveglia un baratro laterale.",
	"Armatura e' incrostata di fango secco e sangue.",
	"Il Custode spinge verso il vuoto.",
	"Pietre cadono nel baratro.")

MOB_DESC(1, boss, 8,
	"re larve",
	"Re delle Larve",
	"Un sovrano viscido e' nato dal cumulo di generazioni.",
	"Occhi multipli si aprono e chiudono a ondate.",
	"Il Re convoca larve reali.",
	"Brusio innaturale.")

MOB_DESC(1, boss, 9,
	"ombra cunicolo",
	"Ombra del Primo Cunicolo",
	"La prima ombra nata nell'antro si stacca dal muro.",
	"Non ha volto: solo fame antica.",
	"L'Ombra ti avvolge.",
	"Il freddo morde le ossa.")

MOB_DESC(1, mob, 0,
	"larva nebbia effimera",
	"Larva della Nebbia Effimera",
	"Una larva traslucida striscia lasciando una scia di vapore gelido.",
	"La pelle viscida pulsa come se respirasse nebbia compressa.",
	"La larva si inarca minacciosa.",
	"Senti un sibilo umido.")

MOB_DESC(1, mob, 1,
	"sciamatore caverna",
	"Sciamatore di Caverna",
	"Un insetto corazzato fruga tra le pietre con antenne tese.",
	"Mandibole lucide hanno morsicato ossa recenti.",
	"Lo sciamatore batte le ali.",
	"Un ronzio acuto riempie la stanza.")

MOB_DESC(1, mob, 2,
	"gnomo fessura",
	"Gnomo delle Fessure",
	"Un piccolo umanoide dalle mani artigliate ti osserva dall'ombra.",
	"Occhi troppo grandi brillano di malizia famelica.",
	"Lo gnomo sibila un'imprecazione.",
	"Senti un riso stridulo.")

MOB_DESC(1, mob, 3,
	"pipistrello vorace",
	"Pipistrello Vorace",
	"Un pipistrello grosso come un cane volteggia sopra la tua testa.",
	"Canini sporgono da un muso allungato e famelico.",
	"Il pipistrello piomba giu'.",
	"Ali scalciano l'aria.")

MOB_DESC(1, mob, 4,
	"rana pietra",
	"Rana di Pietra",
	"Una rana massiccia mimetizzata tra i massi attende immobile.",
	"La pelle sembra pietra viva; solo gli occhi si muovono.",
	"La rana si scaglia con la lingua.",
	"Un colpo secco sulle pietre.")

MOB_DESC(1, mob, 5,
	"servitore muschio",
	"Servitore di Muschio",
	"Un umanoide ricoperto di muschio blocka il passaggio.",
	"Radici sottili serpeggiano lungo le sue gambe.",
	"Il servitore alza braccia nodose.",
	"La terra scricchiola sotto di lui.")

MOB_DESC(1, mob, 6,
	"cane radici",
	"Cane delle Radici",
	"Un cane magro dalle zampe simili a radici ringhia basso.",
	"Pelliccia impregnata di terra e sangue secco.",
	"Il cane si lancia avanti.",
	"Un ringhio profondo.")

MOB_DESC(1, mob, 7,
	"predatore giovane",
	"Predatore delle Profondita'",
	"Un giovane predatore delle caverne ti fissa senza paura.",
	"Zanne non ancora mature, ma gia' affilate.",
	"Il predatore si prepara al balzo.",
	"Un ululato lontano.")

MOB_DESC(1, trap, 8,
	"trappola anguilla fango",
	"Anguilla del Fango",
	"Un serpente viscido emerge dal fango con bocca spalancata.",
	"La pelle oleosa rende difficile colpirla.",
	"L'anguilla si avvolge verso di te.",
	"Schizzi di fango ovunque.")

};

#undef MOB_DESC

#endif /* __PROCAREA_MOB_DESC_INC */
