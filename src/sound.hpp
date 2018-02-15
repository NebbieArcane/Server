/* sound.cpp */
#ifndef __SOUND_HPP
#define __SOUND_HPP
int RecGetObjRoom(struct obj_data *obj);
void MakeNoise(int room, const char *local_snd, const char *distant_snd);
void MakeSound(unsigned long pulse);
#endif
