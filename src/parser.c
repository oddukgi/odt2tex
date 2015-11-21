#include "parser.h"

const char *get_attribute_value( const char **attr, const char *key ) {
  unsigned int idx = 0;
  while ( attr[idx] != NULL ) {
    if ( strcmp(attr[idx],key) == 0 ) {
      return attr[idx+1];
    }
    idx+=2;
  }

  return NULL;
}

void chars( void *data, const char *s, int len ) {
  if ( data == NULL ) {
    return;
  }

  parser_context_t *pc = (parser_context_t*)data;

  FILE *f = pc->f;

  char buffer[len+1];
  memset( buffer, 0, len+1 );
  strncpy( buffer, s, len );

  if ( pc->cmd == TEX_SECTION ) {
    fprintf( f, "\\section{%s}\n", buffer );
  } else
  if ( pc->cmd == TEX_SUBSECTION ) {
    fprintf( f, "\\subsection{%s}\n", buffer );
  } else
  if ( pc->cmd == TEX_SUBSUBSECTION ) {
    fprintf( f, "\\subsubsection{%s}\n", buffer );
  } else
  if ( pc->cmd == TEX_ITEM ) {
    fprintf( f, "\\item %s\n", buffer );
  } else {
    fprintf( f, buffer );
  }
}

void start( void *data, const char *el, const char **attr ) {
  if ( data == NULL ) {
    return;
  }

  parser_context_t *pc = (parser_context_t*)data;

  FILE *f = pc->f;

  if ( strcmp( el, "text:h" ) == 0 ) {
    unsigned int outline_level = atoi( get_attribute_value(attr,"text:outline-level") );
    switch(outline_level) {
      case 1:  pc->cmd = TEX_SECTION;    break;
      case 2:  pc->cmd = TEX_SUBSECTION; break;
      case 3:  pc->cmd = TEX_SUBSECTION; break;
      default: pc->cmd = TEX_SECTION;    break;
    }
  } else
  if ( strcmp( el, "text:list" ) == 0 ) {
    fprintf( f, "\\begin{itemize}\n" );
    pc->env = ENV_LIST;
  } else
  if ( strcmp( el, "text:p" ) == 0 && pc->env == ENV_LIST ) {
    pc->cmd = TEX_ITEM;
  } else
  if ( strcmp( el, "text:soft-page-break" ) == 0 ) {
  } else {
    pc->cmd = TEX_DEFAULT;
  }
}

void end( void *data, const char *el ) {
  if ( data == NULL ) {
    return;
  }

  parser_context_t *pc = (parser_context_t*)data;

  FILE *f = pc->f;
  if ( strcmp( el, "text:list" ) == 0 ) {
    pc->env = ENV_DEFAULT;
    fprintf( f, "\\end{itemize}\n\n" );
  } else
  if ( strcmp( el, "text:p" ) == 0 && pc->env == ENV_DEFAULT ) {
    fprintf( f, "\n\n" );
  }
}
