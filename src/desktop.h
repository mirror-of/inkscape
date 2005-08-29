#ifndef __SP_DESKTOP_H__
#define __SP_DESKTOP_H__

/** \file
 * SPDesktop: an editable view.
 *
 * Author:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   Frank Felfe <innerspace@iname.com>
 *   bulia byak <buliabyak@users.sf.net>
 *
 * Copyright (C) 1999-2005 authors
 * Copyright (C) 2000-2001 Ximian, Inc.
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 *
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <sigc++/sigc++.h>
//#include <libnr/nr-rect.h>
#include <libnr/nr-matrix.h>
#include <libnr/nr-matrix-fns.h>
#include "display/display-forward.h"
#include "helper/helper-forward.h"
#include "forward.h"
#include "sp-metric.h"

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

/// Iterates until true or returns false.
struct StopOnTrue {
  typedef bool result_type;

  template<typename T_iterator>
  result_type operator()(T_iterator first, T_iterator last) const{	
	for (; first != last; ++first)
		if (*first) return true;
	return false;      
  }
};

/// Editable view.
struct SPDesktop : public Inkscape::UI::View::View {

    Inkscape::MessageContext *guidesMessageContext() {
	return _guides_message_context;
    }

    SPDesktopWidget *owner;
    Inkscape::Application *inkscape;
    Inkscape::UI::Dialog::DialogManager *_dlg_mgr;

    SPNamedView *namedview;
    Inkscape::Selection *selection; ///< current selection; will never generally be NULL
    sigc::connection sel_modified_connection;

    sigc::connection sel_changed_connection;

	/* Reconstruction related variables */
	sigc::connection _reconstruction_start_connection;
	sigc::connection _reconstruction_finish_connection;
	gchar * _reconstruction_old_layer_id;

    SPEventContext *event_context;

    unsigned int dkey;

    SPCanvasItem *acetate;
    SPCanvasGroup *main;
    SPCanvasGroup *grid;
    SPCanvasGroup *guides;
    SPCanvasItem *drawing;
    SPCanvasGroup *sketch;
    SPCanvasGroup *controls;

    SPCanvasItem *table;       ///< outside-of-page background
    SPCanvasItem *page;        ///< page background
    SPCanvasItem *page_border; ///< page border

    NR::Matrix d2w, w2d, doc2dt;

    gint number;
    bool active;

    /// \todo fixme: This has to be implemented in different way */
    guint guides_active : 1;

    GList *zooms_past;
    GList *zooms_future;

    bool is_fullscreen;

    /// current style
    SPCSSAttr *current;

    /// storage for selected dragger used by GrDrag as it's created and deleted by tools
    SPItem *gr_item;
    guint gr_point_num;
    bool gr_fill_or_stroke;

    SPObject *currentRoot();
    SPObject *currentLayer();
    void setCurrentLayer(SPObject *object);
    sigc::connection connectCurrentLayerChanged(const sigc::slot<void, SPObject *> & slot) {
	return _layer_changed_signal.connect(slot);
    }
    SPObject *layerForObject(SPObject *object);
    bool isLayer(SPObject *object) const;
    bool isWithinViewport(SPItem *item) const;
    bool itemIsHidden(SPItem const *item) const;
    
    static void _set_status_message(Inkscape::UI::View::View *view, Inkscape::MessageType type, gchar const *message);
    static void _layer_activated(SPObject *layer, SPDesktop *desktop);
    static void _layer_deactivated(SPObject *layer, SPDesktop *desktop);
    static void _layer_hierarchy_changed(SPObject *top, SPObject *bottom, SPDesktop *desktop);
    static void _selection_changed(Inkscape::Selection *selection, SPDesktop *desktop);
    static void _reconstruction_start(SPDesktop * desktop);
    static void _reconstruction_finish(SPDesktop * desktop);

    sigc::signal<bool, ColorComponent, float, bool, bool> _set_colorcomponent_signal;
    
    sigc::signal<bool, const SPCSSAttr *>::accumulated<StopOnTrue> _set_style_signal;
    sigc::connection connectSetStyle(const sigc::slot<bool, const SPCSSAttr *> & slot) {
	return _set_style_signal.connect(slot);
    }

    sigc::signal<int, SPStyle *, int>::accumulated<StopOnTrue> _query_style_signal;
    sigc::connection connectQueryStyle(const sigc::slot<int, SPStyle *, int> & slot) {
	return _query_style_signal.connect(slot);
    }
    
    sigc::signal<void, SPObject *> _layer_changed_signal;
    
    sigc::signal<void, sp_verb_t> _tool_changed;
    
    /// subselection is some sort of selection which is specific to the tool, such as a handle in gradient tool, or a text selection
    sigc::signal<void, gpointer> _tool_subselection_changed;
    sigc::connection connectToolSubselectionChanged(const sigc::slot<void, gpointer> & slot) {
	return _tool_subselection_changed.connect(slot);
    }
    void emitToolSubselectionChanged(gpointer data); 
    
    Inkscape::MessageContext *_guides_message_context;
    
    Inkscape::ObjectHierarchy *_layer_hierarchy;

	// Whiteboard changes

#ifdef WITH_INKBOARD
	Inkscape::Whiteboard::SessionManager* whiteboard_session_manager() {
	return _whiteboard_session_manager;
	}

	Inkscape::Whiteboard::SessionManager* _whiteboard_session_manager;
#endif
};

/// The SPDesktop vtable.
struct SPDesktopClass {
	SPViewClass parent_class;

	void (* activate) (SPDesktop *desktop);
	void (* deactivate) (SPDesktop *desktop);
	void (* modified) (SPDesktop *desktop, guint flags);
	void (* event_context_changed) (SPDesktop *desktop, SPEventContext *ctx);
};

#define SP_DESKTOP_SCROLL_LIMIT 4000.0
#define SP_DESKTOP_ZOOM_MAX 256.0
#define SP_DESKTOP_ZOOM_MIN 0.03125
#define SP_DESKTOP_ZOOM(d) expansion((d)->d2w)
#define SP_DESKTOP_EVENT_CONTEXT(d) ((d)->event_context)

Inkscape::UI::View::View * sp_desktop_new (SPNamedView *namedview, SPCanvas *canvas);
void sp_desktop_set_active (SPDesktop *desktop, bool active);
void sp_desktop_activate_guides(SPDesktop *desktop, bool activate);
void sp_desktop_change_document(SPDesktop *desktop, SPDocument *document);

/* Context */
void sp_desktop_set_event_context (SPDesktop *desktop, GtkType type, const gchar *config);
void sp_desktop_push_event_context (SPDesktop *desktop, GtkType type, const gchar *config, unsigned int key);
void sp_desktop_pop_event_context (SPDesktop *desktop, unsigned int key);

#define SP_COORDINATES_UNDERLINE_NONE (0)
#define SP_COORDINATES_UNDERLINE_X (1 << NR::X)
#define SP_COORDINATES_UNDERLINE_Y (1 << NR::Y)

void sp_desktop_set_coordinate_status (SPDesktop *desktop, NR::Point p, guint underline);

SPItem *sp_desktop_item_from_list_at_point_bottom (SPDesktop const *desktop, const GSList *list, NR::Point const p);
SPItem *sp_desktop_item_at_point (SPDesktop const *desktop, NR::Point const p, bool into_groups, SPItem *upto = NULL);
SPItem *sp_desktop_group_at_point (SPDesktop const *desktop, NR::Point const p);
NR::Point sp_desktop_point (SPDesktop const *desktop);

NRRect *sp_desktop_get_display_area (SPDesktop *dt, NRRect *area);

void sp_desktop_set_display_area (SPDesktop *dt, double x0, double y0, double x1, double y1, double border, bool log = true);
void sp_desktop_zoom_absolute (SPDesktop *dt, double cx, double cy, double zoom);
void sp_desktop_zoom_relative (SPDesktop *dt, double cx, double cy, double zoom);
void sp_desktop_zoom_relative_keep_point (SPDesktop *dt, double cx, double cy, double zoom);

inline void sp_desktop_zoom_relative_keep_point(SPDesktop *dt, NR::Point const &c, double const zoom)
{
	using NR::X;
	using NR::Y;
	sp_desktop_zoom_relative_keep_point(dt, c[X], c[Y], zoom);
}

void sp_desktop_zoom_page (SPDesktop *dt);
void sp_desktop_zoom_page_width (SPDesktop *dt);
void sp_desktop_zoom_drawing (SPDesktop *dt);
void sp_desktop_zoom_selection (SPDesktop *dt);
void sp_desktop_scroll_world(SPDesktop *dt, double dx, double dy);
bool sp_desktop_scroll_to_point (SPDesktop *desktop, NR::Point const *s_dt, gdouble autoscrollspeed = 0);

inline void sp_desktop_scroll_world(SPDesktop *dt, NR::Point const scroll)
{
	using NR::X;
	using NR::Y;
	sp_desktop_scroll_world(dt, scroll[X], scroll[Y]);
}

void sp_desktop_prev_zoom (SPDesktop *dt);
void sp_desktop_next_zoom (SPDesktop *dt);

const SPUnit *sp_desktop_get_default_unit (SPDesktop *dt);
SPMetric sp_desktop_get_default_metric(SPDesktop *dt);


#ifdef HAVE_GTK_WINDOW_FULLSCREEN
void fullscreen(SPDesktop *dt);
#endif /* HAVE_GTK_WINDOW_FULLSCREEN */

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
