#ifndef SP_RADIAL_GRADIENT_H
#define SP_RADIAL_GRADIENT_H
/** \file */

#include <glib/gtypes.h>
#include <glib-object.h>
#include "sp-gradient.h"
#include "svg/svg-types.h"

#define SP_TYPE_RADIALGRADIENT (sp_radialgradient_get_type())
#define SP_RADIALGRADIENT(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), SP_TYPE_RADIALGRADIENT, SPRadialGradient))
#define SP_RADIALGRADIENT_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST((klass), SP_TYPE_RADIALGRADIENT, SPRadialGradientClass))
#define SP_IS_RADIALGRADIENT(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj), SP_TYPE_RADIALGRADIENT))
#define SP_IS_RADIALGRADIENT_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), SP_TYPE_RADIALGRADIENT))

/** Radial gradient. */
struct SPRadialGradient {
    SPGradient gradient;

    SPSVGLength cx;
    SPSVGLength cy;
    SPSVGLength r;
    SPSVGLength fx;
    SPSVGLength fy;
};

struct SPRadialGradientClass {
    SPGradientClass parent_class;
};

GType sp_radialgradient_get_type();

void sp_radialgradient_set_position(SPRadialGradient *rg, gdouble cx, gdouble cy, gdouble fx, gdouble fy, gdouble r);

/** Builds flattened repr tree of gradient - i.e. no href. */
SPRepr *sp_radialgradient_build_repr(SPRadialGradient *lg, gboolean vector);


#endif /* !SP_RADIAL_GRADIENT_H */

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
