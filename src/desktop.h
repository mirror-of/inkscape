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

typedef struct _SPDesktopWidget SPDesktopWidget;
typedef struct _SPDesktopWidgetClass SPDesktopWidgetClass;

#define SP_TYPE_DESKTOP_WIDGET (sp_desktop_widget_get_type ())
#define SP_DESKTOP_WIDGET(o) (GTK_CHECK_CAST ((o), SP_TYPE_DESKTOP_WIDGET, SPDesktopWidget))
#define SP_DESKTOP_WIDGET_CLASS(k) (GTK_CHECK_CLASS_CAST ((k), SP_TYPE_DESKTOP_WIDGET, SPDesktopWidgetClass))
#define SP_IS_DESKTOP_WIDGET(o) (GTK_CHECK_TYPE ((o), SP_TYPE_DESKTOP_WIDGET))
#define SP_IS_DESKTOP_WIDGET_CLASS(k) (GTK_CHECK_CLASS_TYPE ((k), SP_TYPE_DESKTOP_WIDGET))

#include <libnr/nr-rect.h>
#include <libnr/nr-matrix.h>
#include "helper/helper-forward.h"
#include "helper/units.h"
#include "forward.h"
#include "view.h"

struct _SPDesktop {
	SPView view;

	SPDesktopWidget *owner;
	Inkscape::Application *inkscape;

	SPNamedView *namedview;
	SPSelection *selection;
	SPEventContext *event_context;

	unsigned int dkey;

	SPCanvasItem *acetate;
	SPCanvasGroup *main;
	SPCanvasGroup *grid;
  	SPCanvasGroup *guides;
	SPCanvasItem *drawing;
	SPCanvasGroup *sketch;
	SPCanvasGroup *controls;
	SPCanvasItem *page;
	NR::Matrix d2w, w2d, doc2dt;

        gint number;
	gboolean active;
	/* Normalized snap distances */
	gdouble gridsnap;
	gdouble guidesnap;
	/* fixme: This has to be implemented in different way */
	guint guides_active : 1;

	GList *zooms_past;
	GList *zooms_future;
	gboolean can_go_forward;

#ifdef HAVE_GTK_WINDOW_FULLSCREEN
	gboolean is_fullscreen;
#endif /* HAVE_GTK_FULLSCREEN */
};

struct _SPDesktopClass {
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

void sp_desktop_activate_guides (SPDesktop *desktop, gboolean activate);
void sp_desktop_change_document (SPDesktop *desktop, SPDocument * document);

/* Context */
void sp_desktop_set_event_context (SPDesktop *desktop, GtkType type, const gchar *config);
void sp_desktop_push_event_context (SPDesktop *desktop, GtkType type, const gchar *config, unsigned int key);
void sp_desktop_pop_event_context (SPDesktop *desktop, unsigned int key);

#define SP_COORDINATES_UNDERLINE_NONE (0)
#define SP_COORDINATES_UNDERLINE_X (1 << NR::X)
#define SP_COORDINATES_UNDERLINE_Y (1 << NR::Y)

void sp_desktop_set_coordinate_status (SPDesktop *desktop, gdouble x, gdouble y, guint underline);
SPItem *sp_desktop_item_at_point (SPDesktop const *desktop, NR::Point const p, gboolean into_groups);
SPItem *sp_desktop_group_at_point (SPDesktop const *desktop, NR::Point const p);

NRRect *sp_desktop_get_display_area (SPDesktop *dt, NRRect *area);

void sp_desktop_set_display_area (SPDesktop *dt, double x0, double y0, double x1, double y1, double border);
void sp_desktop_zoom_absolute (SPDesktop *dt, double cx, double cy, double zoom);
void sp_desktop_zoom_relative (SPDesktop *dt, double cx, double cy, double zoom);
void sp_desktop_zoom_relative_keep_point (SPDesktop *dt, double cx, double cy, double zoom);
void sp_desktop_zoom_page (SPDesktop *dt);
void sp_desktop_zoom_page_width (SPDesktop *dt);
void sp_desktop_zoom_drawing (SPDesktop *dt);
void sp_desktop_zoom_selection (SPDesktop *dt);
void sp_desktop_scroll_world (SPDesktop *dt, double dx, double dy);
void sp_desktop_prev_zoom (SPDesktop *dt);
void sp_desktop_next_zoom (SPDesktop *dt);

const SPUnit *sp_desktop_get_default_unit (SPDesktop *dt);

/* SPDesktopWidget */

struct _SPDesktopWidget {
	SPViewWidget viewwidget;

	unsigned int update : 1;

	unsigned int decorations : 1;
	unsigned int statusbar : 1;

	SPDesktop *desktop;

        GtkWidget *mbtn;

	GtkWidget *hscrollbar, *vscrollbar;

	GtkWidget *tool_toolbox, *aux_toolbox;

	/* Rulers */
	GtkWidget *hruler, *vruler;
	double dt2r;
	NR::Point ruler_origin;

	GtkWidget *sticky_zoom;
	GtkWidget *coord_status;
        GtkWidget *select_status;
        GtkWidget *zoom_status;
	gulong zoom_update;

        gint coord_status_id, select_status_id;

	SPCanvas *canvas;

	GtkAdjustment *hadj, *vadj;
};

struct _SPDesktopWidgetClass {
	SPViewWidgetClass parent_class;
};

GtkType sp_desktop_widget_get_type (void);

/* Constructor */

SPViewWidget *sp_desktop_widget_new (SPNamedView *namedview);

gint sp_desktop_widget_set_focus (GtkWidget *widget, GdkEvent *event, SPDesktopWidget  *dtw);

void sp_desktop_widget_show_decorations (SPDesktopWidget *dtw, gboolean show);

#ifdef HAVE_GTK_WINDOW_FULLSCREEN
void fullscreen(SPDesktop *dt);
#endif /* HAVE_GTK_WINDOW_FULLSCREEN */

#endif
