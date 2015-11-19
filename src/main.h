#ifndef __main_h
#define __main_h

#include <stdio.h>
#include <string.h>

enum tex_command {
  TEX_DEFAULT = 0,
  TEX_SECTION,
  TEX_SUBSECTION,
  TEX_SUBSUBSECTION,
  TEX_ITEM
};

enum tex_environment {
  ENV_DEFAULT = 0,
  ENV_LIST
};

typedef struct parser_context {
  FILE *f;
  unsigned int cmd;
  unsigned int env;
} parser_context_t;

#endif
