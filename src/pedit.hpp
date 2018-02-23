#ifndef __PEDIT_HPP
#define __PEDIT_HPP
int EditMaster(struct char_data* ch, int cmd, char* arg, struct char_data* mob,int type) ;
void SayMenu(struct char_data* pCh, const char* apchMenu[]) ;
int calc_costopq(int i, int p) ;
int calc_costoxp(int i, int p) ;
static struct char_data* find_editman(struct char_data* ch) ;
#endif // __PEDIT_HPP
