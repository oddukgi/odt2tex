#include <stdio.h>
#include <zip.h>
#include <expat.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <errno.h>
#include <string.h>

#include "main.h"

#define BUF_SIZE 4096

const char *get_attribute_value( const char **attr, const char *key ) {
  unsigned int idx = 0;
  while ( attr[idx] != NULL ) {
    //fprintf( stdout, "Checking for [%s] [%s]\n", attr[idx], key );
    if ( strcmp(attr[idx],key) == 0 ) {
      //fprintf( stdout, "[%s] [%s]\n", attr[idx], attr[idx+1] );
      return attr[idx+1];
    }
    idx+=2;
  }

  return NULL;
}

void chars( void *data, const char *s, int len ) {
  if ( data == NULL ) {
    return;
  }

  parser_context_t *pc = (parser_context_t*)data;

  FILE *f = pc->f;

  char buffer[len+1];
  memset( buffer, 0, len+1 );
  strncpy( buffer, s, len );

  if ( pc->cmd == TEX_SECTION ) {
    fprintf( f, "\n\\section{%s}\n", buffer );
  } else
  if ( pc->cmd == TEX_SUBSECTION ) {
    fprintf( f, "\n\\subsection{%s}\n", buffer );
  } else
  if ( pc->cmd == TEX_SUBSUBSECTION ) {
    fprintf( f, "\n\\subsubsection{%s}\n", buffer );
  } else
  if ( pc->cmd == TEX_ITEM ) {
    fprintf( f, "\\item %s\n", buffer );
  } else {
    fprintf( f, "[%s]\n", buffer );
  }
}

void start( void *data, const char *el, const char **attr ) {
  if ( data == NULL ) {
    return;
  }

  parser_context_t *pc = (parser_context_t*)data;

  FILE *f = pc->f;

  if ( strcmp( el, "text:h" ) == 0 ) {
    unsigned int outline_level = atoi( get_attribute_value(attr,"text:outline-level") );
    switch(outline_level) {
      case 1:  pc->cmd = TEX_SECTION;    break;
      case 2:  pc->cmd = TEX_SUBSECTION; break;
      case 3:  pc->cmd = TEX_SUBSECTION; break;
      default: pc->cmd = TEX_SECTION;    break;
    }
  } else
  if ( strcmp( el, "text:list" ) == 0 ) {
    fprintf( f, "\n\\begin{itemize}\n" );
    pc->env = ENV_LIST;
  } else
  if ( strcmp( el, "text:p" ) == 0 && pc->env == ENV_LIST ) {
    pc->cmd = TEX_ITEM;
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
    pc->env = ENV_DEFAULT;
    fprintf( f, "\\end{itemize}\n\n" );
  }
}

void usage( char *prog_name ) {
  fprintf( stdout,
    "  Usage:\n"
    "  %s [infile.odt] [output directory]\n\n"
    , prog_name
    );
}

int main( int argc, char *argv[] ) {
  fprintf( stdout, "\n ODT2TeX -- Convert ODT files to LaTeX source files\n"
      "  V 0.0.1\n"
      "  by Simon Wilper (sxw@chronowerks.de)\n"
      "  2015-11-18\n\n"
      );

  if ( argc < 3 ) {
    usage( argv[0] );
    return -1;
  }

  const char *infile = argv[1];
  const char *outdir = argv[2];
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

  fprintf( stdout, "  >> ODT file OK\n" );

  const char *contents_name = "content.xml";

  zip_file_t *contents_xml = zip_fopen( odt, contents_name, ZIP_FL_UNCHANGED );
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

  fprintf( f_main, "\\documentclass{article}\n"
      "\\begin{document}\n"
      );

  parser_context_t pc;
  pc.f = f_main;

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

  fclose(f_main);
  zip_fclose(contents_xml);
  zip_close(odt);

  return 0;
}
