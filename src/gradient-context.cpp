#define __SP_GRADIENT_CONTEXT_C__

/*
 * Gradient drawing and editing tool
 *
 * Authors:
 *   bulia byak <buliabyak@users.sf.net>
 *
 * Copyright (C) 2005 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "config.h"

#include <math.h>
#include <string.h>
#include <glib.h>
#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>

#include "macros.h"
#include "display/sp-canvas.h"
#include "sp-rect.h"
#include "inkscape.h"
#include "document.h"
#include "selection.h"
#include "desktop-handles.h"
#include "desktop-affine.h"
#include "snap.h"
#include "desktop.h"
#include "desktop-style.h"
#include "pixmaps/cursor-gradient.xpm"
#include "gradient-context.h"
#include "sp-desktop-widget.h"
#include "sp-metrics.h"
#include <glibmm/i18n.h>
#include "object-edit.h"
#include "knotholder.h"
#include "xml/repr.h"
#include "xml/node-event-vector.h"
#include "prefs-utils.h"
#include "widgets/spw-utilities.h"
#include "selcue.h"
#include "gradient-drag.h"
#include "sp-gradient.h"
#include "gradient-chemistry.h"

static void sp_gradient_context_class_init(SPGradientContextClass *klass);
static void sp_gradient_context_init(SPGradientContext *rect_context);
static void sp_gradient_context_dispose(GObject *object);

static void sp_gradient_context_setup(SPEventContext *ec);

static gint sp_gradient_context_root_handler(SPEventContext *event_context, GdkEvent *event);
static gint sp_gradient_context_item_handler(SPEventContext *event_context, SPItem *item, GdkEvent *event);

static void sp_gradient_drag(SPGradientContext &rc, NR::Point const pt, guint state, guint32 etime);

static SPEventContextClass *parent_class;


GtkType sp_gradient_context_get_type()
{
    static GType type = 0;
    if (!type) {
        GTypeInfo info = {
            sizeof(SPGradientContextClass),
            NULL, NULL,
            (GClassInitFunc) sp_gradient_context_class_init,
            NULL, NULL,
            sizeof(SPGradientContext),
            4,
            (GInstanceInitFunc) sp_gradient_context_init,
            NULL,    /* value_table */
        };
        type = g_type_register_static(SP_TYPE_EVENT_CONTEXT, "SPGradientContext", &info, (GTypeFlags) 0);
    }
    return type;
}

static void sp_gradient_context_class_init(SPGradientContextClass *klass)
{
    GObjectClass *object_class = (GObjectClass *) klass;
    SPEventContextClass *event_context_class = (SPEventContextClass *) klass;

    parent_class = (SPEventContextClass *) g_type_class_peek_parent(klass);

    object_class->dispose = sp_gradient_context_dispose;

    event_context_class->setup = sp_gradient_context_setup;
    event_context_class->root_handler  = sp_gradient_context_root_handler;
    event_context_class->item_handler  = sp_gradient_context_item_handler;
}

static void sp_gradient_context_init(SPGradientContext *rect_context)
{
    SPEventContext *event_context = SP_EVENT_CONTEXT(rect_context);

    event_context->cursor_shape = cursor_gradient_xpm;
    event_context->hot_x = 4;
    event_context->hot_y = 4;
    event_context->xp = 0;
    event_context->yp = 0;
    event_context->tolerance = 0;
    event_context->within_tolerance = false;
    event_context->item_to_select = NULL;

    rect_context->knot_holder = NULL;

    new (&rect_context->sel_changed_connection) sigc::connection();
}

static void sp_gradient_context_dispose(GObject *object)
{
    SPGradientContext *rc = SP_GRADIENT_CONTEXT(object);
    SPEventContext *ec = SP_EVENT_CONTEXT(object);

    ec->enableGrDrag(false);

    rc->sel_changed_connection.disconnect();
    rc->sel_changed_connection.~connection();

    if (rc->knot_holder) {
        sp_knot_holder_destroy(rc->knot_holder);
        rc->knot_holder = NULL;
    }

    if (rc->repr) { // remove old listener
        sp_repr_remove_listener_by_data(rc->repr, ec);
        sp_repr_unref(rc->repr);
        rc->repr = 0;
    }

    if (rc->_message_context) {
        delete rc->_message_context;
    }

    G_OBJECT_CLASS(parent_class)->dispose(object);
}

static void shape_event_attr_changed(Inkscape::XML::Node *repr,
                                     gchar const *name, gchar const *old_value, gchar const *new_value,
                                     bool const is_interactive, gpointer const data)
{
    SPGradientContext *rc = SP_GRADIENT_CONTEXT(data);
    SPEventContext *ec = SP_EVENT_CONTEXT(rc);

    if (rc->knot_holder) {
        sp_knot_holder_destroy(rc->knot_holder);
    }
    rc->knot_holder = NULL;

    SPDesktop *desktop = ec->desktop;

    SPItem *item = SP_DT_SELECTION(desktop)->singleItem();

    if (item) {
        rc->knot_holder = sp_item_knot_holder(item, desktop);
    }
}

static Inkscape::XML::NodeEventVector shape_repr_events = {
    NULL, /* child_added */
    NULL, /* child_removed */
    shape_event_attr_changed,
    NULL, /* content_changed */
    NULL  /* order_changed */
};

/**
\brief  Callback that processes the "changed" signal on the selection;
destroys old and creates new knotholder
*/
void sp_gradient_context_selection_changed(SPSelection *selection, gpointer data)
{
    SPGradientContext *rc = SP_GRADIENT_CONTEXT(data);
    SPEventContext *ec = SP_EVENT_CONTEXT(rc);

    if (rc->knot_holder) { // destroy knotholder
        sp_knot_holder_destroy(rc->knot_holder);
        rc->knot_holder = NULL;
    }

    if (rc->repr) { // remove old listener
        sp_repr_remove_listener_by_data(rc->repr, ec);
        sp_repr_unref(rc->repr);
        rc->repr = 0;
    }

    SPItem *item = selection->singleItem();
    if (item) {
        rc->knot_holder = sp_item_knot_holder(item, ec->desktop);
        Inkscape::XML::Node *repr = SP_OBJECT_REPR(item);
        if (repr) {
            rc->repr = repr;
            sp_repr_ref(repr);
            sp_repr_add_listener(repr, &shape_repr_events, ec);
            sp_repr_synthesize_events(repr, &shape_repr_events, ec);
        }
    }
}

static void sp_gradient_context_setup(SPEventContext *ec)
{
    SPGradientContext *rc = SP_GRADIENT_CONTEXT(ec);

    if (((SPEventContextClass *) parent_class)->setup) {
        ((SPEventContextClass *) parent_class)->setup(ec);
    }

    SPItem *item = SP_DT_SELECTION(ec->desktop)->singleItem();
    if (item) {
        rc->knot_holder = sp_item_knot_holder(item, ec->desktop);
        Inkscape::XML::Node *repr = SP_OBJECT_REPR(item);
        if (repr) {
            rc->repr = repr;
            sp_repr_ref(repr);
            sp_repr_add_listener(repr, &shape_repr_events, ec);
            sp_repr_synthesize_events(repr, &shape_repr_events, ec);
        }
    }

    rc->sel_changed_connection.disconnect();
    rc->sel_changed_connection = SP_DT_SELECTION(ec->desktop)->connectChanged(
        sigc::bind(sigc::ptr_fun(&sp_gradient_context_selection_changed), (gpointer)rc)
    );

    if (prefs_get_int_attribute("tools.gradient", "selcue", 1) != 0) {
        ec->enableSelectionCue();
    }

    ec->enableGrDrag();

    rc->_message_context = new Inkscape::MessageContext(SP_VIEW(ec->desktop)->messageStack());
}


static gint sp_gradient_context_item_handler(SPEventContext *event_context, SPItem *item, GdkEvent *event)
{
    SPDesktop *desktop = event_context->desktop;

    gint ret = FALSE;

    switch (event->type) {
    case GDK_BUTTON_PRESS:
        if ( event->button.button == 1 ) {

            // save drag origin
            event_context->xp = (gint) event->button.x;
            event_context->yp = (gint) event->button.y;
            event_context->within_tolerance = true;

            // remember clicked item, disregarding groups
            event_context->item_to_select = sp_desktop_item_at_point(desktop, NR::Point(event->button.x, event->button.y), TRUE);

            ret = TRUE;
        }
        break;
        // motion and release are always on root (why?)
    default:
        break;
    }

    if (((SPEventContextClass *) parent_class)->item_handler) {
        ret = ((SPEventContextClass *) parent_class)->item_handler(event_context, item, event);
    }

    return ret;
}

static gint sp_gradient_context_root_handler(SPEventContext *event_context, GdkEvent *event)
{
    static bool dragging;

    SPDesktop *desktop = event_context->desktop;
 
    SPGradientContext *rc = SP_GRADIENT_CONTEXT(event_context);

    event_context->tolerance = prefs_get_int_attribute_limited("options.dragtolerance", "value", 0, 0, 100);
    double const nudge = prefs_get_double_attribute_limited("options.nudgedistance", "value", 2, 0, 1000); // in px

    GrDrag *drag = event_context->_grdrag;
    g_assert (drag);

    gint ret = FALSE;
    switch (event->type) {
    case GDK_BUTTON_PRESS:
        if ( event->button.button == 1 ) {
            NR::Point const button_w(event->button.x,
                                     event->button.y);

            // save drag origin
            event_context->xp = (gint) button_w[NR::X];
            event_context->yp = (gint) button_w[NR::Y];
            event_context->within_tolerance = true;

            // remember clicked item, disregarding groups
            event_context->item_to_select = sp_desktop_item_at_point(desktop, button_w, TRUE);

            dragging = true;
            /* Position center */
            NR::Point const button_dt(sp_desktop_w2d_xy_point(event_context->desktop, button_w));
            /* Snap center to nearest magnetic point */

            rc->origin = button_dt;

            ret = TRUE;
        }
        break;
    case GDK_MOTION_NOTIFY:
        if ( dragging
             && ( event->motion.state & GDK_BUTTON1_MASK ) )
        {
            if ( event_context->within_tolerance
                 && ( abs( (gint) event->motion.x - event_context->xp ) < event_context->tolerance )
                 && ( abs( (gint) event->motion.y - event_context->yp ) < event_context->tolerance ) ) {
                break; // do not drag if we're within tolerance from origin
            }
            // Once the user has moved farther than tolerance from the original location
            // (indicating they intend to draw, not click), then always process the
            // motion notify coordinates as given (no snapping back to origin)
            event_context->within_tolerance = false;

            NR::Point const motion_w(event->motion.x,
                                     event->motion.y);
            NR::Point const motion_dt(sp_desktop_w2d_xy_point(event_context->desktop, motion_w));
            
            sp_gradient_drag(*rc, motion_dt, event->motion.state, event->motion.time);

            ret = TRUE;
        }
        break;
    case GDK_BUTTON_RELEASE:
        event_context->xp = event_context->yp = 0;
        if ( event->button.button == 1 ) {
            dragging = false;

            if (!event_context->within_tolerance) {
                // we've been dragging, finish
                //sp_gradient_finish(rc);
            } else if (event_context->item_to_select) {
                // no dragging, select clicked item if any
                SP_DT_SELECTION(desktop)->setItem(event_context->item_to_select);
            } else {
                // click in an empty space
                SP_DT_SELECTION(desktop)->clear();
            }

            event_context->item_to_select = NULL;
            ret = TRUE;
        }
        break;
    case GDK_KEY_PRESS:
        switch (get_group0_keyval (&event->key)) {
        case GDK_Alt_L:
        case GDK_Alt_R:
        case GDK_Control_L:
        case GDK_Control_R:
        case GDK_Shift_L:
        case GDK_Shift_R:
        case GDK_Meta_L:  // Meta is when you press Shift+Alt (at least on my machine)
        case GDK_Meta_R:
            sp_event_show_modifier_tip (event_context->defaultMessageContext(), event,
                                        _("<b>Ctrl</b>: snap gradient angle"),
                                        _("<b>Shift</b>: draw gradient around the starting point"),
                                        NULL);
            break;

        case GDK_x:
        case GDK_X:
            if (MOD__ALT_ONLY) {
                gpointer hb = sp_search_by_data_recursive (desktop->owner->aux_toolbox, (gpointer) "altx-grad");
                if (hb && GTK_IS_WIDGET(hb)) {
                    gtk_widget_grab_focus (GTK_WIDGET (hb));
                }
                ret = TRUE;
            }
            break;

        case GDK_Escape:
            SP_DT_SELECTION(desktop)->clear();
            //TODO: make dragging escapable by Esc
            break;

        case GDK_Tab: // Tab - cycle selection forward
            if (!(MOD__CTRL_ONLY || (MOD__CTRL && MOD__SHIFT))) {
                drag->select_next();
                ret = TRUE;
            }
            break;
        case GDK_ISO_Left_Tab:  // Shift Tab - cycle selection backward
            if (!(MOD__CTRL_ONLY || (MOD__CTRL && MOD__SHIFT))) {
                drag->select_prev();
                ret = TRUE;
            }
            break;

        case GDK_Left: // move handle left
        case GDK_KP_Left:
        case GDK_KP_4:
            if (!MOD__CTRL) { // not ctrl
                if (MOD__ALT) { // alt
                    if (MOD__SHIFT) drag->selected_move_screen(-10, 0); // shift
                    else drag->selected_move_screen(-1, 0); // no shift
                }
                else { // no alt
                    if (MOD__SHIFT) drag->selected_move(-10*nudge, 0); // shift
                    else drag->selected_move(-nudge, 0); // no shift
                }
                ret = TRUE;
            }
            break;
        case GDK_Up: // move handle up
        case GDK_KP_Up:
        case GDK_KP_8:
            if (!MOD__CTRL) { // not ctrl
                if (MOD__ALT) { // alt
                    if (MOD__SHIFT) drag->selected_move_screen(0, 10); // shift
                    else drag->selected_move_screen(0, 1); // no shift
                }
                else { // no alt
                    if (MOD__SHIFT) drag->selected_move(0, 10*nudge); // shift
                    else drag->selected_move(0, nudge); // no shift
                }
                ret = TRUE;
            }
            break;
        case GDK_Right: // move handle right
        case GDK_KP_Right:
        case GDK_KP_6:
            if (!MOD__CTRL) { // not ctrl
                if (MOD__ALT) { // alt
                    if (MOD__SHIFT) drag->selected_move_screen(10, 0); // shift
                    else drag->selected_move_screen(1, 0); // no shift
                }
                else { // no alt
                    if (MOD__SHIFT) drag->selected_move(10*nudge, 0); // shift
                    else drag->selected_move(nudge, 0); // no shift
                }
                ret = TRUE;
            }
            break;
        case GDK_Down: // move handle down
        case GDK_KP_Down:
        case GDK_KP_2:
            if (!MOD__CTRL) { // not ctrl
                if (MOD__ALT) { // alt
                    if (MOD__SHIFT) drag->selected_move_screen(0, -10); // shift
                    else drag->selected_move_screen(0, -1); // no shift
                }
                else { // no alt
                    if (MOD__SHIFT) drag->selected_move(0, -10*nudge); // shift
                    else drag->selected_move(0, -nudge); // no shift
                }
                ret = TRUE;
            }
            break;
        default:
            break;
        }
        break;
    case GDK_KEY_RELEASE:
        switch (get_group0_keyval (&event->key)) {
        case GDK_Alt_L:
        case GDK_Alt_R:
        case GDK_Control_L:
        case GDK_Control_R:
        case GDK_Shift_L:
        case GDK_Shift_R:
        case GDK_Meta_L:  // Meta is when you press Shift+Alt
        case GDK_Meta_R:
            event_context->defaultMessageContext()->clear();
            break;
        default:
            break;
        }
        break;
    default:
        break;
    }

    if (!ret) {
        if (((SPEventContextClass *) parent_class)->root_handler) {
            ret = ((SPEventContextClass *) parent_class)->root_handler(event_context, event);
        }
    }

    return ret;
}

static void sp_gradient_drag(SPGradientContext &rc, NR::Point const pt, guint state, guint32 etime)
{
    SPDesktop *desktop = SP_EVENT_CONTEXT(&rc)->desktop;
    SPSelection *selection = SP_DT_SELECTION(desktop);
    SPDocument *document = SP_DT_DOCUMENT(desktop);
    SPEventContext *ec = SP_EVENT_CONTEXT(&rc);

    if (!selection->isEmpty()) {
        int type = prefs_get_int_attribute ("tools.gradient", "newgradient", 1);
        int fill_or_stroke = prefs_get_int_attribute ("tools.gradient", "newfillorstroke", 1);

        SPGradient *vector;
        if (ec->item_to_select) {
            vector = sp_gradient_vector_for_object(document, desktop, ec->item_to_select, fill_or_stroke);
        } else {
            vector = sp_gradient_vector_for_object(document, desktop, SP_ITEM(selection->itemList()->data), fill_or_stroke);
        }

        for (GSList const *i = selection->itemList(); i != NULL; i = i->next) {
            sp_item_set_gradient(SP_ITEM(i->data), vector, (SPGradientType) type, fill_or_stroke);
            if (type == SP_GRADIENT_TYPE_LINEAR) {
                sp_item_gradient_set_coords (SP_ITEM(i->data), POINT_LG_P1, rc.origin, fill_or_stroke, true, false);
                sp_item_gradient_set_coords (SP_ITEM(i->data), POINT_LG_P2, pt, fill_or_stroke, true, false);
            } else if (type == SP_GRADIENT_TYPE_RADIAL) {
                sp_item_gradient_set_coords (SP_ITEM(i->data), POINT_RG_CENTER, rc.origin, fill_or_stroke, true, false);
                sp_item_gradient_set_coords (SP_ITEM(i->data), POINT_RG_R1, pt, fill_or_stroke, true, false);
            }
            SP_OBJECT (i->data)->requestModified(SP_OBJECT_MODIFIED_FLAG);
        }
        if (ec->_grdrag) {
            ec->_grdrag->updateDraggers();
            // prevent regenerating draggers by selection modified signal, which sometimes
            // comes too late and thus destroys the knot which we will now grab:
            ec->_grdrag->local_change = true;
            // give the grab out-of-bounds values of xp/yp because we're already dragging
            // and therefore are already out of tolerance
            ec->_grdrag->grabKnot (SP_ITEM(selection->itemList()->data), 
                                   type == SP_GRADIENT_TYPE_LINEAR? POINT_LG_P2 : POINT_RG_R1, 
                                   fill_or_stroke, 99999, 99999, etime);
        }
        // We did an undoable action, but sp_document_done will be called by the knot when released

        // status text; we do not track coords because this branch is run once, not all the time
        // during drag
        rc._message_context->setF(Inkscape::NORMAL_MESSAGE, _("<b>Gradient</b> for %d objects; with <b>Ctrl</b> to snap angle"), g_slist_length((GSList *) selection->itemList()));
    } else {
        desktop->messageStack()->flash(Inkscape::WARNING_MESSAGE, _("Select <b>objects</b> on which to create gradient."));
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
