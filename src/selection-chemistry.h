#ifndef __SP_SELECTION_CHEMISTRY_H__
#define __SP_SELECTION_CHEMISTRY_H__

/*
 * Miscellanous operations on selected items
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   Frank Felfe <innerspace@iname.com>
 *
 * Copyright (C) 1999-2002 authors
 * Copyright (C) 2001-2002 Ximian, Inc.
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "forward.h"
#include "libnr/nr-forward.h"

class SPSelection;

void sp_selection_cleanup();

void sp_selection_delete();
void sp_selection_duplicate();
void sp_edit_clear_all();
void sp_edit_select_all();

void sp_selection_clone();

void sp_selection_group();
void sp_selection_ungroup();

void sp_selection_raise();
void sp_selection_raise_to_top();
void sp_selection_lower();
void sp_selection_lower_to_bottom();

void sp_selection_cut();
void sp_selection_copy();
void sp_selection_paste(bool in_place);
void sp_selection_paste_style();

void sp_selection_apply_affine(SPSelection *selection, NR::Matrix const &affine);
void sp_selection_remove_transform (void);
void sp_selection_scale_absolute (SPSelection *selection, double x0, double x1, double y0, double y1);
void sp_selection_scale_relative(SPSelection *selection, NR::Point const &align, NR::scale const &scale);
void sp_selection_rotate_relative (SPSelection *selection, NR::Point const &center, gdouble angle);
void sp_selection_skew_relative (SPSelection *selection, NR::Point const &align, double dx, double dy);
void sp_selection_move_relative (SPSelection *selection, double dx, double dy);

void sp_selection_rotate_90_cw (void);
void sp_selection_rotate_90_ccw (void);
void sp_selection_rotate (SPSelection *selection, gdouble angle);
void sp_selection_rotate_screen (SPSelection *selection, gdouble angle);

void sp_selection_scale (SPSelection *selection, gdouble grow);
void sp_selection_scale_screen (SPSelection *selection, gdouble grow_pixels);
void sp_selection_scale_times (SPSelection *selection, gdouble times);

void sp_selection_move (gdouble dx, gdouble dy);
void sp_selection_move_screen (gdouble dx, gdouble dy);

void sp_selection_item_next (void);
void sp_selection_item_prev (void);

void sp_undo (SPDesktop *desktop, SPDocument *doc);
void sp_redo (SPDesktop *desktop, SPDocument *doc);

/* selection cycling */

typedef enum
{
	SP_CYCLE_SIMPLE,
	SP_CYCLE_VISIBLE, /* cycle only visible items */
	SP_CYCLE_FOCUS /* readjust visible area to view selected item */
} SPCycleType;

/* fixme: This should be moved into preference repr */
#ifndef __SP_SELECTION_CHEMISTRY_C__
extern SPCycleType SP_CYCLING;
#else
SPCycleType SP_CYCLING = SP_CYCLE_FOCUS;
#endif

#endif


