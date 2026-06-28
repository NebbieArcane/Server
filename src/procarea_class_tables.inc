/* Tabelle classe mob procarea: 10 classi PG (no stregone) + none solo corridoio. */
#ifndef __PROCAREA_CLASS_TABLES_INC
#define __PROCAREA_CLASS_TABLES_INC

struct ProcCorridorClassChancePct {
	int warrior;
	int cleric;
	int mage;
	int thief;
	int psi;
	int druid;
	int monk;
	int barbarian;
	int paladin;
	int ranger;
	int none;
};

struct ProcTreasureClassChancePct {
	int warrior;
	int cleric;
	int mage;
	int thief;
	int psi;
	int druid;
	int monk;
	int barbarian;
	int paladin;
	int ranger;
};

/** Corridoio: none 2–5% (più alto in basso); TH/PSI da banda 2. */
static constexpr ProcCorridorClassChancePct kProcCorridorClassByBand[PROCAREA_TEMPLATE_BANDS] = {
	{28, 18, 8, 0, 0, 9, 5, 12, 6, 9, 5},  /* band 0 */
	{27, 18, 9, 0, 0, 9, 5, 12, 7, 9, 4},  /* band 1 */
	{24, 16, 9, 3, 1, 10, 6, 11, 7, 9, 4}, /* band 2 */
	{21, 16, 10, 4, 2, 10, 6, 11, 7, 9, 4}, /* band 3 */
	{20, 15, 11, 4, 2, 10, 7, 10, 8, 9, 4}, /* band 4 */
	{19, 15, 11, 4, 2, 10, 7, 10, 8, 11, 3}, /* band 5 */
	{17, 14, 12, 5, 3, 11, 7, 9, 8, 11, 3}, /* band 6 */
	{16, 14, 13, 5, 3, 11, 8, 9, 9, 10, 2}, /* band 7 */
	{14, 13, 13, 6, 4, 12, 8, 8, 9, 11, 2}, /* band 8 */
	{12, 13, 13, 6, 4, 12, 8, 8, 10, 12, 2}, /* band 9 */
};

/** Tesoro: sempre una classe; TH leggermente più frequente del corridoio. */
static constexpr ProcTreasureClassChancePct kProcTreasureClassByBand[PROCAREA_TEMPLATE_BANDS] = {
	{26, 19, 9, 0, 0, 10, 6, 13, 7, 10}, /* band 0 */
	{28, 18, 9, 0, 0, 10, 6, 12, 7, 10}, /* band 1 */
	{25, 17, 9, 4, 1, 10, 6, 11, 7, 10}, /* band 2 */
	{22, 16, 10, 5, 2, 10, 6, 11, 7, 11}, /* band 3 */
	{21, 15, 11, 5, 2, 10, 7, 10, 8, 11}, /* band 4 */
	{21, 15, 11, 5, 2, 10, 7, 10, 8, 11}, /* band 5 */
	{19, 14, 12, 6, 3, 11, 7, 9, 8, 11}, /* band 6 */
	{17, 14, 13, 6, 3, 11, 8, 9, 9, 10}, /* band 7 */
	{15, 13, 13, 7, 4, 12, 8, 8, 9, 11}, /* band 8 */
	{13, 13, 13, 7, 4, 12, 8, 8, 10, 12}, /* band 9 */
};

[[nodiscard]] constexpr int procarea_corridor_class_sum(const ProcCorridorClassChancePct& row) {
	return row.warrior + row.cleric + row.mage + row.thief + row.psi + row.druid +
		   row.monk + row.barbarian + row.paladin + row.ranger + row.none;
}

[[nodiscard]] constexpr int procarea_treasure_class_sum(const ProcTreasureClassChancePct& row) {
	return row.warrior + row.cleric + row.mage + row.thief + row.psi + row.druid +
		   row.monk + row.barbarian + row.paladin + row.ranger;
}

/** Boss: prevalentemente CL/MU; MuCleric è un bucket a sé. */
enum class ProcBossRollClass : int {
	MuCleric = 0,
	Cleric,
	Mage,
	Warrior,
	Thief,
	Psi,
	Druid,
	Monk,
	Barbarian,
	Paladin,
	Ranger,
	Count,
};

static constexpr int kProcBossClassWeights[static_cast<int>(ProcBossRollClass::Count)] = {
	30, /* MuCleric */
	10, /* Cleric */
	12, /* Mage */
	8,  /* Warrior */
	5,  /* Thief */
	6,  /* Psi */
	7,  /* Druid */
	4,  /* Monk */
	7,  /* Barbarian */
	5,  /* Paladin */
	6,  /* Ranger */
};

#endif /* __PROCAREA_CLASS_TABLES_INC */
