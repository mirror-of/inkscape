/*
 * Authors:
 *   Liam P. White
 *
 * Copyright (C) 2015 authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "objects/guidable.h"

struct SPCanvasGroup;
struct SPGuideLine;

class CanvasGuidable
    : public Inkscape::Objects::Guidable
{
public:
    CanvasGuidable(SPCanvasGroup *parent);
    virtual ~CanvasGuidable();

    virtual void hide();
    virtual void hide_origin();

    virtual void show();
    virtual void show_origin();

    virtual void set_color(guint32 color);
    virtual void set_label(char const *label);
    virtual void set_position(Geom::Point const& pos);
    virtual void set_normal(Geom::Point const& normal);
    virtual void set_sensitive(bool sensitive);

    virtual void const *get_key() const { return _key; }
    virtual void set_key(void *key) { _key = key; }

    SPGuideLine *guide;

protected:
    void *_key;
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
