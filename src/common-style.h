#ifndef SEEN_COMMON_STYLE_H
#define SEEN_COMMON_STYLE_H

#include <glib/gslist.h>
#include <glib/gtypes.h>
#include "fill-or-stroke.h"

guint32 objects_get_common_rgb(GSList const *objects, FillOrStroke is_fill);

guint32 const NO_COLOR = 0, DIFFERENT_COLORS = 1;

inline bool
objects_have_same_color(GSList const *objects, FillOrStroke is_fill)
{
    return objects_get_common_rgb(objects, is_fill) != DIFFERENT_COLORS;
}

/* (We'll probably remove this wrapper very soon.) */
#define items_have_same_color(_os, _if) objects_have_same_color(_os, (_if ? FILL : STROKE))


#endif /* !SEEN_COMMON_STYLE_H */

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
