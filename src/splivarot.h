#ifndef __SP_LIVAROT_H__
#define __SP_LIVAROT_H__

/*
 * boolops and outlines
 *
 * public domain
 */

#include "forward.h"

// boolean operations
// work on the current selection
// selection has 2 contain exactly 2 items
void sp_selected_path_union (void);
void sp_selected_path_intersect (void);
void sp_selected_path_diff (void);
void sp_selected_path_symdiff (void);

// offset/inset of a curve
// takes the fill-rule in consideration
// offset amount is the stroke-width of the curve
void sp_selected_path_offset (void);
void sp_selected_path_inset (void);
// outline of a curve
// uses the stroke-width
void sp_selected_path_outline (void);

#endif
