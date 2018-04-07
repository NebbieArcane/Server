/*ALARMUD* (Do not remove *ALARMUD*, used to automagically manage these lines
 *ALARMUD* AlarMUD 2.0
 *ALARMUD* See COPYING for licence information
 *ALARMUD*/
/***************************  System  include ************************************/
/***************************  Local    include ************************************/
#ifndef __HASH_HPP_
#define __HASH_HPP_
#include "typedefs.hpp"
namespace Alarmud {
struct hash_link {
	int        key;
	struct hash_link* next;
	void*        data;
};

struct hash_header {
	int        rec_size;
	int        table_size;
	int*        keylist, klistsize, klistlen; /* this is really lame,
                                          AMAZINGLY lame */
	struct hash_link**        buckets;
};

#define WORLD_SIZE 50000
void _hash_enter(struct hash_header* ht, long key, void* data);
void destroy_hash_table(struct hash_header* ht, void (*gman)(void*)) ;
int hash_enter(struct hash_header* ht, long key, void* data) ;
void* hash_find(struct hash_header* ht, long key) ;
void* hash_find_or_create(struct hash_header* ht, long key) ;
void hash_iterate(struct hash_header* ht,iterate_func func,void* cdata);
void* hash_remove(struct hash_header* ht, long key) ;
void init_hash_table(struct hash_header* ht, int rec_size, int table_size) ;
void init_world(struct room_data* room_db[]) ;
int room_enter(struct room_data* rb[], long key, struct room_data* rm) ;
struct room_data* room_find(struct room_data* room_db[], long key) ;
struct room_data* room_find_or_create(struct room_data* rb[], long key) ;
void room_iterate(struct room_data* rb[],iterate_func func,void* cdata);
int room_remove(struct room_data* rb[], long key) ;
} // namespace Alarmud
#endif // __HASH_HPP

