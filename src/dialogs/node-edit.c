#define __SP_NODE_EDIT_C__

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

#include "../nodepath.h"
#include "node-edit.h"

void
sp_node_path_edit_add (void)
{
	sp_node_selected_add_node ();
}

void
sp_node_path_edit_delete (void)
{
	sp_node_selected_delete ();
}

void
sp_node_path_edit_break (void)
{
	sp_node_selected_break ();
}

void
sp_node_path_edit_join (void)
{
	sp_node_selected_join ();
}

void
sp_node_path_edit_join_segment (void)
{
	sp_node_selected_join_segment ();
}

void
sp_node_path_edit_toline (void)
{
	sp_node_selected_set_line_type (ART_LINETO);
}

void
sp_node_path_edit_tocurve (void)
{
	sp_node_selected_set_line_type (ART_CURVETO);
}

void
sp_node_path_edit_cusp (void)
{
	sp_node_selected_set_type (SP_PATHNODE_CUSP);
}

void
sp_node_path_edit_smooth (void)
{
	sp_node_selected_set_type (SP_PATHNODE_SMOOTH);
}

void
sp_node_path_edit_symmetric (void)
{
	sp_node_selected_set_type (SP_PATHNODE_SYMM);
}

