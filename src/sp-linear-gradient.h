#ifndef SP_LINEAR_GRADIENT_H
#define SP_LINEAR_GRADIENT_H
/** \file */

#include <glib/gtypes.h>
#include <glib-object.h>
#include "sp-gradient.h"
#include "svg/svg-types.h"

#define SP_TYPE_LINEARGRADIENT (sp_lineargradient_get_type())
#define SP_LINEARGRADIENT(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), SP_TYPE_LINEARGRADIENT, SPLinearGradient))
#define SP_LINEARGRADIENT_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST((klass), SP_TYPE_LINEARGRADIENT, SPLinearGradientClass))
#define SP_IS_LINEARGRADIENT(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj), SP_TYPE_LINEARGRADIENT))
#define SP_IS_LINEARGRADIENT_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), SP_TYPE_LINEARGRADIENT))

/** Linear gradient. */
struct SPLinearGradient {
    SPGradient gradient;

    SPSVGLength x1;
    SPSVGLength y1;
    SPSVGLength x2;
    SPSVGLength y2;
};

struct SPLinearGradientClass {
    SPGradientClass parent_class;
};

GType sp_lineargradient_get_type();

void sp_lineargradient_set_position(SPLinearGradient *lg, gdouble x1, gdouble y1, gdouble x2, gdouble y2);

/** Builds flattened repr tree of gradient - i.e. no href. */
SPRepr *sp_lineargradient_build_repr(SPLinearGradient *lg, gboolean vector);


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
