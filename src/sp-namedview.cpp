#define __SP_NAMEDVIEW_C__

/*
 * <sodipodi:namedview> implementation
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * Copyright (C) 1999-2002 Authors
 * Copyright (C) 2000-2001 Ximian, Inc.
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "config.h"

#include <math.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>

#include <gtk/gtk.h>

#include "display/canvas-grid.h"
#include "helper/units.h"
#include "svg/svg.h"
#include "attributes.h"
#include "document.h"
#include "desktop.h"
#include "desktop-events.h"
#include "desktop-handles.h"
#include "sp-guide.h"
#include "sp-item-group.h"
#include "sp-namedview.h"
#include "prefs-utils.h"

#define PTPERMM (72.0 / 25.4)

#define DEFAULTTOLERANCE 5.0
#define DEFAULTGRIDCOLOR 0x3f3fff2f
#define DEFAULTGUIDECOLOR 0x0000ff7f
#define DEFAULTGUIDEHICOLOR 0xff00007f
#define DEFAULTBORDERCOLOR 0x000000ff
#define DEFAULTPAGECOLOR 0xffffff00

static void sp_namedview_class_init (SPNamedViewClass * klass);
static void sp_namedview_init (SPNamedView * namedview);

static void sp_namedview_build (SPObject * object, SPDocument * document, SPRepr * repr);
static void sp_namedview_release (SPObject *object);
static void sp_namedview_set (SPObject *object, unsigned int key, const gchar *value);
static void sp_namedview_child_added (SPObject * object, SPRepr * child, SPRepr * ref);
static void sp_namedview_remove_child (SPObject *object, SPRepr *child);
static SPRepr *sp_namedview_write (SPObject *object, SPRepr *repr, guint flags);

static void sp_namedview_setup_guides (SPNamedView * nv);

static void sp_namedview_setup_grid (SPNamedView * nv);
static void sp_namedview_setup_grid_item (SPNamedView * nv, SPCanvasItem * item);

static gboolean sp_str_to_bool (const gchar *str);
static gboolean sp_nv_read_length (const gchar *str, guint base, gdouble *val, const SPUnit **unit);
static gboolean sp_nv_read_opacity (const gchar *str, guint32 *color);

static SPObjectGroupClass * parent_class;

GType
sp_namedview_get_type (void)
{
	static GType namedview_type = 0;
	if (!namedview_type) {
		GTypeInfo namedview_info = {
			sizeof (SPNamedViewClass),
			NULL,	/* base_init */
			NULL,	/* base_finalize */
			(GClassInitFunc) sp_namedview_class_init,
			NULL,	/* class_finalize */
			NULL,	/* class_data */
			sizeof (SPNamedView),
			16,	/* n_preallocs */
			(GInstanceInitFunc) sp_namedview_init,
			NULL,	/* value_table */
		};
		namedview_type = g_type_register_static (SP_TYPE_OBJECTGROUP, "SPNamedView", &namedview_info, (GTypeFlags)0);
	}
	return namedview_type;
}

static void
sp_namedview_class_init (SPNamedViewClass * klass)
{
	GObjectClass * gobject_class;
	SPObjectClass * sp_object_class;

	gobject_class = (GObjectClass *) klass;
	sp_object_class = (SPObjectClass *) klass;

	parent_class = (SPObjectGroupClass*)g_type_class_ref (SP_TYPE_OBJECTGROUP);

	sp_object_class->build = sp_namedview_build;
	sp_object_class->release = sp_namedview_release;
	sp_object_class->set = sp_namedview_set;
	sp_object_class->child_added = sp_namedview_child_added;
	sp_object_class->remove_child = sp_namedview_remove_child;
	sp_object_class->write = sp_namedview_write;
}

static void
sp_namedview_init (SPNamedView * nv)
{
	nv->editable = TRUE;
	nv->showgrid = FALSE;
	nv->snaptogrid = FALSE;
	nv->showguides = TRUE;
	nv->snaptoguides = FALSE;
	nv->showborder = TRUE;
	nv->grid_snap_to = SNAP_TO_BBOX;
	nv->guide_snap_to = SNAP_TO_BBOX;

	nv->guides = NULL;
	nv->viewcount = 0;

	new (&nv->grid_snapper) GridSnapper(0);
	new (&nv->guide_snapper) GuideSnapper(0);
}

static void
sp_namedview_build (SPObject * object, SPDocument * document, SPRepr * repr)
{
	SPNamedView * nv;
	SPObjectGroup * og;
        SPObject * o;

	nv = (SPNamedView *) object;
	og = (SPObjectGroup *) object;

	if (((SPObjectClass *) (parent_class))->build)
		(* ((SPObjectClass *) (parent_class))->build) (object, document, repr);

	sp_object_read_attr (object, "viewonly");
	sp_object_read_attr (object, "showgrid");
	sp_object_read_attr (object, "snaptogrid");
	sp_object_read_attr (object, "showguides");
	sp_object_read_attr (object, "snaptoguides");
	sp_object_read_attr (object, "gridtolerance");
	sp_object_read_attr (object, "guidetolerance");
	sp_object_read_attr (object, "gridoriginx");
	sp_object_read_attr (object, "gridoriginy");
	sp_object_read_attr (object, "gridspacingx");
	sp_object_read_attr (object, "gridspacingy");
	sp_object_read_attr (object, "gridcolor");
	sp_object_read_attr (object, "gridopacity");
	sp_object_read_attr (object, "guidecolor");
	sp_object_read_attr (object, "guideopacity");
	sp_object_read_attr (object, "guidehicolor");
	sp_object_read_attr (object, "guidehiopacity");
	sp_object_read_attr (object, "showborder");
	sp_object_read_attr (object, "borderlayer");
	sp_object_read_attr (object, "bordercolor");
	sp_object_read_attr (object, "borderopacity");
	sp_object_read_attr (object, "pagecolor");
	sp_object_read_attr (object, "inkscape:pageopacity");
	sp_object_read_attr (object, "inkscape:pageshadow");
	sp_object_read_attr (object, "inkscape:zoom");
	sp_object_read_attr (object, "inkscape:cx");
	sp_object_read_attr (object, "inkscape:cy");
	sp_object_read_attr (object, "inkscape:window-width");
	sp_object_read_attr (object, "inkscape:window-height");
	sp_object_read_attr (object, "inkscape:window-x");
	sp_object_read_attr (object, "inkscape:window-y");
	sp_object_read_attr (object, "inkscake:grid-bbox");
	sp_object_read_attr (object, "inkscape:guide-bbox");
	sp_object_read_attr (object, "inkscape:grid-points");
	sp_object_read_attr (object, "inkscape:guide-points");

	/* Construct guideline list */

	for (o = sp_object_first_child(SP_OBJECT(og)) ; o != NULL; o = SP_OBJECT_NEXT(o) ) {
		if (SP_IS_GUIDE (o)) {
			SPGuide * g;
			g = SP_GUIDE (o);
			nv->guides = g_slist_prepend (nv->guides, g);
			g_object_set (G_OBJECT (g), "color", nv->guidecolor, "hicolor", nv->guidehicolor, NULL);
		}
	}
}

static void
sp_namedview_release (SPObject * object)
{
	SPNamedView *namedview;

	namedview = (SPNamedView *) object;

	if (namedview->guides) {
		g_slist_free (namedview->guides);
		namedview->guides = NULL;
	}

	while (namedview->gridviews) {
		gtk_object_destroy (GTK_OBJECT (namedview->gridviews->data));
		namedview->gridviews = g_slist_remove (namedview->gridviews, namedview->gridviews->data);
	}

	namedview->grid_snapper.~GridSnapper();
	namedview->guide_snapper.~GuideSnapper();

	if (((SPObjectClass *) parent_class)->release)
		((SPObjectClass *) parent_class)->release (object);
}

static void
sp_namedview_set (SPObject *object, unsigned int key, const gchar *value)
{
	SPNamedView *nv;
	const SPUnit *pt = NULL;
	const SPUnit *px = NULL;
	const SPUnit *mm = NULL;
	GSList * l;

	nv = SP_NAMEDVIEW (object);

	if (!pt) pt = &sp_unit_get_by_id(SP_UNIT_PT);
	if (!px) px = &sp_unit_get_by_id(SP_UNIT_PX);
	if (!mm) mm = &sp_unit_get_by_id(SP_UNIT_MM);

	switch (key) {
	case SP_ATTR_VIEWONLY:
		nv->editable = (!value);
		object->requestModified(SP_OBJECT_MODIFIED_FLAG);
		break;
	case SP_ATTR_SHOWGRID:
		nv->showgrid = sp_str_to_bool (value);
		sp_namedview_setup_grid (nv);
		object->requestModified(SP_OBJECT_MODIFIED_FLAG);
		break;
	case SP_ATTR_SNAPTOGRID:
		nv->snaptogrid = sp_str_to_bool (value);
		object->requestModified(SP_OBJECT_MODIFIED_FLAG);
		break;
	case SP_ATTR_SHOWGUIDES:
		if (!value) { // show guides if not specified, for backwards compatibility
			nv->showguides = TRUE;
		} else {
			nv->showguides = sp_str_to_bool (value);
		}
		sp_namedview_setup_guides (nv);
		object->requestModified(SP_OBJECT_MODIFIED_FLAG);
		break;
	case SP_ATTR_SNAPTOGUIDES:
		nv->snaptoguides = sp_str_to_bool (value);
		object->requestModified(SP_OBJECT_MODIFIED_FLAG);
		break;
	case SP_ATTR_GRIDTOLERANCE:
		nv->gridtoleranceunit = px;
		nv->gridtolerance = DEFAULTTOLERANCE;
		if (value) {
			sp_nv_read_length (value, SP_UNIT_ABSOLUTE | SP_UNIT_DEVICE, &nv->gridtolerance, &nv->gridtoleranceunit);
		}
		object->requestModified(SP_OBJECT_MODIFIED_FLAG);
		break;
	case SP_ATTR_GUIDETOLERANCE:
		nv->guidetoleranceunit = px;
		nv->guidetolerance = DEFAULTTOLERANCE;
		if (value) {
			sp_nv_read_length (value, SP_UNIT_ABSOLUTE | SP_UNIT_DEVICE, &nv->guidetolerance, &nv->guidetoleranceunit);
		}
		object->requestModified(SP_OBJECT_MODIFIED_FLAG);
		break;
	case SP_ATTR_GRIDORIGINX:
	case SP_ATTR_GRIDORIGINY:
	{
		unsigned const d = (key == SP_ATTR_GRIDORIGINY);
		nv->gridunit = mm;
		nv->gridorigin[d] = 0.0;
		if (value) {
			sp_nv_read_length (value, SP_UNIT_ABSOLUTE, &nv->gridorigin[d], &nv->gridunit);
		}
		sp_convert_distance (&nv->gridorigin[d], nv->gridunit, pt);
		sp_namedview_setup_grid (nv);
		object->requestModified(SP_OBJECT_MODIFIED_FLAG);
		break;
	}
	case SP_ATTR_GRIDSPACINGX:
	case SP_ATTR_GRIDSPACINGY:
	{
		unsigned const d = (key == SP_ATTR_GRIDSPACINGY);
		nv->gridunit = mm;
		nv->gridspacing[d] = 5.0;
		if (value) {
			sp_nv_read_length (value, SP_UNIT_ABSOLUTE, &nv->gridspacing[d], &nv->gridunit);
		}
		sp_convert_distance (&nv->gridspacing[d], nv->gridunit, pt);
		sp_namedview_setup_grid (nv);
		object->requestModified(SP_OBJECT_MODIFIED_FLAG);
		break;
	}
	case SP_ATTR_GRIDCOLOR:
		nv->gridcolor = (nv->gridcolor & 0xff) | (DEFAULTGRIDCOLOR & 0xffffff00);
		if (value) {
			nv->gridcolor = (nv->gridcolor & 0xff) | sp_svg_read_color (value, nv->gridcolor);
		}
		sp_namedview_setup_grid (nv);
		object->requestModified(SP_OBJECT_MODIFIED_FLAG);
		break;
	case SP_ATTR_GRIDOPACITY:
		nv->gridcolor = (nv->gridcolor & 0xffffff00) | (DEFAULTGRIDCOLOR & 0xff);
		sp_nv_read_opacity (value, &nv->gridcolor);
		sp_namedview_setup_grid (nv);
		object->requestModified(SP_OBJECT_MODIFIED_FLAG);
		break;
	case SP_ATTR_GUIDECOLOR:
		nv->guidecolor = (nv->guidecolor & 0xff) | (DEFAULTGUIDECOLOR & 0xffffff00);
		if (value) {
			nv->guidecolor = (nv->guidecolor & 0xff) | sp_svg_read_color (value, nv->guidecolor);
		}
		for (l = nv->guides; l != NULL; l = l->next) {
			g_object_set (G_OBJECT (l->data), "color", nv->guidecolor, NULL);
		}
		object->requestModified(SP_OBJECT_MODIFIED_FLAG);
		break;
	case SP_ATTR_GUIDEOPACITY:
		nv->guidecolor = (nv->guidecolor & 0xffffff00) | (DEFAULTGUIDECOLOR & 0xff);
		sp_nv_read_opacity (value, &nv->guidecolor);
		for (l = nv->guides; l != NULL; l = l->next) {
			g_object_set (G_OBJECT (l->data), "color", nv->guidecolor, NULL);
		}
		object->requestModified(SP_OBJECT_MODIFIED_FLAG);
		break;
	case SP_ATTR_GUIDEHICOLOR:
		nv->guidehicolor = (nv->guidehicolor & 0xff) | (DEFAULTGUIDEHICOLOR & 0xffffff00);
		if (value) {
			nv->guidehicolor = (nv->guidehicolor & 0xff) | sp_svg_read_color (value, nv->guidehicolor);
		}
		for (l = nv->guides; l != NULL; l = l->next) {
			g_object_set (G_OBJECT (l->data), "hicolor", nv->guidehicolor, NULL);
		}
		object->requestModified(SP_OBJECT_MODIFIED_FLAG);
		break;
	case SP_ATTR_GUIDEHIOPACITY:
		nv->guidehicolor = (nv->guidehicolor & 0xffffff00) | (DEFAULTGUIDEHICOLOR & 0xff);
		sp_nv_read_opacity (value, &nv->guidehicolor);
		for (l = nv->guides; l != NULL; l = l->next) {
			g_object_set (G_OBJECT (l->data), "hicolor", nv->guidehicolor, NULL);
		}
		object->requestModified(SP_OBJECT_MODIFIED_FLAG);
		break;
	case SP_ATTR_SHOWBORDER:
		nv->showborder = (value) ? sp_str_to_bool (value) : TRUE;
		object->requestModified(SP_OBJECT_MODIFIED_FLAG);
		break;
	case SP_ATTR_BORDERLAYER:
		nv->borderlayer = SP_BORDER_LAYER_BOTTOM;
		if (value && !strcasecmp (value, "top")) nv->borderlayer = SP_BORDER_LAYER_TOP;
		object->requestModified(SP_OBJECT_MODIFIED_FLAG);
		break;
	case SP_ATTR_BORDERCOLOR:
		nv->bordercolor = (nv->bordercolor & 0xff) | (DEFAULTBORDERCOLOR & 0xffffff00);
		if (value) {
			nv->bordercolor = (nv->bordercolor & 0xff) | sp_svg_read_color (value, nv->bordercolor);
		}
		object->requestModified(SP_OBJECT_MODIFIED_FLAG);
		break;
	case SP_ATTR_BORDEROPACITY:
		nv->bordercolor = (nv->bordercolor & 0xffffff00) | (DEFAULTBORDERCOLOR & 0xff);
		sp_nv_read_opacity (value, &nv->bordercolor);
		object->requestModified(SP_OBJECT_MODIFIED_FLAG);
		break;
	case SP_ATTR_PAGECOLOR:
		nv->pagecolor = (nv->pagecolor & 0xff) | (DEFAULTPAGECOLOR & 0xffffff00);
		if (value) {
			nv->pagecolor = (nv->pagecolor & 0xff) | sp_svg_read_color (value, nv->pagecolor);
		}
		object->requestModified(SP_OBJECT_MODIFIED_FLAG);
		break;
	case SP_ATTR_INKSCAPE_PAGEOPACITY:
		nv->pagecolor = (nv->pagecolor & 0xffffff00) | (DEFAULTPAGECOLOR & 0xff);
		sp_nv_read_opacity (value, &nv->pagecolor);
		object->requestModified(SP_OBJECT_MODIFIED_FLAG);
		break;
	case SP_ATTR_INKSCAPE_PAGESHADOW:
		nv->pageshadow = value? atoi (value) : 2; // 2 is the default
		object->requestModified(SP_OBJECT_MODIFIED_FLAG);
		break;
	case SP_ATTR_INKSCAPE_ZOOM:
		nv->zoom = value? g_ascii_strtod (value, NULL) : 0; // zero means not set
		object->requestModified(SP_OBJECT_MODIFIED_FLAG);
		break;
	case SP_ATTR_INKSCAPE_CX:
		nv->cx = value? g_ascii_strtod (value, NULL) : HUGE_VAL; // HUGE_VAL means not set
		object->requestModified(SP_OBJECT_MODIFIED_FLAG);
		break;
	case SP_ATTR_INKSCAPE_CY:
		nv->cy = value? g_ascii_strtod (value, NULL) : HUGE_VAL; // HUGE_VAL means not set
		object->requestModified(SP_OBJECT_MODIFIED_FLAG);
		break;
	case SP_ATTR_INKSCAPE_WINDOW_WIDTH:
		nv->window_width = value? atoi (value) : -1; // -1 means not set
		object->requestModified(SP_OBJECT_MODIFIED_FLAG);
		break;
	case SP_ATTR_INKSCAPE_WINDOW_HEIGHT:
		nv->window_height = value? atoi (value) : -1; // -1 means not set
		object->requestModified(SP_OBJECT_MODIFIED_FLAG);
		break;
	case SP_ATTR_INKSCAPE_WINDOW_X:
		nv->window_x = value? atoi (value) : -1; // -1 means not set
		object->requestModified(SP_OBJECT_MODIFIED_FLAG);
		break;
	case SP_ATTR_INKSCAPE_WINDOW_Y:
		nv->window_y = value? atoi (value) : -1; // -1 means not set
		object->requestModified(SP_OBJECT_MODIFIED_FLAG);
		break;
	case SP_ATTR_INKSCAPE_GRID_BBOX:
		if (value) {
			if (sp_str_to_bool (value)) {
				nv->grid_snap_to |= SNAP_TO_BBOX;
			} else {
				nv->grid_snap_to &= ~SNAP_TO_BBOX;
			}
			sp_object_request_modified (object, SP_OBJECT_MODIFIED_FLAG);
		}
		break;
	case SP_ATTR_INKSCAPE_GUIDE_BBOX:
		if (value) {
			if (sp_str_to_bool (value)) {
				nv->guide_snap_to |= SNAP_TO_BBOX;
			} else {
				nv->guide_snap_to &= ~SNAP_TO_BBOX;
			}
			sp_object_request_modified (object, SP_OBJECT_MODIFIED_FLAG);
		}
		break;
	case SP_ATTR_INKSCAPE_GRID_POINTS:
		if (value) {
			if (sp_str_to_bool (value)) {
				nv->grid_snap_to |= SNAP_TO_POINTS;
			} else {
				nv->grid_snap_to &= ~SNAP_TO_POINTS;
			}
			sp_object_request_modified (object, SP_OBJECT_MODIFIED_FLAG);
		}
		break;
	case SP_ATTR_INKSCAPE_GUIDE_POINTS:
		if (value) {
			if (sp_str_to_bool (value)) {
				nv->guide_snap_to |= SNAP_TO_POINTS;
			} else {
				nv->guide_snap_to &= ~SNAP_TO_POINTS;
			}
			sp_object_request_modified (object, SP_OBJECT_MODIFIED_FLAG);
		}
		break;
	default:
		if (((SPObjectClass *) (parent_class))->set)
			((SPObjectClass *) (parent_class))->set (object, key, value);
		break;
	}
}

static void
sp_namedview_child_added (SPObject * object, SPRepr * child, SPRepr * ref)
{
	SPNamedView * nv;
	SPObject * no;
	const gchar * id;
	GSList *l, *v;

	nv = (SPNamedView *) object;

	if (((SPObjectClass *) (parent_class))->child_added)
		(* ((SPObjectClass *) (parent_class))->child_added) (object, child, ref);

	id = sp_repr_attr (child, "id");
	no = object->document->getObjectById(id);
	g_assert (SP_IS_OBJECT (no));

	if (SP_IS_GUIDE (no)) {
		SPGuide * g;
		g = (SPGuide *) no;
		nv->guides = g_slist_prepend (nv->guides, g);
		g_object_set (G_OBJECT (g), "color", nv->guidecolor, "hicolor", nv->guidehicolor, NULL);
		if (nv->editable) {
			for (l = nv->views; l != NULL; l = l->next) {
				sp_guide_show (g, SP_DESKTOP (l->data)->guides, (GCallback)sp_dt_guide_event);
				if (SP_DESKTOP (l->data)->guides_active) 
					sp_guide_sensitize (g, 
									SP_DT_CANVAS(SP_DESKTOP (l->data)), 
									TRUE);
				if (nv->showguides) {
					for (v = SP_GUIDE (g)->views; v != NULL; v = v->next) {
						sp_canvas_item_show (SP_CANVAS_ITEM (v->data));
					}
				} else {
					for (v = SP_GUIDE (g)->views; v != NULL; v = v->next) {
						sp_canvas_item_hide (SP_CANVAS_ITEM (v->data));
					}
				}
			}
		}
	}
}

static void
sp_namedview_remove_child (SPObject * object, SPRepr * child)
{
	SPNamedView * nv;

	nv = (SPNamedView *) object;

	GSList *iter;
	GSList **ref;

	ref = &nv->guides;
	for ( iter = nv->guides ; iter ; iter = iter->next ) {
		if ( SP_OBJECT_REPR((SPObject *)iter->data) == child ) {
			*ref = iter->next;
			iter->next = NULL;
			g_slist_free_1(iter);
			break;
		}
		ref = &iter->next;
	}

	if (((SPObjectClass *) (parent_class))->remove_child)
		(* ((SPObjectClass *) (parent_class))->remove_child) (object, child);
}

static SPRepr *
sp_namedview_write (SPObject *object, SPRepr *repr, guint flags)
{
	SPNamedView *nv;

	nv = SP_NAMEDVIEW (object);

	if (flags & SP_OBJECT_WRITE_EXT) {
		if (repr) {
			sp_repr_merge (repr, SP_OBJECT_REPR (object), "id");
		} else {
			repr = sp_repr_duplicate (SP_OBJECT_REPR (object));
		}
	}

	return repr;
}

void
sp_namedview_show (SPNamedView * nv, gpointer desktop)
{
	SPDesktop * dt;
	GSList * l, *v;
	SPCanvasItem * item;

	dt = SP_DESKTOP (desktop);

	for (l = nv->guides; l != NULL; l = l->next) {
		sp_guide_show (SP_GUIDE (l->data), dt->guides, (GCallback)sp_dt_guide_event);
		if (dt->guides_active) sp_guide_sensitize (SP_GUIDE (l->data), SP_DT_CANVAS (dt), TRUE);
		if (nv->showguides) {
			for (v = SP_GUIDE (l->data)->views; v != NULL; v = v->next) {
				sp_canvas_item_show (SP_CANVAS_ITEM (v->data));
			}
		} else {
			for (v = SP_GUIDE (l->data)->views; v != NULL; v = v->next) {
				sp_canvas_item_hide (SP_CANVAS_ITEM (v->data));
			}
		}
	}

	nv->views = g_slist_prepend (nv->views, desktop);

	item = sp_canvas_item_new (SP_DT_GRID (dt), SP_TYPE_CGRID, NULL);
	nv->gridviews = g_slist_prepend (nv->gridviews, item);
	sp_namedview_setup_grid_item (nv, item);
}

/*
 * Restores window geometry from the document settings
 */
void 
sp_namedview_window_from_document (SPDesktop *desktop)
{
	SPNamedView *nv = desktop->namedview;
	GtkWindow *win = GTK_WINDOW(gtk_object_get_data (GTK_OBJECT(desktop->owner), "window"));
	gint save_geometry = prefs_get_int_attribute ("options.savewindowgeometry", "value", 0);

	// restore window size and position
	if (save_geometry && win) {
		if (nv->window_width != -1 && nv->window_height != -1) {
			gtk_window_set_default_size (win, nv->window_width, nv->window_height);
			gtk_window_reshow_with_initial_size (win);
		}
		if (nv->window_x != -1 && nv->window_y != -1) {
			gtk_window_move (win, nv->window_x, nv->window_y);
		}
	}

	// restore zoom and view
	if (nv->zoom != 0 && nv->zoom != HUGE_VAL && !isnan (nv->zoom)
			&& nv->cx != HUGE_VAL && !isnan (nv->cx) 
			&& nv->cy != HUGE_VAL && !isnan (nv->cy)) {
		sp_desktop_zoom_absolute (desktop, nv->cx, nv->cy, nv->zoom);
	} 

	// cancel any history of zooms up to this point
	if (desktop->zooms_past) {
		g_list_free (desktop->zooms_past);
		desktop->zooms_past = NULL;
	}
}

void 
sp_namedview_document_from_window (SPDesktop *desktop)
{
	SPRepr *view;
	NRRect r;
	gint w, h, x, y;
	GtkWindow *win = GTK_WINDOW(gtk_object_get_data (GTK_OBJECT(desktop->owner), "window"));
	gint save_geometry = prefs_get_int_attribute ("options.savewindowgeometry", "value", 0);
	view = SP_OBJECT_REPR (desktop->namedview);
	sp_desktop_get_display_area (desktop, &r);

	// saving window geometry is not undoable
	sp_document_set_undo_sensitive (SP_DT_DOCUMENT (desktop), FALSE);

	sp_repr_set_double (view, "inkscape:zoom", SP_DESKTOP_ZOOM (desktop));
	sp_repr_set_double (view, "inkscape:cx", (r.x0+r.x1)*0.5);
	sp_repr_set_double (view, "inkscape:cy", (r.y0+r.y1)*0.5);

	if (save_geometry && win) {
		gtk_window_get_size (win, &w, &h);
		gtk_window_get_position (win, &x, &y);
		sp_repr_set_int (view, "inkscape:window-width", w);
		sp_repr_set_int (view, "inkscape:window-height", h);
		sp_repr_set_int (view, "inkscape:window-x", x);
		sp_repr_set_int (view, "inkscape:window-y", y);
	}

	// restore undoability
	sp_document_set_undo_sensitive (SP_DT_DOCUMENT (desktop), TRUE);
}

void
sp_namedview_hide (SPNamedView * nv, gpointer desktop)
{
	SPDesktop * dt;
	GSList * l;

	g_assert (nv != NULL);
	g_assert (SP_IS_NAMEDVIEW (nv));
	g_assert (desktop != NULL);
	g_assert (SP_IS_DESKTOP (desktop));
	g_assert (g_slist_find (nv->views, desktop));

	dt = SP_DESKTOP (desktop);

	for (l = nv->guides; l != NULL; l = l->next) {
		sp_guide_hide (SP_GUIDE (l->data), SP_DT_CANVAS (dt));
	}

	nv->views = g_slist_remove (nv->views, desktop);

	for (l = nv->gridviews; l != NULL; l = l->next) {
		if (SP_CANVAS_ITEM (l->data)->canvas == SP_DT_CANVAS (dt)) break;
	}

	g_assert (l);

	gtk_object_destroy (GTK_OBJECT (l->data));
	nv->gridviews = g_slist_remove (nv->gridviews, l->data);
}

void
sp_namedview_activate_guides (SPNamedView * nv, gpointer desktop, gboolean active)
{
	SPDesktop * dt;
	GSList * l;

	g_assert (nv != NULL);
	g_assert (SP_IS_NAMEDVIEW (nv));
	g_assert (desktop != NULL);
	g_assert (SP_IS_DESKTOP (desktop));
	g_assert (g_slist_find (nv->views, desktop));

	dt = SP_DESKTOP (desktop);

	for (l = nv->guides; l != NULL; l = l->next) {
		sp_guide_sensitize (SP_GUIDE (l->data), SP_DT_CANVAS (dt), active);
	}
}

static void
sp_namedview_setup_guides (SPNamedView *nv)
{
	GSList *l, *v;

	for (l = nv->guides; l != NULL; l = l->next) {
		if (nv->showguides) {
			for (v = SP_GUIDE (l->data)->views; v != NULL; v = v->next) {
				sp_canvas_item_show (SP_CANVAS_ITEM (v->data));
			}
		} else {
			for (v = SP_GUIDE (l->data)->views; v != NULL; v = v->next) {
				sp_canvas_item_hide (SP_CANVAS_ITEM (v->data));
			}
		}
	}
}

void
sp_namedview_toggle_guides (SPRepr *repr)
{
	unsigned int v;
	unsigned int set = sp_repr_get_boolean (repr, "showguides", &v);
	if (!set) { // hide guides if not specified, for backwards compatibility
		v = FALSE;
	} else {
		v = !v;
	}
	sp_repr_set_boolean (repr, "showguides", v);

	if (v) {
		sp_repr_set_boolean (repr, "snaptoguides", TRUE);
	} else {
		sp_repr_set_boolean (repr, "snaptoguides", FALSE);
	}
}

void
sp_namedview_toggle_grid (SPRepr *repr)
{
	unsigned int v;
	sp_repr_get_boolean (repr, "showgrid", &v);
	v = !v;
	sp_repr_set_boolean (repr, "showgrid", v);

	if (v) {
		sp_repr_set_boolean (repr, "snaptogrid", TRUE);
	} else {
		sp_repr_set_boolean (repr, "snaptogrid", FALSE);
	}
}

static void
sp_namedview_setup_grid (SPNamedView * nv)
{
	GSList * l;

	for (l = nv->gridviews; l != NULL; l = l->next) {
		sp_namedview_setup_grid_item (nv, SP_CANVAS_ITEM (l->data));
	}
}

static void
sp_namedview_setup_grid_item (SPNamedView * nv, SPCanvasItem * item)
{
	const SPUnit *pt = NULL;

	if (nv->showgrid) {
		sp_canvas_item_show (item);
	} else {
		sp_canvas_item_hide (item);
	}

	if (!pt) pt = sp_unit_get_identity (SP_UNIT_ABSOLUTE);

	sp_canvas_item_set ((GtkObject *) item,
			       "color", nv->gridcolor,
			       "originx", nv->gridorigin[NR::X],
			       "originy", nv->gridorigin[NR::Y],
			       "spacingx", nv->gridspacing[NR::X],
			       "spacingy", nv->gridspacing[NR::Y],
			       NULL);
}

gchar const *sp_namedview_get_name(SPNamedView *nv)
{
	SPException ex;
	SP_EXCEPTION_INIT (&ex);
	return sp_object_getAttribute(SP_OBJECT(nv), "id", &ex);
}

guint sp_namedview_viewcount(SPNamedView *nv)
{
 g_assert (SP_IS_NAMEDVIEW (nv));

 return ++nv->viewcount;
}

GSList const *sp_namedview_view_list(SPNamedView *nv)
{
 g_assert (SP_IS_NAMEDVIEW (nv));

 return nv->views;
}

/* This should be moved somewhere */

static gboolean
sp_str_to_bool (const gchar *str)
{
	if (str) {
		if (!g_strcasecmp (str, "true") ||
		    !g_strcasecmp (str, "yes") ||
		    !g_strcasecmp (str, "y") ||
		    (atoi (str) != 0)) return TRUE;
	}

	return FALSE;
}

/* fixme: Collect all these length parsing methods and think common sane API */

static gboolean
sp_nv_read_length (const gchar *str, guint base, gdouble *val, const SPUnit **unit)
{
	gdouble v;
	gchar *u;

	if (!str) return FALSE;

	v = g_ascii_strtod (str, &u);
	if (!u) return FALSE;
	while (isspace (*u)) u += 1;

	if (!*u) {
		/* No unit specified - keep default */
		*val = v;
		return TRUE;
	}

	if (base & SP_UNIT_DEVICE) {
		if (u[0] && u[1] && !isalnum (u[2]) && !strncmp (u, "px", 2)) {
			static const SPUnit *device = NULL;
			if (!device) device = sp_unit_get_identity (SP_UNIT_DEVICE);
			*unit = device;
			*val = v;
			return TRUE;
		}
	}

	if (base & SP_UNIT_ABSOLUTE) {
		static const SPUnit *pt = NULL;
		static const SPUnit *mm = NULL;
		static const SPUnit *cm = NULL;
		static const SPUnit *m = NULL;
		static const SPUnit *in = NULL;
		if (!pt) {
			pt = sp_unit_get_by_abbreviation ("pt");
			mm = sp_unit_get_by_abbreviation ("mm");
			cm = sp_unit_get_by_abbreviation ("cm");
			m = sp_unit_get_by_abbreviation ("m");
			in = sp_unit_get_by_abbreviation ("in");
		}
		if (!strncmp (u, "pt", 2)) {
			*unit = pt;
		} else if (!strncmp (u, "mm", 2)) {
			*unit = mm;
		} else if (!strncmp (u, "cm", 2)) {
			*unit = cm;
		} else if (!strncmp (u, "m", 1)) {
			*unit = m;
		} else if (!strncmp (u, "in", 2)) {
			*unit = in;
		} else {
			return FALSE;
		}
		*val = v;
		return TRUE;
	}

	return FALSE;
}

static gboolean
sp_nv_read_opacity (const gchar *str, guint32 *color)
{
	gdouble v;
	gchar *u;

	if (!str) return FALSE;

	v = strtod (str, &u);
	if (!u) return FALSE;
	v = CLAMP (v, 0.0, 1.0);

	*color = (*color & 0xffffff00) | (guint32) floor (v * 255.9999);

	return TRUE;
}

SPNamedView *
sp_document_namedview (SPDocument *document, const gchar *id)
{
	SPObject *nv;

	g_return_val_if_fail (document != NULL, NULL);
	g_return_val_if_fail (SP_IS_DOCUMENT (document), NULL);

	nv = sp_item_group_get_child_by_name ((SPGroup *) document->root, NULL, "sodipodi:namedview");
	g_assert (nv != NULL);

	if (id == NULL) return (SPNamedView *) nv;

	while (nv && strcmp (nv->id, id)) {
		nv = sp_item_group_get_child_by_name ((SPGroup *) document->root, nv, "sodipodi:namedview");
	}

	return (SPNamedView *) nv;
}
