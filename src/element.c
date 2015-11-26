#include "element.h"

struct element* create_root() {
  struct element* root = malloc( sizeof( struct element ) );
  root->data = 0;
  root->next = 0;
  return root;
}

void dump( struct element *root ) {
  struct element *current = root;
  unsigned int i = 0;
  while ( current != NULL ) {
    fprintf( stdout, "[% 4d] %s\n", i++, (char*)current->data );
    current = current->next;
  }
}

void free_all( struct element *root ) {
  struct element *current = root;
  struct element *temp;
  while ( current != NULL ) {
    temp = current;
    current = current->next;
    free(temp->data);
    free(temp);
  }
}

struct element* append( struct element *element, void *new_data ) {
  element->next = malloc( sizeof( struct element ) );
  element->next->data = new_data;
  element->next->next = 0;
  return element->next;
}
 
