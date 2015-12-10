#ifndef __main_h
#define __main_h

#include <stdio.h>
#include <stdlib.h>
#include <zip.h>
#include <expat.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <errno.h>
#include <string.h>
#include "map.h"
#include "list.h"
#include "parser.h"

#define BUF_SIZE 4096

enum list_style_type {
  LST_BULLET = 0,
  LST_NUMBER
};

enum tex_command {
  TEX_DEFAULT = 0,
  TEX_SECTION,
  TEX_SUBSECTION,
  TEX_SUBSUBSECTION,
  TEX_ITEM
};

enum tex_environment {
  ENV_DEFAULT = 0,
  ENV_LIST,
  ENV_FRAME
};

typedef struct parser_context {
  FILE *f;
  unsigned int cmd;
  unsigned int env;

  struct map *styles;
  struct map *styles_current;
  int current_list_style_type;
  int current_list_level;

  char *imgdir;

} parser_context_t;

int extract_file_to( zip_t*, const char*, const char* );
int extract_all_files( zip_t*, const char*, const char* );

#endif
