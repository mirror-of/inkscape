#define __SP_PATH_CHEMISTRY_C__

/*
 * Here are handlers for modifying selections, specific to paths
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * Copyright (C) 1999-2002 Authors
 * Copyright (C) 2001-2002 Ximian, Inc.
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <string.h>
#include <libart_lgpl/art_misc.h>
#include "xml/repr.h"
#include "svg/svg.h"
#include "sp-path.h"
#include "sp-text.h"
#include "style.h"
#include "sodipodi.h"
#include "document.h"
#include "selection.h"
#include "desktop-handles.h"
#include "path-chemistry.h"
#include "desktop.h"

/* Helper functions for sp_selected_path_to_curves */
static void sp_selected_path_to_curves0 (gboolean do_document_done, guint32 text_grouping_policy);
static SPRepr * sp_selected_item_to_curved_repr(SPItem * item, guint32 text_grouping_policy);
enum {				
  /* Not used yet. This is the placeholder of Lauris's idea. */
	SP_TOCURVE_INTERACTIVE       = 1 << 0,
	SP_TOCURVE_GROUPING_BY_WORD  = 1 << 1,
	SP_TOCURVE_GROUPING_BY_LINE  = 1 << 2,
	SP_TOCURVE_GROUPING_BY_WHOLE = 1 << 3,
};

void
sp_selected_path_combine (void)
{
	SPDesktop * desktop;
	SPSelection * selection;
	GSList * il;
	GSList * l;
	SPRepr * repr;
	SPItem * item;
	SPPath * path;
	SPCurve * c;
	ArtBpath * abp;
	gchar * d, * str, * style;

	sp_selected_path_to_curves0 (FALSE, 0);

	desktop = SP_ACTIVE_DESKTOP;
	if (!SP_IS_DESKTOP(desktop)) return;
	selection = SP_DT_SELECTION (desktop);

	il = (GSList *) sp_selection_item_list (selection);

	if (g_slist_length (il) < 2) return;

	for (l = il; l != NULL; l = l->next) {
		item = (SPItem *) l->data;
		if (!SP_IS_SHAPE (item)) return;
	}

	il = g_slist_copy (il);

	d = "";
	style = g_strdup (sp_repr_attr ((SP_OBJECT (il->data))->repr, "style"));

	for (l = il; l != NULL; l = l->next) {
		NRMatrixF i2root;
		NRMatrixD i2rootd;
		path = (SPPath *) l->data;
		c = sp_shape_get_curve (SP_SHAPE (path));
		sp_item_i2root_affine (SP_ITEM (path), &i2root);
		nr_matrix_d_from_f (&i2rootd, &i2root);
		abp = art_bpath_affine_transform (c->bpath, NR_MATRIX_D_TO_DOUBLE (&i2rootd));
		sp_curve_unref (c);
		str = sp_svg_write_path (abp);
		art_free (abp);
		d = g_strconcat (d, str, NULL);
		g_free (str);
		sp_repr_unparent (SP_OBJECT_REPR (path));
	}

	g_slist_free (il);

	repr = sp_repr_new ("path");
	sp_repr_set_attr (repr, "style", style);
	g_free (style);
	sp_repr_set_attr (repr, "d", d);
	g_free (d);
	item = (SPItem *) sp_document_add_repr (SP_DT_DOCUMENT (desktop), repr);
	sp_document_done (SP_DT_DOCUMENT (desktop));
	sp_repr_unref (repr);

	sp_selection_set_item (selection, item);
}

void
sp_selected_path_break_apart (void)
{
	SPSelection * selection;
	SPRepr * repr;
	SPItem * item;
	SPPath * path;
	SPCurve * curve;
	ArtBpath * abp;
	NRMatrixF i2root;
	NRMatrixD d;
	gchar * style, * str;
	GSList * list, * l;
	SPDesktop * desktop;
	
	desktop = SP_ACTIVE_DESKTOP;
	if (!SP_IS_DESKTOP(desktop)) return;

	selection = SP_DT_SELECTION (desktop);

	item = sp_selection_item (selection);

	if (item == NULL) return;
	if (!SP_IS_PATH (item)) return;

	path = SP_PATH (item);

	curve = sp_shape_get_curve (SP_SHAPE (path));
	if (curve == NULL) return;

	sp_item_i2root_affine (SP_ITEM (path), &i2root);
	nr_matrix_d_from_f (&d, &i2root);
	style = g_strdup (sp_repr_attr (SP_OBJECT (item)->repr, "style"));

	abp = art_bpath_affine_transform (curve->bpath, NR_MATRIX_D_TO_DOUBLE (&d));

	sp_curve_unref (curve);
	sp_repr_unparent (SP_OBJECT_REPR (item));

	curve = sp_curve_new_from_bpath (abp);

	list = sp_curve_split (curve);

	sp_curve_unref (curve);

	for (l = list; l != NULL; l = l->next) {
		curve = (SPCurve *) l->data;

		repr = sp_repr_new ("path");
		sp_repr_set_attr (repr, "style", style);
		str = sp_svg_write_path (curve->bpath);
		sp_repr_set_attr (repr, "d", str);
		g_free (str);
		item = (SPItem *) sp_document_add_repr (SP_DT_DOCUMENT (desktop), repr);
		sp_repr_unref (repr);
		sp_selection_add_item (selection, item);
	}
	sp_document_done (SP_DT_DOCUMENT (desktop));

	g_slist_free (list);
	g_free (style);
}

/* This function is an entry point from GUI */
void
sp_selected_path_to_curves (void)
{
	sp_selected_path_to_curves0(TRUE, SP_TOCURVE_INTERACTIVE);
}

static void
sp_selected_path_to_curves0 (gboolean do_document_done, guint32 text_grouping_policy)
{
	SPDesktop *dt;
	SPItem *item;
	SPRepr  *repr;
	
	GSList * il;
	GSList * l;
	SPObject *parent;
	
	dt = SP_ACTIVE_DESKTOP;
	if (!dt) return;
	il = (GSList *) sp_selection_item_list (SP_DT_SELECTION (dt));
	if (!il) return;

	l = il;
	while (l) {
		item = (SPItem *)l->data;
		l    = l->next;
		repr = sp_selected_item_to_curved_repr (item, 0);
		if (!repr)
			continue;
		parent = SP_OBJECT_PARENT (item);
		sp_repr_add_child (SP_OBJECT_REPR (parent), 
				   repr, SP_OBJECT_REPR (item));
		sp_repr_unparent (SP_OBJECT_REPR (item));
		sp_selection_add_repr (SP_DT_SELECTION (dt), repr);
		sp_repr_unref(repr);
	}
	if (do_document_done)
		sp_document_done (SP_DT_DOCUMENT (dt));
}

static SPRepr *
sp_selected_item_to_curved_repr(SPItem * item, guint32 text_grouping_policy)
{
	SPCurve *curve;
	SPRepr  *repr;
	guchar  *style_str;
	guchar  *def_str;
	
	if (!item)
		return NULL;
		
	if (SP_IS_SHAPE (item)) {
		curve = sp_shape_get_curve (SP_SHAPE (item));
	} else if (SP_IS_TEXT (item)) {
		curve = sp_text_normalized_bpath (SP_TEXT (item));
	} else {
		curve = NULL;
	}
	
	if (!curve) return NULL;
	
	repr = sp_repr_new ("path");
	/* Transformation */
	sp_repr_set_attr (repr, "transform", 
			  sp_repr_attr (SP_OBJECT_REPR (item), "transform"));
	/* Style */
	style_str = sp_style_write_difference (SP_OBJECT_STYLE (item), 
					       SP_OBJECT_STYLE (SP_OBJECT_PARENT (item)));
	sp_repr_set_attr (repr, "style", style_str);
	g_free (style_str);
	/* Definition */
	def_str = sp_svg_write_path (curve->bpath);
	sp_repr_set_attr (repr, "d", def_str);
	g_free (def_str);
	sp_curve_unref (curve);
	return repr;
}

void
sp_path_cleanup (SPPath *path)
{
	SPCurve *curve;
	GSList *curves, *c;
	SPStyle *style;
	gboolean dropped;

	if (strcmp (sp_repr_name (SP_OBJECT_REPR (path)), "path")) return;

	style = SP_OBJECT_STYLE (path);
	if (style->fill.type == SP_PAINT_TYPE_NONE) return;

	curve = sp_shape_get_curve (SP_SHAPE (path));
	if (!curve) return;
	c = sp_curve_split (curve);
	sp_curve_unref (curve);

	dropped = FALSE;
	curves = NULL;
	while (c) {
		curve = (SPCurve *) c->data;
		if (curve->closed) {
			curves = g_slist_prepend (curves, curve);
		} else {
			dropped = TRUE;
			sp_curve_unref (curve);
		}
		c = g_slist_remove (c, c->data);
	}
	curves = g_slist_reverse (curves);

	curve = sp_curve_concat (curves);

	while (curves) {
		sp_curve_unref ((SPCurve *) curves->data);
		curves = g_slist_remove (curves, curves->data);
	}

	if (sp_curve_is_empty (curve)) {
		sp_repr_unparent (SP_OBJECT_REPR (path));
	} else if (dropped) {
		guchar *svgpath;
		svgpath = sp_svg_write_path (curve->bpath);
		sp_repr_set_attr (SP_OBJECT_REPR (path), "d", svgpath);
		g_free (svgpath);
	}

	sp_curve_unref (curve);
}

