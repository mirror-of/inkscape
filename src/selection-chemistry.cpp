#define __SP_SELECTION_CHEMISTRY_C__

/*
 * Miscellanous operations on selected items
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   Frank Felfe <innerspace@iname.com>
 *
 * Copyright (C) 1999-2002 authors
 * Copyright (C) 2001-2002 Ximian, Inc.
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <config.h>

#include <string.h>
#include "svg/svg.h"
#include "xml/repr-private.h"
#include "document.h"
#include "inkscape.h"
#include "desktop.h"
#include "selection.h"
#include "desktop-handles.h"
#include "sp-item-transform.h" 
#include "sp-item-group.h"
#include "sp-path.h"
#include "helper/sp-intl.h"
#include "path-chemistry.h"

#include "selection-chemistry.h"

/* fixme: find a better place */
GSList *clipboard = NULL;

static void sp_matrix_d_set_rotate (NRMatrix *m, double theta);

void
sp_selection_delete (gpointer object, gpointer data)
{
	SPDesktop *desktop;
	SPSelection *selection;
	GSList *selected;

	desktop = SP_ACTIVE_DESKTOP;
	if (desktop == NULL) return;

	selection = SP_DT_SELECTION (desktop);

	// check if something is selected
	if (sp_selection_is_empty (selection)) {
		sp_view_set_statusf_flash (SP_VIEW (desktop), _("Nothing was cut."));
		return;
	}

	selected = g_slist_copy ((GSList *) sp_selection_repr_list (selection));
	sp_selection_empty (selection);

	while (selected) {
		SPRepr *repr = (SPRepr *) selected->data;
		if (sp_repr_parent (repr)) sp_repr_unparent (repr);
		selected = g_slist_remove (selected, selected->data);
	}

	sp_document_done (SP_DT_DOCUMENT (desktop));
}

/* fixme: sequencing */
void sp_selection_duplicate (gpointer object, gpointer data)
{
	SPDesktop * desktop;
	SPSelection * selection;
	GSList *reprs, *newsel, *i;
	SPRepr *copy, *parent;
	//	SPItem *item;
	gboolean sort = TRUE;

	desktop = SP_ACTIVE_DESKTOP;
	if (desktop == NULL) return;

	selection = SP_DT_SELECTION (desktop);

	// check if something is selected
	if (sp_selection_is_empty (selection)) {
		sp_view_set_statusf_flash (SP_VIEW (desktop), _("Select some objects to duplicate."));
		return;
	}

	reprs = g_slist_copy ((GSList *) sp_selection_repr_list (selection));

	sp_selection_empty (selection);

	parent = ((SPRepr *) reprs->data)->parent;
	for (i = reprs->next; i; i = i->next) {
		if ((((SPRepr *) i->data)->parent) != parent) {
			// We can duplicate items from different parents, but we cannot do sorting in this case
			sort = FALSE;
		}
	}

	if (sort)
		reprs = g_slist_sort (reprs, (GCompareFunc) sp_repr_compare_position);

	newsel = NULL;

	while (reprs) {
		parent = ((SPRepr *) reprs->data)->parent;
		copy = sp_repr_duplicate ((SPRepr *) reprs->data);

		sp_repr_append_child (parent, copy);

		//item = (SPItem *) sp_document_add_repr (SP_DT_DOCUMENT (desktop), copy);
		//g_assert (item != NULL);

		newsel = g_slist_prepend (newsel, copy);
		reprs = g_slist_remove (reprs, reprs->data);
		sp_repr_unref (copy);
	}

	sp_document_done (SP_DT_DOCUMENT (desktop));

	sp_selection_set_repr_list (SP_DT_SELECTION (desktop), newsel);

	g_slist_free (newsel);
}

void
sp_edit_clear_all (gpointer object, gpointer data)
{
	SPDesktop *dt;
	SPDocument *doc;
	GSList *items;

	dt = SP_ACTIVE_DESKTOP;
	if (!dt) return;
	doc = SP_DT_DOCUMENT (dt);
	sp_selection_set_empty (SP_DT_SELECTION (dt));

	items = sp_item_group_item_list (SP_GROUP (sp_document_root (doc)));

	while (items) {
		sp_repr_unparent (SP_OBJECT_REPR (items->data));
		items = g_slist_remove (items, items->data);
	}

	sp_document_done (doc);
}

void
sp_edit_select_all (gpointer object, gpointer data)
{
	SPDesktop *dt;
	SPDocument *doc;
	GSList *items;
	SPSelection * selection;
	SPRepr * repr;
	
	dt = SP_ACTIVE_DESKTOP;
	if (!dt) return ;
	doc 	  = SP_DT_DOCUMENT (dt);
	selection = SP_DT_SELECTION (dt);
	items = sp_item_group_item_list (SP_GROUP (sp_document_root (doc)));
	
	while (items) {
		repr = SP_OBJECT_REPR (items->data);
		if (!sp_selection_repr_selected (selection, repr))
			sp_selection_add_repr (selection, repr);
		items = g_slist_remove (items, items->data);
	}
	sp_document_done (doc);
}

static void
sp_group_cleanup (SPGroup *group)
{
	SPObject *child;
	GSList *l;

	l = NULL;
	for (child = group->children; child != NULL; child = child->next) {
		sp_object_ref (child, NULL);
		l = g_slist_prepend (l, child);
	}

	while (l) {
		if (SP_IS_GROUP (l->data)) {
			sp_group_cleanup (SP_GROUP (l->data));
		} else if (SP_IS_PATH (l->data)) {
			sp_path_cleanup (SP_PATH (l->data));
		}
		sp_object_unref (SP_OBJECT (l->data), NULL);
		l = g_slist_remove (l, l->data);
	}


	if (!strcmp (sp_repr_name (SP_OBJECT_REPR (group)), "g")) {
		gint numitems;
		numitems = 0;
		for (child = group->children; child != NULL; child = child->next) {
			if (SP_IS_ITEM (child)) numitems += 1;
		}
		if (numitems <= 1) {
			sp_item_group_ungroup (group, NULL);
		}
	}
}

void
sp_edit_cleanup (gpointer object, gpointer data)
{
	SPDocument *doc;
	SPGroup *root;

	doc = SP_ACTIVE_DOCUMENT;
	if (!doc) return;
	if (SP_ACTIVE_DESKTOP) {
		sp_selection_empty (SP_DT_SELECTION (SP_ACTIVE_DESKTOP));
	}

	root = SP_GROUP (SP_DOCUMENT_ROOT (doc));

	sp_group_cleanup (root);

	sp_document_done (doc);
}

/* fixme: sequencing */

void
sp_selection_group (gpointer object, gpointer data)
{
	SPDesktop * desktop;
	SPSelection * selection;
	SPRepr * current;
	SPRepr * group;
	const GSList * l;
	GSList *p, *i, *reprs;
	SPRepr *parent;

	desktop = SP_ACTIVE_DESKTOP;

	if (desktop == NULL) return;

	selection = SP_DT_SELECTION (desktop);

	// check if something is selected
	if (sp_selection_is_empty (selection)) {
		sp_view_set_statusf_flash (SP_VIEW (desktop), _("Select two or more objects to group."));
		return;
	}

	l = sp_selection_repr_list (selection);

	// check if at least two objects are selected
	if (l->next == NULL) {
		sp_view_set_statusf_flash (SP_VIEW (desktop), _("Select at least two objects to group."));
		return;
	}

	// check if all selected objects have common parent
	reprs = g_slist_copy ((GSList *) sp_selection_repr_list (selection));
	parent = ((SPRepr *) reprs->data)->parent;
	for (i = reprs->next; i; i = i->next) {
		if ((((SPRepr *) i->data)->parent) != parent) {
			sp_view_set_statusf_error (SP_VIEW (desktop), _("You cannot group objects from different groups or layers."));
			return;
		}
	}

	p = g_slist_copy ((GSList *) l);

	sp_selection_empty (SP_DT_SELECTION (desktop));

	p = g_slist_sort (p, (GCompareFunc) sp_repr_compare_position);

	group = sp_repr_new ("g");

	while (p) {
		SPRepr *spnew;
		current = (SPRepr *) p->data;
		spnew = sp_repr_duplicate (current);
		sp_repr_unparent (current);
		sp_repr_append_child (group, spnew);
		sp_repr_unref (spnew);
		p = g_slist_remove (p, current);
	}

	// add the new group to the group members' common parent
	sp_repr_append_child (parent, group);
	sp_document_done (SP_DT_DOCUMENT (desktop));

	sp_selection_set_repr (selection, group);
	sp_repr_unref (group);
}

void
sp_selection_ungroup (gpointer object, gpointer data)
{
	SPDesktop *desktop;
	SPItem *group;
	GSList *children, *items;
	GSList *new_select = NULL;
	int ungrouped = 0;

	desktop = SP_ACTIVE_DESKTOP;
	if (!desktop) return;

	if (sp_selection_is_empty (SP_DT_SELECTION(desktop))) {
		sp_view_set_statusf_flash (SP_VIEW (desktop), _("Select a group to ungroup."));
		return;
	}

	// get a copy of current selection
	items = g_slist_copy ((GSList *) sp_selection_item_list (SP_DT_SELECTION (desktop)));

	for ( ; items ; items = items->next) {
		group = ((SPItem *) items->data);

		/* We do not allow ungrouping <svg> etc. (lauris) */
		if (strcmp (sp_repr_name (SP_OBJECT_REPR (group)), "g")) {
			// keep the non-group item in the new selection
			new_select = g_slist_prepend (new_select, group);
			continue;
		}

		children = NULL;
		/* This is not strictly required, but is nicer to rely on group ::destroy (lauris) */
		sp_item_group_ungroup (SP_GROUP (group), &children);
		ungrouped = 1;
		// add ungrouped items to the new selection
		new_select = g_slist_concat (new_select, children);
	}

	if (new_select) { // set new selection
		sp_selection_empty (SP_DT_SELECTION (desktop));
		sp_selection_set_item_list (SP_DT_SELECTION (desktop), new_select);
		g_slist_free (new_select);
	}
	if (!ungrouped) {
		sp_view_set_statusf_flash (SP_VIEW (desktop), _("No groups to ungroup in the selection."));
	}
}

static SPGroup *
sp_item_list_common_parent_group (const GSList *items)
{
	SPObject *parent;

	if (!items) return NULL;
	parent = SP_OBJECT_PARENT (items->data); 
	/* Strictly speaking this CAN happen, if user selects <svg> from XML editor */
	if (!SP_IS_GROUP (parent)) return NULL;
	for (items = items->next; items; items = items->next) {
		if (SP_OBJECT_PARENT (items->data) != parent) return NULL;
	}

	return SP_GROUP (parent);
}

#if 0
#define PRINT_STR(s) g_print (s)
#define PRINT_OBJ(s, o) g_print ("%s: %s\n", s, (o) ? (gchar *) sp_repr_attr (SP_OBJECT_REPR (o), "id") : "NULL")
#else
#define PRINT_STR(s)
#define PRINT_OBJ(s, o)
#endif

void sp_selection_raise (GtkWidget * widget)
{
	SPDesktop *dt;
	const GSList *items;
	SPGroup *group;
	SPRepr *grepr;
	SPObject *child, *newref;
	GSList *rev;

	dt = SP_ACTIVE_DESKTOP;
	if (!dt) return;
	items = sp_selection_item_list (SP_DT_SELECTION (dt));
	if (!items) return;
	group = sp_item_list_common_parent_group (items);
	if (!group) return;
	grepr = SP_OBJECT_REPR (group);

	/* construct reverse-ordered list of selected children */
	rev = NULL;
	for (child = group->children; child; child = child->next) {
		if (g_slist_find ((GSList *) items, child)) {
			rev = g_slist_prepend (rev, child);
		}
	}

	while (rev) {
		child = SP_OBJECT (rev->data);
		for (newref = child->next; newref; newref = newref->next) {
			if (SP_IS_ITEM (newref)) {
				if (!g_slist_find ((GSList *) items, newref)) {
					/* Found available position */
					sp_repr_change_order (grepr, SP_OBJECT_REPR (child), SP_OBJECT_REPR (newref));
				}
				break;
			}
		}
		rev = g_slist_remove (rev, child);
	}

	sp_document_done (SP_DT_DOCUMENT (dt));
}

void sp_selection_raise_to_top (GtkWidget * widget)
{
	SPDocument * document;
	SPSelection * selection;
	SPDesktop * desktop;
	SPRepr * repr;
	GSList * rl;
	GSList * l;

	desktop = SP_ACTIVE_DESKTOP;
	if (desktop == NULL) return;
	document = SP_DT_DOCUMENT (SP_ACTIVE_DESKTOP);
	selection = SP_DT_SELECTION (SP_ACTIVE_DESKTOP);

	if (sp_selection_is_empty (selection)) return;

	rl = g_slist_copy ((GSList *) sp_selection_repr_list (selection));

	for (l = rl; l != NULL; l = l->next) {
		repr = (SPRepr *) l->data;
		sp_repr_set_position_absolute (repr, -1);
	}

	g_slist_free (rl);

	sp_document_done (document);
}

void
sp_selection_lower (GtkWidget *widget)
{
	SPDesktop *dt;
	const GSList *items;
	SPGroup *group;
	SPRepr *grepr;
	SPObject *child, *newref, *oldref;
	gboolean skip;

	dt = SP_ACTIVE_DESKTOP;
	if (!dt) return;
	items = sp_selection_item_list (SP_DT_SELECTION (dt));
	if (!items) return;
	group = sp_item_list_common_parent_group (items);
	if (!group) return;
	grepr = SP_OBJECT_REPR (group);

	PRINT_STR ("STARTING\n");
	/* Start from beginning */
	skip = TRUE;
	newref = NULL;
	oldref = NULL;
	child = group->children;
	while (child != NULL) {
		if (SP_IS_ITEM (child)) {
			/* We are item */
			skip = FALSE;
			/* fixme: Remove from list (Lauris) */
			if (g_slist_find ((GSList *) items, child)) {
				/* Need lower */
				if (newref != oldref) {
					if (sp_repr_change_order (grepr, SP_OBJECT_REPR (child), (newref) ? SP_OBJECT_REPR (newref) : NULL)) {
						PRINT_STR ("Change order succeeded\n");
						PRINT_OBJ ("  child", child);
						PRINT_OBJ ("  oldref", oldref);
						PRINT_OBJ ("  newref", newref);
						/* Order change succeeded */
						/* Next available position */
						newref = child;
						/* Oldref is just what it was */
						/* Continue from oldref */
						child = oldref->next;
					} else {
						PRINT_STR ("Change order failed\n");
						PRINT_OBJ ("  child", child);
						PRINT_OBJ ("  oldref", oldref);
						PRINT_OBJ ("  newref", newref);
						/* Order change did not succeed */
						newref = oldref;
						oldref = child;
						child = child->next;
					}
				} else {
					/* Item position will not change */
					/* Other items will lower only following positions */
					newref = child;
					oldref = child;
					child = child->next;
				}
			} else {
				PRINT_STR ("Item not in list\n");
				PRINT_OBJ ("  child", child);
				PRINT_OBJ ("  oldref", oldref);
				PRINT_OBJ ("  newref", newref);
				/* We were item, but not in list */
				newref = oldref;
				oldref = child;
				child = child->next;
			}
		} else {
			PRINT_STR ("Not an item\n");
			PRINT_OBJ ("  child", child);
			PRINT_OBJ ("  oldref", oldref);
			PRINT_OBJ ("  newref", newref);
			/* We want to refind newref only to skip initial non-items */
			if (skip) newref = child;
			oldref = child;
			child = child->next;
		}
	}

	sp_document_done (SP_DT_DOCUMENT (dt));
}

void sp_selection_lower_to_bottom (GtkWidget * widget)
{
	SPDocument * document;
	SPSelection * selection;
	SPDesktop * desktop;
	SPRepr * repr;
	GSList * rl;
	GSList * l;

	desktop = SP_ACTIVE_DESKTOP;
	if (desktop == NULL) return;
	document = SP_DT_DOCUMENT (SP_ACTIVE_DESKTOP);
	selection = SP_DT_SELECTION (SP_ACTIVE_DESKTOP);

	if (sp_selection_is_empty (selection)) return;

	rl = g_slist_copy ((GSList *) sp_selection_repr_list (selection));

	rl = g_slist_reverse (rl);

	for (l = rl; l != NULL; l = l->next) {
		gint minpos;
		SPObject *pp, *pc;
		repr = (SPRepr *) l->data;
		pp = sp_document_lookup_id (document, sp_repr_attr (sp_repr_parent (repr), "id"));
		minpos = 0;
		g_assert (SP_IS_GROUP (pp));
		pc = SP_GROUP (pp)->children;
		while (!SP_IS_ITEM (pc)) {
			minpos += 1;
			pc = pc->next;
		}
		sp_repr_set_position_absolute (repr, minpos);
	}

	g_slist_free (rl);

	sp_document_done (document);
}

void
sp_undo (SPDesktop *desktop, SPDocument *doc)
{
	if (SP_IS_DESKTOP(desktop)) {
		if (!sp_document_undo (SP_DT_DOCUMENT (desktop)))
			sp_view_set_statusf_flash (SP_VIEW (desktop), _("Nothing to undo."));
	}
}

void
sp_redo (SPDesktop *desktop, SPDocument *doc)
{
	if (SP_IS_DESKTOP(desktop)) {
		if (!sp_document_redo (SP_DT_DOCUMENT (desktop)))
			sp_view_set_statusf_flash (SP_VIEW (desktop), _("Nothing to redo."));
	}
}

void
sp_selection_cut (GtkWidget * widget)
{
	sp_selection_copy (widget);
	sp_selection_delete (NULL, NULL);
}

void
sp_selection_copy (GtkWidget * widget)
{
	SPDesktop *desktop;
	SPSelection *selection;
	SPRepr *repr, *copy;
	SPCSSAttr *css;
	GSList *sl;

	desktop = SP_ACTIVE_DESKTOP;
	if (desktop == NULL) return;

	selection = SP_DT_SELECTION (desktop);

	// check if something is selected
	if (sp_selection_is_empty (selection)) {
		sp_view_set_statusf_flash (SP_VIEW (desktop), _("Nothing was copied."));
		return;
	}

	sl = g_slist_copy ((GSList *) sp_selection_repr_list (selection));
	sl = g_slist_sort (sl, (GCompareFunc) sp_repr_compare_position);

	/* Clear old clipboard */
	while (clipboard) {
		sp_repr_unref ((SPRepr *) clipboard->data);
		clipboard = g_slist_remove (clipboard, clipboard->data);
	}

	while (sl != NULL) {
		repr = (SPRepr *) sl->data;
		sl = g_slist_remove (sl, repr);
		css = sp_repr_css_attr_inherited (repr, "style");
		copy = sp_repr_duplicate (repr);
		sp_repr_css_set (copy, css, "style");
		sp_repr_css_attr_unref (css);

		clipboard = g_slist_prepend (clipboard, copy);
	}

	clipboard = g_slist_reverse (clipboard);
}

void
sp_selection_paste (GtkWidget * widget)
{
	SPDesktop * desktop;
	SPSelection * selection;
	GSList * l;
	SPRepr * repr, * copy;

	desktop = SP_ACTIVE_DESKTOP;
	if (desktop == NULL) return;
	g_assert (SP_IS_DESKTOP (desktop));

	selection = SP_DT_SELECTION (desktop);
	g_assert (selection != NULL);
	g_assert (SP_IS_SELECTION (selection));

	// check if something is in the clipboard
	if (clipboard == NULL) {
		sp_view_set_statusf_flash (SP_VIEW (desktop), _("Nothing in the clipboard."));
		return;
	}

	sp_selection_empty (selection);

	for (l = clipboard; l != NULL; l = l->next) {
		repr = (SPRepr *) l->data;
		copy = sp_repr_duplicate (repr);
		sp_document_add_repr (SP_DT_DOCUMENT (desktop), copy);
		sp_selection_add_repr (selection, copy);
		sp_repr_unref (copy);
	}

	sp_document_done (SP_DT_DOCUMENT (desktop));
}

void
sp_selection_paste_style (GtkWidget * widget)
{
	SPDesktop * desktop;
	SPSelection * selection;
	GSList *l, *selected;
	SPCSSAttr *css;

	desktop = SP_ACTIVE_DESKTOP;
	if (desktop == NULL) return;
	g_assert (SP_IS_DESKTOP (desktop));

	selection = SP_DT_SELECTION (desktop);
	g_assert (selection != NULL);
	g_assert (SP_IS_SELECTION (selection));

	// check if something is in the clipboard
	if (clipboard == NULL) {
		sp_view_set_statusf_flash (SP_VIEW (desktop), _("Nothing in the clipboard."));
		return;
	}

	// check if something is selected
	if (sp_selection_is_empty (selection)) {
		sp_view_set_statusf_flash (SP_VIEW (desktop), _("Select objects to paste style to."));
		return;
	}

	selected = g_slist_copy ((GSList *) sp_selection_repr_list (selection));

	for (l = selected; l != NULL; l = l->next) {
		css = sp_repr_css_attr_inherited ((SPRepr *) clipboard->data, "style"); // take the css from first object on clipboard
		sp_repr_css_set ((SPRepr *) l->data, css, "style");
	}

	sp_document_done (SP_DT_DOCUMENT (desktop));
}

void
sp_selection_apply_affine (SPSelection * selection, double affine[6]) {
	SPItem * item;
	GSList * l;

	g_assert (SP_IS_SELECTION (selection));

	for (l = selection->items; l != NULL; l = l-> next) {
		NRMatrix curaff, newaff;

		item = SP_ITEM (l->data);

		sp_item_i2d_affine_d (item, &curaff);
		nr_matrix_multiply (&newaff, &curaff, NR_MATRIX_D_FROM_DOUBLE (affine));
		/* fixme: This is far from elegant (Lauris) */
		sp_item_set_i2d_affine_d (item, &newaff);
		/* update repr -  needed for undo */
		sp_item_write_transform (item, SP_OBJECT_REPR (item), &item->transform);
		/* fixme: Check, whether anything changed */
		sp_object_read_attr (SP_OBJECT (item), "transform");
	}
}

void
sp_selection_remove_transform (void)
{
	SPDesktop * desktop;
	SPSelection * selection;
	const GSList * l;

	desktop = SP_ACTIVE_DESKTOP;
	if (desktop == NULL) return;
	selection = SP_DT_SELECTION (desktop);
	if (!SP_IS_SELECTION (selection)) return;

	l = sp_selection_repr_list (selection);

	while (l != NULL) {
		sp_repr_set_attr ((SPRepr*)l->data,"transform", NULL);
		l = l->next;
	}

	//	sp_selection_changed (selection);
	sp_document_done (SP_DT_DOCUMENT (desktop));
}

void
sp_selection_scale_absolute (SPSelection *selection, double x0, double x1, double y0, double y1)
{
	NRRect bbox;
	NRMatrix p2o, o2n, scale, final, s;
	double dx, dy, nx, ny;

	g_assert (SP_IS_SELECTION (selection));

	sp_selection_bbox (selection, &bbox);

	nr_matrix_set_translate (&p2o, -bbox.x0, -bbox.y0);

	dx = (x1-x0) / (bbox.x1 - bbox.x0);
	dy = (y1-y0) / (bbox.y1 - bbox.y0);
	nr_matrix_set_scale (&scale, dx, dy);

	nx = x0;
	ny = y0;
	nr_matrix_set_translate (&o2n, nx, ny);

	nr_matrix_multiply (&s, &p2o, &scale);
	nr_matrix_multiply (&final, &s, &o2n);

	sp_selection_apply_affine (selection, NR_MATRIX_D_TO_DOUBLE (&final));
}


void
sp_selection_scale_relative (SPSelection *selection, NRPoint *align, double dx, double dy)
{
	NRMatrix scale, n2d, d2n, final, s;

	nr_matrix_set_translate (&n2d, -align->x, -align->y);
	nr_matrix_set_translate (&d2n, align->x, align->y);
	nr_matrix_set_scale (&scale, dx, dy);

	nr_matrix_multiply (&s, &n2d, &scale);
	nr_matrix_multiply (&final, &s, &d2n);

	sp_selection_apply_affine (selection, NR_MATRIX_D_TO_DOUBLE (&final));
}

void
sp_selection_rotate_relative (SPSelection *selection, NRPoint *center, gdouble angle_degrees)
{
	NRMatrix rotate, n2d, d2n, final, s;

	nr_matrix_set_translate (&n2d, -center->x, -center->y);
	nr_matrix_invert (&d2n, &n2d);
	sp_matrix_d_set_rotate (&rotate, angle_degrees);

	nr_matrix_multiply (&s, &n2d, &rotate);
	nr_matrix_multiply (&final, &s, &d2n);

	sp_selection_apply_affine (selection, NR_MATRIX_D_TO_DOUBLE (&final));
}

void
sp_selection_skew_relative (SPSelection *selection, NRPoint *align, double dx, double dy)
{
	NRMatrix skew, n2d, d2n, final, s;

	nr_matrix_set_translate (&n2d, -align->x, -align->y);
	nr_matrix_invert (&d2n, &n2d);

	skew.c[0] = 1;
	skew.c[1] = dy;
	skew.c[2] = dx;
	skew.c[3] = 1;
	skew.c[4] = 0;
	skew.c[5] = 0;

	nr_matrix_multiply (&s, &n2d, &skew);
	nr_matrix_multiply (&final, &s, &d2n);

	sp_selection_apply_affine (selection, NR_MATRIX_D_TO_DOUBLE (&final));
}

void
sp_selection_move_relative (SPSelection * selection, double dx, double dy)
{
	NRMatrix move;

	nr_matrix_set_translate (&move, dx, dy);

	sp_selection_apply_affine (selection, NR_MATRIX_D_TO_DOUBLE (&move));
}

void
sp_selection_rotate_90 (void)
{
	SPDesktop * desktop;
	SPSelection * selection;
	SPItem * item;
	GSList * l, * l2;

	desktop = SP_ACTIVE_DESKTOP;
	if (!SP_IS_DESKTOP(desktop)) return;
	selection = SP_DT_SELECTION(desktop);
	if sp_selection_is_empty(selection) return;
	l = selection->items;
	for (l2 = l; l2 != NULL; l2 = l2-> next) {
		item = SP_ITEM (l2->data);
		sp_item_rotate_rel (item,-90);
	}

	//	sp_selection_changed (selection);
	sp_document_done (SP_DT_DOCUMENT (desktop));
}

void
sp_selection_rotate (SPSelection *selection, gdouble angle_degrees)
{
	NRRect bbox;
	NRPoint center;

	sp_selection_bbox (selection, &bbox);

	center.x = 0.5 * (bbox.x0 + bbox.x1);
	center.y = 0.5 * (bbox.y0 + bbox.y1);

	//	g_print ("%g  %g  %g  %g\n", bbox.x0, bbox.x1, bbox.y0, bbox.y1);

	sp_selection_rotate_relative (selection, &center, angle_degrees);

	//	sp_selection_changed (selection);
	if ( angle_degrees > 0 )
		sp_document_maybe_done (SP_DT_DOCUMENT (selection->desktop), "selector:rotate:ccw");
	else
		sp_document_maybe_done (SP_DT_DOCUMENT (selection->desktop), "selector:rotate:cw");
}

/**
\param  angle   the angle in "angular pixels", i.e. how many visible pixels must move the outermost point of the rotated object
*/
void
sp_selection_rotate_screen (SPSelection *selection,  gdouble angle)
{
	gdouble zoom, zmove, zangle, r;
	NRRect bbox;
	NRPoint center;

	sp_selection_bbox (selection, &bbox);

	center.x = 0.5 * (bbox.x0 + bbox.x1);
	center.y = 0.5 * (bbox.y0 + bbox.y1);

	zoom = SP_DESKTOP_ZOOM (selection->desktop);
	zmove = angle / zoom;
	r = hypot(bbox.x1 - center.x, bbox.y1 - center.y);

	zangle = 180 * atan2 (zmove, r) / M_PI;

	sp_selection_rotate_relative (selection, &center, zangle);

	//	sp_selection_changed (selection);
	if (angle > 0)
		sp_document_maybe_done (SP_DT_DOCUMENT (selection->desktop), "selector:rotate:ccw");
	else
		sp_document_maybe_done (SP_DT_DOCUMENT (selection->desktop), "selector:rotate:cw");

}

void
sp_selection_scale (SPSelection *selection, gdouble grow)
{
	NRRect bbox;
	NRPoint center;
	gdouble times;

	sp_selection_bbox (selection, &bbox);

	center.x = 0.5 * (bbox.x0 + bbox.x1);
	center.y = 0.5 * (bbox.y0 + bbox.y1);

	gdouble r = MAX (bbox.x1 - bbox.x0, bbox.y1 - bbox.y0);

	if (r + 2 * grow <= 0) return;

	times = 1.0 + grow / r;

	sp_selection_scale_relative (selection, &center, times, times);

	//	sp_selection_changed (selection);
	if (grow > 0)
		sp_document_maybe_done (SP_DT_DOCUMENT (selection->desktop), "selector:scale:larger");
	else
		sp_document_maybe_done (SP_DT_DOCUMENT (selection->desktop), "selector:scale:smaller");

}

void
sp_selection_scale_screen (SPSelection *selection, gdouble grow_pixels)
{
	NRRect bbox;
	NRPoint center;
	gdouble times;

	sp_selection_bbox (selection, &bbox);

	center.x = 0.5 * (bbox.x0 + bbox.x1);
	center.y = 0.5 * (bbox.y0 + bbox.y1);

	grow_pixels = grow_pixels / SP_DESKTOP_ZOOM (selection->desktop);

	gdouble r = MAX (bbox.x1 - bbox.x0, bbox.y1 - bbox.y0);

	if (r + 2 * grow_pixels <= 0) return;

	times = 1.0 + grow_pixels / r;

	sp_selection_scale_relative (selection, &center, times, times);

	//	sp_selection_changed (selection);
	if (grow_pixels > 0)
		sp_document_maybe_done (SP_DT_DOCUMENT (selection->desktop), "selector:scale:larger");
	else
		sp_document_maybe_done (SP_DT_DOCUMENT (selection->desktop), "selector:scale:smaller");
}

void
sp_selection_scale_times (SPSelection *selection, gdouble times)
{
	NRRect bbox;
	NRPoint center;

	sp_selection_bbox (selection, &bbox);

	center.x = 0.5 * (bbox.x0 + bbox.x1);
	center.y = 0.5 * (bbox.y0 + bbox.y1);

	sp_selection_scale_relative (selection, &center, times, times);

	//	sp_selection_changed (selection);
	sp_document_done (SP_DT_DOCUMENT (selection->desktop));
}

void
sp_selection_move (gdouble dx, gdouble dy)
{
	SPDesktop *desktop;
	SPSelection *selection;

	desktop = SP_ACTIVE_DESKTOP;
	g_return_if_fail(SP_IS_DESKTOP (desktop));
	selection = SP_DT_SELECTION (desktop);
	if (!SP_IS_SELECTION(selection)) {
		return;
	}
	if (sp_selection_is_empty(selection)) {
		return;
	}

	sp_selection_move_relative (selection, dx, dy);

	//	sp_selection_changed (selection);

	if (dx == 0) {
		sp_document_maybe_done (SP_DT_DOCUMENT (desktop), "selector:move:vertical");
	} else if (dy == 0) {
		sp_document_maybe_done (SP_DT_DOCUMENT (desktop), "selector:move:horizontal");
	} else {
		sp_document_done (SP_DT_DOCUMENT (desktop));
	}
}

void
sp_selection_move_screen (gdouble dx, gdouble dy)
{
	SPDesktop *desktop;
	SPSelection *selection;
	gdouble zdx, zdy, zoom;

	desktop = SP_ACTIVE_DESKTOP;
	g_return_if_fail(SP_IS_DESKTOP (desktop));
	selection = SP_DT_SELECTION (desktop);
	if (!SP_IS_SELECTION(selection)) {
		return;
	}
	if (sp_selection_is_empty(selection)) {
		return;
	}

	// same as sp_selection_move but divide deltas by zoom factor
	zoom = SP_DESKTOP_ZOOM (desktop);
	zdx = dx / zoom;
	zdy = dy / zoom;
	sp_selection_move_relative (selection, zdx, zdy);

	//	sp_selection_changed (selection);

	if (dx == 0) {
		sp_document_maybe_done (SP_DT_DOCUMENT (desktop), "selector:move:vertical");
	} else if (dy == 0) {
		sp_document_maybe_done (SP_DT_DOCUMENT (desktop), "selector:move:horizontal");
	} else {
		sp_document_done (SP_DT_DOCUMENT (desktop));
	}
}

static void scroll_to_show_item(SPDesktop *desktop, SPItem *item);

void
sp_selection_item_next (void)
{
	SPDocument *document = SP_ACTIVE_DOCUMENT;
	SPDesktop *desktop = SP_ACTIVE_DESKTOP;
	g_return_if_fail(document != NULL);
	g_return_if_fail(desktop != NULL);
	if (!SP_IS_DESKTOP(desktop)) {
		return;
	}
	SPSelection *selection = SP_DT_SELECTION(desktop);
	g_return_if_fail( selection != NULL );

	// get item list
	GSList *children = NULL;
	if (SP_CYCLING == SP_CYCLE_VISIBLE) {
		NRRect dbox;
		sp_desktop_get_display_area (desktop, &dbox);
		children = sp_document_items_in_box(document, &dbox);
	} else {
		children = sp_item_group_item_list (SP_GROUP(sp_document_root(document)));
	}

	// compute next item
	if (children == NULL) {
		return;
	}
	SPItem *item = NULL;
	if (sp_selection_is_empty(selection)) {
		item = SP_ITEM(children->data);
	} else {
		GSList *l = g_slist_find(children, selection->items->data);
		if ( ( l == NULL ) || ( l->next == NULL ) ) {
			item = SP_ITEM(children->data);
		} else {
			item = SP_ITEM(l->next->data);
		}
	}

	// set selection to item
	if (item != NULL) {
		sp_selection_set_item(selection, item);
	} else {
		return;
	}

	g_slist_free (children);

	// adjust visible area to see whole new selection
	if (SP_CYCLING == SP_CYCLE_FOCUS) {
		scroll_to_show_item(desktop, item);
	}
}

/* TODO: Much copy & paste code here; see if can merge with sp_selection_item_next. */
void
sp_selection_item_prev (void)
{
	SPDocument *document = SP_ACTIVE_DOCUMENT;
	SPDesktop *desktop = SP_ACTIVE_DESKTOP;
	g_return_if_fail(document != NULL);
	g_return_if_fail(desktop != NULL);
	if (!SP_IS_DESKTOP(desktop)) {
		return;
	}
	SPSelection *selection = SP_DT_SELECTION(desktop);
	g_return_if_fail( selection != NULL );

	// get item list
	GSList *children = NULL;
	if (SP_CYCLING == SP_CYCLE_VISIBLE) {
		NRRect dbox;
		sp_desktop_get_display_area (desktop, &dbox);
		children = sp_document_items_in_box(document, &dbox);
	} else {
		children = sp_item_group_item_list (SP_GROUP(sp_document_root(document)));
	}

	// compute prev item
	if (children == NULL) {
		return;
	}
	SPItem *item = NULL;
	if (sp_selection_is_empty(selection)) {
		item = SP_ITEM(g_slist_last(children)->data);
	} else {
		GSList *l = children;
		while ((l->next != NULL) && (l->next->data != selection->items->data)) {
			l = l->next;
		}
		item = SP_ITEM (l->data);
	}

	// set selection to item
	if (item != NULL) {
		sp_selection_set_item(selection, item);
	} else {
		return;
	}

	g_slist_free (children);

	// adjust visible area to see whole new selection
	if (SP_CYCLING == SP_CYCLE_FOCUS) {
		scroll_to_show_item(desktop, item);
	}
}

/**
 * If \a item is not entirely visible then adjust visible area to centre on the centre on of
 * \a item.
 */
static void scroll_to_show_item(SPDesktop *desktop, SPItem *item)
{
	NRRect dbox;
	sp_desktop_get_display_area (desktop, &dbox);
	NRRect sbox;
	sp_item_bbox_desktop (item, &sbox);
	if ( dbox.x0 > sbox.x0  ||
	     dbox.y0 > sbox.y0  ||
	     dbox.x1 < sbox.x1  ||
	     dbox.y1 < sbox.y1 )
	{
		double x, y;
		x = (sbox.x0 + sbox.x1) / 2;
		y = (sbox.y0 + sbox.y1) / 2;
		ArtPoint s;
		s.x = NR_MATRIX_DF_TRANSFORM_X (NR_MATRIX_D_FROM_DOUBLE (desktop->d2w), x, y);
		s.y = NR_MATRIX_DF_TRANSFORM_Y (NR_MATRIX_D_FROM_DOUBLE (desktop->d2w), x, y);
		x = (dbox.x0 + dbox.x1) / 2;
		y = (dbox.y0 + dbox.y1) / 2;
		ArtPoint d;
		d.x = NR_MATRIX_DF_TRANSFORM_X (NR_MATRIX_D_FROM_DOUBLE (desktop->d2w), x, y);
		d.y = NR_MATRIX_DF_TRANSFORM_Y (NR_MATRIX_D_FROM_DOUBLE (desktop->d2w), x, y);
		gint dx = (gint) (d.x - s.x);
		gint dy = (gint) (d.y - s.y);
		sp_desktop_scroll_world (desktop, dx, dy);
	}
}


static void
sp_matrix_d_set_rotate (NRMatrix *m, double theta_degrees)
{
	double s, c;
	s = sin(theta_degrees / 180.0 * M_PI);
	c = cos(theta_degrees / 180.0 * M_PI);
	m->c[0] = c;
	m->c[1] = s;
	m->c[2] = -s;
	m->c[3] = c;
	m->c[4] = 0.0;
	m->c[5] = 0.0;
}

