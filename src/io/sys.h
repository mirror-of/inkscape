#ifndef SEEN_SYS_H
#define SEEN_SYS_H

/*
 * System abstraction utility routines
 *
 * Authors:
 *   Jon A. Cruz <jon@joncruz.org>
 *
 * Copyright (C) 2004-2005 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <stdio.h>
#include <glib.h>

/*#####################
## U T I L I T Y
#####################*/

namespace Inkscape {
namespace IO {

void dump_fopen_call( char const *utf8name, char const *id );

FILE *fopen_utf8name( char const *utf8name, char const *mode );

int mkdir_utf8name( char const *utf8name );

bool file_test( char const *utf8name, GFileTest test );

gchar* sanitizeString( gchar const * str );

}
}


#endif // SEEN_SYS_H
