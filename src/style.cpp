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

#include "config.h"

#if HAVE_STRING_H
#include <string.h>
#endif

#include <ctype.h>

#if HAVE_STDLIB_H
#include <stdlib.h>
#endif

#include <gtk/gtksignal.h>

#include "svg/svg.h"

#include "enums.h"
#include "display/canvas-bpath.h"
#include "attributes.h"
#include "document.h"
#include "extract-uri.h"
#include "marker-status.h"
#include "uri-references.h"
#include "sp-paint-server.h"
#include "style.h"
#include "svg/stringstream.h"
#include "xml/repr.h"
#include "unit-constants.h"

namespace Inkscape {



gchar *parse_css_url(const gchar *string) {
    const gchar *iter;
    gchar *result;
    gchar end_char;
    GString *temp;

    if (!string)
        return NULL;

    iter = string;

    for ( ; g_ascii_isspace(*iter) ; iter = g_utf8_next_char(iter) );
    if (strncmp(iter, "url(", 4))
        return NULL;
    iter += 4;

    if ( *iter == '"' || *iter == '\'' ) {
        end_char = *iter;
        iter += 1;
    } else {
        end_char = *iter;
    }

    temp = g_string_new(NULL);

    for ( ; *iter ; iter = g_utf8_next_char(iter) ) {
        if ( *iter == '(' || *iter == ')'  ||
             *iter == '"' || *iter == '\'' ||
             g_ascii_isspace(*iter)        ||
             g_ascii_iscntrl(*iter)           )
        {
            break;
        }
        if ( *iter == '\\' ) {
            iter = g_utf8_next_char(iter);
        }
        if ( *iter & (gchar)0x80 ) {
            break;
        } else {
            g_string_append_c(temp, *iter);
        }
    }

    if ( *iter == end_char && end_char != ')' ) {
        iter = g_utf8_next_char(iter);
    }
    if ( *iter == ')' ) {
        result = temp->str;
        g_string_free(temp, FALSE);
    } else {
        result = NULL;
        g_string_free(temp, TRUE);
    }

    return result;
}

}

#define BMAX 8192

class SPStyleEnum;

/*#########################
## FORWARD DECLARATIONS
#########################*/
static void sp_style_clear (SPStyle *style);

static void sp_style_merge_property (SPStyle *style, gint id, const gchar *val);

static void sp_style_merge_ipaint (SPStyle *style, SPIPaint *paint, SPIPaint *parent);
static void sp_style_read_dash(SPStyle *style, gchar const *str);

static SPTextStyle *sp_text_style_new (void);
static void sp_text_style_clear (SPTextStyle *ts);
static SPTextStyle *sp_text_style_unref (SPTextStyle *st);
static SPTextStyle *sp_text_style_duplicate_unset (SPTextStyle *st);
static guint sp_text_style_write(gchar *p, guint len, SPTextStyle const *st, guint flags = SP_STYLE_FLAG_IFSET);
static void sp_style_privatize_text (SPStyle *style);

static void sp_style_read_ifloat (SPIFloat *val, const gchar *str);
static void sp_style_read_iscale24 (SPIScale24 *val, const gchar *str);
static void sp_style_read_ienum(SPIEnum *val, gchar const *str, SPStyleEnum const *dict, bool can_explicitly_inherit);
static void sp_style_read_istring (SPIString *val, const gchar *str);
static void sp_style_read_ilength (SPILength *val, const gchar *str);
static void sp_style_read_icolor (SPIPaint *paint, const gchar *str, SPStyle *style, SPDocument *document);
static void sp_style_read_ipaint (SPIPaint *paint, const gchar *str, SPStyle *style, SPDocument *document);
static void sp_style_read_ifontsize (SPIFontSize *val, const gchar *str);

static void sp_style_read_penum(SPIEnum *val, SPRepr *repr, const gchar *key, const SPStyleEnum *dict, bool can_explicitly_inherit);
static void sp_style_read_plength (SPILength *val, SPRepr *repr, const gchar *key);
static void sp_style_read_pfontsize (SPIFontSize *val, SPRepr *repr, const gchar *key);

static gint sp_style_write_ifloat(gchar *p, gint len, gchar const *key, SPIFloat const *val, SPIFloat const *base, guint flags);
static gint sp_style_write_iscale24(gchar *p, gint len, gchar const *key, SPIScale24 const *val, SPIScale24 const *base, guint flags);
static gint sp_style_write_ienum(gchar *p, gint len, gchar const *key, SPStyleEnum const *dict, SPIEnum const *val, SPIEnum const *base, guint flags);
static gint sp_style_write_istring(gchar *p, gint len, gchar const *key, SPIString const *val, SPIString const *base, guint flags);
static gint sp_style_write_ilength(gchar *p, gint len, gchar const *key, SPILength const *val, SPILength const *base, guint flags);
static gint sp_style_write_ipaint(gchar *b, gint len, gchar const *key, SPIPaint const *paint, SPIPaint const *base, guint flags);
static gint sp_style_write_ifontsize(gchar *p, gint len, gchar const *key, SPIFontSize const *val, SPIFontSize const *base, guint flags);

static void sp_style_paint_clear (SPStyle *style, SPIPaint *paint, unsigned int hunref, unsigned int unset);

#define SPS_READ_IENUM_IF_UNSET(v,s,d,i) if (!(v)->set) {sp_style_read_ienum ((v), (s), (d), (i));}
#define SPS_READ_PENUM_IF_UNSET(v,r,k,d,i) if (!(v)->set) {sp_style_read_penum ((v), (r), (k), (d), (i));}

#define SPS_READ_ILENGTH_IF_UNSET(v,s) if (!(v)->set) {sp_style_read_ilength ((v), (s));}
#define SPS_READ_PLENGTH_IF_UNSET(v,r,k) if (!(v)->set) {sp_style_read_plength ((v), (r), (k));}

#define SPS_READ_IFONTSIZE_IF_UNSET(v,s) if (!(v)->set) {sp_style_read_ifontsize ((v), (s));}
#define SPS_READ_PFONTSIZE_IF_UNSET(v,r,k) if (!(v)->set) {sp_style_read_pfontsize ((v), (r), (k));}

struct SPStyleEnum {
    const gchar *key;
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
    {"bolder", SP_CSS_FONT_WEIGHT_BOLDER},
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
    /* Note that using the same enumerator for lr as lr-tb means we write as lr-tb even if the
     * input file said lr.  We prefer writing lr-tb on the grounds that the spec says the initial
     * value is lr-tb rather than lr.
     *
     * ECMA scripts may be surprised to find tb-rl in DOM if they set the attribute to rl, so
     * sharing enumerators for different strings may be a bug (once we support ecma script).
     */
    {"lr-tb", SP_CSS_WRITING_MODE_LR},
    {"rl-tb", SP_CSS_WRITING_MODE_RL},
    {"tb-rl", SP_CSS_WRITING_MODE_TB},
    {"lr", SP_CSS_WRITING_MODE_LR},
    {"rl", SP_CSS_WRITING_MODE_RL},
    {"tb", SP_CSS_WRITING_MODE_TB},
    {NULL, -1}
};

static SPStyleEnum const enum_visibility[] = {
    {"hidden", SP_CSS_VISIBILITY_HIDDEN},
    {"collapse", SP_CSS_VISIBILITY_COLLAPSE},
    {"visible", SP_CSS_VISIBILITY_VISIBLE},
    {NULL, -1}
};

static SPStyleEnum const enum_display[] = {
    {"none",      SP_CSS_DISPLAY_NONE},
    {"inline",    SP_CSS_DISPLAY_INLINE},
    {"block",     SP_CSS_DISPLAY_BLOCK},
    {"list-item", SP_CSS_DISPLAY_LIST_ITEM},
    {"run-in",    SP_CSS_DISPLAY_RUN_IN},
    {"compact",   SP_CSS_DISPLAY_COMPACT},
    {"marker",    SP_CSS_DISPLAY_MARKER},
    {"table",     SP_CSS_DISPLAY_TABLE},
    {"inline-table",  SP_CSS_DISPLAY_INLINE_TABLE},
    {"table-row-group",    SP_CSS_DISPLAY_TABLE_ROW_GROUP},
    {"table-header-group", SP_CSS_DISPLAY_TABLE_HEADER_GROUP},
    {"table-footer-group", SP_CSS_DISPLAY_TABLE_FOOTER_GROUP},
    {"table-row",     SP_CSS_DISPLAY_TABLE_ROW},
    {"table-column-group", SP_CSS_DISPLAY_TABLE_COLUMN_GROUP},
    {"table-column",  SP_CSS_DISPLAY_TABLE_COLUMN},
    {"table-cell",    SP_CSS_DISPLAY_TABLE_CELL},
    {"table-caption", SP_CSS_DISPLAY_TABLE_CAPTION},
    {NULL, -1}
};

static SPStyleEnum const enum_shape_rendering[] = {
    {"auto", 0},
    {"optimizeSpeed", 0},
    {"crispEdges", 0},
    {"geometricPrecision", 0},
    {NULL, -1}
};

static SPStyleEnum const enum_color_rendering[] = {
    {"auto", 0},
    {"optimizeSpeed", 0},
    {"optimizeQuality", 0},
    {NULL, -1}
};

static SPStyleEnum const *const enum_image_rendering = enum_color_rendering;

static SPStyleEnum const enum_text_rendering[] = {
    {"auto", 0},
    {"optimizeSpeed", 0},
    {"optimizeLegibility", 0},
    {"geometricPrecision", 0},
    {NULL, -1}
};

/**
 *
 */
static void
sp_style_object_release (SPObject *object, SPStyle *style)
{
    style->object = NULL;
}




/**
 *
 */
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

    style->cloned = false;

    return style;
}


/**
 *
 */
SPStyle *
sp_style_new_from_object (SPObject *object)
{
    SPStyle *style;

    g_return_val_if_fail (object != NULL, NULL);
    g_return_val_if_fail (SP_IS_OBJECT (object), NULL);

    style = sp_style_new ();

    style->object = object;
    g_signal_connect (G_OBJECT (object), "release", G_CALLBACK (sp_style_object_release), style);

    if (object && SP_OBJECT_IS_CLONED (object)) {
        style->cloned = true;
    }

    return style;
}


/**
 *
 */
SPStyle *
sp_style_ref (SPStyle *style)
{
    g_return_val_if_fail (style != NULL, NULL);
    g_return_val_if_fail (style->refcount > 0, NULL);

    style->refcount += 1;

    return style;
}


/**
 *
 */
SPStyle *
sp_style_unref (SPStyle *style)
{
    g_return_val_if_fail (style != NULL, NULL);
    g_return_val_if_fail (style->refcount > 0, NULL);

    style->refcount -= 1;

    if (style->refcount < 1) {
        // if (style->object)
        //    gtk_signal_disconnect_by_data (GTK_OBJECT (style->object), style);
        if (style->object)
            g_signal_handlers_disconnect_matched (G_OBJECT(style->object),
                      G_SIGNAL_MATCH_DATA, 0, 0, NULL, NULL, style);
        if (style->text) sp_text_style_unref (style->text);
        sp_style_paint_clear (style, &style->fill, TRUE, FALSE);
        sp_style_paint_clear (style, &style->stroke, TRUE, FALSE);
        g_free (style->stroke_dash.dash);
        g_free (style);
    }

    return NULL;
}

/**
 *  Reads the various style parameters for an object
 */
static void
sp_style_read (SPStyle *style, SPObject *object, SPRepr *repr)
{
    const gchar *val;

    g_assert (style != NULL);
    g_assert (repr != NULL);
    g_assert (!object || (SP_OBJECT_REPR (object) == repr));

    sp_style_clear (style);

    if (object && SP_OBJECT_IS_CLONED (object)) {
        style->cloned = true;
    }

    /* 1. Style itself */
    val = sp_repr_attr (repr, "style");
    if (val != NULL) {
        sp_style_merge_from_style_string (style, val);
    }

    // 2. was sodipodi cmyk stuff, removed

    /* fixme: CSS etc. parsing goes here */
    /* 3. Other styling methods */

    /* 4. Presentation attributes */
    /* CSS2 */
    SPS_READ_PENUM_IF_UNSET(&style->visibility, repr, "visibility", enum_visibility, true);
    SPS_READ_PENUM_IF_UNSET(&style->display, repr, "display", enum_display, true);
    /* Font */
    SPS_READ_PFONTSIZE_IF_UNSET (&style->font_size, repr, "font-size");
    SPS_READ_PENUM_IF_UNSET(&style->font_style, repr, "font-style", enum_font_style, true);
    SPS_READ_PENUM_IF_UNSET(&style->font_variant, repr, "font-variant", enum_font_variant, true);
    SPS_READ_PENUM_IF_UNSET(&style->font_weight, repr, "font-weight", enum_font_weight, true);
    SPS_READ_PENUM_IF_UNSET(&style->font_stretch, repr, "font-stretch", enum_font_stretch, true);

    /* opacity */
    if (!style->opacity.set) {
        val = sp_repr_attr (repr, "opacity");
        if (val) {
            sp_style_read_iscale24 (&style->opacity, val);
        }
    }
      /* color */
       if (!style->color.set) {
               val = sp_repr_attr (repr, "color");
               if (val) {
                       /* fixme: Disallow parsing paintservers */
             sp_style_read_icolor (&style->color, val, style, (object) ? SP_OBJECT_DOCUMENT (object) : NULL);
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
    SPS_READ_PENUM_IF_UNSET(&style->fill_rule, repr, "fill-rule", enum_fill_rule, true);
    /* stroke */
    if (!style->stroke.set) {
        val = sp_repr_attr (repr, "stroke");
        if (val) {
            sp_style_read_ipaint (&style->stroke, val, style, (object) ? SP_OBJECT_DOCUMENT (object) : NULL);
        }
    }
    SPS_READ_PLENGTH_IF_UNSET (&style->stroke_width, repr, "stroke-width");
    SPS_READ_PENUM_IF_UNSET(&style->stroke_linecap, repr, "stroke-linecap", enum_stroke_linecap, true);
    SPS_READ_PENUM_IF_UNSET(&style->stroke_linejoin, repr, "stroke-linejoin", enum_stroke_linejoin, true);
 
    /* markers */
    if (!style->marker[SP_MARKER_LOC].set) {
        val = sp_repr_attr (repr, "marker");
        if (val) {
            sp_style_read_istring (&style->marker[SP_MARKER_LOC], val);
        }
    }
    if (!style->marker[SP_MARKER_LOC_START].set) {
        val = sp_repr_attr (repr, "marker-start");
        if (val) {
            sp_style_read_istring (&style->marker[SP_MARKER_LOC_START], val);
        }
    }
    if (!style->marker[SP_MARKER_LOC_MID].set) {
        val = sp_repr_attr (repr, "marker-mid");
        if (val) {
            sp_style_read_istring (&style->marker[SP_MARKER_LOC_MID], val);
        }
    }
    if (!style->marker[SP_MARKER_LOC_END].set) {
        val = sp_repr_attr (repr, "marker-end");
        if (val) {
            sp_style_read_istring (&style->marker[SP_MARKER_LOC_END], val);
        }
    }

     /* stroke-opacity */
    if (!style->stroke_opacity.set) {
        val = sp_repr_attr (repr, "stroke-opacity");
        if (val) {
            sp_style_read_iscale24 (&style->stroke_opacity, val);
        }
    }
    if (!style->stroke_dasharray_set) {
        val = sp_repr_attr (repr, "stroke-dasharray");
        if (val) {
            sp_style_read_dash(style, val);
        }
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
    SPS_READ_PENUM_IF_UNSET(&style->text_anchor, repr, "text-anchor",
                            enum_text_anchor, true);
    SPS_READ_PENUM_IF_UNSET(&style->writing_mode, repr, "writing-mode",
                            enum_writing_mode, true);

    /* 5. Merge from parent */
    if (object) {
        if (object->parent) {
            sp_style_merge_from_parent (style, 
                        SP_OBJECT_STYLE (object->parent));
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


/**
 *
 */
void
sp_style_read_from_object (SPStyle *style, SPObject *object)
{
    g_return_if_fail (style != NULL);
    g_return_if_fail (object != NULL);
    g_return_if_fail (SP_IS_OBJECT (object));

    SPRepr *repr = SP_OBJECT_REPR(object);
    g_return_if_fail(repr != NULL);

    sp_style_read(style, object, repr);
}


/**
 *
 */
void
sp_style_read_from_repr (SPStyle *style, SPRepr *repr)
{
    g_return_if_fail (style != NULL);
    g_return_if_fail (repr != NULL);

    sp_style_read (style, NULL, repr);
}



/**
 *
 */
static void
sp_style_privatize_text (SPStyle *style)
{
    SPTextStyle *text;

    text = style->text;
    style->text = sp_text_style_duplicate_unset (style->text);
    sp_text_style_unref (text);
    style->text_private = TRUE;
}


/**
 * \pre val != NULL.
 */
static void
sp_style_merge_property (SPStyle *style, gint id, const gchar *val)
{
    g_return_if_fail(val != NULL);

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
        if (strcmp(val, "none") != 0) {
            g_warning("Unimplemented style property id SP_PROP_FONT_SIZE_ADJUST: value: %s", val);
        }
        break;
    case SP_PROP_FONT_STYLE:
        SPS_READ_IENUM_IF_UNSET(&style->font_style, val, enum_font_style, true);
        break;
    case SP_PROP_FONT_VARIANT:
        SPS_READ_IENUM_IF_UNSET(&style->font_variant, val, enum_font_variant, true);
        break;
    case SP_PROP_FONT_WEIGHT:
        SPS_READ_IENUM_IF_UNSET(&style->font_weight, val, enum_font_weight, true);
        break;
    case SP_PROP_FONT_STRETCH:
        SPS_READ_IENUM_IF_UNSET(&style->font_stretch, val, enum_font_stretch, true);
        break;
    case SP_PROP_FONT:
        if (!style->text_private) sp_style_privatize_text (style);
        if (!style->text->font.set) {
            g_free (style->text->font.value);
            style->text->font.value = g_strdup (val);
            style->text->font.set = TRUE;
            style->text->font.inherit = (val && !strcmp (val, "inherit"));
        }
        break;
    /* Text */
    case SP_PROP_DIRECTION:
        g_warning ("Unimplemented style property SP_PROP_DIRECTION: value: %s", val);
        break;
    case SP_PROP_LETTER_SPACING:
        if (!style->text_private) sp_style_privatize_text (style);
        style->text->letterspacing_normal = FALSE;
        if (strcmp(val, "normal") == 0) {
            style->text->letterspacing_normal = TRUE;
            val = "0";
        }
        sp_style_read_ilength (&style->text->letterspacing, val);
        style->text->letterspacing.set = TRUE;
        break;
    case SP_PROP_TEXT_DECORATION:
        g_warning ("Unimplemented style property SP_PROP_TEXT_DECORATION: value: %s", val);
        break;
    case SP_PROP_UNICODE_BIDI:
        g_warning ("Unimplemented style property SP_PROP_UNICODE_BIDI: value: %s", val);
        break;
    case SP_PROP_WORD_SPACING:
        if (strcmp(val, "normal") != 0) {
            g_warning("Unimplemented style property SP_PROP_WORD_SPACING: value: %s", val);
        }
        break;
    /* Misc */
    case SP_PROP_CLIP:
        g_warning ("Unimplemented style property SP_PROP_CLIP: value: %s", val);
        break;
    case SP_PROP_COLOR:
             if (!style->color.set) {
                      sp_style_read_icolor (&style->color, val, style, (style->object) ? SP_OBJECT_DOCUMENT (style->object) : NULL);
             }
             break;
    case SP_PROP_CURSOR:
        g_warning ("Unimplemented style property SP_PROP_CURSOR: value: %s", val);
        break;
    case SP_PROP_DISPLAY:
        SPS_READ_IENUM_IF_UNSET(&style->display, val, enum_display, true);
        break;
    case SP_PROP_OVERFLOW:
        // FIXME: temporaily disabled, for our markers.svg uses overflow: visible to show properly in batik.
        // Inkscape acts as if "visible" is set, at least for markers.
        // Replace this with a proper implementation of the property.

        //g_warning ("Unimplemented style property SP_PROP_OVERFLOW: %d value: %s", id, val);
        break;
    case SP_PROP_VISIBILITY:
        SPS_READ_IENUM_IF_UNSET(&style->visibility, val, enum_visibility, true);
        break;
    /* SVG */
    /* Clip/Mask */
    case SP_PROP_CLIP_PATH:
        g_warning ("Unimplemented style property SP_PROP_CLIP_PATH: value: %s", val);
        break;
    case SP_PROP_CLIP_RULE:
        g_warning ("Unimplemented style property SP_PROP_CLIP_RULE: value: %s", val);
        break;
    case SP_PROP_MASK:
        g_warning ("Unimplemented style property SP_PROP_MASK: value: %s", val);
        break;
    case SP_PROP_OPACITY:
        if (!style->opacity.set) {
            sp_style_read_iscale24 (&style->opacity, val);
        }
        break;
    /* Filter */
    case SP_PROP_ENABLE_BACKGROUND:
        g_warning ("Unimplemented style property SP_PROP_ENABLE_BACKGROUND: value: %s", val);
        break;
    case SP_PROP_FILTER:
        g_warning ("Unimplemented style property SP_PROP_FILTER: value: %s", val);
        break;
    case SP_PROP_FLOOD_COLOR:
        g_warning ("Unimplemented style property SP_PROP_FLOOD_COLOR: value: %s", val);
        break;
    case SP_PROP_FLOOD_OPACITY:
        g_warning ("Unimplemented style property SP_PROP_FLOOD_OPACITY: value: %s", val);
        break;
    case SP_PROP_LIGHTING_COLOR:
        g_warning ("Unimplemented style property SP_PROP_LIGHTING_COLOR: value: %s", val);
        break;
    /* Gradient */
    case SP_PROP_STOP_COLOR:
        g_warning ("Unimplemented style property SP_PROP_STOP_COLOR: value: %s", val);
        break;
    case SP_PROP_STOP_OPACITY:
        g_warning ("Unimplemented style property SP_PROP_STOP_OPACITY: value: %s", val);
        break;
    /* Interactivity */
    case SP_PROP_POINTER_EVENTS:
        g_warning ("Unimplemented style property SP_PROP_POINTER_EVENTS: value: %s", val);
        break;
    /* Paint */
    case SP_PROP_COLOR_INTERPOLATION:
        g_warning ("Unimplemented style property SP_PROP_COLOR_INTERPOLATION: value: %s", val);
        break;
    case SP_PROP_COLOR_INTERPOLATION_FILTERS:
        g_warning ("Unimplemented style property SP_PROP_INTERPOLATION_FILTERS: value: %s", val);
        break;
    case SP_PROP_COLOR_PROFILE:
        g_warning ("Unimplemented style property SP_PROP_COLOR_PROFILE: value: %s", val);
        break;
    case SP_PROP_COLOR_RENDERING: {
        /* Ignore the hint. */
        SPIEnum dummy;
        SPS_READ_IENUM_IF_UNSET(&dummy, val, enum_color_rendering, true);
        break;
    }
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
            sp_style_read_ienum(&style->fill_rule, val, enum_fill_rule, true);
        }
        break;
    case SP_PROP_IMAGE_RENDERING: {
        /* Ignore the hint. */
        SPIEnum dummy;
        SPS_READ_IENUM_IF_UNSET(&dummy, val, enum_image_rendering, true);
        break;
    }
    case SP_PROP_MARKER:
        /* TODO:  Call sp_uri_reference_resolve (SPDocument *document, const guchar *uri) */
        /* style->marker[SP_MARKER_LOC] = g_quark_from_string(val); */
        marker_status("Setting SP_PROP_MARKER");
        if (!style->marker[SP_MARKER_LOC].set) {
            g_free (style->marker[SP_MARKER_LOC].value);
            style->marker[SP_MARKER_LOC].value = g_strdup (val);
            style->marker[SP_MARKER_LOC].set = TRUE;
            style->marker[SP_MARKER_LOC].inherit = (val && !strcmp (val, "inherit"));
        }
        break;

    case SP_PROP_MARKER_START:
        /* TODO:  Call sp_uri_reference_resolve (SPDocument *document, const guchar *uri) */
        marker_status("Setting SP_PROP_MARKER_START");
        if (!style->marker[SP_MARKER_LOC_START].set) {
            g_free (style->marker[SP_MARKER_LOC_START].value);
            style->marker[SP_MARKER_LOC_START].value = g_strdup (val);
            style->marker[SP_MARKER_LOC_START].set = TRUE;
            style->marker[SP_MARKER_LOC_START].inherit = (val && !strcmp (val, "inherit"));
        }
        break;
    case SP_PROP_MARKER_MID:
        /* TODO:  Call sp_uri_reference_resolve (SPDocument *document, const guchar *uri) */
        marker_status("Setting SP_PROP_MARKER_MID");
        if (!style->marker[SP_MARKER_LOC_MID].set) {
            g_free (style->marker[SP_MARKER_LOC_MID].value);
            style->marker[SP_MARKER_LOC_MID].value = g_strdup (val);
            style->marker[SP_MARKER_LOC_MID].set = TRUE;
            style->marker[SP_MARKER_LOC_MID].inherit = (val && !strcmp (val, "inherit"));
        }
        break;
    case SP_PROP_MARKER_END:
        /* TODO:  Call sp_uri_reference_resolve (SPDocument *document, const guchar *uri) */
        marker_status("Setting SP_PROP_MARKER_END");
        if (!style->marker[SP_MARKER_LOC_END].set) {
            g_free (style->marker[SP_MARKER_LOC_END].value);
            style->marker[SP_MARKER_LOC_END].value = g_strdup (val);
            style->marker[SP_MARKER_LOC_END].set = TRUE;
            style->marker[SP_MARKER_LOC_END].inherit = (val && !strcmp (val, "inherit"));
        }
        break;

    case SP_PROP_SHAPE_RENDERING: {
        /* Ignore the hint. */
        SPIEnum dummy;
        SPS_READ_IENUM_IF_UNSET(&dummy, val, enum_shape_rendering, true);
        break;
    }

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
            sp_style_read_dash(style, val);
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
            sp_style_read_ienum(&style->stroke_linecap, val, enum_stroke_linecap, true);
        }
        break;
    case SP_PROP_STROKE_LINEJOIN:
        if (!style->stroke_linejoin.set) {
            sp_style_read_ienum(&style->stroke_linejoin, val, enum_stroke_linejoin, true);
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
               
    /* Text */
    case SP_PROP_TEXT_RENDERING: {
        /* Ignore the hint. */
        SPIEnum dummy;
        SPS_READ_IENUM_IF_UNSET(&dummy, val, enum_text_rendering, true);
        break;
    }
    case SP_PROP_ALIGNMENT_BASELINE:
        g_warning ("Unimplemented style property SP_PROP_ALIGNMENT_BASELINE: value: %s", val);
        break;
    case SP_PROP_BASELINE_SHIFT:
        g_warning ("Unimplemented style property SP_PROP_BASELINE_SHIFT: value: %s", val);
        break;
    case SP_PROP_DOMINANT_BASELINE:
        g_warning ("Unimplemented style property SP_PROP_DOMINANT_BASELINE: value: %s", val);
        break;
    case SP_PROP_GLYPH_ORIENTATION_HORIZONTAL:
        g_warning ("Unimplemented style property SP_PROP_ORIENTATION_HORIZONTAL: value: %s", val);
        break;
    case SP_PROP_GLYPH_ORIENTATION_VERTICAL:
        g_warning ("Unimplemented style property SP_PROP_ORIENTATION_VERTICAL: value: %s", val);
        break;
    case SP_PROP_KERNING:
        g_warning ("Unimplemented style property SP_PROP_KERNING: value: %s", val);
        break;
    case SP_PROP_TEXT_ANCHOR:
        SPS_READ_IENUM_IF_UNSET(&style->text_anchor, val, enum_text_anchor, true);
        break;
    case SP_PROP_WRITING_MODE:
        SPS_READ_IENUM_IF_UNSET(&style->writing_mode, val, enum_writing_mode, true);
        break;
    default:
        g_warning ("Invalid style property id: %d value: %s", id, val);
        break;
    }
}


static bool
is_css_S(gchar const c)
{
    /* Like g_ascii_isspace, but false for '\v'. */
    switch (c) {
        case ' ':
        case '\t':
        case '\r':
        case '\n':
        case '\f':
            return true;
        default:
            return false;
    }
}

/* Bug: doesn't allow unicode or other escapes.
   Bug: doesn't distinguish between initial char (non-numeric, not hyphen) and others. */
static bool
is_css_ident_char(guchar const c)
{
    return (g_ascii_isalnum(c)
            || (c == '-')
            || (0x80 <= c));
}

/**
 * Parses a style="" string and merges it with an existing SPStyle
 */
void
sp_style_merge_from_style_string (SPStyle *style, const gchar *p)
{
    /* Reference: http://www.w3.org/TR/SVG11/styling.html#StyleAttribute: ``When CSS styling is
     * used, CSS inline style is specified by including semicolon-separated property declarations
     * of the form "name : value" within the style attribute''.
     *
     * That's fairly ambiguous.  Is a `value' allowed to contain semicolons?  Why does it say
     * "including", what else is allowed in the style attribute value?

     * Note: I believe a strict reading of the spec doesn't allow space at the beginning of a style
     * string: see section D.2 of http://www.w3.org/TR/REC-CSS2/grammar.html, where whitespace is
     * given a specific token S, and see the definitions of `declaration' and `property' at
     * http://www.w3.org/TR/REC-CSS2/syndata.html.
     *
     * The SVG spec is quite ambiguous about what semicolons are allowed.  Probably the intent is
     * that style strings are like CSS ruleset bodies, where there must be exactly one semicolon
     * between declarations, and no semicolon at the beginning or end.  Whereas the SVG 1.1 spec
     * merely says "semicolon-separated" (cf. "space-separated" which is usually understood as
     * allowing any number of spaces at the beginning and end, and any non-zero number of spaces
     * between items).
     *
     * N.B. Inkscape up to 0.40 (and probably sodipodi) write style strings with trailing
     * semicolon.  Given the ambiguity of the spec, we should continue to allow this.
     *
     * Indeed, we currently allow any number of semicolons at the beginning or end,
     * and any non-zero number of semicolons between declarations.
     */

    gchar property [BMAX];
    gchar value [BMAX];

    for (;;) {

        /* Bug: we don't allow CSS comments. */

        while (is_css_S(*p) || *p == ';') {
            ++p;
        }
        if (!*p) {
            return;
        }

        gchar const *property_begin = p;
        while (is_css_ident_char(*p)) {
            ++p;
        }
        gchar const *property_end = p;
        if (property_begin == property_end) {
            /* TODO: Don't use g_warning for SVG errors. */
            g_warning("Empty style property at: %s", property_begin);
            return;
        }
        size_t const property_len = property_end - property_begin;
        if (property_len >= sizeof(property)) {
            /* TODO: Don't use g_warning for SVG errors. */
            g_warning("Exceedingly long style property %.20s...", property_begin);
            return;
        }
        memcpy(property, property_begin, property_len);
        property[property_len] = '\0';

        while (is_css_S(*p)) {
            ++p;
        }

        if (*p++ != ':') {
            /* TODO: Don't use g_warning for SVG errors. */
            g_warning("No separator at style at: %s", property_begin);
            return;
        }

        while (is_css_S(*p)) {
            ++p;
        }

        gchar const *const value_begin = p;
        gchar const *decl_end = strchr(p, ';');
        if (!decl_end) {
            decl_end = p + strlen(p);
        }
        gchar const *value_end = decl_end;
        while (is_css_S(*--value_end))
            ;
        ++value_end;

        gint const idx = sp_attribute_lookup(property);
        if (idx > 0) {
            size_t const value_len = value_end - value_begin;
            if (value_len != 0) {
                memcpy(value, value_begin, value_len);
                value[value_len] = '\0';
                sp_style_merge_property(style, idx, value);
            } else {
                /* TODO: Don't use g_warning for SVG errors. */
                g_warning("No style property value at: %s", property_begin);
            }
        } else {
            /* TODO: Don't use g_warning for SVG errors. */
            g_warning("Unknown style property at: %s", property_begin);
        }

        p = decl_end;
    }
}


/**
 *
 */
void
sp_style_merge_from_parent (SPStyle *style, SPStyle *parent)
{
    int i;

    g_return_if_fail (style != NULL);

    if (!parent)
        return;

    /* CSS2 */
    /* Font */
    /* 'font-size' */
    if (!style->font_size.set || style->font_size.inherit) {
        /* I think inheriting computed value is correct here */
        style->font_size.type = SP_FONT_SIZE_LENGTH;
        style->font_size.computed = parent->font_size.computed;
    } else if (style->font_size.type == SP_FONT_SIZE_LITERAL) {
        static gfloat sizetable[] = {6.0, 8.0, 10.0, 12.0, 14.0, 18.0, 24.0};
        /* fixme: SVG and CSS do not specify clearly, whether we should use user or screen coordinates (Lauris) */
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
        /* it says the parent's. --mental */
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
        unsigned const parent_val = parent->font_weight.computed;
        g_assert (SP_CSS_FONT_WEIGHT_100 == 0);
        style->font_weight.computed = (parent_val == SP_CSS_FONT_WEIGHT_100
                           ? parent_val
                           : parent_val - 1);
        g_assert (style->font_weight.computed <= (unsigned) SP_CSS_FONT_WEIGHT_900);
    } else if (style->font_weight.value == SP_CSS_FONT_WEIGHT_BOLDER) {
        unsigned const parent_val = parent->font_weight.computed;
        g_assert (parent_val <= SP_CSS_FONT_WEIGHT_900);
        style->font_weight.computed = (parent_val == SP_CSS_FONT_WEIGHT_900
                           ? parent_val
                           : parent_val + 1);
        g_assert (style->font_weight.computed <= (unsigned) SP_CSS_FONT_WEIGHT_900);
    }
    /* 'font-stretch' */
    if (!style->font_stretch.set || style->font_stretch.inherit) {
        style->font_stretch.computed = parent->font_stretch.computed;
    } else if (style->font_stretch.value == SP_CSS_FONT_STRETCH_NARROWER) {
        unsigned const parent_val = parent->font_stretch.computed;
        style->font_stretch.computed = (parent_val == SP_CSS_FONT_STRETCH_ULTRA_CONDENSED
                        ? parent_val
                        : parent_val - 1);
        g_assert (style->font_stretch.computed <= (unsigned) SP_CSS_FONT_STRETCH_ULTRA_EXPANDED);
    } else if (style->font_stretch.value == SP_CSS_FONT_STRETCH_WIDER) {
        unsigned const parent_val = parent->font_stretch.computed;
        g_assert (parent_val <= SP_CSS_FONT_STRETCH_ULTRA_EXPANDED);
        style->font_stretch.computed = (parent_val == SP_CSS_FONT_STRETCH_ULTRA_EXPANDED
                        ? parent_val
                        : parent_val + 1);
        g_assert (style->font_stretch.computed <= (unsigned) SP_CSS_FONT_STRETCH_ULTRA_EXPANDED);
    }
    if (style->opacity.inherit) {
        style->opacity.value = parent->opacity.value;
    }

    /* Color */
    if (!style->color.set || style->color.inherit) {
        sp_style_merge_ipaint (style, &style->color, &parent->color);
    }
    if (!style->fill.set || style->fill.inherit || style->fill.currentcolor) {
        sp_style_merge_ipaint (style, &style->fill, &parent->fill);
    }
    if (!style->fill_opacity.set || style->fill_opacity.inherit) {
        style->fill_opacity.value = parent->fill_opacity.value;
    }
    if (!style->fill_rule.set || style->fill_rule.inherit) {
        style->fill_rule.computed = parent->fill_rule.computed;
    }
    /* Stroke */
    if (!style->stroke.set || style->stroke.inherit || style->stroke.currentcolor) {
        sp_style_merge_ipaint (style, &style->stroke, &parent->stroke);
    }
    if (!style->stroke_width.set || style->stroke_width.inherit) {
        style->stroke_width.unit = parent->stroke_width.unit;
        style->stroke_width.value = parent->stroke_width.value;
        /* TODO: The above looks suspicious.  What if the unit is em or ex, and the parent has
           different font-size or font-family (x height) from the child?  The spec says that only
           the computed value is inherited.  I think we should be ignoring value if (!set ||
           inherit).  Hence, I think we should remove the above two lines, as they may be hiding
           bugs. */
        style->stroke_width.computed = parent->stroke_width.computed;
    } else {
        /* Update computed value for any change in font inherited from parent. */
        double const em = style->font_size.computed;
        if (style->stroke_width.unit == SP_CSS_UNIT_EM) {
            style->stroke_width.computed = style->stroke_width.value * em;
        } else if (style->stroke_width.unit == SP_CSS_UNIT_EX) {
            double const ex = em * 0.5;  // fixme: Get x height from libnrtype or pango.
            style->stroke_width.computed = style->stroke_width.value * ex;
        }
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
        /* TODO: This code looks wrong.  Why does the logic differ from the above properties?
         * Similarly dashoffset below. */
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
            g_free (style->text->font_family.value);
            style->text->font_family.value = g_strdup (parent->text->font_family.value);
        }
        if (!style->text->letterspacing.set || style->text->letterspacing.inherit) {
            style->text->letterspacing_normal = parent->text->letterspacing_normal;
            style->text->letterspacing.value = parent->text->letterspacing.value;
            style->text->letterspacing.computed = parent->text->letterspacing.computed;
            style->text->letterspacing.unit = parent->text->letterspacing.unit;
        }
    }

    /* Markers - Free the old value and make copy of the new */
    for (i=SP_MARKER_LOC; i<SP_MARKER_LOC_QTY; i++) {
        if (!style->marker[i].set || style->marker[i].inherit) {
            g_free(style->marker[i].value);
            style->marker[i].value = g_strdup(parent->marker[i].value);
        }
    }
}



/**
 *
 */
static void
sp_style_paint_server_release (SPPaintServer *server, SPStyle *style)
{
    if ((style->fill.type == SP_PAINT_TYPE_PAINTSERVER) && (server == style->fill.value.paint.server)) {
        sp_style_paint_clear (style, &style->fill, TRUE, FALSE);
    } 
    if ((style->stroke.type == SP_PAINT_TYPE_PAINTSERVER) && (server == style->stroke.value.paint.server)) {
        sp_style_paint_clear (style, &style->stroke, TRUE, FALSE);
    } 
}




/**
 *
 */
static void
sp_style_paint_server_modified (SPPaintServer *server, guint flags, SPStyle *style)
{
    if ((style->fill.type == SP_PAINT_TYPE_PAINTSERVER) && (server == style->fill.value.paint.server)) {
        if (style->object) {
            /* fixme: I do not know, whether it is optimal - we are forcing reread of everything (Lauris) */
            /* fixme: We have to use object_modified flag, because parent flag is only available downstreams */
            style->object->requestModified(SP_OBJECT_MODIFIED_FLAG | SP_OBJECT_STYLE_MODIFIED_FLAG);
        }
    } else if ((style->stroke.type == SP_PAINT_TYPE_PAINTSERVER) && (server == style->stroke.value.paint.server)) {
        if (style->object) {
            /* fixme: */
            style->object->requestModified(SP_OBJECT_MODIFIED_FLAG | SP_OBJECT_STYLE_MODIFIED_FLAG);
        }
    } else {
        g_assert_not_reached ();
    }
}



/**
 *
 */
static void
sp_style_merge_ipaint (SPStyle *style, SPIPaint *paint, SPIPaint *parent)
{
    sp_style_paint_clear (style, paint, TRUE, FALSE);

    if ((paint->set && paint->currentcolor) || parent->currentcolor) {
        paint->currentcolor = TRUE;
        paint->type = SP_PAINT_TYPE_COLOR;
        sp_color_copy (&paint->value.color, &style->color.value.color);
        return;
    }

    paint->type = parent->type;
    switch (paint->type) {
        case SP_PAINT_TYPE_COLOR:
            sp_color_copy (&paint->value.color, &parent->value.color);
            break;
        case SP_PAINT_TYPE_PAINTSERVER:
            paint->value.paint.server = parent->value.paint.server;
            paint->value.paint.uri = parent->value.paint.uri;
            if (paint->value.paint.server) {
                if (!style->cloned) {
                    sp_object_href (SP_OBJECT (paint->value.paint.server), style);
                } 
                g_signal_connect (G_OBJECT (paint->value.paint.server), "release",
                      G_CALLBACK (sp_style_paint_server_release), style);
                g_signal_connect (G_OBJECT (paint->value.paint.server), "modified",
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


/**
Dumps the style to a CSS string, with either SP_STYLE_FLAG_IFSET or SP_STYLE_FLAG_ALWAYS
flags. Used with Always for copying an object's complete cascaded style to style_clipboard. When
you need a CSS string for an object in the document tree, you normally call
sp_style_write_difference instead to take into account the object's parent.

\pre flags in {IFSET, ALWAYS}.
*/
gchar *
sp_style_write_string(SPStyle const *const style, guint const flags)
{
    /* TODO: Merge with write_difference, much duplicate code! */
    g_return_val_if_fail (style != NULL, NULL);
    g_return_val_if_fail(((flags == SP_STYLE_FLAG_IFSET) ||
                          (flags == SP_STYLE_FLAG_ALWAYS)  ),
                         NULL);

    gchar c[BMAX];
    gchar *p = c;
    *p = '\0';

    p += sp_style_write_ifontsize (p, c + BMAX - p, "font-size", &style->font_size, NULL, flags);
    p += sp_style_write_ienum (p, c + BMAX - p, "font-style", enum_font_style, &style->font_style, NULL, flags);
    p += sp_style_write_ienum (p, c + BMAX - p, "font-variant", enum_font_variant, &style->font_variant, NULL, flags);
    p += sp_style_write_ienum (p, c + BMAX - p, "font-weight", enum_font_weight, &style->font_weight, NULL, flags);
    p += sp_style_write_ienum (p, c + BMAX - p, "font-stretch", enum_font_stretch, &style->font_stretch, NULL, flags);

    /* fixme: Per type methods need default flag too (lauris)*/
    p += sp_style_write_iscale24 (p, c + BMAX - p, "opacity", &style->opacity, NULL, flags);
    p += sp_style_write_ipaint (p, c + BMAX - p, "color", &style->color, NULL, flags);
    p += sp_style_write_ipaint (p, c + BMAX - p, "fill", &style->fill, NULL, flags);
    p += sp_style_write_iscale24 (p, c + BMAX - p, "fill-opacity", &style->fill_opacity, NULL, flags);
    p += sp_style_write_ienum (p, c + BMAX - p, "fill-rule", enum_fill_rule, &style->fill_rule, NULL, flags);
    p += sp_style_write_ipaint (p, c + BMAX - p, "stroke", &style->stroke, NULL, flags);
    p += sp_style_write_ilength (p, c + BMAX - p, "stroke-width", &style->stroke_width, NULL, flags);
    p += sp_style_write_ienum (p, c + BMAX - p, "stroke-linecap", enum_stroke_linecap, &style->stroke_linecap, NULL, flags);
    p += sp_style_write_ienum (p, c + BMAX - p, "stroke-linejoin", enum_stroke_linejoin, &style->stroke_linejoin, NULL, flags);

    marker_status("sp_style_write_string:  Writing markers");
    if (style->marker[SP_MARKER_LOC].set) {
        p += g_snprintf (p, c + BMAX - p, "marker:%s;", style->marker[SP_MARKER_LOC].value);
    } else if (flags == SP_STYLE_FLAG_ALWAYS) {
        p += g_snprintf (p, c + BMAX - p, "marker:none;");
    }
    if (style->marker[SP_MARKER_LOC_START].set) {
        p += g_snprintf (p, c + BMAX - p, "marker-start:%s;", style->marker[SP_MARKER_LOC_START].value);
    } else if (flags == SP_STYLE_FLAG_ALWAYS) {
        p += g_snprintf (p, c + BMAX - p, "marker-start:none;");
    }
    if (style->marker[SP_MARKER_LOC_MID].set) {
        p += g_snprintf (p, c + BMAX - p, "marker-mid:%s;", style->marker[SP_MARKER_LOC_MID].value);
    } else if (flags == SP_STYLE_FLAG_ALWAYS) {
        p += g_snprintf (p, c + BMAX - p, "marker-mid:none;");
    }
    if (style->marker[SP_MARKER_LOC_END].set) {
        p += g_snprintf (p, c + BMAX - p, "marker-end:%s;", style->marker[SP_MARKER_LOC_END].value);
    } else if (flags == SP_STYLE_FLAG_ALWAYS) {
        p += g_snprintf (p, c + BMAX - p, "marker-end:none;");
    }

    p += sp_style_write_ifloat (p, c + BMAX - p, "stroke-miterlimit", &style->stroke_miterlimit, NULL, flags);

    /* fixme: */
    if ((flags == SP_STYLE_FLAG_ALWAYS)
        || style->stroke_dasharray_set)
    {
        if (style->stroke_dasharray_inherit) {
            p += g_snprintf(p, c + BMAX - p, "stroke-dasharray:inherit;");
        } else if (style->stroke_dash.n_dash && style->stroke_dash.dash) {
            p += g_snprintf (p, c + BMAX - p, "stroke-dasharray:");
            gint i;
            for (i = 0; i < style->stroke_dash.n_dash; i++) {
				Inkscape::SVGOStringStream os;
                os << style->stroke_dash.dash[i] << " ";
				p += g_strlcpy (p, os.str().c_str(), c + BMAX - p);
            }
            if (p < c + BMAX) {
                *p++ = ';';
            }
        } else {
            p += g_snprintf(p, c + BMAX - p, "stroke-dasharray:none;");
        }
    }

    /* fixme: */
    if (style->stroke_dashoffset_set) {
		Inkscape::SVGOStringStream os;
        os << "stroke-dashoffset:" << style->stroke_dash.offset << ";";
		p += g_strlcpy (p, os.str().c_str(), c + BMAX - p);
    } else if (flags == SP_STYLE_FLAG_ALWAYS) {
        p += g_snprintf (p, c + BMAX - p, "stroke-dashoffset:0;");
    }

    p += sp_style_write_iscale24 (p, c + BMAX - p, "stroke-opacity", &style->stroke_opacity, NULL, flags);

    p += sp_style_write_ienum (p, c + BMAX - p, "visibility", enum_visibility, &style->visibility, NULL, flags);
    p += sp_style_write_ienum (p, c + BMAX - p, "display", enum_display, &style->display, NULL, flags);

    /* fixme: */
    p += sp_text_style_write (p, c + BMAX - p, style->text, flags);

    p += sp_style_write_ienum (p, c + BMAX - p, "text-anchor", enum_text_anchor, &style->text_anchor, NULL, flags);
    p += sp_style_write_ienum (p, c + BMAX - p, "writing-mode", enum_writing_mode, &style->writing_mode, NULL, flags);

    /* Get rid of trailing `;'. */
    if (p != c) {
        --p;
        if (*p == ';') {
            *p = '\0';
        }
    }

    return g_strdup (c);
}


#define STYLE_BUF_MAX


/**
 *
 */
gchar *
sp_style_write_difference(SPStyle const *const from, SPStyle const *const to)
{
    gchar c[BMAX], *p;

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
        p += sp_style_write_iscale24 (p, c + BMAX - p, "opacity", &from->opacity, &to->opacity, SP_STYLE_FLAG_IFSET);
    }
    p += sp_style_write_ipaint (p, c + BMAX - p, "color", &from->color, &to->color, SP_STYLE_FLAG_IFSET);
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
        if (from->stroke_dasharray_inherit) {
            p += g_snprintf(p, c + BMAX - p, "stroke-dasharray:inherit;");
        } else if (from->stroke_dash.n_dash && from->stroke_dash.dash) {
            gint i;
            p += g_snprintf (p, c + BMAX - p, "stroke-dasharray:");
            for (i = 0; i < from->stroke_dash.n_dash; i++) {
				Inkscape::SVGOStringStream os;
                os << from->stroke_dash.dash[i] << " ";
				p += g_strlcpy (p, os.str().c_str(), c + BMAX - p);
            }
            p += g_snprintf (p, c + BMAX - p, ";");
        }
    }
    /* fixme: */
    if (from->stroke_dashoffset_set) {
		Inkscape::SVGOStringStream os;
        os << "stroke-dashoffset:" << from->stroke_dash.offset << ";";
		p += g_strlcpy (p, os.str().c_str(), c + BMAX - p);
    }
    p += sp_style_write_iscale24 (p, c + BMAX - p, "stroke-opacity", &from->stroke_opacity, &to->stroke_opacity, SP_STYLE_FLAG_IFDIFF);

    /* markers */
    marker_status("sp_style_write_difference:  Writing markers");
    if (from->marker[SP_MARKER_LOC].value != NULL) {
        p += g_snprintf (p, c + BMAX - p, "marker:%s;",       from->marker[SP_MARKER_LOC].value);
    }
    if (from->marker[SP_MARKER_LOC_START].value != NULL) {
        p += g_snprintf (p, c + BMAX - p, "marker-start:%s;", from->marker[SP_MARKER_LOC_START].value);
    }
    if (from->marker[SP_MARKER_LOC_MID].value != NULL) {
        p += g_snprintf (p, c + BMAX - p, "marker-mid:%s;",   from->marker[SP_MARKER_LOC_MID].value);
    }
    if (from->marker[SP_MARKER_LOC_END].value != NULL) {
        p += g_snprintf (p, c + BMAX - p, "marker-end:%s;",   from->marker[SP_MARKER_LOC_END].value);
    }

    p += sp_style_write_ienum (p, c + BMAX - p, "visibility", enum_visibility, &from->visibility, &to->visibility, SP_STYLE_FLAG_IFSET);
    p += sp_style_write_ienum (p, c + BMAX - p, "display", enum_display, &from->display, &to->display, SP_STYLE_FLAG_IFSET);

    /* fixme: */
    p += sp_text_style_write (p, c + BMAX - p, from->text, SP_STYLE_FLAG_IFDIFF);

    p += sp_style_write_ienum (p, c + BMAX - p, "text-anchor", enum_text_anchor, &from->text_anchor, &to->text_anchor, SP_STYLE_FLAG_IFDIFF);
    p += sp_style_write_ienum (p, c + BMAX - p, "writing-mode", enum_writing_mode, &from->writing_mode, &to->writing_mode, SP_STYLE_FLAG_IFDIFF);

    /* The reason we use IFSET rather than IFDIFF is the belief that the IFDIFF
     * flag is mainly only for attributes that don't handle explicit unset well.
     * We may need to revisit the behaviour of this routine. */

    /* Get rid of trailing `;'. */
    if (p != c) {
        --p;
        if (*p == ';') {
            *p = '\0';
        }
    }

    return g_strdup (c);
}



/**
 *
 */
static void
sp_style_clear (SPStyle *style)
{
    SPObject *object;
    gint refcount;
    SPTextStyle *text;
    unsigned int text_private;
    int i;

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

    style->text->letterspacing_normal = TRUE;
    style->text->letterspacing.value = 0.0;
    style->text->letterspacing.computed = 0.0;
    style->text->letterspacing.set = FALSE;

    style->text->wordspacing_normal = TRUE;
    style->text->wordspacing.value = 0.0;
    style->text->wordspacing.computed = 0.0;
    style->text->wordspacing.set = FALSE;

    style->font_size.set = FALSE;
    style->font_size.type = SP_FONT_SIZE_LITERAL;
    style->font_size.value = SP_CSS_FONT_SIZE_MEDIUM;
    style->font_size.computed = 12.0;
    style->font_style.set = FALSE;
    style->font_style.value = style->font_style.computed = SP_CSS_FONT_STYLE_NORMAL;
    style->font_variant.set = FALSE;
    style->font_variant.value = style->font_variant.computed = SP_CSS_FONT_VARIANT_NORMAL;
    style->font_weight.set = FALSE;
    style->font_weight.value = SP_CSS_FONT_WEIGHT_NORMAL;
    style->font_weight.computed = SP_CSS_FONT_WEIGHT_400;
    style->font_stretch.set = FALSE;
    style->font_stretch.value = style->font_stretch.computed = SP_CSS_FONT_STRETCH_NORMAL;

    style->opacity.value = SP_SCALE24_MAX;
    style->visibility.set = FALSE;
    style->visibility.value = style->visibility.computed = SP_CSS_VISIBILITY_VISIBLE;
    style->display.set = FALSE;
    style->display.value = style->display.computed = SP_CSS_DISPLAY_INLINE;

    style->color.type = SP_PAINT_TYPE_COLOR;
    sp_color_set_rgb_float (&style->color.value.color, 0.0, 0.0, 0.0);

    style->fill.type = SP_PAINT_TYPE_COLOR;
    sp_color_set_rgb_float (&style->fill.value.color, 0.0, 0.0, 0.0);
    style->fill_opacity.value = SP_SCALE24_MAX;
    style->fill_rule.value = style->fill_rule.computed = SP_WIND_RULE_NONZERO;

    style->stroke.type = SP_PAINT_TYPE_NONE;
    style->stroke.set = FALSE;
    sp_color_set_rgb_float (&style->stroke.value.color, 0.0, 0.0, 0.0);
    style->stroke_opacity.value = SP_SCALE24_MAX;

    style->stroke_width.set = FALSE;
    style->stroke_width.unit = SP_CSS_UNIT_NONE;
    style->stroke_width.computed = 1.0;

    style->stroke_linecap.set = FALSE;
    style->stroke_linecap.value = style->stroke_linecap.computed = SP_STROKE_LINECAP_BUTT;
    style->stroke_linejoin.set = FALSE;
    style->stroke_linejoin.value = style->stroke_linejoin.computed = SP_STROKE_LINEJOIN_MITER;

    style->stroke_miterlimit.set = FALSE;
    style->stroke_miterlimit.value = 4.0;

    style->stroke_dash.n_dash = 0;
    style->stroke_dash.dash = NULL;
    style->stroke_dash.offset = 0.0;

    style->text_anchor.set = FALSE;
    style->text_anchor.value = style->text_anchor.computed = SP_CSS_TEXT_ANCHOR_START;
    style->writing_mode.set = FALSE;
    style->writing_mode.value = style->writing_mode.computed = SP_CSS_WRITING_MODE_LR;

    for (i=SP_MARKER_LOC; i<SP_MARKER_LOC_QTY; i++) {
        g_free(style->marker[i].value);
        style->marker[i].set      = FALSE;
    }
}



/**
 *
 */
static void
sp_style_read_dash(SPStyle *style, gchar const *str)
{
    /* Ref: http://www.w3.org/TR/SVG11/painting.html#StrokeDasharrayProperty */
    style->stroke_dasharray_set = TRUE;

    if (strcmp(str, "inherit") == 0) {
        style->stroke_dasharray_inherit = true;
        return;
    }
    style->stroke_dasharray_inherit = false;

    NRVpathDash &dash = style->stroke_dash;
    g_free(dash.dash);
    dash.dash = NULL;

    if (strcmp(str, "none") == 0) {
        dash.n_dash = 0;
        return;
    }

    gint n_dash = 0;
    gdouble d[64];
    gchar *e = NULL;

    while (e != str && n_dash < 64) {
        /* TODO: Should allow <length> rather than just a unitless (px) number. */
        d[n_dash] = g_ascii_strtod (str, (char **) &e);
        if (e != str) {
            n_dash += 1;
            str = e;
        }
        while (str && *str && !isalnum (*str)) str += 1;
    }

    if (n_dash > 0) {
        dash.dash = g_new(gdouble, n_dash);
        memcpy(dash.dash, d, sizeof(gdouble) * n_dash);
        dash.n_dash = n_dash;
    }
}



/**
 *
 */
void
sp_style_set_fill_color_alpha (SPStyle *style, const SPColor* color, gfloat a, unsigned int fill_set, unsigned int opacity_set)
{
    g_return_if_fail (style != NULL);
    g_return_if_fail (color != NULL);

    sp_style_paint_clear (style, &style->fill, TRUE, FALSE);

    style->fill.set = fill_set;
    style->fill.inherit = FALSE;
    style->fill.type = SP_PAINT_TYPE_COLOR;
    sp_color_copy (&style->fill.value.color, color);
    style->fill_opacity.set = opacity_set;
    style->fill_opacity.inherit = FALSE;
    style->fill_opacity.value = SP_SCALE24_FROM_FLOAT (a);

    if (style->object) {
        style->object->requestModified(SP_OBJECT_MODIFIED_FLAG | SP_OBJECT_STYLE_MODIFIED_FLAG);
    }
}



/**
 *
 */
void
sp_style_set_stroke_color_alpha (SPStyle *style, const SPColor* color, gfloat a, unsigned int stroke_set, unsigned int opacity_set)
{
    g_return_if_fail (style != NULL);

    sp_style_paint_clear (style, &style->stroke, TRUE, FALSE);

    style->stroke.set = stroke_set;
    style->stroke.inherit = FALSE;
    style->stroke.type = SP_PAINT_TYPE_COLOR;
    sp_color_copy (&style->stroke.value.color, color);
    style->stroke_opacity.set = opacity_set;
    style->stroke_opacity.inherit = FALSE;
    style->stroke_opacity.value = SP_SCALE24_FROM_FLOAT (a);

    if (style->object) {
        style->object->requestModified(SP_OBJECT_MODIFIED_FLAG | SP_OBJECT_STYLE_MODIFIED_FLAG);
    }
}



/**
 *
 */
void
sp_style_set_opacity (SPStyle *style, gfloat opacity, unsigned int opacity_set)
{
    g_return_if_fail (style != NULL);

    style->opacity.set = opacity_set;
    style->opacity.inherit = FALSE;
    style->opacity.value = SP_SCALE24_FROM_FLOAT (opacity);

    if (style->object) {
        style->object->requestModified(SP_OBJECT_MODIFIED_FLAG | SP_OBJECT_STYLE_MODIFIED_FLAG);
    }
}



/*#########################
## SPTextStyle operations
#########################*/


/**
 *
 */
static SPTextStyle *
sp_text_style_new (void)
{
    SPTextStyle *ts;

    ts = g_new0 (SPTextStyle, 1);

    ts->refcount = 1;

    sp_text_style_clear (ts);

    ts->font.value = g_strdup ("Bitstream Vera Sans");
    ts->font_family.value = g_strdup ("Bitstream Vera Sans");

    return ts;
}


/**
 *
 */
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



/**
 *
 */
static SPTextStyle *
sp_text_style_unref (SPTextStyle *st)
{
    st->refcount -= 1;

    if (st->refcount < 1) {
        g_free (st->font.value);
        g_free (st->font_family.value);
        g_free (st);
    }

    return NULL;
}



/**
 *
 */
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



/**
 *
 */
static guint
sp_text_style_write(gchar *p, guint const len, SPTextStyle const *const st, guint flags)
{
    gint d = 0;

    // We do not do diffing for text style
    if (flags == SP_STYLE_FLAG_IFDIFF)
        flags = SP_STYLE_FLAG_IFSET;

    d += sp_style_write_istring (p + d, len - d, "font-family", &st->font_family, NULL, flags);
    if ((flags == SP_STYLE_FLAG_ALWAYS) || st->letterspacing.set) {
        if (st->letterspacing.inherit) {
            d += g_snprintf(p + d, len - d, "letter-spacing:inherit;");
        } else if (st->letterspacing_normal) {
            d += g_snprintf(p + d, len - d, "letter-spacing:normal;");
        } else {
            d += sp_style_write_ilength(p + d, len - d, "letter-spacing", &st->letterspacing, NULL, flags);
        }
    }

    return d;
}



/**
 *
 */
static void
sp_style_read_ifloat (SPIFloat *val, const gchar *str)
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



/**
 *
 */
static void
sp_style_read_iscale24 (SPIScale24 *val, const gchar *str)
{
    if (!strcmp (str, "inherit")) {
        val->set = TRUE;
        val->inherit = TRUE;
    } else {
        gfloat value;
        if (sp_svg_number_read_f (str, &value)) {
            val->set = TRUE;
            val->inherit = FALSE;
            value = CLAMP (value, 0.0f, (gfloat) SP_SCALE24_MAX);
            val->value = SP_SCALE24_FROM_FLOAT (value);
        }
    }
}

/**
 * Reads a style value and performs lookup based on the given style value enumerations
 */
static void
sp_style_read_ienum(SPIEnum *val, gchar const *str, SPStyleEnum const *dict,
                    bool const can_explicitly_inherit)
{
    if ( can_explicitly_inherit && !strcmp(str, "inherit") ) {
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



/**
 *
 */
static void
sp_style_read_istring (SPIString *val, const gchar *str)
{
    g_free (val->value);

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



/**
 *
 */
static void
sp_style_read_ilength (SPILength *val, const gchar *str)
{
    if (!strcmp (str, "inherit")) {
        val->set = TRUE;
        val->inherit = TRUE;
    } else {
        gdouble value;
        gchar *e;
        /* fixme: Move this to standard place (Lauris) */
        value = g_ascii_strtod (str, &e);
        if ((const gchar *) e != str) {
            /* TODO: Allow the number of px per inch to vary (document preferences, X server or
             * whatever).  E.g. don't fill in computed here, do it at the same time as percentage
             * units are done. */
            if (!*e) {
                /* Userspace */
                val->unit = SP_CSS_UNIT_NONE;
                val->computed = value;
            } else if (!strcmp (e, "px")) {
                /* Userspace */
                val->unit = SP_CSS_UNIT_PX;
                val->computed = value;
            } else if (!strcmp (e, "pt")) {
                /* Userspace / DEVICESCALE */
                val->unit = SP_CSS_UNIT_PT;
                val->computed = value * PX_PER_PT;
            } else if (!strcmp (e, "pc")) {
                /* 1 pica = 12pt; FIXME: add it to SPUnit */
                val->unit = SP_CSS_UNIT_PC;
                val->computed = value * PX_PER_PT * 12;
            } else if (!strcmp (e, "mm")) {
                val->unit = SP_CSS_UNIT_MM;
                val->computed = value * PX_PER_MM;
            } else if (!strcmp (e, "cm")) {
                val->unit = SP_CSS_UNIT_CM;
                val->computed = value * PX_PER_CM;
            } else if (!strcmp (e, "in")) {
                val->unit = SP_CSS_UNIT_IN;
                val->computed = value * PX_PER_IN;
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



/**
 *
 */
static void
sp_style_read_icolor (SPIPaint *paint, const gchar *str, SPStyle *style, SPDocument *document)
{
    if (!strcmp (str, "inherit")) {
        paint->set = TRUE;
        paint->inherit = TRUE;
        paint->currentcolor = FALSE;
    } else {
        guint32 color;

        paint->type = SP_PAINT_TYPE_COLOR;
        color = sp_color_get_rgba32_ualpha (&paint->value.color, 0);
        color = sp_svg_read_color (str, color);
        sp_color_set_rgb_rgba32 (&paint->value.color, color);
        paint->set = TRUE;
        paint->inherit = FALSE;
        paint->currentcolor = FALSE;
    }
}



/**
 *
 */
static void
sp_style_read_ipaint (SPIPaint *paint, const gchar *str, SPStyle *style, SPDocument *document)
{
    while (isspace(*str)) {
        ++str;
    }
 
    if (!strcmp(str, "inherit")) {
        paint->set = TRUE;
        paint->inherit = TRUE;
        paint->currentcolor = FALSE;
    } else if (!strcmp(str, "currentColor")) {
        paint->set = TRUE;
        paint->inherit = FALSE;
        paint->currentcolor = TRUE;
    } else {
        guint32 color;
        if (!strncmp(str, "url", 3)) {
            paint->value.paint.uri = extract_uri(str);
            if (paint->value.paint.uri == NULL || *(paint->value.paint.uri) == '\0') {
                paint->type = SP_PAINT_TYPE_NONE;
                return;
            }
            paint->type = SP_PAINT_TYPE_PAINTSERVER;
            paint->set = TRUE;
            paint->inherit = FALSE;
            paint->currentcolor = FALSE;
            if (document) {
                SPObject *ps;
                ps = sp_uri_reference_resolve(document, str);
                if (ps && SP_IS_PAINT_SERVER(ps)) {
                    paint->value.paint.server = SP_PAINT_SERVER(ps);
                    if (!style->cloned) {
                        sp_object_href(SP_OBJECT(paint->value.paint.server), style);
                    } 
                    g_signal_connect(G_OBJECT(paint->value.paint.server), "release",
                                     G_CALLBACK(sp_style_paint_server_release), style);
                    g_signal_connect(G_OBJECT(paint->value.paint.server), "modified",
                                     G_CALLBACK(sp_style_paint_server_modified), style);
                } else {
		    paint->value.paint.server = NULL;
		}
            } 
            return;
        } else if (!strncmp(str, "none", 4)) {
            paint->type = SP_PAINT_TYPE_NONE;
            paint->set = TRUE;
            paint->inherit = FALSE;
            paint->currentcolor = FALSE;
            return;
        }

        paint->type = SP_PAINT_TYPE_COLOR;
        color = sp_color_get_rgba32_ualpha(&paint->value.color, 0);
        color = sp_svg_read_color(str, color);
        sp_color_set_rgb_rgba32(&paint->value.color, color);
        paint->set = TRUE;
        paint->inherit = FALSE;
        paint->currentcolor = FALSE;
    }
}



/**
 *
 */
static void
sp_style_read_ifontsize (SPIFontSize *val, const gchar *str)
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
        value = g_ascii_strtod (str, &e);
        if ((const gchar *) e != str) {
            if (!*e) {
                /* Userspace */
            } else if (!strcmp (e, "px")) {
                /* Userspace */
            } else if (!strcmp (e, "pt")) {
                /* Userspace * DEVICESCALE */
                value *= PX_PER_PT;
            } else if (!strcmp (e, "pc")) {
                /* 12pt */
                value *= PX_PER_PT * 12.0;
            } else if (!strcmp (e, "mm")) {
                value *= PX_PER_MM;
            } else if (!strcmp (e, "cm")) {
                value *= PX_PER_CM;
            } else if (!strcmp (e, "in")) {
                value *= PX_PER_IN;
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



/**
 *
 */
static void
sp_style_read_penum(SPIEnum *val, SPRepr *repr, gchar const *key, SPStyleEnum const *dict,
                    bool const can_explicitly_inherit)
{
    const gchar *str;
    str = sp_repr_attr (repr, key);
    if (str) {
        sp_style_read_ienum(val, str, dict, can_explicitly_inherit);
    }
}



/**
 *
 */
static void
sp_style_read_plength (SPILength *val, SPRepr *repr, const gchar *key)
{
    const gchar *str;
    str = sp_repr_attr (repr, key);
    if (str) {
        sp_style_read_ilength (val, str);
    }
}



/**
 *
 */
static void
sp_style_read_pfontsize (SPIFontSize *val, SPRepr *repr, const gchar *key)
{
    const gchar *str;
    str = sp_repr_attr (repr, key);
    if (str) {
        sp_style_read_ifontsize (val, str);
    }
}


/**
 *
 */
static gint
sp_style_write_ifloat(gchar *p, gint const len, gchar const *const key,
                      SPIFloat const *const val, SPIFloat const *const base, guint const flags)
{
	Inkscape::SVGOStringStream os;

  if ((flags & SP_STYLE_FLAG_ALWAYS) ||
        ((flags & SP_STYLE_FLAG_IFSET) && val->set) ||
        ((flags & SP_STYLE_FLAG_IFDIFF) && val->set && (!base->set || (val->value != base->value)))) {
        if (val->inherit) {
            return g_snprintf (p, len, "%s:inherit;", key);
        } else {
            os << key << ":" << val->value << ";";
			return g_strlcpy (p, os.str().c_str(), len);
        }
    }
    return 0;
}


/**
 *
 */
static gint
sp_style_write_iscale24(gchar *p, gint const len, gchar const *const key,
                        SPIScale24 const *const val, SPIScale24 const *const base,
                        guint const flags)
{
	Inkscape::SVGOStringStream os;

    if ((flags & SP_STYLE_FLAG_ALWAYS) ||
        ((flags & SP_STYLE_FLAG_IFSET) && val->set) ||
        ((flags & SP_STYLE_FLAG_IFDIFF) && val->set && (!base->set || (val->value != base->value)))) {
        if (val->inherit) {
            return g_snprintf (p, len, "%s:inherit;", key);
        } else {
            os << key << ":" << SP_SCALE24_TO_FLOAT(val->value) << ";";
			return g_strlcpy (p, os.str().c_str(), len);
        }
    }
    return 0;
}


/**
 *
 */
static gint
sp_style_write_ienum(gchar *p, gint const len, gchar const *const key,
                     SPStyleEnum const *const dict,
                     SPIEnum const *const val, SPIEnum const *const base, guint const flags)
{
    if ((flags & SP_STYLE_FLAG_ALWAYS) ||
        ((flags & SP_STYLE_FLAG_IFSET) && val->set) ||
        ((flags & SP_STYLE_FLAG_IFDIFF) && val->set && (!base->set || (val->computed != base->computed)))) {
        if (val->inherit) {
            return g_snprintf(p, len, "%s:inherit;", key);
        }
        unsigned int i;
        for (i = 0; dict[i].key; i++) {
            if (dict[i].value == static_cast< gint > (val->value) ) {
                return g_snprintf (p, len, "%s:%s;", key, dict[i].key);
            }
        }
    }
    return 0;
}



/**
 *
 */
static gint
sp_style_write_istring(gchar *p, gint const len, gchar const *const key,
                       SPIString const *const val, SPIString const *const base, guint const flags)
{
    if ((flags & SP_STYLE_FLAG_ALWAYS) ||
        ((flags & SP_STYLE_FLAG_IFSET) && val->set) ||
        ((flags & SP_STYLE_FLAG_IFDIFF) && val->set && (!base->set || strcmp (val->value, base->value)))) {
        if (val->inherit) {
            return g_snprintf (p, len, "%s:inherit;", key);
        } else {
            return g_snprintf (p, len, "%s:%s;", key, val->value);
        }
    }
    return 0;
}


/**
 *
 */
static bool
sp_length_differ(SPILength const *const a, SPILength const *const b)
{
    if (a->unit != b->unit) {
        if (a->unit == SP_CSS_UNIT_EM) return true;
        if (a->unit == SP_CSS_UNIT_EX) return true;
        if (a->unit == SP_CSS_UNIT_PERCENT) return true;
        if (b->unit == SP_CSS_UNIT_EM) return true;
        if (b->unit == SP_CSS_UNIT_EX) return true;
        if (b->unit == SP_CSS_UNIT_PERCENT) return true;
    }

    return (a->computed != b->computed);
}



/**
 *
 */
static gint
sp_style_write_ilength(gchar *p, gint const len, gchar const *const key,
                       SPILength const *const val, SPILength const *const base, guint const flags)
{
	Inkscape::SVGOStringStream os;

    if ((flags & SP_STYLE_FLAG_ALWAYS) ||
        ((flags & SP_STYLE_FLAG_IFSET) && val->set) ||
        ((flags & SP_STYLE_FLAG_IFDIFF) && val->set && (!base->set || sp_length_differ (val, base)))) {
        if (val->inherit) {
            return g_snprintf (p, len, "%s:inherit;", key);
        } else {
            switch (val->unit) {
            case SP_CSS_UNIT_NONE:
                os << key << ":" << val->computed << ";";
				return g_strlcpy (p, os.str().c_str(), len);
                break;
            case SP_CSS_UNIT_PX:
                os << key << ":" << val->computed << "px;";
				return g_strlcpy (p, os.str().c_str(), len);
                break;
            case SP_CSS_UNIT_PT:
                os << key << ":" << val->computed * PT_PER_PX << "pt;";
				return g_strlcpy (p, os.str().c_str(), len);
                break;
            case SP_CSS_UNIT_PC:
                os << key << ":" << val->computed * PT_PER_PX / 12.0 << "pc;";
				return g_strlcpy (p, os.str().c_str(), len);
                break;
            case SP_CSS_UNIT_MM:
                os << key << ":" << val->computed * MM_PER_PX << "mm;";
				return g_strlcpy (p, os.str().c_str(), len);
                break;
            case SP_CSS_UNIT_CM:
                os << key << ":" << val->computed * CM_PER_PX << "cm;";
				return g_strlcpy (p, os.str().c_str(), len);
                break;
            case SP_CSS_UNIT_IN:
                os << key << ":" << val->computed * IN_PER_PX << "in;";
				return g_strlcpy (p, os.str().c_str(), len);
                break;
            case SP_CSS_UNIT_EM:
                os << key << ":" << val->value << "em;";
				return g_strlcpy (p, os.str().c_str(), len);
                break;
            case SP_CSS_UNIT_EX:
                os << key << ":" << val->value << "ex;";
				return g_strlcpy (p, os.str().c_str(), len);
                break;
            case SP_CSS_UNIT_PERCENT:
                os << key << ":" << val->value << "%;";
				return g_strlcpy (p, os.str().c_str(), len);
                break;
            default:
                /* Invalid */
                break;
            }
        }
    }
    return 0;
}



/**
 *
 */
static bool
sp_paint_differ(SPIPaint const *const a, SPIPaint const *const b)
{
    if (a->type != b->type)
        return true;
    if (a->type == SP_PAINT_TYPE_COLOR)
        return !sp_color_is_equal (&a->value.color, &b->value.color);
    if (a->type == SP_PAINT_TYPE_PAINTSERVER)
        return (a->value.paint.server != b->value.paint.server);
    return false;
}



/**
 *
 */
static gint
sp_style_write_ipaint(gchar *b, gint const len, gchar const *const key,
                      SPIPaint const *const paint, SPIPaint const *const base, guint const flags)
{
    if ((flags & SP_STYLE_FLAG_ALWAYS) ||
        ((flags & SP_STYLE_FLAG_IFSET) && paint->set) ||
        ((flags & SP_STYLE_FLAG_IFDIFF) && paint->set && (!base->set || sp_paint_differ (paint, base)))) {
        if (paint->inherit) {
            return g_snprintf (b, len, "%s:inherit;", key);
        } else if (paint->currentcolor) {
            return g_snprintf (b, len, "%s:currentColor;", key);
        } else {
            switch (paint->type) {
            case SP_PAINT_TYPE_COLOR:
                return g_snprintf (b, len, "%s:#%06x;", key, sp_color_get_rgba32_falpha (&paint->value.color, 0.0) >> 8);
                break;
            case SP_PAINT_TYPE_PAINTSERVER:
                    return g_snprintf (b, len, "%s:url(%s);", key, paint->value.paint.uri);
                break;
            default:
                break;
            }
            return g_snprintf (b, len, "%s:none;", key);
        }
    }
    return 0;
}


/**
 *
 */
static bool
sp_fontsize_differ(SPIFontSize const *const a, SPIFontSize const *const b)
{
    if (a->type != b->type)
        return true;
    if (a->type == SP_FONT_SIZE_LENGTH) {
        if (a->computed != b->computed)
            return true;
    } else {
        if (a->value != b->value)
            return true;
    }
    return false;
}


/**
 *
 */
static gint
sp_style_write_ifontsize(gchar *p, gint const len, gchar const *key,
                         SPIFontSize const *const val, SPIFontSize const *const base,
                         guint const flags)
{
    if ((flags & SP_STYLE_FLAG_ALWAYS) ||
        ((flags & SP_STYLE_FLAG_IFSET) && val->set) ||
        ((flags & SP_STYLE_FLAG_IFDIFF) && val->set && (!base->set || sp_fontsize_differ (val, base)))) {
        if (val->inherit) {
            return g_snprintf (p, len, "%s:inherit;", key);
        } else if (val->type == SP_FONT_SIZE_LITERAL) {
            unsigned int i;
            for (i = 0; enum_font_size[i].key; i++) {
                if (enum_font_size[i].value == static_cast< gint > (val->value) ) {
                    return g_snprintf (p, len, "%s:%s;", key, enum_font_size[i].key);
                }
            }
        } else if (val->type == SP_FONT_SIZE_LENGTH) {
			Inkscape::SVGOStringStream os;
		    os << key << ":" << val->computed << ";";
			return g_strlcpy (p, os.str().c_str(), len);
        } else if (val->type == SP_FONT_SIZE_PERCENTAGE) {
			Inkscape::SVGOStringStream os;
		    os << key << ":" << (SP_F8_16_TO_FLOAT (val->value) * 100.0) << "%;";
			return g_strlcpy (p, os.str().c_str(), len);
        }
    }
    return 0;
}


/**
 *
 */
static void
sp_style_paint_clear (SPStyle *style, SPIPaint *paint,
            unsigned int hunref, unsigned int unset)
{
    if (hunref && (paint->type == SP_PAINT_TYPE_PAINTSERVER) && paint->value.paint.server) {
        if (!style->cloned) {
            sp_object_hunref (SP_OBJECT (paint->value.paint.server), style);
        } 
        // gtk_signal_disconnect_by_data (GTK_OBJECT (paint->value.server),
        //        style);
        g_signal_handlers_disconnect_matched (G_OBJECT(paint->value.paint.server),
                  G_SIGNAL_MATCH_DATA, 0, 0, NULL, NULL, style);
        paint->value.paint.server = NULL;
        paint->value.paint.uri = NULL;
        paint->type = SP_PAINT_TYPE_NONE;
    }

    if (unset) {
        paint->set = FALSE;
        paint->inherit = FALSE;
    }
}

void
sp_style_unset_property_attrs (SPObject *o)
{
    if (!o) return;

    SPStyle *style = SP_OBJECT_STYLE (o);
    if (!style) return;

    SPRepr *repr = SP_OBJECT_REPR (o);
    if (!repr) return;

    if (style->opacity.set) {
        sp_repr_set_attr (repr, "opacity", NULL);
    }
    if (style->color.set) {
        sp_repr_set_attr (repr, "color", NULL);
    }
    if (style->fill.set) {
        sp_repr_set_attr (repr, "fill", NULL);
    }
    if (style->fill_opacity.set) {
        sp_repr_set_attr (repr, "fill-opacity", NULL);
    }
    if (style->fill_rule.set) {
        sp_repr_set_attr (repr, "fill-rule", NULL);
    }
    if (style->stroke.set) {
        sp_repr_set_attr (repr, "stroke", NULL);
    }
    if (style->stroke_width.set) {
        sp_repr_set_attr (repr, "stroke-width", NULL);
    }
    if (style->stroke_linecap.set) {
        sp_repr_set_attr (repr, "stroke-linecap", NULL);
    }
    if (style->stroke_linejoin.set) {
        sp_repr_set_attr (repr, "stroke-linejoin", NULL);
    }
    if (style->marker[SP_MARKER_LOC].set) {
        sp_repr_set_attr (repr, "marker", NULL);
    }
    if (style->marker[SP_MARKER_LOC_START].set) {
        sp_repr_set_attr (repr, "marker-start", NULL);
    }
    if (style->marker[SP_MARKER_LOC_MID].set) {
        sp_repr_set_attr (repr, "marker-mid", NULL);
    }
    if (style->marker[SP_MARKER_LOC_END].set) {
        sp_repr_set_attr (repr, "marker-end", NULL);
    }
    if (style->stroke_opacity.set) {
        sp_repr_set_attr (repr, "stroke-opacity", NULL);
    }
    if (style->stroke_dasharray_set) {
        sp_repr_set_attr (repr, "stroke-dasharray", NULL);
    }
    if (style->stroke_dashoffset_set) {
        sp_repr_set_attr (repr, "stroke-dashoffset", NULL);
    }
    if (style->text_private && style->text->font_family.set) {
        sp_repr_set_attr (repr, "font-family", NULL);
    }
    if (style->text_anchor.set) {
        sp_repr_set_attr (repr, "text-anchor", NULL);
    }
    if (style->writing_mode.set) {
        sp_repr_set_attr (repr, "writing_mode", NULL);
    }
}

/**
 * \pre object != NULL
 * \pre flags in {IFSET, ALWAYS}.
 */
SPCSSAttr *
sp_css_attr_from_style (SPObject *object, guint flags)
{
    g_return_val_if_fail(((flags == SP_STYLE_FLAG_IFSET) ||
                          (flags == SP_STYLE_FLAG_ALWAYS)  ),
                         NULL);
    gchar *style_str = sp_style_write_string (SP_OBJECT_STYLE (object), flags);
    SPCSSAttr *css = sp_repr_css_attr_new ();
    sp_repr_css_attr_add_from_string (css, style_str);
    g_free (style_str);
    return css;
}

/**
Unset any text-related properties
*/
SPCSSAttr *
sp_css_attr_unset_text (SPCSSAttr *css)
{
    sp_repr_css_set_property (css, "font", NULL); // not implemented yet
    sp_repr_css_set_property (css, "font-size", NULL);
    sp_repr_css_set_property (css, "font-size-adjust", NULL); // not implemented yet
    sp_repr_css_set_property (css, "font-style", NULL);
    sp_repr_css_set_property (css, "font-variant", NULL);
    sp_repr_css_set_property (css, "font-weight", NULL);
    sp_repr_css_set_property (css, "font-stretch", NULL);
    sp_repr_css_set_property (css, "font-family", NULL);
    sp_repr_css_set_property (css, "letter-spacing", NULL);
    sp_repr_css_set_property (css, "word-spacing", NULL); // not implemented yet
    sp_repr_css_set_property (css, "kerning", NULL); // not implemented yet
    sp_repr_css_set_property (css, "text-decoration", NULL); // not implemented yet
    sp_repr_css_set_property (css, "text-anchor", NULL);
    sp_repr_css_set_property (css, "dominant-baseline", NULL); // not implemented yet
    sp_repr_css_set_property (css, "alignment-baseline", NULL); // not implemented yet
    sp_repr_css_set_property (css, "baseline-shift", NULL); // not implemented yet
    sp_repr_css_set_property (css, "writing-mode", NULL);

    return css;
}

bool
is_url (const char *p)
{
    if (p == NULL) 
        return false;
// FIXME: I'm not sure if this applies to SVG as well, but CSS2 says any URIs in property values must start with 'url('
    return (g_ascii_strncasecmp (p, "url(", 4) == 0);
}

/**
Unset any properties that contain URI values. Used for storing style that will be reused across
documents when carrying the referenced defs is impractical.
 */
SPCSSAttr *
sp_css_attr_unset_uris (SPCSSAttr *css)
{
// All properties that may hold <uri> or <paint> according to SVG 1.1
    if (is_url (sp_repr_css_property (css, "clip-path", NULL))) sp_repr_css_set_property (css, "clip-path", NULL);
    if (is_url (sp_repr_css_property (css, "color-profile", NULL))) sp_repr_css_set_property (css, "color-profile", NULL);
    if (is_url (sp_repr_css_property (css, "cursor", NULL))) sp_repr_css_set_property (css, "cursor", NULL);
    if (is_url (sp_repr_css_property (css, "filter", NULL))) sp_repr_css_set_property (css, "filter", NULL);
    if (is_url (sp_repr_css_property (css, "marker-start", NULL))) sp_repr_css_set_property (css, "marker-start", NULL);
    if (is_url (sp_repr_css_property (css, "marker-mid", NULL))) sp_repr_css_set_property (css, "marker-mid", NULL);
    if (is_url (sp_repr_css_property (css, "marker-end", NULL))) sp_repr_css_set_property (css, "marker-end", NULL);
    if (is_url (sp_repr_css_property (css, "mask", NULL))) sp_repr_css_set_property (css, "mask", NULL);
    if (is_url (sp_repr_css_property (css, "fill", NULL))) sp_repr_css_set_property (css, "fill", NULL);
    if (is_url (sp_repr_css_property (css, "stroke", NULL))) sp_repr_css_set_property (css, "stroke", NULL);

    return css;
}

void
sp_css_attr_scale_property_single (SPCSSAttr *css, const gchar *property, double ex)
{
    const gchar *w = sp_repr_css_property (css, property, NULL);
    if (w) {
        gchar *units = NULL;
        double wd = g_ascii_strtod (w, &units) * ex;
        if (w == units) // nothing converted, non-numeric value
            return;
        Inkscape::SVGOStringStream os;
        //g_print ("%s; %g; %g %s\n", w, ex, wd, units);
        os << wd << units; // reattach units!
        sp_repr_css_set_property (css, property, os.str().c_str());
    }
}

/**
Scale any properties that may hold <length> by ex
 */
SPCSSAttr *
sp_css_attr_scale (SPCSSAttr *css, double ex)
{
    sp_css_attr_scale_property_single (css, "baseline-shift", ex);
    sp_css_attr_scale_property_single (css, "stroke-width", ex);
   //FIXME: scale stroke-dashoffset too; but only after making scale_property_list for stroke-dasharray
    sp_css_attr_scale_property_single (css, "font-size", ex);
    sp_css_attr_scale_property_single (css, "kerning", ex);
    sp_css_attr_scale_property_single (css, "letter-spacing", ex);
    sp_css_attr_scale_property_single (css, "word-spacing", ex);

    return css;
}



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
