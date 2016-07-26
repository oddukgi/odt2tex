#include "mathmlparser.h"

void start_precheck( void *data, const char* name, const char** attr ) {
  if ( data == NULL ) {
    return;
  }

  parser_context_precheck_t *pc = (parser_context_precheck_t*)data;

  if ( strcmp( name, "math" ) == 0 ) {
    pc->math_flavour = MATH_FLAV_ODF;
  } else
  if ( strcmp( name, "mml:math" ) == 0 ) {
    pc->math_flavour = MATH_FLAV_OOX;
  } else
  if ( strcmp( name, "mtr" ) == 0 ||
       strcmp( name, "mml:mtr" ) == 0 ) {
    pc->line_count++;
  }

}

void chars_odf( void* data, const char* chars, int len ) {
  if ( data == NULL ) {
    return;
  }

  parser_context_math_t *pc = (parser_context_math_t*)data;

  if ( pc->ignore == 1 ) {
    return;
  }

  char buffer[len+1];
  memset( buffer, 0, len+1 );
  strncpy( buffer, chars, len );

  if ( strcmp( buffer, "⋅" ) == 0 ) {
    fprintf( pc->f, "\\cdot" );
  } else
  if ( strcmp( buffer, "=" ) == 0 ) {
    fprintf( pc->f, "& = &" );
  } else {
    fprintf( pc->f, "%s", buffer );
  }

}

void start_oox( void* data, const char* name, const char** attr ) {
  if ( data == NULL ) {
    return;
  }

  parser_context_math_t *pc = (parser_context_math_t*)data;

  if ( strcmp( name, "mml:mtr" ) == 0 ) {
    pc->line_count++;
  } else
  if ( strcmp( name, "mml:msub" ) == 0 ) {
    pc->math_env = MATH_ENV_SUB;
    pc->row_index = 0;
    pc->nesting_level++;
  } else
  if ( strcmp( name, "mml:msup" ) == 0 ) {
    pc->math_env = MATH_ENV_SUP;
    pc->row_index = 0;
    pc->nesting_level++;
  } else
  if ( strcmp( name, "mml:msqrt" ) == 0 ) {
    pc->math_env = MATH_ENV_SQRT;
    pc->row_index = 0;
    pc->item_index = 0;
    fprintf( pc->f, "\\sqrt{" );
    pc->nesting_level++;
  } else
  if ( strcmp( name, "mml:mfrac" ) == 0 ) {
    pc->math_env = MATH_ENV_FRAC;
    pc->row_index = 0;
    fprintf( pc->f, "\\frac" );
    pc->nesting_level++;
  } else
  if ( strcmp( name, "mml:mrow" ) == 0 ) {
    pc->row_index++;
    pc->global_row_index++;
    if ( pc->math_env == MATH_ENV_SUB && pc->row_index == 2 ) {
      fprintf( pc->f, "_{" );
    } else
    if ( pc->math_env == MATH_ENV_SUP && pc->row_index == 2 ) {
      fprintf( pc->f, "^{" );
    } else
    if ( pc->math_env == MATH_ENV_FRAC ) {
      fprintf( pc->f, "{" );
    }
  } else
  if ( strcmp( name, "mml:mo" ) == 0 ) {
    fprintf( pc->f, " " );
  }

  if ( pc->nesting_level < 128 )
    pc->env_stack[pc->nesting_level] = pc->math_env;
}

/** Callback function for end tag
 * @param void* data Parser Context Data
 * @param const char* name Tagname
 */
void end_oox( void* data, const char* name ) {
  if ( data == NULL ) {
    return;
  }

  parser_context_math_t *pc = (parser_context_math_t*)data;

  if (
      strcmp( name, "mml:msub" ) == 0 ||
      strcmp( name, "mml:msup" ) == 0 ||
      strcmp( name, "mml:msqrt" ) == 0 ||
      strcmp( name, "mml:mfrac" ) == 0
     )
  {
    pc->row_index--;
    pc->nesting_level--;
    pc->math_env = pc->env_stack[pc->nesting_level];

    fprintf( pc->f, "}" );
  } else
  if ( strcmp( name, "mml:mrow" ) == 0 ) {
    pc->global_row_index--;
    if ( pc->global_row_index == 0 && pc->line_count < pc->lines ) {
      fprintf( pc->f, " \\\\\n" );
    }

    if ( pc->math_env == MATH_ENV_FRAC && pc->row_index < 2) {
      fprintf( pc->f, "}" );
    }
  } else
  if ( strcmp( name, "mml:mo" ) == 0 ) {
    fprintf( pc->f, " " );
  }
}

void start_odf( void* data, const char* name, const char** attr ) {
  if ( data == NULL ) {
    return;
  }

  parser_context_math_t *pc = (parser_context_math_t*)data;

  if ( strcmp( name, "msub" ) == 0 ) {
    pc->math_env = MATH_ENV_SUB;
    pc->row_index = 0;
    pc->nesting_level++;
    pc->item_index = 0;
    if ( pc->nesting_level > 1 && pc->env_stack[pc->nesting_level-1] == MATH_ENV_FRAC ) {
      fprintf( pc->f, "{" );
    }
  } else
  if ( strcmp( name, "msup" ) == 0 ) {
    pc->math_env = MATH_ENV_SUP;
    pc->row_index = 0;
    pc->nesting_level++;
    pc->item_index = 0;
    if ( pc->nesting_level > 1 && pc->env_stack[pc->nesting_level-1] == MATH_ENV_FRAC ) {
      fprintf( pc->f, "{" );
    }
  } else
  if ( strcmp( name, "msqrt" ) == 0 ) {
    fprintf( pc->f, "\\sqrt{" );
    pc->math_env = MATH_ENV_SQRT;
    pc->row_index = 0;
    pc->item_index = 0;
    pc->nesting_level++;
    pc->item_index = 0;
    if ( pc->nesting_level > 1 && pc->env_stack[pc->nesting_level-1] == MATH_ENV_FRAC ) {
      fprintf( pc->f, "{" );
    }
  } else
  if ( strcmp( name, "mfrac" ) == 0 ) {
    fprintf( pc->f, "\\frac" );
    pc->math_env = MATH_ENV_FRAC;
    pc->row_index = 0;
    pc->nesting_level++;
    pc->item_index = 0;
  } else
  if ( strcmp( name, "annotation" ) == 0 ) {
    pc->ignore = 1;
  } else
  if ( strcmp( name, "mo" ) == 0 ) {
    fprintf( pc->f, " " );
  } else
  if (
    (
     strcmp( name, "mi" ) == 0 ||
     strcmp( name, "mn" ) == 0
    ) &&
    (
     pc->math_env > 0
    )
   ) {
    if ( pc->math_env == MATH_ENV_FRAC ) {
      fprintf( pc->f, "{" );
    }
    pc->item_index++;

    if ( pc->item_index == 2 ) {
      switch( pc->math_env ) {
        case MATH_ENV_SUB:
          fprintf( pc->f, "_{" );
          break;
        case MATH_ENV_SUP:
          fprintf( pc->f, "^{" );
          break;
        default:
          break;
      }
    }
  }

  if ( pc->nesting_level < 128 )
    pc->env_stack[pc->nesting_level] = pc->math_env;
}

void end_odf( void* data, const char* name ) {
  if ( data == NULL ) {
    return;
  }

  parser_context_math_t *pc = (parser_context_math_t*)data;

  if (
      strcmp( name, "msub" ) == 0 ||
      strcmp( name, "msup" ) == 0 ||
      strcmp( name, "msqrt" ) == 0
     )
  {
    fprintf( pc->f, "}" );
    pc->nesting_level--;
    pc->math_env = pc->env_stack[pc->nesting_level];
    if ( pc->math_env == MATH_ENV_FRAC ) {
      fprintf( pc->f, "}" );
    }
  } else
  if ( strcmp( name, "mfrac" ) == 0 ) {
    pc->nesting_level--;
    pc->math_env = pc->env_stack[pc->nesting_level];
  } else
  if ( strcmp( name, "mtr" ) == 0 ) {
    pc->line_count++;
    if ( pc->line_count < pc->lines ) {
      fprintf( pc->f, " \\\\\n" );
    }
  } else
  if ( strcmp( name, "annotation" ) == 0 ) {
    pc->ignore = 0;
  } else
  if ( strcmp( name, "mo" ) == 0 ) {
    fprintf( pc->f, " " );
  } else
  if ( strcmp( name, "mn" ) == 0 || strcmp( name, "mi" ) == 0 ) {
    if ( pc->math_env == MATH_ENV_FRAC ) {
      fprintf( pc->f, "}" );
    }
  }
}
