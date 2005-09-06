#ifndef __SP_DESKTOP_H__
#define __SP_DESKTOP_H__

/** \file
 * SPDesktop: an editable view.
 *
 * Author:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   Frank Felfe <innerspace@iname.com>
 *   bulia byak <buliabyak@users.sf.net>
 *   Ralf Stephan <ralf@ark.in-berlin.de>
 *
 * Copyright (C) 1999-2005 authors
 * Copyright (C) 2000-2001 Ximian, Inc.
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 *
 */

/// @see desktop-handles.h for desktop macros.

#include "config.h"

#include <sigc++/sigc++.h>
#include <gtk/gtktypeutils.h>
#include "forward.h"
#include "display/display-forward.h"
#include "helper/helper-forward.h"
#include <libnr/nr-matrix.h>
#include <libnr/nr-matrix-fns.h>
#include "gc-managed.h"
#include "gc-finalized.h"
#include "gc-anchored.h"

#include "ui/view/view.h"

class NRRect;
class SPCSSAttr;
struct SPItem;

namespace Inkscape { 
  class Selection; 
  class ObjectHierarchy;
  namespace UI { 
      namespace Dialog { 
          class DialogManager; 
      }
  }
  namespace Whiteboard {
      class SessionManager;
  }
}


enum ColorComponent {
  COMPONENT_R,
  COMPONENT_G,
  COMPONENT_B,
  COMPONENT_A,

  COMPONENT_H,
  COMPONENT_S,
  COMPONENT_V,

  COMPONENT_C,
  COMPONENT_Y,
  COMPONENT_M,
  COMPONENT_K
};

/// Editable view.
struct SPDesktop : public Inkscape::UI::View::View,
                   public Inkscape::GC::Managed<>,
                   public Inkscape::GC::Finalized,
                   public Inkscape::GC::Anchored
{

    SPDesktopWidget           *owner;
    Inkscape::Application     *inkscape;
    Inkscape::UI::Dialog::DialogManager *_dlg_mgr;
    SPNamedView               *namedview;
    Inkscape::Selection       *selection;        ///< current selection; will 
                                                 ///< never generally be NULL
    SPEventContext            *event_context;
    Inkscape::ObjectHierarchy *_layer_hierarchy;

    Inkscape::MessageContext *guidesMessageContext() const {
	return _guides_message_context;
    }

    SPCanvasItem  *acetate;
    SPCanvasGroup *main;
    SPCanvasGroup *grid;
    SPCanvasGroup *guides;
    SPCanvasItem  *drawing;
    SPCanvasGroup *sketch;
    SPCanvasGroup *controls;
    SPCanvasItem  *table;       ///< outside-of-page background
    SPCanvasItem  *page;        ///< page background
    SPCanvasItem  *page_border; ///< page border
    SPCSSAttr     *current;     ///< current style

    NR::Matrix d2w, w2d, doc2dt;
    GList *zooms_past;
    GList *zooms_future;
    gchar * _reconstruction_old_layer_id;
    unsigned int dkey;
    gint number;
    bool active;
    bool is_fullscreen;

    /// \todo fixme: This has to be implemented in different way */
    guint guides_active : 1;

    // storage for selected dragger used by GrDrag as it's 
    // created and deleted by tools
    SPItem *gr_item;
    guint  gr_point_num;
    bool   gr_fill_or_stroke;   


    sigc::signal<bool, const SPCSSAttr *>::accumulated<StopOnTrue> _set_style_signal;
    sigc::signal<int, SPStyle *, int>::accumulated<StopOnTrue> _query_style_signal;
    sigc::signal<void, sp_verb_t>      _tool_changed;
    
    sigc::connection connectDestroyed (const sigc::slot<void> & slot) 
    {
        return _destroyed_signal.connect (slot);
    }
    sigc::connection connectEventContextChanged (const sigc::slot<void,SPDesktop*,SPEventContext*> & slot) 
    {
        return _event_context_changed_signal.connect (slot);
    }
    sigc::connection connectSetStyle (const sigc::slot<bool, const SPCSSAttr *> & slot) 
    {
	return _set_style_signal.connect (slot);
    }
    sigc::connection connectQueryStyle (const sigc::slot<int, SPStyle *, int> & slot) 
    {
	return _query_style_signal.connect (slot);
    }
     // subselection is some sort of selection which is specific to the tool, such as a handle in gradient tool, or a text selection
    sigc::connection connectToolSubselectionChanged(const sigc::slot<void, gpointer> & slot) {
	return _tool_subselection_changed.connect(slot);
    }
    void emitToolSubselectionChanged(gpointer data); 
    
	// Whiteboard changes

#ifdef WITH_INKBOARD
	Inkscape::Whiteboard::SessionManager* whiteboard_session_manager() {
	return _whiteboard_session_manager;
	}

	Inkscape::Whiteboard::SessionManager* _whiteboard_session_manager;
#endif
   
    SPDesktop();
    void init (SPNamedView* nv, SPCanvas* canvas);
    ~SPDesktop();
    void set_active (bool new_active);
    SPObject *currentRoot() const;
    SPObject *currentLayer() const;
    void setCurrentLayer(SPObject *object);
    sigc::connection connectCurrentLayerChanged(const sigc::slot<void, SPObject *> & slot) {
	return _layer_changed_signal.connect(slot);
    }
    SPObject *layerForObject(SPObject *object);
    bool isLayer(SPObject *object) const;
    bool isWithinViewport(SPItem *item) const;
    bool itemIsHidden(SPItem const *item) const;
    
    void activate_guides (bool activate);
    void change_document (SPDocument *document);

    void set_event_context (GtkType type, const gchar *config);
    void push_event_context (GtkType type, const gchar *config, unsigned int key);
    void pop_event_context (unsigned int key);

    void set_coordinate_status (NR::Point p, guint underline);
    SPItem *item_from_list_at_point_bottom (const GSList *list, NR::Point const p) const;
    SPItem *item_at_point (NR::Point const p, bool into_groups, SPItem *upto = NULL) const;
    SPItem *group_at_point (NR::Point const p) const;
    NR::Point point() const;

    NRRect *get_display_area (NRRect *area) const;
    void set_display_area (double x0, double y0, double x1, double y1, double border, bool log = true);
    void zoom_absolute (double cx, double cy, double zoom);
    void zoom_relative (double cx, double cy, double zoom);
    void zoom_absolute_keep_point (double cx, double cy, double px, double py, double zoom);
    void zoom_relative_keep_point (double cx, double cy, double zoom);
    void zoom_relative_keep_point (NR::Point const &c, double const zoom)
    {
            using NR::X;
            using NR::Y;
            zoom_relative_keep_point (c[X], c[Y], zoom);
    }

    void zoom_page();
    void zoom_page_width();
    void zoom_drawing();
    void zoom_selection();
    double current_zoom() const  { return d2w.expansion(); }
    void prev_zoom();
    void next_zoom();

    bool scroll_to_point (NR::Point const *s_dt, gdouble autoscrollspeed = 0);
    void scroll_world (double dx, double dy);
    void scroll_world (NR::Point const scroll)
    {
        using NR::X;
	using NR::Y;
	scroll_world(scroll[X], scroll[Y]);
    }
    
    void fullscreen();

    virtual bool shutdown();
    virtual void mouseover() {}
    virtual void mouseout() {}

    static void _set_status_message(Inkscape::UI::View::View *view, Inkscape::MessageType type, gchar const *message);
    // virtual void set_status_message(Inkscape::MessageType type, gchar const *message) { _set_status_message (this, type, message); }
    static void _layer_activated(SPObject *layer, SPDesktop *desktop);
    static void _layer_deactivated(SPObject *layer, SPDesktop *desktop);
    static void _layer_hierarchy_changed(SPObject *top, SPObject *bottom, SPDesktop *desktop);
    static void _selection_changed(Inkscape::Selection *selection, SPDesktop *desktop);
    static void _reconstruction_start(SPDesktop * desktop);
    static void _reconstruction_finish(SPDesktop * desktop);

private:
    Inkscape::MessageContext  *_guides_message_context;
    void push_current_zoom (GList**);

    sigc::signal<void>                 _destroyed_signal;
    sigc::signal<void>                 _activate_signal;
    sigc::signal<void>                 _deactivate_signal;
    sigc::signal<void,SPDesktop*,SPEventContext*> _event_context_changed_signal;
    sigc::signal<void, SPObject *>     _layer_changed_signal;
    sigc::signal<bool, ColorComponent, float, bool, bool> _set_colorcomponent_signal;
    sigc::signal<void, gpointer>       _tool_subselection_changed;
  
    sigc::connection _activate_connection;
    sigc::connection _deactivate_connection;
    sigc::connection _sel_modified_connection;
    sigc::connection _sel_changed_connection;
    sigc::connection _reconstruction_start_connection;
    sigc::connection _reconstruction_finish_connection;
    

    virtual void onPositionSet (double, double);
    virtual void onResized (double, double);
    virtual void onRedrawRequested();
    virtual void onDocumentSet (SPDocument*);
    virtual void onStatusMessage (Inkscape::MessageType type, gchar const *message);
    virtual void onDocumentURISet (gchar const* uri);
    virtual void onDocumentResized (double, double);

};

#endif

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
