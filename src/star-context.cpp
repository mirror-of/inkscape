#define __SP_STAR_CONTEXT_C__

/*
 * Star drawing context
 *
 * Authors:
 *   Mitsuru Oka <oka326@parkcity.ne.jp>
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * Copyright (C) 1999-2002 Lauris Kaplinski
 * Copyright (C) 2001-2002 Mitsuru Oka
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "config.h"

#include <math.h>
#include <string.h>
#include <glib.h>
#include <gtk/gtksignal.h>
#include <gtk/gtktable.h>
#include <gtk/gtkbutton.h>
#include <gtk/gtkspinbutton.h>
#include <gtk/gtklabel.h>
#include <gdk/gdkkeysyms.h>
#include <gtk/gtkcheckbutton.h>

#include "macros.h"
#include "display/sp-canvas.h"
#include "sp-star.h"
#include "inkscape.h"
#include "document.h"
#include "selection.h"
#include "desktop-handles.h"
#include "desktop-affine.h"
#include "desktop-snap.h"
#include "desktop.h"
#include "pixmaps/cursor-star.xpm"
#include "sp-metrics.h"
#include "helper/sp-intl.h"
#include "prefs-utils.h"
#include "knotholder.h"
#include "xml/repr.h"
#include "xml/repr-private.h"
#include "object-edit.h"
#include "widgets/spw-utilities.h"
#include <libnr/nr-point-fns.h>

#include "star-context.h"

static void sp_star_context_class_init (SPStarContextClass * klass);
static void sp_star_context_init (SPStarContext * star_context);
static void sp_star_context_dispose (GObject *object);

static void sp_star_context_setup (SPEventContext *ec);
static void sp_star_context_set (SPEventContext *ec, const gchar *key, const gchar *val);
static gint sp_star_context_root_handler (SPEventContext *ec, GdkEvent *event);

static void sp_star_drag (SPStarContext * sc, NR::Point p, guint state);
static void sp_star_finish (SPStarContext * sc);

static SPEventContextClass * parent_class;

GtkType
sp_star_context_get_type (void)
{
    static GType type = 0;
    if (!type) {
        GTypeInfo info = {
            sizeof (SPStarContextClass),
            NULL, NULL,
            (GClassInitFunc) sp_star_context_class_init,
            NULL, NULL,
            sizeof (SPStarContext),
            4,
            (GInstanceInitFunc) sp_star_context_init,
            NULL,    /* value_table */
        };
        type = g_type_register_static (SP_TYPE_EVENT_CONTEXT, "SPStarContext", &info, (GTypeFlags)0);
    }
    return type;
}

static void
sp_star_context_class_init (SPStarContextClass * klass)
{
    GObjectClass *object_class = (GObjectClass *) klass;
    SPEventContextClass *event_context_class = (SPEventContextClass *) klass;

    parent_class = (SPEventContextClass*)g_type_class_peek_parent (klass);

    object_class->dispose = sp_star_context_dispose;

    event_context_class->setup = sp_star_context_setup;
    event_context_class->set = sp_star_context_set;
    event_context_class->root_handler = sp_star_context_root_handler;
}

static void
sp_star_context_init (SPStarContext * star_context)
{
    SPEventContext *event_context = SP_EVENT_CONTEXT (star_context);

    event_context->cursor_shape = cursor_star_xpm;
    event_context->hot_x = 4;
    event_context->hot_y = 4;
    event_context->xp = 0;
    event_context->yp = 0;
    event_context->tolerance = 0;
    event_context->within_tolerance = false;
    event_context->item_to_select = NULL;

    star_context->item = NULL;

    star_context->magnitude = 5;
    star_context->proportion = 0.5;
    star_context->isflatsided = false;

    new (&star_context->sel_changed_connection) SigC::Connection();
}

static void
sp_star_context_dispose (GObject *object)
{
    SPEventContext *ec = SP_EVENT_CONTEXT (object);

    SPStarContext *sc = SP_STAR_CONTEXT (object);

    sc->sel_changed_connection.disconnect();
    sc->sel_changed_connection.~Connection();

    if (sc->knot_holder) {
        sp_knot_holder_destroy (sc->knot_holder);
        sc->knot_holder = NULL;
    }

    if (sc->repr) { // remove old listener
        sp_repr_remove_listener_by_data (sc->repr, ec);
        sp_repr_unref (sc->repr);
        sc->repr = 0;
    }

    /* fixme: This is necessary because we do not grab */
    if (sc->item) sp_star_finish (sc);

    sp_sel_cue_shutdown(&(ec->selcue));

    G_OBJECT_CLASS (parent_class)->dispose (object);
}

static void shape_event_attr_changed (SPRepr * repr, const gchar * name, const gchar * old_value, const gchar * new_value, bool is_interactive, gpointer data)
{
    SPStarContext *sc = SP_STAR_CONTEXT(data);
    SPEventContext *ec = SP_EVENT_CONTEXT(sc);

    if (sc->knot_holder) {
        sp_knot_holder_destroy (sc->knot_holder);
    }
    sc->knot_holder = NULL;

    SPDesktop *desktop = ec->desktop;

    SPItem *item = SP_DT_SELECTION(desktop)->singleItem();

    if (item) {
        sc->knot_holder = sp_item_knot_holder (item, desktop);
    }
}

static SPReprEventVector shape_repr_events = {
    NULL, /* destroy */
    NULL, /* add_child */
    NULL, /* child_added */
    NULL, /* remove_child */
    NULL, /* child_removed */
    NULL, /* change_attr */
    shape_event_attr_changed,
    NULL, /* change_list */
    NULL, /* content_changed */
    NULL, /* change_order */
    NULL  /* order_changed */
};

/**
\brief  Callback that processes the "changed" signal on the selection;
destroys old and creates new knotholder
\param  selection Should not be NULL.
*/
void
sp_star_context_selection_changed (SPSelection * selection, gpointer data)
{
    g_assert (selection != NULL);

    SPStarContext *sc = SP_STAR_CONTEXT (data);
    SPEventContext *ec = SP_EVENT_CONTEXT (sc);

    if (sc->knot_holder) { // desktroy knotholder
        sp_knot_holder_destroy (sc->knot_holder);
        sc->knot_holder = NULL;
    }

    if (sc->repr) { // remove old listener
        sp_repr_remove_listener_by_data (sc->repr, ec);
        sp_repr_unref (sc->repr);
        sc->repr = 0;
    }

    SPItem *item = selection->singleItem();
    if (item) {
        sc->knot_holder = sp_item_knot_holder (item, ec->desktop);
        SPRepr *repr = SP_OBJECT_REPR (item);
        if (repr) {
            sc->repr = repr;
            sp_repr_ref (repr);
            sp_repr_add_listener (repr, &shape_repr_events, ec);
            sp_repr_synthesize_events (repr, &shape_repr_events, ec);
        }


    }
}

static void
sp_star_context_setup (SPEventContext *ec)
{
   SPStarContext *sc = SP_STAR_CONTEXT (ec);

    if (((SPEventContextClass *) parent_class)->setup)
        ((SPEventContextClass *) parent_class)->setup (ec);

    sp_event_context_read (ec, "magnitude");
    sp_event_context_read (ec, "proportion");
    sp_event_context_read (ec, "isflatsided");

    SPSelection *selection = SP_DT_SELECTION(ec->desktop);

    SPItem *item = selection->singleItem();
        if (item) {
            sc->knot_holder = sp_item_knot_holder (item, ec->desktop);
            SPRepr *repr = SP_OBJECT_REPR (item);
            if (repr) {
                sc->repr = repr;
                sp_repr_ref (repr);
                sp_repr_add_listener (repr, &shape_repr_events, ec);
                sp_repr_synthesize_events (repr, &shape_repr_events, ec);
            }
        }

    sc->sel_changed_connection.disconnect();
    sc->sel_changed_connection = selection->connectChanged(SigC::bind(SigC::slot(&sp_star_context_selection_changed), (gpointer)sc));

    if (prefs_get_int_attribute("tools.shapes", "selcue", 0) != 0)
		sp_sel_cue_init(&(ec->selcue), ec->desktop);
}

static void
sp_star_context_set (SPEventContext *ec, const gchar *key, const gchar *val)
{
    SPStarContext *sc = SP_STAR_CONTEXT (ec);
    if (!strcmp (key, "magnitude")) {
        sc->magnitude = (val) ? atoi (val) : 5;
        sc->magnitude = CLAMP (sc->magnitude, 3, 32);
    } else if (!strcmp (key, "proportion")) {
        sc->proportion = (val) ? g_ascii_strtod (val, NULL) : 0.5;
        sc->proportion = CLAMP (sc->proportion, 0.01, 2.0);
    } else if (!strcmp (key, "isflatsided")) {
        if (val && !strcmp(val, "true"))
            sc->isflatsided = true;
        else
            sc->isflatsided = false;
    }
}

static gint
sp_star_context_item_handler (SPEventContext * event_context, SPItem * item, GdkEvent * event)
{
    SPDesktop *desktop = event_context->desktop;

    gint ret = FALSE;

    switch (event->type) {
    case GDK_BUTTON_PRESS:
        if (event->button.button == 1) {

            // save drag origin
            event_context->xp = (gint) event->button.x;
            event_context->yp = (gint) event->button.y;
            event_context->within_tolerance = true;

            // remember clicked item, disregarding groups
            event_context->item_to_select = sp_desktop_item_at_point (desktop, NR::Point(event->button.x, event->button.y), TRUE);

            ret = TRUE;
        }
        break;
        // motion and release are always on root (why?)
    default:
        break;
    }

    if (((SPEventContextClass *) parent_class)->item_handler)
        ret = ((SPEventContextClass *) parent_class)->item_handler (event_context, item, event);

    return ret;
}


static gint
sp_star_context_root_handler (SPEventContext * event_context, GdkEvent * event)
{
    static gboolean dragging;

    SPDesktop *desktop = event_context->desktop;
    SPStarContext *sc = SP_STAR_CONTEXT (event_context);

    event_context->tolerance = prefs_get_int_attribute_limited ("options.dragtolerance", "value", 0, 0, 100);

    gint ret = FALSE;


    switch (event->type) {
    case GDK_BUTTON_PRESS:
        if (event->button.button == 1) {
           // save drag origin
            event_context->xp = (gint) event->button.x;
            event_context->yp = (gint) event->button.y;
            event_context->within_tolerance = true;

           // remember clicked item, disregarding groups
            event_context->item_to_select = sp_desktop_item_at_point (desktop, NR::Point(event->button.x, event->button.y), TRUE);

            dragging = TRUE;
            /* Position center */
            sc->center = sp_desktop_w2d_xy_point (event_context->desktop, NR::Point(event->button.x, event->button.y));
            /* Snap center to nearest magnetic point */
            sp_desktop_free_snap (event_context->desktop, Snapper::SNAP_POINT, sc->center);
            sp_canvas_item_grab (SP_CANVAS_ITEM (desktop->acetate),
                                 GDK_BUTTON_RELEASE_MASK | GDK_POINTER_MOTION_MASK | GDK_BUTTON_PRESS_MASK,
                                 NULL, event->button.time);
            ret = TRUE;
        }
        break;
    case GDK_MOTION_NOTIFY:
        if (dragging && (event->motion.state & GDK_BUTTON1_MASK)) {

            if ( event_context->within_tolerance
                 && ( abs( (gint) event->motion.x - event_context->xp ) < event_context->tolerance )
                 && ( abs( (gint) event->motion.y - event_context->yp ) < event_context->tolerance ) ) {
                break; // do not drag if we're within tolerance from origin
            }
            // Once the user has moved farther than tolerance from the original location
            // (indicating they intend to draw, not click), then always process the
            // motion notify coordinates as given (no snapping back to origin)
            event_context->within_tolerance = false;

            NR::Point const motion_w(event->motion.x, event->motion.y);
            NR::Point const motion_dt(sp_desktop_w2d_xy_point(event_context->desktop, motion_w));
            sp_star_drag (sc, motion_dt, event->motion.state);
            ret = TRUE;
        }
        break;
    case GDK_BUTTON_RELEASE:
        event_context->xp = event_context->yp = 0;
        if (event->button.button == 1) {
            dragging = FALSE;
            if (!event_context->within_tolerance) {
                // we've been dragging, finish the star
            sp_star_finish (sc);
            } else if (event_context->item_to_select) {
                // no dragging, select clicked item if any
                SP_DT_SELECTION(desktop)->setItem(event_context->item_to_select);
            } else {
                // click in an empty space
                SP_DT_SELECTION(desktop)->clear();
            }

            event_context->item_to_select = NULL;
            ret = TRUE;
            sp_canvas_item_ungrab (SP_CANVAS_ITEM (desktop->acetate), event->button.time);
        }
        break;
    case GDK_KEY_PRESS:
        switch (event->key.keyval) {
        case GDK_Up:
        case GDK_Down:
        case GDK_KP_Up:
        case GDK_KP_Down:
            // prevent the zoom field from activation
            if (!MOD__CTRL_ONLY)
                ret = TRUE;
            break;
        case GDK_x:
        case GDK_X:
            if (MOD__ALT_ONLY) {
                gpointer hb = sp_search_by_data_recursive (desktop->owner->aux_toolbox, (gpointer) "altx-star");
                if (hb && GTK_IS_WIDGET(hb)) {
                    gtk_widget_grab_focus (GTK_WIDGET (hb));
                }
                ret = TRUE;
            }
            break;
        case GDK_Escape:
            SP_DT_SELECTION(desktop)->clear();
            //TODO: make dragging escapable by Esc
        default:
            break;
        }
    default:
        break;
    }

    if (!ret) {
        if (((SPEventContextClass *) parent_class)->root_handler)
            ret = ((SPEventContextClass *) parent_class)->root_handler (event_context, event);
    }

    return ret;
}

static void
sp_star_drag(SPStarContext *sc, NR::Point p, guint state)
{
    SPDesktop *desktop = SP_EVENT_CONTEXT(sc)->desktop;

    int snaps = prefs_get_int_attribute ("options.rotationsnapsperpi", "value", 12);

    if (!sc->item) {
        /* Create object */
        SPRepr *repr = sp_repr_new ("polygon");
        sp_repr_set_attr (repr, "sodipodi:type", "star");
        /* Set style */
        SPRepr *style = inkscape_get_repr (INKSCAPE, "tools.shapes.star");
        if (style) {
            SPCSSAttr *css = sp_repr_css_attr_inherited (style, "style");
            sp_repr_css_set (repr, css, "style");
            sp_repr_css_attr_unref (css);
        }
        sc->item = SP_ITEM(desktop->currentLayer()->appendChildRepr(repr));
        sp_repr_unref (repr);
    }

    /* Free movement for corner point */
    NR::Point p0 = sp_desktop_dt2root_xy_point (desktop, sc->center);
    NR::Point p1 = sp_desktop_dt2root_xy_point (desktop, p);
    sp_desktop_free_snap (desktop, Snapper::SNAP_POINT, p1);

    SPStar *star = SP_STAR(sc->item);

    gdouble sides = (gdouble) sc->magnitude;
    NR::Point d = p1 - p0;
    gdouble r1 = NR::L2 (d);
    gdouble arg1 = atan2 (d);
    if (state & GDK_CONTROL_MASK) {
        arg1 = ( round( arg1 / (M_PI/snaps) )
                 * (M_PI/snaps) );
    }
    bool isflat = sc->isflatsided;
    sp_star_position_set(star, sc->magnitude, p0, r1, r1 * sc->proportion,
                         arg1, arg1 + M_PI / sides, isflat);

    /* status text */
    GString *xs = SP_PT_TO_METRIC_STRING (fabs(p0[NR::X]), SP_DEFAULT_METRIC);
    GString *ys = SP_PT_TO_METRIC_STRING (fabs(p0[NR::Y]), SP_DEFAULT_METRIC);
    gchar *status = g_strdup_printf(( isflat
                                      ? _("Draw polygon at (%s,%s)")
                                      : _("Draw star at (%s,%s)") ),
                                    xs->str, ys->str);
    sp_view_set_status(SP_VIEW(desktop), status, FALSE);
    g_free(status);
    g_string_free(xs, FALSE);
    g_string_free(ys, FALSE);
}

static void
sp_star_finish (SPStarContext * sc)
{
    if (sc->item != NULL) {
        SPDesktop *desktop = SP_EVENT_CONTEXT(sc)->desktop;
        SPObject *object = SP_OBJECT(sc->item);

        sp_shape_set_shape(SP_SHAPE(sc->item));

        object->updateRepr(NULL, SP_OBJECT_WRITE_EXT);

        SP_DT_SELECTION(desktop)->setItem(sc->item);
        sp_document_done(SP_DT_DOCUMENT(desktop));

        sc->item = NULL;
    }
}

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=c++:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :
