/*ALARMUD* (Do not remove *ALARMUD*, used to automagically manage these lines
 *ALARMUD* AlarMUD 2.0
 *ALARMUD* See COPYING for licence information
 *ALARMUD*/
//  Original intial comments
/* AlarMUD */
/* $Id: utility.c,v 1.2 2002/03/11 11:33:34 Thunder Exp $
 * */
/***************************  System  include ************************************/
#include <malloc.h>
#include <cstring>
#include <cctype>
#include <ctime>
#include <sys/file.h>
#include <cstdlib>
#include <unistd.h>
#include <cstdarg>
/***************************  General include ************************************/
#include "config.hpp"
#include "typedefs.hpp"
#include "flags.hpp"
#include "autoenums.hpp"
#include "structs.hpp"
#include "logging.hpp"
#include "constants.hpp"
#include "utils.hpp"
/***************************  Local    include ************************************/
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
#include "opinion.hpp"
#include "skills.hpp"
#include "sound.hpp"
#include "spell_parser.hpp"
#include "trap.hpp"
#include "weather.hpp"

namespace Alarmud {

const char* zonename_by_room(int room) {

    struct zone_data* zd;
    int zone;

    const char* zonename[]= {
        "Scratch zone for static game",
        "il Limbo",
        "le Strade di Myst",
        "la Scuola",
        "l'Isola dell'Oblio",
        "il Trono di Thanatos",
        "l'Area di Cueball",
        "il Luogo dell'Agguato",
        "ai Piedi dell'Albero Colossale",
        "free",
        "i Rifugi Oscuri",
        "Asgaard",
        "la Contea",
        "la Loggia Bianca",
        "polymorph & change form",
        "la Scacchiera",
        "la Torre della Magia",
        "il Monastero dei Monaci",
        "il Circo",
        "la Giungla del Culto",
        "la Piramide del Serpente",
        "le sale dei Preti Tatuati",
        "free",
        "il Piano Astrale",
        "le terre di Emerald",
        "la Capitale delle Nebbie",
        "free",
        "il Fiume di Midgaard",
        "il Palazzo dei Principi",
        "il Territorio di Ator",
        "un Incrocio ad Oriente",
        "la Dimora ed Arena di Felix",
        "le Miniere di Moria",
        "le Miniere di Moria",
        "la Capitale dei Nani",
        "Kamash Darrul",
        "la Piana della Battaglia",
        "la Rocca di Quainor",
        "il Borgo di Imrryr",
        "la Foresta di Gwyldan",
        "il Grande Deserto dell'Est",
        "la Citta' dei Drow",
        "il Monolito",
        "Vecchia Thalos",
        "la Grande Piramide",
        "una Pista Battuta",
        "un Acquitrino",
        "il Villaggio Pigmeo",
        "il Deserto Dipinto",
        "le Colline di Brea",
        "la Palude di Bloslum",
        "Lorien",
        "il Regno dei Sogni",
        "l'Oscura Boscaglia",
        "il Regno degli Elfi",
        "la Fortezza Sotterranea di Kar",
        "Prydain",
        "l'Avamposto dei Dannati",
        "Artica",
        "il Regno dei Morti",
        "le Fogne",
        "la Profondita' delle Fogne",
        "il Labirinto nelle Fogne",
        "i Tunnels",
        "la Scogliera",
        "il Regno dei Norreno",
        "Utgard",
        "dove tutto ha Inizio",
        "la Tenuta dei Redferne",
        "la Banca",
        "l’Ingresso di Bosco Cet",
        "Bosco Cet",
        "il Cuore di Bosco Cet",
        "il Villaggio Isgha",
        "l'Oscura Birreria",
        "la Torre Fantasma",
        "la Citta' Perduta di Nilmys",
        "free",
        "la Casa dei Giganti delle Colline",
        "il Crepaccio di Ghiaccio",
        "il Regno del Gelo",
        "la Foresta dei Ragni",
        "QUestFisse",
        "free",
        "il Labirinto del Tempio",
        "il Maniero di Lord Python",
        "zona case",
        "Nuova Thalos",
        "l'Antica Grecia",
        "le Fatiche di Ercole",
        "la Foresta Aguzza",
        "l'Ade",
        "il Monte Olimpo",
        "le Avventure Elleniche",
        "la Foresta di Rhowyn",
        "free",
        "le Terre Meridionali",
        "gli Avamposti",
        "le Terre dei Troll",
        "free",
        "il Palazzo degli Skexie",
        "le Montagne Orientali",
        "il Villaggio degli Tzigani",
        "una Cava Subacquea",
        "Draconia",
        "free",
        "la Torre dell'Hoeur",
        "Il Sentiero nella Valle",
        "Asgard2",
        "la Citta' dei Licantropi",
        "la Montagna Piuma Bianca",
        "free",
        "le Caverne della Fenice",
        "Imrryr",
        "free",
        "Mordilnia",
        "Highlander",
        "il Castello di Warlock",
        "il Fiume di Nebbia",
        "la Scala di Seta",
        "free",
        "free",
        "free",
        "la Sala della Tortura Aliena",
        "il Castello dei Mistamere",
        "free",
        "free",
        "miner",
        "la Zona Vulcanica",
        "Xarah",
        "Guaracanos",
        "la Foresta dei Delrean",
        "il Regno dei Ragni",
        "free",
        "la Strada di Ershteep",
        "una Strada Antica",
        "le Cave dei Trogloditi",
        "l'Avamposto abbandonato di Rhyodin",
        "free",
        "Sauria",
        "free",
        "la Foresta Elfica",
        "free",
        "l'Accademia di Magia",
        "free",
        "le Catacombe di Myst",
        "l'Abisso",
        "il Territorio di Sinistrad",
        "free",
        "Darkenwood",
        "la Grotta del Solitario",
        "la Rupe Rossa",
        "la Terra di Sotto",
        "Droll Gleinas",
        "il Maniero dei Draghi",
        "l' Arena del Drago",
        "free",
        "Ravenloft",
        "free",
        "Atlantide",
        "le Torri dell'Apocalisse",
        "free",
        "il Municipio della Capitale",
        "free",
        "la Foresta Incantata",
        "il Tempio degli Eroi",
        "i Castelli dei Principi",
        "free",
        "Il Regno di Sauron",
        "last_area",
        "\0"
    };

    for(zone = 0; zone <= top_of_zone_table; zone++) {
        zd = zone_table + zone;
        if(room >= zd->bottom && room <= zd->top && zonename[zone] != NULL) {
            return(zonename[zone]);
        }
    }
    return "un posto che non esiste [Errore]";
}

#define NUM_ZONERANGES 29
int RandomRoomByLevel(int level) {

    int y, t;

    struct range_vnum_type {
        int da_vnum;
        int a_vnum;
    };

    struct range_vnum_type zone_list[NUM_ZONERANGES]= {
        {3004,3049},    /* Myst */
        {32501,32549},  /* Municipio */
        {18010,18249},  /* Mordilnia */
        {1810,1849},    /* Circo */
        {1410,1449},    /* Scacchiera */
        {11001,11059},  /* Castle Python */
        {3910,4149},    /* Moria */
        {4220,4449},    /* BofGorRak */
        {24001,24016},  /* Scuola di Magia */
        {13714,13727},  /* Fatiche di Ercole */
        {13755,13779},  /* Ade */
        {8406,8434},    /* Isgha */
        {9601,9631},    /* Spider Forest */
        {13780,13797},  /* Olimpo */
        {16101,16120},  /* Tzigani */
        {6201,6221},    /* Oscura Boscaglia */
        {27401,27480},  /* Terra di sotto */
        {32800,32880},  /* Foresta Incantata */
        {16901,16931},  /* Licantropia */
        {3302,3320},    /* Palazzo dei Principi */
        {6230,6351},    /* Rhyana */
        {13423,13481},  /* New Thalos */
        {17301,17345},  /* Ymrrir */
        {16615,16630},  /* Hoeur */
        {27002,27092},  /* Tyr Zone */
        {1910,1961},    /* Jungla e Piramide del Culto */
        {2801,2830},    /* Emerald */
        {19806,19882},  /* Guaracanos spiaggia */
        {19901,19926}   /* Foresta Delrean */
    };

    if(level >= PRINCIPE) {
        t = NUM_ZONERANGES-1;
    } else {
        if(level > INIZIATO) {
            t = 21;
        } else {
            t = 5;
        }
    }

    y = number(0,t);
    do {
        t = number(zone_list[y].da_vnum,zone_list[y].a_vnum);
    } while (!real_roomp(t) || IS_SET(real_roomp(t)->room_flags, NO_MOB|DEATH|PRIVATE|PEACEFUL));

    return(t);

}

char* spamAchie(struct char_data* ch, const char *titolo, int valore, const char *stringa, int achievement_type, int achievement_class)
{
    int i, lung[3], max = 0, diff = 0;
    char riga1[100], riga2[100], riga3[100], space[100];
    static char buffer[MAX_STRING_LENGTH];

    sprintf(riga1, "%s ha completato l'achievement", GET_NAME(ch));
    sprintf(riga2, "%s", titolo);
    sprintf(riga3, "%d %s", valore, stringa);

    lung[0] = strlen(riga1);
    lung[1] = strlen(riga2);
    lung[2] = strlen(riga3);

    sprintf(riga1, "$c0009%s $c0007ha completato l'achievement$c0011", GET_NAME(ch));
    sprintf(riga2, "$c0015%s$c0011", titolo);
    sprintf(riga3, "$c0009%d %s$c0011", valore, stringa);

    for(i = 0 ; i < 3 ; i++)
    {
        if(lung[i] > max)
        {
            max = lung[i];
        }
    }

    strcpy(space, " ");
    for (i = 0; i < (max + 7); i++)
    {
        strcat(space, " ");
    }

    strcpy(buffer, "$c0011 _\n(@)");
    strcat(buffer, space);
    strcat(buffer, " _\n| |");
    strcat(buffer, space);
    strcat(buffer, "(@)\n|-|");
    for (i = 0; i < (max + 8); i++)
    {
        strcat(buffer, "-");
    }
    strcat(buffer, "|-|\n| |");
    strcat(buffer, space);
    strcat(buffer, "| |\n|-|");
    strcat(buffer, "    ");
    if(max > lung[0])
    {
        diff = int((max - lung[0]) /2);
        if((diff * 2) != (max - lung[0]))
        {
            diff += 1;
        }
        for(i = 0; i < diff; i++)
        {
            strcat(buffer, " ");
        }
    }
    strcat(buffer, riga1);
    if(max > lung[0])
    {
        diff = int((max - lung[0]) /2);
        for(i = 0; i < diff; i++)
        {
            strcat(buffer, " ");
        }
    }
    strcat(buffer, "    |-|\n| |");

    strcat(buffer, space);
    strcat(buffer, "| |\n|-|");
    strcat(buffer, "    ");
    if(max > lung[1])
    {
        diff = int((max - lung[1]) /2);
        if((diff * 2) != (max - lung[1]))
        {
            diff += 1;
        }
        for(i = 0; i < diff; i++)
        {
            strcat(buffer, " ");
        }
    }
    strcat(buffer, riga2);
    if(max > lung[1])
    {
        diff = int((max - lung[1]) /2);
        for(i = 0; i < diff; i++)
        {
            strcat(buffer, " ");
        }
    }
    strcat(buffer, "    |-|\n| |");
    strcat(buffer, space);
    strcat(buffer, "| |\n|-|");
    strcat(buffer, "    ");
    if(max > lung[2])
    {
        diff = int((max - lung[2]) /2);
        if((diff * 2) != (max - lung[2]))
        {
            diff += 1;
        }
        for(i = 0; i < diff; i++)
        {
            strcat(buffer, " ");
        }
    }
    strcat(buffer, riga3);
    if(max > lung[2])
    {
        diff = int((max - lung[2]) /2);
        for(i = 0; i < diff; i++)
        {
            strcat(buffer, " ");
        }
    }
    strcat(buffer, "    |-|\n| |");
    for (i = 0; i < (max + 8); i++)
    {
        strcat(buffer, "_");
    }
    strcat(buffer, "| |\n(@)");
    strcat(buffer, space);
    strcat(buffer, "| |\n   ");
    strcat(buffer, space);
    strcat(buffer, "(@)\n\n\r");

    return buffer;
}

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

int n_bosskill(int vnumber)
{
    int i;

    for(i = 0; i < MAX_BOSS_ACHIE; i++)
    {
        if(vnumber == AchievementsList[BOSSKILL_ACHIE][i].achie_number)
        {
            return i;
        }
    }

    return -1;
}

bool hasAchievement(struct char_data* ch, int achievement_class)
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
                if(ch->specials.achievements[CLASS_ACHIE][i] > 0)
                {
                    return TRUE;
                }
            }
            break;

        case BOSSKILL_ACHIE:
            for(i = 0; i < MAX_BOSS_ACHIE; i++)
            {
                if(ch->specials.achievements[BOSSKILL_ACHIE][i] > 0)
                {
                    return TRUE;
                }
            }
            break;

        case RACESLAYER_ACHIE:
            for(i = 0; i < MAX_RACE_ACHIE; i++)
            {
                if(ch->specials.achievements[RACESLAYER_ACHIE][i] > 0)
                {
                    return TRUE;
                }
            }
            break;

        case QUEST_ACHIE:
            for(i = 0; i < MAX_QUEST_ACHIE ; i++)
            {
                if(ch->specials.achievements[QUEST_ACHIE][i] > 0)
                {
                    return TRUE;
                }
            }
            break;

        case OTHER_ACHIE:
            for(i = 0; i < MAX_OTHER_ACHIE; i++)
            {
                if(ch->specials.achievements[OTHER_ACHIE][i] > 0)
                {
                    return TRUE;
                }
            }
            break;

        default:
            return FALSE;
            break;
    }
    return FALSE;
}

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
            fmt % (ch->specials.achievements[achievement_class][achievement_type] >= AchievementsList[achievement_class][achievement_type].lvl6_val ? "$c0015" : "$c0007") % (num + 3) % (ch->specials.achievements[achievement_class][achievement_type] >= AchievementsList[achievement_class][achievement_type].lvl6_val ? "$c0015" : "$c0007") % AchievementsList[achievement_class][achievement_type].lvl6 % (ch->specials.achievements[achievement_class][achievement_type] >= AchievementsList[achievement_class][achievement_type].lvl6_val ? AchievementsList[achievement_class][achievement_type].lvl6_val : ch->specials.achievements[achievement_class][achievement_type]) % (ch->specials.achievements[achievement_class][achievement_type] >= AchievementsList[achievement_class][achievement_type].lvl6_val ? "$c0015" : "$c0007") % AchievementsList[achievement_class][achievement_type].lvl6_val % (ch->specials.achievements[achievement_class][achievement_type] >= AchievementsList[achievement_class][achievement_type].lvl6_val ? "$c0015" : "$c0007") % (AchievementsList[achievement_class][achievement_type].lvl6_val == 1 ? AchievementsList[achievement_class][achievement_type].achie_string1 : AchievementsList[achievement_class][achievement_type].achie_string2);
            sb.append(fmt.str().c_str());
            fmt.clear();
            if(check == (num + 3))
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
            fmt % ((num + 3)%2 == 0 ? "$c0015" : "$c0007") % (num + 3) % AchievementsList[achievement_class][achievement_type].lvl6 % AchievementsList[achievement_class][achievement_type].lvl6_val % (AchievementsList[achievement_class][achievement_type].lvl6_val == 1 ? AchievementsList[achievement_class][achievement_type].achie_string1 : AchievementsList[achievement_class][achievement_type].achie_string2);
            sb.append(fmt.str().c_str());
            fmt.clear();
        }
    }
    if (lvl >= 7)
    {
        if(formato)
        {
            boost::format fmt("$c0009[%s%4d$c0009]%s %-55s $c0011%6d%s/$c0011%-6d%s %-30s\n\r");
            fmt % (ch->specials.achievements[achievement_class][achievement_type] >= AchievementsList[achievement_class][achievement_type].lvl7_val ? "$c0015" : "$c0007") % (num + 3) % (ch->specials.achievements[achievement_class][achievement_type] >= AchievementsList[achievement_class][achievement_type].lvl7_val ? "$c0015" : "$c0007") % AchievementsList[achievement_class][achievement_type].lvl7 % (ch->specials.achievements[achievement_class][achievement_type] >= AchievementsList[achievement_class][achievement_type].lvl7_val ? AchievementsList[achievement_class][achievement_type].lvl7_val : ch->specials.achievements[achievement_class][achievement_type]) % (ch->specials.achievements[achievement_class][achievement_type] >= AchievementsList[achievement_class][achievement_type].lvl7_val ? "$c0015" : "$c0007") % AchievementsList[achievement_class][achievement_type].lvl7_val % (ch->specials.achievements[achievement_class][achievement_type] >= AchievementsList[achievement_class][achievement_type].lvl7_val ? "$c0015" : "$c0007") % (AchievementsList[achievement_class][achievement_type].lvl7_val == 1 ? AchievementsList[achievement_class][achievement_type].achie_string1 : AchievementsList[achievement_class][achievement_type].achie_string2);
            sb.append(fmt.str().c_str());
            fmt.clear();
            if(check == (num + 3))
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
            fmt % ((num + 3)%2 == 0 ? "$c0015" : "$c0007") % (num + 3) % AchievementsList[achievement_class][achievement_type].lvl7 % AchievementsList[achievement_class][achievement_type].lvl7_val % (AchievementsList[achievement_class][achievement_type].lvl7_val == 1 ? AchievementsList[achievement_class][achievement_type].achie_string1 : AchievementsList[achievement_class][achievement_type].achie_string2);
            sb.append(fmt.str().c_str());
            fmt.clear();
        }
    }
    if (lvl >= 8)
    {
        if(formato)
        {
            boost::format fmt("$c0009[%s%4d$c0009]%s %-55s $c0011%6d%s/$c0011%-6d%s %-30s\n\r");
            fmt % (ch->specials.achievements[achievement_class][achievement_type] >= AchievementsList[achievement_class][achievement_type].lvl8_val ? "$c0015" : "$c0007") % (num + 3) % (ch->specials.achievements[achievement_class][achievement_type] >= AchievementsList[achievement_class][achievement_type].lvl8_val ? "$c0015" : "$c0007") % AchievementsList[achievement_class][achievement_type].lvl8 % (ch->specials.achievements[achievement_class][achievement_type] >= AchievementsList[achievement_class][achievement_type].lvl8_val ? AchievementsList[achievement_class][achievement_type].lvl8_val : ch->specials.achievements[achievement_class][achievement_type]) % (ch->specials.achievements[achievement_class][achievement_type] >= AchievementsList[achievement_class][achievement_type].lvl8_val ? "$c0015" : "$c0007") % AchievementsList[achievement_class][achievement_type].lvl8_val % (ch->specials.achievements[achievement_class][achievement_type] >= AchievementsList[achievement_class][achievement_type].lvl8_val ? "$c0015" : "$c0007") % (AchievementsList[achievement_class][achievement_type].lvl8_val == 1 ? AchievementsList[achievement_class][achievement_type].achie_string1 : AchievementsList[achievement_class][achievement_type].achie_string2);
            sb.append(fmt.str().c_str());
            fmt.clear();
            if(check == (num + 3))
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
            fmt % ((num + 3)%2 == 0 ? "$c0015" : "$c0007") % (num + 3) % AchievementsList[achievement_class][achievement_type].lvl8 % AchievementsList[achievement_class][achievement_type].lvl8_val % (AchievementsList[achievement_class][achievement_type].lvl8_val == 1 ? AchievementsList[achievement_class][achievement_type].achie_string1 : AchievementsList[achievement_class][achievement_type].achie_string2);
            sb.append(fmt.str().c_str());
            fmt.clear();
        }
    }
    if (lvl >= 9)
    {
        if(formato)
        {
            boost::format fmt("$c0009[%s%4d$c0009]%s %-55s $c0011%6d%s/$c0011%-6d%s %-30s\n\r");
            fmt % (ch->specials.achievements[achievement_class][achievement_type] >= AchievementsList[achievement_class][achievement_type].lvl9_val ? "$c0015" : "$c0007") % (num + 3) % (ch->specials.achievements[achievement_class][achievement_type] >= AchievementsList[achievement_class][achievement_type].lvl9_val ? "$c0015" : "$c0007") % AchievementsList[achievement_class][achievement_type].lvl9 % (ch->specials.achievements[achievement_class][achievement_type] >= AchievementsList[achievement_class][achievement_type].lvl9_val ? AchievementsList[achievement_class][achievement_type].lvl9_val : ch->specials.achievements[achievement_class][achievement_type]) % (ch->specials.achievements[achievement_class][achievement_type] >= AchievementsList[achievement_class][achievement_type].lvl9_val ? "$c0015" : "$c0007") % AchievementsList[achievement_class][achievement_type].lvl9_val % (ch->specials.achievements[achievement_class][achievement_type] >= AchievementsList[achievement_class][achievement_type].lvl9_val ? "$c0015" : "$c0007") % (AchievementsList[achievement_class][achievement_type].lvl9_val == 1 ? AchievementsList[achievement_class][achievement_type].achie_string1 : AchievementsList[achievement_class][achievement_type].achie_string2);
            sb.append(fmt.str().c_str());
            fmt.clear();
            if(check == (num + 3))
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
            fmt % ((num + 3)%2 == 0 ? "$c0015" : "$c0007") % (num + 3) % AchievementsList[achievement_class][achievement_type].lvl9 % AchievementsList[achievement_class][achievement_type].lvl9_val % (AchievementsList[achievement_class][achievement_type].lvl9_val == 1 ? AchievementsList[achievement_class][achievement_type].achie_string1 : AchievementsList[achievement_class][achievement_type].achie_string2);
            sb.append(fmt.str().c_str());
            fmt.clear();
        }
    }
    if (lvl >= 10)
    {
        if(formato)
        {
            boost::format fmt("$c0009[%s%4d$c0009]%s %-55s $c0011%6d%s/$c0011%-6d%s %-30s\n\r");
            fmt % (ch->specials.achievements[achievement_class][achievement_type] >= AchievementsList[achievement_class][achievement_type].lvl10_val ? "$c0015" : "$c0007") % (num + 3) % (ch->specials.achievements[achievement_class][achievement_type] >= AchievementsList[achievement_class][achievement_type].lvl10_val ? "$c0015" : "$c0007") % AchievementsList[achievement_class][achievement_type].lvl10 % (ch->specials.achievements[achievement_class][achievement_type] >= AchievementsList[achievement_class][achievement_type].lvl10_val ? AchievementsList[achievement_class][achievement_type].lvl10_val : ch->specials.achievements[achievement_class][achievement_type]) % (ch->specials.achievements[achievement_class][achievement_type] >= AchievementsList[achievement_class][achievement_type].lvl10_val ? "$c0015" : "$c0007") % AchievementsList[achievement_class][achievement_type].lvl10_val % (ch->specials.achievements[achievement_class][achievement_type] >= AchievementsList[achievement_class][achievement_type].lvl10_val ? "$c0015" : "$c0007") % (AchievementsList[achievement_class][achievement_type].lvl10_val == 1 ? AchievementsList[achievement_class][achievement_type].achie_string1 : AchievementsList[achievement_class][achievement_type].achie_string2);
            sb.append(fmt.str().c_str());
            fmt.clear();
            if(check == (num + 3))
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
            fmt % ((num + 3)%2 == 0 ? "$c0015" : "$c0007") % (num + 3) % AchievementsList[achievement_class][achievement_type].lvl10 % AchievementsList[achievement_class][achievement_type].lvl10_val % (AchievementsList[achievement_class][achievement_type].lvl10_val == 1 ? AchievementsList[achievement_class][achievement_type].achie_string1 : AchievementsList[achievement_class][achievement_type].achie_string2);
            sb.append(fmt.str().c_str());
            fmt.clear();
        }
    }


    if(spam)
        sb = sb2;

    return sb;
}

void CheckAchie(struct char_data* ch, int achievement_type, int achievement_class)
{
    char buf[MAX_STRING_LENGTH], titolo[MAX_STRING_LENGTH], stringa[MAX_STRING_LENGTH];
    int valore = 0, molt = 0;
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

    if(tch->specials.achievements[achievement_class][achievement_type] == AchievementsList[achievement_class][achievement_type].lvl1_val)
    {
        molt = 10;
        valore = AchievementsList[achievement_class][achievement_type].lvl1_val;
        strcpy(titolo, AchievementsList[achievement_class][achievement_type].lvl1);
        if(valore == 1)
        {
            strcpy(stringa, AchievementsList[achievement_class][achievement_type].achie_string1);
        }
        else
        {
                strcpy(stringa, AchievementsList[achievement_class][achievement_type].achie_string2);
        }
    }
    else if(tch->specials.achievements[achievement_class][achievement_type] == AchievementsList[achievement_class][achievement_type].lvl2_val)
    {
        molt = 15;
        valore = AchievementsList[achievement_class][achievement_type].lvl2_val;
        strcpy(titolo, AchievementsList[achievement_class][achievement_type].lvl2);
        strcpy(stringa, AchievementsList[achievement_class][achievement_type].achie_string2);
    }
    else if(tch->specials.achievements[achievement_class][achievement_type] == AchievementsList[achievement_class][achievement_type].lvl3_val)
    {
        molt = 20;
        valore = AchievementsList[achievement_class][achievement_type].lvl3_val;
        strcpy(titolo, AchievementsList[achievement_class][achievement_type].lvl3);
        strcpy(stringa, AchievementsList[achievement_class][achievement_type].achie_string2);
    }
    else if(tch->specials.achievements[achievement_class][achievement_type] == AchievementsList[achievement_class][achievement_type].lvl4_val)
    {
        molt = 25;
        valore = AchievementsList[achievement_class][achievement_type].lvl4_val;
        strcpy(titolo, AchievementsList[achievement_class][achievement_type].lvl4);
        strcpy(stringa, AchievementsList[achievement_class][achievement_type].achie_string2);
    }
    else if(tch->specials.achievements[achievement_class][achievement_type] == AchievementsList[achievement_class][achievement_type].lvl5_val)
    {
        molt = 30;
        valore = AchievementsList[achievement_class][achievement_type].lvl5_val;
        strcpy(titolo, AchievementsList[achievement_class][achievement_type].lvl5);
        strcpy(stringa, AchievementsList[achievement_class][achievement_type].achie_string2);
    }
    else if(tch->specials.achievements[achievement_class][achievement_type] == AchievementsList[achievement_class][achievement_type].lvl6_val)
    {
        molt = 35;
        valore = AchievementsList[achievement_class][achievement_type].lvl6_val;
        strcpy(titolo, AchievementsList[achievement_class][achievement_type].lvl6);
        strcpy(stringa, AchievementsList[achievement_class][achievement_type].achie_string2);
    }
    else if(tch->specials.achievements[achievement_class][achievement_type] == AchievementsList[achievement_class][achievement_type].lvl7_val)
    {
        molt = 40;
        valore = AchievementsList[achievement_class][achievement_type].lvl7_val;
        strcpy(titolo, AchievementsList[achievement_class][achievement_type].lvl7);
        strcpy(stringa, AchievementsList[achievement_class][achievement_type].achie_string2);
    }
    else if(tch->specials.achievements[achievement_class][achievement_type] == AchievementsList[achievement_class][achievement_type].lvl8_val)
    {
        molt = 45;
        valore = AchievementsList[achievement_class][achievement_type].lvl8_val;
        strcpy(titolo, AchievementsList[achievement_class][achievement_type].lvl8);
        strcpy(stringa, AchievementsList[achievement_class][achievement_type].achie_string2);
    }
    else if(tch->specials.achievements[achievement_class][achievement_type] == AchievementsList[achievement_class][achievement_type].lvl9_val)
    {
        molt = 50;
                valore = AchievementsList[achievement_class][achievement_type].lvl9_val;
                strcpy(titolo, AchievementsList[achievement_class][achievement_type].lvl9);
                strcpy(stringa, AchievementsList[achievement_class][achievement_type].achie_string2);
            }
    else if(tch->specials.achievements[achievement_class][achievement_type] == AchievementsList[achievement_class][achievement_type].lvl10_val)
    {
        molt = 60;
        valore = AchievementsList[achievement_class][achievement_type].lvl10_val;
        strcpy(titolo, AchievementsList[achievement_class][achievement_type].lvl10);
        strcpy(stringa, AchievementsList[achievement_class][achievement_type].achie_string2);
    }

    if(valore > 0)
    {
        int reward;

        sprintf(buf, "%s", spamAchie(tch, titolo, valore, stringa, achievement_type, achievement_class));
        send_to_char(buf, ch);
        sprintf(buf, "\n\r$c0009%s $c0007ha completato l'achievement $c0015'%s$c0007' ($c0011%d $c0009%s$c0007).\n\r\n\r", GET_NAME(ch), titolo, valore, stringa);
        send_to_all_not_ch(ch, buf);

        switch(AchievementsList[achievement_class][achievement_type].grado_diff)
        {
            case LEV_BEGINNER:
                reward = RewardXp[GetMaxLevel(ch)].lev_1_xp;
                break;
            case LEV_VERY_EASY:
                reward = RewardXp[GetMaxLevel(ch)].lev_2_xp;
                break;
            case LEV_EASY:
                reward = RewardXp[GetMaxLevel(ch)].lev_3_xp;
                break;
            case LEV_NORMAL:
                reward = RewardXp[GetMaxLevel(ch)].lev_4_xp;
                break;
            case LEV_HARD:
                reward = RewardXp[GetMaxLevel(ch)].lev_5_xp;
                break;
            case LEV_EXPERT:
                reward = RewardXp[GetMaxLevel(ch)].lev_6_xp;
                break;
            case LEV_CHAMPION:
                reward = RewardXp[GetMaxLevel(ch)].lev_7_xp;
                break;
            case LEV_IMPERIAL:
                reward = RewardXp[GetMaxLevel(ch)].lev_8_xp;
                break;
            case LEV_TORMENT:
                reward = RewardXp[GetMaxLevel(ch)].lev_9_xp;
                break;
            case LEV_GOD_MODE:
                reward = RewardXp[GetMaxLevel(ch)].lev_10_xp;
                break;

            default:
                reward = RewardXp[GetMaxLevel(ch)].lev_1_xp;
                mudlog(LOG_CHECK, "Achievement's difficulty level is missing for achievement type %d (class %d)", achievement_type, achievement_class);
                break;
        }
        mudlog(LOG_PLAYERS, "Reward prima della modifiche e' %d", reward);
        switch (HowManyClasses(tch))
        {
                // monoclasee: il pg prende xp pieni
            case 1:
                reward = reward;
                break;
                // biclasse: il pg prende 150% degli xp
            case 2:
                reward = int (reward * 15 / 10);
                break;

                // triclasse o +: il pg prende 200% degli xp
            default:
                reward *= 2;
                break;
        }
        mudlog(LOG_PLAYERS, "Reward dopo la modifica classi e' %d", reward);
        int basso = int(reward - reward * 5 / 100);
        int alto = int(reward + reward * 5 / 100);

        reward = reward * molt / 10;

        reward = number(int(reward - reward * 5 / 100), int(reward + reward * 5 / 100));

        mudlog(LOG_PLAYERS, "Reward finale: %d (numero tra %d e %d), il moltiplicatore era %d", reward, basso, alto, float(molt/10));
        gain_exp(tch, reward);
        sprintf(buf,"$c0014Ricevi $c0015%d$c0014 punti esperienza per aver completato l'achievement '$c0015%s$c0014'.", reward, titolo);
        act(buf, FALSE, tch, 0, 0, TO_CHAR);
        do_save(tch, "", 0);
    }
}

int EgoBladeSave(struct char_data* ch) {
	int total;

	if(GetMaxLevel(ch) <= 10) {
		return(FALSE);
	}
	total = (GetMaxLevel(ch) + GET_STR(ch) + GET_CON(ch));
	if(GET_HIT(ch) == 0) {
		return(FALSE);
	}
	total = total - (GET_MAX_HIT(ch) / GET_HIT(ch));
	if(number(1,101) > total) {
		return(FALSE);
	}
	else {
		return(TRUE);
	}
}

/* Added by Gaia 2001 for limited items EGO */

int EgoSave(struct char_data* ch) {
#ifdef EGO_RARE


	int total=0;
	int nlim=0;
	int LimObj(struct char_data *ch);

	if(GetMaxLevel(ch) <= 15) {
		return(FALSE);
	}
	nlim = LimObj(ch) ;

	if(nlim <= 1) {
		return(TRUE);
	}

	total = GetMaxLevel(ch) + GET_CHR(ch)*3 ;
	if(IS_PRINCE(ch)) {
		total +=50 ;
	}
	if(HasClass(ch, CLASS_MAGIC_USER|CLASS_SORCERER)) {
		total +=20 ;
	}
	if(GET_RACE(ch) == RACE_HALFLING) {
		total += 50 ;
	}
	else if(GET_RACE(ch) == RACE_DWARF || GET_RACE(ch) == RACE_DARK_DWARF
		   ) {
		total += 30 ;
	}
	else if(GET_RACE(ch) == RACE_TROLL) {
		total -= 40 ;
	}
	else if(GET_RACE(ch) == RACE_HALF_GIANT) {
		total -= 20 ;
	}

	total -= 25*(nlim - 1);

	mudlog(LOG_CHECK, "Total limited %d percent %d", nlim,
		   total);

	if(total > 50) {
		return(TRUE);
	}
	else if(total < 1) {
		return(FALSE);
	}
	else if(number(1,51) > total) {
		return(FALSE);
	}
	else {
		return(TRUE);
	}
#else
	return(TRUE) ;
#endif
}

int MIN(int a, int b) {
	return a < b ? a:b;
}


int MAX(int a, int b) {
	return a > b ? a:b;
}

unsigned int GetItemClassRestrictions(struct obj_data* obj) {
	unsigned  int total=0;

	if(IS_SET(obj->obj_flags.extra_flags, ITEM_ANTI_MAGE)) {
		total += CLASS_MAGIC_USER;
	}
	if(IS_SET(obj->obj_flags.extra_flags, ITEM_ANTI_THIEF)) {
		total += CLASS_THIEF;
	}
	if(IS_SET(obj->obj_flags.extra_flags, ITEM_ANTI_FIGHTER)) {
		total += CLASS_WARRIOR;
	}
	if(IS_SET(obj->obj_flags.extra_flags, ITEM_ANTI_CLERIC)) {
		total += CLASS_CLERIC;
	}

	if(IS_SET(obj->obj_flags.extra_flags, ITEM_ANTI_BARBARIAN)) {
		total += CLASS_BARBARIAN;
	}

	if(IS_SET(obj->obj_flags.extra_flags, ITEM_ANTI_RANGER)) {
		total += CLASS_RANGER;
	}

	if(IS_SET(obj->obj_flags.extra_flags, ITEM_ANTI_PALADIN)) {
		total += CLASS_PALADIN;
	}

	if(IS_SET(obj->obj_flags.extra_flags, ITEM_ANTI_PSI)) {
		total += CLASS_PSI;
	}

	if(IS_SET(obj->obj_flags.extra_flags, ITEM_ANTI_MONK)) {
		total += CLASS_MONK;
	}

	if(IS_SET(obj->obj_flags.extra_flags, ITEM_ANTI_DRUID)) {
		total += CLASS_DRUID;
	}

	if(IS_SET(obj->obj_flags.extra_flags,  ITEM_ANTI_SORCERER)) {
		total += CLASS_SORCERER;
	}
	return(total);

}

bool isNullChar(struct char_data* ch) {
	if(ch == NULL || GET_NAME(ch) == NULL) {
		return TRUE;
	}
	else {
		return FALSE;
	}
}



int CAN_SEE(struct char_data* s, struct char_data* o) {
	register int iSLev = GetMaxLevel(s);
	struct room_data* pRoomS;
	struct room_data* pRoomO;
	struct affected_type* aff;
	int s_inv=0;
	int o_inv=0;
	if(!o || s->in_room < 0 || o->in_room < 0) {
		return(FALSE);
	}


	if(o->invis_level > iSLev) {
		return FALSE;
	}

	if(!IS_NPC(s) && iSLev >= IMMORTALE) {
		return(TRUE);
	}

	if((IS_AFFECTED(s, AFF_BLIND) && !HasClass(s,CLASS_PSI)
			&&!IS_AFFECTED(s,AFF_TRUE_SIGHT))
			|| IS_AFFECTED(o, AFF_HIDE)) {
		return(FALSE);
	}

	if(IS_AFFECTED(s, AFF_TRUE_SIGHT) && !IS_AFFECTED(o,AFF_INVISIBLE)) {
		return(TRUE);
	}

	if(IS_AFFECTED(o, AFF_INVISIBLE) || IS_AFFECTED(s,AFF_BLIND)) {
		if(IS_IMMORTAL(o)) {
			return(FALSE);
		}
		if(IS_AFFECTED(s, AFF_DETECT_INVISIBLE) ||
				IS_AFFECTED(s,AFF_TRUE_SIGHT)) {
			for(aff = s->affected; aff ; aff = aff->next) {
				if((aff->type==SPELL_DETECT_INVISIBLE ||
						aff->type==SPELL_TRUE_SIGHT)
						&& aff->location==APPLY_NONE) {
					s_inv=MAX(aff->modifier,s_inv);
				}

			}
			for(aff = o->affected; aff ; aff = aff->next) {
				if(aff->type==SPELL_INVISIBLE&& aff->location==APPLY_NONE) {
					o_inv=aff->modifier;
					break;
				}

			}
			if(s_inv==0) {
				s_inv=MIN(GetMaxLevel(s)+5,45);
			}
			return((o_inv-s_inv)<6);
		} /*is affected det_invisible*/
		return(FALSE);
	}

	pRoomS = real_roomp(s->in_room);
	pRoomO = real_roomp(o->in_room);
	if(pRoomS and pRoomO) {

		if((IS_DARK_P(pRoomS) || IS_DARK_P(pRoomO)) &&
				!IS_AFFECTED(s, AFF_INFRAVISION) && !(GET_RACE(s)==RACE_DARK_ELF)) {
			return(FALSE);
		}
	}
	if(IS_AFFECTED2(o, AFF2_ANIMAL_INVIS) && IsAnimal(s)) {
		return(FALSE);
	}

	return(TRUE);
}

int CAN_SEE_OBJ(struct char_data* ch, struct obj_data* obj) {
	int num=0;

	if(IS_IMMORTAL(ch)) {
		return(1);
	}



	/* changed the act.info.c, hope this works on traps INSIDE chests etc.. */
	/* msw */

	if((ITEM_TYPE(obj) == ITEM_TRAP) && (GET_TRAP_CHARGES(obj) > 0)) {
		num = number(1,101);
		if(CanSeeTrap(num,ch)) {
			return(TRUE);
		}
		else {
			return(FALSE);
		}
	} /* not a trap */

	if(IS_AFFECTED(ch, AFF_TRUE_SIGHT)) {
		return(1);
	}

	if(IS_AFFECTED(ch, AFF_BLIND)) {
		return(0);
	}

	if((long)(obj->equipped_by)==(long)(ch)) {
		return(1);
	}

	if(IS_DARK_P(real_roomp(ch->in_room)) &&
			!IS_OBJ_STAT(obj, ITEM_GLOW) &&
			GET_RACE(ch)!=RACE_DARK_ELF) {
		return(0);
	}


	if(!IS_AFFECTED(ch, AFF_DETECT_INVISIBLE) &&
			IS_OBJ_STAT(obj, ITEM_INVISIBLE)) {
		return(0);
	}
	if(obj->equipped_by &&
			affected_by_spell(obj->equipped_by, SPELL_GLOBE_DARKNESS) &&
			!IS_OBJ_STAT(obj,ITEM_GLOW) &&
			(!IS_OBJ_STAT(obj,ITEM_MAGIC) || !IS_AFFECTED(ch,AFF_DETECT_MAGIC))
	  ) {
		return(0);
	}
	return(1);
}

int exit_ok(struct room_direction_data* exit, struct room_data** rpp) {
	struct room_data* rp;

	if(rpp == NULL) {
		rpp = &rp;
	}
	if(!exit || exit->to_room <= 0) {
		*rpp = NULL;
		return FALSE;
	}
	*rpp = real_roomp(exit->to_room);
	return (*rpp != NULL);
}

int MobVnum(struct char_data* c) {
	if(IS_MOB(c)) {
		return(mob_index[c->nr].iVNum);
	}
	else {
		return(0);
	}
}

int ObjVnum(struct obj_data* o) {
	if(o->item_number >= 0) {
		return(obj_index[o->item_number].iVNum);
	}
	else {
		return(-1);
	}
}


void Zwrite(FILE* fp, char cmd, int tf, int arg1, int arg2, int arg3,
			int arg4, char* desc) {
	char buf[ 256 ], buf2[ 256 ];

	switch(cmd) {
	case 'O':
		sprintf(buf, "%c %d %d %d %d %d", cmd, tf, arg1, arg2, arg3, arg4);
		break;
	case 'M':
	case 'C':
	case 'E':
	case 'P':
	case 'D':
		sprintf(buf, "%c %d %d %d %d", cmd, tf, arg1, arg2, arg3);
		break;
	default:
		sprintf(buf, "%c %d %d %d", cmd, tf, arg1, arg2);
		break;
	}

	if(*desc) {
		sprintf(buf2, "%s   ; %s\n", buf, desc);
	}
	else {
		sprintf(buf2, "%s\n", buf);
	}
	fputs(buf2, fp);
}

void RecZwriteObj(FILE* fp, struct obj_data* o) {
	struct obj_data* t;

	if(ITEM_TYPE(o) == ITEM_CONTAINER) {
		for(t = o->contains; t; t=t->next_content) {
			if(t->item_number >= 0) {
				Zwrite(fp, 'P', 1, ObjVnum(t), obj_index[t->item_number].number,
					   ObjVnum(o), 0, t->short_description);
				RecZwriteObj(fp, t);
			}
		}
	}
	else {
		return;
	}
}


int SaveZoneFile(FILE* fp, int start_room, int end_room) {
	struct char_data* p;
	struct obj_data* o;
	struct room_data* room;
	char cmd, buf[80];
	int i, j, arg1, arg2, arg3, arg4;


	for(i = start_room; i<=end_room; i++) {
		room = real_roomp(i);
		if(room) {
			/*
			 *  first write out monsters
			 */
			for(p = room->people; p; p = p->next_in_room) {
				if(IS_NPC(p)) {
					cmd = 'M';
					arg1 = MobVnum(p);
					arg2 = mob_index[p->nr].number;
					arg3 = i;
					Zwrite(fp, cmd, 0, arg1, arg2, arg3, 0, p->player.short_descr);
					for(j = 0; j<MAX_WEAR; j++) {
						if(p->equipment[j]) {
							if(p->equipment[j]->item_number >= 0) {
								cmd = 'E';
								arg1 = ObjVnum(p->equipment[j]);
								arg2 = obj_index[p->equipment[j]->item_number].number;
								arg3 = j;
								strcpy(buf, p->equipment[j]->short_description);
								Zwrite(fp, cmd,1,arg1, arg2, arg3, 0,
									   buf);
								RecZwriteObj(fp, p->equipment[j]);
							}
						}
					}
					for(o = p->carrying; o; o=o->next_content) {
						if(o->item_number >= 0) {
							cmd = 'G';
							arg1 = ObjVnum(o);
							arg2 = obj_index[o->item_number].number;
							arg3 = 0;
							strcpy(buf, o->short_description);
							Zwrite(fp, cmd, 1, arg1, arg2, arg3, 0, buf);
							RecZwriteObj(fp, o);
						}
					}
				}
			}
			/*
			 *  write out objects in rooms
			 */
			for(o = room->contents; o; o= o->next_content) {
				if(o->item_number >= 0) {
					cmd = 'O';
					arg1 = ObjVnum(o);
					arg2 = obj_index[o->item_number].number;
					arg3 = i;
					arg4 = ObjRoomCount(o->item_number, room);
					strcpy(buf, o->short_description);
					Zwrite(fp, cmd, 0, arg1, arg2, arg3, arg4, buf);
					RecZwriteObj(fp, o);
				}
			}
			/*
			 *  lastly.. doors
			 */

			for(j = 0; j < 6; j++) {
				/*
				 *  if there is an door type exit, write it.
				 */
				if(room->dir_option[j]) {
					/* is a door */
					if(room->dir_option[j]->exit_info) {
						cmd = 'D';
						arg1 = i ;
						arg2 = j;
						arg3 = 0;
						if(IS_SET(room->dir_option[j]->exit_info, EX_CLOSED)) {
							arg3 = 1;
						}
						if(IS_SET(room->dir_option[j]->exit_info, EX_LOCKED)) {
							arg3 = 2;
						}
						Zwrite(fp, cmd, 0, arg1, arg2, arg3, 0, room->name);
					}
				}
			}
		}
	}
	fprintf(fp,"S\n");
	return 1;
}


int LoadZoneFile(FILE* fl, int zon) {
	int cmd_no = 0, expand, tmp, cc = 22;
	char buf[81];

	if(zone_table[zon].cmd) {
		free(zone_table[zon].cmd);
		zone_table[zon].cmd = NULL;
	}

	/* read the command table */
	cmd_no = 0;
	for(expand = 1; !feof(fl);) {
		if(expand) {
			if(!cmd_no) {
				CREATE(zone_table[zon].cmd, struct reset_com, cc);
			}
			else if(cmd_no >= cc) {
				cc += 5;
				if(!(zone_table[zon].cmd =
							(struct reset_com*) realloc(zone_table[zon].cmd,
														(cc * sizeof(struct reset_com)))))  {
					mudlog(LOG_ERROR,"%s:%s","reset command load",strerror(errno));
					assert(0);
				}
			}
		}

		expand = 1;

		fscanf(fl, " "); /* skip blanks */
		fscanf(fl, "%c", &zone_table[zon].cmd[cmd_no].command);

		if(zone_table[zon].cmd[cmd_no].command == 'S') {
			break;
		}

		if(zone_table[zon].cmd[cmd_no].command == '*')   {
			expand = 0;
			fgets(buf, 80, fl); /* skip command */
			continue;
		}

		fscanf(fl, " %d %d %d",
			   &tmp,
			   &zone_table[zon].cmd[cmd_no].arg1,
			   &zone_table[zon].cmd[cmd_no].arg2);
		zone_table[zon].cmd[cmd_no].if_flag=tmp;

		switch(zone_table[zon].cmd[cmd_no].command) {
		case 'M':
		case 'O':
		case 'C':
		case 'E':
		case 'P':
		case 'D':
			fscanf(fl, " %d", &zone_table[zon].cmd[cmd_no].arg3);
			break;
		}
		fgets(buf, 80, fl);       /* read comment */
		cmd_no++;
	}
	return 1;
}

void CleanZone(int zone) {
	struct room_data* rp;
	struct char_data* vict, *next_v;
	struct obj_data* obj, *next_o;
	int start, end, i;

	start=zone?(zone_table[zone-1].top+1):0;
	end=zone_table[zone].top;

	for(i=start; i<=end; i++) {
		rp=real_roomp(i);
		if(!rp) {
			continue;
		}

		for(vict = rp->people; vict; vict=next_v) {
			next_v=vict->next_in_room;
			if(IS_NPC(vict) && (!IS_SET(vict->specials.act, ACT_POLYSELF))) {
				extract_char(vict);
			}
			else {
				send_to_char("\r\n\r\nSwirling winds of Chaos reform reality around you!\r\n\r\n",vict);
			}
		}

		for(obj = rp->contents; obj; obj = next_o) {
			next_o=obj->next_content;
			extract_obj(obj);
		}
	}
}


int FindZone(int zone) {
	int i;
	for(i=0; i<=top_of_zone_table; i++)
		if(zone_table[i].num==zone) {
			break;
		}
	if(zone_table[i].num!=zone) {
		return(-1);
	}
	else {
		return i;
	}
}

FILE* MakeZoneFile(struct char_data* c, int zone) {
	char buf[256];
	FILE* fp;

	sprintf(buf, "zones/%d.zon", zone);

	if((fp = fopen(buf, "wt")) != NULL) {
		return(fp);
	}
	else {
		return(0);
	}

}

FILE* OpenZoneFile(struct char_data* c, int zone) {
	char buf[256];
	FILE* fp;

	sprintf(buf, "zones/%d.zon", zone);

	if((fp = fopen(buf, "rt")) != NULL) {
		return(fp);
	}
	else {
		return(0);
	}
}

int WeaponImmune(struct char_data* ch) {

	if(IS_SET(IMM_NONMAG, ch->M_immune) ||
			IS_SET(IMM_PLUS1, ch->M_immune) ||
			IS_SET(IMM_PLUS2, ch->M_immune) ||
			IS_SET(IMM_PLUS3, ch->M_immune) ||
			IS_SET(IMM_PLUS4, ch->M_immune)) {
		return(TRUE);
	}
	return(FALSE);

}

unsigned IsImmune(struct char_data* ch, int bit) {
	return(IS_SET(bit, ch->M_immune));
}

unsigned IsResist(struct char_data* ch, int bit) {
	return(IS_SET(bit, ch->immune));
}

unsigned IsSusc(struct char_data* ch, int bit) {
	return(IS_SET(bit, ch->susc));
}

/* creates a random number in interval [from;to] */
int number(int from, int to) {
	if(to - from + 1) {
		return((random() % (to - from + 1)) + from);
	}
	else {
		return(from);
	}
}



/* simulates dice roll */
int dice(int number, int size) {
	int r;
	int sum = 0;

#if 0
	assert(size >= 0);
#else
	/* instead of crashing the mud we set it to 1 */
	if(size <= 0) {
		size=1;
	}
#endif

	if(size == 0) {
		return(0);
	}

	for(r = 1; r <= number; r++) {
		sum += ((random() % size)+1);
	}
	return(sum);
}

int scan_number(const char* text, int* rval) {
	int        length;
	if(1!=sscanf(text, " %i %n", rval, &length)) {
		return 0;
	}
	if(text[length] != 0) {
		return 0;
	}
	return 1;
}


/* returns: 0 if equal, 1 if arg1 > arg2, -1 if arg1 < arg2  */
/* scan 'till found different or end of both                 */
int str_cmp(const char* arg1, const char* arg2) {
	int n;
	if(!arg2 || !arg1) {
		return(1);
	}
	n=strcasecmp(arg1,arg2);
	return(n);
}


/**************************************************************************
 * Questa, al contrario della precedente, prende i considerazione solo
 * x caratteri nel confronto. Dove x e la lunghezza di arg1.
 * Quindi le due stringhe 'pip' e 'pippo' sono considerate uguali.
 */
int str_cmp2(const char* arg1, const char* arg2) {
	if(!arg2 || !arg1 || strlen(arg1) == 0) {
		return 1;
	}
	return(strncasecmp(arg1,arg2,strlen(arg1)));
}

/* returns: 0 if equal, 1 if arg1 > arg2, -1 if arg1 < arg2  */
/* scan 'till found different, end of both, or n reached     */
int strn_cmp(const char* arg1, const char* arg2, int n) {
	return(strncasecmp(arg1,arg2,n));

}

void sprintbit(unsigned long vektor, const char* names[], char* result) {
	long nr;

	*result = '\0';

	for(nr=0; vektor; vektor>>=1) {
		if(IS_SET(1, vektor)) {
			if(*names[nr] != '\n') {
				strcat(result,names[nr]);
				strcat(result," ");
			}
			else {
				strcat(result,"UNDEFINED");
				strcat(result," ");
			}
		}
		if(*names[nr] != '\n') {
			nr++;
		}
	}

	if(!*result) {
		strcat(result, "NOBITS");
	}
}

    /* sprintbit2: show extra_flags and extra_flags2 bits on objects */
void sprintbit2(unsigned long vektor, const char* names[], unsigned long vektor2, const char* names2[], char* result)
{
    long nr;
    *result = '\0';

    for(nr=0; vektor; vektor>>=1) {
        if(IS_SET(1, vektor)) {
            if(*names[nr] != '\n') {
                strcat(result,names[nr]);
                strcat(result," ");
            }
            else {
                strcat(result,"UNDEFINED");
                strcat(result," ");
            }
        }
        if(*names[nr] != '\n') {
            nr++;
        }
    }

    for(nr=0; vektor2; vektor2>>=1) {
        if(IS_SET(1, vektor2)) {
            if(*names2[nr] != '\n') {
                strcat(result,names2[nr]);
                strcat(result," ");
            }
            else {
                strcat(result,"UNDEFINED");
                strcat(result," ");
            }
        }
        if(*names2[nr] != '\n') {
            nr++;
        }
    }

    if(!*result) {
        strcat(result, "NOBITS");
    }
}

void sprinttype(int type, const char* names[], char* result) {
	int nr;

	for(nr=0; (*names[nr]!='\n'); nr++);
	if(type < nr) {
		strcpy(result,names[type]);
	}
	else {
		strcpy(result,"UNDEFINED");
	}
}


/* Calculate the REAL time passed over the last t2-t1 centuries (secs) */
struct time_info_data real_time_passed(time_t t2, time_t t1) {
	long secs;
	struct time_info_data now;

	secs = (long)(t2 - t1);

	now.hours = (secs/SECS_PER_REAL_HOUR) % 24;  /* 0..23 hours */
	secs -= SECS_PER_REAL_HOUR*now.hours;

	now.day = (secs/SECS_PER_REAL_DAY);          /* 0..34 days  */
	secs -= SECS_PER_REAL_DAY*now.day;

	now.month = -1;
	now.year  = -1;

	return now;
}




/* Calculate the MUD time passed over the last t2-t1 centuries (secs) */
struct time_info_data mud_time_passed(time_t t2, time_t t1) {
	long secs, monthsecs, daysecs, hoursecs;
	struct time_info_data now;

	/* eld (6-9-93) -- Hopefully, this will fix the negative month, day, etc.  */
	/*                 problems...                                             */

	if(t2 >= t1) {
		secs = (long)(t2 - t1);

		now.year = secs/SECS_PER_MUD_YEAR;

		monthsecs = secs % SECS_PER_MUD_YEAR;
		now.month = monthsecs/SECS_PER_MUD_MONTH;

		daysecs = monthsecs % SECS_PER_MUD_MONTH;
		now.day = daysecs/SECS_PER_MUD_DAY;

		hoursecs = daysecs % SECS_PER_MUD_DAY;
		now.hours = hoursecs/SECS_PER_MUD_HOUR;
	}
	else {
		secs = (long)(t1 - t2);

		now.year = secs/SECS_PER_MUD_YEAR;

		monthsecs = secs % SECS_PER_MUD_YEAR;
		now.month = monthsecs/SECS_PER_MUD_MONTH;

		daysecs = monthsecs % SECS_PER_MUD_MONTH;
		now.day = daysecs/SECS_PER_MUD_DAY;

		hoursecs = daysecs % SECS_PER_MUD_DAY;
		now.hours = hoursecs/SECS_PER_MUD_HOUR;

		if(now.hours) {
			now.hours = 24 - now.hours;
			now.day = now.day + 1;
		}
		if(now.day) {
			now.day = 35 - now.day;
			now.month = now.month + 1;
		}
		if(now.month) {
			now.month = 17 - now.month;
			now.year = now.year + 1;
		}
		if(now.year) {
			now.year = -now.year;
		}
	}
	return(now);

}

void mud_time_passed2(time_t t2, time_t t1, struct time_info_data* t) {
	long secs, monthsecs, daysecs, hoursecs;

	/* eld (6-9-93) -- Hopefully, this will fix the negative month, day, etc.  */
	/*                 problems...                                             */


	if(t2 >= t1) {
		secs = (long)(t2 - t1);

		t->year = secs/SECS_PER_MUD_YEAR;

		monthsecs = secs % SECS_PER_MUD_YEAR;
		t->month = monthsecs/SECS_PER_MUD_MONTH;

		daysecs = monthsecs % SECS_PER_MUD_MONTH;
		t->day = daysecs/SECS_PER_MUD_DAY;

		hoursecs = daysecs % SECS_PER_MUD_DAY;
		t->hours = hoursecs/SECS_PER_MUD_HOUR;
	}
	else {

		secs = (long)(t1 - t2);

		t->year = secs/SECS_PER_MUD_YEAR;

		monthsecs = secs % SECS_PER_MUD_YEAR;
		t->month = monthsecs/SECS_PER_MUD_MONTH;

		daysecs = monthsecs % SECS_PER_MUD_MONTH;
		t->day = daysecs/SECS_PER_MUD_DAY;

		hoursecs = daysecs % SECS_PER_MUD_DAY;
		t->hours = hoursecs/SECS_PER_MUD_HOUR;

		if(t->hours) {
			t->hours = 24 - t->hours;
			t->day = t->day + 1;
		}
		if(t->day) {
			t->day = 35 - t->day;
			t->month = t->month + 1;
		}
		if(t->month) {
			t->month = 17 - t->month;
			t->year = t->year + 1;
		}
		if(t->year) {
			t->year = -t->year;
		}
	}
}


void age2(struct char_data* ch, struct time_info_data* g) {

	mud_time_passed2(time(0),ch->player.time.birth, g);
	g->ayear=g->year;
	g->year += 17;   /* All players start at 17 */
	g->ayear=g->year; /* QUi resta l'eta' anagrafica */
	g->year = MAX(17,g->ayear+ch->AgeModifier); /* Qui l'eta' effettiva */

	/* I principi non invecchiavano oltre un certo limite, poi abbiamo esteso questa procedura a tutti */
//if (IS_PRINCE(ch))
//{
	g->year=(MIN(g->year,50));
	g->ayear=g->year;
//}



}
void age3(struct char_data* ch, struct time_info_data* g) {
	/* Riporta l'eta' incrementata secondo la  razza
	 * modifica solo estetica. In tutte le routine dove l'eta' viene usata
	 * si continua ad usare age2
	 * */
	age2(ch, g);

	switch(GET_RACE(ch)) {
	case RACE_ELVEN:
	case RACE_GOLD_ELF:
	case RACE_WILD_ELF:
	case RACE_SEA_ELF:
	case RACE_DARK_ELF:
		g->ayear+=240;
		g->year+=240;
		break;
	case RACE_DWARF:
	case RACE_DARK_DWARF:
		g->ayear+=150;
		g->year+=150;
		break;
	case RACE_HALFLING:
	case RACE_HALF_ELVEN:
		g->ayear+=70;
		g->year+=70;
		break;
	}
}

struct time_info_data age(struct char_data* ch) {
	struct time_info_data player_age;

	player_age = mud_time_passed(time(0),ch->player.time.birth);

	player_age.year += 17;   /* All players start at 17 */

	return(player_age);
}


char in_group(struct char_data* ch1, struct char_data* ch2) {
	bool rt=FALSE;
	PushStatus("In_group");
	rt=in_group_internal(ch1,ch2,FALSE);
	PopStatus();
	return(rt);
}
char in_group_strict(struct char_data* ch1, struct char_data* ch2) {
	bool rt=FALSE;
	PushStatus("In_group_strict");
	rt=in_group_internal(ch1,ch2,TRUE);
	PopStatus();
	return(rt);
}

char in_group_internal(struct char_data* ch1, struct char_data* ch2, int strict) {

	/*
	 *  possibilities:
	 *  1.  char is char2's master
	 *  2.  char2 is char's master
	 *  3.  char and char2 follow same.
	 *  4.  char rides char2
	 *  5.. char2 rides char
	 *
	 *   otherwise not true.
	 *
	 */
	if(!strict && in_clan(ch1,ch2)) {
		return(TRUE);
	}

	if(ch1 == ch2) {
		return(TRUE);
	}

	if((!ch1) || (!ch2)) {
		return(FALSE);
	}

	if((!ch1->master) && (!ch2->master)) {
		return(FALSE);
	}

	if(ch1==ch2->master) {
		return(TRUE);
	}

	if((ch1->master == ch2) && IS_AFFECTED(ch1,AFF_GROUP)) {
		return(TRUE);
	}

	if(ch1->master == ch2->master) {
		return(TRUE);
	}

	if(MOUNTED(ch1) == ch2 || RIDDEN(ch1) == ch2) {
		return(TRUE);
	}

	return(FALSE);
}

char in_clan(struct char_data* ch1, struct char_data* ch2) {

	/*
	 *  possibilities:
	 *  1.  char is char2's prince
	 *  2.  char2 is char's prince
	 *  3.  char and char2 have same prince.
	 *
	 *   otherwise not true.
	 *
	 */
	if(!IS_PC(ch1) || !(IS_PC(ch2))) {
		return(FALSE);
	}

	if(ch1 == ch2) {
		return(TRUE);
	}

	if((!ch1) || (!ch2)) {
		return(FALSE);
	}

	if((!HAS_PRINCE(ch1)) && (!HAS_PRINCE(ch2))) {
		return(FALSE);
	}

	if(IS_VASSALLOOF(ch1,GET_NAME(ch2))) {
		return(TRUE);
	}

	if(IS_VASSALLOOF(ch2,GET_NAME(ch1))) {
		return(TRUE);
	}

	if((!HAS_PRINCE(ch1)) || (!HAS_PRINCE(ch2))) {
		return(FALSE);
	}

	if(!strcasecmp(GET_PRINCE(ch1),GET_PRINCE(ch2))) {
		return(TRUE);
	}


	return(FALSE);
}


/*
* more new procedures
*/


/*
*  these two procedures give the player the ability to buy 2*bread
*  or put all.bread in bag, or put 2*bread in bag...
*/

char getall(char* name, char* newname) {
	char arg[ 41 ], tmpname[ 81 ], otname[ 81 ];
	char prd;

	arg[0] = '\0';
	tmpname[0] = '\0';
	otname[0] = '\0';

	sscanf(name, "%80s ", otname);     /* reads up to first space */

	if(strlen(otname) < 5) {
		return FALSE;
	}

	sscanf(otname,"%3s%c%80s", arg, &prd, tmpname);

	if(prd != '.') {
		return FALSE;
	}
	if(*tmpname == 0) {
		return FALSE;
	}
	if(strcmp(arg, "all")) {
		return FALSE;
	}

	while(*name != '.') {
		name++;
	}

	name++;

	for(; (*newname = *name) != 0; name++,newname++);

	return TRUE;
}


int getabunch(char* name, char*  newname) {
	int num=0;
	char tmpname[80];

	tmpname[0] = 0;
	sscanf(name,"%d*%s",&num,tmpname);
	if(tmpname[0] == '\0') {
		return(FALSE);
	}
	if(num < 1) {
		return(FALSE);
	}
	if(num>9) {
		num = 9;
	}

	while(*name != '*') {
		name++;
	}

	name++;

	for(; (*newname = *name) != 0; name++,newname++);

	return(num);

}


int DetermineExp(struct char_data* mob, int exp_flags) {
	/* Un intervento qui si riflette su TUTTI i mob */

	int base;
	int phit;
	int sab;

	if(exp_flags > 400) {
		mudlog(LOG_ERROR, "Exp flags on %s are > 400 (%d)", GET_NAME(mob),
			   exp_flags);
	}

	/*
	 * reads in the monster, and adds the flags together
	 * for simplicity, 1 exceptional ability is 2 special abilities
	 */

	if(GetMaxLevel(mob) < 0) {
		return(1);
	}

	switch(GetMaxLevel(mob)) {

	case 0:
		base = 1;
		phit = 1;
		sab =  1;
		break;

	case 1:
		base = 8;
		phit = 2;
		sab =  2;
		break;

	case 2:
		base = 12;
		phit = 2;
		sab =  3;
		break;

	case 3:
		base = 17;
		phit = 3;
		sab =  4;
		break;

	case 4:
		base = 23;
		phit = 3;
		sab =  6;
		break;

	case 5:
		base = 30;
		phit = 3;
		sab =  7;
		break;

	case 6:
		base = 37;
		phit = 3;
		sab =  9;
		break;

	case 7:
		base = 47;
		phit = 4;
		sab =  12;
		break;

	case 8:
		base = 57;
		phit = 4;
		sab  = 14;
		break;

	case 9:
		base = 70;
		phit = 4;
		sab  = 17;
		break;

	case 10:
		base = 84;
		phit  = 4;
		sab   = 21;
		break;

	case 11:
		base = 101;
		phit  = 5;
		sab   = 25;
		break;

	case 12:
		base = 122;
		phit  = 5;
		sab   = 30;
		break;

	case 13:
		base = 146;
		phit  = 5;
		sab   = 36;
		break;

	case 14:
		base = 175;
		phit  = 6;
		sab   = 44;
		break;

	case 15:
		base = 209;
		phit  = 6;
		sab   = 52;
		break;

	case 16:
		base = 251;
		phit  = 6;
		sab   = 63;
		break;

	case 17:
		base = 301;
		phit  = 7;
		sab   = 75;
		break;

	case 18:
		base = 362;
		phit  = 7;
		sab   = 90;
		break;

	case 19:
		base = 435;
		phit  = 8;
		sab   = 109;
		break;

	case 20:
		base = 524;
		phit  = 8;
		sab   = 131;
		break;

	case 21:
		base = 632;
		phit  = 9;
		sab   =  158;
		break;

	case 22:
		base = 762;
		phit  = 9;
		sab   = 190;
		break;

	case 23:
		base = 919;
		phit  = 10;
		sab   = 230;
		break;

	case 24:
		base = 1110;
		phit  = 10;
		sab   = 277;
		break;

	case 25:
		base = 1339;
		phit  = 11;
		sab   = 335;
		break;

	case 26:
		base = 1617;
		phit  = 12;
		sab   = 404;
		break;

	case 27:
		base = 1951;
		phit  = 12;
		sab   = 488;
		break;

	case 28:
		base = 2354;
		phit  = 13;
		sab   = 588;
		break;

	case 29:
		base = 2838;
		phit  = 14;
		sab   = 710;
		break;

	case 30:
		base = 3421;
		phit  = 15;
		sab   = 855;
		break;

	case 31:
		base = 4120;
		phit  = 16;
		sab   = 1030;
		break;

	case 32:
		base = 4960;
		phit  = 17;
		sab   = 1240;
		break;

	case 33:
		base = 5968;
		phit  = 18;
		sab   = 1492;
		break;

	case 34:
		base = 7175;
		phit = 19;
		sab  = 1794;
		break;

	case 35:
		base = 8621;
		phit  = 21;
		sab   = 2155;
		break;

	case 36:
		base = 10351;
		phit  = 22;
		sab   = 2588;
		break;

	case 37:
		base = 12422;
		phit  = 23;
		sab   = 3105;
		break;

	case 38:
		base = 14897;
		phit  = 25;
		sab   = 3724;
		break;

	case 39:
		base = 17857;
		phit = 26;
		sab  = 4464;
		break;

	case 40:
		base = 21394;
		phit  = 28;
		sab   = 5348;
		break;

	case 41:
		base = 25619;
		phit = 29;
		sab  = 6405;
		break;

	case 42:
		base = 30666;
		phit  = 31;
		sab   = 7666;
		break;

	case 43:
		base = 36692;
		phit = 33;
		sab  = 9173;
		break;

	case 44:
		base = 43889;
		phit  = 35;
		sab   = 10975;
		break;

	case 45:
		base = 52482;
		phit = 37;
		sab  = 13121;
		break;

	case 46:
		base = 62742;
		phit = 40;
		sab  = 15685;
		break;

	case 47:
		base = 74992;
		phit = 42;
		sab  = 18748;
		break;

	case 48:
		base = 89618;
		phit = 45;
		sab  = 22405;
		break;

	case 49:
		base = 107084;
		phit = 47;
		sab  = 26771;
		break;

	case 50:
		base = 127942;
		phit = 50;
		sab  = 31985;
		break;

	case 51:
		base = 150000;
		phit = 53;
		sab  = 37500;
		break;

	case 52:
		base = 170000;
		phit = 55;
		sab  = 42500;
		break;

	case 53:
		base = 190000;
		phit = 57;
		sab  = 47500;
		break;

	case 54:
		base = 210000;
		phit = 59;
		sab  = 52500;
		break;

	case 55:
		base = 230000;
		phit = 61;
		sab  = 57500;
		break;

	case 56:
		base = 250000;
		phit = 63;
		sab  = 62500;
		break;

	case 57:
		base = 275000;
		phit = 65;
		sab  = 68750;
		break;

	case 58:
		base = 300000;
		phit = 67;
		sab  = 75000;
		break;

	case 59:
		base = 325000;
		phit = 69;
		sab  = 71250;
		break;

	default:
		base = 350000;
		phit = 70;
		sab  = 87500;
		break;
	}

	return(base + (phit * GET_HIT(mob)) + (sab * exp_flags));
}

void down_river(unsigned long localPulse) {
	struct char_data* ch, *tmp;
	struct obj_data* obj_object, *next_obj;
	int rd;
	int _or;
	char buf[80];
	struct room_data* rp;

	for(ch = character_list; ch; ch = tmp) {
		tmp = ch->next;
		if(!IS_NPC(ch)) {
			if(ch->in_room != NOWHERE) {
				if(real_roomp(ch->in_room)->sector_type == SECT_WATER_NOSWIM) {
					if((real_roomp(ch->in_room))->river_speed > 0) {
						if((localPulse % (real_roomp(ch->in_room))->river_speed)==0) {
							if((real_roomp(ch->in_room))->river_dir <= 5 &&
									(real_roomp(ch->in_room))->river_dir >= 0) {
								rd = (real_roomp(ch->in_room))->river_dir;
								for(obj_object = (real_roomp(ch->in_room))->contents;
										obj_object; obj_object = next_obj) {
									next_obj = obj_object->next_content;
									if((real_roomp(ch->in_room))->dir_option[rd]) {
										obj_from_room(obj_object);
										obj_to_room(obj_object,
													(real_roomp(ch->in_room))->dir_option[rd]->to_room);
									}
								}
								/*
								 * flyers don't get moved
								 */
								if(!IS_AFFECTED(ch,AFF_FLYING) && !MOUNTED(ch)) {
									rp = real_roomp(ch->in_room);
									if(rp && rp->dir_option[rd] &&
											rp->dir_option[rd]->to_room &&
											(EXIT(ch, rd)->to_room != NOWHERE)) {
										if(ch->specials.fighting) {
											stop_fighting(ch);
										}
										if(IS_IMMORTAL(ch) &&
												IS_SET(ch->specials.act, PLR_NOHASSLE)) {
											send_to_char("L'acqua ti scorre intorno.\n\r", ch);
										}
										else {
											sprintf(buf,
													"La corrente ti trascina %s...\n\r",
													dirsTo[ rd ]);
											send_to_char(buf,ch);
											if(RIDDEN(ch)) {
												send_to_char(buf,RIDDEN(ch));
											}

											_or = ch->in_room;
											char_from_room(ch);
											if(RIDDEN(ch)) {
												char_from_room(RIDDEN(ch));
												char_to_room(RIDDEN(ch),
															 (real_roomp(_or))->dir_option[rd]->to_room);
											}
											char_to_room(ch,
														 (real_roomp(_or))->dir_option[rd]->to_room);

											do_look(ch, "\0",15);
											if(RIDDEN(ch)) {
												do_look(RIDDEN(ch), "\0",15);
											}
										}
										if(IS_SET(RM_FLAGS(ch->in_room), DEATH) &&
												GetMaxLevel(ch) < IMMORTALE) {
											if(RIDDEN(ch)) {
												NailThisSucker(RIDDEN(ch));
											}
											NailThisSucker(ch);
										}
									}
								}
							}
						}
					}
				}
			}
		}
	}
}


void RoomSave(struct char_data* ch, long start, long end) {
	char fn[80], temp[2048], dots[500];
	int rstart, rend, i, j, k, x;
	struct extra_descr_data* exptr;
	FILE* fp;
	struct room_data*     rp;
	struct room_direction_data*   rdd;

	rstart = start;
	rend = end;

	if(((rstart <= -1) || (rend <= -1)) ||
			((rstart > WORLD_SIZE) || (rend > WORLD_SIZE))) {
		send_to_char("I don't know those room #s.  make sure they are all\n\r",ch);
		send_to_char("contiguous.\n\r",ch);
		return;
	}

	send_to_char("Saving\n",ch);
	strcpy(dots, "\0");

	for(i=rstart; i<=rend; i++) {
		rp = real_roomp(i);
		if(rp==NULL) {
			continue;
		}

		sprintf(fn, "rooms/%d", i);

		if((fp = fopen(fn,"w")) == NULL) {
			send_to_char("Can't write to disk now..try later \n\r",ch);
			return;
		}

		strcat(dots, ".");

		/*
		 * strip ^Ms from description
		 */
		x = 0;

		if(!rp->description) {
			CREATE(rp->description, char, 8);
			strcpy(rp->description, "Empty");
		}

		for(k = 0; k <= (long)strlen(rp->description); k++) {
			if(rp->description[k] != 13) {
				temp[x++] = rp->description[k];
			}
		}
		temp[x] = '\0';

		if(temp[0] == '\0') {
			strcpy(temp, "Empty");
		}

		fprintf(fp,"#%ld\n%s~\n%s~\n",rp->number,rp->name, temp);
		if(!rp->tele_targ) {
			fprintf(fp,"%ld %ld %ld",rp->zone, rp->room_flags, rp->sector_type);
		}
		else {
			if(!IS_SET(TELE_COUNT, rp->tele_mask)) {
				fprintf(fp, "%ld %ld -1 %d %d %d %ld", rp->zone, rp->room_flags,
						rp->tele_time, rp->tele_targ,
						rp->tele_mask, rp->sector_type);
			}
			else {
				fprintf(fp, "%ld %ld -1 %d %d %d %d %ld", rp->zone, rp->room_flags,
						rp->tele_time, rp->tele_targ,
						rp->tele_mask, rp->tele_cnt, rp->sector_type);
			}
		}
		if(rp->sector_type == SECT_WATER_NOSWIM) {
			fprintf(fp," %d %d",rp->river_speed,rp->river_dir);
		}

		if(rp->room_flags & TUNNEL) {
			fprintf(fp, " %d ", (int)rp->moblim);
		}

		fprintf(fp,"\n");

		for(j=0; j<6; j++) {
			rdd = rp->dir_option[j];
			if(rdd) {
				fprintf(fp,"D%d\n",j);

				if(rdd->general_description && *rdd->general_description) {
					if(strlen(rdd->general_description) > 0) {
						temp[0] = '\0';
						x = 0;

						for(k = 0; k <= (long)strlen(rdd->general_description); k++) {
							if(rdd->general_description[k] != 13) {
								temp[x++] = rdd->general_description[k];
							}
						}
						temp[x] = '\0';

						fprintf(fp,"%s~\n", temp);
					}
					else {
						fprintf(fp,"~\n");
					}
				}
				else {
					fprintf(fp,"~\n");
				}

				if(rdd->keyword) {
					if(strlen(rdd->keyword)>0) {
						fprintf(fp, "%s~\n",rdd->keyword);
					}
					else {
						fprintf(fp, "~\n");
					}
				}
				else {
					fprintf(fp, "~\n");
				}

				fprintf(fp,"%ld ", rdd->exit_info);
				fprintf(fp,"%ld ", rdd->key);
				fprintf(fp,"%ld ", rdd->to_room);
				fprintf(fp,"%ld", rdd->open_cmd);
				fprintf(fp,"\n");
			}
		}

		/*
		 * extra descriptions..
		 */

		for(exptr = rp->ex_description; exptr; exptr = exptr->next) {
			x = 0;

			if(exptr->description) {
				for(k = 0; k <= (long)strlen(exptr->description); k++) {
					if(exptr->description[k] != 13) {
						temp[x++] = exptr->description[k];
					}
				}
				temp[x] = '\0';

				fprintf(fp,"E\n%s~\n%s~\n", exptr->keyword, temp);
			}
		}

		fprintf(fp,"S\n");
		fclose(fp);
	}

	send_to_char(dots, ch);
	send_to_char("\n\rDone\n\r",ch);
}


void RoomLoad(struct char_data* ch, int start, int end) {
	FILE* fp;
	int vnum, found = TRUE;
	char buf[80];
	struct room_data* rp;


	send_to_char("Searching and loading rooms\n\r",ch);

	for(vnum=start; vnum<=end; vnum++) {

		sprintf(buf, "rooms/%d", vnum);
		if((fp = fopen(buf,"r")) == NULL) {
			found = FALSE;
			send_to_char(".",ch);
			continue;
		}
		fscanf(fp,"#%*d\n");
		if((rp=real_roomp(vnum)) == 0) {   /* empty room */
			rp = (struct room_data*)malloc(sizeof(struct room_data));
			if(rp)
#ifdef CYGWIN
				bzero((char*)rp, sizeof(struct room_data));
#else
				bzero(rp, sizeof(struct room_data));
#endif
#if HASH
			/* this still does not work and needs work by someone */
			room_enter(&room_db, vnum, rp);
#else
			room_enter(room_db, vnum, rp);
#endif
			send_to_char("+",ch);
		}
		else {
			if(rp->people) {
				act("$n reaches down and scrambles reality.", FALSE, ch, NULL,
					rp->people, TO_ROOM);
			}
			cleanout_room(rp);
			send_to_char("-",ch);
		}

		rp->number = vnum;
		load_one_room(fp, rp);
		fclose(fp);
	}

	if(!found) {
		send_to_char("\n\rThe room number(s) that you specified could not all be found.\n\r",ch);
	}
	else {
		send_to_char("\n\rDone.\n\r",ch);
	}

}



void fake_setup_dir(FILE* fl, long room, int dir) {
	int tmp;
	char* temp;

	temp = fread_string(fl); /* descr */
	if(temp) {
		free(temp);
	}
	temp = fread_string(fl); /* key */
	if(temp) {
		free(temp);
	}

	fscanf(fl, " %d ", &tmp);
	fscanf(fl, " %d ", &tmp);
	fscanf(fl, " %d ", &tmp);
}


int IsHumanoid(struct char_data* ch) {
	/* these are all very arbitrary */

	switch(GET_RACE(ch)) {
	case RACE_HUMAN:
	case RACE_GNOME:
	case RACE_DEEP_GNOME:
	case RACE_ELVEN:
	case RACE_GOLD_ELF:
	case RACE_WILD_ELF:
	case RACE_SEA_ELF:
	case RACE_DWARF:
	case RACE_DARK_DWARF:
	case RACE_HALFLING:
	case RACE_ORC:
	case RACE_LYCANTH:
	case RACE_UNDEAD:
	case RACE_UNDEAD_VAMPIRE :
	case RACE_UNDEAD_LICH    :
	case RACE_UNDEAD_WIGHT   :
	case RACE_UNDEAD_GHAST   :
	case RACE_UNDEAD_SPECTRE :
	case RACE_UNDEAD_ZOMBIE  :
	case RACE_UNDEAD_SKELETON :
	case RACE_UNDEAD_GHOUL    :
	case RACE_GIANT:
	case RACE_GIANT_HILL   :
	case RACE_GIANT_FROST  :
	case RACE_GIANT_FIRE   :
	case RACE_GIANT_CLOUD  :
	case RACE_GIANT_STORM  :
	case RACE_GIANT_STONE  :
	case RACE_GOBLIN:
	case RACE_DEVIL:
	case RACE_TROLL:
	case RACE_VEGMAN:
	case RACE_MFLAYER:
	case RACE_ENFAN:
	case RACE_PATRYN:
	case RACE_SARTAN:
	case RACE_ROO:
	case RACE_SMURF:
	case RACE_TROGMAN:
	case RACE_LIZARDMAN:
	case RACE_SKEXIE:
	case RACE_TYTAN:
	case RACE_DARK_ELF:
	case RACE_GOLEM:
	case RACE_DEMON:
	case RACE_DRAAGDIM:
	case RACE_ASTRAL:
	case RACE_GOD:
	case RACE_HALF_ELVEN:
	case RACE_HALF_ORC:
	case RACE_HALF_OGRE:
	case RACE_HALF_GIANT:
	case RACE_GNOLL:
		return(TRUE);
		break;

	default:
		return(FALSE);
		break;
	}

}

int IsRideable(struct char_data* ch) {
	if(IS_NPC(ch) && !IS_PC(ch)) {
		switch(GET_RACE(ch)) {
		case RACE_HORSE:
		case RACE_DRAGON:
		case RACE_DRAGON_RED    :
		case RACE_DRAGON_BLACK  :
		case RACE_DRAGON_GREEN  :
		case RACE_DRAGON_WHITE  :
		case RACE_DRAGON_BLUE   :
		case RACE_DRAGON_SILVER :
		case RACE_DRAGON_GOLD   :
		case RACE_DRAGON_BRONZE :
		case RACE_DRAGON_COPPER :
		case RACE_DRAGON_BRASS  :
			return(TRUE);
			break;
		default:
			return(FALSE);
			break;
		}
	}
	else {
		return(FALSE);
	}
}

int IsAnimal(struct char_data* ch) {

	switch(GET_RACE(ch)) {
	case RACE_PREDATOR:
	case RACE_FISH:
	case RACE_BIRD:
	case RACE_HERBIV:
	case RACE_REPTILE:
	case RACE_LABRAT:
	case RACE_ROO:
	case RACE_INSECT:
	case RACE_ARACHNID:
		return(TRUE);
		break;
	default:
		return(FALSE);
		break;
	}

}

int IsVeggie(struct char_data* ch) {

	switch(GET_RACE(ch)) {
	case RACE_PARASITE:
	case RACE_SLIME:
	case RACE_TREE:
	case RACE_VEGGIE:
	case RACE_VEGMAN:
		return(TRUE);
		break;
	default:
		return(FALSE);
		break;
	}

}

int IsUndead(struct char_data* ch) {

	switch(GET_RACE(ch)) {
	case RACE_UNDEAD:
	case RACE_GHOST:
	case RACE_UNDEAD_VAMPIRE :
	case RACE_UNDEAD_LICH    :
	case RACE_UNDEAD_WIGHT   :
	case RACE_UNDEAD_GHAST   :
	case RACE_UNDEAD_SPECTRE :
	case RACE_UNDEAD_ZOMBIE  :
	case RACE_UNDEAD_SKELETON :
	case RACE_UNDEAD_GHOUL    :

		return(TRUE);
		break;
	default:
		return(FALSE);
		break;
	}

}

int IsLycanthrope(struct char_data* ch) {
	switch(GET_RACE(ch)) {
	case RACE_LYCANTH:
		return(TRUE);
		break;
	default:
		return(FALSE);
		break;
	}

}

int IsDiabolic(struct char_data* ch) {

	switch(GET_RACE(ch)) {
	case RACE_DEMON:
	case RACE_DEVIL:
		return(TRUE);
		break;
	default:
		return(FALSE);
		break;
	}

}

int IsReptile(struct char_data* ch) {
	switch(GET_RACE(ch)) {
	case RACE_REPTILE:
	case RACE_DRAGON:
	case RACE_DRAGON_RED    :
	case RACE_DRAGON_BLACK  :
	case RACE_DRAGON_GREEN  :
	case RACE_DRAGON_WHITE  :
	case RACE_DRAGON_BLUE   :
	case RACE_DRAGON_SILVER :
	case RACE_DRAGON_GOLD   :
	case RACE_DRAGON_BRONZE :
	case RACE_DRAGON_COPPER :
	case RACE_DRAGON_BRASS  :
	case RACE_DINOSAUR:
	case RACE_SNAKE:
	case RACE_TROGMAN:
	case RACE_LIZARDMAN:
	case RACE_SKEXIE:
		return(TRUE);
		break;
	default:
		return(FALSE);
		break;
	}
}

int HasHands(struct char_data* ch) {

	if(IsHumanoid(ch)) {
		return(TRUE);
	}
	if(IsUndead(ch)) {
		return(TRUE);
	}
	if(IsLycanthrope(ch)) {
		return(TRUE);
	}
	if(IsDiabolic(ch)) {
		return(TRUE);
	}
	if(GET_RACE(ch) == RACE_GOLEM) {
		return(TRUE);
	}
	return(FALSE);
}

int IsPerson(struct char_data* ch) {

	switch(GET_RACE(ch)) {
	case RACE_HUMAN:
	case RACE_ELVEN:
	case RACE_DARK_ELF:
	case RACE_DWARF:
	case RACE_DARK_DWARF:
	case RACE_HALFLING:
	case RACE_GNOME:
	case RACE_DEEP_GNOME:
	case RACE_GOLD_ELF:
	case RACE_WILD_ELF:
	case RACE_SEA_ELF:
	case RACE_GOBLIN:
	case RACE_ORC:
	case RACE_TROLL:
		return(TRUE);
		break;

	default:
		return(FALSE);
		break;

	}
}

int IsGiantish(struct char_data* ch) {
	switch(GET_RACE(ch)) {
	case RACE_ENFAN:
	case RACE_GOBLIN:  /* giantish for con's only... */
	case RACE_ORC:

	case RACE_GIANT:
	case RACE_GIANT_HILL   :
	case RACE_GIANT_FROST  :
	case RACE_GIANT_FIRE   :
	case RACE_GIANT_CLOUD  :
	case RACE_GIANT_STORM  :
	case RACE_GIANT_STONE  :
	case RACE_TYTAN:
	case RACE_TROLL:
	case RACE_DRAAGDIM:

	case RACE_HALF_ORC:
	case RACE_HALF_OGRE:
	case RACE_HALF_GIANT:
		return(TRUE);
	default:
		return(FALSE);
		break;
	}
}

int IsSmall(struct char_data* ch) {
	switch(GET_RACE(ch)) {
	case RACE_SMURF:
	case RACE_GNOME:
	case RACE_HALFLING:
	case RACE_GOBLIN:
	case RACE_ENFAN:
	case RACE_DEEP_GNOME:
		return(TRUE);
	default:
		return(FALSE);
	}
}

int IsGiant(struct char_data* ch) {
	switch(GET_RACE(ch)) {
	case RACE_GIANT:
	case RACE_GIANT_HILL   :
	case RACE_GIANT_FROST  :
	case RACE_GIANT_FIRE   :
	case RACE_GIANT_CLOUD  :
	case RACE_GIANT_STORM  :
	case RACE_GIANT_STONE  :
	case RACE_HALF_GIANT        :
	case RACE_TYTAN:
	case RACE_GOD:
		return(TRUE);
	case RACE_TROLL:
		return((number(1,100)>20));
	default:
		/*if (IS_SET(ch->specials.act, ACT_HUGE))
		  return(TRUE);
		else*/
		return(FALSE);
	}
}

int IsExtraPlanar(struct char_data* ch) {
	switch(GET_RACE(ch)) {
	case RACE_DEMON:
	case RACE_DEVIL:
	case RACE_PLANAR:
	case RACE_ELEMENT:
	case RACE_ASTRAL:
	case RACE_GOD:
		return(TRUE);
		break;
	default:
		return(FALSE);
		break;
	}
}

int IsOther(struct char_data* ch) {

	switch(GET_RACE(ch)) {
	case RACE_MFLAYER:
	case RACE_SPECIAL:
	case RACE_GOLEM:
	case RACE_ELEMENT:
	case RACE_PLANAR:
	case RACE_LYCANTH:
		return(TRUE);
	default:
		return(FALSE);
		break;
	}
}

int IsGodly(struct char_data* ch) {
	if(GET_RACE(ch) == RACE_GOD) {
		return(TRUE);
	}
	if(GET_RACE(ch) == RACE_DEMON || GET_RACE(ch) == RACE_DEVIL)
		if(GetMaxLevel(ch) >= 45) {
			return(TRUE);
		}

	return FALSE;
}


int IsDragon(struct char_data* ch) {
	switch(GET_RACE(ch)) {
	case RACE_DRAGON:
	case RACE_DRAGON_RED    :
	case RACE_DRAGON_BLACK  :
	case RACE_DRAGON_GREEN  :
	case RACE_DRAGON_WHITE  :
	case RACE_DRAGON_BLUE   :
	case RACE_DRAGON_SILVER :
	case RACE_DRAGON_GOLD   :
	case RACE_DRAGON_BRONZE :
	case RACE_DRAGON_COPPER :
	case RACE_DRAGON_BRASS  :
		return(TRUE);
		break;
	default:
		return(FALSE);
		break;
	}
}

void SetHunting(struct char_data* ch, struct char_data* tch) {
	int persist, dist;

#if NOTRACK
	return;
#endif

	if(!ch || !tch) {
		mudlog(LOG_SYSERR, "!ch || !tch in SetHunting");
		return;
	}

	persist =  GetMaxLevel(ch);
	persist *= (int) GET_ALIGNMENT(ch) / 100;

	if(persist < 0) {
		persist = -persist;
	}

	dist = GET_ALIGNMENT(tch) - GET_ALIGNMENT(ch);
	dist = (dist > 0) ? dist : -dist;
	if(Hates(ch, tch)) {
		dist *=2;
	}

	SET_BIT(ch->specials.act, ACT_HUNTING);
	ch->specials.hunting = tch;
	ch->hunt_dist = dist;
	ch->persist = persist;
	ch->old_room = ch->in_room;
#if 0
	if(GetMaxLevel(tch) >= IMMORTAL) {
		sprintf(buf, ">>%s is hunting you from %s\n\r",
				(ch->player.short_descr[0]?ch->player.short_descr:"(null)"),
				(real_roomp(ch->in_room)->name[0]?real_roomp(ch->in_room)->name:"(null)"));
		send_to_char(buf, tch);
	}
#endif

}


void CallForGuard(struct char_data* ch, struct char_data* vict, int lev,
				  int area) {
	struct char_data* i;
	int type1, type2;

	switch(area) {
	case MIDGAARD:
		type1 = 3060;
		type2 = 3069;
		break;
	case NEWTHALOS:
		type1 = 3661;
		type2 = 3682;
		break;
	case TROGCAVES:
		type1 = 21114;
		type2 = 21118;
		break;
	case OUTPOST:
		type1 = 21138;
		type2 = 21139;
		break;

	case PRYDAIN:
		type1 = 6606;
		type2 = 6614;
		break;

	default:
		type1 = 3060;
		type2 = 3069;
		break;
	}


	if(lev == 0) {
		lev = 3;
	}

	for(i = character_list; i && lev>0; i = i->next) {
		if(IS_NPC(i) && (i != ch)) {
			if(!i->specials.fighting) {
				if(mob_index[i->nr].iVNum == type1) {
					if(number(1,6) == 1) {
						if(!IS_SET(i->specials.act, ACT_HUNTING)) {
							if(vict) {
								SetHunting(i, vict);
								lev--;
							}
						}
					}
				}
				else if(mob_index[i->nr].iVNum == type2) {
					if(number(1,6) == 1) {
						if(!IS_SET(i->specials.act, ACT_HUNTING)) {
							if(vict) {
								SetHunting(i, vict);
								lev-=2;
							}
						}
					}
				}
			}
		}
	}
}

void CallForMobs(struct char_data* pChar, struct char_data* pVict,
				 int iLevel, int iMobToCall) {
	struct char_data* pMobToCall;


	for(pMobToCall = character_list; pMobToCall && iLevel > 0;
			pMobToCall = pMobToCall->next) {
		if(!IS_PC(pMobToCall) && pMobToCall != pChar) {
			if(!pMobToCall->specials.fighting) {
				if(mob_index[ pMobToCall->nr ].iVNum == iMobToCall) {
					if(number(1, 3) == 1) {
						if(!IS_SET(pMobToCall->specials.act, ACT_HUNTING)) {
							if(pVict) {
								SetHunting(pMobToCall, pVict);
								iLevel--;
							}
						}
					}
				}
			}
		}
	}
}

int StandUp(struct char_data* ch) {
	if(GET_POS(ch) < POSITION_FIGHTING &&
			GET_POS(ch) > POSITION_STUNNED && !ch->desc) {
		if(ch->points.hit > (ch->points.max_hit / 2)) {
			act("$n si alza velocemente.", 1, ch,0,0,TO_ROOM);
		}
		else if(ch->points.hit > (ch->points.max_hit / 6)) {
			act("$n si alza lentamente.", 1, ch,0,0,TO_ROOM);
		}
		else {
			act("$n si alza con fatica.", 1, ch,0,0,TO_ROOM);
		}
		GET_POS(ch) = POSITION_STANDING;
		return TRUE;
	}
	return FALSE;
}


void MakeNiftyAttack(struct char_data* ch) {
	int num;


	if(!ch->skills) {
		SpaceForSkills(ch);
		return;
	}

	if(!ch || !ch->skills) {
		mudlog(LOG_SYSERR, "!ch or !ch->skills in MakeNiftyAttack() in "
			   "MakeNiftyAttack (utility.c)");
		return;
	}
	if(!ch->specials.fighting) {
		return;
	}

	num = number(1,4);
	if(num <= 2) {
		if(!ch->skills[SKILL_BASH].learned) {
			ch->skills[SKILL_BASH].learned = 10 + GetMaxLevel(ch)*4;
		}
		do_bash(ch, GET_NAME(ch->specials.fighting), 0);
	}
	else if(num == 3) {
		if(ch->specials.fighting->equipment[WIELD]) {
			if(!ch->skills[SKILL_DISARM].learned) {
				ch->skills[SKILL_DISARM].learned = 10 + GetMaxLevel(ch)*4;
			}
			do_disarm(ch, GET_NAME(ch->specials.fighting), 0);
		}
		else {
			if(!ch->skills[SKILL_KICK].learned) {
				ch->skills[SKILL_KICK].learned = 10 + GetMaxLevel(ch)*4;
			}
			do_kick(ch, GET_NAME(ch->specials.fighting), 0);
		}
	}
	else {
		if(!ch->skills[SKILL_KICK].learned) {
			ch->skills[SKILL_KICK].learned = 10 + GetMaxLevel(ch)*4;
		}
		do_kick(ch, GET_NAME(ch->specials.fighting), 0);
	}

}


void FighterMove(struct char_data* ch) {
	struct char_data* pFriend;

	if(!ch->skills) {
		SET_BIT(ch->player.iClass, CLASS_WARRIOR);
		SpaceForSkills(ch);
	}

	if(ch->specials.fighting) {
		pFriend = ch->specials.fighting->specials.fighting;
		if(pFriend == NULL || pFriend == ch) {
			MakeNiftyAttack(ch);
		}
		else {
			/* rescue on a 1 or 2, other on a 3 or 4 */
			if((IS_AFFECTED(ch, AFF_CHARM) && ch->master == pFriend &&
					GET_MAX_HIT(ch) / 4 < GET_HIT(ch) &&
					GET_MAX_HIT(ch->master) / 2 > GET_HIT(ch->master)) ||
					(GET_RACE(pFriend) == GET_RACE(ch) &&
					 GET_HIT(pFriend) < GET_HIT(ch))) {
				if(!ch->skills[SKILL_RESCUE].learned) {
					ch->skills[SKILL_RESCUE].learned = GetMaxLevel(ch)*3+30;
				}
				do_rescue(ch, GET_NAME(pFriend), 0);
			}
			else {
				MakeNiftyAttack(ch);
			}
		}
	}
	else {
		return;
	}
}



void MonkMove(struct char_data* ch) {

	if(!ch->skills) {
		SpaceForSkills(ch);
		ch->skills[SKILL_DODGE].learned = GetMaxLevel(ch)+50;
		SET_BIT(ch->player.iClass, CLASS_MONK);
	}

	if(!ch->specials.fighting) {
		return;
	}

	if(GET_POS(ch) < POSITION_FIGHTING) {
		if(!ch->skills[SKILL_SPRING_LEAP].learned) {
			ch->skills[SKILL_SPRING_LEAP].learned = (GetMaxLevel(ch)*3)/2+25;
		}
		do_springleap(ch, GET_NAME(ch->specials.fighting), 0);
		return;
	}
	else {
		char buf[100];

		/* Commented out as a temporary fix to monks fleeing challenges. */
		/* Was easier than rooting around in the spec_proc for the monk */
		/* challenge for it, which is proobably what should be done. */
		/* jdb - was commented back in with the change to use
		   command_interpreter */

		if(GET_HIT(ch) < GET_MAX_HIT(ch)/20) {
			if(!ch->skills[SKILL_RETREAT].learned) {
				ch->skills[SKILL_RETREAT].learned = GetMaxLevel(ch)*2+10;
			}
			strcpy(buf, "flee");
			command_interpreter(ch, buf);
			return;
		}
		else {
			if(GetMaxLevel(ch)>30 && !number(0,4)) {
				if(GetMaxLevel(ch->specials.fighting) <= GetMaxLevel(ch)) {
					if(GET_MAX_HIT(ch->specials.fighting) < 2*GET_MAX_HIT(ch)) {
						if((!affected_by_spell(ch->specials.fighting, SKILL_QUIV_PALM)) &&
								(!affected_by_spell(ch, SKILL_QUIV_PALM)) &&
								ch->in_room == 551) {
							if(ch->specials.fighting->skills[SKILL_QUIV_PALM].learned &&
									ch->in_room == 551) {
								if(!ch->skills[SKILL_QUIV_PALM].learned && ch->in_room == 551) {
									ch->skills[SKILL_QUIV_PALM].learned = GetMaxLevel(ch)*2-5;
								}
								do_quivering_palm(ch, GET_NAME(ch->specials.fighting), 0);
								return;
							}
						}
					}
				}
			}
			if(ch->specials.fighting->equipment[WIELD]) {
				if(!ch->skills[SKILL_DISARM].learned) {
					ch->skills[SKILL_DISARM].learned = (GetMaxLevel(ch)*3)/2+25;
				}
				do_disarm(ch, GET_NAME(ch->specials.fighting), 0);
				return;
			}
			if(!ch->skills[SKILL_KICK].learned) {
				ch->skills[SKILL_KICK].learned = (GetMaxLevel(ch)*3)/2+25;
			}
			do_kick(ch, GET_NAME(ch->specials.fighting), 0);
		}
	}
}

void DevelopHatred(struct char_data* ch, struct char_data* v) {
	int diff, patience, var;

	if(Hates(ch, v)) {
		return;
	}

	if(ch == v) {
		return;
	}

	diff = GET_ALIGNMENT(ch) - GET_ALIGNMENT(v);
	if(diff < 0) {
		diff = -diff;
	}

	diff /= 20;

	if(GET_MAX_HIT(ch)) {
		patience = (100 * GET_HIT(ch)) / GET_MAX_HIT(ch);
	}
	else {
		patience = 10;
	}

	var = number(1,40) - 20;

	if(patience+var < diff) {
		AddHated(ch, v);
	}

}

int HasObject(struct char_data* ch, int ob_num) {
	int j, found;
	struct obj_data* i;

	/*
	 * equipment too
	 */

	found = 0;

	for(j=0; j<MAX_WEAR; j++)
		if(ch->equipment[j]) {
			found += RecCompObjNum(ch->equipment[j], ob_num);
		}

	if(found > 0) {
		return(TRUE);
	}

	/* carrying  */
	for(i = ch->carrying; i; i = i->next_content) {
		found += RecCompObjNum(i, ob_num);
	}

	if(found > 0) {
		return(TRUE);
	}
	else {
		return(FALSE);
	}
}


int room_of_object(struct obj_data* obj) {
	if(obj->in_room != NOWHERE) {
		return obj->in_room;
	}
	else if(obj->carried_by) {
		return obj->carried_by->in_room;
	}
	else if(obj->equipped_by) {
		return obj->equipped_by->in_room;
	}
	else if(obj->in_obj) {
		return room_of_object(obj->in_obj);
	}
	else {
		return NOWHERE;
	}
}

struct char_data* char_holding(struct obj_data* obj) {
	if(obj->in_room != NOWHERE) {
		return NULL;
	}
	else if(obj->carried_by) {
		return obj->carried_by;
	}
	else if(obj->equipped_by) {
		return obj->equipped_by;
	}
	else if(obj->in_obj) {
		return char_holding(obj->in_obj);
	}
	else {
		return NULL;
	}
}


int RecCompObjNum(struct obj_data* o, int obj_num) {

	int total=0;
	struct obj_data* i;

	if(o->item_number >= 0 && obj_index[o->item_number].iVNum == obj_num) {
		total = 1;
	}

	if(ITEM_TYPE(o) == ITEM_CONTAINER) {
		for(i = o->contains; i; i = i->next_content) {
			total += RecCompObjNum(i, obj_num);
		}
	}
	return(total);

}

void RestoreChar(struct char_data* ch) {

	GET_MANA(ch) = GET_MAX_MANA(ch);
	GET_HIT(ch) = GET_MAX_HIT(ch);
	GET_MOVE(ch) = GET_MAX_MOVE(ch);
	if(GetMaxLevel(ch) < IMMORTALE) {
		GET_COND(ch,THIRST) = 24;
		GET_COND(ch,FULL) = 24;
	}
	else {
		GET_COND(ch,THIRST) = -1;
		GET_COND(ch,FULL) = -1;
	}

}


void RemAllAffects(struct char_data* ch) {
	spell_dispel_magic(MAESTRO_DEI_CREATORI,ch,ch,0);

}

int CheckForBlockedMove
(struct char_data* ch, int cmd, const char* arg, int room, int dir, int iClass) {

	char buf[256], buf2[256];

	if(cmd>6 || cmd<1) {
		return(FALSE);
	}

	strcpy(buf,  "The guard humiliates you, and block your way.\n\r");
	strcpy(buf2, "The guard humiliates $n, and blocks $s way.");

	if((IS_NPC(ch) && (IS_POLICE(ch))) || (GetMaxLevel(ch) >= DIO) ||
			(IS_AFFECTED(ch, AFF_SNEAK))) {
		return(FALSE);
	}


	if((ch->in_room == room) && (cmd == dir+1)) {
		if(!HasClass(ch,iClass))  {
			act(buf2, FALSE, ch, 0, 0, TO_ROOM);
			send_to_char(buf, ch);
			return TRUE;
		}
	}
	return FALSE;

}


void TeleportPulseStuff(unsigned long localPulse) {
	register struct char_data* ch;
	struct char_data* next;
	int tick, tm;
	struct room_data* rp, *dest;
	struct obj_data* obj_object, *temp_obj;
	struct room_data* apTeleportRoom[ 1000 ];
	int iMaxTeleportRoom = 0;

	tm = localPulse % PULSE_MOBILE;    /* this is dependent on P_M = 3*P_T */

	if(tm == 0) {
		tick = 0;
	}
	else if(tm == PULSE_TELEPORT) {
		tick = 1;
	}
	else if(tm == PULSE_TELEPORT*2) {
		tick = 2;
	}
	else {
		mudlog(LOG_SYSERR,
			   "TeleportPulseStuff called with pulse not %% of PULSE_MOBILE.");
		tick = 3;
	}

	for(ch = character_list; ch; ch = next) {
		next = ch->next;
		if(IS_MOB(ch)) {
			if(ch->specials.tick == tick && !ch->specials.fighting &&
					!ch->specials.tick_to_lag) {
				mobile_activity(ch);
			}
		}
	}

	/* Non posso tenere uniti i due loop, perche' dopo mobile_activity ch
	 * potrebbe non essere piu' valido */
	for(ch = character_list; ch; ch = next) {
		next = ch->next;
		rp = real_roomp(ch->in_room);
		if(rp &&
				rp->tele_targ > 0 &&
				rp->tele_targ != rp->number &&
				rp->tele_time > 0 &&
				(localPulse % rp->tele_time)==0) {
			if(IS_SET(rp->tele_mask, TELE_COUNT) && rp->tele_cnt > 0) {
				ch->nTeleCount--;
				if(ch->nTeleCount) {
					continue;
				}
			}

			dest = real_roomp(rp->tele_targ);
			if(!dest) {
				mudlog(LOG_ERROR, "invalid tele_targ of room %ld", ch->in_room);
				continue;
			}

			/* Qui c'e' un problema: Se la stanza di destinazione e' un altra
			 * Teleport e c'e' un personaggio non ancora incontrato nel loop, ma
			 * che deve essere trasportato, gli oggetti viaggeranno anche con
			 * l'altro personaggio. */
			obj_object = rp->contents;
			while(obj_object) {
				temp_obj = obj_object->next_content;
				obj_from_room(obj_object);
				obj_to_room(obj_object, rp->tele_targ);
				obj_object = temp_obj;
			}

			char_from_room(ch);   /* the list of people in the room has changed */
			char_to_room(ch, rp->tele_targ);

			if(IS_SET(TELE_LOOK, rp->tele_mask) && IS_PC(ch)) {
				do_look(ch, "\0", 0);
			}

			if(IS_SET(dest->room_flags, DEATH) && !IS_IMMORTAL(ch)) {
				if(!IS_PC(ch)) {
					mudlog(LOG_ERROR, "%s hit a death trap.", ch->player.short_descr);
				}
				NailThisSucker(ch);
				continue;
			}
			if(dest->sector_type == SECT_AIR) {
				check_falling(ch);
			}

			apTeleportRoom[ iMaxTeleportRoom ] = rp;
			iMaxTeleportRoom++;
		}
	}

	while(iMaxTeleportRoom) {
		iMaxTeleportRoom--;
		rp = apTeleportRoom[ iMaxTeleportRoom ];

		if(IS_SET(TELE_RANDOM, rp->tele_mask)) {
			rp->tele_time = number(1, 10) * PULSE_TELEPORT;
		}
	}
}

void RiverPulseStuff(unsigned long localPulse) {
	struct descriptor_data* i;
	register struct char_data* ch;
	struct char_data* tmp;
	register struct obj_data* obj_object;
	struct obj_data* next_obj;
	int rd, _or;
	char buf[80], buffer[100];
	struct room_data* rp;

	for(i = descriptor_list; i; i=i->next) {
		if(!i->connected) {
			ch = i->character;

			if(IS_PC(ch) || RIDDEN(ch)) {
				if(ch->in_room != NOWHERE) {
					if((real_roomp(ch->in_room)->sector_type == SECT_WATER_NOSWIM) ||
							(real_roomp(ch->in_room)->sector_type == SECT_UNDERWATER)) {

						if((real_roomp(ch->in_room))->river_speed > 0) {
							if((localPulse % (real_roomp(ch->in_room))->river_speed)==0) {
								if(((real_roomp(ch->in_room))->river_dir<=5)&&
										((real_roomp(ch->in_room))->river_dir>=0)) {
									rd = (real_roomp(ch->in_room))->river_dir;
									for(obj_object = (real_roomp(ch->in_room))->contents;
											obj_object; obj_object = next_obj) {
										next_obj = obj_object->next_content;
										if((real_roomp(ch->in_room))->dir_option[rd]) {
											obj_from_room(obj_object);
											obj_to_room(obj_object,
														(real_roomp(ch->in_room))->dir_option[rd]->to_room);
										}
									}
									/*
									 * flyers dont get moved
									 */
									if(IS_IMMORTAL(ch) &&
											IS_SET(ch->specials.act, PLR_NOHASSLE)) {
										send_to_char("The waters swirl and eddy about you.\n\r",
													 ch);
									}
									else {
										if(!IS_AFFECTED(ch,AFF_FLYING) ||
												(real_roomp(ch->in_room)->sector_type ==
												 SECT_UNDERWATER)) {
											if(!MOUNTED(ch)) {
												rp = real_roomp(ch->in_room);
												if(rp && rp->dir_option[rd] &&
														rp->dir_option[rd]->to_room &&
														(EXIT(ch, rd)->to_room != NOWHERE)) {
													if(ch->specials.fighting) {
														stop_fighting(ch);
													}
													sprintf(buf, "La corrente ti porta %s...\n\r",
															dirsTo[ rd ]);
													send_to_char(buf,ch);
													if(RIDDEN(ch)) {
														send_to_char(buf,RIDDEN(ch));
													}
													_or = ch->in_room;
													char_from_room(ch);
													if(RIDDEN(ch)) {
														char_from_room(RIDDEN(ch));
														char_to_room(RIDDEN(ch),
																	 (real_roomp(_or))->dir_option[rd]->to_room);
													}
													char_to_room(ch,
																 (real_roomp(_or))->dir_option[rd]->to_room);
													do_look(ch, "\0",15);
													if(RIDDEN(ch)) {
														do_look(RIDDEN(ch), "\0",15);
													}


													if(IS_SET(RM_FLAGS(ch->in_room), DEATH) &&
															GetMaxLevel(ch) < IMMORTALE) {
														NailThisSucker(ch);
														if(RIDDEN(ch)) {
															NailThisSucker(RIDDEN(ch));
														}
													}
												}
											}
										}
									}                /* end of else for is_immort() */
								}
							}
						}
					}
				}
			}
		}
	} /* end for descriptor */



	if(!number(0,4)) {
		for(ch = character_list; ch; ch = tmp) {
			tmp = ch->next;

			/*
			 *   mobiles
			 */
			if(!IS_PC(ch) && !ch->specials.fighting && ch->player.sounds &&
					number(0,5) == 0) {
				if(ch->specials.default_pos > POSITION_SLEEPING) {
					if(GET_POS(ch) > POSITION_SLEEPING) {
						/*
						 *  Make the sound;
						 */
						MakeNoise(ch->in_room, ch->player.sounds,
								  ch->player.distant_snds);
					}
					else if(GET_POS(ch) == POSITION_SLEEPING) {
						/*
						 * snore
						 */
						sprintf(buffer, "%s russa sonoramente.\n\r",
								ch->player.short_descr);
						MakeNoise(ch->in_room, buffer,
								  "Qualcuno russa sonoramente qui vicino.\n\r");
					}
				}
				else if(GET_POS(ch) == ch->specials.default_pos) {
					/*
					* Make the sound
					*/
					MakeNoise(ch->in_room, ch->player.sounds, ch->player.distant_snds);
				}
			}
		}
	}

	struct obj_data* pObj, *pNextObj;
	for(pObj = object_list; pObj ; pObj = pNextObj) {
		pNextObj = pObj->next; /* Next in object list */

		/* Sound objects */
		if(ITEM_TYPE(pObj) == ITEM_AUDIO) {
			if((pObj->obj_flags.value[0] &&
					(localPulse % pObj->obj_flags.value[0]) == 0) && !number(0, 2)) {
				long lRoom = RecGetObjRoom(pObj);

				/* broadcast to room */

				if(pObj->action_description) {
					MakeNoise(lRoom, pObj->action_description,
							  pObj->action_description);
				}
			}
		}
		else {
			if(pObj->item_number >= 0 && obj_index[ pObj->item_number ].func) {
				(*obj_index[ pObj->item_number].func)(NULL, localPulse, NULL, pObj,
													  EVENT_TICK);
			}
		}
	}
}

/*
**  Apply soundproof is for ch making noise
*/
int apply_soundproof(struct char_data* ch) {
	struct room_data* rp;

	if(IS_AFFECTED(ch, AFF_SILENCE)) {
		send_to_char("Sei silenziato! Non puoi emettere suoni!\n\r", ch);
		return(TRUE);
	}

	rp = real_roomp(ch->in_room);

	if(!rp) {
		return(FALSE);
	}

	if(IS_SET(rp->room_flags, SILENCE)) {
		send_to_char("Qui dentro non riesci a sentire nemmeno la tua voce!\n\r",
					 ch);
		return(TRUE);   /* for shouts, emotes, etc */
	}

	if(rp->sector_type == SECT_UNDERWATER) {
		send_to_char("Parlare sott'acqua ? Scherzi ?\n\r", ch);
		return(TRUE);
	}
	return(FALSE);

}

/*
**  check_soundproof is for others making noise
*/
int check_soundproof(struct char_data* ch) {
	struct room_data* rp;

	if(IS_AFFECTED(ch, AFF_SILENCE)) {
		return(TRUE);
	}

	rp = real_roomp(ch->in_room);

	if(!rp) {
		return(FALSE);
	}

	if(IS_SET(rp->room_flags, SILENCE)) {
		return(TRUE);   /* for shouts, emotes, etc */
	}
	if(rp->sector_type == SECT_UNDERWATER) {
		return(TRUE);
	}

	return(FALSE);
}

int MobCountInRoom(struct char_data* list) {
	int i;
	struct char_data* tmp;

	for(i=0, tmp = list; tmp; tmp = tmp->next_in_room, i++)
		;

	return(i);

}

void* Mymalloc(long size) {
	if(size < 1) {
		fprintf(stderr, "attempt to malloc negative memory - %ld\n", size);
		assert(0);
	}
	return(malloc(size));
}

void SpaceForSkills(struct char_data* ch) {

	/*
	 * create space for the skills for some mobile or character.
	 */


	ch->skills = (struct char_skill_data*)calloc(MAX_SKILLS,
				 sizeof(struct char_skill_data));

	assert(ch->skills != NULL);

}

int CountLims(struct obj_data* obj) {
	int total=0;

	if(!obj) {
		return(0);
	}

	if(obj->contains) {
		total += CountLims(obj->contains);
	}
	if(obj->next_content) {
		total += CountLims(obj->next_content);
	}
	/* rimettiamo il cost e togliamo il cost_per_day */
	if(obj->obj_flags.cost >= LIM_ITEM_COST_MIN) {
		total += 1;
	}
	return(total);
}

int LimObj(struct char_data* ch) {  // Gaia 2001
	int i,tot=0;

	if(!ch) {
		return(0);
	}

	tot += CountLims(ch->carrying) ;
	for(i=0; i < MAX_WEAR; i++) {
		tot += CountLims(ch->equipment[i]);
	}
	return(tot);
}


char* lower(const char* s) {
	static char c[1000];
	static char* p;
	int i=0;

	strncpy(c, s,999);
	c[999]='\0';

	while(c[i]) {
		if(c[i] < 'a' && c[i] >= 'A' && c[i] <= 'Z') {
			c[i] = (char)(int)c[i]+32;
		}
		i++;
	}
	p = c;
	return(p);
}
char* replace(char* s,char vecchio,char nuovo) {
	static char c[1000];
	static char* p;
	int i=0;

	strncpy(c, s,999);
	c[999]='\0';
	while(c[i]) {
		if(c[i] == vecchio) {
			c[i] = nuovo;
		}
		i++;
	}
	p = c;
	return(p);
}

int getFreeAffSlot(struct obj_data* obj) {
	int i;

	for(i=0; i < MAX_OBJ_AFFECT; i++)
		if(obj->affected[i].location == APPLY_NONE) {
			return(i);
		}

	assert(0);
	return -1;
}

void SetRacialStuff(struct char_data* mob) {

	switch(GET_RACE(mob)) {
	case RACE_BIRD:
		SET_BIT(mob->specials.affected_by, AFF_FLYING);
		break;
	case RACE_FISH:
		SET_BIT(mob->specials.affected_by, AFF_WATERBREATH);
		break;
	case RACE_SEA_ELF:
		/* e poi prosegue per le altre caratteristiche degli elfi */
		SET_BIT(mob->specials.affected_by, AFF_WATERBREATH);
		/* FALLTHRU */
	/* no break */
	case RACE_ELVEN:
	case RACE_DARK_ELF:
	case RACE_GOLD_ELF:
	case RACE_WILD_ELF:
		SET_BIT(mob->specials.affected_by,AFF_INFRAVISION);
		SET_BIT(mob->immune, IMM_CHARM);
		break;
	case RACE_DWARF:
	case RACE_DARK_DWARF:
	case RACE_DEEP_GNOME:
	case RACE_GNOME:
	case RACE_MFLAYER:
	case RACE_TROLL:
	case RACE_ORC:
	case RACE_GOBLIN:
	case RACE_HALFLING:
	case RACE_GNOLL:
		SET_BIT(mob->specials.affected_by, AFF_INFRAVISION);
		break;
	case RACE_INSECT:
	case RACE_ARACHNID:
		if(IS_PC(mob)) {
			GET_STR(mob) = 18;
			GET_ADD(mob) = 100;
		}
		break;
	case RACE_DEMON:
		SET_BIT(mob->M_immune, IMM_FIRE);
		SET_BIT(mob->M_immune, IMM_POISON);
		SET_BIT(mob->specials.affected_by, AFF_FLYING);
		SET_BIT(mob->specials.affected_by, AFF_INFRAVISION);
		break;
	case RACE_LYCANTH:
		SET_BIT(mob->M_immune, IMM_NONMAG);
		break;
	case RACE_PREDATOR:
		if(mob->skills) {
			mob->skills[SKILL_HUNT].learned = 100;
		}
		break;

	case RACE_GIANT_FROST  :
		SET_BIT(mob->M_immune, IMM_COLD);
		SET_BIT(mob->susc,IMM_FIRE);
		break;
	case RACE_GIANT_FIRE   :
		SET_BIT(mob->M_immune, IMM_FIRE);
		SET_BIT(mob->susc,IMM_COLD);
		break;
	case RACE_GIANT_CLOUD  :
		SET_BIT(mob->M_immune, IMM_SLEEP);  /* should be gas... but no IMM_GAS */
		SET_BIT(mob->susc,IMM_ACID);
		break;
	case RACE_GIANT_STORM  :
		SET_BIT(mob->M_immune, IMM_ELEC);
		break;

	case RACE_GIANT_STONE  :
		SET_BIT(mob->M_immune, IMM_PIERCE);
		break;
	case RACE_UNDEAD:
	case RACE_UNDEAD_VAMPIRE :
	case RACE_UNDEAD_LICH    :
	case RACE_UNDEAD_WIGHT   :
	case RACE_UNDEAD_GHAST   :
	case RACE_UNDEAD_GHOUL   :
	case RACE_UNDEAD_SPECTRE :
	case RACE_UNDEAD_ZOMBIE  :
	case RACE_UNDEAD_SKELETON :
		SET_BIT(mob->specials.affected_by, AFF_INFRAVISION);
		SET_BIT(mob->M_immune,IMM_POISON+IMM_DRAIN+IMM_SLEEP+IMM_HOLD+IMM_CHARM);
		break;

	case RACE_DRAGON_RED    :
		SET_BIT(mob->M_immune,IMM_FIRE);
		SET_BIT(mob->susc,IMM_COLD);
		break;
	case RACE_DRAGON_BLACK  :
		SET_BIT(mob->M_immune,IMM_ACID);
		break;

	case RACE_DRAGON_GREEN  :
		SET_BIT(mob->M_immune,IMM_SLEEP);
		break;

	case RACE_DRAGON_WHITE  :
		SET_BIT(mob->M_immune,IMM_COLD);
		SET_BIT(mob->susc,IMM_FIRE);
		break;
	case RACE_DRAGON_BLUE   :
		SET_BIT(mob->M_immune,IMM_ELEC);
		break;
	case RACE_DRAGON_SILVER :
		SET_BIT(mob->M_immune,IMM_ENERGY);
		break;
	case RACE_DRAGON_GOLD   :
		SET_BIT(mob->M_immune,IMM_SLEEP+IMM_ENERGY);
		break;
	case RACE_DRAGON_BRONZE :
		SET_BIT(mob->M_immune,IMM_COLD+IMM_ACID);
		break;
	case RACE_DRAGON_COPPER :
		SET_BIT(mob->M_immune,IMM_FIRE);
		break;
	case RACE_DRAGON_BRASS  :
		SET_BIT(mob->M_immune,IMM_ELEC);
		break;

	case RACE_HALF_ELVEN:
	case RACE_HALF_OGRE:
	case RACE_HALF_ORC:
	case RACE_HALF_GIANT:
		SET_BIT(mob->specials.affected_by, AFF_INFRAVISION);
		break;

	default:
		break;
	}

	/* height and weight      / Hatred Foes! / */
	if(IS_NPC(mob)) {
		switch(GET_RACE(mob)) {
		case RACE_HUMAN:
			break;

		case RACE_ELVEN:
		case RACE_GOLD_ELF:
		case RACE_WILD_ELF:
		case RACE_SEA_ELF:
			AddHatred(mob,OP_RACE,RACE_ORC);
			break;
		case RACE_GNOME:
			break;

		case RACE_DWARF:
			AddHatred(mob,OP_RACE,RACE_GOBLIN);
			AddHatred(mob,OP_RACE,RACE_ORC);
			break;
		case RACE_HALFLING:
			break;

		case RACE_LYCANTH:
			AddHatred(mob,OP_RACE,RACE_HUMAN);
			break;

		case RACE_UNDEAD:
			break;
		case RACE_DARK_DWARF:                  /* these guys hate good people */
		case RACE_UNDEAD_VAMPIRE :
		case RACE_UNDEAD_LICH    :
		case RACE_UNDEAD_WIGHT   :
		case RACE_UNDEAD_GHAST   :
		case RACE_UNDEAD_GHOUL   :
		case RACE_UNDEAD_SPECTRE :
			AddHatred(mob,OP_GOOD,1000);
			break;
		case RACE_UNDEAD_ZOMBIE  :
			break;
		case RACE_UNDEAD_SKELETON :
			break;
		case RACE_VEGMAN:
			break;
		case RACE_MFLAYER:
			break;
		case RACE_DARK_ELF:
			AddHatred(mob,OP_RACE,RACE_ELVEN);
			break;
		case RACE_SKEXIE:
			break;
		case RACE_TROGMAN:
			break;
		case RACE_LIZARDMAN:
			break;
		case RACE_SARTAN:
			break;
		case RACE_PATRYN:
			break;
		case RACE_DRAAGDIM:
			break;
		case RACE_ASTRAL:
			break;

		case RACE_HORSE:
			mob->player.weight = 400;
			mob->player.height = 175;
			break;

		case RACE_ORC:
			AddHatred(mob,OP_GOOD,1000);
			AddHatred(mob,OP_RACE,RACE_ELVEN);
			mob->player.weight = 150;
			mob->player.height = 140;
			break;

		case RACE_SMURF:
			mob->player.weight = 5;
			mob->player.height = 10;
			break;

		case RACE_GOBLIN:
		case RACE_GNOLL:
			AddHatred(mob,OP_GOOD,1000);
			AddHatred(mob,OP_RACE,RACE_DWARF);
			break;

		case RACE_ENFAN:
			mob->player.weight = 120;
			mob->player.height = 100;
			break;

		case RACE_LABRAT:
			break;
		case RACE_INSECT:
			break;
		case RACE_ARACHNID:
			break;
		case RACE_REPTILE:
			break;
		case RACE_DINOSAUR:
			break;
		case RACE_FISH:
			break;
		case RACE_PREDATOR:
			break;
		case RACE_SNAKE:
			break;
		case RACE_HERBIV:
			break;
		case RACE_VEGGIE:
			break;
		case RACE_ELEMENT:
			break;
		case RACE_PRIMATE:
			break;

		case RACE_GOLEM:
			mob->player.weight = 10+GetMaxLevel(mob)*GetMaxLevel(mob)*2;
			mob->player.height = 20+MIN(mob->player.weight,600);
			break;

		case RACE_DRAGON:
		case RACE_DRAGON_RED    :
		case RACE_DRAGON_BLACK  :
		case RACE_DRAGON_GREEN  :
		case RACE_DRAGON_WHITE  :
		case RACE_DRAGON_BLUE   :
		case RACE_DRAGON_SILVER :
		case RACE_DRAGON_GOLD   :
		case RACE_DRAGON_BRONZE :
		case RACE_DRAGON_COPPER :
		case RACE_DRAGON_BRASS  :
			mob->player.weight = MAX(60, GetMaxLevel(mob)*GetMaxLevel(mob)*2);
			mob->player.height = 100+ MIN(mob->player.weight, 500);
			break;

		case RACE_BIRD:
		case RACE_PARASITE:
		case RACE_SLIME:
			mob->player.weight = GetMaxLevel(mob)*(GetMaxLevel(mob)/5);
			mob->player.height = 10*GetMaxLevel(mob);
			break;


		case RACE_GHOST:
			mob->player.weight = GetMaxLevel(mob)*(GetMaxLevel(mob)/5);
			mob->player.height = 10*GetMaxLevel(mob);
			break;
		case RACE_TROLL:
			AddHatred(mob,OP_GOOD,1000);
			mob->player.height = 200+GetMaxLevel(mob)*15;
			mob->player.weight = (int)(mob->player.height * 1.5);
			break;

		case RACE_GIANT:
		case RACE_GIANT_HILL   :
		case RACE_GIANT_FROST  :
		case RACE_GIANT_FIRE   :
		case RACE_GIANT_CLOUD  :
		case RACE_GIANT_STORM  :
			mob->player.height = 200+GetMaxLevel(mob)*15;
			mob->player.weight = (int)(mob->player.height * 1.5);
			AddHatred(mob,OP_RACE,RACE_DWARF);
			break;

		case RACE_DEVIL:
		case RACE_DEMON:
			AddHatred(mob,OP_GOOD,1000);
			mob->player.height = 200+GetMaxLevel(mob)*15;
			mob->player.weight = (int)(mob->player.height * 1.5);
			break;


		case RACE_PLANAR:
			mob->player.height = 200+GetMaxLevel(mob)*15;
			mob->player.weight = (int)(mob->player.height * 1.5);
			break;

		case RACE_GOD:
			break;
		case RACE_TREE:
			break;
		case RACE_TYTAN:
			mob->player.weight = MAX(500, GetMaxLevel(mob)*GetMaxLevel(mob)*10);
			mob->player.height = GetMaxLevel(mob)/2*100;
			break;

		case RACE_HALF_ELVEN:
		case RACE_HALF_OGRE:
		case RACE_HALF_ORC:
		case RACE_HALF_GIANT:
			break;
		} /* end switch */
	}
	else { /* !IS_NPC( mob ) */
		switch(GET_RACE(mob)) {
		case RACE_ORC:
		case RACE_GOBLIN:
		case RACE_TROLL:
		case RACE_DARK_DWARF:
		case RACE_DEEP_GNOME:
		case RACE_DEMON:
			GET_ALIGNMENT(mob) = -1000;
			break;
		default:
			break;
		}
	}
}

int check_nomagic(struct char_data* ch, const char* msg_ch, const char* msg_rm) {
	struct room_data* rp;

	rp = real_roomp(ch->in_room);
	if(rp && rp->room_flags&NO_MAGIC) {
		if(msg_ch) {
			send_to_char(msg_ch, ch);
		}
		if(msg_rm) {
			act(msg_rm, TRUE, ch, 0, 0, TO_ROOM);
		}
		return 1;
	}
	return 0;
}

int check_nomind(struct char_data* ch, const char* msg_ch, const char* msg_rm) {
	struct room_data* rp;

	rp = real_roomp(ch->in_room);
	if(rp && rp->room_flags&NO_MIND) {
		if(msg_ch) {
			send_to_char(msg_ch, ch);
		}
		if(msg_rm) {
			act(msg_rm, TRUE, ch, 0, 0, TO_ROOM);
		}
		return 1;
	}
	return 0;
}

int NumCharmedFollowersInRoom(struct char_data* ch) {
	struct char_data* t;
	long count = 0;
	struct room_data* rp;

	rp = real_roomp(ch->in_room);
	if(rp) {
		count=0;
		for(t = rp->people; t; t= t->next_in_room) {
			if(IS_AFFECTED(t, AFF_CHARM) && (t->master == ch)) {
				count++;
			}
		}
		return(count);
	}
	else {
		return(0);
	}

	return(0);
}


struct char_data* FindMobDiffZoneSameRace(struct char_data* ch) {
	int num;
	struct char_data* t;
	struct room_data* rp1,*rp2;

	num = number(1,100);

	for(t=character_list; t; t=t->next, num--) {
		if(GET_RACE(t) == GET_RACE(ch) && IS_NPC(t) && !IS_PC(t) && num==0) {
			rp1 = real_roomp(ch->in_room);
			rp2 = real_roomp(t->in_room);
			if(rp1->zone != rp2->zone) {
				return(t);
			}
		}
	}
	return(0);
}

int NoSummon(struct char_data* ch) {
	struct room_data* rp;

	rp = real_roomp(ch->in_room);
	if(!rp) {
		return(TRUE);
	}

	if(IS_SET(rp->room_flags, NO_SUM)&& !IS_DIO_MINORE(ch)) {
		send_to_char("Cryptic powers block your summons.\n\r", ch);
		return(TRUE);
	}

	if(IS_SET(rp->room_flags, TUNNEL)) {
		send_to_char("Strange forces collide in your brain,\n\r", ch);
		send_to_char("Laws of nature twist, and dissipate before\n\r", ch);
		send_to_char("your eyes, strange ideas wrestle with green furry\n\r", ch);
		send_to_char("things, which are crawling up your super-ego...\n\r", ch);
		send_to_char("  You lose a sanity point.\n\r\n\r", ch);
		send_to_char("  OOPS!  Sorry, wronge Genre.  :-) \n\r", ch);
		return(TRUE);
	}

	return(FALSE);
}

int GetNewRace(struct char_file_u* s) {
	int ok, newrace, i;
	ok = FALSE ;
	while(1) {
		newrace = number(1, MAX_RACE);
		switch(newrace) {
		/* we allow these races to be used in reincarnations */
		case RACE_HUMAN:
		case RACE_ELVEN:
		case RACE_DWARF:
		case RACE_HALFLING:
		case RACE_GNOME:
		case RACE_ORC:
		case RACE_DARK_ELF:
		case RACE_MFLAYER:
		case RACE_DARK_DWARF:
		case RACE_DEEP_GNOME:
		case RACE_GNOLL:
		case RACE_GOLD_ELF:
		case RACE_WILD_ELF:
		case RACE_SEA_ELF:
		case RACE_LIZARDMAN:
		case RACE_HALF_ELVEN:
		case RACE_HALF_OGRE:
		case RACE_HALF_ORC:
		case RACE_HALF_GIANT:
		case RACE_GIANT_HILL:
		case RACE_GIANT_FROST:
		case RACE_GIANT_FIRE:
		case RACE_GIANT_CLOUD:
		case RACE_GIANT_STORM:
		case RACE_ROO:
		case RACE_PRIMATE:
		case RACE_GOBLIN:
		case RACE_TROLL:
		case RACE_DEMON:
			ok = TRUE;
			break;
		/* not a valid race, try again */
		default:
			ok = FALSE;
			break;
		}

		if(ok) {
			for(i = 0; i < MAX_CLASS; i++) {
				if(RacialMax[ newrace ][ i ] < (s->level[ i ] - 10) && s->level[i] > 0) {
					ok = FALSE;
					break;
				}
			}
		}
		if(ok) {
			/* Se la classe e' okay, provvedo a riportare gli xp al max razziale della
			 * classe messa peggio
			 * */
			// mudlog( LOG_CHECK, "XP totali: %d ", s->points.exp ) ;
			int xplimit;
			int curlevel;
			for(i = 0; i < MAX_CLASS; i++) {
				if(s->level[i] > 0) {
					curlevel=RacialMax[newrace][i] ;
					if(curlevel > 1)
//  mudlog( LOG_CHECK, "Livello attuale: %d ", curlevel ) ;
					{
						xplimit=titles[i][curlevel].exp;
//  mudlog( LOG_CHECK, "Limite Xp per questa classe: %d ", xplimit ) ;
						s->points.exp=MIN(s->points.exp,(xplimit+(s->points.exp/2)));
//  mudlog( LOG_CHECK, "Xp Modificati in: %d ", s->points.exp ) ;
					}
				}
			}
			break ;
		}
	} /* while */
	return(newrace);
}

int GetApprox(int num, int perc) {
	/* perc = 0 - 100 */
	int adj, r;
	float fnum, fadj;

	adj = 100 - perc;
	if(adj < 0) {
		adj = 0;
	}
	adj *=2;  /* percentage of play (+- x%) */

	r = number(1,adj);

	perc += r;

	fnum = (float)num;
	fadj = (float)perc*2;
	fnum *= (float)(fadj/(200.0));

	num = (int)fnum;

	return(num);
}

int MountEgoCheck(struct char_data* ch, struct char_data* horse) {
	int ride_ego, drag_ego, align, check;

	if(IsDragon(horse)) {
		if(ch->skills) {
			drag_ego = GetMaxLevel(horse)*2;
			if(IS_SET(horse->specials.act, ACT_AGGRESSIVE) ||
					IS_SET(horse->specials.act, ACT_META_AGG)) {
				drag_ego += GetMaxLevel(horse);
			}
			ride_ego = ch->skills[SKILL_RIDE].learned/10 + GetMaxLevel(ch)/2;
			if(IS_AFFECTED(ch, AFF_DRAGON_RIDE)) {
				ride_ego += ((GET_INT(ch) + GET_WIS(ch))/2);
			}
			align = GET_ALIGNMENT(ch) - GET_ALIGNMENT(horse);
			if(align < 0) {
				align = -align;
			}
			align/=100;
			align -= 5;
			drag_ego += align;
			if(GET_HIT(horse) > 0) {
				drag_ego -= GET_MAX_HIT(horse)/GET_HIT(horse);
			}
			else {
				drag_ego = 0;
			}
			if(GET_HIT(ch) > 0) {
				ride_ego -= GET_MAX_HIT(ch)/GET_HIT(ch);
			}
			else {
				ride_ego = 0;
			}

			check = drag_ego+number(1,10)-(ride_ego+number(1,10));
			return(check);

		}
		else {
			return(-GetMaxLevel(horse));
		}
	}
	else {
		if(!ch->skills) {
			return(-GetMaxLevel(horse));
		}

		drag_ego = GetMaxLevel(horse);

		if(drag_ego > 15) {
			drag_ego *= 2;
		}

		ride_ego = ch->skills[SKILL_RIDE].learned/10 + GetMaxLevel(ch);

		if(IS_AFFECTED(ch, AFF_DRAGON_RIDE)) {
			ride_ego += (GET_INT(ch) + GET_WIS(ch));
		}
		check = drag_ego+number(1,5)-(ride_ego+number(1,10));
		return(check);
	}
}

int RideCheck(struct char_data* ch, int mod) {
	if(ch->skills) {
		if(!IS_AFFECTED(ch, AFF_DRAGON_RIDE)) {
			if(number(1,70) > MIN(100, ch->skills[SKILL_RIDE].learned) + mod) {
				if(number(1, 91 - mod) >
						MIN(100, ch->skills[SKILL_RIDE].learned) / 2) {
					if(ch->skills[SKILL_RIDE].learned < 90) {
						send_to_char("You learn from your mistake\n\r", ch);
						ch->skills[SKILL_RIDE].learned += 2;
					}
				}
				return(FALSE);
			}
			return(TRUE);
		}
		else {
			if(number(1, 70) > (MIN(100, ch->skills[SKILL_RIDE].learned) +
								GET_LEVEL(ch, BestMagicClass(ch)) + mod)) {
				return(FALSE);
			}
		}
	}
	else {
		return(FALSE);
	}
	return TRUE;
}

void FallOffMount(struct char_data* ch, struct char_data* h) {
	act("$n loses control and falls off of $N", FALSE, ch, 0, h, TO_NOTVICT);
	act("$n loses control and falls off of you", FALSE, ch, 0, h, TO_VICT);
	act("You lose control and fall off of $N", FALSE, ch, 0, h, TO_CHAR);

}

#define ITEM_CLASS ( ITEM_ANTI_CLERIC | ITEM_ANTI_MAGE |                      ITEM_ANTI_THIEF | ITEM_ANTI_FIGHTER |                      ITEM_ANTI_BARBARIAN | ITEM_ANTI_RANGER |                      ITEM_ANTI_PALADIN | ITEM_ANTI_PSI |                      ITEM_ANTI_MONK | ITEM_ANTI_DRUID )

int EqWBits(struct char_data* ch, const unsigned int bits) {
	int i;

	for(i = 0; i < MAX_WEAR; i++) {
		if(ch->equipment[ i ]) {
			if((IS_SET(ch->equipment[ i ]->obj_flags.extra_flags,
					   ITEM_ONLY_CLASS) &&
					IS_SET(ch->equipment[ i ]->obj_flags.extra_flags ^ ITEM_CLASS,
						   bits)) ||
					(!IS_SET(ch->equipment[ i ]->obj_flags.extra_flags,
							 ITEM_ONLY_CLASS) &&
					 IS_SET(ch->equipment[ i ]->obj_flags.extra_flags, bits)))

			{
				return TRUE;
			}
		}
	}
	return FALSE;
}

int InvWBits(struct char_data* ch, int bits) {
	struct obj_data* o;

	for(o = ch->carrying; o; o = o->next_content) {
		if((IS_SET(o->obj_flags.extra_flags, ITEM_ONLY_CLASS) &&
				IS_SET(o->obj_flags.extra_flags ^ ITEM_CLASS, bits)) ||
				(!IS_SET(o->obj_flags.extra_flags, ITEM_ONLY_CLASS) &&
				 IS_SET(o->obj_flags.extra_flags, bits))) {
			return TRUE;
		}
	}
	return FALSE;
}

int HasWBits(struct char_data* ch, int bits) {
	if(EqWBits(ch, bits)) {
		return(TRUE);
	}
	if(InvWBits(ch, bits)) {
		return(TRUE);
	}
	return(FALSE);
}

void LearnFromMistake(struct char_data* ch, int sknum, int silent, int max) {
	if(!ch->skills) {
		return;
	}

	if(!IS_SET(ch->skills[ sknum ].flags, SKILL_KNOWN)) {
		return;
	}

	if(IS_IMMORTAL(ch)) {
		max = 100;
	}
	else if(HowManyClasses(ch) >= 3) {
		max = MIN(max, 81);
	}
	else if(HowManyClasses(ch) == 2) {
		max = MIN(max, 86);
	}
	else {
		max = MIN(max, 95);
	}

	if(ch->skills[ sknum ].learned < max && ch->skills[ sknum ].learned > 0) {
		if(number(1, 101) > ch->skills[ sknum ].learned / 2) {
			if(!silent) {
				send_to_char("Impari dai tuoi errori.\n\r", ch);
			}
			ch->skills[ sknum ].learned += 1;
			if(ch->skills[ sknum ].learned >= max)
				if(!silent) {
					send_to_char("Hai imparato tutto su questa abilita'!\n\r", ch);
				}
		}
	}
}

/* if (!IsOnPmp(roomnumber)) then they are on another plane! */
int IsOnPmp(int room_nr) {
	if(real_roomp(room_nr)) {
		if(!IS_SET(zone_table[real_roomp(room_nr)->zone].reset_mode, ZONE_ASTRAL)) {
			return(TRUE);
		}
		return(FALSE);
	}
	else {
		return(FALSE);
	}

}


int GetSumRaceMaxLevInRoom(struct char_data* ch) {
	struct room_data* rp;
	struct char_data* i;
	int sum=0;

	rp = real_roomp(ch->in_room);

	if(!rp) {
		return(0);
	}

	for(i = rp->people; i; i=i->next_in_room) {
		if(GET_RACE(i) == GET_RACE(ch)) {
			sum += GetMaxLevel(i);
		}
	}
	return(sum);
}

int too_many_followers(struct char_data* ch) {
	struct follow_type* k;
	int max_followers,actual_fol;

	max_followers = (int) chr_apply[(int)GET_CHR(ch) ].num_fol;

	for(k=ch->followers,actual_fol=0; k; k=k->next)
		if(IS_AFFECTED(k->follower, AFF_CHARM)) {
			actual_fol++;
		}

	if(actual_fol < max_followers) {
		return FALSE;
	}
	else {
		return TRUE;
	}
}

int follow_time(struct char_data* ch) {
	int fol_time=0;
	fol_time= (int)(24*GET_CHR(ch)/11);
	return fol_time;
}

int ItemAlignClash(struct char_data* ch, struct obj_data* obj) {
	if((IS_OBJ_STAT(obj, ITEM_ANTI_EVIL) && IS_EVIL(ch)) ||
			(IS_OBJ_STAT(obj, ITEM_ANTI_GOOD) && IS_GOOD(ch)) ||
			(IS_OBJ_STAT(obj, ITEM_ANTI_NEUTRAL) && IS_NEUTRAL(ch))) {
		return(TRUE);
	}
	return(FALSE);
}

int ItemEgoClash(struct char_data* ch, struct obj_data* obj, int bon) {

#if EGO
	int obj_ego, p_ego, tmp;

	obj_ego = obj->obj_flags.cost;

	if(obj->obj_flags.type_flag == ITEM_KEY ||
			obj->obj_flags.type_flag == ITEM_SCROLL ||
			obj->obj_flags.type_flag == ITEM_POTION ||
			obj->obj_flags.type_flag == ITEM_TREASURE ||
			obj->obj_flags.type_flag == ITEM_MONEY) {
		return 0;
	}

	if(obj_ego >= MIN_COST_ITEM_EGO || obj_ego < 0) {

		if(obj_ego < 0) {
			obj_ego = 50000;
		}

		obj_ego /= 500;

		/*  alignment stuff */

		if(IS_OBJ_STAT(obj, ITEM_ANTI_GOOD) ||
				IS_OBJ_STAT(obj, ITEM_ANTI_EVIL)) {
			if(IS_NEUTRAL(ch)) {
				obj_ego += obj_ego / 4;
			}
		}

		if(IS_PC(ch)) {
			p_ego = GetMaxLevel(ch) + HowManyClasses(ch);

			if(p_ego > 40) {
				p_ego = 588 + (p_ego - 40) * 2;
			}
			else if(p_ego > 30) {
				p_ego *= p_ego - 29;
			}
			else if(p_ego > 20) {
				p_ego += p_ego - 20;
			}

		}
		else {
			p_ego = 10000;
		}

		tmp = (p_ego * ((GET_INT(ch) + GET_WIS(ch)) / 2 - 12)) / 10;
		tmp = MAX(0, tmp);

		if(p_ego > 20 && tmp > 0) {
			tmp *= GET_HIT(ch);
			tmp /= GET_MAX_HIT(ch);
		}


		return(p_ego + tmp + bon - obj_ego - number(0, 2));
	}

	return(1);
#else
	return(0);

#endif
}

void IncrementZoneNr(int nr) {
	struct char_data* c;

	if(nr > top_of_zone_table) {
		return;
	}

	if(nr >= 0) {
		for(c = character_list; c; c=c->next) {
			if(c->specials.zone >= nr) {
				c->specials.zone++;
			}
		}
	}
	else {
		for(c = character_list; c; c=c->next) {
			if(c->specials.zone >= nr) {
				c->specials.zone--;
			}
		}
	}
}

int IsDarkOutside(struct room_data* rp) {

	if(gLightLevel >= 4) {
		return FALSE;
	}

	if(IS_SET(rp->room_flags, INDOORS) || IS_SET(rp->room_flags, DEATH)) {
		return FALSE;
	}

	if(rp->sector_type == SECT_FOREST || rp->sector_type == SECT_DARKCITY) {
		if(gLightLevel > 1) {
			return FALSE;
		}
	}
	else {
		if(gLightLevel > 0) {
			return FALSE;
		}
	}
	return TRUE;
}

int anti_barbarian_stuff(struct obj_data* obj_object) {
	if(IS_OBJ_STAT(obj_object, ITEM_GLOW)  ||
			IS_OBJ_STAT(obj_object, ITEM_HUM) ||
			IS_OBJ_STAT(obj_object, ITEM_MAGIC) ||
			IS_OBJ_STAT(obj_object, ITEM_BLESS) ||
			IS_OBJ_STAT(obj_object, ITEM_NODROP)) {
		return TRUE;
	}
	else {
		return FALSE;
	}
}

int CheckGetBarbarianOK(struct char_data* ch, struct obj_data* obj_object) {
	if(GET_LEVEL(ch,BARBARIAN_LEVEL_IND) != 0 &&
			anti_barbarian_stuff(obj_object) && !IS_IMMORTAL(ch)) {
		act("Percepisci la magia su $p e rabbrividisci disgustat$b.", FALSE, ch,
			obj_object, 0, TO_CHAR);
		act("$n scuote la testa e si rifiuta di prendere $p.", TRUE, ch,
			obj_object, 0, TO_ROOM);
		return FALSE;
	}

	return TRUE;
}

int CheckGiveBarbarianOK(struct char_data* ch,struct char_data* vict,
						 struct obj_data* obj) {
	if(GET_LEVEL(vict,BARBARIAN_LEVEL_IND) != 0 &&
			anti_barbarian_stuff(obj) && GetMaxLevel(vict)<IMMORTALE) {
		if(AWAKE(vict)) {

			act("Percepisci la magia su $p e rifiuti l'offerta di $N.", FALSE,
				vict, obj, ch, TO_CHAR);
			act("$n scuote la testa e si rifiuta di prendere $p da $N.", TRUE,
				vict, obj, ch, TO_NOTVICT);
			act("$n scuote la testa e si rifiuta di prendere $p.", TRUE,
				vict, obj, ch, TO_VICT);
		}
		else {
			act("Forse e' meglio non dare $p a $N neanche se dorme.", FALSE, ch,
				obj, vict, TO_CHAR);
		}

		return FALSE;
	}

	return(TRUE);
}

int CheckEgoEquip(struct char_data* ch, struct obj_data* obj) {
	int j=0;

	if(!obj || !ch) {
		mudlog(LOG_SYSERR, "!obj || !ch in CheckEgoEquip, utility.c");
		return(FALSE);
	}

	/*
	 * if the item is limited, check its ego.
	 * use some funky function to determine if pc's ego is higher than objs'
	 * ego.. if it is, proceed.. otherwise, deny.
	 */
	j = ItemEgoClash(ch, obj, 0);
	if(j < 0 && GetMaxLevel(ch) < MAESTRO_DEI_CREATORI) {
		act("Non capisci come usare $p.", 0, ch, obj, 0, TO_CHAR);

		if(j < -10) {
			if(obj->in_obj == NULL) {
				act("$p ti cade in terra.", 0, ch, obj, 0, TO_CHAR);
				act("$p cade dalle mani di $n.", 0, ch, obj, 0, TO_ROOM);
				if(ch->in_room != NOWHERE) {
					obj_to_room(obj, ch->in_room);
					do_save(ch, "", 0);
				}
				else {
					mudlog(LOG_SYSERR,
						   "Ch->in_room = NOWHERE on anti-ego item! (equip_char)");
				}
			}
		}
		else {
			obj_to_char(obj, ch);
		}

		return(FALSE);
	}
	else {
		return(TRUE);
	}
}

int CheckEgoGet(struct char_data* ch, struct obj_data* obj) {
	int j=0;
	/*
	 * if the item is limited, check its ego.
	 * use some funky function to determine if pc's ego is higher than objs'
	 * ego.. if it is, proceed.. otherwise, deny.
	 */
	j = ItemEgoClash(ch, obj, 0);
	if(j < -10 && GetMaxLevel(ch) < MAESTRO_DEI_CREATORI) {
		act("Non capisci neanche come prendere un oggetto potente come $p.",
			FALSE, ch, obj, 0, TO_CHAR);
		return FALSE;
	}

	return(TRUE);
}

int CheckEgoGive(struct char_data* ch,struct char_data* vict,
				 struct obj_data* obj) {
	int j=0;
	/*
	 * if the item is limited, check its ego.
	 * use some funky function to determine if pc's ego is higher than objs'
	 * ego.. if it is, proceed.. otherwise, deny.
	 */
	j = ItemEgoClash(vict, obj, 0);
	if(j < -10 && GetMaxLevel(vict) < MAESTRO_DEI_CREATORI) {
		if(AWAKE(vict)) {
			act("Hai quasi paura a prendere un oggetto potente come $p.",0,
				vict, obj, 0, TO_CHAR);
			act("$n rabbrividisce e rifiuta $p.", 0, vict, obj, ch, TO_VICT);
		}
		else {
			act("Non riesci a dare $p a $n.", 0, vict, obj, ch, TO_VICT);
		}

		act("$N non riesce a dare $p a $n.",0,vict, obj, ch, TO_NOTVICT);
		return(FALSE);
	}
	else {
		return(TRUE);
	}

}





int IsSpecialized(int sk_num) {
	return(IS_SET(sk_num,SKILL_SPECIALIZED));
}

/* this persons max specialized skills */
int HowManySpecials(struct char_data* ch) {
	int i,ii=0;

	for(i=0; i<MAX_SPL_LIST; i++)
		if(IsSpecialized(ch->skills[i].special)) {
			ii++;
		}
	return(ii);
}

int MAX_SPECIALS(struct char_data* ch) {
	return(GET_INT(ch));
}

int CanSeeTrap(int num,struct char_data* ch) {
	if(HasClass(ch,CLASS_THIEF)) {
		return(affected_by_spell(ch,SPELL_FIND_TRAPS) ||
			   (ch->skills &&
				num < MIN(100, ch->skills[SKILL_FIND_TRAP].learned) &&
				!MOUNTED(ch)));
	}
	if(HasClass(ch,CLASS_RANGER) && OUTSIDE(ch)) {
		return(affected_by_spell(ch,SPELL_FIND_TRAPS) ||
			   (ch->skills &&
				num < MIN(100, ch->skills[SKILL_FIND_TRAP].learned) &&
				!MOUNTED(ch)));
	}

	if(affected_by_spell(ch,SPELL_FIND_TRAPS) && !MOUNTED(ch)) {
		return(TRUE);
	}

	return(FALSE);
}

/* this is where we figure the max limited items the char may rent with
* for his/her current level, pc's wanted it this way. MAX_LIM_ITEMS is in
* structs.h .
* Ho diminuito i limit ancora un po'
* */

int MaxLimited(int lev) {
	if(lev <= 10) {
		return(2);
	}
	else if(lev <= 15) {
		return(4);
	}
	else if(lev <= 20) {
		return(6);
	}
	else if(lev <= 25) {
		return(8);
	}
	else if(lev <= 30) {
		return(10);
	}
	else if(lev <= 35) {
		return(12);
	}
	else if(lev <= 40) {
		return(14);
	}
	else if(lev <= 45) {
		return(16);
	}
	else {
		return(MAX_LIM_ITEMS);
	}
}

int MaxDexForRace(struct char_data* ch) {
	if(GetMaxLevel(ch) >= MAESTRO_DEL_CREATO) {
		return(25);
	}
	switch(GET_RACE(ch)) {
	case RACE_ELVEN:
	case RACE_WILD_ELF:
	case RACE_GOLD_ELF:
	case RACE_HALFLING:
	case RACE_DEMON:
		return(19);
		break;
	case RACE_DARK_ELF:
	case RACE_GOD:
		return(20);
		break;
	case RACE_DWARF:
	case RACE_HALF_OGRE:
		return(17);
		break;
	case RACE_HALF_GIANT:
		return(16);
		break;
	default:
		return(18);
		break;
	}/* end switch */
}

int MaxIntForRace(struct char_data* ch) {
	if(GetMaxLevel(ch) >= MAESTRO_DEL_CREATO) {
		return(25);
	}
	switch(GET_RACE(ch)) {
	case RACE_GOLD_ELF:
	case RACE_DEMON:
	case RACE_GOD:
	case RACE_GNOME :
		return(19);
		break;
	case RACE_HALF_GIANT:
	case RACE_HALF_OGRE:
		return(17);
		break;

	default:
		return(18);
		break;
	}/* end switch */
}

int MaxWisForRace(struct char_data* ch) {
	if(GetMaxLevel(ch) >= MAESTRO_DEL_CREATO) {
		return(25);
	}
	switch(GET_RACE(ch)) {
	case RACE_GOLD_ELF:
	case RACE_WILD_ELF:
	case RACE_GNOME:
	case RACE_DEMON:
		return(17);
		break;
	case RACE_HALF_GIANT:
		return(17);
		break;
	default:
		return(18);
		break;
	}/* end switch */
}

int MaxConForRace(struct char_data* ch) {
	if(GetMaxLevel(ch) >= MAESTRO_DEL_CREATO) {
		return(25);
	}
	switch(GET_RACE(ch)) {
	case RACE_HALF_ORC:
	case RACE_DWARF:
	case RACE_GOD:
	case RACE_HALF_OGRE:
		return(19);
		break;
	case RACE_ELVEN:
	case RACE_GOLD_ELF:
	case RACE_SEA_ELF:
	case RACE_DARK_ELF:
		return(17);
		break;
	default:
		return(18);
		break;
	}/* end switch */
}

int MaxChrForRace(struct char_data* ch) {
	if(GetMaxLevel(ch) >= MAESTRO_DEL_CREATO) {
		return(25);
	}
	switch(GET_RACE(ch)) {
	case RACE_HALF_ORC:
	case RACE_ORC:
	case RACE_DARK_ELF:
	case RACE_DWARF:
	case RACE_DARK_DWARF:
		return(17);
		break;
	case RACE_DEMON:
		return(15);
		break;
	default:
		return(18);
		break;
	}/* end switch */
}

int MaxStrForRace(struct char_data* ch) {

	if(GetMaxLevel(ch) >= MAESTRO_DEL_CREATO) {
		return(25);
	}

	switch(GET_RACE(ch)) {
	case RACE_TROLL:
	case RACE_GOD:
	case RACE_HALF_GIANT:
		return(19);
		break;

	case RACE_HALFLING:
	case RACE_DEMON:
	case RACE_GOBLIN:
		return(17);
		break;
	default:
		return(18);
		break;
	}/* end switch */
}


int IS_MURDER(struct char_data* ch) {
	if(IS_PC(ch) && IS_SET(ch->player.user_flags,MURDER_1) && !IS_IMMORTAL(ch)) {
		mudlog(LOG_PLAYERS, "%s has the MURDER set.",GET_NAME(ch));
		return (TRUE);
	}
	return(FALSE);
}

int IS_STEALER(struct char_data* ch) {
	if(IS_PC(ch) && IS_SET(ch->player.user_flags,STOLE_1) && !IS_IMMORTAL(ch)) {
		mudlog(LOG_PLAYERS, "%s has STOLE set.",GET_NAME(ch));
		return(TRUE);
	}
	return(FALSE);
}

int MEMORIZED(struct char_data* ch, int spl) {
	if(ch->skills[ spl ].nummem > 0) {
		return TRUE;
	}
	else if(ch->skills[ spl ].nummem < 0) {
		ch->skills[ spl ].nummem = 0;
	}
	return FALSE;
}

void FORGET(struct char_data* ch, int spl) {
	if(ch->skills[ spl ].nummem) {
		ch->skills[ spl ].nummem -= 1;
	}
}

/* return the amount max a person can memorize a single spell */
int MaxCanMemorize(struct char_data* ch, int spell) {
	int BONUS;  /* use this later to figure item bonuses or something */

	if(OnlyClass(ch, CLASS_SORCERER)) {
		BONUS = 2;
	}
	else {
		BONUS = 0;    /* multies get less... */
	}

	if(GET_INT(ch) > 17) {
		BONUS += (GET_INT(ch) - 17);    /* +1 spell per intel over 17 */
	}

	if(GET_LEVEL(ch, SORCERER_LEVEL_IND) < 4) {
		return(3 + BONUS);
	}
	else if(GET_LEVEL(ch, SORCERER_LEVEL_IND) < 11) {
		return(4 + BONUS);
	}
	else if(GET_LEVEL(ch, SORCERER_LEVEL_IND) < 16) {
		return(5 + BONUS);
	}
	else if(GET_LEVEL(ch, SORCERER_LEVEL_IND) < 21) {
		return(6 + BONUS);
	}
	else if(GET_LEVEL(ch, SORCERER_LEVEL_IND) < 26) {
		return(7 + BONUS);
	}
	else if(GET_LEVEL(ch, SORCERER_LEVEL_IND) < 31) {
		return(8 + BONUS);
	}
	else if(GET_LEVEL(ch, SORCERER_LEVEL_IND) < 36) {
		return(9 + BONUS);
	}
	else if(GET_LEVEL(ch, SORCERER_LEVEL_IND) < 41) {
		return(10 + BONUS);
	}
	else if(GET_LEVEL(ch, SORCERER_LEVEL_IND) < 46) {
		return(11 + BONUS);
	}
	else if(GET_LEVEL(ch, SORCERER_LEVEL_IND) < 52) {
		return(12 + BONUS);
	}
	else { /* should never get here, cept for immos */
		return((int)(GetMaxLevel(ch) / 3));
	}
}




int IS_LINKDEAD(struct char_data* ch) {
	if(IS_PC(ch) && !ch->desc) {
		return(TRUE);
	}

	if(IS_SET(ch->specials.act, ACT_POLYSELF) && !ch->desc) {
		return(TRUE);
	}

	return(FALSE);
}

int IS_UNDERGROUND(struct char_data* ch) {
	struct room_data* rp;

	if((rp = real_roomp(ch->in_room))!=NULL) {
		if(IS_SET(zone_table[rp->zone].reset_mode, ZONE_UNDER_GROUND)) {
			return(TRUE);
		}
	}

	return(FALSE);
}

void SetDefaultLang(struct char_data* ch) {
	int i;

	switch(GET_RACE(ch)) {
	case RACE_ELVEN:
	case RACE_GOLD_ELF:
	case RACE_WILD_ELF:
	case RACE_SEA_ELF:
	case RACE_DARK_ELF:
		i = LANG_ELVISH;
		break;
	case RACE_TROLL:
	case RACE_HALF_GIANT:
		i= LANG_GIANTISH;
		break;
	case RACE_HALF_OGRE:
		i=LANG_OGRE;
		break;
	case RACE_HALFLING:
		i=LANG_HALFLING;
		break;
	case RACE_DWARF:
		i=LANG_DWARVISH;
		break;
	case RACE_DEEP_GNOME:
	case RACE_GNOME:
		i=LANG_GNOMISH;
		break;
	default:
		i= LANG_COMMON;
		break;
	} /* end switch */
	ch->skills[i].learned = 95;
	SET_BIT(ch->skills[i].flags,SKILL_KNOWN);

	/* end default langauges sets */
}

int IsMagicSpell(int spell_num) {
	int tmp=FALSE;

	/* using non magic items, since everything else is almost magic */
	/* lot smaller switch this way */
	switch(spell_num) {
	case SKILL_BACKSTAB:
	case SKILL_SNEAK:
	case SKILL_HIDE:
	case SKILL_PICK_LOCK:
	case SKILL_KICK:
	case SKILL_BASH:
	case SKILL_RESCUE:
	case SKILL_FIRST_AID:
	case SKILL_SIGN:
	case SKILL_RIDE:
	case SKILL_SWITCH_OPP:
	case SKILL_DODGE:
	case SKILL_REMOVE_TRAP:
	case SKILL_RETREAT:
	case SKILL_QUIV_PALM:
	case SKILL_SAFE_FALL:
	case SKILL_FEIGN_DEATH:
	case SKILL_HUNT:
	case SKILL_FIND_TRAP:
	case SKILL_SPRING_LEAP:
	case SKILL_DISARM:
	case SKILL_EVALUATE:
	case SKILL_SPY:
	case SKILL_DOORBASH:
	case SKILL_SWIM:
	case SKILL_CONS_UNDEAD:
	case SKILL_CONS_VEGGIE:
	case SKILL_CONS_DEMON:
	case SKILL_CONS_ANIMAL:
	case SKILL_CONS_REPTILE:
	case SKILL_CONS_PEOPLE:
	case SKILL_CONS_GIANT:
	case SKILL_CONS_OTHER:
	case SKILL_DISGUISE:
	case SKILL_CLIMB:
	case SKILL_BERSERK:
	case SKILL_TAN:
	case SKILL_AVOID_BACK_ATTACK:
	case SKILL_FIND_FOOD:
	case SKILL_FIND_WATER:
	case SPELL_PRAYER:
	case SKILL_MEMORIZE:
	case SKILL_BELLOW:
	case SKILL_DOORWAY:
	case SKILL_PORTAL:
	case SKILL_SUMMON:
	case SKILL_INVIS:
	case SKILL_CANIBALIZE:
	case SKILL_FLAME_SHROUD:
	case SKILL_AURA_SIGHT:
	case SKILL_GREAT_SIGHT:
	case SKILL_PSIONIC_BLAST:
	case SKILL_HYPNOSIS:
	case SKILL_MEDITATE:
	case SKILL_SCRY:
	case SKILL_ADRENALIZE:
	case SKILL_RATION:
	case SKILL_HOLY_WARCRY:
	case SKILL_HEROIC_RESCUE:
	case SKILL_DUAL_WIELD:
	case SKILL_PSI_SHIELD:
	case SKILL_EAVESDROP:
	case LANG_COMMON:
	case LANG_ELVISH:
	case LANG_HALFLING:
	case LANG_DWARVISH:
	case LANG_ORCISH:
	case LANG_GIANTISH:
	case LANG_OGRE:
	case LANG_GNOMISH:
	case SKILL_ESP:                                /* end skills */

	case TYPE_HIT:                                /* weapon types here */
	case TYPE_BLUDGEON:
	case TYPE_PIERCE:
	case TYPE_SLASH:
	case TYPE_WHIP:
	case TYPE_CLAW:
	case TYPE_BITE:
	case TYPE_STING:
	case TYPE_CRUSH:
	case TYPE_CLEAVE:
	case TYPE_STAB:
	case TYPE_SMASH:
	case TYPE_SMITE:
	case TYPE_BLAST:
	case TYPE_SUFFERING:
	case TYPE_RANGE_WEAPON:
		tmp = FALSE;        /* these are NOT magical! */
		break;

	default:
		tmp = TRUE; /* default to IS MAGIC */
		break;
	} /* end switch */

	return(tmp);
}


int exist(char* s) {
	int f;

	f = open(s, O_RDONLY);

	if(f > 0) {
		close(f);
		return TRUE;
	}
	else {
		return FALSE;
	}
}


/* Good side just means they are of the first races and on the */
/* general good side of life, it does NOT refer to alignment */
/* only good side people can kill bad side people PC's */
int IsGoodSide(struct char_data* ch) {

	switch(GET_RACE(ch)) {

	case RACE_HUMAN     :
	case RACE_ELVEN     :
	case RACE_GOLD_ELF:
	case RACE_WILD_ELF:
	case RACE_SEA_ELF:
	case RACE_DWARF     :
	case RACE_HALFLING  :
	case RACE_GNOME     :
	case RACE_HALF_ELVEN  :
	case RACE_HALF_OGRE   :
	case RACE_HALF_ORC    :
	case RACE_HALF_GIANT  :
	case RACE_DARK_ELF:
		return(TRUE);
	} /* */

	return(FALSE);
}

/* this just means the PC is a troll/orc or the like not releated to */
/* to alignment what-so-ever */
/* only bad side people can hit and kill good side people PC's */
int IsBadSide(struct char_data* ch) {

	switch(GET_RACE(ch))  {
	case RACE_GOBLIN:
	case RACE_DARK_DWARF:
	case RACE_ORC:
	case RACE_TROLL:
	case RACE_MFLAYER:
		return(TRUE);
	} /* */

	return(FALSE);
}

/* good side can fight bad side people pc to pc fighting only, not used */
/* for NPC fighting to pc fighting */
int CanFightEachOther(struct char_data* ch,struct char_data* ch2) {

	if(!IS_PC(ch) || !IS_PC(ch2)) {
		return TRUE;
	}

	if(IS_SET(SystemFlags, SYS_NOKILL)) {
		return FALSE;
	}

    if(in_clan(ch, ch2))
    {
        return FALSE;
    }

	if(IS_SET(ch->player.user_flags,RACE_WAR) &&
			IS_SET(ch2->player.user_flags,RACE_WAR) /* &&
      real_roomp(ch->in_room)->sector_type!=SECT_INSIDE &&
      real_roomp(ch->in_room)->sector_type!=SECT_CITY &&
      real_roomp(ch->in_room)->sector_type!=SECT_WATER_SWIM */
	  ) {
		return TRUE;
	}
	return FALSE;
}


int fighting_in_room(int room_n) {
	struct char_data* ch;
	struct room_data* r;

	r=real_roomp(room_n);

	if(!r) {
		return FALSE;
	}

	for(ch=r->people; ch; ch=ch->next_in_room)
		if(ch->specials.fighting) {
			return TRUE;
		}

	return FALSE;
}


/** Perform breadth first search on rooms from start (in_room) **/
/** until end (tgt_room) is reached. Then return the correct   **/
/** direction to take from start to reach end.                 **/

void DoNothing(void* pDummy) {
	return;
}

bool inRange(int low, int high, int x)
{
    return ((x-high)*(x-low) <= 0);
}

// caricare troppo lo stack (WORLD_SIZE e'
// uguale a 50000)

int IsRoomDistanceInRange(int nFirstRoom, int nSecondRoom, int nRange) {
	struct room_q* tmp_q, *q_head, *q_tail;

	int i, tmp_room, count = 0, bThruDoors;
	struct room_data* herep, *therep;
	struct room_direction_data* exitp;

	/* If start = destination we are done */
	if(nFirstRoom == nSecondRoom) {
		return TRUE;
	}

	if(nRange < 0) {
		bThruDoors = FALSE;
		nRange = - nRange;
	}
	else {
		bThruDoors = TRUE;
	}


	memset(aRoom, 0, sizeof(aRoom));
	aRoom[ nFirstRoom ] = -1;

	/* initialize queue */
	q_head = (struct room_q*) malloc(sizeof(struct room_q));
	q_tail = q_head;
	q_tail->room_nr = nFirstRoom;
	q_tail->next_q = 0;

	while(q_head) {
		herep = real_roomp(q_head->room_nr);
		/* for each room test all directions */
		for(i = 0; i <= 5; i++) {
			exitp = herep->dir_option[ i ];
			if(exit_ok(exitp, &therep) &&
					exitp->to_room != NOWHERE &&
					(bThruDoors || !IS_SET(exitp->exit_info, EX_CLOSED))) {
				/* next room */
				tmp_room = herep->dir_option[ i ]->to_room;
				if(tmp_room != nSecondRoom) {
					/* shall we add room to queue ? */
					/* count determines total breadth and depth */
					if(!aRoom[ tmp_room ] && count < nRange) {
						count++;
						/* mark room as visted and put on queue */

						tmp_q = (struct room_q*) malloc(sizeof(struct room_q));
						tmp_q->room_nr = tmp_room;
						tmp_q->next_q = 0;
						q_tail->next_q = tmp_q;
						q_tail = tmp_q;

						/* ancestor for first layer is the direction */
						aRoom[ tmp_room ] = aRoom[ q_head->room_nr ] == -1 ?
											i + 1 : aRoom[ q_head->room_nr ];
					}
				}
				else {
					/* have reached our goal so free queue */
					tmp_room = q_head->room_nr;
					for(; q_head; q_head = tmp_q) {
						tmp_q = q_head->next_q;
						free(q_head);
					}

					return TRUE;
				}
			}
		}

		/* free queue head and point to next entry */
		tmp_q = q_head->next_q;
		free(q_head);
		q_head = tmp_q;
	}
	/* couldn't find path */
	return FALSE;
}


void RecurseRoom(long lInRoom, int iLevel, int iMaxLevel,
				 unsigned char* achVisitedRooms) {
	if(iLevel == 0) {
		achVisitedRooms[ lInRoom ] = 1;
	}
	if(iLevel < iMaxLevel) {
		for(int iEx = 0; iEx < 6; iEx++) {
			struct room_direction_data* pExit;
			pExit = real_roomp(lInRoom)->dir_option[ iEx ];
			if(pExit && real_roomp(pExit->to_room) &&
					(!achVisitedRooms[ pExit->to_room ] ||
					 achVisitedRooms[ pExit->to_room ] > iLevel + 1)) {
				achVisitedRooms[ pExit->to_room ] = iLevel + 1;
				RecurseRoom(pExit->to_room, iLevel + 1, iMaxLevel, achVisitedRooms);
			}
		}
	}
}

bool RecurseRoomChar(long lInRoom, int iLevel, int iMaxLevel,
					 unsigned char* achVisitedRooms, char_data* pChar) {
	if(pChar->in_room == lInRoom) {
		return TRUE;
	}

	if(iLevel == 0) {
		achVisitedRooms[ lInRoom ] = 1;
	}
	if(iLevel < iMaxLevel) {
		for(int iEx = 0; iEx < 6; iEx++) {
			struct room_direction_data* pExit;
			pExit = real_roomp(lInRoom)->dir_option[ iEx ];
			if(pExit && real_roomp(pExit->to_room) &&
					(!achVisitedRooms[ pExit->to_room ] ||
					 achVisitedRooms[ pExit->to_room ] > iLevel + 1)) {
				achVisitedRooms[ pExit->to_room ] = iLevel + 1;
				if(RecurseRoomChar(pExit->to_room, iLevel + 1, iMaxLevel,
								   achVisitedRooms, pChar)) {
					return TRUE;
				}
			}
		}
	}
	return FALSE;
}

bool IsCharInRange(char_data* pChar, char_data* pTarget, int iRange) {
	unsigned char achVisitedRooms[WORLD_SIZE];
	memset(achVisitedRooms, 0, WORLD_SIZE * sizeof(unsigned char));
	bool bRetValue = RecurseRoomChar(pChar->in_room, 0, iRange,
									 achVisitedRooms, pTarget);
	return bRetValue;
}

bool ThereIsPCInRoom(long lRoom) {
	if(real_roomp(lRoom)) {
		for(char_data* pChar = real_roomp(lRoom)->people; pChar;
				pChar = pChar->next_in_room)
			if(IS_PC(pChar)) {
				return TRUE;
			}
	}
	return FALSE;
}
int HowManyPCInRoom(long lRoom) {
	int r=0;
	if(real_roomp(lRoom)) {
		for(char_data* pChar = real_roomp(lRoom)->people; pChar;
				pChar = pChar->next_in_room)
			if(IS_PC(pChar)) {
				r++;
			}
	}
	return r;
}

bool CanSeePCInRoom(char_data* pMob) {
	long lRoom = pMob->in_room;
	if(real_roomp(lRoom)) {
		for(char_data* pChar = real_roomp(lRoom)->people; pChar;
				pChar = pChar->next_in_room)
			if(IS_PC(pChar) && CAN_SEE(pMob, pChar)) {
				return TRUE;
			}
	}
	return FALSE;
}
int HowManyCanSeePCInRoom(char_data* pMob) {
	int r=0;
	long lRoom = pMob->in_room;
	if(real_roomp(lRoom)) {
		for(char_data* pChar = real_roomp(lRoom)->people; pChar;
				pChar = pChar->next_in_room)
			if(IS_PC(pChar) && CAN_SEE(pMob, pChar)) {
				r++;
			}
	}
	return r;
}

bool CheckPrac(int classe, int id, int liv) {  // SALVO implemento un controllo su quello che puo' praccare una classe
	int kwar[]= {
		SKILL_KICK,
		SKILL_BASH,
		SKILL_RESCUE,
		SKILL_TAN,
		SKILL_BELLOW,
		0
	};

	int kthi[]= {
		SKILL_SNEAK,
		SKILL_HIDE,
		SKILL_STEAL,
		SKILL_BACKSTAB,
		SKILL_PICK_LOCK,
		SKILL_SPY,
		SKILL_RETREAT,
		SKILL_FIND_TRAP,
		SKILL_REMOVE_TRAP,
		SKILL_TSPY,
		SKILL_EAVESDROP,
		0
	};

	int kmon[]= {
		SKILL_QUIV_PALM,
		SKILL_FINGER,
		SKILL_FEIGN_DEATH,
		SKILL_RETREAT,
		SKILL_KICK,
		SKILL_HIDE,
		SKILL_SNEAK,
		SKILL_PICK_LOCK,
		SKILL_SAFE_FALL,
		SKILL_DISARM,
		SKILL_DODGE,
		SKILL_SWITCH_OPP,
		SKILL_SPRING_LEAP,
		0
	};

	int kbar[]= {
		SKILL_DISARM,
		SKILL_DOORBASH,
		SKILL_SPY,
		SKILL_RETREAT,
		SKILL_SWITCH_OPP,
		SKILL_DISGUISE,
		SKILL_CLIMB,
		SKILL_HUNT,
		SKILL_DODGE,
		SKILL_BASH,
		SKILL_FIRST_AID,
		SKILL_BERSERK,
		SKILL_AVOID_BACK_ATTACK,
		SKILL_FIND_FOOD,
		SKILL_FIND_WATER,
		SKILL_TAN,
		SKILL_HIDE,
		SKILL_BELLOW,
		0
	};

	int kpri[]= {
		SKILL_RESCUE,
		SKILL_HIDE,
		SKILL_FEIGN_DEATH,
		SKILL_HUNT,
		SKILL_SPY,
		SKILL_DOORWAY,
		SKILL_HOLY_WARCRY,
		SPELL_REINCARNATE,
		SPELL_CHAIN_LIGHTNING,
		SPELL_CURE_CRITIC,
		0
	};

	int f;

	switch(classe) {
	case CLASS_MAGIC_USER:
		if(spell_info[id].min_level_magic && spell_info[id].min_level_magic <= liv) {
			return TRUE;
		}
		break;
	case CLASS_CLERIC:
		if(spell_info[id].min_level_cleric && spell_info[id].min_level_cleric <= liv) {
			return TRUE;
		}
		break;
	case CLASS_WARRIOR:
		for(f=0; kwar[f]; f++) {
			if(id == kwar[f]) {
				return TRUE;
			}
		}
		break;
	case CLASS_THIEF:
		for(f=0; kthi[f]; f++) {
			if(id == kthi[f]) {
				return TRUE;
			}
		}
		break;
	case CLASS_DRUID:
		if(spell_info[id].min_level_druid && spell_info[id].min_level_druid <= liv) {
			return TRUE;
		}
		break;
	case CLASS_MONK:
		for(f=0; kmon[f]; f++) {
			if(id == kmon[f]) {
				return TRUE;
			}
		}
		break;
	case CLASS_BARBARIAN:
		for(f=0; kbar[f]; f++) {
			if(id == kbar[f]) {
				return TRUE;
			}
		}
		break;
	case CLASS_SORCERER:
		if(spell_info[id].min_level_sorcerer && spell_info[id].min_level_sorcerer <= liv) {
			return TRUE;
		}
		break;
	case CLASS_PALADIN:
		if(spell_info[id].min_level_paladin && spell_info[id].min_level_paladin <= liv) {
			return TRUE;
		}
		break;
	case CLASS_RANGER:
		if(spell_info[id].min_level_ranger && spell_info[id].min_level_ranger <= liv) {
			return TRUE;
		}
		break;
	case CLASS_PSI:
		if(spell_info[id].min_level_psi && spell_info[id].min_level_psi <= liv) {
			return TRUE;
		}
		break;
	case 0: // SALVO lo considero principe
		if(liv != PRINCIPE) {
			return FALSE;
		}
		for(f=0; kpri[f]; f++) {
			if(id == kpri[f]) {
				return TRUE;
			}
		}
		break;
	}
	return FALSE;
}
const char* RandomWord() {
	const static char* stringa[50] = {
		"argle",
		"bargle",
		"glop",
		"glyph",
		"hussamah",  /* 5 */
		"rodina",
		"mustafah",
		"angina",
		"il",
		"fribble",  /* 10 */
		"fnort",
		"frobozz",
		"zarp",
		"ripple",
		"yrk",    /* 15 */
		"yid",
		"yerf",
		"oork",
		"grapple",
		"rosso",   /* 20 */
		"blu",
		"tu",
		"me",
		"ftagn",
		"hastur",   /* 25 */
		"brob",
		"gnort",
		"lram",
		"truck",
		"uccidi",    /* 30 */
		"cthulhu",
		"huzzah",
		"acetacitacilicio",
		"idroxipropilene",
		"summah",     /* 35 */
		"hummah",
		"biscotti",
		"ema",
		"voglia",
		"wadapatang",   /* 40 */
		"pterodactilo",
		"frob",
		"yuma",
		"gomma",
		"lo-pan",   /* 45 */
		"sushi",
		"yaya",
		"yoyodine",
		"yaazr",
		"bipsnop"   /* 50 */
	};

	return(stringa[ number(0, 49) ]);

}
char RandomChar() {
	static char stringa[] = "abcdefghijklmnopqrstuwxyz23456789\0";
	return(stringa[ number(0, strlen(stringa)-1) ]);

}

char RemColorString(char * buffer)
{
    for(char* car = buffer; *car != '\0'; car++)
    {
        if(*car == '$' && toupper(*(car + 1)) == 'C')
        {
            char* next = car;
            while (*next != '\0')
            {
                *next = *(next + 6);
                next++;
            }
        }
        else if(*car == '$' && *(car + 1) == '$' && toupper(*(car + 2)) == 'C')
        {
            char* next = car;
            while (*next != '\0')
            {
                *next = *(next + 7);
                next++;
            }
        }
    }
    return *buffer;
}
} // namespace Alarmud
