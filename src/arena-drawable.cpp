/* Authors:
 *   Liam P. White
 *
 * Copyright (C) 2015 authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "arena-drawable.h"
#include "display/drawing-item.h"
#include "display/drawing-image.h"
#include "display/drawing-group.h"
#include "display/drawing-pattern.h"
#include "display/drawing-shape.h"
#include "display/drawing-text.h"

#define ARENAMEM_BOILERPLATE(mem, name) \
    ArenaDrawable##name::ArenaDrawable##name(Inkscape::Drawing &drawing) \
        : mem(new Inkscape::Drawing##name(drawing)) {} \
    ArenaDrawable##name::~ArenaDrawable##name() \
    { delete mem; mem = NULL; } \
    void ArenaDrawable##name::setStyle(SPStyle *style, SPStyle *context_style) \
    { mem->setStyle(style, context_style); } \
    void ArenaDrawable##name::setChildrenStyle(SPStyle *context_style) \
    { mem->setChildrenStyle(context_style); }

// ArenaDrawableGroup
ARENAMEM_BOILERPLATE(_group, Group)

void ArenaDrawableGroup::setChildTransform(Geom::Affine const &new_trans)
{ _group->setChildTransform(new_trans); }

// ArenaDrawableShape
ARENAMEM_BOILERPLATE(_shape, Shape);

void ArenaDrawableShape::setPath(SPCurve *curve)
{ _shape->setPath(curve); }

// ArenaDrawableGlyphs
ARENAMEM_BOILERPLATE(_glyphs, Glyphs)

void ArenaDrawableGlyphs::setGlyph(font_instance *font, int glyph, Geom::Affine const &trans)
{ _glyphs->setGlyph(font, glyph, trans); }

// ArenaDrawableText
ARENAMEM_BOILERPLATE(_text, Text)

void ArenaDrawableText::clear()
{ _text->clear(); }
bool ArenaDrawableText::addComponent(font_instance *font, int glyph, Geom::Affine const &trans, float width, float ascent, float descent, float phase_length)
{ return _text->addComponent(font, glyph, trans, width, ascent, descent, phase_length); }
void ArenaDrawableText::setChildTransform(Geom::Affine const &new_trans)
{ _text->setChildTransform(new_trans); }

// ArenaDrawableImage
ARENAMEM_BOILERPLATE(_image, Image)

void ArenaDrawableImage::setPixbuf(Inkscape::Pixbuf *pb)
{ _image->setPixbuf(pb); }
void ArenaDrawableImage::setScale(double sx, double sy)
{ _image->setScale(sx, sy); }
void ArenaDrawableImage::setOrigin(Geom::Point const &o)
{ _image->setOrigin(o); }
void ArenaDrawableImage::setClipbox(Geom::Rect const &box)
{ _image->setClipbox(box); }
Geom::Rect const& ArenaDrawableImage::bounds() const
{ assert(false); /* didn't happen */ }

// ArenaDrawablePattern
ARENAMEM_BOILERPLATE(_pattern, Pattern)

void ArenaDrawablePattern::setChildTransform(Geom::Affine const &new_trans)
{ _pattern->setChildTransform(new_trans); }
void ArenaDrawablePattern::setPatternToUserTransform(Geom::Affine const &new_trans)
{ _pattern->setPatternToUserTransform(new_trans); }
void ArenaDrawablePattern::setTileRect(Geom::Rect const &tile_rect)
{ _pattern->setTileRect(tile_rect); }
void ArenaDrawablePattern::setOverflow(Geom::Affine const& initial_transform, int steps, Geom::Affine const& step_transform)
{ _pattern->setOverflow(initial_transform, steps, step_transform); }

// ArenaDrawable
ArenaDrawable::ArenaDrawable(Inkscape::Drawing &drawing) : _drawing(drawing) {}
ArenaDrawableText *ArenaDrawable::createText() { return new ArenaDrawableText(_drawing); }
ArenaDrawableGroup *ArenaDrawable::createGroup() { return new ArenaDrawableGroup(_drawing); }
ArenaDrawableShape *ArenaDrawable::createShape() { return new ArenaDrawableShape(_drawing); }
ArenaDrawableImage *ArenaDrawable::createImage() { return new ArenaDrawableImage(_drawing); }
ArenaDrawableGlyphs *ArenaDrawable::createGlyphs() { return new ArenaDrawableGlyphs(_drawing); }
ArenaDrawablePattern *ArenaDrawable::createPattern() { return new ArenaDrawablePattern(_drawing); }

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
