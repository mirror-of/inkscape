#ifndef SEEN_LIBNRTYPE_FONT_STYLE_H
#define SEEN_LIBNRTYPE_FONT_STYLE_H

#include <libnr/nr-matrix.h>
#include <livarot/LivarotDefs.h>
#include <livarot/livarot-forward.h>


// Different raster styles.
struct font_style {
    NR::Matrix    transform;
    bool          vertical;
    double        stroke_width;
    JoinType      stroke_join;
    ButtType      stroke_cap;
    int           nbDash;
    double        dash_offset;
    double*       dashes;

    void          Apply(Path *src, Shape *dst);
};


#endif /* !SEEN_LIBNRTYPE_FONT_STYLE_H */

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
