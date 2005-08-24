#define __SP_DESKTOP_C__

/** \file
 * Editable view and widget implementation
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   MenTaLguY <mental@rydia.net>
 *   bulia byak <buliabyak@users.sf.net>
 *
 * Copyright (C) 2004 MenTaLguY
 * Copyright (C) 1999-2002 Lauris Kaplinski
 * Copyright (C) 2000-2001 Ximian, Inc.
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

/** \class SPDesktop
 * SPDesktop is a subclass of SPView, implementing an editable document
 * canvas.  It is extensively used by many UI controls that need certain
 * visual representations of their own.
 *
 * SPDesktop provides a certain set of SPCanvasItems, serving as GUI
 * layers of different control objects. The one containing the whole
 * document is the drawing layer. In addition to it, there are grid,
 * guide, sketch and control layers. The sketch layer is used for
 * temporary drawing objects, before the real objects in document are
 * created. The control layer contains editing knots, rubberband and
 * similar non-document UI objects.
 *
 * Each SPDesktop is associated with a SPNamedView node of the document
 * tree.  Currently, all desktops are created from a single main named
 * view, but in the future there may be support for different ones.
 * SPNamedView serves as an in-document container for desktop-related
 * data, like grid and guideline placement, snapping options and so on.
 *
 * Associated with each SPDesktop are the two most important editing
 * related objects - SPSelection and SPEventContext.
 *
 * Sodipodi keeps track of the active desktop and invokes notification
 * signals whenever it changes. UI elements can use these to update their
 * display to the selection of the currently active editing window.
 * (Lauris Kaplinski)
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <glib.h>
#include <glibmm/i18n.h>

#include <gtk/gtklabel.h>

#include "macros.h"
#include "forward.h"
#include "inkscape-private.h"
#include "desktop.h"
#include "desktop-events.h"
#include "desktop-affine.h"
#include "widgets/desktop-widget.h"
#include "document.h"
#include "selection.h"
#include "select-context.h"
#include "sp-namedview.h"
#include "sp-text.h"
#include "sp-item.h"
#include "sp-item-group.h"
#include "prefs-utils.h"
#include "object-hierarchy.h"
#include "helper/sp-marshal.h"
#include "helper/units.h"
#include "display/canvas-arena.h"
#include "display/gnome-canvas-acetate.h"
#include "display/sodipodi-ctrlrect.h"
#include "display/sp-canvas-util.h"
#include "libnr/nr-point-matrix-ops.h"
#include "ui/dialog/dialog-manager.h"
#include "xml/repr.h"

#ifdef WITH_INKBOARD
#include "jabber_whiteboard/session-manager.h"
#endif

namespace Inkscape { namespace XML { class Node; }}

enum {
    ACTIVATE,
    DEACTIVATE,
    MODIFIED,
    EVENT_CONTEXT_CHANGED,
    LAST_SIGNAL
};

static void sp_desktop_class_init(SPDesktopClass *klass);
static void sp_desktop_init(SPDesktop *desktop);
static void sp_desktop_dispose(GObject *object);

static void sp_desktop_request_redraw (Inkscape::UI::View::View *view);
static void sp_desktop_set_document (Inkscape::UI::View::View *view, SPDocument *doc);
static void sp_desktop_document_resized (Inkscape::UI::View::View *view, SPDocument *doc, gdouble width, gdouble height);

/* Constructor */

static void sp_dt_namedview_modified (SPNamedView *nv, guint flags, SPDesktop *desktop);
static void sp_desktop_selection_modified (Inkscape::Selection *selection, guint flags, SPDesktop *desktop);

static void sp_dt_update_snap_distances (SPDesktop *desktop);

SPViewClass *parent_class;
static guint signals[LAST_SIGNAL] = { 0 };

/**
 * Registers SPDesktop class and returns its type.
 */
GType
sp_desktop_get_type (void)
{
    static GType type = 0;
    if (!type) {
        GTypeInfo info = {
            sizeof (SPDesktopClass),
            NULL, NULL,
            (GClassInitFunc) sp_desktop_class_init,
            NULL, NULL,
            sizeof (SPDesktop),
            4,
            (GInstanceInitFunc) sp_desktop_init,
            NULL
        };
        type = g_type_register_static (SP_TYPE_VIEW, "SPDesktop", &info, (GTypeFlags)0);
    }
    return type;
}

/**
 * SPDesktop vtable initialization.
 */
static void
sp_desktop_class_init (SPDesktopClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);
    SPViewClass *view_class = (SPViewClass *) klass;

    parent_class = (SPViewClass*)g_type_class_peek_parent (klass);

    signals[ACTIVATE] = g_signal_new ("activate",
                                      G_TYPE_FROM_CLASS(klass),
                                      G_SIGNAL_RUN_FIRST,
                                      G_STRUCT_OFFSET (SPDesktopClass, activate),
                                      NULL, NULL,
                                      sp_marshal_NONE__NONE,
                                      G_TYPE_NONE, 0);
    signals[DEACTIVATE] = g_signal_new ("deactivate",
                                        G_TYPE_FROM_CLASS(klass),
                                        G_SIGNAL_RUN_FIRST,
                                        G_STRUCT_OFFSET (SPDesktopClass, deactivate),
                                        NULL, NULL,
                                        sp_marshal_NONE__NONE,
                                        G_TYPE_NONE, 0);
    signals[MODIFIED] = g_signal_new ("modified",
                                      G_TYPE_FROM_CLASS(klass),
                                      G_SIGNAL_RUN_FIRST,
                                      G_STRUCT_OFFSET (SPDesktopClass, modified),
                                      NULL, NULL,
                                      sp_marshal_NONE__UINT,
                                      G_TYPE_NONE, 1,
                                      G_TYPE_UINT);
    signals[EVENT_CONTEXT_CHANGED] = g_signal_new ("event_context_changed",
                                                   G_TYPE_FROM_CLASS(klass),
                                                   G_SIGNAL_RUN_FIRST,
                                                   G_STRUCT_OFFSET (SPDesktopClass, event_context_changed),
                                                   NULL, NULL,
                                                   sp_marshal_NONE__POINTER,
                                                   G_TYPE_NONE, 1,
                                                   G_TYPE_POINTER);

    object_class->dispose = sp_desktop_dispose;

    view_class->request_redraw = sp_desktop_request_redraw;
    view_class->set_document = sp_desktop_set_document;
    view_class->document_resized = sp_desktop_document_resized;
    view_class->set_status_message = &SPDesktop::_set_status_message;
}

/**
 * Callback for SPDesktop initialization.
 */
static void
sp_desktop_init (SPDesktop *desktop)
{
    desktop->_dlg_mgr = NULL;
    desktop->namedview = NULL;
    desktop->selection = NULL;
    desktop->acetate = NULL;
    desktop->main = NULL;
    desktop->grid = NULL;
    desktop->guides = NULL;
    desktop->drawing = NULL;
    desktop->sketch = NULL;
    desktop->controls = NULL;

    desktop->d2w.set_identity();
    desktop->w2d.set_identity();
    desktop->doc2dt = NR::Matrix(NR::scale(1, -1));

    desktop->guides_active = FALSE;

    desktop->zooms_past = NULL;
    desktop->zooms_future = NULL;

    desktop->is_fullscreen = FALSE;

    desktop->gr_item = NULL;
    desktop->gr_point_num = 0;
    desktop->gr_fill_or_stroke = true;

    new (&desktop->sel_modified_connection) sigc::connection();

    new (&desktop->sel_changed_connection) sigc::connection();


    new (&desktop->_set_colorcomponent_signal) sigc::signal<bool, ColorComponent, float, bool, bool>();

    new (&desktop->_set_style_signal) sigc::signal<bool, const SPCSSAttr *, StopOnTrue>();

    new (&desktop->_layer_changed_signal) sigc::signal<void, SPObject *>();


    desktop->_guides_message_context = new Inkscape::MessageContext(desktop->messageStack());

    desktop->current = sp_repr_css_attr_inherited (inkscape_get_repr (INKSCAPE, "desktop"), "style");

    desktop->_layer_hierarchy = NULL;
}

/**
 * Called before SPDesktop destruction.
 */
static void
sp_desktop_dispose (GObject *object)
{
    SPDesktop *dt = SP_DESKTOP (object);

    while (dt->event_context) {
        SPEventContext *ec = dt->event_context;
        dt->event_context = ec->next;
        sp_event_context_finish (ec);
        g_object_unref (G_OBJECT (ec));
    }

    dt->sel_modified_connection.disconnect();
    dt->sel_modified_connection.~connection();

    dt->sel_changed_connection.disconnect();
    dt->sel_changed_connection.~connection();

    dt->_set_colorcomponent_signal.~signal();
    dt->_set_style_signal.~accumulated();
    dt->_layer_changed_signal.~signal();

    if (dt->_layer_hierarchy) {
        delete dt->_layer_hierarchy;
    }

    if (dt->inkscape) {
        inkscape_remove_desktop (dt);
        dt->inkscape = NULL;
    }

    if (dt->selection) {
        Inkscape::GC::release(dt->selection);
        dt->selection = NULL;
    }

    if (dt->drawing) {
        sp_item_invoke_hide (SP_ITEM (sp_document_root (SP_VIEW_DOCUMENT (dt))), dt->dkey);
        dt->drawing = NULL;
    }

#ifdef WITH_INKBOARD
	if (dt->_whiteboard_session_manager) {
		delete dt->_whiteboard_session_manager;
	}
#endif

    delete dt->_guides_message_context;
    dt->_guides_message_context = NULL;

    sp_signal_disconnect_by_data (G_OBJECT (dt->namedview), dt);
    sp_signal_disconnect_by_data (G_OBJECT (dt->namedview), dt->owner);

    G_OBJECT_CLASS (parent_class)->dispose (object);

    g_list_free (dt->zooms_past);
    g_list_free (dt->zooms_future);
}

/**
 * Returns current root (=bottom) layer.
 */
SPObject *SPDesktop::currentRoot() {
    return _layer_hierarchy ? _layer_hierarchy->top() : NULL;
}

/**
 * Returns current top layer.
 */
SPObject *SPDesktop::currentLayer() {
    return _layer_hierarchy ? _layer_hierarchy->bottom() : NULL;
}

/**
 * Make \a object the top layer.
 */
void SPDesktop::setCurrentLayer(SPObject *object) {
    g_return_if_fail(SP_IS_GROUP(object));
    g_return_if_fail( currentRoot() == object || currentRoot()->isAncestorOf(object));
    // printf("Set Layer to ID: %s\n", SP_OBJECT_ID(object));
    _layer_hierarchy->setBottom(object);
}

/**
 * Return layer that contains \a object.
 */
SPObject *SPDesktop::layerForObject(SPObject *object) {
    g_return_val_if_fail(object != NULL, NULL);

    SPObject *root=currentRoot();
    object = SP_OBJECT_PARENT(object);
    while ( object && object != root && !isLayer(object) ) {
        object = SP_OBJECT_PARENT(object);
    }
    return object;
}

/**
 * True if object is a layer.
 */
bool SPDesktop::isLayer(SPObject *object) const {
    return ( SP_IS_GROUP(object)
             && ( SP_GROUP(object)->effectiveLayerMode(this->dkey)
                  == SPGroup::LAYER ) );
}

/**
 * True if desktop viewport fully contains \a item's bbox.
 */
bool SPDesktop::isWithinViewport(SPItem *item) const {
    NRRect viewport;
    NRRect bbox;
    sp_desktop_get_display_area(const_cast<SPDesktop *>(this), &viewport);
    sp_item_bbox_desktop(item, &bbox);
    return NR::Rect(viewport).contains(NR::Rect(bbox));
}

///
bool SPDesktop::itemIsHidden(SPItem const *item) const {
    return item->isHidden(this->dkey);
}

/**
 * Redraw callback; queues Gtk redraw.
 */
static void
sp_desktop_request_redraw (Inkscape::UI::View::View *view)
{
    SPDesktop *dt = SP_DESKTOP (view);

    if (dt->main) {
        gtk_widget_queue_draw (GTK_WIDGET (SP_CANVAS_ITEM (dt->main)->canvas));
    }
}

/**
 * Resized callback.
 */
static void
sp_desktop_document_resized (Inkscape::UI::View::View *view, SPDocument *doc, gdouble width, gdouble height)
{
    SPDesktop *desktop = SP_DESKTOP (view);

    desktop->doc2dt[5] = height;

    sp_canvas_item_affine_absolute (SP_CANVAS_ITEM (desktop->drawing), desktop->doc2dt);

    sp_ctrlrect_set_area (SP_CTRLRECT (desktop->page), 0.0, 0.0, width, height);
    sp_ctrlrect_set_area (SP_CTRLRECT (desktop->page_border), 0.0, 0.0, width, height);
}

/**
 * Set activate property of desktop; emit signal if changed.
 */
void
sp_desktop_set_active (SPDesktop *desktop, bool active)
{
    if (active != desktop->active) {
        desktop->active = active;
        if (active) {
            g_signal_emit (G_OBJECT (desktop), signals[ACTIVATE], 0);
        } else {
            g_signal_emit (G_OBJECT (desktop), signals[DEACTIVATE], 0);
        }
    }
}

/* fixme: */

/**
 * Calls event handler of current event context.
 * \param arena Unused
 * \todo fixme
 */
static gint
arena_handler (SPCanvasArena *arena, NRArenaItem *ai, GdkEvent *event, SPDesktop *desktop)
{
    if (ai) {
        SPItem *spi = (SPItem*)NR_ARENA_ITEM_GET_DATA (ai);
        return sp_event_context_item_handler (desktop->event_context, spi, event);
    } else {
        return sp_event_context_root_handler (desktop->event_context, event);
    }
}

static void sp_desktop_set_namedview (SPDesktop *desktop, SPNamedView* namedview)
{
    desktop->namedview = namedview;
    g_signal_connect (G_OBJECT (namedview), "modified", G_CALLBACK (sp_dt_namedview_modified), desktop);
    desktop->number = sp_namedview_viewcount (namedview);
}

/* Constructor */

/**
 * Return new desktop object.
 * \pre namedview != NULL.
 * \pre canvas != NULL.
 */
Inkscape::UI::View::View *
sp_desktop_new (SPNamedView *namedview, SPCanvas *canvas)
{
    SPDocument *document = SP_OBJECT_DOCUMENT (namedview);
    /* Kill flicker */
    sp_document_ensure_up_to_date (document);

    /* Setup widget */
    SPDesktop *desktop = (SPDesktop *) g_object_new (SP_TYPE_DESKTOP, NULL);

    /* Setup Dialog Manager */
    desktop->_dlg_mgr = new Inkscape::UI::Dialog::DialogManager();

    desktop->dkey = sp_item_display_key_new (1);

    /* Connect document */
    sp_view_set_document (SP_VIEW (desktop), document);

    sp_desktop_set_namedview (desktop, namedview);

	/* Construct SessionManager */
#ifdef WITH_INKBOARD
	desktop->_whiteboard_session_manager = new Inkscape::Whiteboard::SessionManager(desktop);
#endif

    /* Setup Canvas */
    g_object_set_data (G_OBJECT (canvas), "SPDesktop", desktop);

    SPCanvasGroup *root = sp_canvas_root (canvas);

    /* Setup adminstrative layers */
    desktop->acetate = sp_canvas_item_new (root, GNOME_TYPE_CANVAS_ACETATE, NULL);
    g_signal_connect (G_OBJECT (desktop->acetate), "event", G_CALLBACK (sp_desktop_root_handler), desktop);
    desktop->main = (SPCanvasGroup *) sp_canvas_item_new (root, SP_TYPE_CANVAS_GROUP, NULL);
    g_signal_connect (G_OBJECT (desktop->main), "event", G_CALLBACK (sp_desktop_root_handler), desktop);

    desktop->table = sp_canvas_item_new (desktop->main, SP_TYPE_CTRLRECT, NULL);
    sp_ctrlrect_set_area (SP_CTRLRECT (desktop->table), -15000.0, -15000.0, 15000.0, 15000.0);
    sp_ctrlrect_set_color (SP_CTRLRECT (desktop->table), 0x00000000, TRUE, 0x00000000);
    sp_canvas_item_move_to_z (desktop->table, 0);

    desktop->page = sp_canvas_item_new (desktop->main, SP_TYPE_CTRLRECT, NULL);
    sp_ctrlrect_set_color ((SPCtrlRect *) desktop->page, 0x00000000, FALSE, 0x00000000);
    desktop->page_border = sp_canvas_item_new (desktop->main, SP_TYPE_CTRLRECT, NULL);

    desktop->drawing = sp_canvas_item_new (desktop->main, SP_TYPE_CANVAS_ARENA, NULL);
    g_signal_connect (G_OBJECT (desktop->drawing), "arena_event", G_CALLBACK (arena_handler), desktop);

    desktop->grid = (SPCanvasGroup *) sp_canvas_item_new (desktop->main, SP_TYPE_CANVAS_GROUP, NULL);
    desktop->guides = (SPCanvasGroup *) sp_canvas_item_new (desktop->main, SP_TYPE_CANVAS_GROUP, NULL);
    desktop->sketch = (SPCanvasGroup *) sp_canvas_item_new (desktop->main, SP_TYPE_CANVAS_GROUP, NULL);
    desktop->controls = (SPCanvasGroup *) sp_canvas_item_new (desktop->main, SP_TYPE_CANVAS_GROUP, NULL);

    desktop->selection = new Inkscape::Selection (desktop);

    /* Push select tool to the bottom of stack */
    /** \todo
     * FIXME: this is the only call to this.  Everything else seems to just
     * call "set" instead of "push".  Can we assume that there is only one
     * context ever?
     */
    sp_desktop_push_event_context (desktop, SP_TYPE_SELECT_CONTEXT, "tools.select", SP_EVENT_CONTEXT_STATIC);

    // display rect and zoom are now handled in sp_desktop_widget_realize()
    sp_ctrlrect_set_area (SP_CTRLRECT (desktop->page), 0.0, 0.0, sp_document_width (document), sp_document_height (document));
    sp_ctrlrect_set_area (SP_CTRLRECT (desktop->page_border), 0.0, 0.0, sp_document_width (document), sp_document_height (document));

    /* the following sets the page shadow on the canvas
       It was originally set to 5, which is really cheesy!
       It now is an attribute in the document's namedview. If a value of
       0 is used, then the constructor for a shadow is not initialized.
    */

    if ( desktop->namedview->pageshadow != 0 && 
         desktop->namedview->showpageshadow ) {
        sp_ctrlrect_set_shadow (SP_CTRLRECT (desktop->page_border), 
                                desktop->namedview->pageshadow, 0x3f3f3fff);
    }
    

    /* Connect event for page resize */
    desktop->doc2dt[5] = sp_document_height (document);
    sp_canvas_item_affine_absolute (SP_CANVAS_ITEM (desktop->drawing), desktop->doc2dt);

    desktop->sel_modified_connection.disconnect();
    desktop->sel_modified_connection = desktop->selection->connectModified(
        sigc::bind(
            sigc::ptr_fun(&sp_desktop_selection_modified),
            desktop
        )
    );

    desktop->sel_changed_connection.disconnect();
    desktop->sel_changed_connection = desktop->selection->connectChanged(
        sigc::bind(
            sigc::ptr_fun(&SPDesktop::_selection_changed),
            desktop
        )
    );

    NRArenaItem *ai = sp_item_invoke_show (SP_ITEM (sp_document_root (SP_VIEW_DOCUMENT (desktop))),
                                           SP_CANVAS_ARENA (desktop->drawing)->arena, desktop->dkey, SP_ITEM_SHOW_DISPLAY);
    if (ai) {
        nr_arena_item_add_child (SP_CANVAS_ARENA (desktop->drawing)->root, ai, NULL);
        nr_arena_item_unref (ai);
    }

    sp_namedview_show (desktop->namedview, desktop);
    /* Ugly hack */
    sp_desktop_activate_guides (desktop, TRUE);
    /* Ugly hack */
    sp_dt_namedview_modified (desktop->namedview, SP_OBJECT_MODIFIED_FLAG, desktop);

    /* Set up notification of rebuilding the document, this allows
       for saving object related settings in the document. */
    desktop->_reconstruction_start_connection =
        document->connectReconstructionStart(sigc::bind(sigc::ptr_fun(&SPDesktop::_reconstruction_start), desktop));
    desktop->_reconstruction_finish_connection =
        document->connectReconstructionFinish(sigc::bind(sigc::ptr_fun(&SPDesktop::_reconstruction_finish), desktop));
    desktop->_reconstruction_old_layer_id = NULL;

    // ?
    // sp_active_desktop_set (desktop);
    inkscape_add_desktop (desktop);
    desktop->inkscape = INKSCAPE;

    return SP_VIEW (desktop);
}

/**
 * Namedview_modified callback.
 */
static void
sp_dt_namedview_modified (SPNamedView *nv, guint flags, SPDesktop *desktop)
{
    if (flags & SP_OBJECT_MODIFIED_FLAG) {

        /* Recalculate snap distances */
        sp_dt_update_snap_distances (desktop);

        /* Show/hide page background */
        if (nv->pagecolor & 0xff) {
            sp_canvas_item_show (desktop->table);
            sp_ctrlrect_set_color ((SPCtrlRect *) desktop->table, 0x00000000, 
                TRUE, nv->pagecolor);
            sp_canvas_item_move_to_z (desktop->table, 0);
        } else {
            sp_canvas_item_hide (desktop->table);
        }

        /* Show/hide page border */
        if (nv->showborder) {
            // show
            sp_canvas_item_show (desktop->page_border);
            // set color and shadow
            sp_ctrlrect_set_color ((SPCtrlRect *) desktop->page_border, 
			    nv->bordercolor, FALSE, 0x00000000);
            if (nv->pageshadow)
                sp_ctrlrect_set_shadow ((SPCtrlRect *)desktop->page_border, 
				nv->pageshadow, nv->bordercolor);
            // place in the z-order stack
            if (nv->borderlayer == SP_BORDER_LAYER_BOTTOM) {
                 sp_canvas_item_move_to_z (desktop->page_border, 2);
            } else {
                int order = sp_canvas_item_order (desktop->page_border);
                int morder = sp_canvas_item_order (desktop->drawing);
                if (morder > order) sp_canvas_item_raise (desktop->page_border,
				    morder - order);
            }
        } else {
                sp_canvas_item_hide (desktop->page_border);
                if (nv->pageshadow)
                    sp_ctrlrect_set_shadow ((SPCtrlRect *)desktop->page, 0, 
				    0x00000000);
        }
	
        /* Show/hide page shadow */
        if (nv->showpageshadow && nv->pageshadow) {
            // show
            sp_ctrlrect_set_shadow ((SPCtrlRect *)desktop->page_border, 
                            nv->pageshadow, nv->bordercolor);
        } else {
            // hide
            sp_ctrlrect_set_shadow ((SPCtrlRect *)desktop->page_border, 0, 
                            0x00000000);
        }

        
        
        
    }
}

static void
sp_dt_update_snap_distances (SPDesktop *desktop)
{
    SPUnit const &px = sp_unit_get_by_id(SP_UNIT_PX);

    SPNamedView &nv = *desktop->namedview;

    nv.grid_snapper.setDistance(sp_convert_distance_full(nv.gridtolerance,
                                                         *nv.gridtoleranceunit,
                                                         px));
    nv.guide_snapper.setDistance(sp_convert_distance_full(nv.guidetolerance,
                                                          *nv.guidetoleranceunit,
                                                          px));
}

/**
 * Set activate status of current desktop's named view.
 */
void
sp_desktop_activate_guides(SPDesktop *desktop, bool activate)
{
    desktop->guides_active = activate;
    sp_namedview_activate_guides (desktop->namedview, desktop, activate);
}

/**
 * Associate document with desktop (view?).
 */
static void
sp_desktop_set_document (Inkscape::UI::View::View *view, SPDocument *doc)
{
    SPDesktop *desktop = SP_DESKTOP (view);

    if (view->doc) {
        sp_namedview_hide (desktop->namedview, desktop);
        sp_item_invoke_hide (SP_ITEM (sp_document_root (SP_VIEW_DOCUMENT (desktop))), desktop->dkey);
    }

    if (desktop->_layer_hierarchy) {
        desktop->_layer_hierarchy->clear();
        delete desktop->_layer_hierarchy;
    }
    desktop->_layer_hierarchy = new Inkscape::ObjectHierarchy(NULL);
    desktop->_layer_hierarchy->connectAdded(sigc::bind(sigc::ptr_fun(&SPDesktop::_layer_activated), desktop));
    desktop->_layer_hierarchy->connectRemoved(sigc::bind(sigc::ptr_fun(&SPDesktop::_layer_deactivated), desktop));
    desktop->_layer_hierarchy->connectChanged(sigc::bind(sigc::ptr_fun(&SPDesktop::_layer_hierarchy_changed), desktop));
    desktop->_layer_hierarchy->setTop(SP_DOCUMENT_ROOT(doc));

    /// \todo fixme:
    if (desktop->drawing) {
        NRArenaItem *ai;

        sp_desktop_set_namedview (desktop, sp_document_namedview (doc, NULL));

        ai = sp_item_invoke_show (SP_ITEM (sp_document_root (doc)), SP_CANVAS_ARENA (desktop->drawing)->arena,
                                  desktop->dkey, SP_ITEM_SHOW_DISPLAY);
        if (ai) {
            nr_arena_item_add_child (SP_CANVAS_ARENA (desktop->drawing)->root, ai, NULL);
            nr_arena_item_unref (ai);
        }
        sp_namedview_show (desktop->namedview, desktop);
        /* Ugly hack */
        sp_desktop_activate_guides (desktop, TRUE);
        /* Ugly hack */
        sp_dt_namedview_modified (desktop->namedview, SP_OBJECT_MODIFIED_FLAG, desktop);
    }
}

/**
 * Make desktop switch documents.
 */
void
sp_desktop_change_document (SPDesktop *desktop, SPDocument *document)
{
    g_return_if_fail (desktop != NULL);
    g_return_if_fail (SP_IS_DESKTOP (desktop));
    g_return_if_fail (document != NULL);

    /* unselect everything before switching documents */
    SP_DT_SELECTION (desktop)->clear();

    sp_view_set_document (SP_VIEW (desktop), document);
}

/* Private methods */

static void
sp_desktop_selection_modified (Inkscape::Selection *selection, guint flags, SPDesktop *desktop)
{
    SPDesktopWidget *dtw = (SPDesktopWidget*)g_object_get_data (G_OBJECT (desktop), "widget");
    if (!dtw) return;

    sp_desktop_widget_update_scrollbars (dtw, SP_VIEW_DOCUMENT (desktop), SP_DESKTOP_ZOOM (desktop));
}

/* Public methods */

/* Context switching */

/**
 * Make desktop switch event contexts.
 */
void
sp_desktop_set_event_context (SPDesktop *dt, GtkType type, const gchar *config)
{
    SPEventContext *ec;
    while (dt->event_context) {
        ec = dt->event_context;
        sp_event_context_deactivate (ec);
        dt->event_context = ec->next;
        sp_event_context_finish (ec);
        g_object_unref (G_OBJECT (ec));
    }

    Inkscape::XML::Node *repr = (config) ? inkscape_get_repr (INKSCAPE, config) : NULL;
    ec = sp_event_context_new (type, dt, repr, SP_EVENT_CONTEXT_STATIC);
    ec->next = dt->event_context;
    dt->event_context = ec;
    sp_event_context_activate (ec);
    g_signal_emit (G_OBJECT (dt), signals[EVENT_CONTEXT_CHANGED], 0, ec);
}

/**
 * Push event context onto desktop's context stack.
 * \see sp_desktop_new()
 */
void
sp_desktop_push_event_context (SPDesktop *dt, GtkType type, const gchar *config, unsigned int key)
{
    SPEventContext *ref, *ec;
    Inkscape::XML::Node *repr;

    if (dt->event_context && dt->event_context->key == key) return;
    ref = dt->event_context;
    while (ref && ref->next && ref->next->key != key) ref = ref->next;
    if (ref && ref->next) {
        ec = ref->next;
        ref->next = ec->next;
        sp_event_context_finish (ec);
        g_object_unref (G_OBJECT (ec));
    }

    if (dt->event_context) sp_event_context_deactivate (dt->event_context);
    repr = (config) ? inkscape_get_repr (INKSCAPE, config) : NULL;
    ec = sp_event_context_new (type, dt, repr, key);
    ec->next = dt->event_context;
    dt->event_context = ec;
    sp_event_context_activate (ec);
    g_signal_emit (G_OBJECT (dt), signals[EVENT_CONTEXT_CHANGED], 0, ec);
}

/**
 * Pop event context from desktop's context stack.
 */
void
sp_desktop_pop_event_context (SPDesktop *dt, unsigned int key)
{
    SPEventContext *ec = NULL;

    if (dt->event_context && dt->event_context->key == key) {
        g_return_if_fail (dt->event_context);
        g_return_if_fail (dt->event_context->next);
        ec = dt->event_context;
        sp_event_context_deactivate (ec);
        dt->event_context = ec->next;
        sp_event_context_activate (dt->event_context);
        g_signal_emit (G_OBJECT (dt), signals[EVENT_CONTEXT_CHANGED], 0, ec);
    }

    SPEventContext *ref = dt->event_context;
    while (ref && ref->next && ref->next->key != key)
        ref = ref->next;

    if (ref && ref->next) {
        ec = ref->next;
        ref->next = ec->next;
    }

    if (ec) {
        sp_event_context_finish (ec);
        g_object_unref (G_OBJECT (ec));
    }
}

/* Private helpers */

/**
 * Show current coordinates on desktop.
 * \param underline Unused
 * 
 * \todo
 * below comment is by lauris; I don't quite understand his underlines idea. 
 * Let's wait and see if this ever gets implemented in sodipodi. I filled in 
 * a simple implementation for sp_desktop_set_coordinate_status which was
 * empty. --bb
 * fixme: The idea to have underlines is good, but have to fit it into 
 * desktop/widget framework (Lauris) 
 * set the coordinate statusbar underline single coordinates with undeline-mask
 * x and y are document coordinates \verbatim
 * underline :
 *   0 - don't underline;
 *   1 - underlines x;  (i.e. 1 << NR::X)
 *   2 - underlines y;  (i.e. 1 << NR::Y)
 *   3 - underline both                       \endverbatim
 * Currently this is unimplemented and most callers don't use it.
 * It doesn't work well with non-axis-aligned guideline moving.
 * Thus we may just get rid of it.
 */
void
sp_desktop_set_coordinate_status (SPDesktop *desktop, NR::Point p, guint underline)
{
    gchar cstr[64];

    g_snprintf (cstr, 64, "%6.2f, %6.2f", desktop->owner->dt2r * p[NR::X], desktop->owner->dt2r * p[NR::Y]);

    gtk_label_set_text (GTK_LABEL (desktop->owner->coord_status), cstr);
}

/**
 * Returns desktop's default unit.
 */
const SPUnit *
sp_desktop_get_default_unit (SPDesktop *dt)
{
    return dt->namedview->doc_units;
}

/**
 * Returns desktop's default metric.
 */
SPMetric
sp_desktop_get_default_metric (SPDesktop *dt)
{
    if (sp_desktop_get_default_unit (dt))
        return sp_unit_get_metric (sp_desktop_get_default_unit (dt));
    else
        return SP_PT;
}


SPItem *
sp_desktop_item_from_list_at_point_bottom (SPDesktop const *desktop, const GSList *list, NR::Point const p)
{
    SPDocument *document = SP_VIEW (desktop)->doc;
    g_return_val_if_fail (document != NULL, NULL);
    return sp_document_item_from_list_at_point_bottom (desktop->dkey, SP_GROUP (document->root), list, p);
}

SPItem *
sp_desktop_item_at_point (SPDesktop const *desktop, NR::Point const p, bool into_groups, SPItem *upto)
{
    SPDocument *document = SP_VIEW (desktop)->doc;
    g_return_val_if_fail (document != NULL, NULL);
    return sp_document_item_at_point (document, desktop->dkey, p, into_groups, upto);
}

SPItem *
sp_desktop_group_at_point (SPDesktop const *desktop, NR::Point const p)
{
    SPDocument *document = SP_VIEW (desktop)->doc;
    g_return_val_if_fail (document != NULL, NULL);
    return sp_document_group_at_point (document, desktop->dkey, p);
}

/**
\brief  Returns the mouse point in document coordinates; if mouse is outside the canvas, returns the center of canvas viewpoint
*/
NR::Point
sp_desktop_point (SPDesktop const *desktop)
{
    gint x, y;
    gdk_window_get_pointer (GTK_WIDGET (desktop->owner->canvas)->window, &x, &y, NULL);
    NR::Point pw = sp_canvas_window_to_world (desktop->owner->canvas, NR::Point(x, y));
    NR::Point p = sp_desktop_w2d_xy_point (desktop, pw);

    NRRect r;
    sp_canvas_get_viewbox (desktop->owner->canvas, &r);

    NR::Point r0 = sp_desktop_w2d_xy_point (desktop, NR::Point(r.x0, r.y0));
    NR::Point r1 = sp_desktop_w2d_xy_point (desktop, NR::Point(r.x1, r.y1));

    if (p[NR::X] >= r0[NR::X] &&
        p[NR::X] <= r1[NR::X] &&
        p[NR::Y] >= r1[NR::Y] &&
        p[NR::Y] <= r0[NR::Y])
    {
        return p;
    } else {
        return (r0 + r1) / 2;
    }
}


/**
 * the statusbars
 *
 * we have
 * - coordinate status   set with sp_desktop_coordinate_status which is currently not unset
 * - selection status    which is used in two ways:
 *    * sp_desktop_default_status sets the default status text which is visible
 *      if no other text is displayed
 *    * sp_desktop_set_status sets the status text and can be cleared
 with sp_desktop_clear_status making the default visible
*/
typedef struct {
    GtkStatusbar *sb;
    guint message_id;
} statusbar_data;

void SPDesktop::_set_status_message(Inkscape::UI::View::View *view, Inkscape::MessageType type, const gchar *message)
{
    SPDesktop *desktop=SP_DESKTOP(view);
    if (desktop->owner) {
        desktop->owner->setMessage(type, message);
    }
}

void SPDesktop::_layer_activated(SPObject *layer, SPDesktop *desktop) {
    g_return_if_fail(SP_IS_GROUP(layer));
    SP_GROUP(layer)->setLayerDisplayMode(desktop->dkey, SPGroup::LAYER);
}

void SPDesktop::_layer_deactivated(SPObject *layer, SPDesktop *desktop) {
    g_return_if_fail(SP_IS_GROUP(layer));
    SP_GROUP(layer)->setLayerDisplayMode(desktop->dkey, SPGroup::GROUP);
}

void SPDesktop::_layer_hierarchy_changed(SPObject *top, SPObject *bottom,
                                         SPDesktop *desktop)
{
    desktop->_layer_changed_signal.emit(bottom);
}

void SPDesktop::_selection_changed(Inkscape::Selection *selection, SPDesktop *desktop)
{
    /** \todo 
     * only change the layer for single selections, or what?
     * This seems reasonable -- for multiple selections there can be many
     * different layers involved.
     */
    SPItem *item=selection->singleItem();
    if (item) {
        SPObject *layer=desktop->layerForObject(item);
        if ( layer && layer != desktop->currentLayer() ) {
            desktop->setCurrentLayer(layer);
        }
    }
}

void
SPDesktop::_reconstruction_start (SPDesktop * desktop)
{
    // printf("Desktop, starting reconstruction\n");
    desktop->_reconstruction_old_layer_id = g_strdup(SP_OBJECT_ID(desktop->currentLayer()));
    desktop->_layer_hierarchy->setBottom(desktop->currentRoot());

    /*
    GSList const * selection_objs = desktop->selection->list();
    for (; selection_objs != NULL; selection_objs = selection_objs->next) {

    }
    */
    desktop->selection->clear();

    // printf("Desktop, starting reconstruction end\n");
}

void
SPDesktop::_reconstruction_finish (SPDesktop * desktop)
{
    // printf("Desktop, finishing reconstruction\n");
    if (desktop->_reconstruction_old_layer_id == NULL)
        return;

    SPObject * newLayer = SP_OBJECT_DOCUMENT(desktop->namedview)->getObjectById(desktop->_reconstruction_old_layer_id);
    if (newLayer != NULL)
        desktop->setCurrentLayer(newLayer);

    g_free(desktop->_reconstruction_old_layer_id);
    desktop->_reconstruction_old_layer_id = NULL;
    // printf("Desktop, finishing reconstruction end\n");
    return;
}

void SPDesktop::emitToolSubselectionChanged(gpointer data) {
	_tool_subselection_changed.emit(data);
	inkscape_subselection_changed (this);
}

void
sp_push_current_zoom (SPDesktop *dt, GList **history)
{
    NRRect area;
    sp_desktop_get_display_area (dt, &area);

    NRRect *old_zoom = g_new(NRRect, 1);
    old_zoom->x0 = area.x0;
    old_zoom->x1 = area.x1;
    old_zoom->y0 = area.y0;
    old_zoom->y1 = area.y1;
    if ( *history == NULL
         || !( ( ((NRRect *) ((*history)->data))->x0 == old_zoom->x0 ) &&
               ( ((NRRect *) ((*history)->data))->x1 == old_zoom->x1 ) &&
               ( ((NRRect *) ((*history)->data))->y0 == old_zoom->y0 ) &&
               ( ((NRRect *) ((*history)->data))->y1 == old_zoom->y1 ) ) )
    {
        *history = g_list_prepend (*history, old_zoom);
    }
}

void
sp_desktop_set_display_area (SPDesktop *dt, double x0, double y0, double x1, double y1, double border, bool log)
{
    SPDesktopWidget *dtw = (SPDesktopWidget*)g_object_get_data (G_OBJECT (dt), "widget");
    if (!dtw) return;

    // save the zoom
    if (log) {
        sp_push_current_zoom (dt, &(dt->zooms_past));
        // if we do a logged zoom, our zoom-forward list is invalidated, so delete it
        g_list_free (dt->zooms_future);
        dt->zooms_future = NULL;
    }

    double cx = 0.5 * (x0 + x1);
    double cy = 0.5 * (y0 + y1);

    NRRect viewbox;
    sp_canvas_get_viewbox (dtw->canvas, &viewbox);

    viewbox.x0 += border;
    viewbox.y0 += border;
    viewbox.x1 -= border;
    viewbox.y1 -= border;

    double scale = SP_DESKTOP_ZOOM (dt);
    double newscale;
    if (((x1 - x0) * (viewbox.y1 - viewbox.y0)) > ((y1 - y0) * (viewbox.x1 - viewbox.x0))) {
        newscale = (viewbox.x1 - viewbox.x0) / (x1 - x0);
    } else {
        newscale = (viewbox.y1 - viewbox.y0) / (y1 - y0);
    }

    newscale = CLAMP (newscale, SP_DESKTOP_ZOOM_MIN, SP_DESKTOP_ZOOM_MAX);

    int clear = FALSE;
    if (!NR_DF_TEST_CLOSE (newscale, scale, 1e-4 * scale)) {
        /* Set zoom factors */
        dt->d2w = NR::Matrix(NR::scale(newscale, -newscale));
        dt->w2d = NR::Matrix(NR::scale(1/newscale, 1/-newscale));
        sp_canvas_item_affine_absolute (SP_CANVAS_ITEM (dt->main), dt->d2w);
        clear = TRUE;
    }

    /* Calculate top left corner */
    x0 = cx - 0.5 * (viewbox.x1 - viewbox.x0) / newscale;
    y1 = cy + 0.5 * (viewbox.y1 - viewbox.y0) / newscale;

    /* Scroll */
    sp_canvas_scroll_to (dtw->canvas, x0 * newscale - border, y1 * -newscale - border, clear);

    sp_desktop_widget_update_rulers (dtw);
    sp_desktop_widget_update_scrollbars (dtw, SP_VIEW_DOCUMENT (dt), SP_DESKTOP_ZOOM (dt));
    sp_desktop_widget_update_zoom (dtw);
}

void
sp_desktop_prev_zoom (SPDesktop *dt)
{
    if (dt->zooms_past == NULL) {
        dt->messageStack()->flash(Inkscape::WARNING_MESSAGE, _("No previous zoom."));
        return;
    }

    // push current zoom into forward zooms list
    sp_push_current_zoom (dt, &(dt->zooms_future));

    // restore previous zoom
    sp_desktop_set_display_area (dt,
                                 ((NRRect *) dt->zooms_past->data)->x0,
                                 ((NRRect *) dt->zooms_past->data)->y0,
                                 ((NRRect *) dt->zooms_past->data)->x1,
                                 ((NRRect *) dt->zooms_past->data)->y1,
                                 0, false);

    // remove the just-added zoom from the past zooms list
    dt->zooms_past = g_list_remove (dt->zooms_past, ((NRRect *) dt->zooms_past->data));
}

void
sp_desktop_next_zoom (SPDesktop *dt)
{
    if (dt->zooms_future == NULL) {
        dt->messageStack()->flash(Inkscape::WARNING_MESSAGE, _("No next zoom."));
        return;
    }

    // push current zoom into past zooms list
    sp_push_current_zoom (dt, &(dt->zooms_past));

    // restore next zoom
    sp_desktop_set_display_area (dt,
                                 ((NRRect *) dt->zooms_future->data)->x0,
                                 ((NRRect *) dt->zooms_future->data)->y0,
                                 ((NRRect *) dt->zooms_future->data)->x1,
                                 ((NRRect *) dt->zooms_future->data)->y1,
                                 0, false);

    // remove the just-used zoom from the zooms_future list
    dt->zooms_future = g_list_remove (dt->zooms_future, ((NRRect *) dt->zooms_future->data));
}

NRRect *
sp_desktop_get_display_area (SPDesktop *dt, NRRect *area)
{
    NRRect viewbox;

    SPDesktopWidget *dtw = (SPDesktopWidget*)g_object_get_data (G_OBJECT (dt), "widget");
    if (!dtw) return NULL;

    sp_canvas_get_viewbox (dtw->canvas, &viewbox);

    double scale = dt->d2w[0];

    area->x0 = viewbox.x0 / scale;
    area->y0 = viewbox.y1 / -scale;
    area->x1 = viewbox.x1 / scale;
    area->y1 = viewbox.y0 / -scale;

    return area;
}

void
sp_desktop_zoom_absolute_keep_point (SPDesktop *dt, double cx, double cy, double px, double py, double zoom)
{
    SPDesktopWidget *dtw = (SPDesktopWidget*)g_object_get_data (G_OBJECT (dt), "widget");
    if (!dtw) return;

    zoom = CLAMP (zoom, SP_DESKTOP_ZOOM_MIN, SP_DESKTOP_ZOOM_MAX);

    // maximum or minimum zoom reached, but there's no exact equality because of rounding errors;
    // this check prevents "sliding" when trying to zoom in at maximum zoom;
    // someone please fix calculations properly and remove this hack
    if (fabs(SP_DESKTOP_ZOOM (dt) - zoom) < 0.025 && (fabs(SP_DESKTOP_ZOOM_MAX - zoom) < 0.01 || fabs(SP_DESKTOP_ZOOM_MIN - zoom) < 0.01))
        return;

    NRRect viewbox;
    sp_canvas_get_viewbox (dtw->canvas, &viewbox);

    const double width2 = (viewbox.x1 - viewbox.x0) / zoom;
    const double height2 = (viewbox.y1 - viewbox.y0) / zoom;

    sp_desktop_set_display_area (dt, cx - px * width2, cy - py * height2, cx + (1 - px) * width2, cy + (1 - py) * height2, 0.0);
}

void
sp_desktop_zoom_absolute (SPDesktop *dt, double cx, double cy, double zoom)
{
    sp_desktop_zoom_absolute_keep_point (dt, cx, cy, 0.5, 0.5, zoom);
}

void
sp_desktop_zoom_relative_keep_point (SPDesktop *dt, double cx, double cy, double zoom)
{
    NRRect area;
    sp_desktop_get_display_area (dt, &area);

    if (cx < area.x0)
        cx = area.x0;
    if (cx > area.x1)
        cx = area.x1;
    if (cy < area.y0)
        cy = area.y0;
    if (cy > area.y1)
        cy = area.y1;

    gdouble scale = SP_DESKTOP_ZOOM (dt) * zoom;
    double px = (cx - area.x0)/(area.x1 - area.x0);
    double py = (cy - area.y0)/(area.y1 - area.y0);

    sp_desktop_zoom_absolute_keep_point (dt, cx, cy, px, py, scale);
}

void
sp_desktop_zoom_relative (SPDesktop *dt, double cx, double cy, double zoom)
{
    gdouble scale = SP_DESKTOP_ZOOM (dt) * zoom;
    sp_desktop_zoom_absolute (dt, cx, cy, scale);
}

void
sp_desktop_zoom_page (SPDesktop *dt)
{
    NRRect d;

    d.x0 = d.y0 = 0.0;
    d.x1 = sp_document_width (SP_DT_DOCUMENT (dt));
    d.y1 = sp_document_height (SP_DT_DOCUMENT (dt));

    if ((fabs (d.x1 - d.x0) < 1.0) || (fabs (d.y1 - d.y0) < 1.0)) return;

    sp_desktop_set_display_area (dt, d.x0, d.y0, d.x1, d.y1, 10);
}

void
sp_desktop_zoom_page_width (SPDesktop *dt)
{
    NRRect d;

    sp_desktop_get_display_area (dt, &d);

    d.x0 = 0.0;
    d.x1 = sp_document_width (SP_DT_DOCUMENT (dt));

    if ((fabs (d.x1 - d.x0) < 1.0)) return;

    d.y1 = d.y0 = (d.y1 + d.y0) / 2;

    sp_desktop_set_display_area (dt, d.x0, d.y0, d.x1, d.y1, 10);
}

void
sp_desktop_zoom_selection (SPDesktop *dt)
{
    NRRect d;
    SP_DT_SELECTION(dt)->bounds(&d);

    if ((fabs (d.x1 - d.x0) < 0.1) || (fabs (d.y1 - d.y0) < 0.1)) return;
    sp_desktop_set_display_area (dt, d.x0, d.y0, d.x1, d.y1, 10);
}

void
sp_desktop_zoom_drawing (SPDesktop *dt)
{
    SPDocument *doc = SP_VIEW_DOCUMENT (SP_VIEW (dt));
    g_return_if_fail (doc != NULL);
    SPItem *docitem = SP_ITEM (sp_document_root (doc));
    g_return_if_fail (docitem != NULL);

    NRRect d;
    sp_item_bbox_desktop (docitem, &d);

    /* Note that the second condition here indicates that
    ** there are no items in the drawing.
    */
    if ( (fabs (d.x1 - d.x0) < 1.0 || fabs (d.y1 - d.y0) < 1.0) ||
         (d.x0 > d.x1 && d.y0 > d.y1))
    {
        return;
    }

    sp_desktop_set_display_area (dt, d.x0, d.y0, d.x1, d.y1, 10);
}

void sp_desktop_scroll_world(SPDesktop *dt, double dx, double dy)
{
    SPDesktopWidget *dtw = (SPDesktopWidget*)g_object_get_data (G_OBJECT (dt), "widget");
    if (!dtw) return;

    NRRect viewbox;
    sp_canvas_get_viewbox (dtw->canvas, &viewbox);

    sp_canvas_scroll_to (dtw->canvas, viewbox.x0 - dx, viewbox.y0 - dy, FALSE);

    sp_desktop_widget_update_rulers (dtw);
    sp_desktop_widget_update_scrollbars (dtw, SP_VIEW_DOCUMENT (dt), SP_DESKTOP_ZOOM (dt));
}

bool
sp_desktop_scroll_to_point (SPDesktop *desktop, NR::Point const *p, gdouble autoscrollspeed)
{
    NRRect dbox;
    sp_desktop_get_display_area (desktop, &dbox);

    gdouble autoscrolldistance = (gdouble) prefs_get_int_attribute_limited ("options.autoscrolldistance", "value", 0, -1000, 10000);

    // autoscrolldistance is in screen pixels, but the display area is in document units
    autoscrolldistance /= SP_DESKTOP_ZOOM (desktop);

    /// \todo FIXME: njh: we need an expandBy function for rects
    dbox.x0 -= autoscrolldistance;
    dbox.x1 += autoscrolldistance;
    dbox.y0 -= autoscrolldistance;
    dbox.y1 += autoscrolldistance;

    if (!((*p)[NR::X] > dbox.x0 && (*p)[NR::X] < dbox.x1) ||
        !((*p)[NR::Y] > dbox.y0 && (*p)[NR::Y] < dbox.y1)   ) {

        NR::Point const s_w( (*p) * desktop->d2w );

        gdouble x_to;
        if ((*p)[NR::X] < dbox.x0)
            x_to = dbox.x0;
        else if ((*p)[NR::X] > dbox.x1)
            x_to = dbox.x1;
        else
            x_to = (*p)[NR::X];

        gdouble y_to;
        if ((*p)[NR::Y] < dbox.y0)
            y_to = dbox.y0;
        else if ((*p)[NR::Y] > dbox.y1)
            y_to = dbox.y1;
        else
            y_to = (*p)[NR::Y];

        NR::Point const d_dt(x_to, y_to);
        NR::Point const d_w( d_dt * desktop->d2w );
        NR::Point const moved_w( d_w - s_w );

        if (autoscrollspeed == 0)
            autoscrollspeed = prefs_get_double_attribute_limited ("options.autoscrollspeed", "value", 1, 0, 10);

        if (autoscrollspeed != 0)
            sp_desktop_scroll_world(desktop, autoscrollspeed * moved_w);

        return true;
    }
    return false;
}

#ifdef HAVE_GTK_WINDOW_FULLSCREEN
void
fullscreen(SPDesktop *dt)
{
    GtkWindow *topw = GTK_WINDOW(gtk_widget_get_toplevel(GTK_WIDGET(dt->owner->canvas)));
    if (GTK_IS_WINDOW(topw)) {
        if (dt->is_fullscreen) {
            dt->is_fullscreen = FALSE;
            gtk_window_unfullscreen(topw);
            sp_desktop_widget_layout (dt->owner);
        } else {
            dt->is_fullscreen = TRUE;
            gtk_window_fullscreen(topw);
            sp_desktop_widget_layout (dt->owner);
        }
    }
}
#endif /* HAVE_GTK_WINDOW_FULLSCREEN */

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :
