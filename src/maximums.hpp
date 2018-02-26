/*$Id: maximums.h,v 1.2 2002/02/13 12:30:58 root Exp $
*/
#ifndef __MAXIMUMS_HPP
#define __MAXIMUMS_HPP
#include "config.hpp"

struct title_type {
	const char* title_m;
	const char* title_f;
	int exp;
};

void ClassSpecificStuff( struct char_data* ch) ;
char* ClassTitles(struct char_data* ch) ;
int GetExtimatedHp(struct char_data* ch) ;
int GetHpGain(struct char_data* ch, int iClass,int livello,int compat,int check=0) ;
void ObjFromCorpse( struct obj_data* c) ;
void advance_level(struct char_data* ch, int iClass);
void check_idling(struct char_data* ch) ;
void drop_level(struct char_data* ch, int iClass, int goddrain) ;
void gain_condition(struct char_data* ch,int condition,int value) ;
int gain_corretto(struct char_data* ch,int gain) ;
void gain_exp( struct char_data* ch, int gain ) ;
void gain_exp_regardless( struct char_data* ch, int gain, int iClass, int iMaxLevel );
void gain_exp_rev( struct char_data* ch, int gain ) ;
int graf(int age, int p0, int p1, int p2, int p3, int p4, int p5, int p6) ;
int hit_gain(struct char_data* ch) ;// Gaia 2001
int hit_limit(struct char_data* ch) ;
int mana_gain(struct char_data* ch) ;
int mana_limit(struct char_data* ch) ;
int move_gain(struct char_data* ch) ;
int move_limit(struct char_data* ch) ;
void set_title(struct char_data* ch) ;
#endif // __MAXIMUMS_HPP
