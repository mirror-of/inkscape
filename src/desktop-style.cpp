#define __SP_DESKTOP_STYLE_C__

/*
 * Desktop style management
 *
 * Authors:
 *   bulia byak
 *
 * Copyright (C) 2004 authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "macros.h"
#include "desktop.h"
#include "sp-object.h"
#include "color.h"
#include "color-rgba.h"
#include "svg/stringstream.h"
#include "svg/svg.h"
#include "selection.h"
#include "sp-tspan.h"
#include "fontsize-expansion.h"
#include "inkscape.h"
#include "style.h"
#include "prefs-utils.h"
#include "sp-use.h"
#include "sp-flowtext.h"
#include "sp-flowregion.h"
#include "xml/repr.h"

#include "desktop-style.h"

void
sp_desktop_set_color(SPDesktop *desktop, ColorRGBA const &color, bool is_relative, bool fill)
{
    if (is_relative) {
        g_warning("FIXME: relative color setting not yet implemented");
        return;
    }

    guint32 rgba = SP_RGBA32_F_COMPOSE(color[0], color[1], color[2], color[3]);
    gchar b[64];
    sp_svg_write_color(b, 64, rgba);
    SPCSSAttr *css = sp_repr_css_attr_new();
    if (fill) {
        sp_repr_css_set_property(css, "fill", b);
        Inkscape::SVGOStringStream osalpha;
        osalpha << color[3];
        sp_repr_css_set_property(css, "fill-opacity", osalpha.str().c_str());
    } else {
        sp_repr_css_set_property(css, "stroke", b);
        Inkscape::SVGOStringStream osalpha;
        osalpha << color[3];
        sp_repr_css_set_property(css, "stroke-opacity", osalpha.str().c_str());
    }

    sp_desktop_set_style(desktop, css);

    sp_repr_css_attr_unref(css);
}

void
sp_desktop_apply_css_recursive(SPObject *o, SPCSSAttr *css, bool skip_lines)
{
    // non-items should not have style
    if (!SP_IS_ITEM(o))
        return;

    // 1. tspans with role=line are not regular objects in that they are not supposed to have style of their own,
    // but must always inherit from the parent text. Same for textPath.
    // However, if the line tspan or textPath contains some style (old file?), we reluctantly set our style to it too.

    // 2. Generally we allow setting style on clones (though fill&stroke currently forbids this,
    // will be fixed) but when it's inside flowRegion, do not touch it; it's just styleless shape
    // (because that's how Inkscape does flowtext). We also should not set style to its parents
    // because it will be inherited. So we skip them.

    if (!(skip_lines
          && ((SP_IS_TSPAN(o) && SP_TSPAN(o)->role == SP_TSPAN_ROLE_LINE) || SP_IS_TEXTPATH(o))
          && !SP_OBJECT_REPR(o)->attribute("style"))
        &&
        !(SP_IS_FLOWTEXT(o) ||
          SP_IS_FLOWREGION(o) ||
          SP_IS_FLOWREGIONEXCLUDE(o) ||
          (SP_IS_USE(o) &&
           SP_OBJECT_PARENT(o) &&
           (SP_IS_FLOWREGION(SP_OBJECT_PARENT(o)) ||
            SP_IS_FLOWREGIONEXCLUDE(SP_OBJECT_PARENT(o))
           )
          )
         )
        ) {

        SPCSSAttr *css_set = sp_repr_css_attr_new();
        sp_repr_css_merge(css_set, css);

        // Scale the style by the inverse of the accumulated parent transform in the paste context.
        {
            NR::Matrix const local(sp_item_i2doc_affine(SP_ITEM(o)));
            double const ex(NR::expansion(local));
            if ( ( ex != 0. )
                 && ( ex != 1. ) ) {
                sp_css_attr_scale(css_set, 1/ex);
            }
        }

        sp_repr_css_change(SP_OBJECT_REPR(o), css_set, "style");

        sp_repr_css_attr_unref(css_set);
    }

    // setting style on child of clone spills into the clone original (via shared repr), don't do it!
    if (SP_IS_USE(o))
        return;

    for (SPObject *child = sp_object_first_child(SP_OBJECT(o)) ; child != NULL ; child = SP_OBJECT_NEXT(child) ) {
        if (sp_repr_css_property(css, "opacity", NULL) != NULL) {
            // Unset properties which are accumulating and thus should not be set recursively.
            // For example, setting opacity 0.5 on a group recursively would result in the visible opacity of 0.25 for an item in the group.
            SPCSSAttr *css_recurse = sp_repr_css_attr_new();
            sp_repr_css_merge(css_recurse, css);
            sp_repr_css_set_property(css_recurse, "opacity", NULL);
            sp_desktop_apply_css_recursive(child, css_recurse, skip_lines);
            sp_repr_css_attr_unref(css_recurse);
        } else {
            sp_desktop_apply_css_recursive(child, css, skip_lines);
        }
    }
}

void
sp_desktop_set_style(SPDesktop *desktop, SPCSSAttr *css, bool change)
{
// 1. Set internal value
    sp_repr_css_merge(desktop->current, css);

// 1a. Write to prefs; make a copy and unset any URIs first
    SPCSSAttr *css_write = sp_repr_css_attr_new();
    sp_repr_css_merge(css_write, css);
    sp_css_attr_unset_uris(css_write);
    sp_repr_css_change(inkscape_get_repr(INKSCAPE, "desktop"), css_write, "style");
    sp_repr_css_attr_unref(css_write);

    if (!change)
        return;

// 2. Emit signal
    bool intercepted = desktop->_set_style_signal.emit(css);

// FIXME: in set_style, compensate pattern and gradient fills, stroke width, rect corners, font
// size for the object's own transform so that pasting fills does not depend on preserve/optimize

// 3. If nobody has intercepted the signal, apply the style to the selection
    if (!intercepted) {
        for (GSList const *i = desktop->selection->itemList(); i != NULL; i = i->next) {
            sp_desktop_apply_css_recursive(SP_OBJECT(i->data), css, true);
        }
    }
}

SPCSSAttr *
sp_desktop_get_style(SPDesktop *desktop, bool with_text)
{
    SPCSSAttr *css = sp_repr_css_attr_new();
    sp_repr_css_merge(css, desktop->current);
    if (!css->attributeList()) {
        sp_repr_css_attr_unref(css);
        return NULL;
    } else {
        if (!with_text) {
            css = sp_css_attr_unset_text(css);
        }
        return css;
    }
}

guint32
sp_desktop_get_color(SPDesktop *desktop, bool is_fill)
{
    guint32 r = 0; // if there's no color, return black
    gchar const *property = sp_repr_css_property(desktop->current,
                                                 is_fill ? "fill" : "stroke",
                                                 "#000");

    if (desktop->current && property) { // if there is style and the property in it,
        if (strncmp(property, "url", 3)) { // and if it's not url,
            // read it
            r = sp_svg_read_color(property, r);
        }
    }

    return r;
}

void
sp_desktop_apply_style_tool(SPDesktop *desktop, Inkscape::XML::Node *repr, char const *tool, bool with_text)
{
    SPCSSAttr *css_current = sp_desktop_get_style(desktop, with_text);
    if ((prefs_get_int_attribute(tool, "usecurrent", 0) != 0) && css_current) {
        sp_repr_css_set(repr, css_current, "style");
    } else {
        Inkscape::XML::Node *tool_repr = inkscape_get_repr(INKSCAPE, tool);
        if (tool_repr) {
            SPCSSAttr *css = sp_repr_css_attr_inherited(tool_repr, "style");
            sp_repr_css_set(repr, css, "style");
            sp_repr_css_attr_unref(css);
        }
    }
    if (css_current) {
        sp_repr_css_attr_unref(css_current);
    }
}

/**
Returns the font size (in SVG pixels) of the text tool style (if text tool uses its own style) or desktop style (otherwise)
*/
double
sp_desktop_get_font_size_tool(SPDesktop *desktop)
{
    gchar const *desktop_style = inkscape_get_repr(INKSCAPE, "desktop")->attribute("style");
    gchar const *style_str = NULL;
    if ((prefs_get_int_attribute("tools.text", "usecurrent", 0) != 0) && desktop_style) {
        style_str = desktop_style;
    } else {
        Inkscape::XML::Node *tool_repr = inkscape_get_repr(INKSCAPE, "tools.text");
        if (tool_repr) {
            style_str = tool_repr->attribute("style");
        }
    }

    double ret = 12;
    if (style_str) {
        SPStyle *style = sp_style_new();
        sp_style_merge_from_style_string(style, style_str);
        ret = style->font_size.computed;
        sp_style_unref(style);
    }
    return ret;
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :
