#ifndef SEEN_SP_STOP_H
#define SEEN_SP_STOP_H

#include <glib/gtypes.h>
#include <glib-object.h>
#include "sp-object.h"
#include "color.h"

#define SP_TYPE_STOP (sp_stop_get_type())
#define SP_STOP(o) (G_TYPE_CHECK_INSTANCE_CAST((o), SP_TYPE_STOP, SPStop))
#define SP_STOP_CLASS(k) (G_TYPE_CHECK_CLASS_CAST((k), SP_TYPE_STOP, SPStopClass))
#define SP_IS_STOP(o) (G_TYPE_CHECK_INSTANCE_TYPE((o), SP_TYPE_STOP))
#define SP_IS_STOP_CLASS(k) (G_TYPE_CHECK_CLASS_TYPE((k), SP_TYPE_STOP))


/** Gradient stop. */
struct SPStop {
    SPObject object;

    /* fixme: Should be SPSVGPercentage */
    gfloat offset;

    SPColor color;
    /* fixme: Implement SPSVGNumber or something similar */
    gfloat opacity;
};

struct SPStopClass {
	SPObjectClass parent_class;
};

GType sp_stop_get_type (void);


#endif /* !SEEN_SP_STOP_H */

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
