#ifndef __SP_SELECTION_H__
#define __SP_SELECTION_H__

/*
 * Per-desktop selection container
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * Copyright (C) 1999-2002 authors
 * Copyright (C) 2001-2002 Ximian, Inc.
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <libnr/nr-types.h>
#include "forward.h"
#include "xml/repr.h"

struct _SPSelection {
	GObject object;
	SPDesktop *desktop;
	GSList *reprs;
	GSList *items;
	guint idle;
	guint flags;
};

struct _SPSelectionClass {
	GObjectClass parent_class;

	void (* changed) (SPSelection *selection);

	/* fixme: use fine granularity */
	void (* modified) (SPSelection *selection, guint flags);
};

/* Constructor */

SPSelection * sp_selection_new (SPDesktop * desktop);

/* This are private methods & will be removed from this file */

void sp_selection_changed (SPSelection * selection);
void sp_selection_update_statusbar (SPSelection * selection);

gboolean sp_selection_item_selected (SPSelection * selection, SPItem *item);
gboolean sp_selection_repr_selected (SPSelection * selection, SPRepr *repr);
#define sp_selection_is_empty(s) (s->items == NULL)

void sp_selection_add_item (SPSelection * selection, SPItem * item);
void sp_selection_add_repr (SPSelection * selection, SPRepr * repr);
void sp_selection_set_item (SPSelection * selection, SPItem * item);
void sp_selection_set_repr (SPSelection * selection, SPRepr * repr);
void sp_selection_remove_item (SPSelection * selection, SPItem * item);
void sp_selection_remove_repr (SPSelection * selection, SPRepr * repr);
void sp_selection_set_item_list (SPSelection * selection, const GSList * list);
void sp_selection_set_repr_list (SPSelection * selection, const GSList * list);
#define sp_selection_set_empty(s) sp_selection_empty (s)
void sp_selection_empty (SPSelection * selection);
const GSList *sp_selection_item_list (SPSelection * selection);
const GSList *sp_selection_repr_list (SPSelection * selection);
SPItem *sp_selection_item (SPSelection * selection);
SPRepr *sp_selection_repr (SPSelection * selection);

NRRectF *sp_selection_bbox (SPSelection *selection, NRRectF *bbox);
NRRectF *sp_selection_bbox_document (SPSelection *selection, NRRectF *bbox);

/* Returns number of points used */
int sp_selection_snappoints (SPSelection *selection, NRPointF *points, int size);

#endif
