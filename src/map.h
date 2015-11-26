#ifndef __map_h
#define __map_h

#include "main.h"

#define MAP_KEY_BUFFER_SIZE 8

struct map {
  char *key;
  int value;
  struct map *next;
};

struct map *map_create();
void map_dump( struct map* );
void map_free_all( struct map* );
struct map *map_append( struct map*, const char*, int );
struct map *map_search( struct map*, const char* );

#endif
