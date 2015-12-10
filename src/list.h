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

struct list *create_root();
void dump( struct list* );
void free_all( struct list* );
struct list *append( struct list*, const void* );
struct list *find( struct list*, const char* );

#endif
