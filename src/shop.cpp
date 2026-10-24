/*ALARMUD* (Do not remove *ALARMUD*, used to automagically manage these lines
 *ALARMUD* AlarMUD 2.0
 *ALARMUD* See COPYING for licence information
 *ALARMUD*/
//  Original intial comments
/* AlarMUD
* $Id: shop.c,v 1.2 2002/02/13 12:30:59 root Exp $
 * */
/***************************  System  include ************************************/
#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <string>
#include <string_view>
/***************************  General include ************************************/
#include "config.hpp"
#include "typedefs.hpp"
#include "flags.hpp"
#include "autoenums.hpp"
#include "structs.hpp"
#include "logging.hpp"
#include "constants.hpp"
#include "utils.hpp"
#include "utility.hpp"
/***************************  Local    include ************************************/
#include "shop.hpp"
#include "act.comm.hpp"
#include "act.social.hpp"
#include "act.wizard.hpp"
#include "character_item_loss.hpp"
#include "comm.hpp"
#include "db.hpp"
#include "handler.hpp"
#include "interpreter.hpp"
#include "regen.hpp"
#include "spec_procs.hpp"
#include "spell_parser.hpp"

namespace Alarmud {

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
	if(shop_index[shop_nr].open1>time_info.hours) {
		do_say(keeper,
			   "Come back later!",17);
		return(FALSE);
	}
	else if(shop_index[shop_nr].close1<time_info.hours) {
		if(shop_index[shop_nr].open2>time_info.hours) {
			do_say(keeper,
				   "Sorry, we have closed, but come back later.",17);
			return(FALSE);
		}
		else if(shop_index[shop_nr].close2<time_info.hours) {
			do_say(keeper,
				   "Sorry, come back tomorrow.",17);
			return(FALSE);
		};
	}
	if(!(CAN_SEE(keeper,ch))) {
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
long object_cost(struct obj_data* temp1,struct char_data* ch,int shop_nr,int vende) {
	long cost;
	double profitto;
	cost=0;
	profitto=0.0;
	profitto=vende?shop_index[shop_nr].profit_sell:shop_index[shop_nr].profit_buy;
	cost = temp1->obj_flags.cost;
	if((ITEM_TYPE(temp1) == ITEM_WAND) ||
			(ITEM_TYPE(temp1) == ITEM_STAFF)) {
		if(temp1->obj_flags.value[1]) {
			cost = (int)(cost * ((float)temp1->obj_flags.value[2] /
								 (float)temp1->obj_flags.value[1]));
		}
		else {
			cost = 0;
		}
	}
	else if(ITEM_TYPE(temp1) == ITEM_ARMOR) {
		if(temp1->obj_flags.value[1]) {
			cost = (int)(cost * ((float)temp1->obj_flags.value[0] /
								 (float)temp1->obj_flags.value[1]));
		}
		else {
			cost = 0;
		}
	}

//   temp1->obj_flags.cost = cost;
	if(vende) {
		cost = (int)(cost* profitto +
					 ((chr_apply[(int)GET_CHR(ch) ].reaction *
					   cost)/100));
	}
	else {
		cost = (int)(cost* profitto -
					 ((chr_apply[(int)GET_CHR(ch) ].reaction *
					   cost)/100));
	}

	if(cost < 0) {
		cost=0;
	}
	mudlog(LOG_CHECK,"obj_cost: profitto:%d.%d actualcost:%d",
		   static_cast<long>(profitto),
		   (static_cast<long>(profitto *1000) % 1000),
		   (long)cost);
	return(cost);
}

int trade_with(struct obj_data* item, int shop_nr) {
	int counter;

	if(item->obj_flags.cost < 1) {
		return(FALSE);
	}

	for(counter=0; counter<MAX_TRADE; counter++)
		if(shop_index[shop_nr].type[counter]==item->obj_flags.type_flag) {
			return(TRUE);
		}
	return(FALSE);
}

int shop_producing(struct obj_data* item, int shop_nr) {
	int counter;

	if(item->item_number<0) {
		return(FALSE);
	}

	for(counter=0; counter<MAX_PROD; counter++)
		if(shop_index[shop_nr].producing[counter] == item->item_number
		  ) {
			return(TRUE);
		}
	return(FALSE);
}

namespace {
/**
 * Costruisce un messaggio del negoziante (myst.shp) senza passare il
 * formato del file a sprintf: il primo %s diventa il nome, il primo %d
 * (o %ld) l'importo gia' formattato, %% diventa %. Ogni altra sequenza
 * con % resta com'e', quindi un formato sbagliato nel file non puo'
 * causare UB, al massimo un testo strano.
 */
[[nodiscard]] std::string shop_format(const char* tmpl, std::string_view name,
									  std::string_view amount = {}) {
	std::string out;
	if(tmpl == nullptr) {
		return out;
	}
	const std::string_view text(tmpl);
	bool name_done = false;
	bool amount_done = false;
	for(std::size_t i = 0; i < text.size(); ++i) {
		if(text[i] != '%' || i + 1 >= text.size()) {
			out.push_back(text[i]);
			continue;
		}
		const char next = text[i + 1];
		if(next == '%') {
			out.push_back('%');
			++i;
		}
		else if(next == 's' && !name_done) {
			out += name;
			name_done = true;
			++i;
		}
		else if(next == 'd' && !amount_done) {
			out += amount;
			amount_done = true;
			++i;
		}
		else if(next == 'l' && i + 2 < text.size() && text[i + 2] == 'd' && !amount_done) {
			out += amount;
			amount_done = true;
			i += 2;
		}
		else {
			out.push_back(text[i]);
		}
	}
	return out;
}

/** Testo sicuro anche per short_description nulle. */
[[nodiscard]] std::string_view shop_text(const char* text, std::size_t max_len = std::string_view::npos) {
	const std::string_view view(text != nullptr ? text : "");
	return view.substr(0, max_len);
}

void shop_tell(struct char_data* keeper, const std::string& message) {
	do_tell(keeper, message.c_str(), CMD_TELL);
}

/** Il giocatore non ha abbastanza soldi: messaggio e reazione del negoziante. */
void shop_player_broke(struct char_data* keeper, struct char_data* ch, const shop_data& shop) {
	shop_tell(keeper, shop_format(shop.missing_cash2, GET_NAME(ch)));
	switch(shop.temper1) {
	case 0:
		do_action(keeper, GET_NAME(ch), 30);
		break;
	case 1:
		do_emote(keeper, "grins happily", 36);
		break;
	default:
		break;
	}
}
} // namespace

void shopping_buy(char* arg, struct char_data* ch,
				  struct char_data* keeper, int shop_nr) {
	if(!is_ok(keeper, ch, shop_nr)) {
		return;
	}
	const shop_data& shop = shop_index[shop_nr];

	float mult = 0;
	if(keeper->generic != 0) {
		for(int i = 0; i < MAX_TRADE; i++) {
			/* carestia e vendiamo cibo: prezzi alle stelle ;-) -DM */
			if(keeper->generic == FAMINE && shop.type[i] == ITEM_FOOD) {
				mult = shop_multiplier;
				break;
			}
			if(keeper->generic == DWARVES_STRIKE &&
					(shop.type[i] == ITEM_ARMOR || shop.type[i] == ITEM_WEAPON)) {
				mult = shop_multiplier;
				break;
			}
		}
	}
	mult++;

	std::array<char, 100> argm{};
	only_argument(arg, argm.data());
	if(argm[0] == '\0') {
		shop_tell(keeper, std::string(GET_NAME(ch)) + " what do you want to buy??");
		return;
	}
	std::array<char, 100> newarg{};
	int num = getabunch(argm.data(), newarg.data());
	if(num != 0) {
		argm = newarg;
	}
	else {
		num = 1;
	}

	struct obj_data* item = get_obj_in_list_vis(ch, argm.data(), keeper->carrying);
	if(item == nullptr) {
		shop_tell(keeper, shop_format(shop.no_such_item1, GET_NAME(ch)));
		return;
	}
	if(item->obj_flags.cost <= 0) {
		shop_tell(keeper, shop_format(shop.no_such_item1, GET_NAME(ch)));
		extract_obj(item);
		return;
	}

	const long actualcost = static_cast<long>(object_cost(item, ch, shop_nr, 0) * mult);
	if(GET_GOLD(ch) < actualcost) {
		shop_player_broke(keeper, ch, shop);
		return;
	}
	if(!CheckGetBarbarianOK(ch, item)) {
		return;
	}
	if((IS_CARRYING_N(ch) + num) > CAN_CARRY_N(ch)) {
		send_to_char((std::string(fname(item->name)) +
					  " : You can't carry that many items.\n\r").c_str(), ch);
		return;
	}
	if((IS_CARRYING_W(ch) + (num * item->obj_flags.weight)) > CAN_CARRY_W(ch)) {
		send_to_char((std::string(fname(item->name)) +
					  " : You can't carry that much weight.\n\r").c_str(), ch);
		return;
	}

	act("$n buys $p.", false, ch, item, nullptr, TO_ROOM);
	shop_tell(keeper, shop_format(shop.message_buy, GET_NAME(ch),
								  format_thousands(actualcost)));
	send_to_char(("You now have " + std::string(shop_text(item->short_description)) +
				  " (*" + std::to_string(num) + ").\n\r").c_str(), ch);

	while(num-- > 0) {
		if(GET_GOLD(ch) < actualcost) {
			shop_player_broke(keeper, ch, shop);
			return;
		}
		if(GetMaxLevel(ch) < DIO) {
			GET_GOLD(ch) -= actualcost;
		}
		GET_GOLD(keeper) += actualcost;
		/* Test if producing shop ! */
		if(shop_producing(item, shop_nr)) {
			item = read_object(item->item_number, REAL);
		}
		else {
			obj_from_char(item);
			if(item == nullptr) {
				send_to_char("Sorry, I just ran out of those.\n\r", ch);
				GET_GOLD(ch) += actualcost;
				return;
			}
		}
		obj_to_char(item, ch);
	}
}

void shopping_sell(char* arg, struct char_data* ch,
				   struct char_data* keeper, int shop_nr) {
	if(!is_ok(keeper, ch, shop_nr)) {
		return;
	}
	const shop_data& shop = shop_index[shop_nr];

	std::array<char, 100> argm{};
	only_argument(arg, argm.data());
	if(argm[0] == '\0') {
		shop_tell(keeper, std::string(GET_NAME(ch)) + " What do you want to sell??");
		return;
	}
	struct obj_data* item = get_obj_in_list_vis(ch, argm.data(), ch->carrying);
	if(item == nullptr) {
		shop_tell(keeper, shop_format(shop.no_such_item2, GET_NAME(ch)));
		return;
	}
	if(IS_OBJ_STAT(item, ITEM_NODROP) && !IS_IMMORTAL(ch)) {
		send_to_char("You can't let go of it, it must be CURSED!\n\r", ch);
		return;
	}
	if(!trade_with(item, shop_nr) || item->obj_flags.cost < 1) {
		shop_tell(keeper, shop_format(shop.do_not_buy, GET_NAME(ch)));
		return;
	}

	const long actualcost = object_cost(item, ch, shop_nr, 1);
	if(GET_GOLD(keeper) < actualcost) {
		shop_tell(keeper, shop_format(shop.missing_cash1, GET_NAME(ch)));
		return;
	}

	act("$n sells $p.", false, ch, item, nullptr, TO_ROOM);
	shop_tell(keeper, shop_format(shop.message_sell, GET_NAME(ch),
								  format_thousands(actualcost)));
	send_to_char(("The shopkeeper now has " + std::string(shop_text(item->short_description)) +
				  ".\n\r").c_str(), ch);

	GET_GOLD(ch) += actualcost;
	GET_GOLD(keeper) -= actualcost;
	character_item_loss_log(ch, item, kItemLossShopSell);
	obj_from_char(item);
	if(item == nullptr) {
		send_to_char("As far as I am concerned, you are out..\n\r", ch);
		return;
	}
	if(get_obj_in_list(argm.data(), keeper->carrying) != nullptr ||
			GET_ITEM_TYPE(item) == ITEM_TRASH) {
		extract_obj(item);
	}
	else {
		obj_to_char(item, keeper);
	}
}

void shopping_value(char* arg, struct char_data* ch,
					struct char_data* keeper, int shop_nr) {
	if(!is_ok(keeper, ch, shop_nr)) {
		return;
	}
	const shop_data& shop = shop_index[shop_nr];

	std::array<char, 100> argm{};
	only_argument(arg, argm.data());
	if(argm[0] == '\0') {
		shop_tell(keeper, std::string(GET_NAME(ch)) + " What do you want me to evaluate??");
		return;
	}
	struct obj_data* item = get_obj_in_list_vis(ch, argm.data(), ch->carrying);
	if(item == nullptr) {
		shop_tell(keeper, shop_format(shop.no_such_item2, GET_NAME(ch)));
		return;
	}
	if(!trade_with(item, shop_nr)) {
		shop_tell(keeper, shop_format(shop.do_not_buy, GET_NAME(ch)));
		return;
	}
	shop_tell(keeper, std::string(GET_NAME(ch)) + " I'll give you " +
			  format_thousands(object_cost(item, ch, shop_nr, 1)) + " gold coins for that!");
}

void shopping_list(char* arg, struct char_data* ch,
				   struct char_data* keeper, int shop_nr) {
	if(!is_ok(keeper, ch, shop_nr)) {
		return;
	}
	std::string out = "You can buy:\n\r";
	bool found_obj = false;
	for(struct obj_data* item = keeper->carrying; item != nullptr; item = item->next_content) {
		const long actualcost = object_cost(item, ch, shop_nr, 0);
		if(!CAN_SEE_OBJ(ch, item) || item->obj_flags.cost <= 0) {
			continue;
		}
		found_obj = true;
		std::string line;
		if(item->obj_flags.type_flag != ITEM_DRINKCON) {
			line = shop_text(item->short_description, 220);
		}
		else if(item->obj_flags.value[1]) {
			line = std::string(shop_text(item->short_description, 180)) + " of " +
				   std::string(shop_text(drinks[item->obj_flags.value[2]], 60));
			line.resize(std::min<std::size_t>(line.size(), 220));
		}
		else {
			line = shop_text(item->short_description, 220);
		}
		line += " for " + format_thousands(actualcost) + " gold coins.\n\r";
		line.front() = static_cast<char>(UPPER(line.front()));
		out += line;
	}
	if(!found_obj) {
		out += "Nothing!\n\r";
	}
	send_to_char(out.c_str(), ch);
}


int shopping_kill(char* arg, struct char_data* ch,
				  struct char_data* keeper, int shop_nr) {
	char buf[100];

	switch(shop_index[shop_nr].temper2) {
	case 0:
		sprintf(buf,"%s Non pensarci nemmeno!",
				GET_NAME(ch));
		do_tell(keeper,buf,CMD_TELL);
		return(TRUE);

	case 1:
		sprintf(buf,"%s Fottiti, verme!",
				GET_NAME(ch));
		do_tell(keeper,buf,CMD_TELL);
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

	for(temp_char = real_roomp(ch->in_room)->people; (!keeper) && (temp_char) ;
			temp_char = temp_char->next_in_room)
		if(IS_MOB(temp_char))
			if(mob_index[temp_char->nr].func == reinterpret_cast<genericspecial_func>(shop_keeper)) {
				keeper = temp_char;
			}



	for(shop_nr=0 ; shop_index[shop_nr].keeper != keeper->nr; shop_nr++);


	if(!cmd) {
		if(keeper->specials.fighting) {
			return(citizen(keeper,0,"", keeper,0));
		}
	}

	if((cmd == CMD_BUY) && (ch->in_room == shop_index[shop_nr].in_room))
		/* Buy */
	{
		shopping_buy(arg,ch,keeper,shop_nr);
		return(TRUE);
	}

	if((cmd == CMD_SELL) && (ch->in_room == shop_index[shop_nr].in_room))
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

	if((cmd == CMD_KILL) || (cmd==CMD_HIT)) {  /* Kill or Hit */
		only_argument(arg, argm);

		if(keeper == get_char_room(argm,ch->in_room)) {
			return(shopping_kill(arg,ch,keeper,shop_nr));
			return(TRUE);
		}
	}
	else if((cmd==CMD_CAST) || (cmd==CMD_RECITE) || (cmd==CMD_USE) || (cmd==CMD_RECALL) || (cmd==CMD_MIND)) {
		/* Cast, recite, use */
		if(GetMaxLevel(ch)<IMMORTALE) {
			act("$N tells you 'No magic or mistical powers here - kid!'.", FALSE, ch, 0, keeper, TO_CHAR);
			return TRUE;
		}
		if(shopping_kill(arg,ch,keeper,shop_nr)) {
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
	if(!(shop_f = fopen(SHOP_FILE, "r"))) {
		mudlog(LOG_ERROR,"%s:%s","Error in boot shop\n",strerror(errno));
		exit(0);
	}

	number_of_shops = 0;

	for(;;)    {
		buf = fread_string(shop_f);
		if(*buf == '#') {      /* a new shop */
			mudlog(LOG_SAVE,"Booting shop %s",buf);

			if(!number_of_shops) {      /* first shop */
				CREATE(shop_index, struct shop_data, 1);
			}
			else if(!(shop_index=
						  (struct shop_data*) realloc(
							  shop_index,(number_of_shops + 1)*
							  sizeof(struct shop_data)))) {
				mudlog(LOG_ERROR,"%s:%s","Error in boot shop\n",strerror(errno));
				exit(0);
			}

			for(count=0; count<MAX_PROD; count++) {
				fscanf(shop_f,"%d \n", &temp);
				mudlog(LOG_SAVE,"Obj %d",temp);
				if(temp >= 0)
					shop_index[number_of_shops].producing[count]=
						real_object(temp);
				else {
					shop_index[number_of_shops].producing[count]= temp;
				}
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
			mudlog(LOG_SAVE,"ShopKeeper is %d",shop_index[number_of_shops].keeper);
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
		else if(*buf == '$') {     /* EOF */
			break;
		}
	}
	mudlog(LOG_CHECK,"Booting shops done");

	fclose(shop_f);
}

void assign_the_shopkeepers() {
	int temp1;
	mudlog(LOG_CHECK,"Assign shopkeepers");

	for(temp1=0 ; temp1<number_of_shops ; temp1++) {
		mob_index[shop_index[temp1].keeper].func = reinterpret_cast<genericspecial_func>(shop_keeper);
		mob_index[shop_index[temp1].keeper].specname = strdup("shop_keeper");
		mob_index[shop_index[temp1].keeper].specparms = strdup("none");
		mudlog(LOG_WORLD,"%d. Loaded %s (%d)",
			   temp1,
			   mob_index[shop_index[temp1].keeper].name,
			   mob_index[shop_index[temp1].keeper].iVNum);
	}
	mudlog(LOG_CHECK,"Assign shopkeepers done");
}

} // namespace Alarmud

