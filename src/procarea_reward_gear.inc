/* Slot premio effimero: 6 varianti per band (12 se doppio). Esclusi shield, back, loaded_weapon. */
static constexpr int kProcRewardGearCosts[PROCAREA_TEMPLATE_BANDS] = {
	500, 1200, 2200, 3500, 5000, 7500,
};

struct ProcRewardGearSlotDef {
	ProcRewardGearSlot slot;
	int type_flag;
	const char* wear_flags;
	int variant_count;
	int ac_min;
	int ac_max;
	int weapon_dice_num_min;
	int weapon_dice_num_max;
	int weapon_dice_side_min;
	int weapon_dice_side_max;
	int light_hours;
	const char* keyword;
	const char* short_noun;
	const char* dual_side_a;
	const char* dual_side_b;
};

static constexpr ProcRewardGearSlotDef kProcRewardGearSlots[] = {
	{
		ProcRewardGearSlot::Light,
		ITEM_LIGHT,
		"1",
		PROCAREA_TEMPLATE_BANDS,
		0, 0,
		0, 0, 0, 0,
		-1,
		"fiammella luce effimera",
		"una fiammella",
		nullptr, nullptr,
	},
	{
		ProcRewardGearSlot::Finger,
		ITEM_JEWEL,
		"1|2",
		PROCAREA_TEMPLATE_BANDS * 2,
		0, 0,
		0, 0, 0, 0,
		0,
		"anello effimero",
		"un anello",
		"sinistro", "destro",
	},
	{
		ProcRewardGearSlot::Neck,
		ITEM_JEWEL,
		"1|4",
		PROCAREA_TEMPLATE_BANDS * 2,
		0, 0,
		0, 0, 0, 0,
		0,
		"collare effimero",
		"un collare",
		"corto", "lungo",
	},
	{
		ProcRewardGearSlot::Body,
		ITEM_ARMOR,
		"1|8",
		PROCAREA_TEMPLATE_BANDS,
		6, 14,
		0, 0, 0, 0,
		0,
		"corazza effimera",
		"una corazza",
		nullptr, nullptr,
	},
	{
		ProcRewardGearSlot::Head,
		ITEM_ARMOR,
		"1|16",
		PROCAREA_TEMPLATE_BANDS,
		3, 8,
		0, 0, 0, 0,
		0,
		"elmo effimero",
		"un elmo",
		nullptr, nullptr,
	},
	{
		ProcRewardGearSlot::Legs,
		ITEM_ARMOR,
		"1|32",
		PROCAREA_TEMPLATE_BANDS,
		4, 10,
		0, 0, 0, 0,
		0,
		"gambali effimera",
		"dei gambali",
		nullptr, nullptr,
	},
	{
		ProcRewardGearSlot::Feet,
		ITEM_ARMOR,
		"1|64",
		PROCAREA_TEMPLATE_BANDS,
		2, 6,
		0, 0, 0, 0,
		0,
		"calzari effimera",
		"dei calzari",
		nullptr, nullptr,
	},
	{
		ProcRewardGearSlot::Hands,
		ITEM_ARMOR,
		"1|128",
		PROCAREA_TEMPLATE_BANDS,
		2, 5,
		0, 0, 0, 0,
		0,
		"guanti effimera",
		"dei guanti",
		nullptr, nullptr,
	},
	{
		ProcRewardGearSlot::Arms,
		ITEM_ARMOR,
		"1|256",
		PROCAREA_TEMPLATE_BANDS,
		3, 7,
		0, 0, 0, 0,
		0,
		"avambracci effimera",
		"degli avambracci",
		nullptr, nullptr,
	},
	{
		ProcRewardGearSlot::About,
		ITEM_ARMOR,
		"1|1024",
		PROCAREA_TEMPLATE_BANDS,
		3, 8,
		0, 0, 0, 0,
		0,
		"mantello effimero",
		"un mantello",
		nullptr, nullptr,
	},
	{
		ProcRewardGearSlot::Waist,
		ITEM_ARMOR,
		"1|2048",
		PROCAREA_TEMPLATE_BANDS,
		2, 6,
		0, 0, 0, 0,
		0,
		"cintura effimera",
		"una cintura",
		nullptr, nullptr,
	},
	{
		ProcRewardGearSlot::Wrist,
		ITEM_JEWEL,
		"1|4096",
		PROCAREA_TEMPLATE_BANDS * 2,
		0, 0,
		0, 0, 0, 0,
		0,
		"bracciale effimero",
		"un bracciale",
		"sinistro", "destro",
	},
	{
		ProcRewardGearSlot::Wield,
		ITEM_WEAPON,
		"1|8192",
		PROCAREA_TEMPLATE_BANDS * 3,
		0, 0,
		0, 0, 0, 0,
		0,
		"arma effimera",
		"un'arma",
		nullptr, nullptr,
	},
	{
		ProcRewardGearSlot::Hold,
		ITEM_JEWEL,
		"1|16384",
		PROCAREA_TEMPLATE_BANDS,
		0, 0,
		0, 0, 0, 0,
		0,
		"focus effimero",
		"un focus",
		nullptr, nullptr,
	},
	{
		ProcRewardGearSlot::Ear,
		ITEM_JEWEL,
		"1|262144",
		PROCAREA_TEMPLATE_BANDS * 2,
		0, 0,
		0, 0, 0, 0,
		0,
		"orecchino effimero",
		"un orecchino",
		"sinistro", "destro",
	},
	{
		ProcRewardGearSlot::Eyes,
		ITEM_JEWEL,
		"1|524288",
		PROCAREA_TEMPLATE_BANDS,
		0, 0,
		0, 0, 0, 0,
		0,
		"lente effimera",
		"una lente",
		nullptr, nullptr,
	},
};

static_assert(std::size(kProcRewardGearSlots) == static_cast<std::size_t>(ProcRewardGearSlot::Count),
			  "procarea reward gear slot table incomplete");

static constexpr int kProcRewardGearTotalVariants = []() constexpr {
	int total = 0;
	for(const ProcRewardGearSlotDef& def : kProcRewardGearSlots) {
		total += def.variant_count;
	}
	return total;
}();

static_assert(kProcRewardGearTotalVariants == PROCAREA_REWARD_GEAR_COUNT,
			  "procarea reward gear vnum span mismatch");
