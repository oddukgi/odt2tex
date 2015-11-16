#include <stdio.h>
#include <zip.h>
#include <expat.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <errno.h>
#include <string.h>

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

  char *infile = argv[1];
  char *outdir = argv[2];
  int rc = -1;

  fprintf( stdout, "  >> Processing: %s to Directory: %s\n", infile, outdir );

  opendir(outdir);
  if ( errno == ENOENT ) {
    fprintf( stdout, "  !! Output Directory does not exist, creating...\n" );
    rc = mkdir( outdir, S_IRWXU );
    if ( rc < 0 ) {
      fprintf( stderr, "  !! Unable to create output directory: %s\n", strerror(errno) );
      return -1;
    }
  }

  fprintf( stdout, "  >> Output Directory OK\n" );
  return 0;
}
