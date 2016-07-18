#include "parser.h"

/** Get an attribute from an XML element by giving a key
 * @param const char **attr attribute list
 * @param const char  *key key to get the value for
 * @returns char* value
 */
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

/** Process XML characters
 * @param void *data
 * @param const char *s
 * @param int len
 */
void chars( void *data, const char *s, int len ) {
  if ( data == NULL ) {
    return;
  }

  parser_context_t *pc = (parser_context_t*)data;

  FILE *f = pc->f;

  char buffer[len+1];
  memset( buffer, 0, len+1 );
  strncpy( buffer, s, len );

  if ( pc->cmd == TEX_CHAPTER ) {
    escape_to_stream_pre_post( f, "\\chapter{", "{\n", buffer );
  } else
  if ( pc->cmd == TEX_SECTION ) {
    escape_to_stream_pre_post( f, "\\section{", "}\n", buffer );
  } else
  if ( pc->cmd == TEX_SUBSECTION ) {
    escape_to_stream_pre_post( f, "\\subsection{", "}\n", buffer );
  } else
  if ( pc->cmd == TEX_SUBSUBSECTION ) {
    escape_to_stream_pre_post( f, "\\subsubsection{", "}\n", buffer );
  } else
  if ( pc->env == ENV_FRAME ) {
    memset( pc->last_frame_chars, 0, 128 );
    strncpy( pc->last_frame_chars, buffer, 127 );
  } else
  if ( pc->env == ENV_TABLE_CAPTION ) {
    memset( pc->table_caption, 0, 128 );
    strncpy( pc->table_caption, buffer, 127 );
  }
  else
  if ( pc->env == ENV_TABLE_CELL ) {
    if ( strlen(pc->table_current->data) > 0 ) {
      pc->table_current->data = str_append(
          pc->table_current->data, " " );
    }
    pc->table_current->data = str_append(
        pc->table_current->data, buffer );
  }
  else {
    escape_to_stream( f, buffer );
  }
}

void start( void *data, const char *el, const char **attr )
{
  if ( data == NULL ) {
    return;
  }

  parser_context_t *pc = (parser_context_t*)data;
  pc->current_xml_tag = (char*)el;

  FILE *f = pc->f;

  if ( strcmp( el, "text:h" ) == 0 ) {
    unsigned int outline_level = atoi(
        get_attribute_value(attr,"text:outline-level")
        );
    switch(outline_level) {
      case 1:  pc->cmd = TEX_CHAPTER;       break;
      case 2:  pc->cmd = TEX_SECTION;       break;
      case 3:  pc->cmd = TEX_SUBSECTION;    break;
      case 4:  pc->cmd = TEX_SUBSUBSECTION; break;
      default: pc->cmd = TEX_SECTION;       break;
    }
  } else
  if ( strcmp( el, "text:list" ) == 0 )
  {
    struct map *style = map_search(
        pc->styles, get_attribute_value(attr,"text:style-name") );
    char *tex_list_style = "itemize";
    pc->current_list_style_type = LST_BULLET;

    if ( style != NULL ) {
      switch ( style->value ) {
        case LST_NUMBER:
          tex_list_style = "enumerate";
          pc->current_list_style_type = LST_NUMBER;
          break;
        case LST_BULLET:
          tex_list_style = "itemize";
          pc->current_list_style_type = LST_BULLET;
          break;
        default:
          tex_list_style = "itemize";
          pc->current_list_style_type = LST_BULLET;
          break;
      }
    }
    fprintf( f, "\\begin{%s}\n", tex_list_style );
    pc->env = ENV_LIST;
    pc->current_list_level++;
  }
  else if (
      strcmp( el, "text:list-item" ) == 0
      )
  {
    fprintf( f, "\\item " );
  }
  else if (
      (
       strcmp( el, "text:p" ) == 0 ||
       strcmp( el, "text:span" ) == 0
      ) &&
      pc->env == ENV_LIST
    ) {
    pc->cmd = TEX_ITEM;
  } else
  if ( strcmp( el, "text:soft-page-break" ) == 0 ) {
    // ignore page breaks
  } else
  if ( strcmp( el, "text:list-style" ) == 0 ) {

    const char *stylename = get_attribute_value( attr,"style:name");

    if ( stylename != NULL ) {
      pc->styles_current = map_append(
          pc->styles_current,
          stylename,
          -1
          );
    }

  } else
  if ( strcmp( el, "text:list-level-style-bullet" ) == 0 &&
      strcmp( get_attribute_value( attr, "text:level" ), "1") == 0 ) {
    pc->styles_current->value = LST_BULLET;
  } else
  if ( strcmp( el, "text:list-level-style-number" ) == 0 &&
      strcmp( get_attribute_value( attr, "text:level" ), "1") == 0 ) {
    pc->styles_current->value = LST_NUMBER;
  } else
  if ( strcmp( el, "draw:image" ) == 0 ) {
    const char *picture_name = get_attribute_value( attr, "xlink:href" );
    if ( strstr( picture_name, "Pictures/" ) ) {
      fprintf( f, "\\begin{figure}[%s]\n",
          pc->float_pos
        );
      fprintf( f, "\\centering\n" );
      fprintf( f, "  \\includegraphics[width=%lfmm]{%s/%s}\n",
          pc->current_frame_width,
          pc->imgdir,
          strstr( picture_name, "/" )+1 );
    }
  } else
  if ( strcmp( el, "draw:frame" ) == 0 ) {
    pc->env = ENV_FRAME;
    pc->current_frame_level++;
    pc->current_frame_width = atof(
        get_attribute_value( attr, "svg:width" ) );
  } else
  if ( (strcmp( el, "style:style" ) == 0) &&
     (
      strcmp(get_attribute_value(
          attr, "style:family" ),"text") == 0 ||
      strcmp(get_attribute_value(
          attr, "style:family" ),"paragraph") == 0
     )
      ) {
    const char *stylename = get_attribute_value( attr, "style:name" );
    pc->text_styles_current = map_append( pc->text_styles_current,
        stylename, 0 );
  } else
  if ( strcmp( el, "style:text-properties" ) == 0 ) {

    const char *font_weight = get_attribute_value(
        attr, "fo:font-weight" );
    if ( font_weight != NULL && strcmp(font_weight, "bold" ) == 0 )
      pc->text_styles_current->value |= TXT_BOLD;

    const char *font_style = get_attribute_value(
        attr, "fo:font-style" );
    if ( font_style != NULL && strcmp(font_style, "italic" ) == 0 )
      pc->text_styles_current->value |= TXT_ITALIC;

    const char *text_underline = get_attribute_value(
        attr, "style:text-underline-style" );
    if ( text_underline != NULL &&
        strcmp(text_underline, "solid" ) == 0 )
      pc->text_styles_current->value |= TXT_UNDERLINE;

    const char *font_variant = get_attribute_value(
        attr, "fo:font-variant" );
    if ( font_variant != NULL && strcmp(font_variant, "small-caps" ) == 0 )
      pc->text_styles_current->value |= TXT_SMALLCAPS;

  } else
  if ( strcmp( el, "text:span" ) == 0 ) {
    const char *stylename = get_attribute_value( attr, "text:style-name" );
    struct map *result = map_search( pc->text_styles, stylename );
    if ( result != NULL ) {
      if ( (result->value & TXT_BOLD) == TXT_BOLD ) {
        fprintf( f, "\\textbf{" );
        pc->span_level++;
      }
      if ( (result->value & TXT_ITALIC) == TXT_ITALIC ) {
        fprintf( f, "\\textit{" );
        pc->span_level++;
      }
      if ( (result->value & TXT_UNDERLINE) == TXT_UNDERLINE ) {
        fprintf( f, "\\underline{" );
        pc->span_level++;
      }
      if ( (result->value & TXT_SMALLCAPS) == TXT_SMALLCAPS ) {
        fprintf( f, "\\textsc{" );
        pc->span_level++;
      }
    }
  } else
  if ( strcmp( el, "table:table" ) == 0 ) {
    pc->env = ENV_TABLE;

    fprintf( f,
        "\\begin{table}[%s]\n"
        "  \\centering\n",
        pc->float_pos
        );

    // step 1: create a new table list in parsing context table and
    // assign the current cell to table_current
    pc->table = list_create();
    pc->table_current = pc->table;
  } else
  if ( strcmp( el, "table:table-row" ) == 0 ) {
    pc->env = ENV_TABLE_ROW;
  } else
  if ( strcmp( el, "table:table-cell" ) == 0 ) {
    pc->env = ENV_TABLE_CELL;
    pc->table_current = list_append( pc->table_current, "" );
  } else
  if (
    strcmp( el, "text:p" ) == 0 &&
    strcmp( get_attribute_value( attr, "text:style-name" ), "Table") == 0
    )
  {
    pc->style_group = STY_TABLE;
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
    switch( pc->current_list_style_type ) {
      case LST_NUMBER:
        fprintf( f, "\\end{enumerate}\n\n" );
        break;
      case LST_BULLET:
        fprintf( f, "\\end{itemize}\n\n" );
        break;
      default:
        fprintf( f, "\\end{itemize}\n\n" );
        break;
    }
    pc->current_list_level--;
    if ( pc->current_list_level <= 0 )
      pc->env = ENV_DEFAULT;
  } else
  if ( strcmp( el, "text:p" ) == 0 &&
      (
       pc->env == ENV_DEFAULT || pc->env == -1
      ) )
  {
    fprintf( f, "\n\n" );
  } else
  if ( strcmp( el, "text:p" ) == 0 && pc->env == ENV_TABLE_CAPTION ) {
    pc->env = ENV_DEFAULT;
    fprintf( f,
        "  \\caption{%s}\n"
        "  \\label{tab:%d}\n"
        "\\end{table}\n",
        ( pc->style_group == STY_TABLE ?
          pc->table_caption + pc->caption_string_offset :
          "TODO: No caption"
        ),
        (++pc->table_count)
        );

    pc->style_group = STY_NONE;
  }
  if ( strcmp( el, "draw:frame" ) == 0 ) {
    pc->current_frame_level--;
    if ( pc->current_frame_level == 0 ) {
      pc->graphics_count++;
      if ( pc->last_frame_chars != NULL &&
          strlen(pc->last_frame_chars) > 0 )
        fprintf( f, "  \\caption{%s}\n",
            pc->last_frame_chars+pc->caption_string_offset );

      fprintf( f, "  \\label{fig%d}\n", pc->graphics_count );
      fprintf( f, "\\end{figure}\n\n" );
      memset( pc->last_frame_chars, 0, 128 );
      pc->env = ENV_DEFAULT;
    }
  } else
  if ( strcmp( el, "table:table" ) == 0 ) {
    pc->env = ENV_TABLE_CAPTION;

    pc->table_current = pc->table;
    if ( pc->table->next ) pc->table_current = pc->table_current->next;

    unsigned int ncols = 0;
    while(pc->table_current) {
      if ( pc->table_current->data != NULL ) ncols++; else break;
      pc->table_current = pc->table_current->next;
    }

    pc->table_current = pc->table;
    if ( pc->table->next ) pc->table_current = pc->table_current->next;

    int i;
    fprintf( f, "  \\begin{tabular}{" );
    for ( i = 0; i<ncols; i++ ) {
      fprintf( f, "p{%dmm}", pc->table_column_width_mm );
    }
    fprintf( f, "}\n" );

    i = 0;
    unsigned int nrows = 0;
    while(pc->table_current) {
      if ( pc->table_current->data != NULL ) {
        fprintf( f, "%s",
            (nrows==0?"\\bfseries ":"") );
        escape_to_stream( f,
            (char*)pc->table_current->data
            );
        i++;
        if ( i < ncols ) {
          fprintf( f, " & " );
        } else {
          i=0;
        }
      } else {
        fprintf( f, " \\\\\n" );
        if ( nrows == 0 ) fprintf( f, "\\hline\n" );
        nrows++;
      }
      pc->table_current = pc->table_current->next;
    }

    fprintf( f,
        "  \\end{tabular}\n"
        );

    list_free( pc->table );
    pc->table = 0;
  } else
  if ( strcmp( el, "table:table-row" ) == 0 ) {
    pc->env = ENV_TABLE;
    pc->table_current = list_append( pc->table_current, 0 );
  } else
  if ( strcmp( el, "table:table-cell" ) == 0 ) {
  } else
  if ( strcmp( el, "text:span" ) == 0 && pc->span_level > 0 ) {
    pc->span_level--;
    fprintf( f, "}" );
  }
}
