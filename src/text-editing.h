#ifndef __SP_TEXT_EDITING_H__
#define __SP_TEXT_EDITING_H__

/*
 * Text editing functions common for for text and flowtext
 *
 * Author:
 *   bulia byak
 *
 * Copyright (C) 2004 author
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */


bool sp_te_is_empty (SPItem *item);
gint sp_te_get_length (SPItem *item);

gint sp_te_up (SPItem *item, gint i_position);
gint sp_te_down (SPItem *item, gint i_position);
gint sp_te_start_of_line (SPItem *item, gint i_position);
gint sp_te_end_of_line (SPItem *item, gint i_position);

guint sp_te_get_position_by_coords (SPItem *item, NR::Point &i_p);
void sp_te_get_cursor_coords (SPItem *item, gint i_position, NR::Point &p0, NR::Point &p1);

gint sp_te_insert(SPItem *item, gint i_ucs4_pos, gchar const *utf8);
gint sp_te_delete (SPItem *item, gint i_start, gint i_end);


#endif
