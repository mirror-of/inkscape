#ifndef SEEN_COLOR_RGBA_H
#define SEEN_COLOR_RGBA_H

#include <glib/gmessages.h>


class ColorRGBA {
public:
    ColorRGBA(float c0, float c1, float c2, float c3)
    {
        _c[0] = c0; _c[1] = c1;
        _c[2] = c2; _c[3] = c3;
    }


    ColorRGBA &operator=(ColorRGBA const &m) {
        for (unsigned i = 0 ; i < 4 ; ++i) {
            _c[i] = m._c[i];
        }
        return *this;
    }

    float operator[](int const i) const {
        g_assert( unsigned(i) < 4 );
        return _c[i];
    }

private:
    float _c[4];
};


#endif /* !SEEN_COLOR_RGBA_H */

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
