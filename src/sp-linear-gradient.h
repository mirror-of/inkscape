#ifndef SP_LINEAR_GRADIENT_H
#define SP_LINEAR_GRADIENT_H
/** \file */

#include "sp-gradient.h"
#include "svg/svg-types.h"
#include "sp-linear-gradient-fns.h"

/** Linear gradient. */
struct SPLinearGradient : public SPGradient {
    SPSVGLength x1;
    SPSVGLength y1;
    SPSVGLength x2;
    SPSVGLength y2;
};

struct SPLinearGradientClass {
    SPGradientClass parent_class;
};

#endif /* !SP_LINEAR_GRADIENT_H */

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
