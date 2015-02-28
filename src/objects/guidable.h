/*
 * Authors:
 *   Liam P. White
 *
 * Copyright (C) 2015 authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

typedef unsigned int guint32;

namespace Geom {
class Point;
}

namespace Inkscape {
namespace Objects {

class Guidable {
public:
    virtual void hide() = 0;
    virtual void hide_origin() = 0;

    virtual void show() = 0;
    virtual void show_origin() = 0;

    virtual void set_color(guint32 color) = 0;
    virtual void set_label(char const *label) = 0;
    virtual void set_position(Geom::Point const& pos) = 0;
    virtual void set_normal(Geom::Point const& normal) = 0;
    virtual void set_sensitive(bool sensitive) = 0;

protected:
    Guidable() {}
    virtual ~Guidable() {}
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
