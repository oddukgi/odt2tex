#ifndef __list_h
#define __list_h

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define ELEM_BUFFER_SIZE 128

struct list {
  void *data;
  struct list *next;
};

struct list *list_create();
void list_dump( struct list* );
void list_free( struct list* );
struct list *list_append( struct list*, const void* );
struct list *list_find( struct list*, const char* );
struct list *list_reverse( struct list* );
struct list *list_insert( struct list*, unsigned int, const void* );
struct list *list_remove( struct list*, unsigned int );
struct list *list_at( struct list*, unsigned int );

#endif
