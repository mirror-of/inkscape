#ifndef __SP_PATTERN_H__
#define __SP_PATTERN_H__

/*
 * SVG <pattern> implementation
 *
 * Author:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * Copyright (C) 2002 Lauris Kaplinski
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "forward.h"

#define SP_TYPE_PATTERN (sp_pattern_get_type ())
#define SP_PATTERN(o) (GTK_CHECK_CAST ((o), SP_TYPE_PATTERN, SPPattern))
#define SP_PATTERN_CLASS(k) (GTK_CHECK_CLASS_CAST ((k), SP_TYPE_PATTERN, SPPatternClass))
#define SP_IS_PATTERN(o) (GTK_CHECK_TYPE ((o), SP_TYPE_PATTERN))
#define SP_IS_PATTERN_CLASS(k) (GTK_CHECK_CLASS_TYPE ((k), SP_TYPE_PATTERN))

GType sp_pattern_get_type (void);

class SPPatternClass;

#include <libnr/nr-rect.h>
#include <libnr/nr-matrix.h>
#include "svg/svg-types.h"
#include "sp-paint-server.h"
#include "uri-references.h"

class SPPatternReference : public Inkscape::URIReference {
public:
        SPPatternReference (SPObject *obj) : URIReference(obj) {}
        SPPattern *getObject() const {
                return (SPPattern *)URIReference::getObject();
        }
protected:
        bool _acceptObject(SPObject *obj) const {
                return SP_IS_PATTERN (obj);
        }
};

enum {
	SP_PATTERN_UNITS_USERSPACEONUSE,
	SP_PATTERN_UNITS_OBJECTBOUNDINGBOX
};

struct SPPattern {
	SPPaintServer paint_server;

	/* Reference (href) */
	gchar *href;
	SPPatternReference *ref;

	/* patternUnits and patternContentUnits attribute */
	guint patternUnits : 1;
	guint patternUnits_set : 1;
	guint patternContentUnits : 1;
	guint patternContentUnits_set : 1;
	/* patternTransform attribute */
	NRMatrix patternTransform;
	guint patternTransform_set : 1;
	/* Tile rectangle */
	SPSVGLength x;
	SPSVGLength y;
	SPSVGLength width;
	SPSVGLength height;
	/* VieBox */
	NRRect viewBox;
	guint viewBox_set : 1;
};

struct SPPatternClass {
	SPPaintServerClass parent_class;
};

#endif
