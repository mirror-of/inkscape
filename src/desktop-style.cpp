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
#include "xml/repr-private.h"
#include "selection.h"
#include "sp-tspan.h"
#include "inkscape.h"
#include "style.h"
#include "prefs-utils.h"

#include "desktop-style.h"

void
sp_desktop_set_color (SPDesktop *desktop, const ColorRGBA &color, bool is_relative, bool fill)
{
    if (is_relative) {
        g_warning ("FIXME: relative color setting not yet implemented");
        return;
    }

    guint32 rgba = SP_RGBA32_F_COMPOSE(color[0], color[1], color[2], color[3]);
    gchar b[64];
    sp_svg_write_color (b, 64, rgba);
    SPCSSAttr *css = sp_repr_css_attr_new ();
    if (fill) {
        sp_repr_css_set_property (css, "fill", b);
        Inkscape::SVGOStringStream osalpha;
        osalpha << color[3];
        sp_repr_css_set_property (css, "fill-opacity", osalpha.str().c_str());
    } else {
        sp_repr_css_set_property (css, "stroke", b);
        Inkscape::SVGOStringStream osalpha;
        osalpha << color[3];
        sp_repr_css_set_property (css, "stroke-opacity", osalpha.str().c_str());
    }

    sp_desktop_set_style (desktop, css);

    sp_repr_css_attr_unref (css);
}

void
sp_desktop_apply_css_recursive (SPObject *o, SPCSSAttr *css, bool skip_lines)
{
    // tspans with role=line are not regular objects in that they are not assignable style of their own,
    // but must always inherit from the parent text
    // However, if the line tspan contains some style (old file?), we reluctantly set our style to it too
    if (!(skip_lines && SP_IS_TSPAN(o) && SP_TSPAN(o)->role == SP_TSPAN_ROLE_LINE && !sp_repr_attr(SP_OBJECT_REPR(o), "style"))) {
        sp_repr_css_change (SP_OBJECT_REPR (o), css, "style");
    }

    for (SPObject *child = sp_object_first_child(SP_OBJECT(o)) ; child != NULL ; child = SP_OBJECT_NEXT(child) ) {
        sp_desktop_apply_css_recursive (child, css, skip_lines);
    }
}

void
sp_desktop_set_style (SPDesktop *desktop, SPCSSAttr *css, bool change)
{
// 1. Set internal value
    sp_repr_css_merge (desktop->current, css);

// 1a. Write to prefs; make a copy and unset any URIs first
    SPCSSAttr *css_write = sp_repr_css_attr_new ();
    sp_repr_css_merge (css_write, css);
    sp_css_attr_unset_uris (css_write);
    sp_repr_css_change (inkscape_get_repr (INKSCAPE, "desktop"), css_write, "style");
    sp_repr_css_attr_unref (css_write);

    if (!change) 
        return;

// 2. Emit signal
    bool intercepted = desktop->_set_style_signal.emit (css);

// FIXME: in set_style, compensate pattern and gradient fills, stroke width, rect corners, font
// size for the object's own transform so that pasting fills does not depend on preserve/optimize

// 3. If nobody has intercepted the signal, apply the style to the selection
    if (!intercepted) {
        for (const GSList *i = desktop->selection->itemList(); i != NULL; i = i->next) {
            sp_desktop_apply_css_recursive (SP_OBJECT (i->data), css, true);
        }
    }
}

SPCSSAttr *
sp_desktop_get_style (SPDesktop *desktop, bool with_text)
{
	SPCSSAttr *css = sp_repr_css_attr_new ();
	sp_repr_css_merge (css, desktop->current);
	if (((SPRepr *) css)->attributes == NULL) {
		sp_repr_css_attr_unref(css);
		return NULL;
	} else {
		if (!with_text) {
			css = sp_css_attr_unset_text (css);
		}
		return css;
	}
}

guint32 
sp_desktop_get_color (SPDesktop *desktop, bool is_fill)
{
    guint32 r = 0; // if there's no color, return black
    const gchar *property = sp_repr_css_property (desktop->current, is_fill? "fill" : "stroke", "#000");

    if (desktop->current && property) { // if there is style and the property in it,
        if (strncmp(property, "url", 3)) { // and if it's not url,
             // read it
            r = sp_svg_read_color (property, r);
        }
    }

    return r;
}

void
sp_desktop_apply_style_tool (SPDesktop *desktop, SPRepr *repr, const char *tool, bool with_text)
{
	SPCSSAttr *css_current = sp_desktop_get_style (desktop, with_text);
	if ((prefs_get_int_attribute (tool, "usecurrent", 0) != 0) && css_current) {
		sp_repr_css_set(repr, css_current, "style");
		sp_repr_css_attr_unref(css_current);
	} else {
		SPRepr *tool_repr = inkscape_get_repr(INKSCAPE, tool);
		if (tool_repr) {
			SPCSSAttr *css = sp_repr_css_attr_inherited (tool_repr, "style");
			sp_repr_css_set (repr, css, "style");
			sp_repr_css_attr_unref (css);
		}
	}
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
