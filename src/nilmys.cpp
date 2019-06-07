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

MOBSPECIAL_FUNC(Arkhat)
{
    struct char_data* tch;
    struct char_data* arkhat, *boris;
    struct obj_data* co, *o, *corpse;
    struct char_data* targ;
    struct room_data* rp;
    char buf[1024];

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
            if(mob_index[tch->nr].iVNum == BORIS_IVANHOE)
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

    if(AWAKE(arkhat))
    {
        if((targ = FindAnAttacker(arkhat)) != NULL)
        {
            act("$c0009Spalanchi la sua enorme bocca ed attacchi!", FALSE, arkhat, 0, 0, TO_CHAR);
            act("$c0009$n $c0009spalanca la sua enorme bocca ed attacca!", FALSE, arkhat, 0, 0, TO_ROOM);
            if(!CAN_SEE(ch, targ))
            {
                if(saves_spell(targ, SAVING_PARA))
                {
                    act("$c0014$N $c0014evita con un balzo fulmineo il tuo morso!", FALSE, arkhat, 0, targ, TO_CHAR);
                    act("$c0014$N $c0014evita con un balzo fulmineo il morso di $n$c0014!", FALSE, arkhat, 0, targ, TO_NOTVICT);
                    switch(number(0, 1))
                    {
                        case 0:
                            act("$c0014Ti sposti rapidamente alla tua sinistra ed eviti le fauci di $n$c0014!", FALSE, arkhat, 0, targ, TO_VICT);
                            break;

                        default:
                            act("$c0014Ti sposti rapidamente alla tua destra ed eviti le fauci di $n$c0014!", FALSE, arkhat, 0, targ, TO_VICT);
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

                GET_HIT(targ) = 0;
                alter_hit(targ, 0);
                mudlog(LOG_PLAYERS, "%s killed by %s (being swallowed whole)", GET_NAME(targ), GET_NAME(arkhat));
                die(targ, 0, NULL);
                /*
                 all stuff to monster:  this one is tricky.  assume that corpse is
                 top item on item_list now that corpse has been made.
                 */
                rp = real_roomp(ch->in_room);
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

                        sprintf(buf, "resti %s", GET_NAME(targ));
                        free(corpse->name);
                        corpse->name = (char*)strdup(buf);
                        sprintf(buf, "i resti martoriati di $c0015%s$c0007", GET_NAME(targ));
                        free(corpse->short_description);
                        corpse->short_description = (char*)strdup(buf);
                        sprintf(buf, "Tutto quello che rimane dell'equipaggiamento di $c0015%s$c0007 e' sparso qui a terra.", GET_NAME(targ));
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
                        act("$c0014$N$c0014 abbassa velocemente la testa ed evita le tue fauci!", FALSE, arkhat, 0, targ, TO_CHAR);
                        act("$c0014Abbassi velocemente la testa ed eviti le fauci di $n$c0014!", FALSE, arkhat, 0, targ, TO_VICT);
                        act("$c0014$N$c0014 abbassa velocemente la testa ed evita le fauci di $n$c0014!", FALSE, arkhat, 0, targ, TO_NOTVICT);
                    break;

                    case 1:
                        act("$c0014$N$c0014 indietreggia ed evita le tue fauci!", FALSE, arkhat, 0, targ, TO_CHAR);
                        act("$c0014Eviti le fauci di $n$c0014 facendo due passi indietro!", FALSE, arkhat, 0, targ, TO_VICT);
                        act("$c0014$N$c0014 indietreggia ed evita le fauci di $n$c0014!", FALSE, arkhat, 0, targ, TO_NOTVICT);
                    break;

                    case 2:
                        act("$c0014$N$c0014 si sposta alla sua sinistra ed evita le tue fauci!", FALSE, arkhat, 0, targ, TO_CHAR);
                        act("$c0014Ti sposti rapidamente a sinistra ed eviti le fauci di $n$c0014!", FALSE, arkhat, 0, targ, TO_VICT);
                        act("$c0014$N$c0014 si sposta alla sua sinistra ed evita le fauci di $n$c0014!", FALSE, arkhat, 0, targ, TO_NOTVICT);
                    break;

                    case 3:
                        act("$c0014$N$c0014 si sposta alla sua destra ed evita le tue fauci!", FALSE, arkhat, 0, targ, TO_CHAR);
                        act("$c0014Ti sposti rapidamente a destra ed eviti le fauci di $n$c0014!", FALSE, arkhat, 0, targ, TO_VICT);
                        act("$c0014$N$c0014 si sposta alla sua destra ed evita le fauci di $n$c0014!", FALSE, arkhat, 0, targ, TO_NOTVICT);
                    break;

                    default:
                    {
                        if(targ->equipment[WEAR_SHIELD])
                        {
                            act("$c0014$N$c0014 blocca con lo scudo le tue fauci!", FALSE, arkhat, 0, targ, TO_CHAR);
                            act("$c0014Blocchi con lo scudo le fauci di $n$c0014!", FALSE, arkhat, 0, targ, TO_VICT);
                            act("$c0014$N$c0014 blocca con lo scudo le fauci di $n$c0014!", FALSE, arkhat, 0, targ, TO_NOTVICT);
                        }
                        else
                        {
                            act("$c0014$N$c0014 abbassa velocemente la testa ed evita le tue fauci!", FALSE, arkhat, 0, targ, TO_CHAR);
                            act("$c0014Abbassi velocemente la testa ed eviti le fauci di $n$c0014!", FALSE, arkhat, 0, targ, TO_VICT);
                            act("$c0014$N$c0014 abbassa velocemente la testa ed evita le fauci di $n$c0014!", FALSE, arkhat, 0, targ, TO_NOTVICT);
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
            send_to_room("\n\r$c0014con una fragorosa esplosione di $c0015luce $c0013Arkhat$c0014, il divoratore, di dissolve come cancellato\n\r", boris->in_room);
            send_to_room("\n\r$c0014dall'esistenza e dalla memoria.\n\r", boris->in_room);
            break;

        case 1002:
            send_to_room("\n\r$c0014Le $c0015anime$c0014 che lo circondavano e si tormentavano in una macabra danza attorno al suo corpo,\n\r", boris->in_room);
            send_to_room("\n\r$c0014iniziano ad emettere $c0015luce$c0014 ed a convergere verso un punto in alto poco sopra la tua testa.\n\r", boris->in_room);
            send_to_room("\n\r$c0014Noti una di esse che indugia sulla carcassa materiale rimasta del $c0013Dio$c0014 sconfitto.\n\r", boris->in_room);
            break;

        case 1003:
            send_to_room("\n\r$c0014E' lei la prima anima presa da $c0013Arkhat$c0014, e' lei che lentamente sale fino al punto in cui tutte\n\r", boris->in_room);
            send_to_room("\n\r$c0014le altre sono radunate.\n\r", boris->in_room);
            break;

        case 1004:
            send_to_room("\n\r$c0014Un varco si apre e tutte le $c0015anime$c0014 entrano rapidamente disperdendosi verso una grande $c0015luce$c0014.\n\r", boris->in_room);
            portale = read_object(real_object(VICTORY_PORTAL), REAL);
            obj_to_room(portale, boris->in_room);
            break;

        case 1005:
            send_to_room("\n\r$c0014Boris esclama '$c0011VITTORIA!$c0014', e sale verso la $c0015luce$c0014.\n\r", boris->in_room);
            do_enter(boris, "portale", 0);
            break;

        default:
            break;
    }
    boris->commandp2 += 1;
}

MOBSPECIAL_FUNC(Boris_Ivanhoe)
{
    struct char_data* tch;
    struct char_data* boris;
    struct obj_data* eq_boris;

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

    if(!AWAKE(boris))
    {
        return FALSE;
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

    if(cmd == CMD_TELL)
    {
        if(strstr(arg, "seguimi"))
        {
            boris->specials.quest_ref = ch;
            return FALSE;
        }
        else if(strstr(arg, "casa"))
        {
            char_from_room(ch);
            char_to_room(ch, 3001);
            do_look(ch, NULL, 0);
            return FALSE;
        }
        else
        {
            return FALSE;
        }
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
                if(!IS_PC(ch) && ch->master == NULL)
                {
                    boris->generic = 4;
                }
                else if(IS_PC(ch) && ch->master == NULL)
                {
                    extract_char(boris);
                    boris = read_mobile(real_mobile(BORIS_IVANHOE), REAL);
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
                    boris = read_mobile(real_mobile(BORIS_IVANHOE), REAL);
                    char_to_room(boris, BORIS_HOME);
                    boris->specials.quest_ref = (ch)->master;
                    boris->generic = 4;
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
                do_say(boris, "Chi vuole comandare mi dica di seguirlo!", 0);
                return FALSE;
            }
            break;

        case 3:
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
                send_to_room("$c0015[$c0005Boris Ivanhoe Gudonov$c0015] dice: 'Abbiamo perso la nostra guida!'\n\r", boris->in_room);
                send_to_room("$c0015[$c0005Boris Ivanhoe Gudonov$c0015] dice: 'Il tempo scorre, ditemi chi devo seguire!'\n\r", boris->in_room);
                do_say(boris, "Chi vuole comandare mi dica di seguirlo!", 0);
                return FALSE;
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

        default:
            break;
    }

    return FALSE;
}

void CheckBorisRoom(struct char_data* boris)
{
    if(boris->specials.fighting)
    {
        return;
    }

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
        {
            if(boris->in_room != boris->commandp)
            {
                send_to_room("$c0015[$c0005Boris Ivanhoe Gudonov$c0015] dice:\n\r", 9008);
                send_to_room("$c0015 'Attento al Golem di Umag, l'ha piazzato qui a protezione della grotta, se non lo tocchi non ti attacchera'.\n\r  Ci sono diverse trappole lungo il cammino, Umag e' molto prudente, state attenti.\n\r", 9008);
            }
            boris->commandp = boris->in_room;
        }
            break;

        case 9020:
        {
            if(boris->in_room != boris->commandp)
            {
                send_to_room("$c0015[$c0005Boris Ivanhoe Gudonov$c0015] dice:\n\r", 9020);
                send_to_room("$c0015 'Attenti ora, c'e' una piana molto pericolosa qui sotto.\n\r  $c0015Diverse creature si aggirano in cerca di prede, il mio mago ha messo un altro Golem per bloccarli,\n\r  $c0015ed e' molto piu' potente di prima. Non temetelo, non vi attacchera'.'\n\r", 9020);
            }
            boris->commandp = boris->in_room;
        }
            break;

        case 9025:
        {
            if(boris->in_room != boris->commandp)
            {
                send_to_room("$c0015[$c0005Boris Ivanhoe Gudonov$c0015] dice:\n\r", 9025);
                send_to_room("$c0015 'Il custode di questi luoghi e' una creatura temibile, non sottovalutatelo!\n\r  $c0015Deve essere vicino, non e' necessario affrontarlo, vi suggerisco di aggirarlo.\n\r  $c0015Se deciderete di affrontarlo sappiate che non temo la morte, nessuna sfida e' troppo grande per Boris!'\n\r", 9025);
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
                    send_to_room("$c0015[$c0005Boris Ivanhoe Gudonov$c0015] dice:\n\r", 9032);
                    send_to_room("$c0015 'Fermi ora. Qui dobbiamo decidere quale strada intraprendere.\n\r  $c0015So per certo che ad est e' accampato l'intero esercito di Arkhat, il lezzo arriva fin qui perfino in questa dimensione.\n\r  $c0015Non so cosa aspettarmi a sud.\n\r  $c0015E' una decisione definitiva, il rituale aprira' solo uno dei due muri oscuri.'\n\r", 9032);
                }
                else
                {
                    send_to_room("$c0015[$c0005Boris Ivanhoe Gudonov$c0015] dice:\n\r", 9032);
                    send_to_room("$c0015 'Da qui non si passa. Dobbiamo trovare un altro modo di farlo, cerchiamo ancora nella vallata.'\n\r", 9032);
                }
            }
            boris->commandp = boris->in_room;
        }
            break;

        case 9034:
        {
            if(boris->in_room != boris->commandp)
            {
                send_to_room("$c0015[$c0005Boris Ivanhoe Gudonov$c0015] dice:\n\r", 9034);
                send_to_room("$c0015 'Bene, siamo tra le tende di comando. Forse riusciremo ad aggirare l'esercito.\n\r  $c0015I due scheletri a guardia non ci impensieriranno...'\n\r", 9034);
            }
            boris->commandp = boris->in_room;
        }
            break;

        case 9035:
        {
            if(boris->in_room != boris->commandp)
            {
                do_say(boris, "Sembra proprio che i due Generali non amino condividere gli spazi.", 0);
                do_action(boris, NULL, CMD_CHUCKLE);
                do_say(boris, "Meglio per noi, li affronteremo uno alla volta.", 0);
                send_to_room("$c0005[$c0015Boris Ivanhoe Gudonov$c0005] sussurra:\n\r", 9035);
                send_to_room("$c0005 'Avverto un brivido lungo la schiena guardando verso sud, qualcosa mi dice che dobbiamo riuscire a passare.'\n\r", 9035);
            }
            boris->commandp = boris->in_room;
        }
            break;

        case 9037:
        {
            if(boris->in_room != boris->commandp)
            {
                send_to_room("$c0015[$c0005Boris Ivanhoe Gudonov$c0015] dice:\n\r", 9037);
                send_to_room("$c0015 'Da qui si vede la tenda di Athelstan.\n\r  $c0015E' uno dei due generali maggiori dell'esercito, e' potente ma per andare avanti dobbiamo sconfiggerlo.\n\r  $c0015Odio i suoi baffoni, sembra uscito da altri tempi, strappiamoglieli da quella faccia da carogna!'\n\r", 9037);
            }
            boris->commandp = boris->in_room;
        }
            break;

        case 9040:
        {
            if(boris->in_room != boris->commandp)
            {
                send_to_room("$c0015[$c0005Boris Ivanhoe Gudonov$c0015] dice:\n\r", 9040);
                send_to_room("$c0015 'Da qui si vede la tenda di Rexar.\n\r  $c0015La sua fama la precede, l'avrete sicuramente sentita nominare.\n\r  $c0015E' una degna avversaria, non commettete il mio stesso errore,\n\r  $c0015la scambiai anni fa per una donna che vendeva amore e fedelta' a pagamento... fui in errore.'\n\r", 9040);
            }
            boris->commandp = boris->in_room;
        }
            break;

        case 9043:
        {
            if(boris->in_room != boris->commandp)
            {
                send_to_room("$c0015[$c0005Boris Ivanhoe Gudonov$c0015] dice:\n\r", 9043);
                send_to_room("$c0015 'Dannazione, lo sapevo!\n\r  $c0015Da qualche parte qui intorno ci deve essere il laboratorio della mente deviata,\n\r  $c0015al servizio di Garebeth, che crea gli ibridi.\n\r", 9043);
                send_to_room("  $c0015Dobbiamo trovare quel bastardo!\n\r  $c0015Umag mi ha detto che ci sara' utile e non possiamo ucciderlo...\n\r  $c0015trattenetemi dal farlo per piacere!\n\r  $c0015Si chiama Uguik e so che ci devono essere anche le cavie che usa per i suoi esperimenti, cerchiamole!'\n\r", 9043);
            }
            boris->commandp = boris->in_room;
        }
            break;

        case 9046:
        {
            if(boris->in_room != boris->commandp)
            {
                send_to_room("$c0015[$c0005Boris Ivanhoe Gudonov$c0015] dice:\n\r", 9046);
                send_to_room("$c0015 'Quell'essere immondo e' il boia di Uguik! Muori maledetto!'\n\r", 9046);
            }
            boris->commandp = boris->in_room;
        }
            break;

        case 9047:
        case 9048:
        {
            if(boris->in_room != boris->commandp)
            {
                send_to_room("$c0008Ti sembra di sentire ringhi di rabbia misti a lacrime provenire da dietro alla maschera di Boris.\n\r", 9048);
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

            if(uguik->commandp2 != 1)
            {
                if(boris->in_room != boris->commandp)
                {
                    send_to_room("$c0015[$c0005Boris Ivanhoe Gudonov$c0015] dice:\n\r", 9051);
                    send_to_room("$c0015 'Ti ucciderei volentieri dopo averti torturato, ma Umag mi ha detto che solo tu\n\r  $c0015possiedi il sapere per attivare il portale.\n\r  $c0015Dicci quello che dobbiamo fare o giuro che il tuo dio Arkhat sembrera'\n\r  un chierichetto in confronto al trattamento che ti riservero'!'\n\r", 9051);
                }
                boris->commandp = boris->in_room;
                boris->commandp2 += 1;
                switch(boris->commandp2)
                {
                    case 0:
                        break;

                    case 2:
                        send_to_room("$c0008Uguik tremante, con le mani di Boris ancora attorno al collo, risponde:\n\r", 9051);
                        send_to_room("$c0011 'Per attivare il portale servono i resti degli Stanislav,\n\r  $c0011ci abbiamo provato ma a quanto pare c'era un altro Stanislav in giro nel piano materiale...\n\r  $c0011E' per questo che siamo bloccati qui!\n\r  $c0011Mi dispiace informarvi che avete fallito, non possiamo evocare Arkhat.\n\r  $c0011Sembra che il vostro piano sia naufragato!'\n\r", 9051);
                        do_action(uguik, NULL, CMD_GRIN);
                        break;

                    case 3:
                        send_to_room("$c0015[$c0009Boris Ivanhoe Gudonov$c0015] $c0009urla:\n\r $c0009'IDIOTA! Sappiamo noi dove si trova l'altro Stanislav! Ti porteremo il suo sangue, dacci lo strumento adatto.'\n\r", 9051);
                        break;

                    case 4:
                        sprintf(buf, "salassatore %s", GET_NAME((boris)->master));
                        if((boris)->master->in_room == uguik->in_room)
                        {
                            do_give(uguik, buf, 0);
                        }
                        else
                        {
                            do_drop(uguik, "salassatore", 0);
                        }
                        sprintf(buf, "chiave %s", GET_NAME((boris)->master));
                        do_give(uguik, buf, 0);
                        if((boris)->master->in_room == uguik->in_room)
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
                            send_to_room("$c0015[$c0005Boris Ivanhoe Gudonov$c0015] dice: 'Bene, ora resta qui! O ti verro' a cercare ovunque tu sia e ti faro' a pezzi lentamente!'\n\r", 9051);
                        }
                        else
                        {
                            boris->commandp2 = 5;
                        }
                        break;

                    case 7:
                        send_to_room("$c0015Boris si volta verso di te e dice:\n\r $c0015'Torniamo sui nostri passi per recuperare le ossa degli Stanislav ed il sangue di Iskra.\n\r  $c0015Uguik ci dira' poi come procedere.'\n\r", 9051);
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
                send_to_room("$c0015[$c0005Boris Ivanhoe Gudonov$c0015] dice:\n\r", 9052);
                send_to_room("$c0015 'Fermatevi un attimo. Sappiate che siamo entrati nella dimensione ombra del piano materiale di Nilmys.\n\r  $c0015Questo significa che i nemici che troveremo sono proiezioni di quelli gia' uccisi in precedenza,\n\r  $c0015alcuni riescono a muoversi tra le dimensioni.\n\r  $c0015Il problema e' che qui sono molto, molto piu' potenti, quindi fate attenzione.'\n\r", 9052);
            }
            boris->commandp = boris->in_room;
        }
            break;

        case 9088:
        {
            if(boris->in_room != boris->commandp && boris->commandp2 > 14)
            {
                send_to_room("$c0015[$c0005Boris Ivanhoe Gudonov$c0015] dice:\n\r", 9088);
                send_to_room("$c0015 'Quanto detesto quello gnomo, se penso a quanta sofferenza ha inflitto...\n\r  $c0015Ora torno indietro e lo faccio a pezzi!'\n\r", 9088);
            }
            boris->commandp = boris->in_room;
        }
            break;

        case 9091:
        {
            if(boris->in_room != boris->commandp)
            {
                send_to_room("$c0015[$c0005Boris Ivanhoe Gudonov$c0015] dice:\n\r", 9091);
                send_to_room("$c0015 'Siamo molto cauti qui, dietro al Gonhag scorgo l'esercito di Garebeth,\n\r  $c0015forse se riuscissimo a liberare la bestia...'\n\r", 9091);
            }
            boris->commandp = boris->in_room;
        }
            break;

        case 9093:
        {
            if(boris->in_room != boris->commandp)
            {
                send_to_room("$c0015[$c0005Boris Ivanhoe Gudonov$c0015] dice:\n\r", 9093);
                send_to_room("$c0015 'Bene ora non ci resta che scalare la montagna, da qui non si torna indietro,\n\r  $c0015sembra che la dimensione d'ombra sia piu' debole.\n\r  $c0015E' l'ultima occasione per prepararci alla scalata.\n\r", 9093);
                send_to_room("  $c0015Possano gli Dei vegliare su di noi... e se non lo fanno... VADANO ALLA MALORA!'\n\r", 9093);
            }
            boris->commandp = boris->in_room;
        }
            break;

        case 9104:
        {
            if(boris->in_room != boris->commandp)
            {
                send_to_room("$c0015[$c0005Boris Ivanhoe Gudonov$c0015] dice:\n\r", 9008);
                send_to_room("$c0015 'Non e' finita a quanto pare. Dobbiamo scendere.\n\r  $c0015Se siamo arrivati fin qui non mi tirero' certo indietro ora.'\n\r", 9008);
            }
            boris->commandp = boris->in_room;
        }
            break;

        case 9114:
        {
            if(boris->in_room != boris->commandp)
            {
                send_to_room("$c0015[$c0005Boris Ivanhoe Gudonov$c0015] dice:\n\r", 9114);
                send_to_room("$c0015 'Troveremo una forte resistenza sul portale, sicuramente Garebeth in persona lo difendera'.\n\r  $c0015Restiamo calmi e poi potremo far uscire Arkhat l'immondo dal suo lungo sonno\n\r  $c0015ed impartirgli una sonora lezione!'\n\r", 9114);
            }
            boris->commandp = boris->in_room;
        }
            break;

        case 9115:
        {
            if(boris->commandp2 >= 1000)
            {
                ArkhatDeath(boris);
            }
            else
            {
                boris->commandp = boris->in_room;
            }
        }
            break;

        case 9116:
        {
            if(boris->commandp2 >= 1000)
            {
                ArkhatDeath(boris);
            }
            else
            {
                boris->commandp = boris->in_room;
            }
        }
            break;

        case 9117:
        {
            if(boris->commandp2 >= 1000)
            {
                ArkhatDeath(boris);
            }
            else
            {
                boris->commandp = boris->in_room;
            }
        }
            break;

        case 9118:
        {
            if(boris->commandp2 >= 1000)
            {
                ArkhatDeath(boris);
            }
            else
            {
                boris->commandp = boris->in_room;
            }
        }
            break;

        case 9119:
        {
            if(boris->commandp2 >= 1000)
            {
                ArkhatDeath(boris);
            }
            else if(boris->in_room != boris->commandp)
            {
                send_to_room("$c0015[$c0005Boris Ivanhoe Gudonov$c0015] dice:\n\r", 9119);
                send_to_room("$c0015 'Garebeth finalmente ci troviamo faccia a faccia! MUORI LURIDO VERME!'\n\r", 9119);
                boris->commandp = boris->in_room;
            }
        }
            break;

        case 9121:
        {
            if(boris->in_room != boris->commandp)
            {
                send_to_room("$c0015Boris vi abbraccia uno ad uno poi dice:\n\r", 9021);
                send_to_room("$c0015 'Grazie amici, non dimentichero' mai cio' che avete fatto per noi.\n\r  $c0015E' stato un onore per me combattere al vostro fianco.\n\r  $c0015Vi prego accettare questo come segno della nostra riconoscenza.'\n\r", 9008);

                // mettere la parte relativa al premio
                do_say(boris, "mo ve premio, daje!", 0);

                do_say(boris, "Possano le nostre strade reincontrarsi un giorno! Addio.", 0);
            }
            boris->commandp = boris->in_room;
        }
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

            switch(number(0, 50))
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

bool FindKeyByNumber(struct char_data* ch, int number)
{
    struct obj_data* key;
    struct char_data* tch;
    int vnum;

    vnum = real_object(number);

    key = get_obj_vis(ch, obj_index[vnum].name);

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
            if(mob_index[tch->nr].iVNum == BORIS_IVANHOE)
            {
                boris = tch;
            }
        }
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
                send_to_room("$c0008 'In questa ampolla ci sono le cinque dosi che ti serviranno per cospargere il portale di Arkhat.\n\r  $c0008Ricordate di cospargere una dose su ogni punta ed una al centro altrimenti il rituale fallira'.\n\r\n\r", uguik->in_room);
                break;

            case 3:
            {
                oggetto = get_obj_in_list_vis(uguik, obj_index[real_object(NILMYS_FLASK)].name, uguik->carrying);
                
                if(oggetto)
                {
                    if((boris)->master->in_room == uguik->in_room)
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
                    
                    if((boris)->master->in_room == uguik->in_room)
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
                send_to_room("$c0008  Scalate la montagna e raggiungerete il circolo rituale.\n\r  $c0008Ho fatto quello che mi avevate chiesto ora abbiate pieta' di me, non uccidetemi!'\n\r", uguik->in_room);
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
                send_to_room("\n\r\n\r$c0008Boris scaraventa Uguik a terra e sputa al di la' della propria spalla.\n\r\n\r", uguik->in_room);
                break;

            case 10:
                send_to_room("$c0015[$c0005Boris Ivanhoe Gudonov$c0015] si gira verso di te, poi dice 'Andiamo!'\n\r", uguik->in_room);
                uguik->generic = 4;
                break;

        }
        
        uguik->commandp += 1;

        return TRUE;
    }
    return FALSE;
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
                        act("Infine... ti giri con aria soddisfatta e il pollice in su!", FALSE, ch, 0, 0, TO_CHAR);
                        act("Infine... si gira con aria soddisfatta e il pollice in su!", FALSE, ch, 0, 0, TO_ROOM);
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

} // namespace Alarmud

