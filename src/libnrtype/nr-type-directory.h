#ifndef __NR_TYPE_DIRECTORY_H__
#define __NR_TYPE_DIRECTORY_H__

/*
 * Typeface and script library
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * This code is in public domain
 */

#include <libnrtype/nr-type-primitives.h>
#include <libnrtype/nr-typeface.h>

NRTypeFace *nr_type_directory_lookup (const unsigned char *name);
NRTypeFace *nr_type_directory_lookup_fuzzy (const unsigned char *family, const unsigned char *style);

NRNameList *nr_type_directory_family_list_get (NRNameList *flist);
NRNameList *nr_type_directory_style_list_get (const unsigned char *family, NRNameList *slist);

unsigned int nr_type_register (NRTypeFaceDef *def);
NRTypeFace *nr_type_build (const unsigned char *name, const unsigned char *family,
			   const unsigned char *data, unsigned int size, unsigned int face);

void nr_type_directory_forget_face (NRTypeFace *tf);

#endif
