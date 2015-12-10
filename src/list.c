#include "list.h"

struct list* create_root() {
  struct list* root = malloc( sizeof( struct list ) );
  root->data = 0;
  root->next = 0;
  return root;
}

void dump( struct list *root ) {
  struct list *current = root;
  unsigned int i = 0;
  while ( current != NULL ) {
    fprintf( stdout, "[% 4d] %s\n", i++, (char*)current->data );
    current = current->next;
  }
}

void free_all( struct list *root ) {
  struct list *current = root;
  struct list *temp;
  while ( current != NULL ) {
    temp = current;
    current = current->next;
    free(temp->data);
    free(temp);
  }
}

struct list* append( struct list *list, const void *new_data ) {
  list->next = malloc( sizeof( struct list ) );
  list->next->data = 0;
  list->next->next = 0;

  if ( new_data != NULL ) {
    char *buffer = malloc(ELEM_BUFFER_SIZE);
    memset( buffer, 0, ELEM_BUFFER_SIZE );
    strncpy( buffer, new_data, ELEM_BUFFER_SIZE-1 );
    list->next->data = buffer;
  }  

  return list->next;
}
 
struct list* find( struct list *root, const char *key ) {
  struct list *current = root;
  while ( current != NULL ) {
    if ( current->data != NULL && strcmp( current->data, key ) == 0 ) {
      return current;
    }
    current = current->next;
  }
  return NULL;
}
