/*ALARMUD* (Do not remove *ALARMUD*, used to automagically manage these lines
 *ALARMUD* AlarMUD 2.0
 *ALARMUD* See COPYING for licence information
 *ALARMUD*/
//  Original intial comments
/* AlarMUD
* $Id: shop.c,v 1.2 2002/02/13 12:30:59 root Exp $
 * */
/***************************  System  include ************************************/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
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
#include "shop.hpp"
#include "act.comm.hpp"
#include "act.social.hpp"
#include "act.wizard.hpp"
#include "comm.hpp"
#include "db.hpp"
#include "handler.hpp"
#include "interpreter.hpp"
#include "regen.hpp"

namespace Alarmud {

#define SHOP_FILE "myst.shp"
#define MAX_TRADE 5
#define MAX_PROD 5

float  shop_multiplier = 0;
int gevent = 0; /* Global Event happening currently */

struct shop_data {
	int producing[MAX_PROD];/* Which item to produce (virtual)      */
	float profit_buy;       /* Factor to multiply cost with.        */
	float profit_sell;      /* Factor to multiply cost with.        */
	byte type[MAX_TRADE];   /* Which item to trade.                 */
	char* no_such_item1;    /* Message if keeper hasn't got an item */
	char* no_such_item2;    /* Message if player hasn't got an item */
	char* missing_cash1;    /* Message if keeper hasn't got cash    */
	char* missing_cash2;    /* Message if player hasn't got cash    */
	char* do_not_buy;        /* If keeper dosn't buy such things.         */
	char* message_buy;      /* Message when player buys item        */
	char* message_sell;     /* Message when player sells item       */
	int temper1;                   /* How does keeper react if no money    */
	int temper2;                   /* How does keeper react when attacked  */
	int keeper;             /* The mobil who owns the shop (virtual)*/
	int with_who;                /* Who does the shop trade with?        */
	int in_room;                /* Where is the shop?                        */
	int open1,open2;        /* When does the shop open?                */
	int close1,close2;        /* When does the shop close?                */
};

struct shop_data* shop_index;
int number_of_shops;

int is_ok(struct char_data* keeper, struct char_data* ch, int shop_nr) {
	if (shop_index[shop_nr].open1>time_info.hours) {
		do_say(keeper,
			   "Come back later!",17);
		return(FALSE);
	}
	else if (shop_index[shop_nr].close1<time_info.hours)
		if (shop_index[shop_nr].open2>time_info.hours) {
			do_say(keeper,
				   "Sorry, we have closed, but come back later.",17);
			return(FALSE);
		}
		else if (shop_index[shop_nr].close2<time_info.hours) {
			do_say(keeper,
				   "Sorry, come back tomorrow.",17);
			return(FALSE);
		};

	if(!(CAN_SEE(keeper,ch)))        {
		do_say(keeper,
			   "I don't trade with someone I can't see!",17);
		return(FALSE);
	};

	switch(shop_index[shop_nr].with_who) {
	case 0 :
		return(TRUE);
	case 1 :
		return(TRUE);
	default :
		return(TRUE);
	};
}
long obj_cost(struct obj_data* temp1,struct char_data* ch,int shop_nr,int vende) {
	long cost;
	double profitto;
	cost=0;
	profitto=0.0;
	profitto=vende?shop_index[shop_nr].profit_sell:shop_index[shop_nr].profit_buy;
	cost = temp1->obj_flags.cost;
	if ((ITEM_TYPE(temp1) == ITEM_WAND) ||
			(ITEM_TYPE(temp1) == ITEM_STAFF)) {
		if (temp1->obj_flags.value[1]) {
			cost = (int)( cost * ( (float)temp1->obj_flags.value[2] /
								   (float)temp1->obj_flags.value[1] ) );
		}
		else {
			cost = 0;
		}
	}
	else if (ITEM_TYPE(temp1) == ITEM_ARMOR) {
		if (temp1->obj_flags.value[1]) {
			cost = (int)( cost * ( (float)temp1->obj_flags.value[0] /
								   (float)temp1->obj_flags.value[1] ) );
		}
		else {
			cost = 0;
		}
	}

//   temp1->obj_flags.cost = cost;
	if (vende) {
		cost = (int) (cost* profitto +
					  ((chr_apply[ (int)GET_CHR(ch) ].reaction *
						cost)/100));
	}
	else {
		cost = (int) (cost* profitto -
					  ((chr_apply[ (int)GET_CHR(ch) ].reaction *
						cost)/100));
	}

	if(cost < 0) { cost=0; }
	mudlog(LOG_CHECK,"obj_cost: profitto:%d.%d actualcost:%d",
		   static_cast<long>(profitto),
		   (static_cast<long>(profitto *1000) % 1000),
		   (long)cost);
	return(cost);
}

int trade_with(struct obj_data* item, int shop_nr) {
	int counter;

	if(item->obj_flags.cost < 1) { return(FALSE); }

	for(counter=0; counter<MAX_TRADE; counter++)
		if(shop_index[shop_nr].type[counter]==item->obj_flags.type_flag)
		{ return(TRUE); }
	return(FALSE);
}

int shop_producing(struct obj_data* item, int shop_nr) {
	int counter;

	if(item->item_number<0) { return(FALSE); }

	for(counter=0; counter<MAX_PROD; counter++)
		if (shop_index[shop_nr].producing[counter] == item->item_number
		   )
		{ return(TRUE); }
	return(FALSE);
}

void shopping_buy( char* arg, struct char_data* ch,
				   struct char_data* keeper, int shop_nr) {
	char argm[100], buf[MAX_STRING_LENGTH], newarg[100];
	int num = 1;
	struct obj_data* temp1;
	int i;
	float mult = 0;
	long actualcost;
	if(!(is_ok(keeper,ch,shop_nr)))
	{ return; }

	if(keeper->generic != 0)
		for(i = 0; i < MAX_TRADE; i++) {
			if(keeper->generic == FAMINE)
				if(shop_index[shop_nr].type[i] == ITEM_FOOD) {
					mult = shop_multiplier; /* famine, we sell food, so we */
					break;             /* rise our prices to hell ;-) -DM */
				}
			if(keeper->generic == DWARVES_STRIKE)
				if((shop_index[shop_nr].type[i] == ITEM_ARMOR) || (shop_index[shop_nr].type[i] == ITEM_WEAPON)) {
					mult = shop_multiplier;
					break;
				}
		}
	mult++;
	only_argument(arg, argm);
	if(!(*argm)) {
		sprintf(buf,
				"%s what do you want to buy??"
				,GET_NAME(ch));
		do_tell(keeper,buf,19);
		return;
	};

	if( ( num = getabunch( argm, newarg ) ) != 0 ) {
		strcpy(argm,newarg);
	}
	if (num == 0) { num = 1; }

	if(!( temp1 =
				get_obj_in_list_vis(ch,argm,keeper->carrying))) {
		sprintf(buf,
				shop_index[shop_nr].no_such_item1
				,GET_NAME(ch));
		do_tell(keeper,buf,19);
		return;
	}

	if(temp1->obj_flags.cost <= 0) {
		sprintf(buf,
				shop_index[shop_nr].no_such_item1
				,GET_NAME(ch));
		do_tell(keeper,buf,19);
		extract_obj(temp1);
		return;
	}
	actualcost=(long)(obj_cost(temp1,ch,shop_nr,0)* mult);
	if ( GET_GOLD(ch) < actualcost) {
		sprintf(buf, shop_index[shop_nr].missing_cash2, GET_NAME(ch));
		do_tell(keeper,buf,19);

		switch(shop_index[shop_nr].temper1) {
		case 0:
			do_action(keeper,GET_NAME(ch),30);
			return;
		case 1:
			do_emote(keeper,"grins happily",36);
			return;
		default:
			return;
		}
	}

	/*if (!CheckEgoGet(ch, temp1))
		   return;*/

	if (!CheckGetBarbarianOK(ch, temp1)) {
		return;
	}

	if ((IS_CARRYING_N(ch) + num) > (CAN_CARRY_N(ch))) {
		sprintf(buf,"%s : You can't carry that many items.\n\r",
				fname(temp1->name));
		send_to_char(buf, ch);
		return;
	}

	if ((IS_CARRYING_W(ch) + (num * temp1->obj_flags.weight)) > CAN_CARRY_W(ch)) {
		sprintf(buf,"%s : You can't carry that much weight.\n\r",
				fname(temp1->name));
		send_to_char(buf, ch);
		return;
	}

	act("$n buys $p.", FALSE, ch, temp1, 0, TO_ROOM);

	sprintf(buf, shop_index[shop_nr].message_buy,
			GET_NAME(ch), (int) actualcost);

	do_tell(keeper,buf,19);

	sprintf(buf,"You now have %s (*%d).\n\r",
			temp1->short_description,num);

	send_to_char(buf,ch);

	while (num-- > 0) {

		if ( GET_GOLD(ch) < actualcost) {
			sprintf(buf, shop_index[shop_nr].missing_cash2, GET_NAME(ch));
			do_tell(keeper,buf,19);

			switch(shop_index[shop_nr].temper1) {
			case 0:
				do_action(keeper,GET_NAME(ch),30);
				return;
			case 1:
				do_emote(keeper,"grins happily",36);
				return;
			default:
				return;
			}
		}
		if (GetMaxLevel(ch)<DIO)
		{ GET_GOLD(ch) -= actualcost; }

		GET_GOLD(keeper) += actualcost;

		/* Test if producing shop ! */
		if (shop_producing(temp1,shop_nr))
		{ temp1 = read_object(temp1->item_number, REAL); }
		else {
			obj_from_char(temp1);
			if (temp1 == NULL) {
				send_to_char("Sorry, I just ran out of those.\n\r",ch);
				GET_GOLD(ch) += actualcost;
				return;
			}
		}

		obj_to_char(temp1,ch);

	}
	return;
}

void shopping_sell( char* arg, struct char_data* ch,
					struct char_data* keeper,int shop_nr) {
	char argm[100], buf[MAX_STRING_LENGTH];
	long actualcost;
	struct obj_data* temp1;

	if(!(is_ok(keeper,ch,shop_nr)))
	{ return; }
	argm[0]='\0';

	only_argument(arg, argm);

	if(!(*argm))        {
		sprintf(buf, "%s What do you want to sell??"
				,GET_NAME(ch));
		do_tell(keeper,buf,19);
		return;
	}

	if (!( temp1 = get_obj_in_list_vis(ch,argm,ch->carrying))) {
		sprintf(buf, shop_index[shop_nr].no_such_item2,GET_NAME(ch));
		do_tell(keeper,buf,19);
		return;
	}

	if( IS_OBJ_STAT( temp1, ITEM_NODROP ) && !IS_IMMORTAL( ch ) ) {
		send_to_char("You can't let go of it, it must be CURSED!\n\r", ch);
		return;
	}

	if (!(trade_with(temp1,shop_nr))||(temp1->obj_flags.cost<1)) {
		sprintf(buf,shop_index[shop_nr].do_not_buy,
				GET_NAME(ch));
		do_tell(keeper,buf,19);
		return;
	}
	actualcost = obj_cost(temp1,ch,shop_nr,1);

	if ( GET_GOLD(keeper) < actualcost) {
		sprintf(buf,shop_index[shop_nr].missing_cash1,GET_NAME(ch));
		do_tell(keeper,buf,19);
		return;
	}

	act("$n sells $p.", FALSE, ch, temp1, 0, TO_ROOM);

	sprintf(buf,shop_index[shop_nr].message_sell,GET_NAME(ch),actualcost);

	do_tell(keeper,buf,19);

	sprintf(buf,"The shopkeeper now has %s.\n\r",
			temp1->short_description);
	send_to_char(buf,ch);

	if (GET_GOLD(keeper)< actualcost) {
		sprintf(buf,shop_index[shop_nr].missing_cash1,GET_NAME(ch));
		do_tell(keeper,buf,19);
		return;
	}

	GET_GOLD(ch) += actualcost;
	GET_GOLD(keeper) -= actualcost;
	obj_from_char(temp1);
	if (temp1 == NULL) {
		send_to_char("As far as I am concerned, you are out..\n\r",ch);
		return;
	}
	if ((get_obj_in_list(argm,keeper->carrying)) ||
			(GET_ITEM_TYPE(temp1) == ITEM_TRASH)) {
		extract_obj(temp1);
	}
	else {
		obj_to_char(temp1,keeper);
	}
	return;
}

void shopping_value( char* arg, struct char_data* ch,
					 struct char_data* keeper, int shop_nr) {
	char argm[100], buf[MAX_STRING_LENGTH];
	struct obj_data* temp1;

	if(!(is_ok(keeper,ch,shop_nr)))
	{ return; }

	only_argument(arg, argm);

	if(!(*argm))    {
		sprintf(buf,"%s What do you want me to evaluate??",
				GET_NAME(ch));
		do_tell(keeper,buf,19);
		return;
	}

	if(!( temp1 = get_obj_in_list_vis(ch,argm,ch->carrying)))    {
		sprintf(buf,shop_index[shop_nr].no_such_item2,
				GET_NAME(ch));
		do_tell(keeper,buf,19);
		return;
	}

	if(!(trade_with(temp1,shop_nr)))    {
		sprintf(buf,
				shop_index[shop_nr].do_not_buy,
				GET_NAME(ch));
		do_tell(keeper,buf,19);
		return;
	}

	sprintf(buf,"%s I'll give you %d gold coins for that!",
			GET_NAME(ch),(int)(obj_cost(temp1,ch,shop_nr,1)));
	do_tell(keeper,buf,19);

	return;
}

void shopping_list( char* arg, struct char_data* ch,
					struct char_data* keeper, int shop_nr) {
	char buf[MAX_STRING_LENGTH], buf2[100],buf3[100];
	struct obj_data* temp1;
	int found_obj;
	int i;
	float mult = 0;
	long actualcost;

	if(!(is_ok(keeper,ch,shop_nr)))
	{ return; }


	if(keeper->generic != 0)
		for(i = 0; i < MAX_TRADE; i++) {
			if(keeper->generic == FAMINE)
				if(shop_index[shop_nr].type[i] == ITEM_FOOD) {
					mult = shop_multiplier; /* we're in a famine, we sell food, so we */
					break;             /* our prices to hell ;-) -DM */
				}
			if(keeper->generic == DWARVES_STRIKE)
				if((shop_index[shop_nr].type[i] == ITEM_ARMOR) || (shop_index[shop_nr].type[i] == ITEM_WEAPON)) {
					mult = shop_multiplier;
					break;
				}
		}

	strcpy(buf,"You can buy:\n\r");
	found_obj = FALSE;
	if(keeper->carrying)
		for(temp1=keeper->carrying; temp1; temp1 = temp1->next_content) {
			actualcost=obj_cost(temp1,ch,shop_nr,0);

			if((CAN_SEE_OBJ(ch,temp1)) && (temp1->obj_flags.cost>0)) {
				found_obj = TRUE;
				if(temp1->obj_flags.type_flag != ITEM_DRINKCON)
					sprintf( buf2,"%s for %d gold coins.\n\r",
							 (temp1->short_description),
							 (int)actualcost);

				else {
					if (temp1->obj_flags.value[1])
						sprintf(buf3,"%s of %s",(temp1->short_description)
								,drinks[temp1->obj_flags.value[2]]);
					else
					{ sprintf(buf3,"%s",(temp1->short_description)); }
					sprintf(buf2,"%s for %d gold coins.\n\r",buf3,
							(int)actualcost);
				}
				CAP( buf2 );
				strcat( buf, buf2 );
			}
		}


	if(!found_obj)
	{ strcat(buf,"Nothing!\n\r"); }

	send_to_char(buf,ch);
	return;
}

int shopping_kill( char* arg, struct char_data* ch,
				   struct char_data* keeper, int shop_nr) {
	char buf[100];

	switch(shop_index[shop_nr].temper2) {
	case 0:
		sprintf(buf,"%s Non pensarci nemmeno!",
				GET_NAME(ch));
		do_tell(keeper,buf,19);
		return(TRUE);

	case 1:
		sprintf(buf,"%s Fottiti, verme!",
				GET_NAME(ch));
		do_tell(keeper,buf,19);
		GET_HIT(keeper)=MAX(GET_HIT(keeper),10000);
		return(FALSE);

	default :
		return(FALSE);
	}
}


int shop_keeper(struct char_data* ch, int cmd, char* arg, char* mob, int type) {
	char argm[100];
	struct char_data* temp_char;
	struct char_data* keeper;
	int shop_nr;

	int citizen(struct char_data *ch, int cmd, char* arg, struct char_data *mob, int type);

	if(type == EVENT_DWARVES_STRIKE) {
		ch->generic = DWARVES_STRIKE;
		return FALSE;
	}
	if(type == EVENT_END_STRIKE) {
		ch->generic = 0;
		return FALSE;
	}

	if(type == EVENT_FAMINE) {
		ch->generic = FAMINE;
		return FALSE;
	}
	if(type == EVENT_END_FAMINE) {
		ch->generic = 0;
		return FALSE;
	}

	keeper = 0;

	for (temp_char = real_roomp(ch->in_room)->people; (!keeper) && (temp_char) ;
			temp_char = temp_char->next_in_room)
		if (IS_MOB(temp_char))
			if (mob_index[temp_char->nr].func == shop_keeper)
			{ keeper = temp_char; }



	for(shop_nr=0 ; shop_index[shop_nr].keeper != keeper->nr; shop_nr++);


	if (!cmd) {
		if (keeper->specials.fighting) {
			return(citizen(keeper,0,"", keeper,0));
		}
	}

	if((cmd == CMD_BUY) && (ch->in_room == shop_index[shop_nr].in_room))
		/* Buy */
	{
		shopping_buy(arg,ch,keeper,shop_nr);
		return(TRUE);
	}

	if((cmd ==CMD_SELL ) && (ch->in_room == shop_index[shop_nr].in_room))
		/* Sell */
	{
		shopping_sell(arg,ch,keeper,shop_nr);
		return(TRUE);
	}

	if((cmd == CMD_VALUE) && (ch->in_room == shop_index[shop_nr].in_room))
		/* value */
	{
		shopping_value(arg,ch,keeper,shop_nr);
		return(TRUE);
	}

	if((cmd == CMD_LIST) && (ch->in_room == shop_index[shop_nr].in_room))
		/* List */
	{
		shopping_list(arg,ch,keeper,shop_nr);
		return(TRUE);
	}

	if ((cmd == CMD_KILL) || (cmd==CMD_HIT)) { /* Kill or Hit */
		only_argument(arg, argm);

		if (keeper == get_char_room(argm,ch->in_room)) {
			return(shopping_kill(arg,ch,keeper,shop_nr));
			return(TRUE);
		}
	}
	else if ((cmd==84) || (cmd==207) || (cmd==172) || (cmd==283) || (cmd==370) ) {
		/* Cast, recite, use */
		if (GetMaxLevel(ch)<IMMORTALE) {
			act("$N tells you 'No magic or mistical powers here - kid!'.", FALSE, ch, 0, keeper, TO_CHAR);
			return TRUE;
		}
		if (shopping_kill(arg,ch,keeper,shop_nr)) {
			GET_HIT(keeper)+=20;
			alter_hit(keeper,0);
		}

	}

	return(FALSE);
}

void boot_the_shops() {
	char* buf;
	int temp;
	int count;
	FILE* shop_f;
	mudlog(LOG_CHECK,"Booting shops");
	if (!(shop_f = fopen(SHOP_FILE, "r"))) {
		perror("Error in boot shop\n");
		exit(0);
	}

	number_of_shops = 0;

	for(;;)    {
		buf = fread_string(shop_f);
		if(*buf == '#') {      /* a new shop */
			mudlog(LOG_CHECK,"Booting shop %s",buf);

			if(!number_of_shops)        /* first shop */
			{ CREATE(shop_index, struct shop_data, 1); }
			else if(!(shop_index=
						  (struct shop_data*) realloc(
							  shop_index,(number_of_shops + 1)*
							  sizeof(struct shop_data)))) {
				perror("Error in boot shop\n");
				exit(0);
			}

			for(count=0; count<MAX_PROD; count++) {
				fscanf(shop_f,"%d \n", &temp);
				mudlog(LOG_CHECK,"Obj %d",temp);
				if (temp >= 0)
					shop_index[number_of_shops].producing[count]=
						real_object(temp);
				else
				{ shop_index[number_of_shops].producing[count]= temp; }
			}
			fscanf(shop_f,"%f \n",
				   &shop_index[number_of_shops].profit_buy);
			fscanf(shop_f,"%f \n",
				   &shop_index[number_of_shops].profit_sell);
			for(count=0; count<MAX_TRADE; count++) {
				fscanf(shop_f,"%d \n", &temp);
				shop_index[number_of_shops].type[count] =
					(byte) temp;
			}
			shop_index[number_of_shops].no_such_item1 =
				fread_string(shop_f);
			shop_index[number_of_shops].no_such_item2 =
				fread_string(shop_f);
			shop_index[number_of_shops].do_not_buy =
				fread_string(shop_f);
			shop_index[number_of_shops].missing_cash1 =
				fread_string(shop_f);
			shop_index[number_of_shops].missing_cash2 =
				fread_string(shop_f);
			shop_index[number_of_shops].message_buy =
				fread_string(shop_f);
			shop_index[number_of_shops].message_sell =
				fread_string(shop_f);
			fscanf(shop_f,"%d \n",
				   &shop_index[number_of_shops].temper1);
			fscanf(shop_f,"%d \n",
				   &shop_index[number_of_shops].temper2);
			fscanf(shop_f,"%d \n",
				   &shop_index[number_of_shops].keeper);
			mudlog(LOG_CHECK,"ShopKeeper is %d",shop_index[number_of_shops].keeper);
			shop_index[number_of_shops].keeper =
				real_mobile(shop_index[number_of_shops].keeper);

			fscanf(shop_f,"%d \n",
				   &shop_index[number_of_shops].with_who);
			fscanf(shop_f,"%d \n",
				   &shop_index[number_of_shops].in_room);
			fscanf(shop_f,"%d \n",
				   &shop_index[number_of_shops].open1);
			fscanf(shop_f,"%d \n",
				   &shop_index[number_of_shops].close1);
			fscanf(shop_f,"%d \n",
				   &shop_index[number_of_shops].open2);
			fscanf(shop_f,"%d \n",
				   &shop_index[number_of_shops].close2);

			number_of_shops++;
		}
		else if(*buf == '$')       /* EOF */
		{ break; }
	}
	mudlog(LOG_CHECK,"Booting shops done");

	fclose(shop_f);
}

void assign_the_shopkeepers() {
	int temp1;
	mudlog(LOG_CHECK,"Assign shopkeepers");

	for(temp1=0 ; temp1<number_of_shops ; temp1++) {
		mob_index[shop_index[temp1].keeper].func = shop_keeper;
		mob_index[shop_index[temp1].keeper].specname = strdup("shop_keeper");
		mob_index[shop_index[temp1].keeper].specparms = strdup("none");
		mudlog(LOG_CHECK,"%d. Loaded %s (%d)",
			   temp1,
			   mob_index[shop_index[temp1].keeper].name,
			   mob_index[shop_index[temp1].keeper].iVNum);
	}
	mudlog(LOG_CHECK,"Assign shopkeepers done");
}

} // namespace Alarmud

