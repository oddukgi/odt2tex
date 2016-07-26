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
    escape_to_stream_pre_post( f, "\\chapter{", "}\n", buffer );
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
  if ( strcmp( el, "draw:object" ) == 0 ) {
    pc->env = ENV_EQUATION;
    const char *object_name = get_attribute_value( attr, "xlink:href" );

    char object_name_rel[BUF_SIZE];
    memset( object_name_rel, 0, BUF_SIZE );

    if ( object_name[0] == '.' && object_name[1] == '/' ) {
      char *object_name_stripped = (char*)object_name+2;
      strcat( object_name_rel, object_name_stripped );
    } else {
      strcat( object_name_rel, object_name );
    }

    if ( object_name_rel[strlen(object_name_rel)-1] != '/' ) {
      strcat( object_name_rel, "/content.xml" );
    } else {
      strcat( object_name_rel, "content.xml" );
    }

    zip_file_t *object_contents_xml = zip_fopen(
        pc->odt,
        object_name_rel,
        ZIP_FL_UNCHANGED
        );
    if ( object_contents_xml == NULL ) {
      int zep = 0;
      int sep = 0;
      char buffer[BUF_SIZE];
      zip_error_get( pc->odt, &zep, &sep );
      zip_error_to_str( buffer, BUF_SIZE, zep, sep );
      fprintf( stderr, "  !! Unable to open %s (%s): %s\n",
          object_name_rel,
          object_name,
          buffer
        );
      return;
    }

    zip_stat_t stat_info;
    zip_stat(
        pc->odt,
        object_name_rel,
        ZIP_FL_ENC_GUESS | ZIP_FL_UNCHANGED,
        &stat_info
      );
  
    unsigned long content_length = stat_info.size;

    fprintf( stdout, "  >> Ecountered draw:object %s of size %ld\n",
        object_name_rel,
        content_length
        );
  
    void *buffer = malloc(content_length);
    unsigned long bytes_read = zip_fread(
        object_contents_xml,
        buffer,
        content_length
        );
    zip_fclose( object_contents_xml );
  
    if ( bytes_read != content_length ) {
      zip_fclose(object_contents_xml);
      free(buffer);
      fprintf( stderr, "  !! Content fragmented\n" );
      return;
    }

    parser_context_math_t mpc;
    mpc.f = pc->f;
    mpc.math_env = 0;
    mpc.row_index = 0;
    mpc.global_row_index = 0;
    mpc.nesting_level = 0;
    for ( int i = 0; i<128; i++ ) mpc.env_stack[i] = MATH_ENV_NONE;
    mpc.item_index = 0;

    parser_context_precheck_t pc_pre;
    pc_pre.line_count = 0;
    pc_pre.math_flavour = -1;

    XML_Parser p_pre = XML_ParserCreate("UTF-8");
    XML_SetUserData( p_pre, &pc_pre );
    XML_SetElementHandler( p_pre, start_precheck, NULL );

    XML_Parse( p_pre, buffer, bytes_read, 1 );
    XML_ParserFree(p_pre);

    mpc.lines = pc_pre.line_count;
    mpc.line_count = 0;
    mpc.math_flavour = pc_pre.math_flavour;

    XML_Parser p = XML_ParserCreate("UTF-8");
    XML_SetUserData( p, &mpc );

    if ( mpc.math_flavour == MATH_FLAV_ODF ) {
      XML_SetElementHandler( p, start_odf, end_odf );
    } else
    if ( mpc.math_flavour == MATH_FLAV_OOX ) {
      XML_SetElementHandler( p, start_oox, end_oox );
    } else {
      fprintf( stderr, "  !! ERROR: Math flavour not detected\n" );
      return;
    }

    XML_SetCharacterDataHandler( p, chars_odf );

    fprintf( f, "\\begin{IEEEeqnarray}{rCl}\n" );
    XML_Parse( p, buffer, bytes_read, 1 );
    fprintf( f, "\n\\end{IEEEeqnarray}\n" );

    XML_ParserFree(p);
    free(buffer);

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

      if ( pc->env != ENV_EQUATION ) {
        fprintf( f, "  \\label{fig%d}\n", pc->graphics_count );
        fprintf( f, "\\end{figure}\n\n" );
      }
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
