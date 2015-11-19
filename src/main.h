#ifndef __main_h
#define __main_h

#include <stdio.h>
#include <string.h>

enum tex_command {
  TEX_DEFAULT = 0,
  TEX_SECTION,
  TEX_SUBSECTION,
  TEX_MATH,
  TEX_FIGURE
};

typedef struct parser_context {
  FILE *f;
  int cmd;
} parser_context_t;

#endif
