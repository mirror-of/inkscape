#define __SP_DESKTOP_C__

/** \file
 * Editable view implementation
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   MenTaLguY <mental@rydia.net>
 *   bulia byak <buliabyak@users.sf.net>
 *   Ralf Stephan <ralf@ark.in-berlin.de>
 *
 * Copyright (C) 2004 MenTaLguY
 * Copyright (C) 1999-2002 Lauris Kaplinski
 * Copyright (C) 2000-2001 Ximian, Inc.
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

/** \class SPDesktop
 * SPDesktop is a subclass of View, implementing an editable document
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

#include "macros.h"
#include "forward.h"
#include "inkscape-private.h"
#include "desktop.h"
#include "desktop-events.h"
#include "desktop-handles.h"
#include "desktop-affine.h"
#include "widgets/desktop-widget.h"
#include "document.h"
#include "message-stack.h"
#include "selection.h"
#include "select-context.h"
#include "sp-namedview.h"
#include "color.h"
#include "sp-item.h"
#include "sp-item-group.h"
#include "prefs-utils.h"
#include "object-hierarchy.h"
#include "helper/sp-marshal.h"
#include "helper/units.h"
#include "display/canvas-arena.h"
#include "display/nr-arena.h"
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

// Callback declarations
static void _onSelectionChanged (Inkscape::Selection *selection, SPDesktop *desktop);
static gint _arena_handler (SPCanvasArena *arena, NRArenaItem *ai, GdkEvent *event, SPDesktop *desktop);
static void _layer_activated(SPObject *layer, SPDesktop *desktop);
static void _layer_deactivated(SPObject *layer, SPDesktop *desktop);
static void _layer_hierarchy_changed(SPObject *top, SPObject *bottom, SPDesktop *desktop);
static void _reconstruction_start(SPDesktop * desktop);
static void _reconstruction_finish(SPDesktop * desktop);
static void _namedview_modified (SPNamedView *nv, guint flags, SPDesktop *desktop);
static void _update_snap_distances (SPDesktop *desktop);

/**
 * Return new desktop object.
 * \pre namedview != NULL.
 * \pre canvas != NULL.
 */
SPDesktop::SPDesktop()
{
    _dlg_mgr = NULL;
    _widget = 0;
    namedview = NULL;
    selection = NULL;
    acetate = NULL;
    main = NULL;
    grid = NULL;
    guides = NULL;
    drawing = NULL;
    sketch = NULL;
    controls = NULL;
    event_context = 0;

    d2w.set_identity();
    w2d.set_identity();
    doc2dt = NR::Matrix(NR::scale(1, -1));

    guides_active = false;

    zooms_past = NULL;
    zooms_future = NULL;

    is_fullscreen = false;

    gr_item = NULL;
    gr_point_num = 0;
    gr_fill_or_stroke = true;

    _layer_hierarchy = NULL;
    _active = false;

    selection = Inkscape::GC::release (new Inkscape::Selection (this));
}

void 
SPDesktop::init (SPNamedView *nv, SPCanvas *aCanvas)

{
    _guides_message_context = new Inkscape::MessageContext(const_cast<Inkscape::MessageStack*>(messageStack()));

    current = sp_repr_css_attr_inherited (inkscape_get_repr (INKSCAPE, "desktop"), "style");

    namedview = nv;
    canvas = aCanvas;
   
    SPDocument *document = SP_OBJECT_DOCUMENT (namedview);
    /* Kill flicker */
    sp_document_ensure_up_to_date (document);

    /* Setup Dialog Manager */
    _dlg_mgr = new Inkscape::UI::Dialog::DialogManager();

    dkey = sp_item_display_key_new (1);

    /* Connect document */
    setDocument (document);

    number = sp_namedview_viewcount (namedview);


    /* Setup Canvas */
    g_object_set_data (G_OBJECT (canvas), "SPDesktop", this);

    SPCanvasGroup *root = sp_canvas_root (canvas);

    /* Setup adminstrative layers */
    acetate = sp_canvas_item_new (root, GNOME_TYPE_CANVAS_ACETATE, NULL);
    g_signal_connect (G_OBJECT (acetate), "event", G_CALLBACK (sp_desktop_root_handler), this);
    main = (SPCanvasGroup *) sp_canvas_item_new (root, SP_TYPE_CANVAS_GROUP, NULL);
    g_signal_connect (G_OBJECT (main), "event", G_CALLBACK (sp_desktop_root_handler), this);

    table = sp_canvas_item_new (main, SP_TYPE_CTRLRECT, NULL);
    sp_ctrlrect_set_area (SP_CTRLRECT (table), -80000.0, -80000.0, 80000.0, 80000.0);
    sp_ctrlrect_set_color (SP_CTRLRECT (table), 0x00000000, TRUE, 0x00000000);
    sp_canvas_item_move_to_z (table, 0);

    page = sp_canvas_item_new (main, SP_TYPE_CTRLRECT, NULL);
    sp_ctrlrect_set_color ((SPCtrlRect *) page, 0x00000000, FALSE, 0x00000000);
    page_border = sp_canvas_item_new (main, SP_TYPE_CTRLRECT, NULL);

    drawing = sp_canvas_item_new (main, SP_TYPE_CANVAS_ARENA, NULL);
    g_signal_connect (G_OBJECT (drawing), "arena_event", G_CALLBACK (_arena_handler), this);

    SP_CANVAS_ARENA (drawing)->arena->delta = prefs_get_double_attribute ("options.cursortolerance", "value", 1.0); // default is 1 px

    // Start always in normal mode
    SP_CANVAS_ARENA (drawing)->arena->rendermode = RENDERMODE_NORMAL;
    canvas->rendermode = RENDERMODE_NORMAL; // canvas needs that for choosing the best buffer size

    grid = (SPCanvasGroup *) sp_canvas_item_new (main, SP_TYPE_CANVAS_GROUP, NULL);
    guides = (SPCanvasGroup *) sp_canvas_item_new (main, SP_TYPE_CANVAS_GROUP, NULL);
    sketch = (SPCanvasGroup *) sp_canvas_item_new (main, SP_TYPE_CANVAS_GROUP, NULL);
    controls = (SPCanvasGroup *) sp_canvas_item_new (main, SP_TYPE_CANVAS_GROUP, NULL);

    /* Push select tool to the bottom of stack */
    /** \todo
     * FIXME: this is the only call to this.  Everything else seems to just
     * call "set" instead of "push".  Can we assume that there is only one
     * context ever?
     */
    push_event_context (SP_TYPE_SELECT_CONTEXT, "tools.select", SP_EVENT_CONTEXT_STATIC);

    // display rect and zoom are now handled in sp_desktop_widget_realize()
    sp_ctrlrect_set_area (SP_CTRLRECT (page), 0.0, 0.0, sp_document_width (document), sp_document_height (document));
    sp_ctrlrect_set_area (SP_CTRLRECT (page_border), 0.0, 0.0, sp_document_width (document), sp_document_height (document));

    /* the following sets the page shadow on the canvas
       It was originally set to 5, which is really cheesy!
       It now is an attribute in the document's namedview. If a value of
       0 is used, then the constructor for a shadow is not initialized.
    */

    if ( namedview->pageshadow != 0 && namedview->showpageshadow ) {
        sp_ctrlrect_set_shadow (SP_CTRLRECT (page_border), 
                                namedview->pageshadow, 0x3f3f3fff);
    }
    

    /* Connect event for page resize */
    doc2dt[5] = sp_document_height (document);
    sp_canvas_item_affine_absolute (SP_CANVAS_ITEM (drawing), doc2dt);

    g_signal_connect (G_OBJECT (namedview), "modified", G_CALLBACK (_namedview_modified), this);


    NRArenaItem *ai = sp_item_invoke_show (SP_ITEM (sp_document_root (document)),
            SP_CANVAS_ARENA (drawing)->arena, 
            dkey, 
            SP_ITEM_SHOW_DISPLAY);
    if (ai) {
        nr_arena_item_add_child (SP_CANVAS_ARENA (drawing)->root, ai, NULL);
        nr_arena_item_unref (ai);
    }

    sp_namedview_show (namedview, this);
    /* Ugly hack */
    activate_guides (true);
    /* Ugly hack */
    _namedview_modified (namedview, SP_OBJECT_MODIFIED_FLAG, this);

	/* Construct SessionManager 
	 * 
	 * SessionManager construction needs to be done after document connection 
	 */
#ifdef WITH_INKBOARD
	_whiteboard_session_manager = new Inkscape::Whiteboard::SessionManager(this);
#endif

/* Set up notification of rebuilding the document, this allows
       for saving object related settings in the document. */
    _reconstruction_start_connection =
        document->connectReconstructionStart(sigc::bind(sigc::ptr_fun(_reconstruction_start), this));
    _reconstruction_finish_connection =
        document->connectReconstructionFinish(sigc::bind(sigc::ptr_fun(_reconstruction_finish), this));
    _reconstruction_old_layer_id = NULL;

    // ?
    // sp_active_desktop_set (desktop);
    _inkscape = INKSCAPE;

    _activate_connection = _activate_signal.connect(
        sigc::bind(
            sigc::ptr_fun(_onActivate),
            this
        )
    );
     _deactivate_connection = _deactivate_signal.connect(
        sigc::bind(
            sigc::ptr_fun(_onDeactivate),
            this
        )
    );

    _sel_modified_connection = selection->connectModified(
        sigc::bind(
            sigc::ptr_fun(&_onSelectionModified),
            this
        )
    );
    _sel_changed_connection = selection->connectChanged(
        sigc::bind(
            sigc::ptr_fun(&_onSelectionChanged),
            this
        )
    );

}


void SPDesktop::destroy()
{
    _activate_connection.disconnect();
    _deactivate_connection.disconnect();
    _sel_modified_connection.disconnect();
    _sel_changed_connection.disconnect();

    while (event_context) {
        SPEventContext *ec = event_context;
        event_context = ec->next;
        sp_event_context_finish (ec);
        g_object_unref (G_OBJECT (ec));
    }

    if (_layer_hierarchy) {
        delete _layer_hierarchy;
    }

    if (_inkscape) {
        _inkscape = NULL;
    }

    if (drawing) {
        sp_item_invoke_hide (SP_ITEM (sp_document_root (doc())), dkey);
        drawing = NULL;
    }

#ifdef WITH_INKBOARD
	if (_whiteboard_session_manager) {
		delete _whiteboard_session_manager;
	}
#endif

    delete _guides_message_context;
    _guides_message_context = NULL;

    sp_signal_disconnect_by_data (G_OBJECT (namedview), this);

    g_list_free (zooms_past);
    g_list_free (zooms_future);
}

SPDesktop::~SPDesktop() {}

//--------------------------------------------------------------------
/* Public methods */

void SPDesktop::setDisplayModeNormal()
{
    SP_CANVAS_ARENA (drawing)->arena->rendermode = RENDERMODE_NORMAL;
    canvas->rendermode = RENDERMODE_NORMAL; // canvas needs that for choosing the best buffer size
    sp_canvas_item_affine_absolute (SP_CANVAS_ITEM (main), d2w); // redraw
}

void SPDesktop::setDisplayModeOutline()
{
    SP_CANVAS_ARENA (drawing)->arena->rendermode = RENDERMODE_OUTLINE;
    canvas->rendermode = RENDERMODE_OUTLINE; // canvas needs that for choosing the best buffer size
    sp_canvas_item_affine_absolute (SP_CANVAS_ITEM (main), d2w); // redraw
}

/**
 * Returns current root (=bottom) layer.
 */
SPObject *SPDesktop::currentRoot() const 
{
    return _layer_hierarchy ? _layer_hierarchy->top() : NULL;
}

/**
 * Returns current top layer.
 */
SPObject *SPDesktop::currentLayer() const
{
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
bool SPDesktop::isWithinViewport (SPItem *item) const 
{
    NRRect viewport;
    NRRect bbox;
    get_display_area (&viewport);
    sp_item_bbox_desktop(item, &bbox);
    return NR::Rect(viewport).contains(NR::Rect(bbox));
}

///
bool SPDesktop::itemIsHidden(SPItem const *item) const {
    return item->isHidden(this->dkey);
}

/**
 * Set activate property of desktop; emit signal if changed.
 */
void
SPDesktop::set_active (bool new_active)
{
    if (new_active != _active) {
        _active = new_active;
        if (new_active) {
            _activate_signal.emit();
        } else {
            _deactivate_signal.emit();
        }
    }
}

/**
 * Set activate status of current desktop's named view.
 */
void
SPDesktop::activate_guides(bool activate)
{
    guides_active = activate;
    sp_namedview_activate_guides (namedview, this, activate);
}

/**
 * Make desktop switch documents.
 */
void
SPDesktop::change_document (SPDocument *document)
{
    g_return_if_fail (document != NULL);

    /* unselect everything before switching documents */
    selection->clear();

    setDocument (document);
}

/**
 * Make desktop switch event contexts.
 */
void
SPDesktop::set_event_context (GtkType type, const gchar *config)
{
    SPEventContext *ec;
    while (event_context) {
        ec = event_context;
        sp_event_context_deactivate (ec);
        event_context = ec->next;
        sp_event_context_finish (ec);
        g_object_unref (G_OBJECT (ec));
    }

    Inkscape::XML::Node *repr = (config) ? inkscape_get_repr (_inkscape, config) : NULL;
    ec = sp_event_context_new (type, this, repr, SP_EVENT_CONTEXT_STATIC);
    ec->next = event_context;
    event_context = ec;
    sp_event_context_activate (ec);
    _event_context_changed_signal.emit (this, ec);
}

/**
 * Push event context onto desktop's context stack.
 */
void
SPDesktop::push_event_context (GtkType type, const gchar *config, unsigned int key)
{
    SPEventContext *ref, *ec;
    Inkscape::XML::Node *repr;

    if (event_context && event_context->key == key) return;
    ref = event_context;
    while (ref && ref->next && ref->next->key != key) ref = ref->next;
    if (ref && ref->next) {
        ec = ref->next;
        ref->next = ec->next;
        sp_event_context_finish (ec);
        g_object_unref (G_OBJECT (ec));
    }

    if (event_context) sp_event_context_deactivate (event_context);
    repr = (config) ? inkscape_get_repr (INKSCAPE, config) : NULL;
    ec = sp_event_context_new (type, this, repr, key);
    ec->next = event_context;
    event_context = ec;
    sp_event_context_activate (ec);
    _event_context_changed_signal.emit (this, ec);
}

void
SPDesktop::set_coordinate_status (NR::Point p) {
    _widget->setCoordinateStatus(p);
}


SPItem *
SPDesktop::item_from_list_at_point_bottom (const GSList *list, NR::Point const p) const
{
    g_return_val_if_fail (doc() != NULL, NULL);
    return sp_document_item_from_list_at_point_bottom (dkey, SP_GROUP (doc()->root), list, p);
}

SPItem *
SPDesktop::item_at_point (NR::Point const p, bool into_groups, SPItem *upto) const
{
    g_return_val_if_fail (doc() != NULL, NULL);
    return sp_document_item_at_point (doc(), dkey, p, into_groups, upto);
}

SPItem *
SPDesktop::group_at_point (NR::Point const p) const
{
    g_return_val_if_fail (doc() != NULL, NULL);
    return sp_document_group_at_point (doc(), dkey, p);
}

/**
 * \brief  Returns the mouse point in document coordinates; if mouse is 
 * outside the canvas, returns the center of canvas viewpoint
 */
NR::Point
SPDesktop::point() const
{
    NR::Point p = _widget->getPointer();
    NR::Point pw = sp_canvas_window_to_world (canvas, p);
    p = sp_desktop_w2d_xy_point (this, pw);

    NRRect r;
    sp_canvas_get_viewbox (canvas, &r);

    NR::Point r0 = sp_desktop_w2d_xy_point (this, NR::Point(r.x0, r.y0));
    NR::Point r1 = sp_desktop_w2d_xy_point (this, NR::Point(r.x1, r.y1));

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
 * Put current zoom data in history list.
 */
void
SPDesktop::push_current_zoom (GList **history)
{
    NRRect area;
    get_display_area (&area);

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

/**
 * Set viewbox.
 */
void
SPDesktop::set_display_area (double x0, double y0, double x1, double y1, double border, bool log)
{
    g_assert (_widget);

    // save the zoom
    if (log) {
        push_current_zoom (&zooms_past);
        // if we do a logged zoom, our zoom-forward list is invalidated, so delete it
        g_list_free (zooms_future);
        zooms_future = NULL;
    }

    double cx = 0.5 * (x0 + x1);
    double cy = 0.5 * (y0 + y1);

    NRRect viewbox;
    sp_canvas_get_viewbox (canvas, &viewbox);

    viewbox.x0 += border;
    viewbox.y0 += border;
    viewbox.x1 -= border;
    viewbox.y1 -= border;

    double scale = expansion(d2w);
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
        d2w = NR::Matrix(NR::scale(newscale, -newscale));
        w2d = NR::Matrix(NR::scale(1/newscale, 1/-newscale));
        sp_canvas_item_affine_absolute (SP_CANVAS_ITEM (main), d2w);
        clear = TRUE;
    }

    /* Calculate top left corner */
    x0 = cx - 0.5 * (viewbox.x1 - viewbox.x0) / newscale;
    y1 = cy + 0.5 * (viewbox.y1 - viewbox.y0) / newscale;

    /* Scroll */
    sp_canvas_scroll_to (canvas, x0 * newscale - border, y1 * -newscale - border, clear);

    _widget->updateRulers();
    _widget->updateScrollbars (expansion(d2w));
    _widget->updateZoom();
}

/**
 * Return viewbox dimensions.
 */
NRRect *
SPDesktop::get_display_area (NRRect *area) const
{
    NRRect viewbox;

    sp_canvas_get_viewbox (canvas, &viewbox);

    double scale = d2w[0];

    area->x0 = viewbox.x0 / scale;
    area->y0 = viewbox.y1 / -scale;
    area->x1 = viewbox.x1 / scale;
    area->y1 = viewbox.y0 / -scale;

    return area;
}

/**
 * Revert back to previous zoom if possible.
 */
void
SPDesktop::prev_zoom()
{
    if (zooms_past == NULL) {
        messageStack()->flash(Inkscape::WARNING_MESSAGE, _("No previous zoom."));
        return;
    }

    // push current zoom into forward zooms list
    push_current_zoom (&zooms_future);

    // restore previous zoom
    set_display_area (((NRRect *) zooms_past->data)->x0,
            ((NRRect *) zooms_past->data)->y0,
            ((NRRect *) zooms_past->data)->x1,
            ((NRRect *) zooms_past->data)->y1,
            0, false);

    // remove the just-added zoom from the past zooms list
    zooms_past = g_list_remove (zooms_past, ((NRRect *) zooms_past->data));
}

/**
 * Set zoom to next in list.
 */
void
SPDesktop::next_zoom()
{
    if (zooms_future == NULL) {
        this->messageStack()->flash(Inkscape::WARNING_MESSAGE, _("No next zoom."));
        return;
    }

    // push current zoom into past zooms list
    push_current_zoom (&zooms_past);

    // restore next zoom
    set_display_area (((NRRect *) zooms_future->data)->x0,
            ((NRRect *) zooms_future->data)->y0,
            ((NRRect *) zooms_future->data)->x1,
            ((NRRect *) zooms_future->data)->y1,
            0, false);

    // remove the just-used zoom from the zooms_future list
    zooms_future = g_list_remove (zooms_future, ((NRRect *) zooms_future->data));
}

/**
 * Zoom to point with absolute zoom factor.
 */
void
SPDesktop::zoom_absolute_keep_point (double cx, double cy, double px, double py, double zoom)
{
    zoom = CLAMP (zoom, SP_DESKTOP_ZOOM_MIN, SP_DESKTOP_ZOOM_MAX);

    // maximum or minimum zoom reached, but there's no exact equality because of rounding errors;
    // this check prevents "sliding" when trying to zoom in at maximum zoom;
    /// \todo someone please fix calculations properly and remove this hack
    if (fabs(expansion(d2w) - zoom) < 0.0001*zoom && (fabs(SP_DESKTOP_ZOOM_MAX - zoom) < 0.01 || fabs(SP_DESKTOP_ZOOM_MIN - zoom) < 0.000001))
        return;

    NRRect viewbox;
    sp_canvas_get_viewbox (canvas, &viewbox);

    const double width2 = (viewbox.x1 - viewbox.x0) / zoom;
    const double height2 = (viewbox.y1 - viewbox.y0) / zoom;

    set_display_area (cx - px * width2, 
            cy - py * height2, 
            cx + (1 - px) * width2, 
            cy + (1 - py) * height2, 
            0.0);
}

/**
 * Zoom to center with absolute zoom factor.
 */
void
SPDesktop::zoom_absolute (double cx, double cy, double zoom)
{
    zoom_absolute_keep_point (cx, cy, 0.5, 0.5, zoom);
}

/**
 * Zoom to point with relative zoom factor.
 */
void
SPDesktop::zoom_relative_keep_point (double cx, double cy, double zoom)
{
    NRRect area;
    get_display_area (&area);

    if (cx < area.x0)
        cx = area.x0;
    if (cx > area.x1)
        cx = area.x1;
    if (cy < area.y0)
        cy = area.y0;
    if (cy > area.y1)
        cy = area.y1;

    gdouble scale = expansion(d2w) * zoom;
    double px = (cx - area.x0)/(area.x1 - area.x0);
    double py = (cy - area.y0)/(area.y1 - area.y0);

    zoom_absolute_keep_point (cx, cy, px, py, scale);
}

/**
 * Zoom to center with relative zoom factor.
 */
void
SPDesktop::zoom_relative (double cx, double cy, double zoom)
{
    gdouble scale = expansion(d2w) * zoom;
    zoom_absolute (cx, cy, scale);
}

/**
 * Set display area to origin and current document dimensions.
 */
void
SPDesktop::zoom_page()
{
    NRRect d;

    d.x0 = d.y0 = 0.0;
    d.x1 = sp_document_width (doc());
    d.y1 = sp_document_height (doc());

    if ((fabs (d.x1 - d.x0) < 1.0) || (fabs (d.y1 - d.y0) < 1.0)) return;

    set_display_area (d.x0, d.y0, d.x1, d.y1, 10);
}

/**
 * Set display area to current document width.
 */
void
SPDesktop::zoom_page_width()
{
    NRRect d;

    get_display_area (&d);

    d.x0 = 0.0;
    d.x1 = sp_document_width (doc());

    if ((fabs (d.x1 - d.x0) < 1.0)) return;

    d.y1 = d.y0 = (d.y1 + d.y0) / 2;

    set_display_area (d.x0, d.y0, d.x1, d.y1, 10);
}

/**
 * Zoom to selection.
 */
void
SPDesktop::zoom_selection()
{
    NRRect d;
    selection->bounds(&d);

    if ((fabs (d.x1 - d.x0) < 0.1) || (fabs (d.y1 - d.y0) < 0.1)) return;
    set_display_area (d.x0, d.y0, d.x1, d.y1, 10);
}

/**
 * Tell widget to let zoom widget grab keyboard focus.
 */
void
SPDesktop::zoom_grab_focus()
{
    _widget->letZoomGrabFocus();
}

/**
 * Zoom to whole drawing.
 */
void
SPDesktop::zoom_drawing()
{
    g_return_if_fail (doc() != NULL);
    SPItem *docitem = SP_ITEM (sp_document_root (doc()));
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

    set_display_area (d.x0, d.y0, d.x1, d.y1, 10);
}

/**
 * Scroll canvas by specific coordinate amount.
 */
void 
SPDesktop::scroll_world (double dx, double dy)
{
   g_assert (_widget); 

    NRRect viewbox;
    sp_canvas_get_viewbox (canvas, &viewbox);

    sp_canvas_scroll_to (canvas, viewbox.x0 - dx, viewbox.y0 - dy, FALSE);

    _widget->updateRulers();
    _widget->updateScrollbars (expansion(d2w));
}

bool
SPDesktop::scroll_to_point (NR::Point const *p, gdouble autoscrollspeed)
{
    NRRect dbox;
    get_display_area (&dbox);

    gdouble autoscrolldistance = (gdouble) prefs_get_int_attribute_limited ("options.autoscrolldistance", "value", 0, -1000, 10000);

    // autoscrolldistance is in screen pixels, but the display area is in document units
    autoscrolldistance /= expansion(d2w);

    /// \todo FIXME: njh: we need an expandBy function for rects
    dbox.x0 -= autoscrolldistance;
    dbox.x1 += autoscrolldistance;
    dbox.y0 -= autoscrolldistance;
    dbox.y1 += autoscrolldistance;

    if (!((*p)[NR::X] > dbox.x0 && (*p)[NR::X] < dbox.x1) ||
        !((*p)[NR::Y] > dbox.y0 && (*p)[NR::Y] < dbox.y1)   ) {

        NR::Point const s_w( (*p) * d2w );

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
        NR::Point const d_w( d_dt * d2w );
        NR::Point const moved_w( d_w - s_w );

        if (autoscrollspeed == 0)
            autoscrollspeed = prefs_get_double_attribute_limited ("options.autoscrollspeed", "value", 1, 0, 10);

        if (autoscrollspeed != 0)
            scroll_world (autoscrollspeed * moved_w);

        return true;
    }
    return false;
}

void
SPDesktop::fullscreen()
{
    _widget->setFullscreen();
}
    
void 
SPDesktop::getWindowGeometry (gint &x, gint &y, gint &w, gint &h)
{
    _widget->getGeometry (x, y, w, h);
}

void 
SPDesktop::setWindowPosition (NR::Point p)
{
    _widget->setPosition (p);
}

void
SPDesktop::setWindowSize (gint w, gint h)
{
    _widget->setSize (w, h);
}

void
SPDesktop::setWindowTransient (void *p, int transient_policy)
{
    _widget->setTransient (p, transient_policy);
}

void
SPDesktop::presentWindow()
{
    _widget->present();
}

bool
SPDesktop::warnDialog (gchar *text)
{
    return _widget->warnDialog (text);
}

void
SPDesktop::toggleRulers()
{
    _widget->toggleRulers();
}

void
SPDesktop::toggleScrollbars()
{
    _widget->toggleScrollbars();
}

void
SPDesktop::layoutWidget()
{
    _widget->layout();
}

void
SPDesktop::destroyWidget()
{
    _widget->destroy();
}

bool
SPDesktop::shutdown()
{
    return _widget->shutdown();
}

void
SPDesktop::setToolboxFocusTo (gchar const *label)
{
    _widget->setToolboxFocusTo (label);
}

void 
SPDesktop::setToolboxAdjustmentValue (gchar const* id, double val)
{
    _widget->setToolboxAdjustmentValue (id, val);
}

bool
SPDesktop::isToolboxButtonActive (gchar const *id)
{
    return _widget->isToolboxButtonActive (id);
}

void 
SPDesktop::emitToolSubselectionChanged(gpointer data) 
{
	_tool_subselection_changed.emit(data);
	inkscape_subselection_changed (this);
}

//----------------------------------------------------------------------
// Callback implementations. The virtual ones are connected by the view.

void 
SPDesktop::onPositionSet (double x, double y)
{
    _widget->viewSetPosition (NR::Point(x,y));
}

void
SPDesktop::onResized (double x, double y)
{
   // Nothing called here 
}

/**
 * Redraw callback; queues Gtk redraw; connected by View.
 */
void
SPDesktop::onRedrawRequested ()
{
    if (main) {
        _widget->requestCanvasUpdate();
    }
}

/**
 * Associate document with desktop.
 */
void
SPDesktop::setDocument (SPDocument *doc)
{
    if (this->doc() && doc) {
        sp_namedview_hide (namedview, this);
        sp_item_invoke_hide (SP_ITEM (sp_document_root (this->doc())), dkey);
    }

    if (_layer_hierarchy) {
        _layer_hierarchy->clear();
        delete _layer_hierarchy;
    }
    _layer_hierarchy = new Inkscape::ObjectHierarchy(NULL);
    _layer_hierarchy->connectAdded(sigc::bind(sigc::ptr_fun(_layer_activated), this));
    _layer_hierarchy->connectRemoved(sigc::bind(sigc::ptr_fun(_layer_deactivated), this));
    _layer_hierarchy->connectChanged(sigc::bind(sigc::ptr_fun(_layer_hierarchy_changed), this));
    _layer_hierarchy->setTop(SP_DOCUMENT_ROOT(doc));

    /// \todo fixme: This condition exists to make sure the code
    /// inside is called only once on initialization. But there
    /// are surely more safe methods to accomplish this.
    if (drawing) {
        NRArenaItem *ai;

        namedview = sp_document_namedview (doc, NULL);
        g_signal_connect (G_OBJECT (namedview), "modified", G_CALLBACK (_namedview_modified), this);
        number = sp_namedview_viewcount (namedview);

        ai = sp_item_invoke_show (SP_ITEM (sp_document_root (doc)), 
                SP_CANVAS_ARENA (drawing)->arena,
                dkey, 
                SP_ITEM_SHOW_DISPLAY);
        if (ai) {
            nr_arena_item_add_child (SP_CANVAS_ARENA (drawing)->root, ai, NULL);
            nr_arena_item_unref (ai);
        }
        sp_namedview_show (namedview, this);
        /* Ugly hack */
        activate_guides (true);
        /* Ugly hack */
        _namedview_modified (namedview, SP_OBJECT_MODIFIED_FLAG, this);
    }

    _document_replaced_signal.emit (this, doc);

    View::setDocument (doc);
}

void
SPDesktop::onStatusMessage
(Inkscape::MessageType type, gchar const *message)
{
    if (_widget) {
        _widget->setMessage(type, message);
    }
}

void
SPDesktop::onDocumentURISet (gchar const* uri)
{
    _widget->setTitle(uri);
}

/**
 * Resized callback.
 */
void
SPDesktop::onDocumentResized (gdouble width, gdouble height)
{
    doc2dt[5] = height;
    sp_canvas_item_affine_absolute (SP_CANVAS_ITEM (drawing), doc2dt);
    sp_ctrlrect_set_area (SP_CTRLRECT (page), 0.0, 0.0, width, height);
    sp_ctrlrect_set_area (SP_CTRLRECT (page_border), 0.0, 0.0, width, height);
}


void
SPDesktop::_onActivate (SPDesktop* dt)
{
    if (!dt->_widget) return;
    dt->_widget->activateDesktop();
}

void
SPDesktop::_onDeactivate (SPDesktop* dt)
{
    if (!dt->_widget) return;
    dt->_widget->deactivateDesktop();
}

void
SPDesktop::_onSelectionModified 
(Inkscape::Selection *selection, guint flags, SPDesktop *dt)
{
    if (!dt->_widget) return;
    dt->_widget->updateScrollbars (expansion(dt->d2w));
}

static void 
_onSelectionChanged
(Inkscape::Selection *selection, SPDesktop *desktop)
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

/**
 * Calls event handler of current event context.
 * \param arena Unused
 * \todo fixme
 */
static gint
_arena_handler (SPCanvasArena *arena, NRArenaItem *ai, GdkEvent *event, SPDesktop *desktop)
{
    if (ai) {
        SPItem *spi = (SPItem*)NR_ARENA_ITEM_GET_DATA (ai);
        return sp_event_context_item_handler (desktop->event_context, spi, event);
    } else {
        return sp_event_context_root_handler (desktop->event_context, event);
    }
}

static void 
_layer_activated(SPObject *layer, SPDesktop *desktop) {
    g_return_if_fail(SP_IS_GROUP(layer));
    SP_GROUP(layer)->setLayerDisplayMode(desktop->dkey, SPGroup::LAYER);
}

/// Callback
static void 
_layer_deactivated(SPObject *layer, SPDesktop *desktop) {
    g_return_if_fail(SP_IS_GROUP(layer));
    SP_GROUP(layer)->setLayerDisplayMode(desktop->dkey, SPGroup::GROUP);
}

/// Callback
static void 
_layer_hierarchy_changed(SPObject *top, SPObject *bottom,
                                         SPDesktop *desktop)
{
    desktop->_layer_changed_signal.emit (bottom);
}

/// Called when document is starting to be rebuilt.
static void
_reconstruction_start (SPDesktop * desktop)
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

/// Called when document rebuild is finished.
static void
_reconstruction_finish (SPDesktop * desktop)
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

/**
 * Namedview_modified callback.
 */
static void
_namedview_modified (SPNamedView *nv, guint flags, SPDesktop *desktop)
{
    if (flags & SP_OBJECT_MODIFIED_FLAG) {

        /* Recalculate snap distances */
        _update_snap_distances (desktop);

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

        if (SP_RGBA32_A_U(nv->pagecolor) < 128 ||
            (SP_RGBA32_R_U(nv->pagecolor) + 
             SP_RGBA32_G_U(nv->pagecolor) + 
             SP_RGBA32_B_U(nv->pagecolor)) >= 384) { 
            // the background color is light or transparent, use black outline
            SP_CANVAS_ARENA (desktop->drawing)->arena->outlinecolor = 0xff;
        } else { // use white outline
            SP_CANVAS_ARENA (desktop->drawing)->arena->outlinecolor = 0xffffffff;
        }        
    }
}

/**
 * Callback to reset snapper's distances.
 */
static void
_update_snap_distances (SPDesktop *desktop)
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
 * Pop event context from desktop's context stack. Never used.
 */
// void
// SPDesktop::pop_event_context (unsigned int key)
// {
//    SPEventContext *ec = NULL;
//
//    if (event_context && event_context->key == key) {
//        g_return_if_fail (event_context);
//        g_return_if_fail (event_context->next);
//        ec = event_context;
//        sp_event_context_deactivate (ec);
//        event_context = ec->next;
//        sp_event_context_activate (event_context);
//        _event_context_changed_signal.emit (this, ec);
//    }
//
//    SPEventContext *ref = event_context;
//    while (ref && ref->next && ref->next->key != key)
//        ref = ref->next;
//
//    if (ref && ref->next) {
//        ec = ref->next;
//        ref->next = ec->next;
//    }
//
//    if (ec) {
//        sp_event_context_finish (ec);
//        g_object_unref (G_OBJECT (ec));
//    }
// }

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
