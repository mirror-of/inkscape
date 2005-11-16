#ifndef SP_ITEM_TRANSFORM_H
#define SP_ITEM_TRANSFORM_H

#include "forward.h"
namespace NR {
class translate;
class rotate;
}

void sp_item_rotate_rel(SPItem *item, NR::rotate const &rotation);
void sp_item_scale_rel (SPItem *item, NR::scale const &scale);
void sp_item_skew_rel (SPItem *item, double skewX, double skewY);
void sp_item_move_rel(SPItem *item, NR::translate const &tr);


#endif /* !SP_ITEM_TRANSFORM_H */

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
