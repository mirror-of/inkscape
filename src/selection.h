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

#include "forward.h"
#include "xml/repr.h"



struct SPSelection : public GObject {
public:
	GSList *reprs;
	GSList *items;

	SPDesktop *desktop() { return _desktop; }

	static SPSelection *create(SPDesktop *desktop);

	static GType gobject_type();
	static void item_modified(SPItem *item, guint flags, SPSelection *selection);
	static void release_item(SPItem *item, SPSelection *selection);

private:
	SPSelection();
	~SPSelection();
	SPSelection(SPSelection const &);
	void operator=(SPSelection const &);

	static void _init(void *mem);
	static void _class_init(SPSelectionClass *klass);
	static void _dispose(GObject *object);

	static gboolean _idle_handler(SPSelection *selection);

	SPDesktop *_desktop;
	guint _flags;
	guint _idle;
};

struct SPSelectionClass {
	GObjectClass parent_class;

	void (* changed) (SPSelection *selection);

	/* fixme: use fine granularity */
	void (* modified) (SPSelection *selection, guint flags);
};

/* Constructor */

inline SPSelection *sp_selection_new(SPDesktop *desktop) {
	return SPSelection::create(desktop);
}

/* This are private methods & will be removed from this file */

void sp_selection_changed(SPSelection *selection);
void sp_selection_update_statusbar(SPSelection *selection);

gboolean sp_selection_item_selected(SPSelection *selection, SPItem *item);
gboolean sp_selection_repr_selected(SPSelection *selection, SPRepr *repr);
#define sp_selection_is_empty(s) (s->items == NULL)

void sp_selection_add_item(SPSelection *selection, SPItem *item);
void sp_selection_add_repr(SPSelection *selection, SPRepr *repr);
void sp_selection_set_item(SPSelection *selection, SPItem *item);
void sp_selection_set_repr(SPSelection *selection, SPRepr *repr);
void sp_selection_remove_item(SPSelection *selection, SPItem *item);
void sp_selection_remove_repr(SPSelection *selection, SPRepr *repr);
void sp_selection_set_item_list(SPSelection *selection, GSList const *list);
void sp_selection_set_repr_list(SPSelection *selection, GSList const *list);
#define sp_selection_set_empty(s) sp_selection_empty(s)
void sp_selection_empty(SPSelection *selection);
const GSList *sp_selection_item_list (SPSelection *selection);
const GSList *sp_selection_repr_list (SPSelection *selection);
SPItem *sp_selection_item (SPSelection *selection);
SPRepr *sp_selection_repr (SPSelection *selection);

NRRect *sp_selection_bbox (SPSelection *selection, NRRect *bbox);
NR::Rect sp_selection_bbox (SPSelection *selection);
NRRect *sp_selection_bbox_document (SPSelection *selection, NRRect *bbox);
NR::Rect sp_selection_bbox_document (SPSelection *selection);

/* Returns number of points used */
int sp_selection_snappoints(SPSelection *selection, NR::Point points[], int size);


#endif /* !__SP_SELECTION_H__ */
