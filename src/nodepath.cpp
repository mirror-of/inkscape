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
#include "display/sp-canvas-util.h"
#include "display/sp-ctrlline.h"
#include "display/sodipodi-ctrl.h"
#include "helper/sp-intl.h"
#include "knot.h"
#include "inkscape.h"
#include "document.h"
#include "desktop.h"
#include "desktop-handles.h"
#include "desktop-affine.h"
#include "snap.h"
#include "node-context.h"
#include "nodepath.h"
#include "selection-chemistry.h"
#include "selection.h"
#include "selection.h"
#include "xml/repr.h"
#include "xml/repr-private.h"
#include "object-edit.h"
#include "prefs-utils.h"

#include <libnr/nr-matrix-ops.h>
#include <libnr/nr-point-fns.h>

/* fixme: Implement these via preferences */

#define NODE_FILL          0xafafaf00
#define NODE_STROKE        0x000000ff
#define NODE_FILL_HI       0xff669900
#define NODE_STROKE_HI     0x000000ff
#define NODE_FILL_SEL      0x5020ffff
#define NODE_STROKE_SEL    0x000000ff
#define NODE_FILL_SEL_HI   0xff669900
#define NODE_STROKE_SEL_HI 0x000000ff
#define KNOT_FILL          0x00000000
#define KNOT_STROKE        0x000000ff
#define KNOT_FILL_HI       0xff669900
#define KNOT_STROKE_HI     0x000000ff

static GMemChunk *nodechunk = NULL;

/* Creation from object */

static NArtBpath *subpath_from_bpath(Path::Path *np, NArtBpath *b, gchar const *t);
static gchar *parse_nodetypes(gchar const *types, gint length);

/* Object updating */

static void stamp_repr(Path::Path *np);
static SPCurve *create_curve(Path::Path *np);
static gchar *create_typestr(Path::Path *np);

static void sp_node_ensure_ctrls(Path::Node *node);

static void sp_nodepath_node_select(Path::Node *node, gboolean incremental, gboolean override);

static void sp_node_set_selected(Path::Node *node, gboolean selected);

/* Control knot placement, if node or other knot is moved */

static void sp_node_adjust_knot(Path::Node *node, gint which_adjust);
static void sp_node_adjust_knots(Path::Node *node);

/* Knot event handlers */

static void node_clicked(SPKnot *knot, guint state, gpointer data);
static void node_grabbed(SPKnot *knot, guint state, gpointer data);
static void node_ungrabbed(SPKnot *knot, guint state, gpointer data);
static gboolean node_request(SPKnot *knot, NR::Point *p, guint state, gpointer data);
static void node_ctrl_clicked(SPKnot *knot, guint state, gpointer data);
static void node_ctrl_grabbed(SPKnot *knot, guint state, gpointer data);
static void node_ctrl_ungrabbed(SPKnot *knot, guint state, gpointer data);
static gboolean node_ctrl_request(SPKnot *knot, NR::Point *p, guint state, gpointer data);
static void node_ctrl_moved(SPKnot *knot, NR::Point *p, guint state, gpointer data);

/* Constructors and destructors */

static Path::SubPath *sp_nodepath_subpath_new(Path::Path *nodepath);
static void sp_nodepath_subpath_destroy(Path::SubPath *subpath);
static void sp_nodepath_subpath_close(Path::SubPath *sp);
static void sp_nodepath_subpath_open(Path::SubPath *sp, Path::Node *n);
static Path::Node * sp_nodepath_node_new(Path::SubPath *sp, Path::Node *next, Path::NodeType type, NRPathcode code,
					 NR::Point *ppos, NR::Point *pos, NR::Point *npos);
static void sp_nodepath_node_destroy(Path::Node *node);

/* Helpers */

static Path::NodeSide *sp_node_get_side(Path::Node *node, gint which);
static Path::NodeSide *sp_node_opposite_side(Path::Node *node, Path::NodeSide *me);
static NRPathcode sp_node_path_code_from_side(Path::Node *node, Path::NodeSide *me);

// active_node indicates mouseover node
static Path::Node *active_node = NULL;

/**
\brief Creates new nodepath from item 
*/
Path::Path *sp_nodepath_new(SPDesktop *desktop, SPItem *item)
{
	SPRepr *repr = SP_OBJECT (item)->repr;

	if (!SP_IS_PATH (item))
        return NULL;
	SPPath *path = SP_PATH(item);
	SPCurve *curve = sp_shape_get_curve(SP_SHAPE(path));
	if (curve == NULL)
		return NULL;

	NArtBpath *bpath = sp_curve_first_bpath (curve);
	gint length = curve->end;
	if (length == 0)
        return NULL; // prevent crash for one-node paths

	const gchar *nodetypes = sp_repr_attr(repr, "sodipodi:nodetypes");
	gchar *typestr = parse_nodetypes(nodetypes, length);

	//Create new nodepath
	Path::Path *np = g_new(Path::Path, 1);
	if (!np)
		return NULL;

	// Set defaults
	np->desktop     = desktop;
	np->path        = path;
	np->subpaths    = NULL;
	np->selected    = NULL;
	np->nodeContext = NULL; //Let the context that makes this set it

	// we need to update item's transform from the repr here,
	// because they may be out of sync when we respond 
	// to a change in repr by regenerating nodepath     --bb
	sp_object_read_attr (SP_OBJECT (item), "transform");

	np->i2d  = sp_item_i2d_affine(SP_ITEM(path));
	np->d2i  = np->i2d.inverse();
	np->repr = repr;

	/* Now the bitchy part (lauris) */

	NArtBpath *b = bpath;

	while (b->code != NR_END) {
		b = subpath_from_bpath (np, b, typestr + (b - bpath));
	}

	g_free (typestr);
	sp_curve_unref (curve);

	return np;
}

void sp_nodepath_destroy(Path::Path *np) {

	if (!np)  //soft fail, like delete
		return;

	while (np->subpaths) {
		sp_nodepath_subpath_destroy ((Path::SubPath *) np->subpaths->data);
	}

	//Inform the context that made me, if any, that I am gone.
	if (np->nodeContext)
		np->nodeContext->nodepath = NULL;

	g_assert (!np->selected);

	np->desktop = NULL;

	g_free (np);
}


/**
 *  Return the node count of a given NodeSubPath
 */
gint sp_nodepath_subpath_get_node_count(Path::SubPath *subpath)
{
    if (!subpath)
        return 0;
    gint nodeCount = g_list_length(subpath->nodes);
    return nodeCount;
}

/**
 *  Return the node count of a given NodePath
 */
gint sp_nodepath_get_node_count(Path::Path *np)
{
    if (!np)
        return 0;
    gint nodeCount = 0;
    for (GList *item = np->subpaths ; item ; item=item->next) {
        Path::SubPath *subpath = (Path::SubPath *)item->data;
        nodeCount += g_list_length(subpath->nodes);
    }
    return nodeCount;
}


/**
 * Clean up a nodepath after editing.
 * Currently we are deleting trivial subpaths
 */
void
sp_nodepath_cleanup(Path::Path *nodepath)
{
	GList *badSubPaths = NULL;

	//Check all subpaths to be >=2 nodes
	for (GList *l = nodepath->subpaths; l ; l=l->next) {
		Path::SubPath *sp = (Path::SubPath *)l->data;
		if (sp_nodepath_subpath_get_node_count(sp)<2)
			badSubPaths = g_list_append(badSubPaths, sp);
	}

	//Delete them.  This second step is because sp_nodepath_subpath_destroy()
	//also removes the subpath from nodepath->subpaths
	for (GList *l = badSubPaths; l ; l=l->next) {
		Path::SubPath *sp = (Path::SubPath *)l->data;
		sp_nodepath_subpath_destroy(sp);
	}

	g_list_free(badSubPaths);

}



/**
\brief Returns true if the argument nodepath and the d attribute in its repr do not match. 
 This may happen if repr was changed in e.g. XML editor or by undo. 
 UGLY HACK, think how we can eliminate it.
*/
gboolean nodepath_repr_d_changed(Path::Path *np, char const *newd)
{
	g_assert (np);

	SPCurve *curve = create_curve(np);

	gchar *svgpath = sp_svg_write_path(curve->bpath);

	char const *attr_d = ( newd
			       ? newd
			       : sp_repr_attr(SP_OBJECT(np->path)->repr, "d") );

	gboolean ret;
	if (attr_d && svgpath)
		ret = strcmp(attr_d, svgpath);
	else 
		ret = TRUE;

	g_free (svgpath);
	sp_curve_unref (curve);

	return ret;
}

/**
\brief Returns true if the argument nodepath and the sodipodi:nodetypes attribute in its repr do not match. 
 This may happen if repr was changed in e.g. XML editor or by undo.
*/
gboolean nodepath_repr_typestr_changed(Path::Path *np, char const *newtypestr)
{
	g_assert (np);
	gchar *typestr = create_typestr(np);
	char const *attr_typestr = ( newtypestr
				     ? newtypestr
				     : sp_repr_attr(SP_OBJECT(np->path)->repr, "sodipodi:nodetypes") );
	gboolean const ret = (attr_typestr && strcmp(attr_typestr, typestr));

	g_free (typestr);

	return ret;
}

static NArtBpath *subpath_from_bpath(Path::Path *np, NArtBpath *b, gchar const *t)
// XXX: Fixme: t should be a proper type, rather than gchar
{
	NR::Point ppos, pos, npos;

	g_assert ((b->code == NR_MOVETO) || (b->code == NR_MOVETO_OPEN));

	Path::SubPath *sp = sp_nodepath_subpath_new (np);
	bool const closed = (b->code == NR_MOVETO);

	pos = NR::Point(b->x3, b->y3) * np->i2d;
	if (b[1].code == NR_CURVETO) {
		npos = NR::Point(b[1].x1, b[1].y1) * np->i2d;
	} else {
		npos = pos;
	}
	Path::Node *n;
	n = sp_nodepath_node_new(sp, NULL, (Path::NodeType) *t, NR_MOVETO, &pos, &pos, &npos);
	g_assert (sp->first == n);
	g_assert (sp->last  == n);

	b++;
	t++;
	while ((b->code == NR_CURVETO) || (b->code == NR_LINETO)) {
		pos = NR::Point(b->x3, b->y3) * np->i2d;
		if (b->code == NR_CURVETO) {
			ppos = NR::Point(b->x2, b->y2) * np->i2d;
		} else {
			ppos = pos;
		}
		if (b[1].code == NR_CURVETO) {
			npos = NR::Point(b[1].x1, b[1].y1) * np->i2d;
		} else {
			npos = pos;
		}
		n = sp_nodepath_node_new (sp, NULL, (Path::NodeType)*t, b->code, &ppos, &pos, &npos);
		b++;
		t++;
	}

	if (closed) sp_nodepath_subpath_close (sp);

	return b;
}

static gchar *parse_nodetypes(gchar const *types, gint length)
{
	g_assert (length > 0);

	gchar *typestr = g_new(gchar, length + 1);

	gint pos = 0;

	if (types) {
		for (gint i = 0; types[i] && ( i < length ); i++) {
			while ((types[i] > '\0') && (types[i] <= ' ')) i++;
			if (types[i] != '\0') {
				switch (types[i]) {
				case 's':
					typestr[pos++] = Path::NODE_SMOOTH;
					break;
				case 'z':
					typestr[pos++] = Path::NODE_SYMM;
					break;
				case 'c':
				default:
					typestr[pos++] = Path::NODE_CUSP;
					break;
				}
			}
		}
	}

	while (pos < length) typestr[pos++] = Path::NODE_CUSP;

	return typestr;
}

static void update_object(Path::Path *np)
{
	g_assert(np);

	SPCurve *curve = create_curve(np);

	sp_shape_set_curve(SP_SHAPE(np->path), curve, TRUE);

	sp_curve_unref(curve);
}

static void update_repr_internal(Path::Path *np)
{
	SPRepr *repr = SP_OBJECT(np->path)->repr;

	g_assert(np);

	SPCurve *curve = create_curve(np);
	gchar *typestr = create_typestr(np);
	gchar *svgpath = sp_svg_write_path(curve->bpath);

	sp_repr_set_attr (repr, "d", svgpath);
	sp_repr_set_attr (repr, "sodipodi:nodetypes", typestr);

	g_free (svgpath);
	g_free (typestr);
	sp_curve_unref (curve);
}

static void update_repr(Path::Path *np)
{
	update_repr_internal(np);
	sp_document_done(SP_DT_DOCUMENT(np->desktop));
}

static void update_repr_keyed(Path::Path *np, const gchar *key)
{
	update_repr_internal(np);
	sp_document_maybe_done(SP_DT_DOCUMENT(np->desktop), key);
}


static void stamp_repr(Path::Path *np)
{
	g_assert (np);

	SPRepr *old_repr = SP_OBJECT(np->path)->repr;
	SPRepr *new_repr = sp_repr_duplicate(old_repr);

	// remember the position of the item
	gint pos = sp_repr_position (old_repr);
	// remember parent
	SPRepr *parent = sp_repr_parent (old_repr);
	
	SPCurve *curve = create_curve(np);
	gchar *typestr = create_typestr(np);

	gchar *svgpath = sp_svg_write_path (curve->bpath);

	sp_repr_set_attr (new_repr, "d", svgpath);
	sp_repr_set_attr (new_repr, "sodipodi:nodetypes", typestr);

	// add the new repr to the parent
	sp_repr_append_child (parent, new_repr);
	// move to the saved position 
	sp_repr_set_position_absolute (new_repr, pos > 0 ? pos : 0);

	sp_document_done (SP_DT_DOCUMENT (np->desktop));

	sp_repr_unref (new_repr);
	g_free (svgpath);
	g_free (typestr);
	sp_curve_unref (curve);
}

static SPCurve *create_curve(Path::Path *np)
{
	SPCurve *curve = sp_curve_new ();

	for (GList *spl = np->subpaths; spl != NULL; spl = spl->next) {
		Path::SubPath *sp = (Path::SubPath *) spl->data;
		sp_curve_moveto(curve,
				sp->first->pos * np->d2i);
		Path::Node *n = sp->first->n.other;
		while (n) {
			NR::Point const end_pt = n->pos * np->d2i;
			switch (n->code) {
			case NR_LINETO:
				sp_curve_lineto(curve, end_pt);
				break;
			case NR_CURVETO:
				sp_curve_curveto(curve,
						 n->p.other->n.pos * np->d2i,
						 n->p.pos * np->d2i,
						 end_pt);
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

static gchar *create_typestr(Path::Path *np)
{
	gchar *typestr = g_new(gchar, 32);
	gint len = 32;
	gint pos = 0;

	for (GList *spl = np->subpaths; spl != NULL; spl = spl->next) {
		Path::SubPath *sp = (Path::SubPath *) spl->data;

		if (pos >= len) {
			typestr = g_renew (gchar, typestr, len + 32);
			len += 32;
		}

		typestr[pos++] = 'c';

		Path::Node *n;
		n = sp->first->n.other;
		while (n) {
			gchar code;

			switch (n->type) {
			case Path::NODE_CUSP:
				code = 'c';
				break;
			case Path::NODE_SMOOTH:
				code = 's';
				break;
			case Path::NODE_SYMM:
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

static Path::Path *sp_nodepath_current()
{
	if (!SP_ACTIVE_DESKTOP) return NULL;

	SPEventContext *event_context = (SP_ACTIVE_DESKTOP)->event_context;

	if (!SP_IS_NODE_CONTEXT (event_context)) return NULL;

	return SP_NODE_CONTEXT (event_context)->nodepath;
}




/**
 \brief Fills node and control positions for three nodes, splitting line
  marked by end at distance t
 */
static void sp_nodepath_line_midpoint(Path::Node *new_path, Path::Node *end, gdouble t)
{
	g_assert (new_path != NULL);
	g_assert (end      != NULL);

	g_assert (end->p.other == new_path);
	Path::Node *start = new_path->p.other;
	g_assert (start);

	if (end->code == NR_LINETO) {
		new_path->type = Path::NODE_CUSP;
		new_path->code = NR_LINETO;
		new_path->pos  = (t * start->pos + (1 - t) * end->pos);
	} else {
		new_path->type = Path::NODE_SMOOTH;
		new_path->code = NR_CURVETO;
		gdouble s      = 1 - t;
		for(int dim = 0; dim < 2; dim++) {
			const NR::Coord f000 = start->pos[dim];
			const NR::Coord f001 = start->n.pos[dim];
			const NR::Coord f011 = end->p.pos[dim];
			const NR::Coord f111 = end->pos[dim];
			const NR::Coord f00t = s * f000 + t * f001;
			const NR::Coord f01t = s * f001 + t * f011;
			const NR::Coord f11t = s * f011 + t * f111;
			const NR::Coord f0tt = s * f00t + t * f01t;
			const NR::Coord f1tt = s * f01t + t * f11t;
			const NR::Coord fttt = s * f0tt + t * f1tt;
			start->n.pos[dim]    = f00t;
			new_path->p.pos[dim] = f0tt;
			new_path->pos[dim]   = fttt;
			new_path->n.pos[dim] = f1tt;
			end->p.pos[dim]      = f11t;
		}
	}
}

static Path::Node *sp_nodepath_line_add_node(Path::Node *end, gdouble t)
{
	g_assert (end);
	g_assert (end->subpath);
	g_assert (g_list_find (end->subpath->nodes, end));

	Path::Node *start = end->p.other;
	g_assert( start->n.other == end );
	Path::Node *newnode = sp_nodepath_node_new(end->subpath,
						   end,
						   Path::NODE_SMOOTH,
						   (NRPathcode)end->code,
						   &start->pos, &start->pos, &start->n.pos);
	sp_nodepath_line_midpoint(newnode, end, t);

	sp_node_ensure_ctrls (start);
	sp_node_ensure_ctrls (newnode);
	sp_node_ensure_ctrls (end);

	return newnode;
}

/**
\brief Break the path at the node: duplicate the argument node, start a new subpath with the duplicate, and copy all nodes after the argument node to it
*/
static Path::Node *sp_nodepath_node_break(Path::Node *node)
{
	g_assert (node);
	g_assert (node->subpath);
	g_assert (g_list_find (node->subpath->nodes, node));

	Path::SubPath *sp = node->subpath;
	Path::Path *np    = sp->nodepath;

	if (sp->closed) {
		sp_nodepath_subpath_open (sp, node);
		return sp->first;
	} else {
		// no break for end nodes
		if (node == sp->first) return NULL;
		if (node == sp->last ) return NULL;

		// create a new subpath
		Path::SubPath *newsubpath = sp_nodepath_subpath_new(np);

		// duplicate the break node as start of the new subpath
		Path::Node *newnode = sp_nodepath_node_new (newsubpath, NULL, (Path::NodeType)node->type, NR_MOVETO, &node->pos, &node->pos, &node->n.pos);

		while (node->n.other) { // copy the remaining nodes into the new subpath
			Path::Node *n  = node->n.other;
			Path::Node *nn = sp_nodepath_node_new (newsubpath, NULL, (Path::NodeType)n->type, (NRPathcode)n->code, &n->p.pos, &n->pos, &n->n.pos);
			if (n->selected) {
				sp_nodepath_node_select (nn, TRUE, TRUE); //preserve selection
			}
			sp_nodepath_node_destroy (n); // remove the point on the original subpath
		}

		return newnode;
	}
}

static Path::Node *sp_nodepath_node_duplicate(Path::Node *node)
{
	g_assert (node);
	g_assert (node->subpath);
	g_assert (g_list_find (node->subpath->nodes, node));

	Path::SubPath *sp = node->subpath;

	NRPathcode code = (NRPathcode) node->code;
	if (code == NR_MOVETO) { // if node is the endnode,
		node->code = NR_LINETO; // new one is inserted before it, so change that to line
	}

	Path::Node *newnode = sp_nodepath_node_new(sp, node, (Path::NodeType)node->type, code,
						   &node->p.pos, &node->pos, &node->n.pos);

	return newnode;
}

static void sp_node_control_mirror_n_to_p(Path::Node *node)
{
	node->p.pos = (node->pos + (node->pos - node->n.pos));
}

static void sp_node_control_mirror_p_to_n(Path::Node *node)
{
	node->n.pos = (node->pos + (node->pos - node->p.pos));
}


static void sp_nodepath_set_line_type(Path::Node *end, NRPathcode code)
{
	g_assert (end);
	g_assert (end->subpath);
	g_assert (end->p.other);

	if (end->code == static_cast< guint > ( code ) )
		return;

	Path::Node *start = end->p.other;

	end->code = code;

	if (code == NR_LINETO) {
		if (start->code == NR_LINETO) start->type = Path::NODE_CUSP;
		if (end->n.other) {
			if (end->n.other->code == NR_LINETO) end->type = Path::NODE_CUSP;
		}
		sp_node_adjust_knot (start, -1);
		sp_node_adjust_knot (end, 1);
	} else {
		NR::Point delta = end->pos - start->pos;
		start->n.pos = start->pos + delta / 3;
		end->p.pos = end->pos - delta / 3;
		sp_node_adjust_knot (start, 1);
		sp_node_adjust_knot (end, -1);
	}

	sp_node_ensure_ctrls (start);
	sp_node_ensure_ctrls (end);
}

static Path::Node *sp_nodepath_set_node_type(Path::Node *node, Path::NodeType type)
{
	g_assert (node);
	g_assert (node->subpath);

	if (type == static_cast< Path::NodeType> (static_cast< guint > (node->type) ) )
		return node;

	if ((node->p.other != NULL) && (node->n.other != NULL)) {
		if ((node->code == NR_LINETO) && (node->n.other->code == NR_LINETO)) {
			type = Path::NODE_CUSP;
		}
	}

	node->type = type;

	if (node->type == Path::NODE_CUSP) {
		g_object_set (G_OBJECT (node->knot), "shape", SP_KNOT_SHAPE_DIAMOND, "size", 9, NULL);
	} else {
		g_object_set (G_OBJECT (node->knot), "shape", SP_KNOT_SHAPE_SQUARE, "size", 7, NULL);
	}

	sp_node_adjust_knots (node);

	sp_nodepath_update_statusbar (node->subpath->nodepath);

	return node;
}

static void sp_node_moveto(Path::Node *node, NR::Point p)
{
	NR::Point delta = p - node->pos;
	node->pos = p;

	node->p.pos += delta;
	node->n.pos += delta;

	if (node->p.other) {
		if (node->code == NR_LINETO) {
			sp_node_adjust_knot (node, 1);
			sp_node_adjust_knot (node->p.other, -1);
		}
	}
	if (node->n.other) {
		if (node->n.other->code == NR_LINETO) {
			sp_node_adjust_knot (node, -1);
			sp_node_adjust_knot (node->n.other, 1);
		}
	}

	sp_node_ensure_ctrls (node);
}

static void sp_nodepath_selected_nodes_move(Path::Path *nodepath, NR::Coord dx, NR::Coord dy,
					    bool snap = true)
{
	NR::Coord best[2] = { NR_HUGE, NR_HUGE };
	NR::Point delta(dx, dy);
	NR::Point best_pt = delta;

	if (snap) {
		for (GList *l = nodepath->selected; l != NULL; l = l->next) {
			Path::Node *n = (Path::Node *) l->data;
			NR::Point p = n->pos + delta;
			for(int dim = 0; dim < 2; dim++) {
				NR::Coord dist = namedview_dim_snap (nodepath->desktop->namedview,
								     Snapper::SNAP_POINT, p,
								     NR::Dim2(dim));
				if (dist < best[dim]) {
					best[dim] = dist;
					best_pt[dim] = p[dim] - n->pos[dim];
				}
			}
		}
	}

	for (GList *l = nodepath->selected; l != NULL; l = l->next) {
		Path::Node *n = (Path::Node *) l->data;
		sp_node_moveto (n, n->pos + best_pt);
	}

	update_object (nodepath);
}

void
sp_node_selected_move (gdouble dx, gdouble dy)
{
	Path::Path *nodepath = sp_nodepath_current ();
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
	// borrowed from sp_selection_move_screen in selection-chemistry.c
	// we find out the current zoom factor and divide deltas by it
	SPDesktop *desktop = SP_ACTIVE_DESKTOP;
	g_return_if_fail(SP_IS_DESKTOP (desktop));

	gdouble zoom = SP_DESKTOP_ZOOM (desktop);
	gdouble zdx = dx / zoom;
	gdouble zdy = dy / zoom;

	Path::Path *nodepath = sp_nodepath_current ();
	if (!nodepath) return;

	sp_nodepath_selected_nodes_move (nodepath, zdx, zdy);

	if (dx == 0) {
		update_repr_keyed (nodepath, "node:move:vertical");
	} else if (dy == 0) {
		update_repr_keyed (nodepath, "node:move:horizontal");
	} else 
		update_repr (nodepath);
}

static void sp_node_ensure_knot(Path::Node *node, gint which, gboolean show_knot)
{
	g_assert (node != NULL);

	Path::NodeSide *side = sp_node_get_side (node, which);
	NRPathcode code = sp_node_path_code_from_side (node, side);

	show_knot = show_knot && (code == NR_CURVETO);

	if (show_knot) {
		if (!SP_KNOT_IS_VISIBLE (side->knot)) {
			sp_knot_show (side->knot);
		}

		sp_knot_set_position (side->knot, &side->pos, 0);
		sp_canvas_item_show (side->line);

	} else {
		if (SP_KNOT_IS_VISIBLE (side->knot)) {
			sp_knot_hide (side->knot);
		}
		sp_canvas_item_hide (side->line);
	}
}

void sp_node_ensure_ctrls(Path::Node *node)
{
	g_assert (node != NULL);

	if (!SP_KNOT_IS_VISIBLE (node->knot)) {
		sp_knot_show (node->knot);
	}

	sp_knot_set_position (node->knot, &node->pos, 0);

	gboolean show_knots = node->selected;
	if (node->p.other != NULL) {
		if (node->p.other->selected) show_knots = TRUE;
	}
	if (node->n.other != NULL) {
		if (node->n.other->selected) show_knots = TRUE;
	}

	sp_node_ensure_knot (node, -1, show_knots);
	sp_node_ensure_knot (node, 1, show_knots);
}

static void sp_nodepath_subpath_ensure_ctrls(Path::SubPath *subpath)
{
	g_assert (subpath != NULL);

	for (GList *l = subpath->nodes; l != NULL; l = l->next) {
		sp_node_ensure_ctrls ((Path::Node *) l->data);
	}
}

static void sp_nodepath_ensure_ctrls(Path::Path *nodepath)
{
	g_assert (nodepath != NULL);

	for (GList *l = nodepath->subpaths; l != NULL; l = l->next)
		sp_nodepath_subpath_ensure_ctrls ((Path::SubPath *) l->data);
}

void
sp_node_selected_add_node (void)
{
	Path::Path *nodepath = sp_nodepath_current ();
	if (!nodepath) return;

	GList *nl = NULL;

	for (GList *l = nodepath->selected; l != NULL; l = l->next) {
		Path::Node *t = (Path::Node *) l->data;
		g_assert (t->selected);
		if (t->p.other && t->p.other->selected) nl = g_list_prepend (nl, t);
	}

	while (nl) {
		Path::Node *t = (Path::Node *) nl->data;
		Path::Node *n = sp_nodepath_line_add_node(t, 0.5);
		sp_nodepath_node_select (n, TRUE, FALSE);
		nl = g_list_remove (nl, t);
	}

	/* fixme: adjust ? */
	sp_nodepath_ensure_ctrls (nodepath);

	update_repr (nodepath);

	sp_nodepath_update_statusbar (nodepath);
}




void sp_node_selected_break()
{
	Path::Path *nodepath = sp_nodepath_current ();
	if (!nodepath) return;

	GList *temp = NULL;
	for (GList *l = nodepath->selected; l != NULL; l = l->next) {
		Path::Node *n = (Path::Node *) l->data;
		Path::Node *nn = sp_nodepath_node_break(n);
		if (nn == NULL) continue; // no break, no new node 
		temp = g_list_prepend (temp, nn);
	}

	if (temp) sp_nodepath_deselect (nodepath);
	for (GList *l = temp; l != NULL; l = l->next) {
		sp_nodepath_node_select ((Path::Node *) l->data, TRUE, TRUE);
	}

	sp_nodepath_ensure_ctrls (nodepath);

	update_repr (nodepath);
}




/**
\brief duplicate selected nodes
*/
void sp_node_selected_duplicate()
{
	Path::Path *nodepath = sp_nodepath_current ();
	if (!nodepath) return;

	GList *temp = NULL;
	for (GList *l = nodepath->selected; l != NULL; l = l->next) {
		Path::Node *n = (Path::Node *) l->data;
		Path::Node *nn = sp_nodepath_node_duplicate(n);
		if (nn == NULL) continue; // could not duplicate
		temp = g_list_prepend(temp, nn);
	}

	if (temp) sp_nodepath_deselect (nodepath);
	for (GList *l = temp; l != NULL; l = l->next) {
		sp_nodepath_node_select ((Path::Node *) l->data, TRUE, TRUE);
	}

	sp_nodepath_ensure_ctrls (nodepath);

	update_repr (nodepath);
}



void sp_node_selected_join()
{
	Path::Path *nodepath = sp_nodepath_current ();
	if (!nodepath) return; // there's no nodepath when editing rects, stars, spirals or ellipses

	if (g_list_length (nodepath->selected) != 2) {
	    nodepath->desktop->messageStack()->flash(Inkscape::ERROR_MESSAGE, _("To join, you must have two endnodes selected."));
	    return;
	}

	Path::Node *a = (Path::Node *) nodepath->selected->data;
	Path::Node *b = (Path::Node *) nodepath->selected->next->data;

	g_assert (a != b);
	g_assert (a->p.other || a->n.other);
	g_assert (b->p.other || b->n.other);

	if (((a->subpath->closed) || (b->subpath->closed)) || (a->p.other && a->n.other) || (b->p.other && b->n.other)) {
	    nodepath->desktop->messageStack()->flash(Inkscape::ERROR_MESSAGE, _("To join, you must have two endnodes selected."));
	    return;
	}

	/* a and b are endpoints */

	NR::Point c = (a->pos + b->pos) / 2;

	if (a->subpath == b->subpath) {
		Path::SubPath *sp = a->subpath;
		sp_nodepath_subpath_close (sp);

		sp_nodepath_ensure_ctrls (sp->nodepath);

		update_repr (nodepath);

		return;
	}

	/* a and b are separate subpaths */
	Path::SubPath *sa = a->subpath;
	Path::SubPath *sb = b->subpath;
	NR::Point p;
	Path::Node *n;
	NRPathcode code;
	if (a == sa->first) {
		p = sa->first->n.pos;
		code = (NRPathcode)sa->first->n.other->code;
		Path::SubPath *t = sp_nodepath_subpath_new (sa->nodepath);
		n = sa->last;
		sp_nodepath_node_new (t, NULL, Path::NODE_CUSP, NR_MOVETO, &n->n.pos, &n->pos, &n->p.pos);
		n = n->p.other;
		while (n) {
			sp_nodepath_node_new (t, NULL, (Path::NodeType)n->type, (NRPathcode)n->n.other->code, &n->n.pos, &n->pos, &n->p.pos);
			n = n->p.other;
			if (n == sa->first) n = NULL;
		}
		sp_nodepath_subpath_destroy (sa);
		sa = t;
	} else if (a == sa->last) {
		p = sa->last->p.pos;
		code = (NRPathcode)sa->last->code;
		sp_nodepath_node_destroy (sa->last);
	} else {
		code = NR_END;
		g_assert_not_reached ();
	}

	if (b == sb->first) {
		sp_nodepath_node_new (sa, NULL, Path::NODE_CUSP, code, &p, &c, &sb->first->n.pos);
		for (n = sb->first->n.other; n != NULL; n = n->n.other) {
			sp_nodepath_node_new (sa, NULL, (Path::NodeType)n->type, (NRPathcode)n->code, &n->p.pos, &n->pos, &n->n.pos);
		}
	} else if (b == sb->last) {
		sp_nodepath_node_new (sa, NULL, Path::NODE_CUSP, code, &p, &c, &sb->last->p.pos);
		for (n = sb->last->p.other; n != NULL; n = n->p.other) {
			sp_nodepath_node_new (sa, NULL, (Path::NodeType)n->type, (NRPathcode)n->n.other->code, &n->n.pos, &n->pos, &n->p.pos);
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



void sp_node_selected_join_segment()
{
	Path::Path *nodepath = sp_nodepath_current ();
	if (!nodepath) return; // there's no nodepath when editing rects, stars, spirals or ellipses

	if (g_list_length (nodepath->selected) != 2) {
	    nodepath->desktop->messageStack()->flash(Inkscape::ERROR_MESSAGE, _("To join, you must have two endnodes selected."));
	    return;
	}

	Path::Node *a = (Path::Node *) nodepath->selected->data;
	Path::Node *b = (Path::Node *) nodepath->selected->next->data;

	g_assert (a != b);
	g_assert (a->p.other || a->n.other);
	g_assert (b->p.other || b->n.other);

	if (((a->subpath->closed) || (b->subpath->closed)) || (a->p.other && a->n.other) || (b->p.other && b->n.other)) {
	    nodepath->desktop->messageStack()->flash(Inkscape::ERROR_MESSAGE, _("To join, you must have two endnodes selected."));
	    return;
	}

	if (a->subpath == b->subpath) {
		Path::SubPath *sp = a->subpath;

		/*similar to sp_nodepath_subpath_close (sp), without the node destruction*/
		sp->closed = TRUE;

		sp->first->p.other = sp->last;
		sp->last->n.other  = sp->first;
		
		sp_node_control_mirror_p_to_n (sp->last);
		sp_node_control_mirror_n_to_p (sp->first);

		sp->first->code = sp->last->code;
		sp->first       = sp->last;

		sp_nodepath_ensure_ctrls (sp->nodepath);

		update_repr (nodepath);

		return;
	}

	/* a and b are separate subpaths */
	Path::SubPath *sa = a->subpath;
	Path::SubPath *sb = b->subpath;

	Path::Node *n;
	NR::Point p;
	NRPathcode code;
	if (a == sa->first) {
		code = (NRPathcode) sa->first->n.other->code;
		Path::SubPath *t = sp_nodepath_subpath_new (sa->nodepath);
		n = sa->last;
		sp_nodepath_node_new (t, NULL, Path::NODE_CUSP, NR_MOVETO, &n->n.pos, &n->pos, &n->p.pos);
		for (n = n->p.other; n != NULL; n = n->p.other) {
			sp_nodepath_node_new (t, NULL, (Path::NodeType)n->type, (NRPathcode)n->n.other->code, &n->n.pos, &n->pos, &n->p.pos);
		}
		sp_nodepath_subpath_destroy (sa);
		sa = t;
	} else if (a == sa->last) {
		code = (NRPathcode)sa->last->code;
	} else {
		code = NR_END;
		g_assert_not_reached ();
	}

	if (b == sb->first) {
		n = sb->first;
		sp_node_control_mirror_p_to_n (sa->last);
		sp_nodepath_node_new (sa, NULL, Path::NODE_CUSP, code, &n->p.pos, &n->pos, &n->n.pos);
		sp_node_control_mirror_n_to_p (sa->last);
		for (n = n->n.other; n != NULL; n = n->n.other) {
			sp_nodepath_node_new (sa, NULL, (Path::NodeType)n->type, (NRPathcode)n->code, &n->p.pos, &n->pos, &n->n.pos);
		}
	} else if (b == sb->last) {
		n = sb->last;
		sp_node_control_mirror_p_to_n (sa->last);
		sp_nodepath_node_new (sa, NULL, Path::NODE_CUSP, code, &p, &n->pos, &n->p.pos);
		sp_node_control_mirror_n_to_p (sa->last);
		for (n = n->p.other; n != NULL; n = n->p.other) {
 			sp_nodepath_node_new (sa, NULL, (Path::NodeType)n->type, (NRPathcode)n->n.other->code, &n->n.pos, &n->pos, &n->p.pos);
		}
	} else {
		g_assert_not_reached ();
	}
	/* and now destroy sb */

	sp_nodepath_subpath_destroy (sb);

	sp_nodepath_ensure_ctrls (sa->nodepath);

	update_repr (nodepath);
}

void sp_node_selected_delete()
{
	Path::Path *nodepath = sp_nodepath_current();
	if (!nodepath) return;
	if (!nodepath->selected) return;

	/* fixme: do it the right way */
	while (nodepath->selected) {
		Path::Node *node = (Path::Node *) nodepath->selected->data;
		sp_nodepath_node_destroy (node);
	}


	//clean up the nodepath (such as for trivial subpaths)
	sp_nodepath_cleanup(nodepath);

	sp_nodepath_ensure_ctrls (nodepath);

	update_repr (nodepath);

	// if the entire nodepath is removed, delete the selected object.
	if (nodepath->subpaths == NULL ||
		sp_nodepath_get_node_count(nodepath) < 2) {
			sp_nodepath_destroy (nodepath);
			sp_selection_delete();
			return;
	}

	sp_nodepath_update_statusbar (nodepath);
}



/**
 * This is the code for 'split'
 */
void
sp_node_selected_delete_segment (void)
{
	Path::Node *start, *end;     //Start , end nodes.  not inclusive
	Path::Node *curr, *next;     //Iterators

	Path::Path *nodepath = sp_nodepath_current ();
	if (!nodepath) return; // there's no nodepath when editing rects, stars, spirals or ellipses

	if (g_list_length (nodepath->selected) != 2) {
	    nodepath->desktop->messageStack()->flash(Inkscape::ERROR_MESSAGE, 
                _("You must select two non-endpoint nodes on a path between which to delete segments."));
	    return;
	}
	
    //Selected nodes, not inclusive
	Path::Node *a = (Path::Node *) nodepath->selected->data;
	Path::Node *b = (Path::Node *) nodepath->selected->next->data;

	if ( ( a==b)                       ||  //same node
             (a->subpath  != b->subpath )  ||  //not the same path
             (!a->p.other || !a->n.other)  ||  //one of a's sides does not have a segment
             (!b->p.other || !b->n.other) )    //one of b's sides does not have a segment
		{
		    nodepath->desktop->messageStack()->flash(Inkscape::ERROR_MESSAGE, 
		        _("You must select two non-endpoint nodes on a path between which to delete segments."));
		    return;
		}

	//###########################################
	//# BEGIN EDITS
	//###########################################
	//##################################
	//# CLOSED PATH
	//##################################
	if (a->subpath->closed) {


        gboolean reversed = FALSE;

		//Since we can go in a circle, we need to find the shorter distance.
		//  a->b or b->a
		start = end = NULL;
		int distance    = 0;
		int minDistance = 0;
		for (curr = a->n.other ; curr && curr!=a ; curr=curr->n.other) {
			if (curr==b) {
				//printf("a to b:%d\n", distance);
				start = a;//go from a to b
				end   = b;
				minDistance = distance;
				//printf("A to B :\n");
				break;
			}
			distance++;
		}

		//try again, the other direction
		distance = 0;
		for (curr = b->n.other ; curr && curr!=b ; curr=curr->n.other) {
			if (curr==a) {
				//printf("b to a:%d\n", distance);
				if (distance < minDistance) {
					start    = b;  //we go from b to a
					end      = a;
                    reversed = TRUE;
					//printf("B to A\n");
				}
				break;
			}
			distance++;
		}

		
		//Copy everything from 'end' to 'start' to a new subpath
		Path::SubPath *t = sp_nodepath_subpath_new (nodepath);
		for (curr=end ; curr ; curr=curr->n.other) {
            NRPathcode code = (NRPathcode) curr->code;
            if (curr == end)
                code = NR_MOVETO;
			sp_nodepath_node_new (t, NULL, 
                (Path::NodeType)curr->type, code,
			    &curr->p.pos, &curr->pos, &curr->n.pos);
			if (curr == start)
				break;
		}
		sp_nodepath_subpath_destroy(a->subpath);


	}



	//##################################
	//# OPEN PATH
	//##################################
	else {

		//We need to get the direction of the list between A and B
		//Can we walk from a to b?
		start = end = NULL;
		for (curr = a->n.other ; curr && curr!=a ; curr=curr->n.other) {
			if (curr==b) {
				start = a;  //did it!  we go from a to b
				end   = b;
				//printf("A to B\n");
				break;
			}
		}
		if (!start) {//didn't work?  let's try the other direction
			for (curr = b->n.other ; curr && curr!=b ; curr=curr->n.other) {
				if (curr==a) {
					start = b;  //did it!  we go from b to a
					end   = a;
					//printf("B to A\n");
					break;
				}
			}
		}
		if (!start) {
		    nodepath->desktop->messageStack()->flash(Inkscape::ERROR_MESSAGE, 
			_("Cannot find path between nodes."));
		    return;
		}



		//Copy everything after 'end' to a new subpath
		Path::SubPath *t = sp_nodepath_subpath_new (nodepath);
		for (curr=end ; curr ; curr=curr->n.other) {
			sp_nodepath_node_new (t, NULL, (Path::NodeType)curr->type, (NRPathcode)curr->code,
			&curr->p.pos, &curr->pos, &curr->n.pos);
		}

		//Now let us do our deletion.  Since the tail has been saved, go all the way to the end of the list
		for (curr = start->n.other ; curr  ; curr=next) {
			next = curr->n.other;
			sp_nodepath_node_destroy (curr);
		}

	}
	//###########################################
	//# END EDITS
	//###########################################

	//clean up the nodepath (such as for trivial subpaths)
	sp_nodepath_cleanup(nodepath);

	sp_nodepath_ensure_ctrls (nodepath);

	update_repr (nodepath);

	// if the entire nodepath is removed, delete the selected object.
	if (nodepath->subpaths == NULL ||
		sp_nodepath_get_node_count(nodepath) < 2) {
			sp_nodepath_destroy (nodepath);
			sp_selection_delete();
			return;
	}

	sp_nodepath_update_statusbar (nodepath);
}


void
sp_node_selected_set_line_type (NRPathcode code)
{
	Path::Path *nodepath = sp_nodepath_current();
	if (nodepath == NULL) return;

	for (GList *l = nodepath->selected; l != NULL; l = l->next) {
		Path::Node *n = (Path::Node *) l->data;
		g_assert (n->selected);
		if (n->p.other && n->p.other->selected) {
			sp_nodepath_set_line_type (n, code);
		}
	}

	update_repr (nodepath);
}

void
sp_node_selected_set_type (Path::NodeType type)
{
	/* fixme: do it the right way */
    /* What is the right way?  njh */
	Path::Path *nodepath = sp_nodepath_current ();
	if (nodepath == NULL) return;

	for (GList *l = nodepath->selected; l != NULL; l = l->next) {
		sp_nodepath_set_node_type ((Path::Node *) l->data, type);
	}

	update_repr (nodepath);
}

static void sp_node_set_selected(Path::Node *node, gboolean selected)
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
static void sp_nodepath_node_select(Path::Node *node, gboolean incremental, gboolean override)
{
	Path::Path *nodepath = node->subpath->nodepath;

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
\brief deselect all nodes in the nodepath
*/
void
sp_nodepath_deselect (Path::Path *nodepath)
{
	if (!nodepath) return; // there's no nodepath when editing rects, stars, spirals or ellipses

	while (nodepath->selected) {
		sp_node_set_selected ((Path::Node *) nodepath->selected->data, FALSE);
		nodepath->selected = g_list_remove (nodepath->selected, nodepath->selected->data);
	}
	sp_nodepath_update_statusbar (nodepath);
}

/**
\brief select all nodes in the nodepath
*/
void
sp_nodepath_select_all (Path::Path *nodepath)
{
	if (!nodepath) return;

	for (GList *spl = nodepath->subpaths; spl != NULL; spl = spl->next) {
		Path::SubPath *subpath = (Path::SubPath *) spl->data;
		for (GList *nl = subpath->nodes; nl != NULL; nl = nl->next) {
			Path::Node *node = (Path::Node *) nl->data;
			sp_nodepath_node_select (node, TRUE, TRUE);
		}
	}
}

/**
\brief select the node after the last selected; if none is selected, select the first within path
*/
void sp_nodepath_select_next(Path::Path *nodepath)
{
	if (!nodepath) return; // there's no nodepath when editing rects, stars, spirals or ellipses

	Path::Node *last = NULL;
	if (nodepath->selected) {
		for (GList *spl = nodepath->subpaths; spl != NULL; spl = spl->next) {
			Path::SubPath *subpath, *subpath_next;
			subpath = (Path::SubPath *) spl->data;
			for (GList *nl = subpath->nodes; nl != NULL; nl = nl->next) {
				Path::Node *node = (Path::Node *) nl->data;
				if (node->selected) {
					if (node->n.other == (Path::Node *) subpath->last) {
						if (node->n.other == (Path::Node *) subpath->first) { // closed subpath 
							if (spl->next) { // there's a next subpath
								subpath_next = (Path::SubPath *) spl->next->data;
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
								subpath_next = (Path::SubPath *) spl->next->data;
								last = subpath_next->first;
							} else if (spl->prev) { // there's a previous subpath
								last = NULL; // to be set later to the first node of first subpath
							} else {
								last = (Path::Node *) subpath->first;
							}
						}
					}
				}
			}
		}
		sp_nodepath_deselect (nodepath);
	}

	if (last) { // there's at least one more node after selected
		sp_nodepath_node_select ((Path::Node *) last, TRUE, TRUE);
	} else { // no more nodes, select the first one in first subpath
		Path::SubPath *subpath = (Path::SubPath *) nodepath->subpaths->data;
		sp_nodepath_node_select ((Path::Node *) subpath->first, TRUE, TRUE);
	}
}

/**
\brief select the node before the first selected; if none is selected, select the last within path
*/
void sp_nodepath_select_prev(Path::Path *nodepath)
{
	if (!nodepath) return; // there's no nodepath when editing rects, stars, spirals or ellipses

	Path::Node *last = NULL;
	if (nodepath->selected) {
		for (GList *spl = g_list_last(nodepath->subpaths); spl != NULL; spl = spl->prev) {
			Path::SubPath *subpath = (Path::SubPath *) spl->data;
			for (GList *nl = g_list_last(subpath->nodes); nl != NULL; nl = nl->prev) {
				Path::Node *node = (Path::Node *) nl->data;
				if (node->selected) {
					if (node->p.other == (Path::Node *) subpath->first) {
						if (node->p.other == (Path::Node *) subpath->last) { // closed subpath 
							if (spl->prev) { // there's a prev subpath
								Path::SubPath *subpath_prev = (Path::SubPath *) spl->prev->data;
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
								Path::SubPath *subpath_prev = (Path::SubPath *) spl->prev->data;
								last = subpath_prev->last;
							} else if (spl->next) { // there's a next subpath
								last = NULL; // to be set later to the last node of last subpath
							} else {
								last = (Path::Node *) subpath->last;
							}
						}
					}
				}
			}
		}
		sp_nodepath_deselect (nodepath);
	}

	if (last) { // there's at least one more node before selected
		sp_nodepath_node_select ((Path::Node *) last, TRUE, TRUE);
	} else { // no more nodes, select the last one in last subpath
		GList *spl = g_list_last(nodepath->subpaths);
		Path::SubPath *subpath = (Path::SubPath *) spl->data;
		sp_nodepath_node_select ((Path::Node *) subpath->last, TRUE, TRUE);
	}
}

/**
\brief select all nodes that are within the rectangle
*/
void sp_nodepath_select_rect(Path::Path *nodepath, NRRect *b, gboolean incremental)
{
	if (!incremental) {
		sp_nodepath_deselect (nodepath);
	}

	for (GList *spl = nodepath->subpaths; spl != NULL; spl = spl->next) {
		Path::SubPath *subpath = (Path::SubPath *) spl->data;
		for (GList *nl = subpath->nodes; nl != NULL; nl = nl->next) {
			Path::Node *node = (Path::Node *) nl->data;

			NR::Point p = node->pos;

			if ((p[NR::X] > b->x0) && (p[NR::X] < b->x1) && (p[NR::Y] > b->y0) && (p[NR::Y] < b->y1)) {
				sp_nodepath_node_select (node, TRUE, FALSE);
			}
		}
	}
}

/**
\brief  Saves selected nodes in a nodepath into a list containing integer positions of all selected nodes
*/
GList *save_nodepath_selection (Path::Path *nodepath)
{
	if (!nodepath->selected) {
		return NULL;
	}

	GList *r = NULL;
	guint i = 0;
	for (GList *spl = nodepath->subpaths; spl != NULL; spl = spl->next) {
		Path::SubPath *subpath = (Path::SubPath *) spl->data;
		for (GList *nl = subpath->nodes; nl != NULL; nl = nl->next) {
			Path::Node *node = (Path::Node *) nl->data;
			i++;
			if (node->selected) {
				r = g_list_append (r, GINT_TO_POINTER (i));
			}
		}
	}
	return r;
}

/**
\brief  Restores selection by selecting nodes whose positions are in the list
*/
void restore_nodepath_selection (Path::Path *nodepath, GList *r)
{
	sp_nodepath_deselect (nodepath);

	guint i = 0;
	for (GList *spl = nodepath->subpaths; spl != NULL; spl = spl->next) {
		Path::SubPath *subpath = (Path::SubPath *) spl->data;
		for (GList *nl = subpath->nodes; nl != NULL; nl = nl->next) {
			Path::Node *node = (Path::Node *) nl->data;
			i++;
			if (g_list_find (r, GINT_TO_POINTER (i))) {
				sp_nodepath_node_select (node, TRUE, TRUE);
			}
		}
	}

}

/**
\brief adjusts control point according to node type and line code
*/
static void sp_node_adjust_knot(Path::Node *node, gint which_adjust)
{
	double len, otherlen, linelen;

	g_assert (node);

	Path::NodeSide *me = sp_node_get_side (node, which_adjust);
	Path::NodeSide *other = sp_node_opposite_side (node, me);

	/* fixme: */
	if (me->other == NULL) return;
	if (other->other == NULL) return;

	/* I have line */

	NRPathcode mecode, ocode;
	if (which_adjust == 1) {
		mecode = (NRPathcode)me->other->code;
		ocode = (NRPathcode)node->code;
	} else {
		mecode = (NRPathcode)node->code;
		ocode = (NRPathcode)other->other->code;
	}

	if (mecode == NR_LINETO) return;

	/* I am curve */

	if (other->other == NULL) return;

	/* Other has line */

	if (node->type == Path::NODE_CUSP) return;
	
	NR::Point delta;
	if (ocode == NR_LINETO) {
		/* other is lineto, we are either smooth or symm */
		Path::Node *othernode = other->other;
		len = NR::L2(me->pos - node->pos);
		delta = node->pos - othernode->pos;
		linelen = NR::L2(delta);
		if (linelen < 1e-18) return;

		me->pos = node->pos + (len / linelen)*delta;
		sp_knot_set_position (me->knot, &me->pos, 0);

		sp_node_ensure_ctrls (node);
		return;
	}

	if (node->type == Path::NODE_SYMM) {

		me->pos = 2 * node->pos - other->pos;
		sp_knot_set_position (me->knot, &me->pos, 0);

		sp_node_ensure_ctrls (node);
		return;
	}

	/* We are smooth */

	len = NR::L2 (me->pos - node->pos);
	delta = other->pos - node->pos;
	otherlen = NR::L2 (delta);
	if (otherlen < 1e-18) return;

	me->pos = node->pos - (len / otherlen) * delta;
	sp_knot_set_position (me->knot, &me->pos, 0);

	sp_node_ensure_ctrls (node);
}

/**
 \brief Adjusts control point according to node type and line code
 */
static void sp_node_adjust_knots(Path::Node *node)
{
	g_assert (node);

	if (node->type == Path::NODE_CUSP) return;

	/* we are either smooth or symm */

	if (node->p.other == NULL) return;

	if (node->n.other == NULL) return;

	if (node->code == NR_LINETO) {
		if (node->n.other->code == NR_LINETO) return;
		sp_node_adjust_knot (node, 1);
		sp_node_ensure_ctrls (node);
		return;
	}

	if (node->n.other->code == NR_LINETO) {
		if (node->code == NR_LINETO) return;
		sp_node_adjust_knot (node, -1);
		sp_node_ensure_ctrls (node);
		return;
	}

	/* both are curves */

	const NR::Point delta  = node->n.pos - node->p.pos;

	if (node->type == Path::NODE_SYMM) {
		node->p.pos = node->pos - delta / 2;
		node->n.pos = node->pos + delta / 2;
		sp_node_ensure_ctrls (node);
		return;
	}

	/* We are smooth */

	double plen = NR::L2(node->p.pos - node->pos);
	if (plen < 1e-18) return;
	double nlen = NR::L2(node->n.pos - node->pos);
	if (nlen < 1e-18) return;
	node->p.pos = node->pos - (plen / (plen + nlen)) * delta;
	node->n.pos = node->pos + (nlen / (plen + nlen)) * delta;
	sp_node_ensure_ctrls (node);
}

/*
 * Knot events
 */
static gboolean node_event(SPKnot *knot, GdkEvent *event, Path::Node *n)
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
				Path::Path *nodepath = n->subpath->nodepath;
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

gboolean node_key(GdkEvent *event)
{
	Path::Path *np;

	// there is no way to verify nodes so set active_node to nil when deleting!!
	if (active_node == NULL) return FALSE;

	if ((event->type == GDK_KEY_PRESS) && !(event->key.state & (GDK_SHIFT_MASK | GDK_CONTROL_MASK))) {
		gint ret = FALSE;
		switch (event->key.keyval) {
		case GDK_BackSpace:
			np = active_node->subpath->nodepath;
			sp_nodepath_node_destroy (active_node);
			update_repr (np);
			active_node = NULL;
			ret = TRUE;
			break;
		case GDK_c:
			sp_nodepath_set_node_type (active_node, Path::NODE_CUSP);
			ret = TRUE;
			break;
		case GDK_s:
			sp_nodepath_set_node_type (active_node, Path::NODE_SMOOTH);
			ret = TRUE;
			break;
		case GDK_y:
			sp_nodepath_set_node_type (active_node, Path::NODE_SYMM);
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

static void node_clicked(SPKnot *knot, guint state, gpointer data)
{
	Path::Node *n = (Path::Node *) data;

	if (state & GDK_CONTROL_MASK) {
		if (!(state & GDK_MOD1_MASK)) { // ctrl+click: toggle node type
			if (n->type == Path::NODE_CUSP) {
				sp_nodepath_set_node_type (n, Path::NODE_SMOOTH);
			} else if (n->type == Path::NODE_SMOOTH) {
				sp_nodepath_set_node_type (n, Path::NODE_SYMM);
			} else {
				sp_nodepath_set_node_type (n, Path::NODE_CUSP);
			}
		} else { //ctrl+alt+click: delete node
			Path::Path *nodepath = n->subpath->nodepath;
			sp_nodepath_node_destroy (n);
			if (nodepath->subpaths == NULL) { // if the entire nodepath is removed, delete the selected object.
				sp_nodepath_destroy (nodepath);
				sp_selection_delete();
			} else {
				sp_nodepath_ensure_ctrls (nodepath);
				update_repr (nodepath);
				sp_nodepath_update_statusbar (nodepath);
			}
		}
	} else {
		sp_nodepath_node_select (n, (state & GDK_SHIFT_MASK), FALSE);
	}
}

static void node_grabbed(SPKnot *knot, guint state, gpointer data)
{
	Path::Node *n = (Path::Node *) data;

	n->origin = knot->pos;

	if (!n->selected) {
		sp_nodepath_node_select (n, (state & GDK_SHIFT_MASK), FALSE);
	}
}

static void node_ungrabbed(SPKnot *knot, guint state, gpointer data)
{
	Path::Node *n = (Path::Node *) data;

	update_repr (n->subpath->nodepath);
}

/**
\brief The point on a line, given by its angle, closest to the given point
\param p   point
\param a   angle of the line; it is assumed to go through coordinate origin
\param closest   pointer to the point struct where the result is stored
*/
// FIXME: use dot product perhaps?
static void point_line_closest(NR::Point *p, double a, NR::Point *closest)
{
	if (a == HUGE_VAL) { // vertical
		*closest = NR::Point(0, (*p)[NR::Y]);
	} else {
		//		*closest = NR::Point(( ( a * (*p)[NR::Y] + (*p)[NR::X]) / (a*a + 1) ),
		//				     a * (*closest)[NR::X]);
		(*closest)[NR::X] = ( a * (*p)[NR::Y] + (*p)[NR::X]) / (a*a + 1);
		(*closest)[NR::Y] = a * (*closest)[NR::X];
	}
}

/**
\brief Distance from the point to a line given by its angle
\param p   point
\param a   angle of the line; it is assumed to go through coordinate origin
*/
static double point_line_distance(NR::Point *p, double a)
{
	NR::Point c;
	point_line_closest (p, a, &c);
	return sqrt (((*p)[NR::X] - c[NR::X])*((*p)[NR::X] - c[NR::X]) + ((*p)[NR::Y] - c[NR::Y])*((*p)[NR::Y] - c[NR::Y]));
}


/* fixme: This goes to "moved" event? (lauris) */
static gboolean
node_request (SPKnot *knot, NR::Point *p, guint state, gpointer data)
{
	double yn, xn, yp, xp;
	double an, ap, na, pa;
	double d_an, d_ap, d_na, d_pa;
	gboolean collinear = FALSE;
	NR::Point c;
	NR::Point pr;

	Path::Node *n = (Path::Node *) data;

	if (state & GDK_CONTROL_MASK) { // constrained motion 

		// calculate relative distances of control points
		yn = n->n.pos[NR::Y] - n->pos[NR::Y]; 
		xn = n->n.pos[NR::X] - n->pos[NR::X];
		if (xn < 0) { xn = -xn; yn = -yn; } // limit the handle angle to between 0 and pi
		if (yn < 0) { xn = -xn; yn = -yn; } 

		yp = n->p.pos[NR::Y] - n->pos[NR::Y];
		xp = n->p.pos[NR::X] - n->pos[NR::X];
		if (xp < 0) { xp = -xp; yp = -yp; } // limit the handle angle to between 0 and pi
		if (yp < 0) { xp = -xp; yp = -yp; } 

		if (state & GDK_MOD1_MASK && !(xn == 0 && xp == 0)) { 
			// sliding on handles, only if at least one of the handles is non-vertical

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
			pr = (*p) - n->origin;

			// distances to the four lines (two handles and two perpendiculars)
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
			sp_nodepath_selected_nodes_move (n->subpath->nodepath,
							 n->origin[NR::X] + c[NR::X] - n->pos[NR::X],
							 n->origin[NR::Y] + c[NR::Y] - n->pos[NR::Y]);

		} else {  // constraining to hor/vert

			if (fabs((*p)[NR::X] - n->origin[NR::X]) > fabs((*p)[NR::Y] - n->origin[NR::Y])) { // snap to hor
				sp_nodepath_selected_nodes_move (n->subpath->nodepath, (*p)[NR::X] - n->pos[NR::X], n->origin[NR::Y] - n->pos[NR::Y]);
			} else { // snap to vert
				sp_nodepath_selected_nodes_move (n->subpath->nodepath, n->origin[NR::X] - n->pos[NR::X], (*p)[NR::Y] - n->pos[NR::Y]);
			}
		}
	} else { // move freely
		sp_nodepath_selected_nodes_move (n->subpath->nodepath,
						 (*p)[NR::X] - n->pos[NR::X],
						 (*p)[NR::Y] - n->pos[NR::Y],
						 (state & GDK_SHIFT_MASK) == 0);
	}

	sp_desktop_scroll_to_point (n->subpath->nodepath->desktop, p);

	return TRUE;
}

static void node_ctrl_clicked(SPKnot *knot, guint state, gpointer data)
{
	Path::Node *n = (Path::Node *) data;

	sp_nodepath_node_select (n, (state & GDK_SHIFT_MASK), FALSE);
}

static void node_ctrl_grabbed(SPKnot *knot, guint state, gpointer data)
{
	Path::Node *n = (Path::Node *) data;

	if (!n->selected) {
		sp_nodepath_node_select (n, (state & GDK_SHIFT_MASK), FALSE);
	}

	// remember the origin of the control
	if (n->p.knot == knot) {
		n->p.origin = Radial(n->p.pos - n->pos);
	} else if (n->n.knot == knot) {
		n->n.origin = Radial(n->n.pos - n->pos);
	} else {
		g_assert_not_reached ();
	}

}

static void node_ctrl_ungrabbed(SPKnot *knot, guint state, gpointer data)
{
	Path::Node *n = (Path::Node *) data;

	// forget origin and set knot position once more (because it can be wrong now due to restrictions)
	if (n->p.knot == knot) {
		n->p.origin.a = 0;
		sp_knot_set_position (knot, &n->p.pos, state);
	} else if (n->n.knot == knot) {
		n->n.origin.a = 0;
		sp_knot_set_position (knot, &n->n.pos, state);
	} else {
		g_assert_not_reached ();
	}

	update_repr (n->subpath->nodepath);
}

static gboolean node_ctrl_request(SPKnot *knot, NR::Point *p, guint state, gpointer data)
{
	Path::Node *n = (Path::Node *) data;

	Path::NodeSide *me, *opposite;
	gint which;
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

	NRPathcode othercode = sp_node_path_code_from_side (n, opposite);

	if (opposite->other && (n->type != Path::NODE_CUSP) && (othercode == NR_LINETO)) {
		gdouble len, linelen, scal;
		/* We are smooth node adjacent with line */
		NR::Point delta = *p - n->pos;
		len = NR::L2(delta);
		Path::Node *othernode = opposite->other;
		NR::Point ndelta = n->pos - othernode->pos;
		linelen = NR::L2(ndelta);
		if ((len > 1e-18) && (linelen > 1e-18)) {
			scal = dot(delta, ndelta) / linelen;
			(*p) = n->pos + (scal / linelen) * ndelta;
		}
		namedview_vector_snap (n->subpath->nodepath->desktop->namedview, Snapper::SNAP_POINT, *p, ndelta);
	} else {
		namedview_free_snap (n->subpath->nodepath->desktop->namedview, Snapper::SNAP_POINT, *p);
	}

	sp_node_adjust_knot (n, -which);

	return FALSE;
}

static void node_ctrl_moved (SPKnot *knot, NR::Point *p, guint state, gpointer data)
{
	Path::Node *n = (Path::Node *) data;

	Path::NodeSide *me;
	Path::NodeSide *other;
	if (n->p.knot == knot) {
		me = &n->p;
		other = &n->n;
	} else if (n->n.knot == knot) {
		me = &n->n;
		other = &n->p;
	} else {
		me = NULL;
		other = NULL;
		g_assert_not_reached ();
	}

	// calculate radial coordinates of the grabbed control, other control, and the mouse point
	Radial rme(me->pos - n->pos);
	Radial rother(other->pos - n->pos);
	Radial rnew(*p - n->pos);

	if (state & GDK_CONTROL_MASK && rnew.a != HUGE_VAL) { 
		double a_snapped, a_ortho;

		int snaps = prefs_get_int_attribute ("options.rotationsnapsperpi", "value", 12);
		/* 0 interpreted as "no snapping". */

		// the closest PI/snaps angle, starting from zero
		a_snapped = floor (rnew.a/(M_PI/snaps) + 0.5) * (M_PI/snaps);
		// the closest PI/2 angle, starting from original angle (i.e. snapping to original, its opposite and perpendiculars)
		a_ortho = me->origin.a + floor ((rnew.a - me->origin.a)/(M_PI/2) + 0.5) * (M_PI/2);

		// snap to the closest
		if (fabs (a_snapped - rnew.a) < fabs (a_ortho - rnew.a))
			rnew.a = a_snapped;
		else 
			rnew.a = a_ortho;
	} 

	if (state & GDK_MOD1_MASK) { 
		// lock handle length
		rnew.r = me->origin.r;
	} 

	if (( n->type != Path::NODE_CUSP || (state & GDK_SHIFT_MASK)) 
             && rme.a != HUGE_VAL && rnew.a != HUGE_VAL && fabs (rme.a - rnew.a) > 0.001) { 
		// rotate the other handle correspondingly, if both old and new angles exist and are not the same
		rother.a += rnew.a - rme.a;
		other->pos = NR::Point(rother) + n->pos;
		sp_ctrlline_set_coords (SP_CTRLLINE (other->line), n->pos, other->pos);
		sp_knot_set_position (other->knot, &other->pos, 0);
	} 

	me->pos = NR::Point(rnew) + n->pos;
	sp_ctrlline_set_coords (SP_CTRLLINE (me->line), n->pos, me->pos);

	// this is what sp_knot_set_position does, but without emitting the signal:
	// we cannot emit a "moved" signal because we're now processing it
	if (me->knot->item) SP_CTRL (me->knot->item)->moveto (me->pos);

	sp_desktop_set_coordinate_status (knot->desktop, me->pos, 0);

	update_object (n->subpath->nodepath);
}

static gboolean node_ctrl_event(SPKnot *knot, GdkEvent *event, Path::Node *n)
{
	gboolean ret = FALSE;
	switch (event->type) {
	case GDK_KEY_PRESS:
		switch (event->key.keyval) {
		case GDK_space:
			if (event->key.state & GDK_BUTTON1_MASK) {
				Path::Path *nodepath = n->subpath->nodepath;
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

void
node_rotate_internal (Path::Node *n, gdouble angle, Radial &rme, Radial &rother, gboolean both)
{
	rme.a += angle; 
	if (both || n->type == Path::NODE_SMOOTH || n->type == Path::NODE_SYMM) 
		rother.a += angle;
}

void
node_rotate_internal_screen (Path::Node *n, gdouble const angle, Radial &rme, Radial &rother, gboolean both)
{
	gdouble r;

	gdouble const norm_angle = angle / SP_DESKTOP_ZOOM (n->subpath->nodepath->desktop);

	if (both || n->type == Path::NODE_SMOOTH || n->type == Path::NODE_SYMM) 
		r = MAX (rme.r, rother.r);
	else 
		r = rme.r;

	gdouble const weird_angle = atan2 (norm_angle, r);
/* Bulia says norm_angle is just the visible distance that the
 * object's end must travel on the screen.  Left as 'angle' for want of
 * a better name.*/

	rme.a += weird_angle; 
	if (both || n->type == Path::NODE_SMOOTH || n->type == Path::NODE_SYMM)  
		rother.a += weird_angle;
}

void
node_rotate_common (Path::Node *n, gdouble angle, int which, gboolean screen)
{
	Path::NodeSide *me, *other;
	gboolean both = FALSE;

	if (which > 0) {
		me = &(n->n);
		other = &(n->p);
	} else if (which < 0){
		me = &(n->p);
		other = &(n->n);
	} else {
		me = &(n->n);
		other = &(n->p);
		both = TRUE;
	}

	Radial rme(me->pos - n->pos);
	Radial rother(other->pos - n->pos);

	if (screen) {
		node_rotate_internal_screen (n, angle, rme, rother, both);
	} else {
		node_rotate_internal (n, angle, rme, rother, both);
	}

	me->pos = n->pos + NR::Point(rme);

	if (both || n->type == Path::NODE_SMOOTH || n->type == Path::NODE_SYMM) {
		other->pos =  n->pos + NR::Point(rother);
	}

	sp_node_ensure_ctrls (n);
}

void sp_nodepath_selected_nodes_rotate(Path::Path *nodepath, gdouble angle, int which)
{
	if (!nodepath) return;

	for (GList *l = nodepath->selected; l != NULL; l = l->next) {
		Path::Node *n = (Path::Node *) l->data;
		node_rotate_common (n, angle, which, FALSE);
	}

	update_object (nodepath);
	// fixme: use _keyed
	update_repr (nodepath);
}

void sp_nodepath_selected_nodes_rotate_screen(Path::Path *nodepath, gdouble angle, int which)
{
	if (!nodepath) return;

	for (GList *l = nodepath->selected; l != NULL; l = l->next) {
		Path::Node *n = (Path::Node *) l->data;
		node_rotate_common (n, angle, which, TRUE);
	}

	update_object (nodepath);
	// fixme: use _keyed
	update_repr (nodepath);
}

void node_scale(Path::Node *n, gdouble grow, int which)
{
	bool both = false;
	Path::NodeSide *me, *other;
	if (which > 0) {
		me = &(n->n);
		other = &(n->p);
	} else if (which < 0){
		me = &(n->p);
		other = &(n->n);
	} else {
		me = &(n->n);
		other = &(n->p);
		both = true;
	}

	Radial rme(me->pos - n->pos);
	Radial rother(other->pos - n->pos);

	rme.r += grow; 
	if (rme.r < 0) rme.r = 1e-6; // not 0, so that direction is not lost
	if (rme.a == HUGE_VAL) {
		rme.a = 0; // if direction is unknown, initialize to 0
		sp_node_selected_set_line_type (NR_CURVETO);
	}
	if (both || n->type == Path::NODE_SYMM) {
		rother.r += grow;
		if (rother.r < 0) rother.r = 1e-6;
		if (rother.r == HUGE_VAL) {
			rother.a = 0;
			sp_node_selected_set_line_type (NR_CURVETO);
		}
	}

	me->pos = n->pos + NR::Point(rme);

	if (both || n->type == Path::NODE_SYMM) {
		other->pos = n->pos + NR::Point(rother);
	}

	sp_node_ensure_ctrls (n);
}

void node_scale_screen (Path::Node *n, gdouble const grow, int which)
{
	node_scale (n, grow / SP_DESKTOP_ZOOM (n->subpath->nodepath->desktop), which);
}

void sp_nodepath_selected_nodes_scale(Path::Path *nodepath, gdouble const grow, int which)
{
	if (!nodepath) return;

	for (GList *l = nodepath->selected; l != NULL; l = l->next) {
		Path::Node *n = (Path::Node *) l->data;
		node_scale (n, grow, which);
	}

	update_object (nodepath);
	// fixme: use _keyed
	update_repr (nodepath);
}

void sp_nodepath_selected_nodes_scale_screen (Path::Path *nodepath, gdouble const grow, int which)
{
	if (!nodepath) return;

	for (GList *l = nodepath->selected; l != NULL; l = l->next) {
		Path::Node *n = (Path::Node *) l->data;
		node_scale_screen (n, grow, which);
	}

	update_object (nodepath);
	// fixme: use _keyed
	update_repr (nodepath);
}


/*
 * Constructors and destructors
 */

static Path::SubPath *sp_nodepath_subpath_new(Path::Path *nodepath)
{
	g_assert (nodepath);
	g_assert (nodepath->desktop);

	Path::SubPath *s = g_new (Path::SubPath, 1);

	s->nodepath = nodepath;
	s->closed = FALSE;
	s->nodes = NULL;
	s->first = NULL;
	s->last = NULL;

  // do not use prepend here because:
  // if you have a path like "subpath_1 subpath_2 ... subpath_k" in the svg, you end up with
  // subpath_k -> ... ->subpath_1 in the nodepath structure. thus the i-th node of the svg is not
  // the i-th node in the nodepath (only if there are multiple subpaths)
  // note that the problem only arise when called from subpath_from_bpath(), since for all the other
  // cases, the repr is updated after the call to sp_nodepath_subpath_new()
	nodepath->subpaths = g_list_append /*g_list_prepend*/ (nodepath->subpaths, s);

	return s;
}

static void sp_nodepath_subpath_destroy(Path::SubPath *subpath)
{
	g_assert (subpath);
	g_assert (subpath->nodepath);
	g_assert (g_list_find (subpath->nodepath->subpaths, subpath));

	while (subpath->nodes) {
		sp_nodepath_node_destroy ((Path::Node *) subpath->nodes->data);
	}

	subpath->nodepath->subpaths = g_list_remove (subpath->nodepath->subpaths, subpath);

	g_free (subpath);
}

static void sp_nodepath_subpath_close(Path::SubPath *sp)
{
	g_assert (!sp->closed);
	g_assert (sp->last != sp->first);
	g_assert (sp->first->code == NR_MOVETO);

	sp->closed = TRUE;

    //Link the head to the tail
	sp->first->p.other = sp->last;
	sp->last->n.other  = sp->first;
	sp->last->n.pos    = sp->first->n.pos;
	sp->first          = sp->last;

    //Remove the extra end node
	sp_nodepath_node_destroy (sp->last->n.other);
}

static void sp_nodepath_subpath_open(Path::SubPath *sp, Path::Node *n)
{
	g_assert (sp->closed);
	g_assert (n->subpath == sp);
	g_assert (sp->first == sp->last);

	/* We create new startpoint, current node will become last one */

	Path::Node *new_path = sp_nodepath_node_new(sp, n->n.other, Path::NODE_CUSP, NR_MOVETO,
						    &n->pos, &n->pos, &n->n.pos);


	sp->closed        = FALSE;

    //Unlink to make a head and tail
	sp->first         = new_path;
	sp->last          = n;
	n->n.other        = NULL;
	new_path->p.other = NULL;
}

Path::Node *
sp_nodepath_node_new (Path::SubPath *sp, Path::Node *next, Path::NodeType type, NRPathcode code,
		      NR::Point *ppos, NR::Point *pos, NR::Point *npos)
{
	g_assert (sp);
	g_assert (sp->nodepath);
	g_assert (sp->nodepath->desktop);

	if (nodechunk == NULL)
		nodechunk = g_mem_chunk_create (Path::Node, 32, G_ALLOC_AND_FREE);

	Path::Node *n = (Path::Node*)g_mem_chunk_alloc (nodechunk);

	n->subpath  = sp;
	n->type     = type;
	n->code     = code;
	n->selected = FALSE;
	n->pos      = *pos;
	n->p.pos    = *ppos;
	n->n.pos    = *npos;

	Path::Node *prev;
	if (next) {
		g_assert (g_list_find (sp->nodes, next));
		prev = next->p.other;
	} else {
		prev = sp->last;
	}

	if (prev)
		prev->n.other = n;
	else
		sp->first = n;

	if (next)
		next->p.other = n;
	else
		sp->last = n;

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
		      NULL);
	if (n->type == Path::NODE_CUSP)
		g_object_set (G_OBJECT (n->knot), "shape", SP_KNOT_SHAPE_DIAMOND, "size", 9, NULL);
	else
		g_object_set (G_OBJECT (n->knot), "shape", SP_KNOT_SHAPE_SQUARE, "size", 7, NULL);

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

static void sp_nodepath_node_destroy (Path::Node *node)
{
	g_assert (node);
	g_assert (node->subpath);
	g_assert (SP_IS_KNOT (node->knot));
	g_assert (SP_IS_KNOT (node->p.knot));
	g_assert (SP_IS_KNOT (node->n.knot));
	g_assert (g_list_find (node->subpath->nodes, node));

	Path::SubPath *sp = node->subpath;

	if (node->selected) { // first, deselect
		g_assert (g_list_find (node->subpath->nodepath->selected, node));
		node->subpath->nodepath->selected = g_list_remove (node->subpath->nodepath->selected, node);
	}

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
				sp->first->code = NR_MOVETO;
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

static Path::NodeSide *sp_node_get_side(Path::Node *node, gint which)
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

static Path::NodeSide *sp_node_opposite_side(Path::Node *node, Path::NodeSide *me)
{
	g_assert (node);

	if (me == &node->p) return &node->n;
	if (me == &node->n) return &node->p;

	g_assert_not_reached ();

	return NULL;
}

static NRPathcode sp_node_path_code_from_side(Path::Node *node, Path::NodeSide *me)
{
	g_assert (node);

	if (me == &node->p) {
		if (node->p.other) return (NRPathcode)node->code;
		return NR_MOVETO;
	}

	if (me == &node->n) {
		if (node->n.other) return (NRPathcode)node->n.other->code;
		return NR_MOVETO;
	}

	g_assert_not_reached ();

	return NR_END;
}

gchar const *sp_node_type_description(Path::Node *n)
{
	switch (n->type) {
	case Path::NODE_CUSP:
		// TRANSLATORS: "cusp" means "sharp" (cusp node); see also the Advanced Tutorial
		return _("cusp");
	case Path::NODE_SMOOTH:
		return _("smooth");
	case Path::NODE_SYMM:
		return _("symmetric");
	}
	return NULL;
}

void
sp_nodepath_update_statusbar (Path::Path *nodepath)
{
	if (!nodepath) return;

	const gchar* when_selected = _("Drag nodes or control points to edit the path");

	gint total = 0;

	for (GList *spl = nodepath->subpaths; spl != NULL; spl = spl->next) {
		Path::SubPath *subpath = (Path::SubPath *) spl->data;
		total += g_list_length (subpath->nodes);
	}

	gint selected = g_list_length (nodepath->selected);

	if (selected == 0) {
	    SPSelection *sel = nodepath->desktop->selection;
	    if (!sel || sel->isEmpty()) {
		nodepath->desktop->messageStack()->flash(Inkscape::NORMAL_MESSAGE, 
			 _("Select one path object with selector first, then switch back to node tool."));
	    } else {
		nodepath->desktop->messageStack()->flashF(Inkscape::NORMAL_MESSAGE, 
			 _("0 out of %i nodes selected. Click, Shift+click, or drag around nodes to select."), total);
	    }
	} else if (selected == 1) {
	    nodepath->desktop->messageStack()->flashF(Inkscape::NORMAL_MESSAGE, 
		     _("%i of %i nodes selected; %s. %s."), selected, total, sp_node_type_description ((Path::Node *) nodepath->selected->data), when_selected);
	} else {
	    nodepath->desktop->messageStack()->flashF(Inkscape::NORMAL_MESSAGE, 
		    _("%i of %i nodes selected. %s."), selected, total, when_selected);
	}
}
