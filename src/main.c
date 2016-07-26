#include "main.h"

int extract_file_to(
    zip_t* zipfile, const char* filename, const char *target )
{
  fprintf( stdout, "  >> Extracting %s to %s\n", filename, target );
  zip_file_t *f = zip_fopen( zipfile, filename, ZIP_FL_UNCHANGED );
  if ( f == NULL ) {
    int zep = 0;
    int sep = 0;
    char buffer[BUF_SIZE];
    zip_error_get( zipfile, &zep, &sep );
    zip_error_to_str( buffer, BUF_SIZE, zep, sep );
    fprintf( stderr, "  !! Unable to open %s: %s\n",
        filename,
        buffer
      );
    return -1;
  }

  zip_stat_t stat_info;
  zip_stat( zipfile,
      filename, ZIP_FL_ENC_GUESS|ZIP_FL_UNCHANGED, &stat_info );

  unsigned long content_length = stat_info.size;

  void *buffer = malloc(content_length);

  unsigned long bytes_read = zip_fread( f, buffer, content_length );

  if ( bytes_read != content_length ) {
    zip_fclose(f);
    free(buffer);
    fprintf( stderr, "  !! Content fragmented\n" );
    return -1;
  }

  FILE *f_out = fopen( target, "wb" );
  if ( f_out == NULL ) {
    free(buffer);
    fprintf( stderr, "  !! Unable to open output file: %s\n",
        strerror( errno ) );
    return -1;
  }
  unsigned long bytes_written = fwrite(
      buffer, 1, content_length, f_out );
  fclose(f_out);
  zip_fclose(f);

  if ( content_length != bytes_written ) {
    fprintf( stderr,
        "  !! Content (%ld) differs from Bytes written (%ld).\n",
       content_length, bytes_written );
    free(buffer);
    return -1;
  }
  fprintf( stdout, "    >> %ldK written\n", bytes_written/1024 );
  free(buffer);

  return 0;
}

/** extract all files that start with prefix to target
 *
 * @param zip_t* pointer to open zip file
 * @param struct list* the list to append the items to
 * @param const char* prefix
 * @param const char* target directory
 *
 * @returns 0 on success, -1 on failure
 */
int extract_all_files(
    zip_t* zip, const char *prefix, const char *target_dir )
{
  if ( zip == NULL ) {
    return -1;
  }

  long nfiles = zip_get_num_entries( zip, ZIP_FL_UNCHANGED );

  long i;
  for ( i=0; i<nfiles; i++ ) {
    const char *zip_filename_full = zip_get_name(
        zip, i, ZIP_FL_UNCHANGED );
    if ( strstr( zip_filename_full, prefix ) ) {
      const char *zip_filename = zip_filename_full+strlen(prefix);

      char buffer[BUF_SIZE];
      memset(buffer, 0, BUF_SIZE);
      snprintf( buffer, BUF_SIZE, "%s/%s", target_dir, zip_filename );

      if ( extract_file_to( zip, zip_filename_full, buffer ) < 0 ) {
        fprintf( stderr, "  !! Failed\n" );
      }
    }
  }

  return 0;

}

/* print the program usage
 *
 * @param char* prog_name The program name to show in the usage
 */
void usage( char *prog_name ) {
  fprintf( stdout,
    "  Usage:\n"
    "    %s in={infile} out={directory}\n\n"
    "  Where:\n"
    "               in: Input ODT file containing a content.xml\n"
    "              out: Output directory to write the files to\n"
    "              enc: TeX Encoding (defaults to UTF-8)\n"
    "             lang: Language for Babel Hyphenation (defaults to en)\n"
    "    captionoffset: Number of characters to strip from\n"
    "                   caption (0-128, default 2)\n"
    "    tablecolwidth: Table column Width (0-200, default 30)\n"
    "         floatpos: Float Position (default H)\n"
    "\n"
    , prog_name
    );
}

/* Parse the command line options. All options are given as key=value
 * pairs so we can easily use strtok on the elements and then push it to
 * a linked list. Since I was too lazy to implement a hashtable I push
 * key and value one after another so that if we want to find a value we
 * can get the value from the next of the key element.
 *
 * @param int argc number of arguments
 * @param char** argv array of argument strings
 * @param struct list *arg_list argument linked list to append the
 * arguments to
 * @return int number of arguments parsed
 */
int parse_options( int argc, char** argv, struct list *arg_list ) {
  int n = 0;
  int i;
  for ( i=0; i<argc; i++ ) {
    char *key = strtok( argv[i], "=" );
    char *value = strtok( NULL, "=" );

    arg_list = list_append( arg_list, key );
    arg_list = list_append( arg_list, value );

    n++;
  }
  return n;
}

/* Get argument from a linked list by given key
 *
 * @param struct list *arguments Linked list of arguments
 * @param const char *arg_name key to search
 * @returns pointer to the string value of the corresponding key, NULL
 * if key was not found in list
 */
char *get_argument( struct list *arguments, const char *arg_name ) {
  struct list *e = list_find( arguments, arg_name );
  if ( e && e->next && e->next->data )
    return (char*)e->next->data;
  return NULL;
}

int main( int argc, char *argv[] ) {
  struct list *arguments = list_create();
  struct list  *arguments_current = arguments;
  int nargs = parse_options( argc, argv, arguments_current );

  fprintf( stdout, "\n ODT2TeX -- Convert ODT files to LaTeX source files\n"
      "  V " VERSION " (" BUILDNAME ") - " DATE "\n"
      "  by Simon Wilper (sxw@chronowerks.de)\n"
      "\n"
      );

  if ( nargs < 3 ) {
    usage( argv[0] );
    return -1;
  }

  char *infile = get_argument( arguments, "in" );
  char *outdir = get_argument( arguments, "out" );

  if ( infile == NULL || outdir == NULL ) {
    usage( argv[0] );
    return -1;
  }

  char *encoding = get_argument( arguments, "enc" );
  if ( encoding == NULL ) {
    encoding = "utf8";
  }

  char *babel = get_argument( arguments, "babel" );
  if ( babel == NULL ) {
    babel = "english";
  }

  int rc = -1;

  fprintf( stdout, "  >> Processing: %s to Directory: %s\n", infile, outdir );

  DIR *d = opendir(outdir);
  if ( errno == ENOENT ) {
    fprintf( stdout, "  !! Output Directory does not exist, creating...\n" );
    rc = mkdir( outdir, S_IRWXU );
    if ( rc < 0 ) {
      fprintf( stderr, "  !! Unable to create output directory: %s\n", strerror(errno) );
      return -1;
    }
  }

  closedir(d);

  fprintf( stdout, "  >> Output Directory OK\n" );

  char imgdir[128];
  snprintf( imgdir, 128, "%s/%s", outdir, "img" );
  rc = mkdir( imgdir, S_IRWXU );
  if ( rc < -1 ) {
    fprintf( stderr, "  !! Unable to create image output directory: %s\n", strerror(errno) );
    return -1;
  }
  fprintf( stdout, "  >> Image Directory OK\n" );

  int zip_error = 0;
  zip_t *odt = zip_open( infile, ZIP_RDONLY, &zip_error );
  if ( odt == NULL ) {
    zip_error_t error;
    zip_error_init_with_code( &error, zip_error );
    fprintf( stderr, "  !! Unable to open ODT file: %s\n",
        zip_error_strerror(&error)
      );
    return -1;
  }

  fprintf( stdout, "  >> Document file OK\n" );

  // extract all pictures to imgdir
  extract_all_files( odt, "Pictures/", imgdir );

  char *contents_name = "content.xml";

  zip_file_t *contents_xml = zip_fopen( odt, contents_name, ZIP_FL_UNCHANGED );
  if ( contents_xml == NULL ) {
    contents_name = "word/document.xml";
    contents_xml = zip_fopen( odt, contents_name, ZIP_FL_UNCHANGED );
    if ( contents_xml == NULL ) {
      int zep = 0;
      int sep = 0;
      char buffer[BUF_SIZE];
      zip_error_get( odt, &zep, &sep );
      zip_error_to_str( buffer, BUF_SIZE, zep, sep );
      fprintf( stderr, "  !! Unable to open %s: %s\n",
          contents_name,
          buffer
        );
      zip_close(odt);
      return -1;
    }
  }

  zip_stat_t stat_info;
  zip_stat( odt, contents_name, ZIP_FL_ENC_GUESS|ZIP_FL_UNCHANGED, &stat_info );

  unsigned long content_length = stat_info.size;

  char buffer[content_length+1];
  memset( buffer, 0, content_length+1 );

  unsigned long bytes_read = zip_fread( contents_xml, buffer, content_length );

  if ( bytes_read != content_length ) {
    zip_fclose(contents_xml);
    zip_close(odt);
    fprintf( stderr, "  !! Content fragmented\n" );
    return -1;
  }

  char buf_main_file[BUF_SIZE];
  memset(buf_main_file, 0, BUF_SIZE);
  snprintf( buf_main_file, BUF_SIZE, "%s/main.tex", outdir );
  FILE *f_main = fopen( buf_main_file, "w" );

  if ( f_main == NULL ) {
    zip_fclose(contents_xml);
    zip_close(odt);
    fprintf( stderr, "  !! Unable to open main tex output file: %s\n",
        strerror(errno)
      );
    return -1;
  }

  fprintf( f_main, "\\documentclass{report}\n\n"
      "\\usepackage[%s]{inputenc}\n"
      "\\usepackage{graphicx}\n"
      "\\usepackage{float}\n"
      "\\usepackage{textcomp}\n"
      "\\usepackage[retainorgcmds]{IEEEtrantools}\n"
      "\\usepackage[%s]{babel}\n\n"
      "\\begin{document}\n\n",
      encoding,
      babel
      );

  parser_context_t pc;
  pc.current_xml_tag = 0;
  pc.styles = map_create();
  pc.styles_current = pc.styles;
  pc.current_list_style_type = -1;
  pc.f = f_main;
  pc.odt = odt;
  pc.cmd = -1;
  pc.env = -1;
  pc.current_list_level = 0;
  pc.current_frame_level = 0;

  pc.last_frame_chars = (char*)malloc(128);
  memset( pc.last_frame_chars, 0, 128 );

  pc.table_caption = (char*)malloc(128);
  memset( pc.table_caption, 0, 128 );

  memset(pc.last_frame_chars,0,128);
  pc.current_frame_width = 0.0;
  pc.graphics_count = 0;
  pc.imgdir = "img";
  pc.caption_string_offset = 2;
  pc.text_styles = map_create();
  pc.text_styles_current = pc.text_styles;
  pc.span_level = 0;
  pc.table_column_width_mm = 30;
  pc.table = 0;
  pc.table_count = 0;
  pc.table_current = 0;
  pc.float_pos = 0;
  pc.style_group = STY_NONE;

  char *str_float_pos = get_argument( arguments, "floatpos" );
  if ( str_float_pos == NULL ) {
    str_float_pos = "H";
  }
  pc.float_pos = str_float_pos;

  char *str_table_column_width_mm = get_argument( arguments, "tablecolwidth" );
  if ( str_table_column_width_mm != NULL ) {
    int table_column_width_mm = atoi(str_table_column_width_mm);
    if ( table_column_width_mm >= 0 && table_column_width_mm < 200 ) {
      pc.table_column_width_mm = table_column_width_mm;
    }
  }
  
  char *caption_offset = get_argument( arguments, "captionoffset" );
  if ( caption_offset != NULL ) {
    int offset = atoi(caption_offset);
    if ( offset >= 0 && offset < 128 ) {
      pc.caption_string_offset = offset;
    }
  }

  XML_Parser p = XML_ParserCreate("UTF-8");
  XML_SetUserData( p, &pc );
  XML_SetElementHandler( p, start, end );
  XML_SetCharacterDataHandler( p, chars );
  int parse_rc = XML_Parse( p, buffer, content_length, 1 );

  if ( parse_rc == 0 ) {
    fprintf( stderr, "  !! Parse Failed: %d %s\n",
        parse_rc,
        XML_ErrorString(XML_GetErrorCode(p))
      );
  }

  XML_ParserFree(p);
  fprintf( stdout, "  >> Done\n" );

  fprintf( f_main, "\\end{document}\n" );

  free( pc.table_caption );
  free( pc.last_frame_chars );
  list_free(arguments);
  map_free_all(pc.styles);
  map_free_all(pc.text_styles);
  list_free( pc.table );
  fclose(f_main);
  zip_fclose(contents_xml);
  zip_close(odt);

  return 0;
}
