#include "map.h"

struct map* map_create() {
  struct map* root = malloc( sizeof( struct map ) );
  root->key = 0;
  root->value = -1;
  root->next = 0;
  return root;
}

void map_dump( struct map *root ) {
  struct map *current = root;
  unsigned int i = 0;
  while ( current != NULL ) {
    fprintf( stdout, "[% 4d] %s => %d\n", i++, current->key, current->value );
    current = current->next;
  }
}

void map_free_all( struct map *root ) {
  struct map *current = root;
  struct map *temp;
  while ( current != NULL ) {
    temp = current;
    current = current->next;
    //fprintf( stdout, "Freeing: %s\n", temp->key );
    free(temp->key);
    free(temp);
  }
}

struct map* map_append( struct map *current, const char *new_key, int new_value ) {
  current->next = malloc( sizeof( struct map ) );

  char *buffer = malloc(MAP_KEY_BUFFER_SIZE);
  memset( buffer, 0, MAP_KEY_BUFFER_SIZE );
  strncpy( buffer, new_key, MAP_KEY_BUFFER_SIZE-1 );

  current->next->key = buffer;
  current->next->value = new_value;
  current->next->next = 0;
  return current->next;
}

struct map* map_search ( struct map* root, const char *search_key ) {
  if ( search_key == 0 )
    return 0;
  struct map *current = root;
  while ( current != NULL ) {
    if ( current->key == 0 ) {
      current = current->next;
      continue;
    }
    if ( strcmp( current->key, search_key ) == 0 )
      return current;
    current = current->next;
  }
  return 0;
}
