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

NRTypeFace *nr_type_directory_lookup (const gchar *name);
NRTypeFace *nr_type_directory_lookup_fuzzy (const gchar *family, const gchar *style);

NRNameList *nr_type_directory_family_list_get (NRNameList *flist);
NRNameList *nr_type_directory_style_list_get (const gchar *family, NRNameList *slist);

unsigned int nr_type_register (NRTypeFaceDef *def);
NRTypeFace *nr_type_build (const gchar *name, const gchar *family,
			   const gchar *data, unsigned int size, unsigned int face);

void nr_type_directory_forget_face (NRTypeFace *tf);

#endif
