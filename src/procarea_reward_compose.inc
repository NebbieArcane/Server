/*ALARMUD* (Do not remove *ALARMUD*, used to automagically manage these lines
 *ALARMUD* AlarMUD 2.0
 *ALARMUD* See COPYING for licence information
 *ALARMUD*/
/*
 * Dimensione Effimera: nomi composti per gli oggetti premio.
 *
 * Schema: <articolo+nome> [lato] <materiale per fascia> [epiteto del tema]
 *   es. "il focus d'osso della tana"
 *
 * Materiale: ogni materiale ha una CLASSE (maschera kMat*) e un prestigio (0 umile,
 * 1 normale, 2 pregiato). Il materiale va scelto tra quelli compatibili con il nome
 * (maschera del nome, o default dello slot/famiglia) e con la fascia (gruppo di
 * materiali); tra i compatibili si preferisce il prestigio piu' vicino a quello che
 * ProcComposeQuality ricava dal tiro reale dell'oggetto (bonus, resistenze, ecc.).
 * I nomi con materiale proprio ("anello d'osso", "strisce di cuoio") non ne ricevono.
 *
 * Regola assoluta: corazza (Body) con resistenza IMM_BLUNT = "la corazza di skaglie
 * di drago" (scritto cosi' di proposito: "skaglie" con la k).
 *
 * Come gli altri .inc di procarea_spawn.cpp: nessun namespace qui, viene incluso
 * dal .cpp dentro Alarmud::procarea_internal, DOPO procarea_themes.inc (serve
 * kThemeSets per lo static_assert sul numero di temi). Solo testo ASCII.
 *
 * Limiti (rent: name[128] / sd[128] / desc[256], DB varchar(128/128/256)):
 * short <= 50, keywords <= 60, long <= 120.
 */
#ifndef __PROCAREA_REWARD_COMPOSE_INC
#define __PROCAREA_REWARD_COMPOSE_INC

/* Genere e numero del nome: indice nelle forme dei materiali. */
enum class ProcComposeGN : int { MS = 0, FS, MP, FP, Count };

/* Classi di materiale (maschera di bit). */
inline constexpr unsigned kMatSoft = 1u << 0;  /* stoffa, cuoio, pelle, seta, lana */
inline constexpr unsigned kMatWood = 1u << 1;  /* legni */
inline constexpr unsigned kMatMetal = 1u << 2; /* metalli e leghe */
inline constexpr unsigned kMatStone = 1u << 3; /* pietre, gemme, cristallo, vetro, ghiaccio */
inline constexpr unsigned kMatBone = 1u << 4;  /* osso, avorio, chitina, conchiglie, ambra */
inline constexpr unsigned kMatMagic = 1u << 5; /* eclisse, stella, luna, ombra, luce */
inline constexpr unsigned kMatAny = kMatSoft | kMatWood | kMatMetal | kMatStone | kMatBone | kMatMagic;

struct ProcComposeNoun {
	const char* text;  /* nome SENZA articolo ("anello", "strisce di cuoio"): l'articolo
						* e' generato da compose_with_article() da genere/numero/iniziale */
	const char* kw;    /* keyword del nome */
	ProcComposeGN gn;
	bool sided;        /* bracciali: aggiunge sinistro/destro concordato */
	bool own_material; /* il nome contiene gia' un materiale ("anello d'osso"): niente
						* materiale di fascia (si compone solo nome + epiteto) */
	unsigned mats;     /* classi di materiale ammesse; 0 = default dello slot/famiglia */
};

struct ProcComposeNounList {
	const ProcComposeNoun* items;
	std::size_t count;
	const char* base_kw;    /* keyword di slot, sempre presente */
	unsigned default_mats;  /* classi ammesse per i nomi con mats == 0 */
};

template <std::size_t N>
constexpr ProcComposeNounList compose_nouns(const ProcComposeNoun (&items)[N], const char* base_kw,
											unsigned default_mats) noexcept {
	return {items, N, base_kw, default_mats};
}

/* ---- Nomi per slot ---------------------------------------------------- */

static constexpr ProcComposeNoun kComposeLight[] = {
	{"lanterna", "lanterna", ProcComposeGN::FS, false, false, kMatMetal},
	{"candela", "candela", ProcComposeGN::FS, false, true, 0},
	{"lume", "lume", ProcComposeGN::MS, false, false, kMatMetal | kMatStone},
	{"stella", "stella", ProcComposeGN::FS, false, true, 0},
	{"luce", "luce", ProcComposeGN::FS, false, true, 0},
	{"fiamma", "fiamma", ProcComposeGN::FS, false, true, 0},
	{"torcia", "torcia", ProcComposeGN::FS, false, false, kMatWood},
	{"globo", "globo", ProcComposeGN::MS, false, false, kMatStone | kMatMagic},
	{"globi", "globi", ProcComposeGN::MP, false, false, kMatStone | kMatMagic},
	{"fiaccola", "fiaccola", ProcComposeGN::FS, false, false, kMatWood},
	{"lampada", "lampada", ProcComposeGN::FS, false, false, kMatMetal | kMatStone},
	{"sfera", "sfera", ProcComposeGN::FS, false, false, kMatStone | kMatMagic},
	{"gemma", "gemma", ProcComposeGN::FS, false, true, 0},
	{"diamante", "diamante", ProcComposeGN::MS, false, true, 0},
	{"frammento di stella", "frammento stella", ProcComposeGN::MS, false, true, 0},
	{"lucerna", "lucerna", ProcComposeGN::FS, false, false, kMatMetal},
	{"fatina", "fatina", ProcComposeGN::FS, false, true, 0},
	{"lanterna da minatore", "lanterna minatore", ProcComposeGN::FS, false, false, kMatMetal},
};
static constexpr ProcComposeNoun kComposeFinger[] = {
	{"anello", "anello", ProcComposeGN::MS, false, false, 0},
	{"fede", "fede", ProcComposeGN::FS, false, false, kMatMetal},
	{"sigillo", "sigillo", ProcComposeGN::MS, false, false, kMatMetal | kMatStone},
	{"vera", "vera", ProcComposeGN::FS, false, false, kMatMetal},
	{"fedina", "fedina", ProcComposeGN::FS, false, false, kMatMetal},
	{"artigli", "artigli", ProcComposeGN::MP, false, false, kMatBone | kMatMetal},
	{"solitario", "solitario", ProcComposeGN::MS, false, false, kMatMetal},
	{"ditale", "ditale", ProcComposeGN::MS, false, false, kMatMetal | kMatBone | kMatWood},
	{"anellino", "anellino", ProcComposeGN::MS, false, false, 0},
	{"cerchietto", "cerchietto", ProcComposeGN::MS, false, false, kMatMetal | kMatBone | kMatWood},
	{"fascetta", "fascetta", ProcComposeGN::FS, false, false, kMatMetal | kMatSoft},
};
static constexpr ProcComposeNoun kComposeNeck[] = {
	{"collana", "collana", ProcComposeGN::FS, false, false, kMatMetal | kMatStone | kMatBone | kMatSoft},
	{"amuleto", "amuleto", ProcComposeGN::MS, false, false, 0},
	{"collare", "collare", ProcComposeGN::MS, false, false, kMatMetal | kMatSoft | kMatBone},
	{"pendente", "pendente", ProcComposeGN::MS, false, false, 0},
	{"medaglione", "medaglione", ProcComposeGN::MS, false, false, kMatMetal | kMatStone},
	{"catenina", "catenina", ProcComposeGN::FS, false, false, kMatMetal},
	{"pendaglio", "pendaglio", ProcComposeGN::MS, false, false, 0},
	{"collanina", "collanina", ProcComposeGN::FS, false, false, kMatMetal | kMatStone | kMatBone},
	{"girocollo", "girocollo", ProcComposeGN::MS, false, false, kMatSoft | kMatMetal},
	{"ciondolo", "ciondolo", ProcComposeGN::MS, false, false, 0},
	{"scarabeo", "scarabeo", ProcComposeGN::MS, false, false, kMatStone | kMatMetal | kMatBone},
	{"talismano", "talismano", ProcComposeGN::MS, false, false, 0},
	{"monile", "monile", ProcComposeGN::MS, false, false, kMatMetal | kMatStone},
};
static constexpr ProcComposeNoun kComposeBody[] = {
	{"corazza", "corazza", ProcComposeGN::FS, false, false, kMatMetal | kMatBone | kMatMagic},
	{"usbergo", "usbergo", ProcComposeGN::MS, false, false, kMatMetal},
	{"cotta", "cotta", ProcComposeGN::FS, false, false, kMatMetal | kMatSoft},
	{"pettorale", "pettorale", ProcComposeGN::MS, false, false, kMatMetal | kMatBone},
	{"armatura", "armatura", ProcComposeGN::FS, false, false, kMatMetal | kMatBone | kMatMagic},
	{"giubba", "giubba", ProcComposeGN::FS, false, false, kMatSoft},
	{"corpetto", "corpetto", ProcComposeGN::MS, false, false, kMatSoft | kMatMetal},
	{"giustacuore", "giustacuore", ProcComposeGN::MS, false, false, kMatSoft},
	{"veste", "veste", ProcComposeGN::FS, false, false, kMatSoft | kMatMagic},
	{"saio", "saio", ProcComposeGN::MS, false, false, kMatSoft},
	{"tunica", "tunica", ProcComposeGN::FS, false, false, kMatSoft},
	{"casacca", "casacca", ProcComposeGN::FS, false, false, kMatSoft},
	{"gambesone", "gambesone", ProcComposeGN::MS, false, false, kMatSoft},
	{"brigantina", "brigantina", ProcComposeGN::FS, false, false, kMatMetal | kMatSoft},
	{"loriga", "loriga", ProcComposeGN::FS, false, false, kMatMetal},
};
static constexpr ProcComposeNoun kComposeHead[] = {
	{"elmo", "elmo", ProcComposeGN::MS, false, false, 0},
	{"corona", "corona", ProcComposeGN::FS, false, false, kMatMetal | kMatStone | kMatMagic},
	{"cappuccio", "cappuccio", ProcComposeGN::MS, false, false, kMatSoft},
	{"diadema", "diadema", ProcComposeGN::MS, false, false, kMatMetal | kMatStone | kMatMagic},
	{"copricapo", "copricapo", ProcComposeGN::MS, false, false, kMatSoft},
	{"elmetto", "elmetto", ProcComposeGN::MS, false, false, 0},
	{"celata", "celata", ProcComposeGN::FS, false, false, 0},
	{"bacinetto", "bacinetto", ProcComposeGN::MS, false, false, 0},
	{"cappello", "cappello", ProcComposeGN::MS, false, false, kMatSoft},
	{"bandana", "bandana", ProcComposeGN::FS, false, false, kMatSoft},
	{"fascia", "fascia", ProcComposeGN::FS, false, false, kMatSoft},
	{"tiara", "tiara", ProcComposeGN::FS, false, false, kMatMetal | kMatStone | kMatMagic},
	{"barbuta", "barbuta", ProcComposeGN::FS, false, false, kMatMetal},
	{"morione", "morione", ProcComposeGN::MS, false, false, kMatMetal},
	{"cervelliera", "cervelliera", ProcComposeGN::FS, false, false, kMatMetal},
};
static constexpr ProcComposeNoun kComposeLegs[] = {
	{"gambali", "gambali", ProcComposeGN::MP, false, false, kMatMetal | kMatSoft | kMatBone},
	{"schinieri", "schinieri", ProcComposeGN::MP, false, false, kMatMetal | kMatBone},
	{"chausses", "chausses", ProcComposeGN::FP, false, false, kMatMetal | kMatSoft},
	{"strisce di cuoio", "strisce cuoio", ProcComposeGN::FP, false, true, 0},
	{"stecche di ferro", "stecche ferro", ProcComposeGN::FP, false, true, 0},
	{"pantaloni", "pantaloni", ProcComposeGN::MP, false, false, kMatSoft},
	{"calze", "calze", ProcComposeGN::FP, false, false, kMatSoft},
	{"fasciature", "fasciature", ProcComposeGN::FP, false, false, kMatSoft},
	{"protezioni", "protezioni", ProcComposeGN::FP, false, false, kMatMetal | kMatSoft | kMatBone},
	{"cosciali", "cosciali", ProcComposeGN::MP, false, false, kMatMetal | kMatBone},
	{"gambiere", "gambiere", ProcComposeGN::FP, false, false, kMatMetal | kMatSoft | kMatBone},
	{"calzoni", "calzoni", ProcComposeGN::MP, false, false, kMatSoft},
};
static constexpr ProcComposeNoun kComposeFeet[] = {
	{"stivali", "stivali", ProcComposeGN::MP, false, false, kMatSoft | kMatMetal},
	{"calzari", "calzari", ProcComposeGN::MP, false, false, kMatSoft | kMatMetal},
	{"scarpe", "scarpe", ProcComposeGN::FP, false, false, 0},
	{"scarpini", "scarpini", ProcComposeGN::MP, false, false, 0},
	{"poulaines", "poulaines", ProcComposeGN::FP, false, false, 0},
	{"scarpette", "scarpette", ProcComposeGN::FP, false, false, 0},
	{"pantoffole", "pantoffole", ProcComposeGN::FP, false, false, 0},
	{"sandali", "sandali", ProcComposeGN::MP, false, false, kMatSoft | kMatWood},
	{"scarponi", "scarponi", ProcComposeGN::MP, false, false, kMatSoft | kMatMetal},
	{"coturni", "coturni", ProcComposeGN::MP, false, false, kMatSoft | kMatMetal},
	{"babbucce", "babbucce", ProcComposeGN::FP, false, false, 0},
	{"zoccoli", "zoccoli", ProcComposeGN::MP, false, false, kMatWood},
	{"calzature", "calzature", ProcComposeGN::FP, false, false, 0},
};
static constexpr ProcComposeNoun kComposeHands[] = {
	{"guanti", "guanti", ProcComposeGN::MP, false, false, 0},
	{"manopole", "manopole", ProcComposeGN::FP, false, false, kMatMetal | kMatSoft},
	{"mezziguanti", "mezziguanti", ProcComposeGN::MP, false, false, 0},
	{"guantoni", "guantoni", ProcComposeGN::MP, false, false, 0},
	{"fasce", "fasce", ProcComposeGN::FP, false, false, 0},
	{"placche", "placche", ProcComposeGN::FP, false, false, kMatMetal | kMatBone},
	{"gusci", "gusci", ProcComposeGN::MP, false, false, kMatBone | kMatMetal | kMatStone},
	{"uncini", "uncini", ProcComposeGN::MP, false, false, kMatMetal | kMatBone},
	{"tirapugni", "tirapugni", ProcComposeGN::MP, false, false, kMatMetal | kMatBone | kMatStone},
	{"guantelli", "guantelli", ProcComposeGN::MP, false, false, kMatMetal | kMatSoft},
};
static constexpr ProcComposeNoun kComposeArms[] = {
	{"parabraccia", "parabraccia", ProcComposeGN::MP, false, false, 0},
	{"avambracci", "avambracci", ProcComposeGN::MP, false, false, 0},
	{"maniche", "maniche", ProcComposeGN::FP, false, false, kMatSoft},
	{"bracciali", "bracciali", ProcComposeGN::MP, false, false, 0},
	{"lastre", "lastre", ProcComposeGN::FP, false, false, kMatMetal | kMatBone},
	{"cubitiere", "cubitiere", ProcComposeGN::FP, false, false, kMatMetal | kMatBone},
	{"bende", "bende", ProcComposeGN::FP, false, false, kMatSoft},
	{"gomitiere", "gomitiere", ProcComposeGN::FP, false, false, 0},
	{"protezioni", "protezioni", ProcComposeGN::FP, false, false, 0},
	{"paragomiti", "paragomiti", ProcComposeGN::MP, false, false, kMatMetal | kMatBone},
};
static constexpr ProcComposeNoun kComposeAbout[] = {
	{"mantello", "mantello", ProcComposeGN::MS, false, false, 0},
	{"cappa", "cappa", ProcComposeGN::FS, false, false, 0},
	{"scialle", "scialle", ProcComposeGN::MS, false, false, 0},
	{"mantella", "mantella", ProcComposeGN::FS, false, false, 0},
	{"giornea", "giornea", ProcComposeGN::FS, false, false, 0},
	{"tabardo", "tabardo", ProcComposeGN::MS, false, false, 0},
	{"pelicon", "pelicon", ProcComposeGN::MS, false, false, 0},
	{"manto", "manto", ProcComposeGN::MS, false, false, 0},
	{"pelliccia", "pelliccia", ProcComposeGN::FS, false, true, 0},
	{"vello", "vello", ProcComposeGN::MS, false, true, 0},
	{"tabarro", "tabarro", ProcComposeGN::MS, false, false, 0},
	{"pianeta", "pianeta", ProcComposeGN::FS, false, false, 0},
	{"caftano", "caftano", ProcComposeGN::MS, false, false, 0},
	{"mantellina", "mantellina", ProcComposeGN::FS, false, false, 0},
	{"clamide", "clamide", ProcComposeGN::FS, false, false, 0},
};
static constexpr ProcComposeNoun kComposeWaist[] = {
	{"cintura", "cintura", ProcComposeGN::FS, false, false, kMatSoft | kMatMetal},
	{"cinturone", "cinturone", ProcComposeGN::MS, false, false, kMatSoft | kMatMetal},
	{"fascia", "fascia", ProcComposeGN::FS, false, false, 0},
	{"corda", "corda", ProcComposeGN::FS, false, false, 0},
	{"cingolo", "cingolo", ProcComposeGN::MS, false, false, kMatSoft | kMatMetal},
	{"baltei", "baltei", ProcComposeGN::MP, false, false, kMatSoft | kMatMetal},
	{"nastro", "nastro", ProcComposeGN::MS, false, false, 0},
	{"liana", "liana", ProcComposeGN::FS, false, true, 0},
	{"catena", "catena", ProcComposeGN::FS, false, false, kMatMetal},
	{"fusciacca", "fusciacca", ProcComposeGN::FS, false, false, 0},
	{"cordone", "cordone", ProcComposeGN::MS, false, false, 0},
	{"cinghia", "cinghia", ProcComposeGN::FS, false, false, 0},
	{"bandoliera", "bandoliera", ProcComposeGN::FS, false, false, 0},
	{"gonnellino", "gonnellino", ProcComposeGN::MS, false, false, 0},
};
static constexpr ProcComposeNoun kComposeWrist[] = {
	{"bracciale", "bracciale", ProcComposeGN::MS, true, false, 0},
	{"polsino", "polsino", ProcComposeGN::MS, true, false, kMatSoft},
	{"manetta", "manetta", ProcComposeGN::FS, true, false, kMatMetal},
	{"braccialetto", "braccialetto", ProcComposeGN::MS, true, false, kMatMetal | kMatStone | kMatBone | kMatWood},
	{"bracciali", "bracciali", ProcComposeGN::MP, false, false, 0},
	{"cinturino", "cinturino", ProcComposeGN::MS, true, false, kMatSoft},
	{"fascia", "fascia", ProcComposeGN::FS, true, false, kMatSoft},
	{"cerchietto", "cerchietto", ProcComposeGN::MS, true, false, kMatMetal | kMatBone},
	{"manette", "manette", ProcComposeGN::FP, false, false, kMatMetal},
};
static constexpr ProcComposeNoun kComposeHold[] = {
	{"focus", "focus", ProcComposeGN::MS, false, false, kMatStone | kMatMetal | kMatMagic},
	{"sfera", "sfera", ProcComposeGN::FS, false, false, 0},
	{"prisma", "prisma", ProcComposeGN::MS, false, false, 0},
	{"cristallo", "cristallo", ProcComposeGN::MS, false, true, 0},
	{"dado", "dado", ProcComposeGN::MS, false, false, kMatWood | kMatBone | kMatStone},
	{"dadi", "dadi", ProcComposeGN::MP, false, false, kMatWood | kMatBone | kMatStone},
	{"teschio", "teschio", ProcComposeGN::MS, false, false, kMatBone | kMatStone | kMatMetal | kMatMagic},
	{"osso", "osso", ProcComposeGN::MS, false, true, 0},
	{"pietra", "pietra", ProcComposeGN::FS, false, false, kMatStone},
	{"pietre", "pietre", ProcComposeGN::FP, false, false, kMatStone},
	{"roccia", "roccia", ProcComposeGN::FS, false, false, kMatStone},
	{"gemma", "gemma", ProcComposeGN::FS, false, true, 0},
	{"tomo", "tomo", ProcComposeGN::MS, false, false, kMatSoft},
	{"grimorio", "grimorio", ProcComposeGN::MS, false, false, kMatSoft},
	{"corno", "corno", ProcComposeGN::MS, false, false, kMatBone | kMatMetal},
	{"rosario", "rosario", ProcComposeGN::MS, false, false, kMatWood | kMatBone | kMatStone},
	{"bacchetta", "bacchetta", ProcComposeGN::FS, false, false, kMatWood | kMatBone | kMatMetal},
	{"vincastro", "vincastro", ProcComposeGN::MS, false, false, kMatWood | kMatBone},
	{"caduceo", "caduceo", ProcComposeGN::MS, false, false, kMatMetal | kMatWood | kMatMagic},
	{"rametto", "rametto", ProcComposeGN::MS, false, false, kMatWood | kMatBone},
	{"rabdo", "rabdo", ProcComposeGN::MS, false, false, kMatWood | kMatMetal},
};
static constexpr ProcComposeNoun kComposeEar[] = {
	{"orecchino", "orecchino", ProcComposeGN::MS, false, false, 0},
	{"pendaglio", "pendaglio", ProcComposeGN::MS, false, false, 0},
	{"perla", "perla", ProcComposeGN::FS, false, true, 0},
	{"anello di ferro", "anello ferro", ProcComposeGN::MS, false, true, 0},
	{"anello d'osso", "anello osso", ProcComposeGN::MS, false, true, 0},
	{"pendente", "pendente", ProcComposeGN::MS, false, false, 0},
	{"monile", "monile", ProcComposeGN::MS, false, false, 0},
	{"gioiello", "gioiello", ProcComposeGN::MS, false, false, 0},
	{"buccola", "buccola", ProcComposeGN::FS, false, false, 0},
	{"orecchini", "orecchini", ProcComposeGN::MP, false, false, 0},
	{"cerchio", "cerchio", ProcComposeGN::MS, false, false, kMatMetal | kMatBone},
	{"gocciola", "gocciola", ProcComposeGN::FS, false, false, kMatStone | kMatMetal | kMatBone},
	{"anellino", "anellino", ProcComposeGN::MS, false, false, 0},
};
static constexpr ProcComposeNoun kComposeEyes[] = {
	{"lenti", "lenti", ProcComposeGN::FP, false, false, 0},
	{"occhiali", "occhiali", ProcComposeGN::MP, false, false, 0},
	{"benda", "benda", ProcComposeGN::FS, false, false, kMatSoft},
	{"maschera", "maschera", ProcComposeGN::FS, false, false, kMatSoft | kMatMetal | kMatWood | kMatBone},
	{"lente", "lente", ProcComposeGN::FS, false, false, 0},
	{"fascia", "fascia", ProcComposeGN::FS, false, false, kMatSoft},
	{"monocolo", "monocolo", ProcComposeGN::MS, false, false, 0},
	{"occhialoni", "occhialoni", ProcComposeGN::MP, false, false, 0},
	{"visiera", "visiera", ProcComposeGN::FS, false, false, kMatMetal | kMatSoft},
	{"mascherina", "mascherina", ProcComposeGN::FS, false, false, kMatSoft},
	{"visore", "visore", ProcComposeGN::MS, false, false, kMatMetal | kMatStone},
};

/* Scudi premio (vnum 65100+, ITEM_ARMOR con wear flag ITEM_WEAR_SHIELD scritti nel
 * file oggetto al boot: il restring non li tocca). Base keyword sempre "scudo",
 * anche se i nomi fissi di fascia 0/4/6 iniziano con disco/buckler/pavese. */
static constexpr ProcComposeNoun kComposeShield[] = {
	{"scudo", "scudo", ProcComposeGN::MS, false, false, 0},
	{"brocchiero", "brocchiero", ProcComposeGN::MS, false, false, kMatMetal | kMatWood},
	{"targa", "targa", ProcComposeGN::FS, false, false, kMatMetal | kMatWood},
	{"pavese", "pavese", ProcComposeGN::MS, false, false, kMatWood | kMatMetal},
	{"rotella", "rotella", ProcComposeGN::FS, false, false, kMatWood | kMatMetal},
	{"scutum", "scutum", ProcComposeGN::MS, false, false, kMatWood | kMatMetal},
	{"buckler", "buckler", ProcComposeGN::MS, false, false, kMatMetal},
	{"scudo a goccia", "scudo goccia", ProcComposeGN::MS, false, false, 0},
	{"scudo rotondo", "scudo rotondo", ProcComposeGN::MS, false, false, 0},
	{"scudo triangolare", "scudo triangolare", ProcComposeGN::MS, false, false, 0},
	{"scudo crociato", "scudo crociato", ProcComposeGN::MS, false, false, 0},
};
static constexpr ProcComposeNounList kComposeShieldNouns =
	compose_nouns(kComposeShield, "scudo", kMatWood | kMatMetal | kMatBone);

/* ---- Armi (Wield): nomi per famiglia, raggruppati per tipo di danno ------------
 * La famiglia e' quella del template (0 taglio, 1 punta, 2 botta, come i 3
 * sub-variant di kProcRewardWeaponDamageValue). Il tipo di danno (value[3] dell'oggetto,
 * vedi Getw_type in fight.cpp) viene scelto a caso DENTRO la famiglia, poi il nome tra
 * quelli di quel tipo. Le famiglie coincidono con i gruppi di resistenza di fight.cpp
 * (IMM_SLASH / IMM_PIERCE / IMM_BLUNT). Esclusi BITE e RANGE_WEAPON. */
struct ProcComposeWeaponNoun {
	ProcComposeNoun noun;
	int damage_type; /* value[3]: 0 SMITE 1 STAB 2 WHIP 3 SLASH 4 SMASH 5 CLEAVE 6 CRUSH
						7 BLUDGEON 8 CLAW 10 STING 11 PIERCE 12 BLAST */
};

static constexpr ProcComposeWeaponNoun kComposeWeaponSlash[] = {
	{{"spada", "spada", ProcComposeGN::FS, false, false, 0}, 3},
	{{"sciabola", "sciabola", ProcComposeGN::FS, false, false, 0}, 3},
	{{"gladio", "gladio", ProcComposeGN::MS, false, false, 0}, 3},
	{{"scimitarra", "scimitarra", ProcComposeGN::FS, false, false, 0}, 3},
	{{"spada corta", "spada corta", ProcComposeGN::FS, false, false, 0}, 3},
	{{"spadone", "spadone", ProcComposeGN::MS, false, false, 0}, 3},
	{{"katana", "katana", ProcComposeGN::FS, false, false, 0}, 3},
	{{"falce", "falce", ProcComposeGN::FS, false, false, kMatMetal | kMatBone}, 3},
	{{"flamberga", "flamberga", ProcComposeGN::FS, false, false, 0}, 3},
	{{"ascia", "ascia", ProcComposeGN::FS, false, false, 0}, 5},
	{{"scure", "scure", ProcComposeGN::FS, false, false, 0}, 5},
	{{"mannaia", "mannaia", ProcComposeGN::FS, false, false, 0}, 5},
	{{"azza", "azza", ProcComposeGN::FS, false, false, 0}, 5},
	{{"falcione", "falcione", ProcComposeGN::MS, false, false, 0}, 5},
	{{"ascia bipenne", "ascia bipenne", ProcComposeGN::FS, false, false, 0}, 5},
	{{"alabarda", "alabarda", ProcComposeGN::FS, false, false, 0}, 5},
	{{"tomahawk", "tomahawk", ProcComposeGN::MS, false, false, 0}, 5},
	{{"flagello", "flagello", ProcComposeGN::MS, false, false, kMatSoft | kMatMetal}, 2},
	{{"frusta", "frusta", ProcComposeGN::FS, false, false, kMatSoft | kMatBone}, 2},
	{{"frustino", "frustino", ProcComposeGN::MS, false, false, kMatSoft | kMatBone}, 2},
	{{"nerbo", "nerbo", ProcComposeGN::MS, false, false, kMatSoft}, 2},
	{{"staffile", "staffile", ProcComposeGN::MS, false, false, kMatSoft}, 2},
	{{"scudiscio", "scudiscio", ProcComposeGN::MS, false, false, kMatSoft}, 2},
	{{"gatto a nove code", "gatto code", ProcComposeGN::MS, false, false, kMatSoft | kMatBone}, 2},
	{{"artiglio", "artiglio", ProcComposeGN::MS, false, false, kMatBone | kMatMetal}, 8},
	{{"rampino", "rampino", ProcComposeGN::MS, false, false, kMatMetal}, 8},
	{{"uncino", "uncino", ProcComposeGN::MS, false, false, kMatMetal | kMatBone}, 8},
	{{"raffio", "raffio", ProcComposeGN::MS, false, false, kMatMetal}, 8},
	{{"falcetto", "falcetto", ProcComposeGN::MS, false, false, kMatMetal | kMatBone}, 8},
	{{"artigli", "artigli", ProcComposeGN::MP, false, false, kMatBone | kMatMetal | kMatStone}, 8},
};
static constexpr ProcComposeWeaponNoun kComposeWeaponPierce[] = {
	{{"lancia", "lancia", ProcComposeGN::FS, false, false, kMatWood | kMatMetal | kMatBone | kMatStone}, 11},
	{{"picca", "picca", ProcComposeGN::FS, false, false, kMatWood | kMatMetal}, 11},
	{{"giavellotto", "giavellotto", ProcComposeGN::MS, false, false, kMatWood | kMatMetal | kMatBone}, 11},
	{{"tridente", "tridente", ProcComposeGN::MS, false, false, kMatMetal}, 11},
	{{"spiedo", "spiedo", ProcComposeGN::MS, false, false, kMatMetal | kMatWood}, 11},
	{{"partigiana", "partigiana", ProcComposeGN::FS, false, false, kMatMetal}, 11},
	{{"ronca", "ronca", ProcComposeGN::FS, false, false, kMatMetal}, 11},
	{{"arpione", "arpione", ProcComposeGN::MS, false, false, kMatMetal | kMatBone}, 11},
	{{"pilum", "pilum", ProcComposeGN::MS, false, false, kMatMetal}, 11},
	{{"zagaglia", "zagaglia", ProcComposeGN::FS, false, false, kMatWood | kMatBone | kMatStone}, 11},
	{{"pugnale", "pugnale", ProcComposeGN::MS, false, false, 0}, 1},
	{{"daga", "daga", ProcComposeGN::FS, false, false, kMatMetal | kMatBone}, 1},
	{{"stiletto", "stiletto", ProcComposeGN::MS, false, false, kMatMetal | kMatBone}, 1},
	{{"coltello", "coltello", ProcComposeGN::MS, false, false, kMatMetal | kMatBone}, 1},
	{{"punteruolo", "punteruolo", ProcComposeGN::MS, false, false, kMatMetal | kMatBone}, 1},
	{{"kris", "kris", ProcComposeGN::MS, false, false, kMatMetal}, 1},
	{{"misericordia", "misericordia", ProcComposeGN::FS, false, false, kMatMetal | kMatBone}, 1},
	{{"baselardo", "baselardo", ProcComposeGN::MS, false, false, kMatMetal | kMatBone}, 1},
	{{"stocco", "stocco", ProcComposeGN::MS, false, false, 0}, 10},
	{{"fioretto", "fioretto", ProcComposeGN::MS, false, false, 0}, 10},
	{{"spina", "spina", ProcComposeGN::FS, false, false, kMatBone}, 10},
	{{"rapiera", "rapiera", ProcComposeGN::FS, false, false, kMatMetal}, 10},
	{{"spillone", "spillone", ProcComposeGN::MS, false, false, 0}, 10},
	{{"pungolo", "pungolo", ProcComposeGN::MS, false, false, kMatWood | kMatMetal | kMatBone}, 10},
	{{"aculeo", "aculeo", ProcComposeGN::MS, false, false, kMatBone | kMatStone}, 10},
	{{"spadino", "spadino", ProcComposeGN::MS, false, false, kMatMetal}, 10},
};
static constexpr ProcComposeWeaponNoun kComposeWeaponBlunt[] = {
	{{"clava", "clava", ProcComposeGN::FS, false, false, kMatWood | kMatBone}, 7},
	{{"bastone ferrato", "bastone ferrato", ProcComposeGN::MS, false, true, 0}, 7},
	{{"manganello", "manganello", ProcComposeGN::MS, false, false, kMatWood | kMatMetal}, 7},
	{{"bastone", "bastone", ProcComposeGN::MS, false, false, 0}, 7},
	{{"randello", "randello", ProcComposeGN::MS, false, false, kMatWood}, 7},
	{{"mazza ferrata", "mazza ferrata", ProcComposeGN::FS, false, true, 0}, 7},
	{{"bastone nodoso", "bastone nodoso", ProcComposeGN::MS, false, true, 0}, 7},
	{{"maglio", "maglio", ProcComposeGN::MS, false, false, kMatMetal | kMatStone | kMatWood}, 6},
	{{"martello", "martello", ProcComposeGN::MS, false, false, kMatMetal | kMatStone}, 6},
	{{"mazzuolo", "mazzuolo", ProcComposeGN::MS, false, false, kMatWood | kMatMetal}, 6},
	{{"martellone", "martellone", ProcComposeGN::MS, false, false, kMatMetal | kMatStone}, 6},
	{{"mazzapicchio", "mazzapicchio", ProcComposeGN::MS, false, false, kMatWood | kMatMetal}, 6},
	{{"mazza", "mazza", ProcComposeGN::FS, false, false, 0}, 4},
	{{"mazzafrusto", "mazzafrusto", ProcComposeGN::MS, false, false, kMatMetal}, 4},
	{{"stella del mattino", "stella mattino", ProcComposeGN::FS, false, false, kMatMetal}, 4},
	{{"mazza d'arme", "mazza arme", ProcComposeGN::FS, false, false, kMatMetal}, 4},
	{{"bastone da guerra", "bastone guerra", ProcComposeGN::MS, false, false, kMatWood | kMatMetal}, 4},
	{{"martello da guerra", "martello guerra", ProcComposeGN::MS, false, false, kMatMetal}, 0},
	{{"scettro", "scettro", ProcComposeGN::MS, false, false, kMatMetal | kMatWood | kMatStone | kMatBone | kMatMagic}, 0},
	{{"pastorale", "pastorale", ProcComposeGN::MS, false, false, kMatMetal | kMatWood}, 0},
	{{"verga", "verga", ProcComposeGN::FS, false, false, kMatMetal | kMatWood | kMatStone | kMatBone | kMatMagic}, 0},
	{{"caduceo", "caduceo", ProcComposeGN::MS, false, false, kMatMetal | kMatWood | kMatMagic}, 0},
	{{"martello sacro", "martello sacro", ProcComposeGN::MS, false, false, kMatMetal | kMatStone | kMatMagic}, 0},
	{{"mazza chiodata", "mazza chiodata", ProcComposeGN::FS, false, true, 0}, 12},
	{{"maglio tonante", "maglio tonante", ProcComposeGN::MS, false, true, 0}, 12},
	{{"mazza tonante", "mazza tonante", ProcComposeGN::FS, false, true, 0}, 12},
	{{"martello del tuono", "martello tuono", ProcComposeGN::MS, false, true, 0}, 12},
	{{"ariete", "ariete", ProcComposeGN::MS, false, false, kMatWood | kMatMetal}, 12},
};

struct ProcComposeWeaponFamily {
	const ProcComposeWeaponNoun* items;
	std::size_t count;
	std::array<int, 5> types; /* tipi di danno della famiglia, terminati da -1 */
	unsigned default_mats;    /* classi ammesse per i nomi con mats == 0 */
};

template <std::size_t N>
constexpr ProcComposeWeaponFamily compose_weapon_family(const ProcComposeWeaponNoun (&items)[N],
														std::array<int, 5> types,
														unsigned default_mats) noexcept {
	return {items, N, types, default_mats};
}

static constexpr ProcComposeWeaponFamily kComposeWeaponFamilies[] = {
	compose_weapon_family(kComposeWeaponSlash, {3, 5, 2, 8, -1},
						  kMatMetal | kMatStone | kMatBone | kMatMagic), /* taglio */
	compose_weapon_family(kComposeWeaponPierce, {11, 1, 10, -1, -1},
						  kMatMetal | kMatBone | kMatStone), /* punta */
	compose_weapon_family(kComposeWeaponBlunt, {7, 6, 4, 0, 12},
						  kMatWood | kMatMetal | kMatBone), /* botta */
};
static_assert(std::size(kComposeWeaponFamilies) == 3, "families: slash, pierce, blunt");
static_assert(std::size(kComposeWeaponSlash) == 30 && std::size(kComposeWeaponPierce) == 26 &&
				  std::size(kComposeWeaponBlunt) == 28,
			  "weapon noun counts per family (update here when adding nouns)");

/* Ogni tipo della famiglia ha almeno un nome e ogni nome ha un tipo della famiglia:
 * cosi' la scelta in procarea_compose_weapon_name() non resta mai a mani vuote. */
[[nodiscard]] constexpr bool compose_weapon_family_ok(const ProcComposeWeaponFamily& fam) noexcept {
	std::size_t types = 0;
	while(types < fam.types.size() && fam.types[types] >= 0) {
		++types;
	}
	if(types == 0) {
		return false;
	}
	for(std::size_t t = 0; t < types; ++t) {
		bool found = false;
		for(std::size_t i = 0; i < fam.count; ++i) {
			found = found || fam.items[i].damage_type == fam.types[t];
		}
		if(!found) {
			return false;
		}
	}
	for(std::size_t i = 0; i < fam.count; ++i) {
		bool in_family = false;
		for(std::size_t t = 0; t < types; ++t) {
			in_family = in_family || fam.items[i].damage_type == fam.types[t];
		}
		if(!in_family) {
			return false;
		}
	}
	return true;
}
static_assert(compose_weapon_family_ok(kComposeWeaponFamilies[0]) &&
				  compose_weapon_family_ok(kComposeWeaponFamilies[1]) &&
				  compose_weapon_family_ok(kComposeWeaponFamilies[2]),
			  "every damage type needs a noun and every noun must belong to its family");

/* Corazza con resistenza IMM_BLUNT: nome fisso (con la k, voluto), nessun materiale. */
static constexpr ProcComposeNoun kComposeDragonArmor = {
	"corazza di skaglie di drago", "corazza skaglie drago", ProcComposeGN::FS, false, true, 0};

/* Base keyword = prima keyword dei nomi fissi attuali (procarea_reward_names.inc),
 * cosi' i comandi gia' in uso continuano a funzionare. Ordine = ProcRewardGearSlot. */
static constexpr ProcComposeNounList kComposeNounsBySlot[] = {
	compose_nouns(kComposeLight, "candela", kMatMetal | kMatStone),
	compose_nouns(kComposeFinger, "anello", kMatMetal | kMatStone | kMatBone),
	compose_nouns(kComposeNeck, "collare", kMatMetal | kMatStone | kMatBone | kMatMagic),
	compose_nouns(kComposeBody, "corazza", kMatSoft | kMatMetal),
	compose_nouns(kComposeHead, "elmo", kMatMetal | kMatBone),
	compose_nouns(kComposeLegs, "gambali", kMatSoft | kMatMetal),
	compose_nouns(kComposeFeet, "calzari", kMatSoft),
	compose_nouns(kComposeHands, "guanti", kMatSoft),
	compose_nouns(kComposeArms, "avambraccia", kMatMetal | kMatSoft | kMatBone),
	compose_nouns(kComposeAbout, "mantello", kMatSoft | kMatMagic),
	compose_nouns(kComposeWaist, "cintura", kMatSoft),
	compose_nouns(kComposeWrist, "bracciale", kMatMetal | kMatStone | kMatBone),
	/* Wield: nessuna lista qui, i nomi arma passano da procarea_compose_weapon_name(). */
	ProcComposeNounList{nullptr, 0, "arma", 0},
	compose_nouns(kComposeHold, "focus", kMatStone | kMatMagic),
	compose_nouns(kComposeEar, "orecchino", kMatMetal | kMatStone | kMatBone),
	compose_nouns(kComposeEyes, "lente", kMatMetal | kMatBone | kMatWood),
};
static_assert(std::size(kComposeNounsBySlot) == static_cast<std::size_t>(ProcRewardGearSlot::Count),
			  "kComposeNounsBySlot must follow ProcRewardGearSlot");

/* ---- Materiali per gruppo di fascia ----------------------------------- */

struct ProcComposeMaterial {
	std::array<const char*, static_cast<std::size_t>(ProcComposeGN::Count)> forms; /* MS FS MP FP */
	const char* kw;
	bool adjective;  /* false = complemento invariabile "di X"/"d'X" */
	unsigned mats;   /* classi di materiale a cui si applica (kMat*) */
	int prestige;    /* 0 umile, 1 normale, 2 pregiato */
};

static constexpr ProcComposeMaterial kComposeMatLow[] = { /* fasce 0-2 */
	{{{"grezzo", "grezza", "grezzi", "grezze"}}, "grezzo", true, kMatAny, 0},
	{{{"consunto", "consunta", "consunti", "consunte"}}, "consunto", true, kMatAny, 0},
	{{{"opaco", "opaca", "opachi", "opache"}}, "opaco", true, kMatAny, 0},
	{{{"scheggiato", "scheggiata", "scheggiati", "scheggiate"}}, "scheggiato", true, kMatStone | kMatWood | kMatBone | kMatMetal, 0},
	{{{"ammaccato", "ammaccata", "ammaccati", "ammaccate"}}, "ammaccato", true, kMatMetal | kMatWood, 0},
	{{{"usurato", "usurata", "usurati", "usurate"}}, "usurato", true, kMatAny, 0},
	{{{"rinforzato", "rinforzata", "rinforzati", "rinforzate"}}, "rinforzato", true, kMatSoft | kMatWood, 1},
	{{{"grigio", "grigia", "grigi", "grigie"}}, "grigio", true, kMatAny, 0},
	{{{"marrone", "marrone", "marroni", "marroni"}}, "marrone", true, kMatAny, 0},
	{{{"nero", "nera", "neri", "nere"}}, "nero", true, kMatAny, 1},
	{{{"d'osso", "d'osso", "d'osso", "d'osso"}}, "osso", false, kMatBone, 0},
	{{{"di bronzo", "di bronzo", "di bronzo", "di bronzo"}}, "bronzo", false, kMatMetal, 0},
	{{{"di cuoio", "di cuoio", "di cuoio", "di cuoio"}}, "cuoio", false, kMatSoft, 0},
	{{{"di ferro", "di ferro", "di ferro", "di ferro"}}, "ferro", false, kMatMetal, 1},
	{{{"di legno", "di legno", "di legno", "di legno"}}, "legno", false, kMatWood, 0},
	{{{"di quercia", "di quercia", "di quercia", "di quercia"}}, "quercia", false, kMatWood, 1},
	{{{"di frassino", "di frassino", "di frassino", "di frassino"}}, "frassino", false, kMatWood, 1},
	{{{"di rame", "di rame", "di rame", "di rame"}}, "rame", false, kMatMetal, 0},
	{{{"di stagno", "di stagno", "di stagno", "di stagno"}}, "stagno", false, kMatMetal, 0},
	{{{"di roccia", "di roccia", "di roccia", "di roccia"}}, "roccia", false, kMatStone, 0},
	{{{"d'argilla", "d'argilla", "d'argilla", "d'argilla"}}, "argilla", false, kMatStone, 0},
	{{{"di conchiglie", "di conchiglie", "di conchiglie", "di conchiglie"}}, "conchiglie", false, kMatBone, 0},
	{{{"di corteccia", "di corteccia", "di corteccia", "di corteccia"}}, "corteccia", false, kMatWood, 0},
	{{{"di pelle di lupo", "di pelle di lupo", "di pelle di lupo", "di pelle di lupo"}}, "lupo", false, kMatSoft, 1},
	{{{"di lana", "di lana", "di lana", "di lana"}}, "lana", false, kMatSoft, 0},
	{{{"di tela", "di tela", "di tela", "di tela"}}, "tela", false, kMatSoft, 0},
	{{{"di pelle", "di pelle", "di pelle", "di pelle"}}, "pelle", false, kMatSoft, 0},
};
static constexpr ProcComposeMaterial kComposeMatMid[] = { /* fasce 3-5 */
	{{{"brunito", "brunita", "bruniti", "brunite"}}, "brunito", true, kMatMetal, 1},
	{{{"inciso", "incisa", "incisi", "incise"}}, "inciso", true, kMatAny, 1},
	{{{"cesellato", "cesellata", "cesellati", "cesellate"}}, "cesellato", true, kMatMetal | kMatStone | kMatWood, 2},
	{{{"intarsiato", "intarsiata", "intarsiati", "intarsiate"}}, "intarsiato", true, kMatWood | kMatMetal | kMatBone | kMatStone, 2},
	{{{"decorato", "decorata", "decorati", "decorate"}}, "decorato", true, kMatAny, 1},
	{{{"dorato", "dorata", "dorati", "dorate"}}, "dorato", true, kMatAny, 2},
	{{{"argentato", "argentata", "argentati", "argentate"}}, "argentato", true, kMatAny, 2},
	{{{"benedetto", "benedetta", "benedetti", "benedette"}}, "benedetto", true, kMatAny, 2},
	{{{"maledetto", "maledetta", "maledetti", "maledette"}}, "maledetto", true, kMatAny, 1},
	{{{"rosso", "rossa", "rossi", "rosse"}}, "rosso", true, kMatAny, 1},
	{{{"verde", "verde", "verdi", "verdi"}}, "verde", true, kMatAny, 1},
	{{{"bianco", "bianca", "bianci", "biance"}}, "bianco", true, kMatAny, 1},
	{{{"blu", "blu", "blu", "blu"}}, "blu", true, kMatAny, 1},
	{{{"arancione", "arancione", "arancioni", "arancioni"}}, "arancione", true, kMatAny, 1},
	{{{"d'argento", "d'argento", "d'argento", "d'argento"}}, "argento", false, kMatMetal, 1},
	{{{"d'acciaio", "d'acciaio", "d'acciaio", "d'acciaio"}}, "acciaio", false, kMatMetal, 1},
	{{{"d'ambra", "d'ambra", "d'ambra", "d'ambra"}}, "ambra", false, kMatStone, 1},
	{{{"di chitina", "di chitina", "di chitina", "di chitina"}}, "chitina", false, kMatBone, 1},
	{{{"d'ebano", "d'ebano", "d'ebano", "d'ebano"}}, "ebano", false, kMatWood, 2},
	{{{"di granito", "di granito", "di granito", "di granito"}}, "granito", false, kMatStone, 1},
	{{{"di marmo", "di marmo", "di marmo", "di marmo"}}, "marmo", false, kMatStone, 1},
	{{{"di quarzo", "di quarzo", "di quarzo", "di quarzo"}}, "quarzo", false, kMatStone, 2},
	{{{"di basalto", "di basalto", "di basalto", "di basalto"}}, "basalto", false, kMatStone, 1},
	{{{"d'avorio", "d'avorio", "d'avorio", "d'avorio"}}, "avorio", false, kMatBone, 2},
	{{{"di pelle di serpente", "di pelle di serpente", "di pelle di serpente", "di pelle di serpente"}}, "serpente", false, kMatSoft, 1},
	{{{"di vetro scuro", "di vetro scuro", "di vetro scuro", "di vetro scuro"}}, "vetro", false, kMatStone, 1},
	{{{"di lino", "di lino", "di lino", "di lino"}}, "lino", false, kMatSoft, 1},
	{{{"di seta", "di seta", "di seta", "di seta"}}, "seta", false, kMatSoft, 2},
};
static constexpr ProcComposeMaterial kComposeMatHigh[] = { /* fasce 6-8 */
	{{{"runico", "runica", "runici", "runiche"}}, "runico", true, kMatAny, 1},
	{{{"lucente", "lucente", "lucenti", "lucenti"}}, "lucente", true, kMatMetal | kMatStone | kMatMagic, 1},
	{{{"scintillante", "scintillante", "scintillanti", "scintillanti"}}, "scintillante", true, kMatMetal | kMatStone | kMatMagic, 2},
	{{{"luminoso", "luminosa", "luminosi", "luminose"}}, "luminoso", true, kMatAny, 2},
	{{{"trasparente", "trasparente", "trasparenti", "trasparenti"}}, "trasparente", true, kMatStone | kMatMagic, 2},
	{{{"antico", "antica", "antichi", "antiche"}}, "antico", true, kMatAny, 2},
	{{{"sacro", "sacra", "sacri", "sacre"}}, "sacro", true, kMatAny, 2},
	{{{"magico", "magica", "magici", "magiche"}}, "magico", true, kMatAny, 2},
	{{{"insanguinato", "insanguinata", "insanguinati", "insanguinate"}}, "insanguinato", true, kMatAny, 1},
	{{{"infuocato", "infuocata", "infuocati", "infuocate"}}, "infuocato", true, kMatAny, 1},
	{{{"viola", "viola", "viola", "viola"}}, "viola", true, kMatAny, 1},
	{{{"celeste", "celeste", "celesti", "celesti"}}, "celeste", true, kMatAny, 2},
	{{{"amaranto", "amaranto", "amaranto", "amaranto"}}, "amaranto", true, kMatAny, 2},
	{{{"rosso fuoco", "rosso fuoco", "rosso fuoco", "rosso fuoco"}}, "rosso fuoco", true, kMatAny, 2},
	{{{"nero", "nera", "neri", "nere"}}, "nero", true, kMatAny, 1},
	{{{"di mithril", "di mithril", "di mithril", "di mithril"}}, "mithril", false, kMatMetal, 1},
	{{{"d'ossidiana", "d'ossidiana", "d'ossidiana", "d'ossidiana"}}, "ossidiana", false, kMatStone, 1},
	{{{"di DarkStar", "di DarkStar", "di DarkStar", "di DarkStar"}}, "darkstar", false, kMatMetal | kMatStone, 2},
	{{{"d'oro bianco", "d'oro bianco", "d'oro bianco", "d'oro bianco"}}, "oro", false, kMatMetal, 2},
	{{{"di platino", "di platino", "di platino", "di platino"}}, "platino", false, kMatMetal, 2},
	{{{"d'adamantio", "d'adamantio", "d'adamantio", "d'adamantio"}}, "adamantio", false, kMatMetal, 2},
	{{{"di diamante", "di diamante", "di diamante", "di diamante"}}, "diamante", false, kMatStone, 2},
	{{{"di cristallo", "di cristallo", "di cristallo", "di cristallo"}}, "cristallo", false, kMatStone, 2},
	{{{"di pelle di drago", "di pelle di drago", "di pelle di drago", "di pelle di drago"}}, "drago", false, kMatSoft, 2},
	{{{"di ghiaccio", "di ghiaccio", "di ghiaccio", "di ghiaccio"}}, "ghiaccio", false, kMatStone | kMatMagic, 1},
	{{{"di sangue condensato", "di sangue condensato", "di sangue condensato", "di sangue condensato"}}, "sangue", false, kMatBone | kMatMagic, 1},
	{{{"di gromril", "di gromril", "di gromril", "di gromril"}}, "gromril", false, kMatMetal, 2},
	{{{"d'ebano", "d'ebano", "d'ebano", "d'ebano"}}, "ebano", false, kMatWood, 2},
	{{{"di quercia nera", "di quercia nera", "di quercia nera", "di quercia nera"}}, "quercia", false, kMatWood, 1},
};
static constexpr ProcComposeMaterial kComposeMatLegend[] = { /* fascia 9 */
	{{{"eterno", "eterna", "eterni", "eterne"}}, "eterno", true, kMatAny, 2},
	{{{"stellato", "stellata", "stellati", "stellate"}}, "stellato", true, kMatAny, 2},
	{{{"leggendario", "leggendaria", "leggendarii", "leggendarie"}}, "leggendario", true, kMatAny, 2},
	{{{"antichissimo", "antichissima", "antichissimi", "antichissime"}}, "antichissimo", true, kMatAny, 2},
	{{{"ammantato d'ombra", "ammantata d'ombra", "ammantati d'ombra", "ammantate d'ombra"}}, "ammantato", true, kMatAny, 2},
	{{{"d'eclisse", "d'eclisse", "d'eclisse", "d'eclisse"}}, "eclisse", false, kMatMagic, 2},
	{{{"di stella", "di stella", "di stella", "di stella"}}, "stella", false, kMatMagic, 2},
	{{{"di luna", "di luna", "di luna", "di luna"}}, "luna", false, kMatMagic, 1},
	{{{"d'ombra", "d'ombra", "d'ombra", "d'ombra"}}, "ombra", false, kMatMagic, 1},
	{{{"di luce", "di luce", "di luce", "di luce"}}, "luce", false, kMatMagic, 2},
	{{{"di mithril", "di mithril", "di mithril", "di mithril"}}, "mithril", false, kMatMetal, 2},
	{{{"d'adamantio", "d'adamantio", "d'adamantio", "d'adamantio"}}, "adamantio", false, kMatMetal, 2},
	{{{"di pelle di drago", "di pelle di drago", "di pelle di drago", "di pelle di drago"}}, "drago", false, kMatSoft, 2},
	{{{"di cristallo", "di cristallo", "di cristallo", "di cristallo"}}, "cristallo", false, kMatStone, 2},
	{{{"d'osso di drago", "d'osso di drago", "d'osso di drago", "d'osso di drago"}}, "drago", false, kMatBone, 2},
	{{{"d'ebano", "d'ebano", "d'ebano", "d'ebano"}}, "ebano", false, kMatWood, 2},
};

struct ProcComposeMaterialList {
	const ProcComposeMaterial* items;
	std::size_t count;
};

template <std::size_t N>
constexpr ProcComposeMaterialList compose_mats(const ProcComposeMaterial (&items)[N]) noexcept {
	return {items, N};
}

static constexpr ProcComposeMaterialList kComposeMaterialGroups[] = {
	compose_mats(kComposeMatLow),
	compose_mats(kComposeMatMid),
	compose_mats(kComposeMatHigh),
	compose_mats(kComposeMatLegend),
};
static_assert(std::size(kComposeMaterialGroups) == 4, "4 material groups: 0-2, 3-5, 6-8, 9");
static_assert(PROCAREA_TEMPLATE_BANDS == 10, "material band groups assume 10 bands");

/* Capienza dell'array di lavoro di compose_pick_material(). */
inline constexpr std::size_t kComposeMaxMaterials = 48;
static_assert(std::size(kComposeMatLow) <= kComposeMaxMaterials &&
				  std::size(kComposeMatMid) <= kComposeMaxMaterials &&
				  std::size(kComposeMatHigh) <= kComposeMaxMaterials &&
				  std::size(kComposeMatLegend) <= kComposeMaxMaterials,
			  "raise kComposeMaxMaterials");

[[nodiscard]] static constexpr int procarea_compose_material_group(int band) noexcept {
	if(band <= 2) {
		return 0;
	}
	if(band <= 5) {
		return 1;
	}
	if(band <= 8) {
		return 2;
	}
	return 3;
}

/* ---- Epiteti (complementi invariabili: nessuna concordanza) ----------- */

struct ProcComposeEpithet {
	const char* text;
	const char* kw;
};

struct ProcComposeEpithetList {
	const ProcComposeEpithet* items;
	std::size_t count;
};

template <std::size_t N>
constexpr ProcComposeEpithetList compose_epis(const ProcComposeEpithet (&items)[N]) noexcept {
	return {items, N};
}

/* Famiglie generiche */
static constexpr ProcComposeEpithet kEpiBestia[] = {
	{"della tana", "tana"}, {"del predatore", "predatore"}, {"dell'orda", "orda"},
	{"dell'artiglio", "artiglio"}};
static constexpr ProcComposeEpithet kEpiCripta[] = {
	{"della cripta", "cripta"}, {"dei re morti", "re"}, {"della stirpe antica", "stirpe"}};
static constexpr ProcComposeEpithet kEpiNebbia[] = {
	{"della nebbia", "nebbia"}, {"della bruma", "bruma"}, {"del velo", "velo"}};
static constexpr ProcComposeEpithet kEpiAcqua[] = {
	{"della marea", "marea"}, {"delle acque nere", "acque"}, {"della luna", "luna"},
	{"del lago nero", "lago"}};
static constexpr ProcComposeEpithet kEpiForgia[] = {
	{"della forgia", "forgia"}, {"dell'incudine", "incudine"}, {"dei minatori", "minatori"},
	{"del martello", "martello"}};
static constexpr ProcComposeEpithet kEpiGelo[] = {
	{"del gelo", "gelo"}, {"del sale", "sale"}, {"della brina", "brina"},
	{"del ghiaccio", "ghiaccio"}};
static constexpr ProcComposeEpithet kEpiRagni[] = {
	{"del ragno cieco", "ragno"}, {"della tessitrice", "tessitrice"}, {"dei bozzoli", "bozzoli"},
	{"della ragnatela", "ragnatela"}};
static constexpr ProcComposeEpithet kEpiFuoco[] = {
	{"delle ceneri", "ceneri"}, {"del drago", "drago"}, {"della brace", "brace"},
	{"della pira", "pira"}, {"della fiamma eterna", "fiamma"}};
static constexpr ProcComposeEpithet kEpiCristallo[] = {
	{"del prisma", "prisma"}, {"del varco", "varco"}, {"della luce rifratta", "luce"}};
static constexpr ProcComposeEpithet kEpiFunghi[] = {
	{"delle spore", "spore"}, {"del micelio", "micelio"}, {"della muffa", "muffa"}};
static constexpr ProcComposeEpithet kEpiSacro[] = {
	{"del culto", "culto"}, {"dell'altare", "altare"}, {"della luce antica", "luce"},
	{"dell'alba", "alba"}};
static constexpr ProcComposeEpithet kEpiAbisso[] = {
	{"del baratro", "baratro"}, {"del tuono", "tuono"}, {"della tempesta", "tempesta"}};
static constexpr ProcComposeEpithet kEpiOssa[] = {
	{"dell'ossuario", "ossuario"}, {"delle ossa", "osso"}, {"del sangue versato", "sangue"}};
static constexpr ProcComposeEpithet kEpiNatura[] = {
	{"del giardino", "giardino"}, {"del rovo nero", "rovo"}, {"della foresta", "foresta"},
	{"delle spine", "spine"}};
static constexpr ProcComposeEpithet kEpiRovina[] = {
	{"della torre", "torre"}, {"delle mura", "mura"}, {"del custode", "custode"}};
static constexpr ProcComposeEpithet kEpiSapere[] = {
	{"della biblioteca", "biblioteca"}, {"del tomo", "tomo"}, {"dell'indice", "indice"}};
static constexpr ProcComposeEpithet kEpiArena[] = {
	{"dell'arena", "arena"}, {"del campione", "campione"}, {"del custode", "custode"}};
static constexpr ProcComposeEpithet kEpiPiume[] = {
	{"del corvo re", "corvo"}, {"delle piume nere", "piume"}, {"del volo notturno", "volo"}};
static constexpr ProcComposeEpithet kEpiElfi[] = {
	{"degli elfi", "elfi"}, {"delle stelle", "stelle"}, {"dell'alba", "alba"}};
static constexpr ProcComposeEpithet kEpiOmbra[] = {
	{"dell'ombra", "ombra"}, {"dell'occhio", "occhio"}, {"della notte", "notte"},
	{"del crepuscolo", "crepuscolo"}};
static constexpr ProcComposeEpithet kEpiEffimera[] = {
	{"della dimensione", "dimensione"}, {"del custode", "custode"}, {"dell'alba", "alba"}};

/* Famiglie con luoghi nominati (temi 25-49, omaggio Tolkien / Forgotten Realms) */
static constexpr ProcComposeEpithet kEpiMoria[] = {
	{"di Khazad-dum", "khazad"}, {"di Moria", "moria"}, {"delle Miniere", "miniere"}};
static constexpr ProcComposeEpithet kEpiFangorn[] = {
	{"di Fangorn", "fangorn"}, {"degli Ent", "ent"}};
static constexpr ProcComposeEpithet kEpiDunedain[] = {
	{"dei Dunedain", "dunedain"}, {"di Numenor", "numenor"}, {"di Arnor", "arnor"}};
static constexpr ProcComposeEpithet kEpiPonte[] = {
	{"del Balrog", "balrog"}, {"di Durin", "durin"}};
static constexpr ProcComposeEpithet kEpiGondor[] = {
	{"di Gondor", "gondor"}, {"di Minas Tirith", "tirith"}, {"di Osgiliath", "osgiliath"},
	{"di Pelargir", "pelargir"}};
static constexpr ProcComposeEpithet kEpiUruk[] = {
	{"degli Uruk-hai", "uruk"}, {"della Mano Bianca", "mano"}};
static constexpr ProcComposeEpithet kEpiLorien[] = {
	{"di Lorien", "lorien"}, {"di Imladris", "imladris"}, {"di Galadriel", "galadriel"}};
static constexpr ProcComposeEpithet kEpiMordor[] = {
	{"di Mordor", "mordor"}, {"di Gorgoroth", "gorgoroth"}, {"di Barad-dur", "baraddur"}};
static constexpr ProcComposeEpithet kEpiRohan[] = {
	{"di Rohan", "rohan"}, {"di Helm", "helm"}, {"della Marca", "marca"}};
static constexpr ProcComposeEpithet kEpiDunclivo[] = {
	{"di Dunharrow", "dunharrow"}, {"dei Morti", "morti"}};
static constexpr ProcComposeEpithet kEpiIsengard[] = {
	{"di Isengard", "isengard"}, {"di Orthanc", "orthanc"}, {"di Saruman", "saruman"}};
static constexpr ProcComposeEpithet kEpiUngol[] = {
	{"di Cirith Ungol", "ungol"}, {"di Shelob", "shelob"}};
static constexpr ProcComposeEpithet kEpiBoscoAtro[] = {
	{"di Bosco Atro", "atro"}, {"di Thranduil", "thranduil"}};
static constexpr ProcComposeEpithet kEpiContea[] = {
	{"della Contea", "contea"}, {"di Brea", "brea"}, {"del Puledro", "puledro"},
	{"di Hobbiville", "hobbiville"}};
static constexpr ProcComposeEpithet kEpiSmaug[] = {
	{"di Smaug", "smaug"}, {"della Montagna", "montagna"}};
static constexpr ProcComposeEpithet kEpiErebor[] = {
	{"di Erebor", "erebor"}, {"di Thorin", "thorin"}, {"del re nano", "nano"}};
static constexpr ProcComposeEpithet kEpiUnderdark[] = {
	{"di Menzoberranzan", "menzoberranzan"}, {"del Sottosuolo", "sottosuolo"}};
static constexpr ProcComposeEpithet kEpiWaterdeep[] = {
	{"di Waterdeep", "waterdeep"}, {"di Undermountain", "undermountain"},
	{"di Skullport", "skullport"}};
static constexpr ProcComposeEpithet kEpiNeverwinter[] = {
	{"di Neverwinter", "neverwinter"}, {"della Costa", "costa"}, {"di Luskan", "luskan"}};
static constexpr ProcComposeEpithet kEpiBaldur[] = {
	{"di Baldur's Gate", "baldur"}, {"di Bhaal", "bhaal"}};
static constexpr ProcComposeEpithet kEpiPlanare[] = {
	{"di Sigil", "sigil"}, {"dell'Astrale", "astrale"}};
static constexpr ProcComposeEpithet kEpiRavenloft[] = {
	{"di Ravenloft", "ravenloft"}, {"di Barovia", "barovia"}};
static constexpr ProcComposeEpithet kEpiTiamat[] = {
	{"di Tiamat", "tiamat"}, {"del Calimshan", "calimshan"}};
static constexpr ProcComposeEpithet kEpiBeholder[] = {
	{"del beholder", "beholder"}, {"del tiranno", "tiranno"}};
static constexpr ProcComposeEpithet kEpiAvernus[] = {
	{"di Avernus", "avernus"}, {"dei Nove Inferi", "inferi"}};

/* Indice = theme_id (ordine di kThemeSets, procarea_themes.inc).
 * [0] famiglia generica, [1] famiglia nominata (o nullptr): se presente,
 * scelta al 50%. */
struct ProcComposeThemeEpithets {
	ProcComposeEpithetList generic;
	ProcComposeEpithetList named;
};

static constexpr ProcComposeEpithetList kNoEpis{nullptr, 0};

static constexpr ProcComposeThemeEpithets kComposeThemeEpithets[] = {
	{compose_epis(kEpiBestia), kNoEpis},      /* 0 Antro Selvaggio */
	{compose_epis(kEpiCripta), kNoEpis},      /* 1 Cripta Sepolta */
	{compose_epis(kEpiNebbia), kNoEpis},      /* 2 Labirinto di Nebbia */
	{compose_epis(kEpiAcqua), kNoEpis},       /* 3 Rovine Sommerse */
	{compose_epis(kEpiForgia), kNoEpis},      /* 4 Forgia Abbandonata */
	{compose_epis(kEpiGelo), kNoEpis},        /* 5 Galleria di Ghiaccio */
	{compose_epis(kEpiRagni), kNoEpis},       /* 6 Tana dei Ragni */
	{compose_epis(kEpiFuoco), kNoEpis},       /* 7 Catacombe Bruciate */
	{compose_epis(kEpiCristallo), kNoEpis},   /* 8 Labirinto di Cristallo */
	{compose_epis(kEpiFunghi), kNoEpis},      /* 9 Antro Fungino */
	{compose_epis(kEpiSacro), kNoEpis},       /* 10 Tempio Rovinato */
	{compose_epis(kEpiAbisso), kNoEpis},      /* 11 Voragine Profonda */
	{compose_epis(kEpiOssa), kNoEpis},        /* 12 Ossuario Antico */
	{compose_epis(kEpiSacro), kNoEpis},       /* 13 Santuario Profanato */
	{compose_epis(kEpiNatura), kNoEpis},      /* 14 Giardino Pietrificato */
	{compose_epis(kEpiForgia), kNoEpis},      /* 15 Miniera Abbandonata */
	{compose_epis(kEpiAcqua), kNoEpis},       /* 16 Palude Nera */
	{compose_epis(kEpiRovina), kNoEpis},      /* 17 Torre Spezzata */
	{compose_epis(kEpiAbisso), kNoEpis},      /* 18 Canyon del Tuono */
	{compose_epis(kEpiPiume), kNoEpis},       /* 19 Nido del Corvo Re */
	{compose_epis(kEpiSapere), kNoEpis},      /* 20 Biblioteca Sepolta */
	{compose_epis(kEpiArena), kNoEpis},       /* 21 Arena Dimenticata */
	{compose_epis(kEpiNatura), kNoEpis},      /* 22 Labirinto di Spine */
	{compose_epis(kEpiAcqua), kNoEpis},       /* 23 Grotta Lunare */
	{compose_epis(kEpiGelo), kNoEpis},        /* 24 Caverna di Sale */
	{compose_epis(kEpiOmbra), compose_epis(kEpiMoria)},        /* 25 Moria delle Ombre */
	{compose_epis(kEpiNatura), compose_epis(kEpiFangorn)},     /* 26 Fangorn Profondo */
	{compose_epis(kEpiCripta), compose_epis(kEpiDunedain)},    /* 27 Tomba dei Re Caduti */
	{compose_epis(kEpiAbisso), compose_epis(kEpiPonte)},       /* 28 Ponte del Baratro */
	{compose_epis(kEpiRovina), compose_epis(kEpiGondor)},      /* 29 Rovine di Osgiliath */
	{compose_epis(kEpiBestia), compose_epis(kEpiUruk)},        /* 30 Covo degli Uruk */
	{compose_epis(kEpiElfi), compose_epis(kEpiLorien)},        /* 31 Galleria Elfica */
	{compose_epis(kEpiFuoco), compose_epis(kEpiMordor)},       /* 32 Ceneri di Gorgoroth */
	{compose_epis(kEpiRovina), compose_epis(kEpiRohan)},       /* 33 Fortezza di Helm */
	{compose_epis(kEpiOssa), compose_epis(kEpiDunclivo)},      /* 34 Sentieri dei Morti */
	{compose_epis(kEpiForgia), compose_epis(kEpiIsengard)},    /* 35 Fosse di Isengard */
	{compose_epis(kEpiRagni), compose_epis(kEpiUngol)},        /* 36 Tunnels di Ungol */
	{compose_epis(kEpiNebbia), compose_epis(kEpiBoscoAtro)},   /* 37 Bosco di Mirkwood */
	{compose_epis(kEpiRovina), compose_epis(kEpiContea)},      /* 38 Cantine di Bree */
	{compose_epis(kEpiFuoco), compose_epis(kEpiSmaug)},        /* 39 Tesoro di Smaug */
	{compose_epis(kEpiForgia), compose_epis(kEpiErebor)},      /* 40 Sale di Erebor */
	{compose_epis(kEpiOmbra), compose_epis(kEpiUnderdark)},    /* 41 Rocca Nera Profonda */
	{compose_epis(kEpiAcqua), compose_epis(kEpiWaterdeep)},    /* 42 Fogne di Waterdeep */
	{compose_epis(kEpiRovina), compose_epis(kEpiNeverwinter)}, /* 43 Rovine di Neverwinter */
	{compose_epis(kEpiCripta), compose_epis(kEpiBaldur)},      /* 44 Cripta di Baldur */
	{compose_epis(kEpiCristallo), compose_epis(kEpiPlanare)},  /* 45 Fessura Planare */
	{compose_epis(kEpiOmbra), compose_epis(kEpiRavenloft)},    /* 46 Castello delle Ombre */
	{compose_epis(kEpiFuoco), compose_epis(kEpiTiamat)},       /* 47 Antro del Drago */
	{compose_epis(kEpiOmbra), compose_epis(kEpiBeholder)},     /* 48 Caverna degli Occhi */
	{compose_epis(kEpiFuoco), compose_epis(kEpiAvernus)},      /* 49 Fosso Infernale */
};
static_assert(std::size(kComposeThemeEpithets) == std::size(kThemeSets),
			  "kComposeThemeEpithets must have one row per theme in kThemeSets");

/* ---- Composizione ------------------------------------------------------ */

inline constexpr std::size_t kComposeMaxShort = 50;
inline constexpr std::size_t kComposeMaxKeywords = 60;
inline constexpr std::size_t kComposeMaxLong = 120;
inline constexpr int kComposeNoEpithetPct = 40;     /* prestigio normale */
inline constexpr int kComposeNoEpithetLowPct = 60;  /* prestigio umile */
inline constexpr int kComposeNoEpithetHighPct = 15; /* prestigio pregiato */
inline constexpr int kComposeNamedPct = 50;         /* epiteto nominato (temi 25-49) */
inline constexpr int kComposeNamedHighPct = 75;     /* idem, prestigio pregiato */
inline constexpr int kComposeLegendAdjPct = 50;

struct ProcComposedName {
	std::string keywords;
	std::string short_descr;
	std::string long_descr;
};

/* Riassunto del tiro reale dell'oggetto, costruito in procarea_spawn.cpp DOPO il tiro
 * dei bonus (procarea_build_reward_quality). La fascia resta la base del nome; il
 * prestigio sposta materiale/aggettivo/epiteto dentro il pool compatibile. */
struct ProcComposeQuality {
	int band = 0;             /* fascia premio (gia' con spostamenti) */
	int prestige = 1;         /* 0 umile, 1 normale, 2 pregiato (ricavato dal tiro) */
	bool blunt_resist = false; /* APPLY_IMMUNE con IMM_BLUNT: solo corazza, vedi regola */
	int bonus_count = 0;      /* solo log: bonus aggiunti dal tiro (escluso l'immune) */
	int magnitude_pct = 50;   /* solo log: entita' media dei bonus nel loro range, 0..100 */
};

template <typename T>
[[nodiscard]] static const T& compose_pick(const T* items, std::size_t count) {
	return items[static_cast<std::size_t>(number(0, static_cast<int>(count) - 1))];
}

/* ---- Articoli generati dal codice ------------------------------------- */

enum class ProcComposeArt { Definite, Indefinite, Partitive };

[[nodiscard]] static bool compose_is_vowel(char c) noexcept {
	switch(c) {
	case 'a': case 'e': case 'i': case 'o': case 'u':
	case 'A': case 'E': case 'I': case 'O': case 'U':
		return true;
	default:
		return false;
	}
}

/* Iniziale che vuole lo/uno/gli/dello: s+consonante, z, x, gn, ps, pn. */
[[nodiscard]] static bool compose_wants_lo(std::string_view w) noexcept {
	if(w.empty()) {
		return false;
	}
	const auto lower = [](char c) {
		return static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
	};
	const char a = lower(w[0]);
	const char b = w.size() > 1 ? lower(w[1]) : '\0';
	if(a == 'z' || a == 'x') {
		return true;
	}
	if(a == 's') {
		return b != '\0' && !compose_is_vowel(b);
	}
	return (a == 'g' && b == 'n') || (a == 'p' && (b == 's' || b == 'n'));
}

/* Articolo + nome ("l'anello", "lo stiletto", "una corazza", "delle scarpe").
 * L'articolo dipende dalla PRIMA parola del nome (anche nei composti). Singolare
 * maschile: il/lo/l'; femminile: la/l'; plurali: i/gli, le. Indeterminativo: un/uno,
 * una/un'; plurale = partitivo dei/degli/delle. Partitive = preposizione articolata
 * (del/dello/dell', della/dell', dei/degli, delle). */
[[nodiscard]] static std::string compose_with_article(ProcComposeArt art, ProcComposeGN gn,
													  std::string_view noun) {
	const bool vowel = !noun.empty() && compose_is_vowel(noun[0]);
	const bool lo = compose_wants_lo(noun);
	const bool fem = gn == ProcComposeGN::FS || gn == ProcComposeGN::FP;
	const bool plural = gn == ProcComposeGN::MP || gn == ProcComposeGN::FP;
	std::string_view article;
	bool elide = false;
	switch(art) {
	case ProcComposeArt::Definite:
		if(plural) {
			article = fem ? "le" : (vowel || lo ? "gli" : "i");
		} else if(fem) {
			article = vowel ? "l'" : "la";
			elide = vowel;
		} else if(vowel) {
			article = "l'";
			elide = true;
		} else {
			article = lo ? "lo" : "il";
		}
		break;
	case ProcComposeArt::Indefinite:
		if(plural) {
			article = fem ? "delle" : (vowel || lo ? "degli" : "dei");
		} else if(fem) {
			article = vowel ? "un'" : "una";
			elide = vowel;
		} else {
			article = lo ? "uno" : "un";
		}
		break;
	case ProcComposeArt::Partitive:
		if(plural) {
			article = fem ? "delle" : (vowel || lo ? "degli" : "dei");
		} else if(fem) {
			article = vowel ? "dell'" : "della";
			elide = vowel;
		} else if(vowel) {
			article = "dell'";
			elide = true;
		} else {
			article = lo ? "dello" : "del";
		}
		break;
	}
	std::string out(article);
	if(!elide) {
		out += ' ';
	}
	out += noun;
	return out;
}

[[nodiscard]] static const char* compose_side(ProcComposeGN gn, bool left) noexcept {
	const bool fem = gn == ProcComposeGN::FS;
	if(left) {
		return fem ? "sinistra" : "sinistro";
	}
	return fem ? "destra" : "destro";
}

[[nodiscard]] static bool compose_is_plural(ProcComposeGN gn) noexcept {
	return gn == ProcComposeGN::MP || gn == ProcComposeGN::FP;
}

[[nodiscard]] static const char* compose_avvolto(ProcComposeGN gn) noexcept {
	switch(gn) {
	case ProcComposeGN::FS:
		return "avvolta";
	case ProcComposeGN::MP:
		return "avvolti";
	case ProcComposeGN::FP:
		return "avvolte";
	default:
		return "avvolto";
	}
}

/* Aggiunge le parole di `kw` (anche piu' d'una: "anello ferro") una alla volta, senza
 * doppioni a parola intera e senza superare kComposeMaxKeywords. */
static void compose_add_kw(std::string& kws, std::string_view kw) {
	std::size_t start = 0;
	while(start < kw.size()) {
		const std::size_t end = std::min(kw.find(' ', start), kw.size());
		const std::string_view word = kw.substr(start, end - start);
		start = end + 1;
		if(word.empty()) {
			continue;
		}
		bool dup = false;
		std::size_t pos = 0;
		while(pos < kws.size() && !dup) {
			const std::size_t stop = std::min(kws.find(' ', pos), kws.size());
			dup = std::string_view(kws).substr(pos, stop - pos) == word;
			pos = stop + 1;
		}
		if(dup || kws.size() + word.size() + 1 > kComposeMaxKeywords) {
			continue;
		}
		if(!kws.empty()) {
			kws += ' ';
		}
		kws += word;
	}
}

/*
 * Sceglie il materiale tra quelli del gruppo compatibili con la maschera `allowed`
 * (e, se adjective_only, solo aggettivi). Tra i compatibili tiene solo quelli col
 * prestigio piu' vicino a `prestige`, poi ne estrae uno a caso. nullptr = nessun
 * materiale compatibile (il nome resta senza materiale).
 */
[[nodiscard]] static const ProcComposeMaterial*
compose_pick_material(const ProcComposeMaterialList& list, unsigned allowed, int prestige,
					  bool adjective_only) {
	std::array<const ProcComposeMaterial*, kComposeMaxMaterials> pool{};
	std::size_t n = 0;
	int best = std::numeric_limits<int>::max();
	for(std::size_t i = 0; i < list.count && i < pool.size(); ++i) {
		const ProcComposeMaterial& m = list.items[i];
		if((m.mats & allowed) == 0 || (adjective_only && !m.adjective)) {
			continue;
		}
		const int dist = std::abs(m.prestige - prestige);
		if(dist < best) {
			best = dist;
			n = 0;
		}
		if(dist == best) {
			pool[n++] = &m;
		}
	}
	if(n == 0) {
		return nullptr;
	}
	return pool[static_cast<std::size_t>(number(0, static_cast<int>(n) - 1))];
}

/* Epiteto del tema: piu' raro se il pezzo e' umile, piu' frequente (e piu' spesso
 * nominato) se e' pregiato. force = sempre un epiteto (aggettivo leggendario). */
[[nodiscard]] static const ProcComposeEpithet* compose_pick_epithet(int theme_id, int prestige,
																	bool force) {
	int none_pct = kComposeNoEpithetPct;
	int named_pct = kComposeNamedPct;
	if(prestige <= 0) {
		none_pct = kComposeNoEpithetLowPct;
	} else if(prestige >= 2) {
		none_pct = kComposeNoEpithetHighPct;
		named_pct = kComposeNamedHighPct;
	}
	if(!force && number(1, 100) <= none_pct) {
		return nullptr;
	}
	ProcComposeEpithetList list = compose_epis(kEpiEffimera);
	if(theme_id >= 0 && static_cast<std::size_t>(theme_id) < std::size(kComposeThemeEpithets)) {
		const ProcComposeThemeEpithets& te = kComposeThemeEpithets[static_cast<std::size_t>(theme_id)];
		list = te.generic;
		if(te.named.count > 0 && number(1, 100) <= named_pct) {
			list = te.named;
		}
	}
	if(list.count == 0) {
		return nullptr;
	}
	return &compose_pick(list.items, list.count);
}

/* true se `word` e' una delle parole separate da spazio di `kws` (confronto per parola
 * intera: "martello guerra" contiene "martello" ma non "marte"). */
[[nodiscard]] static bool compose_kw_has_word(std::string_view kws, std::string_view word) noexcept {
	std::size_t pos = 0;
	while(pos < kws.size()) {
		const std::size_t stop = std::min(kws.find(' ', pos), kws.size());
		if(kws.substr(pos, stop - pos) == word) {
			return true;
		}
		pos = stop + 1;
	}
	return false;
}

/*
 * Compone keywords / short / long per un nome gia' scelto.
 *   noun          nome (senza articolo, genere, keyword, maschera materiali)
 *   base_kw       keyword base dello slot, sempre presente
 *   default_mats  classi di materiale ammesse se noun.mats == 0
 *   quality       fascia + prestigio dal tiro dell'oggetto
 *   theme_id      ProcAreaInstance::theme_id (fuori range -> epiteto generico)
 * Casuale (number()); da chiamare al momento della consegna.
 */
[[nodiscard]] static ProcComposedName
procarea_compose_with_noun(const ProcComposeNoun& noun, const char* base_kw, unsigned default_mats,
						   const ProcComposeQuality& quality, int theme_id) {
	const int b = std::clamp(quality.band, 0, PROCAREA_TEMPLATE_BANDS - 1);
	const int prestige = std::clamp(quality.prestige, 0, 2);
	const unsigned allowed = noun.mats != 0 ? noun.mats : default_mats;
	const auto gn_idx = static_cast<std::size_t>(noun.gn);

	/* materiale: compatibile con il nome, di fascia, prestigio dal tiro. Fascia 9 al 50%
	 * forza un aggettivo leggendario + epiteto. Nomi con materiale proprio: nessuno. */
	const ProcComposeMaterialList& mats = kComposeMaterialGroups[procarea_compose_material_group(b)];
	bool force_epithet = false;
	const ProcComposeMaterial* mat = nullptr;
	if(!noun.own_material) {
		mat = compose_pick_material(mats, allowed, prestige, false);
		if(b == PROCAREA_TEMPLATE_BANDS - 1 && number(1, 100) <= kComposeLegendAdjPct) {
			if(const ProcComposeMaterial* adj = compose_pick_material(mats, allowed, prestige, true)) {
				mat = adj;
				force_epithet = true;
			}
		}
	}

	/* epiteto: mai con una keyword gia' presente (per parola) nel nome o nel materiale:
	 * "luna" + "della luna", "martello da guerra" + "del martello" */
	const ProcComposeEpithet* epi = compose_pick_epithet(theme_id, prestige, force_epithet);
	if(epi != nullptr) {
		const std::string_view epi_kw = epi->kw;
		if(compose_kw_has_word(noun.kw, epi_kw) || (mat != nullptr && compose_kw_has_word(mat->kw, epi_kw))) {
			epi = nullptr;
		}
	}

	/* short: nome [lato] materiale [epiteto], con fallback di lunghezza */
	const std::string noun_phrase = compose_with_article(ProcComposeArt::Definite, noun.gn, noun.text);
	std::string head = noun_phrase;
	const char* side = nullptr;
	if(noun.sided) {
		side = compose_side(noun.gn, number(0, 1) == 0);
		head += ' ';
		head += side;
	}
	if(mat != nullptr) {
		head += ' ';
		head += mat->forms[gn_idx];
	}

	ProcComposedName out;
	out.short_descr = head;
	if(epi != nullptr) {
		std::string full = head + ' ' + epi->text;
		if(full.size() <= kComposeMaxShort) {
			out.short_descr = std::move(full);
		} else {
			epi = nullptr; /* troppo lungo: niente epiteto */
		}
	}
	if(out.short_descr.size() > kComposeMaxShort) {
		out.short_descr = noun_phrase; /* ultima difesa */
		mat = nullptr;
		side = nullptr;
	}

	/* keywords: base slot, nome, lato, materiale, epiteto, effimera */
	compose_add_kw(out.keywords, base_kw);
	compose_add_kw(out.keywords, noun.kw);
	if(side != nullptr) {
		compose_add_kw(out.keywords, side);
	}
	if(mat != nullptr) {
		compose_add_kw(out.keywords, mat->kw);
	}
	if(epi != nullptr) {
		compose_add_kw(out.keywords, epi->kw);
	}
	compose_add_kw(out.keywords, "effimera");

	/* long: "<Short> giace qui, avvolto da un tremolio effimero." */
	out.long_descr = out.short_descr;
	if(!out.long_descr.empty()) {
		out.long_descr[0] =
			static_cast<char>(std::toupper(static_cast<unsigned char>(out.long_descr[0])));
	}
	out.long_descr += compose_is_plural(noun.gn) ? " giacciono qui, " : " giace qui, ";
	out.long_descr += compose_avvolto(noun.gn);
	out.long_descr += " da un tremolio effimero.";
	if(out.long_descr.size() > kComposeMaxLong) {
		out.long_descr = out.short_descr + '.'; /* mai oltre il limite */
	}
	return out;
}

/* Nome da lista (slot non-arma): sceglie a caso un nome della lista. */
[[nodiscard]] static ProcComposedName procarea_compose_from_nouns(const ProcComposeNounList& nouns,
																  const ProcComposeQuality& quality,
																  int theme_id) {
	if(nouns.items == nullptr || nouns.count == 0) {
		return {}; /* slot senza lista (Wield): restring_reward_gear ignora i testi vuoti */
	}
	return procarea_compose_with_noun(compose_pick(nouns.items, nouns.count), nouns.base_kw,
									  nouns.default_mats, quality, theme_id);
}

/* Arma premio: nome + tipo di danno scelti insieme. */
struct ProcComposedWeapon {
	ProcComposedName name;
	int damage_type; /* da assegnare a obj_flags.value[3] */
};

/*
 * family: 0 taglio, 1 punta, 2 botta (clamp) = famiglia del template.
 * Sceglie un tipo di danno uniforme tra quelli della famiglia, poi uno dei nomi di
 * quel tipo (almeno uno per tipo: static_assert compose_weapon_family_ok).
 */
[[nodiscard]] [[maybe_unused]] static ProcComposedWeapon
procarea_compose_weapon_name(int family, const ProcComposeQuality& quality, int theme_id) {
	const ProcComposeWeaponFamily& fam =
		kComposeWeaponFamilies[static_cast<std::size_t>(std::clamp(family, 0, 2))];
	std::size_t type_count = 0;
	while(type_count < fam.types.size() && fam.types[type_count] >= 0) {
		++type_count;
	}
	const int dtype = fam.types[static_cast<std::size_t>(number(0, static_cast<int>(type_count) - 1))];

	std::size_t matches = 0;
	for(std::size_t i = 0; i < fam.count; ++i) {
		matches += fam.items[i].damage_type == dtype ? 1 : 0;
	}
	int pick = number(0, static_cast<int>(matches) - 1);
	const ProcComposeWeaponNoun* chosen = &fam.items[0];
	for(std::size_t i = 0; i < fam.count; ++i) {
		if(fam.items[i].damage_type == dtype && pick-- == 0) {
			chosen = &fam.items[i];
			break;
		}
	}
	return {procarea_compose_with_noun(chosen->noun, "arma", fam.default_mats, quality, theme_id),
			dtype};
}

/*
 * Premio gear di uno slot diverso da Wield (le armi usano procarea_compose_weapon_name).
 * Regola assoluta: corazza (Body) con resistenza IMM_BLUNT = "la corazza di skaglie di
 * drago" (kComposeDragonArmor); il resto (epiteto) segue le regole normali e salta se
 * sfora il cap del short.
 */
[[nodiscard]] [[maybe_unused]] static ProcComposedName
procarea_compose_reward_name(ProcRewardGearSlot slot, const ProcComposeQuality& quality,
							 int theme_id) {
	if(slot == ProcRewardGearSlot::Body && quality.blunt_resist) {
		return procarea_compose_with_noun(kComposeDragonArmor, "corazza", 0, quality, theme_id);
	}
	const auto slot_idx = static_cast<std::size_t>(
		std::clamp(static_cast<int>(slot), 0, static_cast<int>(ProcRewardGearSlot::Count) - 1));
	return procarea_compose_from_nouns(kComposeNounsBySlot[slot_idx], quality, theme_id);
}

/* Scudo premio: stessi materiali per fascia e stessi epiteti per tema. */
[[nodiscard]] [[maybe_unused]] static ProcComposedName
procarea_compose_shield_name(const ProcComposeQuality& quality, int theme_id) {
	return procarea_compose_from_nouns(kComposeShieldNouns, quality, theme_id);
}

#endif /* __PROCAREA_REWARD_COMPOSE_INC */
