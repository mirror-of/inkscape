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

static void sp_pattern_href_destroy (SPObject *href, SPPattern *pattern);
static void sp_pattern_href_modified (SPObject *href, guint flags, SPPattern *pattern);

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
	pat->patternUnits = SP_PATTERN_UNITS_OBJECTBOUNDINGBOX;

	nr_matrix_set_identity (&pat->patternTransform);

	sp_svg_length_unset (&pat->x, SP_SVG_UNIT_NONE, 0.0, 0.0);
	sp_svg_length_unset (&pat->y, SP_SVG_UNIT_NONE, 0.0, 0.0);
	sp_svg_length_unset (&pat->width, SP_SVG_UNIT_NONE, 0.0, 0.0);
	sp_svg_length_unset (&pat->height, SP_SVG_UNIT_NONE, 0.0, 0.0);
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

	if (pat->href) {
		sp_signal_disconnect_by_data (pat->href, pat);
		sp_object_hunref (SP_OBJECT (pat->href), object);
	}

	if (((SPObjectClass *) pattern_parent_class)->release)
		((SPObjectClass *) pattern_parent_class)->release (object);
}

static void
sp_pattern_set (SPObject *object, unsigned int key, const gchar *value)
{
	SPPattern *pat;

	pat = SP_PATTERN (object);

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
		if (pat->href) {
			sp_signal_disconnect_by_data (pat->href, pat);
			pat->href = (SPPattern *) sp_object_hunref (SP_OBJECT (pat->href), object);
		}
		if (value && *value == '#') {
			SPObject *href;
			href = sp_document_lookup_id (object->document, value + 1);
			if (SP_IS_PATTERN (href)) {
				pat->href = (SPPattern *) sp_object_href (href, object);
				//g_signal_connect (G_OBJECT (href), "destroy", G_CALLBACK (sp_pattern_href_destroy), pat);
				g_signal_connect (G_OBJECT (href), "modified", G_CALLBACK (sp_pattern_href_modified), pat);
			}
		}
		sp_object_request_modified (object, SP_OBJECT_MODIFIED_FLAG);
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
	SPPattern *pat;

	pat = SP_PATTERN (object);

	if (((SPObjectClass *) (pattern_parent_class))->child_added)
		(* ((SPObjectClass *) (pattern_parent_class))->child_added) (object, child, ref);

	SPObject *ochild = sp_object_get_child_by_repr(object, child);
	if (SP_IS_ITEM (ochild)) {
		SPPaintServer *ps;
		SPPainter *p;
		unsigned position = sp_item_pos_in_parent(SP_ITEM(ochild));
		/* Huh (Lauris) */
		ps = SP_PAINT_SERVER (pat);
		for (p = ps->painters; p != NULL; p = p->next) {
			SPPatPainter *pp;
			NRArenaItem *ai;
			pp = (SPPatPainter *) p;
			ai = sp_item_invoke_show (SP_ITEM (ochild), pp->arena, pp->dkey, SP_ITEM_REFERENCE_FLAGS);
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

/* fixme: Transformation and stuff (Lauris) */

static void
sp_pattern_update (SPObject *object, SPCtx *ctx, unsigned int flags)
{
	SPPattern *pat;
	SPObject *child;
	GSList *l;

	pat = SP_PATTERN (object);

	if (flags & SP_OBJECT_MODIFIED_FLAG) flags |= SP_OBJECT_PARENT_MODIFIED_FLAG;
	flags &= SP_OBJECT_MODIFIED_CASCADE;

	l = NULL;
	for (child = sp_object_first_child(object) ; child != NULL ; child = SP_OBJECT_NEXT(child) ) {
		sp_object_ref (child, NULL);
		l = g_slist_prepend (l, child);
	}
	l = g_slist_reverse (l);
	while (l) {
		child = SP_OBJECT (l->data);
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
	SPPattern *pat;
	SPObject *child;
	GSList *l;

	pat = SP_PATTERN (object);

	if (flags & SP_OBJECT_MODIFIED_FLAG) flags |= SP_OBJECT_PARENT_MODIFIED_FLAG;
	flags &= SP_OBJECT_MODIFIED_CASCADE;

	l = NULL;
	for (child = sp_object_first_child(object) ; child != NULL; child = SP_OBJECT_NEXT(child) ) {
		sp_object_ref (child, NULL);
		l = g_slist_prepend (l, child);
	}
	l = g_slist_reverse (l);
	while (l) {
		child = SP_OBJECT (l->data);
		l = g_slist_remove (l, child);
		if (flags || (child->mflags & (SP_OBJECT_MODIFIED_FLAG | SP_OBJECT_CHILD_MODIFIED_FLAG))) {
			sp_object_invoke_modified (child, flags);
		}
		sp_object_unref (child, NULL);
	}
}

static void
sp_pattern_href_destroy (SPObject *href, SPPattern *pattern)
{
	pattern->href = (SPPattern *) sp_object_hunref (href, pattern);
	sp_object_request_modified (SP_OBJECT (pattern), SP_OBJECT_MODIFIED_FLAG);
}

static void
sp_pattern_href_modified (SPObject *href, guint flags, SPPattern *pattern)
{
	sp_object_request_modified (SP_OBJECT (pattern), SP_OBJECT_MODIFIED_FLAG);
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

	if (pat->patternUnits == SP_PATTERN_UNITS_OBJECTBOUNDINGBOX) {
		/* BBox to user coordinate system */
		NR::Matrix bbox2user (bbox->x1 - bbox->x0, 0.0, 0.0, bbox->y1 - bbox->y0, bbox->x0, bbox->y0);

		// the final patternTransform, taking into account bbox
		NR::Matrix ps2user = NR::Matrix (&pat->patternTransform) * bbox2user;

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

		//NR::Matrix ps2px = (full_transform * parent_transform.inverse()) * NR::Matrix (&pat->patternTransform) * parent_transform;
		NR::Matrix ps2px = NR::Matrix (&pat->patternTransform) * full_transform;

		ps2px.copyto (&pp->ps2px);
	}

	nr_matrix_invert (&pp->px2ps, &pp->ps2px);

	if (pat->viewBox_set) {
		/* Forget content units at all (lauris) */
		gdouble tmp_x = pat->width.computed / (pat->viewBox.x1 - pat->viewBox.x0);
		gdouble tmp_y = pat->height.computed / (pat->viewBox.y1 - pat->viewBox.y0);

		NR::Matrix vb2ps (tmp_x, 0.0, 0.0, tmp_y, -pat->viewBox.x0 * tmp_x, -pat->viewBox.y0 * tmp_y);

		/* Problem: What to do, if we have mixed lengths and percentages? (Lauris) */
		/* Currently we do ignore percentages at all, but that is not good (Lauris) */

		NR::Matrix vb2us = vb2ps * NR::Matrix (&pat->patternTransform);

		// see (*)
		NR::Matrix pcs2px = vb2us * full_transform;

		pcs2px.copyto (&pp->pcs2px);
	} else {
		NR::Matrix pcs2px;

		/* No viewbox, have to parse units */
		if (pat->patternContentUnits == SP_PATTERN_UNITS_OBJECTBOUNDINGBOX) {
			/* BBox to user coordinate system */
			NR::Matrix bbox2user (bbox->x1 - bbox->x0, 0.0, 0.0, bbox->y1 - bbox->y0, bbox->x0, bbox->y0);

			NR::Matrix pcs2user = NR::Matrix (&pat->patternTransform) * bbox2user;

			// see (*)
			pcs2px = pcs2user * full_transform;
		} else {
			// see (*)
			//pcs2px = (full_transform * parent_transform.inverse()) * NR::Matrix (&pat->patternTransform) * parent_transform;
			pcs2px = NR::Matrix (&pat->patternTransform) * full_transform;
		}

		pcs2px = NR::translate (pat->x.computed, pat->y.computed) * pcs2px; 

		pcs2px.copyto (&pp->pcs2px);
	}

	/* fixme: Create arena */
	/* fixme: Actually we need some kind of constructor function */
	/* fixme: But to do that, we need actual arena implementaion */
	pp->arena = NRArena::create();

	pp->dkey = sp_item_display_key_new (1);

	/* fixme: Create group */
	pp->root = NRArenaGroup::create(pp->arena);

	/* fixme: Show items */
	for (SPPattern *pat_i = pat; pat_i != NULL; pat_i = pat_i->href) {
		for (child = sp_object_first_child(SP_OBJECT(pat_i)) ; child != NULL; child = SP_OBJECT_NEXT(child) ) {
			if (SP_IS_ITEM (child)) {
				NRArenaItem *cai;
				cai = sp_item_invoke_show (SP_ITEM (child), pp->arena, pp->dkey, SP_ITEM_REFERENCE_FLAGS);
				nr_arena_item_append_child (pp->root, cai);
				nr_arena_item_unref (cai);
			}
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

	for (SPPattern *pat_i = pat; pat_i != NULL; pat_i = pat_i->href) {
		for (child = sp_object_first_child(SP_OBJECT(pat_i)) ; child != NULL; child = SP_OBJECT_NEXT(child) ) {
			if (SP_IS_ITEM (child)) {
				g_print ("item %p\n", child);
				sp_item_invoke_hide (SP_ITEM (child), pp->dkey);
			}
		}
	}

	if (pp->root) {
		nr_arena_item_unref (pp->root);
	}

	if (pp->arena) {
		nr_object_unref ((NRObject *) pp->arena);
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

	if (pp->pat->width.computed < NR_EPSILON) return;
	if (pp->pat->height.computed < NR_EPSILON) return;

	/* Find buffer area in gradient space */
	/* fixme: This is suboptimal (Lauris) */

	ba.x0 = pb->area.x0;
	ba.y0 = pb->area.y0;
	ba.x1 = pb->area.x1;
	ba.y1 = pb->area.y1;
	nr_rect_d_matrix_transform (&psa, &ba, &pp->px2ps);

	psa.x0 = floor ((psa.x0 - pp->pat->x.computed) / pp->pat->width.computed);
	psa.y0 = floor ((psa.y0 - pp->pat->y.computed) / pp->pat->height.computed);
	psa.x1 = ceil ((psa.x1 - pp->pat->x.computed) / pp->pat->width.computed);
	psa.y1 = ceil ((psa.y1 - pp->pat->y.computed) / pp->pat->height.computed);

	for (y = psa.y0; y < psa.y1; y++) {
		for (x = psa.x0; x < psa.x1; x++) {
			NRPixBlock ppb;
			double psx, psy;

			psx = x * pp->pat->width.computed;
			psy = y * pp->pat->height.computed;

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

