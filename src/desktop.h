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

#if 1
	SPDesktopWidget *owner;
#endif
	Sodipodi *sodipodi;

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
	gdouble d2w[6], w2d[6], doc2dt[6];

        gint number;
	gboolean active;
	/* Normalized snap distances */
	gdouble gridsnap;
	gdouble guidesnap;
	/* fixme: This has to be implemented in different way */
	guint guides_active : 1;
};

struct _SPDesktopClass {
	SPViewClass parent_class;

	void (* activate) (SPDesktop *desktop);
	void (* desactivate) (SPDesktop *desktop);
	void (* modified) (SPDesktop *desktop, guint flags);
};

#define SP_DESKTOP_SCROLL_LIMIT 4000.0
#define SP_DESKTOP_ZOOM_INC 1.414213562
#define SP_DESKTOP_ZOOM_MAX 32.0
#define SP_DESKTOP_ZOOM_MIN 0.03125
#define SP_DESKTOP_ZOOM(d) NR_MATRIX_DF_EXPANSION(NR_MATRIX_D_FROM_DOUBLE ((d)->d2w))
#define SP_MOUSEMOVE_STEP 40

void sp_desktop_set_active (SPDesktop *desktop, gboolean active);

#ifndef __SP_DESKTOP_C__
extern gboolean SPShowFullFielName;
#else
gboolean SPShowFullFielName = TRUE;
#endif

/* Show/hide rulers & scrollbars */

void sp_desktop_activate_guides (SPDesktop *desktop, gboolean activate);

void sp_desktop_change_document (SPDesktop *desktop, SPDocument * document);

/* fixme: */
void sp_desktop_toggle_borders (GtkWidget * widget);

/* Context */
void sp_desktop_set_event_context (SPDesktop *desktop, GtkType type, const unsigned char *config);
void sp_desktop_push_event_context (SPDesktop *desktop, GtkType type, const unsigned char *config, unsigned int key);
void sp_desktop_pop_event_context (SPDesktop *desktop, unsigned int key);

#define SP_COORDINATES_UNDERLINE_X (1 << 0)
#define SP_COORDINATES_UNDERLINE_Y (1 << 1)

void sp_desktop_set_coordinate_status (SPDesktop *desktop, gdouble x, gdouble y, guint underline);

NRRectF *sp_desktop_get_display_area (SPDesktop *dt, NRRectF *area);

void sp_desktop_set_display_area (SPDesktop *dt, float x0, float y0, float x1, float y1, float border);
void sp_desktop_zoom_absolute (SPDesktop *dt, float cx, float cy, float zoom);
void sp_desktop_zoom_relative (SPDesktop *dt, float cx, float cy, float zoom);
void sp_desktop_zoom_page (SPDesktop *dt);
void sp_desktop_zoom_drawing (SPDesktop *dt);
void sp_desktop_zoom_selection (SPDesktop *dt);
void sp_desktop_scroll_world (SPDesktop *dt, float dx, float dy);

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

	/* Rulers */
	GtkWidget *hruler, *vruler;
	double dt2r;
	double rx0, ry0;

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

#endif
