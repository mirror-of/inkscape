#ifndef __SP_NODEPATH_H__
#define __SP_NODEPATH_H__

/*
 * Path handler in node edit mode
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * This code is in public domain
 */

#include "xml/repr.h"
#include "knot.h"
#include "sp-path.h"
#include "desktop-handles.h"

typedef struct _SPNodePath SPNodePath;
typedef struct _SPNodeSubPath SPNodeSubPath;
typedef struct _SPPathNode SPPathNode;

typedef struct {
	double r;
	double a;
} radial;

typedef enum {
	SP_PATHNODE_NONE,
	SP_PATHNODE_CUSP,
	SP_PATHNODE_SMOOTH,
	SP_PATHNODE_SYMM
} SPPathNodeType;

struct _SPNodePath {
	SPDesktop * desktop;
	SPPath * path;
	GList * subpaths;
	GList * selected;
	NRMatrix i2d, d2i;
};

struct _SPNodeSubPath {
	SPNodePath * nodepath;
	gboolean closed;
	GList * nodes;
	SPPathNode * first;
	SPPathNode * last;
};

typedef struct {
	SPPathNode * other;
	NRPoint pos;
	SPKnot * knot;
	SPCanvasItem * line;
	radial origin;
} SPPathNodeSide;

struct _SPPathNode {
	SPNodeSubPath * subpath;
	guint type : 4;
	guint code : 4;
	guint selected : 1;
	NRPoint pos;
	NRPoint origin;
	SPKnot * knot;
	SPPathNodeSide n;
	SPPathNodeSide p;
};

SPNodePath * sp_nodepath_new (SPDesktop * desktop, SPItem * item);
void sp_nodepath_destroy (SPNodePath * nodepath);

void update_object (SPNodePath * np);

void sp_nodepath_deselect (SPNodePath *nodepath);
void sp_nodepath_select_all (SPNodePath *nodepath);
void sp_nodepath_select_next (SPNodePath *nodepath);
void sp_nodepath_select_prev (SPNodePath *nodepath);
void sp_nodepath_select_rect (SPNodePath * nodepath, NRRect * b, gboolean incremental);
GList *save_nodepath_selection (SPNodePath *nodepath);
void restore_nodepath_selection (SPNodePath *nodepath, GList *r);
gboolean nodepath_repr_changed (SPNodePath * np);
void update_repr (SPNodePath * np);
gboolean node_key (GdkEvent * event);
void sp_nodepath_update_statusbar (SPNodePath *nodepath);

/* possibly private functions */

void sp_node_selected_add_node (void);
void sp_node_selected_delete (void);
void sp_node_selected_break (void);
void sp_node_selected_duplicate (void);
void sp_node_selected_join (void);
void sp_node_selected_join_segment (void);
void sp_node_selected_set_type (SPPathNodeType type);
void sp_node_selected_set_line_type (ArtPathcode code);
void sp_node_selected_move (gdouble dx, gdouble dy);
void sp_node_selected_move_screen (gdouble dx, gdouble dy);

#endif
