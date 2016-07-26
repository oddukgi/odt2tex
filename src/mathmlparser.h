#ifndef __mathmlparser_h
#define __mathmlparser_h

#include <stdio.h>
#include <stdlib.h>
#include <expat.h>
#include <string.h>

#include "main.h"

enum MATH_ENV {
  MATH_ENV_NONE = 0,
  MATH_ENV_SUB,  // 1
  MATH_ENV_SUP,  // 2
  MATH_ENV_FRAC, // 3
  MATH_ENV_SQRT  // 4
};

enum MATH_FLAVOUR {
  MATH_FLAV_OOX = 0,
  MATH_FLAV_ODF
};

typedef struct parser_context_math {
  FILE *f;
  int math_env;
  int env_stack[128];
  int row_index;
  int global_row_index;
  int item_index;
  int nesting_level;
  unsigned long line_count;
  unsigned long lines;
  int ignore;
  int math_flavour;
} parser_context_math_t;

typedef struct parser_context_precheck {
  unsigned long line_count;
  int math_flavour;
} parser_context_precheck_t;

void start_precheck( void*, const char*, const char** );
void chars_odf( void*, const char*, int );

void start_oox( void*, const char*, const char** );
void end_oox( void*, const char* );

void start_odf( void*, const char*, const char** );
void end_odf( void*, const char* );

#endif

