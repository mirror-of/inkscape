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
void sp_selected_path_union ();
void sp_selected_path_intersect ();
void sp_selected_path_diff ();
void sp_selected_path_symdiff ();

// offset/inset of a curve
// takes the fill-rule in consideration
// offset amount is the stroke-width of the curve
void sp_selected_path_offset ();
void sp_selected_path_inset ();
void sp_selected_path_create_offset ();
void sp_selected_path_create_inset ();
void sp_selected_path_create_updating_offset ();
void sp_selected_path_create_updating_inset ();
// outline of a curve
// uses the stroke-width
void sp_selected_path_outline ();

// simplifies a path (removes small segments and the like)
void sp_selected_path_simplify ();

#endif
