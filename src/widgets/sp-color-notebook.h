#ifndef __SP_COLOR_NOTEBOOK_H__
#define __SP_COLOR_NOTEBOOK_H__

/*
 * A block of 3 color sliders plus spinbuttons
 *
 * Author:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * Copyright (C) 2001-2002 Lauris Kaplinski
 *
 * This code is in public domain
 */

#include <gtk/gtkvbox.h>
#include "../color.h"
#include "sp-color-selector.h"

#include <glib.h>

G_BEGIN_DECLS

typedef struct _SPColorNotebook SPColorNotebook;
typedef struct _SPColorNotebookClass SPColorNotebookClass;
typedef struct _SPColorNotebookTracker SPColorNotebookTracker;


#define SP_TYPE_COLOR_NOTEBOOK (sp_color_notebook_get_type ())
#define SP_COLOR_NOTEBOOK(o) (GTK_CHECK_CAST ((o), SP_TYPE_COLOR_NOTEBOOK, SPColorNotebook))
#define SP_COLOR_NOTEBOOK_CLASS(k) (GTK_CHECK_CLASS_CAST ((k), SP_TYPE_COLOR_NOTEBOOK, SPColorNotebookClass))
#define SP_IS_COLOR_NOTEBOOK(o) (GTK_CHECK_TYPE ((o), SP_TYPE_COLOR_NOTEBOOK))
#define SP_IS_COLOR_NOTEBOOK_CLASS(k) (GTK_CHECK_CLASS_TYPE ((k), SP_TYPE_COLOR_NOTEBOOK))

struct _SPColorNotebook {
	SPColorSelector parent;    /* Parent */
	gboolean updating : 1;
	gboolean updatingrgba : 1;
	gboolean dragging : 1;
	gulong id;
	GtkWidget *book;
	GtkWidget *rgbal, *rgbae; /* RGBA entry */
	GtkWidget *p; /* Color preview */
	GtkWidget *btn;
	GtkWidget *popup;
	GPtrArray *trackerList;
};

struct _SPColorNotebookClass {
	SPColorSelectorClass parent_class;

	void (* grabbed) (SPColorNotebook *rgbsel);
	void (* dragged) (SPColorNotebook *rgbsel);
	void (* released) (SPColorNotebook *rgbsel);
	void (* changed) (SPColorNotebook *rgbsel);
};

GtkType sp_color_notebook_get_type (void);

GtkWidget *sp_color_notebook_new (void);

/* void sp_color_notebook_set_mode (SPColorNotebook *csel, SPColorNotebookMode mode); */
/* SPColorNotebookMode sp_color_notebook_get_mode (SPColorNotebook *csel); */

/* Syntax is: ..._set_[colorspace]_valuetype */
/* Missing colorspace means, that selector mode will be adjusted */
/* Any means, that selector colorspace remains unchanged */

void sp_color_notebook_set_any_color_alpha (SPColorSelector *csel, const SPColor *color, gfloat alpha);
void sp_color_notebook_set_color_alpha (SPColorSelector *csel, const SPColor *color, gfloat alpha);
void sp_color_notebook_get_color_alpha (SPColorSelector *csel, SPColor *color, gfloat *alpha);

void sp_color_notebook_set_any_rgba_float (SPColorSelector *csel, gfloat r, gfloat g, gfloat b, gfloat a);
void sp_color_notebook_set_any_cmyka_float (SPColorSelector *csel, gfloat c, gfloat m, gfloat y, gfloat k, gfloat a);
void sp_color_notebook_set_any_rgba32 (SPColorSelector *csel, guint32 rgba);

void sp_color_notebook_set_rgba_float (SPColorSelector *csel, gfloat r, gfloat g, gfloat b, gfloat a);
void sp_color_notebook_set_cmyka_float (SPColorSelector *csel, gfloat c, gfloat m, gfloat y, gfloat k, gfloat a);
void sp_color_notebook_set_rgba32 (SPColorSelector *csel, guint32 rgba);

void sp_color_notebook_get_rgba_floatv (SPColorSelector *csel, gfloat *rgba);
void sp_color_notebook_get_cmyka_floatv (SPColorSelector *csel, gfloat *cmyka);

gfloat sp_color_notebook_get_r (SPColorSelector *csel);
gfloat sp_color_notebook_get_g (SPColorSelector *csel);
gfloat sp_color_notebook_get_b (SPColorSelector *csel);
gfloat sp_color_notebook_get_a (SPColorSelector *csel);

guint32 sp_color_notebook_get_rgba32 (SPColorSelector *csel);

G_END_DECLS

#endif
