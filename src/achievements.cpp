/*ALARMUD* (Do not remove *ALARMUD*, used to automagically manage these lines
 *ALARMUD* AlarMUD 2.0
 *ALARMUD* See COPYING for licence information
 *ALARMUD*/
/* Achievement: controllo, premi, quest Mercy e comandi achievements/checkachie. */
/***************************  System  include ************************************/
#include <malloc.h>
#include <cstring>
#include <cctype>
#include <charconv>
#include <optional>
#include <string_view>
#include <system_error>
#include <ctime>
#include <sys/file.h>
#include <cstdlib>
#include <unistd.h>
#include <cstdarg>
#include <algorithm>
#include <array>
#include <cmath>
#include <limits>
#include <random>
#include <string>
#include <utility>
#include <cstdio>
#include <cstdint>
#include <cerrno>
#include <iomanip>
#include <sstream>
#include <unordered_map>
#include <vector>
/***************************  Local    include ************************************/
#include "config.hpp"
#include "typedefs.hpp"
#include "flags.hpp"
#include "autoenums.hpp"
#include "structs.hpp"
#include "logging.hpp"
#include "constants.hpp"
#include "utils.hpp"
#include "utility.hpp"
#include "act.info.hpp"
#include "act.off.hpp"
#include "act.other.hpp"
#include "aree.hpp"
#include "comm.hpp"
#include "db.hpp"
#include "fight.hpp"
#include "handler.hpp"
#include "interpreter.hpp"
#include "magic2.hpp"
#include "mobact.hpp"
#include "modify.hpp"
#include "opinion.hpp"
#include "reception.hpp"
#include "skills.hpp"
#include "sound.hpp"
#include "spell_parser.hpp"
#include "spells.hpp"
#include "trap.hpp"
#include "weather.hpp"
#include "procarea.hpp"
#include "act.comm.hpp"
#include "act.wizard.hpp"
#include "object_instance.hpp"
#include "breath.hpp"
#include "spec_dragon.hpp"
#include "cmdid.hpp"
#include "events.hpp"
#include "magicutils.hpp"
#include "maximums.hpp"
#include "multiclass.hpp"
#include "parser.hpp"
#include "procarea_fatigue.hpp"
#include "procarea_rune_fragments.hpp"
#include "signals.hpp"
#include "snew.hpp"
#include "spec_procs.hpp"
#include "spec_procs2.hpp"
#include "specialproc_other.hpp"
#include "spells.hpp"        // for spell_info_type, SKILL_EVALUATE, SPELL_G...
#include "Sql.hpp"
#include "server_text.hpp"
#include "version.hpp"
#include "vt100c.hpp"
#include "act.info.hpp"
#include "achievements.hpp"

namespace Alarmud {

namespace {
/*
 * A capo nel cartiglio: CRLF completo e cornice di nuovo gialla su ogni riga,
 * qualunque client. \r\n (non \n\r) cosi' l'OutputBatch non lo spezza in righe
 * e non raggruppa le righe "| |   | |" uguali in (xN).
 */
constexpr std::string_view kScrollBreak = "\r\n$c0011";

/** Riga del cartiglio: testo colorato centrato su max colonne (len = lunghezza senza colori). */
[[nodiscard]] std::string centered_row(const std::string& colored, std::size_t len, std::size_t max) {
	std::string row = "    ";
	if(max > len) {
		const std::size_t gap = max - len;
		row.append(gap - gap / 2, ' ');
		row += colored;
		row.append(gap / 2, ' ');
	}
	else {
		row += colored;
	}
	row += "    |-|";
	row += kScrollBreak;
	row += "| |";
	return row;
}

/** Cartiglio ASCII mostrato a chi completa un achievement. */
[[nodiscard]] std::string spamAchie(char_data* ch, const std::string& titolo, int valore, const std::string& stringa) {
	const std::string name = GET_NAME(ch);
	const std::string numero = format_thousands(valore);
	const std::array<std::string, 3> plain{
		name + " ha completato l'achievement",
		titolo,
		numero + " " + stringa
	};
	const std::array<std::string, 3> colored{
		"$c0009" + name + " $c0007ha completato l'achievement$c0011",
		"$c0015" + titolo + "$c0011",
		"$c0009" + numero + " " + stringa + "$c0011"
	};

	std::size_t max = 0;
	for(const auto& row : plain) {
		max = std::max(max, row.size());
	}

	const std::size_t padded_width = std::min<std::size_t>(max, 90) + 8;
	const std::string space(padded_width, ' ');

	const std::string br(kScrollBreak);
	std::string buffer = "$c0011 _" + br + "(@)";
	buffer += space + " _" + br + "| |" + space + "(@)" + br + "|-|";
	buffer += std::string(padded_width, '-') + "|-|" + br + "| |";
	for(std::size_t i = 0; i < plain.size(); ++i) {
		buffer += space + "| |" + br + "|-|";
		buffer += centered_row(colored[i], plain[i].size(), max);
	}
	buffer += std::string(padded_width, '_') + "| |" + br + "(@)";
	buffer += space + "| |" + br + "   " + space + "(@)\r\n\n\r";
	return buffer;
}
} // namespace

namespace {
int MaxValueAchievement(int achievement_class, int achievement_type, int achievement_level)
{
    int massimo = 0;

    switch(achievement_level)
    {
        case 1:
            massimo = AchievementsList[achievement_class][achievement_type].lvl1_val;
            break;
        case 2:
            massimo = AchievementsList[achievement_class][achievement_type].lvl2_val;
            break;
        case 3:
            massimo = AchievementsList[achievement_class][achievement_type].lvl3_val;
            break;
        case 4:
            massimo = AchievementsList[achievement_class][achievement_type].lvl4_val;
            break;
        case 5:
            massimo = AchievementsList[achievement_class][achievement_type].lvl5_val;
            break;
        case 6:
            massimo = AchievementsList[achievement_class][achievement_type].lvl6_val;
            break;
        case 7:
            massimo = AchievementsList[achievement_class][achievement_type].lvl7_val;
            break;
        case 8:
            massimo = AchievementsList[achievement_class][achievement_type].lvl8_val;
            break;
        case 9:
            massimo = AchievementsList[achievement_class][achievement_type].lvl9_val;
            break;
        case 10:
            massimo = AchievementsList[achievement_class][achievement_type].lvl10_val;
            break;

        default:
            mudlog(LOG_CHECK, "Something wrong in MaxValueAchievement, check the Achievement table");
            break;
    }

    return massimo;
}
} // namespace

namespace {
int maxAchievements(struct char_data* ch)
{
    struct char_data* tch;
    int conteggio = 0, i;

    tch = ch;

    if(IS_POLY(tch))
    {
        tch = ch->desc->original;
    }

    // Race Achievements
    for(i = 0; i < MAX_RACE_ACHIE; i++)
    {
        if(HasClass(tch, AchievementsList[RACESLAYER_ACHIE][i].classe) || AchievementsList[RACESLAYER_ACHIE][i].classe == 0)
        {
            conteggio += AchievementsList[RACESLAYER_ACHIE][i].n_livelli;
        }
    }

    // Boss Achievements
    for(i = 0; i < MAX_BOSS_ACHIE; i++)
    {
        if(HasClass(tch, AchievementsList[BOSSKILL_ACHIE][i].classe) || AchievementsList[BOSSKILL_ACHIE][i].classe == 0)
        {
            conteggio += AchievementsList[BOSSKILL_ACHIE][i].n_livelli;
        }
    }

    // Class Skill Achievements
    for(i = 1; i < MAX_CLASS_ACHIE; i++)
    {
        if(HasClass(tch, AchievementsList[CLASS_ACHIE][i].classe) || AchievementsList[CLASS_ACHIE][i].classe == 0)
        {
            conteggio += AchievementsList[CLASS_ACHIE][i].n_livelli;
        }
    }

    // Quest Achievements
    for(i = 0; i < MAX_QUEST_ACHIE; i++)
    {
        if(HasClass(tch, AchievementsList[QUEST_ACHIE][i].classe) || AchievementsList[QUEST_ACHIE][i].classe == 0)
        {
            conteggio += AchievementsList[QUEST_ACHIE][i].n_livelli;
        }
    }

    // Various Achievements
    for(i = 0; i < MAX_OTHER_ACHIE; i++)
    {
        if(HasClass(tch, AchievementsList[OTHER_ACHIE][i].classe) || AchievementsList[OTHER_ACHIE][i].classe == 0)
        {
            conteggio += AchievementsList[OTHER_ACHIE][i].n_livelli;
        }
    }

    return conteggio;
}
} // namespace

void CheckQuestFail(struct char_data* ch)
{
    struct char_data* tch;
    long long diff_hunt = 0, diff_resc = 0, diff_resea = 0, diff_deliv = 0;

    tch = ch;

    if(IS_POLY(tch))
    {
        tch = ch->desc->original;
    }

    if(!IS_PC(tch))
    {
        return;
    }

    const long long quest_total_target = static_cast<long long>(tch->specials.achievements[OTHER_ACHIE][ACHIE_QUEST_TOTAL]) -
                                         static_cast<long long>(tch->specials.achievements[OTHER_ACHIE][ACHIE_QUEST_COMPLETE]);
    const long long quest_total_failed_target = (quest_total_target > 0LL) ? quest_total_target : 0LL;
    if(static_cast<long long>(tch->specials.achievements[OTHER_ACHIE][ACHIE_QUEST_FAILED]) != quest_total_failed_target)
    {
        if(static_cast<long long>(tch->specials.achievements[OTHER_ACHIE][ACHIE_QUEST_FAILED]) < quest_total_failed_target)
        {
            mudlog(LOG_CHECK, "Something going wrong in Quest Fail Total on %s, the value is less than the correct", GET_NAME(tch));

            while(static_cast<long long>(tch->specials.achievements[OTHER_ACHIE][ACHIE_QUEST_FAILED]) < quest_total_failed_target)
            {
                tch->specials.achievements[OTHER_ACHIE][ACHIE_QUEST_FAILED] += 1;
                CheckAchie(ch, ACHIE_QUEST_FAILED, OTHER_ACHIE);
            }

            mudlog(LOG_CHECK, "Fixed amount for Quest Fail Total on %s", GET_NAME(tch));
        }
        else if(static_cast<long long>(tch->specials.achievements[OTHER_ACHIE][ACHIE_QUEST_FAILED]) > quest_total_failed_target)
        {
            mudlog(LOG_CHECK, "Something going wrong in Quest Fail Total on %s, the value is greater than the correct", GET_NAME(tch));

            while(static_cast<long long>(tch->specials.achievements[OTHER_ACHIE][ACHIE_QUEST_FAILED]) > quest_total_failed_target)
            {
                tch->specials.achievements[OTHER_ACHIE][ACHIE_QUEST_FAILED] -= 1;
                CheckAchie(ch, ACHIE_QUEST_FAILED, OTHER_ACHIE);
            }

            mudlog(LOG_CHECK, "Fixed amount for Quest Fail Total on %s", GET_NAME(tch));
        }
        else
        {
            tch->specials.achievements[OTHER_ACHIE][ACHIE_QUEST_FAILED] += 1;
            CheckAchie(ch, ACHIE_QUEST_FAILED, OTHER_ACHIE);
        }
    }

    for(int pass = 0; pass < 8; ++pass)
    {
    diff_hunt   = static_cast<long long>(tch->specials.achievements[OTHER_ACHIE][ACHIE_QUEST_HUNT_TOTAL]) - static_cast<long long>(tch->specials.achievements[OTHER_ACHIE][ACHIE_QUEST_HUNT_COMPLETE]) - static_cast<long long>(tch->specials.achievements[OTHER_ACHIE][ACHIE_QUEST_HUNT_FAILED]);
    diff_resc   = static_cast<long long>(tch->specials.achievements[OTHER_ACHIE][ACHIE_QUEST_RESCUE_TOTAL]) - static_cast<long long>(tch->specials.achievements[OTHER_ACHIE][ACHIE_QUEST_RESCUE_COMPLETE]) - static_cast<long long>(tch->specials.achievements[OTHER_ACHIE][ACHIE_QUEST_RESCUE_FAILED]);
    diff_resea  = static_cast<long long>(tch->specials.achievements[OTHER_ACHIE][ACHIE_QUEST_RESEARCH_TOTAL]) - static_cast<long long>(tch->specials.achievements[OTHER_ACHIE][ACHIE_QUEST_RESEARCH_COMPLETE]) - static_cast<long long>(tch->specials.achievements[OTHER_ACHIE][ACHIE_QUEST_RESEARCH_FAILED]);
    diff_deliv  = static_cast<long long>(tch->specials.achievements[OTHER_ACHIE][ACHIE_QUEST_DELIVERY_TOTAL]) - static_cast<long long>(tch->specials.achievements[OTHER_ACHIE][ACHIE_QUEST_DELIVERY_COMPLETE]) - static_cast<long long>(tch->specials.achievements[OTHER_ACHIE][ACHIE_QUEST_DELIVERY_FAILED]);

    if(diff_hunt == 0 && diff_resc == 0 && diff_resea == 0 && diff_deliv == 0)
    {
        return;
    }
    if(pass > 0)
    {
        mudlog(LOG_CHECK, "Check the Quest's values on %s", GET_NAME(tch));
    }

    if(diff_hunt != 0)
    {
        const long long quest_hunt_target = static_cast<long long>(tch->specials.achievements[OTHER_ACHIE][ACHIE_QUEST_HUNT_TOTAL]) -
                                            static_cast<long long>(tch->specials.achievements[OTHER_ACHIE][ACHIE_QUEST_HUNT_COMPLETE]);
        const long long quest_hunt_failed_target = (quest_hunt_target > 0LL) ? quest_hunt_target : 0LL;
        if(static_cast<long long>(tch->specials.achievements[OTHER_ACHIE][ACHIE_QUEST_HUNT_FAILED]) < quest_hunt_failed_target)
        {
            mudlog(LOG_CHECK, "Something going wrong in Quest Fail Hunt on %s, the value is less than the correct", GET_NAME(tch));

            while(static_cast<long long>(tch->specials.achievements[OTHER_ACHIE][ACHIE_QUEST_HUNT_FAILED]) < quest_hunt_failed_target)
            {
                tch->specials.achievements[OTHER_ACHIE][ACHIE_QUEST_HUNT_FAILED] += 1;
                CheckAchie(ch, ACHIE_QUEST_HUNT_FAILED, OTHER_ACHIE);
            }

            mudlog(LOG_CHECK, "Fixed amount for Quest Fail Hunt on %s", GET_NAME(tch));
        }
        else if(static_cast<long long>(tch->specials.achievements[OTHER_ACHIE][ACHIE_QUEST_HUNT_FAILED]) > quest_hunt_failed_target)
        {
            mudlog(LOG_CHECK, "Something going wrong in Quest Fail Hunt on %s, the value is greater than the correct", GET_NAME(tch));

            while(static_cast<long long>(tch->specials.achievements[OTHER_ACHIE][ACHIE_QUEST_HUNT_FAILED]) > quest_hunt_failed_target)
            {
                tch->specials.achievements[OTHER_ACHIE][ACHIE_QUEST_HUNT_FAILED] -= 1;
                CheckAchie(ch, ACHIE_QUEST_HUNT_FAILED, OTHER_ACHIE);
            }

            mudlog(LOG_CHECK, "Fixed amount for Quest Fail Hunt on %s", GET_NAME(tch));
        }
        else
        {
            mudlog(LOG_SYSERR, "CheckQuestFail Hunt: %s complete>total, skip", GET_NAME(tch));
            break;
        }
    }
    else if(diff_resc != 0)
    {
        const long long quest_rescue_target = static_cast<long long>(tch->specials.achievements[OTHER_ACHIE][ACHIE_QUEST_RESCUE_TOTAL]) -
                                              static_cast<long long>(tch->specials.achievements[OTHER_ACHIE][ACHIE_QUEST_RESCUE_COMPLETE]);
        const long long quest_rescue_failed_target = (quest_rescue_target > 0LL) ? quest_rescue_target : 0LL;
        if(static_cast<long long>(tch->specials.achievements[OTHER_ACHIE][ACHIE_QUEST_RESCUE_FAILED]) < quest_rescue_failed_target)
        {
            mudlog(LOG_CHECK, "Something going wrong in Quest Fail Rescue on %s, the value is less than the correct", GET_NAME(tch));

            while(static_cast<long long>(tch->specials.achievements[OTHER_ACHIE][ACHIE_QUEST_RESCUE_FAILED]) < quest_rescue_failed_target)
            {
                tch->specials.achievements[OTHER_ACHIE][ACHIE_QUEST_RESCUE_FAILED] += 1;
                CheckAchie(ch, ACHIE_QUEST_RESCUE_FAILED, OTHER_ACHIE);
            }

            mudlog(LOG_CHECK, "Fixed amount for Quest Fail Rescue on %s", GET_NAME(tch));
        }
        else if(static_cast<long long>(tch->specials.achievements[OTHER_ACHIE][ACHIE_QUEST_RESCUE_FAILED]) > quest_rescue_failed_target)
        {
            mudlog(LOG_CHECK, "Something going wrong in Quest Fail Rescue on %s, the value is greater than the correct", GET_NAME(tch));

            while(static_cast<long long>(tch->specials.achievements[OTHER_ACHIE][ACHIE_QUEST_RESCUE_FAILED]) > quest_rescue_failed_target)
            {
                tch->specials.achievements[OTHER_ACHIE][ACHIE_QUEST_RESCUE_FAILED] -= 1;
                CheckAchie(ch, ACHIE_QUEST_RESCUE_FAILED, OTHER_ACHIE);
            }

            mudlog(LOG_CHECK, "Fixed amount for Quest Fail Rescue on %s", GET_NAME(tch));
        }
        else
        {
            mudlog(LOG_SYSERR, "CheckQuestFail Rescue: %s complete>total, skip", GET_NAME(tch));
            break;
        }
    }
    else if(diff_resea != 0)
    {
        const long long quest_research_target = static_cast<long long>(tch->specials.achievements[OTHER_ACHIE][ACHIE_QUEST_RESEARCH_TOTAL]) -
                                                static_cast<long long>(tch->specials.achievements[OTHER_ACHIE][ACHIE_QUEST_RESEARCH_COMPLETE]);
        const long long quest_research_failed_target = (quest_research_target > 0LL) ? quest_research_target : 0LL;
        if(static_cast<long long>(tch->specials.achievements[OTHER_ACHIE][ACHIE_QUEST_RESEARCH_FAILED]) < quest_research_failed_target)
        {
            mudlog(LOG_CHECK, "Something going wrong in Quest Fail Research on %s, the value is less than the correct", GET_NAME(tch));

            while(static_cast<long long>(tch->specials.achievements[OTHER_ACHIE][ACHIE_QUEST_RESEARCH_FAILED]) < quest_research_failed_target)
            {
                tch->specials.achievements[OTHER_ACHIE][ACHIE_QUEST_RESEARCH_FAILED] += 1;
                CheckAchie(ch, ACHIE_QUEST_RESEARCH_FAILED, OTHER_ACHIE);
            }

            mudlog(LOG_CHECK, "Fixed amount for Quest Fail Research on %s", GET_NAME(tch));
        }
        else if(static_cast<long long>(tch->specials.achievements[OTHER_ACHIE][ACHIE_QUEST_RESEARCH_FAILED]) > quest_research_failed_target)
        {
            mudlog(LOG_CHECK, "Something going wrong in Quest Fail Research on %s, the value is greater than the correct", GET_NAME(tch));

            while(static_cast<long long>(tch->specials.achievements[OTHER_ACHIE][ACHIE_QUEST_RESEARCH_FAILED]) > quest_research_failed_target)
            {
                tch->specials.achievements[OTHER_ACHIE][ACHIE_QUEST_RESEARCH_FAILED] -= 1;
                CheckAchie(ch, ACHIE_QUEST_RESEARCH_FAILED, OTHER_ACHIE);
            }

            mudlog(LOG_CHECK, "Fixed amount for Quest Fail Research on %s", GET_NAME(tch));
        }
        else
        {
            mudlog(LOG_SYSERR, "CheckQuestFail Research: %s complete>total, skip", GET_NAME(tch));
            break;
        }
    }
    else if(diff_deliv != 0)
    {
        const long long quest_delivery_target = static_cast<long long>(tch->specials.achievements[OTHER_ACHIE][ACHIE_QUEST_DELIVERY_TOTAL]) -
                                                static_cast<long long>(tch->specials.achievements[OTHER_ACHIE][ACHIE_QUEST_DELIVERY_COMPLETE]);
        const long long quest_delivery_failed_target = (quest_delivery_target > 0LL) ? quest_delivery_target : 0LL;
        if(static_cast<long long>(tch->specials.achievements[OTHER_ACHIE][ACHIE_QUEST_DELIVERY_FAILED]) < quest_delivery_failed_target)
        {
            mudlog(LOG_CHECK, "Something going wrong in Quest Fail Delivery on %s, the value is less than the correct", GET_NAME(tch));

            while(static_cast<long long>(tch->specials.achievements[OTHER_ACHIE][ACHIE_QUEST_DELIVERY_FAILED]) < quest_delivery_failed_target)
            {
                tch->specials.achievements[OTHER_ACHIE][ACHIE_QUEST_DELIVERY_FAILED] += 1;
                CheckAchie(ch, ACHIE_QUEST_DELIVERY_FAILED, OTHER_ACHIE);
            }

            mudlog(LOG_CHECK, "Fixed amount for Quest Fail Delivery on %s", GET_NAME(tch));
        }
        else if(static_cast<long long>(tch->specials.achievements[OTHER_ACHIE][ACHIE_QUEST_DELIVERY_FAILED]) > quest_delivery_failed_target)
        {
            mudlog(LOG_CHECK, "Something going wrong in Quest Fail Delivery on %s, the value is greater than the correct", GET_NAME(tch));

            while(static_cast<long long>(tch->specials.achievements[OTHER_ACHIE][ACHIE_QUEST_DELIVERY_FAILED]) > quest_delivery_failed_target)
            {
                tch->specials.achievements[OTHER_ACHIE][ACHIE_QUEST_DELIVERY_FAILED] -= 1;
                CheckAchie(ch, ACHIE_QUEST_DELIVERY_FAILED, OTHER_ACHIE);
            }

            mudlog(LOG_CHECK, "Fixed amount for Quest Fail Delivery on %s", GET_NAME(tch));
        }
        else
        {
            mudlog(LOG_SYSERR, "CheckQuestFail Delivery: %s complete>total, skip", GET_NAME(tch));
            break;
        }
    }
    }

    mudlog(LOG_SYSERR, "CheckQuestFail: still inconsistent after 8 passes on %s",
           GET_NAME(tch));
}

int n_bosskill(int vnumber, int achievement_class)
{
    int i, max = 0;

    switch (achievement_class)
    {
        case BOSSKILL_ACHIE:
            max = MAX_BOSS_ACHIE;
            break;

        case CLASS_ACHIE:
            max = MAX_CLASS_ACHIE;
            break;

        default:
            return -1;
            break;
    }

    for(i = 0; i < max; i++)
    {
        if(vnumber == AchievementsList[achievement_class][i].achie_number)
        {
            return i;
        }
    }

    return -1;
}

/*
 *  display = 0     ==> comando 'achievement' senza argomenti
 *  display = 1     ==> comando 'achievement all'
 */
namespace {
bool hasAchievement(struct char_data* ch, int achievement_class, int display)
{
    struct char_data* tch;
    int i;

    tch = ch;

    if(IS_POLY(tch))
        tch = ch->desc->original;

    switch(achievement_class)
    {
        case CLASS_ACHIE:
            for(i = 1; i < MAX_CLASS_ACHIE; i++)
            {
                if(display == 0)
                {
                    if(ch->specials.achievements[CLASS_ACHIE][i] >= AchievementsList[CLASS_ACHIE][i].lvl1_val && AchievementsList[CLASS_ACHIE][i].classe > -1)
                    {
                        return TRUE;
                    }
                }
                else if (display == 1)
                {
                    if(ch->specials.achievements[CLASS_ACHIE][i] > 0 && AchievementsList[CLASS_ACHIE][i].classe > -1)
                    {
                        return TRUE;
                    }
                }
            }
            break;

        case BOSSKILL_ACHIE:
            for(i = 0; i < MAX_BOSS_ACHIE; i++)
            {
                if(display == 0)
                {
                    if(ch->specials.achievements[BOSSKILL_ACHIE][i] >= AchievementsList[BOSSKILL_ACHIE][i].lvl1_val && AchievementsList[BOSSKILL_ACHIE][i].classe > -1)
                    {
                        return TRUE;
                    }
                }
                else if (display == 1)
                {
                    if(ch->specials.achievements[BOSSKILL_ACHIE][i] > 0 && AchievementsList[BOSSKILL_ACHIE][i].classe > -1)
                    {
                        return TRUE;
                    }
                }
            }
            break;

        case RACESLAYER_ACHIE:
            for(i = 0; i < MAX_RACE_ACHIE; i++)
            {
                if(display == 0)
                {
                    if(ch->specials.achievements[RACESLAYER_ACHIE][i] >= AchievementsList[RACESLAYER_ACHIE][i].lvl1_val && AchievementsList[RACESLAYER_ACHIE][i].classe > -1)
                    {
                        return TRUE;
                    }
                }
                else if (display == 1)
                {
                    if(ch->specials.achievements[RACESLAYER_ACHIE][i] > 0 && AchievementsList[RACESLAYER_ACHIE][i].classe > -1)
                    {
                        return TRUE;
                    }
                }
            }
            break;

        case QUEST_ACHIE:
            for(i = 0; i < MAX_QUEST_ACHIE ; i++)
            {
                if(display == 0)
                {
                    if(ch->specials.achievements[QUEST_ACHIE][i] >= AchievementsList[QUEST_ACHIE][i].lvl1_val && AchievementsList[QUEST_ACHIE][i].classe > -1)
                    {
                        return TRUE;
                    }
                }
                else if (display == 1)
                {
                    if(ch->specials.achievements[QUEST_ACHIE][i] > 0 && AchievementsList[QUEST_ACHIE][i].classe > -1)
                    {
                        return TRUE;
                    }
                }
            }
            break;

        case OTHER_ACHIE:
            for(i = 0; i < MAX_OTHER_ACHIE; i++)
            {
                if(display == 0)
                {
                    if(ch->specials.achievements[OTHER_ACHIE][i] >= AchievementsList[OTHER_ACHIE][i].lvl1_val && AchievementsList[OTHER_ACHIE][i].classe > -1)
                    {
                        return TRUE;
                    }
                }
                else if (display == 1)
                {
                    if(ch->specials.achievements[OTHER_ACHIE][i] > 0 && AchievementsList[OTHER_ACHIE][i].classe > -1)
                    {
                        return TRUE;
                    }
                }
            }
            break;

        default:
            return FALSE;
            break;
    }
    return FALSE;
}
} // namespace

namespace {
std::string AchievementNumber(struct char_data* ch, int achievement_type, int achievement_class)
{
    int num = 0, i;
    struct char_data* tch;
    std::string sb;
    LivelloAchie stringa;

    tch = ch;

    if(IS_POLY(tch))
    {
        tch = ch->desc->original;
    }

    // Race Achievements
    for(i = 0; i < MAX_RACE_ACHIE; i++)
    {
        if(AchievementsList[RACESLAYER_ACHIE][i].classe == -1)
        {
            // se l'achievement classe e' -1 viene skippato
            continue;
        }
        else if(HasClass(tch, AchievementsList[RACESLAYER_ACHIE][i].classe) || IS_QUESTMASTER(ch))
        {
            num += 1;
            if(achievement_class == RACESLAYER_ACHIE && achievement_type == AchievementsList[RACESLAYER_ACHIE][i].achie_number)
            {
                // ho trovato l'achievement
                stringa = StringaAchie(tch->specials.achievements[achievement_class][i], i, achievement_class);
                num += stringa.livello;
                boost::format fmt("$c0014Race Achievement $c0015#$c0009%d $c0014'$c0015%s$c0014' $c0009%d$c0015/$c0009%d$c0014.\n\r");
                fmt % num % stringa.stringa % tch->specials.achievements[achievement_class][i] % stringa.valore;
                sb.append(fmt.str().c_str());
                fmt.clear();
                return sb;
            }
            num += AchievementsList[RACESLAYER_ACHIE][i].n_livelli - 1;
        }
        else if(tch->specials.achievements[RACESLAYER_ACHIE][i] > 0)
        {
            num += 1;
            if(achievement_class == RACESLAYER_ACHIE && achievement_type == AchievementsList[RACESLAYER_ACHIE][i].achie_number)
            {
                // ho trovato l'achievement
                stringa = StringaAchie(tch->specials.achievements[achievement_class][i], i, achievement_class);
                num += stringa.livello;
                boost::format fmt("$c0014Race Achievement $c0015#$c0009%d $c0014'$c0015%s$c0014' $c0009%d$c0015/$c0009%d$c0014.\n\r");
                fmt % num % stringa.stringa % tch->specials.achievements[achievement_class][i] % stringa.valore;
                sb.append(fmt.str().c_str());
                fmt.clear();
                return sb;
            }
            num += AchievementsList[RACESLAYER_ACHIE][i].n_livelli - 1;
        }
    }

    // Boss Achievements
    for(i = 0; i < MAX_BOSS_ACHIE; i++)
    {
        if(AchievementsList[BOSSKILL_ACHIE][i].classe == -1)
        {
            // se l'achievement classe e' -1 viene skippato
            continue;
        }
        else if(HasClass(tch, AchievementsList[BOSSKILL_ACHIE][i].classe) || IS_QUESTMASTER(ch))
        {
            num += 1;
            if(achievement_class == BOSSKILL_ACHIE && achievement_type == n_bosskill(AchievementsList[BOSSKILL_ACHIE][i].achie_number, BOSSKILL_ACHIE))
            {
                // ho trovato l'achievement
                stringa = StringaAchie(tch->specials.achievements[achievement_class][i], i, achievement_class);
                num += stringa.livello;
                boost::format fmt("$c0014Boss Achievement $c0015#$c0009%d $c0014'$c0015%s$c0014' $c0009%d$c0015/$c0009%d$c0014.\n\r");
                fmt % num % stringa.stringa % tch->specials.achievements[achievement_class][i] % stringa.valore;
                sb.append(fmt.str().c_str());
                fmt.clear();
                return sb;
            }
            num += AchievementsList[BOSSKILL_ACHIE][i].n_livelli - 1;
        }
        else if(tch->specials.achievements[BOSSKILL_ACHIE][i] > 0)
        {
            num += 1;
            if(achievement_class == BOSSKILL_ACHIE && achievement_type == n_bosskill(AchievementsList[BOSSKILL_ACHIE][i].achie_number, BOSSKILL_ACHIE))
            {
                // ho trovato l'achievement
                stringa = StringaAchie(tch->specials.achievements[achievement_class][i], i, achievement_class);
                num += stringa.livello;
                boost::format fmt("$c0014Boss Achievement $c0015#$c0009%d $c0014'$c0015%s$c0014' $c0009%d$c0015/$c0009%d$c0014.\n\r");
                fmt % num % stringa.stringa % tch->specials.achievements[achievement_class][i] % stringa.valore;
                sb.append(fmt.str().c_str());
                fmt.clear();
                return sb;
            }
            num += AchievementsList[BOSSKILL_ACHIE][i].n_livelli - 1;
        }
    }

    // Class Skill Achievements
    for(i = 1; i < MAX_CLASS_ACHIE; i++)
    {
        if(AchievementsList[CLASS_ACHIE][i].classe == -1)
        {
            // se l'achievement classe e' -1 viene skippato
            continue;
        }
        else if(HasClass(tch, AchievementsList[CLASS_ACHIE][i].classe) || AchievementsList[CLASS_ACHIE][i].classe == 0)
        {
            num += 1;
            if(achievement_class == CLASS_ACHIE && achievement_type == AchievementsList[CLASS_ACHIE][i].achie_number)
            {
                // ho trovato l'achievement
                stringa = StringaAchie(tch->specials.achievements[achievement_class][i], i, achievement_class);
                num += stringa.livello;
                boost::format fmt("$c0014Skill Achievement $c0015#$c0009%d $c0014'$c0015%s$c0014' $c0009%d$c0015/$c0009%d$c0014.\n\r");
                fmt % num % stringa.stringa % tch->specials.achievements[achievement_class][i] % stringa.valore;
                sb.append(fmt.str().c_str());
                fmt.clear();
                return sb;
            }
            num += AchievementsList[CLASS_ACHIE][i].n_livelli - 1;
        }
    }

    // Quest Achievements
    for(i = 0; i < MAX_QUEST_ACHIE; i++)
    {
        if(AchievementsList[QUEST_ACHIE][i].classe == -1)
        {
            // se l'achievement classe e' -1 viene skippato
            continue;
        }
        else if(HasClass(tch, AchievementsList[QUEST_ACHIE][i].classe) || IS_QUESTMASTER(ch))
        {
            num += 1;
            if(achievement_class == QUEST_ACHIE && achievement_type == AchievementsList[QUEST_ACHIE][i].achie_number)
            {
                // ho trovato l'achievement
                stringa = StringaAchie(tch->specials.achievements[achievement_class][i], i, achievement_class);
                num += stringa.livello;
                boost::format fmt("$c0014Quest Achievement $c0015#$c0009%d $c0014'$c0015%s$c0014' $c0009%d$c0015/$c0009%d$c0014.\n\r");
                fmt % num % stringa.stringa % tch->specials.achievements[achievement_class][i] % stringa.valore;
                sb.append(fmt.str().c_str());
                fmt.clear();
                return sb;;
            }
            num += AchievementsList[QUEST_ACHIE][i].n_livelli - 1;
        }
        else if(tch->specials.achievements[QUEST_ACHIE][i] > 0)
        {
            num += 1;
            if(achievement_class == QUEST_ACHIE && achievement_type == AchievementsList[QUEST_ACHIE][i].achie_number)
            {
                // ho trovato l'achievement
                stringa = StringaAchie(tch->specials.achievements[achievement_class][i], i, achievement_class);
                num += stringa.livello;
                boost::format fmt("$c0014Quest Achievement $c0015#$c0009%d $c0014'$c0015%s$c0014' $c0009%d$c0015/$c0009%d$c0014.\n\r");
                fmt % num % stringa.stringa % tch->specials.achievements[achievement_class][i] % stringa.valore;
                sb.append(fmt.str().c_str());
                fmt.clear();
                return sb;
            }
            num += AchievementsList[QUEST_ACHIE][i].n_livelli - 1;
        }
    }

    // Various Achievements
    for(i = 0; i < MAX_OTHER_ACHIE; i++)
    {
        if(AchievementsList[OTHER_ACHIE][i].classe == -1)
        {
            // se l'achievement classe e' -1 viene skippato
            continue;
        }
        else if(HasClass(tch, AchievementsList[OTHER_ACHIE][i].classe) || IS_QUESTMASTER(ch))
        {
            num += 1;
            if(achievement_class == OTHER_ACHIE && achievement_type == AchievementsList[OTHER_ACHIE][i].achie_number)
            {
                // ho trovato l'achievement
                stringa = StringaAchie(tch->specials.achievements[achievement_class][i], i, achievement_class);
                num += stringa.livello;
                boost::format fmt("$c0014Other Achievement $c0015#$c0009%d $c0014'$c0015%s$c0014' $c0009%d$c0015/$c0009%d$c0014.\n\r");
                fmt % num % stringa.stringa % tch->specials.achievements[achievement_class][i] % stringa.valore;
                sb.append(fmt.str().c_str());
                fmt.clear();
                return sb;
            }
            num += AchievementsList[OTHER_ACHIE][i].n_livelli - 1;
        }
        else if(tch->specials.achievements[OTHER_ACHIE][i] > 0)
        {
            num += 1;
            if(achievement_class == OTHER_ACHIE && achievement_type == AchievementsList[OTHER_ACHIE][i].achie_number)
            {
                // ho trovato l'achievement
                stringa = StringaAchie(tch->specials.achievements[achievement_class][i], i, achievement_class);
                num += stringa.livello;
                boost::format fmt("$c0014Other Achievement $c0015#$c0009%d $c0014'$c0015%s$c0014' $c0009%d$c0015/$c0009%d$c0014.\n\r");
                fmt % num % stringa.stringa % tch->specials.achievements[achievement_class][i] % stringa.valore;
                sb.append(fmt.str().c_str());
                fmt.clear();
                return sb;
            }
            num += AchievementsList[OTHER_ACHIE][i].n_livelli - 1;
        }
    }
    return sb;
}
} // namespace

LivelloAchie StringaAchie(int valore, int achievement_type, int achievement_class)
{
    std::string sb;
    int val = 0, livello = 0;

    if(valore < AchievementsList[achievement_class][achievement_type].lvl1_val)
    {
        sb.append(AchievementsList[achievement_class][achievement_type].lvl1);
        val = AchievementsList[achievement_class][achievement_type].lvl1_val;
        livello = 0;
    }
    else if(valore < AchievementsList[achievement_class][achievement_type].lvl2_val)
    {
        sb.append(AchievementsList[achievement_class][achievement_type].lvl2);
        val = AchievementsList[achievement_class][achievement_type].lvl2_val;
        livello = 1;
    }
    else if(valore < AchievementsList[achievement_class][achievement_type].lvl3_val)
    {
        sb.append(AchievementsList[achievement_class][achievement_type].lvl3);
        val = AchievementsList[achievement_class][achievement_type].lvl3_val;
        livello = 2;
    }
    else if(valore < AchievementsList[achievement_class][achievement_type].lvl4_val)
    {
        sb.append(AchievementsList[achievement_class][achievement_type].lvl4);
        val = AchievementsList[achievement_class][achievement_type].lvl4_val;
        livello = 3;
    }
    else if(valore < AchievementsList[achievement_class][achievement_type].lvl5_val)
    {
        sb.append(AchievementsList[achievement_class][achievement_type].lvl5);
        val = AchievementsList[achievement_class][achievement_type].lvl5_val;
        livello = 4;
    }
    else if(valore < AchievementsList[achievement_class][achievement_type].lvl6_val)
    {
        sb.append(AchievementsList[achievement_class][achievement_type].lvl6);
        val = AchievementsList[achievement_class][achievement_type].lvl6_val;
        livello = 5;
    }
    else if(valore < AchievementsList[achievement_class][achievement_type].lvl7_val)
    {
        sb.append(AchievementsList[achievement_class][achievement_type].lvl7);
        val = AchievementsList[achievement_class][achievement_type].lvl7_val;
        livello = 6;
    }
    else if(valore < AchievementsList[achievement_class][achievement_type].lvl8_val)
    {
        sb.append(AchievementsList[achievement_class][achievement_type].lvl8);
        val = AchievementsList[achievement_class][achievement_type].lvl8_val;
        livello = 7;
    }
    else if(valore < AchievementsList[achievement_class][achievement_type].lvl9_val)
    {
        sb.append(AchievementsList[achievement_class][achievement_type].lvl9);
        val = AchievementsList[achievement_class][achievement_type].lvl9_val;
        livello = 8;
    }
    else if(valore < AchievementsList[achievement_class][achievement_type].lvl10_val)
    {
        sb.append(AchievementsList[achievement_class][achievement_type].lvl10);
        val = AchievementsList[achievement_class][achievement_type].lvl10_val;
        livello = 9;
    }
    else
    {
        sb.append(AchievementsList[achievement_class][achievement_type].lvl10);
        val = AchievementsList[achievement_class][achievement_type].lvl10_val;
        livello = 9;
    }

    LivelloAchie stringa =
    {
        sb, livello, val
    };

    return stringa;
}

namespace {
std::string bufferAchie(struct char_data* ch, int achievement_type, int achievement_class, int lvl, int num, bool formato, int check)
{
    std::string sb;
    std::string sb2;
    struct char_data* tch;
    bool spam = FALSE;

    tch = ch;

    if(IS_POLY(tch))
        tch = ch->desc->original;

    if (lvl >= 1)
    {
        if(formato)
        {
            boost::format fmt("$c0009[%s%4d$c0009]%s %-55s $c0011%6d%s/$c0011%-6d%s %-30s\n\r");
            fmt % (ch->specials.achievements[achievement_class][achievement_type] >= AchievementsList[achievement_class][achievement_type].lvl1_val ? "$c0015" : "$c0007") % (num) % (ch->specials.achievements[achievement_class][achievement_type] >= AchievementsList[achievement_class][achievement_type].lvl1_val ? "$c0015" : "$c0007") % AchievementsList[achievement_class][achievement_type].lvl1 % (ch->specials.achievements[achievement_class][achievement_type] >= AchievementsList[achievement_class][achievement_type].lvl1_val ? AchievementsList[achievement_class][achievement_type].lvl1_val : ch->specials.achievements[achievement_class][achievement_type]) % (ch->specials.achievements[achievement_class][achievement_type] >= AchievementsList[achievement_class][achievement_type].lvl1_val ? "$c0015" : "$c0007") % AchievementsList[achievement_class][achievement_type].lvl1_val % (ch->specials.achievements[achievement_class][achievement_type] >= AchievementsList[achievement_class][achievement_type].lvl1_val ? "$c0015" : "$c0007") % (AchievementsList[achievement_class][achievement_type].lvl1_val == 1 ? AchievementsList[achievement_class][achievement_type].achie_string1 : AchievementsList[achievement_class][achievement_type].achie_string2);
            sb.append(fmt.str().c_str());
            fmt.clear();
            if(check == num)
            {
                if(ch->specials.achievements[achievement_class][achievement_type] >= AchievementsList[achievement_class][achievement_type].lvl1_val)
                {
                    boost::format fmt("completato '$c0011%s$c0007' ($c0011%d$c0007 %s)");
                    fmt % AchievementsList[achievement_class][achievement_type].lvl1 % AchievementsList[achievement_class][achievement_type].lvl1_val % (AchievementsList[achievement_class][achievement_type].lvl1_val == 1 ? AchievementsList[achievement_class][achievement_type].achie_string1 : AchievementsList[achievement_class][achievement_type].achie_string2);
                    sb2.append(fmt.str().c_str());
                    fmt.clear();
                }
                else
                {
                    boost::format fmt("manca%s %s $c0011%d$c0007 %s per completare '$c0011%s$c0007'");
                    fmt % ((AchievementsList[achievement_class][achievement_type].lvl1_val - ch->specials.achievements[achievement_class][achievement_type]) == 1 ? "" : "no") % ((AchievementsList[achievement_class][achievement_type].lvl1_val - ch->specials.achievements[achievement_class][achievement_type]) < 20 ? "solo" : "ancora") % (AchievementsList[achievement_class][achievement_type].lvl1_val - ch->specials.achievements[achievement_class][achievement_type]) % ((AchievementsList[achievement_class][achievement_type].lvl1_val - ch->specials.achievements[achievement_class][achievement_type]) == 1 ? AchievementsList[achievement_class][achievement_type].achie_string1 : AchievementsList[achievement_class][achievement_type].achie_string2) % AchievementsList[achievement_class][achievement_type].lvl1;
                    sb2.append(fmt.str().c_str());
                    fmt.clear();
                }
                spam = TRUE;
            }
        }
        else
        {
            boost::format fmt("%s%4d %-55s %6d %-30s\n\r");
            fmt % (num%2 == 0 ? "$c0015" : "$c0007") % (num) % AchievementsList[achievement_class][achievement_type].lvl1 % AchievementsList[achievement_class][achievement_type].lvl1_val % (AchievementsList[achievement_class][achievement_type].lvl1_val == 1 ? AchievementsList[achievement_class][achievement_type].achie_string1 : AchievementsList[achievement_class][achievement_type].achie_string2);
                sb.append(fmt.str().c_str());
            fmt.clear();
        }
    }
    if (lvl >= 2)
    {
        if(formato)
        {
            boost::format fmt("$c0009[%s%4d$c0009]%s %-55s $c0011%6d%s/$c0011%-6d%s %-30s\n\r");
            fmt % (ch->specials.achievements[achievement_class][achievement_type] >= AchievementsList[achievement_class][achievement_type].lvl2_val ? "$c0015" : "$c0007") % (num + 1) % (ch->specials.achievements[achievement_class][achievement_type] >= AchievementsList[achievement_class][achievement_type].lvl2_val ? "$c0015" : "$c0007") % AchievementsList[achievement_class][achievement_type].lvl2 % (ch->specials.achievements[achievement_class][achievement_type] >= AchievementsList[achievement_class][achievement_type].lvl2_val ? AchievementsList[achievement_class][achievement_type].lvl2_val : ch->specials.achievements[achievement_class][achievement_type]) % (ch->specials.achievements[achievement_class][achievement_type] >= AchievementsList[achievement_class][achievement_type].lvl2_val ? "$c0015" : "$c0007") % AchievementsList[achievement_class][achievement_type].lvl2_val % (ch->specials.achievements[achievement_class][achievement_type] >= AchievementsList[achievement_class][achievement_type].lvl2_val ? "$c0015" : "$c0007") % (AchievementsList[achievement_class][achievement_type].lvl2_val == 1 ? AchievementsList[achievement_class][achievement_type].achie_string1 : AchievementsList[achievement_class][achievement_type].achie_string2);
            sb.append(fmt.str().c_str());
            fmt.clear();
            if(check == (num + 1))
            {
                if(ch->specials.achievements[achievement_class][achievement_type] >= AchievementsList[achievement_class][achievement_type].lvl2_val)
                {
                    boost::format fmt("completato '$c0011%s$c0007' ($c0011%d$c0007 %s)");
                    fmt % AchievementsList[achievement_class][achievement_type].lvl2 % AchievementsList[achievement_class][achievement_type].lvl2_val % (AchievementsList[achievement_class][achievement_type].lvl2_val == 1 ? AchievementsList[achievement_class][achievement_type].achie_string1 : AchievementsList[achievement_class][achievement_type].achie_string2);
                    sb2.append(fmt.str().c_str());
                    fmt.clear();
                }
                else
                {
                    boost::format fmt("manca%s %s $c0011%d$c0007 %s per completare '$c0011%s$c0007'");
                    fmt % ((AchievementsList[achievement_class][achievement_type].lvl2_val - ch->specials.achievements[achievement_class][achievement_type]) == 1 ? "" : "no") % ((AchievementsList[achievement_class][achievement_type].lvl2_val - ch->specials.achievements[achievement_class][achievement_type]) < 20 ? "solo" : "ancora") % (AchievementsList[achievement_class][achievement_type].lvl2_val - ch->specials.achievements[achievement_class][achievement_type]) % ((AchievementsList[achievement_class][achievement_type].lvl2_val - ch->specials.achievements[achievement_class][achievement_type]) == 1 ? AchievementsList[achievement_class][achievement_type].achie_string1 : AchievementsList[achievement_class][achievement_type].achie_string2) % AchievementsList[achievement_class][achievement_type].lvl2;
                    sb2.append(fmt.str().c_str());
                    fmt.clear();
                }
                spam = TRUE;
            }
        }
        else
        {
            boost::format fmt("%s%4d %-55s %6d %-30s\n\r");
            fmt % ((num + 1)%2 == 0 ? "$c0015" : "$c0007") % (num + 1) % AchievementsList[achievement_class][achievement_type].lvl2 % AchievementsList[achievement_class][achievement_type].lvl2_val % (AchievementsList[achievement_class][achievement_type].lvl2_val == 1 ? AchievementsList[achievement_class][achievement_type].achie_string1 : AchievementsList[achievement_class][achievement_type].achie_string2);
            sb.append(fmt.str().c_str());
            fmt.clear();
        }
    }
    if (lvl >= 3)
    {
        if(formato)
        {
            boost::format fmt("$c0009[%s%4d$c0009]%s %-55s $c0011%6d%s/$c0011%-6d%s %-30s\n\r");
            fmt % (ch->specials.achievements[achievement_class][achievement_type] >= AchievementsList[achievement_class][achievement_type].lvl3_val ? "$c0015" : "$c0007") % (num + 2) % (ch->specials.achievements[achievement_class][achievement_type] >= AchievementsList[achievement_class][achievement_type].lvl3_val ? "$c0015" : "$c0007") % AchievementsList[achievement_class][achievement_type].lvl3 % (ch->specials.achievements[achievement_class][achievement_type] >= AchievementsList[achievement_class][achievement_type].lvl3_val ? AchievementsList[achievement_class][achievement_type].lvl3_val : ch->specials.achievements[achievement_class][achievement_type]) % (ch->specials.achievements[achievement_class][achievement_type] >= AchievementsList[achievement_class][achievement_type].lvl3_val ? "$c0015" : "$c0007") % AchievementsList[achievement_class][achievement_type].lvl3_val % (ch->specials.achievements[achievement_class][achievement_type] >= AchievementsList[achievement_class][achievement_type].lvl3_val ? "$c0015" : "$c0007") % (AchievementsList[achievement_class][achievement_type].lvl3_val == 1 ? AchievementsList[achievement_class][achievement_type].achie_string1 : AchievementsList[achievement_class][achievement_type].achie_string2);
            sb.append(fmt.str().c_str());
            fmt.clear();
            if(check == (num + 2))
            {
                if(ch->specials.achievements[achievement_class][achievement_type] >= AchievementsList[achievement_class][achievement_type].lvl3_val)
                {
                    boost::format fmt("completato '$c0011%s$c0007' ($c0011%d$c0007 %s)");
                    fmt % AchievementsList[achievement_class][achievement_type].lvl3 % AchievementsList[achievement_class][achievement_type].lvl3_val % (AchievementsList[achievement_class][achievement_type].lvl3_val == 1 ? AchievementsList[achievement_class][achievement_type].achie_string1 : AchievementsList[achievement_class][achievement_type].achie_string2);
                    sb2.append(fmt.str().c_str());
                    fmt.clear();
                }
                else
                {
                    boost::format fmt("manca%s %s $c0011%d$c0007 %s per completare '$c0011%s$c0007'");
                    fmt % ((AchievementsList[achievement_class][achievement_type].lvl3_val - ch->specials.achievements[achievement_class][achievement_type]) == 1 ? "" : "no") % ((AchievementsList[achievement_class][achievement_type].lvl3_val - ch->specials.achievements[achievement_class][achievement_type]) < 20 ? "solo" : "ancora") % (AchievementsList[achievement_class][achievement_type].lvl3_val - ch->specials.achievements[achievement_class][achievement_type]) % ((AchievementsList[achievement_class][achievement_type].lvl3_val - ch->specials.achievements[achievement_class][achievement_type]) == 1 ? AchievementsList[achievement_class][achievement_type].achie_string1 : AchievementsList[achievement_class][achievement_type].achie_string2) % AchievementsList[achievement_class][achievement_type].lvl3;
                    sb2.append(fmt.str().c_str());
                    fmt.clear();
                }
                spam = TRUE;
            }
        }
        else
        {
            boost::format fmt("%s%4d %-55s %6d %-30s\n\r");
            fmt % ((num + 2)%2 == 0 ? "$c0015" : "$c0007") % (num + 2) % AchievementsList[achievement_class][achievement_type].lvl3 % AchievementsList[achievement_class][achievement_type].lvl3_val % (AchievementsList[achievement_class][achievement_type].lvl3_val == 1 ? AchievementsList[achievement_class][achievement_type].achie_string1 : AchievementsList[achievement_class][achievement_type].achie_string2);
            sb.append(fmt.str().c_str());
            fmt.clear();
        }
    }
    if (lvl >= 4)
    {
        if(formato)
        {
            boost::format fmt("$c0009[%s%4d$c0009]%s %-55s $c0011%6d%s/$c0011%-6d%s %-30s\n\r");
            fmt % (ch->specials.achievements[achievement_class][achievement_type] >= AchievementsList[achievement_class][achievement_type].lvl4_val ? "$c0015" : "$c0007") % (num + 3) % (ch->specials.achievements[achievement_class][achievement_type] >= AchievementsList[achievement_class][achievement_type].lvl4_val ? "$c0015" : "$c0007") % AchievementsList[achievement_class][achievement_type].lvl4 % (ch->specials.achievements[achievement_class][achievement_type] >= AchievementsList[achievement_class][achievement_type].lvl4_val ? AchievementsList[achievement_class][achievement_type].lvl4_val : ch->specials.achievements[achievement_class][achievement_type]) % (ch->specials.achievements[achievement_class][achievement_type] >= AchievementsList[achievement_class][achievement_type].lvl4_val ? "$c0015" : "$c0007") % AchievementsList[achievement_class][achievement_type].lvl4_val % (ch->specials.achievements[achievement_class][achievement_type] >= AchievementsList[achievement_class][achievement_type].lvl4_val ? "$c0015" : "$c0007") % (AchievementsList[achievement_class][achievement_type].lvl4_val == 1 ? AchievementsList[achievement_class][achievement_type].achie_string1 : AchievementsList[achievement_class][achievement_type].achie_string2);
            sb.append(fmt.str().c_str());
            fmt.clear();
            if(check == (num + 3))
            {
                if(ch->specials.achievements[achievement_class][achievement_type] >= AchievementsList[achievement_class][achievement_type].lvl4_val)
                {
                    boost::format fmt("completato '$c0011%s$c0007' ($c0011%d$c0007 %s)");
                    fmt % AchievementsList[achievement_class][achievement_type].lvl4 % AchievementsList[achievement_class][achievement_type].lvl4_val % (AchievementsList[achievement_class][achievement_type].lvl4_val == 1 ? AchievementsList[achievement_class][achievement_type].achie_string1 : AchievementsList[achievement_class][achievement_type].achie_string2);
                    sb2.append(fmt.str().c_str());
            fmt.clear();
                }
                else
                {
                    boost::format fmt("manca%s %s $c0011%d$c0007 %s per completare '$c0011%s$c0007'");
                    fmt % ((AchievementsList[achievement_class][achievement_type].lvl4_val - ch->specials.achievements[achievement_class][achievement_type]) == 1 ? "" : "no") % ((AchievementsList[achievement_class][achievement_type].lvl4_val - ch->specials.achievements[achievement_class][achievement_type]) < 20 ? "solo" : "ancora") % (AchievementsList[achievement_class][achievement_type].lvl4_val - ch->specials.achievements[achievement_class][achievement_type]) % ((AchievementsList[achievement_class][achievement_type].lvl4_val - ch->specials.achievements[achievement_class][achievement_type]) == 1 ? AchievementsList[achievement_class][achievement_type].achie_string1 : AchievementsList[achievement_class][achievement_type].achie_string2) % AchievementsList[achievement_class][achievement_type].lvl4;
                    sb2.append(fmt.str().c_str());
                    fmt.clear();
                }
                spam = TRUE;
            }
        }
        else
        {
            boost::format fmt("%s%4d %-55s %6d %-30s\n\r");
            fmt % ((num + 3)%2 == 0 ? "$c0015" : "$c0007") % (num + 3) % AchievementsList[achievement_class][achievement_type].lvl4 % AchievementsList[achievement_class][achievement_type].lvl4_val % (AchievementsList[achievement_class][achievement_type].lvl4_val == 1 ? AchievementsList[achievement_class][achievement_type].achie_string1 : AchievementsList[achievement_class][achievement_type].achie_string2);
            sb.append(fmt.str().c_str());
            fmt.clear();
        }
    }
    if (lvl >= 5)
    {
        if(formato)
        {
            boost::format fmt("$c0009[%s%4d$c0009]%s %-55s $c0011%6d%s/$c0011%-6d%s %-30s\n\r");
            fmt % (ch->specials.achievements[achievement_class][achievement_type] >= AchievementsList[achievement_class][achievement_type].lvl5_val ? "$c0015" : "$c0007") % (num + 4) % (ch->specials.achievements[achievement_class][achievement_type] >= AchievementsList[achievement_class][achievement_type].lvl5_val ? "$c0015" : "$c0007") % AchievementsList[achievement_class][achievement_type].lvl5 % (ch->specials.achievements[achievement_class][achievement_type] >= AchievementsList[achievement_class][achievement_type].lvl5_val ? AchievementsList[achievement_class][achievement_type].lvl5_val : ch->specials.achievements[achievement_class][achievement_type]) % (ch->specials.achievements[achievement_class][achievement_type] >= AchievementsList[achievement_class][achievement_type].lvl5_val ? "$c0015" : "$c0007") % AchievementsList[achievement_class][achievement_type].lvl5_val % (ch->specials.achievements[achievement_class][achievement_type] >= AchievementsList[achievement_class][achievement_type].lvl5_val ? "$c0015" : "$c0007") % (AchievementsList[achievement_class][achievement_type].lvl5_val == 1 ? AchievementsList[achievement_class][achievement_type].achie_string1 : AchievementsList[achievement_class][achievement_type].achie_string2);
            sb.append(fmt.str().c_str());
            fmt.clear();
            if(check == (num + 4))
            {
                if(ch->specials.achievements[achievement_class][achievement_type] >= AchievementsList[achievement_class][achievement_type].lvl5_val)
                {
                    boost::format fmt("completato '$c0011%s$c0007' ($c0011%d$c0007 %s)");
                    fmt % AchievementsList[achievement_class][achievement_type].lvl5 % AchievementsList[achievement_class][achievement_type].lvl5_val % (AchievementsList[achievement_class][achievement_type].lvl5_val == 1 ? AchievementsList[achievement_class][achievement_type].achie_string1 : AchievementsList[achievement_class][achievement_type].achie_string2);
                    sb2.append(fmt.str().c_str());
                    fmt.clear();
                }
                else
                {
                    boost::format fmt("manca%s %s $c0011%d$c0007 %s per completare '$c0011%s$c0007'");
                    fmt % ((AchievementsList[achievement_class][achievement_type].lvl5_val - ch->specials.achievements[achievement_class][achievement_type]) == 1 ? "" : "no") % ((AchievementsList[achievement_class][achievement_type].lvl5_val - ch->specials.achievements[achievement_class][achievement_type]) < 20 ? "solo" : "ancora") % (AchievementsList[achievement_class][achievement_type].lvl5_val - ch->specials.achievements[achievement_class][achievement_type]) % ((AchievementsList[achievement_class][achievement_type].lvl5_val - ch->specials.achievements[achievement_class][achievement_type]) == 1 ? AchievementsList[achievement_class][achievement_type].achie_string1 : AchievementsList[achievement_class][achievement_type].achie_string2) % AchievementsList[achievement_class][achievement_type].lvl5;
                    sb2.append(fmt.str().c_str());
                    fmt.clear();
                }
                spam = TRUE;
            }
        }
        else
        {
            boost::format fmt("%s%4d %-55s %6d %-30s\n\r");
            fmt % ((num + 4)%2 == 0 ? "$c0015" : "$c0007") % (num + 4) % AchievementsList[achievement_class][achievement_type].lvl5 % AchievementsList[achievement_class][achievement_type].lvl5_val % (AchievementsList[achievement_class][achievement_type].lvl5_val == 1 ? AchievementsList[achievement_class][achievement_type].achie_string1 : AchievementsList[achievement_class][achievement_type].achie_string2);
            sb.append(fmt.str().c_str());
            fmt.clear();
        }
    }
    if (lvl >= 6)
    {
        if(formato)
        {
            boost::format fmt("$c0009[%s%4d$c0009]%s %-55s $c0011%6d%s/$c0011%-6d%s %-30s\n\r");
            fmt % (ch->specials.achievements[achievement_class][achievement_type] >= AchievementsList[achievement_class][achievement_type].lvl6_val ? "$c0015" : "$c0007") % (num + 5) % (ch->specials.achievements[achievement_class][achievement_type] >= AchievementsList[achievement_class][achievement_type].lvl6_val ? "$c0015" : "$c0007") % AchievementsList[achievement_class][achievement_type].lvl6 % (ch->specials.achievements[achievement_class][achievement_type] >= AchievementsList[achievement_class][achievement_type].lvl6_val ? AchievementsList[achievement_class][achievement_type].lvl6_val : ch->specials.achievements[achievement_class][achievement_type]) % (ch->specials.achievements[achievement_class][achievement_type] >= AchievementsList[achievement_class][achievement_type].lvl6_val ? "$c0015" : "$c0007") % AchievementsList[achievement_class][achievement_type].lvl6_val % (ch->specials.achievements[achievement_class][achievement_type] >= AchievementsList[achievement_class][achievement_type].lvl6_val ? "$c0015" : "$c0007") % (AchievementsList[achievement_class][achievement_type].lvl6_val == 1 ? AchievementsList[achievement_class][achievement_type].achie_string1 : AchievementsList[achievement_class][achievement_type].achie_string2);
            sb.append(fmt.str().c_str());
            fmt.clear();
            if(check == (num + 5))
            {
                if(ch->specials.achievements[achievement_class][achievement_type] >= AchievementsList[achievement_class][achievement_type].lvl6_val)
                {
                    boost::format fmt("completato '$c0011%s$c0007' ($c0011%d$c0007 %s)");
                    fmt % AchievementsList[achievement_class][achievement_type].lvl6 % AchievementsList[achievement_class][achievement_type].lvl6_val % (AchievementsList[achievement_class][achievement_type].lvl6_val == 1 ? AchievementsList[achievement_class][achievement_type].achie_string1 : AchievementsList[achievement_class][achievement_type].achie_string2);
                    sb2.append(fmt.str().c_str());
                    fmt.clear();
                }
                else
                {
                    boost::format fmt("manca%s %s $c0011%d$c0007 %s per completare '$c0011%s$c0007'");
                    fmt % ((AchievementsList[achievement_class][achievement_type].lvl6_val - ch->specials.achievements[achievement_class][achievement_type]) == 1 ? "" : "no") % ((AchievementsList[achievement_class][achievement_type].lvl6_val - ch->specials.achievements[achievement_class][achievement_type]) < 20 ? "solo" : "ancora") % (AchievementsList[achievement_class][achievement_type].lvl6_val - ch->specials.achievements[achievement_class][achievement_type]) % ((AchievementsList[achievement_class][achievement_type].lvl6_val - ch->specials.achievements[achievement_class][achievement_type]) == 1 ? AchievementsList[achievement_class][achievement_type].achie_string1 : AchievementsList[achievement_class][achievement_type].achie_string2) % AchievementsList[achievement_class][achievement_type].lvl6;
                    sb2.append(fmt.str().c_str());
                    fmt.clear();
                }
                spam = TRUE;
            }
        }
        else
        {
            boost::format fmt("%s%4d %-55s %6d %-30s\n\r");
            fmt % ((num + 5)%2 == 0 ? "$c0015" : "$c0007") % (num + 5) % AchievementsList[achievement_class][achievement_type].lvl6 % AchievementsList[achievement_class][achievement_type].lvl6_val % (AchievementsList[achievement_class][achievement_type].lvl6_val == 1 ? AchievementsList[achievement_class][achievement_type].achie_string1 : AchievementsList[achievement_class][achievement_type].achie_string2);
            sb.append(fmt.str().c_str());
            fmt.clear();
        }
    }
    if (lvl >= 7)
    {
        if(formato)
        {
            boost::format fmt("$c0009[%s%4d$c0009]%s %-55s $c0011%6d%s/$c0011%-6d%s %-30s\n\r");
            fmt % (ch->specials.achievements[achievement_class][achievement_type] >= AchievementsList[achievement_class][achievement_type].lvl7_val ? "$c0015" : "$c0007") % (num + 6) % (ch->specials.achievements[achievement_class][achievement_type] >= AchievementsList[achievement_class][achievement_type].lvl7_val ? "$c0015" : "$c0007") % AchievementsList[achievement_class][achievement_type].lvl7 % (ch->specials.achievements[achievement_class][achievement_type] >= AchievementsList[achievement_class][achievement_type].lvl7_val ? AchievementsList[achievement_class][achievement_type].lvl7_val : ch->specials.achievements[achievement_class][achievement_type]) % (ch->specials.achievements[achievement_class][achievement_type] >= AchievementsList[achievement_class][achievement_type].lvl7_val ? "$c0015" : "$c0007") % AchievementsList[achievement_class][achievement_type].lvl7_val % (ch->specials.achievements[achievement_class][achievement_type] >= AchievementsList[achievement_class][achievement_type].lvl7_val ? "$c0015" : "$c0007") % (AchievementsList[achievement_class][achievement_type].lvl7_val == 1 ? AchievementsList[achievement_class][achievement_type].achie_string1 : AchievementsList[achievement_class][achievement_type].achie_string2);
            sb.append(fmt.str().c_str());
            fmt.clear();
            if(check == (num + 6))
            {
                if(ch->specials.achievements[achievement_class][achievement_type] >= AchievementsList[achievement_class][achievement_type].lvl7_val)
                {
                    boost::format fmt("completato '$c0011%s$c0007' ($c0011%d$c0007 %s)");
                    fmt % AchievementsList[achievement_class][achievement_type].lvl7 % AchievementsList[achievement_class][achievement_type].lvl7_val % (AchievementsList[achievement_class][achievement_type].lvl7_val == 1 ? AchievementsList[achievement_class][achievement_type].achie_string1 : AchievementsList[achievement_class][achievement_type].achie_string2);
                    sb2.append(fmt.str().c_str());
                    fmt.clear();
                }
                else
                {
                    boost::format fmt("manca%s %s $c0011%d$c0007 %s per completare '$c0011%s$c0007'");
                    fmt % ((AchievementsList[achievement_class][achievement_type].lvl7_val - ch->specials.achievements[achievement_class][achievement_type]) == 1 ? "" : "no") % ((AchievementsList[achievement_class][achievement_type].lvl7_val - ch->specials.achievements[achievement_class][achievement_type]) < 20 ? "solo" : "ancora") % (AchievementsList[achievement_class][achievement_type].lvl7_val - ch->specials.achievements[achievement_class][achievement_type]) % ((AchievementsList[achievement_class][achievement_type].lvl7_val - ch->specials.achievements[achievement_class][achievement_type]) == 1 ? AchievementsList[achievement_class][achievement_type].achie_string1 : AchievementsList[achievement_class][achievement_type].achie_string2) % AchievementsList[achievement_class][achievement_type].lvl7;
                    sb2.append(fmt.str().c_str());
                    fmt.clear();
                }
                spam = TRUE;
            }
        }
        else
        {
            boost::format fmt("%s%4d %-55s %6d %-30s\n\r");
            fmt % ((num + 6)%2 == 0 ? "$c0015" : "$c0007") % (num + 6) % AchievementsList[achievement_class][achievement_type].lvl7 % AchievementsList[achievement_class][achievement_type].lvl7_val % (AchievementsList[achievement_class][achievement_type].lvl7_val == 1 ? AchievementsList[achievement_class][achievement_type].achie_string1 : AchievementsList[achievement_class][achievement_type].achie_string2);
            sb.append(fmt.str().c_str());
            fmt.clear();
        }
    }
    if (lvl >= 8)
    {
        if(formato)
        {
            boost::format fmt("$c0009[%s%4d$c0009]%s %-55s $c0011%6d%s/$c0011%-6d%s %-30s\n\r");
            fmt % (ch->specials.achievements[achievement_class][achievement_type] >= AchievementsList[achievement_class][achievement_type].lvl8_val ? "$c0015" : "$c0007") % (num + 7) % (ch->specials.achievements[achievement_class][achievement_type] >= AchievementsList[achievement_class][achievement_type].lvl8_val ? "$c0015" : "$c0007") % AchievementsList[achievement_class][achievement_type].lvl8 % (ch->specials.achievements[achievement_class][achievement_type] >= AchievementsList[achievement_class][achievement_type].lvl8_val ? AchievementsList[achievement_class][achievement_type].lvl8_val : ch->specials.achievements[achievement_class][achievement_type]) % (ch->specials.achievements[achievement_class][achievement_type] >= AchievementsList[achievement_class][achievement_type].lvl8_val ? "$c0015" : "$c0007") % AchievementsList[achievement_class][achievement_type].lvl8_val % (ch->specials.achievements[achievement_class][achievement_type] >= AchievementsList[achievement_class][achievement_type].lvl8_val ? "$c0015" : "$c0007") % (AchievementsList[achievement_class][achievement_type].lvl8_val == 1 ? AchievementsList[achievement_class][achievement_type].achie_string1 : AchievementsList[achievement_class][achievement_type].achie_string2);
            sb.append(fmt.str().c_str());
            fmt.clear();
            if(check == (num + 7))
            {
                if(ch->specials.achievements[achievement_class][achievement_type] >= AchievementsList[achievement_class][achievement_type].lvl8_val)
                {
                    boost::format fmt("completato '$c0011%s$c0007' ($c0011%d$c0007 %s)");
                    fmt % AchievementsList[achievement_class][achievement_type].lvl8 % AchievementsList[achievement_class][achievement_type].lvl8_val % (AchievementsList[achievement_class][achievement_type].lvl8_val == 1 ? AchievementsList[achievement_class][achievement_type].achie_string1 : AchievementsList[achievement_class][achievement_type].achie_string2);
                    sb2.append(fmt.str().c_str());
                    fmt.clear();
                }
                else
                {
                    boost::format fmt("manca%s %s $c0011%d$c0007 %s per completare '$c0011%s$c0007'");
                    fmt % ((AchievementsList[achievement_class][achievement_type].lvl8_val - ch->specials.achievements[achievement_class][achievement_type]) == 1 ? "" : "no") % ((AchievementsList[achievement_class][achievement_type].lvl8_val - ch->specials.achievements[achievement_class][achievement_type]) < 20 ? "solo" : "ancora") % (AchievementsList[achievement_class][achievement_type].lvl8_val - ch->specials.achievements[achievement_class][achievement_type]) % ((AchievementsList[achievement_class][achievement_type].lvl8_val - ch->specials.achievements[achievement_class][achievement_type]) == 1 ? AchievementsList[achievement_class][achievement_type].achie_string1 : AchievementsList[achievement_class][achievement_type].achie_string2) % AchievementsList[achievement_class][achievement_type].lvl8;
                    sb2.append(fmt.str().c_str());
                    fmt.clear();
                }
                spam = TRUE;
            }
        }
        else
        {
            boost::format fmt("%s%4d %-55s %6d %-30s\n\r");
            fmt % ((num + 7)%2 == 0 ? "$c0015" : "$c0007") % (num + 7) % AchievementsList[achievement_class][achievement_type].lvl8 % AchievementsList[achievement_class][achievement_type].lvl8_val % (AchievementsList[achievement_class][achievement_type].lvl8_val == 1 ? AchievementsList[achievement_class][achievement_type].achie_string1 : AchievementsList[achievement_class][achievement_type].achie_string2);
            sb.append(fmt.str().c_str());
            fmt.clear();
        }
    }
    if (lvl >= 9)
    {
        if(formato)
        {
            boost::format fmt("$c0009[%s%4d$c0009]%s %-55s $c0011%6d%s/$c0011%-6d%s %-30s\n\r");
            fmt % (ch->specials.achievements[achievement_class][achievement_type] >= AchievementsList[achievement_class][achievement_type].lvl9_val ? "$c0015" : "$c0007") % (num + 8) % (ch->specials.achievements[achievement_class][achievement_type] >= AchievementsList[achievement_class][achievement_type].lvl9_val ? "$c0015" : "$c0007") % AchievementsList[achievement_class][achievement_type].lvl9 % (ch->specials.achievements[achievement_class][achievement_type] >= AchievementsList[achievement_class][achievement_type].lvl9_val ? AchievementsList[achievement_class][achievement_type].lvl9_val : ch->specials.achievements[achievement_class][achievement_type]) % (ch->specials.achievements[achievement_class][achievement_type] >= AchievementsList[achievement_class][achievement_type].lvl9_val ? "$c0015" : "$c0007") % AchievementsList[achievement_class][achievement_type].lvl9_val % (ch->specials.achievements[achievement_class][achievement_type] >= AchievementsList[achievement_class][achievement_type].lvl9_val ? "$c0015" : "$c0007") % (AchievementsList[achievement_class][achievement_type].lvl9_val == 1 ? AchievementsList[achievement_class][achievement_type].achie_string1 : AchievementsList[achievement_class][achievement_type].achie_string2);
            sb.append(fmt.str().c_str());
            fmt.clear();
            if(check == (num + 8))
            {
                if(ch->specials.achievements[achievement_class][achievement_type] >= AchievementsList[achievement_class][achievement_type].lvl9_val)
                {
                    boost::format fmt("completato '$c0011%s$c0007' ($c0011%d$c0007 %s)");
                    fmt % AchievementsList[achievement_class][achievement_type].lvl9 % AchievementsList[achievement_class][achievement_type].lvl9_val % (AchievementsList[achievement_class][achievement_type].lvl9_val == 1 ? AchievementsList[achievement_class][achievement_type].achie_string1 : AchievementsList[achievement_class][achievement_type].achie_string2);
                    sb2.append(fmt.str().c_str());
                    fmt.clear();
                }
                else
                {
                    boost::format fmt("manca%s %s $c0011%d$c0007 %s per completare '$c0011%s$c0007'");
                    fmt % ((AchievementsList[achievement_class][achievement_type].lvl9_val - ch->specials.achievements[achievement_class][achievement_type]) == 1 ? "" : "no") % ((AchievementsList[achievement_class][achievement_type].lvl9_val - ch->specials.achievements[achievement_class][achievement_type]) < 20 ? "solo" : "ancora") % (AchievementsList[achievement_class][achievement_type].lvl9_val - ch->specials.achievements[achievement_class][achievement_type]) % ((AchievementsList[achievement_class][achievement_type].lvl9_val - ch->specials.achievements[achievement_class][achievement_type]) == 1 ? AchievementsList[achievement_class][achievement_type].achie_string1 : AchievementsList[achievement_class][achievement_type].achie_string2) % AchievementsList[achievement_class][achievement_type].lvl9;
                    sb2.append(fmt.str().c_str());
                    fmt.clear();
                }
                spam = TRUE;
            }
        }
        else
        {
            boost::format fmt("%s%4d %-55s %6d %-30s\n\r");
            fmt % ((num + 8)%2 == 0 ? "$c0015" : "$c0007") % (num + 8) % AchievementsList[achievement_class][achievement_type].lvl9 % AchievementsList[achievement_class][achievement_type].lvl9_val % (AchievementsList[achievement_class][achievement_type].lvl9_val == 1 ? AchievementsList[achievement_class][achievement_type].achie_string1 : AchievementsList[achievement_class][achievement_type].achie_string2);
            sb.append(fmt.str().c_str());
            fmt.clear();
        }
    }
    if (lvl >= 10)
    {
        if(formato)
        {
            boost::format fmt("$c0009[%s%4d$c0009]%s %-55s $c0011%6d%s/$c0011%-6d%s %-30s\n\r");
            fmt % (ch->specials.achievements[achievement_class][achievement_type] >= AchievementsList[achievement_class][achievement_type].lvl10_val ? "$c0015" : "$c0007") % (num + 9) % (ch->specials.achievements[achievement_class][achievement_type] >= AchievementsList[achievement_class][achievement_type].lvl10_val ? "$c0015" : "$c0007") % AchievementsList[achievement_class][achievement_type].lvl10 % (ch->specials.achievements[achievement_class][achievement_type] >= AchievementsList[achievement_class][achievement_type].lvl10_val ? AchievementsList[achievement_class][achievement_type].lvl10_val : ch->specials.achievements[achievement_class][achievement_type]) % (ch->specials.achievements[achievement_class][achievement_type] >= AchievementsList[achievement_class][achievement_type].lvl10_val ? "$c0015" : "$c0007") % AchievementsList[achievement_class][achievement_type].lvl10_val % (ch->specials.achievements[achievement_class][achievement_type] >= AchievementsList[achievement_class][achievement_type].lvl10_val ? "$c0015" : "$c0007") % (AchievementsList[achievement_class][achievement_type].lvl10_val == 1 ? AchievementsList[achievement_class][achievement_type].achie_string1 : AchievementsList[achievement_class][achievement_type].achie_string2);
            sb.append(fmt.str().c_str());
            fmt.clear();
            if(check == (num + 9))
            {
                if(ch->specials.achievements[achievement_class][achievement_type] >= AchievementsList[achievement_class][achievement_type].lvl10_val)
                {
                    boost::format fmt("completato '$c0011%s$c0007' ($c0011%d$c0007 %s)");
                    fmt % AchievementsList[achievement_class][achievement_type].lvl10 % AchievementsList[achievement_class][achievement_type].lvl10_val % (AchievementsList[achievement_class][achievement_type].lvl10_val == 1 ? AchievementsList[achievement_class][achievement_type].achie_string1 : AchievementsList[achievement_class][achievement_type].achie_string2);
                    sb2.append(fmt.str().c_str());
                    fmt.clear();
                }
                else
                {
                    boost::format fmt("manca%s %s $c0011%d$c0007 %s per completare '$c0011%s$c0007'");
                    fmt % ((AchievementsList[achievement_class][achievement_type].lvl10_val - ch->specials.achievements[achievement_class][achievement_type]) == 1 ? "" : "no") % ((AchievementsList[achievement_class][achievement_type].lvl10_val - ch->specials.achievements[achievement_class][achievement_type]) < 20 ? "solo" : "ancora") % (AchievementsList[achievement_class][achievement_type].lvl10_val - ch->specials.achievements[achievement_class][achievement_type]) % ((AchievementsList[achievement_class][achievement_type].lvl10_val - ch->specials.achievements[achievement_class][achievement_type]) == 1 ? AchievementsList[achievement_class][achievement_type].achie_string1 : AchievementsList[achievement_class][achievement_type].achie_string2) % AchievementsList[achievement_class][achievement_type].lvl10;
                    sb2.append(fmt.str().c_str());
                    fmt.clear();
                }
                spam = TRUE;
            }
        }
        else
        {
            boost::format fmt("%s%4d %-55s %6d %-30s\n\r");
            fmt % ((num + 9)%2 == 0 ? "$c0015" : "$c0007") % (num + 9) % AchievementsList[achievement_class][achievement_type].lvl10 % AchievementsList[achievement_class][achievement_type].lvl10_val % (AchievementsList[achievement_class][achievement_type].lvl10_val == 1 ? AchievementsList[achievement_class][achievement_type].achie_string1 : AchievementsList[achievement_class][achievement_type].achie_string2);
            sb.append(fmt.str().c_str());
            fmt.clear();
        }
    }


    if(spam)
        sb = sb2;

    return sb;
}
} // namespace

int race_achievement(int race)
{
    int razza = -1;

    switch(race)
    {
        case RACE_INSECT:
        case RACE_ARACHNID:
            razza = GROUP_INSECTOID;
            break;

        case RACE_DINOSAUR:
        case RACE_FISH:
        case RACE_BIRD:
        case RACE_PREDATOR:
        case RACE_PARASITE:
        case RACE_SNAKE:
        case RACE_HERBIV:
        case RACE_HORSE:
        case RACE_PRIMATE:
        case RACE_ROO:
            razza = GROUP_ANIMAL;
            break;

        case RACE_TREE:
        case RACE_VEGGIE:
        case RACE_VEGMAN:
            razza = GROUP_VEGGIE;
            break;

        case RACE_SPECIAL:
        case RACE_LYCANTH:
        case RACE_SLIME:
        case RACE_ENFAN:
        case RACE_SKEXIE:
        case RACE_SMURF:
        case RACE_PATRYN:
        case RACE_LABRAT:
        case RACE_DRAAGDIM:
            razza = GROUP_SPECIALS;
            break;

        case RACE_GOLEM:
            razza = RACE_GOLEM;
            break;

        case RACE_DARK_DWARF:
        case RACE_DEEP_GNOME:
        case RACE_DARK_ELF:
            razza = GROUP_DARKRACES;
            break;

        case RACE_DRAGON:
        case RACE_DRAGON_RED:
        case RACE_DRAGON_BLACK:
        case RACE_DRAGON_GREEN:
        case RACE_DRAGON_WHITE:
        case RACE_DRAGON_BLUE:
        case RACE_DRAGON_SILVER:
        case RACE_DRAGON_GOLD:
        case RACE_DRAGON_BRONZE:
        case RACE_DRAGON_COPPER:
        case RACE_DRAGON_BRASS:
            razza = GROUP_DRAKES;
            break;

        case RACE_GIANT:
        case RACE_GIANT_HILL:
        case RACE_GIANT_FROST:
        case RACE_GIANT_FIRE:
        case RACE_GIANT_CLOUD:
        case RACE_GIANT_STORM:
        case RACE_GIANT_STONE:
        case RACE_TYTAN:
            razza = GROUP_GIANTS;
            break;

        case RACE_HALF_ELVEN:
        case RACE_HALF_OGRE:
        case RACE_HALF_ORC:
        case RACE_HALF_GIANT:
        case RACE_HALFBREED:
            razza = GROUP_RACEHALFBREED;
            break;

        case RACE_PLANAR:
        case RACE_ASTRAL:
        case RACE_ELEMENT:
        case RACE_MFLAYER:
        case RACE_SARTAN:
            razza = GROUP_PLANAR;
            break;

        case RACE_UNDEAD:
        case RACE_UNDEAD_VAMPIRE:
        case RACE_UNDEAD_LICH:
        case RACE_UNDEAD_WIGHT:
        case RACE_UNDEAD_GHAST:
        case RACE_UNDEAD_SPECTRE:
        case RACE_UNDEAD_ZOMBIE:
        case RACE_UNDEAD_SKELETON:
        case RACE_UNDEAD_GHOUL:
        case RACE_GHOST:
            razza = GROUP_UNDEAD;
            break;

        case RACE_ORC:
        case RACE_GOBLIN:
        case RACE_TROLL:
        case RACE_REPTILE:
        case RACE_LIZARDMAN:
        case RACE_GNOLL:
            razza = GROUP_GREENSKIN;
            break;

        case RACE_HUMAN:
        case RACE_ELVEN:
        case RACE_DWARF:
        case RACE_HALFLING:
        case RACE_GNOME:
        case RACE_GOLD_ELF:
        case RACE_WILD_ELF:
        case RACE_SEA_ELF:
        case RACE_TROGMAN:
            razza = GROUP_HUMANOID;
            break;

        case RACE_DEVIL:
            razza = RACE_DEVIL;
            break;

        case RACE_DEMON:
            razza = RACE_DEMON;
            break;

        case RACE_GOD:
            razza = RACE_GOD;
            break;

        default:
            mudlog(LOG_CHECK, "Wrong race found in CheckAchie");
            break;
    }

    return razza;
}

namespace {
/** Premio Mercy System: biglietto della quest con descrizione casuale. */
void RewardQAchie(char_data* ch, int quest_number) {
	static constexpr std::array<std::string_view, 9> rand_desc{
		"un buono per ricevere un premio di %s",
		"un piccolo biglietto con su scritto '$c5009%s'$c0007",
		"la fortuna ti ha baciato ed a %s hai raccattato",
		"$c0011un bel %sstale d'oro$c0007",
		"il biglietto della lotteria di %s",
		"un buono per %s gentilmente offerto da LadyOfPain",
		"un buono per un NukeFouler o per un premio di %s",
		"$c0009un buono marchiato dal simbolo di %s$c0007",
		"$c0014il premio per aver nerdato a %s$c0007"
	};

	obj_data* obj = read_object(real_object(QUEST_REWARD), REAL);
	if(obj == nullptr) {
		mudlog(LOG_CHECK, "RewardQAchie: quest reward object %d missing", QUEST_REWARD);
		return;
	}

	const std::string mercy_name = QuestNumber[quest_number].mercy_name;

	if(obj->short_description) {
		free(obj->short_description);
		std::string desc(rand_desc[number(0, static_cast<int>(rand_desc.size()) - 1)]);
		const std::size_t pos = desc.find("%s");
		if(pos != std::string::npos) {
			desc.replace(pos, 2, mercy_name);
		}
		obj->short_description = strdup(desc.c_str());
	}

	if(obj->name) {
		free(obj->name);
		const std::string name = "biglietto buono premio " + mercy_name;
		obj->name = strdup(name.c_str());
	}

	if(!IS_OBJ_STAT(obj, ITEM_IMMUNE)) {
		SET_BIT(obj->obj_flags.extra_flags, ITEM_IMMUNE);
	}

	SetPersonOnSave(ch, obj);
	obj_to_char(obj, ch);

	send_to_char("\n\rUn mistico portale appare all'improvviso!\n\r\n\r", ch);
	act("\n\rUn mistico portale appare all'improvviso!\n\r", false, ch, nullptr, ch, TO_ROOM);
	act("Improvvisamente un paio di braccia sbucano fuori dal portale...\n\rLa mano destra ha in mano una frusta mentre la sinistra ti porge $p.\n\r", false, ch, obj, nullptr, TO_CHAR);
	act("Improvvisamente un paio di braccia sbucano fuori dal portale...\n\rLa mano destra ha in mano una frusta mentre la sinistra porge $p a $N.\n\r", false, ch, obj, ch, TO_ROOM);
	send_to_char("Senti un boato ed il portale, braccia comprese, spariscono in un PUFF!\n\r\n\r", ch);
	act("Senti un boato ed il portale, braccia comprese, spariscono in un PUFF!\n\r", false, ch, nullptr, ch, TO_ROOM);
}
} // namespace

namespace {
void RewardAll(struct char_data* ch, int achievement_type, int achievement_class, int achievement_level)
{
    int reward[4] = {0, 0, 0, 0}, i, percent = 0, premio = 0, god = -1;
    bool win = FALSE;
    string sbch;
    string sbroom;

    const string rand_god[] = {
        "Xanathon",
        "Alar",
        "Darkstar",
        "LadyOfPain",
        "Requiem",
        "Isildur",
        "Flyp",
        "Jethro",
        "Denethor",
        "Ryltar",
        "Croneh",
        "Tethys",
        "Sirio"
    };

    // coin / pozioni / pietre
    reward[0] = AchievementsList[achievement_class][achievement_type].grado_diff * 5 + 45;
    // rune solo dal 51
    reward[1] = AchievementsList[achievement_class][achievement_type].grado_diff * achievement_level * 2;
    // oggetti - no prince
    reward[2] = 50 - GetMaxLevel(ch) + (AchievementsList[achievement_class][achievement_type].grado_diff * 3) + (achievement_level * 5);
    // bonus
    reward[3] = AchievementsList[achievement_class][achievement_type].grado_diff + (achievement_level * 2);

    god = number(0, 12);

    for(i = 0; i < 4; i++)
    {
        if(IS_PRINCE(ch) && i == 2)
        {
            // se il toon è Prince e il reward sono gli oggetti lo ignoro
            i++;
        }
        else if(!IS_PRINCE(ch) && i == 1)
        {
            // se il toon non è Prince e il reward sono rune lo ignoro
            i++;
        }
        percent = number(1, 100);

        if(percent <= reward[i])
        {
            win = TRUE;

            switch(i)
            {
                case 0:     // reward coin, pozioni e pietre
                    {
                        int r_num[10] = {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1}, x = 0, nperc = 0, vnumber, xyz;
                        struct obj_data* obj;
                        struct obj_data* container;

                        vnumber = real_object(CONT_REWARD);
                        container = read_object(vnumber, REAL);
                        obj_to_char(container, ch);

                        percent = number(1,100);
                        percent += GetMaxLevel(ch);

                        if(percent <= 60)
                        {
                            vnumber = real_object(GOLD_REWARD);
                            obj = read_object(vnumber, REAL);
                            obj->obj_flags.value[0] = percent * (number (1000, 3000)) * achievement_level * AchievementsList[achievement_class][achievement_type].grado_diff;

                            if(obj->short_description)
                            {
                                free(obj->short_description);
                            }

                            if(obj->obj_flags.value[0] <= 20000)
                            {
                                obj->short_description = (char*)strdup("un mucchio di monete");
                            }
                            else if(obj->obj_flags.value[0] <= 100000)
                            {
                                obj->short_description = (char*)strdup("un bel mucchio di monete");
                            }
                            else if(obj->obj_flags.value[0] <= 250000)
                            {
                                obj->short_description = (char*)strdup("un grosso mucchio di monete");
                            }
                            else if(obj->obj_flags.value[0] <= 500000)
                            {
                                obj->short_description = (char*)strdup("un ENORME mucchio di monete");
                            }
                            else
                            {
                                obj->short_description = (char*)strdup("una grossa fetta del tesoro di Smaug");
                                if(obj->name)
                                {
                                    free(obj->name);
                                }
                                obj->name = (char*)strdup("tesoro fetta monete");
                            }

                            obj_to_obj(obj, container);
                        }
                        else if(percent > 60 && percent <= 100)
                        {
                            nperc = int(((percent - 60) / 10) + achievement_level + AchievementsList[achievement_class][achievement_type].grado_diff);
                            if(nperc > 8)
                            {
                                nperc = 8;
                            }

                            for(x = 0; x < nperc; x++)
                            {
                                xyz = number(0, POT_REWARD_STAFF_COUNT - 1);
                                vnumber = real_object(POT_REWARD_STAFF_BASE + xyz);
                                obj = read_object(vnumber, REAL);
                                if(obj) {
                                    obj_to_obj(obj, container);
                                }
                            }
                            mudlog(LOG_PLAYERS, "%s won some potions with Achievements.", GET_NAME(ch));
                        }
                        else if(percent > 100 && percent <= 120)
                        {
                            nperc = int (((percent - 60) / 10) + achievement_level + AchievementsList[achievement_class][achievement_type].grado_diff);
                            if(nperc > 8)
                            {
                                nperc = 8;
                            }

                            vnumber = real_object(GOLD_REWARD);
                            obj = read_object(vnumber, REAL);
                            obj->obj_flags.value[0] = percent * (number (1000, 3000)) * achievement_level * AchievementsList[achievement_class][achievement_type].grado_diff;
                            if(obj->short_description)
                            {
                                free(obj->short_description);
                            }

                            if(obj->obj_flags.value[0] <= 20000)
                            {
                                obj->short_description = (char*)strdup("un mucchio di monete");
                            }
                            else if(obj->obj_flags.value[0] <= 100000)
                            {
                                obj->short_description = (char*)strdup("un bel mucchio di monete");
                            }
                            else if(obj->obj_flags.value[0] <= 250000)
                            {
                                obj->short_description = (char*)strdup("un grosso mucchio di monete");
                            }
                            else if(obj->obj_flags.value[0] <= 500000)
                            {
                                obj->short_description = (char*)strdup("un ENORME mucchio di monete");
                            }
                            else
                            {
                                obj->short_description = (char*)strdup("una grossa fetta del tesoro di Smaug");
                                if(obj->name)
                                {
                                    free(obj->name);
                                }
                                obj->name = (char*)strdup("tesoro fetta monete");
                            }

                            obj_to_obj(obj, container);
                            mudlog(LOG_PLAYERS, "%s won %d gold coins with Achievements and...", GET_NAME(ch), obj->obj_flags.value[0]);


                            for(x = 0; x < nperc; x++)
                            {
                                xyz = number(0, POT_REWARD_STAFF_COUNT - 1);
                                vnumber = real_object(POT_REWARD_STAFF_BASE + xyz);
                                obj = read_object(vnumber, REAL);
                                if(obj) {
                                    obj_to_obj(obj, container);
                                }
                            }
                            mudlog(LOG_PLAYERS, "... %d potion$s too.", nperc, (nperc == 1 ? "" : "s"));
                        }
                        else if(percent > 120 && percent <= 130)
                        {
                            nperc = percent - 120;
                            nperc = nperc - 3;
                            if(IS_PRINCE(ch))
                            {
                                nperc += 1;
                            }
                            if(nperc < 1)
                            {
                                nperc = 1;
                            }

                            for(x = 0; x < nperc; x++)
                            {
                                percent = number(1,100);
                                if(percent <= 10)
                                {
                                    r_num[x] = real_object(19509);    //quarzo comune
                                }
                                else if(percent > 10 && percent <= 20)
                                {
                                    r_num[x] = real_object(19510);    //ossidiana
                                }
                                else if(percent > 20 && percent <= 30)
                                {
                                    r_num[x] = real_object(19511);    //opale
                                }
                                else if(percent > 30 && percent <= 40)
                                {
                                    r_num[x] = real_object(19512);    //turchese
                                }
                                else if(percent > 40 && percent <= 50)
                                {
                                    r_num[x] = real_object(19513);    //zircone
                                }
                                else if(percent > 50 && percent <= 60)
                                {
                                    r_num[x] = real_object(19514);    //lapislazzuli
                                }
                                else if(percent > 60 && percent <= 70)
                                {
                                    r_num[x] = real_object(19515);    //onice
                                }
                                else if(percent > 70 && percent <= 80)
                                {
                                    r_num[x] = real_object(19516);    //malachite
                                }
                                else if(percent > 80 && percent <= 90)
                                {
                                    r_num[x] = real_object(19517);    //ematite
                                }
                                else if(percent > 90)
                                {
                                    r_num[x] = real_object(19518);    //giada
                                }

                                if(r_num[x] >= 0)
                                {
                                    obj = read_object(r_num[x], REAL);
                                    obj_to_obj(obj, container);
                                }
                                mudlog(LOG_PLAYERS, "%s won %d gem%s with Achievements.", GET_NAME(ch), x, (x == 1 ? "" : "s"));
                            }
                        }
                        else if(percent > 130 && percent <= 140)
                        {
                            nperc = percent - 130;
                            if(IS_PRINCE(ch))
                            {
                                nperc = int(nperc / 2) + 1;
                            }
                            else
                            {
                                nperc = int(nperc / 2);
                            }
                            if(nperc < 1)
                            {
                                nperc = 1;
                            }

                            for(x = 0; x < nperc; x++)
                            {
                                percent = number(1,100);
                                if(percent <= 10)
                                {
                                    r_num[x] = real_object(19519);    //resina fossile
                                }
                                else if(percent > 10 && percent <= 20)
                                {
                                    r_num[x] = real_object(19520);    //crisoberillo
                                }
                                else if(percent > 20 && percent <= 30)
                                {
                                    r_num[x] = real_object(19521);    //spinello blu
                                }
                                else if(percent > 30 && percent <= 40)
                                {
                                    r_num[x] = real_object(19522);    //tormalina
                                }
                                else if(percent > 40 && percent <= 50)
                                {
                                    r_num[x] = real_object(19523);    //quarzo comune, clone
                                }
                                else if(percent > 50 && percent <= 60)
                                {
                                    r_num[x] = real_object(19524);    //quarzo rosa
                                }
                                else if(percent > 60 && percent <= 70)
                                {
                                    r_num[x] = real_object(19525);    //agata
                                }
                                else if(percent > 70 && percent <= 80)
                                {
                                    r_num[x] = real_object(19526);    //acquamarina
                                }
                                else if(percent > 80 && percent <= 90)
                                {
                                    r_num[x] = real_object(19527);    //berillo
                                }
                                else if(percent > 90)
                                {
                                    r_num[x] = real_object(19528);    //topazio
                                }

                                if(r_num[x] >= 0)
                                {
                                    obj = read_object(r_num[x], REAL);
                                    obj_to_obj(obj, container);
                                }
                                mudlog(LOG_PLAYERS, "%s won %d gem%s with Achievements.", GET_NAME(ch), x, (x == 1 ? "" : "s"));
                            }
                        }
                        else if(percent > 140)
                        {
                            nperc = percent - 140;
                            if(IS_PRINCE(ch))
                            {
                                nperc = int(nperc / 3) + 1;
                            }
                            else
                            {
                                nperc = int(nperc / 3);
                            }
                            if(nperc < 1)
                            {
                                nperc = 1;
                            }

                            for(x = 0; x < nperc; x++)
                            {
                                percent = number(1,100);
                                if(percent <= 12)
                                {
                                    r_num[x] = real_object(19529);    //spinello nero
                                }
                                else if(percent > 12 && percent <= 24)
                                {
                                    r_num[x] = real_object(19530);    //fluorite
                                }
                                else if(percent > 24 && percent <= 36)
                                {
                                    r_num[x] = real_object(19531);    //ametista
                                }
                                else if(percent > 36 && percent <= 48)
                                {
                                    r_num[x] = real_object(19532);    //corindone
                                }
                                else if(percent > 48 && percent <= 60)
                                {
                                    r_num[x] = real_object(19533);    //granato
                                }
                                else if(percent > 60 && percent <= 70)
                                {
                                    r_num[x] = real_object(19534);    //zaffiro
                                }
                                else if(percent > 70 && percent <= 80)
                                {
                                    r_num[x] = real_object(19535);    //smeraldo
                                }
                                else if(percent > 80 && percent <= 90)
                                {
                                    r_num[x] = real_object(19536);    //rubino
                                }
                                else if(percent > 90)
                                {
                                    r_num[x] = real_object(19537);    //diamante
                                }

                                if(r_num[x] >= 0)
                                {
                                    obj = read_object(r_num[x], REAL);
                                    obj_to_obj(obj, container);
                                }
                                mudlog(LOG_PLAYERS, "%s won %d gem%s with Achievements.", GET_NAME(ch), x, (x == 1 ? "" : "s"));
                            }
                        }

                        boost::format fmt("$c0014L'ombra di %s ti consegna un elaborato cofanetto di avorio.\n\r");
                        fmt % rand_god[god];
                        sbch.append(fmt.str().c_str());
                        fmt.clear();
                        boost::format fmt2("$c0014L'ombra di %s consegna un elaborato cofanetto di avorio a $N.\n\r");
                        fmt2 % rand_god[god];
                        sbroom.append(fmt2.str().c_str());
                        fmt2.clear();
                    }
                    break;

                case 1:     // reward rune
                    {
                        premio = AchievementsList[achievement_class][achievement_type].grado_diff + number(1, (2 * achievement_level)) - 1;
                        boost::format fmt("$c0011L'ombra di %s incide %d run%s sulla tua pelle.\n\r");
                        fmt % rand_god[god] % premio % (premio == 1 ? "a" : "e");
                        sbch.append(fmt.str().c_str());
                        fmt.clear();
                        boost::format fmt2("$c0011L'ombra di %s incide %s run%s sulla pelle di $N.\n\r");
                        fmt2 % rand_god[god] % (premio == 1 ? "una" : "alcune") % (premio == 1 ? "a" : "e");
                        sbroom.append(fmt2.str().c_str());
                        fmt2.clear();
                        GET_RUNEDEI(ch) += premio;
                        mudlog(LOG_PLAYERS, "%s won %d rune%s with Achievements.", GET_NAME(ch), premio, (premio == 1 ? "" : "s"));
                    }
                    break;

                case 2:     // reward oggetti
                    {
                        struct obj_data* obj;
                        int x, y, vnum, random_obj = 0, apply[5] = {0, 0, 0, 0, 0}, bonus[5] = {0, 0, 0, 0, 0}, roll, classe = 0, livello, weapon_rand;

                        random_obj = number(0, 19);

                        if(HasClass(ch, CLASS_MONK) && random_obj == 17)
                        {
                            random_obj = 5;
                        }

                        if(IS_MELEE(ch) && random_obj > 17)
                        {
                            random_obj = number(0, 17);
                            if(HasClass(ch, CLASS_MONK) && random_obj == 17)
                            {
                                random_obj = 5;
                            }
                        }

                        if(IS_MELEE(ch))
                        {
                            classe = CLASS_MELEE;
                        }
                        else if(IS_CASTER_N(ch))
                        {
                            classe = CLASS_CASTER;
                        }
                        else if(IS_MULTI(ch))
                        {
                            classe = CLASS_MULTI;
                        }
                        else if(IS_IMMORTAL(ch))
                        {
                            classe = number(0, 2);
                        }

                        if(GetMaxLevel(ch) <= 10)
                        {
                            livello = 0;
                        }
                        else if(GetMaxLevel(ch) > 10 && GetMaxLevel(ch) <= 20)
                        {
                            livello = 1;
                        }
                        else if(GetMaxLevel(ch) > 20 && GetMaxLevel(ch) <= 30)
                        {
                            livello = 2;
                        }
                        else if(GetMaxLevel(ch) > 30 && GetMaxLevel(ch) <= 40)
                        {
                            livello = 3;
                        }
                        else if(GetMaxLevel(ch) > 40 && GetMaxLevel(ch) < 51)
                        {
                            livello = 4;
                        }
                        else if(GetMaxLevel(ch) > 51)
                        {
                            livello = number(0, 4);
                        }

                        for(x = 0; x < MAX_OBJ_AFFECT; x++)
                        {
                            roll = number(1, 100);
                            roll -= (GetMaxLevel(ch) / 3 + number(1, 5));

                            switch(x)
                            {
                                case 0:
                                    if(roll <= RewardObj[classe][random_obj].perc1)
                                    {
                                        apply[x] = RewardObj[classe][random_obj].bonus1;
                                    }
                                    break;

                                case 1:
                                    if(roll <= RewardObj[classe][random_obj].perc2)
                                    {
                                        apply[x] = RewardObj[classe][random_obj].bonus2;
                                    }
                                    break;

                                case 2:
                                    if(roll <= RewardObj[classe][random_obj].perc3)
                                    {
                                        apply[x] = RewardObj[classe][random_obj].bonus3;
                                    }
                                    break;

                                case 3:
                                    if(roll <= RewardObj[classe][random_obj].perc4)
                                    {
                                        apply[x] = RewardObj[classe][random_obj].bonus4;
                                    }
                                    break;

                                case 4:
                                    if(roll <= RewardObj[classe][random_obj].perc5)
                                    {
                                        apply[x] = RewardObj[classe][random_obj].bonus5;
                                    }
                                    break;

                                default:
                                    break;
                            }

                            switch(apply[x])
                            {
                                case APPLY_HIT:
                                    {
                                        switch(livello)
                                        {
                                            case 0:
                                                {
                                                    for(y = 0; y < MAX_CLASS; y++)
                                                    {
                                                        if(HasClass(ch, (1 << y)))
                                                        {
                                                            bonus[x] += AchieBonus[y][ACHIE_HIT_TABLE].lev1_10;
                                                        }
                                                    }
                                                }
                                                break;

                                            case 1:
                                                {
                                                    for(y = 0; y < MAX_CLASS; y++)
                                                    {
                                                        if(HasClass(ch, (1 << y)))
                                                        {
                                                            bonus[x] += AchieBonus[y][ACHIE_HIT_TABLE].lev11_20;
                                                        }
                                                    }
                                                }
                                                break;

                                            case 2:
                                                {
                                                    for(y = 0; y < MAX_CLASS; y++)
                                                    {
                                                        if(HasClass(ch, (1 << y)))
                                                        {
                                                            bonus[x] += AchieBonus[y][ACHIE_HIT_TABLE].lev21_30;
                                                        }
                                                    }
                                                }
                                                break;

                                            case 3:
                                                {
                                                    for(y = 0; y < MAX_CLASS; y++)
                                                    {
                                                        if(HasClass(ch, (1 << y)))
                                                        {
                                                            bonus[x] += AchieBonus[y][ACHIE_HIT_TABLE].lev31_40;
                                                        }
                                                    }
                                                }
                                                break;

                                            case 4:
                                                {
                                                    for(y = 0; y < MAX_CLASS; y++)
                                                    {
                                                        if(HasClass(ch, (1 << y)))
                                                        {
                                                            bonus[x] += AchieBonus[y][ACHIE_HIT_TABLE].lev41_50;
                                                        }
                                                    }
                                                }
                                                break;

                                            default:
                                                break;
                                        }
                                        bonus[x] /= HowManyClasses(ch);
                                    }
                                    break;

                                case APPLY_MANA:
                                    {
                                        switch(livello)
                                        {
                                            case 0:
                                                {
                                                    for(y = 0; y < MAX_CLASS; y++)
                                                    {
                                                        if(HasClass(ch, (1 << y)))
                                                        {
                                                            bonus[x] += AchieBonus[y][ACHIE_MANA_TABLE].lev1_10;
                                                        }
                                                    }
                                                }
                                                break;

                                            case 1:
                                                {
                                                    for(y = 0; y < MAX_CLASS; y++)
                                                    {
                                                        if(HasClass(ch, (1 << y)))
                                                        {
                                                            bonus[x] += AchieBonus[y][ACHIE_MANA_TABLE].lev11_20;
                                                        }
                                                    }
                                                }
                                                break;

                                            case 2:
                                                {
                                                    for(y = 0; y < MAX_CLASS; y++)
                                                    {
                                                        if(HasClass(ch, (1 << y)))
                                                        {
                                                            bonus[x] += AchieBonus[y][ACHIE_MANA_TABLE].lev21_30;
                                                        }
                                                    }
                                                }
                                                break;

                                            case 3:
                                                {
                                                    for(y = 0; y < MAX_CLASS; y++)
                                                    {
                                                        if(HasClass(ch, (1 << y)))
                                                        {
                                                            bonus[x] += AchieBonus[y][ACHIE_MANA_TABLE].lev31_40;
                                                        }
                                                    }
                                                }
                                                break;

                                            case 4:
                                                {
                                                    for(y = 0; y < MAX_CLASS; y++)
                                                    {
                                                        if(HasClass(ch, (1 << y)))
                                                        {
                                                            bonus[x] += AchieBonus[y][ACHIE_MANA_TABLE].lev41_50;
                                                        }
                                                    }
                                                }
                                                break;

                                            default:
                                                break;
                                        }
                                        bonus[x] /= HowManyClasses(ch);
                                    }
                                    break;

                                case APPLY_MOVE:
                                    {
                                        switch(livello)
                                        {
                                            case 0:
                                                {
                                                    for(y = 0; y < MAX_CLASS; y++)
                                                    {
                                                        if(HasClass(ch, (1 << y)))
                                                        {
                                                            bonus[x] += AchieBonus[y][ACHIE_MOVE_TABLE].lev1_10;
                                                        }
                                                    }
                                                }
                                                break;

                                            case 1:
                                                {
                                                    for(y = 0; y < MAX_CLASS; y++)
                                                    {
                                                        if(HasClass(ch, (1 << y)))
                                                        {
                                                            bonus[x] += AchieBonus[y][ACHIE_MOVE_TABLE].lev11_20;
                                                        }
                                                    }
                                                }
                                                break;

                                            case 2:
                                                {
                                                    for(y = 0; y < MAX_CLASS; y++)
                                                    {
                                                        if(HasClass(ch, (1 << y)))
                                                        {
                                                            bonus[x] += AchieBonus[y][ACHIE_MOVE_TABLE].lev21_30;
                                                        }
                                                    }
                                                }
                                                break;

                                            case 3:
                                                {
                                                    for(y = 0; y < MAX_CLASS; y++)
                                                    {
                                                        if(HasClass(ch, (1 << y)))
                                                        {
                                                            bonus[x] += AchieBonus[y][ACHIE_MOVE_TABLE].lev31_40;
                                                        }
                                                    }
                                                }
                                                break;

                                            case 4:
                                                {
                                                    for(y = 0; y < MAX_CLASS; y++)
                                                    {
                                                        if(HasClass(ch, (1 << y)))
                                                        {
                                                            bonus[x] += AchieBonus[y][ACHIE_MOVE_TABLE].lev41_50;
                                                        }
                                                    }
                                                }
                                                break;

                                            default:
                                                break;
                                        }
                                        bonus[x] /= HowManyClasses(ch);
                                    }
                                    break;

                                case APPLY_HIT_REGEN:
                                    {
                                        switch(livello)
                                        {
                                            case 0:
                                                {
                                                    for(y = 0; y < MAX_CLASS; y++)
                                                    {
                                                        if(HasClass(ch, (1 << y)))
                                                        {
                                                            bonus[x] += AchieBonus[y][ACHIE_HITREG_TABLE].lev1_10;
                                                        }
                                                    }
                                                }
                                                break;

                                            case 1:
                                                {
                                                    for(y = 0; y < MAX_CLASS; y++)
                                                    {
                                                        if(HasClass(ch, (1 << y)))
                                                        {
                                                            bonus[x] += AchieBonus[y][ACHIE_HITREG_TABLE].lev11_20;
                                                        }
                                                    }
                                                }
                                                break;

                                            case 2:
                                                {
                                                    for(y = 0; y < MAX_CLASS; y++)
                                                    {
                                                        if(HasClass(ch, (1 << y)))
                                                        {
                                                            bonus[x] += AchieBonus[y][ACHIE_HITREG_TABLE].lev21_30;
                                                        }
                                                    }
                                                }
                                                break;

                                            case 3:
                                                {
                                                    for(y = 0; y < MAX_CLASS; y++)
                                                    {
                                                        if(HasClass(ch, (1 << y)))
                                                        {
                                                            bonus[x] += AchieBonus[y][ACHIE_HITREG_TABLE].lev31_40;
                                                        }
                                                    }
                                                }
                                                break;

                                            case 4:
                                                {
                                                    for(y = 0; y < MAX_CLASS; y++)
                                                    {
                                                        if(HasClass(ch, (1 << y)))
                                                        {
                                                            bonus[x] += AchieBonus[y][ACHIE_HITREG_TABLE].lev41_50;
                                                        }
                                                    }
                                                }
                                                break;

                                            default:
                                                break;
                                        }
                                        bonus[x] /= HowManyClasses(ch);
                                    }
                                    break;

                                case APPLY_MANA_REGEN:
                                    {
                                        switch(livello)
                                        {
                                            case 0:
                                                {
                                                    for(y = 0; y < MAX_CLASS; y++)
                                                    {
                                                        if(HasClass(ch, (1 << y)))
                                                        {
                                                            bonus[x] += AchieBonus[y][ACHIE_MANAREG_TABLE].lev1_10;
                                                        }
                                                    }
                                                }
                                                break;

                                            case 1:
                                                {
                                                    for(y = 0; y < MAX_CLASS; y++)
                                                    {
                                                        if(HasClass(ch, (1 << y)))
                                                        {
                                                            bonus[x] += AchieBonus[y][ACHIE_MANAREG_TABLE].lev11_20;
                                                        }
                                                    }
                                                }
                                                break;

                                            case 2:
                                                {
                                                    for(y = 0; y < MAX_CLASS; y++)
                                                    {
                                                        if(HasClass(ch, (1 << y)))
                                                        {
                                                            bonus[x] += AchieBonus[y][ACHIE_MANAREG_TABLE].lev21_30;
                                                        }
                                                    }
                                                }
                                                break;

                                            case 3:
                                                {
                                                    for(y = 0; y < MAX_CLASS; y++)
                                                    {
                                                        if(HasClass(ch, (1 << y)))
                                                        {
                                                            bonus[x] += AchieBonus[y][ACHIE_MANAREG_TABLE].lev31_40;
                                                        }
                                                    }
                                                }
                                                break;

                                            case 4:
                                                {
                                                    for(y = 0; y < MAX_CLASS; y++)
                                                    {
                                                        if(HasClass(ch, (1 << y)))
                                                        {
                                                            bonus[x] += AchieBonus[y][ACHIE_MANAREG_TABLE].lev41_50;
                                                        }
                                                    }
                                                }
                                                break;

                                            default:
                                                break;
                                        }
                                        bonus[x] /= HowManyClasses(ch);
                                    }
                                    break;

                                case APPLY_MOVE_REGEN:
                                    {
                                        switch(livello)
                                        {
                                            case 0:
                                                {
                                                    for(y = 0; y < MAX_CLASS; y++)
                                                    {
                                                        if(HasClass(ch, (1 << y)))
                                                        {
                                                            bonus[x] += AchieBonus[y][ACHIE_MOVEREG_TABLE].lev1_10;
                                                        }
                                                    }
                                                }
                                                break;

                                            case 1:
                                                {
                                                    for(y = 0; y < MAX_CLASS; y++)
                                                    {
                                                        if(HasClass(ch, (1 << y)))
                                                        {
                                                            bonus[x] += AchieBonus[y][ACHIE_MOVEREG_TABLE].lev11_20;
                                                        }
                                                    }
                                                }
                                                break;

                                            case 2:
                                                {
                                                    for(y = 0; y < MAX_CLASS; y++)
                                                    {
                                                        if(HasClass(ch, (1 << y)))
                                                        {
                                                            bonus[x] += AchieBonus[y][ACHIE_MOVEREG_TABLE].lev21_30;
                                                        }
                                                    }
                                                }
                                                break;

                                            case 3:
                                                {
                                                    for(y = 0; y < MAX_CLASS; y++)
                                                    {
                                                        if(HasClass(ch, (1 << y)))
                                                        {
                                                            bonus[x] += AchieBonus[y][ACHIE_MOVEREG_TABLE].lev31_40;
                                                        }
                                                    }
                                                }
                                                break;

                                            case 4:
                                                {
                                                    for(y = 0; y < MAX_CLASS; y++)
                                                    {
                                                        if(HasClass(ch, (1 << y)))
                                                        {
                                                            bonus[x] += AchieBonus[y][ACHIE_MOVEREG_TABLE].lev41_50;
                                                        }
                                                    }
                                                }
                                                break;

                                            default:
                                                break;
                                        }
                                        bonus[x] /= HowManyClasses(ch);
                                    }
                                    break;

                                case STAT_RANDOM:
                                    {
                                        apply[x] = number(1, 6);
                                        bonus[x] = MAX(number(0, (livello-1)), 1);
                                    }
                                    break;

                                case APPLY_SPELLFAIL:
                                    {
                                        switch(livello)
                                        {
                                            case 0:
                                                {
                                                    for(y = 0; y < MAX_CLASS; y++)
                                                    {
                                                        if(HasClass(ch, (1 << y)))
                                                        {
                                                            bonus[x] -= AchieBonus[y][ACHIE_SPELLFAIL_TABLE].lev1_10;
                                                        }
                                                    }
                                                }
                                                break;

                                            case 1:
                                                {
                                                    for(y = 0; y < MAX_CLASS; y++)
                                                    {
                                                        if(HasClass(ch, (1 << y)))
                                                        {
                                                            bonus[x] -= AchieBonus[y][ACHIE_SPELLFAIL_TABLE].lev11_20;
                                                        }
                                                    }
                                                }
                                                break;

                                            case 2:
                                                {
                                                    for(y = 0; y < MAX_CLASS; y++)
                                                    {
                                                        if(HasClass(ch, (1 << y)))
                                                        {
                                                            bonus[x] -= AchieBonus[y][ACHIE_SPELLFAIL_TABLE].lev21_30;
                                                        }
                                                    }
                                                }
                                                break;

                                            case 3:
                                                {
                                                    for(y = 0; y < MAX_CLASS; y++)
                                                    {
                                                        if(HasClass(ch, (1 << y)))
                                                        {
                                                            bonus[x] -= AchieBonus[y][ACHIE_SPELLFAIL_TABLE].lev31_40;
                                                        }
                                                    }
                                                }
                                                break;

                                            case 4:
                                                {
                                                    for(y = 0; y < MAX_CLASS; y++)
                                                    {
                                                        if(HasClass(ch, (1 << y)))
                                                        {
                                                            bonus[x] -= AchieBonus[y][ACHIE_SPELLFAIL_TABLE].lev41_50;
                                                        }
                                                    }
                                                }
                                                break;

                                            default:
                                                break;
                                        }
                                        bonus[x] /= HowManyClasses(ch);
                                    }
                                    break;

                                case ELEM_RESI_RANDOM:
                                    {
                                        apply[x] = APPLY_IMMUNE;
                                        switch(number(1,5))
                                        {
                                            case 1:
                                                bonus[x] = IMM_FIRE;
                                                break;

                                            case 2:
                                                bonus[x] = IMM_COLD;
                                                break;

                                            case 3:
                                                bonus[x] = IMM_ELEC;
                                                break;

                                            case 4:
                                                bonus[x] = IMM_ENERGY;
                                                break;

                                            case 5:
                                                bonus[x] = IMM_ACID;
                                                break;

                                            default:
                                                break;
                                        }
                                    }
                                    break;

                                case OBJ_RESI_PIERCE:
                                    {
                                        apply[x] = APPLY_IMMUNE;
                                        bonus[x] = IMM_PIERCE;
                                    }
                                    break;

                                case OBJ_RESI_SLASH:
                                    {
                                        apply[x] = APPLY_IMMUNE;
                                        bonus[x] = IMM_SLASH;
                                    }
                                    break;

                                case APPLY_HITROLL:
                                    {
                                        switch(livello)
                                        {
                                            case 0:
                                                {
                                                    for(y = 0; y < MAX_CLASS; y++)
                                                    {
                                                        if(HasClass(ch, (1 << y)))
                                                        {
                                                            bonus[x] += AchieBonus[y][ACHIE_HITROLL_TABLE].lev1_10;
                                                        }
                                                    }
                                                }
                                                break;

                                            case 1:
                                                {
                                                    for(y = 0; y < MAX_CLASS; y++)
                                                    {
                                                        if(HasClass(ch, (1 << y)))
                                                        {
                                                            bonus[x] += AchieBonus[y][ACHIE_HITROLL_TABLE].lev11_20;
                                                        }
                                                    }
                                                }
                                                break;

                                            case 2:
                                                {
                                                    for(y = 0; y < MAX_CLASS; y++)
                                                    {
                                                        if(HasClass(ch, (1 << y)))
                                                        {
                                                            bonus[x] += AchieBonus[y][ACHIE_HITROLL_TABLE].lev21_30;
                                                        }
                                                    }
                                                }
                                                break;

                                            case 3:
                                                {
                                                    for(y = 0; y < MAX_CLASS; y++)
                                                    {
                                                        if(HasClass(ch, (1 << y)))
                                                        {
                                                            bonus[x] += AchieBonus[y][ACHIE_HITROLL_TABLE].lev31_40;
                                                        }
                                                    }
                                                }
                                                break;

                                            case 4:
                                                {
                                                    for(y = 0; y < MAX_CLASS; y++)
                                                    {
                                                        if(HasClass(ch, (1 << y)))
                                                        {
                                                            bonus[x] += AchieBonus[y][ACHIE_HITROLL_TABLE].lev41_50;
                                                        }
                                                    }
                                                }
                                                break;

                                            default:
                                                break;
                                        }
                                        bonus[x] /= HowManyClasses(ch);
                                    }
                                    break;

                                case APPLY_DAMROLL:
                                    {
                                        switch(livello)
                                        {
                                            case 0:
                                                {
                                                    for(y = 0; y < MAX_CLASS; y++)
                                                    {
                                                        if(HasClass(ch, (1 << y)))
                                                        {
                                                            bonus[x] += AchieBonus[y][ACHIE_DAMROLL_TABLE].lev1_10;
                                                        }
                                                    }
                                                }
                                                break;

                                            case 1:
                                                {
                                                    for(y = 0; y < MAX_CLASS; y++)
                                                    {
                                                        if(HasClass(ch, (1 << y)))
                                                        {
                                                            bonus[x] += AchieBonus[y][ACHIE_DAMROLL_TABLE].lev11_20;
                                                        }
                                                    }
                                                }
                                                break;

                                            case 2:
                                                {
                                                    for(y = 0; y < MAX_CLASS; y++)
                                                    {
                                                        if(HasClass(ch, (1 << y)))
                                                        {
                                                            bonus[x] += AchieBonus[y][ACHIE_DAMROLL_TABLE].lev21_30;
                                                        }
                                                    }
                                                }
                                                break;

                                            case 3:
                                                {
                                                    for(y = 0; y < MAX_CLASS; y++)
                                                    {
                                                        if(HasClass(ch, (1 << y)))
                                                        {
                                                            bonus[x] += AchieBonus[y][ACHIE_DAMROLL_TABLE].lev31_40;
                                                        }
                                                    }
                                                }
                                                break;

                                            case 4:
                                                {
                                                    for(y = 0; y < MAX_CLASS; y++)
                                                    {
                                                        if(HasClass(ch, (1 << y)))
                                                        {
                                                            bonus[x] += AchieBonus[y][ACHIE_DAMROLL_TABLE].lev41_50;
                                                        }
                                                    }
                                                }
                                                break;

                                            default:
                                                break;
                                        }
                                        bonus[x] /= HowManyClasses(ch);
                                    }
                                    break;

                                case APPLY_HITNDAM:
                                    {
                                        switch(livello)
                                        {
                                            case 0:
                                                {
                                                    for(y = 0; y < MAX_CLASS; y++)
                                                    {
                                                        if(HasClass(ch, (1 << y)))
                                                        {
                                                            bonus[x] += AchieBonus[y][ACHIE_HITNDAM_TABLE].lev1_10;
                                                        }
                                                    }
                                                }
                                                break;

                                            case 1:
                                                {
                                                    for(y = 0; y < MAX_CLASS; y++)
                                                    {
                                                        if(HasClass(ch, (1 << y)))
                                                        {
                                                            bonus[x] += AchieBonus[y][ACHIE_HITNDAM_TABLE].lev11_20;
                                                        }
                                                    }
                                                }
                                                break;

                                            case 2:
                                                {
                                                    for(y = 0; y < MAX_CLASS; y++)
                                                    {
                                                        if(HasClass(ch, (1 << y)))
                                                        {
                                                            bonus[x] += AchieBonus[y][ACHIE_HITNDAM_TABLE].lev21_30;
                                                        }
                                                    }
                                                }
                                                break;

                                            case 3:
                                                {
                                                    for(y = 0; y < MAX_CLASS; y++)
                                                    {
                                                        if(HasClass(ch, (1 << y)))
                                                        {
                                                            bonus[x] += AchieBonus[y][ACHIE_HITNDAM_TABLE].lev31_40;
                                                        }
                                                    }
                                                }
                                                break;

                                            case 4:
                                                {
                                                    for(y = 0; y < MAX_CLASS; y++)
                                                    {
                                                        if(HasClass(ch, (1 << y)))
                                                        {
                                                            bonus[x] += AchieBonus[y][ACHIE_HITNDAM_TABLE].lev41_50;
                                                        }
                                                    }
                                                }
                                                break;

                                            default:
                                                break;
                                        }
                                        bonus[x] /= HowManyClasses(ch);
                                    }
                                    break;

                                case APPLY_AC:
                                    {
                                        switch(livello)
                                        {
                                            case 0:
                                                {
                                                    for(y = 0; y < MAX_CLASS; y++)
                                                    {
                                                        if(HasClass(ch, (1 << y)))
                                                        {
                                                            bonus[x] -= AchieBonus[y][ACHIE_ARMOR_TABLE].lev1_10;
                                                        }
                                                    }
                                                }
                                                break;

                                            case 1:
                                                {
                                                    for(y = 0; y < MAX_CLASS; y++)
                                                    {
                                                        if(HasClass(ch, (1 << y)))
                                                        {
                                                            bonus[x] -= AchieBonus[y][ACHIE_ARMOR_TABLE].lev11_20;
                                                        }
                                                    }
                                                }
                                                break;

                                            case 2:
                                                {
                                                    for(y = 0; y < MAX_CLASS; y++)
                                                    {
                                                        if(HasClass(ch, (1 << y)))
                                                        {
                                                            bonus[x] -= AchieBonus[y][ACHIE_ARMOR_TABLE].lev21_30;
                                                        }
                                                    }
                                                }
                                                break;

                                            case 3:
                                                {
                                                    for(y = 0; y < MAX_CLASS; y++)
                                                    {
                                                        if(HasClass(ch, (1 << y)))
                                                        {
                                                            bonus[x] -= AchieBonus[y][ACHIE_ARMOR_TABLE].lev31_40;
                                                        }
                                                    }
                                                }
                                                break;

                                            case 4:
                                                {
                                                    for(y = 0; y < MAX_CLASS; y++)
                                                    {
                                                        if(HasClass(ch, (1 << y)))
                                                        {
                                                            bonus[x] -= AchieBonus[y][ACHIE_ARMOR_TABLE].lev41_50;
                                                        }
                                                    }
                                                }
                                                break;

                                            default:
                                                break;
                                        }
                                        bonus[x] /= HowManyClasses(ch);
                                    }
                                    break;

                                case OBJ_ARTIFACT:
                                    break;

                                case OBJ_SENCE_LIFE:
                                    {
                                        apply[x] = APPLY_SPELL;
                                        bonus[x] = AFF_SENSE_LIFE;
                                    }
                                    break;

                                case OBJ_TRUE_SIGHT:
                                    {
                                        apply[x] = APPLY_SPELL;
                                        bonus[x] = AFF_TRUE_SIGHT;
                                    }
                                    break;

                                case OBJ_SNEAK:
                                    {
                                        apply[x] = APPLY_SPELL;
                                        bonus[x] = AFF_SNEAK;
                                    }
                                    break;

                                case OBJ_FLY:
                                    {
                                        apply[x] = APPLY_SPELL;
                                        bonus[x] = AFF_FLYING;
                                    }
                                    break;

                                case OBJ_INVISIBILITY:
                                    {
                                        apply[x] = APPLY_SPELL;
                                        bonus[x] = AFF_INVISIBLE;
                                    }
                                    break;

                                case OBJ_LIFE_PROT:
                                    {
                                        apply[x] = APPLY_SPELL;
                                        bonus[x] = AFF_LIFE_PROT;
                                    }
                                    break;

                                case OBJ_TELEPATHY:
                                    {
                                        apply[x] = APPLY_SPELL;
                                        bonus[x] = AFF_TELEPATHY;
                                    }
                                    break;

                                case OBJ_SPY:
                                    {
                                        apply[x] = APPLY_SPELL;
                                        bonus[x] = AFF_SCRYING;
                                    }
                                    break;

                                default:
                                    break;
                            }
                        }

                        vnum = RewardObj[classe][random_obj].vnum_obj;

                        random_obj = real_object(vnum);

                        obj = read_object(random_obj, REAL);

                        roll = number(1, 100);
                        if(roll < 100)
                        {
                            roll -= (GetMaxLevel(ch) / 2);
                        }
                        if(roll <= 70)
                        {
                            SET_BIT(obj->obj_flags.extra_flags, ITEM_RESISTANT);
                        }

                        if(GET_ITEM_TYPE(obj) == ITEM_ARMOR)
                        {
                            switch(vnum)
                            {
                                case 1382:  //  about body
                                case 1383:  //  waist
                                case 1394:  //  wrist melee
                                case 1395:  //  eyes melee
                                    {
                                        obj->obj_flags.value[0] = number(1, livello);
                                        obj->obj_flags.value[1] = obj->obj_flags.value[0];
                                    }
                                    break;

                                case 1384:  //  feet
                                case 1385:  //  hands
                                case 1386:  //  body
                                case 1387:  //  head
                                case 1388:  //  legs
                                case 1389:  //  arms
                                case 1390:  //  shield
                                case 1396:  //  back melee
                                    {
                                        obj->obj_flags.value[0] = 0;
                                        switch(livello)
                                        {
                                            case 0:
                                                {
                                                    for(y = 0; y < MAX_CLASS; y++)
                                                    {
                                                        if(HasClass(ch, (1 << y)))
                                                        {
                                                            obj->obj_flags.value[0] += AchieBonus[y][ACHIE_AC_TABLE].lev1_10;
                                                        }
                                                    }
                                                }
                                                break;

                                            case 1:
                                                {
                                                    for(y = 0; y < MAX_CLASS; y++)
                                                    {
                                                        if(HasClass(ch, (1 << y)))
                                                        {
                                                            obj->obj_flags.value[0] += AchieBonus[y][ACHIE_AC_TABLE].lev11_20;
                                                        }
                                                    }
                                                }
                                                break;

                                            case 2:
                                                {
                                                    for(y = 0; y < MAX_CLASS; y++)
                                                    {
                                                        if(HasClass(ch, (1 << y)))
                                                        {
                                                            obj->obj_flags.value[0] += AchieBonus[y][ACHIE_AC_TABLE].lev21_30;
                                                        }
                                                    }
                                                }
                                                break;

                                            case 3:
                                                {
                                                    for(y = 0; y < MAX_CLASS; y++)
                                                    {
                                                        if(HasClass(ch, (1 << y)))
                                                        {
                                                            obj->obj_flags.value[0] += AchieBonus[y][ACHIE_AC_TABLE].lev31_40;
                                                        }
                                                    }
                                                }
                                                break;

                                            case 4:
                                                {
                                                    for(y = 0; y < MAX_CLASS; y++)
                                                    {
                                                        if(HasClass(ch, (1 << y)))
                                                        {
                                                            obj->obj_flags.value[0] += AchieBonus[y][ACHIE_AC_TABLE].lev41_50;
                                                        }
                                                    }
                                                }
                                                break;

                                            default:
                                                break;
                                        }
                                        obj->obj_flags.value[0] /= HowManyClasses(ch);
                                        if(vnum > 1385 && vnum <= 1390)
                                        {
                                            obj->obj_flags.value[0] += number(1, 2);
                                        }
                                        obj->obj_flags.value[1] = obj->obj_flags.value[0];
                                    }

                                default:
                                    break;
                            }
                        }

                        if(GET_ITEM_TYPE(obj) == ITEM_WEAPON)
                        {
                            if(IS_CASTER_N(ch))
                            {
                                obj->obj_flags.value[1] = 4;
                                obj->obj_flags.value[2] = 4;
                            }
                            else
                            {
                                obj->obj_flags.value[1] = 5;
                                obj->obj_flags.value[2] = 4;
                            }

                            if(GetMaxLevel(ch) < 21)
                            {
                                obj->obj_flags.value[1] -= 1;
                            }
                        }

                        if(GET_ITEM_TYPE(obj) == ITEM_STAFF || GET_ITEM_TYPE(obj) == ITEM_WAND)
                        {
                            const int randomSpell[] = {
                                SPELL_DISINTEGRATE,
                                SPELL_INCENDIARY_CLOUD,
                                SPELL_FIRESHIELD,
                                SPELL_HASTE,
                                SPELL_PROT_BREATH_FIRE,
                                SPELL_PROT_BREATH_ACID,
                                SPELL_PROT_BREATH_ELEC,
                                SPELL_PROT_BREATH_FROST,
                                SPELL_PROT_BREATH_GAS,
                                SPELL_WIZARDEYE,
                                SPELL_GLOBE_MAJOR_INV,
                                SPELL_SANCTUARY,
                                SPELL_FIREBALL,
                                SPELL_CHAIN_LIGHTNING,
                                SPELL_H_FEAST,
                                SPELL_ENERGY_DRAIN,
                                SPELL_FLY_GROUP,
                                SPELL_PARALYSIS,
                                SPELL_GLOBE_MINOR_INV,
                                SPELL_TRUE_SIGHT,
                                SPELL_SLOW,
                                SPELL_TELEPORT,
                                SPELL_MAJOR_TRACK,
                                SPELL_PRISMATIC_SPRAY,
                                SPELL_EARTHQUAKE,
                                SPELL_FIND_TRAPS,
                                SPELL_GUST_OF_WIND,
                                SPELL_DISPEL_MAGIC,
                                SPELL_POISON,
                                SPELL_BURNING_HANDS,
                                SPELL_STRENGTH,
                                SPELL_REMOVE_PARALYSIS,
                                SPELL_INVISIBLE,
                                SPELL_BARKSKIN,
                                SPELL_SHOCKING_GRASP,
                                SPELL_SHIELD
                            };

                            obj->obj_flags.value[0] = GetMaxLevel(ch);
                            obj->obj_flags.value[1] = MAX(5, number(1, int(GetMaxLevel(ch) / 2)));
                            obj->obj_flags.value[2] = obj->obj_flags.value[1];
                            obj->obj_flags.value[3] = randomSpell[number(0, 35)];
                        }

                        for(x = 0; x < MAX_OBJ_AFFECT; x++)
                        {
                            switch(apply[x])
                            {
                                case APPLY_NONE:
                                    break;

                                case OBJ_ARTIFACT:
                                    {
                                        SET_BIT(obj->obj_flags.extra_flags, ITEM_IMMUNE);
                                    }
                                    break;

                                default:
                                    {
                                        obj->affected[x].location = apply[x];
                                        obj->affected[x].modifier = bonus[x];
                                    }
                                    break;
                            }
                        }

                        switch(vnum)
                        {
                            case 1374:  //  finger
                                restringReward(obj, 0, 9, 12);
                                break;

                            case 1375:  //  wrist
                            case 1394:
                                restringReward(obj, 1, 9, 12);
                                break;

                            case 1376:  //  ear
                                restringReward(obj, 2, 9, 12);
                                break;

                            case 1377:  //  neck
                                restringReward(obj, 3, 11, 15);
                                break;

                            case 1378:  //  eyes
                            case 1395:
                                restringReward(obj, 4, 11, 15);
                                break;

                            case 1379:  //  hold
                                restringReward(obj, 5, 11, 15);
                                break;

                            case 1380:  //  light
                                restringReward(obj, 6, 14, 20);
                                break;

                            case 1381:  //  back
                                restringReward(obj, 7, 10, 13);
                                break;

                            case 1382:  //  about body
                                restringReward(obj, 8, 9, 12);
                                break;

                            case 1383:  //  waist
                                restringReward(obj, 9, 9, 12);
                                break;

                            case 1384:  //  feet
                                restringReward(obj, 10, 9, 12);
                                break;

                            case 1385:  //  hands
                                restringReward(obj, 11, 9, 12);
                                break;

                            case 1386:  //  body
                                restringReward(obj, 12, 10, 13);
                                break;

                            case 1387:  //  head
                                restringReward(obj, 13, 9, 12);
                                break;

                            case 1388:  //  legs
                                restringReward(obj, 14, 9, 12);
                                break;

                            case 1389:  //  arms
                                restringReward(obj, 15, 9, 12);
                                break;

                            case 1390:  //  shield
                            case 1396:  //  back
                                restringReward(obj, 16, 14, 20);
                                break;

                            case 1391:  //  weapon
                            {
                                weapon_rand = number(0, 2);

                                if(HasClass(ch, CLASS_THIEF) && weapon_rand != 2)
                                {
                                    weapon_rand = number(0, 2);
                                    if(weapon_rand != 2)
                                    {
                                        weapon_rand = number(0, 2);
                                    }
                                    if(weapon_rand != 2)
                                    {
                                        weapon_rand = number(0, 2);
                                    }
                                }

                                switch(weapon_rand)
                                {
                                    case 0:
                                    {
                                        switch(number(1, 6))
                                        {
                                            case 1:
                                                obj->obj_flags.value[3] = 7; /* BLUDGEON */
                                                break;

                                            case 2:
                                                obj->obj_flags.value[3] = 6; /* CRUSH */
                                                break;

                                            case 3:
                                                obj->obj_flags.value[3] = 9; /* BITE */
                                                break;

                                            case 4:
                                                obj->obj_flags.value[3] = 4; /* SMASH */
                                                break;

                                            case 5:
                                                obj->obj_flags.value[3] = 0; /* SMITE */
                                                break;

                                            case 6:
                                                obj->obj_flags.value[3] = 12; /* BLAST */
                                                break;

                                            default:
                                                obj->obj_flags.value[3] = 4; /* SMASH */
                                                break;
                                        }
                                        restringReward(obj, 17, 15, 21);
                                    }
                                        break;

                                    case 1:
                                    {
                                        switch(number(1, 4))
                                        {
                                            case 1:
                                                obj->obj_flags.value[3] = 3; /* SLASH */
                                                break;

                                            case 2:
                                                obj->obj_flags.value[3] = 2; /* WHIP */
                                                break;

                                            case 3:
                                                obj->obj_flags.value[3] = 5; /* CLEAVE */
                                                break;

                                            case 4:
                                                obj->obj_flags.value[3] = 8; /* CLAW */
                                                break;

                                            default:
                                                obj->obj_flags.value[3] = 3; /* SLASH */
                                                break;
                                        }
                                        restringReward(obj, 18, 19, 27);
                                    }
                                        break;

                                    case 2:
                                    {
                                        switch(number(1, 3))
                                        {
                                            case 1:
                                                obj->obj_flags.value[3] = 11; /* PIERCE */
                                                break;

                                            case 2:
                                                obj->obj_flags.value[3] = 10; /* STING */
                                                break;

                                            case 3:
                                                obj->obj_flags.value[3] = 1; /* STAB */
                                                break;

                                            default:
                                                obj->obj_flags.value[3] = 11; /* PIERCE */
                                                break;
                                        }
                                        restringReward(obj, 19, 15, 21);
                                    }
                                        break;

                                    default:
                                        break;
                                }
                            }
                                break;

                            case 1392:  //  wand
                                restringReward(obj, 20, 9, 12);
                                break;

                            case 1393:  //  staff
                                restringReward(obj, 21, 9, 12);
                                break;

                            default:
                                break;
                        }

                        SetPersonOnSave(ch, obj);

                        obj_to_char(obj, ch);

                        boost::format fmt("$c0010L'ombra di %s ti da' %s$c0010.\n\r");
                        fmt % rand_god[god] % obj->short_description;
                        sbch.append(fmt.str().c_str());
                        fmt.clear();
                        boost::format fmt2("$c0010L'ombra di %s da' %s$c0010 a $N.\n\r");
                        fmt2 % rand_god[god] % obj->short_description;
                        sbroom.append(fmt2.str().c_str());
                        fmt2.clear();
                        mudlog(LOG_PLAYERS, "%s won %s with Achievements.", GET_NAME(ch), obj->short_description);
                    }
                    break;

                case 3:     // reward bonus
                    {
                        struct obj_data* obj;
                        int reward, dust;
                        int dust_vnum = DUST_REWARD_STAFF_BASE; /* blu */
                        string oggetto;

                        dust = number(1, 11);
                        switch(dust)
                        {
                            case 1:
                            case 6:
                            case 8:
                                dust_vnum = DUST_REWARD_STAFF_BASE + 0; /* blu */
                                break;
                            case 2:
                            case 10:
                                dust_vnum = DUST_REWARD_STAFF_BASE + 1; /* viola */
                                break;
                            case 3:
                            case 7:
                            case 9:
                                dust_vnum = DUST_REWARD_STAFF_BASE + 2; /* rossa */
                                break;
                            case 4:
                            case 11:
                                dust_vnum = DUST_REWARD_STAFF_BASE + 3; /* gialla */
                                break;
                            case 5:
                                dust_vnum = DUST_REWARD_STAFF_BASE + 4; /* celeste */
                                break;
                            default:
                                dust_vnum = DUST_REWARD_STAFF_BASE;
                                break;
                        }

                        reward = real_object(dust_vnum);
                        obj = read_object(reward, REAL);
                        if(!obj) {
                            break;
                        }

                        SetPersonOnSave(ch, obj);

                        obj_to_char(obj, ch);

                        boost::format fmt("$c0011L'ombra di %s ti sorride, subito dopo fa apparire tra le tue mani %s.\n\r");
                        fmt % rand_god[god] % obj->short_description;
                        sbch.append(fmt.str().c_str());
                        fmt.clear();
                        boost::format fmt2("$c0011L'ombra di %s sorride a $N, poi improvvisamente %s $D appare tra le mani.\n\r");
                        fmt2 % rand_god[god] % obj->short_description;
                        sbroom.append(fmt2.str().c_str());
                        fmt2.clear();
                        mudlog(LOG_PLAYERS, "%s won %s with Achievements.", GET_NAME(ch), obj->short_description);
                    }
                    break;

                default:
                    break;
            }
        }
    }

    if(win)
    {
        send_to_char("Una figura ultraterrena appare improvvisamente di fronte a te.\n\r\n\r", ch);
        act("Una figura eterea appare improvvisamente di fronte a te.\n\r", FALSE, ch, 0, ch, TO_ROOM);
        act(sbch.c_str(), FALSE, ch, 0, 0, TO_CHAR);
        act(sbroom.c_str(), FALSE, ch, 0, ch, TO_ROOM);
        send_to_char("Poi, lentamente, la figura scompare ai tuoi occhi.\n\r", ch);
        act("Poi, lentamente, la figura scompare ai tuoi occhi.", FALSE, ch, 0, ch, TO_ROOM);
    }
    sbch.clear();
    sbroom.clear();
    return;
}
} // namespace

namespace {

struct AchiePotionSig {
	int v1;
	int v2;
	int v3;
	int vnum;
};

/* Ordine = switch achievement (case 0..17). */
constexpr AchiePotionSig kAchiePotionSigs[POT_REWARD_STAFF_COUNT] = {
	{28, -1, -1, 37051},   /* heal */
	{28, 28, -1, 37052},   /* heal heal */
	{28, 28, 28, 37053},   /* heal x3 */
	{36, -1, -1, 37054},   /* sanc */
	{81, -1, -1, 37055},   /* fireshield */
	{81, 36, -1, 37056},   /* fireshield sanc */
	{81, 28, -1, 37057},   /* fireshield heal */
	{28, 81, 36, 37058},   /* heal fireshield sanc */
	{100, -1, -1, 37059},  /* mana */
	{100, 100, -1, 37060}, /* mana mana */
	{100, 100, 100, 37061},/* mana x3 */
	{86, -1, -1, 37062},   /* second wind */
	{86, 108, -1, 37063},  /* second wind major track */
	{86, 108, 69, 37064},  /* + fly */
	{39, -1, -1, 37065},   /* strength */
	{92, 73, -1, 37066},   /* stone skin shield */
	{155, 28, 36, 37067},  /* haste heal sanc */
	{101, -1, -1, 37068},  /* astral */
};

int potion_vnum_from_values_impl(int v1, int v2, int v3) noexcept {
	for(const AchiePotionSig& sig : kAchiePotionSigs) {
		if(sig.v1 == v1 && sig.v2 == v2 && sig.v3 == v3) {
			return sig.vnum;
		}
	}
	return 0;
}

const AchiePotionSig* potion_sig_by_vnum(int vnum) noexcept {
	const int idx = vnum - POT_REWARD_STAFF_BASE;
	if(idx < 0 || idx >= POT_REWARD_STAFF_COUNT) {
		return nullptr;
	}
	return &kAchiePotionSigs[idx];
}

void copy_obj_strings_from(struct obj_data* dst, const struct obj_data* src) {
	if(!dst || !src) {
		return;
	}
	auto replace = [](char*& field, const char* val) {
		if(field) {
			free(field);
			field = nullptr;
		}
		if(val) {
			field = strdup(val);
		}
	};
	replace(dst->name, src->name);
	replace(dst->short_description, src->short_description);
	replace(dst->description, src->description);
}

bool achie_reward_rebind_to_vnum(struct obj_data* obj, int new_vnum) {
	if(!obj || new_vnum <= 0) {
		return false;
	}
	const int new_rnum = real_object(new_vnum);
	if(new_rnum < 0) {
		return false;
	}
	const int old_rnum = obj->item_number;
	if(old_rnum == new_rnum) {
		return true;
	}
	if(old_rnum >= 0 && old_rnum < top_of_objt && obj_index[old_rnum].number > 0) {
		obj_index[old_rnum].number--;
	}
	obj->item_number = new_rnum;
	obj_index[new_rnum].number++;
	obj->char_vnum = new_vnum;

	struct obj_data* proto = nullptr;
	bool proto_temp = false;
	if(obj_index[new_rnum].data) {
		proto = static_cast<struct obj_data*>(obj_index[new_rnum].data);
	}
	else {
		proto = read_object(new_rnum, REAL);
		proto_temp = (proto != nullptr);
	}
	if(proto) {
		copy_obj_strings_from(obj, proto);
		obj->obj_flags.type_flag = proto->obj_flags.type_flag;
		obj->obj_flags.extra_flags = proto->obj_flags.extra_flags;
		obj->obj_flags.extra_flags2 = proto->obj_flags.extra_flags2;
		obj->obj_flags.wear_flags = proto->obj_flags.wear_flags;
		obj->obj_flags.weight = proto->obj_flags.weight;
		obj->obj_flags.cost = proto->obj_flags.cost;
		obj->obj_flags.cost_per_day = proto->obj_flags.cost_per_day;
		if(proto_temp) {
			extract_obj(proto);
		}
	}
	return true;
}

} // namespace

namespace {
int achie_dust_vnum_from_obj(const struct obj_data* obj) noexcept {
	if(!obj) {
		return 0;
	}
	for(int i = 0; i < MAX_OBJ_AFFECT; ++i) {
		const int loc = obj->affected[i].location;
		const int mod = obj->affected[i].modifier;
		if(loc == APPLY_MANA && mod == 1) {
			return DUST_REWARD_STAFF_BASE + 0;
		}
		if(loc == APPLY_MANA_REGEN && mod == 1) {
			return DUST_REWARD_STAFF_BASE + 1;
		}
		if(loc == APPLY_HIT && mod == 1) {
			return DUST_REWARD_STAFF_BASE + 2;
		}
		if(loc == APPLY_HIT_REGEN && mod == 1) {
			return DUST_REWARD_STAFF_BASE + 3;
		}
		if(loc == APPLY_SPELLFAIL && mod == -1) {
			return DUST_REWARD_STAFF_BASE + 4;
		}
	}
	return 0;
}
} // namespace

bool is_achie_dust_vnum(int vnum) noexcept {
	if(vnum == OBJ_REWARD) {
		return true;
	}
	constexpr int dust_max = DUST_REWARD_STAFF_BASE + DUST_REWARD_STAFF_COUNT;
	return vnum >= DUST_REWARD_STAFF_BASE && vnum < dust_max;
}

bool achie_reward_remap_obj(struct obj_data* obj) {
	if(!obj || obj->item_number < 0 || obj->item_number >= top_of_objt) {
		return false;
	}
	const int vnum = obj_index[obj->item_number].iVNum;
	int new_vnum = 0;
	if(vnum == POT_REWARD) {
		new_vnum = potion_vnum_from_values_impl(obj->obj_flags.value[1],
												obj->obj_flags.value[2],
												obj->obj_flags.value[3]);
		if(new_vnum <= 0) {
			mudlog(LOG_CHECK,
				   "achie_reward_remap: unknown potion signature v1=%d v2=%d v3=%d",
				   obj->obj_flags.value[1], obj->obj_flags.value[2],
				   obj->obj_flags.value[3]);
			return false;
		}
		if(!achie_reward_rebind_to_vnum(obj, new_vnum)) {
			return false;
		}
		obj->obj_flags.value[0] = POT_REWARD_LEVEL_FIXED;
		if(const auto* sig = potion_sig_by_vnum(new_vnum)) {
			obj->obj_flags.value[1] = sig->v1;
			obj->obj_flags.value[2] = sig->v2;
			obj->obj_flags.value[3] = sig->v3;
		}
		return true;
	}
	if(vnum == OBJ_REWARD) {
		new_vnum = achie_dust_vnum_from_obj(obj);
		if(new_vnum <= 0) {
			mudlog(LOG_CHECK, "achie_reward_remap: unknown dust APPLY on legacy 1373");
			return false;
		}
		/* Preserve APPLY already overlaid; rebind refreshes strings/flags from proto. */
		struct obj_affected_type keep[MAX_OBJ_AFFECT];
		for(int i = 0; i < MAX_OBJ_AFFECT; ++i) {
			keep[i] = obj->affected[i];
		}
		if(!achie_reward_rebind_to_vnum(obj, new_vnum)) {
			return false;
		}
		for(int i = 0; i < MAX_OBJ_AFFECT; ++i) {
			obj->affected[i] = keep[i];
		}
		return true;
	}
	return false;
}

void restringReward(struct obj_data* obj, int obj_slot_number, int max_name, int val_random)
{
    int z, mn, zz;
    string oggetto;

    z = number(1, val_random);
    if(z >= max_name)
    {
        z = 0;
    }
    mn = number(0, 4);
    switch(mn)
    {
        case 0:
            zz = number(0, 15);
            break;

        case 1:
            zz = number(0, 16);
            break;

        case 2:
            zz = number(0, 28);
            break;

        case 3:
            zz = number(0, 23);
            break;

        case 4:
            zz = number(0, 14);
            break;

        default:
            zz = 0;
            break;
    }
    // short description
    if(GET_ITEM_TYPE(obj) == ITEM_LIGHT)
    {
        boost::format fmt("%s");
        fmt % EquipName[obj_slot_number][z].name;
        oggetto.append(fmt.str().c_str());
        fmt.clear();
    }
    else
    {
        boost::format fmt("%s %s");
        fmt % EquipName[obj_slot_number][z].name % (EquipName[obj_slot_number][z].gender == SEX_FEMALE ? MaterialName[mn][zz].fem_gen : EquipName[obj_slot_number][z].gender == SEX_NEUTRAL ? MaterialName[mn][zz].neu_gen : EquipName[obj_slot_number][z].gender == 4 ? MaterialName[mn][zz].m_fem_gen : MaterialName[mn][zz].m_neu_gen);
        oggetto.append(fmt.str().c_str());
        fmt.clear();
    }
    obj->short_description = (char*)strdup(oggetto.c_str());
    oggetto.clear();
    // long description
    if(GET_ITEM_TYPE(obj) == ITEM_LIGHT)
    {
        boost::format fmt1("%s %s qui per terra.");
        fmt1 % EquipName[obj_slot_number][z].name % (EquipName[obj_slot_number][z].gender < 4 ? "e'" : "sono");
        oggetto.append(fmt1.str().c_str());
        fmt1.clear();
    }
    else
    {
        boost::format fmt1("%s %s %s qui per terra.");
        fmt1 % EquipName[obj_slot_number][z].name % (EquipName[obj_slot_number][z].gender == SEX_FEMALE ? MaterialName[mn][zz].fem_gen : EquipName[obj_slot_number][z].gender == SEX_NEUTRAL ? MaterialName[mn][zz].neu_gen : EquipName[obj_slot_number][z].gender == 4 ? MaterialName[mn][zz].m_fem_gen : MaterialName[mn][zz].m_neu_gen) % (EquipName[obj_slot_number][z].gender < 4 ? "e'" : "sono");
        oggetto.append(fmt1.str().c_str());
        fmt1.clear();
    }
    obj->description = (char*)strdup(oggetto.c_str());
    oggetto.clear();
    // name
    if(GET_ITEM_TYPE(obj) == ITEM_LIGHT)
    {
        boost::format fmt2("%s");
        fmt2 % EquipName[obj_slot_number][z].key;
        oggetto.append(fmt2.str().c_str());
        fmt2.clear();
    }
    else
    {
        boost::format fmt2("%s%s");
        fmt2 % EquipName[obj_slot_number][z].key % MaterialName[mn][zz].key;
        oggetto.append(fmt2.str().c_str());
        fmt2.clear();
    }
    obj->name = (char*)strdup(oggetto.c_str());
    oggetto.clear();
}

namespace {
/** Livello raggiunto: il primo (da 1 a 10) il cui valore e' esattamente quello attuale. */
struct AchieLevelHit {
	int lvl;
	int value;
	std::string title;
};

[[nodiscard]] std::optional<AchieLevelHit> reached_achie_level(const ClassAchieTable& entry, int current) {
	const std::array<std::pair<const char*, int>, 10> levels{{
		{entry.lvl1, entry.lvl1_val}, {entry.lvl2, entry.lvl2_val},
		{entry.lvl3, entry.lvl3_val}, {entry.lvl4, entry.lvl4_val},
		{entry.lvl5, entry.lvl5_val}, {entry.lvl6, entry.lvl6_val},
		{entry.lvl7, entry.lvl7_val}, {entry.lvl8, entry.lvl8_val},
		{entry.lvl9, entry.lvl9_val}, {entry.lvl10, entry.lvl10_val}
	}};
	for(std::size_t i = 0; i < levels.size(); ++i) {
		if(current == levels[i].second) {
			const char* title = levels[i].first;
			return AchieLevelHit{static_cast<int>(i) + 1, levels[i].second, title ? title : ""};
		}
	}
	return std::nullopt;
}

/** Moltiplicatore XP (in decimi) per livello 1..10. */
[[nodiscard]] int achie_level_multiplier(int lvl) {
	static constexpr std::array<int, 10> molt{10, 15, 20, 25, 30, 35, 40, 45, 50, 60};
	return molt[static_cast<std::size_t>(lvl - 1)];
}

/** XP base per grado di difficolta'; nullopt se il grado non e' valido. */
[[nodiscard]] std::optional<int> achie_base_reward(const XpAchieTable& row, int grado_diff) {
	switch(grado_diff) {
	case LEV_BEGINNER:
		return row.lev_1_xp;
	case LEV_VERY_EASY:
		return row.lev_2_xp;
	case LEV_EASY:
		return row.lev_3_xp;
	case LEV_NORMAL:
		return row.lev_4_xp;
	case LEV_HARD:
		return row.lev_5_xp;
	case LEV_EXPERT:
		return row.lev_6_xp;
	case LEV_CHAMPION:
		return row.lev_7_xp;
	case LEV_IMPERIAL:
		return row.lev_8_xp;
	case LEV_TORMENT:
		return row.lev_9_xp;
	case LEV_GOD_MODE:
		return row.lev_10_xp;
	default:
		return std::nullopt;
	}
}

/** Bonus multiclasse: mono 100%, bi 150%, tri o piu' 200%. */
[[nodiscard]] int achie_class_scaled(int reward, int classes) {
	switch(classes) {
	case 1:
		return reward;
	case 2:
		return reward * 15 / 10;
	default:
		return reward * 2;
	}
}

/** XP finali: base * molt / 10 con variazione casuale del 5%. */
[[nodiscard]] int achie_random_reward(int reward, int molt) {
	const long long scaled_reward = (static_cast<long long>(reward) * static_cast<long long>(molt)) / 10LL;
	const long long reward_delta_ll = static_cast<long long>(std::llround(static_cast<double>(scaled_reward) * 0.05));
	const long long min_reward = std::max(scaled_reward - reward_delta_ll, static_cast<long long>(std::numeric_limits<int>::min()));
	const long long max_reward = std::min(scaled_reward + reward_delta_ll, static_cast<long long>(std::numeric_limits<int>::max()));
	return number(static_cast<int>(min_reward), static_cast<int>(max_reward));
}
} // namespace

/*
 * tch: corpo originale (contatori, flag, nome nel cartiglio);
 * ch: chi riceve testo, XP, premi e salvataggio.
 */
void CheckAchie(struct char_data* ch, int achievement_type, int achievement_class, int amount,
				bool defer_save)
{
	if(!IS_PC(ch)) {
		return;
	}

	char_data* tch = ch;
	if(IS_POLY(ch)) {
		if(!ch->desc || !ch->desc->original) {
			return;
		}
		tch = ch->desc->original;
	}

	if(!tch) {
		return;
	}

	const ClassAchieTable& entry = AchievementsList[achievement_class][achievement_type];
	if(entry.classe == -1) {
		// se l'achievement classe e' -1 non effettua il check
		return;
	}

	const int current = tch->specials.achievements[achievement_class][achievement_type];
	const int max_value = MaxValueAchievement(achievement_class, achievement_type, entry.n_livelli);

	const bool quest = achievement_class == QUEST_ACHIE &&
					   CheckMercyTable(tch, entry.achie_number, current);

	// se un pg arriva al valore massimo in un achievement mando una mail ai coder
	if(current == max_value) {
		const std::string mail = std::string(GET_NAME(tch)) + " ha raggiunto il valore massimo di '" +
								 (entry.achie_string2 ? entry.achie_string2 : "") + "' pari a '" +
								 format_thousands(current) + "'.\n\r\n\r";
		mail_to_god(ch, "Requiem", mail.c_str());
		mail_to_god(ch, "Croneh", mail.c_str());
	}

	const std::optional<AchieLevelHit> hit = reached_achie_level(entry, current);
	const int valore = hit ? hit->value : 0;

	if(hit && valore > 0 && amount > 0) {
		const char* raw_stringa = (hit->lvl == 1 && current == 1) ? entry.achie_string1 : entry.achie_string2;
		const std::string stringa = raw_stringa ? raw_stringa : "";

		send_to_char(spamAchie(tch, hit->title, valore, stringa).c_str(), ch);
		const std::string announce = "\n\r$c0009" + std::string(GET_NAME(ch)) +
									 " $c0007ha completato l'achievement $c0015'" + hit->title +
									 "$c0007' ($c0011" + format_thousands(valore) + " $c0009" + stringa +
									 "$c0007).\n\r\n\r";
		send_to_all_not_ch(ch, announce.c_str());

		const std::optional<int> base = achie_base_reward(RewardXp[GetMaxLevel(ch)], entry.grado_diff);
		if(!base) {
			mudlog(LOG_CHECK, "Achievement's difficulty level is missing for achievement type %d (class %d)", achievement_type, achievement_class);
			return;
		}

		const int reward = achie_random_reward(achie_class_scaled(*base, HowManyClasses(tch)),
											   achie_level_multiplier(hit->lvl));
		const long long exp_before = static_cast<long long>(GET_EXP(ch));
		gain_exp(ch, reward);
		const bool xp_gained = static_cast<long long>(GET_EXP(ch)) > exp_before;

		RewardAll(ch, achievement_type, achievement_class, hit->lvl);

		if(xp_gained) {
			send_to_char("\n\r", ch);
			const std::string xp_line = "$c0014Ricevi $c0015" + format_thousands(reward) +
										"$c0014 punti esperienza per aver completato l'achievement '$c0015" +
										hit->title + "$c0014'.";
			act(xp_line.c_str(), false, ch, nullptr, nullptr, TO_CHAR);
		}
		if(!defer_save) {
			obj_cost cost;
			save_obj(ch, &cost, 0);
		}
	}

	if(quest && amount > 0) {
		RewardQAchie(ch, entry.achie_number);
		const int mercy_count = ch->specials.mercy[entry.achie_number];
		const std::string mercy_line = "$c0014Congratulazioni! Hai ottenuto $c0015" + format_thousands(mercy_count) +
									   "$c0014 volt" + (mercy_count == 1 ? "a" : "e") +
									   " il premio con il Mercy System per la quest di $c0015" +
									   QuestNumber[entry.achie_number].mercy_name + "$c0014.\n\r";
		act(mercy_line.c_str(), false, ch, nullptr, nullptr, TO_CHAR);
		if(!defer_save) {
			obj_cost cost;
			save_obj(ch, &cost, 0);
		}
	}

	if(valore <= 0 && IS_SET(tch->player.user_flags, ACHIE_MODE)) {
		if((amount == 1 || amount == -1) && current <= max_value) {
			const std::string sb = AchievementNumber(tch, achievement_type, achievement_class);
			page_string(ch->desc, sb.c_str(), true);
		}
	}
}

bool CheckMercyTable(struct char_data* ch, int quest, int amount)
{
    int val = 0, num = 0, mercy[4], check;

    val = static_cast<int> (amount / QuestNumber[quest].mercy_max);
    check = number(1, 100);

    if(val > ch->specials.mercy[quest])
    {
//        mudlog(LOG_CHECK, "%s ha effettuato %d achievements per %s ed ha preso solo %d premi%s ha quindi vinto il premio!", GET_NAME(ch), amount, QuestNumber[quest].mercy_name, ch->specials.mercy[quest], (ch->specials.mercy[quest] == 1 ? "o," : ","));
        ch->specials.mercy[quest] += 1;
        return TRUE;
    }

//    mudlog(LOG_CHECK, "Il rapporto tra numero di achievements ed i premi ottenibili da %s e' pari a %d. Ne ha ottenuti %d per ora (%s).", GET_NAME(ch), val, ch->specials.mercy[quest], QuestNumber[quest].mercy_name);

    if(val > 0)
    {
        num = amount - (QuestNumber[quest].mercy_max * val);
//        mudlog(LOG_CHECK, "Il numero di achievements valutabili per %s su questo step del mercy system e' %d.", GET_NAME(ch), num);
    }
    else if(val == 0 && ch->specials.mercy[quest] == 0)
    {
        num = amount;
//        mudlog(LOG_CHECK, "Il numero di achievements valutabili per %s su questo step del mercy system e' %d (non ha mai ottenuto il premio).", GET_NAME(ch), num);
    }
    else
    {
//        mudlog(LOG_CHECK, "%s (%d achievements) ha gia' ottenuto %d premi%s per '%s' (mercy_max = %d).", GET_NAME(ch), amount, ch->specials.mercy[quest], (ch->specials.mercy[quest] == 1 ? "o" : ""), QuestNumber[quest].mercy_name, QuestNumber[quest].mercy_max);
        return FALSE;
    }

    if(num < QuestNumber[quest].mercy_min)
    {
//        mudlog(LOG_CHECK, "Il numero di achievements valutabili per %s su questo step del mercy system e' %d, ed e' inferiore al valore minimo che e' %d.", GET_NAME(ch), num, QuestNumber[quest].mercy_min);
        return FALSE;
    }

    mercy[0] = (QuestNumber[quest].mercy_1 - QuestNumber[quest].mercy_min) * QuestNumber[quest].increment_min;
    mercy[1] = (QuestNumber[quest].mercy_2 - QuestNumber[quest].mercy_1) * QuestNumber[quest].increment_1;
    mercy[2] = (QuestNumber[quest].mercy_3 - QuestNumber[quest].mercy_2) * QuestNumber[quest].increment_2;
    mercy[3] = (QuestNumber[quest].mercy_4 - QuestNumber[quest].mercy_3) * QuestNumber[quest].increment_3;

    if(num >= QuestNumber[quest].mercy_min && num < QuestNumber[quest].mercy_1)
    {
        val = (num - QuestNumber[quest].mercy_min) * QuestNumber[quest].increment_min;
    }
    else if(num >= QuestNumber[quest].mercy_1 && num < QuestNumber[quest].mercy_2)
    {
        val = ((num - QuestNumber[quest].mercy_1) * QuestNumber[quest].increment_1) + mercy[0];
    }
    if(num >= QuestNumber[quest].mercy_2 && num < QuestNumber[quest].mercy_3)
    {
        val = ((num - QuestNumber[quest].mercy_2) * QuestNumber[quest].increment_2) + mercy[0] + mercy[1];
    }
    if(num >= QuestNumber[quest].mercy_3 && num < QuestNumber[quest].mercy_4)
    {
        val = ((num - QuestNumber[quest].mercy_3) * QuestNumber[quest].increment_3) + mercy[0] + mercy[1] + mercy[2];
    }
    if(num >= QuestNumber[quest].mercy_4 && num < QuestNumber[quest].mercy_max)
    {
        val = ((num - QuestNumber[quest].mercy_4) * QuestNumber[quest].increment_4) + mercy[0] + mercy[1] + mercy[2] + mercy[3];
    }
//    mudlog(LOG_CHECK, "Il numero di achievements valutabili per %s su questo step del mercy system e' %d, ha quindi una probabilita' pari a %d/100.", GET_NAME(ch), num, val);

//    mudlog(LOG_CHECK, "Il valore di check per %s e' di %d quindi...", GET_NAME(ch), check);

    if(val > check)
    {
//        mudlog(LOG_CHECK, "%s vince il premio!", GET_NAME(ch));
        ch->specials.mercy[quest] += 1;
        return TRUE;
    }

//    mudlog(LOG_CHECK, "%s non vince un tubo!", GET_NAME(ch));
    return FALSE;
}

int CheckMobQuest(int vnumber)
{
    int i;

    for(i = 0; i < MAX_QUEST_ACHIE; i++)
    {
        if(QuestMobAchie[i].mob_0 == vnumber && QuestMobAchie[i].mob_0 != 0)
        {
            return i;
        }
        if(QuestMobAchie[i].mob_1 == vnumber && QuestMobAchie[i].mob_1 != 0)
        {
            return i;
        }
        if(QuestMobAchie[i].mob_2 == vnumber && QuestMobAchie[i].mob_2 != 0)
        {
            return i;
        }
        if(QuestMobAchie[i].mob_3 == vnumber && QuestMobAchie[i].mob_3 != 0)
        {
            return i;
        }
        if(QuestMobAchie[i].mob_4 == vnumber && QuestMobAchie[i].mob_4 != 0)
        {
            return i;
        }
        if(QuestMobAchie[i].mob_5 == vnumber && QuestMobAchie[i].mob_5 != 0)
        {
            return i;
        }
        if(QuestMobAchie[i].mob_6 == vnumber && QuestMobAchie[i].mob_6 != 0)
        {
            return i;
        }
        if(QuestMobAchie[i].mob_7 == vnumber && QuestMobAchie[i].mob_7 != 0)
        {
            return i;
        }
        if(QuestMobAchie[i].mob_8 == vnumber && QuestMobAchie[i].mob_8 != 0)
        {
            return i;
        }
        if(QuestMobAchie[i].mob_9 == vnumber && QuestMobAchie[i].mob_9 != 0)
        {
            return i;
        }
    }

    return -1;
}

void AssignMobQuestToToon(struct char_data* ch, int quest, int vnumber)
{
    int i;
    struct char_data* tch;

    tch = ch;

    if(!IS_PC(ch))
    {
        return;
    }

    if(IS_POLY(tch))
    {
        tch = ch->desc->original;
    }

    for(i = 0; i < QuestMobAchie[quest].numero_mob; i++)
    {
        if(tch->specials.quest_mob[quest][i] == vnumber && vnumber > 0)
        {
            // ho gia' ucciso questo mob
            return;
        }
    }

    for(i = 0; i < QuestMobAchie[quest].numero_mob; i++)
    {
        if(tch->specials.quest_mob[quest][i] == 0)
        {
            tch->specials.quest_mob[quest][i] = vnumber;
            return;
        }
    }
}

bool CheckQuest(struct char_data* ch, int quest_number)
{
    int i, check[10] = {0,0,0,0,0,0,0,0,0,0}, totale = 0;
    struct char_data* tch;

    tch = ch;

    if(!IS_PC(ch))
    {
        return FALSE;
    }

    if(IS_POLY(tch))
    {
        tch = ch->desc->original;
    }

    if(QuestMobAchie[quest_number].numero_mob == 10)
    {
        for(i = 0; i < quest_number; i++)
        {
            if(tch->specials.quest_mob[quest_number][i] == QuestMobAchie[quest_number].mob_9)
            {
                // trovato il mob
                check[9] = 1;
            }
        }
    }
    if(QuestMobAchie[quest_number].numero_mob >= 9)
    {
        for(i = 0; i < 10; i++)
        {
            if(tch->specials.quest_mob[quest_number][i] == QuestMobAchie[quest_number].mob_8)
            {
                // trovato il mob
                check[8] = 1;
            }
        }
    }
    if(QuestMobAchie[quest_number].numero_mob >= 8)
    {
        for(i = 0; i < 10; i++)
        {
            if(tch->specials.quest_mob[quest_number][i] == QuestMobAchie[quest_number].mob_7)
            {
                // trovato il mob
                check[7] = 1;
            }
        }
    }
    if(QuestMobAchie[quest_number].numero_mob >= 7)
    {
        for(i = 0; i < 10; i++)
        {
            if(tch->specials.quest_mob[quest_number][i] == QuestMobAchie[quest_number].mob_6)
            {
                // trovato il mob
                check[6] = 1;
            }
        }
    }
    if(QuestMobAchie[quest_number].numero_mob >= 6)
    {
        for(i = 0; i < 10; i++)
        {
            if(tch->specials.quest_mob[quest_number][i] == QuestMobAchie[quest_number].mob_5)
            {
                // trovato il mob
                check[5] = 1;
            }
        }
    }
    if(QuestMobAchie[quest_number].numero_mob >= 5)
    {
        for(i = 0; i < 10; i++)
        {
            if(tch->specials.quest_mob[quest_number][i] == QuestMobAchie[quest_number].mob_4)
            {
                // trovato il mob
                check[4] = 1;
            }
        }
    }
    if(QuestMobAchie[quest_number].numero_mob >= 4)
    {
        for(i = 0; i < 10; i++)
        {
            if(tch->specials.quest_mob[quest_number][i] == QuestMobAchie[quest_number].mob_3)
            {
                // trovato il mob
                check[3] = 1;
            }
        }
    }
    if(QuestMobAchie[quest_number].numero_mob >= 3)
    {
        for(i = 0; i < 10; i++)
        {
            if(tch->specials.quest_mob[quest_number][i] == QuestMobAchie[quest_number].mob_2)
            {
                // trovato il mob
                check[2] = 1;
            }
        }
    }
    if(QuestMobAchie[quest_number].numero_mob >= 2)
    {
        for(i = 0; i < 10; i++)
        {
            if(tch->specials.quest_mob[quest_number][i] == QuestMobAchie[quest_number].mob_1)
            {
                // trovato il mob
                check[1] = 1;
            }
        }
    }
    if(QuestMobAchie[quest_number].numero_mob >= 1)
    {
        for(i = 0; i < 10; i++)
        {
            if(tch->specials.quest_mob[quest_number][i] == QuestMobAchie[quest_number].mob_0)
            {
                // trovato il mob
                check[0] = 1;
            }
        }
    }

    for(i = 0; i < 10; i++)
    {
        totale += check[i];
    }

    if(totale == QuestMobAchie[quest_number].numero_mob)
    {
        // tutti i mob uccisi, azzero i contatori
        for(i = 0; i < 10; i++)
        {
            tch->specials.quest_mob[quest_number][i] = 0;
        }
        do_save(tch, "", 0);
        return TRUE;
    }

    return FALSE;
}

namespace {

const char* AchievementClasseLabelForDifficulty(int classe) {
	if(classe == 0) {
		return "tutte le classi";
	}
	if(classe == -1) {
		return "non abilitato";
	}
	return pc_class_types[classe];
}

struct AchievementMissingDiffScan {
	int category;
	int beginIndex;
	int endExclusive;
	const char* rowFormat;
	const char* typeLabel;
};

static constexpr std::array<AchievementMissingDiffScan, 5> kAchievementMissingDiffScans = {{
    {RACESLAYER_ACHIE, 0, MAX_RACE_ACHIE,
     "$c0009%6d $c0010%-12s %6d %-34s %s\n\r", "Raceslayer:"},
    {BOSSKILL_ACHIE, 0, MAX_BOSS_ACHIE,
     "$c0009%6d $c0011%-12s %6d %-34s %s\n\r", "Bosskill:"},
    {CLASS_ACHIE, 1, MAX_CLASS_ACHIE,
     "$c0009%6d $c0012%%-12s %6d %-34s %s\n\r", "Class:"},
    {QUEST_ACHIE, 0, MAX_QUEST_ACHIE,
     "$c0009%6d $c0013%-12s %6d %-34s %s\n\r", "Quest:"},
    {OTHER_ACHIE, 0, MAX_OTHER_ACHIE,
     "$c0009%6d $c0014%-12s %6d %-34s %s\n\r", "Other:"},
}};

void AppendAchievementRowsWithUnsetDifficulty(std::string& sb, int& rowNumber) {
	for(const AchievementMissingDiffScan& scan : kAchievementMissingDiffScans) {
		for(int i = scan.beginIndex; i < scan.endExclusive; ++i) {
			const ClassAchieTable& entry = AchievementsList[scan.category][i];
			if(entry.grado_diff != 0) {
				continue;
			}
			++rowNumber;
			boost::format fmt(scan.rowFormat);
			fmt % rowNumber % scan.typeLabel % entry.achie_number % entry.achie_string2
			    % AchievementClasseLabelForDifficulty(entry.classe);
			sb.append(fmt.str().c_str());
			fmt.clear();
		}
	}
}

void AppendAchievementSectionBanner(std::string& sb, const unsigned dashCount, const char* title) {
	sb.append("$c0009");
	sb.append(dashCount, '-').append(" $c0015");
	sb.append(title);
	sb.append("$c0009 ");
	sb.append(dashCount, '-').append("\n\r");
}

int AchievementThresholdForTier(const ClassAchieTable& entry, const int tier) {
	switch(tier) {
	case 10:
		return entry.lvl10_val;
	case 9:
		return entry.lvl9_val;
	case 8:
		return entry.lvl8_val;
	case 7:
		return entry.lvl7_val;
	case 6:
		return entry.lvl6_val;
	case 5:
		return entry.lvl5_val;
	case 4:
		return entry.lvl4_val;
	case 3:
		return entry.lvl3_val;
	case 2:
		return entry.lvl2_val;
	case 1:
		return entry.lvl1_val;
	default:
		return entry.lvl1_val;
	}
}

void AppendCompactAchievementForIndex(std::string& sb, int& num, char_data* tch, const int category,
                                    const int index) {
	const int progress = tch->specials.achievements[category][index];
	if(progress <= 0) {
		return;
	}
	const ClassAchieTable& entry = AchievementsList[category][index];
	for(int tier = 10; tier >= 1; --tier) {
		if(progress >= AchievementThresholdForTier(entry, tier) && entry.n_livelli >= tier) {
			num += 1;
			sb.append(bufferAchie(tch, index, category, tier, num, false, 0));
			num += tier - 1;
			return;
		}
	}
}

void AppendCompactAchievementCategory(std::string& sb, int& num, char_data* tch, const int category,
                                     const int indexFrom, const int indexToExclusive,
                                     const int hasAchievementBannerFlag, const unsigned dashWidth,
                                     const char* sectionTitle) {
	if(hasAchievement(tch, category, hasAchievementBannerFlag)) {
		AppendAchievementSectionBanner(sb, dashWidth, sectionTitle);
	}
	for(int i = indexFrom; i < indexToExclusive; ++i) {
		AppendCompactAchievementForIndex(sb, num, tch, category, i);
	}
}

void AppendFullAchievementRowRaceBossQuestOther(std::string& sb, int& num, char_data* tch,
                                                char_data* ch, const int category, const int index) {
	if(AchievementsList[category][index].classe == -1) {
		return;
	}
	if(HasClass(tch, AchievementsList[category][index].classe) || IS_QUESTMASTER(ch)) {
		num += 1;
		sb.append(bufferAchie(tch, index, category, AchievementsList[category][index].n_livelli, num, true,
		                      0));
		num += AchievementsList[category][index].n_livelli - 1;
	}
	else if(tch->specials.achievements[category][index] > 0) {
		num += 1;
		sb.append(bufferAchie(tch, index, category, AchievementsList[category][index].n_livelli, num, true,
		                      0));
		num += AchievementsList[category][index].n_livelli - 1;
	}
}

void AppendFullAchievementRowClass(std::string& sb, int& num, char_data* tch, const int index) {
	if(AchievementsList[CLASS_ACHIE][index].classe == -1) {
		return;
	}
	if(HasClass(tch, AchievementsList[CLASS_ACHIE][index].classe) ||
	   AchievementsList[CLASS_ACHIE][index].classe == 0) {
		num += 1;
		sb.append(bufferAchie(tch, index, CLASS_ACHIE, AchievementsList[CLASS_ACHIE][index].n_livelli, num,
		                      true, 0));
		num += AchievementsList[CLASS_ACHIE][index].n_livelli - 1;
	}
}

void AppendFullListCategoryRaceBossQuestOther(std::string& sb, int& num, char_data* tch, char_data* ch,
                                              const int category, const int indexFrom,
                                              const int indexToExclusive, const unsigned dashWidth,
                                              const char* sectionTitle) {
	if(hasAchievement(tch, category, 1) || IS_QUESTMASTER(ch)) {
		AppendAchievementSectionBanner(sb, dashWidth, sectionTitle);
	}
	for(int i = indexFrom; i < indexToExclusive; ++i) {
		AppendFullAchievementRowRaceBossQuestOther(sb, num, tch, ch, category, i);
	}
}

void AppendFullListCategoryClass(std::string& sb, int& num, char_data* tch, char_data* ch,
                                 const int indexFrom, const int indexToExclusive,
                                 const unsigned dashWidth, const char* sectionTitle) {
	if(hasAchievement(tch, CLASS_ACHIE, 1) || IS_QUESTMASTER(ch)) {
		AppendAchievementSectionBanner(sb, dashWidth, sectionTitle);
	}
	for(int i = indexFrom; i < indexToExclusive; ++i) {
		AppendFullAchievementRowClass(sb, num, tch, i);
	}
}

} // namespace

ACTION_FUNC(do_checkachielevel)
{
    if(ch == nullptr) {
        mudlog(LOG_SYSERR, "ch==nullptr in do_checkachielevel (act.info.cpp)");
        return;
    }
    if(ch->desc == nullptr) {
        return;
    }

    static constexpr std::array<const char*, 3> kAchievementLevelReviewers = {{
        "Croneh", "Alar", "Requiem"}};
    const char* const playerName = GET_NAME(ch);
    const bool isReviewer = std::any_of(
        kAchievementLevelReviewers.begin(),
        kAchievementLevelReviewers.end(),
        [playerName](const char* allowedName) {
            return playerName != nullptr && allowedName != nullptr &&
                   std::string_view(playerName) == std::string_view(allowedName);
        });
    if(!isReviewer) {
        send_to_char("Pardon?\n\r", ch);
        return;
    }

    int num = 0;
    std::string sb;

    sb.append("$c0011I seguenti achievement non hanno impostato il grado di difficolta':\n\r");
    AppendAchievementRowsWithUnsetDifficulty(sb, num);
    page_string(ch->desc, sb.c_str(), true);
}

ACTION_FUNC(do_achievements)
{
    if(ch == nullptr) {
        mudlog(LOG_SYSERR, "ch==nullptr in do_achievements (act.info.cpp)");
        return;
    }
    if(ch->desc == nullptr) {
        return;
    }
    if(!IS_PC(ch)) {
        return;
    }

    std::array<char, 128> arg1{};
    std::array<char, 128> arg2{};
    std::string sb;

    arg = one_argument(arg, arg1.data());
    const auto isCommand = [&](std::string_view cmd) {
        return std::string_view(arg1.data()) == cmd;
    };
    const auto applyPolyOriginalOrAbort = [&](char_data*& who) -> bool {
        if(!IS_POLY(who)) {
            return true;
        }
        if(who->desc == nullptr || who->desc->original == nullptr) {
            mudlog(LOG_SYSERR,
                   "IS_POLY without desc/original in do_achievements (applyPolyOriginalOrAbort)");
            send_to_char("In questo momento non riesci a identificarlo.\n\r", ch);
            return false;
        }
        who = who->desc->original;
        return true;
    };
    const auto resolveVisiblePlayerTarget = [&](char_data*& target, const char* name, const char* notFoundMsg, const char* notPcMsg) {
        target = get_char_vis_world(ch, name, nullptr);
        if(target == nullptr)
        {
            send_to_char(notFoundMsg, ch);
            return false;
        }

        if(!IS_PC(target))
        {
            send_to_char(notPcMsg, ch);
            return false;
        }

        return applyPolyOriginalOrAbort(target);
    };
    const auto parsePositiveIntArg = [&](const char* rawValue,
                                         const char* invalidMsg,
                                         const char* tooHighMsg,
                                         const char* nonPositiveMsg,
                                         int& outValue) {
        char* parseEnd = nullptr;
        errno = 0;
        const long long parsedValue = std::strtoll(rawValue, &parseEnd, 10);
        if(parseEnd == rawValue || *parseEnd != '\0')
        {
            send_to_char(invalidMsg, ch);
            return false;
        }
        if(errno == ERANGE || parsedValue > static_cast<long long>(std::numeric_limits<int>::max()))
        {
            send_to_char(tooHighMsg, ch);
            return false;
        }
        if(parsedValue <= 0)
        {
            send_to_char(nonPositiveMsg, ch);
            return false;
        }
        outValue = static_cast<int>(parsedValue);
        return true;
    };
    struct AchievementSlot
    {
        int category;
        int index;
        int baseNum;
        int levels;
    };
    constexpr std::array<int, 5> achievementCategoryOrder = {{
        RACESLAYER_ACHIE, BOSSKILL_ACHIE, CLASS_ACHIE, QUEST_ACHIE, OTHER_ACHIE
    }};
    const auto categoryStartIndex = [](int category) {
        return (category == CLASS_ACHIE) ? 1 : 0;
    };
    const auto categoryMaxCount = [](int category) {
        switch(category)
        {
            case RACESLAYER_ACHIE: return MAX_RACE_ACHIE;
            case BOSSKILL_ACHIE:   return MAX_BOSS_ACHIE;
            case CLASS_ACHIE:      return MAX_CLASS_ACHIE;
            case QUEST_ACHIE:      return MAX_QUEST_ACHIE;
            case OTHER_ACHIE:      return MAX_OTHER_ACHIE;
            default:               return 0;
        }
    };
    const auto isCategorySlotVisible = [&](int category, int index, char_data* target, char_data* actor) {
        if(AchievementsList[category][index].classe == -1)
        {
            return false;
        }
        if(category == CLASS_ACHIE)
        {
            return HasClass(target, AchievementsList[category][index].classe) ||
                   AchievementsList[category][index].classe == 0;
        }
        return HasClass(target, AchievementsList[category][index].classe) || IS_QUESTMASTER(actor);
    };
    const auto hasCategoryProgress = [&](int category, int index, char_data* target) {
        if(category == CLASS_ACHIE)
        {
            return false;
        }
        return target->specials.achievements[category][index] > 0;
    };
    const auto findAchievementSlotByNumber = [&](char_data* target,
                                                 char_data* actor,
                                                 const int requestedNumber,
                                                 const bool includeProgressFallback)
                                                 -> std::optional<AchievementSlot> {
        int num = 0;
        for(const int category : achievementCategoryOrder)
        {
            const int start = categoryStartIndex(category);
            const int maxCount = categoryMaxCount(category);
            for(int index = start; index < maxCount; ++index)
            {
                const bool visible = isCategorySlotVisible(category, index, target, actor);
                const bool includeByProgress = includeProgressFallback &&
                                               hasCategoryProgress(category, index, target);
                if(!visible && !includeByProgress)
                {
                    continue;
                }

                const int levels = AchievementsList[category][index].n_livelli;
                if(requestedNumber > num && requestedNumber <= (num + levels))
                {
                    return AchievementSlot{category, index, num, levels};
                }
                num += levels;
            }
        }
        return std::nullopt;
    };

    if(arg1[0] != '\0')
    {
        if(isCommand("all"))
        {
            char_data* tch = ch;

            if(IS_QUESTMASTER(ch))
            {
                arg = one_argument(arg, arg2.data());

                if(arg2[0] != '\0')
                {
                    tch = get_char_vis_world(ch, arg2.data(), nullptr);
                    if(tch == nullptr)
                    {
                        send_to_char("Non c'e' nessuno con quel nome qui.\n\r", ch);
                        return;
                    }
                }
            }

            if(!IS_PC(tch))
            {
                send_to_char("Vuoi veramente vedere gli achievements di uno stupido mob?\n\r", ch);
                return;
            }

            if(!applyPolyOriginalOrAbort(tch)) {
                return;
            }

            if(IS_SET(tch->specials.act, PLR_ACHIE) && (hasAchievement(tch, RACESLAYER_ACHIE, 1) || hasAchievement(tch, QUEST_ACHIE, 1) || hasAchievement(tch, OTHER_ACHIE, 1) || hasAchievement(tch, BOSSKILL_ACHIE, 1) || hasAchievement(tch, CLASS_ACHIE, 1)))
            {
                int num = 0;

                if(ch == tch)
                {
                    send_to_char("\n\r$c0011Ecco tutti i tuoi achievements:\n\r", ch);
                }
                else
                {
                    act("\n\r$c0011Lista completa degli achievements di $c0015$N$c0011:", false, ch, nullptr, tch, TO_CHAR);
                }

                AppendFullListCategoryRaceBossQuestOther(sb, num, tch, ch, RACESLAYER_ACHIE, 0, MAX_RACE_ACHIE,
                                                         45u, "Race  Achievements");
                AppendFullListCategoryRaceBossQuestOther(sb, num, tch, ch, BOSSKILL_ACHIE, 0, MAX_BOSS_ACHIE,
                                                         45u, "Boss  Achievements");
                AppendFullListCategoryClass(sb, num, tch, ch, 1, MAX_CLASS_ACHIE, 45u, "Skill Achievements");
                AppendFullListCategoryRaceBossQuestOther(sb, num, tch, ch, QUEST_ACHIE, 0, MAX_QUEST_ACHIE,
                                                         45u, "Quest Achievements");
                AppendFullListCategoryRaceBossQuestOther(sb, num, tch, ch, OTHER_ACHIE, 0, MAX_OTHER_ACHIE,
                                                         45u, "Other Achievements");

                page_string(ch->desc, sb.c_str(), true);
            }
            else if(ch == tch)
            {
                send_to_char("Non hai completato nessun achievement.\n\r", ch);
            }
            else
            {
                act("$N non ha completato nessun achievement.", false, ch, nullptr, tch, TO_CHAR);
            }
        }
        else if(isCommand("spam"))
        {
            char_data* tch = ch;

            static const std::array<const char*, 20> rand_spamAchie = {{
                "$c0011$n$c0007 e' proprio $u sborone:",
                "$c0011$n$c0007 si gigioneggia mostrandoti il suo achievement:",
                "$c0011$n$c0007 ti dice: 'Si ho fatto anche questo!',",
                "$c0011$n$c0007 mostra a tutti il cartello 'W i criceti' e poi dice:",
                "$c0011$n$c0007 canticchia allegr$b 'In the name of my new Achievemeeent!':",
                "$c0011$n$c0007 sogghigna mentre ti mostra che",
                "Inizi a schiumare di rabbia appena $c0011$n$c0007 ti dice che",
                "Ma davero davero? Nuovo achievement di $c0011$n$c0007? WTF,",
                "$c0011$n$c0007 si muove come $u supercafone mentre esclama che",
                "Mentre tu stai a rosica' $c0011$n$c0007 urla 'Daje!',",
                "Fai lo sborone e mostri a tutti che",
                "Ti gigioneggi mentre fai vedere il tuo achievement:",
                "Dici: 'E anche questo l'ho fatto,",
                "Mostri il cartello 'W i criceti', poi dici che",
                "Canti allegramente 'In the name of my new Achievemeeeent!':",
                "Sogghigni mentre mostri a tutti che",
                "Fai schiumare i presenti dalla rabbia appena esclami che",
                "Fai rimanere tutti a bocca aperta:",
                "Con il passo da $c5011K$c0007oatto esclami che",
                "Esclami 'Daje!' e fai rosicare tutti urlando che"
            }};

            constexpr std::size_t spamVariantsPerTarget = rand_spamAchie.size() / 2;
            constexpr int nMaxSpamAchie = static_cast<int>(spamVariantsPerTarget - 1);

            arg = one_argument(arg, arg2.data());
            while(*arg == ' ')
            {
                ++arg;
            }

            if(*arg != '\0')
            {
                send_to_char("Sintassi: achievements spam <numero>\n\r", ch);
                return;
            }

            if(arg2[0] == '\0')
            {
                send_to_char("Quale achievement vuoi mostrare? Digita 'achievements all' per vedere il numero.\n\r", ch);
                return;
            }

            int n_spam = 0;
            if(!parsePositiveIntArg(arg2.data(),
                "Il numero achievement deve essere un intero valido.\n\r",
                "Il numero achievement inserito e' troppo alto.\n\r",
                "Quale achievement vuoi mostrare? Digita 'achievements all' per vedere il numero.\n\r",
                n_spam))
            {
                return;
            }

            if(!applyPolyOriginalOrAbort(tch)) {
                return;
            }

            if(n_spam > maxAchievements(tch))
            {
                std::string msgToChar = "Il numero massimo di achievements per te e' ";
                msgToChar += std::to_string(maxAchievements(tch));
                msgToChar += ".\n\r";
                send_to_char(msgToChar.c_str(), ch);
                return;
            }

            if(IS_SET(tch->specials.act, PLR_ACHIE))
            {
                const auto slot = findAchievementSlotByNumber(tch, tch, n_spam, true);
                if(!slot.has_value())
                {
                    send_to_char("Quell'achievement esiste solo nella tua fantasia...\n\r", ch);
                    return;
                }
                const int displayNum = slot->baseNum + 1;
                sb = bufferAchie(tch,
                                 slot->index,
                                 slot->category,
                                 slot->levels,
                                 displayNum,
                                 true,
                                 n_spam);

                const int spamIndex = number(0, nMaxSpamAchie);
                const std::size_t spamIndexSz = static_cast<std::size_t>(spamIndex);
                constexpr std::size_t selfSpamOffset = spamVariantsPerTarget;
                constexpr char completedPrefix[] = "com";
                constexpr std::size_t completedPrefixLen = sizeof(completedPrefix) - 1;
                const bool completedVerb = (sb.compare(0, completedPrefixLen, completedPrefix) == 0);
                const char* const toCharAux = completedVerb ? "hai" : "ti";
                const char* const toRoomAux = completedVerb ? "ha" : "$d";
                const auto emitSpamLine = [&](const char* prefix, const char* aux, int target) {
                    std::string spamLine = prefix;
                    spamLine += ' ';
                    spamLine += aux;
                    spamLine += ' ';
                    spamLine += sb;
                    spamLine += '.';
                    act(spamLine.c_str(), false, ch, nullptr, nullptr, target);
                };

                emitSpamLine(rand_spamAchie[spamIndexSz + selfSpamOffset], toCharAux, TO_CHAR);
                emitSpamLine(rand_spamAchie[spamIndexSz], toRoomAux, TO_ROOM);
            }
            else
            {
                send_to_char("Ma se non hai completato nessun achievement...\n\r", ch);
            }
        }
        else if(isCommand("delete") && IS_QUESTMASTER(ch))
        {
            char_data* tch = nullptr;
            std::array<char, 128> arg3{};
            std::string msgToChar;
            std::string msgToVict;

            arg = one_argument(arg, arg2.data());
            arg = one_argument(arg, arg3.data());
            while(*arg == ' ')
            {
                ++arg;
            }

            if(*arg != '\0')
            {
                send_to_char("Sintassi: achievements delete <nome_pg> <numero>\n\r", ch);
                return;
            }

            if(arg2[0] != '\0')
            {
                if(!resolveVisiblePlayerTarget(tch, arg2.data(),
                    "Non c'e' nessuno con quel nome qui, a chi vorresti cancellare gli achievements?\n\r",
                    "Vuoi veramente cancellare gli achivements ad uno stupido mob?\n\r"))
                {
                    return;
                }

                if(arg3[0] != '\0')
                {
                    int achi_d = 0;
                    if(!parsePositiveIntArg(arg3.data(),
                        "Il numero achievement deve essere un intero valido.\n\r",
                        "Il numero achievement inserito e' troppo alto.\n\r",
                        "Quale achievement di $N vuoi cancellare? Il numero lo vedi con $c0015achievement all $N$c0007.\n\r",
                        achi_d))
                    {
                        return;
                    }

                    if(achi_d > maxAchievements(tch))
                    {
                        std::string msgToChar = "Il numero massimo di achievements per $N e' ";
                        msgToChar += std::to_string(maxAchievements(tch));
                        msgToChar += ".";
                        act(msgToChar.c_str(), false, ch, nullptr, tch, TO_CHAR);
                        return;
                    }

                    if(IS_SET(tch->specials.act, PLR_ACHIE))
                    {
                        const auto setAlreadyZeroMsg = [&](const char* achievementName) {
                            msgToChar = "L'achievement del numero di ";
                            msgToChar += achievementName;
                            msgToChar += " di $N e' gia' 0.";
                        };
                        const auto setDeletedMsgs = [&](const char* achievementName) {
                            msgToChar = "Hai cancellato l'achievement del numero di ";
                            msgToChar += achievementName;
                            msgToChar += " di $N.";
                            msgToVict = "$n ti ha cancellato l'achievement del numero di ";
                            msgToVict += achievementName;
                            msgToVict += ".";
                        };
                        const auto slot = findAchievementSlotByNumber(tch, ch, achi_d, false);
                        if(!slot.has_value())
                        {
                            act("$N non ha completato nessun achievement!", false, ch, nullptr, tch, TO_CHAR);
                            return;
                        }
                        const int category = slot->category;
                        const int index = slot->index;
                        if(tch->specials.achievements[category][index] == 0)
                        {
                            setAlreadyZeroMsg(AchievementsList[category][index].achie_string2);
                            act(msgToChar.c_str(), false, ch, nullptr, tch, TO_CHAR);
                            return;
                        }
                        tch->specials.achievements[category][index] = 0;
                        mudlog(LOG_PLAYERS, "%s deleted the achievement related on '%s' on %s",
                               GET_NAME(ch), AchievementsList[category][index].achie_string2, GET_NAME(tch));
                        setDeletedMsgs(AchievementsList[category][index].achie_string2);
                        do_save(tch, "", 0);

                        act(msgToChar.c_str(), false, ch, nullptr, tch, TO_CHAR);
                        if(char_data* const vict = get_char_vis_world(ch, arg2.data(), nullptr)) {
                            act(msgToVict.c_str(), false, ch, nullptr, vict, TO_VICT);
                        }
                    }
                    else
                    {
                        act("$N non ha completato nessun achievement!", false, ch, nullptr, tch, TO_CHAR);
                        return;
                    }
                }
                else
                {
                    act("Quale achievement di $N vuoi cancellare? Il numero lo vedi con $c0015achievement all $N$c0007.", false, ch, nullptr, tch, TO_CHAR);
                    return;
                }
            }
            else
            {
                send_to_char("A chi vorresti cancellare l'achievement?\n\r", ch);
                return;
            }
        }
        else if(isCommand("set") && IS_MAESTRO_DEL_CREATO(ch))
        {
            char_data* tch = nullptr;
            std::array<char, 128> arg3{};
            std::array<char, 128> arg4{};
            std::string msgToChar;
            std::string msgToVict;

            arg = one_argument(arg, arg2.data());
            arg = one_argument(arg, arg3.data());
            arg = one_argument(arg, arg4.data());
            while(*arg == ' ')
            {
                ++arg;
            }

            if(*arg != '\0')
            {
                send_to_char("Sintassi: achievements set <nome_pg> <numero> <valore>\n\r", ch);
                return;
            }

            if(arg2[0] != '\0')
            {
                if(!resolveVisiblePlayerTarget(tch, arg2.data(),
                    "Non c'e' nessuno con quel nome qui!\n\r",
                    "Vuoi veramente assegnare un achivement ad uno stupido mob?\n\r"))
                {
                    return;
                }

                if(arg3[0] != '\0')
                {
                    int achi_n = 0;
                    if(!parsePositiveIntArg(arg3.data(),
                        "Il numero achievement deve essere un intero valido.\n\r",
                        "Il numero achievement inserito e' troppo alto.\n\r",
                        "A quale achievement di $N vuoi assegnare un valore? Il numero lo vedi con $c0015achievement all $N$c0007.\n\r",
                        achi_n))
                    {
                        return;
                    }

                    if(achi_n > maxAchievements(tch))
                    {
                        msgToChar = "Il numero massimo di achievements per $N e' ";
                        msgToChar += std::to_string(maxAchievements(tch));
                        msgToChar += ".";
                        act(msgToChar.c_str(), false, ch, nullptr, tch, TO_CHAR);
                        return;
                    }

                    if(arg4[0] != '\0')
                    {
                        int achi_v = 0;
                        if(!parsePositiveIntArg(arg4.data(),
                            "Il valore deve essere un numero intero valido.\n\r",
                            "Il valore inserito e' troppo alto.\n\r",
                            "Puoi assegnare solo un valore maggiore di zero.\n\r",
                            achi_v))
                        {
                            return;
                        }

                        if(!IS_SET(tch->specials.act, PLR_ACHIE))
                        {
                            SET_BIT(tch->specials.act, PLR_ACHIE);
                        }
                        const auto setMaxAssignableMsg = [&](const char* achievementName, const int maxValue) {
                            msgToChar = "Il numero massimo di ";
                            msgToChar += achievementName;
                            msgToChar += " assegnabili e' ";
                            msgToChar += std::to_string(maxValue);
                            msgToChar += ".";
                        };
                        const auto setAssignedMsgs = [&](const char* achievementName, const int assignedValue) {
                            msgToChar = "Hai assegnato all'achievement del numero di ";
                            msgToChar += achievementName;
                            msgToChar += " di $N un valore pari a ";
                            msgToChar += std::to_string(assignedValue);
                            msgToChar += ".";
                            msgToVict = "$n ti ha assegnato all'achievement del numero di ";
                            msgToVict += achievementName;
                            msgToVict += " un valore pari a ";
                            msgToVict += std::to_string(assignedValue);
                            msgToVict += ".";
                        };
                        const auto slot = findAchievementSlotByNumber(tch, ch, achi_n, false);
                        if(!slot.has_value())
                        {
                            act("Quale achievement di $N vuoi assegnare? Il numero lo vedi con $c0015achievement all $N$c0007.",
                                false, ch, nullptr, tch, TO_CHAR);
                            return;
                        }

                        const int category = slot->category;
                        const int index = slot->index;
                        const int maxAssignableValue = MaxValueAchievement(category, index, slot->levels);
                        if(achi_v > maxAssignableValue)
                        {
                            setMaxAssignableMsg(AchievementsList[category][index].achie_string2, maxAssignableValue);
                            act(msgToChar.c_str(), false, ch, nullptr, tch, TO_CHAR);
                            return;
                        }
                        tch->specials.achievements[category][index] = achi_v;
                        mudlog(LOG_PLAYERS, "%s set the achievement related on '%s' on %s to %d",
                               GET_NAME(ch), AchievementsList[category][index].achie_string2, GET_NAME(tch), achi_v);
                        setAssignedMsgs(AchievementsList[category][index].achie_string2, achi_v);
                        do_save(tch, "", 0);

                        act(msgToChar.c_str(), false, ch, nullptr, tch, TO_CHAR);
                        if(char_data* const vict = get_char_vis_world(ch, arg2.data(), nullptr)) {
                            act(msgToVict.c_str(), false, ch, nullptr, vict, TO_VICT);
                        }
                    }
                    else
                    {
                        act("Quale valore vuoi assegnare all'achievement di $N?", false, ch, nullptr, tch, TO_CHAR);
                        return;
                    }
                }
                else
                {
                    act("Quale achievement di $N vuoi assegnare? Il numero lo vedi con $c0015achievement all $N$c0007.", false, ch, nullptr, tch, TO_CHAR);
                    return;
                }
            }
            else
            {
                send_to_char("A chi vorresti assegnare l'achievement?\n\r", ch);
                return;
            }
        }
        else if(isCommand("reset") && IS_MAESTRO_DEL_CREATO(ch))
        {
            char_data* tch = nullptr;

            arg = one_argument(arg, arg2.data());
            while(*arg == ' ')
            {
                ++arg;
            }

            if(*arg != '\0')
            {
                send_to_char("Sintassi: achievements reset <nome_pg>\n\r", ch);
                return;
            }

            if(arg2[0] != '\0')
            {
                if(!resolveVisiblePlayerTarget(tch, arg2.data(),
                    "Non c'e' nessuno con quel nome qui, a chi vorresti resettare gli achievements?\n\r",
                    "Vuoi veramente resettare gli achievements di uno stupido mob?\n\r"))
                {
                    return;
                }

                mudlog(LOG_PLAYERS, "%s starts to delete all the achievements on %s", GET_NAME(ch), GET_NAME(tch));
                const auto resetAchievementCategory = [&](const int category, const int maxCount) {
                    if(hasAchievement(tch, category, 1))
                    {
                        for(int idx = 0; idx < maxCount; ++idx)
                        {
                            tch->specials.achievements[category][idx] = 0;
                        }
                    }
                };
                struct AchievementCategorySpec
                {
                    int category;
                    int maxCount;
                };
                constexpr std::array<AchievementCategorySpec, 5> achievementCategories = {{
                    {RACESLAYER_ACHIE, MAX_RACE_ACHIE},
                    {BOSSKILL_ACHIE, MAX_BOSS_ACHIE},
                    {CLASS_ACHIE, MAX_CLASS_ACHIE},
                    {QUEST_ACHIE, MAX_QUEST_ACHIE},
                    {OTHER_ACHIE, MAX_OTHER_ACHIE}
                }};
                for(const auto& categorySpec : achievementCategories)
                {
                    resetAchievementCategory(categorySpec.category, categorySpec.maxCount);
                }
                if(IS_SET(tch->specials.act, PLR_ACHIE))
                {
                    REMOVE_BIT(tch->specials.act, PLR_ACHIE);
                }
                mudlog(LOG_PLAYERS, "Done. %s has deleted all the achievements on %s", GET_NAME(ch), GET_NAME(tch));
                do_save(tch, "", 0);

                act("Tutti gli achievements di $N sono stati resettati.", false, ch, nullptr, tch, TO_CHAR);
                if(char_data* const vict = get_char_vis_world(ch, arg2.data(), nullptr)) {
                    act("$n ti ha resettato tutti gli achievements.", false, ch, nullptr, vict, TO_VICT);
                }
            }
            else
            {
                send_to_char("A chi vorresti resettare gli achievements?\n\r", ch);
                return;
            }
        }
        else
        {
            send_to_char("$c0009Sintassi:\n\r", ch);
            if(IS_QUESTMASTER(ch))
            {
                send_to_char("$c0015Achievements$c0007 - mostra gli achievements che hai completato\n\r", ch);
            }
            else
            {
                send_to_char("$c0015Achievements$c0007 - mostra gli achievements che hai completato\n\r", ch);
            }
            if(IS_QUESTMASTER(ch))
            {
                send_to_char("$c0015Achievements all$c0007 - mostra tutti i tuoi achievements\n\r", ch);
            }
            if(IS_QUESTMASTER(ch))
            {
                send_to_char("$c0015Achievements all $c0007<$c0015nome_pg$c0007> - mostra tutti gli achievements del personaggio scelto\n\r", ch);
            }
            else
            {
                send_to_char("$c0015Achievements all$c0007 - mostra tutti i tuoi achievements\n\r", ch);
            }
            if(IS_QUESTMASTER(ch))
            {
                send_to_char("$c0015Achievements spam $c0007<$c0015numero$c0007> - mostra in stanza l'achievement scelto\n\r", ch);
            }
            else
            {
                send_to_char("$c0015Achievements spam $c0007<$c0015numero$c0007> - mostra in stanza l'achievement scelto\n\r", ch);
            }
            if(IS_QUESTMASTER(ch))
            {
                send_to_char("$c0015Achievements delete $c0007<$c0015nome_pg$c0007> <$c0015numero$c0007> - cancella un determinato achievement dal personaggio scelto\n\r", ch);
            }
            if(IS_MAESTRO_DEL_CREATO(ch))
            {
                send_to_char("$c0015Achievements set $c0007<$c0015nome_pg$c0007> <$c0015numero$c0007> <$c0015valore$c0007> - assegna al personaggio scelto un valore ad un determinato achievement\n\r", ch);
            }
            if(IS_MAESTRO_DEL_CREATO(ch))
            {
                send_to_char("$c0015Achievements reset $c0007<$c0015nome_pg$c0007> - resetta tutti gli achievements del personaggio scelto\n\r", ch);
            }

        }
    }
    else
    {
        char_data* tch = ch;

        if(!applyPolyOriginalOrAbort(tch)) {
            return;
        }

        if(IS_SET(tch->specials.act, PLR_ACHIE) && (hasAchievement(tch, RACESLAYER_ACHIE, 1) || hasAchievement(tch, QUEST_ACHIE, 1) || hasAchievement(tch, OTHER_ACHIE, 1) || hasAchievement(tch, BOSSKILL_ACHIE, 1) || hasAchievement(tch, CLASS_ACHIE, 1)))
        {
            int num = 0;
            send_to_char("\n\r$c0011Ecco i tuoi achievements:\n\r", ch);

            AppendCompactAchievementCategory(sb, num, tch, RACESLAYER_ACHIE, 0, MAX_RACE_ACHIE, 0, 40u,
                                             "Race  Achievements");
            AppendCompactAchievementCategory(sb, num, tch, BOSSKILL_ACHIE, 0, MAX_BOSS_ACHIE, 0, 40u,
                                             "Boss  Achievements");
            AppendCompactAchievementCategory(sb, num, tch, CLASS_ACHIE, 1, MAX_CLASS_ACHIE, 0, 40u,
                                             "Skill Achievements");
            AppendCompactAchievementCategory(sb, num, tch, QUEST_ACHIE, 0, MAX_QUEST_ACHIE, 0, 40u,
                                             "Quest Achievements");
            AppendCompactAchievementCategory(sb, num, tch, OTHER_ACHIE, 0, MAX_OTHER_ACHIE, 0, 40u,
                                             "Other Achievements");

            page_string(ch->desc, sb.c_str(), true);
        }
        else {
            send_to_char("Non hai completato nessun achievement.\n\r", ch);
        }
    }
}

} // namespace Alarmud
