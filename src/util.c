#include "util.h"

int escape_to_stream_pre_post( FILE *f,
    const char *pre, const char *post, char *buffer ) {

  fprintf( f, pre );
  int n = escape_to_stream( f, buffer );
  fprintf( f, post );
  return n;
}

int escape_to_stream( FILE *f, char* buffer ) {
  int n_escaped_chars = 0;

  char *p = buffer;
  while ( *p ) {
    char C = *p;

    switch( C )
    {
      case '~':
      case '{':
      case '}':
      case '^':
      case '%':
      case '$':
      case '#':
      case '&':
      case '_':
        fprintf( f, "\\%c", C );
        n_escaped_chars++;
        break;
      case '\\':
        fprintf( f, "\\textbackslash" );
        n_escaped_chars++;
        break;
      default:
        fprintf( f, "%c", C );
    }
    p++;
  }

  return n_escaped_chars;
}
