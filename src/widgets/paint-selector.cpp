#define __SP_PAINT_SELECTOR_C__

/** \file
 * SPPaintSelector: Generic paint selector widget.
 */

/*
 * Copyright (C) Lauris 2002
 */

#define noSP_PS_VERBOSE

#include <config.h>

#include <math.h>
#include <string.h>
#include <stdlib.h>

#include <libnr/nr-values.h>
#include <libnr/nr-matrix.h>
#include <libnr/nr-matrix-div.h>
#include <libnr/nr-matrix-fns.h>
#include <libnr/nr-matrix-ops.h>

#include <gtk/gtksignal.h>
#include <gtk/gtkhbox.h>
#include <gtk/gtkradiobutton.h>
#include <gtk/gtkhseparator.h>
#include <gtk/gtkframe.h>
#include <gtk/gtklabel.h>
#include <gtk/gtkoptionmenu.h>
#include "menu.h"
#include <gtk/gtkmenuitem.h>
#include <gtk/gtkimage.h>

#include "../sp-item.h"
#include "../sp-pattern.h"
#include "../helper/sp-intl.h"
#include "../widgets/icon.h"
#include "../inkscape-stock.h"
#include "widgets/widget-sizes.h"
#include "xml/repr.h"

#include "sp-color-selector.h"
#include "sp-color-notebook.h"
#include "sp-linear-gradient-fns.h"
#include "sp-radial-gradient-fns.h"
/* fixme: Move it from dialogs to here */
#include "gradient-selector.h"
#include <inkscape.h>
#include <document-private.h>
#include <desktop-handles.h>
#include <selection.h>
#include <style.h>

#include "paint-selector.h"


enum {
	MODE_CHANGED,
	GRABBED,
	DRAGGED,
	RELEASED,
	CHANGED,
	LAST_SIGNAL
};

static void sp_paint_selector_class_init (SPPaintSelectorClass *klass);
static void sp_paint_selector_init (SPPaintSelector *slider);
static void sp_paint_selector_destroy (GtkObject *object);

static GtkWidget *sp_paint_selector_style_button_add (SPPaintSelector *psel, const gchar *px, SPPaintSelectorMode mode, GtkRadioButton *last, GtkTooltips *tt, const gchar *tip);
static void sp_paint_selector_style_button_toggled (GtkToggleButton *tb, SPPaintSelector *psel);

static void sp_paint_selector_set_mode_empty (SPPaintSelector *psel);
static void sp_paint_selector_set_mode_multiple (SPPaintSelector *psel);
static void sp_paint_selector_set_mode_clone (SPPaintSelector *psel);
static void sp_paint_selector_set_mode_none (SPPaintSelector *psel);
static void sp_paint_selector_set_mode_color (SPPaintSelector *psel, SPPaintSelectorMode mode);
static void sp_paint_selector_set_mode_gradient (SPPaintSelector *psel, SPPaintSelectorMode mode);
static void sp_paint_selector_set_mode_pattern (SPPaintSelector *psel, SPPaintSelectorMode mode);


static void sp_paint_selector_set_style_buttons (SPPaintSelector *psel, GtkWidget *active);

static GtkVBoxClass *parent_class;
static guint psel_signals[LAST_SIGNAL] = {0};

GtkType
sp_paint_selector_get_type (void)
{
	static GtkType type = 0;
	if (!type) {
		GtkTypeInfo info = {
			"SPPaintSelector",
			sizeof (SPPaintSelector),
			sizeof (SPPaintSelectorClass),
			(GtkClassInitFunc) sp_paint_selector_class_init,
			(GtkObjectInitFunc) sp_paint_selector_init,
			NULL, NULL, NULL
		};
		type = gtk_type_unique (GTK_TYPE_VBOX, &info);
	}
	return type;
}

static void
sp_paint_selector_class_init (SPPaintSelectorClass *klass)
{
	GtkObjectClass *object_class;
	GtkWidgetClass *widget_class;

	object_class = (GtkObjectClass *) klass;
	widget_class = (GtkWidgetClass *) klass;

	parent_class = (GtkVBoxClass*)gtk_type_class (GTK_TYPE_VBOX);

	psel_signals[MODE_CHANGED] = gtk_signal_new ("mode_changed",
						 (GtkSignalRunType)(GTK_RUN_FIRST | GTK_RUN_NO_RECURSE),
						 GTK_CLASS_TYPE(object_class),
						 GTK_SIGNAL_OFFSET (SPPaintSelectorClass, mode_changed),
						 gtk_marshal_NONE__UINT,
						 GTK_TYPE_NONE, 1, GTK_TYPE_UINT);
	psel_signals[GRABBED] =  gtk_signal_new ("grabbed",
						 (GtkSignalRunType)(GTK_RUN_FIRST | GTK_RUN_NO_RECURSE),
						 GTK_CLASS_TYPE(object_class),
						 GTK_SIGNAL_OFFSET (SPPaintSelectorClass, grabbed),
						 gtk_marshal_NONE__NONE,
						 GTK_TYPE_NONE, 0);
	psel_signals[DRAGGED] =  gtk_signal_new ("dragged",
						 (GtkSignalRunType)(GTK_RUN_FIRST | GTK_RUN_NO_RECURSE),
						 GTK_CLASS_TYPE(object_class),
						 GTK_SIGNAL_OFFSET (SPPaintSelectorClass, dragged),
						 gtk_marshal_NONE__NONE,
						 GTK_TYPE_NONE, 0);
	psel_signals[RELEASED] = gtk_signal_new ("released",
						 (GtkSignalRunType)(GTK_RUN_FIRST | GTK_RUN_NO_RECURSE),
						 GTK_CLASS_TYPE(object_class),
						 GTK_SIGNAL_OFFSET (SPPaintSelectorClass, released),
						 gtk_marshal_NONE__NONE,
						 GTK_TYPE_NONE, 0);
	psel_signals[CHANGED] =  gtk_signal_new ("changed",
						 (GtkSignalRunType)(GTK_RUN_FIRST | GTK_RUN_NO_RECURSE),
						 GTK_CLASS_TYPE(object_class),
						 GTK_SIGNAL_OFFSET (SPPaintSelectorClass, changed),
						 gtk_marshal_NONE__NONE,
						 GTK_TYPE_NONE, 0);

	object_class->destroy = sp_paint_selector_destroy;
}

#define XPAD 4
#define YPAD 1

static void
sp_paint_selector_init (SPPaintSelector *psel)
{
	GtkTooltips *tt = gtk_tooltips_new();

	psel->mode = (SPPaintSelectorMode)-1; // huh?  do you mean 0xff?  --  I think this means "not in the enum"

	/* Paint style button box */
	psel->style = gtk_hbox_new (FALSE, 0);
	gtk_widget_show (psel->style);
	gtk_container_set_border_width (GTK_CONTAINER (psel->style), 4);
	gtk_box_pack_start (GTK_BOX (psel), psel->style, FALSE, FALSE, 0);

	/* Buttons */
	psel->none = sp_paint_selector_style_button_add (psel, INKSCAPE_STOCK_FILL_NONE,
							 SP_PAINT_SELECTOR_MODE_NONE, NULL, tt, _("No paint"));
	psel->solid = sp_paint_selector_style_button_add (psel, INKSCAPE_STOCK_FILL_SOLID,
							  SP_PAINT_SELECTOR_MODE_COLOR_RGB, GTK_RADIO_BUTTON (psel->none), tt, _("Flat color"));
	psel->gradient = sp_paint_selector_style_button_add (psel, INKSCAPE_STOCK_FILL_GRADIENT,
						     SP_PAINT_SELECTOR_MODE_GRADIENT_LINEAR, GTK_RADIO_BUTTON (psel->solid), tt, _("Linear gradient"));
	psel->radial = sp_paint_selector_style_button_add (psel, INKSCAPE_STOCK_FILL_RADIAL,
							 SP_PAINT_SELECTOR_MODE_GRADIENT_RADIAL, GTK_RADIO_BUTTON (psel->gradient), tt, _("Radial gradient")),
	psel->pattern = sp_paint_selector_style_button_add (psel, INKSCAPE_STOCK_FILL_PATTERN,
                               SP_PAINT_SELECTOR_MODE_PATTERN, GTK_RADIO_BUTTON (psel->radial), tt, _("Pattern"));

	/* Frame */
	psel->frame = gtk_frame_new ("");
	gtk_widget_show (psel->frame);
	gtk_container_set_border_width (GTK_CONTAINER (psel->frame), 0);
	gtk_box_pack_start (GTK_BOX (psel), psel->frame, TRUE, TRUE, 0);

	/* Last used color */
	sp_color_set_rgb_float (&psel->color, 0.0, 0.0, 0.0);
	psel->alpha = 1.0;
}

static void
sp_paint_selector_destroy (GtkObject *object)
{
	SPPaintSelector *psel = SP_PAINT_SELECTOR (object);

	// clean up our long-living pattern menu
	g_object_set_data (G_OBJECT(psel),"patternmenu",NULL);

	if (((GtkObjectClass *) (parent_class))->destroy)
		(* ((GtkObjectClass *) (parent_class))->destroy) (object);
}

static GtkWidget *
sp_paint_selector_style_button_add (SPPaintSelector *psel, const gchar *pixmap, SPPaintSelectorMode mode, GtkRadioButton *last, GtkTooltips *tt, const gchar *tip)
{
	GtkWidget *b, *w;

	b = gtk_radio_button_new ((last) ? gtk_radio_button_group (last) : NULL);
	gtk_tooltips_set_tip (tt, b, tip, NULL);
	gtk_widget_show (b);

	gtk_container_set_border_width (GTK_CONTAINER (b), 0);

	gtk_toggle_button_set_mode (GTK_TOGGLE_BUTTON (b), FALSE);
	gtk_object_set_data (GTK_OBJECT (b), "mode", GUINT_TO_POINTER (mode));

	w = sp_icon_new (16, pixmap);
	gtk_widget_show (w);
	gtk_container_add (GTK_CONTAINER (b), w);

	gtk_box_pack_start (GTK_BOX (psel->style), b, FALSE, FALSE, 0);
	gtk_signal_connect (GTK_OBJECT (b), "toggled", GTK_SIGNAL_FUNC (sp_paint_selector_style_button_toggled), psel);

	return b;
}

static void
sp_paint_selector_style_button_toggled (GtkToggleButton *tb, SPPaintSelector *psel)
{
	if (!psel->update && gtk_toggle_button_get_active (tb)) {
		sp_paint_selector_set_mode (psel, (SPPaintSelectorMode)GPOINTER_TO_UINT (gtk_object_get_data (GTK_OBJECT (tb), "mode")));
	}
}

GtkWidget *
sp_paint_selector_new (void)
{
	SPPaintSelector *psel;

	psel = (SPPaintSelector*)gtk_type_new (SP_TYPE_PAINT_SELECTOR);

	sp_paint_selector_set_mode (psel, SP_PAINT_SELECTOR_MODE_MULTIPLE);

	return GTK_WIDGET (psel);
}

void
sp_paint_selector_set_mode (SPPaintSelector *psel, SPPaintSelectorMode mode)
{
	if (psel->mode != mode) {
		psel->update = TRUE;
#ifdef SP_PS_VERBOSE
		g_print ("Mode change %d -> %d\n", psel->mode, mode);
#endif
		switch (mode) {
		case SP_PAINT_SELECTOR_MODE_EMPTY:
			sp_paint_selector_set_mode_empty (psel);
			break;
		case SP_PAINT_SELECTOR_MODE_MULTIPLE:
			sp_paint_selector_set_mode_multiple (psel);
			break;
		case SP_PAINT_SELECTOR_MODE_NONE:
			sp_paint_selector_set_mode_none (psel);
			break;
		case SP_PAINT_SELECTOR_MODE_COLOR_RGB:
		case SP_PAINT_SELECTOR_MODE_COLOR_CMYK:
			sp_paint_selector_set_mode_color (psel, mode);
			break;
		case SP_PAINT_SELECTOR_MODE_GRADIENT_LINEAR:
		case SP_PAINT_SELECTOR_MODE_GRADIENT_RADIAL:
			sp_paint_selector_set_mode_gradient (psel, mode);
			break;
		case SP_PAINT_SELECTOR_MODE_PATTERN:
			sp_paint_selector_set_mode_pattern (psel, mode);
			break;
		case SP_PAINT_SELECTOR_MODE_CLONE:
			sp_paint_selector_set_mode_clone (psel);
			break;
		default:
			g_warning ("file %s: line %d: Unknown paint mode %d", __FILE__, __LINE__, mode);
			break;
		}
		psel->mode = mode;
		gtk_signal_emit (GTK_OBJECT (psel), psel_signals[MODE_CHANGED], psel->mode);
		psel->update = FALSE;
	}
}

void
sp_paint_selector_set_color_alpha (SPPaintSelector *psel, const SPColor *color, float alpha)
{
	g_return_if_fail( ( 0.0 <= alpha ) && ( alpha <= 1.0 ) );
	SPColorSelector *csel;
	guint32 rgba;

	if ( sp_color_get_colorspace_type (color) == SP_COLORSPACE_TYPE_CMYK )
	{
#ifdef SP_PS_VERBOSE
		g_print ("PaintSelector set CMYKA\n");
#endif
		sp_paint_selector_set_mode (psel, SP_PAINT_SELECTOR_MODE_COLOR_CMYK);
	}
	else
	{
#ifdef SP_PS_VERBOSE
		g_print ("PaintSelector set RGBA\n");
#endif
		sp_paint_selector_set_mode (psel, SP_PAINT_SELECTOR_MODE_COLOR_RGB);
	}

	csel = (SPColorSelector*)gtk_object_get_data (GTK_OBJECT (psel->selector), "color-selector");
	rgba = sp_color_get_rgba32_falpha( &*color, alpha );
	csel->base->setColorAlpha( *color, alpha );
}

void
sp_paint_selector_set_gradient_linear (SPPaintSelector *psel, SPGradient *vector)
{
	SPGradientSelector *gsel;
#ifdef SP_PS_VERBOSE
	g_print ("PaintSelector set GRADIENT LINEAR\n");
#endif
	sp_paint_selector_set_mode (psel, SP_PAINT_SELECTOR_MODE_GRADIENT_LINEAR);

	gsel = (SPGradientSelector*)gtk_object_get_data (GTK_OBJECT (psel->selector), "gradient-selector");

	sp_gradient_selector_set_mode (gsel, SP_GRADIENT_SELECTOR_MODE_LINEAR);
	sp_gradient_selector_set_vector (gsel, (vector) ? SP_OBJECT_DOCUMENT (vector) : NULL, vector);
}

void
sp_paint_selector_set_lgradient_position (SPPaintSelector *psel, gdouble x0, gdouble y0, gdouble x1, gdouble y1)
{
	SPGradientSelector *gsel;

	g_return_if_fail (psel != NULL);
	g_return_if_fail (SP_IS_PAINT_SELECTOR (psel));
	g_return_if_fail (psel->mode == SP_PAINT_SELECTOR_MODE_GRADIENT_LINEAR);

	gsel = (SPGradientSelector*)gtk_object_get_data (GTK_OBJECT (psel->selector), "gradient-selector");

	sp_gradient_selector_set_lgradient_position (gsel, x0, y0, x1, y1);
}

void
sp_paint_selector_set_gradient_radial (SPPaintSelector *psel, SPGradient *vector)
{
	SPGradientSelector *gsel;
#ifdef SP_PS_VERBOSE
	g_print ("PaintSelector set GRADIENT RADIAL\n");
#endif
	sp_paint_selector_set_mode (psel, SP_PAINT_SELECTOR_MODE_GRADIENT_RADIAL);

	gsel = (SPGradientSelector*)gtk_object_get_data (GTK_OBJECT (psel->selector), "gradient-selector");

	sp_gradient_selector_set_mode (gsel, SP_GRADIENT_SELECTOR_MODE_RADIAL);
	sp_gradient_selector_set_vector (gsel, (vector) ? SP_OBJECT_DOCUMENT (vector) : NULL, vector);
}

void
sp_paint_selector_set_rgradient_position (SPPaintSelector *psel, gdouble cx, gdouble cy, gdouble fx, gdouble fy, gdouble r)
{
	SPGradientSelector *gsel;

	g_return_if_fail (psel != NULL);
	g_return_if_fail (SP_IS_PAINT_SELECTOR (psel));
	g_return_if_fail (psel->mode == SP_PAINT_SELECTOR_MODE_GRADIENT_RADIAL);

	gsel = (SPGradientSelector*)gtk_object_get_data (GTK_OBJECT (psel->selector), "gradient-selector");

	sp_gradient_selector_set_rgradient_position (gsel, cx, cy, fx, fy, r);
}

void
sp_paint_selector_set_gradient_bbox (SPPaintSelector *psel, gdouble x0, gdouble y0, gdouble x1, gdouble y1)
{
	SPGradientSelector *gsel;

	g_return_if_fail (psel != NULL);
	g_return_if_fail (SP_IS_PAINT_SELECTOR (psel));
	g_return_if_fail ((psel->mode == SP_PAINT_SELECTOR_MODE_GRADIENT_LINEAR) ||
			  (psel->mode == SP_PAINT_SELECTOR_MODE_GRADIENT_RADIAL));

	gsel = (SPGradientSelector*)gtk_object_get_data (GTK_OBJECT (psel->selector), "gradient-selector");

	sp_gradient_selector_set_bbox (gsel, x0, y0, x1, y1);
}

void
sp_paint_selector_set_gradient_gs2d_matrix_f(SPPaintSelector *psel, NRMatrix const *gs2d)
{
	sp_paint_selector_set_gradient_gs2d_matrix(psel, *gs2d);
}

void
sp_paint_selector_set_gradient_gs2d_matrix(SPPaintSelector *psel, NR::Matrix const &gs2d)
{
	g_return_if_fail (psel != NULL);
	g_return_if_fail (SP_IS_PAINT_SELECTOR (psel));
	g_return_if_fail ((psel->mode == SP_PAINT_SELECTOR_MODE_GRADIENT_LINEAR) ||
			  (psel->mode == SP_PAINT_SELECTOR_MODE_GRADIENT_RADIAL));

	SPGradientSelector *gsel = (SPGradientSelector*) gtk_object_get_data(GTK_OBJECT(psel->selector),
									     "gradient-selector");

	sp_gradient_selector_set_gs2d_matrix_f (gsel, gs2d);
}

void
sp_paint_selector_get_gradient_gs2d_matrix_f(SPPaintSelector const *psel, NRMatrix *gs2d)
{
	SPGradientSelector *gsel;

	g_return_if_fail (psel != NULL);
	g_return_if_fail (SP_IS_PAINT_SELECTOR (psel));
	g_return_if_fail ((psel->mode == SP_PAINT_SELECTOR_MODE_GRADIENT_LINEAR) ||
			  (psel->mode == SP_PAINT_SELECTOR_MODE_GRADIENT_RADIAL));

	gsel = (SPGradientSelector*)gtk_object_get_data (GTK_OBJECT (psel->selector), "gradient-selector");

	sp_gradient_selector_get_gs2d_matrix_f (gsel, gs2d);
}

void
sp_paint_selector_set_gradient_properties (SPPaintSelector *psel, SPGradientUnits units, SPGradientSpread spread)
{
	SPGradientSelector *gsel;
	g_return_if_fail (SP_IS_PAINT_SELECTOR (psel));
	g_return_if_fail ((psel->mode == SP_PAINT_SELECTOR_MODE_GRADIENT_LINEAR) ||
			  (psel->mode == SP_PAINT_SELECTOR_MODE_GRADIENT_RADIAL));
	gsel = (SPGradientSelector*)gtk_object_get_data (GTK_OBJECT (psel->selector), "gradient-selector");
	sp_gradient_selector_set_units (gsel, units);
	sp_gradient_selector_set_spread (gsel, spread);
}

void
sp_paint_selector_get_gradient_properties (SPPaintSelector *psel, SPGradientUnits *units, SPGradientSpread *spread)
{
	SPGradientSelector *gsel;
	g_return_if_fail (SP_IS_PAINT_SELECTOR (psel));
	g_return_if_fail ((psel->mode == SP_PAINT_SELECTOR_MODE_GRADIENT_LINEAR) ||
			  (psel->mode == SP_PAINT_SELECTOR_MODE_GRADIENT_RADIAL));
	gsel = (SPGradientSelector*)gtk_object_get_data (GTK_OBJECT (psel->selector), "gradient-selector");
	if (units) *units = (SPGradientUnits)gsel->gradientUnits;
	if (spread) *spread = (SPGradientSpread)gsel->gradientSpread;
}

/**
 * \post (alpha == NULL) || (*alpha in [0.0, 1.0]).
 */
void
sp_paint_selector_get_color_alpha (SPPaintSelector *psel, SPColor *color, gfloat *alpha)
{
	SPColorSelector *csel;

	csel = (SPColorSelector*)gtk_object_get_data (GTK_OBJECT (psel->selector), "color-selector");

	csel->base->getColorAlpha( *color, alpha );

	g_assert( !alpha
		  || ( ( 0.0 <= *alpha )
		       && ( *alpha <= 1.0 ) ) );
}

SPGradient *
sp_paint_selector_get_gradient_vector (SPPaintSelector *psel)
{
	SPGradientSelector *gsel;

	g_return_val_if_fail ((psel->mode == SP_PAINT_SELECTOR_MODE_GRADIENT_LINEAR) ||
			      (psel->mode == SP_PAINT_SELECTOR_MODE_GRADIENT_RADIAL), NULL);

	gsel = (SPGradientSelector*)gtk_object_get_data (GTK_OBJECT (psel->selector), "gradient-selector");

	return sp_gradient_selector_get_vector (gsel);
}

void
sp_paint_selector_get_gradient_position_floatv (SPPaintSelector *psel, gfloat *pos)
{
	SPGradientSelector *gsel;

	g_return_if_fail ((psel->mode == SP_PAINT_SELECTOR_MODE_GRADIENT_LINEAR) ||
			  (psel->mode == SP_PAINT_SELECTOR_MODE_GRADIENT_RADIAL));

	gsel = (SPGradientSelector*)gtk_object_get_data (GTK_OBJECT (psel->selector), "gradient-selector");

	if (psel->mode == SP_PAINT_SELECTOR_MODE_GRADIENT_LINEAR) {
		sp_gradient_selector_get_lgradient_position_floatv (gsel, pos);
	} else {
		sp_gradient_selector_get_rgradient_position_floatv (gsel, pos);
	}
}

void
sp_paint_selector_write_lineargradient (SPPaintSelector *psel, SPLinearGradient *lg, SPItem *item)
{
	g_return_if_fail (psel->mode == SP_PAINT_SELECTOR_MODE_GRADIENT_LINEAR);

	/* Units have to be set before position calculations */
	SPGradientUnits units;
	SPGradientSpread spread;
	sp_paint_selector_get_gradient_properties (psel, &units, &spread);
	sp_gradient_set_units (SP_GRADIENT (lg), units);
	sp_gradient_set_spread (SP_GRADIENT (lg), spread);

	/* Get positions from the selector*/
	gfloat p[4];
	sp_paint_selector_get_gradient_position_floatv (psel, p);
	NR::Point p1(p[0], p[1]);
	NR::Point p2(p[2], p[3]);

	/* Calculate raw gradient transform */
	NRRect fbb_;
	sp_item_invoke_bbox(item, &fbb_, NR::identity(), TRUE);
	NR::Rect fbb(fbb_);

	NR::Matrix fctm = sp_item_i2doc_affine (item);

	NRMatrix gs2d;
	sp_paint_selector_get_gradient_gs2d_matrix_f (psel, &gs2d);

	NR::Matrix g2d = sp_gradient_get_g2d_matrix (SP_GRADIENT (lg), fctm, fbb);

	NR::Matrix gs2g = NR::Matrix(gs2d) / g2d;

	/* Normalize transform */
	NR::Point gp1 = p1 * gs2g;
	NR::Point gp2 = p2 * gs2g;

	double e = NR::expansion(gs2g);
	if (e < 0.001) e = 0.001;
	gs2g[0] /= e;
	gs2g[1] /= e;
	gs2g[2] /= e;
	gs2g[3] /= e;
	gs2g[4] = 0.0;
	gs2g[5] = 0.0;

	gs2d = gs2g * NR::Matrix(g2d);
	sp_gradient_set_gs2d_matrix (SP_GRADIENT (lg), fctm, fbb, gs2d);

	p1 = gp1 * gs2g.inverse();
	p2 = gp2 * gs2g.inverse();

	sp_lineargradient_set_position (lg, p1[NR::X], p1[NR::Y], p2[NR::X], p2[NR::Y]);
}

void
sp_paint_selector_write_radialgradient (SPPaintSelector *psel, SPRadialGradient *rg, SPItem *item)
{
	g_return_if_fail (psel->mode == SP_PAINT_SELECTOR_MODE_GRADIENT_RADIAL);

	SPGradientUnits units;
	SPGradientSpread spread;
	sp_paint_selector_get_gradient_properties (psel, &units, &spread);
	sp_gradient_set_units (SP_GRADIENT (rg), units);
	sp_gradient_set_spread (SP_GRADIENT (rg), spread);

	/* Now position */
	gfloat p[5];
	sp_paint_selector_get_gradient_position_floatv (psel, p);
	NR::Point p1(p[0], p[1]);
	NR::Point p2(p[2], p[3]);
	gdouble r = p[4];

	/* Calculate raw gradient transform */
	NRRect fbb_;
	sp_item_invoke_bbox(item, &fbb_, NR::identity(), TRUE);
	NR::Rect fbb(fbb_);

	NR::Matrix fctm = sp_item_i2doc_affine (item);

	NRMatrix gs2d;
	sp_paint_selector_get_gradient_gs2d_matrix_f (psel, &gs2d);

	NR::Matrix g2d = sp_gradient_get_g2d_matrix (SP_GRADIENT (rg), fctm, fbb);

	NR::Matrix gs2g = NR::Matrix(gs2d) / g2d;

	/* Normalize transform */
	NR::Point gp1 = p1 * gs2g;
	NR::Point gp2 = p2 * gs2g;

	double e = NR::expansion(gs2g);
	if (e < 0.001) e = 0.001;
	gs2g[0] /= e;
	gs2g[1] /= e;
	gs2g[2] /= e;
	gs2g[3] /= e;
	gs2g[4] = 0.0;
	gs2g[5] = 0.0;

	gs2d = gs2g * NR::Matrix(g2d);
	sp_gradient_set_gs2d_matrix (SP_GRADIENT (rg), fctm, fbb, gs2d);

	p1 = gp1 * gs2g.inverse();
	p2 = gp2 * gs2g.inverse();
	r *= e;

	sp_radialgradient_set_position (rg, p1[NR::X], p1[NR::Y], p2[NR::X], p2[NR::Y], r);
}

static void
sp_paint_selector_clear_frame(SPPaintSelector *psel)
{
	g_return_if_fail ( psel != NULL);
	
	if (psel->selector) {

		/* before we destroy the frame contents, we must detach
		 * the patternmenu so that Gtk doesn't gtk_widget_destroy
		 * all the children of the menu.  (We also have a g_object_ref
		 * count set on it too so that the gtk_container_remove doesn't
		 * end up destroying it.
		 */
		GtkWidget *patterns = (GtkWidget *)g_object_get_data (G_OBJECT(psel), "patternmenu");
		if (patterns != NULL) {
			GtkWidget * parent = gtk_widget_get_parent ( GTK_WIDGET (patterns));
			if ( parent != NULL ) {
				g_assert ( GTK_IS_CONTAINER (parent) );
				gtk_container_remove ( GTK_CONTAINER (parent), patterns );
			}
		}

		gtk_widget_destroy (psel->selector);
		psel->selector = NULL;
	}
}

static void
sp_paint_selector_set_mode_empty (SPPaintSelector *psel)
{
	sp_paint_selector_set_style_buttons (psel, NULL);
	gtk_widget_set_sensitive (psel->style, FALSE);

	sp_paint_selector_clear_frame(psel);

	gtk_frame_set_label (GTK_FRAME (psel->frame), _("No objects"));
}

static void
sp_paint_selector_set_mode_multiple (SPPaintSelector *psel)
{
	sp_paint_selector_set_style_buttons (psel, NULL);
	gtk_widget_set_sensitive (psel->style, TRUE);

	sp_paint_selector_clear_frame(psel);

	gtk_frame_set_label (GTK_FRAME (psel->frame), _("Multiple styles"));
}

static void
sp_paint_selector_set_mode_clone (SPPaintSelector *psel)
{
    sp_paint_selector_set_style_buttons (psel, NULL);
    gtk_widget_set_sensitive (psel->style, FALSE);

    sp_paint_selector_clear_frame(psel);

    // TRANSLATORS: "Clone" is a noun here. This message is to notify the user
    // that the currently selected object is a clone.
    gtk_frame_set_label (GTK_FRAME (psel->frame), _("Clone Selected"));
}

static void
sp_paint_selector_set_mode_none (SPPaintSelector *psel)
{
	sp_paint_selector_set_style_buttons (psel, psel->none);
	gtk_widget_set_sensitive (psel->style, TRUE);

	sp_paint_selector_clear_frame(psel);

	gtk_frame_set_label (GTK_FRAME (psel->frame), _("No paint"));
}

/* Color paint */

static void
sp_paint_selector_color_grabbed (SPColorSelector *csel, SPPaintSelector *psel)
{
	gtk_signal_emit (GTK_OBJECT (psel), psel_signals[GRABBED]);
}

static void
sp_paint_selector_color_dragged (SPColorSelector *csel, SPPaintSelector *psel)
{
	gtk_signal_emit (GTK_OBJECT (psel), psel_signals[DRAGGED]);
}

static void
sp_paint_selector_color_released (SPColorSelector *csel, SPPaintSelector *psel)
{
	gtk_signal_emit (GTK_OBJECT (psel), psel_signals[RELEASED]);
}

static void
sp_paint_selector_color_changed (SPColorSelector *csel, SPPaintSelector *psel)
{
	csel->base->getColorAlpha( psel->color, &psel->alpha );

	gtk_signal_emit (GTK_OBJECT (psel), psel_signals[CHANGED]);
}

static void
sp_paint_selector_set_mode_color (SPPaintSelector *psel, SPPaintSelectorMode mode)
{
	GtkWidget *csel;

	sp_paint_selector_set_style_buttons (psel, psel->solid);
	gtk_widget_set_sensitive (psel->style, TRUE);

	if ((psel->mode == SP_PAINT_SELECTOR_MODE_COLOR_RGB) || (psel->mode == SP_PAINT_SELECTOR_MODE_COLOR_CMYK)) {
		/* Already have color selector */
		csel = (GtkWidget*)gtk_object_get_data (GTK_OBJECT (psel->selector), "color-selector");
	} else {

		sp_paint_selector_clear_frame(psel);
		/* Create new color selector */
		/* Create vbox */
		GtkWidget *vb = gtk_vbox_new (FALSE, 4);
		gtk_widget_show (vb);

		/* Color selector */
		csel = sp_color_selector_new (SP_TYPE_COLOR_NOTEBOOK, SP_COLORSPACE_TYPE_NONE);
		gtk_widget_show (csel);
		gtk_object_set_data (GTK_OBJECT (vb), "color-selector", csel);
		gtk_box_pack_start (GTK_BOX (vb), csel, TRUE, TRUE, 0);
		gtk_signal_connect (GTK_OBJECT (csel), "grabbed", GTK_SIGNAL_FUNC (sp_paint_selector_color_grabbed), psel);
		gtk_signal_connect (GTK_OBJECT (csel), "dragged", GTK_SIGNAL_FUNC (sp_paint_selector_color_dragged), psel);
		gtk_signal_connect (GTK_OBJECT (csel), "released", GTK_SIGNAL_FUNC (sp_paint_selector_color_released), psel);
		gtk_signal_connect (GTK_OBJECT (csel), "changed", GTK_SIGNAL_FUNC (sp_paint_selector_color_changed), psel);
		/* Pack everything to frame */
		gtk_container_add (GTK_CONTAINER (psel->frame), vb);
		psel->selector = vb;

		/* Set color */
		SP_COLOR_SELECTOR( csel )->base->setColorAlpha( psel->color, psel->alpha );

	}

	gtk_frame_set_label (GTK_FRAME (psel->frame), _("Flat color"));
#ifdef SP_PS_VERBOSE
	g_print ("Color req\n");
#endif
}

/* Gradient */

static void
sp_paint_selector_gradient_grabbed (SPColorSelector *csel, SPPaintSelector *psel)
{
	gtk_signal_emit (GTK_OBJECT (psel), psel_signals[GRABBED]);
}

static void
sp_paint_selector_gradient_dragged (SPColorSelector *csel, SPPaintSelector *psel)
{
	gtk_signal_emit (GTK_OBJECT (psel), psel_signals[DRAGGED]);
}

static void
sp_paint_selector_gradient_released (SPColorSelector *csel, SPPaintSelector *psel)
{
	gtk_signal_emit (GTK_OBJECT (psel), psel_signals[RELEASED]);
}

static void
sp_paint_selector_gradient_changed (SPColorSelector *csel, SPPaintSelector *psel)
{
	gtk_signal_emit (GTK_OBJECT (psel), psel_signals[CHANGED]);
}

static void
sp_paint_selector_set_mode_gradient (SPPaintSelector *psel, SPPaintSelectorMode mode)
{
	GtkWidget *gsel;

	/* fixme: We do not need function-wide gsel at all */

	if (mode == SP_PAINT_SELECTOR_MODE_GRADIENT_LINEAR) {
		sp_paint_selector_set_style_buttons (psel, psel->gradient);
	} else {
		sp_paint_selector_set_style_buttons (psel, psel->radial);
	}
	gtk_widget_set_sensitive (psel->style, TRUE);

	if ((psel->mode == SP_PAINT_SELECTOR_MODE_GRADIENT_LINEAR) || (psel->mode == SP_PAINT_SELECTOR_MODE_GRADIENT_RADIAL)) {
		/* Already have gradient selector */
		gsel = (GtkWidget*)gtk_object_get_data (GTK_OBJECT (psel->selector), "gradient-selector");
	} else {
		sp_paint_selector_clear_frame(psel);
		/* Create new gradient selector */
		gsel = sp_gradient_selector_new ();
		gtk_widget_show (gsel);
		gtk_signal_connect (GTK_OBJECT (gsel), "grabbed", GTK_SIGNAL_FUNC (sp_paint_selector_gradient_grabbed), psel);
		gtk_signal_connect (GTK_OBJECT (gsel), "dragged", GTK_SIGNAL_FUNC (sp_paint_selector_gradient_dragged), psel);
		gtk_signal_connect (GTK_OBJECT (gsel), "released", GTK_SIGNAL_FUNC (sp_paint_selector_gradient_released), psel);
		gtk_signal_connect (GTK_OBJECT (gsel), "changed", GTK_SIGNAL_FUNC (sp_paint_selector_gradient_changed), psel);
		/* Pack everything to frame */
		gtk_container_add (GTK_CONTAINER (psel->frame), gsel);
		psel->selector = gsel;
		gtk_object_set_data (GTK_OBJECT (psel->selector), "gradient-selector", gsel);
	}

	/* Actually we have to set optiomenu history here */
	if (mode == SP_PAINT_SELECTOR_MODE_GRADIENT_LINEAR) {
		sp_gradient_selector_set_mode (SP_GRADIENT_SELECTOR (gsel), SP_GRADIENT_SELECTOR_MODE_LINEAR);
		gtk_frame_set_label (GTK_FRAME (psel->frame), _("Linear gradient"));
	} else {
		sp_gradient_selector_set_mode (SP_GRADIENT_SELECTOR (gsel), SP_GRADIENT_SELECTOR_MODE_RADIAL);
		gtk_frame_set_label (GTK_FRAME (psel->frame), _("Radial gradient"));
	}
#ifdef SP_PS_VERBOSE
	g_print ("Gradient req\n");
#endif
}

static void
sp_paint_selector_set_style_buttons (SPPaintSelector *psel, GtkWidget *active)
{
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (psel->none), (active == psel->none));
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (psel->solid), (active == psel->solid));
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (psel->gradient), (active == psel->gradient));
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (psel->radial), (active == psel->radial));
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (psel->pattern), (active == psel->pattern));
}

static void
sp_psel_pattern_destroy (GtkWidget *widget,  SPPaintSelector *psel)
{
	// drop our reference to the pattern menu widget
	g_object_unref ( G_OBJECT (widget) );
}

static void
sp_psel_pattern_change (GtkWidget *widget,  SPPaintSelector *psel)
{
	gtk_signal_emit (GTK_OBJECT (psel), psel_signals[CHANGED]);
}

static GtkWidget*
ink_pattern_menu (GtkWidget *mnu)
{
	/* Create new menu widget */
	GtkWidget *m = gtk_menu_new ();
	gtk_widget_show (m);

	/* Pick up all patterns  */
	SPDocument *doc = SP_ACTIVE_DOCUMENT;
	GSList *pl = NULL;
	const GSList *patterns = sp_document_get_resource_list (doc, "pattern");
	for (GSList *l = (GSList *) patterns; l != NULL; l = l->next) {
		if (SP_PATTERN (l->data) == pattern_getroot (SP_PATTERN (l->data))) {  // only if this is a root pattern
			pl = g_slist_prepend (pl, l->data);
		}
	}

	pl = g_slist_reverse (pl);

	if (!doc) {
		GtkWidget *i;
		i = gtk_menu_item_new_with_label (_("No document selected"));
		gtk_widget_show (i);
		gtk_menu_append (GTK_MENU (m), i);
		gtk_widget_set_sensitive (mnu, FALSE);
	} else if (!pl) {
		GtkWidget *i;
		i = gtk_menu_item_new_with_label (_("No patterns in document"));
		gtk_widget_show (i);
		gtk_menu_append (GTK_MENU (m), i);
		gtk_widget_set_sensitive (mnu, FALSE);
	} else {
		for (; pl != NULL; pl = pl->next){
			if (SP_IS_PATTERN(pl->data)){
				SPPattern *pat = SP_PATTERN (pl->data);
				GtkWidget *i = gtk_menu_item_new ();
				gtk_widget_show (i);
				g_object_set_data (G_OBJECT (i), "pattern", pat);
				//        g_signal_connect (G_OBJECT (i), "activate", G_CALLBACK (sp_gvs_gradient_activate), gvs);
				GtkWidget *hb = gtk_hbox_new (FALSE, 4);
				gtk_widget_show (hb);
				SPRepr *repr = SP_OBJECT_REPR((SPItem *) pl->data);
				GtkWidget *l = gtk_label_new (sp_repr_attr(repr,"id"));
				gtk_widget_show (l);
				gtk_misc_set_alignment (GTK_MISC (l), 1.0, 0.5);
				gtk_box_pack_start (GTK_BOX (hb), l, TRUE, TRUE, 0);
				gtk_widget_show (hb);
				gtk_container_add (GTK_CONTAINER (i), hb);
				gtk_menu_append (GTK_MENU (m), i);
			}
		}

		gtk_widget_set_sensitive (mnu, TRUE);
	}
	gtk_option_menu_set_menu (GTK_OPTION_MENU (mnu), m);

	/* Set history */
	//gtk_option_menu_set_history (GTK_OPTION_MENU (mnu), 0);

	g_slist_free (pl);
	return mnu;
}


/*update pattern list*/
void
sp_update_pattern_list ( SPPaintSelector *psel,  SPPattern *pattern)
{
	if (psel->update) return;
	GtkWidget *mnu = (GtkWidget *)g_object_get_data (G_OBJECT(psel), "patternmenu");
	g_assert ( mnu != NULL );

	/* Clear existing menu if any */
	gtk_option_menu_remove_menu (GTK_OPTION_MENU (mnu));

	ink_pattern_menu (mnu);

	/* Set history */

	if (!gtk_object_get_data(GTK_OBJECT(mnu), "update")) {

		gtk_object_set_data(GTK_OBJECT(mnu), "update", GINT_TO_POINTER(TRUE));

		gchar *patname = (gchar *) sp_repr_attr(SP_OBJECT_REPR(pattern),"id");

		GtkMenu *m = GTK_MENU(gtk_option_menu_get_menu (GTK_OPTION_MENU(mnu)));
		GList *kids = GTK_MENU_SHELL(m)->children;

		int patpos = 0;
		int i = 0;

		for (; kids != NULL; kids = kids->next) {
			gchar *men_pat = (gchar *) sp_repr_attr(SP_OBJECT_REPR(g_object_get_data(G_OBJECT(kids->data), "pattern")),"id");
			if ( strcmp(men_pat, patname) == 0 ) {
				patpos = i;
			}
			i++;
		}

		gtk_option_menu_set_history(GTK_OPTION_MENU (mnu), patpos);
		gtk_object_set_data(GTK_OBJECT (mnu), "update", GINT_TO_POINTER (FALSE));
	}
	//gtk_option_menu_set_history (GTK_OPTION_MENU (mnu), 0);
}


static void
sp_paint_selector_set_mode_pattern (SPPaintSelector *psel, SPPaintSelectorMode mode)
{
	if (mode == SP_PAINT_SELECTOR_MODE_PATTERN)
		sp_paint_selector_set_style_buttons (psel, psel->pattern);

	gtk_widget_set_sensitive (psel->style, TRUE);

	GtkWidget *tbl=NULL;

	if (psel->mode == SP_PAINT_SELECTOR_MODE_PATTERN){
		/* Already have pattern menu */
		tbl = (GtkWidget*)gtk_object_get_data (GTK_OBJECT (psel->selector), "pattern-selector");
	} else {
		sp_paint_selector_clear_frame(psel);

		/* Create vbox */
		tbl = gtk_vbox_new (FALSE, 4);
		gtk_widget_show (tbl);

		GtkWidget *hb = gtk_hbox_new (FALSE, 1);

		/*
		 * We want to keep this menu widget around since it takes
		 * time to generate.  As a result, we must either locate the
		 * existing one, or generate a new one and bump the ref
		 * count up before adding it to the container.
		 */
		GtkWidget *mnu = (GtkWidget*)gtk_object_get_data (GTK_OBJECT (psel), "patternmenu");
		if ( mnu == NULL ) {
			mnu = gtk_option_menu_new ();
			ink_pattern_menu (mnu);
			gtk_signal_connect (GTK_OBJECT (mnu), "changed", GTK_SIGNAL_FUNC (sp_psel_pattern_change), psel);
			gtk_signal_connect (GTK_OBJECT (mnu), "destroy", GTK_SIGNAL_FUNC (sp_psel_pattern_destroy), psel);
			gtk_object_set_data (GTK_OBJECT (psel), "patternmenu", mnu);
			g_object_ref( G_OBJECT (mnu));
		}
		gtk_widget_show (mnu);
		gtk_container_add (GTK_CONTAINER (hb), mnu);

		gtk_box_pack_start (GTK_BOX (tbl), hb, FALSE, FALSE, AUX_BETWEEN_BUTTON_GROUPS);

		gtk_widget_show (hb);
		gtk_widget_show_all (tbl);

		gtk_container_add (GTK_CONTAINER (psel->frame), tbl);
		psel->selector = tbl;
		gtk_object_set_data (GTK_OBJECT (psel->selector), "pattern-selector", tbl);

		gtk_frame_set_label (GTK_FRAME (psel->frame), _("Pattern fill"));
	}
#ifdef SP_PS_VERBOSE
	g_print ("Pattern req\n");
#endif
}

SPPattern *
sp_paint_selector_get_pattern (SPPaintSelector *psel)
{
	SPPattern *pat;

	g_return_val_if_fail ((psel->mode == SP_PAINT_SELECTOR_MODE_PATTERN) , NULL);

	GtkWidget *patmnu = (GtkWidget *) g_object_get_data (G_OBJECT(psel), "patternmenu");
	/* no pattern menu if we were just selected */
	if ( patmnu == NULL ) return NULL;

	GtkMenu *m = GTK_MENU(gtk_option_menu_get_menu (GTK_OPTION_MENU(patmnu)));

	pat = pattern_getroot (SP_PATTERN(g_object_get_data (G_OBJECT(gtk_menu_get_active (m)), "pattern")));

	return pat;
}


