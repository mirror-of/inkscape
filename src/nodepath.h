#ifndef __SP_NODEPATH_H__
#define __SP_NODEPATH_H__

/**
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

/**
 * In the following data model.   Nodepaths are made up of subpaths which
 * are comprised of nodes.
 *
 * Nodes are linked thus:
 *<pre>
 *        n            other
 *  node----->nodeside------> node
 *
 *</pre>
 *
 */


/**
 * This is a node on a subpath
 */
typedef struct _SPNodePath SPNodePath;

/**
 * This is a subdivision of a NodePath
 */
typedef struct _SPNodeSubPath SPNodeSubPath;

/**
 * This is a node (point) along a subpath
 */
typedef struct _SPPathNode SPPathNode;


/**
 *
 */
typedef struct {
/**  Radius */
	double r;
/**  Amplitude */
	double a;
} radial;


/**
 *  What kind of node is this?  This the value for the node->type field.
 * Describes the kind of rendering which shall be done for this node.
 */
typedef enum {
/**  A normal node */
	SP_PATHNODE_NONE,
/**  This node non-continuously joins two segments*/
	SP_PATHNODE_CUSP,
/**  This node continuously joins two segments */
	SP_PATHNODE_SMOOTH,
/**  This node is symmetric */
	SP_PATHNODE_SYMM
} SPPathNodeType;



/** defined in node-context.h */
typedef struct _SPNodeContext SPNodeContext;


/**
 *  This is a collection of subpaths which contain nodes
 */
struct _SPNodePath {
/**  Pointer to the current desktop, for reporting purposes */
	SPDesktop * desktop;
/**  The parent path of this nodepath */
	SPPath * path;
/**  The context which created this nodepath.  Important if this nodepath is deleted */
	SPNodeContext * nodeContext;
/**  The subpaths which comprise this NodePath */
	GList * subpaths;
/**  A list of nodes which are currently selected */
	GList * selected;
/**  Transforms (userspace <---> virtual space?   someone please describe )*/
	NRMatrix i2d, d2i;
/**  The DOM node which describes this NodePath */
	SPRepr *repr;
};






/**
 *  This is the lowest list item.  A simple list of nodes
 */
struct _SPNodeSubPath {
/**  The parent of this subpath */
	SPNodePath * nodepath;
/**  Is this path closed (no endpoints) or not?*/
	gboolean closed;
/**  The nodes in this subpath. */
	GList * nodes;
/**  The first node of the subpath (does not imply open/closed)*/
	SPPathNode * first;
/**  The last node of the subpath */
	SPPathNode * last;
};






/**
 * A NodeSide is a datarecord which may be on either side (n or p) of a node,
 * which describes the segment going to the next node.
 */
typedef struct {
/**  Pointer to the next node, */
	SPPathNode * other;
/**  Position */
	NRPoint pos;
/**  */
	SPKnot * knot;
/**  What kind of rendering? */
	SPCanvasItem * line;
/**  */
	radial origin;
} SPPathNodeSide;






/**
 * A node along a NodePath
 */
struct _SPPathNode {
/**  The parent subpath of this node */
	SPNodeSubPath * subpath;
/**  */
	guint type : 4;
/**  */
	guint code : 4;
/**  Boolean.  Am I currently selected or not? */
	guint selected : 1;
/**  */
	NRPoint pos;
/**  */
	NRPoint origin;
/**  */
	SPKnot * knot;
/**  The NodeSide in the 'next' direction */
	SPPathNodeSide n;
/**  The NodeSide in the 'previous' direction */
	SPPathNodeSide p;
};

/**
 *
 */
SPNodePath * sp_nodepath_new (SPDesktop * desktop, SPItem * item);

/**
 *
 */
void sp_nodepath_destroy (SPNodePath * nodepath);

/**
 *
 */
void sp_nodepath_deselect (SPNodePath *nodepath);

/**
 *
 */
void sp_nodepath_select_all (SPNodePath *nodepath);

/**
 *
 */
void sp_nodepath_select_next (SPNodePath *nodepath);

/**
 *
 */
void sp_nodepath_select_prev (SPNodePath *nodepath);

/**
 *
 */
void sp_nodepath_select_rect (SPNodePath * nodepath, NRRect * b, gboolean incremental);

/**
 *
 */
GList *save_nodepath_selection (SPNodePath *nodepath);

/**
 *
 */
void restore_nodepath_selection (SPNodePath *nodepath, GList *r);

/**
 *
 */
gboolean nodepath_repr_d_changed (SPNodePath * np, const char *newd);

/**
 *
 */
gboolean nodepath_repr_typestr_changed (SPNodePath * np, const char *newtypestr);

/**
 *
 */
gboolean node_key (GdkEvent * event);

/**
 *
 */
void sp_nodepath_update_statusbar (SPNodePath *nodepath);





/* possibly private functions */


/**
 *
 */
void sp_node_selected_add_node (void);

/**
 *
 */
void sp_node_selected_break (void);

/**
 * Duplicate the selected node(s)
 */
void sp_node_selected_duplicate (void);

/**
 *  Join two nodes by merging them into one.
 */
void sp_node_selected_join (void);

/**
 *  Join two nodes by adding a segment between them.
 */
void sp_node_selected_join_segment (void);

/**
 * Delete one or more selected nodes.
 */
void sp_node_selected_delete (void);

/**
 * Delete one or more segments between two selected nodes.
 */
void sp_node_selected_delete_segment (void);

/**
 *
 */
void sp_node_selected_set_type (SPPathNodeType type);

/**
 *
 */
void sp_node_selected_set_line_type (ArtPathcode code);

/**
 *
 */
void sp_node_selected_move (gdouble dx, gdouble dy);

/**
 *
 */
void sp_node_selected_move_screen (gdouble dx, gdouble dy);


void sp_nodepath_selected_nodes_rotate (SPNodePath * nodepath, gdouble angle, int which);
void sp_nodepath_selected_nodes_rotate_screen (SPNodePath * nodepath, gdouble angle, int which);
void sp_nodepath_selected_nodes_scale (SPNodePath * nodepath, gdouble grow, int which);
void sp_nodepath_selected_nodes_scale_screen (SPNodePath * nodepath, gdouble grow, int which);









#endif
