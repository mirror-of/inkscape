#define __SP_COLOR_SELECTOR_C__

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

#define noSPCS_PREVIEW

#include <config.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <gtk/gtksignal.h>
#include <gtk/gtkhbox.h>
#include <gtk/gtklabel.h>
#include <gtk/gtktable.h>
#include <gtk/gtkspinbutton.h>
#include "../color.h"
#include "../helper/sp-intl.h"
#include "sp-color-preview.h"
#include "sp-color-selector.h"

enum {
	GRABBED,
	DRAGGED,
	RELEASED,
	CHANGED,
	LAST_SIGNAL
};

#define CSEL_CHANNEL_R (1 << 0)
#define CSEL_CHANNEL_G (1 << 1)
#define CSEL_CHANNEL_B (1 << 2)
#define CSEL_CHANNEL_A (1 << 3)
#define CSEL_CHANNEL_H (1 << 0)
#define CSEL_CHANNEL_S (1 << 1)
#define CSEL_CHANNEL_V (1 << 2)
#define CSEL_CHANNEL_C (1 << 0)
#define CSEL_CHANNEL_M (1 << 1)
#define CSEL_CHANNEL_Y (1 << 2)
#define CSEL_CHANNEL_K (1 << 3)
#define CSEL_CHANNEL_CMYKA (1 << 4)

#define CSEL_CHANNELS_ALL 0

static void sp_color_selector_class_init (SPColorSelectorClass *klass);
static void sp_color_selector_init (SPColorSelector *slider);
static void sp_color_selector_destroy (GtkObject *object);

static void sp_color_selector_show_all (GtkWidget *widget);
static void sp_color_selector_hide_all (GtkWidget *widget);

static void sp_color_selector_adjustment_any_changed (GtkAdjustment *adjustment, SPColorSelector *csel);
static void sp_color_selector_slider_any_grabbed (SPColorSlider *slider, SPColorSelector *csel);
static void sp_color_selector_slider_any_released (SPColorSlider *slider, SPColorSelector *csel);
static void sp_color_selector_slider_any_changed (SPColorSlider *slider, SPColorSelector *csel);

static void sp_color_selector_adjustment_changed (SPColorSelector *csel, guint channel);

static void sp_color_selector_rgba_entry_changed (GtkEntry *entry, SPColorSelector *csel);

static void sp_color_selector_update_sliders (SPColorSelector *csel, guint channels);

static const guchar *sp_color_selector_hue_map (void);

static GtkVBoxClass *parent_class;
static guint csel_signals[LAST_SIGNAL] = {0};

GtkType
sp_color_selector_get_type (void)
{
	static GtkType type = 0;
	if (!type) {
		GtkTypeInfo info = {
			"SPColorSelector",
			sizeof (SPColorSelector),
			sizeof (SPColorSelectorClass),
			(GtkClassInitFunc) sp_color_selector_class_init,
			(GtkObjectInitFunc) sp_color_selector_init,
			NULL, NULL, NULL
		};
		type = gtk_type_unique (GTK_TYPE_VBOX, &info);
	}
	return type;
}

static void
sp_color_selector_class_init (SPColorSelectorClass *klass)
{
	GtkObjectClass *object_class;
	GtkWidgetClass *widget_class;

	object_class = (GtkObjectClass *) klass;
	widget_class = (GtkWidgetClass *) klass;

	parent_class = gtk_type_class (GTK_TYPE_VBOX);

	csel_signals[GRABBED] =  gtk_signal_new ("grabbed",
						 GTK_RUN_FIRST | GTK_RUN_NO_RECURSE,
						 GTK_CLASS_TYPE(object_class),
						 GTK_SIGNAL_OFFSET (SPColorSelectorClass, grabbed),
						 gtk_marshal_NONE__NONE,
						 GTK_TYPE_NONE, 0);
	csel_signals[DRAGGED] =  gtk_signal_new ("dragged",
						 GTK_RUN_FIRST | GTK_RUN_NO_RECURSE,
						 GTK_CLASS_TYPE(object_class),
						 GTK_SIGNAL_OFFSET (SPColorSelectorClass, dragged),
						 gtk_marshal_NONE__NONE,
						 GTK_TYPE_NONE, 0);
	csel_signals[RELEASED] = gtk_signal_new ("released",
						 GTK_RUN_FIRST | GTK_RUN_NO_RECURSE,
						 GTK_CLASS_TYPE(object_class),
						 GTK_SIGNAL_OFFSET (SPColorSelectorClass, released),
						 gtk_marshal_NONE__NONE,
						 GTK_TYPE_NONE, 0);
	csel_signals[CHANGED] =  gtk_signal_new ("changed",
						 GTK_RUN_FIRST | GTK_RUN_NO_RECURSE,
						 GTK_CLASS_TYPE(object_class),
						 GTK_SIGNAL_OFFSET (SPColorSelectorClass, changed),
						 gtk_marshal_NONE__NONE,
						 GTK_TYPE_NONE, 0);

	object_class->destroy = sp_color_selector_destroy;

	widget_class->show_all = sp_color_selector_show_all;
	widget_class->hide_all = sp_color_selector_hide_all;
}

#define XPAD 4
#define YPAD 1

static void
sp_color_selector_init (SPColorSelector *csel)
{
	GtkWidget *t;
	gint i;

	csel->updating = FALSE;
	csel->dragging = FALSE;

	t = gtk_table_new (5, 3, FALSE);
	gtk_widget_show (t);
	gtk_box_pack_start (GTK_BOX (csel), t, TRUE, TRUE, 0);

	/* Create components */
	for (i = 0; i < 5; i++) {
		/* Label */
		csel->l[i] = gtk_label_new ("");
		gtk_misc_set_alignment (GTK_MISC (csel->l[i]), 1.0, 0.5);
		gtk_widget_show (csel->l[i]);
		gtk_table_attach (GTK_TABLE (t), csel->l[i], 0, 1, i, i + 1, GTK_FILL, GTK_FILL, XPAD, YPAD);
		/* Adjustment */
		csel->a[i] = (GtkAdjustment *) gtk_adjustment_new (0.0, 0.0, 1.0, 0.01, 0.1, 0.1);
		/* Slider */
		csel->s[i] = sp_color_slider_new (csel->a[i]);
		gtk_widget_show (csel->s[i]);
		gtk_table_attach (GTK_TABLE (t), csel->s[i], 1, 2, i, i + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, XPAD, YPAD);

		/* Spinbutton */
		csel->b[i] = gtk_spin_button_new (GTK_ADJUSTMENT (csel->a[i]), 0.01, 2);
		gtk_widget_show (csel->b[i]);
		gtk_table_attach (GTK_TABLE (t), csel->b[i], 2, 3, i, i + 1, 0, 0, XPAD, YPAD);

		/* Attach channel value to adjustment */
		gtk_object_set_data (GTK_OBJECT (csel->a[i]), "channel", GINT_TO_POINTER (i));
		/* Signals */
		gtk_signal_connect (GTK_OBJECT (csel->a[i]), "value_changed",
				    GTK_SIGNAL_FUNC (sp_color_selector_adjustment_any_changed), csel);
		gtk_signal_connect (GTK_OBJECT (csel->s[i]), "grabbed",
				    GTK_SIGNAL_FUNC (sp_color_selector_slider_any_grabbed), csel);
		gtk_signal_connect (GTK_OBJECT (csel->s[i]), "released",
				    GTK_SIGNAL_FUNC (sp_color_selector_slider_any_released), csel);
		gtk_signal_connect (GTK_OBJECT (csel->s[i]), "changed",
				    GTK_SIGNAL_FUNC (sp_color_selector_slider_any_changed), csel);
	}

	/* Create RGBA entry and color preview */
	csel->rgbal = gtk_label_new ("RGBA:");
	gtk_misc_set_alignment (GTK_MISC (csel->l[i]), 1.0, 0.5);
	gtk_widget_show (csel->rgbal);
	gtk_table_attach (GTK_TABLE (t), csel->rgbal, 0, 1, i, i + 1, GTK_FILL, GTK_FILL, XPAD, YPAD);
	csel->rgbae = gtk_entry_new ();
	gtk_entry_set_max_length (GTK_ENTRY (csel->rgbae), 16);
	gtk_entry_set_width_chars (GTK_ENTRY (csel->rgbae), 10);
	gtk_widget_show (csel->rgbae);
	gtk_table_attach (GTK_TABLE (t), csel->rgbae, 1, 2, i, i + 1, GTK_FILL, GTK_FILL, XPAD, YPAD);
#ifdef SPCS_PREVIEW
	csel->p = sp_color_preview_new (0xffffffff);
	gtk_widget_show (csel->p);
	gtk_table_attach (GTK_TABLE (t), csel->p, 2, 3, i, i + 1, GTK_FILL, GTK_FILL, XPAD, YPAD);
#endif

	gtk_signal_connect (GTK_OBJECT (csel->rgbae), "changed", GTK_SIGNAL_FUNC (sp_color_selector_rgba_entry_changed), csel);

	/* Initial mode is none, so it works */
	sp_color_selector_set_mode (csel, SP_COLOR_SELECTOR_MODE_RGB);
}

static void
sp_color_selector_destroy (GtkObject *object)
{
	SPColorSelector *csel;
	gint i;

	csel = SP_COLOR_SELECTOR (object);

	for (i = 0; i < 5; i++) {
		csel->l[i] = NULL;
		csel->a[i] = NULL;
		csel->s[i] = NULL;
		csel->b[i] = NULL;
	}

	if (((GtkObjectClass *) (parent_class))->destroy)
		(* ((GtkObjectClass *) (parent_class))->destroy) (object);
}

static void
sp_color_selector_show_all (GtkWidget *widget)
{
	gtk_widget_show (widget);
}

static void
sp_color_selector_hide_all (GtkWidget *widget)
{
	gtk_widget_hide (widget);
}

GtkWidget *
sp_color_selector_new (void)
{
	SPColorSelector *csel;

	csel = gtk_type_new (SP_TYPE_COLOR_SELECTOR);

	sp_color_selector_set_any_rgba_float (csel, 1.0, 1.0, 1.0, 1.0);

	return GTK_WIDGET (csel);
}

void
sp_color_selector_set_any_color_alpha (SPColorSelector *cs, const SPColor *color, gfloat alpha)
{
	gfloat c[4];

	g_return_if_fail (cs != NULL);
	g_return_if_fail (SP_IS_COLOR_SELECTOR (cs));
	g_return_if_fail (color != NULL);

	switch (cs->mode) {
	case SP_COLOR_SELECTOR_MODE_RGB:
	case SP_COLOR_SELECTOR_MODE_HSV:
		sp_color_get_rgb_floatv (color, c);
		sp_color_selector_set_any_rgba_float (cs, c[0], c[1], c[2], alpha);
		break;
	case SP_COLOR_SELECTOR_MODE_CMYK:
		sp_color_get_cmyk_floatv (color, c);
		sp_color_selector_set_any_cmyka_float (cs, c[0], c[1], c[2], c[3], alpha);
		break;
	default:
		g_warning ("file %s: line %d: Illegal color selector mode %d", __FILE__, __LINE__, cs->mode);
		break;
	}
}

void
sp_color_selector_set_color_alpha (SPColorSelector *cs, const SPColor *color, gfloat alpha)
{
	SPColorSpaceClass csclass;
	SPColorSpaceType cstype;
	gfloat c[4];

	g_return_if_fail (cs != NULL);
	g_return_if_fail (SP_IS_COLOR_SELECTOR (cs));
	g_return_if_fail (color != NULL);

	csclass = sp_color_get_colorspace_class (color);
	if (csclass != SP_COLORSPACE_CLASS_PROCESS) {
		g_warning ("file %s: line %d: Unsupported colorspace class %d", __FILE__, __LINE__, csclass);
		return;
	}

	cstype = sp_color_get_colorspace_type (color);
	switch (cstype) {
	case SP_COLORSPACE_TYPE_RGB:
		sp_color_get_rgb_floatv (color, c);
		sp_color_selector_set_rgba_float (cs, c[0], c[1], c[2], alpha);
		break;
	case SP_COLORSPACE_TYPE_CMYK:
		sp_color_get_cmyk_floatv (color, c);
		sp_color_selector_set_cmyka_float (cs, c[0], c[1], c[2], c[3], alpha);
		break;
	default:
		g_warning ("file %s: line %d: Unsupported colorspace type %d", __FILE__, __LINE__, cstype);
		break;
	}
}

void
sp_color_selector_get_color_alpha (SPColorSelector *cs, SPColor *color, gfloat *alpha)
{
	gfloat c[5];

	g_return_if_fail (cs != NULL);
	g_return_if_fail (SP_IS_COLOR_SELECTOR (cs));
	g_return_if_fail (color != NULL);

	switch (cs->mode) {
	case SP_COLOR_SELECTOR_MODE_RGB:
	case SP_COLOR_SELECTOR_MODE_HSV:
		sp_color_selector_get_rgba_floatv (cs, c);
		sp_color_set_rgb_float (color, c[0], c[1], c[2]);
		if (alpha) *alpha = c[3];
		break;
	case SP_COLOR_SELECTOR_MODE_CMYK:
		sp_color_selector_get_cmyka_floatv (cs, c);
		sp_color_set_cmyk_float (color, c[0], c[1], c[2], c[3]);
		if (alpha) *alpha = c[4];
		break;
	default:
		g_warning ("file %s: line %d: Illegal color selector mode %d", __FILE__, __LINE__, cs->mode);
		break;
	}
}

/* Helpers for setting color value */

static void
spcs_set_color_rgba_mode (SPColorSelector *cs, gfloat r, gfloat g, gfloat b, gfloat a)
{
	gfloat c[3];

	if (cs->mode == SP_COLOR_SELECTOR_MODE_HSV) {
		c[0] = cs->a[0]->value;
		sp_color_rgb_to_hsv_floatv (c, r, g, b);
	} else {
		c[0] = r;
		c[1] = g;
		c[2] = b;
	}
	cs->updating = TRUE;
	gtk_adjustment_set_value (cs->a[0], c[0]);
	gtk_adjustment_set_value (cs->a[1], c[1]);
	gtk_adjustment_set_value (cs->a[2], c[2]);
	gtk_adjustment_set_value (cs->a[3], a);
	sp_color_selector_update_sliders (cs, CSEL_CHANNELS_ALL);
	cs->updating = FALSE;
	gtk_signal_emit (GTK_OBJECT (cs), csel_signals[CHANGED]);
}

static void
spcs_set_color_cmyka_mode (SPColorSelector *cs, gfloat c, gfloat m, gfloat y, gfloat k, gfloat a)
{
	cs->updating = TRUE;
	gtk_adjustment_set_value (cs->a[0], c);
	gtk_adjustment_set_value (cs->a[1], m);
	gtk_adjustment_set_value (cs->a[2], y);
	gtk_adjustment_set_value (cs->a[3], k);
	gtk_adjustment_set_value (cs->a[4], a);
	sp_color_selector_update_sliders (cs, CSEL_CHANNELS_ALL);
	cs->updating = FALSE;
	gtk_signal_emit (GTK_OBJECT (cs), csel_signals[CHANGED]);
}

/* fixme: This sucks, but doing HSV<->RGB with floats gives us big error */

#define CLOSE_ENOUGH(a,b) (fabs ((a) - (b)) < 1e-4)

void
sp_color_selector_set_any_rgba_float (SPColorSelector *csel, gfloat r, gfloat g, gfloat b, gfloat a)
{
	gfloat c[4];

	g_return_if_fail (csel != NULL);
	g_return_if_fail (SP_IS_COLOR_SELECTOR (csel));

	sp_color_selector_get_rgba_floatv (csel, c);

	if (CLOSE_ENOUGH (r, c[0]) && CLOSE_ENOUGH (g, c[1]) && CLOSE_ENOUGH (b, c[2]) && CLOSE_ENOUGH (a, c[3])) return;

	switch (csel->mode) {
	case SP_COLOR_SELECTOR_MODE_RGB:
	case SP_COLOR_SELECTOR_MODE_HSV:
		spcs_set_color_rgba_mode (csel, r, g, b, a);
		break;
	case SP_COLOR_SELECTOR_MODE_CMYK:
		sp_color_rgb_to_cmyk_floatv (c, r, g, b);
		spcs_set_color_cmyka_mode (csel, c[0], c[1], c[2], c[3], a);
		break;
	default:
		g_warning ("file %s: line %d: Illegal color selector mode %d", __FILE__, __LINE__, csel->mode);
		break;
	}
}

void
sp_color_selector_set_any_cmyka_float (SPColorSelector *csel, gfloat c, gfloat m, gfloat y, gfloat k, gfloat a)
{
	gfloat v[5];

	g_return_if_fail (csel != NULL);
	g_return_if_fail (SP_IS_COLOR_SELECTOR (csel));

	sp_color_selector_get_cmyka_floatv (csel, v);
	if (CLOSE_ENOUGH (c, v[0]) && CLOSE_ENOUGH (m, v[1]) && CLOSE_ENOUGH (y, v[2]) && CLOSE_ENOUGH (k, v[3]) && CLOSE_ENOUGH (a, v[4])) return;

	switch (csel->mode) {
	case SP_COLOR_SELECTOR_MODE_RGB:
	case SP_COLOR_SELECTOR_MODE_HSV:
		sp_color_cmyk_to_rgb_floatv (v, c, m, y, k);
		spcs_set_color_rgba_mode (csel, v[0], v[1], v[2], a);
		break;
	case SP_COLOR_SELECTOR_MODE_CMYK:
		spcs_set_color_cmyka_mode (csel, c, m, y, k, a);
		break;
	default:
		g_warning ("file %s: line %d: Illegal color selector mode %d", __FILE__, __LINE__, csel->mode);
		break;
	}
}

void
sp_color_selector_set_any_rgba32 (SPColorSelector *csel, guint32 rgba)
{
	g_return_if_fail (csel != NULL);
	g_return_if_fail (SP_IS_COLOR_SELECTOR (csel));

	sp_color_selector_set_any_rgba_float (csel, SP_RGBA32_R_F (rgba), SP_RGBA32_G_F (rgba), SP_RGBA32_B_F (rgba), SP_RGBA32_A_F (rgba));
}

void
sp_color_selector_set_rgba_float (SPColorSelector *cs, gfloat r, gfloat g, gfloat b, gfloat a)
{
	g_return_if_fail (cs != NULL);
	g_return_if_fail (SP_IS_COLOR_SELECTOR (cs));

	sp_color_selector_set_mode (cs, SP_COLOR_SELECTOR_MODE_RGB);
	sp_color_selector_set_any_rgba_float (cs, r, g, b, a);
}

void
sp_color_selector_set_cmyka_float (SPColorSelector *cs, gfloat c, gfloat m, gfloat y, gfloat k, gfloat a)
{
	g_return_if_fail (cs != NULL);
	g_return_if_fail (SP_IS_COLOR_SELECTOR (cs));

	sp_color_selector_set_mode (cs, SP_COLOR_SELECTOR_MODE_CMYK);
	sp_color_selector_set_any_cmyka_float (cs, c, m, y, k, a);
}

void
sp_color_selector_set_rgba32 (SPColorSelector *cs, guint32 rgba)
{
	g_return_if_fail (cs != NULL);
	g_return_if_fail (SP_IS_COLOR_SELECTOR (cs));

	sp_color_selector_set_mode (cs, SP_COLOR_SELECTOR_MODE_RGB);
	sp_color_selector_set_any_rgba32 (cs, rgba);
}

void
sp_color_selector_get_rgba_floatv (SPColorSelector *csel, gfloat *rgba)
{
	g_return_if_fail (csel != NULL);
	g_return_if_fail (SP_IS_COLOR_SELECTOR (csel));
	g_return_if_fail (rgba != NULL);

	switch (csel->mode) {
	case SP_COLOR_SELECTOR_MODE_RGB:
		rgba[0] = csel->a[0]->value;
		rgba[1] = csel->a[1]->value;
		rgba[2] = csel->a[2]->value;
		rgba[3] = csel->a[3]->value;
		break;
	case SP_COLOR_SELECTOR_MODE_HSV:
		sp_color_hsv_to_rgb_floatv (rgba, csel->a[0]->value, csel->a[1]->value, csel->a[2]->value);
		rgba[3] = csel->a[3]->value;
		break;
	case SP_COLOR_SELECTOR_MODE_CMYK:
		sp_color_cmyk_to_rgb_floatv (rgba, csel->a[0]->value, csel->a[1]->value, csel->a[2]->value, csel->a[3]->value);
		rgba[3] = csel->a[4]->value;
		break;
	default:
		g_warning ("file %s: line %d: Illegal color selector mode", __FILE__, __LINE__);
		break;
	}
}

void
sp_color_selector_get_cmyka_floatv (SPColorSelector *csel, gfloat *cmyka)
{
	gfloat rgb[3];

	g_return_if_fail (csel != NULL);
	g_return_if_fail (SP_IS_COLOR_SELECTOR (csel));
	g_return_if_fail (cmyka != NULL);

	switch (csel->mode) {
	case SP_COLOR_SELECTOR_MODE_RGB:
		sp_color_rgb_to_cmyk_floatv (cmyka, csel->a[0]->value, csel->a[1]->value, csel->a[2]->value);
		cmyka[4] = csel->a[3]->value;
		break;
	case SP_COLOR_SELECTOR_MODE_HSV:
		sp_color_hsv_to_rgb_floatv (rgb, csel->a[0]->value, csel->a[1]->value, csel->a[2]->value);
		sp_color_rgb_to_cmyk_floatv (cmyka, rgb[0], rgb[1], rgb[2]);
		cmyka[4] = csel->a[3]->value;
		break;
	case SP_COLOR_SELECTOR_MODE_CMYK:
		cmyka[0] = csel->a[0]->value;
		cmyka[1] = csel->a[1]->value;
		cmyka[2] = csel->a[2]->value;
		cmyka[3] = csel->a[3]->value;
		cmyka[4] = csel->a[4]->value;
		break;
	default:
		g_warning ("file %s: line %d: Illegal color selector mode", __FILE__, __LINE__);
		break;
	}
}

gfloat
sp_color_selector_get_r (SPColorSelector *csel)
{
	gfloat rgba[4];

	g_return_val_if_fail (csel != NULL, 0.0);
	g_return_val_if_fail (SP_IS_COLOR_SELECTOR (csel), 0.0);

	sp_color_selector_get_rgba_floatv (csel, rgba);

	return rgba[0];
}

gfloat
sp_color_selector_get_g (SPColorSelector *csel)
{
	gfloat rgba[4];

	g_return_val_if_fail (csel != NULL, 0.0);
	g_return_val_if_fail (SP_IS_COLOR_SELECTOR (csel), 0.0);

	sp_color_selector_get_rgba_floatv (csel, rgba);

	return rgba[1];
}

gfloat
sp_color_selector_get_b (SPColorSelector *csel)
{
	gfloat rgba[4];

	g_return_val_if_fail (csel != NULL, 0.0);
	g_return_val_if_fail (SP_IS_COLOR_SELECTOR (csel), 0.0);

	sp_color_selector_get_rgba_floatv (csel, rgba);

	return rgba[2];
}

gfloat
sp_color_selector_get_a (SPColorSelector *csel)
{
	gfloat rgba[4];

	g_return_val_if_fail (csel != NULL, 0.0);
	g_return_val_if_fail (SP_IS_COLOR_SELECTOR (csel), 0.0);

	sp_color_selector_get_rgba_floatv (csel, rgba);

	return rgba[3];
}

guint32
sp_color_selector_get_rgba32 (SPColorSelector *csel)
{
	gfloat c[4];
	guint32 rgba;

	g_return_val_if_fail (csel != NULL, 0.0);
	g_return_val_if_fail (SP_IS_COLOR_SELECTOR (csel), 0.0);

	sp_color_selector_get_rgba_floatv (csel, c);

	rgba = SP_RGBA32_F_COMPOSE (c[0], c[1], c[2], c[3]);

	return rgba;
}

void
sp_color_selector_set_mode (SPColorSelector *csel, SPColorSelectorMode mode)
{
	gfloat r, g, b, a;
	gfloat c[4];

	if (csel->mode == mode) return;

	if ((csel->mode == SP_COLOR_SELECTOR_MODE_RGB) ||
	    (csel->mode == SP_COLOR_SELECTOR_MODE_HSV) ||
	    (csel->mode == SP_COLOR_SELECTOR_MODE_CMYK)) {
		r = sp_color_selector_get_r (csel);
		g = sp_color_selector_get_g (csel);
		b = sp_color_selector_get_b (csel);
		a = sp_color_selector_get_a (csel);
	} else {
		r = g = b = a = 1.0;
	}

	csel->mode = mode;

	switch (mode) {
	case SP_COLOR_SELECTOR_MODE_RGB:
		gtk_label_set_text (GTK_LABEL (csel->l[0]), _("Red:"));
		gtk_label_set_text (GTK_LABEL (csel->l[1]), _("Green:"));
		gtk_label_set_text (GTK_LABEL (csel->l[2]), _("Blue:"));
		gtk_label_set_text (GTK_LABEL (csel->l[3]), _("Alpha:"));
		sp_color_slider_set_map (SP_COLOR_SLIDER (csel->s[0]), NULL);
		gtk_widget_hide (csel->l[4]);
		gtk_widget_hide (csel->s[4]);
		gtk_widget_hide (csel->b[4]);
		gtk_widget_show (csel->rgbal);
		gtk_widget_show (csel->rgbae);
		csel->updating = TRUE;
		gtk_adjustment_set_value (csel->a[0], r);
		gtk_adjustment_set_value (csel->a[1], g);
		gtk_adjustment_set_value (csel->a[2], b);
		gtk_adjustment_set_value (csel->a[3], a);
		csel->updating = FALSE;
		sp_color_selector_update_sliders (csel, CSEL_CHANNELS_ALL);
		gtk_signal_emit (GTK_OBJECT (csel), csel_signals[CHANGED]);
		break;
	case SP_COLOR_SELECTOR_MODE_HSV:
		gtk_label_set_text (GTK_LABEL (csel->l[0]), _("Hue:"));
		gtk_label_set_text (GTK_LABEL (csel->l[1]), _("Saturation:"));
		gtk_label_set_text (GTK_LABEL (csel->l[2]), _("Value:"));
		gtk_label_set_text (GTK_LABEL (csel->l[3]), _("Alpha:"));
		sp_color_slider_set_map (SP_COLOR_SLIDER (csel->s[0]), sp_color_selector_hue_map ());
		gtk_widget_hide (csel->l[4]);
		gtk_widget_hide (csel->s[4]);
		gtk_widget_hide (csel->b[4]);
		gtk_widget_show (csel->rgbal);
		gtk_widget_show (csel->rgbae);
		csel->updating = TRUE;
		sp_color_rgb_to_hsv_floatv (c, r, g, b);
		gtk_adjustment_set_value (csel->a[0], c[0]);
		gtk_adjustment_set_value (csel->a[1], c[1]);
		gtk_adjustment_set_value (csel->a[2], c[2]);
		gtk_adjustment_set_value (csel->a[3], a);
		csel->updating = FALSE;
		sp_color_selector_update_sliders (csel, CSEL_CHANNELS_ALL);
		gtk_signal_emit (GTK_OBJECT (csel), csel_signals[CHANGED]);
		break;
	case SP_COLOR_SELECTOR_MODE_CMYK:
		gtk_label_set_text (GTK_LABEL (csel->l[0]), _("Cyan:"));
		gtk_label_set_text (GTK_LABEL (csel->l[1]), _("Magenta:"));
		gtk_label_set_text (GTK_LABEL (csel->l[2]), _("Yellow:"));
		gtk_label_set_text (GTK_LABEL (csel->l[3]), _("Black:"));
		gtk_label_set_text (GTK_LABEL (csel->l[4]), _("Alpha:"));
		sp_color_slider_set_map (SP_COLOR_SLIDER (csel->s[0]), NULL);
		gtk_widget_show (csel->l[4]);
		gtk_widget_show (csel->s[4]);
		gtk_widget_show (csel->b[4]);
		gtk_widget_hide (csel->rgbal);
		gtk_widget_hide (csel->rgbae);
		csel->updating = TRUE;
		sp_color_rgb_to_cmyk_floatv (c, r, g, b);
		gtk_adjustment_set_value (csel->a[0], c[0]);
		gtk_adjustment_set_value (csel->a[1], c[1]);
		gtk_adjustment_set_value (csel->a[2], c[2]);
		gtk_adjustment_set_value (csel->a[3], c[3]);
		gtk_adjustment_set_value (csel->a[4], a);
		csel->updating = FALSE;
		sp_color_selector_update_sliders (csel, CSEL_CHANNELS_ALL);
		gtk_signal_emit (GTK_OBJECT (csel), csel_signals[CHANGED]);
		break;
	default:
		g_warning ("file %s: line %d: Illegal color selector mode", __FILE__, __LINE__);
		break;
	}
}

SPColorSelectorMode
sp_color_selector_get_mode (SPColorSelector *csel)
{
	g_return_val_if_fail (csel != NULL, SP_COLOR_SELECTOR_MODE_NONE);
	g_return_val_if_fail (SP_IS_COLOR_SELECTOR (csel), SP_COLOR_SELECTOR_MODE_NONE);

	return csel->mode;
}

static void
sp_color_selector_adjustment_any_changed (GtkAdjustment *adjustment, SPColorSelector *csel)
{
	gint channel;

	channel = GPOINTER_TO_INT (gtk_object_get_data (GTK_OBJECT (adjustment), "channel"));

	sp_color_selector_adjustment_changed (csel, channel);
}

static void
sp_color_selector_slider_any_grabbed (SPColorSlider *slider, SPColorSelector *csel)
{
	if (!csel->dragging) {
		csel->dragging = TRUE;
		gtk_signal_emit (GTK_OBJECT (csel), csel_signals[GRABBED]);
	}
}

static void
sp_color_selector_slider_any_released (SPColorSlider *slider, SPColorSelector *csel)
{
	if (csel->dragging) {
		csel->dragging = FALSE;
		gtk_signal_emit (GTK_OBJECT (csel), csel_signals[RELEASED]);
	}
}

static void
sp_color_selector_slider_any_changed (SPColorSlider *slider, SPColorSelector *csel)
{
	gtk_signal_emit (GTK_OBJECT (csel), csel_signals[CHANGED]);
}

static void
sp_color_selector_adjustment_changed (SPColorSelector *csel, guint channel)
{
	if (csel->updating) return;

	csel->updating = TRUE;

	sp_color_selector_update_sliders (csel, (1 << channel));

	if (csel->dragging) {
		gtk_signal_emit (GTK_OBJECT (csel), csel_signals[DRAGGED]);
	} else {
		gtk_signal_emit (GTK_OBJECT (csel), csel_signals[CHANGED]);
	}

	csel->updating = FALSE;
}

static void
sp_color_selector_rgba_entry_changed (GtkEntry *entry, SPColorSelector *csel)
{
	const gchar *t;
	gchar *e;
	guint rgba;

	if (csel->updating) return;
	if (csel->updatingrgba) return;

	t = gtk_entry_get_text (entry);

	if (t) {
		rgba = strtoul (t, &e, 16);
		if (e && e != t) {
			if (strlen (t) < 5) {
				/* treat as rgba instead of rrggbbaa */
				rgba = ((rgba << 16) & 0xf0000000) |
					((rgba << 12) & 0xff00000) |
					((rgba << 8) & 0xff000) |
					((rgba << 4) & 0xff0) |
					(rgba & 0xf);
			}
			csel->updatingrgba = TRUE;
			sp_color_selector_set_any_rgba32 (csel, rgba);
			csel->updatingrgba = FALSE;
		}
	}
}

static void
sp_color_selector_update_sliders (SPColorSelector *csel, guint channels)
{
	gfloat rgb0[3], rgb1[3];
#ifdef SPCS_PREVIEW
	guint32 rgba;
#endif
	switch (csel->mode) {
	case SP_COLOR_SELECTOR_MODE_RGB:
		if ((channels != CSEL_CHANNEL_R) && (channels != CSEL_CHANNEL_A)) {
			/* Update red */
			sp_color_slider_set_colors (SP_COLOR_SLIDER (csel->s[0]),
						    SP_RGBA32_F_COMPOSE (0.0, csel->a[1]->value, csel->a[2]->value, 1.0),
						    SP_RGBA32_F_COMPOSE (1.0, csel->a[1]->value, csel->a[2]->value, 1.0));
		}
		if ((channels != CSEL_CHANNEL_G) && (channels != CSEL_CHANNEL_A)) {
			/* Update green */
			sp_color_slider_set_colors (SP_COLOR_SLIDER (csel->s[1]),
						    SP_RGBA32_F_COMPOSE (csel->a[0]->value, 0.0, csel->a[2]->value, 1.0),
						    SP_RGBA32_F_COMPOSE (csel->a[0]->value, 1.0, csel->a[2]->value, 1.0));
		}
		if ((channels != CSEL_CHANNEL_B) && (channels != CSEL_CHANNEL_A)) {
			/* Update blue */
			sp_color_slider_set_colors (SP_COLOR_SLIDER (csel->s[2]),
						    SP_RGBA32_F_COMPOSE (csel->a[0]->value, csel->a[1]->value, 0.0, 1.0),
						    SP_RGBA32_F_COMPOSE (csel->a[0]->value, csel->a[1]->value, 1.0, 1.0));
		}
		if (channels != CSEL_CHANNEL_A) {
			/* Update alpha */
			sp_color_slider_set_colors (SP_COLOR_SLIDER (csel->s[3]),
						    SP_RGBA32_F_COMPOSE (csel->a[0]->value, csel->a[1]->value, csel->a[2]->value, 0.0),
						    SP_RGBA32_F_COMPOSE (csel->a[0]->value, csel->a[1]->value, csel->a[2]->value, 1.0));
		}
		if (!csel->updatingrgba) {
			guchar s[32];
			/* Update RGBA entry */
			g_snprintf (s, 32, "%08x", SP_RGBA32_F_COMPOSE (csel->a[0]->value, csel->a[1]->value, csel->a[2]->value, csel->a[3]->value));
			gtk_entry_set_text (GTK_ENTRY (csel->rgbae), s);
		}
		break;
	case SP_COLOR_SELECTOR_MODE_HSV:
		/* Hue is never updated */
		if ((channels != CSEL_CHANNEL_S) && (channels != CSEL_CHANNEL_A)) {
			/* Update saturation */
			sp_color_hsv_to_rgb_floatv (rgb0, csel->a[0]->value, 0.0, csel->a[2]->value);
			sp_color_hsv_to_rgb_floatv (rgb1, csel->a[0]->value, 1.0, csel->a[2]->value);
			sp_color_slider_set_colors (SP_COLOR_SLIDER (csel->s[1]),
						    SP_RGBA32_F_COMPOSE (rgb0[0], rgb0[1], rgb0[2], 1.0),
						    SP_RGBA32_F_COMPOSE (rgb1[0], rgb1[1], rgb1[2], 1.0));
		}
		if ((channels != CSEL_CHANNEL_V) && (channels != CSEL_CHANNEL_A)) {
			/* Update value */
			sp_color_hsv_to_rgb_floatv (rgb0, csel->a[0]->value, csel->a[1]->value, 0.0);
			sp_color_hsv_to_rgb_floatv (rgb1, csel->a[0]->value, csel->a[1]->value, 1.0);
			sp_color_slider_set_colors (SP_COLOR_SLIDER (csel->s[2]),
						    SP_RGBA32_F_COMPOSE (rgb0[0], rgb0[1], rgb0[2], 1.0),
						    SP_RGBA32_F_COMPOSE (rgb1[0], rgb1[1], rgb1[2], 1.0));
		}
		if (channels != CSEL_CHANNEL_A) {
			/* Update alpha */
			sp_color_hsv_to_rgb_floatv (rgb0, csel->a[0]->value, csel->a[1]->value, csel->a[2]->value);
			sp_color_slider_set_colors (SP_COLOR_SLIDER (csel->s[3]),
						    SP_RGBA32_F_COMPOSE (rgb0[0], rgb0[1], rgb0[2], 0.0),
						    SP_RGBA32_F_COMPOSE (rgb0[0], rgb0[1], rgb0[2], 1.0));
		}
		if (!csel->updatingrgba) {
			guchar s[32];
			/* Update RGBA entry */
			sp_color_hsv_to_rgb_floatv (rgb0, csel->a[0]->value, csel->a[1]->value, csel->a[2]->value);
			g_snprintf (s, 32, "%08x", SP_RGBA32_F_COMPOSE (rgb0[0], rgb0[1], rgb0[2], csel->a[3]->value));
			gtk_entry_set_text (GTK_ENTRY (csel->rgbae), s);
		}
		break;
	case SP_COLOR_SELECTOR_MODE_CMYK:
		if ((channels != CSEL_CHANNEL_C) && (channels != CSEL_CHANNEL_CMYKA)) {
			/* Update saturation */
			sp_color_cmyk_to_rgb_floatv (rgb0, 0.0, csel->a[1]->value, csel->a[2]->value, csel->a[3]->value);
			sp_color_cmyk_to_rgb_floatv (rgb1, 1.0, csel->a[1]->value, csel->a[2]->value, csel->a[3]->value);
			sp_color_slider_set_colors (SP_COLOR_SLIDER (csel->s[0]),
						    SP_RGBA32_F_COMPOSE (rgb0[0], rgb0[1], rgb0[2], 1.0),
						    SP_RGBA32_F_COMPOSE (rgb1[0], rgb1[1], rgb1[2], 1.0));
		}
		if ((channels != CSEL_CHANNEL_M) && (channels != CSEL_CHANNEL_CMYKA)) {
			/* Update saturation */
			sp_color_cmyk_to_rgb_floatv (rgb0, csel->a[0]->value, 0.0, csel->a[2]->value, csel->a[3]->value);
			sp_color_cmyk_to_rgb_floatv (rgb1, csel->a[0]->value, 1.0, csel->a[2]->value, csel->a[3]->value);
			sp_color_slider_set_colors (SP_COLOR_SLIDER (csel->s[1]),
						    SP_RGBA32_F_COMPOSE (rgb0[0], rgb0[1], rgb0[2], 1.0),
						    SP_RGBA32_F_COMPOSE (rgb1[0], rgb1[1], rgb1[2], 1.0));
		}
		if ((channels != CSEL_CHANNEL_Y) && (channels != CSEL_CHANNEL_CMYKA)) {
			/* Update saturation */
			sp_color_cmyk_to_rgb_floatv (rgb0, csel->a[0]->value, csel->a[1]->value, 0.0, csel->a[3]->value);
			sp_color_cmyk_to_rgb_floatv (rgb1, csel->a[0]->value, csel->a[1]->value, 1.0, csel->a[3]->value);
			sp_color_slider_set_colors (SP_COLOR_SLIDER (csel->s[2]),
						    SP_RGBA32_F_COMPOSE (rgb0[0], rgb0[1], rgb0[2], 1.0),
						    SP_RGBA32_F_COMPOSE (rgb1[0], rgb1[1], rgb1[2], 1.0));
		}
		if ((channels != CSEL_CHANNEL_K) && (channels != CSEL_CHANNEL_CMYKA)) {
			/* Update saturation */
			sp_color_cmyk_to_rgb_floatv (rgb0, csel->a[0]->value, csel->a[1]->value, csel->a[2]->value, 0.0);
			sp_color_cmyk_to_rgb_floatv (rgb1, csel->a[0]->value, csel->a[1]->value, csel->a[2]->value, 1.0);
			sp_color_slider_set_colors (SP_COLOR_SLIDER (csel->s[3]),
						    SP_RGBA32_F_COMPOSE (rgb0[0], rgb0[1], rgb0[2], 1.0),
						    SP_RGBA32_F_COMPOSE (rgb1[0], rgb1[1], rgb1[2], 1.0));
		}
		if (channels != CSEL_CHANNEL_CMYKA) {
			/* Update saturation */
			sp_color_cmyk_to_rgb_floatv (rgb0, csel->a[0]->value, csel->a[1]->value, csel->a[2]->value, csel->a[3]->value);
			sp_color_slider_set_colors (SP_COLOR_SLIDER (csel->s[4]),
						    SP_RGBA32_F_COMPOSE (rgb0[0], rgb0[1], rgb0[2], 0.0),
						    SP_RGBA32_F_COMPOSE (rgb0[0], rgb0[1], rgb0[2], 1.0));
		}
		break;
	default:
		g_warning ("file %s: line %d: Illegal color selector mode", __FILE__, __LINE__);
		break;
	}
#ifdef SPCS_PREVIEW
	rgba = sp_color_selector_get_rgba32 (csel);
	sp_color_preview_set_rgba32 (SP_COLOR_PREVIEW (csel->p), rgba);
#endif
}

static const guchar *
sp_color_selector_hue_map (void)
{
	static guchar *map = NULL;

	if (!map) {
		guchar *p;
		gint h;
		map = g_new (guchar, 4 * 1024);
		p = map;
		for (h = 0; h < 1024; h++) {
			gfloat rgb[3];
			sp_color_hsv_to_rgb_floatv (rgb, h / 1024.0, 1.0, 1.0);
			*p++ = SP_COLOR_F_TO_U (rgb[0]);
			*p++ = SP_COLOR_F_TO_U (rgb[1]);
			*p++ = SP_COLOR_F_TO_U (rgb[2]);
			*p++ = 255;
		}
	}

	return map;
}

