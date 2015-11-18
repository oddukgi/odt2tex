#include <stdio.h>
#include <zip.h>
#include <expat.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <errno.h>
#include <string.h>

#define BUF_SIZE 4096

void start( void *data, const char *el, const char **attr ) {
  fprintf( stdout, "Element START: [%s]\n", el );
}

void end( void *data, const char *el ) {
  fprintf( stdout, "Element END: [%s]\n", el );
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

  zip_file_t *contents_xml = zip_fopen( odt, "content.xml", ZIP_FL_UNCHANGED );
  if ( contents_xml == NULL ) {
    int zep = 0;
    int sep = 0;
    char buffer[BUF_SIZE];
    zip_error_get( odt, &zep, &sep );
    zip_error_to_str( buffer, BUF_SIZE, zep, sep );
    fprintf( stderr, "  !! Unable to open content.xml: %s\n",
        buffer
      );
    zip_close(odt);
    return -1;
  }

  XML_Parser p = XML_ParserCreate("UTF-8");
  XML_SetElementHandler( p, start, end );

  char buffer[BUF_SIZE];
  memset( buffer, 0, BUF_SIZE );

  int bytes_read = -1;
  int parse_rc = -1;
  while ( (bytes_read = zip_fread(contents_xml,buffer,BUF_SIZE-1)) > 0 ) {
    //fprintf( stdout, buffer );
    parse_rc = XML_Parse(p,buffer,BUF_SIZE, 0 );
    fprintf( stdout, "RC=%d\n", parse_rc );
    fprintf( stderr, "Error: [%s]\n", XML_ErrorString(XML_GetErrorCode(p)));
    memset( buffer, 0, BUF_SIZE );
  }

  zip_fclose(contents_xml);
  zip_close(odt);

  return 0;
}
