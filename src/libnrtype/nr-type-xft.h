#ifndef __NR_TYPE_XFT_H__
#define __NR_TYPE_XFT_H__

/*
 * Typeface and script library
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * This code is in public domain
 */

#include <libnr/nr-macros.h>

#include "nr-type-primitives.h"
#include "nr-type-ft2.h"

void nr_type_xft_typefaces_get (NRNameList *names);
void nr_type_xft_families_get (NRNameList *names);

void nr_type_xft_build_def (NRTypeFaceDefFT2 *dft2, const unsigned char *name, const unsigned char *family);

void nr_type_read_xft_list (void);

#endif
