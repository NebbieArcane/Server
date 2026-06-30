/*ALARMUD* (Do not remove *ALARMUD*, used to automagically manage these lines
 *ALARMUD* AlarMUD 2.0
 *ALARMUD* See COPYING for licence information
 *ALARMUD*/
#ifndef __PROCAREA_TOP_HPP
#define __PROCAREA_TOP_HPP
#include "typedefs.hpp"
#include <string>
#include <vector>
namespace Alarmud {

struct ProcareaTopFilter {
	/** Fascia effettiva minima (1-10, come in gioco). 0 = nessun filtro. */
	int min_band_display = 0;
	/** Potenza minima al clear. 0 = nessun filtro. */
	int min_power_centi = 0;
};

enum class ProcareaTopKind {
	TodaySolo,
	TodayGroup,
	TodayAll,
	MonthlySolo,
	MonthlyGroup,
	MonthlyAll,
	WeekSolo,
	WeekGroup,
	WeekAll,
	LifetimeSolo,
	LifetimeGroup,
	LifetimeAll,
	Fragments,
	BestEffBand,
	BestPower,
	FastDay,
	FastWeek,
	CaptainClears,
	BestHoard,
	BestSigils,
};

struct ProcareaTopEntry {
	std::string name;
	int value = 0;
};

constexpr int PROCAREA_TOP_LIMIT_DEFAULT = 10;
constexpr int PROCAREA_TOP_LIMIT_MAX = 25;

[[nodiscard]] const char* procarea_top_kind_title(ProcareaTopKind kind);
[[nodiscard]] const char* procarea_top_kind_unit(ProcareaTopKind kind);
[[nodiscard]] bool procarea_top_higher_is_better(ProcareaTopKind kind);
[[nodiscard]] std::string procarea_top_format_value(ProcareaTopKind kind, int value);

[[nodiscard]] int procarea_top_value_for_char(const char_data* ch, ProcareaTopKind kind);
[[nodiscard]] int procarea_top_value_for_name(const char* name, ProcareaTopKind kind);

void procarea_top_fetch(ProcareaTopKind kind, int limit, std::vector<ProcareaTopEntry>& out,
						const ProcareaTopFilter& filter = {});
[[nodiscard]] int procarea_top_rank_for_name(const char* name, ProcareaTopKind kind, int value,
										   const ProcareaTopFilter& filter = {});

} // namespace Alarmud
#endif // __PROCAREA_TOP_HPP
