/*ALARMUD* (Do not remove *ALARMUD*, used to automagically manage these lines
 *ALARMUD* AlarMUD 2.0
 *ALARMUD* See COPYING for licence information
 *ALARMUD*/
/***************************  System  include ************************************/
#include <cstdio>
#include <cstring>
#include <cctype>
#include <cstdlib>
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
#include "nilmys.hpp"
#include "comm.hpp"
#include "db.hpp"
#include "handler.hpp"
namespace Alarmud {

#define NILMYS_MOB 8916
#define NILMYS_ROOM 8957

MOBSPECIAL_FUNC(stanislav_spirit)
{
    struct char_data* pNilmys;
    struct char_data* p;
    struct obj_data* object;
    int r_num;

    if(type == EVENT_DEATH && ch->in_room == NILMYS_ROOM)
    {
        if((pNilmys = read_mobile(real_mobile(NILMYS_MOB), REAL)))
        {
            char_to_room(pNilmys, NILMYS_ROOM);

            r_num = real_object(8922);
            object = read_object(r_num, REAL);
            obj_to_char(object, pNilmys);
            obj_from_char(object);
            equip_char(pNilmys, object, WEAR_BODY);

            r_num = real_object(8933);
            object = read_object(r_num, REAL);
            obj_to_char(object, pNilmys);
            obj_from_char(object);
            equip_char(pNilmys, object, WEAR_ARMS);

            r_num = real_object(8917);
            object = read_object(r_num, REAL);
            obj_to_char(object, pNilmys);

            act("\n\rAlla morte di $n una tetra ed eterea figura si leva dal suo cadavere,\n"
                "la sua anima ormai corrotta appartiene ad Arkhat, e brama $c0009sangue$c0015!", FALSE, mob, NULL, pNilmys, TO_ROOM);
            act("$c0015[$c0013$n$c0015] dice 'Non raggiungerete mai Boris\n\r"
                "e i suoi compagni... oggi perirete per mano mia!'",FALSE, pNilmys, NULL, NULL, TO_ROOM);

            for(p = real_roomp(ch->in_room)->people; p; p=p->next_in_room)
            {
                if(p->lastmkill != NULL && strstr(p->lastmkill, GET_NAME(ch)) && !pNilmys->specials.fighting)
                {
                    hit(pNilmys, p, 0);
                }
            }

        }
        return(TRUE);
    }

    if(type != EVENT_TICK)
    {
        return(FALSE);
    }

    if(!AWAKE(ch))
    {
        return(FALSE);
    }

    return(fighter(mob,cmd,arg,mob,type));
}

#define NILMYS_DIAMOND  9096

OBJSPECIAL_FUNC(urna_nilmys) {
    int numero, iVnum, r_num;
    int s1 = 0, s2 = 0, s3 = 0, s4 = 0;
    struct obj_data* tmp, *tmp_urna, *diamond;

    if(obj->iGeneric == 0)
    {
        for(tmp = obj->contains; tmp; tmp = tmp->next_content)
        {
            iVnum = (tmp->item_number >= 0) ? obj_index[tmp->item_number].iVNum : 0;
            switch(iVnum)
            {
                case 8914:
                    s1 = 1;
                    break;
                case 8916:
                    s2 = 1;
                    break;
                case 8917:
                    s3 = 1;
                    break;
                case 8937:
                    s4 = 1;
                default:
                    break;
            }
            if( s1 + s2 + s3 +s4 == 4)
            {
                numero = room_of_object(obj);
                send_to_room(" \n\r", numero);
                send_to_room("$c0008Un vento gelido entra nella cripta non appena seppellisci le $c0015ossa$c0008...\n\r", numero);
                send_to_room("$c0008Improvvisamente l'urna si illumina di $c0009rosso$c0008 poi, subito dopo, la luce scompare inghiottita dalle tenebre...\n\r", numero);
                send_to_room("$c0008Subito dopo un rumore sordo rimbomba nel sottosuolo, giureresti che provenga dalla chiesa.\n\r", numero);
                send_to_room(" \n\r", numero);

                for(tmp_urna = obj->contains; tmp_urna; tmp_urna = tmp_urna->next_content)
                {
                    extract_obj(tmp_urna);
                }

                r_num = real_object(NILMYS_DIAMOND);
                diamond = read_object(r_num, REAL);
                obj_to_obj(diamond, obj);

                obj->iGeneric = 1;
            }
        }
    }
    return FALSE;
}


} // namespace Alarmud

