#include "main.h"

int extract_file_to( zip_t* zipfile, const char* filename, const char *target_dir ) {
  /*
  zip_file_t *f = zip_fopen( zipfile, filename, ZIP_FL_UNCHANGED );
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
  */
  return -1;
}

/* print the program usage
 *
 * @param char* prog_name The program name to show in the usage
 */
void usage( char *prog_name ) {
  fprintf( stdout,
    "  Usage:\n"
    "    %s in={infile} out={directory} enc={encoding} lang={language}\n\n"
    "  Where:\n"
    "    infile: Input ODT file containing a content.xml\n"
    "       out: Output directory to write the files to\n"
    "       enc: TeX Encoding (defaults to UTF-8)\n"
    "      lang: Language for Babel Hyphenation (defaults to en)\n"
    "\n\n"
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
 * @param struct element *arg_list argument linked list to append the
 * arguments to
 * @return int number of arguments parsed
 */
int parse_options( int argc, char** argv, struct element *arg_list ) {
  int n = 0;
  int i;
  for ( i=0; i<argc; i++ ) {
    char *key = strtok( argv[i], "=" );
    char *value = strtok( NULL, "=" );

    arg_list = append( arg_list, key );
    arg_list = append( arg_list, value );

    n++;
  }
  return n;
}

/* Get argument from a linked list by given key
 *
 * @param struct element *arguments Linked list of arguments
 * @param const char *arg_name key to search
 * @returns pointer to the string value of the corresponding key, NULL
 * if key was not found in list
 */
char *get_argument( struct element *arguments, const char *arg_name ) {
  struct element *e = find( arguments, arg_name );
  if ( e && e->next && e->next->data )
    return (char*)e->next->data;
  return NULL;
}

int main( int argc, char *argv[] ) {

  struct element *arguments = create_root();
  struct element *arguments_current = arguments;
  int nargs = parse_options( argc, argv, arguments_current );

  fprintf( stdout, "\n ODT2TeX -- Convert ODT files to LaTeX source files\n"
      "  V 0.0.1\n"
      "  by Simon Wilper (sxw@chronowerks.de)\n"
      "  2015-11-18\n\n"
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

  fprintf( f_main, "\\documentclass{article}\n\n"
      "\\usepackage[%s]{inputenc}\n"
      "\\usepackage[%s]{babel}\n\n"
      "\\begin{document}\n\n",
      encoding,
      babel
      );

  parser_context_t pc;
  pc.styles = map_create();
  pc.styles_current = pc.styles;
  pc.current_list_style_type = -1;
  pc.f = f_main;
  pc.cmd = -1;
  pc.env = -1;
  pc.current_list_level = 0;

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

  free_all(arguments);
  map_free_all(pc.styles);
  fclose(f_main);
  zip_fclose(contents_xml);
  zip_close(odt);

  return 0;
}
