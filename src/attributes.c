#define __SP_ATTRIBUTES_C__

/*
 * Lookup dictionary for attributes/properties
 *
 * Author:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * Copyright (C) 2002 Lauris Kaplinski
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <glib.h>
#include "attributes.h"

typedef struct {
	gint code;
	guchar *name;
} SPStyleProp;

static const SPStyleProp props[] = {
	{SP_ATTR_INVALID, NULL},
	/* SPObject */
	{SP_ATTR_ID, "id"},
	/* SPItem */
	{SP_ATTR_TRANSFORM, "transform"},
	{SP_ATTR_SODIPODI_INSENSITIVE, "sodipodi:insensitive"},
	{SP_ATTR_SODIPODI_NONPRINTABLE, "sodipodi:nonprintable"},
	{SP_ATTR_STYLE, "style"},
	/* SPAnchor */
	{SP_ATTR_XLINK_HREF, "xlink:href"},
	{SP_ATTR_XLINK_TYPE, "xlink:type"},
	{SP_ATTR_XLINK_ROLE, "xlink:role"},
	{SP_ATTR_XLINK_ARCROLE, "xlink:arcrole"},
	{SP_ATTR_XLINK_TITLE, "xlink:title"},
	{SP_ATTR_XLINK_SHOW, "xlink:show"},
	{SP_ATTR_XLINK_ACTUATE, "xlink:actuate"},
	{SP_ATTR_TARGET, "target"},
	/* SPRoot */
	{SP_ATTR_VERSION, "version"},
	{SP_ATTR_WIDTH, "width"},
	{SP_ATTR_HEIGHT, "height"},
	{SP_ATTR_VIEWBOX, "viewBox"},
	{SP_ATTR_PRESERVEASPECTRATIO, "preserveAspectRatio"},
	{SP_ATTR_SODIPODI_VERSION, "sodipodi:version"},
	/* SPNamedView */
	{SP_ATTR_VIEWONLY, "viewonly"},
	{SP_ATTR_SHOWGRID, "showgrid"},
	{SP_ATTR_SNAPTOGRID, "snaptogrid"},
	{SP_ATTR_SHOWGUIDES, "showguides"},
	{SP_ATTR_SNAPTOGUIDES, "snaptoguides"},
	{SP_ATTR_GRIDTOLERANCE, "gridtolerance"},
	{SP_ATTR_GUIDETOLERANCE, "guidetolerance"},
	{SP_ATTR_GRIDORIGINX, "gridoriginx"},
	{SP_ATTR_GRIDORIGINY, "gridoriginy"},
	{SP_ATTR_GRIDSPACINGX, "gridspacingx"},
	{SP_ATTR_GRIDSPACINGY, "gridspacingy"},
	{SP_ATTR_GRIDCOLOR, "gridcolor"},
	{SP_ATTR_GRIDOPACITY, "gridopacity"},
	{SP_ATTR_GUIDECOLOR, "guidecolor"},
	{SP_ATTR_GUIDEOPACITY, "guideopacity"},
	{SP_ATTR_GUIDEHICOLOR, "guidehicolor"},
	{SP_ATTR_GUIDEHIOPACITY, "guidehiopacity"},
	{SP_ATTR_SHOWBORDER, "showborder"},
	{SP_ATTR_BORDERLAYER, "borderlayer"},
	/* SPGuide */
	{SP_ATTR_ORIENTATION, "orientation"},
	{SP_ATTR_POSITION, "position"},
	/* SPImage */
	{SP_ATTR_X, "x"},
	{SP_ATTR_Y, "y"},
	/* SPPath */
	{SP_ATTR_D, "d"},
	/* SPRect */
	{SP_ATTR_RX, "rx"},
	{SP_ATTR_RY, "ry"},
	/* SPEllipse */
	{SP_ATTR_R, "r"},
	{SP_ATTR_CX, "cx"},
	{SP_ATTR_CY, "cy"},
	{SP_ATTR_SODIPODI_CX, "sodipodi:cx"},
	{SP_ATTR_SODIPODI_CY, "sodipodi:cy"},
	{SP_ATTR_SODIPODI_RX, "sodipodi:rx"},
	{SP_ATTR_SODIPODI_RY, "sodipodi:ry"},
	{SP_ATTR_SODIPODI_START, "sodipodi:start"},
	{SP_ATTR_SODIPODI_END, "sodipodi:end"},
	{SP_ATTR_SODIPODI_OPEN, "sodipodi:open"},
	/* SPStar */
	{SP_ATTR_SODIPODI_SIDES, "sodipodi:sides"},
	{SP_ATTR_SODIPODI_R1, "sodipodi:r1"},
	{SP_ATTR_SODIPODI_R2, "sodipodi:r2"},
	{SP_ATTR_SODIPODI_ARG1, "sodipodi:arg1"},
	{SP_ATTR_SODIPODI_ARG2, "sodipodi:arg2"},
	/* SPSpiral */
	{SP_ATTR_SODIPODI_EXPANSION, "sodipodi:expansion"},
	{SP_ATTR_SODIPODI_REVOLUTION, "sodipodi:revolution"},
	{SP_ATTR_SODIPODI_RADIUS, "sodipodi:radius"},
	{SP_ATTR_SODIPODI_ARGUMENT, "sodipodi:argument"},
	{SP_ATTR_SODIPODI_T0, "sodipodi:t0"},
	/* SPLine */
	{SP_ATTR_X1, "x1"},
	{SP_ATTR_Y1, "y1"},
	{SP_ATTR_X2, "x2"},
	{SP_ATTR_Y2, "y2"},
	/* SPPolyline */
	{SP_ATTR_POINTS, "points"},
	/* SPTSpan */
	{SP_ATTR_DX, "dx"},
	{SP_ATTR_DY, "dy"},
	{SP_ATTR_ROTATE, "rotate"},
	{SP_ATTR_SODIPODI_ROLE, "sodipodi:role"},
	/* SPText */
	{SP_ATTR_SODIPODI_LINESPACING, "sodipodi:linespacing"},
	/* SPStop */
	{SP_ATTR_OFFSET, "offset"},
	/* SPGradient */
	{SP_ATTR_GRADIENTUNITS, "gradientUnits"},
	{SP_ATTR_GRADIENTTRANSFORM, "gradientTransform"},
	{SP_ATTR_SPREADMETHOD, "spreadMethod"},
	/* SPRadialGradient */
	{SP_ATTR_FX, "fx"},
	{SP_ATTR_FY, "fy"},
	/* SPPattern */
	{SP_ATTR_PATTERNUNITS, "patternUnits"},
	{SP_ATTR_PATTERNCONTENTUNITS, "patternContentUnits"},
	{SP_ATTR_PATTERNTRANSFORM, "patternTransform"},
	/* SPClipPath */
	{SP_ATTR_CLIPPATHUNITS, "clipPathUnits"},
	/* SPMask */
	{SP_ATTR_MASKUNITS, "maskUnits"},
	{SP_ATTR_MASKCONTENTUNITS, "maskContentUnits"},
	/* SPMarker */
	{SP_ATTR_MARKERUNITS, "markerUnits"},
	{SP_ATTR_REFX, "refX"},
	{SP_ATTR_REFY, "refY"},
	{SP_ATTR_MARKERWIDTH, "markerWidth"},
	{SP_ATTR_MARKERHEIGHT, "markerHeight"},
	{SP_ATTR_ORIENT, "orient"},
	/* Animations */
	{SP_ATTR_ATTRIBUTENAME, "attributeName"},
	{SP_ATTR_ATTRIBUTETYPE, "attributeType"},
	{SP_ATTR_BEGIN, "begin"},
	{SP_ATTR_DUR, "dur"},
	{SP_ATTR_END, "end"},
	{SP_ATTR_MIN, "min"},
	{SP_ATTR_MAX, "max"},
	{SP_ATTR_RESTART, "restart"},
	{SP_ATTR_REPEATCOUNT, "repeatCount"},
	{SP_ATTR_REPEATDUR, "repeatDur"},
	/* Interpolating animations */
	{SP_ATTR_CALCMODE, "calcMode"},
	{SP_ATTR_VALUES, "values"},
	{SP_ATTR_KEYTIMES, "keyTimes"},
	{SP_ATTR_KEYSPLINES, "keySplines"},
	{SP_ATTR_FROM, "from"},
	{SP_ATTR_TO, "to"},
	{SP_ATTR_BY, "by"},
	{SP_ATTR_ADDITIVE, "additive"},
	{SP_ATTR_ACCUMULATE, "accumulate"},

	/* CSS2 */
	/* Font */
	{SP_PROP_FONT, "font"},
	{SP_PROP_FONT_FAMILY, "font-family"},
	{SP_PROP_FONT_SIZE, "font-size"},
	{SP_PROP_FONT_SIZE_ADJUST, "font-size-adjust"},
	{SP_PROP_FONT_STRETCH, "font-stretch"},
	{SP_PROP_FONT_STYLE, "font-style"},
	{SP_PROP_FONT_VARIANT, "font-variant"},
	{SP_PROP_FONT_WEIGHT, "font-weight"},
	/* Text */
	{SP_PROP_DIRECTION, "direction"},
	{SP_PROP_LETTER_SPACING, "letter-spacing"},
	{SP_PROP_TEXT_DECORATION, "text-decoration"},
	{SP_PROP_UNICODE_BIDI, "unicode-bidi"},
	{SP_PROP_WORD_SPACING, "word-spacing"},
	/* Misc */
	{SP_PROP_CLIP, "clip"},
	{SP_PROP_COLOR, "color"},
	{SP_PROP_CURSOR, "cursor"},
	{SP_PROP_DISPLAY, "display"},
	{SP_PROP_OVERFLOW, "overflow"},
	{SP_PROP_VISIBILITY, "visibility"},
	/* SVG */
	/* Clip/Mask */
	{SP_PROP_CLIP_PATH, "clip-path"},
	{SP_PROP_CLIP_RULE, "clip-rule"},
	{SP_PROP_MASK, "mask"},
	{SP_PROP_OPACITY, "opacity"},
	/* Filter */
	{SP_PROP_ENABLE_BACKGROUND, "enable-background"},
	{SP_PROP_FILTER, "filter"},
	{SP_PROP_FLOOD_COLOR, "flood-color"},
	{SP_PROP_FLOOD_OPACITY, "flood-opacity"},
	{SP_PROP_LIGHTING_COLOR, "lighting-color"},
	/* Gradient */
	{SP_PROP_STOP_COLOR, "stop-color"},
	{SP_PROP_STOP_OPACITY, "stop-opacity"},
	/* Interactivity */
	{SP_PROP_POINTER_EVENTS, "pointer-events"},
	/* Paint */
	{SP_PROP_COLOR_INTERPOLATION, "color-interpolation"},
	{SP_PROP_COLOR_INTERPOLATION_FILTERS, "color-interpolation-filters"},
	{SP_PROP_COLOR_PROFILE, "color-profile"},
	{SP_PROP_COLOR_RENDERING, "color-rendering"},
	{SP_PROP_FILL, "fill"},
	{SP_PROP_FILL_OPACITY, "fill-opacity"},
	{SP_PROP_FILL_RULE, "fill-rule"},
	{SP_PROP_IMAGE_RENDERING, "image-rendering"},
	{SP_PROP_MARKER, "marker"},
	{SP_PROP_MARKER_END, "marker-end"},
	{SP_PROP_MARKER_MID, "marker-mid"},
	{SP_PROP_MARKER_START, "marker-start"},
	{SP_PROP_SHAPE_RENDERING, "shape-rendering"},
	{SP_PROP_STROKE, "stroke"},
	{SP_PROP_STROKE_DASHARRAY, "stroke-dasharray"},
	{SP_PROP_STROKE_DASHOFFSET, "stroke-dashoffset"},
	{SP_PROP_STROKE_LINECAP, "stroke-linecap"},
	{SP_PROP_STROKE_LINEJOIN, "stroke-linejoin"},
	{SP_PROP_STROKE_MITERLIMIT, "stroke-miterlimit"},
	{SP_PROP_STROKE_OPACITY, "stroke-opacity"},
	{SP_PROP_STROKE_WIDTH, "stroke-width"},
	{SP_PROP_TEXT_RENDERING, "text-rendering"},
	/* Text */
	{SP_PROP_ALIGNMENT_BASELINE, "alignment-baseline"},
	{SP_PROP_BASELINE_SHIFT, "baseline-shift"},
	{SP_PROP_DOMINANT_BASELINE, "dominant-baseline"},
	{SP_PROP_GLYPH_ORIENTATION_HORIZONTAL, "glyph-orientation-horizontal"},
	{SP_PROP_GLYPH_ORIENTATION_VERTICAL, "glyph-orientation-vertical"},
	{SP_PROP_KERNING, "kerning"},
	{SP_PROP_TEXT_ANCHOR, "text-anchor"},
	{SP_PROP_WRITING_MODE, "writing-mode"}
};

#define n_attrs (sizeof (props) / sizeof (props[0]))

unsigned int
sp_attribute_lookup (const unsigned char *key)
{
	static GHashTable *propdict = NULL;

	if (!propdict) {
		int i;
		propdict = g_hash_table_new (g_str_hash, g_str_equal);
		for (i = 1; i < n_attrs; i++) {
			g_assert (props[i].code == i);
			g_hash_table_insert (propdict, props[i].name, GINT_TO_POINTER (props[i].code));
		}
	}

	return GPOINTER_TO_UINT (g_hash_table_lookup (propdict, key));
}

const unsigned char *
sp_attribute_name (unsigned char id)
{
	if (id >= n_attrs) return NULL;

	return props[id].name;
}

