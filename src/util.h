#ifndef __util_h
#define __util_h

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

char *str_append( char* a, char *b );
int escape_to_stream( FILE *f, char* buffer );
int escape_to_stream_pre_post( FILE *f,
    const char *pre, const char *post, char *buffer );

#endif
