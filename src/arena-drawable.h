/*
 * Authors:
 *   Liam P. White
 *
 * Copyright (C) 2015 authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "objects/display-item.h"

namespace Inkscape {
class Drawing;
class DrawingImage;
class DrawingGroup;
class DrawingGlyphs;
class DrawingPattern;
class DrawingShape;
class DrawingText;
}

class ArenaDrawableGroup
    : public Inkscape::Objects::DrawableGroup
{
public:
    ArenaDrawableGroup(Inkscape::Drawing &drawing);
    virtual ~ArenaDrawableGroup();

    virtual void setStyle(SPStyle *style, SPStyle *context_style = 0);
    virtual void setChildTransform(Geom::Affine const &new_trans);
    virtual void setChildrenStyle(SPStyle *context_style);

    Inkscape::DrawingGroup *_group; // proxied member
};

class ArenaDrawableShape
    : public Inkscape::Objects::DrawableShape
{
public:
    ArenaDrawableShape(Inkscape::Drawing &drawing);
    virtual ~ArenaDrawableShape();

    virtual void setPath(SPCurve *curve);
    virtual void setStyle(SPStyle *style, SPStyle *context_style = 0);
    virtual void setChildrenStyle(SPStyle *context_style);

    Inkscape::DrawingShape *_shape; // proxied member
};

class ArenaDrawableGlyphs
    : public Inkscape::Objects::DrawableGlyphs
{
public:
    ArenaDrawableGlyphs(Inkscape::Drawing &drawing);
    virtual ~ArenaDrawableGlyphs();

    virtual void setStyle(SPStyle *style, SPStyle *context_style = 0);
    virtual void setChildrenStyle(SPStyle *context_style);
    virtual void setGlyph(font_instance *font, int glyph, Geom::Affine const &trans);

    Inkscape::DrawingGlyphs *_glyphs;
};

class ArenaDrawableText
    : public Inkscape::Objects::DrawableText
{
public:
    ArenaDrawableText(Inkscape::Drawing &drawing);
    virtual ~ArenaDrawableText();

    virtual void setStyle(SPStyle *style, SPStyle *context_style = 0);
    virtual void setChildrenStyle(SPStyle *context_style);
    virtual void setChildTransform(Geom::Affine const &new_trans);
    virtual void clear();
    virtual bool addComponent(font_instance *font, int glyph, Geom::Affine const &trans, 
        float width, float ascent, float descent, float phase_length);

    Inkscape::DrawingText *_text;
};

class ArenaDrawableImage
    : public Inkscape::Objects::DrawableImage
{
public:
    ArenaDrawableImage(Inkscape::Drawing &drawing);
    virtual ~ArenaDrawableImage();

    virtual void setPixbuf(Inkscape::Pixbuf *pb);
    virtual void setScale(double sx, double sy);
    virtual void setOrigin(Geom::Point const &o);
    virtual void setClipbox(Geom::Rect const &box);
    virtual Geom::Rect const& bounds() const;

    virtual void setStyle(SPStyle *style, SPStyle *context_style = 0);
    virtual void setChildrenStyle(SPStyle *context_style);

    Inkscape::DrawingImage *_image;
};

class ArenaDrawablePattern
    : public Inkscape::Objects::DrawablePattern
{
public:
    ArenaDrawablePattern(Inkscape::Drawing &drawing);
    virtual ~ArenaDrawablePattern();

    virtual void setStyle(SPStyle *style, SPStyle *context_style = 0);
    virtual void setChildTransform(Geom::Affine const &new_trans);
    virtual void setChildrenStyle(SPStyle *context_style);

    virtual void setPatternToUserTransform(Geom::Affine const &new_trans);
    virtual void setTileRect(Geom::Rect const &tile_rect);
    virtual void setOverflow(Geom::Affine const& initial_transform, int steps, Geom::Affine const& step_transform);

    Inkscape::DrawingPattern *_pattern;
};

class ArenaDrawable
    : public Inkscape::Objects::Drawable
{
public:
    ArenaDrawable(Inkscape::Drawing &drawing);
    virtual ~ArenaDrawable() {}

    virtual ArenaDrawableText *createText();
    virtual ArenaDrawableGroup *createGroup();
    virtual ArenaDrawableShape *createShape();
    virtual ArenaDrawableImage *createImage();
    virtual ArenaDrawableGlyphs *createGlyphs();
    virtual ArenaDrawablePattern *createPattern();

    Inkscape::Drawing &_drawing;
};

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
