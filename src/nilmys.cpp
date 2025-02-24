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
#include "act.info.hpp"
#include "act.move.hpp"
#include "act.obj1.hpp"
#include "act.social.hpp"
#include "act.wizard.hpp"
#include "comm.hpp"
#include "db.hpp"
#include "fight.hpp"
#include "handler.hpp"
#include "interpreter.hpp"
#include "opinion.hpp"
#include "reception.hpp"
#include "regen.hpp"
#include "spec_procs.hpp"
#include "spell_parser.hpp"
namespace Alarmud {

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

MOBSPECIAL_FUNC(Atropal)
{
    struct char_data* tch;
    struct char_data* atropal, *boris;
    struct obj_data* key;

    atropal = 0;

    for(tch = real_roomp(ch->in_room)->people; (!atropal) && (tch); tch = tch->next_in_room)
    {
        if(IS_MOB(tch))
        {
            if(mob_index[tch->nr].iVNum == ATROPAL)
            {
                atropal = tch;
            }
        }
    }

    boris = 0;

    for(tch = real_roomp(ch->in_room)->people; (!boris) && (tch); tch = tch->next_in_room)
    {
        if(IS_MOB(tch))
        {
            if(mob_index[tch->nr].iVNum == BORIS_IVANHOE_CLONE)
            {
                boris = tch;
            }
        }
    }

    if(type == EVENT_DEATH)
    {
        key = read_object(real_object(SHADOW_WALLS_KEY), REAL);
        obj_to_room(key, 9032);

        send_to_room("\n\r\n\rImprovvisamente una strana nebbia $c0013purpurea$c0007 si forma dal corpo ormai senza vita\n\rdi Atropal e, lentamente, inizia a formarsi quello che ti sembra essere un $c0015portale$c0007.\n\r\n\r", ATROPAL_ROOM);

        if(boris)
        {
            do_say(boris, "Presto entriamo prima che il portale scompaia!", 0);
            send_to_room("\n\rBoris entra nel portale e, subito dopo, una strana forza, proveniente dal portale stesso, ti risucchia dentro.\n\r", ATROPAL_ROOM);
            char_from_room(boris);
            char_to_room(boris, 9032);
        }
        else
        {
            send_to_room("\n\rTi avvicini titubante al portale e, subito dopo, una strana forza, proveniente dal portale stesso, ti risucchia dentro.\n\r", ATROPAL_ROOM);
        }

        MoveToonInRangeToRoom(ATROPAL_ROOM, ATROPAL_ROOM, 9032);

        send_to_room("\n\rAppena riapri gli occhi ti rendi conto di trovarti nuovamente al bivio che hai passato poche ore fa.\n\rNoti qualcosa per terra.\n\r", 9032);

        return TRUE;
    }

    return FALSE;
}

MOBSPECIAL_FUNC(Arkhat)
{
    struct char_data* tch;
    struct char_data* arkhat, *boris;
    struct obj_data* co, *o, *corpse;
    struct char_data* targ;
    struct room_data* rp;
    char buf[1024], name[128], sName[128];

    arkhat = 0;

    for(tch = real_roomp(ch->in_room)->people; (!arkhat) && (tch); tch = tch->next_in_room)
    {
        if(IS_MOB(tch))
        {
            if(mob_index[tch->nr].iVNum == ARKHAT_GOD)
            {
                arkhat = tch;
            }
        }
    }

    boris = 0;

    for(tch = real_roomp(ch->in_room)->people; (!boris) && (tch); tch = tch->next_in_room)
    {
        if(IS_MOB(tch))
        {
            if(mob_index[tch->nr].iVNum == BORIS_IVANHOE_CLONE)
            {
                boris = tch;
            }
        }
    }

    if(type == EVENT_DEATH)
    {
        if(boris)
        {
            boris->commandp2 = 1000;
            return TRUE;
        }
        else
        {
            mudlog(LOG_PLAYERS, "Arkhat was killed by players but Boris is not alive, this is not possible.")
            return TRUE;
        }
    }

    if(cmd && cmd != CMD_STEAL)
    {
        return(FALSE);
    }

    if(cmd == CMD_STEAL)
    {
        send_to_char("Hai troppa paura per provare a rubare qualcosa!\n\r", ch);
        return(TRUE);
    }

    DestroyedItems = 0;

    DamageStuff(arkhat, SPELL_ACID_BLAST, 100, 5);

    if(DestroyedItems)
    {
        act("$n emette un potente ruggito!", FALSE, arkhat, 0, 0, TO_ROOM);
        DestroyedItems = 0;
    }

    rp = real_roomp(arkhat->in_room);
    if(!rp)
    {
        return(FALSE);
    }

    for(co = rp->contents; co; co = co->next_content)
    {
        if(IS_CORPSE(co))
        {
            act("\n\r$c0009Arkhat divora $p.\n\r", TRUE, arkhat, co, 0, TO_ROOM);
            while(co->contains)
            {
                o = co->contains;
                obj_from_obj(o);
                obj_to_char(o, arkhat);
            }

            sprintf(sName, "%s", co->short_description);
            sprintf(name, "%s", co->name);
            strncpy(sName, &sName[12], strlen(sName));
            if(strlen(name) > 5)
            {
                strncpy(name, &name[6], strlen(name));
            }

            // carico il contenitore, gli do il nome del pg morto e ci metto l'eq, poi lo porto nella room cimitero
            corpse = read_object(real_object(NILMYS_CORPSE), REAL);

            sprintf(buf, "resti %s", name);
            free(corpse->name);
            corpse->name = (char*)strdup(buf);
            sprintf(buf, "i resti martoriati di $c0015%s$c0007", sName);
            free(corpse->short_description);
            corpse->short_description = (char*)strdup(buf);
            sprintf(buf, "Tutto quello che rimane dell'equipaggiamento di $c0015%s$c0007 e' sparso qui a terra.", sName);
            free(corpse->description);
            corpse->description = (char*)strdup(buf);

            extract_obj(co);  /* rimuovo il corpo */
            /* danneggio l'equipaggiamento della vittima */
            DamageStuff(arkhat, SPELL_ACID_BLAST, 100, 5);

            while(arkhat->carrying)
            {
                o = arkhat->carrying;
                if(o->obj_flags.type_flag == ITEM_CONTAINER)
                {
                    obj_from_char(o);
                    obj_to_room(o, MASS_GRAVE);
                }
                else
                {
                    obj_from_char(o);
                    obj_to_obj(o, corpse);
                }
            }
            obj_to_room(corpse, MASS_GRAVE);
            do_action(arkhat, NULL, CMD_GRIN);
            act("\n\r$c0008Arkhat rigurgita gli avanzi della sua vittima che si perdono nell'oscuro vuoto sotto al circolo rituale.\n\r", TRUE, arkhat, 0, 0, TO_ROOM);
            sprintf(buf, "\n\rImprovvisamente, dall'alto, cadono %s... che fine ingloriosa...\n\r", corpse->short_description);
            send_to_room(buf, MASS_GRAVE);
            return(TRUE);
        }
    }

    if(AWAKE(arkhat))
    {
        if((targ = FindAnAttacker(arkhat)) != NULL)
        {
            act("$c0009Spalanchi la sua enorme bocca ed attacchi!\n\r", FALSE, arkhat, 0, 0, TO_CHAR);
            act("$c0009$n $c0009spalanca la sua enorme bocca ed attacca!\n\r", FALSE, arkhat, 0, 0, TO_ROOM);
            if(!CAN_SEE(ch, targ))
            {
                if(saves_spell(targ, SAVING_PARA))
                {
                    act("$c0014$N $c0014evita con un balzo fulmineo il tuo morso!\n\r", FALSE, arkhat, 0, targ, TO_CHAR);
                    act("$c0014$N $c0014evita con un balzo fulmineo il morso di $n$c0014!\n\r", FALSE, arkhat, 0, targ, TO_NOTVICT);
                    switch(number(0, 1))
                    {
                        case 0:
                            act("$c0014Ti sposti rapidamente alla tua sinistra ed eviti le fauci di $n$c0014!\n\r", FALSE, arkhat, 0, targ, TO_VICT);
                            break;

                        default:
                            act("$c0014Ti sposti rapidamente alla tua destra ed eviti le fauci di $n$c0014!\n\r", FALSE, arkhat, 0, targ, TO_VICT);
                            break;
                    }
                    return(FALSE);
                }
            }
            if(!saves_spell(targ, SAVING_PARA))
            {
                act("$c0011Spalanchi le enormi fauci sbavanti e divori avidamente $N$c0011!", FALSE, arkhat, 0, targ, TO_CHAR);
                act("$c0011$n$c0011 spalanca le enormi fauci sbavanti e divora $N$c0011 avidamente!", FALSE, arkhat, 0, targ, TO_NOTVICT);
                act("$c0011Le fauci sbavanti di Arkhat si avvicinano sempre piu' a te... finche' non ti afferra con la mascella e ti divora con avidita'!", FALSE, arkhat, 0, targ, TO_VICT);
                send_to_char("$c0011Senti la tua carne dilaniarsi sotto la potenza delle fauci del Dio, poi... $c0008il buio.\n\r", targ);
                send_to_char("MMM.  Burp!\n\r", arkhat);

                sprintf(sName, "%s", (!IS_PC(targ) ? targ->player.short_descr : GET_NAME(ch)));
                sprintf(name, "%s", GET_NAME(targ));

                GET_HIT(targ) = 0;
                alter_hit(targ, 0);
                mudlog(LOG_PLAYERS, "%s killed by %s (being swallowed whole)", GET_NAME(targ), GET_NAME(arkhat));
                die(targ, 0, NULL);
                /*
                 all stuff to monster:  this one is tricky.  assume that corpse is
                 top item on item_list now that corpse has been made.
                 */
                rp = real_roomp(arkhat->in_room);
                if(!rp)
                {
                    return(FALSE);
                }
                for(co = rp->contents; co; co = co->next_content)
                {
                    if(IS_CORPSE(co))
                    {
                        /* il corpo della vittima dovrebbe essere il primo oggetto in lista */
                        while(co->contains)
                        {
                            o = co->contains;
                            obj_from_obj(o);
                            obj_to_char(o, arkhat);
                        }
                        extract_obj(co);  /* rimuovo il corpo */
                        /* danneggio l'equipaggiamento della vittima */
                        DamageStuff(arkhat, SPELL_ACID_BLAST, 100, 5);

                        // carico il contenitore, gli do il nome del pg morto e ci metto l'eq, poi lo porto nella room cimitero
                        corpse = read_object(real_object(NILMYS_CORPSE), REAL);

                        sprintf(buf, "resti %s", name);
                        free(corpse->name);
                        corpse->name = (char*)strdup(buf);
                        sprintf(buf, "i resti martoriati di $c0015%s$c0007", sName);
                        free(corpse->short_description);
                        corpse->short_description = (char*)strdup(buf);
                        sprintf(buf, "Tutto quello che rimane dell'equipaggiamento di $c0015%s$c0007 e' sparso qui a terra.", sName);
                        free(corpse->description);
                        corpse->description = (char*)strdup(buf);

                        while(arkhat->carrying)
                        {
                            o = arkhat->carrying;
                            if(o->obj_flags.type_flag == ITEM_CONTAINER)
                            {
                                obj_from_char(o);
                                obj_to_room(o, MASS_GRAVE);
                            }
                            else
                            {
                                obj_from_char(o);
                                obj_to_obj(o, corpse);
                            }
                        }
                        obj_to_room(corpse, MASS_GRAVE);
                        do_action(arkhat, NULL, CMD_GRIN);
                        act("\n\r$c0008Arkhat rigurgita gli avanzi della sua vittima che si perdono nell'oscuro vuoto sotto al circolo rituale.\n\r", TRUE, arkhat, 0, 0, TO_ROOM);
                        sprintf(buf, "\n\rImprovvisamente, dall'alto, cadono %s... che fine ingloriosa...\n\r", corpse->short_description);
                        send_to_room(buf, MASS_GRAVE);
                        return(TRUE);
                    }
                }
            }
            else
            {
                switch(number(0, 4))
                {
                    case 0:
                        act("$c0014$N$c0014 abbassa velocemente la testa ed evita le tue fauci!\n\r", FALSE, arkhat, 0, targ, TO_CHAR);
                        act("$c0014Abbassi velocemente la testa ed eviti le fauci di $n$c0014!\n\r", FALSE, arkhat, 0, targ, TO_VICT);
                        act("$c0014$N$c0014 abbassa velocemente la testa ed evita le fauci di $n$c0014!\n\r", FALSE, arkhat, 0, targ, TO_NOTVICT);
                    break;

                    case 1:
                        act("$c0014$N$c0014 indietreggia ed evita le tue fauci!\n\r", FALSE, arkhat, 0, targ, TO_CHAR);
                        act("$c0014Eviti le fauci di $n$c0014 facendo due passi indietro!\n\r", FALSE, arkhat, 0, targ, TO_VICT);
                        act("$c0014$N$c0014 indietreggia ed evita le fauci di $n$c0014!\n\r", FALSE, arkhat, 0, targ, TO_NOTVICT);
                    break;

                    case 2:
                        act("$c0014$N$c0014 si sposta alla sua sinistra ed evita le tue fauci!\n\r", FALSE, arkhat, 0, targ, TO_CHAR);
                        act("$c0014Ti sposti rapidamente a sinistra ed eviti le fauci di $n$c0014!\n\r", FALSE, arkhat, 0, targ, TO_VICT);
                        act("$c0014$N$c0014 si sposta alla sua sinistra ed evita le fauci di $n$c0014!\n\r", FALSE, arkhat, 0, targ, TO_NOTVICT);
                    break;

                    case 3:
                        act("$c0014$N$c0014 si sposta alla sua destra ed evita le tue fauci!\n\r", FALSE, arkhat, 0, targ, TO_CHAR);
                        act("$c0014Ti sposti rapidamente a destra ed eviti le fauci di $n$c0014!\n\r", FALSE, arkhat, 0, targ, TO_VICT);
                        act("$c0014$N$c0014 si sposta alla sua destra ed evita le fauci di $n$c0014!\n\r", FALSE, arkhat, 0, targ, TO_NOTVICT);
                    break;

                    default:
                    {
                        if(targ->equipment[WEAR_SHIELD])
                        {
                            act("$c0014$N$c0014 blocca con lo scudo le tue fauci!\n\r", FALSE, arkhat, 0, targ, TO_CHAR);
                            act("$c0014Blocchi con lo scudo le fauci di $n$c0014!\n\r", FALSE, arkhat, 0, targ, TO_VICT);
                            act("$c0014$N$c0014 blocca con lo scudo le fauci di $n$c0014!\n\r", FALSE, arkhat, 0, targ, TO_NOTVICT);
                        }
                        else
                        {
                            act("$c0014$N$c0014 abbassa velocemente la testa ed evita le tue fauci!\n\r", FALSE, arkhat, 0, targ, TO_CHAR);
                            act("$c0014Abbassi velocemente la testa ed eviti le fauci di $n$c0014!\n\r", FALSE, arkhat, 0, targ, TO_VICT);
                            act("$c0014$N$c0014 abbassa velocemente la testa ed evita le fauci di $n$c0014!\n\r", FALSE, arkhat, 0, targ, TO_NOTVICT);
                        }
                    }
                    break;
                }
            }
        }
    }
    return FALSE;
}

void ArkhatDeath(struct char_data* boris)
{
    struct obj_data* portale;

    switch(boris->commandp2)
    {
        case 1000:
            send_to_room("\n\r\n\r$c0014Con l'ultimo poderoso colpo $c0013Arkhat$c0014 vacilla, la spada di Vlad penetra profondamente\n\r", boris->in_room);
            send_to_room("$c0014nel suo mostruoso petto.\n\r", boris->in_room);
            break;

        case 1001:
            send_to_room("\n\r$c0014Una grande $c0015luce$c0014 inizia a generarsi dalla spada e diventa sempre piu' intesa... poi...\n\r", boris->in_room);
            send_to_room("$c0014con una fragorosa esplosione di $c0015luce $c0013Arkhat$c0014, il divoratore, di dissolve come cancellato\n\r", boris->in_room);
            send_to_room("$c0014dall'esistenza e dalla memoria.\n\r", boris->in_room);
            break;

        case 1002:
            send_to_room("\n\r$c0014Le $c0015anime$c0014 che lo circondavano e si tormentavano in una macabra danza attorno al suo corpo,\n\r", boris->in_room);
            send_to_room("$c0014iniziano ad emettere $c0015luce$c0014 ed a convergere verso un punto in alto poco sopra la tua testa.\n\r\n\r", boris->in_room);
            send_to_room("$c0014Noti una di esse che indugia sulla carcassa materiale rimasta del $c0013Dio$c0014 sconfitto.\n\r", boris->in_room);
            break;

        case 1003:
            send_to_room("$c0014E' lei la prima $c0015anima$c0014 presa da $c0013Arkhat$c0014, e' lei che lentamente sale fino al punto in cui tutte\n\r", boris->in_room);
            send_to_room("$c0014le altre sono radunate.\n\r", boris->in_room);
            break;

        case 1004:
            send_to_room("\n\r$c0014Un varco si apre e tutte le $c0015anime$c0014 entrano rapidamente disperdendosi verso una grande $c0015luce$c0014.\n\r", boris->in_room);
            portale = read_object(real_object(VICTORY_PORTAL), REAL);
            obj_to_room(portale, boris->in_room);
            break;

        case 1005:
            send_to_room("\n\r$c0014Boris esclama '$c0011VITTORIA!$c0014', e sale verso la $c0015luce$c0014.\n\r\n\r", boris->in_room);
            boris->generic = 100;
            act("Entri nel Portale della Vittoria.", FALSE, boris, NULL, NULL, TO_CHAR);
            act("$n entra nel Portale della Vittoria.", FALSE, boris, NULL, NULL, TO_ROOM);
            char_from_room(boris);
            char_to_room(boris, 9121);
            break;

        default:
            break;
    }
    boris->commandp2 += 1;
}

int CountBorisParty(struct char_data* boris)
{
    struct follow_type* fol;
    int totale = 0;

    if(!boris->master)
    {
        return 0;
    }

    for(fol = boris->master->followers; fol; fol = fol->next)
    {
        if(boris->in_room == fol->follower->in_room)
        {
            if(IS_PC(fol->follower) && !IS_IMMORTALE(fol->follower))
            {
                totale += 1;
            }
        }
    }
    return totale;
}

void BorisInFight(struct char_data* boris)
{
    struct char_data* targ;
    struct follow_type* fol;
    char buf[256];
    int found = FALSE;

    const char* rand_Boris_says[] = {
        "\n\r$c0009[$c0015$n$c0009] urla 'Per Vlad!'\n\r",
        "\n\r$c0009[$c0015$n$c0009] urla 'Assaggia questo, oscura creatura!'\n\r",
        "\n\r$c0009[$c0015$n$c0009] urla 'Ora capirai che significa sfidare Boris!'\n\r",
        "\n\r$c0009[$c0015$n$c0009] urla 'Mi piace annientare bestie come voi!'\n\r",
        "\n\r$c0009[$c0015$n$c0009] urla 'Su fatemi divertire, cerchiamo un degno avversario!'\n\r",
        "\n\r$c0009[$c0015$n$c0009] urla 'Il primo che lo ammazza si prende tutto l'eq!'\n\r",
        "\n\r$c0009[$c0015$n$c0009] urla 'Mi sto solo riscaldando!'\n\r",
        "\n\r$c0009[$c0015$n$c0009] urla 'Cercate di starmi dietro!'\n\r",
        "\n\r$c0009[$c0015$n$c0009] urla 'Io lo colpisco alla testa, tu mira alle gambe!'\n\r",
        "\n\r$c0009[$c0015$n$c0009] urla 'Posso unirmi alla festa?!?'\n\r",
        "\n\r$c0009[$c0015$n$c0009] urla 'Vediamo quanto dura questo!'\n\r",
        "\n\r$c0009[$c0015$n$c0009] urla 'Fate largo al Re del DPS!'\n\r"
    };

    const int maxBorisSay = 11;

    if(boris->in_room > -1)
    {
        if(!boris->master)
        {
            return;
        }

        if(boris->in_room == boris->master->in_room && boris->master->specials.fighting)
        {
            targ = FindAnAttacker(boris->master);

            if(targ)
            {
                found = TRUE;
                sprintf(buf, "%s", rand_Boris_says[number(0, maxBorisSay)]);
            }

            if(found)
            {
                act(buf, FALSE, boris, 0, 0, TO_ROOM);

                if(CAN_SEE(boris, targ))
                {
                    hit(boris, targ, 0);
                }
            }
            return;
        }

        for(fol = boris->master->followers; fol; fol = fol->next)
        {
            if(boris->in_room == fol->follower->in_room && fol->follower->specials.fighting)
            {
                targ = FindAnAttacker(fol->follower);

                if(targ)
                {
                    found = TRUE;
                    sprintf(buf, "%s", rand_Boris_says[number(0, maxBorisSay)]);
                }

                if(!found)
                {
                    return;
                }

                act(buf, FALSE, boris, 0, 0, TO_ROOM);

                if(CAN_SEE(boris, targ))
                {
                    hit(boris, targ, 0);
                }
            }
        }
    }
}

MOBSPECIAL_FUNC(Boris_Ivanhoe)
{
    struct char_data* tch;
    struct char_data* boris, *umag;
    struct obj_data* eq_boris;
    int j;
    char tmp[MAX_INPUT_LENGTH];

    boris = 0;

    for(tch = real_roomp(ch->in_room)->people; (!boris) && (tch); tch = tch->next_in_room)
    {
        if(IS_MOB(tch))
        {
            if(mob_index[tch->nr].iVNum == BORIS_IVANHOE_CLONE)
            {
                boris = tch;
            }
        }
    }

    umag = 0;

    for(tch = real_roomp(UMAG_ROOM)->people; (!umag) && (tch); tch = tch->next_in_room)
    {
        if(IS_MOB(tch))
        {
            if(mob_index[tch->nr].iVNum == UMAG_ULBAR)
            {
                umag = tch;
            }
        }
    }

    if(type == EVENT_DEATH)
    {
        if(umag)
        {
            send_to_zone("\n\r$c0011Boris si accascia morente... con lui la speranza svanisce.\n\r", umag);
            umag->commandp = 1;
            BorisDeath(umag);
            return TRUE;
        }
        else
        {
            mudlog(LOG_PLAYERS, "Umag is missing, this is not possible.")
            return TRUE;
        }
    }

    if(!AWAKE(boris))
    {
        return FALSE;
    }
    else if(!(boris->specials.fighting))
    {
        if(CountBorisParty(boris) < 5 && CountBorisParty(boris) > 0)
        {
            BorisInFight(boris);
        }
    }

    if(cmd && cmd != CMD_TELL && cmd != CMD_ASK && cmd != CMD_NOD)
    {
        return(FALSE);
    }

    if(cmd == CMD_TELL)
    {
        one_argument(arg, tmp);
        if((tch = get_char_room_vis(ch, tmp)) != boris)
        {
          return FALSE;
        }

        if(strstr(arg, "seguimi"))
        {
            boris->specials.quest_ref = ch;
            stop_fighting(boris);
            StopAllFightingWith(boris);
            FreeHates(boris);
            return FALSE;
        }
        else if(strstr(arg, "casa") && boris->master)
        {
            if(boris->specials.fighting)
            {
                act("\n\rDici a $N di voler tornare a casa.\n\r", FALSE, ch, NULL, boris, TO_CHAR);
                act("\n\r$c0013[$c0015$n$c0015]$c0013 ti dice 'Voglio tornare a casa.'\n\r", FALSE, ch, NULL, boris, TO_VICT);
                act("\n\r$c0013[$c0015$n$c0015]$c0013 dice qualcosa a $N$c0013.\n\r", FALSE, ch, NULL, boris, TO_NOTVICT);

                do_say(boris, "Non ora, sono troppo impegnato!", 0);
                return TRUE;
            }

            act("\n\rDici a $N di voler tornare a casa.\n\r", FALSE, ch, NULL, boris, TO_CHAR);
            act("\n\r$c0013[$c0015$n$c0015]$c0013 ti dice 'Voglio tornare a casa.'\n\r", FALSE, ch, NULL, boris, TO_VICT);
            act("\n\r$c0013[$c0015$n$c0015]$c0013 dice qualcosa a $N$c0013.\n\r", FALSE, ch, NULL, boris, TO_NOTVICT);

            do_say(boris, "Capisco... Umag per tua fortuna mi ha istruito su di un sistema per rimandarti a casa immediatamente.", 0);

            act("\n\rSubito dopo $N tira fuori da un sacchetto un rametto runico bianco e te lo porge.\n\r", FALSE, ch, NULL, boris, TO_CHAR);
            act("\n\rTiri fuori da un sacchetto un rametto runico bianco e lo porgi a $n.\n\r", FALSE, ch, NULL, boris, TO_VICT);
            act("\n\r$N tira fuori da un sacchetto un rametto runico bianco e lo porge a $n.\n\r", FALSE, ch, NULL, boris, TO_NOTVICT);

            do_say(boris, "Spezzalo e tornerai a casa... buona fortuna.", 0);

            act("\n\rNon appena spezzi il rametto un $c0015lampo di luce$c0007 smaterializza il tuo corpo...\n\rTi ricomponi a Myst... finalmente sei a casa.\n\r", FALSE, ch, NULL, boris, TO_CHAR);
            act("\n\rNon appena $n spezza il rametto si smaterializza in un $c0015lampo di luce$c0007.\n\r", FALSE, ch, NULL, boris, TO_ROOM);

            char_from_room(ch);
            char_to_room(ch, 3001);
            do_look(ch, "\0", 15);
            return TRUE;
        }
        else
        {
            return FALSE;
        }
    }

    if(boris->specials.fighting)
    {
        return FALSE;
    }

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
                do_say(boris, "Se per qualsiasi ragione qualcun'altro volesse guidarci fino alla vittoria, mi dica di seguirlo ed io lo faro'!", 0);
                do_say(boris, "Inoltre, se qualcuno volesse ritirarsi, deve dirmi che vuole tornare a casa!", 0);
                do_say(boris, "Umag ha aperto un passaggio dalla cripta per tornare qui piu' velocemente in caso di necessita'.", 0);
                if(!IS_PC(ch) && ch->master == NULL)
                {
                    boris->generic = 4;
                }
                else if(IS_PC(ch) && ch->master == NULL)
                {
                    extract_char(boris);
                    boris = read_mobile(real_mobile(BORIS_IVANHOE_CLONE), REAL);
                    char_to_room(boris, BORIS_HOME);
                    if(IS_SET(boris->specials.act, ACT_IMMORTAL))
                    {
                        REMOVE_BIT(boris->specials.act, ACT_IMMORTAL);
                    }
                    boris->specials.quest_ref = ch;
                    boris->generic = 4;
                }
                else if(IS_PC((ch)->master))
                {
                    extract_char(boris);
                    boris = read_mobile(real_mobile(BORIS_IVANHOE_CLONE), REAL);
                    char_to_room(boris, BORIS_HOME);
                    if(IS_SET(boris->specials.act, ACT_IMMORTAL))
                    {
                        REMOVE_BIT(boris->specials.act, ACT_IMMORTAL);
                    }
                    boris->specials.quest_ref = (ch)->master;
                    boris->generic = 4;
                }
            }
        }
            break;

        case 2:
        {
            if(cmd && cmd != CMD_TELL)
            {
                return(FALSE);
            }
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
                send_to_room("$c0015[$c0005Boris Ivanhoe Gudonov$c0015] dice 'Non fatemi perdere tempo!'\n\r", boris->in_room);
                send_to_room("$c0015[$c0005Boris Ivanhoe Gudonov$c0015] dice 'C'e' qualcuno che comanda qui?'\n\r", boris->in_room);
                do_say(boris, "Chi vuole comandare mi dica di seguirlo!", 0);
                return FALSE;
            }
        }
            break;

        case 3:
        {
            if(cmd && cmd != CMD_TELL)
            {
                return(FALSE);
            }
            if(boris->master != NULL)
            {
                if(boris->master != boris->specials.quest_ref)
                {
                    boris->generic = 2;
                }
                CheckBorisRoom(boris);
                return FALSE;
            }
            else
            {
                send_to_room("$c0015[$c0005Boris Ivanhoe Gudonov$c0015] dice 'Abbiamo perso la nostra guida!'\n\r", boris->in_room);
                send_to_room("$c0015[$c0005Boris Ivanhoe Gudonov$c0015] dice 'Il tempo scorre, ditemi chi devo seguire!'\n\r", boris->in_room);
                do_say(boris, "Chi vuole comandare mi dica di seguirlo!", 0);
                boris->generic = 2;
                return FALSE;
            }
        }
            break;

        case 4:
        {
            eq_boris = read_object(real_object(9102), REAL);
            obj_to_char(eq_boris, boris);
            obj_from_char(eq_boris);
            equip_char(boris, eq_boris, WEAR_EYES);

            eq_boris = read_object(real_object(9103), REAL);
            obj_to_char(eq_boris, boris);
            obj_from_char(eq_boris);
            equip_char(boris, eq_boris, WIELD);

            eq_boris = read_object(real_object(9104), REAL);
            obj_to_char(eq_boris, boris);
            obj_from_char(eq_boris);
            equip_char(boris, eq_boris, WEAR_HANDS);

            eq_boris = read_object(real_object(9105), REAL);
            obj_to_char(eq_boris, boris);
            obj_from_char(eq_boris);
            equip_char(boris, eq_boris, WEAR_SHIELD);

            eq_boris = read_object(real_object(9106), REAL);
            obj_to_char(eq_boris, boris);
            obj_from_char(eq_boris);
            equip_char(boris, eq_boris, WEAR_LEGS);

            eq_boris = read_object(real_object(9107), REAL);
            obj_to_char(eq_boris, boris);
            obj_from_char(eq_boris);
            equip_char(boris, eq_boris, WEAR_FEET);

            boris->generic = 2;
        }
            break;

        case 100:
        {
            if(boris->master != NULL)
            {
                CheckBorisRoom(boris);
                return FALSE;
            }

            if(cmd == CMD_NOD)
            {
                struct char_data* new_boris;
                struct obj_data* equip;

                do_say(boris, "Bene. Possano le nostre strade incontrarsi di nuovo un giorno! Addio.", 0);
                send_to_room("\n\r", boris->in_room);

                //  estraggo l'equipaggiamento di Boris

                for(j = 0; j < MAX_WEAR; j++)
                {
                    if(boris->equipment[ j ])
                    {
                        obj_to_char(unequip_char(boris, j), boris);
                    }
                }

                if(boris->carrying)
                {
                    while(boris->carrying)
                    {
                        equip = boris->carrying;
                        obj_from_char(equip);
                        extract_obj(equip);
                    }
                }

                extract_char(boris);

                //  carico la versione di Boris senza special
                new_boris = read_mobile(real_mobile(BORIS_IVANHOE), REAL);
                char_to_room(new_boris, 9121);
            }

            if(cmd == CMD_ASK)
            {
                if(strstr(arg, "ricompensa") || strstr(arg, "premio"))
                {
                    act("$c0006$N$c0006 ti chiede la ricompensa.\n\r", FALSE, boris, NULL, ch, TO_CHAR);
                    act("$c0006Chiedi il giusto premio a $n.\n\r", FALSE, boris, NULL, ch, TO_VICT);
                    act("$c0006$N$c0006 chiede qualcosa a $n.\n\r", FALSE, boris, NULL, ch, TO_NOTVICT);
                    if(ch->generic == 10)
                    {
                        GiveRewardNilmys(boris, ch);
                    }
                    else if(ch->generic == 20)
                    {
                        act("$c0015[$c0013$n$c0015] dice '$N puoi ricevere il tuo premio una sola volta!'", FALSE, boris, NULL, ch, TO_ROOM);
                    }
                    else
                    {
                        act("$c0015[$c0013$n$c0015] dice '$N non sei degn$B di ricevere il premio!'", FALSE, boris, NULL, ch, TO_ROOM);
                    }
                    return TRUE;
                }
                else
                {
                    act("$c0006$N$c0006 ti chiede qualcosa di completamente inutile.", FALSE, boris, NULL, ch, TO_CHAR);
                    act("$c0006Chiedi qualcosa di completamente inutile a $n.", FALSE, boris, NULL, ch, TO_VICT);
                    act("$c0006$N$c0006 chiede qualcosa a $n.", FALSE, boris, NULL, ch, TO_NOTVICT);
                    return TRUE;
                }
            }
        }
            break;

        default:
            break;
    }

    return FALSE;
}

MOBSPECIAL_FUNC(Umag_Ulbar)
{
    struct char_data* umag, *tch;

    umag = 0;

    for(tch = real_roomp(UMAG_ROOM)->people; (!umag) && (tch); tch = tch->next_in_room)
    {
        if(IS_MOB(tch))
        {
            if(mob_index[tch->nr].iVNum == UMAG_ULBAR)
            {
                umag = tch;
            }
        }
    }

    if(!AWAKE(umag))
    {
        return FALSE;
    }

    if(cmd && cmd != CMD_GIVE)
    {
        return(FALSE);
    }

    if(umag->commandp > 1)
    {
        BorisDeath(umag);
    }
    return FALSE;
}

void BorisDeath(struct char_data* umag)
{
    struct obj_data* portale;

    portale = read_object(real_object(FAREWELL_PORTAL), REAL);
    obj_to_room(portale, 9012);

    switch(umag->commandp)
    {
        case 1:
            send_to_zone("\n\r$c0008Non appena la vita scivola via dal corpo di Boris, un varco dimensionale si apre.\n\r", umag);
            send_to_zone("$c0008Tamarang e Daggar prendono il corpo di Boris mentre Umag vi dice mestamente di avvicinarvi al varco.\n\r", umag);
            send_to_zone("$c0008Mentre vi avvicinate al passaggio, Umag compie un gesto con il suo bastone.\n\r", umag);
            send_to_zone("$c0008Chiudi per un attimo gli occhi e quando li riapri ti ritrovi nella caverna di Boris.\n\r", umag);
            MoveToonInRangeToRoom(9000, 9199, BORIS_HOME);
            break;

        case 3:
            send_to_room("\n\r$c0015Mentre gli altri iniziano a prendersi cura del corpo del compagno caduto, il mago vi parla per congedarvi:\n\r", BORIS_HOME);
            send_to_room("\n\r$c0015 'Dovete sapere che $c0013Arkhat$c0015 era possibile sconfiggerlo solo brandendo la spada del nipote di Boris.\n\r", BORIS_HOME);
            send_to_room("  $c0015Ed il solo Boris aveva il potere di evocarla.\n\r", BORIS_HOME);
            send_to_room("  $c0015Le speranze sono, ormai, perdute.\n\r", BORIS_HOME);
            send_to_room("  $c0015Noi resteremo in eterno in questa dimensione d'Ombra per non permettere ad Arkhat di manifestarsi.\n\r", BORIS_HOME);
            send_to_room("  $c0015Grazie a tutti voi, per il vostro tentativo.\n\r", BORIS_HOME);
            send_to_room("  $c0015Le nostre giornate saranno uguali all'infinito... non e' detto che non ci rincontreremo.'\n\r", BORIS_HOME);
            break;

        case 4:
            send_to_all("\n\r\n\r$c0009Daggar, Tamarang, Cormac, Ireiin, Isrka ed Umag vi ringraziano dal profondo del cuore per averci provato.\n\r");
            send_to_room("\n\r\n\r$c0008Il mago pronuncia delle parole arcane e, lentamente, l'immagine di Umag e della caverna iniziano a sbiadire...\n\r", BORIS_HOME);
            send_to_room("\n\r$c0015Una forte luce ti investe ed improvvisamente ti ritrovi di fronte alla fontana di Myst.\n\r", BORIS_HOME);
            MoveToonInRangeToRoom(9000, 9199, 3004);
            umag->commandp = 0;
            break;

        default:
            break;
    }
    umag->commandp += 1;
}

void CheckReward(struct char_data* boris)
{
    struct char_data* tch;

    if(boris)
    {
        for(tch = real_roomp(boris->in_room)->people; tch; tch = tch->next_in_room)
        {
            if(IS_PC(tch) && tch->generic == 10)
            {
                GiveRewardNilmys(boris, tch);
            }
        }
    }

}

void GiveRewardNilmys(struct char_data* boris, struct char_data* ch)
{
    struct obj_data* coin;
    char buf[256];
    int rune;

    ch->generic = 20;

    rune = ch->commandp;
    ch->commandp = 0;

    coin = read_object(real_object(NILMYS_COIN), REAL);

    sprintf(buf, "un Augustale di proprieta' di %s", GET_NAME(ch));
    free(coin->short_description);
    coin->short_description = (char*)strdup(buf);
    SetPersonOnSave(ch, coin);

    act("Dai $p a $N.", FALSE, boris, coin, ch, TO_CHAR);
    act("$n ti da' $p.", FALSE, boris, coin, ch, TO_VICT);
    act("$n da' $p a $N.", FALSE, boris, coin, ch, TO_NOTVICT);

    act("$c0013Tu dici a $N$c0013 'Ricordati $N$c0013, potrai scambiare tre monete di Nilmys con un premio.'\n\r", FALSE, boris, NULL, ch, TO_CHAR);
    act("$c0013[$c0015$n$c0015]$c0013 ti dice 'Ricordati $N$c0013, potrai scambiare tre monete di Nilmys con un premio.'\n\r", FALSE, boris, NULL, ch, TO_VICT);
    act("$c0013[$c0015$n$c0015]$c0013 dice qualcosa a $N$c0013.\n\r", FALSE, boris, NULL, ch, TO_NOTVICT);

    GET_RUNEDEI(ch) += rune;
    sprintf(buf,"$c0011Vieni marchiat%s con %d run%s degli Dei!$c0007\n\r\n\r",SSLF(ch), rune, (rune == 1 ? "a" : "e"));
    send_to_char(buf, ch);

    obj_to_char(coin, ch);

    do_say(boris, "Se tutti quanti avete ricevuto il vostro premio, fatemi un cenno con la testa!", 0);
    send_to_room("\n\r", 9121);
}

void MoveToonInRangeToRoom(int low_number, int high_number, int room)
{
    struct descriptor_data* i;

    if(low_number && high_number && room)
    {
        for(i = descriptor_list; i; i = i->next)
        {
            if(i->character->in_room >= low_number && i->character->in_room <= high_number)
            {
                if(!IS_IMMORTALE(i->character))
                {
                    char_from_room(i->character);
                    char_to_room(i->character, room);
                }
            }
        }
    }
}


void CheckBorisRoom(struct char_data* boris)
{
    if(!boris)
    {
        return;
    }

    if(boris->specials.fighting)
    {
        return;
    }

    switch(boris->in_room)
    {
        case BORIS_HOME:
            if(boris->generic == 0)
            {
                struct room_data* rp;

                send_to_room("\n\r$c0015[$c0005Boris Ivanhoe Gudonov$c0015] dice:\n\r", BORIS_HOME);
                send_to_room("$c0015 'Ora che ci siete abbiamo qualche speranza. Abbiamo interrotto il rituale di Garebeth, colui che viaggia tra i piani.\n\r", BORIS_HOME);
                send_to_room("$c0015  Stava per evocare Arkhat, il dio divoratore, ma aveva bisogno di una parte di ognuno degli Stanislav,\n\r  la famiglia che eoni fa lo confino' li' dove risiede ora.\n\r", BORIS_HOME);
                send_to_room("$c0015  Abbiamo scoperto pero' che a Nilmys c'era un'altra Stanislav : Isrka, figlia illegittima del signore della citta' Aaron.\n\r", BORIS_HOME);
                send_to_room("$c0015  L'abbiamo protetta e portata in salvo nel portale un attimo prima che il rituale che ha convertito tutti i cittadini\n\r$c0015  in non morti avesse effetto su di noi e su di lei.\n\r", BORIS_HOME);
                send_to_room("$c0015  Alcuni dei miei compagni sono feriti, gli altri devono restare a proteggere Iskra.\n\r", BORIS_HOME);
                send_to_room("$c0015  Solo voi potete aiutarci.\n\r", BORIS_HOME);
                send_to_room("$c0015  Chiunque di voi comandi o sia colui che guida in battaglia il gruppo mi annuisca e io lo seguiro'.\n\r$c0015  Vi raccontero' tutto lungo la strada.'\n\r\n\r", BORIS_HOME);

                rp = real_roomp(9133);
                CREATE(rp->dir_option[2], struct room_direction_data, 1);
                rp->dir_option[2]->exit_info = 0;
                rp->dir_option[2]->to_room = BORIS_HOME;
            }
            else
            {
                if(number(0, 30) == 26)
                {
                    act("\n\r$c0015[$c0005$n$c0015] dice 'Cosa facciamo fermi qui? Andiamo!'\n\r", FALSE, boris, NULL, NULL, TO_ROOM);
                }
            }
            break;

        case 9008:
        {
            if(boris->in_room != boris->commandp)
            {
                send_to_room("\n\r$c0015[$c0005Boris Ivanhoe Gudonov$c0015] dice:\n\r", 9008);
                send_to_room("$c0015 'Attento al Golem di Umag, l'ha piazzato qui a protezione della grotta, se non lo tocchi non ti attacchera'.\n\r  Ci sono diverse trappole lungo il cammino, Umag e' molto prudente, state attenti.\n\r\n\r", 9008);
            }
            boris->commandp = boris->in_room;
        }
            break;

        case 9020:
        {
            if(boris->in_room != boris->commandp)
            {
                send_to_room("\n\r$c0015[$c0005Boris Ivanhoe Gudonov$c0015] dice:\n\r", 9020);
                send_to_room("$c0015 'Attenti ora, c'e' una piana molto pericolosa qui sotto.\n\r  $c0015Diverse creature si aggirano in cerca di prede, il mio mago ha messo un altro Golem per bloccarli,\n\r  $c0015ed e' molto piu' potente di prima. Non temetelo, non vi attacchera'.'\n\r\n\r", 9020);
            }
            boris->commandp = boris->in_room;
        }
            break;

        case 9025:
        {
            if(boris->in_room != boris->commandp)
            {
                send_to_room("\n\r$c0015[$c0005Boris Ivanhoe Gudonov$c0015] dice:\n\r", 9025);
                send_to_room("$c0015 'Il custode di questi luoghi e' una creatura temibile, non sottovalutatelo!\n\r  $c0015Deve essere vicino, non e' necessario affrontarlo, vi suggerisco di aggirarlo.\n\r  $c0015Se deciderete di affrontarlo sappiate che non temo la morte, nessuna sfida e' troppo grande per Boris!'\n\r\n\r", 9025);
            }
            boris->commandp = boris->in_room;
        }
            break;

        case 9032:
        {
            if(boris->in_room != boris->commandp)
            {
                if(FindKeyByNumber(boris, SHADOW_WALLS_KEY))
                {
                    send_to_room("\n\r$c0015[$c0005Boris Ivanhoe Gudonov$c0015] dice:\n\r", 9032);
                    send_to_room("$c0015 'Fermi ora. Qui dobbiamo decidere quale strada intraprendere.\n\r  $c0015So per certo che ad est e' accampato l'intero esercito di Arkhat, il lezzo arriva fin qui perfino in questa dimensione.\n\r  $c0015Non so cosa aspettarmi a sud.\n\r  $c0015E' una decisione definitiva, il rituale aprira' solo uno dei due muri oscuri.'\n\r\n\r", 9032);
                }
                else
                {
                    send_to_room("\n\r$c0015[$c0005Boris Ivanhoe Gudonov$c0015] dice:\n\r", 9032);
                    send_to_room("$c0015 'Da qui non si passa. Dobbiamo trovare un altro modo di farlo, cerchiamo ancora nella vallata.'\n\r\n\r", 9032);
                }
            }
            boris->commandp = boris->in_room;
        }
            break;

        case 9034:
        {
            if(boris->in_room != boris->commandp)
            {
                send_to_room("\n\r$c0015[$c0005Boris Ivanhoe Gudonov$c0015] dice:\n\r", 9034);
                send_to_room("$c0015 'Bene, siamo tra le tende di comando. Forse riusciremo ad aggirare l'esercito.\n\r  $c0015I due scheletri a guardia non ci impensieriranno...'\n\r\n\r", 9034);
            }
            boris->commandp = boris->in_room;
        }
            break;

        case 9035:
        {
            if(boris->in_room != boris->commandp)
            {
                send_to_room("\n\r", 9035);
                do_say(boris, "Sembra proprio che i due Generali non amino condividere gli spazi.", 0);
                do_action(boris, NULL, CMD_CHUCKLE);
                do_say(boris, "Meglio per noi, li affronteremo uno alla volta.", 0);
                send_to_room("$c0013[$c0015Boris Ivanhoe Gudonov$c0013] sussurra:\n\r", 9035);
                send_to_room("$c0013 'Avverto un brivido lungo la schiena guardando verso sud, qualcosa mi dice che dobbiamo riuscire a passare.'\n\r\n\r", 9035);
            }
            boris->commandp = boris->in_room;
        }
            break;

        case 9037:
        {
            if(boris->in_room != boris->commandp)
            {
                send_to_room("\n\r$c0015[$c0005Boris Ivanhoe Gudonov$c0015] dice:\n\r", 9037);
                send_to_room("$c0015 'Da qui si vede la tenda di Athelstan.\n\r  $c0015E' uno dei due generali maggiori dell'esercito, e' potente ma per andare avanti dobbiamo sconfiggerlo.\n\r  $c0015Odio i suoi baffoni, sembra uscito da altri tempi, strappiamoglieli da quella faccia da carogna!'\n\r\n\r", 9037);
            }
            boris->commandp = boris->in_room;
        }
            break;

        case 9040:
        {
            if(boris->in_room != boris->commandp)
            {
                send_to_room("\n\r$c0015[$c0005Boris Ivanhoe Gudonov$c0015] dice:\n\r", 9040);
                send_to_room("$c0015 'Da qui si vede la tenda di Rexar.\n\r  $c0015La sua fama la precede, l'avrete sicuramente sentita nominare.\n\r  $c0015E' una degna avversaria, non commettete il mio stesso errore,\n\r  $c0015la scambiai anni fa per una donna che vendeva amore e fedelta' a pagamento... fui in errore.'\n\r\n\r", 9040);
            }
            boris->commandp = boris->in_room;
        }
            break;

        case 9043:
        {
            if(boris->in_room != boris->commandp)
            {
                send_to_room("\n\r$c0015[$c0005Boris Ivanhoe Gudonov$c0015] dice:\n\r", 9043);
                send_to_room("$c0015 'Dannazione, lo sapevo!\n\r  $c0015Da qualche parte qui intorno ci deve essere il laboratorio della mente deviata,\n\r  $c0015al servizio di Garebeth, che crea gli ibridi.\n\r", 9043);
                send_to_room("  $c0015Dobbiamo trovare quel bastardo!\n\r  $c0015Umag mi ha detto che ci sara' utile e non possiamo ucciderlo...\n\r  $c0015trattenetemi dal farlo per piacere!\n\r  $c0015Si chiama Uguik e so che ci devono essere anche le cavie che usa per i suoi esperimenti, cerchiamole!'\n\r\n\r", 9043);
            }
            boris->commandp = boris->in_room;
        }
            break;

        case 9046:
        {
            if(boris->in_room != boris->commandp)
            {
                send_to_room("\n\r$c0015[$c0005Boris Ivanhoe Gudonov$c0015] dice:\n\r", 9046);
                send_to_room("$c0015 'Quell'essere immondo e' il boia di Uguik! Muori maledetto!'\n\r\n\r", 9046);
            }
            boris->commandp = boris->in_room;
        }
            break;

        case 9047:
        case 9048:
        {
            if(boris->in_room != boris->commandp)
            {
                send_to_room("\n\r$c0008Ti sembra di sentire ringhi di rabbia misti a lacrime provenire da dietro alla maschera di Boris.\n\r\n\r", 9048);
            }
            boris->commandp = boris->in_room;
        }
            break;

        case 9051:
        {
            struct char_data* tch;
            struct char_data* uguik;
            char buf[256];

            uguik = 0;

            for(tch = real_roomp(boris->in_room)->people; (!uguik) && (tch); tch = tch->next_in_room)
            {
                if(IS_MOB(tch))
                {
                    if(mob_index[tch->nr].iVNum == UGUIK_AURUM)
                    {
                        uguik = tch;
                    }
                }
            }

            if(!uguik)
            {
              mudlog(LOG_CHECK, "Something goes wrong in Nilmys quest: Uguik is not on room 9051");
              send_to_room("\n\r$c0015[$c0005Boris Ivanhoe Gudonov$c0015] dice: 'Per Alar, dove ti sei nascosto gnomo?!?'\n\r\n\r", 9051);
              if((uguik = read_mobile(real_mobile(UGUIK_AURUM), REAL)))
              {
                char_to_room(uguik, 9051);
                if(boris->commandp2 == 5)
                {
                  uguik->commandp2 = 1;
                }
              }
              send_to_room("\n\r$c0008All'improvviso Uguik sbuca da dietro una cassa e, risponde:\n\r$c0011 'Ehm mi ero appisolato un secondo...'\n\r", 9051);
              return;
            }

            if(uguik->commandp2 != 1)
            {
                if(boris->in_room != boris->commandp)
                {
                    send_to_room("\n\r$c0015[$c0005Boris Ivanhoe Gudonov$c0015] dice:\n\r", 9051);
                    send_to_room("$c0015 'Ti ucciderei volentieri dopo averti torturato, ma Umag mi ha detto che solo tu\n\r  $c0015possiedi il sapere per attivare il portale.\n\r  $c0015Dicci quello che dobbiamo fare o giuro che il tuo dio Arkhat sembrera'\n\r  un chierichetto in confronto al trattamento che ti riservero'!'\n\r\n\r", 9051);
                }
                boris->commandp = boris->in_room;
                boris->commandp2 += 1;
                switch(boris->commandp2)
                {
                    case 0:
                        break;

                    case 2:
                        send_to_room("\n\r$c0008Uguik tremante, con le mani di Boris ancora attorno al collo, risponde:\n\r", 9051);
                        send_to_room("$c0011 'Per attivare il portale servono i resti degli Stanislav,\n\r  $c0011ci abbiamo provato ma a quanto pare c'era un altro Stanislav in giro nel piano materiale...\n\r  $c0011E' per questo che siamo bloccati qui!\n\r  $c0011Mi dispiace informarvi che avete fallito, non possiamo evocare Arkhat.\n\r  $c0011Sembra che il vostro piano sia naufragato!'\n\r\n\r", 9051);
                        do_action(uguik, NULL, CMD_GRIN);
                        break;

                    case 3:
                        send_to_room("\n\r$c0015[$c0009Boris Ivanhoe Gudonov$c0015] $c0009urla:\n\r $c0009'IDIOTA! Sappiamo noi dove si trova l'altro Stanislav! Ti porteremo il suo sangue, dacci lo strumento adatto.'\n\r\n\r", 9051);
                        break;

                    case 4:
                        sprintf(buf, "salassatore %s", GET_NAME((boris)->master));
                        if((boris)->master->in_room == uguik->in_room && CAN_SEE(uguik, (boris)->master))
                        {
                            do_give(uguik, buf, 0);
                        }
                        else
                        {
                            do_drop(uguik, "salassatore", 0);
                        }
                        sprintf(buf, "chiave %s", GET_NAME((boris)->master));
                        do_give(uguik, buf, 0);
                        if((boris)->master->in_room == uguik->in_room && CAN_SEE(uguik, (boris)->master))
                        {
                            do_give(uguik, buf, 0);
                        }
                        else
                        {
                            do_drop(uguik, "chiave", 0);
                        }
                        break;

                    case 6:
                        if(FindKeyByNumber(boris, NILMYS_BLEEDER) && FindKeyByNumber(boris, NILMYS_SHADOW_KEY))
                        {
                            send_to_room("\n\r$c0015[$c0005Boris Ivanhoe Gudonov$c0015] dice: 'Bene, ora resta qui! O ti verro' a cercare ovunque tu sia e ti faro' a pezzi lentamente!'\n\r\n\r", 9051);
                        }
                        else
                        {
                            boris->commandp2 = 5;
                        }
                        break;

                    case 7:
                        send_to_room("\n\r$c0015Boris si volta verso di te e dice:\n\r $c0015'Torniamo sui nostri passi per recuperare le ossa degli Stanislav ed il sangue di Iskra.\n\r  $c0015Uguik ci dira' poi come procedere.'\n\r\n\r", 9051);
                        uguik->commandp2 = 1;
                        break;

                    default:
                        break;
                }
            }
        }
            break;

        case 9052:
        {
            if(boris->in_room != boris->commandp)
            {
                send_to_room("\n\r$c0015[$c0005Boris Ivanhoe Gudonov$c0015] dice:\n\r", 9052);
                send_to_room("$c0015 'Fermatevi un attimo. Sappiate che siamo entrati nella dimensione ombra del piano materiale di Nilmys.\n\r  $c0015Questo significa che i nemici che troveremo sono proiezioni di quelli gia' uccisi in precedenza,\n\r  $c0015alcuni riescono a muoversi tra le dimensioni.\n\r  $c0015Il problema e' che qui sono molto, molto piu' potenti, quindi fate attenzione.'\n\r\n\r", 9052);
            }
            boris->commandp = boris->in_room;
        }
            break;

        case 9088:
        {
            if(boris->in_room != boris->commandp && boris->commandp2 > 14)
            {
                send_to_room("\n\r$c0015[$c0005Boris Ivanhoe Gudonov$c0015] dice:\n\r", 9088);
                send_to_room("$c0015 'Quanto detesto quello gnomo, se penso a quanta sofferenza ha inflitto...\n\r  $c0015Ora torno indietro e lo faccio a pezzi!'\n\r\n\r", 9088);
            }
            boris->commandp = boris->in_room;
        }
            break;

        case 9091:
        {
            if(boris->in_room != boris->commandp)
            {
                send_to_room("\n\r$c0015[$c0005Boris Ivanhoe Gudonov$c0015] dice:\n\r", 9091);
                send_to_room("$c0015 'Siamo molto cauti qui, dietro al Gonhag scorgo l'esercito di Garebeth,\n\r  $c0015forse se riuscissimo a liberare la bestia...'\n\r\n\r", 9091);
            }
            boris->commandp = boris->in_room;
        }
            break;

        case 9093:
        {
            if(boris->in_room != boris->commandp)
            {
                send_to_room("\n\r$c0015[$c0005Boris Ivanhoe Gudonov$c0015] dice:\n\r", 9093);
                send_to_room("$c0015 'Bene ora non ci resta che scalare la montagna, da qui non si torna indietro,\n\r  $c0015sembra che la dimensione d'ombra sia piu' debole.\n\r  $c0015E' l'ultima occasione per prepararci alla scalata.\n\r", 9093);
                send_to_room("  $c0015Possano gli Dei vegliare su di noi... e se non lo fanno... VADANO ALLA MALORA!'\n\r\n\r", 9093);
            }
            boris->commandp = boris->in_room;
        }
            break;

        case 9104:
        {
            if(boris->in_room != boris->commandp)
            {
                send_to_room("\n\r$c0015[$c0005Boris Ivanhoe Gudonov$c0015] dice:\n\r", 9008);
                send_to_room("$c0015 'Non e' finita a quanto pare. Dobbiamo scendere.\n\r  $c0015Se siamo arrivati fin qui non mi tirero' certo indietro ora.'\n\r\n\r", 9008);
            }
            boris->commandp = boris->in_room;
        }
            break;

        case 9114:
        {
            if(boris->in_room != boris->commandp)
            {
                send_to_room("\n\r$c0015[$c0005Boris Ivanhoe Gudonov$c0015] dice:\n\r", 9114);
                send_to_room("$c0015 'Troveremo una forte resistenza sul portale, sicuramente Garebeth in persona lo difendera'.\n\r  $c0015Restiamo calmi e poi potremo far uscire Arkhat l'immondo dal suo lungo sonno\n\r  $c0015ed impartirgli una sonora lezione!'\n\r\n\r", 9114);
            }
            boris->commandp = boris->in_room;
        }
            break;

        case 9115:
        {
            if(boris->commandp2 >= 1000 && boris->commandp2 < 2000)
            {
                ArkhatDeath(boris);
            }
            else if(boris->commandp2 >= 2000)
            {
                GarebethDeath(boris);
            }
        }
            break;

        case 9116:
        {
            if(boris->commandp2 >= 1000 && boris->commandp2 < 2000)
            {
                ArkhatDeath(boris);
            }
            else if(boris->commandp2 >= 2000)
            {
                GarebethDeath(boris);
            }
        }
            break;

        case 9117:
        {
            if(boris->commandp2 >= 1000 && boris->commandp2 < 2000)
            {
                ArkhatDeath(boris);
            }
            else if(boris->commandp2 >= 2000)
            {
                GarebethDeath(boris);
            }
        }
            break;

        case 9118:
        {
            if(boris->commandp2 >= 1000 && boris->commandp2 < 2000)
            {
                ArkhatDeath(boris);
            }
            else if(boris->commandp2 >= 2000)
            {
                GarebethDeath(boris);
            }
        }
            break;

        case 9119:
        {
            if(boris->commandp2 >= 1000 && boris->commandp2 < 2000)
            {
                ArkhatDeath(boris);
            }
            else if(boris->commandp2 >= 2000)
            {
                GarebethDeath(boris);
            }
            else if(boris->in_room != boris->commandp)
            {
                send_to_room("\n\r$c0015[$c0005Boris Ivanhoe Gudonov$c0015] dice:\n\r", 9119);
                send_to_room("$c0015 'Garebeth finalmente ci troviamo faccia a faccia! MUORI LURIDO VERME!'\n\r\n\r", 9119);
                boris->commandp = boris->in_room;
            }
        }
            break;

        case 9121:
        {
            if(boris->in_room != boris->commandp && boris->in_room == (boris)->master->in_room)
            {
                stop_follower(boris);

                send_to_zone("\n\r$c0015Boris vi abbraccia uno ad uno poi dice:\n\r", boris);
                send_to_zone("$c0015 'Grazie amici, non dimentichero' mai cio' che avete fatto per noi.\n\r  $c0015E' stato un onore per me combattere al vostro fianco.\n\r  $c0015Vi prego, accettate questo come segno della nostra riconoscenza.'\n\r\n\r", boris);

                // ed infine... il premio!
                CheckReward(boris);

                do_say(boris, "Se qualcuno di voi degno di ricevere il premio non ha ottenuto la ricompensa me la chieda!", 0);
                send_to_room("\n\r", 9121);
            }
        }
            break;

        default:
        {
            if(boris->in_room == boris->commandp)
            {
                if(number(0, 30) == 28)
                {
                    act("\n\r$c0015[$c0005$n$c0015] dice 'Cosa facciamo fermi qui? Andiamo!'\n\r", FALSE, boris, NULL, NULL, TO_ROOM);
                }
            }
            else
            {
                switch(number(0, 70))
                {
                    case 0:
                        send_to_room("\n\r", boris->in_room);
                        do_say(boris, "Quel dannato Garebeth la paghera'... ah se la paghera'!", 0);
                        send_to_room("\n\r", boris->in_room);
                        break;

                    case 2:
                        send_to_room("\n\r", boris->in_room);
                        do_say(boris, "Non indugiate troppo! Il destino di molta gente dipende da noi!", 0);
                        send_to_room("\n\r", boris->in_room);
                        break;

                    case 7:
                        send_to_room("\n\r", boris->in_room);
                        do_say(boris, "Valutiamo bene ogni scelta e non sottovalutiamo nessun nemico...", 0);
                        send_to_room("\n\r", boris->in_room);
                        break;

                    case 11:
                        send_to_room("\n\r", boris->in_room);
                        do_say(boris, "Arkhat probabilmente ci ridurra' in pezzi! Non temo il mio destino, non temiate il vostro.", 0);
                        send_to_room("\n\r", boris->in_room);
                        break;

                    case 15:
                        send_to_room("\n\r", boris->in_room);
                        do_say(boris, "Andiamo avanti, non esitiamo!", 0);
                        do_say(boris, "Dobbiamo farlo per la gente di Nilmys, dobbiamo farlo per Vlad e per i compagni caduti!", 0);
                        do_say(boris, "Non ci arrenderemo mai!", 0);
                        send_to_room("\n\r", boris->in_room);
                        break;

                    case 18:
                        send_to_room("\n\r", boris->in_room);
                        do_say(boris, "I miei occhi ormai si sono abituati a questo buio, temo solo di sbattere inavvertitamente con un mignolo ad uno spigolo.", 0);
                        send_to_room("\n\r", boris->in_room);
                        break;

                    case 22:
                        send_to_room("\n\r", boris->in_room);
                        do_say(boris, "Qualunque cosa accada, non temiate l'oscurita'...", 0);
                        do_say(boris, "Del resto siamo proprio dentro di essa...", 0);
                        do_say(boris, "Cosa potrebbe andare peggio?", 0);
                        send_to_room("\n\r", boris->in_room);
                        break;

                    case 26:
                        send_to_room("\n\r", boris->in_room);
                        do_say(boris, "Quando mettero' le mani su Gorath vedrete come lo ridurro'! Lo avete gia' seccato?! Mi dovete una birra allora, era mio!", 0);
                        send_to_room("\n\r", boris->in_room);
                        break;

                    case 30:
                        send_to_room("\n\r", boris->in_room);
                        do_say(boris, "Vi ho mai raccontato di quando Umag provo' a creare il suo primo golem?", 0);
                        do_say(boris, "Vi dico solo che uso' gli scarti alimentari di una taverna per farlo...", 0);
                        send_to_room("\n\r", boris->in_room);
                        break;

                    case 32:
                        send_to_room("\n\r", boris->in_room);
                        do_say(boris, "Sapete che ero solito avere incontri con donne che vendevano amore e fedelta' a pagamento?", 0);
                        do_say(boris, "Beh una di queste aveva un odore simile alla creatura appena incontrata!", 0);
                        send_to_room("\n\r", boris->in_room);
                        break;

                    case 41:
                        send_to_room("\n\r", boris->in_room);
                        do_say(boris, "Ci vogliono 3 monete, non una di piu' non una di meno, per ottenere finalmente il vostro premio!", 0);
                        send_to_room("\n\r", boris->in_room);
                        break;

                    case 65:
                        send_to_room("\n\r", boris->in_room);
                        do_say(boris, "Ricordatevi questa cosa: io ed i miei compagni possiamo scambiare le vostre monete con un premio favoloso!", 0);
                        send_to_room("\n\r", boris->in_room);
                        break;

                    default:
                        break;
                }

                if(boris->in_room != 9121)
                {
                    boris->commandp = boris->in_room;
                }
            }
        }
            break;
    }
}

bool FindKeyByNumber(struct char_data* ch, int number)
{
    struct obj_data* key;
    struct char_data* tch;
    int vnum;

    vnum = real_object(number);

    key = get_obj_vis(ch, obj_index[vnum].name);

    if(!key)
    {
        return FALSE;
    }

    if(key->carried_by != NULL)
    {
        for(tch = real_roomp(ch->in_room)->people; tch; tch = tch->next_in_room)
        {
            if(key->carried_by == tch && (is_same_group(ch, tch) || IS_DIO(tch)))
            {
                return TRUE;
            }
        }
    }
    return FALSE;
}

MOBSPECIAL_FUNC(Uguik_Aurum)
{
    struct obj_data* oggetto;
    struct char_data* uguik, *tch, *boris;
    int iVNum;
    char buf[512];

    const char* rand_Uguik_says[] = {
        "$c0015$n$c0015 dice 'Cosa ci faccio con questa spazzatura?'",
        "$c0015$n$c0015 dice '$p$c0015? Si mi piace, ma al momento non mi serve!'",
        "$c0015$n$c0015 dice 'E che cosa dovrei fare con $p$c0015?'",
        "$c0011$n$c0011 piagnucolando 'Basta, ti prego, basta. Smetti di portarmi questo ciarpame!'",
        "$c0015$n$c0015 dice '$p$c0015? Interessante... forse... ma anche no!'",
        "$c0014$n$c0014 esclama 'Non mi interessa minimamente!'",
        "$c0013$n$c0013 domanda a $N$c0013 'Mi hai scambiato per un accumulatore seriale?'",
        "$c0009$n$c0009, imprecando, 'Questa cosa qui vai a darla a tua sorella!'",
    };

    const int maxUguikSay = 7;

    uguik = 0;

    for(tch = real_roomp(ch->in_room)->people; (!uguik) && (tch); tch = tch->next_in_room)
    {
        if(IS_MOB(tch))
        {
            if(mob_index[tch->nr].iVNum == UGUIK_AURUM)
            {
                uguik = tch;
            }
        }
    }

    boris = 0;

    for(tch = real_roomp(ch->in_room)->people; (!boris) && (tch); tch = tch->next_in_room)
    {
        if(IS_MOB(tch))
        {
            if(mob_index[tch->nr].iVNum == BORIS_IVANHOE_CLONE)
            {
                boris = tch;
            }
        }
    }

    if(!boris)
    {
        return FALSE;
    }

    if(CheckUguikRoom(uguik, boris))
    {
        return TRUE;
    }

    if(uguik->generic == 4)
    {
        switch(number(0,30))
        {
            case 11:
                do_say(uguik, "Cosa volete nuovamente da me? Vi ho gia' detto tutto!", 0);
                do_say(uguik, "Andatevene!", 0);
                break;

            default:
                break;
        }
        return FALSE;
    }

    if(uguik->commandp2 == 1)
    {
        switch(number(0,15))
        {
            case 8:
                do_say(uguik, "Avete tutto quello che vi ho chiesto?", 0);
                if(uguik->commandp != 1)
                {
                    do_say(uguik, "Cosa aspettate ad andare?!?", 0);
                    uguik->commandp = 1;
                }
                break;

            default:
                break;
        }
    }
    else
    {
        return FALSE;
    }

    if(cmd && cmd != CMD_GIVE)
    {
        return(FALSE);
    }

    if(cmd == CMD_GIVE)
    {
        if(AWAKE(uguik))
        {
            if(!arg)
            {
                return FALSE;
            }

            arg = one_argument(arg, buf);

            if(!buf)
            {
                return FALSE;
            }

            oggetto = get_obj_in_list_vis(ch, buf, ch->carrying);

            if(!oggetto)
            {
                return FALSE;
            }

            iVNum = (oggetto->item_number >= 0) ? obj_index[oggetto->item_number].iVNum : 0;

            arg = one_argument(arg, buf);

            if(!buf)
            {
              return FALSE;
            }
        		else if((tch = get_char_room_vis(ch, buf)) != uguik)
            {
        			return FALSE;
            }

            act("Dai $p a $n.", FALSE, uguik, oggetto, ch, TO_VICT);
            act("$N ti da' $p.", FALSE, uguik, oggetto, ch, TO_CHAR);
            act("$N da' $p a $n.", FALSE, uguik, oggetto, ch, TO_NOTVICT);

            if(iVNum == STANISLAV_BONES)
            {
                extract_obj(oggetto);
                if(uguik->generic == 2)
                {
                    uguik->generic = 3;
                    do_say(uguik, "Ottimo, ora ho tutto!", 0);
                    do_action(uguik, NULL, CMD_GRIN);
                    return TRUE;
                }
                else
                {
                    uguik->generic = 1;
                    do_say(uguik, "Bene, molto bene.", 0);
                    do_emote(uguik, "si sfrega le mani.", 0);
                    do_say(uguik, "Dovete riportarmi il salassatore con il sangue!", 0);
                    return TRUE;
                }
            }
            else if(iVNum == NILMYS_BLEEDER)
            {
                if(oggetto->iGeneric != 1)
                {
                    do_say(uguik, "Il salassatore non contiene neppure una goccia di sangue!", 0);
                    do_say(uguik, "Tornate quando ne avrete recuperato un quantitativo sufficente.", 0);
                    act("Restituisci $p a $N.", FALSE, uguik, oggetto, ch, TO_CHAR);
                    act("$n ti restituisce $p.", FALSE, uguik, oggetto, ch, TO_VICT);
                    act("$n restituisce $p a $N.", FALSE, uguik, oggetto, ch, TO_NOTVICT);
                    return TRUE;
                }

                extract_obj(oggetto);

                if(uguik->generic == 1)
                {
                    uguik->generic = 3;
                    do_say(uguik, "Perfetto!", 0);
                    return TRUE;
                }
                else
                {
                    uguik->generic = 2;
                    do_say(uguik, "Bene, molto bene.", 0);
                    do_emote(uguik, "si sfrega le mani.", 0);
                    do_say(uguik, "Mancano ancora le ossa, fate presto!", 0);
                    return TRUE;
                }

            }
            else
            {
                act("Farfugli qualcosa di incomprensibile.", FALSE, uguik, NULL, NULL, TO_CHAR);
                sprintf(buf, "%s", rand_Uguik_says[number(0, maxUguikSay)]);
                act(buf, FALSE, uguik, oggetto, ch, TO_ROOM);
                act("Restituisci $p a $N.", FALSE, uguik, oggetto, ch, TO_CHAR);
                act("$n ti restituisce $p con disprezzo.", FALSE, uguik, oggetto, ch, TO_VICT);
                act("$n restituisce con disprezzo $p a $N.", FALSE, uguik, oggetto, ch, TO_NOTVICT);
                return TRUE;
            }
        }
        else
        {
            send_to_char("Non puoi farlo!\n\r", ch);
            return TRUE;
        }
    }

    return FALSE;
}

bool CheckUguikRoom(struct char_data* uguik, struct char_data* boris)
{
    struct obj_data* oggetto;
    char buf[512];

    if(uguik->generic == 3)
    {
        switch(uguik->commandp)
        {
            case 1:
                act("$c0008Dopo aver armeggiato con alambicchi, sostanze alchemiche e fuoco,\n\r$c0008porgi un'ampolla piena di un liquido traslucido a $N$c0008.", FALSE, uguik, NULL, boris->master, TO_CHAR);
                act("$c0008Dopo aver armeggiato con alambicchi, sostanze alchemiche e fuoco,\n\r$c0008Uguik Aurum ti porge un'ampolla piena di un liquido traslucido.", FALSE, uguik, NULL, boris->master, TO_VICT);
                act("$c0008Dopo aver armeggiato con alambicchi, sostanze alchemiche e fuoco,\n\r$c0008Uguik Aurum porge un'ampolla piena di un liquido traslucido a $N$c0008.", FALSE, uguik, NULL, boris->master, TO_NOTVICT);
                break;

            case 2:
                send_to_room("$c0008Uguik Aurum si gira verso di te poi dice:\n\r", uguik->in_room);
                send_to_room("$c0008 'In questa ampolla ci sono le cinque dosi che ti serviranno per cospargere il portale di Arkhat.\n\r  $c0008Ricordate di cospargere una dose su ogni punta ed una al centro altrimenti il rituale fallira'.'\n\r\n\r", uguik->in_room);
                break;

            case 3:
            {
                oggetto = get_obj_in_list_vis(uguik, obj_index[real_object(NILMYS_FLASK)].name, uguik->carrying);

                if(oggetto)
                {
                    if((boris)->master->in_room == uguik->in_room && CAN_SEE(uguik, (boris)->master))
                    {
                        sprintf(buf, "ampolla %s", GET_NAME((boris)->master));
                        do_give(uguik, buf, 0);
                    }
                    else
                    {
                        do_drop(uguik, "ampolla", 0);
                    }
                }
                else
                {
                    oggetto = read_object(real_object(NILMYS_FLASK), REAL);
                    obj_to_char(oggetto, uguik);

                    if((boris)->master->in_room == uguik->in_room && CAN_SEE(uguik, (boris)->master))
                    {
                        sprintf(buf, "ampolla %s", GET_NAME((boris)->master));
                        do_give(uguik, buf, 0);
                    }
                    else
                    {
                        do_drop(uguik, "ampolla", 0);
                    }
                }
            }
                break;

            case 4:
                send_to_room("$c0008Uguik Aurum riprende a parlare:\n\r", uguik->in_room);
                send_to_room("$c0008 'Scalate la montagna e raggiungerete il circolo rituale.\n\r  $c0008Ho fatto quello che mi avevate chiesto ora abbiate pieta' di me, non uccidetemi!'\n\r", uguik->in_room);
                break;

            case 5:
                send_to_room("\n\r\n\r$c0015Boris stringe il collo dello gnomo.\n\r\n\r", uguik->in_room);
                break;

            case 6:
                do_say(uguik, "$c0011Fermo! Vi daro' un ultimo aiuto! Ma non uccidermi!", 0);
                break;

            case 7:
                send_to_room("\n\r\n\r$c0015Boris allenta la presa.\n\r\n\r", uguik->in_room);
                break;

            case 8:
                do_say(uguik, "$c0011Uscite dal retro del mio laboratorio, da li' eviterete la maggior parte dell'esercito...", 0);
                do_say(uguik, "$c0011Nel mio forziere segreto c'e' la chiave per uscire dal retro, prendetela pure e seguite la strada.", 0);
                do_say(uguik, "Certo c'e' il Gonhag mutato, il mio capolavoro... ma sono sicuro che non vi creera' problemi.", 0);
                do_emote(uguik, "aggiunge con un ghigno beffardo.", 0);
                break;

            case 9:
            {
                oggetto = read_object(real_object(UGUIK_CHEST_KEY), REAL);
                obj_to_char(oggetto, uguik);
                do_unlock(uguik, "forziere", 0);
                do_open(uguik, "forziere", 0);
            }
                break;

            case 10:
                send_to_room("\n\r\n\r$c0008Boris scaraventa Uguik a terra e sputa al di la' della propria spalla.\n\r\n\r", uguik->in_room);
                break;

            case 11:
                send_to_room("$c0015[$c0005Boris Ivanhoe Gudonov$c0015] si gira verso di te, poi dice 'Andiamo!'\n\r", uguik->in_room);
                uguik->generic = 4;
                break;

        }

        uguik->commandp += 1;

        return TRUE;
    }
    return FALSE;
}

MOBSPECIAL_FUNC(Garebeth)
{
    struct char_data* tch, *boris;
    struct char_data* tar, *garebeth;
    int i;

    garebeth = 0;

    for(tch = real_roomp(ch->in_room)->people; (!garebeth) && (tch); tch = tch->next_in_room)
    {
        if(IS_MOB(tch))
        {
            if(mob_index[tch->nr].iVNum == GAREBETH)
            {
                garebeth = tch;
            }
        }
    }

    boris = 0;

    for(tch = real_roomp(ch->in_room)->people; (!boris) && (tch); tch = tch->next_in_room)
    {
        if(IS_MOB(tch))
        {
            if(mob_index[tch->nr].iVNum == BORIS_IVANHOE_CLONE)
            {
                boris = tch;
            }
        }
    }

    if(type == EVENT_DEATH)
    {
        if(boris)
        {
            boris->commandp2 = 2000;
            return (TRUE);
        }
        else
        {
            mudlog(LOG_PLAYERS, "Garebeth was killed by players but Boris is not alive, this is not possible.")
            return (TRUE);
        }
    }

    if(cmd || !AWAKE(garebeth))
    {
        return(FALSE);
    }

    if(StandUp(garebeth))
    {
        return(TRUE);
    }

    if((GET_POS(garebeth) > POSITION_STUNNED) && (GET_POS(garebeth) < POSITION_FIGHTING))
    {
        return(FALSE);
    }

    if((tar = garebeth->specials.fighting) && (garebeth->specials.fighting->in_room == garebeth->in_room))
    {
        if(HitOrMiss(ch, tar, CalcThaco(ch, NULL)))
        {
            act("Mordi $N e gli succhi forza vitale!", 1, garebeth, 0, tar, TO_CHAR);
            act("$n morde $N e gli succhia forza vitale!", 1, garebeth, 0, tar, TO_NOTVICT);
            act("$n ti morde e ti succhia forza vitale!", 1, garebeth, 0, tar, TO_VICT);

            /*toglie 1 hp per livello e se ne prende met�*/
            i = GetMaxLevel(garebeth);
            GET_HIT(tar) -= i;
            i = (i/2);
            GET_HIT(garebeth) += i;
            GET_HIT(garebeth) = MIN(GET_HIT(garebeth), GET_MAX_HIT(garebeth));
            alter_hit(garebeth, 0);
            alter_hit(tar, 0);
            return TRUE;
        }
        else {
            act("Provi a mordere $N!", 1, garebeth, 0, tar, TO_CHAR);
            act("$n prova a mordere $N!", 1, garebeth, 0, tar, TO_NOTVICT);
            act("$n prova a morderti!", 1, garebeth, 0, tar, TO_VICT);
            return TRUE;
        }
    }
    return FALSE;
}

void GarebethDeath(struct char_data* boris)
{
    struct obj_data* armor, *sword;
    struct follow_type* fol;
    bool ulrich = FALSE;

    switch(boris->commandp2)
    {
        case 2000:
        {
            send_to_room("\n\r", boris->in_room);
            do_say(boris, "Queste sono le vestigia del mio amato nipote Vladimir.", 0);
            act("\n\r$n ti mostra l'$c0015Armatura$c0007 $c0015Consacrata $c0007dalle vittime di $c0013Arkhat$c0007 e la $c0015Divina $c0007Spada di Vlad.", FALSE, boris, NULL, NULL, TO_ROOM);
        }
            break;

        case 2001:
        {
            send_to_room("\n\r", boris->in_room);
            do_say(boris, "Usate questa spada amici miei, ho giurato al mio carissimo nipote che la sua arma sarebbe vissuta in eterno...", 0);
            do_say(boris, "E che solo i piu' valorosi guerrieri mortali l'avrebbero brandita!", 0);

            sword = read_object(real_object(VLAD_SWORD), REAL);

            for(fol = boris->master->followers; fol; fol = fol->next)
            {
                if(boris->in_room == fol->follower->in_room)
                {
                    if(!strcmp(GET_NAME(fol->follower), "Ulrich"))
                    {
                        act("Stai per passare $p a $N ma poi ti fermi di colpo.", FALSE, boris, sword, (boris)->master, TO_CHAR);
                        act("$n ti per passare $p ma poi indugia.", FALSE, boris, sword, (boris)->master, TO_VICT);
                        act("$n sta per passare $p a $N ma poi si ferma di colpo.", FALSE, boris, sword, (boris)->master, TO_NOTVICT);

                        act("\n\rTi giri verso $N e dici 'Tu... ho come l'impressione di conoscerti da sempre.'\n\r", FALSE, boris, sword, (fol)->follower, TO_CHAR);
                        act("\n\r$n si gira verso di te e dice 'Tu... ho come l'impressione di conoscerti da sempre.'\n\r", FALSE, boris, sword, (fol)->follower, TO_VICT);
                        act("\n\r$n si gira verso $N e dice 'Tu... ho come l'impressione di conoscerti da sempre.'\n\r", FALSE, boris, sword, (fol)->follower, TO_NOTVICT);

                        act("Dai $p a $N.", FALSE, boris, sword, (fol)->follower, TO_CHAR);
                        act("$n ti da' $p.", FALSE, boris, sword, (fol)->follower, TO_VICT);
                        act("$n da' $p a $N.", FALSE, boris, sword, (fol)->follower, TO_NOTVICT);
                        obj_to_char(sword, fol->follower);
                        ulrich = TRUE;
                    }
                }
                send_to_room("\n\r", boris->in_room);
            }

            if(boris->in_room == (boris)->master->in_room && !ulrich)
            {
                act("Dai $p a $N.", FALSE, boris, sword, (boris)->master, TO_CHAR);
                act("$n ti da' $p.", FALSE, boris, sword, (boris)->master, TO_VICT);
                act("$n da' $p a $N.", FALSE, boris, sword, (boris)->master, TO_NOTVICT);
                obj_to_char(sword, boris->master);
            }
            else if(!ulrich)
            {
                do_say(boris, "Il piu' valoroso di voi estragga la spada!", 0);
                send_to_room("\n\r", boris->in_room);
                act("Conficchi $p nel terreno.", FALSE, boris, sword, NULL, TO_CHAR);
                act("$n conficca $p nel terreno.", FALSE, boris, sword, NULL, TO_ROOM);
                obj_to_room(sword, boris->in_room);
            }
        }
            break;

        case 2002:
        {
            send_to_room("\n\r", boris->in_room);
            do_say(boris, "Usate la sua armatura e datemi l'opportunita' di combattere ancora una volta al suo fianco.", 0);
            do_say(boris, "Portatela con rispetto ed usatela con onore!", 0);
            send_to_room("\n\r", boris->in_room);

            armor = read_object(real_object(VLAD_ARMOR), REAL);

            for(fol = boris->master->followers; fol; fol = fol->next)
            {
                if(boris->in_room == fol->follower->in_room)
                {
                    if(!strcmp(GET_NAME(fol->follower), "Ulrich"))
                    {
                        act("Dai $p a $N.", FALSE, boris, armor, (fol)->follower, TO_CHAR);
                        act("$n ti da' $p.", FALSE, boris, armor, (fol)->follower, TO_VICT);
                        act("$n da' $p a $N.", FALSE, boris, armor, (fol)->follower, TO_NOTVICT);
                        obj_to_char(armor, fol->follower);
                        ulrich = TRUE;
                    }
                }
            }

            if(boris->in_room == (boris)->master->in_room && !ulrich)
            {
                act("Dai $p a $N.", FALSE, boris, armor, (boris)->master, TO_CHAR);
                act("$n ti da' $p.", FALSE, boris, armor, (boris)->master, TO_VICT);
                act("$n da' $p a $N.", FALSE, boris, armor, (boris)->master, TO_NOTVICT);
                obj_to_char(armor, boris->master);
            }
            else if(!ulrich)
            {
                do_say(boris, "Chi vuole sfidare $c0013Arkhat$c0007 indossi l'armatura!", 0);
                send_to_room("\n\r", boris->in_room);
                act("Adagi $p a terra.", FALSE, boris, armor, NULL, TO_CHAR);
                act("$n adagia $p a terra.", FALSE, boris, armor, NULL, TO_ROOM);
                obj_to_room(armor, boris->in_room);
            }
        }
            break;

        case 2003:
            send_to_room("\n\r", boris->in_room);
            do_say(boris, "Vi ringrazio a tutti, comunque andra' a finire!", 0);
            do_say(boris, "Ora andiamo e non temete l'oscurita'!", 0);
            break;

        default:
            break;
    }

    boris->commandp2 += 1;
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
            if( s1 + s2 + s3 +s4 == 4 && obj->iGeneric == 0)
            {
                numero = room_of_object(obj);
                send_to_room(" \n\r", numero);
                send_to_room("$c0008Un vento gelido entra nella cripta non appena seppellisci le $c0015ossa$c0008...\n\r", numero);
                send_to_room("$c0008Improvvisamente l'urna si illumina di $c0009rosso$c0008 poi, subito dopo, la luce scompare inghiottita dalle tenebre...\n\r", numero);
                send_to_room("$c0008Poi un rumore sordo rimbomba nel sottosuolo, giureresti che provenga dalla chiesa.\n\r", numero);
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

ROOMSPECIAL_FUNC(gonhag_block)
{
    if(type != EVENT_COMMAND)
    {
        return FALSE;
    }

    if(cmd == CMD_PUT)
    {
        int iVNum;
        char buf[128];
        struct obj_data* tool, *key;

        arg = one_argument(arg, buf);

        if(!strcmp("strumento", buf))
        {
            tool = get_obj_in_list_vis(ch, buf, ch->carrying);

            if(tool)
            {
                iVNum = (tool->item_number >= 0) ? obj_index[tool->item_number].iVNum : 0;
                if(iVNum == GONHAG_TOOL)
                {
                    arg = one_argument(arg, buf);

                    if(!strcmp("meccanismo", buf))
                    {
                        act("Quando metti questo pezzo nel meccanismo noti subito che la manovella puo' adesso ruotare liberamente...", FALSE, ch, 0, 0, TO_CHAR);
                        WAIT_STATE(ch, PULSE_VIOLENCE * 2);
                        act("Vedi $n armeggiare con il pezzo meccanico e sembra che stia riuscendo a farlo collimare nel meccanismo.", FALSE, ch, 0, 0, TO_ROOM);
                        act("Infine... ti giri con aria soddisfatta ed il pollice in su!", FALSE, ch, 0, 0, TO_CHAR);
                        act("Infine... si gira con aria soddisfatta ed il pollice in su!", FALSE, ch, 0, 0, TO_ROOM);
                        extract_obj(tool);

                        key = get_obj(obj_index[real_object(GONHAG_KEY)].name);
                        if(key)
                        {
                            //  se la key esiste gli assegno il valore di 1 a iGeneric
                            key->iGeneric = 1;
                            mudlog(LOG_PLAYERS, "key generic = %d", key->iGeneric);
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

ROOMSPECIAL_FUNC(gonhag_chain)
{
    if(type != EVENT_COMMAND)
    {
        return FALSE;
    }

    if(cmd == CMD_UNLOCK)
    {
        char buf[128];
        struct obj_data* key;
        struct char_data* gonhag, *tch;

        gonhag = 0;

        for(tch = real_roomp(ch->in_room)->people; (!gonhag) && (tch); tch = tch->next_in_room)
        {
            if(IS_MOB(tch))
            {
                if(mob_index[tch->nr].iVNum == GONHAG)
                {
                    gonhag = tch;
                }
            }
        }

        arg = one_argument(arg, buf);

        if(!strcmp("collare", buf))
        {
            key = get_obj_in_list_vis(ch, obj_index[real_object(GONHAG_KEY)].name, ch->carrying);

            if(key)
            {
                //  controllo se il meccanismo e' stato attivato, se non e' attivo il Gonhag attacca
                if(key->iGeneric != 1)
                {
                    act("Silenziosamente ti avvicini alla creatura ma ahime' non riesci ad inserire la chiave.", FALSE, ch, 0, gonhag, TO_CHAR);
                    act("$N ti attacca!", FALSE, ch, 0, gonhag, TO_CHAR);
                    act("Osservi attentamente $n avvicinarsi alla tremenda bestia, proprio quando e' a un metro da lui... inciampa!", FALSE, ch, 0, gonhag, TO_NOTVICT);
                    act("Il $N si solleva guardandolo furioso... buona fortuna $n!", FALSE, ch, 0, gonhag, TO_NOTVICT);
                    act("Osservi $n avvicinarsi con fare sospetto, ne sei indispettit$B e l$b attacchi!", FALSE, ch, 0, gonhag, TO_VICT);
                    do_hit(gonhag, GET_NAME(ch), 0);
                    return TRUE;
                }
                else
                {
                    act("Silenziosamente ti avvicini alla creatura, inserisci la chiave e... *CLICK*!", FALSE, ch, 0, gonhag, TO_CHAR);
                    act("L'enorme collare si apre e il $N sembra non essersi accorto di te!", FALSE, ch, 0, gonhag, TO_CHAR);
                    act("Osservi $n avvicinarsi silenziosamente alla creatura colossale... trattieni il fiato... *CLICK*!", FALSE, ch, 0, gonhag, TO_NOTVICT);
                    act("L'enorme collare si apre e il $N sembra non essersi accorto di $n!", FALSE, ch, 0, gonhag, TO_NOTVICT);
                    act("*CLICK*\n\rSei liber$B!", FALSE, ch, 0, gonhag, TO_VICT);

                    if(IS_SET(gonhag->specials.act, ACT_SENTINEL))
                    {
                        REMOVE_BIT(gonhag->specials.act, ACT_SENTINEL);
                    }

                    if(gonhag->specials.fighting)
                    {
                        stop_fighting(gonhag);
                    }
                    WAIT_STATE(gonhag, PULSE_VIOLENCE*5);
                    send_to_zone("\n\r\n\r$c0008Un fragoroso ruggito riecheggia nel silenzio della dimensione ombra...\n\r$c0008Il Gonhag e' libero!\n\r\n\r", gonhag);

                    return TRUE;
                }
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
    return FALSE;
}

ROOMSPECIAL_FUNC(reward_giver)
{
    struct char_data *shopper, *tch;
    struct obj_data* coin, *reward, *obj_to_sell;
    char buf[256], sell[100];
    int i, j, count = 0, premio, rnum;
    bool Ok = FALSE;

    if(type != EVENT_COMMAND)
    {
        return FALSE;
    }

    shopper = 0;

    for(tch = real_roomp(ch->in_room)->people; (!shopper) && (tch); tch = tch->next_in_room)
    {
        if(IS_MOB(tch))
        {
            if(mob_index[tch->nr].iVNum >= BORIS_IVANHOE && mob_index[tch->nr].iVNum <= CORMAC_RUNAR)
            {
                shopper = tch;
            }
        }
    }

    if(!shopper)
    {
        return FALSE;
    }

    if(!IS_PC(ch))
    {
        return FALSE;
    }

    if(!AWAKE(shopper))
    {
        return FALSE;
    }

    if(cmd == CMD_SELL)
    {
        sell[0]='\0';

        only_argument(arg, sell);

        if(!(*sell))
        {
            if(CAN_SEE(shopper, ch))
            {
                sprintf(buf, "%s Cosa vorresti vendermi?", GET_NAME(ch));
            }
            else
            {
                sprintf(buf, "Non compro niente da chi non posso vedere!");
            }
            do_tell(shopper, buf, 0);
            return TRUE;
        }

        if(!(obj_to_sell = get_obj_in_list_vis(ch, sell, ch->carrying)))
        {
            if(CAN_SEE(shopper, ch))
            {
                sprintf(buf, "%s Non hai niente del genere con te!", GET_NAME(ch));
            }
            else
            {
                sprintf(buf, "Non compro niente da chi non posso vedere!");
            }
            do_tell(shopper, buf, 0);
            return TRUE;
        }

        if(IS_OBJ_STAT(obj_to_sell, ITEM_NODROP) && !IS_IMMORTAL(ch))
        {
            send_to_char("Non puoi lasciarlo, deve essere MALEDETTO!\n\r", ch);
            return TRUE;
        }

        rnum = (obj_to_sell->item_number >= 0) ? obj_index[obj_to_sell->item_number].iVNum : 0;

        switch(rnum)
        {
            case 9019:
            case 9018:
            case 9020:
            case 9021:
            case 9098:
            case 9099:
            {
                if(mob_index[shopper->nr].iVNum == BORIS_IVANHOE)
                {
                    act("\n\r$c0013[$c0015$n$c0013] ti dice 'Ok, ti posso dare una moneta di Nilmys in cambio di $p$c0013!'\n\r", FALSE, shopper, obj_to_sell, ch, TO_VICT);
                    Ok = TRUE;
                }
                else
                {
                    act("\n\r$c0013[$c0015$n$c0013] ti dice 'Mi dispiace, non mi interessa! Prova da Boris!'\n\r", FALSE, shopper, obj_to_sell, ch, TO_VICT);
                }
            }
                break;

            case 9005:
            case 9006:
            case 9007:
            case 9009:
            case 9097:
            case 9087:
            {
                if(mob_index[shopper->nr].iVNum == TAMARANG_PRINCE)
                {
                    act("\n\r$c0013[$c0015$n$c0013] ti dice 'Ok, ma posso darti solo una moneta di Nilmys in cambio di $p$c0013!'\n\r", FALSE, shopper, obj_to_sell, ch, TO_VICT);
                    Ok = TRUE;
                }
                else
                {
                    act("\n\r$c0013[$c0015$n$c0013] ti dice 'Io non prendo quella cosa li'! Prova da Tamarang!'\n\r", FALSE, shopper, obj_to_sell, ch, TO_VICT);
                }
            }
                break;

            case 9015:
            case 9016:
            case 9017:
            case 9014:
            case 9095:
            case 9094:
            {
                if(mob_index[shopper->nr].iVNum == UMAG_ULBAR)
                {
                    act("\n\r$c0013[$c0015$n$c0013] ti dice 'Mi interessa $p$c0013: ecco un Augustale per te!'\n\r", FALSE, shopper, obj_to_sell, ch, TO_VICT);
                    Ok = TRUE;
                }
                else
                {
                    act("\n\r$c0013[$c0015$n$c0013] ti dice 'Non mi interessa $p$c0013! Forse Umag potrebbe essere interessato.'\n\r", FALSE, shopper, obj_to_sell, ch, TO_VICT);
                }
            }
                break;

            case 9010:
            case 9011:
            case 9012:
            case 9013:
            case 9090:
            case 9091:
            {
                if(mob_index[shopper->nr].iVNum == DAGGAR_IVRAM)
                {
                    act("\n\r$c0013[$c0015$n$c0013] ti dice 'Per $p$c0013 posso darti ben una moneta!'\n\r", FALSE, shopper, obj_to_sell, ch, TO_VICT);
                    Ok = TRUE;
                }
                else
                {
                    act("\n\r$c0013[$c0015$n$c0013] ti dice '$p$c0013? E cosa ci faccio? Chiedi a Daggar, magari a lui interessa.'\n\r", FALSE, shopper, obj_to_sell, ch, TO_VICT);
                }
            }
                break;

            case 9029:
            case 9031:
            case 9030:
            case 9028:
            case 9092:
            case 9093:
            {
                if(mob_index[shopper->nr].iVNum == IREIIN_DRUID)
                {
                    act("\n\r$c0013[$c0015$n$c0013] ti dice 'Si, $p$c0013 mi interessa. Ti do un Augustale!'\n\r", FALSE, shopper, obj_to_sell, ch, TO_VICT);
                    Ok = TRUE;
                }
                else
                {
                    act("\n\r$c0013[$c0015$n$c0013] ti dice 'Prova con Ireiin, io non tratto questa robaccia!'\n\r", FALSE, shopper, obj_to_sell, ch, TO_VICT);
                }
            }
                break;

            case 9023:
            case 9025:
            case 9024:
            case 9026:
            case 9088:
            case 9089:
            {
                if(mob_index[shopper->nr].iVNum == CORMAC_RUNAR)
                {
                    act("\n\r$c0013[$c0015$n$c0013] ti dice 'Per $p$c0013 ti posso dare... una moneta di Nilmys!'\n\r", FALSE, shopper, obj_to_sell, ch, TO_VICT);
                    Ok = TRUE;
                }
                else
                {
                    act("\n\r$c0013[$c0015$n$c0013] ti dice 'Non ci faccio niente ma sicuramente puo' interessare a Cormac!'\n\r", FALSE, shopper, obj_to_sell, ch, TO_VICT);
                }
            }
                break;

            default:
                act("\n\r$c0013[$c0015$n$c0013] ti dice 'Cosa ci faccio con $p$c0013?'\n\r", FALSE, shopper, obj_to_sell, ch, TO_VICT);
                break;
        }

        if(Ok)
        {
            act("\n\r$c0013Tu dici a $N$c0013 'Ok, ti posso dare una moneta di Nilmys per $p$c0013.'\n\r",FALSE, shopper, obj_to_sell, ch, TO_CHAR);
            act("\n\r$c0013$n$c0013 dice qualcosa a $N$c0013.\n\r",FALSE, shopper, NULL, ch, TO_NOTVICT);

            act("$c0015$N$c0015 ti da' $p$c0015.", FALSE, shopper, obj_to_sell, ch, TO_CHAR);
            act("$c0015Dai $p$c0015 a $n$c0015.", FALSE, shopper, obj_to_sell, ch, TO_VICT);
            act("$c0015$N$c0015 da' $p$c0015 a $n$c0015.", FALSE, shopper, obj_to_sell, ch, TO_NOTVICT);

            extract_obj(obj_to_sell);

            coin = read_object(real_object(NILMYS_COIN), REAL);
            obj_to_char(coin, ch);

            sprintf(buf, "un Augustale di proprieta' di %s", GET_NAME(ch));
            free(coin->short_description);
            coin->short_description = (char*)strdup(buf);
            SetPersonOnSave(ch, coin);

            act("$c0011Consegni $p$c0011 a $N$c0011.", FALSE, shopper, coin, ch, TO_CHAR);
            act("$c0011$n$c0011 ti consegna $p$c0011.", FALSE, shopper, coin, ch, TO_VICT);
            act("$c0011$n$c0011 consegna $p$c0011 a $N$c0011.", FALSE, shopper, coin, ch, TO_NOTVICT);
        }
        else
        {
            act("\n\r$c0013Scuoti la testa e fai intendere a $N$c0013 che non sei interessat$b a $p$c0013.\n\r",FALSE, shopper, obj_to_sell, ch, TO_CHAR);
            act("\n\r$c0013$n$c0013 scuote la testa e dice qualcosa a $N$c0013.\n\r",FALSE, shopper, NULL, ch, TO_NOTVICT);
        }

        return TRUE;
    }
    else if(cmd == CMD_BUY)
    {
        for(i = 0; i < 3; i++)
        {
            coin = get_obj_in_list_vis(ch, obj_index[real_object(NILMYS_COIN)].name, ch->carrying);

            if(coin)
            {
                if((IS_OBJ_STAT2(coin, ITEM2_PERSONAL) && !pers_on(ch, coin)) || !IS_OBJ_STAT2(coin, ITEM2_PERSONAL))
                {
                    act("Mi dispiace ma $p non ti appartiene!", FALSE, ch, coin, NULL, TO_CHAR);

                    //  restituisco le monete prese dato che non sono personal sul toon
                    if(count > 0)
                    {
                        for(j = 0; j < count; j++)
                        {
                            coin = read_object(real_object(NILMYS_COIN), REAL);
                            obj_to_char(coin, ch);

                            sprintf(buf, "un Augustale di proprieta' di %s", GET_NAME(ch));
                            free(coin->short_description);
                            coin->short_description = (char*)strdup(buf);
                            SetPersonOnSave(ch, coin);
                        }
                    }
                    return TRUE;
                }

                count += 1;
                extract_obj(coin);
            }
            else
            {
                act("Mi dispiace ma non hai abbastanza monete con te!", FALSE, ch, NULL, NULL, TO_CHAR);
                //  ha meno di 3 monete, le restituisco al proprietario
                if(count > 0)
                {
                    for(j = 0; j < count; j++)
                    {
                        coin = read_object(real_object(NILMYS_COIN), REAL);
                        obj_to_char(coin, ch);

                        sprintf(buf, "un Augustale di proprieta' di %s", GET_NAME(ch));
                        free(coin->short_description);
                        coin->short_description = (char*)strdup(buf);
                        SetPersonOnSave(ch, coin);
                    }
                }

                return TRUE;
            }
        }
    }
    else
    {
        return FALSE;
    }

    if(count == 3)
    {
        premio = number(1, 100);
        switch(mob_index[shopper->nr].iVNum)
        {
            case BORIS_IVANHOE:
            {
                if(premio <= 7)
                {
                    rnum = 9019;
                }
                else if(premio > 7 && premio <= 18)
                {
                    rnum = 9018;
                }
                else if(premio > 18 && premio <= 29)
                {
                    rnum = 9020;
                }
                else if(premio > 29 && premio <= 59)
                {
                    rnum = 9021;
                }
                else if(premio > 59 && premio <= 70)
                {
                    rnum = 9098;
                }
                else if(premio > 70)
                {
                    rnum = 9099;
                }
            }
                break;

            case TAMARANG_PRINCE:
            {
                if(premio <= 7)
                {
                    rnum = 9005;
                }
                else if(premio > 7 && premio <= 18)
                {
                    rnum = 9006;
                }
                else if(premio > 18 && premio <= 29)
                {
                    rnum = 9007;
                }
                else if(premio > 29 && premio <= 59)
                {
                    rnum = 9009;
                }
                else if(premio > 59 && premio <= 70)
                {
                    rnum = 9097;
                }
                else if(premio > 70)
                {
                    rnum = 9087;
                }
            }
                break;

            case UMAG_ULBAR:
            {
                if(premio <= 7)
                {
                    rnum = 9015;
                }
                else if(premio > 7 && premio <= 18)
                {
                    rnum = 9016;
                }
                else if(premio > 18 && premio <= 29)
                {
                    rnum = 9017;
                }
                else if(premio > 29 && premio <= 59)
                {
                    rnum = 9014;
                }
                else if(premio > 59 && premio <= 89)
                {
                    rnum = 9095;
                }
                else if(premio > 89)
                {
                    rnum = 9094;
                }
            }
                break;

            case DAGGAR_IVRAM:
            {
                if(premio <= 7)
                {
                    rnum = 9010;
                }
                else if(premio > 7 && premio <= 18)
                {
                    rnum = 9011;
                }
                else if(premio > 18 && premio <= 29)
                {
                    rnum = 9012;
                }
                else if(premio > 29 && premio <= 59)
                {
                    rnum = 9013;
                }
                else if(premio > 59 && premio <= 89)
                {
                    rnum = 9090;
                }
                else if(premio > 89)
                {
                    rnum = 9091;
                }
            }
                break;

            case IREIIN_DRUID:
            {
                if(premio <= 7)
                {
                    rnum = 9029;
                }
                else if(premio > 7 && premio <= 37)
                {
                    rnum = 9031;
                }
                else if(premio > 37 && premio <= 48)
                {
                    rnum = 9030;
                }
                else if(premio > 48 && premio <= 78)
                {
                    rnum = 9028;
                }
                else if(premio > 78 && premio <= 89)
                {
                    rnum = 9092;
                }
                else if(premio > 89)
                {
                    rnum = 9093;
                }
            }
                break;

            case CORMAC_RUNAR:
            {
                if(premio <= 7)
                {
                    rnum = 9023;
                }
                else if(premio > 7 && premio <= 18)
                {
                    rnum = 9025;
                }
                else if(premio > 18 && premio <= 48)
                {
                    rnum = 9024;
                }
                else if(premio > 48 && premio <= 78)
                {
                    rnum = 9026;
                }
                else if(premio > 78 && premio <= 89)
                {
                    rnum = 9088;
                }
                else if(premio > 89)
                {
                    rnum = 9089;
                }
            }
                break;

            default:
                break;
        }

        //  consegna premio al player
        reward = read_object(real_object(rnum), REAL);
        SetPersonOnSave(ch, reward);

        act("\n\r$c0015$N$c0015 ti da' tre monete di Nilmys.", FALSE, shopper, NULL, ch, TO_CHAR);
        act("\n\r$c0015Dai tre monete di Nilmys a $n$c0015.", FALSE, shopper, NULL, ch, TO_VICT);
        act("\n\r$c0015$N$c0015 da' alcune monete di Nilmys a $n$c0015.", FALSE, shopper, NULL, ch, TO_NOTVICT);

        act("\n\r$c0011Consegni $p$c0011 a $N$c0011.", FALSE, shopper, reward, ch, TO_CHAR);
        act("\n\r$c0011$n$c0011 ti consegna $p$c0011.", FALSE, shopper, reward, ch, TO_VICT);
        act("\n\r$c0011$n$c0011 consegna $p$c0011 a $N$c0011.", FALSE, shopper, reward, ch, TO_NOTVICT);
        obj_to_char(reward, ch);
        return TRUE;
    }

    return FALSE;
}

} // namespace Alarmud
