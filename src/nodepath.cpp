#define __SP_NODEPATH_C__

/*
 * Path handler in node edit mode
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * This code is in public domain
 */

#include "config.h"

#include <math.h>
#include <string.h>
#include <gdk/gdkkeysyms.h>
#include "svg/svg.h"
#include "helper/sp-canvas-util.h"
#include "helper/sp-ctrlline.h"
#include "helper/sodipodi-ctrl.h"
#include "knot.h"
#include "inkscape.h"
#include "document.h"
#include "desktop.h"
#include "desktop-handles.h"
#include "desktop-affine.h"
#include "desktop-snap.h"
#include "node-context.h"
#include "nodepath.h"
#include "selection-chemistry.h"
#include "selection.h"
#include "selection.h"

#define hypot(a,b) sqrt ((a) * (a) + (b) * (b))

/* fixme: Implement these via preferences */

#define NODE_FILL 0xbfbfbf7f
#define NODE_STROKE 0x3f3f3f7f
#define NODE_FILL_HI 0xff7f7f7f
#define NODE_STROKE_HI 0xff3f3fbf
#define NODE_FILL_SEL 0x7f7fff7f
#define NODE_STROKE_SEL 0x3f3fffbf
#define NODE_FILL_SEL_HI 0x3f3fffbf
#define NODE_STROKE_SEL_HI 0x3f3fffff
#define KNOT_FILL 0xbfbfbf7f
#define KNOT_STROKE 0x3f3f3f7f
#define KNOT_FILL_HI 0xff7f7f7f
#define KNOT_STROKE_HI 0xff3f3fbf

GMemChunk * nodechunk = NULL;

/* Creation from object */

static ArtBpath * subpath_from_bpath (SPNodePath * np, ArtBpath * b, const gchar * t);
static gchar * parse_nodetypes (const gchar * types, gint length);

/* Object updating */

static void stamp_repr  (SPNodePath * np);
static SPCurve * create_curve (SPNodePath * np);
static gchar * create_typestr (SPNodePath * np);

static void sp_node_ensure_ctrls (SPPathNode * node);

void sp_nodepath_node_select (SPPathNode * node, gboolean incremental, gboolean override);
void sp_nodepath_node_deselect (SPPathNode * node);
void sp_nodepath_select_rect (SPNodePath * nodepath, NRRect *b, gboolean incremental);

static void sp_node_set_selected (SPPathNode * node, gboolean selected);

/* Control knot placement, if node or other knot is moved */

static void sp_node_adjust_knot (SPPathNode * node, gint which_adjust);
static void sp_node_adjust_knots (SPPathNode * node);

/* Knot event handlers */

static void node_clicked (SPKnot * knot, guint state, gpointer data);
static void node_grabbed (SPKnot * knot, guint state, gpointer data);
static void node_ungrabbed (SPKnot * knot, guint state, gpointer data);
static gboolean node_request (SPKnot * knot, NRPoint *p, guint state, gpointer data);
static void node_ctrl_clicked (SPKnot * knot, guint state, gpointer data);
static void node_ctrl_grabbed (SPKnot * knot, guint state, gpointer data);
static void node_ctrl_ungrabbed (SPKnot * knot, guint state, gpointer data);
static gboolean node_ctrl_request (SPKnot * knot, NRPoint *p, guint state, gpointer data);
static void node_ctrl_moved (SPKnot * knot, NRPoint *p, guint state, gpointer data);

/* Constructors and destructors */

static SPNodeSubPath * sp_nodepath_subpath_new (SPNodePath * nodepath);
static void sp_nodepath_subpath_destroy (SPNodeSubPath * subpath);
static void sp_nodepath_subpath_close (SPNodeSubPath * sp);
static void sp_nodepath_subpath_open (SPNodeSubPath * sp, SPPathNode * n);
static SPPathNode * sp_nodepath_node_new (SPNodeSubPath * sp, SPPathNode * next, SPPathNodeType type, ArtPathcode code,
					  NRPoint *ppos, NRPoint *pos, NRPoint *npos);
static void sp_nodepath_node_destroy (SPPathNode * node);

/* Helpers */

static SPPathNodeSide * sp_node_get_side (SPPathNode * node, gint which);
static SPPathNodeSide * sp_node_opposite_side (SPPathNode * node, SPPathNodeSide * me);
static ArtPathcode sp_node_path_code_from_side (SPPathNode * node, SPPathNodeSide * me);

// active_node indicates mouseover node
static SPPathNode * active_node = NULL;

/**
\brief Creates new nodepath from item 
*/

SPNodePath *
sp_nodepath_new (SPDesktop * desktop, SPItem * item)
{
	SPNodePath * np;
	SPPath * path;
	SPCurve * curve;
	ArtBpath * bpath, * b;
	const gchar * nodetypes;
	gchar * typestr;
	gint length;
	NRMatrix i2d;

	if (!SP_IS_PATH (item)) return NULL;
	path = SP_PATH (item);
	curve = sp_shape_get_curve (SP_SHAPE (path));
	g_return_val_if_fail (curve != NULL, NULL);

	bpath = sp_curve_first_bpath (curve);
	length = curve->end;
	nodetypes = sp_repr_attr (SP_OBJECT (item)->repr, "sodipodi:nodetypes");
	typestr = parse_nodetypes (nodetypes, length);

	np = g_new (SPNodePath, 1);

	np->desktop = desktop;
	np->path = path;
	np->subpaths = NULL;
	np->selected = NULL;
	sp_item_i2d_affine (SP_ITEM (path), &i2d);
	nr_matrix_d_from_f (&np->i2d, &i2d);
	nr_matrix_invert (&np->d2i, &np->i2d);

	/* Now the bitchy part */

	b = bpath;

	while (b->code != ART_END) {
		b = subpath_from_bpath (np, b, typestr + (b - bpath));
	}

	g_free (typestr);
	sp_curve_unref (curve);

	return np;
}

static ArtBpath *
subpath_from_bpath (SPNodePath * np, ArtBpath * b, const gchar * t)
// XXX: Fixme: t should be a proper type, rather than gchar
{
	SPNodeSubPath * sp;
	SPPathNode * n;
	NRPoint ppos, pos, npos;
	gboolean closed;

	g_assert ((b->code == ART_MOVETO) || (b->code == ART_MOVETO_OPEN));

	sp = sp_nodepath_subpath_new (np);
	closed = (b->code == ART_MOVETO);

	pos.x = NR_MATRIX_DF_TRANSFORM_X (&np->i2d, b->x3, b->y3);
	pos.y = NR_MATRIX_DF_TRANSFORM_Y (&np->i2d, b->x3, b->y3);
	if (b[1].code == ART_CURVETO) {
		npos.x = NR_MATRIX_DF_TRANSFORM_X (&np->i2d, b[1].x1, b[1].y1);
		npos.y = NR_MATRIX_DF_TRANSFORM_Y (&np->i2d, b[1].x1, b[1].y1);
	} else {
		npos = pos;
	}
	n = sp_nodepath_node_new (sp, NULL, (SPPathNodeType)*t, ART_MOVETO, &pos, &pos, &npos);
	g_assert (sp->first == n);
	g_assert (sp->last == n);

	b++;
	t++;

	while ((b->code == ART_CURVETO) || (b->code == ART_LINETO)) {
		pos.x = NR_MATRIX_DF_TRANSFORM_X (&np->i2d, b->x3, b->y3);
		pos.y = NR_MATRIX_DF_TRANSFORM_Y (&np->i2d, b->x3, b->y3);
		if (b->code == ART_CURVETO) {
			ppos.x = NR_MATRIX_DF_TRANSFORM_X (&np->i2d, b->x2, b->y2);
			ppos.y = NR_MATRIX_DF_TRANSFORM_Y (&np->i2d, b->x2, b->y2);
		} else {
			ppos = pos;
		}
		if (b[1].code == ART_CURVETO) {
			npos.x = NR_MATRIX_DF_TRANSFORM_X (&np->i2d, b[1].x1, b[1].y1);
			npos.y = NR_MATRIX_DF_TRANSFORM_Y (&np->i2d, b[1].x1, b[1].y1);
		} else {
			npos = pos;
		}
		n = sp_nodepath_node_new (sp, NULL, (SPPathNodeType)*t, b->code, &ppos, &pos, &npos);
		b++;
		t++;
	}

	if (closed) sp_nodepath_subpath_close (sp);

	return b;
}

static gchar *
parse_nodetypes (const gchar * types, gint length)
{
	gchar * typestr;
	gint pos, i;

	g_assert (length > 0);

	typestr = g_new (gchar, length + 1);

	pos = 0;

	if (types) {
		for (i = 0; types[i] && (i < length); i++) {
			while ((types[i] > '\0') && (types[i] <= ' ')) i++;
			if (types[i] != '\0') {
				switch (types[i]) {
				case 's':
					typestr[pos++] = SP_PATHNODE_SMOOTH;
					break;
				case 'z':
					typestr[pos++] = SP_PATHNODE_SYMM;
					break;
				case 'c':
				default:
					typestr[pos++] = SP_PATHNODE_CUSP;
					break;
				}
			}
		}
	}

	while (pos < length) typestr[pos++] = SP_PATHNODE_CUSP;

	return typestr;
}

void
update_object (SPNodePath * np)
{
	SPCurve * curve;

	g_assert (np);

	curve = create_curve (np);

	sp_shape_set_curve (SP_SHAPE (np->path), curve, TRUE);

	sp_curve_unref (curve);
}

/**
\brief Returns true if the argument nodepath and its repr do not match. This may happen if repr was changed in e.g. XML editor or by undo.
*/
gboolean
nodepath_repr_changed (SPNodePath * np)
{
	SPCurve * curve;
	gchar * typestr;
	gchar * svgpath;
	const char *attr_d;
	const char *attr_typestr;
	gboolean r; 

	g_assert (np);

	curve = create_curve (np);
	typestr = create_typestr (np);

	svgpath = sp_svg_write_path (curve->bpath);

	attr_d = sp_repr_attr (SP_OBJECT (np->path)->repr, "d");
	attr_typestr = sp_repr_attr (SP_OBJECT (np->path)->repr, "sodipodi:nodetypes");

	r = strcmp (attr_d, svgpath) || (attr_typestr && strcmp (attr_typestr, typestr));

	g_free (svgpath);
	g_free (typestr);
	sp_curve_unref (curve);
	return r;
}

void
update_repr_internal (SPNodePath * np)
{
	SPCurve * curve;
	gchar * typestr;
	gchar * svgpath;

	g_assert (np);

	curve = create_curve (np);
	typestr = create_typestr (np);

	svgpath = sp_svg_write_path (curve->bpath);

	sp_repr_set_attr (SP_OBJECT (np->path)->repr, "d", svgpath);
	sp_repr_set_attr (SP_OBJECT (np->path)->repr, "sodipodi:nodetypes", typestr);

	g_free (svgpath);
	g_free (typestr);
	sp_curve_unref (curve);
}

void
update_repr (SPNodePath * np)
{
	update_repr_internal (np);
	sp_document_done (SP_DT_DOCUMENT (np->desktop));
}

void
update_repr_keyed (SPNodePath * np, gchar *key)
{
	update_repr_internal (np);
	sp_document_maybe_done (SP_DT_DOCUMENT (np->desktop), key);
}


static void
stamp_repr  (SPNodePath * np)
{
	SPCurve * curve;
	gchar * typestr;
	gchar * svgpath;
	SPRepr * old_repr;
	SPRepr * new_repr;
	
	g_assert (np);

	old_repr = SP_OBJECT (np->path)->repr;
	new_repr = sp_repr_duplicate(old_repr);
	
	curve = create_curve (np);
	typestr = create_typestr (np);

	svgpath = sp_svg_write_path (curve->bpath);

	sp_repr_set_attr (new_repr, "d", svgpath);
	sp_repr_set_attr (new_repr, "sodipodi:nodetypes", typestr);

	sp_document_add_repr(SP_DT_DOCUMENT (np->desktop),
			     new_repr);
	sp_document_done (SP_DT_DOCUMENT (np->desktop));

	sp_repr_unref (new_repr);
	g_free (svgpath);
	g_free (typestr);
	sp_curve_unref (curve);
}

static SPCurve *
create_curve (SPNodePath * np)
{
	SPCurve * curve;
	GList * spl;
	NRPoint p1, p2, p3;

	curve = sp_curve_new ();

	for (spl = np->subpaths; spl != NULL; spl = spl->next) {
		SPNodeSubPath * sp;
		SPPathNode * n;
		sp = (SPNodeSubPath *) spl->data;
		p3.x = NR_MATRIX_DF_TRANSFORM_X (&np->d2i, sp->first->pos.x, sp->first->pos.y);
		p3.y = NR_MATRIX_DF_TRANSFORM_Y (&np->d2i, sp->first->pos.x, sp->first->pos.y);
		sp_curve_moveto (curve, p3.x, p3.y);
		n = sp->first->n.other;
		while (n) {
			p3.x = NR_MATRIX_DF_TRANSFORM_X (&np->d2i, n->pos.x, n->pos.y);
			p3.y = NR_MATRIX_DF_TRANSFORM_Y (&np->d2i, n->pos.x, n->pos.y);
			switch (n->code) {
			case ART_LINETO:
				sp_curve_lineto (curve, p3.x, p3.y);
				break;
			case ART_CURVETO:
				p1.x = NR_MATRIX_DF_TRANSFORM_X (&np->d2i, n->p.other->n.pos.x, n->p.other->n.pos.y);
				p1.y = NR_MATRIX_DF_TRANSFORM_Y (&np->d2i, n->p.other->n.pos.x, n->p.other->n.pos.y);
				p2.x = NR_MATRIX_DF_TRANSFORM_X (&np->d2i, n->p.pos.x, n->p.pos.y);
				p2.y = NR_MATRIX_DF_TRANSFORM_Y (&np->d2i, n->p.pos.x, n->p.pos.y);
				sp_curve_curveto (curve, p1.x, p1.y, p2.x, p2.y, p3.x, p3.y);
				break;
			default:
				g_assert_not_reached ();
				break;
			}
			if (n != sp->last) {
				n = n->n.other;
			} else {
				n = NULL;
			}
		}
		if (sp->closed) {
			sp_curve_closepath (curve);
		}
	}

	return curve;
}

static gchar *
create_typestr (SPNodePath * np)
{
	gchar * typestr;
	gint len, pos;
	GList * spl;

	typestr = g_new (gchar, 32);
	len = 32;
	pos = 0;

	for (spl = np->subpaths; spl != NULL; spl = spl->next) {
		SPNodeSubPath * sp;
		SPPathNode * n;
		sp = (SPNodeSubPath *) spl->data;

		if (pos >= len) {
			typestr = g_renew (gchar, typestr, len + 32);
			len += 32;
		}

		typestr[pos++] = 'c';

		n = sp->first->n.other;
		while (n) {
			gchar code;

			switch (n->type) {
			case SP_PATHNODE_CUSP:
				code = 'c';
				break;
			case SP_PATHNODE_SMOOTH:
				code = 's';
				break;
			case SP_PATHNODE_SYMM:
				code = 'z';
				break;
			default:
				g_assert_not_reached ();
				code = '\0';
				break;
			}

			if (pos >= len) {
				typestr = g_renew (gchar, typestr, len + 32);
				len += 32;
			}

			typestr[pos++] = code;

			if (n != sp->last) {
				n = n->n.other;
			} else {
				n = NULL;
			}
		}
	}

	if (pos >= len) {
		typestr = g_renew (gchar, typestr, len + 1);
		len += 1;
	}

	typestr[pos++] = '\0';

	return typestr;
}

void
sp_nodepath_destroy (SPNodePath * np)
{
	g_assert (np);

	while (np->subpaths) {
		sp_nodepath_subpath_destroy ((SPNodeSubPath *) np->subpaths->data);
	}

	g_assert (!np->selected);

	g_free (np);
}

static SPNodePath *
sp_nodepath_current (void)
{
	SPEventContext * event_context;

	if (!SP_ACTIVE_DESKTOP) return NULL;

	event_context = (SP_ACTIVE_DESKTOP)->event_context;

	if (!SP_IS_NODE_CONTEXT (event_context)) return NULL;

	return SP_NODE_CONTEXT (event_context)->nodepath;
}

/**
 \brief Fills node and control positions for three nodes, splitting line
  marked by end at distance t
 */
static void
sp_nodepath_line_midpoint (SPPathNode * new_path, SPPathNode * end, gdouble t)
{
	SPPathNode * start;
	gdouble s;
	gdouble f000, f001, f011, f111, f00t, f0tt, fttt, f11t, f1tt, f01t;

	g_assert (new_path != NULL);
	g_assert (end != NULL);

	g_assert (end->p.other == new_path);
	start = new_path->p.other;
	g_assert (start);

	if (end->code == ART_LINETO) {
		new_path->type = SP_PATHNODE_CUSP;
		new_path->code = ART_LINETO;
		new_path->pos.x = (t * start->pos.x + (1 - t) * end->pos.x);
		new_path->pos.y = (t * start->pos.y + (1 - t) * end->pos.y);
	} else {
		new_path->type = SP_PATHNODE_SMOOTH;
		new_path->code = ART_CURVETO;
		s = 1 - t;
		f000 = start->pos.x;
		f001 = start->n.pos.x;
		f011 = end->p.pos.x;
		f111 = end->pos.x;
		f00t = s * f000 + t * f001;
		f01t = s * f001 + t * f011;
		f11t = s * f011 + t * f111;
		f0tt = s * f00t + t * f01t;
		f1tt = s * f01t + t * f11t;
		fttt = s * f0tt + t * f1tt;
		start->n.pos.x = f00t;
		new_path->p.pos.x = f0tt;
		new_path->pos.x = fttt;
		new_path->n.pos.x = f1tt;
		end->p.pos.x = f11t;
		f000 = start->pos.y;
		f001 = start->n.pos.y;
		f011 = end->p.pos.y;
		f111 = end->pos.y;
		f00t = s * f000 + t * f001;
		f01t = s * f001 + t * f011;
		f11t = s * f011 + t * f111;
		f0tt = s * f00t + t * f01t;
		f1tt = s * f01t + t * f11t;
		fttt = s * f0tt + t * f1tt;
		start->n.pos.y = f00t;
		new_path->p.pos.y = f0tt;
		new_path->pos.y = fttt;
		new_path->n.pos.y = f1tt;
		end->p.pos.y = f11t;
	}
}

static SPPathNode *
sp_nodepath_line_add_node (SPPathNode * end, gdouble t)
{
	SPNodePath * np;
	SPNodeSubPath * sp;
	SPPathNode * start;
	SPPathNode * newnode;

	g_assert (end);
	g_assert (end->subpath);
	g_assert (g_list_find (end->subpath->nodes, end));

	sp = end->subpath;
	np = sp->nodepath;

	start = end->p.other;

	g_assert (start->n.other == end);

	newnode = sp_nodepath_node_new (sp, end, SP_PATHNODE_SMOOTH, (ArtPathcode)end->code, &start->pos, &start->pos, &start->n.pos);
	sp_nodepath_line_midpoint (newnode, end, t);

	sp_node_ensure_ctrls (start);
	sp_node_ensure_ctrls (newnode);
	sp_node_ensure_ctrls (end);

	return newnode;
}

/**
\brief Break the path at the node: duplicate the argument node, start a new subpath with the duplicate, and copy all nodes after the argument node to it
*/
static SPPathNode *
sp_nodepath_node_break (SPPathNode * node)
{
	SPNodePath * np;
	SPNodeSubPath * sp;

	g_assert (node);
	g_assert (node->subpath);
	g_assert (g_list_find (node->subpath->nodes, node));

	sp = node->subpath;
	np = sp->nodepath;

	if (sp->closed) {
		sp_nodepath_subpath_open (sp, node);

		return sp->first;
	} else {
		SPNodeSubPath * newsubpath;
		SPPathNode * newnode;
		SPPathNode * n, *nn;

		// no break for end nodes
		if (node == sp->first) return NULL;
		if (node == sp->last) return NULL;

		// create a new subpath
		newsubpath = sp_nodepath_subpath_new (np);

		// duplicate the break node as start of the new subpath
		newnode = sp_nodepath_node_new (newsubpath, NULL, (SPPathNodeType)node->type, ART_MOVETO, &node->pos, &node->pos, &node->n.pos);

		while (node->n.other) { // copy the remaining nodes into the new subpath
			n = node->n.other;
			nn = sp_nodepath_node_new (newsubpath, NULL, (SPPathNodeType)n->type, (ArtPathcode)n->code, &n->p.pos, &n->pos, &n->n.pos);
			if (n->selected) {
				sp_nodepath_node_select (nn, TRUE, TRUE); //preserve selection
			}
			sp_nodepath_node_destroy (n); // remove the point on the original subpath
		}

		return newnode;
	}
}

static SPPathNode *
sp_nodepath_node_duplicate (SPPathNode * node)
{
	SPNodePath * np;
	SPNodeSubPath * sp;
	SPPathNode * newnode;
	ArtPathcode code; 

	g_assert (node);
	g_assert (node->subpath);
	g_assert (g_list_find (node->subpath->nodes, node));

	sp = node->subpath;
	np = sp->nodepath;

	code = (ArtPathcode) node->code; 
	if (code == ART_MOVETO) { // if node is the endnode,
		node->code = ART_LINETO; // new one is inserted before it, so change that to line
	}

	newnode = sp_nodepath_node_new (sp, node, (SPPathNodeType)node->type, code, &node->p.pos, &node->pos, &node->n.pos);

	return newnode;
}

static void
sp_node_control_mirror_n_to_p (SPPathNode * node)
{
	node->p.pos.x = (node->pos.x + (node->pos.x - node->n.pos.x));
	node->p.pos.y = (node->pos.y + (node->pos.y - node->n.pos.y));
}

static void
sp_node_control_mirror_p_to_n (SPPathNode * node)
{
	node->n.pos.x = (node->pos.x + (node->pos.x - node->p.pos.x));
	node->n.pos.y = (node->pos.y + (node->pos.y - node->p.pos.y));
}


static void
sp_nodepath_set_line_type (SPPathNode * end, ArtPathcode code)
{
	SPPathNode * start;
	double dx, dy;

	g_assert (end);
	g_assert (end->subpath);
	g_assert (end->p.other);

	if (end->code == INK_STATIC_CAST(guint, code) )
		return;

	start = end->p.other;

	end->code = code;

	if (code == ART_LINETO) {
		if (start->code == ART_LINETO) start->type = SP_PATHNODE_CUSP;
		if (end->n.other) {
			if (end->n.other->code == ART_LINETO) end->type = SP_PATHNODE_CUSP;
		}
		sp_node_adjust_knot (start, -1);
		sp_node_adjust_knot (end, 1);
	} else {
		dx = end->pos.x - start->pos.x;
		dy = end->pos.y - start->pos.y;
		start->n.pos.x = start->pos.x + dx / 3;
		start->n.pos.y = start->pos.y + dy / 3;
		end->p.pos.x = end->pos.x - dx / 3;
		end->p.pos.y = end->pos.y - dy / 3;
		sp_node_adjust_knot (start, 1);
		sp_node_adjust_knot (end, -1);
	}

	sp_node_ensure_ctrls (start);
	sp_node_ensure_ctrls (end);
}

static SPPathNode *
sp_nodepath_set_node_type (SPPathNode * node, SPPathNodeType type)
{
	g_assert (node);
	g_assert (node->subpath);

	if (type == INK_STATIC_CAST(SPPathNodeType, INK_STATIC_CAST(guint, node->type) ) )
		return node;

	if ((node->p.other != NULL) && (node->n.other != NULL)) {
		if ((node->code == ART_LINETO) && (node->n.other->code == ART_LINETO)) {
			type = SP_PATHNODE_CUSP;
		}
	}

	node->type = type;

	if (node->type == SP_PATHNODE_CUSP) {
		g_object_set (G_OBJECT (node->knot), "shape", SP_KNOT_SHAPE_DIAMOND, "size", 9, NULL);
	} else {
		g_object_set (G_OBJECT (node->knot), "shape", SP_KNOT_SHAPE_SQUARE, "size", 7, NULL);
	}

	sp_node_adjust_knots (node);

	sp_nodepath_update_statusbar (node->subpath->nodepath);

	return node;
}

static void
sp_node_moveto (SPPathNode * node, double x, double y)
{
	SPNodePath * nodepath;
	ArtPoint p;
	double dx, dy;

	nodepath = node->subpath->nodepath;

	p.x = x;
	p.y = y;

	dx = p.x - node->pos.x;
	dy = p.y - node->pos.y;
	node->pos.x = p.x;
	node->pos.y = p.y;

	node->p.pos.x += dx;
	node->p.pos.y += dy;
	node->n.pos.x += dx;
	node->n.pos.y += dy;

	if (node->p.other) {
		if (node->code == ART_LINETO) {
			sp_node_adjust_knot (node, 1);
			sp_node_adjust_knot (node->p.other, -1);
		}
	}
	if (node->n.other) {
		if (node->n.other->code == ART_LINETO) {
			sp_node_adjust_knot (node, -1);
			sp_node_adjust_knot (node->n.other, 1);
		}
	}

	sp_node_ensure_ctrls (node);
}

static void
sp_nodepath_selected_nodes_move (SPNodePath * nodepath, gdouble dx, gdouble dy)
{
	gdouble dist, besth, bestv, bx, by;
	GList * l;

	besth = bestv = 1e18;
	bx = dx;
	by = dy;

	for (l = nodepath->selected; l != NULL; l = l->next) {
		SPPathNode * n;
		NRPoint p;
		n = (SPPathNode *) l->data;
		p.x = n->pos.x + dx;
		p.y = n->pos.y + dy;
		dist = sp_desktop_horizontal_snap (nodepath->desktop, &p);
		if (dist < besth) {
			besth = dist;
			bx = p.x - n->pos.x;
		}
		dist = sp_desktop_vertical_snap (nodepath->desktop, &p);
		if (dist < bestv) {
			bestv = dist;
			by = p.y - n->pos.y;
		}
	}

	for (l = nodepath->selected; l != NULL; l = l->next) {
		SPPathNode * n;
		n = (SPPathNode *) l->data;
		sp_node_moveto (n, n->pos.x + bx, n->pos.y + by);
	}

	update_object (nodepath);
}

void
sp_node_selected_move (gdouble dx, gdouble dy)
{
	SPNodePath * nodepath;

	nodepath = sp_nodepath_current ();
	if (!nodepath) return;

	sp_nodepath_selected_nodes_move (nodepath, dx, dy);

	if (dx == 0) {
		update_repr_keyed (nodepath, "node:move:vertical");
	} else if (dy == 0) {
		update_repr_keyed (nodepath, "node:move:horizontal");
	} else 
		update_repr (nodepath);
}

void
sp_node_selected_move_screen (gdouble dx, gdouble dy)
{
	SPDesktop * desktop;
	gdouble zdx, zdy, zoom;
	SPNodePath * nodepath;

	// borrowed from sp_selection_move_screen in selection-chemistry.c
	// we find out the current zoom factor and divide deltas by it
	desktop = SP_ACTIVE_DESKTOP;
	g_return_if_fail(SP_IS_DESKTOP (desktop));

	zoom = SP_DESKTOP_ZOOM (desktop);
	zdx = dx / zoom;
	zdy = dy / zoom;

	nodepath = sp_nodepath_current ();
	if (!nodepath) return;

	sp_nodepath_selected_nodes_move (nodepath, zdx, zdy);

	if (dx == 0) {
		update_repr_keyed (nodepath, "node:move:vertical");
	} else if (dy == 0) {
		update_repr_keyed (nodepath, "node:move:horizontal");
	} else 
		update_repr (nodepath);
}

static void
sp_node_ensure_knot (SPPathNode * node, gint which, gboolean show_knot)
{
	SPNodePath * nodepath;
	SPPathNodeSide * side;
	ArtPathcode code;
	NRPoint p;

	g_assert (node != NULL);

	nodepath = node->subpath->nodepath;

	side = sp_node_get_side (node, which);
	code = sp_node_path_code_from_side (node, side);

	show_knot = show_knot && (code == ART_CURVETO);

	if (show_knot) {
		if (!SP_KNOT_IS_VISIBLE (side->knot)) {
			sp_knot_show (side->knot);
		}

		p.x = side->pos.x;
		p.y = side->pos.y;

		sp_knot_set_position (side->knot, &p, 0);
		sp_canvas_item_show (side->line);

	} else {
		if (SP_KNOT_IS_VISIBLE (side->knot)) {
			sp_knot_hide (side->knot);
		}
		sp_canvas_item_hide (side->line);
	}
}

void
sp_node_ensure_ctrls (SPPathNode * node)
{
	SPNodePath * nodepath;
	NRPoint p;
	gboolean show_knots;

	g_assert (node != NULL);

	nodepath = node->subpath->nodepath;

	if (!SP_KNOT_IS_VISIBLE (node->knot)) {
		sp_knot_show (node->knot);
	}

	p.x = node->pos.x;
	p.y = node->pos.y;

	sp_knot_set_position (node->knot, &p, 0);

	show_knots = node->selected;
	if (node->p.other != NULL) {
		if (node->p.other->selected) show_knots = TRUE;
	}
	if (node->n.other != NULL) {
		if (node->n.other->selected) show_knots = TRUE;
	}

	sp_node_ensure_knot (node, -1, show_knots);
	sp_node_ensure_knot (node, 1, show_knots);
}

static void
sp_nodepath_subpath_ensure_ctrls (SPNodeSubPath * subpath)
{
	GList * l;

	g_assert (subpath != NULL);

	for (l = subpath->nodes; l != NULL; l = l->next) {
		sp_node_ensure_ctrls ((SPPathNode *) l->data);
	}
}

static void
sp_nodepath_ensure_ctrls (SPNodePath * nodepath)
{
	GList * l;

	g_assert (nodepath != NULL);

	for (l = nodepath->subpaths; l != NULL; l = l->next)
		sp_nodepath_subpath_ensure_ctrls ((SPNodeSubPath *) l->data);
}

void
sp_node_selected_add_node (void)
{
	SPNodePath * nodepath;
	GList * l, * nl;

	nodepath = sp_nodepath_current ();
	if (!nodepath) return;

	nl = NULL;

	for (l = nodepath->selected; l != NULL; l = l->next) {
		SPPathNode * t;
		t = (SPPathNode *) l->data;
		g_assert (t->selected);
		if (t->p.other && t->p.other->selected) nl = g_list_prepend (nl, t);
	}

	while (nl) {
		SPPathNode * t, * n;
		t = (SPPathNode *) nl->data;
		n = sp_nodepath_line_add_node (t, 0.5);
		sp_nodepath_node_select (n, TRUE, FALSE);
		nl = g_list_remove (nl, t);
	}

	/* fixme: adjust ? */
	sp_nodepath_ensure_ctrls (nodepath);

	update_repr (nodepath);

	sp_nodepath_update_statusbar (nodepath);
}

void
sp_node_selected_break (void)
{
	SPNodePath * nodepath;
	SPPathNode * n, * nn;
	GList * l;
	GList *temp = NULL;

	nodepath = sp_nodepath_current ();
	if (!nodepath) return;

	for (l = nodepath->selected; l != NULL; l = l->next) {
		n = (SPPathNode *) l->data;
		nn = sp_nodepath_node_break (n);
		if (nn == NULL) continue; // no break, no new node 
		temp = g_list_prepend (temp, nn);
	}

	if (temp) sp_nodepath_deselect (nodepath);
	for (l = temp; l != NULL; l = l->next) {
		sp_nodepath_node_select ((SPPathNode *) l->data, TRUE, TRUE);
	}

	sp_nodepath_ensure_ctrls (nodepath);

	update_repr (nodepath);
}

/**
\brief duplicate selected nodes
*/
void
sp_node_selected_duplicate (void)
{
	SPNodePath * nodepath;
	SPPathNode * n, * nn;
	GList * l;
	GList *temp = NULL;

	nodepath = sp_nodepath_current ();
	if (!nodepath) return;

	for (l = nodepath->selected; l != NULL; l = l->next) {
		n = (SPPathNode *) l->data;
		nn = sp_nodepath_node_duplicate (n);
		if (nn == NULL) continue; // could not duplicate
		temp = g_list_prepend (temp, nn);
	}

	if (temp) sp_nodepath_deselect (nodepath);
	for (l = temp; l != NULL; l = l->next) {
		sp_nodepath_node_select ((SPPathNode *) l->data, TRUE, TRUE);
	}

	sp_nodepath_ensure_ctrls (nodepath);

	update_repr (nodepath);
}

void
sp_node_selected_join (void)
{
	SPNodePath * nodepath;
	SPNodeSubPath * sa, * sb;
	SPPathNode * a, * b, * n;
	NRPoint p, c;
	ArtPathcode code;

	nodepath = sp_nodepath_current ();
	if (!nodepath || g_list_length (nodepath->selected) != 2) {
		sp_view_set_statusf_error (SP_VIEW(nodepath->desktop), "To join, you must have two endnodes selected.");
		return;
	}

	a = (SPPathNode *) nodepath->selected->data;
	b = (SPPathNode *) nodepath->selected->next->data;

	g_assert (a != b);
	g_assert (a->p.other || a->n.other);
	g_assert (b->p.other || b->n.other);

	if (((a->subpath->closed) || (b->subpath->closed)) || (a->p.other && a->n.other) || (b->p.other && b->n.other)) {
		sp_view_set_statusf_error (SP_VIEW(nodepath->desktop), "To join, you must have two endnodes selected.");
		return;
	}

	/* a and b are endpoints */

	c.x = (a->pos.x + b->pos.x) / 2;
	c.y = (a->pos.y + b->pos.y) / 2;

	if (a->subpath == b->subpath) {
		SPNodeSubPath * sp;

		sp = a->subpath;
		sp_nodepath_subpath_close (sp);

		sp_nodepath_ensure_ctrls (sp->nodepath);

		update_repr (nodepath);

		return;
	}

	/* a and b are separate subpaths */
	sa = a->subpath;
	sb = b->subpath;

	if (a == sa->first) {
		SPNodeSubPath *t;
		p = sa->first->n.pos;
		code = (ArtPathcode)sa->first->n.other->code;
		t = sp_nodepath_subpath_new (sa->nodepath);
		n = sa->last;
		sp_nodepath_node_new (t, NULL, SP_PATHNODE_CUSP, ART_MOVETO, &n->n.pos, &n->pos, &n->p.pos);
		n = n->p.other;
		while (n) {
			sp_nodepath_node_new (t, NULL, (SPPathNodeType)n->type, (ArtPathcode)n->n.other->code, &n->n.pos, &n->pos, &n->p.pos);
			n = n->p.other;
			if (n == sa->first) n = NULL;
		}
		sp_nodepath_subpath_destroy (sa);
		sa = t;
	} else if (a == sa->last) {
		p = sa->last->p.pos;
		code = (ArtPathcode)sa->last->code;
		sp_nodepath_node_destroy (sa->last);
	} else {
		code = ART_END;
		g_assert_not_reached ();
	}

	if (b == sb->first) {
		sp_nodepath_node_new (sa, NULL, SP_PATHNODE_CUSP, code, &p, &c, &sb->first->n.pos);
		for (n = sb->first->n.other; n != NULL; n = n->n.other) {
			sp_nodepath_node_new (sa, NULL, (SPPathNodeType)n->type, (ArtPathcode)n->code, &n->p.pos, &n->pos, &n->n.pos);
		}
	} else if (b == sb->last) {
		sp_nodepath_node_new (sa, NULL, SP_PATHNODE_CUSP, code, &p, &c, &sb->last->p.pos);
		for (n = sb->last->p.other; n != NULL; n = n->p.other) {
			sp_nodepath_node_new (sa, NULL, (SPPathNodeType)n->type, (ArtPathcode)n->n.other->code, &n->n.pos, &n->pos, &n->p.pos);
		}
	} else {
		g_assert_not_reached ();
	}
	/* and now destroy sb */

	sp_nodepath_subpath_destroy (sb);

	sp_nodepath_ensure_ctrls (sa->nodepath);

	update_repr (nodepath);

	sp_nodepath_update_statusbar (nodepath);
}

void
sp_node_selected_join_segment (void)
{
	SPNodePath * nodepath;
	SPNodeSubPath * sa, * sb;
	SPPathNode * a, * b, * n;
	NRPoint p;
	ArtPathcode code;

	nodepath = sp_nodepath_current ();
	if (!nodepath || g_list_length (nodepath->selected) != 2) {
		sp_view_set_statusf_error (SP_VIEW(nodepath->desktop), "To join, you must have two endnodes selected.");
		return;
	}

	a = (SPPathNode *) nodepath->selected->data;
	b = (SPPathNode *) nodepath->selected->next->data;

	g_assert (a != b);
	g_assert (a->p.other || a->n.other);
	g_assert (b->p.other || b->n.other);

	if (((a->subpath->closed) || (b->subpath->closed)) || (a->p.other && a->n.other) || (b->p.other && b->n.other)) {
		sp_view_set_statusf_error (SP_VIEW(nodepath->desktop), "To join, you must have two endnodes selected.");
		return;
	}

	if (a->subpath == b->subpath) {
		SPNodeSubPath * sp;

		sp = a->subpath;

		/*similar to sp_nodepath_subpath_close (sp), without the node destruction*/
		sp->closed = TRUE;

		sp->first->p.other = sp->last;
		sp->last->n.other = sp->first;
		
		sp_node_control_mirror_p_to_n (sp->last);
		sp_node_control_mirror_n_to_p (sp->first);

		sp->first->code = sp->last->code;
		sp->first = sp->last;

		sp_nodepath_ensure_ctrls (sp->nodepath);

		update_repr (nodepath);

		return;
	}

	/* a and b are separate subpaths */
	sa = a->subpath;
	sb = b->subpath;

	if (a == sa->first) {
		SPNodeSubPath *t;
		code = (ArtPathcode) sa->first->n.other->code;
		t = sp_nodepath_subpath_new (sa->nodepath);
		n = sa->last;
		sp_nodepath_node_new (t, NULL, SP_PATHNODE_CUSP, ART_MOVETO, &n->n.pos, &n->pos, &n->p.pos);
		for (n = n->p.other; n != NULL; n = n->p.other) {
			sp_nodepath_node_new (t, NULL, (SPPathNodeType)n->type, (ArtPathcode)n->n.other->code, &n->n.pos, &n->pos, &n->p.pos);
		}
		sp_nodepath_subpath_destroy (sa);
		sa = t;
	} else if (a == sa->last) {
		code = (ArtPathcode)sa->last->code;
	} else {
		code = ART_END;
		g_assert_not_reached ();
	}

	if (b == sb->first) {
		n = sb->first;
		sp_node_control_mirror_p_to_n (sa->last);
		sp_nodepath_node_new (sa, NULL, SP_PATHNODE_CUSP, code, &n->p.pos, &n->pos, &n->n.pos);
		sp_node_control_mirror_n_to_p (sa->last);
		for (n = n->n.other; n != NULL; n = n->n.other) {
			sp_nodepath_node_new (sa, NULL, (SPPathNodeType)n->type, (ArtPathcode)n->code, &n->p.pos, &n->pos, &n->n.pos);
		}
	} else if (b == sb->last) {
		n = sb->last;
		sp_node_control_mirror_p_to_n (sa->last);
		sp_nodepath_node_new (sa, NULL, SP_PATHNODE_CUSP, code, &p, &n->pos, &n->p.pos);
		sp_node_control_mirror_n_to_p (sa->last);
		for (n = n->p.other; n != NULL; n = n->p.other) {
 			sp_nodepath_node_new (sa, NULL, (SPPathNodeType)n->type, (ArtPathcode)n->n.other->code, &n->n.pos, &n->pos, &n->p.pos);
		}
	} else {
		g_assert_not_reached ();
	}
	/* and now destroy sb */

	sp_nodepath_subpath_destroy (sb);

	sp_nodepath_ensure_ctrls (sa->nodepath);

	update_repr (nodepath);
}

void
sp_node_selected_delete (void)
{
	SPNodePath * nodepath;
	SPPathNode * node;

	nodepath = sp_nodepath_current ();
	if (!nodepath) return;
	if (!nodepath->selected) return;

	/* fixme: do it the right way */
	while (nodepath->selected) {
		node = (SPPathNode *) nodepath->selected->data;
		sp_nodepath_node_destroy (node);
	}

	sp_nodepath_ensure_ctrls (nodepath);

	update_repr (nodepath);

	if (g_list_length (nodepath->subpaths) == 0) { // if the entire nodepath is removed, delete the selected object
		sp_nodepath_destroy (nodepath);
		sp_selection_delete (NULL, NULL);
	}

	sp_nodepath_update_statusbar (nodepath);
}

void
sp_node_selected_set_line_type (ArtPathcode code)
{
	SPNodePath * nodepath;
	GList * l;

	nodepath = sp_nodepath_current ();
	if (nodepath == NULL) return;

	for (l = nodepath->selected; l != NULL; l = l->next) {
		SPPathNode * n;
		n = (SPPathNode *) l->data;
		g_assert (n->selected);
		if (n->p.other && n->p.other->selected) {
			sp_nodepath_set_line_type (n, code);
		}
	}

	update_repr (nodepath);
}

void
sp_node_selected_set_type (SPPathNodeType type)
{
	SPNodePath * nodepath;
	GList * l;

	/* fixme: do it the right way */
	nodepath = sp_nodepath_current ();
	if (nodepath == NULL) return;

	for (l = nodepath->selected; l != NULL; l = l->next) {
		sp_nodepath_set_node_type ((SPPathNode *) l->data, type);
	}

	update_repr (nodepath);
}

static void
sp_node_set_selected (SPPathNode * node, gboolean selected)
{
	node->selected = selected;

	if (selected) {
		g_object_set (G_OBJECT (node->knot),
			      "fill", NODE_FILL_SEL,
			      "fill_mouseover", NODE_FILL_SEL_HI,
			      "stroke", NODE_STROKE_SEL,
			      "stroke_mouseover", NODE_STROKE_SEL_HI,
			      NULL);
	} else {
		g_object_set (G_OBJECT (node->knot),
			      "fill", NODE_FILL,
			      "fill_mouseover", NODE_FILL_HI,
			      "stroke", NODE_STROKE,
			      "stroke_mouseover", NODE_STROKE_HI,
			      NULL);
	}

	sp_node_ensure_ctrls (node);
	if (node->n.other) sp_node_ensure_ctrls (node->n.other);
	if (node->p.other) sp_node_ensure_ctrls (node->p.other);
}

/**
\brief select a node
\param node     the node to select
\param incremental   if true, add to selection, otherwise deselect others
\param override   if true, always select this node, otherwise toggle selected status
*/
void
sp_nodepath_node_select (SPPathNode * node, gboolean incremental, gboolean override)
{
	SPNodePath * nodepath;

	nodepath = node->subpath->nodepath;

	if (incremental) {
		if (override) {
			if (!g_list_find (nodepath->selected, node)) {
				nodepath->selected = g_list_append (nodepath->selected, node);
			}
			sp_node_set_selected (node, TRUE);
		} else { // toggle
			if (node->selected) {
				g_assert (g_list_find (nodepath->selected, node));
				nodepath->selected = g_list_remove (nodepath->selected, node);
			} else {
				g_assert (!g_list_find (nodepath->selected, node));
				nodepath->selected = g_list_append (nodepath->selected, node);
			}
			sp_node_set_selected (node, !node->selected);
		}
	} else {
		sp_nodepath_deselect (nodepath);
		nodepath->selected = g_list_append (nodepath->selected, node);
		sp_node_set_selected (node, TRUE);
	}

	sp_nodepath_update_statusbar (nodepath);
}

/**
\brief deselect a node
\param node     the node to deselect
*/
void
sp_nodepath_node_deselect (SPPathNode * node)
{
	SPNodePath * nodepath;

	nodepath = node->subpath->nodepath;

	if (nodepath->selected) {
		sp_node_set_selected (node, FALSE);
		nodepath->selected = g_list_remove (nodepath->selected, node);
	}

	sp_nodepath_update_statusbar (nodepath);
}


/**
\brief deselect all nodes in the nodepath
*/
void
sp_nodepath_deselect (SPNodePath *nodepath)
{
	if (!nodepath) return; // there's no nodepath when editing rects, stars, spirals or ellipses

	while (nodepath->selected) {
		sp_node_set_selected ((SPPathNode *) nodepath->selected->data, FALSE);
		nodepath->selected = g_list_remove (nodepath->selected, nodepath->selected->data);
	}
	sp_nodepath_update_statusbar (nodepath);
}

/**
\brief select all nodes in the nodepath
*/
void
sp_nodepath_select_all (SPNodePath *nodepath)
{
	SPNodeSubPath * subpath;
	SPPathNode * node;
	GList * spl, * nl;

	if (!nodepath) return;

	for (spl = nodepath->subpaths; spl != NULL; spl = spl->next) {
		subpath = (SPNodeSubPath *) spl->data;
		for (nl = subpath->nodes; nl != NULL; nl = nl->next) {
			node = (SPPathNode *) nl->data;
			sp_nodepath_node_select (node, TRUE, TRUE);
		}
	}
}

/**
\brief select the node after the last selected; if none is selected, select the first within path
*/
void
sp_nodepath_select_next (SPNodePath *nodepath)
{
	SPPathNode *node, *last = NULL;
	SPNodeSubPath * subpath, *subpath_next;
	GList * spl, * nl = NULL;

	if (nodepath->selected) {
		for (spl = nodepath->subpaths; spl != NULL; spl = spl->next) {
			subpath = (SPNodeSubPath *) spl->data;
			for (nl = subpath->nodes; nl != NULL; nl = nl->next) {
				node = (SPPathNode *) nl->data;
				if (node->selected) {
					if (node->n.other == (SPPathNode *) subpath->last) {
						if (node->n.other == (SPPathNode *) subpath->first) { // closed subpath 
							if (spl->next) { // there's a next subpath
								subpath_next = (SPNodeSubPath *) spl->next->data;
								last = subpath_next->first;
							} else if (spl->prev) { // there's a previous subpath
								last = NULL; // to be set later to the first node of first subpath
							} else {
								last = node->n.other;
							}
						} else {
							last = node->n.other;
						}
					} else {
						if (node->n.other) {
							last = node->n.other;
						} else {
							if (spl->next) { // there's a next subpath
								subpath_next = (SPNodeSubPath *) spl->next->data;
								last = subpath_next->first;
							} else if (spl->prev) { // there's a previous subpath
								last = NULL; // to be set later to the first node of first subpath
							} else {
								last = (SPPathNode *) subpath->first;
							}
						}
					}
				}
			}
		}
		sp_nodepath_deselect (nodepath);
	}

	if (last) { // there's at least one more node after selected
		sp_nodepath_node_select ((SPPathNode *) last, TRUE, TRUE);
	} else { // no more nodes, select the first one in first subpath
		subpath = (SPNodeSubPath *) nodepath->subpaths->data;
		sp_nodepath_node_select ((SPPathNode *) subpath->first, TRUE, TRUE);
	}
}

/**
\brief select the node before the first selected; if none is selected, select the last within path
*/
void
sp_nodepath_select_prev (SPNodePath *nodepath)
{
	SPPathNode *node, *last = NULL;
	SPNodeSubPath * subpath, *subpath_prev;
	GList * spl, * nl = NULL;

	if (nodepath->selected) {
		for (spl = g_list_last (nodepath->subpaths); spl != NULL; spl = spl->prev) {
			subpath = (SPNodeSubPath *) spl->data;
			for (nl = g_list_last (subpath->nodes); nl != NULL; nl = nl->prev) {
				node = (SPPathNode *) nl->data;
				if (node->selected) {
					if (node->p.other == (SPPathNode *) subpath->first) {
						if (node->p.other == (SPPathNode *) subpath->last) { // closed subpath 
							if (spl->prev) { // there's a prev subpath
								subpath_prev = (SPNodeSubPath *) spl->prev->data;
								last = subpath_prev->last;
							} else if (spl->next) { // there's a next subpath
								last = NULL; // to be set later to the last node of last subpath
							} else {
								last = node->p.other;
							}
						} else {
							last = node->p.other;
						}
					} else {
						if (node->p.other) {
							last = node->p.other;
						} else {
							if (spl->prev) { // there's a prev subpath
								subpath_prev = (SPNodeSubPath *) spl->prev->data;
								last = subpath_prev->last;
							} else if (spl->next) { // there's a next subpath
								last = NULL; // to be set later to the last node of last subpath
							} else {
								last = (SPPathNode *) subpath->last;
							}
						}
					}
				}
			}
		}
		sp_nodepath_deselect (nodepath);
	}

	if (last) { // there's at least one more node after selected
		sp_nodepath_node_select ((SPPathNode *) last, TRUE, TRUE);
	} else { // no more nodes, select the last one in last subpath
		spl = g_list_last (nodepath->subpaths);
		subpath = (SPNodeSubPath *) spl->data;
		sp_nodepath_node_select ((SPPathNode *) subpath->last, TRUE, TRUE);
	}
}

/**
\brief select all nodes that are within the rectangle
*/
void
sp_nodepath_select_rect (SPNodePath *nodepath, NRRect *b, gboolean incremental)
{
	SPNodeSubPath * subpath;
	SPPathNode * node;
	ArtPoint p;
	GList * spl, * nl;

	if (!incremental) {
		sp_nodepath_deselect (nodepath);
	}

	for (spl = nodepath->subpaths; spl != NULL; spl = spl->next) {
		subpath = (SPNodeSubPath *) spl->data;
		for (nl = subpath->nodes; nl != NULL; nl = nl->next) {
			node = (SPPathNode *) nl->data;
#if 0
			art_affine_point (&p, &node->pos, nodepath->i2d);
#else
			p.x = node->pos.x;
			p.y = node->pos.y;
#endif
			if ((p.x > b->x0) && (p.x < b->x1) && (p.y > b->y0) && (p.y < b->y1)) {
				sp_nodepath_node_select (node, TRUE, FALSE);
			}
		}
	}
}

/**
\brief saves selected nodes in a nodepath into a list containing integer positions of all selected nodes
*/
GList *save_nodepath_selection (SPNodePath *nodepath)
{
	GList *r = NULL;
	SPPathNode *node, *last = NULL;
	SPNodeSubPath * subpath, *subpath_next;
	GList * spl, * nl = NULL;
	guint i = 0;

	if (!nodepath->selected) return NULL; 

		for (spl = nodepath->subpaths; spl != NULL; spl = spl->next) {
			subpath = (SPNodeSubPath *) spl->data;
			for (nl = subpath->nodes; nl != NULL; nl = nl->next) {
				node = (SPPathNode *) nl->data;
				i ++; 
				if (node->selected) {
					r = g_list_append (r, GINT_TO_POINTER (i));
					}
				}
			}
		return r;
}

/**
\brief restores selection by selecting nodes whose positions are in the list
*/
void restore_nodepath_selection (SPNodePath *nodepath, GList *r)
{
	SPPathNode *node, *last = NULL;
	SPNodeSubPath * subpath, *subpath_next;
	GList * spl, * nl = NULL;
	guint i = 0;

		sp_nodepath_deselect (nodepath);

		for (spl = nodepath->subpaths; spl != NULL; spl = spl->next) {
			subpath = (SPNodeSubPath *) spl->data;
			for (nl = subpath->nodes; nl != NULL; nl = nl->next) {
				node = (SPPathNode *) nl->data;
				i ++; 
				if (g_list_find (r, GINT_TO_POINTER (i))) {
					sp_nodepath_node_select (node, TRUE, TRUE);
					}
				}
			}

}

/**
\brief adjusts control point according to node type and line code
*/
static void
sp_node_adjust_knot (SPPathNode * node, gint which_adjust)
{
	SPPathNode * othernode;
	SPPathNodeSide * me, * other;
	ArtPathcode mecode, ocode;
	double len, otherlen, linelen, dx, dy;

	g_assert (node);

	me = sp_node_get_side (node, which_adjust);
	other = sp_node_opposite_side (node, me);

	/* fixme: */
	if (me->other == NULL) return;
	if (other->other == NULL) return;

	/* I have line */

	if (which_adjust == 1) {
		mecode = (ArtPathcode)me->other->code;
		ocode = (ArtPathcode)node->code;
	} else {
		mecode = (ArtPathcode)node->code;
		ocode = (ArtPathcode)other->other->code;
	}

	if (mecode == ART_LINETO) return;

	/* I am curve */

	if (other->other == NULL) return;

	/* Other has line */

	if (node->type == SP_PATHNODE_CUSP) return;

	if (ocode == ART_LINETO) {
		/* other is lineto, we are either smooth or symm */
		othernode = other->other;
		dx = me->pos.x - node->pos.x;
		dy = me->pos.y - node->pos.y;
		len = hypot (dx, dy);
		dx = node->pos.x - othernode->pos.x;
		dy = node->pos.y - othernode->pos.y;
		linelen = hypot (dx, dy);
		if (linelen < 1e-18) return;

		me->pos.x = node->pos.x + dx * len / linelen;
		me->pos.y = node->pos.y + dy * len / linelen;
		sp_knot_set_position (me->knot, &me->pos, 0);

		sp_node_ensure_ctrls (node);
		return;
	}

	if (node->type == SP_PATHNODE_SYMM) {

		me->pos.x = 2 * node->pos.x - other->pos.x;
		me->pos.y = 2 * node->pos.y - other->pos.y;
		sp_knot_set_position (me->knot, &me->pos, 0);

		sp_node_ensure_ctrls (node);
		return;
	}

	/* We are smooth */

	dx = me->pos.x - node->pos.x;
	dy = me->pos.y - node->pos.y;
	len = hypot (dx, dy);
	dx = other->pos.x - node->pos.x;
	dy = other->pos.y - node->pos.y;
	otherlen = hypot (dx, dy);
	if (otherlen < 1e-18) return;

	me->pos.x = node->pos.x - dx * len / otherlen;
	me->pos.y = node->pos.y - dy * len / otherlen;
	sp_knot_set_position (me->knot, &me->pos, 0);

	sp_node_ensure_ctrls (node);
}

/**
 \brief Adjusts control point according to node type and line code
 */
static void
sp_node_adjust_knots (SPPathNode * node)
{
	SPNodePath * nodepath;
	double dx, dy, pdx, pdy, ndx, ndy, plen, nlen, scale;

	g_assert (node);

	nodepath = node->subpath->nodepath;

	if (node->type == SP_PATHNODE_CUSP) return;

	/* we are either smooth or symm */

	if (node->p.other == NULL) return;

	if (node->n.other == NULL) return;

	if (node->code == ART_LINETO) {
		if (node->n.other->code == ART_LINETO) return;
		sp_node_adjust_knot (node, 1);
		sp_node_ensure_ctrls (node);
		return;
	}

	if (node->n.other->code == ART_LINETO) {
		if (node->code == ART_LINETO) return;
		sp_node_adjust_knot (node, -1);
		sp_node_ensure_ctrls (node);
		return;
	}

	/* both are curves */

	dx = node->n.pos.x - node->p.pos.x;
	dy = node->n.pos.y - node->p.pos.y;

	if (node->type == SP_PATHNODE_SYMM) {
		node->p.pos.x = node->pos.x - dx / 2;
		node->p.pos.y = node->pos.y - dy / 2;
		node->n.pos.x = node->pos.x + dx / 2;
		node->n.pos.y = node->pos.y + dy / 2;
		sp_node_ensure_ctrls (node);
		return;
	}

	/* We are smooth */

	pdx = node->p.pos.x - node->pos.x;
	pdy = node->p.pos.y - node->pos.y;
	plen = hypot (pdx, pdy);
	if (plen < 1e-18) return;
	ndx = node->n.pos.x - node->pos.x;
	ndy = node->n.pos.y - node->pos.y;
	nlen = hypot (ndx, ndy);
	if (nlen < 1e-18) return;
	scale = plen / (plen + nlen);
	node->p.pos.x = node->pos.x - dx * scale;
	node->p.pos.y = node->pos.y - dy * scale;
	scale = nlen / (plen + nlen);
	node->n.pos.x = node->pos.x + dx * scale;
	node->n.pos.y = node->pos.y + dy * scale;
	sp_node_ensure_ctrls (node);
}

/*
 * Knot events
 */
static gboolean
node_event (SPKnot * knot, GdkEvent * event, SPPathNode * n)
{
	gboolean ret = FALSE;
	switch (event->type) {
	case GDK_ENTER_NOTIFY:
		active_node = n;
		break;
	case GDK_LEAVE_NOTIFY:
		active_node = NULL;
		break;
	case GDK_KEY_PRESS:
		switch (event->key.keyval) {
		case GDK_space:
			if (event->key.state & GDK_BUTTON1_MASK) {
				SPNodePath * nodepath;
				nodepath = n->subpath->nodepath;
				stamp_repr(nodepath);
				ret = TRUE;
			}
			break;
		default:
			break;
		}
		break;
	default:
		break;
	}

	return ret;
}

gboolean node_key (GdkEvent * event)
{
	SPNodePath *np;
	gint ret;

	// there is no way to verify nodes so set active_node to nil when deleting!!
	if (active_node == NULL) return FALSE;

	if ((event->type == GDK_KEY_PRESS) && !(event->key.state & (GDK_SHIFT_MASK | GDK_CONTROL_MASK))) {
		ret = FALSE;
		switch (event->key.keyval) {
		case GDK_BackSpace:
			np = active_node->subpath->nodepath;
			sp_nodepath_node_destroy (active_node);
			update_repr (np);
			active_node = NULL;
			ret = TRUE;
			break;
		case GDK_c:
			sp_nodepath_set_node_type (active_node, SP_PATHNODE_CUSP);
			ret = TRUE;
			break;
		case GDK_s:
			sp_nodepath_set_node_type (active_node, SP_PATHNODE_SMOOTH);
			ret = TRUE;
			break;
		case GDK_y:
			sp_nodepath_set_node_type (active_node, SP_PATHNODE_SYMM);
			ret = TRUE;
			break;
		case GDK_b:
			sp_nodepath_node_break (active_node);
			ret = TRUE;
			break;
		}
		return ret;
	}
	return FALSE;
}

static void
node_clicked (SPKnot * knot, guint state, gpointer data)
{
	SPPathNode * n;

	n = (SPPathNode *) data;

	if (state & GDK_CONTROL_MASK) {
		if (n->type == SP_PATHNODE_CUSP) {
			sp_nodepath_set_node_type (n, SP_PATHNODE_SMOOTH);
		} else {
			sp_nodepath_set_node_type (n, SP_PATHNODE_CUSP);
		}
	} else {
		sp_nodepath_node_select (n, (state & GDK_SHIFT_MASK), FALSE);
	}
}

static void
node_grabbed (SPKnot * knot, guint state, gpointer data)
{
	SPPathNode * n;

	n = (SPPathNode *) data;

	n->origin.x = knot->x;
	n->origin.y = knot->y;

	if (!n->selected) {
		sp_nodepath_node_select (n, (state & GDK_SHIFT_MASK), FALSE);
	}
}

static void
node_ungrabbed (SPKnot * knot, guint state, gpointer data)
{
	SPPathNode * n;

	n = (SPPathNode *) data;

	update_repr (n->subpath->nodepath);
}

void
xy_to_radial (double x, double y, radial *r)
{
	r->r = sqrt(x*x + y*y);
	if (r->r > 0) r->a = atan2 (y, x);
	else r->a = HUGE_VAL; //undefined
}

void
radial_to_xy (radial *r, NRPoint *origin, NRPoint *p)
{
	if (r->a == HUGE_VAL) {
		p->x = origin->x;
		p->y = origin->y;
	} else {
		p->x = origin->x + cos(r->a)*(r->r);
		p->y = origin->y + sin(r->a)*(r->r);
	}
}

double
closest_of_three (double x, double a, double b, double c)
{
	if (fabs(x-a) <= fabs(x-b) && fabs(x-a) <= fabs(x-c))
		return a; 
	if (fabs(x-b) <= fabs(x-a) && fabs(x-b) <= fabs(x-c))
		return b; 
	if (fabs(x-c) <= fabs(x-a) && fabs(x-c) <= fabs(x-b))
		return c; 
	g_assert_not_reached ();
}

double
closest_of_six (double x, double a, double b, double c, double d, double e, double f)
{
	if (fabs(x-a) <= fabs(x-b) && fabs(x-a) <= fabs(x-c) && fabs(x-a) <= fabs(x-d) && fabs(x-a) <= fabs(x-e) && fabs(x-a) <= fabs(x-f))
		return a; 
	if (fabs(x-b) <= fabs(x-a) && fabs(x-b) <= fabs(x-c) && fabs(x-b) <= fabs(x-d) && fabs(x-b) <= fabs(x-e) && fabs(x-b) <= fabs(x-f))
		return b; 
	if (fabs(x-c) <= fabs(x-a) && fabs(x-c) <= fabs(x-b) && fabs(x-c) <= fabs(x-d) && fabs(x-c) <= fabs(x-e) && fabs(x-c) <= fabs(x-f))
		return c; 
	if (fabs(x-d) <= fabs(x-a) && fabs(x-d) <= fabs(x-b) && fabs(x-d) <= fabs(x-c) && fabs(x-d) <= fabs(x-e) && fabs(x-d) <= fabs(x-f))
		return d; 
	if (fabs(x-e) <= fabs(x-a) && fabs(x-e) <= fabs(x-b) && fabs(x-e) <= fabs(x-c) && fabs(x-e) <= fabs(x-d) && fabs(x-e) <= fabs(x-f))
		return e; 
	if (fabs(x-f) <= fabs(x-a) && fabs(x-f) <= fabs(x-b) && fabs(x-f) <= fabs(x-c) && fabs(x-f) <= fabs(x-d) && fabs(x-f) <= fabs(x-e))
		return f; 
	g_assert_not_reached ();
}

double
angle_normalize (double a)
{
	if (a > M_PI) return a - 2*M_PI;
	if (a < -M_PI) return a + 2*M_PI;
	return a;
}

/**
\brief The point on a line, given by its angle, closest to the given point
\param p   point
\param a   angle of the line; it is assumed to go through coordinate origin
\param closest   pointer to the point struct where the result is stored
*/
void
point_line_closest (NRPoint *p, double a, NRPoint *closest)
{
	if (a == HUGE_VAL) { // vertical
		closest->x = 0; 
		closest->y = p->y; 
	} else {
		closest->x = (a*p->y + p->x)/(a*a + 1);
		closest->y = a*closest->x;
	}
}

/**
\brief Distance from the point to a line given by its angle
\param p   point
\param a   angle of the line; it is assumed to go through coordinate origin
*/
double
point_line_distance (NRPoint *p, double a)
{
	NRPoint c;
	point_line_closest (p, a, &c);
	return sqrt ((p->x - c.x)*(p->x - c.x) + (p->y - c.y)*(p->y - c.y));
}


/* fixme: This goes to "moved" event? */
static gboolean
node_request (SPKnot *knot, NRPoint *p, guint state, gpointer data)
{
	SPPathNode * n;
	double yn, xn, yp, xp;
	double an, ap, na, pa;
	double d_an, d_ap, d_na, d_pa;
	gboolean collinear = FALSE;
	NRPoint c;
	NRPoint pr; 

	n = (SPPathNode *) data;

	if (state & GDK_CONTROL_MASK) { // constrained motion 

		if (state & GDK_MOD1_MASK && !(xn == 0 && xp == 0)) { 
			// sliding on handles, only if at least one of the handles is non-vertical

			// calculate relative distances of control points
			yn = n->n.pos.y - n->pos.y; 
			xn = n->n.pos.x - n->pos.x;
			if (xn < 0) { xn = -xn; yn = -yn; } // limit the handle angle to between 0 and pi
			if (yn < 0) { xn = -xn; yn = -yn; } 

			yp = n->p.pos.y - n->pos.y;
			xp = n->p.pos.x - n->pos.x;
			if (xp < 0) { xp = -xp; yp = -yp; } // limit the handle angle to between 0 and pi
			if (yp < 0) { xp = -xp; yp = -yp; } 

			// calculate angles of the control handles
			if (xn == 0) {
				if (yn == 0) { // no handle, consider it the continuation of the other one
					an = 0; 
					collinear = TRUE;
				} 
				else an = 0; // vertical; set the angle to horizontal
			} else an = yn/xn;

			if (xp == 0) {
				if (yp == 0) { // no handle, consider it the continuation of the other one
					ap = an; 
				}
				else ap = 0; // vertical; set the angle to horizontal
			} else  ap = yp/xp; 

			if (collinear) an = ap;

			// angles of the perpendiculars; HUGE_VAL means vertical
			if (an == 0) na = HUGE_VAL; else na = -1/an;
			if (ap == 0) pa = HUGE_VAL; else pa = -1/ap;

			//	g_print("an %g    ap %g\n", an, ap);

			// mouse point relative to the node's original pos
			pr.x = p->x - n->origin.x;
			pr.y = p->y - n->origin.y;

			// distances to the four lines (two handles and to perpendiculars)
			d_an = point_line_distance(&pr, an);
			d_na = point_line_distance(&pr, na);
			d_ap = point_line_distance(&pr, ap);
			d_pa = point_line_distance(&pr, pa);

			// find out which line is the closest, save its closest point in c
			if (d_an <= d_na && d_an <= d_ap && d_an <= d_pa) {
				point_line_closest(&pr, an, &c);
			} else if (d_ap <= d_an && d_ap <= d_na && d_ap <= d_pa) {
				point_line_closest(&pr, ap, &c);
			} else if (d_na <= d_an && d_na <= d_ap && d_na <= d_pa) {
				point_line_closest(&pr, na, &c);
			} else if (d_pa <= d_an && d_pa <= d_ap && d_pa <= d_na) {
				point_line_closest(&pr, pa, &c);
			}

			// move the node to the closest point
			sp_nodepath_selected_nodes_move (n->subpath->nodepath, n->origin.x + c.x - n->pos.x, n->origin.y + c.y - n->pos.y);

		} else {  // constraining to hor/vert

			if (fabs(p->x - n->origin.x) > fabs(p->y - n->origin.y)) { // snap to hor
				sp_nodepath_selected_nodes_move (n->subpath->nodepath, p->x - n->pos.x, n->origin.y - n->pos.y);
			} else { // snap to vert
				sp_nodepath_selected_nodes_move (n->subpath->nodepath, n->origin.x - n->pos.x, p->y - n->pos.y);
			}
		}
	} else { // move freely
		sp_nodepath_selected_nodes_move (n->subpath->nodepath, p->x - n->pos.x, p->y - n->pos.y);
	}

	return TRUE;
}

static void
node_ctrl_clicked (SPKnot * knot, guint state, gpointer data)
{
	SPPathNode * n;

	n = (SPPathNode *) data;

	sp_nodepath_node_select (n, (state & GDK_SHIFT_MASK), FALSE);
}

static void
node_ctrl_grabbed (SPKnot * knot, guint state, gpointer data)
{
	SPPathNode * n;

	n = (SPPathNode *) data;

	if (!n->selected) {
		sp_nodepath_node_select (n, (state & GDK_SHIFT_MASK), FALSE);
	}

	// remember the origin of the control
	if (n->p.knot == knot) {
		xy_to_radial (n->p.pos.x - n->pos.x, n->p.pos.y - n->pos.y, &(n->p.origin));
	} else if (n->n.knot == knot) {
		xy_to_radial (n->n.pos.x - n->pos.x, n->n.pos.y - n->pos.y, &(n->n.origin));
	} else {
		g_assert_not_reached ();
	}

}

static void
node_ctrl_ungrabbed (SPKnot * knot, guint state, gpointer data)
{
	SPPathNode * n;

	n = (SPPathNode *) data;

	// forget origin and set knot position once more (because it can be wrong now due to restrictions)
	if (n->p.knot == knot) {
		n->p.origin.a = 0;
		sp_knot_set_position (knot, &(n->p.pos), state);
	} else if (n->n.knot == knot) {
		n->n.origin.a = 0;
		sp_knot_set_position (knot, &(n->n.pos), state);
	} else {
		g_assert_not_reached ();
	}

	update_repr (n->subpath->nodepath);
}

static gboolean
node_ctrl_request (SPKnot * knot, NRPoint *p, guint state, gpointer data)
{
	SPPathNode * n;
	SPPathNodeSide * me, * opposite;
	ArtPathcode othercode;
	gint which;

	n = (SPPathNode *) data;

	if (n->p.knot == knot) {
		me = &n->p;
		opposite = &n->n;
		which = -1;
	} else if (n->n.knot == knot) {
		me = &n->n;
		opposite = &n->p;
		which = 1;
	} else {
		me = opposite = NULL;
		which = 0;
		g_assert_not_reached ();
	}

	othercode = sp_node_path_code_from_side (n, opposite);

	if (opposite->other && (n->type != SP_PATHNODE_CUSP) && (othercode == ART_LINETO)) {
		SPPathNode * othernode;
		gdouble dx, dy, ndx, ndy, len, linelen, scal;
		/* We are smooth node adjacent with line */
		dx = p->x - n->pos.x;
		dy = p->y - n->pos.y;
		len = hypot (dx, dy);
		othernode = opposite->other;
		ndx = n->pos.x - othernode->pos.x;
		ndy = n->pos.y - othernode->pos.y;
		linelen = hypot (ndx, ndy);
		if ((len > 1e-18) && (linelen > 1e-18)) {
			scal = (dx * ndx + dy * ndy) / linelen;
			p->x = n->pos.x + ndx / linelen * scal;
			p->y = n->pos.y + ndy / linelen * scal;
		}
		sp_desktop_vector_snap (n->subpath->nodepath->desktop, p, ndx, ndy);
	} else {
		sp_desktop_free_snap (n->subpath->nodepath->desktop, p);
	}

	sp_node_adjust_knot (n, -which);

	return FALSE;
}

static void
node_ctrl_moved (SPKnot *knot, NRPoint *p, guint state, gpointer data)
{
	SPPathNode * n;
	SPPathNodeSide *me, *other;
	double r_down, r_up;
	radial rme, rother, rnew; 
	NRPoint o;

	n = (SPPathNode *) data;

	if (n->p.knot == knot) {
		me = &n->p;
		other = &n->n;
	} else if (n->n.knot == knot) {
		me = &n->n;
		other = &n->p;
	} else {
		me = NULL;
		g_assert_not_reached ();
	}

	// calculate radial coordinates of the grabbed control, other control, and the mouse point
	xy_to_radial (me->pos.x - n->pos.x, me->pos.y - n->pos.y, &rme);
	xy_to_radial (other->pos.x - n->pos.x, other->pos.y - n->pos.y, &rother);
	xy_to_radial (p->x - n->pos.x, p->y - n->pos.y, &rnew);

	if (state & GDK_CONTROL_MASK && rnew.a != HUGE_VAL) { 
		// restrict to 15 degree steps
		double opposite, plus90, minus90;

		// the two closest 15 degree steps
		r_down = floor(rnew.a/(M_PI/12))*M_PI/12;
		r_up = r_down + M_PI/12;

		// the 90 degree steps starting from origin angle
		opposite = angle_normalize (me->origin.a + M_PI); 
		plus90 = angle_normalize (me->origin.a + M_PI/2); 
		minus90 = angle_normalize (me->origin.a - M_PI/2); 

		// snap to closest
		rnew.a = closest_of_six (rnew.a, me->origin.a, opposite, plus90, minus90, r_down, r_up); 
	} 

	if (state & GDK_MOD1_MASK) { 
		// lock handle length
		rnew.r = me->origin.r;
	} 

	if ((state & GDK_SHIFT_MASK) && rme.a != HUGE_VAL && rnew.a != HUGE_VAL) { 
		// rotate the other handle correspondingly, if both old and new angles exist
		rother.a += rnew.a - rme.a;
		radial_to_xy (&rother, &(n->pos), &o);
		other->pos.x = o.x;
		other->pos.y = o.y;
		sp_ctrlline_set_coords (SP_CTRLLINE (other->line), n->pos.x, n->pos.y, other->pos.x, other->pos.y);
		sp_knot_set_position (other->knot, &(other->pos), 0);
	} 

	radial_to_xy (&rnew, &(n->pos), &o);
	me->pos.x = o.x;
	me->pos.y = o.y;
	sp_ctrlline_set_coords (SP_CTRLLINE (me->line), n->pos.x, n->pos.y, me->pos.x, me->pos.y);

	// this is what sp_knot_set_position does, but without emitting the signal:
	// we cannot emit a "moved" signal because we're now processing it
	if (me->knot->item) sp_ctrl_moveto (SP_CTRL (me->knot->item), me->pos.x, me->pos.y);

	sp_desktop_set_coordinate_status (knot->desktop, me->pos.x, me->pos.y, 0);

	update_object (n->subpath->nodepath);
}

static gboolean
node_ctrl_event (SPKnot * knot, GdkEvent * event, SPPathNode * n)
{
	gboolean ret = FALSE;
	switch (event->type) {
	case GDK_KEY_PRESS:
		switch (event->key.keyval) {
		case GDK_space:
			if (event->key.state & GDK_BUTTON1_MASK) {
				SPNodePath * nodepath;
				nodepath = n->subpath->nodepath;
				stamp_repr(nodepath);
				ret = TRUE;
			}
			break;
		default:
			break;
		}
		break;
	default:
		break;
	}

	return ret;
}

/*
 * Constructors and destructors
 */

static SPNodeSubPath *
sp_nodepath_subpath_new (SPNodePath * nodepath)
{
	SPNodeSubPath * s;

	g_assert (nodepath);
	g_assert (nodepath->desktop);

	s = g_new (SPNodeSubPath, 1);

	s->nodepath = nodepath;
	s->closed = FALSE;
	s->nodes = NULL;
	s->first = NULL;
	s->last = NULL;

	nodepath->subpaths = g_list_prepend (nodepath->subpaths, s);

	return s;
}

static void
sp_nodepath_subpath_destroy (SPNodeSubPath * subpath)
{
	g_assert (subpath);
	g_assert (subpath->nodepath);
	g_assert (g_list_find (subpath->nodepath->subpaths, subpath));

	while (subpath->nodes) {
		sp_nodepath_node_destroy ((SPPathNode *) subpath->nodes->data);
	}

	subpath->nodepath->subpaths = g_list_remove (subpath->nodepath->subpaths, subpath);

	g_free (subpath);
}

static void
sp_nodepath_subpath_close (SPNodeSubPath * sp)
{
	g_assert (!sp->closed);
	g_assert (sp->last != sp->first);
	g_assert (sp->first->code == ART_MOVETO);

	sp->closed = TRUE;

	sp->first->p.other = sp->last;
	sp->last->n.other = sp->first;
	sp->last->n.pos = sp->first->n.pos;

	sp->first = sp->last;

	sp_nodepath_node_destroy (sp->last->n.other);
}

static void
sp_nodepath_subpath_open (SPNodeSubPath * sp, SPPathNode * n)
{
	SPPathNode * new_path;

	g_assert (sp->closed);
	g_assert (n->subpath == sp);
	g_assert (sp->first == sp->last);

	/* We create new startpoint, current node will become last one */

	new_path = sp_nodepath_node_new (sp, n->n.other, SP_PATHNODE_CUSP, ART_MOVETO, &n->pos, &n->pos, &n->n.pos);

	sp->closed = FALSE;

	sp->first = new_path;
	sp->last = n;
	n->n.other = NULL;
	new_path->p.other = NULL;
}

SPPathNode *
sp_nodepath_node_new (SPNodeSubPath *sp, SPPathNode *next, SPPathNodeType type, ArtPathcode code,
		      NRPoint *ppos, NRPoint *pos, NRPoint *npos)
{
	SPPathNode * n, * prev;

	g_assert (sp);
	g_assert (sp->nodepath);
	g_assert (sp->nodepath->desktop);

	if (nodechunk == NULL) {
		nodechunk = g_mem_chunk_create (SPPathNode, 32, G_ALLOC_AND_FREE);
	}

	n = (SPPathNode*)g_mem_chunk_alloc (nodechunk);

	n->subpath = sp;
	n->type = type;
	n->code = code;
	n->selected = FALSE;
	n->pos = *pos;
	n->p.pos = *ppos;
	n->n.pos = *npos;

	if (next) {
		g_assert (g_list_find (sp->nodes, next));
		prev = next->p.other;
	} else {
		prev = sp->last;
	}

	if (prev) {
		prev->n.other = n;
	} else {
		sp->first = n;
	}

	if (next) {
		next->p.other = n;
	} else {
		sp->last = n;
	}

	n->p.other = prev;
	n->n.other = next;

	n->knot = sp_knot_new (sp->nodepath->desktop);
	sp_knot_set_position (n->knot, pos, 0);
	g_object_set (G_OBJECT (n->knot),
		      "anchor", GTK_ANCHOR_CENTER,
		      "fill", NODE_FILL,
		      "fill_mouseover", NODE_FILL_HI,
		      "stroke", NODE_STROKE,
		      "stroke_mouseover", NODE_STROKE_HI,
#if 0
		      "cursor_mouseover", CursorNodeMouseover,
		      "cursor_dragging", CursorNodeDragging,
#endif
		      NULL);
	if (n->type == SP_PATHNODE_CUSP) {
		g_object_set (G_OBJECT (n->knot), "shape", SP_KNOT_SHAPE_DIAMOND, "size", 9, NULL);
	} else {
		g_object_set (G_OBJECT (n->knot), "shape", SP_KNOT_SHAPE_SQUARE, "size", 7, NULL);
	}
	g_signal_connect (G_OBJECT (n->knot), "event", G_CALLBACK (node_event), n);
	g_signal_connect (G_OBJECT (n->knot), "clicked", G_CALLBACK (node_clicked), n);
	g_signal_connect (G_OBJECT (n->knot), "grabbed", G_CALLBACK (node_grabbed), n);
	g_signal_connect (G_OBJECT (n->knot), "ungrabbed", G_CALLBACK (node_ungrabbed), n);
	g_signal_connect (G_OBJECT (n->knot), "request", G_CALLBACK (node_request), n);
	sp_knot_show (n->knot);

	n->p.knot = sp_knot_new (sp->nodepath->desktop);
	sp_knot_set_position (n->p.knot, ppos, 0);
	g_object_set (G_OBJECT (n->p.knot),
		      "shape", SP_KNOT_SHAPE_CIRCLE,
		      "size", 7,
		      "anchor", GTK_ANCHOR_CENTER,
		      "fill", KNOT_FILL,
		      "fill_mouseover", KNOT_FILL_HI,
		      "stroke", KNOT_STROKE,
		      "stroke_mouseover", KNOT_STROKE_HI,
#if 0
		      "cursor_mouseover", CursorNodeMouseover,
		      "cursor_dragging", CursorNodeDragging,
#endif
		      NULL);
	g_signal_connect (G_OBJECT (n->p.knot), "clicked", G_CALLBACK (node_ctrl_clicked), n);
	g_signal_connect (G_OBJECT (n->p.knot), "grabbed", G_CALLBACK (node_ctrl_grabbed), n);
	g_signal_connect (G_OBJECT (n->p.knot), "ungrabbed", G_CALLBACK (node_ctrl_ungrabbed), n);
	g_signal_connect (G_OBJECT (n->p.knot), "request", G_CALLBACK (node_ctrl_request), n);
	g_signal_connect (G_OBJECT (n->p.knot), "moved", G_CALLBACK (node_ctrl_moved), n);
	g_signal_connect (G_OBJECT (n->p.knot), "event", G_CALLBACK (node_ctrl_event), n);
	
	sp_knot_hide (n->p.knot);
	n->p.line = sp_canvas_item_new (SP_DT_CONTROLS (n->subpath->nodepath->desktop),
					       SP_TYPE_CTRLLINE, NULL);
	sp_canvas_item_hide (n->p.line);

	n->n.knot = sp_knot_new (sp->nodepath->desktop);
	sp_knot_set_position (n->n.knot, npos, 0);
	g_object_set (G_OBJECT (n->n.knot),
		      "shape", SP_KNOT_SHAPE_CIRCLE,
		      "size", 7,
		      "anchor", GTK_ANCHOR_CENTER,
		      "fill", KNOT_FILL,
		      "fill_mouseover", KNOT_FILL_HI,
		      "stroke", KNOT_STROKE,
		      "stroke_mouseover", KNOT_STROKE_HI,
#if 0
		      "cursor_mouseover", CursorNodeMouseover,
		      "cursor_dragging", CursorNodeDragging,
#endif
		      NULL);
	g_signal_connect (G_OBJECT (n->n.knot), "clicked", G_CALLBACK (node_ctrl_clicked), n);
	g_signal_connect (G_OBJECT (n->n.knot), "grabbed", G_CALLBACK (node_ctrl_grabbed), n);
	g_signal_connect (G_OBJECT (n->n.knot), "ungrabbed", G_CALLBACK (node_ctrl_ungrabbed), n);
	g_signal_connect (G_OBJECT (n->n.knot), "request", G_CALLBACK (node_ctrl_request), n);
	g_signal_connect (G_OBJECT (n->n.knot), "moved", G_CALLBACK (node_ctrl_moved), n);
	g_signal_connect (G_OBJECT (n->n.knot), "event", G_CALLBACK (node_ctrl_event), n);
	sp_knot_hide (n->n.knot);
	n->n.line = sp_canvas_item_new (SP_DT_CONTROLS (n->subpath->nodepath->desktop),
					       SP_TYPE_CTRLLINE, NULL);
	sp_canvas_item_hide (n->n.line);

	sp->nodes = g_list_prepend (sp->nodes, n);

	return n;
}

static void
sp_nodepath_node_destroy (SPPathNode * node)
{
	SPNodeSubPath * sp;

	g_assert (node);
	g_assert (node->subpath);
	g_assert (SP_IS_KNOT (node->knot));
	g_assert (SP_IS_KNOT (node->p.knot));
	g_assert (SP_IS_KNOT (node->n.knot));
	g_assert (g_list_find (node->subpath->nodes, node));

	sp = node->subpath;

	if (node->selected) { // first, deselect
		g_assert (g_list_find (node->subpath->nodepath->selected, node));
		node->subpath->nodepath->selected = g_list_remove (node->subpath->nodepath->selected, node);
	}

	/*
	sp_knot_hide (node->knot);
	sp_knot_hide (node->p.knot);
	sp_knot_hide (node->n.knot);
	*/
	node->subpath->nodes = g_list_remove (node->subpath->nodes, node);

	g_object_unref (G_OBJECT (node->knot));
	g_object_unref (G_OBJECT (node->p.knot));
	g_object_unref (G_OBJECT (node->n.knot));

	gtk_object_destroy (GTK_OBJECT (node->p.line));
	gtk_object_destroy (GTK_OBJECT (node->n.line));
	
	if (sp->nodes) { // there are others nodes on the subpath
		if (sp->closed) {
			if (sp->first == node) {
				g_assert (sp->last == node);
				sp->first = node->n.other;
				sp->last = sp->first;
			}
			node->p.other->n.other = node->n.other;
			node->n.other->p.other = node->p.other;
		} else {
			if (sp->first == node) {
				sp->first = node->n.other;
				sp->first->code = ART_MOVETO;
			}
			if (sp->last == node) sp->last = node->p.other;
			if (node->p.other) node->p.other->n.other = node->n.other;
			if (node->n.other) node->n.other->p.other = node->p.other;
		}
	} else { // this was the last node on subpath
		sp->nodepath->subpaths = g_list_remove (sp->nodepath->subpaths, sp);
	}

	g_mem_chunk_free (nodechunk, node);
}

/*
 * Helpers
 */

static SPPathNodeSide *
sp_node_get_side (SPPathNode * node, gint which)
{
	g_assert (node);

	switch (which) {
	case -1:
		return &node->p;
	case 1:
		return &node->n;
	default:
		break;
	}

	g_assert_not_reached ();

	return NULL;
}

static SPPathNodeSide *
sp_node_opposite_side (SPPathNode * node, SPPathNodeSide * me)
{
	g_assert (node);

	if (me == &node->p) return &node->n;
	if (me == &node->n) return &node->p;

	g_assert_not_reached ();

	return NULL;
}

static ArtPathcode
sp_node_path_code_from_side (SPPathNode * node, SPPathNodeSide * me)
{
	g_assert (node);

	if (me == &node->p) {
		if (node->p.other) return (ArtPathcode)node->code;
		return ART_MOVETO;
	}

	if (me == &node->n) {
		if (node->n.other) return (ArtPathcode)node->n.other->code;
		return ART_MOVETO;
	}

	g_assert_not_reached ();

	return ART_END;
}

const gchar* 
sp_node_type_description (SPPathNode *n)
{
	switch (n->type) {
	case SP_PATHNODE_CUSP:
		return ("cusp");
	case SP_PATHNODE_SMOOTH:
		return ("smooth");
	case SP_PATHNODE_SYMM:
		return ("symmetric");
	}
	return NULL;
}

void
sp_nodepath_update_statusbar (SPNodePath *nodepath)
{
	gint total, selected;
	SPNodeSubPath * subpath;
	SPPathNode * node;
	GList * spl, * nl;
	SPSelection *sel;

	if (!nodepath) return;

	const gchar* when_selected = "Drag nodes or control points to edit the path";

	total = selected = 0;

	for (spl = nodepath->subpaths; spl != NULL; spl = spl->next) {
		subpath = (SPNodeSubPath *) spl->data;
		total += g_list_length (subpath->nodes);
	}

	selected = g_list_length (nodepath->selected);

	if (selected == 0) {
		sel = nodepath->desktop->selection;
		GSList *i = sel->items;
		if (g_slist_length (sel->items) == 0)
		sp_view_set_statusf (SP_VIEW(nodepath->desktop), "Select one path object with selector first, then switch back to node editor.");
		else 
		sp_view_set_statusf (SP_VIEW(nodepath->desktop), "No nodes selected. Click, Shift+click, drag around nodes to select.");
	} else if (selected == 1) {
		sp_view_set_statusf (SP_VIEW(nodepath->desktop), "%i of %i nodes selected; %s. %s.", selected, total, sp_node_type_description ((SPPathNode *) nodepath->selected->data), when_selected);
	} else {
		sp_view_set_statusf (SP_VIEW(nodepath->desktop), "%i of %i nodes selected. %s.", selected, total, when_selected);
	}
}
