#ifndef __SP_NODE_EDIT_H__
#define __SP_NODE_EDIT_H__

/*
 * Node editing dialog and frontends
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * Copyright (C) 2001-2002 Lauris Kaplinski
 * Copyright (C) 2001 Ximian, Inc.
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

/* fixme: This is not the right place for these frontends (Lauris) */

void sp_node_path_edit_add (void);
void sp_node_path_edit_delete (void);
void sp_node_path_edit_break (void);
void sp_node_path_edit_join (void);
void sp_node_path_edit_join_segment (void);
void sp_node_path_edit_toline (void);
void sp_node_path_edit_tocurve (void);
void sp_node_path_edit_cusp (void);
void sp_node_path_edit_smooth (void);
void sp_node_path_edit_symmetric (void);

#endif
