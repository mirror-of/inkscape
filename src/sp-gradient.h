#ifndef __SP_GRADIENT_H__
#define __SP_GRADIENT_H__

/*
 * SVG <stop> <linearGradient> and <radialGradient> implementation
 *
 * Author:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * Copyright (C) 1999-2002 Lauris Kaplinski
 * Copyright (C) 2000-2001 Ximian, Inc.
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <glib.h>

#define SP_TYPE_GRADIENT (sp_gradient_get_type ())
#define SP_GRADIENT(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), SP_TYPE_GRADIENT, SPGradient))
#define SP_GRADIENT_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), SP_TYPE_GRADIENT, SPGradientClass))
#define SP_IS_GRADIENT(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), SP_TYPE_GRADIENT))
#define SP_IS_GRADIENT_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), SP_TYPE_GRADIENT))

#include "libnr/nr-matrix.h"
#include "forward.h"
#include "sp-paint-server.h"
#include "sp-stop.h"             /* TODO: Remove this #include. */
#include "sp-gradient-vector.h"	 /* TODO: Remove this #include. */

/*
 * Gradient
 *
 * Implement spread, stops list
 * fixme: Implement more here (Lauris)
 */

class SPGradientStop;
class SPGradientVector;

typedef enum {
	SP_GRADIENT_TYPE_UNKNOWN,
	SP_GRADIENT_TYPE_LINEAR,
	SP_GRADIENT_TYPE_RADIAL
} SPGradientType;

typedef enum {
	SP_GRADIENT_STATE_UNKNOWN,
	SP_GRADIENT_STATE_VECTOR,
	SP_GRADIENT_STATE_PRIVATE
} SPGradientState;

typedef enum {
	SP_GRADIENT_UNITS_OBJECTBOUNDINGBOX,
	SP_GRADIENT_UNITS_USERSPACEONUSE
} SPGradientUnits;

typedef enum {
	SP_GRADIENT_SPREAD_PAD,
	SP_GRADIENT_SPREAD_REFLECT,
	SP_GRADIENT_SPREAD_REPEAT
} SPGradientSpread;

#define SP_GRADIENT_STATE_IS_SET(g) (SP_GRADIENT(g)->state != SP_GRADIENT_STATE_UNKNOWN)
#define SP_GRADIENT_IS_VECTOR(g) (SP_GRADIENT(g)->state == SP_GRADIENT_STATE_VECTOR)
#define SP_GRADIENT_IS_PRIVATE(g) (SP_GRADIENT(g)->state == SP_GRADIENT_STATE_PRIVATE)
#define SP_GRADIENT_HAS_STOPS(g) (SP_GRADIENT(g)->has_stops)
#define SP_GRADIENT_SPREAD(g) (SP_GRADIENT (g)->spread)
#define SP_GRADIENT_UNITS(g) (SP_GRADIENT (g)->units)

struct SPGradient {
	SPPaintServer paint_server;
	/* Reference (href) */
	SPGradientReference *ref;
	/* State in Inkscape gradient system */
	guint state : 2;
	/* gradientUnits attribute */
	SPGradientUnits units;
	guint units_set : 1;
	/* gradientTransform attribute */
	NR::Matrix gradientTransform;
	guint gradientTransform_set : 1;
	/* spreadMethod attribute */
	SPGradientSpread spread;
	guint spread_set : 1;
	/* Gradient stops */
	guint has_stops : 1;
	/* Composed vector */
	SPGradientVector *vector;
	/* Rendered color array (4 * 1024 bytes at moment) */
	guchar *color;
	/* Length of vector */
	gdouble len;
};

struct SPGradientClass {
	SPPaintServerClass parent_class;
};

GType sp_gradient_get_type (void);

/* Forces vector to be built, if not present (i.e. changed) */
void sp_gradient_ensure_vector (SPGradient *gradient);
/* Ensures that color array is populated */
void sp_gradient_ensure_colors (SPGradient *gradient);
/* Sets gradient vector to given value, does not update reprs */
void sp_gradient_set_vector (SPGradient *gradient, SPGradientVector *vector);

void sp_gradient_set_units (SPGradient *gr, SPGradientUnits units);
void sp_gradient_set_spread (SPGradient *gr, SPGradientSpread spread);

/* Gradient repr methods */
void sp_gradient_repr_set_vector (SPGradient *gradient, SPRepr *repr, SPGradientVector *vector);

/*
 * Renders gradient vector to buffer
 *
 * len, width, height, rowstride - buffer parameters (1 or 2 dimensional)
 * span - full integer width of requested gradient
 * pos - buffer starting position in span
 *
 * RGB buffer background should be set up before
 */
void sp_gradient_render_vector_line_rgba (SPGradient *gr, guchar *px, gint len, gint pos, gint span);
void sp_gradient_render_vector_line_rgb (SPGradient *gr, guchar *px, gint len, gint pos, gint span);
void sp_gradient_render_vector_block_rgba (SPGradient *gr, guchar *px, gint w, gint h, gint rs, gint pos, gint span, gboolean horizontal);
void sp_gradient_render_vector_block_rgb (SPGradient *gr, guchar *px, gint w, gint h, gint rs, gint pos, gint span, gboolean horizontal);

/* Transforms to/from gradient position space in given environment */
NRMatrix *sp_gradient_get_g2d_matrix_f(SPGradient const *gr, NRMatrix const *ctm, NRRect const *bbox,
				       NRMatrix *g2d);
NRMatrix *sp_gradient_get_gs2d_matrix_f(SPGradient const *gr, NRMatrix const *ctm, NRRect const *bbox,
					NRMatrix *gs2d);
void sp_gradient_set_gs2d_matrix_f(SPGradient *gr, NRMatrix const *ctm, NRRect const *bbox, NRMatrix const *gs2d);


#include "sp-gradient-reference.h"  /* TODO: Get rid of this #include. */
#include "sp-linear-gradient.h"  /* TODO: Get rid of this #include. */
#include "sp-radial-gradient.h"  /* TODO: Get rid of this #include. */

#endif /* !__SP_GRADIENT_H__ */

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=99 :
