#include <config.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <gtk/gtksignal.h>
#include <gtk/gtkhbox.h>
#include <gtk/gtklabel.h>
#include <gtk/gtktable.h>
#include <gtk/gtkspinbutton.h>
#include <gtk/gtknotebook.h>
#include <gtk/gtkcolorsel.h>
#include "../color.h"
#include "../helper/sp-intl.h"
#include "../dialogs/dialog-events.h"
#include "sp-color-preview.h"
#include "sp-color-scales.h"

enum {
	GRABBED,
	DRAGGED,
	RELEASED,
	CHANGED,
	LAST_SIGNAL
};

#define CSC_CHANNEL_R (1 << 0)
#define CSC_CHANNEL_G (1 << 1)
#define CSC_CHANNEL_B (1 << 2)
#define CSC_CHANNEL_A (1 << 3)
#define CSC_CHANNEL_H (1 << 0)
#define CSC_CHANNEL_S (1 << 1)
#define CSC_CHANNEL_V (1 << 2)
#define CSC_CHANNEL_C (1 << 0)
#define CSC_CHANNEL_M (1 << 1)
#define CSC_CHANNEL_Y (1 << 2)
#define CSC_CHANNEL_K (1 << 3)
#define CSC_CHANNEL_CMYKA (1 << 4)

#define CSC_CHANNELS_ALL 0


G_BEGIN_DECLS

static void sp_color_scales_class_init (SPColorScalesClass *klass);
static void sp_color_scales_init (SPColorScales *cs);
static void sp_color_scales_destroy (GtkObject *object);

static void sp_color_scales_show_all (GtkWidget *widget);
static void sp_color_scales_hide_all (GtkWidget *widget);

static void sp_color_scales_set_submode (SPColorSelector *csel, guint submode);
static guint sp_color_scales_get_submode (SPColorSelector *csel);

static void sp_color_scales_adjustment_any_changed (GtkAdjustment *adjustment, SPColorScales *cs);
static void sp_color_scales_slider_any_grabbed (SPColorSlider *slider, SPColorScales *cs);
static void sp_color_scales_slider_any_released (SPColorSlider *slider, SPColorScales *cs);
static void sp_color_scales_slider_any_changed (SPColorSlider *slider, SPColorScales *cs);

static void sp_color_scales_adjustment_changed (SPColorScales *cs, guint channel);

static void sp_color_scales_update_sliders (SPColorScales *cs, guint channels);

static const gchar *sp_color_scales_hue_map (void);

static void sp_color_scales_get_rgba_floatv (SPColorSelector *csel, gfloat *rgba);
static void sp_color_scales_get_cmyka_floatv (SPColorSelector *csel, gfloat *cmyka);

static void spcs_set_color (SPColorScales *cs, const SPColor* color, gfloat alpha);

G_END_DECLS

static SPColorSelectorClass *parent_class;
static guint cs_signals[LAST_SIGNAL] = {0};

#define XPAD 4
#define YPAD 1

GType
sp_color_scales_get_type (void)
{
	static GType type = 0;
	if (!type) {
		static const GTypeInfo info = {
			sizeof (SPColorScalesClass),
			NULL, /* base_init */
			NULL, /* base_finalize */
			(GClassInitFunc) sp_color_scales_class_init,
			NULL, /* class_finalize */
			NULL, /* class_data */
			sizeof (SPColorScales),
			0,	  /* n_preallocs */
			(GInstanceInitFunc) sp_color_scales_init,
		};

		type = g_type_register_static (SP_TYPE_COLOR_SELECTOR,
									   "SPColorScales",
									   &info,
									   INK_STATIC_CAST( GTypeFlags, 0));
	}
	return type;
}

static void
sp_color_scales_class_init (SPColorScalesClass *klass)
{
	static const gchar* nameset[] = {N_("RGB"), N_("HSV"), N_("CMYK"), 0};
	GtkObjectClass *object_class;
	GtkWidgetClass *widget_class;
	SPColorSelectorClass *selector_class;

	object_class = (GtkObjectClass *) klass;
	widget_class = (GtkWidgetClass *) klass;
	selector_class = SP_COLOR_SELECTOR_CLASS (klass);

	parent_class = SP_COLOR_SELECTOR_CLASS (g_type_class_peek_parent (klass));

	selector_class->name = nameset;
	selector_class->submode_count = 3;

	cs_signals[GRABBED] =  gtk_signal_lookup ("grabbed", SP_TYPE_COLOR_SELECTOR);
	cs_signals[DRAGGED] =  gtk_signal_lookup ("dragged", SP_TYPE_COLOR_SELECTOR);
	cs_signals[RELEASED] = gtk_signal_lookup ("released", SP_TYPE_COLOR_SELECTOR);
	cs_signals[CHANGED] =  gtk_signal_lookup ("changed", SP_TYPE_COLOR_SELECTOR);

	selector_class->set_submode = sp_color_scales_set_submode;
	selector_class->get_submode = sp_color_scales_get_submode;

	selector_class->set_color_alpha = sp_color_scales_set_color_alpha;
	selector_class->get_color_alpha = sp_color_scales_get_color_alpha;

	object_class->destroy = sp_color_scales_destroy;

	widget_class->show_all = sp_color_scales_show_all;
	widget_class->hide_all = sp_color_scales_hide_all;
}

void sp_color_scales_init (SPColorScales *cs)
{
	GtkWidget *t;
	gint i;

	cs->updating = FALSE;
	cs->dragging = FALSE;

	t = gtk_table_new (5, 3, FALSE);
	gtk_widget_show (t);
	gtk_box_pack_start (GTK_BOX (cs), t, TRUE, TRUE, 0);

	/* Create components */
	for (i = 0; i < INK_STATIC_CAST(gint, G_N_ELEMENTS(cs->a)); i++) {
		/* Label */
		cs->l[i] = gtk_label_new ("");
		gtk_misc_set_alignment (GTK_MISC (cs->l[i]), 1.0, 0.5);
		gtk_widget_show (cs->l[i]);
		gtk_table_attach (GTK_TABLE (t), cs->l[i], 0, 1, i, i + 1, GTK_FILL, GTK_FILL, XPAD, YPAD);
		/* Adjustment */
		cs->a[i] = (GtkAdjustment *) gtk_adjustment_new (0.0, 0.0, 1.0, 0.01, 0.1, 0.1);
		/* Slider */
		cs->s[i] = sp_color_slider_new (cs->a[i]);
		gtk_widget_show (cs->s[i]);
		gtk_table_attach (GTK_TABLE (t), cs->s[i], 1, 2, i, i + 1, (GtkAttachOptions)(GTK_EXPAND | GTK_FILL), (GtkAttachOptions)GTK_FILL, XPAD, YPAD);

		/* Spinbutton */
		cs->b[i] = gtk_spin_button_new (GTK_ADJUSTMENT (cs->a[i]), 0.01, 2);
		sp_dialog_defocus_on_enter (cs->b[i]);
		gtk_widget_show (cs->b[i]);
		gtk_table_attach (GTK_TABLE (t), cs->b[i], 2, 3, i, i + 1, (GtkAttachOptions)0, (GtkAttachOptions)0, XPAD, YPAD);

		/* Attach channel value to adjustment */
		gtk_object_set_data (GTK_OBJECT (cs->a[i]), "channel", GINT_TO_POINTER (i));
		/* Signals */
		gtk_signal_connect (GTK_OBJECT (cs->a[i]), "value_changed",
					GTK_SIGNAL_FUNC (sp_color_scales_adjustment_any_changed), cs);
		gtk_signal_connect (GTK_OBJECT (cs->s[i]), "grabbed",
					GTK_SIGNAL_FUNC (sp_color_scales_slider_any_grabbed), cs);
		gtk_signal_connect (GTK_OBJECT (cs->s[i]), "released",
					GTK_SIGNAL_FUNC (sp_color_scales_slider_any_released), cs);
		gtk_signal_connect (GTK_OBJECT (cs->s[i]), "changed",
					GTK_SIGNAL_FUNC (sp_color_scales_slider_any_changed), cs);
	}

	/* Initial mode is none, so it works */
	sp_color_scales_set_mode (cs, SP_COLOR_SCALES_MODE_RGB);
}

static void
sp_color_scales_destroy (GtkObject *object)
{
	SPColorScales *cs;
	gint i;

	cs = SP_COLOR_SCALES (object);

	for (i = 0; i < 5; i++) {
		cs->l[i] = NULL;
		cs->a[i] = NULL;
		cs->s[i] = NULL;
		cs->b[i] = NULL;
	}

	if (((GtkObjectClass *) (parent_class))->destroy)
		(* ((GtkObjectClass *) (parent_class))->destroy) (object);
}

static void
sp_color_scales_show_all (GtkWidget *widget)
{
	gtk_widget_show (widget);
}

static void
sp_color_scales_hide_all (GtkWidget *widget)
{
	gtk_widget_hide (widget);
}

GtkWidget *
sp_color_scales_new (void)
{
	SPColorScales *csel;
	SPColor color;

	csel = (SPColorScales*)gtk_type_new (SP_TYPE_COLOR_SCALES);

	sp_color_set_rgb_rgba32 (&color, 0);
	sp_color_selector_set_color_alpha (SP_COLOR_SELECTOR(csel), &color, 1.0);

	return GTK_WIDGET (csel);
}

void
sp_color_scales_set_color_alpha (SPColorSelector *csel, const SPColor *color, gfloat alpha)
{
	SPColorScales* cs;

	g_return_if_fail (csel != NULL);
	g_return_if_fail (SP_IS_COLOR_SCALES (csel));
	g_return_if_fail (color != NULL);

	cs = SP_COLOR_SCALES(csel);

	if ( sp_color_is_close (color, &csel->color, 1e-4)
		&& (fabs ((csel->alpha) - (alpha)) < 1e-4) )
	{
		return;
	}

	switch (cs->mode) {
	case SP_COLOR_SCALES_MODE_RGB:
	case SP_COLOR_SCALES_MODE_HSV:
	case SP_COLOR_SCALES_MODE_CMYK:
		spcs_set_color (SP_COLOR_SCALES (csel), color, alpha);
		break;
	default:
		g_warning ("file %s: line %d: Illegal color selector mode %d", __FILE__, __LINE__, cs->mode);
		break;
	}
}

void
sp_color_scales_get_color_alpha (SPColorSelector *csel, SPColor *color, gfloat *alpha)
{
	SPColorScales *cs;
	gfloat c[5];

	g_return_if_fail (csel != NULL);
	g_return_if_fail (SP_IS_COLOR_SCALES (csel));
	g_return_if_fail (color != NULL);

	cs = SP_COLOR_SCALES (csel);

	switch (cs->mode) {
	case SP_COLOR_SCALES_MODE_RGB:
	case SP_COLOR_SCALES_MODE_HSV:
		sp_color_scales_get_rgba_floatv (csel, c);
		sp_color_set_rgb_float (color, c[0], c[1], c[2]);
		if (alpha) *alpha = c[3];
		break;
	case SP_COLOR_SCALES_MODE_CMYK:
		sp_color_scales_get_cmyka_floatv (csel, c);
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
spcs_set_color (SPColorScales *cs, const SPColor* color, gfloat alpha)
{
	SPColorSelector *csel;
	gfloat tmp[3];
	gfloat c[5] = {0.0, 0.0, 0.0, 0.0};

	switch (cs->mode) {
	case SP_COLOR_SCALES_MODE_RGB:
		sp_color_get_rgb_floatv (color, c);
		c[3] = alpha;
		c[4] = 0.0;
		break;
	case SP_COLOR_SCALES_MODE_HSV:
		sp_color_get_rgb_floatv (color, tmp);
		c[0] = cs->a[0]->value;
		sp_color_rgb_to_hsv_floatv (c, tmp[0], tmp[1], tmp[2]);
		c[3] = alpha;
		c[4] = 0.0;
		break;
	case SP_COLOR_SCALES_MODE_CMYK:
		sp_color_get_cmyk_floatv (color, c);
		c[4] = alpha;
		break;
	default:
		g_warning ("file %s: line %d: Illegal color selector mode %d", __FILE__, __LINE__, cs->mode);
		break;
	}

	cs->updating = TRUE;
	gtk_adjustment_set_value (cs->a[0], c[0]);
	gtk_adjustment_set_value (cs->a[1], c[1]);
	gtk_adjustment_set_value (cs->a[2], c[2]);
	gtk_adjustment_set_value (cs->a[3], c[3]);
	gtk_adjustment_set_value (cs->a[4], c[4]);
	sp_color_scales_update_sliders (cs, CSC_CHANNELS_ALL);
	cs->updating = FALSE;

	csel = SP_COLOR_SELECTOR (cs);
	sp_color_copy (&csel->color, color);
	csel->alpha = alpha;
	gtk_signal_emit (GTK_OBJECT (cs), cs_signals[CHANGED]);
}

void
sp_color_scales_get_rgba_floatv (SPColorSelector *csel, gfloat *rgba)
{
	SPColorScales *cs;

	g_return_if_fail (csel != NULL);
	g_return_if_fail (SP_IS_COLOR_SCALES (csel));
	g_return_if_fail (csel != NULL);

	cs = SP_COLOR_SCALES (csel);

	switch (cs->mode) {
	case SP_COLOR_SCALES_MODE_RGB:
		rgba[0] = cs->a[0]->value;
		rgba[1] = cs->a[1]->value;
		rgba[2] = cs->a[2]->value;
		rgba[3] = cs->a[3]->value;
		break;
	case SP_COLOR_SCALES_MODE_HSV:
		sp_color_hsv_to_rgb_floatv (rgba, cs->a[0]->value, cs->a[1]->value, cs->a[2]->value);
		rgba[3] = cs->a[3]->value;
		break;
	case SP_COLOR_SCALES_MODE_CMYK:
		sp_color_cmyk_to_rgb_floatv (rgba, cs->a[0]->value, cs->a[1]->value, cs->a[2]->value, cs->a[3]->value);
		rgba[3] = cs->a[4]->value;
		break;
	default:
		g_warning ("file %s: line %d: Illegal color selector mode", __FILE__, __LINE__);
		break;
	}
}

void
sp_color_scales_get_cmyka_floatv (SPColorSelector *csel, gfloat *cmyka)
{
	SPColorScales* cs;
	gfloat rgb[3];

	g_return_if_fail (csel != NULL);
	g_return_if_fail (SP_IS_COLOR_SCALES (csel));
	g_return_if_fail (cmyka != NULL);

	cs = SP_COLOR_SCALES (csel);

	switch (cs->mode) {
	case SP_COLOR_SCALES_MODE_RGB:
		sp_color_rgb_to_cmyk_floatv (cmyka, cs->a[0]->value, cs->a[1]->value, cs->a[2]->value);
		cmyka[4] = cs->a[3]->value;
		break;
	case SP_COLOR_SCALES_MODE_HSV:
		sp_color_hsv_to_rgb_floatv (rgb, cs->a[0]->value, cs->a[1]->value, cs->a[2]->value);
		sp_color_rgb_to_cmyk_floatv (cmyka, rgb[0], rgb[1], rgb[2]);
		cmyka[4] = cs->a[3]->value;
		break;
	case SP_COLOR_SCALES_MODE_CMYK:
		cmyka[0] = cs->a[0]->value;
		cmyka[1] = cs->a[1]->value;
		cmyka[2] = cs->a[2]->value;
		cmyka[3] = cs->a[3]->value;
		cmyka[4] = cs->a[4]->value;
		break;
	default:
		g_warning ("file %s: line %d: Illegal color selector mode", __FILE__, __LINE__);
		break;
	}
}

guint32
sp_color_scales_get_rgba32 (SPColorSelector *csel)
{
	gfloat c[4];
	guint32 rgba;

	g_return_val_if_fail (csel != NULL, 0L);
	g_return_val_if_fail (SP_IS_COLOR_SCALES (csel), 0L);

	sp_color_scales_get_rgba_floatv (csel, c);

	rgba = SP_RGBA32_F_COMPOSE (c[0], c[1], c[2], c[3]);

	return rgba;
}

void
sp_color_scales_set_mode (SPColorScales *cs, SPColorScalesMode mode)
{
	SPColorSelector *csel;
	gfloat rgba[4];
	gfloat c[4];

	csel = SP_COLOR_SELECTOR (cs);

	if (cs->mode == mode) return;

	if ((cs->mode == SP_COLOR_SCALES_MODE_RGB) ||
		(cs->mode == SP_COLOR_SCALES_MODE_HSV) ||
		(cs->mode == SP_COLOR_SCALES_MODE_CMYK)) {
		sp_color_scales_get_rgba_floatv (csel, rgba);
	} else {
		rgba[0] = rgba[1] = rgba[2] = rgba[3] = 1.0;
	}

	cs->mode = mode;

	switch (mode) {
	case SP_COLOR_SCALES_MODE_RGB:
		gtk_label_set_text (GTK_LABEL (cs->l[0]), _("Red:"));
		gtk_label_set_text (GTK_LABEL (cs->l[1]), _("Green:"));
		gtk_label_set_text (GTK_LABEL (cs->l[2]), _("Blue:"));
		gtk_label_set_text (GTK_LABEL (cs->l[3]), _("Alpha:"));
		sp_color_slider_set_map (SP_COLOR_SLIDER (cs->s[0]), NULL);
		gtk_widget_hide (cs->l[4]);
		gtk_widget_hide (cs->s[4]);
		gtk_widget_hide (cs->b[4]);
		cs->updating = TRUE;
		gtk_adjustment_set_value (cs->a[0], rgba[0]);
		gtk_adjustment_set_value (cs->a[1], rgba[1]);
		gtk_adjustment_set_value (cs->a[2], rgba[2]);
		gtk_adjustment_set_value (cs->a[3], rgba[3]);
		cs->updating = FALSE;
		sp_color_scales_update_sliders (cs, CSC_CHANNELS_ALL);
		gtk_signal_emit (GTK_OBJECT (csel), cs_signals[CHANGED]);
		break;
	case SP_COLOR_SCALES_MODE_HSV:
		gtk_label_set_text (GTK_LABEL (cs->l[0]), _("Hue:"));
		gtk_label_set_text (GTK_LABEL (cs->l[1]), _("Saturation:"));
		gtk_label_set_text (GTK_LABEL (cs->l[2]), _("Value:"));
		gtk_label_set_text (GTK_LABEL (cs->l[3]), _("Alpha:"));
		sp_color_slider_set_map (SP_COLOR_SLIDER (cs->s[0]), (guchar*)sp_color_scales_hue_map ());
		gtk_widget_hide (cs->l[4]);
		gtk_widget_hide (cs->s[4]);
		gtk_widget_hide (cs->b[4]);
		cs->updating = TRUE;
		c[0] = 0.0;
		sp_color_rgb_to_hsv_floatv (c, rgba[0], rgba[1], rgba[2]);
		gtk_adjustment_set_value (cs->a[0], c[0]);
		gtk_adjustment_set_value (cs->a[1], c[1]);
		gtk_adjustment_set_value (cs->a[2], c[2]);
		gtk_adjustment_set_value (cs->a[3], rgba[3]);
		cs->updating = FALSE;
		sp_color_scales_update_sliders (cs, CSC_CHANNELS_ALL);
		gtk_signal_emit (GTK_OBJECT (csel), cs_signals[CHANGED]);
		break;
	case SP_COLOR_SCALES_MODE_CMYK:
		gtk_label_set_text (GTK_LABEL (cs->l[0]), _("Cyan:"));
		gtk_label_set_text (GTK_LABEL (cs->l[1]), _("Magenta:"));
		gtk_label_set_text (GTK_LABEL (cs->l[2]), _("Yellow:"));
		gtk_label_set_text (GTK_LABEL (cs->l[3]), _("Black:"));
		gtk_label_set_text (GTK_LABEL (cs->l[4]), _("Alpha:"));
		sp_color_slider_set_map (SP_COLOR_SLIDER (cs->s[0]), NULL);
		gtk_widget_show (cs->l[4]);
		gtk_widget_show (cs->s[4]);
		gtk_widget_show (cs->b[4]);
		cs->updating = TRUE;
		sp_color_rgb_to_cmyk_floatv (c, rgba[0], rgba[1], rgba[2]);
		gtk_adjustment_set_value (cs->a[0], c[0]);
		gtk_adjustment_set_value (cs->a[1], c[1]);
		gtk_adjustment_set_value (cs->a[2], c[2]);
		gtk_adjustment_set_value (cs->a[3], c[3]);
		gtk_adjustment_set_value (cs->a[4], rgba[3]);
		cs->updating = FALSE;
		sp_color_scales_update_sliders (cs, CSC_CHANNELS_ALL);
		gtk_signal_emit (GTK_OBJECT (csel), cs_signals[CHANGED]);
		break;
	default:
		g_warning ("file %s: line %d: Illegal color selector mode", __FILE__, __LINE__);
		break;
	}
}

SPColorScalesMode
sp_color_scales_get_mode (SPColorScales *cs)
{
	g_return_val_if_fail (cs != NULL, SP_COLOR_SCALES_MODE_NONE);
	g_return_val_if_fail (SP_IS_COLOR_SCALES (cs), SP_COLOR_SCALES_MODE_NONE);

	return cs->mode;
}

static void
sp_color_scales_set_submode (SPColorSelector *csel, guint submode)
{
	SPColorScales* cs;
	g_return_if_fail (csel != NULL);
	g_return_if_fail (SP_IS_COLOR_SCALES (csel));
	g_return_if_fail (submode < 3);
	cs = SP_COLOR_SCALES(csel);

	switch ( submode )
	{
	default:
	case 0:
		sp_color_scales_set_mode (cs, SP_COLOR_SCALES_MODE_RGB);
		break;
	case 1:
		sp_color_scales_set_mode (cs, SP_COLOR_SCALES_MODE_HSV);
		break;
	case 2:
		sp_color_scales_set_mode (cs, SP_COLOR_SCALES_MODE_CMYK);
		break;
	}
}

static guint
sp_color_scales_get_submode (SPColorSelector *csel)
{
	guint submode = 0;
	SPColorScales* cs;
	g_return_val_if_fail (csel != NULL, 0);
	g_return_val_if_fail (SP_IS_COLOR_SCALES (csel), 0);
	cs = SP_COLOR_SCALES(csel);

	switch ( cs->mode )
	{
	case SP_COLOR_SCALES_MODE_HSV:
		submode = 1;
		break;
	case SP_COLOR_SCALES_MODE_CMYK:
		submode = 2;
		break;
	case SP_COLOR_SCALES_MODE_RGB:
	default:
		submode = 0;
	}

	return submode;
}

static void
sp_color_scales_adjustment_any_changed (GtkAdjustment *adjustment, SPColorScales *cs)
{
	gint channel;

	channel = GPOINTER_TO_INT (gtk_object_get_data (GTK_OBJECT (adjustment), "channel"));

	sp_color_scales_adjustment_changed (cs, channel);
}

static void
sp_color_scales_slider_any_grabbed (SPColorSlider *slider, SPColorScales *cs)
{
	if (!cs->dragging) {
		cs->dragging = TRUE;
		gtk_signal_emit (GTK_OBJECT (cs), cs_signals[GRABBED]);
	}
}

static void
sp_color_scales_slider_any_released (SPColorSlider *slider, SPColorScales *cs)
{
	if (cs->dragging) {
		cs->dragging = FALSE;
		gtk_signal_emit (GTK_OBJECT (cs), cs_signals[RELEASED]);
	}
}

static void
sp_color_scales_slider_any_changed (SPColorSlider *slider, SPColorScales *cs)
{
	SPColorSelector *csel;

	csel = SP_COLOR_SELECTOR (cs);
	sp_color_scales_get_color_alpha (csel, &csel->color, &csel->alpha);

	gtk_signal_emit (GTK_OBJECT (cs), cs_signals[CHANGED]);
}

static void
sp_color_scales_adjustment_changed (SPColorScales *cs, guint channel)
{
	if (cs->updating) return;

	cs->updating = TRUE;

	sp_color_scales_update_sliders (cs, (1 << channel));

	if (cs->dragging) {
		gtk_signal_emit (GTK_OBJECT (cs), cs_signals[DRAGGED]);
	} else {
		gtk_signal_emit (GTK_OBJECT (cs), cs_signals[CHANGED]);
	}

	cs->updating = FALSE;
}

static void
sp_color_scales_update_sliders (SPColorScales *cs, guint channels)
{
	SPColorSelector *csel;
	gfloat rgb0[3], rgb1[3];
#ifdef SPCS_PREVIEW
	guint32 rgba;
#endif
	switch (cs->mode) {
	case SP_COLOR_SCALES_MODE_RGB:
		if ((channels != CSC_CHANNEL_R) && (channels != CSC_CHANNEL_A)) {
			/* Update red */
			sp_color_slider_set_colors (SP_COLOR_SLIDER (cs->s[0]),
							SP_RGBA32_F_COMPOSE (0.0, cs->a[1]->value, cs->a[2]->value, 1.0),
							SP_RGBA32_F_COMPOSE (1.0, cs->a[1]->value, cs->a[2]->value, 1.0));
		}
		if ((channels != CSC_CHANNEL_G) && (channels != CSC_CHANNEL_A)) {
			/* Update green */
			sp_color_slider_set_colors (SP_COLOR_SLIDER (cs->s[1]),
							SP_RGBA32_F_COMPOSE (cs->a[0]->value, 0.0, cs->a[2]->value, 1.0),
							SP_RGBA32_F_COMPOSE (cs->a[0]->value, 1.0, cs->a[2]->value, 1.0));
		}
		if ((channels != CSC_CHANNEL_B) && (channels != CSC_CHANNEL_A)) {
			/* Update blue */
			sp_color_slider_set_colors (SP_COLOR_SLIDER (cs->s[2]),
							SP_RGBA32_F_COMPOSE (cs->a[0]->value, cs->a[1]->value, 0.0, 1.0),
							SP_RGBA32_F_COMPOSE (cs->a[0]->value, cs->a[1]->value, 1.0, 1.0));
		}
		if (channels != CSC_CHANNEL_A) {
			/* Update alpha */
			sp_color_slider_set_colors (SP_COLOR_SLIDER (cs->s[3]),
							SP_RGBA32_F_COMPOSE (cs->a[0]->value, cs->a[1]->value, cs->a[2]->value, 0.0),
							SP_RGBA32_F_COMPOSE (cs->a[0]->value, cs->a[1]->value, cs->a[2]->value, 1.0));
		}
		break;
	case SP_COLOR_SCALES_MODE_HSV:
		/* Hue is never updated */
		if ((channels != CSC_CHANNEL_S) && (channels != CSC_CHANNEL_A)) {
			/* Update saturation */
			sp_color_hsv_to_rgb_floatv (rgb0, cs->a[0]->value, 0.0, cs->a[2]->value);
			sp_color_hsv_to_rgb_floatv (rgb1, cs->a[0]->value, 1.0, cs->a[2]->value);
			sp_color_slider_set_colors (SP_COLOR_SLIDER (cs->s[1]),
							SP_RGBA32_F_COMPOSE (rgb0[0], rgb0[1], rgb0[2], 1.0),
							SP_RGBA32_F_COMPOSE (rgb1[0], rgb1[1], rgb1[2], 1.0));
		}
		if ((channels != CSC_CHANNEL_V) && (channels != CSC_CHANNEL_A)) {
			/* Update value */
			sp_color_hsv_to_rgb_floatv (rgb0, cs->a[0]->value, cs->a[1]->value, 0.0);
			sp_color_hsv_to_rgb_floatv (rgb1, cs->a[0]->value, cs->a[1]->value, 1.0);
			sp_color_slider_set_colors (SP_COLOR_SLIDER (cs->s[2]),
							SP_RGBA32_F_COMPOSE (rgb0[0], rgb0[1], rgb0[2], 1.0),
							SP_RGBA32_F_COMPOSE (rgb1[0], rgb1[1], rgb1[2], 1.0));
		}
		if (channels != CSC_CHANNEL_A) {
			/* Update alpha */
			sp_color_hsv_to_rgb_floatv (rgb0, cs->a[0]->value, cs->a[1]->value, cs->a[2]->value);
			sp_color_slider_set_colors (SP_COLOR_SLIDER (cs->s[3]),
							SP_RGBA32_F_COMPOSE (rgb0[0], rgb0[1], rgb0[2], 0.0),
							SP_RGBA32_F_COMPOSE (rgb0[0], rgb0[1], rgb0[2], 1.0));
		}
		break;
	case SP_COLOR_SCALES_MODE_CMYK:
		if ((channels != CSC_CHANNEL_C) && (channels != CSC_CHANNEL_CMYKA)) {
			/* Update saturation */
			sp_color_cmyk_to_rgb_floatv (rgb0, 0.0, cs->a[1]->value, cs->a[2]->value, cs->a[3]->value);
			sp_color_cmyk_to_rgb_floatv (rgb1, 1.0, cs->a[1]->value, cs->a[2]->value, cs->a[3]->value);
			sp_color_slider_set_colors (SP_COLOR_SLIDER (cs->s[0]),
							SP_RGBA32_F_COMPOSE (rgb0[0], rgb0[1], rgb0[2], 1.0),
							SP_RGBA32_F_COMPOSE (rgb1[0], rgb1[1], rgb1[2], 1.0));
		}
		if ((channels != CSC_CHANNEL_M) && (channels != CSC_CHANNEL_CMYKA)) {
			/* Update saturation */
			sp_color_cmyk_to_rgb_floatv (rgb0, cs->a[0]->value, 0.0, cs->a[2]->value, cs->a[3]->value);
			sp_color_cmyk_to_rgb_floatv (rgb1, cs->a[0]->value, 1.0, cs->a[2]->value, cs->a[3]->value);
			sp_color_slider_set_colors (SP_COLOR_SLIDER (cs->s[1]),
							SP_RGBA32_F_COMPOSE (rgb0[0], rgb0[1], rgb0[2], 1.0),
							SP_RGBA32_F_COMPOSE (rgb1[0], rgb1[1], rgb1[2], 1.0));
		}
		if ((channels != CSC_CHANNEL_Y) && (channels != CSC_CHANNEL_CMYKA)) {
			/* Update saturation */
			sp_color_cmyk_to_rgb_floatv (rgb0, cs->a[0]->value, cs->a[1]->value, 0.0, cs->a[3]->value);
			sp_color_cmyk_to_rgb_floatv (rgb1, cs->a[0]->value, cs->a[1]->value, 1.0, cs->a[3]->value);
			sp_color_slider_set_colors (SP_COLOR_SLIDER (cs->s[2]),
							SP_RGBA32_F_COMPOSE (rgb0[0], rgb0[1], rgb0[2], 1.0),
							SP_RGBA32_F_COMPOSE (rgb1[0], rgb1[1], rgb1[2], 1.0));
		}
		if ((channels != CSC_CHANNEL_K) && (channels != CSC_CHANNEL_CMYKA)) {
			/* Update saturation */
			sp_color_cmyk_to_rgb_floatv (rgb0, cs->a[0]->value, cs->a[1]->value, cs->a[2]->value, 0.0);
			sp_color_cmyk_to_rgb_floatv (rgb1, cs->a[0]->value, cs->a[1]->value, cs->a[2]->value, 1.0);
			sp_color_slider_set_colors (SP_COLOR_SLIDER (cs->s[3]),
							SP_RGBA32_F_COMPOSE (rgb0[0], rgb0[1], rgb0[2], 1.0),
							SP_RGBA32_F_COMPOSE (rgb1[0], rgb1[1], rgb1[2], 1.0));
		}
		if (channels != CSC_CHANNEL_CMYKA) {
			/* Update saturation */
			sp_color_cmyk_to_rgb_floatv (rgb0, cs->a[0]->value, cs->a[1]->value, cs->a[2]->value, cs->a[3]->value);
			sp_color_slider_set_colors (SP_COLOR_SLIDER (cs->s[4]),
							SP_RGBA32_F_COMPOSE (rgb0[0], rgb0[1], rgb0[2], 0.0),
							SP_RGBA32_F_COMPOSE (rgb0[0], rgb0[1], rgb0[2], 1.0));
		}
		break;
	default:
		g_warning ("file %s: line %d: Illegal color selector mode", __FILE__, __LINE__);
		break;
	}

	// Force the internal color to be updated
	csel = SP_COLOR_SELECTOR (cs);
	sp_color_selector_get_color_alpha (csel, &csel->color, &csel->alpha);

#ifdef SPCS_PREVIEW
	rgba = sp_color_scales_get_rgba32 (cs);
	sp_color_preview_set_rgba32 (SP_COLOR_PREVIEW (cs->p), rgba);
#endif
}

static const gchar *
sp_color_scales_hue_map (void)
{
	static gchar *map = NULL;

	if (!map) {
		gchar *p;
		gint h;
		map = g_new (gchar, 4 * 1024);
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

