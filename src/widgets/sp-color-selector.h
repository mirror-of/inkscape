#ifndef __SP_COLOR_SELECTOR_H__
#define __SP_COLOR_SELECTOR_H__

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
#include "sp-color-slider.h"

typedef struct _SPColorSelector SPColorSelector;
typedef struct _SPColorSelectorClass SPColorSelectorClass;

#define SP_TYPE_COLOR_SELECTOR (sp_color_selector_get_type ())
#define SP_COLOR_SELECTOR(o) (GTK_CHECK_CAST ((o), SP_TYPE_COLOR_SELECTOR, SPColorSelector))
#define SP_COLOR_SELECTOR_CLASS(k) (GTK_CHECK_CLASS_CAST ((k), SP_TYPE_COLOR_SELECTOR, SPColorSelectorClass))
#define SP_IS_COLOR_SELECTOR(o) (GTK_CHECK_TYPE ((o), SP_TYPE_COLOR_SELECTOR))
#define SP_IS_COLOR_SELECTOR_CLASS(k) (GTK_CHECK_CLASS_TYPE ((k), SP_TYPE_COLOR_SELECTOR))

typedef enum {
	SP_COLOR_SELECTOR_MODE_NONE,
	SP_COLOR_SELECTOR_MODE_RGB,
	SP_COLOR_SELECTOR_MODE_HSV,
	SP_COLOR_SELECTOR_MODE_CMYK
} SPColorSelectorMode;

struct _SPColorSelector {
	GtkVBox vbox;
	gint8 mode;
	guint updating : 1;
	guint updatingrgba : 1;
	guint dragging : 1;
	GtkAdjustment *a[5]; /* Channel adjustments */
	GtkWidget *s[5]; /* Channel sliders */
	GtkWidget *b[5]; /* Spinbuttons */
	GtkWidget *l[5]; /* Labels */
	GtkWidget *rgbal, *rgbae; /* RGBA entry */
	GtkWidget *p; /* Color preview */
};

struct _SPColorSelectorClass {
	GtkVBoxClass parent_class;

	void (* grabbed) (SPColorSelector *rgbsel);
	void (* dragged) (SPColorSelector *rgbsel);
	void (* released) (SPColorSelector *rgbsel);
	void (* changed) (SPColorSelector *rgbsel);
};

GtkType sp_color_selector_get_type (void);

GtkWidget *sp_color_selector_new (void);

void sp_color_selector_set_mode (SPColorSelector *csel, SPColorSelectorMode mode);
SPColorSelectorMode sp_color_selector_get_mode (SPColorSelector *csel);

/* Syntax is: ..._set_[colorspace]_valuetype */
/* Missing colorspace means, that selector mode will be adjusted */
/* Any means, that selector colorspace remains unchanged */

void sp_color_selector_set_any_color_alpha (SPColorSelector *csel, const SPColor *color, gfloat alpha);
void sp_color_selector_set_color_alpha (SPColorSelector *csel, const SPColor *color, gfloat alpha);
void sp_color_selector_get_color_alpha (SPColorSelector *csel, SPColor *color, gfloat *alpha);

void sp_color_selector_set_any_rgba_float (SPColorSelector *csel, gfloat r, gfloat g, gfloat b, gfloat a);
void sp_color_selector_set_any_cmyka_float (SPColorSelector *csel, gfloat c, gfloat m, gfloat y, gfloat k, gfloat a);
void sp_color_selector_set_any_rgba32 (SPColorSelector *csel, guint32 rgba);

void sp_color_selector_set_rgba_float (SPColorSelector *csel, gfloat r, gfloat g, gfloat b, gfloat a);
void sp_color_selector_set_cmyka_float (SPColorSelector *csel, gfloat c, gfloat m, gfloat y, gfloat k, gfloat a);
void sp_color_selector_set_rgba32 (SPColorSelector *csel, guint32 rgba);

void sp_color_selector_get_rgba_floatv (SPColorSelector *csel, gfloat *rgba);
void sp_color_selector_get_cmyka_floatv (SPColorSelector *csel, gfloat *cmyka);

gfloat sp_color_selector_get_r (SPColorSelector *csel);
gfloat sp_color_selector_get_g (SPColorSelector *csel);
gfloat sp_color_selector_get_b (SPColorSelector *csel);
gfloat sp_color_selector_get_a (SPColorSelector *csel);

guint32 sp_color_selector_get_rgba32 (SPColorSelector *csel);

#endif
