#define __NR_TYPE_PRIMITIVES_C__

/*
 * Typeface and script library
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * This code is in public domain
 */

/* This should be enough for approximately 10000 fonts */
#define NR_DICTSIZE 2777

#include <string.h>
#include <libnr/nr-macros.h>
#include "nr-type-primitives.h"

typedef struct _NRTDEntry NRTDEntry;

struct _NRTDEntry {
	NRTDEntry *next;
	const unsigned char *key;
	void *val;
};

struct _NRTypeDict {
	unsigned int size;
	NRTDEntry **entries;
};

static NRTDEntry *nr_td_entry_new (void);

void
nr_name_list_release (NRNameList *list)
{
	if (list->destructor) {
		list->destructor (list);
	}
}

NRTypeDict *
nr_type_dict_new (void)
{
	NRTypeDict *td;
	int i;

	td = nr_new (NRTypeDict, 1);

	td->size = NR_DICTSIZE;
	td->entries = nr_new (NRTDEntry *, td->size);
	for (i = 0; i < NR_DICTSIZE; i++) {
		td->entries[i] = NULL;
	}

	return td;
}

static unsigned int
nr_str_hash (const unsigned char *p)
{
	unsigned int h;

	h = *p;

	if (h != 0) {
		for (p += 1; *p; p++) h = (h << 5) - h + *p;
	}

	return h;
}

void
nr_type_dict_insert (NRTypeDict *td, const unsigned char *key, void *val)
{
	if (key) {
		NRTDEntry *tde;
		unsigned int hval;

		hval = nr_str_hash (key) % td->size;

		for (tde = td->entries[hval]; tde; tde = tde->next) {
			if (!strcmp (key, tde->key)) {
				tde->val = val;
				return;
			}
		}

		tde = nr_td_entry_new ();
		tde->next = td->entries[hval];
		tde->key = key;
		tde->val = val;
		td->entries[hval] = tde;
	}
}

void *
nr_type_dict_lookup (NRTypeDict *td, const unsigned char *key)
{
	if (key) {
		NRTDEntry *tde;
		unsigned int hval;
		hval = nr_str_hash (key) % td->size;
		for (tde = td->entries[hval]; tde; tde = tde->next) {
			if (!strcmp (key, tde->key)) return tde->val;
		}
	}

	return NULL;
}

#define NR_TDE_BLOCK_SIZE 32

static NRTDEntry *nr_tde_free_list;

static NRTDEntry *
nr_td_entry_new (void)
{
	NRTDEntry *tde;

	if (!nr_tde_free_list) {
		int i;
		nr_tde_free_list = nr_new (NRTDEntry, NR_TDE_BLOCK_SIZE);
		for (i = 0; i < (NR_TDE_BLOCK_SIZE - 1); i++) {
			nr_tde_free_list[i].next = nr_tde_free_list + i + 1;
		}
		nr_tde_free_list[i].next = NULL;
	}

	tde = nr_tde_free_list;
	nr_tde_free_list = tde->next;

	return tde;
}

