#include <glib/gmessages.h>
#include "common-style.h"
#include "color.h"
#include "sp-object.h"
#include "style.h"


/**
 * Returns true iff each o in \a objects with flat color (i.e.\ SP_PAINT_TYPE_COLOR) has the same
 * fill/stroke color (excluding alpha).
 *
 * (Returns true if \a objects is empty or contains no objects with SP_PAINT_TYPE_COLOR.)
 *
 * \param is_fill True to test fill style, false to test stroke style.
 *
 * \pre all[o in objects] SP_IS_OBJECT(o).
 */
guint32
objects_get_common_rgb(GSList const *const objects, FillOrStroke const is_fill)
{
    guint32 ret = 0;
    g_assert(ret == NO_COLOR);
    g_assert(NO_COLOR != DIFFERENT_COLORS);

    for (GSList const *i = objects; i != NULL; i = i->next) {
        SPObject const *const o = SP_OBJECT(i->data);
        SPStyle const &style = *SP_OBJECT_STYLE(o);
        SPIPaint const &paint = ( is_fill
                                  ? style.fill
                                  : style.stroke );
        if (paint.type == SP_PAINT_TYPE_COLOR) {
            guint32 const this_rgb = sp_color_get_rgba32_ualpha(&paint.value.color, 0xff);
            if (ret) {
                if ( this_rgb != ret ) {
                    return DIFFERENT_COLORS;
                }
            } else {
                ret = this_rgb;
                g_assert(ret);
            }
        }
    }
    return ret;
}


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
