#define __SP_PATTERN_C__

/*
 * SVG <pattern> implementation
 *
 * Author:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * Copyright (C) 2002 Lauris Kaplinski
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "config.h"

#include <string.h>
#include <libnr/nr-rect.h>
#include <libnr/nr-matrix.h>
#include <libnr/nr-matrix-ops.h>
#include <gtk/gtksignal.h>
#include "macros.h"
#include "xml/repr-private.h"
#include "svg/svg.h"
#include "display/nr-arena.h"
#include "display/nr-arena-group.h"
#include "attributes.h"
#include "document.h"
#include "sp-object-repr.h"
#include "sp-item.h"
#include "document.h"
#include "document-private.h"

#include "sp-pattern.h"

/*
 * Pattern
 */

class SPPatPainter;

struct SPPatPainter {
	SPPainter painter;
	SPPattern *pat;

	NRMatrix ps2px;
	NRMatrix px2ps;
	NRMatrix pcs2px;

	NRArena *arena;
	unsigned int dkey;
	NRArenaItem *root;
};

static void sp_pattern_class_init (SPPatternClass *klass);
static void sp_pattern_init (SPPattern *gr);

static void sp_pattern_build (SPObject *object, SPDocument *document, SPRepr *repr);
static void sp_pattern_release (SPObject *object);
static void sp_pattern_set (SPObject *object, unsigned int key, const gchar *value);
static void sp_pattern_child_added (SPObject *object, SPRepr *child, SPRepr *ref);
static void sp_pattern_update (SPObject *object, SPCtx *ctx, unsigned int flags);
static void sp_pattern_modified (SPObject *object, unsigned int flags);

static void pattern_ref_changed(SPObject *old_ref, SPObject *ref, SPPattern *pat);
static void pattern_ref_modified (SPObject *ref, guint flags, SPPattern *pattern);

static SPPainter *sp_pattern_painter_new (SPPaintServer *ps, NR::Matrix const &full_transform, NR::Matrix const &parent_transform, const NRRect *bbox);
static void sp_pattern_painter_free (SPPaintServer *ps, SPPainter *painter);

static SPPaintServerClass * pattern_parent_class;

GType
sp_pattern_get_type (void)
{
	static GType pattern_type = 0;
	if (!pattern_type) {
		GTypeInfo pattern_info = {
			sizeof (SPPatternClass),
			NULL,	/* base_init */
			NULL,	/* base_finalize */
			(GClassInitFunc) sp_pattern_class_init,
			NULL,	/* class_finalize */
			NULL,	/* class_data */
			sizeof (SPPattern),
			16,	/* n_preallocs */
			(GInstanceInitFunc) sp_pattern_init,
			NULL,	/* value_table */
		};
		pattern_type = g_type_register_static (SP_TYPE_PAINT_SERVER, "SPPattern", &pattern_info, (GTypeFlags)0);
	}
	return pattern_type;
}

static void
sp_pattern_class_init (SPPatternClass *klass)
{
	SPObjectClass *sp_object_class;
	SPPaintServerClass *ps_class;

	sp_object_class = (SPObjectClass *) klass;
	ps_class = (SPPaintServerClass *) klass;

	pattern_parent_class = (SPPaintServerClass*)g_type_class_ref (SP_TYPE_PAINT_SERVER);

	sp_object_class->build = sp_pattern_build;
	sp_object_class->release = sp_pattern_release;
	sp_object_class->set = sp_pattern_set;
	sp_object_class->child_added = sp_pattern_child_added;
	sp_object_class->update = sp_pattern_update;
	sp_object_class->modified = sp_pattern_modified;

	ps_class->painter_new = sp_pattern_painter_new;
	ps_class->painter_free = sp_pattern_painter_free;
}

static void
sp_pattern_init (SPPattern *pat)
{
	pat->ref = new SPPatternReference(SP_OBJECT(pat));
	pat->ref->changedSignal().connect(SigC::bind(SigC::slot(pattern_ref_changed), pat));

	pat->patternUnits = SP_PATTERN_UNITS_OBJECTBOUNDINGBOX;
	pat->patternUnits_set = FALSE;

	pat->patternContentUnits = SP_PATTERN_UNITS_USERSPACEONUSE;
	pat->patternContentUnits_set = FALSE;

	nr_matrix_set_identity (&pat->patternTransform);
	pat->patternTransform_set = FALSE;

	sp_svg_length_unset (&pat->x, SP_SVG_UNIT_NONE, 0.0, 0.0);
	sp_svg_length_unset (&pat->y, SP_SVG_UNIT_NONE, 0.0, 0.0);
	sp_svg_length_unset (&pat->width, SP_SVG_UNIT_NONE, 0.0, 0.0);
	sp_svg_length_unset (&pat->height, SP_SVG_UNIT_NONE, 0.0, 0.0);

	pat->viewBox_set = FALSE;
}

static void
sp_pattern_build (SPObject *object, SPDocument *document, SPRepr *repr)
{
	if (((SPObjectClass *) pattern_parent_class)->build)
		(* ((SPObjectClass *) pattern_parent_class)->build) (object, document, repr);

	sp_object_read_attr (object, "patternUnits");
	sp_object_read_attr (object, "patternContentUnits");
	sp_object_read_attr (object, "patternTransform");
	sp_object_read_attr (object, "x");
	sp_object_read_attr (object, "y");
	sp_object_read_attr (object, "width");
	sp_object_read_attr (object, "height");
	sp_object_read_attr (object, "viewBox");
	sp_object_read_attr (object, "xlink:href");

	/* Register ourselves */
	sp_document_add_resource (document, "pattern", object);
}

static void
sp_pattern_release (SPObject *object)
{
	SPPattern *pat;

	pat = (SPPattern *) object;

	if (SP_OBJECT_DOCUMENT (object)) {
		/* Unregister ourselves */
		sp_document_remove_resource (SP_OBJECT_DOCUMENT (object), "pattern", SP_OBJECT (object));
	}

	if (pat->ref) {
		if (pat->ref->getObject())
			sp_signal_disconnect_by_data(pat->ref->getObject(), pat);
		pat->ref->detach();
		delete pat->ref;
		pat->ref = NULL;
	}

	if (((SPObjectClass *) pattern_parent_class)->release)
		((SPObjectClass *) pattern_parent_class)->release (object);
}

static void
sp_pattern_set (SPObject *object, unsigned int key, const gchar *value)
{
	SPPattern *pat = SP_PATTERN (object);

	/* fixme: We should unset properties, if val == NULL */
	switch (key) {
	case SP_ATTR_PATTERNUNITS:
		if (value) {
			if (!strcmp (value, "userSpaceOnUse")) {
				pat->patternUnits = SP_PATTERN_UNITS_USERSPACEONUSE;
			} else {
				pat->patternUnits = SP_PATTERN_UNITS_OBJECTBOUNDINGBOX;
			}
			pat->patternUnits_set = TRUE;
		} else {
			pat->patternUnits_set = FALSE;
		}
		sp_object_request_modified (object, SP_OBJECT_MODIFIED_FLAG);
		break;
	case SP_ATTR_PATTERNCONTENTUNITS:
		if (value) {
			if (!strcmp (value, "userSpaceOnUse")) {
				pat->patternContentUnits = SP_PATTERN_UNITS_USERSPACEONUSE;
			} else {
				pat->patternContentUnits = SP_PATTERN_UNITS_OBJECTBOUNDINGBOX;
			}
			pat->patternContentUnits_set = TRUE;
		} else {
			pat->patternContentUnits_set = FALSE;
		}
		sp_object_request_modified (object, SP_OBJECT_MODIFIED_FLAG);
		break;
	case SP_ATTR_PATTERNTRANSFORM: {
		NRMatrix t;
		if (value && sp_svg_transform_read (value, &t)) {
			int i;
			for (i = 0; i < 6; i++) pat->patternTransform.c[i] = t.c[i];
			pat->patternTransform_set = TRUE;
		} else {
			nr_matrix_set_identity (&pat->patternTransform);
			pat->patternTransform_set = FALSE;
		}
		sp_object_request_modified (object, SP_OBJECT_MODIFIED_FLAG);
		break;
	}
	case SP_ATTR_X:
		if (!sp_svg_length_read (value, &pat->x)) {
			sp_svg_length_unset (&pat->x, SP_SVG_UNIT_NONE, 0.0, 0.0);
		}
		sp_object_request_modified (object, SP_OBJECT_MODIFIED_FLAG);
		break;
	case SP_ATTR_Y:
		if (!sp_svg_length_read (value, &pat->y)) {
			sp_svg_length_unset (&pat->y, SP_SVG_UNIT_NONE, 0.0, 0.0);
		}
		sp_object_request_modified (object, SP_OBJECT_MODIFIED_FLAG);
		break;
	case SP_ATTR_WIDTH:
		if (!sp_svg_length_read (value, &pat->width)) {
			sp_svg_length_unset (&pat->width, SP_SVG_UNIT_NONE, 0.0, 0.0);
		}
		sp_object_request_modified (object, SP_OBJECT_MODIFIED_FLAG);
		break;
	case SP_ATTR_HEIGHT:
		if (!sp_svg_length_read (value, &pat->height)) {
			sp_svg_length_unset (&pat->height, SP_SVG_UNIT_NONE, 0.0, 0.0);
		}
		sp_object_request_modified (object, SP_OBJECT_MODIFIED_FLAG);
		break;
	case SP_ATTR_VIEWBOX: {
		/* fixme: Think (Lauris) */
		double x, y, width, height;
		char *eptr;

		if (value) {
			eptr = (gchar *) value;
			x = g_ascii_strtod (eptr, &eptr);
			while (*eptr && ((*eptr == ',') || (*eptr == ' '))) eptr++;
			y = g_ascii_strtod (eptr, &eptr);
			while (*eptr && ((*eptr == ',') || (*eptr == ' '))) eptr++;
			width = g_ascii_strtod (eptr, &eptr);
			while (*eptr && ((*eptr == ',') || (*eptr == ' '))) eptr++;
			height = g_ascii_strtod (eptr, &eptr);
			while (*eptr && ((*eptr == ',') || (*eptr == ' '))) eptr++;
			if ((width > 0) && (height > 0)) {
				pat->viewBox.x0 = x;
				pat->viewBox.y0 = y;
				pat->viewBox.x1 = x + width;
				pat->viewBox.y1 = y + height;
				pat->viewBox_set = TRUE;
			} else {
				pat->viewBox_set = FALSE;
			}
		} else {
			pat->viewBox_set = FALSE;
		}
		sp_object_request_modified (object, SP_OBJECT_MODIFIED_FLAG | SP_OBJECT_VIEWPORT_MODIFIED_FLAG);
		break;
	}
	case SP_ATTR_XLINK_HREF:
		if ( value && pat->href && ( strcmp(value, pat->href) == 0 ) ) {
			/* Href unchanged, do nothing. */
		} else {
			g_free(pat->href);
			pat->href = NULL;
			if (value) {
				// First, set the href field; it's only used in the "unchanged" check above.
				pat->href = g_strdup(value);
				// Now do the attaching, which emits the changed signal.
				if (value) {
					try {
						pat->ref->attach(Inkscape::URI(value));
					} catch (Inkscape::BadURIException &e) {
						g_warning("%s", e.what());
						pat->ref->detach();
					}
				} else {
					pat->ref->detach();
				}
			}
		}
		break;
	default:
		if (((SPObjectClass *) pattern_parent_class)->set)
			((SPObjectClass *) pattern_parent_class)->set (object, key, value);
		break;
	}
}

static void
sp_pattern_child_added (SPObject *object, SPRepr *child, SPRepr *ref)
{
	SPPattern *pat = SP_PATTERN (object);

	if (((SPObjectClass *) (pattern_parent_class))->child_added)
		(* ((SPObjectClass *) (pattern_parent_class))->child_added) (object, child, ref);

	SPObject *ochild = sp_object_get_child_by_repr(object, child);
	if (SP_IS_ITEM (ochild)) {

		SPPaintServer *ps = SP_PAINT_SERVER (pat);
		unsigned position = sp_item_pos_in_parent(SP_ITEM(ochild));

		for (SPPainter *p = ps->painters; p != NULL; p = p->next) {

			SPPatPainter *pp = (SPPatPainter *) p;
			NRArenaItem *ai = sp_item_invoke_show (SP_ITEM (ochild), pp->arena, pp->dkey, SP_ITEM_REFERENCE_FLAGS);

			if (ai) {
				nr_arena_item_add_child (pp->root, ai, NULL);
				nr_arena_item_set_order (ai, position);
				nr_arena_item_unref (ai);
			}
		}
	}
}

/* TODO: do we need a ::remove_child handler? */
 
/* fixme: We need ::order_changed handler too (Lauris) */

GSList *
pattern_getchildren (SPPattern *pat)
{
	GSList *l = NULL;

	for (SPPattern *pat_i = pat; pat_i != NULL; pat_i = pat_i->ref ? pat_i->ref->getObject() : NULL) {
		if (sp_object_first_child(SP_OBJECT(pat_i))) { // find the first one with children
			for (SPObject *child = sp_object_first_child(SP_OBJECT (pat)) ; child != NULL ; child = SP_OBJECT_NEXT(child) ) {
				l = g_slist_prepend (l, child);
			}
			break; // do not go further up the chain if children are found
		}
	}

	return l;
}

static void
sp_pattern_update (SPObject *object, SPCtx *ctx, unsigned int flags)
{
	SPPattern *pat = SP_PATTERN (object);

	if (flags & SP_OBJECT_MODIFIED_FLAG) flags |= SP_OBJECT_PARENT_MODIFIED_FLAG;
	flags &= SP_OBJECT_MODIFIED_CASCADE;

	GSList *l = pattern_getchildren (pat);
	l = g_slist_reverse (l);

	while (l) {
		SPObject *child = SP_OBJECT (l->data);
		sp_object_ref (child, NULL);
		l = g_slist_remove (l, child);
		if (flags || (child->mflags & (SP_OBJECT_MODIFIED_FLAG | SP_OBJECT_CHILD_MODIFIED_FLAG))) {
			sp_object_invoke_update (child, ctx, flags);
		}
		sp_object_unref (child, NULL);
	}
}

static void
sp_pattern_modified (SPObject *object, guint flags)
{
	SPPattern *pat = SP_PATTERN (object);

	if (flags & SP_OBJECT_MODIFIED_FLAG) flags |= SP_OBJECT_PARENT_MODIFIED_FLAG;
	flags &= SP_OBJECT_MODIFIED_CASCADE;

	GSList *l = pattern_getchildren (pat);
	l = g_slist_reverse (l);

	while (l) {
		SPObject *child = SP_OBJECT (l->data);
		sp_object_ref (child, NULL);
		l = g_slist_remove (l, child);
		if (flags || (child->mflags & (SP_OBJECT_MODIFIED_FLAG | SP_OBJECT_CHILD_MODIFIED_FLAG))) {
			sp_object_invoke_modified (child, flags);
		}
		sp_object_unref (child, NULL);
	}
}

/**
Gets called when the pattern is reattached to another <pattern>
*/
static void
pattern_ref_changed(SPObject *old_ref, SPObject *ref, SPPattern *pat)
{
	if (old_ref) {
		sp_signal_disconnect_by_data(old_ref, pat);
	}
	if (SP_IS_PATTERN (ref)) {
		g_signal_connect(G_OBJECT (ref), "modified", G_CALLBACK (pattern_ref_modified), pat);
	}

	pattern_ref_modified (ref, 0, pat);
}

/**
Gets called when the referenced <pattern> is changed
*/
static void
pattern_ref_modified (SPObject *ref, guint flags, SPPattern *pattern)
{
	if (SP_IS_OBJECT (pattern))
		sp_object_request_modified (SP_OBJECT (pattern), SP_OBJECT_MODIFIED_FLAG);
}

guint
pattern_users (SPPattern *pattern)
{
	return SP_OBJECT (pattern)->hrefcount;
}

SPPattern *
pattern_chain (SPPattern *pattern)
{
	SPDocument *document = SP_OBJECT_DOCUMENT (pattern);
	SPRepr *defsrepr = SP_OBJECT_REPR (SP_DOCUMENT_DEFS (document));

	SPRepr *repr = sp_repr_new ("pattern");
	gchar *parent_ref = g_strconcat ("#", sp_repr_attr(SP_OBJECT_REPR(pattern), "id"), NULL);
	sp_repr_set_attr (repr, "xlink:href",  parent_ref);
	g_free (parent_ref);

	sp_repr_add_child (defsrepr, repr, NULL);
	const gchar *child_id = sp_repr_attr(repr, "id");
	SPObject *child = sp_document_lookup_id (document, child_id);
	g_assert (SP_IS_PATTERN (child));

	return SP_PATTERN (child);
}

// Access functions that look up fields up the chain of referenced patterns and return the first one which is set

guint pattern_patternUnits (SPPattern *pat)
{
	for (SPPattern *pat_i = pat; pat_i != NULL; pat_i = pat_i->ref ? pat_i->ref->getObject() : NULL) {
		if (pat_i->patternUnits_set)
			return pat_i->patternUnits;
	}
	return pat->patternUnits;
}

guint pattern_patternContentUnits (SPPattern *pat)
{
	for (SPPattern *pat_i = pat; pat_i != NULL; pat_i = pat_i->ref ? pat_i->ref->getObject() : NULL) {
		if (pat_i->patternContentUnits_set)
			return pat_i->patternContentUnits;
	}
	return pat->patternContentUnits;
}

NRMatrix *pattern_patternTransform (SPPattern *pat)
{
	for (SPPattern *pat_i = pat; pat_i != NULL; pat_i = pat_i->ref ? pat_i->ref->getObject() : NULL) {
		if (pat_i->patternTransform_set)
			return &(pat_i->patternTransform);
	}
	return &(pat->patternTransform);
}

gdouble pattern_x (SPPattern *pat)
{
	for (SPPattern *pat_i = pat; pat_i != NULL; pat_i = pat_i->ref ? pat_i->ref->getObject() : NULL) {
		if (pat_i->x.set)
			return pat_i->x.computed;
	}
	return 0;
}

gdouble pattern_y (SPPattern *pat)
{
	for (SPPattern *pat_i = pat; pat_i != NULL; pat_i = pat_i->ref ? pat_i->ref->getObject() : NULL) {
		if (pat_i->y.set)
			return pat_i->y.computed;
	}
	return 0;
}

gdouble pattern_width (SPPattern *pat)
{
	for (SPPattern *pat_i = pat; pat_i != NULL; pat_i = pat_i->ref ? pat_i->ref->getObject() : NULL) {
		if (pat_i->width.set)
			return pat_i->width.computed;
	}
	return 0;
}

gdouble pattern_height (SPPattern *pat)
{
	for (SPPattern *pat_i = pat; pat_i != NULL; pat_i = pat_i->ref ? pat_i->ref->getObject() : NULL) {
		if (pat_i->height.set)
			return pat_i->height.computed;
	}
	return 0;
}

NRRect *pattern_viewBox (SPPattern *pat)
{
	for (SPPattern *pat_i = pat; pat_i != NULL; pat_i = pat_i->ref ? pat_i->ref->getObject() : NULL) {
		if (pat_i->viewBox_set)
			return &(pat_i->viewBox);
	}
	return &(pat->viewBox);
}


/* Painter */

static void sp_pat_fill (SPPainter *painter, NRPixBlock *pb);

/**
Creates a painter (i.e. the thing that does actual filling at the given zoom).
See (*) below for why the parent_transform may be necessary.
*/
static SPPainter *
sp_pattern_painter_new (SPPaintServer *ps, NR::Matrix const &full_transform, NR::Matrix const &parent_transform, const NRRect *bbox)
{
	SPPattern *pat = SP_PATTERN (ps);
	SPPatPainter *pp = g_new (SPPatPainter, 1);

	SPObject *child;

	pp->painter.type = SP_PAINTER_IND;
	pp->painter.fill = sp_pat_fill;

	pp->pat = pat;

	if (pattern_patternUnits (pat) == SP_PATTERN_UNITS_OBJECTBOUNDINGBOX) {
		/* BBox to user coordinate system */
		NR::Matrix bbox2user (bbox->x1 - bbox->x0, 0.0, 0.0, bbox->y1 - bbox->y0, bbox->x0, bbox->y0);

		// the final patternTransform, taking into account bbox
		NR::Matrix ps2user = NR::Matrix (pattern_patternTransform (pat)) * bbox2user;

		// see (*) comment below
		NR::Matrix ps2px = ps2user * full_transform;

		ps2px.copyto (&pp->ps2px);

	} else {
		/* Problem: What to do, if we have mixed lengths and percentages? */
		/* Currently we do ignore percentages at all, but that is not good (lauris) */

		/* fixme: We may try to normalize here too, look at linearGradient (Lauris) */

		// (*) The spec says, "This additional transformation matrix [patternTransform] is
		// post-multiplied to (i.e., inserted to the right of) any previously defined
		// transformations, including the implicit transformation necessary to convert from
		// object bounding box units to user space." To me, this means that the order should be:
		// item_transform * patternTransform * parent_transform
		// However both Batik and Adobe plugin use:
		// patternTransform * item_transform * parent_transform
		// So here I comply with the majority opinion, but leave my interpretation commented out below.
		// (To get item_transform, I subtract parent from full.)

		//NR::Matrix ps2px = (full_transform * parent_transform.inverse()) * NR::Matrix (pattern_patternTransform (pat)) * parent_transform;
		NR::Matrix ps2px = NR::Matrix (pattern_patternTransform (pat)) * full_transform;

		ps2px.copyto (&pp->ps2px);
	}

	nr_matrix_invert (&pp->px2ps, &pp->ps2px);

	if (pat->viewBox_set) {
		/* Forget content units at all (lauris) */
		gdouble tmp_x = pattern_width (pat) / (pattern_viewBox(pat)->x1 - pattern_viewBox(pat)->x0);
		gdouble tmp_y = pattern_height (pat) / (pattern_viewBox(pat)->y1 - pattern_viewBox(pat)->y0);

		NR::Matrix vb2ps (tmp_x, 0.0, 0.0, tmp_y, -pattern_viewBox(pat)->x0 * tmp_x, -pattern_viewBox(pat)->y0 * tmp_y);

		/* Problem: What to do, if we have mixed lengths and percentages? (Lauris) */
		/* Currently we do ignore percentages at all, but that is not good (Lauris) */

		NR::Matrix vb2us = vb2ps * NR::Matrix (pattern_patternTransform (pat));

		// see (*)
		NR::Matrix pcs2px = vb2us * full_transform;

		pcs2px.copyto (&pp->pcs2px);
	} else {
		NR::Matrix pcs2px;

		/* No viewbox, have to parse units */
		if (pattern_patternContentUnits (pat) == SP_PATTERN_UNITS_OBJECTBOUNDINGBOX) {
			/* BBox to user coordinate system */
			NR::Matrix bbox2user (bbox->x1 - bbox->x0, 0.0, 0.0, bbox->y1 - bbox->y0, bbox->x0, bbox->y0);

			NR::Matrix pcs2user = NR::Matrix (pattern_patternTransform (pat)) * bbox2user;

			// see (*)
			pcs2px = pcs2user * full_transform;
		} else {
			// see (*)
			//pcs2px = (full_transform * parent_transform.inverse()) * NR::Matrix (pattern_patternTransform (pat)) * parent_transform;
			pcs2px = NR::Matrix (pattern_patternTransform (pat)) * full_transform;
		}

		pcs2px = NR::translate (pattern_x (pat), pattern_y (pat)) * pcs2px; 

		pcs2px.copyto (&pp->pcs2px);
	}

	/* Create arena */
	pp->arena = NRArena::create();

	pp->dkey = sp_item_display_key_new (1);

	/* Create group */
	pp->root = NRArenaGroup::create(pp->arena);

	/* Show items */
	for (SPPattern *pat_i = pat; pat_i != NULL; pat_i = pat_i->ref ? pat_i->ref->getObject() : NULL) {
		if (sp_object_first_child(SP_OBJECT(pat_i))) { // find the first one with children
			for (child = sp_object_first_child(SP_OBJECT(pat_i)) ; child != NULL; child = SP_OBJECT_NEXT(child) ) {
				if (SP_IS_ITEM (child)) {
					NRArenaItem *cai;
					cai = sp_item_invoke_show (SP_ITEM (child), pp->arena, pp->dkey, SP_ITEM_REFERENCE_FLAGS);
					nr_arena_item_append_child (pp->root, cai);
					nr_arena_item_unref (cai);
				}
			}
			break; // do not go further up the chain if children are found
		}
	}

	NRGC gc(NULL);
	gc.transform = pp->pcs2px;
	nr_arena_item_invoke_update (pp->root, NULL, &gc, NR_ARENA_ITEM_STATE_ALL, NR_ARENA_ITEM_STATE_ALL);

	return (SPPainter *) pp;
}

static void
sp_pattern_painter_free (SPPaintServer *ps, SPPainter *painter)
{
	SPPatPainter *pp;
	SPPattern *pat;
	SPObject *child;

	pp = (SPPatPainter *) painter;
	pat = pp->pat;

	for (SPPattern *pat_i = pat; pat_i != NULL; pat_i = pat_i->ref ? pat_i->ref->getObject() : NULL) {
		if (pat_i && SP_IS_OBJECT (pat_i) && sp_object_first_child(SP_OBJECT(pat_i))) { // find the first one with children
			for (child = sp_object_first_child(SP_OBJECT(pat_i)) ; child != NULL; child = SP_OBJECT_NEXT(child) ) {
				if (SP_IS_ITEM (child)) {
						sp_item_invoke_hide (SP_ITEM (child), pp->dkey);
				}
			}
			break; // do not go further up the chain if children are found
		}
	}

	g_free (pp);
}

static void
sp_pat_fill (SPPainter *painter, NRPixBlock *pb)
{
	SPPatPainter *pp;
	NRRect ba, psa;
	NRRectL area;
	double x, y;

	pp = (SPPatPainter *) painter;

	if (pattern_width (pp->pat) < NR_EPSILON) return;
	if (pattern_height (pp->pat) < NR_EPSILON) return;

	/* Find buffer area in gradient space */
	/* fixme: This is suboptimal (Lauris) */

	ba.x0 = pb->area.x0;
	ba.y0 = pb->area.y0;
	ba.x1 = pb->area.x1;
	ba.y1 = pb->area.y1;
	nr_rect_d_matrix_transform (&psa, &ba, &pp->px2ps);

	psa.x0 = floor ((psa.x0 - pattern_x (pp->pat)) / pattern_width (pp->pat));
	psa.y0 = floor ((psa.y0 - pattern_y (pp->pat)) / pattern_height (pp->pat));
	psa.x1 = ceil ((psa.x1 - pattern_x (pp->pat)) / pattern_width (pp->pat));
	psa.y1 = ceil ((psa.y1 - pattern_y (pp->pat)) / pattern_height (pp->pat));

	for (y = psa.y0; y < psa.y1; y++) {
		for (x = psa.x0; x < psa.x1; x++) {
			NRPixBlock ppb;
			double psx, psy;

			psx = x * pattern_width (pp->pat);
			psy = y * pattern_height (pp->pat);

			area.x0 = (gint32)(pb->area.x0 - (pp->ps2px.c[0] * psx + pp->ps2px.c[2] * psy));
			area.y0 = (gint32)(pb->area.y0 - (pp->ps2px.c[1] * psx + pp->ps2px.c[3] * psy));
			area.x1 = area.x0 + pb->area.x1 - pb->area.x0;
			area.y1 = area.y0 + pb->area.y1 - pb->area.y0;

			/* We do not update here anymore */

			/* Set up buffer */
			/* fixme: (Lauris) */
			nr_pixblock_setup_extern (&ppb, pb->mode, area.x0, area.y0, area.x1, area.y1, NR_PIXBLOCK_PX (pb), pb->rs, FALSE, FALSE);

			nr_arena_item_invoke_render (pp->root, &area, &ppb, 0);

			nr_pixblock_release (&ppb);
		}
	}
}

