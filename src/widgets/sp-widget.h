#ifndef __SP_WIDGET_H__
#define __SP_WIDGET_H__

/*
 * Abstract base class for dynamic control widgets
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * Copyright (C) 1999-2002 Lauris Kaplinski
 * Copyright (C) 2000-2001 Ximian, Inc.
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <glib.h>

G_BEGIN_DECLS

#define SP_TYPE_WIDGET (sp_widget_get_type ())
#define SP_WIDGET(obj) (GTK_CHECK_CAST ((obj), SP_TYPE_WIDGET, SPWidget))
#define SP_WIDGET_CLASS(klass) (GTK_CHECK_CLASS_CAST ((klass), SP_TYPE_WIDGET, SPWidgetClass))
#define SP_IS_WIDGET(obj) (GTK_CHECK_TYPE ((obj), SP_TYPE_WIDGET))
#define SP_IS_WIDGET_CLASS(klass) (GTK_CHECK_CLASS_TYPE ((klass), SP_TYPE_WIDGET))

typedef struct _SPWidget SPWidget;
typedef struct _SPWidgetClass SPWidgetClass;

#include <gtk/gtkbin.h>
#include "../forward.h"
#include "../xml/repr.h"
#include "../sodipodi.h"
#include "../desktop-handles.h"

struct _SPWidget {
	GtkBin bin;

	Sodipodi *sodipodi;
	SPObject *object;
	SPRepr *repr;

	/* fixme: We do not need these probably (Lauris) */
	/* fixme: Or maybe configurable? For text widget? (Lauris) */
	guint dirty : 1;
	guint autoupdate : 1;
};

struct _SPWidgetClass {
	GtkBinClass bin_class;
	void (* construct) (SPWidget *spw);
	/* Selection change handlers */
	void (* modify_selection) (SPWidget *spw, SPSelection *selection, guint flags);
	void (* change_selection) (SPWidget *spw, SPSelection *selection);
	void (* set_selection) (SPWidget *spw, SPSelection *selection);
	void (* attr_changed) (SPWidget *spw, const guchar *key, const guchar *oldval, const guchar *newval);
	/* Signal */
	void (* set_dirty) (SPWidget *spw, gboolean dirty);
};

GtkType sp_widget_get_type (void);

/* fixme: Think (Lauris) */
/* Generic constructor for global widget */
GtkWidget *sp_widget_new_global (Sodipodi *sodipodi);
GtkWidget *sp_widget_construct_global (SPWidget *spw, Sodipodi *sodipodi);
/* Generic constructor for global widget */
GtkWidget *sp_widget_new_repr (SPRepr *repr);
GtkWidget *sp_widget_construct_repr (SPWidget *spw, SPRepr *repr);

void sp_widget_set_dirty (SPWidget *spw, gboolean dirty);
void sp_widget_set_autoupdate (SPWidget *spw, gboolean autoupdate);

const GSList *sp_widget_get_item_list (SPWidget *spw);

/* fixme: Do clean way (Lauris) */
#define SP_WIDGET_DOCUMENT(spw) SP_ACTIVE_DOCUMENT
#define SP_WIDGET_DESKTOP(spw) SP_ACTIVE_DESKTOP
#define SP_WIDGET_SELECTION(spw) SP_DT_SELECTION (SP_ACTIVE_DESKTOP)

G_END_DECLS

#endif
