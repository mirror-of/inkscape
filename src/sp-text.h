#ifndef __SP_TEXT_H__
#define __SP_TEXT_H__

/*
 * SVG <text> and <tspan> implementation
 *
 * Author:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * Copyright (C) 1999-2002 Lauris Kaplinski
 * Copyright (C) 2000-2001 Ximian, Inc.
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <glib/gtypes.h>

#include <sigc++/sigc++.h>

#include "forward.h"
#include "sp-item.h"
#include "sp-string.h"
#include "display/display-forward.h"
#include "libnr/nr-point.h"
#include "svg/svg-types.h"
#include "libnrtype/Layout-TNG.h"


#define SP_TYPE_TEXT (sp_text_get_type())
#define SP_TEXT(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), SP_TYPE_TEXT, SPText))
#define SP_TEXT_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST((klass), SP_TYPE_TEXT, SPTextClass))
#define SP_IS_TEXT(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj), SP_TYPE_TEXT))
#define SP_IS_TEXT_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), SP_TYPE_TEXT))

/* Text specific flags */
#define SP_TEXT_CONTENT_MODIFIED_FLAG SP_OBJECT_USER_MODIFIED_FLAG_A
#define SP_TEXT_LAYOUT_MODIFIED_FLAG SP_OBJECT_USER_MODIFIED_FLAG_A

/** \brief contains and manages the attributes common to all types of text tag

The five attributes x, y, dx, dy and rotate (todo: textlength, lengthadjust)
are permitted on all of text, tspan and textpath elements so we need a class
to abstract the management of those attributes from the actual type of the
element.
*/
class TextTagAttributes {
public:
    TextTagAttributes() {}
    TextTagAttributes(Inkscape::Text::Layout::OptionalTextTagAttrs const &attrs)
        : attributes(attrs) {}

    /// fill in all the fields of #attributes from the given node
    void readFrom(Inkscape::XML::Node const *node);

    /** process the parameters from the set() function of SPObject.
    Returns true if \a key was a recognised attribute. */
    bool readSingleAttribute(unsigned key, gchar const *value);

    /// write out all the contents of #attributes to the given node
    void writeTo(Inkscape::XML::Node *node) const;

    /** for tspan role=line elements we should not use the set x,y
    coordinates since that would overrule the values calculated by the
    text layout engine, however if there are more than one element in
    the x or y vectors we can presume that the user set them and hence
    they should be copied. This function detects that condition so the
    \a use_xy parameter to mergeInto() can be set correctly. */
    bool singleXYCoordinates() const;

    /** returns false if all of the vectors are zero length. */
    bool anyAttributesSet() const;

    /** implements the rules for overlaying the contents of the class
    (treated as the child object) on top of previously existing
    attributes from \a parent_attrs using the rules described in
    SVG 1.1 section 10.5. \a parent_attrs_offset can be used to require
    that only fields from \a parent_attrs starting at that index will
    be used. Basically, the algorithm is that if a child attribute
    exists that will be used, otherwise the parent attribute will be used,
    otherwise the vector will end. */
    void mergeInto(Inkscape::Text::Layout::OptionalTextTagAttrs *output, Inkscape::Text::Layout::OptionalTextTagAttrs const &parent_attrs, unsigned parent_attrs_offset, bool copy_xy, bool copy_dxdyrotate) const;

    /** deletes all the values from all the vectors beginning at
    \a start_index and extending for \a n fields. This is what you want
    to do when deleting characters from the corresponding text. */
    void erase(unsigned start_index, unsigned n);

    /** inserts \a n new values in all the stored vectors at \a
    start_index. This is what you want to do when inserting characters
    in the corresponding text. If a vector is shorter than \a start_index
    it will not be extended (the defaults are fine). dx, dy and rotate
    will be extended with zero values, x and y will be extended with
    linearly interpolated values. TODO: The inserted values should probably
    be unset but sp_svg_length_list_read() can't cope with that. */
    void insert(unsigned start_index, unsigned n);

    /** divides the stored attributes into two, at the given index. The
    first section (0..index-1) stay in this object, the second section
    (index..end) go in \a second. This function is generally used when
    line breaking. */
    void split(unsigned index, TextTagAttributes *second);

    /** overwrites all the attributes contained in this object with the
    given parameters by putting \a first at the beginning, then the
    contents of \a second after \a second_index. */
    void join(TextTagAttributes const &first, TextTagAttributes const &second, unsigned second_index);

    /** applies the given transformation to the stored coordinates. Pairs
    of x and y coordinates are multiplied by the matrix and the dx and dy
    vectors are multiplied by the given parameters. rotate is not altered.
    If \a extend_zero_length is true, then if the x or y vectors are empty
    they will be made length 1 in order to store the newly calculated
    position. */
    void transform(NR::Matrix const &matrix, double scale_x, double scale_y, bool extend_zero_length = false);

    /** adds the given values to the dx and dy vectors at the given
    \a index. The vectors are extended if necessary. */
    void addToDxDy(unsigned index, NR::Point const &adjust);

    /** adds the given value to the rotate vector at the given \a index. The
    vector is extended if necessary. Delta is measured in degrees, clockwise
    positive. */
    void addToRotate(unsigned index, double delta);

    /** returns the first coordinates in the x and y vectors. If either
    is zero length, 0.0 is used for that coordinate. */
    NR::Point firstXY() const;

private:
    /// this holds the actual values.
    Inkscape::Text::Layout::OptionalTextTagAttrs attributes;

    /** does the reverse of readSingleAttribute(), converting a vector<> to
    its SVG string representation and writing it in to \a node. Used by
    writeTo(). */
    static void writeSingleAttribute(Inkscape::XML::Node *node, gchar const *key, std::vector<SPSVGLength> const &attr_vector);

    /** does mergeInto() for one member of #attributes. If \a overlay_list
    is NULL then it does a simple copy of parent elements, starting at
    \a parent_offset. */
    static void mergeSingleAttribute(std::vector<SPSVGLength> *output_list, std::vector<SPSVGLength> const &parent_list, unsigned parent_offset, std::vector<SPSVGLength> const *overlay_list = NULL);

    /// does the work for erase()
    static void eraseSingleAttribute(std::vector<SPSVGLength> *attr_vector, unsigned start_index, unsigned n);

    /// does the work for insert()
    static void insertSingleAttribute(std::vector<SPSVGLength> *attr_vector, unsigned start_index, unsigned n, bool is_xy);

    /// does the work for split()
    static void splitSingleAttribute(std::vector<SPSVGLength> *first_vector, unsigned index, std::vector<SPSVGLength> *second_vector, bool trimZeros);

    /// does the work for join()
    static void joinSingleAttribute(std::vector<SPSVGLength> *dest_vector, std::vector<SPSVGLength> const &first_vector, std::vector<SPSVGLength> const &second_vector, unsigned second_index);
};


/* SPText */

struct SPText : public SPItem {
    /** Converts the text object to its component curves */
    SPCurve *getNormalizedBpath() const
        {return layout.convertToCurves();}

    /** Completely recalculates the layout. */
    void rebuildLayout();

//semiprivate:  (need to be accessed by the C-style functions still)
    TextTagAttributes attributes;
    Inkscape::Text::Layout layout;
	
    /** when the object is transformed it's nicer to change the font size
    and coordinates when we can, rather than just applying a matrix
    transform. is_root is used to indicate to the function that it should
    extend zero-length position vectors to length 1 in order to record the
    new position. This is necessary to convert from objects whose position is
    completely specified by transformations. */
    static void _adjustCoordsRecursive(SPItem *item, NR::Matrix const &m, double ex, bool is_root = true);
    static void _adjustFontsizeRecursive(SPItem *item, double ex, bool is_root = true);
	
    /** discards the NRArena objects representing this text. */
    void _clearFlow(NRArenaGroup *in_arena);

private:
    /** Recursively walks the xml tree adding tags and their contents. The
    non-trivial code does two things: firstly, it manages the positioning
    attributes and their inheritance rules, and secondly it keeps track of line
    breaks and makes sure both that they are assigned the correct SPObject and
    that we don't get a spurious extra one at the end of the flow. */
    unsigned _buildLayoutInput(SPObject *root, Inkscape::Text::Layout::OptionalTextTagAttrs const &parent_optional_attrs, unsigned parent_attrs_offset, bool in_textpath);
};

struct SPTextClass {
    SPItemClass parent_class;
};

GType sp_text_get_type();

#endif

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
