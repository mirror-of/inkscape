#ifndef SEEN_SP_GRADIENT_VECTOR_H
#define SEEN_SP_GRADIENT_VECTOR_H

#include <glib/gtypes.h>
#include "color.h"

struct SPGradientStop {
    gdouble offset;
    SPColor color;
    gfloat opacity;
};

struct SPGradientVector {
    gint nstops;
    gdouble start, end;
    SPGradientStop stops[1];
};


#endif /* !SEEN_SP_GRADIENT_VECTOR_H */

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
