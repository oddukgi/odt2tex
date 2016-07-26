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
#include "util.h"

#define  BUF_SIZE 4096

#define   VERSION "0.0.12"
#define      DATE "2016-07-25"
#define BUILDNAME "Kington"

enum list_style_type {
  LST_BULLET = 0,
  LST_NUMBER
};

enum tex_command {
  TEX_DEFAULT = 0,
  TEX_CHAPTER,
  TEX_SECTION,
  TEX_SUBSECTION,
  TEX_SUBSUBSECTION,
  TEX_ITEM
};

enum tex_environment {
  ENV_DEFAULT = 0,
  ENV_LIST,
  ENV_FRAME,
  ENV_TABLE,
  ENV_TABLE_ROW,
  ENV_TABLE_CELL,
  ENV_TABLE_CAPTION,
  ENV_EQUATION,
  ENV_IMAGE
};

enum text_style {
  TXT_BOLD       = 1,
  TXT_ITALIC     = 2,
  TXT_UNDERLINE  = 4,
  TXT_SMALLCAPS  = 8
};

enum style_group {
  STY_NONE = 0,
  STY_TABLE = 1
};

typedef struct parser_context {
  FILE *f;
  zip_t *odt;
  char *current_xml_tag;
  unsigned int cmd;
  unsigned int env;

  struct map *styles;
  struct map *styles_current;
  int current_list_style_type;
  int current_list_level;
  int current_frame_level;
  double current_frame_width;
  char *last_frame_chars;
  char *table_caption;
  int caption_string_offset;
  int graphics_count;
  char *imgdir;
  struct map *text_styles;
  struct map *text_styles_current;
  int span_level;
  int table_column_width_mm;
  struct list *table;
  struct list *table_current;
  int table_count;
  char *float_pos;
  int style_group;

} parser_context_t;

int extract_file_to( zip_t*, const char*, const char* );
int extract_all_files( zip_t*, const char*, const char* );

#endif
