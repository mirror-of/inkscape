#ifndef SP_ITEM_TRANSFORM_H
#define SP_ITEM_TRANSFORM_H

#include "sp-item.h"

void sp_item_rotate_rel (SPItem * item, double angle);
void sp_item_scale_rel (SPItem * item, double dx, double dy);
void sp_item_skew_rel (SPItem * item, double dx, double dy); 
void sp_item_move_rel (SPItem * item, double dx, double dy);

#endif
