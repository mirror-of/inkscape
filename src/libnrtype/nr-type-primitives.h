#ifndef __NR_TYPE_PRIMITIVES_H__
#define __NR_TYPE_PRIMITIVES_H__

/*
 * Typeface and script library
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   g++ port: Nathan Hurst <njh@mail.csse.monash.edu.au>
 *
 * This code is in public domain
 */

#include <glib.h>

struct NRNameList;
struct NRTypeDict;

typedef void (* NRNameListDestructor) (NRNameList *list);

struct NRNameList {
	unsigned long length;
      guchar **names;
      guchar **families;
	NRNameListDestructor destructor;
};

void nr_name_list_release (NRNameList *list);

NRTypeDict *nr_type_dict_new (void);

void nr_type_dict_insert (NRTypeDict *td, const gchar *key, void *val);

void *nr_type_dict_lookup (NRTypeDict *td, const gchar *key);

#endif
