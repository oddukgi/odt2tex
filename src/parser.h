#ifndef __parser_h
#define __parser_h

#include <stdio.h>
#include <stdlib.h>

#include "main.h"
#include "mathmlparser.h"

const char *get_attribute_value( const char**, const char* );
void chars( void*, const char*, int );
void start( void*, const char*, const char** );
void end( void*, const char* );

#endif
