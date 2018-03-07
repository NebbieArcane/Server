/*ALARMUD* (Do not remove *ALARMUD*, used to automagically manage these lines
 *ALARMUD* AlarMUD 2.0
 *ALARMUD* See COPYING for licence information
 *ALARMUD*/
#ifndef __WEATHER_HPP
#define __WEATHER_HPP
/* what stage is moon in?  (1 - 32) */
/***************************  System  include ************************************/
/***************************  Local    include ************************************/
namespace Alarmud {
extern unsigned char moontype;

extern int gSeason;   /* global variable --- the season */
extern int gMoonSet;
extern int gSunRise;
extern int gSunSet;
extern int gMoonRise;
extern int gLightLevel;  /* defaults to sunlight */

void ChangeSeason(int month) ;
void ChangeWeather( int change) ;
void GetMonth( int month) ;
void another_hour(int mode) ;
void switch_light(byte why) ;
void weather_and_time(int mode) ;
void weather_change() ;
} // namespace Alarmud
#endif // __WEATHER_HPP

