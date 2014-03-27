#ifndef SEEN_SP_STYLE_H
#define SEEN_SP_STYLE_H

/** \file
 * SPStyle - a style object for SPItem objects
 */
/* Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   Jon A. Cruz <jon@joncruz.org>
 *
 * Copyright (C) 2010 Jon A. Cruz
 * Copyright (C) 2001-2002 Lauris Kaplinski
 * Copyright (C) 2001 Ximian, Inc.
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "style-enums.h"
#include "style-internal.h"

#include <stddef.h>
#include <sigc++/connection.h>
#include <iostream>
#include <vector>

namespace Inkscape {
namespace XML {
class Node;
}
}

#include "libcroco/cr-declaration.h"
#include "libcroco/cr-prop-list.h"
//struct CRDeclaration;
//struct CRPropList;

/// An SVG style object.
class SPStyle {

public:
    SPStyle(SPDocument *document = NULL); // emf, wmf export uses without document
    SPStyle(SPObject *object);
    ~SPStyle();
    void clear();
    void read(SPObject *object, Inkscape::XML::Node *repr);
    void read_from_object(SPObject *object);
    void read_from_prefs(Glib::ustring const &path);
    void read_if_unset( gint id, gchar const *val );
    Glib::ustring write( guint const flags, SPStyle const *const base = NULL ) const;
    void cascade( SPStyle const *const parent );
    void merge(   SPStyle const *const parent );
    bool operator==(const SPStyle& rhs);

    int ref()   { ++refcount; return refcount; }
    int unref() { --refcount; return refcount; }

//FIXME: Make private
public:
    void merge_string( gchar const *const p );
    void merge_decl_list( CRDeclaration const *const decl_list );
    void merge_decl(      CRDeclaration const *const decl );
    void merge_props( CRPropList *const props );
    void merge_object_stylesheet( SPObject const *const object );

// FIXME: Make private
public:
    int refcount;
    static int count;

public:
    /** Object we are attached to */
    SPObject *object;
    /** Document we are associated with */
    SPDocument *document;

    /// Pointers to all the properties (for looping through them)
    std::vector<SPIBase *> properties;

    /** Our font style component */
    SPFontStyle *text; // FIXME: Break into font, font-family, ...

    /* CSS2 */
    /* Font */
    /** Size of the font */
    SPIFontSize font_size;
    /** Style of the font */
    SPIEnum font_style;
    /** Which substyle of the font */
    SPIEnum font_variant;
    /** Weight of the font */
    SPIEnum font_weight;
    /** Stretch of the font */
    SPIEnum font_stretch;

    /** First line indent of paragraphs (css2 16.1) */
    SPILength text_indent;
    /** text alignment (css2 16.2) (not to be confused with text-anchor) */
    SPIEnum text_align;
    /** text decoration (css2 16.3.1) */
    SPITextDecoration      text_decoration; 
    /** CSS 3 2.1, 2.2, 2.3 */
    /** Not done yet, test_decoration3        = css3 2.4*/
    SPITextDecorationLine  text_decoration_line;
    SPITextDecorationStyle text_decoration_style;  // SPIEnum? Only one can be set at time.
    //SPIColor               text_decoration_color; FIXME
    SPIPaint               text_decoration_color;
    // used to implement text_decoration, not saved to or read from SVG file
    SPITextDecorationData  text_decoration_data;

    // 16.3.2 is text-shadow. That's complicated.
    /** Line spacing (css2 10.8.1) */
    SPILengthOrNormal line_height;
    /** letter spacing (css2 16.4) */
    SPILengthOrNormal letter_spacing;
    /** word spacing (also css2 16.4) */
    SPILengthOrNormal word_spacing;
    /** capitalization (css2 16.5) */
    SPIEnum text_transform;

    /* CSS3 Text */
    /** text direction (css3 text 3.2) */
    SPIEnum direction;
    /** block progression (css3 text 3.2) */
    SPIEnum block_progression;
    /** Writing mode (css3 text 3.2 and svg1.1 10.7.2) */
    SPIEnum writing_mode;
    /** Baseline shift (svg1.1 10.9.2) */
    SPIBaselineShift baseline_shift;

    /* SVG */
    /** Anchor of the text (svg1.1 10.9.1) */
    SPIEnum text_anchor;

    /* Misc attributes */
    unsigned clip_set : 1;
    unsigned color_set : 1;
    unsigned cursor_set : 1;
    unsigned overflow_set : 1;
    unsigned clip_path_set : 1;
    unsigned mask_set : 1;

    /** clip-rule: 0 nonzero, 1 evenodd */
    SPIEnum clip_rule;

    /** display */
    SPIEnum display;

    /** overflow */
    SPIEnum overflow;

    /** visibility */
    SPIEnum visibility;

    /** opacity */
    SPIScale24 opacity;

    /** mix-blend-mode:  CSS Compositing and Blending Level 1 */
    SPIEnum isolation;
    // Could be shared with Filter blending mode
    SPIEnum blend_mode;

    /** color */
    // SPIColor color; FIXME
    SPIPaint color;
    /** color-interpolation */
    SPIEnum color_interpolation;
    /** color-interpolation-filters */
    SPIEnum color_interpolation_filters;

    /** fill */
    SPIPaint fill;
    /** fill-opacity */
    SPIScale24 fill_opacity;
    /** fill-rule: 0 nonzero, 1 evenodd */
    SPIEnum fill_rule;

    /** stroke */
    SPIPaint stroke;
    /** stroke-width */
    SPILength stroke_width;
    /** stroke-linecap */
    SPIEnum stroke_linecap;
    /** stroke-linejoin */
    SPIEnum stroke_linejoin;
    /** stroke-miterlimit */
    SPIFloat stroke_miterlimit;
    /** stroke-dasharray */
    SPIDashArray stroke_dasharray;
    /** stroke-dashoffset */
    SPILength stroke_dashoffset;
    /** stroke-opacity */
    SPIScale24 stroke_opacity;

    /** Marker list */
    SPIString marker[SP_MARKER_LOC_QTY];

    SPIPaintOrder paint_order;

    /** Filter effect */
    SPIFilter filter;

    SPIEnum filter_blend_mode;

   /** normally not used, but duplicates the Gaussian blur deviation (if any) from the attached
        filter when the style is used for querying */
    SPILength filter_gaussianBlur_deviation;

    /** hints on how to render: e.g. speed vs. accuracy.
     * As of April, 2013, only image_rendering used. */
    SPIEnum color_rendering;
    SPIEnum image_rendering;
    SPIEnum shape_rendering;
    SPIEnum text_rendering;

    /** enable-background, used for defining where filter effects get
     * their background image */
    SPIEnum enable_background;

    /// style belongs to a cloned object
    bool cloned;

    sigc::connection release_connection;

    sigc::connection filter_modified_connection;
    sigc::connection fill_ps_modified_connection;
    sigc::connection stroke_ps_modified_connection;

    SPObject       *getFilter()          { return (filter.href) ? filter.href->getObject() : NULL; }
    SPObject const *getFilter()    const { return (filter.href) ? filter.href->getObject() : NULL; }
    gchar    const *getFilterURI() const { return (filter.href) ? filter.href->getURI()->toString() : NULL; }

    SPPaintServer       *getFillPaintServer()         { return (fill.value.href) ? fill.value.href->getObject() : NULL; }
    SPPaintServer const *getFillPaintServer()   const { return (fill.value.href) ? fill.value.href->getObject() : NULL; }
    gchar         const *getFillURI()           const { return (fill.value.href) ? fill.value.href->getURI()->toString() : NULL; }

    SPPaintServer       *getStrokePaintServer()       { return (stroke.value.href) ? stroke.value.href->getObject() : NULL; }
    SPPaintServer const *getStrokePaintServer() const { return (stroke.value.href) ? stroke.value.href->getObject() : NULL; }
    gchar        const  *getStrokeURI()         const { return (stroke.value.href) ? stroke.value.href->getURI()->toString() : NULL; }
};

SPStyle *sp_style_new(SPDocument *document); // SPStyle::SPStyle( SPDocument *document = NULL );

SPStyle *sp_style_new_from_object(SPObject *object); // SPStyle::SPStyle( SPObject *object );

SPStyle *sp_style_ref(SPStyle *style); // SPStyle::ref();

SPStyle *sp_style_unref(SPStyle *style); // SPStyle::unref();

void sp_style_read_from_object(SPStyle *style, SPObject *object); //SPStyle::read( SPObject * object);

void sp_style_read_from_prefs(SPStyle *style, Glib::ustring const &path); // SPStyle::read( ... );

void sp_style_merge_from_style_string(SPStyle *style, gchar const *p); // SPStyle::merge( ... );?

void sp_style_merge_from_parent(SPStyle *style, SPStyle const *parent); // SPStyle::cascade( ... );

void sp_style_merge_from_dying_parent(SPStyle *style, SPStyle const *parent); // SPStyle::merge( ... )

gchar *sp_style_write_string(SPStyle const *style, guint flags = SP_STYLE_FLAG_IFSET);//SPStyle::write

gchar *sp_style_write_difference(SPStyle const *from, SPStyle const *to); // SPStyle::write

void sp_style_set_to_uri_string (SPStyle *style, bool isfill, const gchar *uri); // ?

gchar const *sp_style_get_css_unit_string(int unit);  // No change?
double sp_style_css_size_px_to_units(double size, int unit); // No change?
double sp_style_css_size_units_to_px(double size, int unit); // No change?


SPCSSAttr *sp_css_attr_from_style (SPStyle const *const style, guint flags);
SPCSSAttr *sp_css_attr_from_object(SPObject *object, guint flags = SP_STYLE_FLAG_IFSET);
SPCSSAttr *sp_css_attr_unset_text(SPCSSAttr *css);
SPCSSAttr *sp_css_attr_unset_uris(SPCSSAttr *css);
SPCSSAttr *sp_css_attr_scale(SPCSSAttr *css, double ex);

void sp_style_unset_property_attrs(SPObject *o);

void sp_style_set_property_url (SPObject *item, gchar const *property, SPObject *linked, bool recursive);

gchar *attribute_unquote(gchar const *val);
Glib::ustring css2_escape_quote(gchar const *val);

#endif // SEEN_SP_STYLE_H


/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8:textwidth=99 :
 
