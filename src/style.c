#define __SP_STYLE_C__

/*
 * SPStyle - a style object for SPItems
 *
 * Author:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * Copyright (C) 2001-2002 Lauris Kaplinski
 * Copyright (C) 2001 Ximian, Inc.
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <gtk/gtksignal.h>

#include "svg/svg.h"

#include "enums.h"
#include "attributes.h"
#include "document.h"
#include "uri-references.h"
#include "sp-paint-server.h"
#include "style.h"

#define BMAX 8192

#define SP_STYLE_FLAG_IFSET (1 << 0)
#define SP_STYLE_FLAG_IFDIFF (1 << 1)

typedef struct _SPStyleEnum SPStyleEnum;

static void sp_style_clear (SPStyle *style);

static void sp_style_merge_from_style_string (SPStyle *style, const guchar *p);
static void sp_style_merge_property (SPStyle *style, gint id, const guchar *val);

static void sp_style_merge_ipaint (SPStyle *style, SPIPaint *paint, SPIPaint *parent);
static void sp_style_read_dash (ArtVpathDash *dash, const guchar *str);

static SPTextStyle *sp_text_style_new (void);
static void sp_text_style_clear (SPTextStyle *ts);
static SPTextStyle *sp_text_style_unref (SPTextStyle *st);
static SPTextStyle *sp_text_style_duplicate_unset (SPTextStyle *st);
static guint sp_text_style_write (guchar *p, guint len, SPTextStyle *st);
static void sp_style_privatize_text (SPStyle *style);

static void sp_style_read_ifloat (SPIFloat *val, const guchar *str);
static void sp_style_read_iscale24 (SPIScale24 *val, const guchar *str);
static void sp_style_read_ienum (SPIEnum *val, const guchar *str, const SPStyleEnum *dict, unsigned int inherit);
static void sp_style_read_istring (SPIString *val, const guchar *str);
static void sp_style_read_ilength (SPILength *val, const guchar *str);
static void sp_style_read_ipaint (SPIPaint *paint, const guchar *str, SPStyle *style, SPDocument *document);
static void sp_style_read_ifontsize (SPIFontSize *val, const guchar *str);

#if 0
static void sp_style_read_pfloat (SPIFloat *val, SPRepr *repr, const guchar *key);
#endif
static void sp_style_read_penum (SPIEnum *val, SPRepr *repr, const guchar *key, const SPStyleEnum *dict, unsigned int inherit);
static void sp_style_read_plength (SPILength *val, SPRepr *repr, const guchar *key);
static void sp_style_read_pfontsize (SPIFontSize *val, SPRepr *repr, const guchar *key);

static gint sp_style_write_ifloat (guchar *p, gint len, const guchar *key, SPIFloat *val, SPIFloat *base, guint flags);
static gint sp_style_write_iscale24 (guchar *p, gint len, const guchar *key, SPIScale24 *val, SPIScale24 *base, guint flags);
static gint sp_style_write_ienum (guchar *p, gint len, const guchar *key, const SPStyleEnum *dict, SPIEnum *val, SPIEnum *base, guint flags);
static gint sp_style_write_istring (guchar *p, gint len, const guchar *key, SPIString *val, SPIString *base, guint flags);
static gint sp_style_write_ilength (guchar *p, gint len, const guchar *key, SPILength *val, SPILength *base, guint flags);
static gint sp_style_write_ipaint (guchar *b, gint len, const guchar *key, SPIPaint *paint, SPIPaint *base, guint flags);
static gint sp_style_write_ifontsize (guchar *p, gint len, const guchar *key, SPIFontSize *val, SPIFontSize *base, guint flags);

static SPColor *sp_style_read_color_cmyk (SPColor *color, const guchar *str);

static void sp_style_paint_clear (SPStyle *style, SPIPaint *paint, unsigned int hunref, unsigned int unset);

#define SPS_READ_IFLOAT_IF_UNSET(v,s) if (!(v)->set) {sp_style_read_ifloat ((v), (s));}
#define SPS_READ_PFLOAT_IF_UNSET(v,r,k) if (!(v)->set) {sp_style_read_pfloat ((v), (r), (k));}

#define SPS_READ_IENUM_IF_UNSET(v,s,d,i) if (!(v)->set) {sp_style_read_ienum ((v), (s), (d), (i));}
#define SPS_READ_PENUM_IF_UNSET(v,r,k,d,i) if (!(v)->set) {sp_style_read_penum ((v), (r), (k), (d), (i));}

#define SPS_READ_ILENGTH_IF_UNSET(v,s) if (!(v)->set) {sp_style_read_ilength ((v), (s));}
#define SPS_READ_PLENGTH_IF_UNSET(v,r,k) if (!(v)->set) {sp_style_read_plength ((v), (r), (k));}

#define SPS_READ_IFONTSIZE_IF_UNSET(v,s) if (!(v)->set) {sp_style_read_ifontsize ((v), (s));}
#define SPS_READ_PFONTSIZE_IF_UNSET(v,r,k) if (!(v)->set) {sp_style_read_pfontsize ((v), (r), (k));}

struct _SPStyleEnum {
	const guchar *key;
	gint value;
};

static const SPStyleEnum enum_fill_rule[] = {
	{"nonzero", SP_WIND_RULE_NONZERO},
	{"evenodd", SP_WIND_RULE_EVENODD},
	{NULL, -1}
};

static const SPStyleEnum enum_stroke_linecap[] = {
	{"butt", SP_STROKE_LINECAP_BUTT},
	{"round", SP_STROKE_LINECAP_ROUND},
	{"square", SP_STROKE_LINECAP_SQUARE},
	{NULL, -1}
};

static const SPStyleEnum enum_stroke_linejoin[] = {
	{"miter", SP_STROKE_LINEJOIN_MITER},
	{"round", SP_STROKE_LINEJOIN_ROUND},
	{"bevel", SP_STROKE_LINEJOIN_BEVEL},
	{NULL, -1}
};

static const SPStyleEnum enum_font_style[] = {
	{"normal", SP_CSS_FONT_STYLE_NORMAL},
	{"italic", SP_CSS_FONT_STYLE_ITALIC},
	{"oblique", SP_CSS_FONT_STYLE_OBLIQUE},
	{NULL, -1}
};

static const SPStyleEnum enum_font_size[] = {
	{"xx-small", SP_CSS_FONT_SIZE_XX_SMALL},
	{"x-small", SP_CSS_FONT_SIZE_X_SMALL},
	{"small", SP_CSS_FONT_SIZE_SMALL},
	{"medium", SP_CSS_FONT_SIZE_MEDIUM},
	{"large", SP_CSS_FONT_SIZE_LARGE},
	{"x-large", SP_CSS_FONT_SIZE_X_LARGE},
	{"xx-large", SP_CSS_FONT_SIZE_XX_LARGE},
	{"smaller", SP_CSS_FONT_SIZE_SMALLER},
	{"larger", SP_CSS_FONT_SIZE_LARGER},
	{NULL, -1}
};

static const SPStyleEnum enum_font_variant[] = {
	{"normal", SP_CSS_FONT_VARIANT_NORMAL},
	{"small-caps", SP_CSS_FONT_VARIANT_SMALL_CAPS},
	{NULL, -1}
};

static const SPStyleEnum enum_font_weight[] = {
	{"100", SP_CSS_FONT_WEIGHT_100},
	{"200", SP_CSS_FONT_WEIGHT_200},
	{"300", SP_CSS_FONT_WEIGHT_300},
	{"400", SP_CSS_FONT_WEIGHT_400},
	{"500", SP_CSS_FONT_WEIGHT_500},
	{"600", SP_CSS_FONT_WEIGHT_600},
	{"700", SP_CSS_FONT_WEIGHT_700},
	{"800", SP_CSS_FONT_WEIGHT_800},
	{"900", SP_CSS_FONT_WEIGHT_900},
	{"normal", SP_CSS_FONT_WEIGHT_NORMAL},
	{"bold", SP_CSS_FONT_WEIGHT_BOLD},
	{"lighter", SP_CSS_FONT_WEIGHT_LIGHTER},
	{"darker", SP_CSS_FONT_WEIGHT_DARKER},
	{NULL, -1}
};

static const SPStyleEnum enum_font_stretch[] = {
	{"ultra-condensed", SP_CSS_FONT_STRETCH_ULTRA_CONDENSED},
	{"extra-condensed", SP_CSS_FONT_STRETCH_EXTRA_CONDENSED},
	{"condensed", SP_CSS_FONT_STRETCH_CONDENSED},
	{"semi-condensed", SP_CSS_FONT_STRETCH_SEMI_CONDENSED},
	{"normal", SP_CSS_FONT_STRETCH_NORMAL},
	{"semi-expanded", SP_CSS_FONT_STRETCH_SEMI_EXPANDED},
	{"expanded", SP_CSS_FONT_STRETCH_EXPANDED},
	{"extra-expanded", SP_CSS_FONT_STRETCH_EXTRA_EXPANDED},
	{"ultra-expanded", SP_CSS_FONT_STRETCH_ULTRA_EXPANDED},
	{"narrower", SP_CSS_FONT_STRETCH_NARROWER},
	{"wider", SP_CSS_FONT_STRETCH_WIDER},
	{NULL, -1}
};

static const SPStyleEnum enum_text_anchor[] = {
	{"start", SP_CSS_TEXT_ANCHOR_START},
	{"middle", SP_CSS_TEXT_ANCHOR_MIDDLE},
	{"end", SP_CSS_TEXT_ANCHOR_END},
	{NULL, -1}
};

static const SPStyleEnum enum_writing_mode[] = {
	{"lr", SP_CSS_WRITING_MODE_LR},
	{"rl", SP_CSS_WRITING_MODE_RL},
	{"tb", SP_CSS_WRITING_MODE_TB},
	{"lr-tb", SP_CSS_WRITING_MODE_LR},
	{"rl-tb", SP_CSS_WRITING_MODE_RL},
	{"tb-rl", SP_CSS_WRITING_MODE_TB},
	{NULL, -1}
};

static void
sp_style_object_release (SPObject *object, SPStyle *style)
{
	style->object = NULL;
}

SPStyle *
sp_style_new (void)
{
	SPStyle *style;

	style = g_new0 (SPStyle, 1);

	style->refcount = 1;
	style->object = NULL;
	style->text = sp_text_style_new ();
	style->text_private = TRUE;

	sp_style_clear (style);

	return style;
}

SPStyle *
sp_style_new_from_object (SPObject *object)
{
	SPStyle *style;

	g_return_val_if_fail (object != NULL, NULL);
	g_return_val_if_fail (SP_IS_OBJECT (object), NULL);

	style = sp_style_new ();

	style->object = object;
	g_signal_connect (G_OBJECT (object), "release", G_CALLBACK (sp_style_object_release), style);

	return style;
}

SPStyle *
sp_style_ref (SPStyle *style)
{
	g_return_val_if_fail (style != NULL, NULL);
	g_return_val_if_fail (style->refcount > 0, NULL);

	style->refcount += 1;

	return style;
}

SPStyle *
sp_style_unref (SPStyle *style)
{
	g_return_val_if_fail (style != NULL, NULL);
	g_return_val_if_fail (style->refcount > 0, NULL);

	style->refcount -= 1;

	if (style->refcount < 1) {
/* 		if (style->object) gtk_signal_disconnect_by_data (GTK_OBJECT (style->object), style); */
		if (style->object) g_signal_handlers_disconnect_matched (G_OBJECT(style->object), G_SIGNAL_MATCH_DATA, 0, 0, NULL, NULL, style);
		if (style->text) sp_text_style_unref (style->text);
		sp_style_paint_clear (style, &style->fill, TRUE, FALSE);
		sp_style_paint_clear (style, &style->stroke, TRUE, FALSE);
		if (style->stroke_dash.dash) {
			g_free (style->stroke_dash.dash);
		}
		g_free (style);
	}

	return NULL;
}

static void
sp_style_read (SPStyle *style, SPObject *object, SPRepr *repr)
{
	const guchar *val;

	g_assert (style != NULL);
	g_assert (repr != NULL);
	g_assert (!object || (SP_OBJECT_REPR (object) == repr));

	sp_style_clear (style);

	/* 1. Style itself */
	val = sp_repr_attr (repr, "style");
	if (val != NULL) {
		sp_style_merge_from_style_string (style, val);
	}

	/* 2. Presentation-only attributes */
	/* CMYK has precedence and can only be presentation attribute */
	if (!style->fill.set || (style->fill.type == SP_PAINT_TYPE_COLOR)) {
		/* backwards-compatability */
		val = sp_repr_attr (repr, "fill-cmyk");
		if (val) {
			sp_repr_set_attr (repr, "sodipodi:fill-cmyk", val);
			sp_repr_set_attr (repr, "fill-cmyk", NULL);
		}
		val = sp_repr_attr (repr, "sodipodi:fill-cmyk");
		if (val && sp_style_read_color_cmyk (&style->fill.value.color, val)) {
			style->fill.set = TRUE;
			style->fill.inherit = FALSE;
		}
	}
	if (!style->stroke.set || (style->stroke.type == SP_PAINT_TYPE_COLOR)) {
		/* backwards-compatability */
		val = sp_repr_attr (repr, "stroke-cmyk");
		if (val) {
			sp_repr_set_attr (repr, "sodipodi:stroke-cmyk", val);
			sp_repr_set_attr (repr, "stroke-cmyk", NULL);
		}
		val = sp_repr_attr (repr, "sodipodi:stroke-cmyk");
		if (val && sp_style_read_color_cmyk (&style->stroke.value.color, val)) {
			style->stroke.set = TRUE;
			style->stroke.inherit = FALSE;
		}
	}

	/* fixme: CSS etc. parsing goes here */
	/* 3. Other styling methods */

	/* 4. Presentation attributes */
	/* CSS2 */
	/* Font */
	SPS_READ_PFONTSIZE_IF_UNSET (&style->font_size, repr, "font-size");
	SPS_READ_PENUM_IF_UNSET (&style->font_style, repr, "font-style", enum_font_style, TRUE);
	SPS_READ_PENUM_IF_UNSET (&style->font_variant, repr, "font-variant", enum_font_variant, TRUE);
	SPS_READ_PENUM_IF_UNSET (&style->font_weight, repr, "font-weight", enum_font_weight, TRUE);
	SPS_READ_PENUM_IF_UNSET (&style->font_stretch, repr, "font-stretch", enum_font_stretch, TRUE);
	/* opacity */
	if (!style->opacity.set) {
		val = sp_repr_attr (repr, "opacity");
		if (val) {
			sp_style_read_iscale24 (&style->opacity, val);
		}
	}
	/* fill */
	if (!style->fill.set) {
		val = sp_repr_attr (repr, "fill");
		if (val) {
			sp_style_read_ipaint (&style->fill, val, style, (object) ? SP_OBJECT_DOCUMENT (object) : NULL);
		}
	}
	/* fill-opacity */
	if (!style->fill_opacity.set) {
		val = sp_repr_attr (repr, "fill-opacity");
		if (val) {
			sp_style_read_iscale24 (&style->fill_opacity, val);
		}
	}
	/* fill-rule */
	SPS_READ_PENUM_IF_UNSET (&style->fill_rule, repr, "fill-rule", enum_fill_rule, TRUE);
	/* stroke */
	if (!style->stroke.set) {
		val = sp_repr_attr (repr, "stroke");
		if (val) {
			sp_style_read_ipaint (&style->stroke, val, style, (object) ? SP_OBJECT_DOCUMENT (object) : NULL);
		}
	}
	SPS_READ_PLENGTH_IF_UNSET (&style->stroke_width, repr, "stroke-width");
	SPS_READ_PENUM_IF_UNSET (&style->stroke_linecap, repr, "stroke-linecap", enum_stroke_linecap, TRUE);
	SPS_READ_PENUM_IF_UNSET (&style->stroke_linejoin, repr, "stroke-linejoin", enum_stroke_linejoin, TRUE);
	/* stroke-opacity */
	if (!style->stroke_opacity.set) {
		val = sp_repr_attr (repr, "stroke-opacity");
		if (val) {
			sp_style_read_iscale24 (&style->stroke_opacity, val);
		}
	}
	if (!style->stroke_dasharray_set) {
		val = sp_repr_attr (repr, "stroke-dasharray");
		sp_style_read_dash (&style->stroke_dash, val);
		style->stroke_dasharray_set = TRUE;
	}
	if (!style->stroke_dashoffset_set) {
		/* fixme */
		val = sp_repr_attr (repr, "stroke-dashoffset");
		if (sp_svg_number_read_d (val, &style->stroke_dash.offset)) {
			style->stroke_dashoffset_set = TRUE;
		} else {
			style->stroke_dashoffset_set = FALSE;
		}
	}

	/* font-family */
	if (!style->text_private || !style->text->font_family.set) {
		val = sp_repr_attr (repr, "font-family");
		if (val) {
			if (!style->text_private) sp_style_privatize_text (style);
			sp_style_read_istring (&style->text->font_family, val);
		}
	}
	/* SVG */
	SPS_READ_PENUM_IF_UNSET (&style->text_anchor, repr, "text-anchor", enum_text_anchor, TRUE);
	SPS_READ_PENUM_IF_UNSET (&style->writing_mode, repr, "writing-mode", enum_writing_mode, TRUE);

	/* 5. Merge from parent */
	if (object) {
		if (object->parent) {
			sp_style_merge_from_parent (style, SP_OBJECT_STYLE (object->parent));
		}
	} else {
		if (sp_repr_parent (repr)) {
			SPStyle *parent;
			/* fixme: This is not the prettiest thing (Lauris) */
			parent = sp_style_new ();
			sp_style_read (parent, NULL, sp_repr_parent (repr));
			sp_style_merge_from_parent (style, parent);
			sp_style_unref (parent);
		}
	}
}

void
sp_style_read_from_object (SPStyle *style, SPObject *object)
{
	g_return_if_fail (style != NULL);
	g_return_if_fail (object != NULL);
	g_return_if_fail (SP_IS_OBJECT (object));

	sp_style_read (style, object, SP_OBJECT_REPR (object));
}

void
sp_style_read_from_repr (SPStyle *style, SPRepr *repr)
{
	g_return_if_fail (style != NULL);
	g_return_if_fail (repr != NULL);

	sp_style_read (style, NULL, repr);
}

static void
sp_style_privatize_text (SPStyle *style)
{
	SPTextStyle *text;

	text = style->text;
	style->text = sp_text_style_duplicate_unset (style->text);
	sp_text_style_unref (text);
	style->text_private = TRUE;
}

static void
sp_style_merge_property (SPStyle *style, gint id, const guchar *val)
{
	switch (id) {
	/* CSS2 */
	/* Font */
	case SP_PROP_FONT_FAMILY:
		if (!style->text_private) sp_style_privatize_text (style);
		if (!style->text->font_family.set) {
			sp_style_read_istring (&style->text->font_family, val);
		}
		break;
	case SP_PROP_FONT_SIZE:
		SPS_READ_IFONTSIZE_IF_UNSET (&style->font_size, val);
		break;
	case SP_PROP_FONT_SIZE_ADJUST:
		g_warning ("Unimplemented style property id: %d value: %s", id, val);
		break;
	case SP_PROP_FONT_STYLE:
		SPS_READ_IENUM_IF_UNSET (&style->font_style, val, enum_font_style, TRUE);
		break;
	case SP_PROP_FONT_VARIANT:
		SPS_READ_IENUM_IF_UNSET (&style->font_variant, val, enum_font_variant, TRUE);
		break;
	case SP_PROP_FONT_WEIGHT:
		SPS_READ_IENUM_IF_UNSET (&style->font_weight, val, enum_font_weight, TRUE);
		break;
	case SP_PROP_FONT_STRETCH:
		SPS_READ_IENUM_IF_UNSET (&style->font_stretch, val, enum_font_stretch, TRUE);
		break;
	case SP_PROP_FONT:
		if (!style->text_private) sp_style_privatize_text (style);
		if (!style->text->font.set) {
			if (style->text->font.value) g_free (style->text->font.value);
			style->text->font.value = g_strdup (val);
			style->text->font.set = TRUE;
			style->text->font.inherit = (val && !strcmp (val, "inherit"));
		}
		break;
	/* Text */
	case SP_PROP_DIRECTION:
	case SP_PROP_LETTER_SPACING:
	case SP_PROP_TEXT_DECORATION:
	case SP_PROP_UNICODE_BIDI:
	case SP_PROP_WORD_SPACING:
	/* Misc */
	case SP_PROP_CLIP:
	case SP_PROP_COLOR:
	case SP_PROP_CURSOR:
		g_warning ("Unimplemented style property id: %d value: %s", id, val);
		break;
	case SP_PROP_DISPLAY:
		if (!style->display_set) {
			/* fixme: */
			style->display = strncmp (val, "none", 4);
			style->display_set = TRUE;
		}
		break;
	case SP_PROP_OVERFLOW:
		g_warning ("Unimplemented style property id: %d value: %s", id, val);
		break;
	case SP_PROP_VISIBILITY:
		if (!style->visibility_set) {
			/* fixme: */
			style->visibility = !strncmp (val, "visible", 7);
			style->visibility_set = TRUE;
		}
		break;
	/* SVG */
	/* Clip/Mask */
	case SP_PROP_CLIP_PATH:
	case SP_PROP_CLIP_RULE:
	case SP_PROP_MASK:
		g_warning ("Unimplemented style property id: %d value: %s", id, val);
		break;
	case SP_PROP_OPACITY:
		if (!style->opacity.set) {
			sp_style_read_iscale24 (&style->opacity, val);
		}
		break;
	/* Filter */
	case SP_PROP_ENABLE_BACKGROUND:
	case SP_PROP_FILTER:
	case SP_PROP_FLOOD_COLOR:
	case SP_PROP_FLOOD_OPACITY:
	case SP_PROP_LIGHTING_COLOR:
	/* Gradient */
	case SP_PROP_STOP_COLOR:
	case SP_PROP_STOP_OPACITY:
	/* Interactivity */
	case SP_PROP_POINTER_EVENTS:
	/* Paint */
	case SP_PROP_COLOR_INTERPOLATION:
	case SP_PROP_COLOR_INTERPOLATION_FILTERS:
	case SP_PROP_COLOR_PROFILE:
	case SP_PROP_COLOR_RENDERING:
		g_warning ("Unimplemented style property id: %d value: %s", id, val);
		break;
	case SP_PROP_FILL:
		if (!style->fill.set) {
			sp_style_read_ipaint (&style->fill, val, style, (style->object) ? SP_OBJECT_DOCUMENT (style->object) : NULL);
		}
		break;
	case SP_PROP_FILL_OPACITY:
		if (!style->fill_opacity.set) {
			sp_style_read_iscale24 (&style->fill_opacity, val);
		}
		break;
	case SP_PROP_FILL_RULE:
		if (!style->fill_rule.set) {
			sp_style_read_ienum (&style->fill_rule, val, enum_fill_rule, TRUE);
		}
		break;
	case SP_PROP_IMAGE_RENDERING:
	case SP_PROP_MARKER:
	case SP_PROP_MARKER_END:
	case SP_PROP_MARKER_MID:
	case SP_PROP_MARKER_START:
	case SP_PROP_SHAPE_RENDERING:
		g_warning ("Unimplemented style property id: %d value: %s", id, val);
		break;
	case SP_PROP_STROKE:
		if (!style->stroke.set) {
			sp_style_read_ipaint (&style->stroke, val, style, (style->object) ? SP_OBJECT_DOCUMENT (style->object) : NULL);
		}
		break;
	case SP_PROP_STROKE_WIDTH:
		SPS_READ_ILENGTH_IF_UNSET (&style->stroke_width, val);
		break;
	case SP_PROP_STROKE_DASHARRAY:
		if (!style->stroke_dasharray_set) {
			sp_style_read_dash (&style->stroke_dash, val);
			style->stroke_dasharray_set = TRUE;
		}
		break;
	case SP_PROP_STROKE_DASHOFFSET:
		if (!style->stroke_dashoffset_set) {
			/* fixme */
			if (sp_svg_number_read_d (val, &style->stroke_dash.offset)) {
				style->stroke_dashoffset_set = TRUE;
			} else {
				style->stroke_dashoffset_set = FALSE;
			}
		}
		break;
	case SP_PROP_STROKE_LINECAP:
		if (!style->stroke_linecap.set) {
			sp_style_read_ienum (&style->stroke_linecap, val, enum_stroke_linecap, TRUE);
		}
		break;
	case SP_PROP_STROKE_LINEJOIN:
		if (!style->stroke_linejoin.set) {
			sp_style_read_ienum (&style->stroke_linejoin, val, enum_stroke_linejoin, TRUE);
		}
		break;
	case SP_PROP_STROKE_MITERLIMIT:
		if (!style->stroke_miterlimit.set) {
			sp_style_read_ifloat (&style->stroke_miterlimit, val);
		}
		break;
	case SP_PROP_STROKE_OPACITY:
		if (!style->stroke_opacity.set) {
			sp_style_read_iscale24 (&style->stroke_opacity, val);
		}
		break;
	case SP_PROP_TEXT_RENDERING:
	/* Text */
	case SP_PROP_ALIGNMENT_BASELINE:
	case SP_PROP_BASELINE_SHIFT:
	case SP_PROP_DOMINANT_BASELINE:
	case SP_PROP_GLYPH_ORIENTATION_HORIZONTAL:
	case SP_PROP_GLYPH_ORIENTATION_VERTICAL:
	case SP_PROP_KERNING:
		g_warning ("Unimplemented style property id: %d value: %s", id, val);
		break;
	case SP_PROP_TEXT_ANCHOR:
		SPS_READ_IENUM_IF_UNSET (&style->text_anchor, val, enum_text_anchor, TRUE);
		break;
	case SP_PROP_WRITING_MODE:
		SPS_READ_IENUM_IF_UNSET (&style->writing_mode, val, enum_writing_mode, TRUE);
		break;
	default:
		g_warning ("Invalid style property id: %d value: %s", id, val);
		break;
	}
}

/*
 * Parses style="fill:red;fill-rule:evenodd;" type string
 */

static void
sp_style_merge_from_style_string (SPStyle *style, const guchar *p)
{
	guchar c[BMAX];

	while (*p) {
		const guchar *s, *e;
		gint len, idx;
		while (!isalpha (*p)) {
			if (!*p) return;
			p += 1;
		}
		s = strchr (p, ':');
		if (!s) {
			g_warning ("No separator at style at: %s", p);
			return;
		}
		e = strchr (p, ';');
		if (!e) {
			e = p + strlen (p);
			if (*e) g_warning ("No end marker at style at: %s", p);
		}
		len = MIN (s - p, 4095);
		if (len < 1) {
			g_warning ("Zero length style property at: %s", p);
			return;
		}
		memcpy (c, p, len);
		c[len] = '\0';
		idx = sp_attribute_lookup (c);
		if (idx > 0) {
			len = MIN (e - s - 1, 4095);
			if (len > 0) memcpy (c, s + 1, len);
			c[len] = '\0';
			sp_style_merge_property (style, idx, c);
		} else {
			g_warning ("Unknown style property at: %s", p);
		}
		if (!*e) return;
		p = e + 1;
	}
}

void
sp_style_merge_from_parent (SPStyle *style, SPStyle *parent)
{
	g_return_if_fail (style != NULL);

	if (!parent) return;

	/* CSS2 */
	/* Font */
	/* 'font-size' */
	if (!style->font_size.set || style->font_size.inherit) {
		/* I think inheriting computed value is correct here */
		style->font_size.type = SP_FONT_SIZE_LENGTH;
		style->font_size.computed = parent->font_size.computed;
	} else if (style->font_size.type == SP_FONT_SIZE_LITERAL) {
		static gfloat sizetable[] = {6.0, 8.0, 10.0, 12.0, 14.0, 18.0, 24.0};
		/* fixme: SVG and CSS do no specify clearly, whether we should use user or screen coordinates (Lauris) */
		if (style->font_size.value < SP_CSS_FONT_SIZE_SMALLER) {
			style->font_size.computed = sizetable[style->font_size.value];
		} else if (style->font_size.value == SP_CSS_FONT_SIZE_SMALLER) {
			style->font_size.computed = parent->font_size.computed / 1.2;
		} else if (style->font_size.value == SP_CSS_FONT_SIZE_LARGER) {
			style->font_size.computed = parent->font_size.computed * 1.2;
		} else {
			/* Illegal value */
		}
	} else if (style->font_size.type == SP_FONT_SIZE_PERCENTAGE) {
		/* fixme: SVG and CSS do no specify clearly, whether we should use parent or viewport values here (Lauris) */
#if 0
		g_print ("Parent %g PC %g own %g\n",
			 parent->font_size.computed,
			 SP_F8_16_TO_FLOAT (style->font_size.value),
			 style->font_size.computed);
#endif
		style->font_size.computed = parent->font_size.computed * SP_F8_16_TO_FLOAT (style->font_size.value);
	}
	/* 'font-style' */
	if (!style->font_style.set || style->font_style.inherit) {
		style->font_style.computed = parent->font_style.computed;
	}
	/* 'font-variant' */
	if (!style->font_variant.set || style->font_variant.inherit) {
		style->font_variant.computed = parent->font_variant.computed;
	}
	/* 'font-weight' */
	if (!style->font_weight.set || style->font_weight.inherit) {
		style->font_weight.computed = parent->font_weight.computed;
	} else if (style->font_weight.value == SP_CSS_FONT_WEIGHT_NORMAL) {
		/* fixme: This is unconditional, i.e. happens even if parent not present */
		style->font_weight.computed = SP_CSS_FONT_WEIGHT_400;
	} else if (style->font_weight.value == SP_CSS_FONT_WEIGHT_BOLD) {
		style->font_weight.computed = SP_CSS_FONT_WEIGHT_700;
	} else if (style->font_weight.value == SP_CSS_FONT_WEIGHT_LIGHTER) {
		style->font_weight.computed = CLAMP (parent->font_weight.computed - 1, SP_CSS_FONT_WEIGHT_100, SP_CSS_FONT_WEIGHT_900);
	} else if (style->font_weight.value == SP_CSS_FONT_WEIGHT_DARKER) {
		style->font_weight.computed = CLAMP (parent->font_weight.computed + 1, SP_CSS_FONT_WEIGHT_100, SP_CSS_FONT_WEIGHT_900);
	}
	/* 'font-stretch' */
	if (!style->font_stretch.set || style->font_stretch.inherit) {
		style->font_stretch.computed = parent->font_stretch.computed;
	} else if (style->font_stretch.value == SP_CSS_FONT_STRETCH_NARROWER) {
		style->font_stretch.computed = CLAMP (parent->font_stretch.computed - 1,
						      SP_CSS_FONT_STRETCH_ULTRA_CONDENSED,
						      SP_CSS_FONT_STRETCH_ULTRA_EXPANDED);
	} else if (style->font_stretch.value == SP_CSS_FONT_STRETCH_WIDER) {
		style->font_stretch.computed = CLAMP (parent->font_stretch.computed + 1,
						      SP_CSS_FONT_STRETCH_ULTRA_CONDENSED,
						      SP_CSS_FONT_STRETCH_ULTRA_EXPANDED);
	}
	if (style->opacity.inherit) {
		style->opacity.value = parent->opacity.value;
	}
	if (!style->display_set && parent->display_set) {
		style->display = parent->display;
		style->display_set = TRUE;
	}
	if (!style->visibility_set && parent->visibility_set) {
		style->visibility = parent->visibility;
		style->visibility_set = TRUE;
	}
	if (!style->fill.set || style->fill.inherit) {
		sp_style_merge_ipaint (style, &style->fill, &parent->fill);
	}
	if (!style->fill_opacity.set || style->fill_opacity.inherit) {
		style->fill_opacity.value = parent->fill_opacity.value;
	}
	if (!style->fill_rule.set || style->fill_rule.inherit) {
		style->fill_rule.computed = parent->fill_rule.computed;
	}
	/* Stroke */
	if (!style->stroke.set || style->stroke.inherit) {
		sp_style_merge_ipaint (style, &style->stroke, &parent->stroke);
	}
	if (!style->stroke_width.set || style->stroke_width.inherit) {
		style->stroke_width.unit = parent->stroke_width.unit;
		style->stroke_width.value = parent->stroke_width.value;
		style->stroke_width.computed = parent->stroke_width.computed;
	} else if (style->stroke_width.unit == SP_CSS_UNIT_EM) {
		/* fixme: Must have sure font size is updated BEFORE us */
		style->stroke_width.computed = style->font_size.computed;
	} else if (style->stroke_width.unit == SP_CSS_UNIT_EX) {
		/* fixme: Must have sure font size is updated BEFORE us */
		/* fixme: Real x height - but should this go to item? (Lauris) */
		style->stroke_width.computed = style->font_size.computed * 0.5;
	}
	if (!style->stroke_linecap.set || style->stroke_linecap.inherit) {
		style->stroke_linecap.computed = parent->stroke_linecap.computed;
	}
	if (!style->stroke_linejoin.set || style->stroke_linejoin.inherit) {
		style->stroke_linejoin.computed = parent->stroke_linejoin.computed;
	}
	if (!style->stroke_miterlimit.set || style->stroke_miterlimit.inherit) {
		style->stroke_miterlimit.value = parent->stroke_miterlimit.value;
	}
	if (!style->stroke_dasharray_set && parent->stroke_dasharray_set) {
		style->stroke_dash.n_dash = parent->stroke_dash.n_dash;
		if (style->stroke_dash.n_dash > 0) {
			style->stroke_dash.dash = g_new (gdouble, style->stroke_dash.n_dash);
			memcpy (style->stroke_dash.dash, parent->stroke_dash.dash, style->stroke_dash.n_dash * sizeof (gdouble));
		}
		style->stroke_dasharray_set = TRUE;
	}
	if (!style->stroke_dashoffset_set && parent->stroke_dashoffset_set) {
		style->stroke_dash.offset = parent->stroke_dash.offset;
		style->stroke_dashoffset_set = TRUE;
	}
	if (!style->stroke_opacity.set || style->stroke_opacity.inherit) {
		style->stroke_opacity.value = parent->stroke_opacity.value;
	}
	/* 'text-anchor' */
	if (!style->text_anchor.set || style->text_anchor.inherit) {
		style->text_anchor.computed = parent->text_anchor.computed;
	}
	if (!style->writing_mode.set || style->writing_mode.inherit) {
		style->writing_mode.computed = parent->writing_mode.computed;
	}

	if (style->text && parent->text) {
		if (!style->text->font_family.set || style->text->font_family.inherit) {
			if (style->text->font_family.value) g_free (style->text->font_family.value);
			style->text->font_family.value = g_strdup (parent->text->font_family.value);
		}
	}
}

static void
sp_style_paint_server_release (SPPaintServer *server, SPStyle *style)
{
	if ((style->fill.type == SP_PAINT_TYPE_PAINTSERVER) && (server == style->fill.value.server)) {
		sp_style_paint_clear (style, &style->fill, TRUE, FALSE);
	} else if ((style->stroke.type == SP_PAINT_TYPE_PAINTSERVER) && (server == style->stroke.value.server)) {
		sp_style_paint_clear (style, &style->stroke, TRUE, FALSE);
	} else {
		g_assert_not_reached ();
	}
}

static void
sp_style_paint_server_modified (SPPaintServer *server, guint flags, SPStyle *style)
{
	if ((style->fill.type == SP_PAINT_TYPE_PAINTSERVER) && (server == style->fill.value.server)) {
		if (style->object) {
			/* fixme: I do not know, whether it is optimal - we are forcing reread of everything (Lauris) */
			/* fixme: We have to use object_modified flag, because parent flag is only available downstreams */
			sp_object_request_modified (style->object, SP_OBJECT_MODIFIED_FLAG | SP_OBJECT_STYLE_MODIFIED_FLAG);
		}
	} else if ((style->stroke.type == SP_PAINT_TYPE_PAINTSERVER) && (server == style->stroke.value.server)) {
		if (style->object) {
			/* fixme: */
			sp_object_request_modified (style->object, SP_OBJECT_MODIFIED_FLAG | SP_OBJECT_STYLE_MODIFIED_FLAG);
		}
	} else {
		g_assert_not_reached ();
	}
}

static void
sp_style_merge_ipaint (SPStyle *style, SPIPaint *paint, SPIPaint *parent)
{
	sp_style_paint_clear (style, paint, TRUE, FALSE);
	paint->type = parent->type;
	switch (paint->type) {
	case SP_PAINT_TYPE_COLOR:
		sp_color_copy (&paint->value.color, &parent->value.color);
		break;
	case SP_PAINT_TYPE_PAINTSERVER:
		paint->value.server = parent->value.server;
		if (paint->value.server) {
			sp_object_href (SP_OBJECT (paint->value.server), style);
			g_signal_connect (G_OBJECT (paint->value.server), "release",
					  G_CALLBACK (sp_style_paint_server_release), style);
			g_signal_connect (G_OBJECT (paint->value.server), "modified",
					  G_CALLBACK (sp_style_paint_server_modified), style);
		}
		break;
	case SP_PAINT_TYPE_NONE:
		break;
	default:
		g_assert_not_reached ();
		break;
	}
}

/* fixme: Write real thing */

guchar *
sp_style_write_string (SPStyle *style)
{
	guchar c[BMAX], *p;

	g_return_val_if_fail (style != NULL, NULL);

	p = c;
	*p = '\0';

	p += sp_style_write_ifontsize (p, c + BMAX - p, "font-size", &style->font_size, NULL, SP_STYLE_FLAG_IFSET);
	p += sp_style_write_ienum (p, c + BMAX - p, "font-style", enum_font_style, &style->font_style, NULL, SP_STYLE_FLAG_IFSET);
	p += sp_style_write_ienum (p, c + BMAX - p, "font-variant", enum_font_variant, &style->font_variant, NULL, SP_STYLE_FLAG_IFSET);
	p += sp_style_write_ienum (p, c + BMAX - p, "font-weight", enum_font_weight, &style->font_weight, NULL, SP_STYLE_FLAG_IFSET);
	p += sp_style_write_ienum (p, c + BMAX - p, "font-stretch", enum_font_stretch, &style->font_stretch, NULL, SP_STYLE_FLAG_IFSET);

	/* fixme: Per type methods need default flag too */
	if (style->opacity.set && style->opacity.value != SP_SCALE24_MAX) {
		p += sp_style_write_iscale24 (p, c + BMAX - p, "opacity", &style->opacity, NULL, SP_STYLE_FLAG_IFSET);
	}
	p += sp_style_write_ipaint (p, c + BMAX - p, "fill", &style->fill, NULL, SP_STYLE_FLAG_IFSET);
	p += sp_style_write_iscale24 (p, c + BMAX - p, "fill-opacity", &style->fill_opacity, NULL, SP_STYLE_FLAG_IFSET);
	p += sp_style_write_ienum (p, c + BMAX - p, "fill-rule", enum_fill_rule, &style->fill_rule, NULL, SP_STYLE_FLAG_IFSET);
	p += sp_style_write_ipaint (p, c + BMAX - p, "stroke", &style->stroke, NULL, SP_STYLE_FLAG_IFSET);
	p += sp_style_write_ilength (p, c + BMAX - p, "stroke-width", &style->stroke_width, NULL, SP_STYLE_FLAG_IFSET);
	p += sp_style_write_ienum (p, c + BMAX - p, "stroke-linecap", enum_stroke_linecap, &style->stroke_linecap, NULL, SP_STYLE_FLAG_IFSET);
	p += sp_style_write_ienum (p, c + BMAX - p, "stroke-linejoin", enum_stroke_linejoin, &style->stroke_linejoin, NULL, SP_STYLE_FLAG_IFSET);
	p += sp_style_write_ifloat (p, c + BMAX - p, "stroke-miterlimit", &style->stroke_miterlimit, NULL, SP_STYLE_FLAG_IFSET);
	/* fixme: */
	if (style->stroke_dasharray_set) {
		if (style->stroke_dash.n_dash && style->stroke_dash.dash) {
			gint i;
			p += g_snprintf (p, c + BMAX - p, "stroke-dasharray:");
			for (i = 0; i < style->stroke_dash.n_dash; i++) {
				p += g_snprintf (p, c + BMAX - p, "%g ", style->stroke_dash.dash[i]);
			}
			p += g_snprintf (p, c + BMAX - p, ";");
		}
	}
	/* fixme: */
	if (style->stroke_dashoffset_set) {
		p += g_snprintf (p, c + BMAX - p, "stroke-dashoffset:%g;", style->stroke_dash.offset);
	}
	p += sp_style_write_iscale24 (p, c + BMAX - p, "stroke-opacity", &style->stroke_opacity, NULL, SP_STYLE_FLAG_IFSET);

	/* fixme: */
	sp_text_style_write (p, c + BMAX - p, style->text);

	p += sp_style_write_ienum (p, c + BMAX - p, "text-anchor", enum_text_anchor, &style->text_anchor, NULL, SP_STYLE_FLAG_IFSET);
	p += sp_style_write_ienum (p, c + BMAX - p, "writing-mode", enum_writing_mode, &style->writing_mode, NULL, SP_STYLE_FLAG_IFSET);

	return g_strdup (c);
}

#define STYLE_BUF_MAX

guchar *
sp_style_write_difference (SPStyle *from, SPStyle *to)
{
	guchar c[BMAX], *p;

	g_return_val_if_fail (from != NULL, NULL);
	g_return_val_if_fail (to != NULL, NULL);

	p = c;
	*p = '\0';

	p += sp_style_write_ifontsize (p, c + BMAX - p, "font-size", &from->font_size, &to->font_size, SP_STYLE_FLAG_IFDIFF);
	p += sp_style_write_ienum (p, c + BMAX - p, "font-style", enum_font_style, &from->font_style, &to->font_style, SP_STYLE_FLAG_IFDIFF);
	p += sp_style_write_ienum (p, c + BMAX - p, "font-variant", enum_font_variant, &from->font_variant, &to->font_variant, SP_STYLE_FLAG_IFDIFF);
	p += sp_style_write_ienum (p, c + BMAX - p, "font-weight", enum_font_weight, &from->font_weight, &to->font_weight, SP_STYLE_FLAG_IFDIFF);
	p += sp_style_write_ienum (p, c + BMAX - p, "font-stretch", enum_font_stretch, &from->font_stretch, &to->font_stretch, SP_STYLE_FLAG_IFDIFF);

	/* fixme: Per type methods need default flag too */
	if (from->opacity.set && from->opacity.value != SP_SCALE24_MAX) {
		p += sp_style_write_iscale24 (p, c + BMAX - p, "opacity", &from->opacity, &to->opacity, SP_STYLE_FLAG_IFDIFF);
	}
	p += sp_style_write_ipaint (p, c + BMAX - p, "fill", &from->fill, &to->fill, SP_STYLE_FLAG_IFDIFF);
	p += sp_style_write_iscale24 (p, c + BMAX - p, "fill-opacity", &from->fill_opacity, &to->fill_opacity, SP_STYLE_FLAG_IFDIFF);
	p += sp_style_write_ienum (p, c + BMAX - p, "fill-rule", enum_fill_rule, &from->fill_rule, &to->fill_rule, SP_STYLE_FLAG_IFDIFF);
	p += sp_style_write_ipaint (p, c + BMAX - p, "stroke", &from->stroke, &to->stroke, SP_STYLE_FLAG_IFDIFF);
	p += sp_style_write_ilength (p, c + BMAX - p, "stroke-width", &from->stroke_width, &to->stroke_width, SP_STYLE_FLAG_IFDIFF);
	p += sp_style_write_ienum (p, c + BMAX - p, "stroke-linecap", enum_stroke_linecap,
				   &from->stroke_linecap, &to->stroke_linecap, SP_STYLE_FLAG_IFDIFF);
	p += sp_style_write_ienum (p, c + BMAX - p, "stroke-linejoin", enum_stroke_linejoin,
				   &from->stroke_linejoin, &to->stroke_linejoin, SP_STYLE_FLAG_IFDIFF);
	p += sp_style_write_ifloat (p, c + BMAX - p, "stroke-miterlimit",
				    &from->stroke_miterlimit, &to->stroke_miterlimit, SP_STYLE_FLAG_IFDIFF);
	/* fixme: */
	if (from->stroke_dasharray_set) {
		if (from->stroke_dash.n_dash && from->stroke_dash.dash) {
			gint i;
			p += g_snprintf (p, c + BMAX - p, "stroke-dasharray:");
			for (i = 0; i < from->stroke_dash.n_dash; i++) {
				p += g_snprintf (p, c + BMAX - p, "%g ", from->stroke_dash.dash[i]);
			}
			p += g_snprintf (p, c + BMAX - p, ";");
		}
	}
	/* fixme: */
	if (from->stroke_dashoffset_set) {
		p += g_snprintf (p, c + BMAX - p, "stroke-dashoffset:%g;", from->stroke_dash.offset);
	}
	p += sp_style_write_iscale24 (p, c + BMAX - p, "stroke-opacity", &from->stroke_opacity, &to->stroke_opacity, SP_STYLE_FLAG_IFDIFF);

	/* fixme: */
	p += sp_text_style_write (p, c + BMAX - p, from->text);

	p += sp_style_write_ienum (p, c + BMAX - p, "text-anchor", enum_text_anchor, &from->text_anchor, &to->text_anchor, SP_STYLE_FLAG_IFDIFF);
	p += sp_style_write_ienum (p, c + BMAX - p, "writing-mode", enum_writing_mode, &from->writing_mode, &to->writing_mode, SP_STYLE_FLAG_IFDIFF);


	return g_strdup (c);
}

static void
sp_style_clear (SPStyle *style)
{
	SPObject *object;
	gint refcount;
	SPTextStyle *text;
	unsigned int text_private;

	g_return_if_fail (style != NULL);

	sp_style_paint_clear (style, &style->fill, TRUE, FALSE);
	sp_style_paint_clear (style, &style->stroke, TRUE, FALSE);
	if (style->stroke_dash.dash) {
		g_free (style->stroke_dash.dash);
	}

	/* fixme: Do that text manipulation via parents */
	object = style->object;
	refcount = style->refcount;
	text = style->text;
	text_private = style->text_private;
	memset (style, 0, sizeof (SPStyle));
	style->refcount = refcount;
	style->object = object;
	style->text = text;
	style->text_private = text_private;
	/* fixme: */
	style->text->font.set = FALSE;
	style->text->font_family.set = FALSE;

	style->font_size.set = FALSE;
	style->font_size.type = SP_FONT_SIZE_LITERAL;
	style->font_size.value = SP_CSS_FONT_SIZE_MEDIUM;
	style->font_size.computed = 12.0;
	style->font_style.set = FALSE;
	style->font_style.computed = SP_CSS_FONT_STYLE_NORMAL;
	style->font_variant.set = FALSE;
	style->font_variant.value = SP_CSS_FONT_VARIANT_NORMAL;
	style->font_weight.set = FALSE;
	style->font_weight.value = SP_CSS_FONT_WEIGHT_400;
	style->font_stretch.set = FALSE;
	style->font_stretch.value = SP_CSS_FONT_STRETCH_NORMAL;

	style->opacity.value = SP_SCALE24_MAX;
	style->display = TRUE;
	style->visibility = TRUE;

	style->fill.type = SP_PAINT_TYPE_COLOR;
	sp_color_set_rgb_float (&style->fill.value.color, 0.0, 0.0, 0.0);
	style->fill_opacity.value = SP_SCALE24_MAX;
	style->fill_rule.value = SP_WIND_RULE_NONZERO;

	style->stroke.set = FALSE;
	style->stroke.type = SP_PAINT_TYPE_NONE;
	sp_color_set_rgb_float (&style->stroke.value.color, 0.0, 0.0, 0.0);
	style->stroke_width.set = FALSE;
	style->stroke_width.unit = SP_CSS_UNIT_NONE;
	style->stroke_width.computed = 1.0;
	style->stroke_linecap.value = SP_STROKE_LINECAP_BUTT;
	style->stroke_linejoin.value = SP_STROKE_LINEJOIN_MITER;
	style->stroke_miterlimit.value = 4.0;
	style->stroke_dash.n_dash = 0;
	style->stroke_dash.dash = NULL;
	style->stroke_dash.offset = 0.0;
	style->stroke_opacity.value = SP_SCALE24_MAX;

	style->text_anchor.set = FALSE;
	style->text_anchor.computed = SP_CSS_TEXT_ANCHOR_START;
	style->writing_mode.set = FALSE;
	style->writing_mode.computed = SP_CSS_WRITING_MODE_LR;
}

static void
sp_style_read_dash (ArtVpathDash *dash, const guchar *str)
{
	gint n_dash;
	gdouble d[64];
	guchar *e;

	n_dash = 0;
	e = NULL;

	while (e != str && n_dash < 64) {
		d[n_dash] = strtod (str, (char **) &e);
		if (e != str) {
			n_dash += 1;
			str = e;
		}
		while (str && *str && !isalnum (*str)) str += 1;
	}

	if (n_dash > 0) {
		dash->dash = g_new (gdouble, n_dash);
		memcpy (dash->dash, d, sizeof (gdouble) * n_dash);
		dash->n_dash = n_dash;
	}
}

void
sp_style_set_fill_color_rgba (SPStyle *style, gfloat r, gfloat g, gfloat b, gfloat a, unsigned int fill_set, unsigned int opacity_set)
{
	g_return_if_fail (style != NULL);

	sp_style_paint_clear (style, &style->fill, TRUE, FALSE);

	style->fill.set = fill_set;
	style->fill.inherit = FALSE;
	style->fill.type = SP_PAINT_TYPE_COLOR;
	sp_color_set_rgb_float (&style->fill.value.color, r, g, b);
	style->fill_opacity.set = opacity_set;
	style->fill_opacity.inherit = FALSE;
	style->fill_opacity.value = SP_SCALE24_FROM_FLOAT (a);

	if (style->object) {
		sp_object_request_modified (style->object, SP_OBJECT_MODIFIED_FLAG | SP_OBJECT_STYLE_MODIFIED_FLAG);
	}
}

void
sp_style_set_fill_color_cmyka (SPStyle *style, gfloat c, gfloat m, gfloat y, gfloat k, gfloat a, unsigned int fill_set, unsigned int opacity_set)
{
	g_return_if_fail (style != NULL);

	sp_style_paint_clear (style, &style->fill, TRUE, FALSE);

	style->fill.set = fill_set;
	style->fill.inherit = FALSE;
	style->fill.type = SP_PAINT_TYPE_COLOR;
	sp_color_set_cmyk_float (&style->fill.value.color, c, m, y, k);
	style->fill_opacity.set = opacity_set;
	style->fill_opacity.inherit = FALSE;
	style->fill_opacity.value = SP_SCALE24_FROM_FLOAT (a);

	if (style->object) {
		sp_object_request_modified (style->object, SP_OBJECT_MODIFIED_FLAG | SP_OBJECT_STYLE_MODIFIED_FLAG);
	}
}

void
sp_style_set_stroke_color_rgba (SPStyle *style, gfloat r, gfloat g, gfloat b, gfloat a, unsigned int stroke_set, unsigned int opacity_set)
{
	g_return_if_fail (style != NULL);

	sp_style_paint_clear (style, &style->stroke, TRUE, FALSE);

	style->stroke.set = stroke_set;
	style->stroke.inherit = FALSE;
	style->stroke.type = SP_PAINT_TYPE_COLOR;
	sp_color_set_rgb_float (&style->stroke.value.color, r, g, b);
	style->stroke_opacity.set = opacity_set;
	style->stroke_opacity.inherit = FALSE;
	style->stroke_opacity.value = SP_SCALE24_FROM_FLOAT (a);

	if (style->object) {
		sp_object_request_modified (style->object, SP_OBJECT_MODIFIED_FLAG | SP_OBJECT_STYLE_MODIFIED_FLAG);
	}
}

void
sp_style_set_stroke_color_cmyka (SPStyle *style, gfloat c, gfloat m, gfloat y, gfloat k, gfloat a, unsigned int stroke_set, unsigned int opacity_set)
{
	g_return_if_fail (style != NULL);

	sp_style_paint_clear (style, &style->stroke, TRUE, FALSE);

	style->stroke.set = stroke_set;
	style->stroke.inherit = FALSE;
	style->stroke.type = SP_PAINT_TYPE_COLOR;
	sp_color_set_cmyk_float (&style->stroke.value.color, c, m, y, k);
	style->stroke_opacity.set = opacity_set;
	style->stroke_opacity.inherit = FALSE;
	style->stroke_opacity.value = SP_SCALE24_FROM_FLOAT (a);

	if (style->object) {
		sp_object_request_modified (style->object, SP_OBJECT_MODIFIED_FLAG | SP_OBJECT_STYLE_MODIFIED_FLAG);
	}
}

void
sp_style_set_opacity (SPStyle *style, gfloat opacity, unsigned int opacity_set)
{
	g_return_if_fail (style != NULL);

	style->opacity.set = opacity_set;
	style->opacity.inherit = FALSE;
	style->opacity.value = SP_SCALE24_FROM_FLOAT (opacity);

	if (style->object) {
		sp_object_request_modified (style->object, SP_OBJECT_MODIFIED_FLAG | SP_OBJECT_STYLE_MODIFIED_FLAG);
	}
}

/* SPTextStyle operations */

static SPTextStyle *
sp_text_style_new (void)
{
	SPTextStyle *ts;

	ts = g_new0 (SPTextStyle, 1);

	ts->refcount = 1;

	sp_text_style_clear (ts);

	ts->font.value = g_strdup ("Bitstream Cyberbit 12");
	ts->font_family.value = g_strdup ("Bitstream Cyberbit");

	return ts;
}

static void
sp_text_style_clear (SPTextStyle *ts)
{
	ts->font.set = FALSE;
	ts->font_family.set = FALSE;
	ts->font_size_adjust_set = FALSE;

	ts->direction_set = FALSE;
	ts->text_decoration_set = FALSE;
	ts->unicode_bidi_set = FALSE;
}

static SPTextStyle *
sp_text_style_unref (SPTextStyle *st)
{
	st->refcount -= 1;

	if (st->refcount < 1) {
		if (st->font.value) g_free (st->font.value);
		if (st->font_family.value) g_free (st->font_family.value);
		g_free (st);
	}

	return NULL;
}

static SPTextStyle *
sp_text_style_duplicate_unset (SPTextStyle *st)
{
	SPTextStyle *nt;

	nt = g_new0 (SPTextStyle, 1);

	nt->refcount = 1;

	nt->font.value = g_strdup (st->font.value);
	nt->font_family.value = g_strdup (st->font_family.value);

	return nt;
}

static guint
sp_text_style_write (guchar *p, guint len, SPTextStyle *st)
{
	gint d;

	d = 0;

	if (st->font_family.set) {
		d += sp_style_write_istring (p + d, len - d, "font-family", &st->font_family, NULL, SP_STYLE_FLAG_IFSET);
	}

	return d;
}

static void
sp_style_read_ifloat (SPIFloat *val, const guchar *str)
{
	if (!strcmp (str, "inherit")) {
		val->set = TRUE;
		val->inherit = TRUE;
	} else {
		gfloat value;
		if (sp_svg_number_read_f (str, &value)) {
			val->set = TRUE;
			val->inherit = FALSE;
			val->value = value;
		}
	}
}

static void
sp_style_read_iscale24 (SPIScale24 *val, const guchar *str)
{
	if (!strcmp (str, "inherit")) {
		val->set = TRUE;
		val->inherit = TRUE;
	} else {
		gfloat value;
		if (sp_svg_number_read_f (str, &value)) {
			val->set = TRUE;
			val->inherit = FALSE;
			val->value = SP_SCALE24_FROM_FLOAT (value);
			val->value = CLAMP (val->value, 0, SP_SCALE24_MAX);
		}
	}
}

static void
sp_style_read_ienum (SPIEnum *val, const guchar *str, const SPStyleEnum *dict, unsigned int inherit)
{
	if (inherit && !strcmp (str, "inherit")) {
		val->set = TRUE;
		val->inherit = TRUE;
	} else {
		gint i;
		for (i = 0; dict[i].key; i++) {
			if (!strcmp (str, dict[i].key)) {
				val->set = TRUE;
				val->inherit = FALSE;
				val->value = dict[i].value;
				/* Save copying for values not needing it */
				val->computed = val->value;
				break;
			}
		}
	}
}

static void
sp_style_read_istring (SPIString *val, const guchar *str)
{
	if (val->value) g_free (val->value);

	if (!strcmp (str, "inherit")) {
		val->set = TRUE;
		val->inherit = TRUE;
		val->value = NULL;
	} else {
		val->set = TRUE;
		val->inherit = FALSE;
		val->value = g_strdup (str);
	}
}

static void
sp_style_read_ilength (SPILength *val, const guchar *str)
{
	if (!strcmp (str, "inherit")) {
		val->set = TRUE;
		val->inherit = TRUE;
	} else {
		gdouble value;
		gchar *e;
		/* fixme: Move this to standard place (Lauris) */
		value = strtod (str, &e);
		if ((const guchar *) e != str) {
			if (!*e) {
				/* Userspace */
				val->unit = SP_CSS_UNIT_NONE;
				val->computed = value;
			} else if (!strcmp (e, "px")) {
				/* Userspace */
				val->unit = SP_CSS_UNIT_PX;
				val->computed = value;
			} else if (!strcmp (e, "pt")) {
				/* Userspace * 1.25 */
				val->unit = SP_CSS_UNIT_PT;
				val->computed = value * 1.25;
			} else if (!strcmp (e, "pc")) {
				/* Userspace * 15 */
				val->unit = SP_CSS_UNIT_PC;
				val->computed = value * 15.0;
			} else if (!strcmp (e, "mm")) {
				/* Userspace * 3.543307 */
				val->unit = SP_CSS_UNIT_MM;
				val->computed = value * 3.543307;
			} else if (!strcmp (e, "cm")) {
				/* Userspace * 35.43307 */
				val->unit = SP_CSS_UNIT_CM;
				val->computed = value * 35.43307;
			} else if (!strcmp (e, "in")) {
				/* Userspace * 90 */
				val->unit = SP_CSS_UNIT_IN;
				val->computed = value * 90.0;
			} else if (!strcmp (e, "em")) {
				/* EM square */
				val->unit = SP_CSS_UNIT_EM;
				val->value = value;
			} else if (!strcmp (e, "ex")) {
				/* ex square */
				val->unit = SP_CSS_UNIT_EX;
				val->value = value;
			} else if (!strcmp (e, "%")) {
				/* Percentage */
				val->unit = SP_CSS_UNIT_PERCENT;
				val->value = value * 0.01;
				return;
			} else {
				/* Invalid */
				return;
			}
			val->set = TRUE;
			val->inherit = FALSE;
		}
	}
}

static void
sp_style_read_ipaint (SPIPaint *paint, const guchar *str, SPStyle *style, SPDocument *document)
{
	if (!strcmp (str, "inherit")) {
		paint->set = TRUE;
		paint->inherit = TRUE;
	} else {
		guint32 color;
		if (!strncmp (str, "url", 3)) {
			if (document) {
				SPObject *ps;
				ps = sp_uri_reference_resolve (document, str);
				if (!ps || !SP_IS_PAINT_SERVER (ps)) {
					paint->type = SP_PAINT_TYPE_NONE;
					return;
				}
				paint->type = SP_PAINT_TYPE_PAINTSERVER;
				paint->value.server = SP_PAINT_SERVER (ps);
				sp_object_href (SP_OBJECT (paint->value.server), style);
				g_signal_connect (G_OBJECT (paint->value.server), "release",
						  G_CALLBACK (sp_style_paint_server_release), style);
				g_signal_connect (G_OBJECT (paint->value.server), "modified",
						  G_CALLBACK (sp_style_paint_server_modified), style);
				paint->set = TRUE;
				paint->inherit = FALSE;
				return;
			} else {
				paint->set = FALSE;
				return;
			}
		} else if (!strncmp (str, "none", 4)) {
			paint->type = SP_PAINT_TYPE_NONE;
			paint->set = TRUE;
			paint->inherit = FALSE;
			return;
		}

		paint->type = SP_PAINT_TYPE_COLOR;
		color = sp_color_get_rgba32_ualpha (&paint->value.color, 0);
		color = sp_svg_read_color (str, color);
		sp_color_set_rgb_rgba32 (&paint->value.color, color);
		paint->set = TRUE;
		paint->inherit = FALSE;
	}
}

static void
sp_style_read_ifontsize (SPIFontSize *val, const guchar *str)
{
	if (!strcmp (str, "inherit")) {
		val->set = TRUE;
		val->inherit = TRUE;
	} else if ((*str == 'x') || (*str == 's') || (*str == 'm') || (*str == 'l')) {
		gint i;
		for (i = 0; enum_font_size[i].key; i++) {
			if (!strcmp (str, enum_font_size[i].key)) {
				val->set = TRUE;
				val->inherit = FALSE;
				val->type = SP_FONT_SIZE_LITERAL;
				val->value = enum_font_size[i].value;
				return;
			}
		}
		/* Invalid */
		return;
	} else {
		gdouble value;
		gchar *e;
		/* fixme: Move this to standard place (Lauris) */
		value = strtod (str, &e);
		if ((const guchar *) e != str) {
			if (!*e) {
				/* Userspace */
			} else if (!strcmp (e, "px")) {
				/* Userspace */
			} else if (!strcmp (e, "pt")) {
				/* Userspace * 1.25 */
				value *= 1.25;
			} else if (!strcmp (e, "pc")) {
				/* Userspace * 15 */
				value *= 15.0;
			} else if (!strcmp (e, "mm")) {
				/* Userspace * 3.543307 */
				value *= 3.543307;
			} else if (!strcmp (e, "cm")) {
				/* Userspace * 35.43307 */
				value *= 35.43307;
			} else if (!strcmp (e, "in")) {
				/* Userspace * 90 */
				value *= 90;
			} else if (!strcmp (e, "%")) {
				/* Percentage */
				val->set = TRUE;
				val->inherit = FALSE;
				val->type = SP_FONT_SIZE_PERCENTAGE;
				val->value = SP_F8_16_FROM_FLOAT (value / 100.0);
				return;
			} else {
				/* Invalid */
				return;
			}
			/* Length */
			val->set = TRUE;
			val->inherit = FALSE;
			val->type = SP_FONT_SIZE_LENGTH;
			val->computed = value;
			return;
		}
	}
}

#if 0
static void
sp_style_read_pfloat (SPIFloat *val, SPRepr *repr, const guchar *key)
{
	const guchar *str;
	str = sp_repr_attr (repr, key);
	if (str) {
		sp_style_read_ifloat (val, str);
	}
}
#endif

static void
sp_style_read_penum (SPIEnum *val, SPRepr *repr, const guchar *key, const SPStyleEnum *dict, unsigned int inherit)
{
	const guchar *str;
	str = sp_repr_attr (repr, key);
	if (str) {
		sp_style_read_ienum (val, str, dict, inherit);
	}
}

static void
sp_style_read_plength (SPILength *val, SPRepr *repr, const guchar *key)
{
	const guchar *str;
	str = sp_repr_attr (repr, key);
	if (str) {
		sp_style_read_ilength (val, str);
	}
}

static void
sp_style_read_pfontsize (SPIFontSize *val, SPRepr *repr, const guchar *key)
{
	const guchar *str;
	str = sp_repr_attr (repr, key);
	if (str) {
		sp_style_read_ifontsize (val, str);
	}
}

static gint
sp_style_write_ifloat (guchar *p, gint len, const guchar *key, SPIFloat *val, SPIFloat *base, guint flags)
{
	if (((flags & SP_STYLE_FLAG_IFSET) && val->set) ||
	    ((flags & SP_STYLE_FLAG_IFDIFF) && (val->value != base->value))) {
		if (val->inherit) {
			return g_snprintf (p, len, "%s:inherit;", key);
		} else {
			return g_snprintf (p, len, "%s:%g;", key, val->value);
		}
	}
	return 0;
}

static gint
sp_style_write_iscale24 (guchar *p, gint len, const guchar *key, SPIScale24 *val, SPIScale24 *base, guint flags)
{
	if (((flags & SP_STYLE_FLAG_IFSET) && val->set) ||
	    ((flags & SP_STYLE_FLAG_IFDIFF) && (val->value != base->value))) {
		if (val->inherit) {
			return g_snprintf (p, len, "%s:inherit;", key);
		} else {
			return g_snprintf (p, len, "%s:%g;", key, SP_SCALE24_TO_FLOAT (val->value));
		}
	}
	return 0;
}

static gint
sp_style_write_ienum (guchar *p, gint len, const guchar *key, const SPStyleEnum *dict, SPIEnum *val, SPIEnum *base, guint flags)
{
	if (((flags & SP_STYLE_FLAG_IFSET) && val->set) ||
	    ((flags & SP_STYLE_FLAG_IFDIFF) && (val->computed != base->computed))) {
		gint i;
		for (i = 0; dict[i].key; i++) {
			if (dict[i].value == val->value) {
				return g_snprintf (p, len, "%s:%s;", key, dict[i].key);
			}
		}
	}
	return 0;
}

static gint
sp_style_write_istring (guchar *p, gint len, const guchar *key, SPIString *val, SPIString *base, guint flags)
{
	if (((flags & SP_STYLE_FLAG_IFSET) && val->set) ||
	    ((flags & SP_STYLE_FLAG_IFDIFF) && strcmp (val->value, base->value))) {
		if (val->inherit) {
			return g_snprintf (p, len, "%s:inherit;", key);
		} else {
			return g_snprintf (p, len, "%s:%s;", key, val->value);
		}
	}
	return 0;
}

static unsigned int
sp_length_differ (SPILength *a, SPILength *b)
{
	if (a->unit != b->unit) {
		if (a->unit == SP_CSS_UNIT_EM) return TRUE;
		if (a->unit == SP_CSS_UNIT_EX) return TRUE;
		if (a->unit == SP_CSS_UNIT_PERCENT) return TRUE;
		if (b->unit == SP_CSS_UNIT_EM) return TRUE;
		if (b->unit == SP_CSS_UNIT_EX) return TRUE;
		if (b->unit == SP_CSS_UNIT_PERCENT) return TRUE;
	}

	return (a->computed != b->computed);
}

static gint
sp_style_write_ilength (guchar *p, gint len, const guchar *key, SPILength *val, SPILength *base, guint flags)
{
	if (((flags & SP_STYLE_FLAG_IFSET) && val->set) ||
	    ((flags & SP_STYLE_FLAG_IFDIFF) && sp_length_differ (val, base))) {
		if (val->inherit) {
			return g_snprintf (p, len, "%s:inherit;", key);
		} else {
			switch (val->unit) {
			case SP_CSS_UNIT_NONE:
				return g_snprintf (p, len, "%s:%g;", key, val->computed);
				break;
			case SP_CSS_UNIT_PX:
				return g_snprintf (p, len, "%s:%gpx;", key, val->computed);
				break;
			case SP_CSS_UNIT_PT:
				return g_snprintf (p, len, "%s:%gpt;", key, val->computed / 1.25);
				break;
			case SP_CSS_UNIT_PC:
				return g_snprintf (p, len, "%s:%gpc;", key, val->computed / 15.0);
				break;
			case SP_CSS_UNIT_MM:
				return g_snprintf (p, len, "%s:%gmm;", key, val->computed / 3.543307);
				break;
			case SP_CSS_UNIT_CM:
				return g_snprintf (p, len, "%s:%gmm;", key, val->computed / 35.43307);
				break;
			case SP_CSS_UNIT_IN:
				return g_snprintf (p, len, "%s:%gin;", key, val->computed / 90.0);
				break;
			case SP_CSS_UNIT_EM:
				return g_snprintf (p, len, "%s:%gem;", key, val->value);
				break;
			case SP_CSS_UNIT_EX:
				return g_snprintf (p, len, "%s:%gex;", key, val->value);
				break;
			case SP_CSS_UNIT_PERCENT:
				return g_snprintf (p, len, "%s:%g%%;", key, val->value);
				break;
			default:
				/* Invalid */
				break;
			}
		}
	}
	return 0;
}

static unsigned int
sp_paint_differ (SPIPaint *a, SPIPaint *b)
{
	if (a->type != b->type) return TRUE;
	if (a->type == SP_PAINT_TYPE_COLOR) return !sp_color_is_equal (&a->value.color, &b->value.color);
	if (a->type == SP_PAINT_TYPE_PAINTSERVER) return (a->value.server != b->value.server);
	return FALSE;
}

static gint
sp_style_write_ipaint (guchar *b, gint len, const guchar *key, SPIPaint *paint, SPIPaint *base, guint flags)
{
	unsigned int set;

	set = FALSE;
	if (((flags & SP_STYLE_FLAG_IFSET) && paint->set) ||
	    ((flags & SP_STYLE_FLAG_IFDIFF) && sp_paint_differ (paint, base))) {
		if (paint->inherit) {
			return g_snprintf (b, len, "%s:inherit;", key);
		} else {
			switch (paint->type) {
			case SP_PAINT_TYPE_COLOR:
				return g_snprintf (b, len, "%s:#%06x;", key, sp_color_get_rgba32_falpha (&paint->value.color, 0.0) >> 8);
				break;
			case SP_PAINT_TYPE_PAINTSERVER:
				if (paint->value.server) {
					return g_snprintf (b, len, "%s:url(#%s);", key, SP_OBJECT (paint->value.server)->id);
				}
				break;
			default:
				break;
			}
			return g_snprintf (b, len, "%s:none;", key);
		}
	}
	return 0;
}

static unsigned int
sp_fontsize_differ (SPIFontSize *a, SPIFontSize *b)
{
	if (a->type != b->type) return TRUE;
	if (a->type == SP_FONT_SIZE_LENGTH) {
		if (a->computed != b->computed) return TRUE;
	} else {
		if (a->value != b->value) return TRUE;
	}
	return FALSE;
}

static gint
sp_style_write_ifontsize (guchar *p, gint len, const guchar *key, SPIFontSize *val, SPIFontSize *base, guint flags)
{
	if (((flags & SP_STYLE_FLAG_IFSET) && val->set) ||
	    ((flags & SP_STYLE_FLAG_IFDIFF) && sp_fontsize_differ (val, base))) {
		if (val->inherit) {
			return g_snprintf (p, len, "%s:inherit;", key);
		} else if (val->type == SP_FONT_SIZE_LITERAL) {
			gint i;
			for (i = 0; enum_font_size[i].key; i++) {
				if (enum_font_size[i].value == val->value) {
					return g_snprintf (p, len, "%s:%s;", key, enum_font_size[i].key);
				}
			}
		} else if (val->type == SP_FONT_SIZE_LENGTH) {
			return g_snprintf (p, len, "%s:%g;", key, val->computed);
		} else if (val->type == SP_FONT_SIZE_PERCENTAGE) {
			return g_snprintf (p, len, "%s:%g%%;", key, SP_F8_16_TO_FLOAT (val->value) * 100.0);
		}
	}
	return 0;
}

/*
 * (C M Y K) tetraplet
 */

static SPColor *
sp_style_read_color_cmyk (SPColor *color, const guchar *str)
{
	gdouble c, m, y, k;
	gchar *cptr, *eptr;

	g_return_val_if_fail (str != NULL, NULL);

	c = m = y = k = 0.0;
	cptr = (gchar *) str + 1;
	c = strtod (cptr, &eptr);
	if (eptr && (eptr != cptr)) {
		cptr = eptr;
		m = strtod (cptr, &eptr);
		if (eptr && (eptr != cptr)) {
			cptr = eptr;
			y = strtod (cptr, &eptr);
			if (eptr && (eptr != cptr)) {
				cptr = eptr;
				k = strtod (cptr, &eptr);
				if (eptr && (eptr != cptr)) {
					sp_color_set_cmyk_float (color, c, m, y, k);
					return color;
				}
			}
		}
	}

	return NULL;
}

static void
sp_style_paint_clear (SPStyle *style, SPIPaint *paint, unsigned int hunref, unsigned int unset)
{
	if ((paint->type == SP_PAINT_TYPE_PAINTSERVER) && paint->value.server) {
		sp_object_hunref (SP_OBJECT (paint->value.server), style);
/*  		gtk_signal_disconnect_by_data (GTK_OBJECT (paint->value.server), style); */
		g_signal_handlers_disconnect_matched (G_OBJECT(paint->value.server), G_SIGNAL_MATCH_DATA, 0, 0, NULL, NULL, style);
		paint->value.server = NULL;
		paint->type = SP_PAINT_TYPE_NONE;
	}

	if (unset) {
		paint->set = FALSE;
		paint->inherit = FALSE;
	}
}

