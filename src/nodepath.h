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

#include "knot.h"
#include "sp-path.h"
#include "desktop-handles.h"
#include "xml/xml-forward.h"


/** Radial objects are represented by an angle and a distance from
 * 0,0.  0,0 is represented by a == big_num.
 */
class Radial{
 public:
/**  Radius */
	double r;
/**  Amplitude */
	double a;
	Radial() {}
	//	Radial(NR::Point const &p); // Convert a point to radial coordinates
	Radial(Radial &p) : r(p.r),a(p.a) {}
	//	operator NR::Point() const;

Radial(NR::Point const &p)
{
	r = NR::L2(p);
	if (r > 0) {
		a = NR::atan2 (p);
	} else {
		a = HUGE_VAL; //undefined
	}
}

operator NR::Point() const
{
	if (a == HUGE_VAL) {
		return NR::Point(0,0);
	} else {
		return r*NR::Point(cos(a), sin(a));
	}
}

};




/** defined in node-context.h */
class SPNodeContext;






namespace Path{

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
class Path;

/**
 * This is a subdivision of a NodePath
 */
class SubPath;

class NodeSide;

/**
 * This is a node (point) along a subpath
 */
class Node;


/**
 *  This is a collection of subpaths which contain nodes
 */
class Path {
 public:
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
/**  Transforms (userspace <---> virtual space?   someone please describe )
	 njh: I'd be guessing that these are item <-> desktop transforms.*/
	NR::Matrix i2d, d2i;
/**  The DOM node which describes this NodePath */
	SPRepr *repr;
	
};


/**
 *  This is the lowest list item.  A simple list of nodes
 */
class SubPath {
 public:
/**  The parent of this subpath */
	Path * nodepath;
/**  Is this path closed (no endpoints) or not?*/
	gboolean closed;
/**  The nodes in this subpath. */
	GList * nodes;
/**  The first node of the subpath (does not imply open/closed)*/
	Node * first;
/**  The last node of the subpath */
	Node * last;
};



/**
 *  What kind of node is this?  This the value for the node->type
 *  field.  NodeType indicates the degree of continuity required for
 *  the node.  I think that the corresponding integer indicates which
 *  derivate is connected. (Thus 2 means that the node is continuous
 *  to the second derivative, i.e. has matching endpoints and tangents)
 */
typedef enum {
/**  A normal node */
	NODE_NONE,
/**  This node non-continuously joins two segments.*/
	NODE_CUSP,
/**  This node continuously joins two segments. */
	NODE_SMOOTH,
/**  This node is symmetric. */
	NODE_SYMM
} NodeType;



/**
 * A NodeSide is a datarecord which may be on either side (n or p) of a node,
 * which describes the segment going to the next node.
 */
class NodeSide{
 public:
/**  Pointer to the next node, */
	Node * other;
/**  Position */
	NR::Point pos;
/**  Knots are Inkscape's way of providing draggable points.  This
 *  Knot is the point on the curve representing the control point in a
 *  bezier curve.*/
	SPKnot * knot;
/**  What kind of rendering? */
	SPCanvasItem * line;
/**  */
	Radial origin;
};

/**
 * A node along a NodePath
 */
class Node {
 public:
/**  The parent subpath of this node */
	SubPath * subpath;
/**  Type is selected from NodeType.*/
	guint type : 4;
/**  Code refers to which ArtCode is used to represent the segment
 *  (which segment?).*/
	guint code : 4;
/**  Boolean.  Am I currently selected or not? */
	guint selected : 1;
/**  */
	NR::Point pos;
/**  */
	NR::Point origin;
/**  Knots are Inkscape's way of providing draggable points.  This
 *  Knot is the point on the curve representing the endpoint.*/
	SPKnot * knot;
/**  The NodeSide in the 'next' direction */
	NodeSide n;
/**  The NodeSide in the 'previous' direction */
	NodeSide p;
};

};

/**
 *
 */
Path::Path * sp_nodepath_new (SPDesktop * desktop, SPItem * item);

/**
 *
 */
void sp_nodepath_destroy (Path::Path * nodepath);

/**
 *
 */
void sp_nodepath_deselect (Path::Path *nodepath);

/**
 *
 */
void sp_nodepath_select_all (Path::Path *nodepath);

/**
 *
 */
void sp_nodepath_select_next (Path::Path *nodepath);

/**
 *
 */
void sp_nodepath_select_prev (Path::Path *nodepath);

/**
 *
 */
void sp_nodepath_select_rect (Path::Path * nodepath, NRRect * b, gboolean incremental);

/**
 *
 */
GList *save_nodepath_selection (Path::Path *nodepath);

/**
 *
 */
void restore_nodepath_selection (Path::Path *nodepath, GList *r);

/**
 *
 */
gboolean nodepath_repr_d_changed (Path::Path * np, const char *newd);

/**
 *
 */
gboolean nodepath_repr_typestr_changed (Path::Path * np, const char *newtypestr);

/**
 *
 */
gboolean node_key (GdkEvent * event);

/**
 *
 */
void sp_nodepath_update_statusbar (Path::Path *nodepath);





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
void sp_node_selected_set_type (Path::NodeType type);

/**
 *
 */
void sp_node_selected_set_line_type (NRPathcode code);

/**
 *
 */
void sp_node_selected_move (gdouble dx, gdouble dy);

/**
 *
 */
void sp_node_selected_move_screen (gdouble dx, gdouble dy);


void sp_nodepath_selected_nodes_rotate (Path::Path * nodepath, gdouble angle, int which);
void sp_nodepath_selected_nodes_rotate_screen (Path::Path * nodepath, gdouble angle, int which);
void sp_nodepath_selected_nodes_scale (Path::Path * nodepath, gdouble grow, int which);
void sp_nodepath_selected_nodes_scale_screen (Path::Path * nodepath, gdouble grow, int which);









#endif
