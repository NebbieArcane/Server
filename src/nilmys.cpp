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
#include "act.comm.hpp"
#include "act.social.hpp"
#include "comm.hpp"
#include "db.hpp"
#include "fight.hpp"
#include "handler.hpp"
#include "interpreter.hpp"
#include "spec_procs.hpp"
#include "spell_parser.hpp"
namespace Alarmud {

    // mob's define
#define STANISLAV_SPECTRE   8916
#define BORIS_IVANHOE       9000

    //  room's define
#define STANISLAV_ROOM      8957
#define SHADOWS_PORTAL      8987
#define BORIS_HOME          9013

    //  obj's define
#define NILMYS_DIAMOND      9096

MOBSPECIAL_FUNC(stanislav_spirit)
{
    struct char_data* pNilmys;
    struct char_data* p;
    struct obj_data* object;
    int r_num;

    if(type == EVENT_DEATH && ch->in_room == STANISLAV_ROOM)
    {
        if((pNilmys = read_mobile(real_mobile(STANISLAV_SPECTRE), REAL)))
        {
            char_to_room(pNilmys, STANISLAV_ROOM);

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
            act("$c0015[$c0005$n$c0015] dice 'Non raggiungerete mai Boris\n\r"
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

MOBSPECIAL_FUNC(Boris_Ivanhoe)
{
    struct char_data* tch;
    struct char_data* boris;

    boris = 0;

    for(tch = real_roomp(ch->in_room)->people; (!boris) && (tch); tch = tch->next_in_room)
    {
        if(IS_MOB(tch))
        {
            if(mob_index[tch->nr].iVNum == BORIS_IVANHOE)
            {
                boris = tch;
            }
        }
    }

    mudlog(LOG_PLAYERS, "generic = %d e room = %d", boris->generic, boris->in_room);

    switch(boris->generic)
    {
        case 0:
        {
            for(tch = real_roomp(boris->in_room)->people; tch; tch = tch->next_in_room)
            {
                if(IS_PC(tch))
                {
                    if(CAN_SEE(boris, tch))
                    {
                        CheckBorisRoom(boris);
                        boris->generic = 1;
                        break;
                    }
                }
            }
            return FALSE;
        }
        case 1:
        {
            if(cmd == CMD_NOD)
            {
                if(!IS_PC(ch) && ch->master == NULL)
                {
                    boris->generic = 2;
                }
                else if(IS_PC(ch) && ch->master == NULL)
                {
                    extract_char(boris);
                    boris = read_mobile(real_mobile(BORIS_IVANHOE), REAL);
                    char_to_room(boris, BORIS_HOME);
                    boris->specials.quest_ref = ch;
                    boris->generic = 2;
                }
                else if(IS_PC((ch)->master))
                {
                    extract_char(boris);
                    boris = read_mobile(real_mobile(BORIS_IVANHOE), REAL);
                    char_to_room(boris, BORIS_HOME);
                    boris->specials.quest_ref = (ch)->master;
                    boris->generic = 2;
                }
            }
        }
            break;

        case 2:
            if(boris->specials.quest_ref != NULL)
            {
                if(boris->master)
                {
                    stop_follower(boris);
                }
                add_follower(boris, (boris)->specials.quest_ref);
                act("$c0015[$c0005$n$c0015] dice 'Ora andiamo, abbiamo poco tempo!'", FALSE, boris, NULL, NULL, TO_ROOM);
                boris->generic = 3;
            }
            else
            {
                send_to_room("$c0015[$c0005Boris Ivanhoe Gudonov$c0015] dice: 'Non fatemi perdere tempo!'\n\r", ch->in_room);
                send_to_room("$c0015[$c0005Boris Ivanhoe Gudonov$c0015] dice: 'C'e' qualcuno che comanda qui?'\n\r", ch->in_room);
                boris->generic = 1;
                return FALSE;
            }
            break;

        case 3:
            if(boris->master != NULL)
            {
                CheckBorisRoom(boris);
                return FALSE;
            }
            else
            {
                send_to_room("$c0015[$c0005Boris Ivanhoe Gudonov$c0015] dice: 'Abbiamo perso la nostra guida!'\n\r", boris->in_room);
                send_to_room("$c0015[$c0005Boris Ivanhoe Gudonov$c0015] dice: 'Il tempo scorre, ditemi chi devo seguire!'\n\r", boris->in_room);
                boris->generic = 1;
                return FALSE;
            }
            break;

        default:
            break;
    }

    return FALSE;
}

void CheckBorisRoom(struct char_data* boris)
{
    switch(boris->in_room)
    {
        case BORIS_HOME:
            if(boris->generic == 0)
            {
                send_to_room("$c0015[$c0005Boris Ivanhoe Gudonov$c0015] dice:\n\r", BORIS_HOME);
                send_to_room("$c0015 'Ora che ci siete abbiamo qualche speranza. Abbiamo interrotto il rituale di Garebeth, colui che viaggia tra i piani.\n\r", BORIS_HOME);
                send_to_room("$c0015  Stava per evocare Arkhat, il dio divoratore, ma aveva bisogno di una parte di ognuno degli Stanislav,\n\r  la famiglia che eoni fa lo confino' li' dove risiede ora.\n\r", BORIS_HOME);
                send_to_room("$c0015  Abbiamo scoperto pero' che a Nilmys c'era un'altra Stanislav : Isrka, figlia illegittima del signore della citta' Aaron.\n\r", BORIS_HOME);
                send_to_room("$c0015  L'abbiamo protetta e portata in salvo nel portale un attimo prima che il rituale che ha convertito tutti i cittadini\n\r$c0015  in non morti avesse effetto su di noi e su di lei.\n\r", BORIS_HOME);
                send_to_room("$c0015  Alcuni dei miei compagni sono feriti, gli altri devono restare a proteggere Iskra.\n\r", BORIS_HOME);
                send_to_room("$c0015  Solo voi potete aiutarci.\n\r", BORIS_HOME);
                send_to_room("$c0015  Chiunque di voi comandi o sia colui che guida in battaglia il gruppo mi annuisca e io lo seguiro'.\n\r$c0015  Vi raccontero' tutto lungo la strada.'\n\r", BORIS_HOME);
            }
            else
            {
                if(number(0, 12) == 5)
                {
                    act("$c0015[$c0005$n$c0015] dice 'Cosa facciamo fermi qui? Andiamo!'", FALSE, boris, NULL, NULL, TO_ROOM);
                }
            }
            break;

        case 9008:
            break;

        case 9020:
            break;

        case 9025:
            break;

        case 9032:
            // se chiave o non chiave
            break;

        case 9034:
            break;

        case 9035:
            break;

        case 9037:
            break;

        default:
        {
            if(boris->in_room == boris->commandp)
            {
                if(number(0, 12) == 5)
                {
                    act("$c0015[$c0005$n$c0015] dice 'Cosa facciamo fermi qui? Andiamo!'", FALSE, boris, NULL, NULL, TO_ROOM);
                }
            }
            boris->commandp = boris->in_room;

            switch(number(0, 35))
            {
                case 0:
                    do_say(boris, "Quel dannato Garebeth la paghera'... ah se la paghera'!", 0);
                    break;
                case 2:
                    do_say(boris, "Non indugiate troppo! Il destino di molta gente dipende da noi!", 0);
                    break;
                case 7:
                    do_say(boris, "Valutiamo bene ogni scelta e non sottovalutiamo nessun nemico...", 0);
                    break;
                case 11:
                    do_say(boris, "Arkhat probabilmente ci ridurra' in pezzi! Non temo il mio destino, non temiate il vostro.", 0);
                    break;
                case 15:
                    do_say(boris, "Andiamo avanti, non esitiamo!", 0);
                    do_say(boris, "Dobbiamo farlo per la gente di Nilmys, dobbiamo farlo per Vlad e per i compagni caduti!", 0);
                    do_say(boris, "Non ci arrenderemo mai!", 0);
                    break;
                case 18:
                    do_say(boris, "I miei occhi ormai si sono abituati a questo buio, temo solo di sbattere inavvertitamente con un mignolo ad uno spigolo.", 0);
                    break;
                case 22:
                    do_say(boris, "Qualunque cosa accada, non temiate l'oscurita'...", 0);
                    do_say(boris, "Del resto siamo proprio dentro di essa...", 0);
                    do_say(boris, "Cosa potrebbe andare peggio?", 0);
                    break;
                case 26:
                    do_say(boris, "Quando mettero' le mani su Gorath vedrete come lo ridurro'! Lo avete gia' seccato?! Mi dovete una birra allora, era mio!", 0);
                    break;
                case 30:
                    do_say(boris, "Vi ho mai raccontato di quando Umag provo' a creare il suo primo golem?", 0);
                    do_say(boris, "Vi dico solo che uso' gli scarti alimentari di una taverna per farlo...", 0);
                    break;
                case 32:
                    do_say(boris, "Sapete che ero solito avere incontri con donne che vendevano amore e fedelta' a pagamento?", 0);
                    do_say(boris, "Beh una di queste aveva un odore simile alla creatura appena incontrata!", 0);
                    break;
                default:
                    break;
            }
        }
            break;
    }
}

OBJSPECIAL_FUNC(urna_nilmys)
    {
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

ROOMSPECIAL_FUNC(portale_ombra)
{
    if(type != EVENT_COMMAND)
    {
        return FALSE;
    }

    if(cmd == CMD_PUT)
    {
        int iVNum;
        char buf[128];
        struct obj_data* diamond;
        struct room_direction_data* exitp;

        arg = one_argument(arg, buf);

        if(!strcmp("diamante", buf))
        {
            diamond = get_obj_in_list_vis(ch, buf, ch->carrying);

            if(diamond)
            {
                iVNum = (diamond->item_number >= 0) ? obj_index[diamond->item_number].iVNum : 0;
                if(iVNum == NILMYS_DIAMOND)
                {
                    arg = one_argument(arg, buf);

                    if(!strcmp("pentacolo", buf))
                    {
                        act("Metti $p dentro al pentacolo.", FALSE, ch, diamond, 0, TO_CHAR);
                        act("$n mette $p dentro al pentacolo.", FALSE, ch, diamond, 0, TO_ROOM);
                        extract_obj(diamond);
                        send_to_room("\n\r", SHADOWS_PORTAL);
                        send_to_room("$c0008 Il pentacolo davanti a te si illumina ed inizia a pulsare di una luce nera abbagliante\n\r$c0008 che confonde i tuoi sensi e che la tua mente non riesce a far collimare.\n\r", SHADOWS_PORTAL);
                        send_to_room("$c0008 La parete di roccia inizia a vibrare e sembra lentamente assumere una consistenza d'ombra.\n\r\n\r", SHADOWS_PORTAL);
                        send_to_room("$c0008 Il portale e' aperto, puoi attraversare l'oscurita' e lanciarti verso l'ignoto.\n\r\n\r", SHADOWS_PORTAL);

                        exitp = EXIT(ch, 2);
                        if(IS_SET(exitp->exit_info, EX_LOCKED))
                        {
                            REMOVE_BIT(exitp->exit_info, EX_LOCKED);
                        }
                        if(IS_SET(exitp->exit_info, EX_CLOSED))
                        {
                            REMOVE_BIT(exitp->exit_info, EX_CLOSED);
                        }

                        return TRUE;
                    }
                    else
                    {
                        return FALSE;
                    }
                }
                else
                {
                    return FALSE;
                }
            }
        }
        else
        {
            return FALSE;
        }
    }

    return FALSE;
}

} // namespace Alarmud

