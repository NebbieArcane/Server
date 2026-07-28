/*ALARMUD* (Do not remove *ALARMUD*, used to automagically manage these lines
 *ALARMUD* AlarMUD 2.0
 *ALARMUD* See COPYING for licence information
 *ALARMUD*/
#ifndef __OBJ_VALUE_HPP
#define __OBJ_VALUE_HPP
/***************************  System  include ************************************/
#include <string>
/***************************  Local    include ************************************/
#include "typedefs.hpp"

namespace Alarmud {

/**
 * Valore di edit di un oggetto (idea da branch ProvaLocale).
 *
 * - valore: costo "edit" stimato dagli APPLY sull'oggetto
 * - derent: componente legata a cost_per_day (affitto)
 * - rune:  rune stimata da cost_per_day
 *
 * CheckValueObj restituisce unita' "raw".
 * CheckDiffValue restituisce la differenza rispetto al prototipo in unita'
 * storage legacy (raw * kObjValueStorageScale) per valore/derent, come in
 * ProvaLocale quando i campi venivano cachati su obj->value_exp.
 */
struct ExpValue {
	long valore{};
	long derent{};
	int rune{};
};

/** Report completo per wiz (stat): valori + elenco modifiche vs prototipo. */
struct ObjEditAnalysis {
	ExpValue absolute{};
	ExpValue diff{};
	/** Righe descrittive delle differenze (vuoto se identico al prototipo). */
	std::string changes;
	/**
	 * true se l'oggetto e' marcato EDIT/PERSONAL, oppure ha diff di valore,
	 * oppure ha differenze strutturali vs prototipo.
	 */
	bool has_edit{};
};

/** Fattore usato in ProvaLocale per cachare value_exp / value_exp_total. */
inline constexpr long kObjValueStorageScale = 10000L;

/** Allineati a pedit.cpp (RentEditor). */
inline constexpr int kObjValuePriceRune = 1;
inline constexpr long kObjValuePriceExp = 2000000L;

/**
 * Stima il valore assoluto di un oggetto dagli affect + cost_per_day.
 * Non modifica l'oggetto. obj == nullptr -> {0,0,0}.
 */
[[nodiscard]] ExpValue CheckValueObj(const struct obj_data* obj);

/**
 * Differenza di valore rispetto al prototipo (vnum / char_vnum se PERSONAL).
 * Unita': valore/derent in scala storage (* kObjValueStorageScale), rune raw.
 * Se l'oggetto ha ITEM_IMMUNE e il prototipo no, il valore e' aumentato del 50%.
 */
[[nodiscard]] ExpValue CheckDiffValue(struct obj_data* obj);

/**
 * Analisi unica (un solo load del prototipo): valori + testo modifiche.
 * Usare questo da wiz invece di chiamare Check* + Describe separati.
 */
[[nodiscard]] ObjEditAnalysis AnalyzeObjEdit(struct obj_data* obj);

/**
 * Applica la scala storage legacy a un ExpValue raw (valore/derent * scale).
 */
[[nodiscard]] ExpValue ScaleObjExpValue(const ExpValue& raw,
										long scale = kObjValueStorageScale) noexcept;

} // namespace Alarmud
#endif // __OBJ_VALUE_HPP
