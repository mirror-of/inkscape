#ifndef SEEN_LIBNRTYPE_FONT_GLYPH_H
#define SEEN_LIBNRTYPE_FONT_GLYPH_H

#include <libnrtype/nrtype-forward.h>
#include <livarot/livarot-forward.h>


struct font_glyph {
    double         h_advance, h_width;
    double         v_advance, v_width;
    double         bbox[4];
    Path*          outline;
    void*          artbpath;
};


#endif /* !SEEN_LIBNRTYPE_FONT_GLYPH_H */

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
