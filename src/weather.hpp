#ifndef __WEATHER_HPP
#define __WEATHER_HPP
#include "config.hpp"
#include "typedefs.hpp"
/* what stage is moon in?  (1 - 32) */
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
#endif // __WEATHER_HPP
