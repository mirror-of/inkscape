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

#include "libnr/nr-matrix.h"
#include "forward.h"
#include "sp-gradient-spread.h"
#include "sp-gradient-units.h"
#include "sp-paint-server.h"
class SPGradientStop;
class SPGradientVector;

/*
 * Gradient
 *
 * Implement spread, stops list
 * fixme: Implement more here (Lauris)
 */

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

struct SPGradient : public SPPaintServer {
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


#include "sp-gradient-fns.h"

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
