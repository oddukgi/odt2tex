#ifndef __element_h
#define __element_h

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define ELEM_BUFFER_SIZE 128

struct element {
  void *data;
  struct element *next;
};

struct element *create_root();
void dump( struct element* );
void free_all( struct element* );
struct element *append( struct element*, void* );
struct element *find( struct element*, const char* );

#endif
