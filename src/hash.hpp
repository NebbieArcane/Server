/*$Id: hash.h,v 1.2 2002/02/13 12:30:58 root Exp $
*/
#ifndef __HASH_HPP_
#define __HASH_HPP_
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
typedef void (*pIterateFunc)( int, struct room_data*, void* );

#define WORLD_SIZE 50000
void _hash_enter(struct hash_header* ht, long key, void* data);
void destroy_hash_table(struct hash_header* ht, void (*gman)(void*)) ;
int hash_enter(struct hash_header* ht, long key, void* data) ;
void* hash_find(struct hash_header* ht, long key) ;
void* hash_find_or_create(struct hash_header* ht, long key) ;
void hash_iterate( struct hash_header* ht,pIterateFunc func,void* cdata);
void* hash_remove(struct hash_header* ht, long key) ;
void init_hash_table(struct hash_header* ht, int rec_size, int table_size) ;
void init_world(struct room_data* room_db[]) ;
int room_enter(struct room_data* rb[], long key, struct room_data* rm) ;
struct room_data* room_find( struct room_data* room_db[], long key) ;
struct room_data* room_find_or_create(struct room_data* rb[], long key) ;
void room_iterate( struct room_data* rb[],pIterateFunc func,void* cdata );
int room_remove(struct room_data* rb[], long key) ;
#endif // __HASH_HPP
