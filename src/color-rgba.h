/*
    Author:
      bulia byak <buliabyak@gmail.com>

    Copyright (C) 2004 Authors

    Released under GNU GPL, read the file 'COPYING' for more information
*/
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

    bool operator== (ColorRGBA other) const {
        return ((_c[0] == other[0]) &&
                (_c[1] == other[1]) &&
                (_c[2] == other[2]) &&
                (_c[3] == other[3]));
    }

    ColorRGBA average (ColorRGBA second, float weight = 0.5) {
        return ColorRGBA((_c[0] + second[0]) * weight,
                         (_c[1] + second[1]) * weight,
                         (_c[2] + second[2]) * weight,
                         (_c[3] + second[3]) * weight);
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
