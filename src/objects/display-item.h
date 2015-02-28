/*
 * Authors:
 *   Krzysztof Kosiński <tweenk.pl@gmail.com>
 *   Liam P. White
 *
 * Copyright (C) 2015 authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

namespace Geom {
class Affine;
class Point;
class Rect;
}

class SPCurve;
class SPStyle;
class font_instance;

namespace Inkscape {

class Pixbuf;

namespace Objects {

class DrawableGlyphs;
class DrawableGroup;
class DrawableImage;
class DrawableItem;
class DrawablePattern;
class DrawableShape;
class DrawableText;

class Drawable
{
public:
    virtual DrawableText *createText();
    virtual DrawableGroup *createGroup() = 0;
    virtual DrawableShape *createShape() = 0;
    virtual DrawableImage *createImage() = 0;
    virtual DrawableGlyphs *createGlyphs() = 0;
    virtual DrawablePattern *createPattern() = 0;
};

/**
 * Renderable item belonging to a renderer (Drawable).
 */
class DrawableItem {
public:
    virtual void setStyle(SPStyle *style, SPStyle *context_style = 0) = 0;

protected:
    DrawableItem() {}
    virtual ~DrawableItem() {}
};

/**
 * Group of items belonging to a renderer.
 */
class DrawableGroup : public DrawableItem
{
public:
    virtual void setChildTransform(Geom::Affine const &new_trans) = 0;

protected:
    DrawableGroup() {}
    virtual ~DrawableGroup() {}
};

/**
 * Path belonging to a renderer.
 */
class DrawableShape : public DrawableItem
{
public:
    virtual void setPath(SPCurve *curve) = 0;

protected:
    DrawableShape() {}
    virtual ~DrawableShape() {}
};

/**
 * Glyph belonging to a renderer. 
 * Calling setStyle() on objects of this class will have no effect.
 */
class DrawableGlyphs : public DrawableItem
{
public:
    virtual void setGlyph(font_instance *font, int glyph, Geom::Affine const &trans);

protected:
    DrawableGlyphs() {}
    virtual ~DrawableGlyphs() {}
};

/**
 * Text
 */
class DrawableText : public DrawableGroup
{
public:
    virtual void clear() = 0;
    virtual bool addComponent(font_instance *font, int glyph, Geom::Affine const &trans, 
        float width, float ascent, float descent, float phase_length) = 0;

protected:
    DrawableText() {}
    virtual ~DrawableText() {}
};

/**
 * Image 
 */
class DrawableImage : public DrawableItem
{
public:
    virtual void setPixbuf(Inkscape::Pixbuf *pb) = 0;
    virtual void setScale(double sx, double sy) = 0;
    virtual void setOrigin(Geom::Point const &o) = 0;
    virtual void setClipbox(Geom::Rect const &box) = 0;
    virtual Geom::Rect const& bounds() const = 0;

protected:
    DrawableImage() {}
    virtual ~DrawableImage() {}
};

class DrawablePattern : public DrawableGroup
{
public:
    /**
     * Set the transformation from pattern to user coordinate systems.
     * @see SPPattern description for explanation of coordinate systems.
     */
    virtual void setPatternToUserTransform(Geom::Affine const &new_trans) = 0;
    /**
     * Set the tile rect position and dimentions in content coordinate system
     */
    virtual void setTileRect(Geom::Rect const &tile_rect) = 0;
    /**
     * Turn on overflow rendering.
     *
     * Overflow is implemented as repeated rendering of pattern contents. In every step
     * a translation transform is applied.
     */
    virtual void setOverflow(Geom::Affine const& initial_transform, int steps, Geom::Affine const& step_transform) = 0;

protected:
    DrawablePattern() {}
    virtual ~DrawablePattern() {}
};

} // namespace Objects
} // namespace Inkscape

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
