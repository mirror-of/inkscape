#ifndef __SP_DESKTOP_H__
#define __SP_DESKTOP_H__

/*
 * Editable view and widget implementation
 *
 * Author:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   Frank Felfe <innerspace@iname.com>
 *
 * Copyright (C) 1999-2002 authors
 * Copyright (C) 2000-2001 Ximian, Inc.
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 *
 */

#include <sigc++/sigc++.h>
#include <libnr/nr-rect.h>
#include <libnr/nr-matrix.h>
#include <libnr/nr-matrix-fns.h>
#include "display/display-forward.h"
#include "helper/helper-forward.h"
#include "xml/xml-forward.h"
#include "forward.h"
#include "view.h"

namespace Inkscape { class ObjectHierarchy; }

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







struct StopOnTrue {
  typedef bool result_type;

  template<typename T_iterator>
  result_type operator()(T_iterator first, T_iterator last) const{	
	for (; first != last; ++first)
		if (*first) return true;
	return false;      
  }
};




class SPSelection;

struct SPDesktop : public SPView {
	Inkscape::MessageContext *guidesMessageContext() {
		return _guides_message_context;
	}

	SPDesktopWidget *owner;
	Inkscape::Application *inkscape;

	SPNamedView *namedview;
	SPSelection *selection; ///< current selection; will never generally be NULL
	sigc::connection sel_modified_connection;

	sigc::connection sel_changed_connection;

	SPEventContext *event_context;

	unsigned int dkey;

	SPCanvasItem *acetate;
	SPCanvasGroup *main;
	SPCanvasGroup *grid;
  	SPCanvasGroup *guides;
	SPCanvasItem *drawing;
	SPCanvasGroup *sketch;
	SPCanvasGroup *controls;

	SPCanvasItem *page; // background
	SPCanvasItem *page_border; // border

	NR::Matrix d2w, w2d, doc2dt;

	gint number;
	gboolean active;

	/* fixme: This has to be implemented in different way */
	guint guides_active : 1;

	GList *zooms_past;
	GList *zooms_future;

	gboolean is_fullscreen;

	// current values
	SPCSSAttr *current;

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

	void startRenameLayer();

	static void _set_status_message(SPView *view, Inkscape::MessageType type, gchar const *message);
	static void _layer_activated(SPObject *layer, SPDesktop *desktop);
	static void _layer_deactivated(SPObject *layer, SPDesktop *desktop);
	static void _layer_hierarchy_changed(SPObject *top, SPObject *bottom, SPDesktop *desktop);
	static void _selection_changed(SPSelection *selection, SPDesktop *desktop);

	sigc::signal<bool, ColorComponent, float, bool, bool> _set_colorcomponent_signal;

	sigc::signal<bool, const SPCSSAttr *>::accumulated<StopOnTrue> _set_style_signal;

	sigc::signal<void, SPObject *> _layer_changed_signal;

	sigc::signal<void, sp_verb_t> _tool_changed;


	Inkscape::MessageContext *_guides_message_context;

	Inkscape::ObjectHierarchy *_layer_hierarchy;
};

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

void sp_desktop_set_active (SPDesktop *desktop, gboolean active);

#ifndef __SP_DESKTOP_C__
extern gboolean SPShowFullFielName;
#else
gboolean SPShowFullFielName = TRUE;
#endif

/* Show/hide rulers & scrollbars */
void sp_desktop_toggle_rulers (SPDesktop *dt);
void sp_desktop_toggle_scrollbars (SPDesktop *dt);

void sp_desktop_activate_guides(SPDesktop *desktop, gboolean activate);
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
SPItem *sp_desktop_item_at_point (SPDesktop const *desktop, NR::Point const p, gboolean into_groups, SPItem *upto = NULL);
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


#ifdef HAVE_GTK_WINDOW_FULLSCREEN
void fullscreen(SPDesktop *dt);
#endif /* HAVE_GTK_WINDOW_FULLSCREEN */

#endif
