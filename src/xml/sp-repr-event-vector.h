#ifndef SEEN_INKSCAPE_XML_SP_REPR_EVENT_VECTOR
#define SEEN_INKSCAPE_XML_SP_REPR_EVENT_VECTOR

/*
 * Fuzzy DOM-like tree implementation
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   Frank Felfe <innerspace@iname.com>
 *
 * Copyright (C) 1999-2002 Lauris Kaplinski and Frank Felfe
 * Copyright (C) 2000-2002 Ximian, Inc.
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <glib/gtypes.h>

struct SPRepr;

struct SPReprEventVector {
	/* Immediate signals */
	unsigned int (* add_child) (SPRepr *repr, SPRepr *child, SPRepr *ref, void * data);
	void (* child_added) (SPRepr *repr, SPRepr *child, SPRepr *ref, void * data);
	unsigned int (* remove_child) (SPRepr *repr, SPRepr *child, SPRepr *ref, void * data);
	void (* child_removed) (SPRepr *repr, SPRepr *child, SPRepr *ref, void * data);
	unsigned int (* change_attr) (SPRepr *repr, const gchar *key, const gchar *oldval, const gchar *newval, void * data);
	void (* attr_changed) (SPRepr *repr, const gchar *key, const gchar *oldval, const gchar *newval, bool is_interactive, void * data);
	unsigned int (* change_content) (SPRepr *repr, const gchar *oldcontent, const gchar *newcontent, void * data);
	void (* content_changed) (SPRepr *repr, const gchar *oldcontent, const gchar *newcontent, void * data);
	unsigned int (* change_order) (SPRepr *repr, SPRepr *child, SPRepr *oldref, SPRepr *newref, void * data);
	void (* order_changed) (SPRepr *repr, SPRepr *child, SPRepr *oldref, SPRepr *newref, void * data);
};

void sp_repr_synthesize_events (SPRepr *repr, const SPReprEventVector *vector, void * data);
                                                                                
void sp_repr_add_listener (SPRepr *repr, const SPReprEventVector *vector, void * data);
void sp_repr_remove_listener_by_data (SPRepr *repr, void * data);


#endif
