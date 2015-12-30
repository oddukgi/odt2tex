#include "list.h"

struct list* list_create() {
  struct list* root = malloc( sizeof( struct list ) );
  root->data = 0;
  root->next = 0;
  return root;
}

void list_dump( struct list *root ) {
  struct list *current = root;
  unsigned int i = 0;
  while ( current != NULL ) {
    fprintf( stdout, "[% 4d] %s\n", i++, (char*)current->data );
    current = current->next;
  }
}

void list_free( struct list *root ) {
  struct list *current = root;
  struct list *temp;
  while ( current != NULL ) {
    temp = current;
    current = current->next;
    free(temp->data);
    free(temp);
  }
}

struct list* list_append( struct list *list, const void *new_data ) {
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
 
struct list* list_find( struct list *root, const char *key ) {
  struct list *current = root;
  while ( current != NULL ) {
    if ( current->data != NULL && strcmp( current->data, key ) == 0 ) {
      return current;
    }
    current = current->next;
  }
  return NULL;
}

struct list *list_reverse( struct list* root ) {
  if ( root == NULL ) return 0;

  struct list *current = root;
  struct list *new_root = 0;

  while(current) {
    struct list *next = current->next;
    current->next = new_root;
    new_root = current;
    current = next;
  }

  return new_root;

}

struct list *list_insert( struct list *root, unsigned int inspos, const void *new_data ) {
  if ( new_data == NULL ) return 0;
  if ( root == NULL ) return 0;

  struct list *current = root;

  unsigned int i = 0;
  while ( current != NULL ) {

    if ( i == inspos && current != NULL ) {
      struct list *temp = current->next;

      struct list *new_item = malloc( sizeof( struct list ) );
      new_item->data = 0;
      new_item->next = temp;

      current->next = new_item;

      char *buffer = malloc(ELEM_BUFFER_SIZE);
      memset( buffer, 0, ELEM_BUFFER_SIZE );
      strncpy( buffer, new_data, ELEM_BUFFER_SIZE-1 );
      new_item->data = buffer;

      return new_item;
    }

    current = current->next;
    i++;
  }

  return 0;
}

struct list *list_remove( struct list *root, unsigned int index ) {

  struct list *current = root;

  unsigned int i = 0;
  while ( current != NULL ) {

    if ( i == index && current != NULL && current->next != NULL ) {
      struct list *new_next = current->next->next;
      free(current->next->data);
      free(current->next);
      current->next = new_next;

      return new_next;
    }

    current = current->next;
    i++;
  }

  return 0;
}

struct list *list_at( struct list* root, unsigned int index ) {
  if ( root == NULL ) return 0;

  struct list *current = root;

  unsigned int i = 0;
  while ( current ) {
    if ( i == index && current != NULL ) return current;
    current = current->next;
    i++;
  }

  return 0;
}
