/*ALARMUD* (Do not remove *ALARMUD*, used to automagically manage these lines
 *ALARMUD* AlarMUD 2.0
 *ALARMUD* See COPYING for licence information
 *ALARMUD*/
//  Original intial comments
/* sound.cpp */
#ifndef __SOUND_HPP
#define __SOUND_HPP
/***************************  System  include ************************************/
/***************************  Local    include ************************************/
namespace Alarmud {
int RecGetObjRoom(struct obj_data* obj);
void MakeNoise(int room, const char* local_snd, const char* distant_snd);
void MakeSound(unsigned long pulse);
} // namespace Alarmud
#endif

