#define __SP_SHAPE_C__

/*
 * Base class for shapes, including <path> element
 *
 * Author:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * Copyright (C) 1999-2002 Lauris Kaplinski
 * Copyright (C) 2000-2001 Ximian, Inc.
 * Copyright (C) 2004 John Cliff
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <config.h>

#include <string.h>

#include <libnr/nr-pixblock.h>

#include "macros.h"
#include "helper/sp-intl.h"
#include "svg/svg.h"
#include "svg/stringstream.h"
#include "display/nr-arena-shape.h"
#include "uri-references.h"
#include "attributes.h"
#include "print.h"
#include "document.h"
#include "desktop.h"
#include "marker-status.h"
#include "selection.h"
#include "desktop-handles.h"
#include "sp-paint-server.h"
#include "style.h"
#include "sp-root.h"
#include "sp-marker.h"
#include "sp-shape.h"
#include "sp-path.h"
#include "sp-defs.h"

#define noSHAPE_VERBOSE

static void sp_shape_class_init (SPShapeClass *klass);
static void sp_shape_init (SPShape *shape);

static void sp_shape_build (SPObject * object, SPDocument * document, SPRepr * repr);
static void sp_shape_release (SPObject *object);

static void sp_shape_update (SPObject *object, SPCtx *ctx, unsigned int flags);
static void sp_shape_modified (SPObject *object, unsigned int flags);

static void sp_shape_bbox (SPItem *item, NRRect *bbox, const NRMatrix *transform, unsigned int flags);
void sp_shape_print (SPItem * item, SPPrintContext * ctx);
static NRArenaItem *sp_shape_show (SPItem *item, NRArena *arena, unsigned int key, unsigned int flags);
static void sp_shape_hide (SPItem *item, unsigned int key);

static void sp_shape_update_marker_view (SPShape *shape, NRArenaItem *ai);

static SPItemClass *parent_class;

GType
sp_shape_get_type (void)
{
	static GType type = 0;
	if (!type) {
		GTypeInfo info = {
			sizeof (SPShapeClass),
			NULL, NULL,
			(GClassInitFunc) sp_shape_class_init,
			NULL, NULL,
			sizeof (SPShape),
			16,
			(GInstanceInitFunc) sp_shape_init,
			NULL,	/* value_table */
		};
		type = g_type_register_static (SP_TYPE_ITEM, "SPShape", &info, (GTypeFlags)0);
	}
	return type;
}

static void
sp_shape_class_init (SPShapeClass *klass)
{
	SPObjectClass *sp_object_class;
	SPItemClass * item_class;
	SPPathClass * path_class;

	sp_object_class = (SPObjectClass *) klass;
	item_class = (SPItemClass *) klass;
	path_class = (SPPathClass *) klass;

	parent_class = (SPItemClass *)g_type_class_peek_parent (klass);

	sp_object_class->build = sp_shape_build;
	sp_object_class->release = sp_shape_release;
	sp_object_class->update = sp_shape_update;
	sp_object_class->modified = sp_shape_modified;

	item_class->bbox = sp_shape_bbox;
	item_class->print = sp_shape_print;
	item_class->show = sp_shape_show;
	item_class->hide = sp_shape_hide;
}

static void
sp_shape_init (SPShape *shape)
{
	/* Nothing here */
}

static void
sp_shape_build (SPObject *object, SPDocument *document, SPRepr *repr)
{
	SPVersion version;

	version = sp_object_get_sodipodi_version (object);

	if (sp_version_inside_range (version, 0, 0, 0, 25)) {
		SPCSSAttr *css;
		const gchar *val;
		gboolean changed;
		/* Have to check for percentage opacities */
		css = sp_repr_css_attr (repr, "style");
		/* We force style rewrite at moment (Lauris) */
		changed = TRUE;
		val = sp_repr_css_property (css, "opacity", NULL);
		if (val && strchr (val, '%')) {
			Inkscape::SVGOStringStream os;
			os << sp_svg_read_percentage (val, 1.0);
			sp_repr_css_set_property (css, "opacity", os.str().c_str());
			changed = TRUE;
		}
		val = sp_repr_css_property (css, "fill-opacity", NULL);
		if (val && strchr (val, '%')) {
			Inkscape::SVGOStringStream os;
			os << sp_svg_read_percentage (val, 1.0);
			sp_repr_css_set_property (css, "fill-opacity", os.str().c_str());
			changed = TRUE;
		}
		val = sp_repr_css_property (css, "stroke-opacity", NULL);
		if (val && strchr (val, '%')) {
			Inkscape::SVGOStringStream os;
			os << sp_svg_read_percentage (val, 1.0);
			sp_repr_css_set_property (css, "stroke-opacity", os.str().c_str());
			changed = TRUE;
		}
		if (changed) {
		  sp_repr_css_set (repr, css, "style");
		}
		sp_repr_css_attr_unref (css);
	}

	if (((SPObjectClass *) (parent_class))->build) {
	  (*((SPObjectClass *) (parent_class))->build) (object, document, repr);
	}
}

static void
sp_shape_release (SPObject *object)
{
	SPItem *item;
	SPShape *shape;
	SPItemView *v;
	int i;

	item = (SPItem *) object;
	shape = (SPShape *) object;

	for (i=SP_MARKER_LOC_START; i<SP_MARKER_LOC_QTY; i++) {
	  if (shape->marker[i]) {
	    sp_signal_disconnect_by_data (shape->marker[i], object);
	    for (v = item->display; v != NULL; v = v->next) {
	      sp_marker_hide ((SPMarker *) shape->marker[i], NR_ARENA_ITEM_GET_KEY (v->arenaitem) + i);
	    }
	    shape->marker[i] = sp_object_hunref (shape->marker[i], object);
	  }
	}
	if (shape->curve) {
		shape->curve = sp_curve_unref (shape->curve);
	}

	if (((SPObjectClass *) parent_class)->release) {
	  ((SPObjectClass *) parent_class)->release (object);
	}
}

static void
sp_shape_update (SPObject *object, SPCtx *ctx, unsigned int flags)
{
	SPItem *item;
	SPShape *shape;

	item = (SPItem *) object;
	shape = (SPShape *) object;

	if (((SPObjectClass *) (parent_class))->update) {
	  (* ((SPObjectClass *) (parent_class))->update) (object, ctx, flags);
	}

	/* This stanza checks that an object's marker style agrees with
	 * the marker objects it has allocated.  sp_shape_set_marker ensures
	 * that the appropriate marker objects are present (or absent) to
	 * match the style.
	 */
	/* TODO:  It would be nice if this could be done at an earlier level */
	for (int i = 0 ; i < SP_MARKER_LOC_QTY ; i++) {
	    sp_shape_set_marker (object, i, object->style->marker[i].value);
	  }

	if (flags & (SP_OBJECT_STYLE_MODIFIED_FLAG | SP_OBJECT_VIEWPORT_MODIFIED_FLAG)) {
		SPStyle *style;
		style = SP_OBJECT_STYLE (object);
		if (style->stroke_width.unit == SP_CSS_UNIT_PERCENT) {
			SPItemCtx *ictx;
			SPItemView *v;
			double aw;
			ictx = (SPItemCtx *) ctx;
			aw = 1.0 / NR_MATRIX_DF_EXPANSION (&ictx->i2vp);
			style->stroke_width.computed = style->stroke_width.value * aw;
			for (v = ((SPItem *) (shape))->display; v != NULL; v = v->next) {
				nr_arena_shape_set_style ((NRArenaShape *) v->arenaitem, style);
			}
		}
	}

	if (flags & (SP_OBJECT_MODIFIED_FLAG | SP_OBJECT_PARENT_MODIFIED_FLAG)) {
		SPItemView *v;
		NRRect paintbox;
		/* This is suboptimal, because changing parent style schedules recalculation */
		/* But on the other hand - how can we know that parent does not tie style and transform */
		sp_item_invoke_bbox (SP_ITEM (object), &paintbox, NULL, TRUE);
		for (v = SP_ITEM (shape)->display; v != NULL; v = v->next) {
			if (flags & SP_OBJECT_MODIFIED_FLAG) {
				nr_arena_shape_set_path(NR_ARENA_SHAPE(v->arenaitem), shape->curve,(flags & SP_OBJECT_USER_MODIFIED_FLAG_B));
			}
			nr_arena_shape_set_paintbox (NR_ARENA_SHAPE (v->arenaitem), &paintbox);
		}
	}

	/* Note, we're ignoring 'marker' settings, which technically should apply for
	   all three settings.  This should be fixed later such that if 'marker' is
	   specified, then all three should appear. */
	if (shape->curve && (shape->marker[SP_MARKER_LOC_START] 
			     || shape->marker[SP_MARKER_LOC_MID] 
			     || shape->marker[SP_MARKER_LOC_END])) {
		SPItemView *v;
		ArtBpath *bp;
		int nmarker[SP_MARKER_LOC_QTY];
		/* Determine the number of markers needed */
		nmarker[SP_MARKER_LOC_START] = 0;
		nmarker[SP_MARKER_LOC_MID] = 0;
		nmarker[SP_MARKER_LOC_END] = 0;
		for (bp = shape->curve->bpath; bp->code != ART_END; bp++) {
			if ((bp[0].code == ART_MOVETO) || (bp[0].code == ART_MOVETO_OPEN)) {
				nmarker[SP_MARKER_LOC_START] += 1;
			} else if ((bp[1].code != ART_LINETO) && (bp[1].code != ART_CURVETO)) {
				nmarker[SP_MARKER_LOC_END] += 1;
			} else {
				nmarker[SP_MARKER_LOC_MID] += 1;
			}
		}
		/* Dimension marker views */
		for (v = item->display; v != NULL; v = v->next) {

		  if (!v->arenaitem->key) {
		    /* Get enough keys for all, start, mid and end marker types,
		    ** and set this view's arenaitem key to the first of these keys.
		    */
		    NR_ARENA_ITEM_SET_KEY (
		      v->arenaitem,
		      sp_item_display_key_new (SP_MARKER_LOC_QTY)
		      );
		  }
			
		  for (int i = 0 ; i < SP_MARKER_LOC_QTY ; i++) {
			  if (shape->marker[i]) {
			    sp_marker_show_dimension ((SPMarker *) shape->marker[i],
						NR_ARENA_ITEM_GET_KEY (v->arenaitem) + i-SP_MARKER_LOC,
						      nmarker[i]);
			  }
			}
		}
		/* Update marker views */
		for (v = item->display; v != NULL; v = v->next) {
			sp_shape_update_marker_view (shape, v->arenaitem);
		}
	}
}


/**
* Works out whether a marker of a given type is required at a particular
* point on a shape.
*
* \param shape Shape of interest.
* \param m Marker type (e.g. SP_MARKER_LOC_START)
* \param bp Path segment.
* \return 1 if a marker is required here, otherwise 0.
*/
static int
sp_shape_marker_required (SPShape* shape, int m, ArtBpath* bp)
{
    if (shape->marker[m] == NULL)
        return 0;

    if (m == SP_MARKER_LOC_START && (bp->code == ART_MOVETO || bp->code == ART_MOVETO_OPEN))
        return 1;

    if (m == SP_MARKER_LOC_END && (bp[1].code != ART_LINETO) && (bp[1].code != ART_CURVETO))
        return 1;

    if (m == SP_MARKER_LOC_MID && ((bp->code != ART_MOVETO && bp->code != ART_MOVETO_OPEN)
                && ((bp[1].code == ART_LINETO) || (bp[1].code == ART_CURVETO))))
        return 1;

    return 0;
}


/**
* Calculate the transform required to get a marker's path object in the
* right place for particular path segment on a shape.  You should
* call sp_shape_marker_required first to see if a marker is required
* at this point.
*
* \see sp_shape_marker_required.
*
* \param shape Shape which the marker is for.
* \param m Marker type (e.g. SP_MARKER_LOC_START)
* \param bp Path segment which the arrow is for.
* \return Transform matrix.
*/
static NRMatrix
sp_shape_marker_get_transform (SPShape* shape, int m, ArtBpath* bp)
{
    NRMatrix t;
    
    switch (m)
    {
        case SP_MARKER_LOC_START:
        {
			float dx, dy, h;
			if (bp[1].code == ART_LINETO) {
				dx = bp[1].x3 - bp[0].x3;
				dy = bp[1].y3 - bp[0].y3;
			} else if (bp[1].code == ART_CURVETO) {
				dx = bp[1].x1 - bp[0].x3;
				dy = bp[1].y1 - bp[0].y3;
			} else {
				dx = 1.0;
				dy = 0.0;
			}
			h = hypot (dx, dy);
			if (h > 1e-9) {
                t.c[0] = dx / h;
                t.c[1] = dy / h;
                t.c[2] = -dy / h;
                t.c[3] = dx / h;
                t.c[4] = bp->x3;
                t.c[5] = bp->y3;
			} else {
                nr_matrix_set_translate (&t, bp->x3, bp->y3);
			}
            break;
        }

        case SP_MARKER_LOC_END:
        {
			float dx, dy, h;
			if ((bp->code == ART_LINETO) && (bp > shape->curve->bpath)) {
				dx = bp->x3 - (bp - 1)->x3;
				dy = bp->y3 - (bp - 1)->y3;
            } else if (bp->code == ART_CURVETO) {
				dx = bp->x3 - bp->x2;
				dy = bp->y3 - bp->y2;

                /* If the second Bezier control point x2 and the end y3
                ** are coincident, the arrow will not be rotated in a
                ** sensible fashion.  In this case, this code tries to
                ** use the second control point on a previous segment to decide
                ** the arrow's direction.  FIXME: this may not actually
                ** be in spec for SVG...
                */
                if (hypot (dx, dy) < 1e-9 && (bp > shape->curve->bpath)) {
                    dx = (bp - 1)->x2 - bp->x3;
                    dy = (bp - 1)->y2 - bp->y3;
                }
			} else {
				dx = 1.0;
				dy = 0.0;
			}
			h = hypot (dx, dy);
			if (h > 1e-9) {
                t.c[0] = dx / h;
                t.c[1] = dy / h;
                t.c[2] = -dy / h;
                t.c[3] = dx / h;
                t.c[4] = bp->x3;
                t.c[5] = bp->y3;
			} else {
                nr_matrix_set_translate (&t, bp->x3, bp->y3);
            }
            break;
			}

        /* the following works on the average of the incoming
        and outgoing curve tangents.*/

        case SP_MARKER_LOC_MID:
        {
            float dx, dy, h;
            if ((bp->code == ART_LINETO) && (bp > shape->curve->bpath)) {
                dx = ((bp->x3 - (bp - 1)->x3)+(bp[1].x3 - bp[0].x3))/2;
                dy = ((bp->y3 - (bp - 1)->y3)+(bp[1].y3 - bp[0].y3))/2;
            } else if (bp->code == ART_CURVETO) {
                dx = ((bp->x3 - bp->x2) + (bp[1].x1 - bp[0].x3))/2;
                dy = ((bp->y3 - bp->y2) + (bp[1].y1 - bp[0].y3))/2;

                /* If the second Bezier control point x2 and the end y3
                ** are coincident, the arrow will not be rotated in a
                ** sensible fashion.  In this case, this code tries to
                ** use the second control point on a previous segment to decide
                ** the arrow's direction.  FIXME: this may not actually
                ** be in spec for SVG...
                */
                if (hypot (dx, dy) < 1e-9 && (bp > shape->curve->bpath)) {
                    dx = (bp - 1)->x2 - bp->x3 ;
                    dy = (bp - 1)->y2 - bp->y3 ;

                }
            } else {
                dx = 1.0;
                dy = 0.0;
            }
            h = hypot (dx, dy);
            if (h > 1e-9) {
                t.c[0] = dx / h;
                t.c[1] = dy / h;
                t.c[2] = -dy / h;
                t.c[3] = dx / h;
                t.c[4] = bp->x3;
                t.c[5] = bp->y3;
            } else {
            nr_matrix_set_translate (&t, bp->x3, bp->y3);
            }
            break;
    }

    }

    return t;
}

  

/* Marker views have to be scaled already */

static void
sp_shape_update_marker_view (SPShape *shape, NRArenaItem *ai)
{
	SPStyle *style;
	int nstart, nmid, nend;
	ArtBpath *bp;

	style = ((SPObject *) shape)->style;

	marker_status("sp_shape_update_marker_view:  Updating views of markers");
	nstart = 0;
	nmid = 0;
	nend = 0;
	for (bp = shape->curve->bpath; bp->code != ART_END; bp++) {
		NRMatrix m;
                if (sp_shape_marker_required (shape, SP_MARKER_LOC_START, bp)) {
                    m = sp_shape_marker_get_transform (shape, SP_MARKER_LOC_START, bp);
                    sp_marker_show_instance ((SPMarker *) shape->marker[SP_MARKER_LOC_START], ai,
                                             NR_ARENA_ITEM_GET_KEY (ai) + SP_MARKER_LOC_START,
                                             nstart,
                                             &m, style->stroke_width.computed);
                    nstart += 1;
		} else if (sp_shape_marker_required (shape, SP_MARKER_LOC_END, bp)) {
                    m = sp_shape_marker_get_transform (shape, SP_MARKER_LOC_END, bp);
			sp_marker_show_instance ((SPMarker *) shape->marker[SP_MARKER_LOC_END], ai,
						 NR_ARENA_ITEM_GET_KEY (ai) + SP_MARKER_LOC_END
						 - SP_MARKER_LOC, nend,
						 &m, style->stroke_width.computed);
			nend += 1;
		} else if (sp_shape_marker_required (shape, SP_MARKER_LOC_MID, bp)) {
                    m = sp_shape_marker_get_transform (shape, SP_MARKER_LOC_MID, bp);
			sp_marker_show_instance ((SPMarker *) shape->marker[SP_MARKER_LOC_MID], ai,
						 NR_ARENA_ITEM_GET_KEY (ai) + SP_MARKER_LOC_MID
						 - SP_MARKER_LOC, nmid,
						 &m, style->stroke_width.computed);
			nmid += 1;
		}
	}
}

static void
sp_shape_modified (SPObject *object, unsigned int flags)
{
	SPShape *shape;

	shape = SP_SHAPE (object);

	if (((SPObjectClass *) (parent_class))->modified) {
	  (* ((SPObjectClass *) (parent_class))->modified) (object, flags);
	}

	if (flags & SP_OBJECT_STYLE_MODIFIED_FLAG) {
		SPItemView *v;
		for (v = SP_ITEM (shape)->display; v != NULL; v = v->next) {
			nr_arena_shape_set_style (NR_ARENA_SHAPE (v->arenaitem), object->style);
		}
	}
}

static void sp_shape_bbox(SPItem *item, NRRect *bbox, NRMatrix const *transform, unsigned flags)
{
	SPShape *shape;

	shape = SP_SHAPE (item);

	if (shape->curve) {
    NRRect  cbbox;
		NRBPath bp;
		bp.path = SP_CURVE_BPATH (shape->curve);
    cbbox.x0 = cbbox.y0 = NR_HUGE;
    cbbox.x1 = cbbox.y1 = -NR_HUGE;
		nr_path_matrix_f_bbox_f_union(&bp, transform, &cbbox, 0.25);
    SPStyle* style=SP_OBJECT_STYLE (item);
    if (style->stroke.type != SP_PAINT_TYPE_NONE) {
      float width, scale;
      scale = NR_MATRIX_DF_EXPANSION (transform);
      if ( fabsf(style->stroke_width.computed * scale) > 0.01 ) { // sinon c'est 0=oon veut pas de bord
        width = MAX (0.125, style->stroke_width.computed * scale);
        if ( fabs(cbbox.x1-cbbox.x0) > -0.00001 && fabs(cbbox.y1-cbbox.y0) > -0.00001 ) {
          cbbox.x0-=0.5*width;
          cbbox.x1+=0.5*width;
          cbbox.y0-=0.5*width;
          cbbox.y1+=0.5*width;
        }      
      }
    }
    if ( fabs(cbbox.x1-cbbox.x0) > -0.00001 && fabs(cbbox.y1-cbbox.y0) > -0.00001 ) {
      NRRect tbbox=*bbox;
      nr_rect_d_union (bbox, &cbbox, &tbbox);
    }
	}
}

void
sp_shape_print (SPItem *item, SPPrintContext *ctx)
{
	SPShape *shape;
	NRRect pbox, dbox, bbox;
	NRMatrix i2d;

	shape = SP_SHAPE (item);

	if (!shape->curve) return;

	/* fixme: Think (Lauris) */
	sp_item_invoke_bbox (item, &pbox, NULL, TRUE);
	dbox.x0 = 0.0;
	dbox.y0 = 0.0;
	dbox.x1 = sp_document_width (SP_OBJECT_DOCUMENT (item));
	dbox.y1 = sp_document_height (SP_OBJECT_DOCUMENT (item));
	sp_item_bbox_desktop (item, &bbox);
	sp_item_i2d_affine (item, &i2d);

	if (SP_OBJECT_STYLE (item)->fill.type != SP_PAINT_TYPE_NONE) {
		NRBPath bp;
		bp.path = shape->curve->bpath;
		sp_print_fill (ctx, &bp, &i2d, SP_OBJECT_STYLE (item), &pbox, &dbox, &bbox);
	}

	if (SP_OBJECT_STYLE (item)->stroke.type != SP_PAINT_TYPE_NONE) {
		NRBPath bp;
		bp.path = shape->curve->bpath;
		sp_print_stroke (ctx, &bp, &i2d, SP_OBJECT_STYLE (item), &pbox, &dbox, &bbox);
	}

        for (ArtBpath* bp = shape->curve->bpath; bp->code != ART_END; bp++) {
            for (int m = SP_MARKER_LOC_START; m < SP_MARKER_LOC_QTY; m++) {
                if (sp_shape_marker_required (shape, m, bp)) {

                    SPMarker* marker = SP_MARKER (shape->marker[m]);
                    SPItem* marker_path = SP_ITEM (shape->marker[m]->children);

                    NRMatrix tr = sp_shape_marker_get_transform (shape, m, bp);
                    nr_matrix_multiply (&tr, &marker->c2p, &tr);

                    NRMatrix old_tr = marker_path->transform;
                    marker_path->transform = tr;
                    sp_item_invoke_print (marker_path, ctx);
                    marker_path->transform = old_tr;
                }
            }
        }
}

static NRArenaItem *
sp_shape_show (SPItem *item, NRArena *arena, unsigned int key, unsigned int flags)
{
	SPObject *object;
	SPShape *shape;
	NRRect paintbox;
	NRArenaItem *arenaitem;

	object = SP_OBJECT (item);
	shape = SP_SHAPE (item);

	arenaitem = nr_arena_item_new (arena, NR_TYPE_ARENA_SHAPE);
	nr_arena_shape_set_style (NR_ARENA_SHAPE (arenaitem), object->style);
	nr_arena_shape_set_path(NR_ARENA_SHAPE(arenaitem), shape->curve,false);
	sp_item_invoke_bbox (item, &paintbox, NULL, TRUE);
	nr_arena_shape_set_paintbox (NR_ARENA_SHAPE (arenaitem), &paintbox);

	if (shape->curve && (shape->marker[SP_MARKER_LOC_START] ||
			     shape->marker[SP_MARKER_LOC_MID] || 
			     shape->marker[SP_MARKER_LOC_END])) {
		ArtBpath *bp;
		int nstart, nmid, nend;
		/* Determine the number of markers needed */
		nstart = 0;
		nmid = 0;
		nend = 0;
		for (bp = shape->curve->bpath; bp->code != ART_END; bp++) {
			if ((bp[0].code == ART_MOVETO) || (bp[0].code == ART_MOVETO_OPEN)) {
				nstart += 1;
			} else if ((bp[1].code != ART_LINETO) && (bp[1].code != ART_CURVETO)) {
				nend += 1;
			} else {
				nmid += 1;
			}
		}
		/* Dimension marker views */
		if (!arenaitem->key) {
		  NR_ARENA_ITEM_SET_KEY (arenaitem, sp_item_display_key_new (3));
		}
		if (shape->marker[SP_MARKER_LOC_START]) {
		  sp_marker_show_dimension ((SPMarker *) shape->marker[SP_MARKER_LOC_START],
					    NR_ARENA_ITEM_GET_KEY (arenaitem) 
					    + SP_MARKER_LOC_START - SP_MARKER_LOC,
					    nstart);
		}
		if (shape->marker[SP_MARKER_LOC_MID]) {
		  sp_marker_show_dimension ((SPMarker *) shape->marker[SP_MARKER_LOC_MID],
					    NR_ARENA_ITEM_GET_KEY (arenaitem) 
					    + SP_MARKER_LOC_MID - SP_MARKER_LOC,
					    nmid);
		}
		if (shape->marker[SP_MARKER_LOC_END]) {
		  sp_marker_show_dimension ((SPMarker *) shape->marker[SP_MARKER_LOC_END],
					    NR_ARENA_ITEM_GET_KEY (arenaitem) 
					    + SP_MARKER_LOC_END - SP_MARKER_LOC,
					    nend);
		}
		/* Update marker views */
		sp_shape_update_marker_view (shape, arenaitem);
	}

	return arenaitem;
}

static void
sp_shape_hide (SPItem *item, unsigned int key)
{
	SPShape *shape;
	SPItemView *v;
	int i;

	shape = (SPShape *) item;

	for (i=0; i<SP_MARKER_LOC_QTY; i++) {
	  if (shape->marker[i]) {
	    for (v = item->display; v != NULL; v = v->next) {
                if (key == v->key) {
	      sp_marker_hide ((SPMarker *) shape->marker[i], 
                                    NR_ARENA_ITEM_GET_KEY (v->arenaitem) + i);
                }
	    }
	  }
	}
	
	if (((SPItemClass *) parent_class)->hide) {
	  ((SPItemClass *) parent_class)->hide (item, key);
	}
}

/* Marker stuff */

static void
sp_shape_marker_release (SPObject *marker, SPShape *shape)
{
	SPItem *item;
	int i;

	item = (SPItem *) shape;

	marker_status("sp_shape_marker_release:  Releasing markers");
	for (i = SP_MARKER_LOC_START; i < SP_MARKER_LOC_QTY; i++) {
	  if (marker == shape->marker[i]) {
	    SPItemView *v;
	    /* Hide marker */
	    for (v = item->display; v != NULL; v = v->next) {
	      sp_marker_hide ((SPMarker *) (shape->marker[i]), NR_ARENA_ITEM_GET_KEY (v->arenaitem) + i);
	      /* fixme: Do we need explicit remove here? (Lauris) */
	      /* nr_arena_item_set_mask (v->arenaitem, NULL); */
	    }
	    /* Detach marker */
	    sp_signal_disconnect_by_data (shape->marker[i], item);
	    shape->marker[i] = sp_object_hunref (shape->marker[i], item);
	  }
	}
}

static void
sp_shape_marker_modified (SPObject *marker, guint flags, SPItem *item)
{
	/* I think mask does update automagically */
	/* g_warning ("Item %s mask %s modified", SP_OBJECT_ID (item), SP_OBJECT_ID (mask)); */
}

void
sp_shape_set_marker (SPObject *object, unsigned int key, const gchar *value)
{
	SPItem *item;
	SPShape *shape;
	SPObject *mrk;

	item = (SPItem *) object;
	shape = (SPShape *) object;

        if (key < SP_MARKER_LOC_START || key > SP_MARKER_LOC_END) {
	  return;
	}

	mrk = sp_uri_reference_resolve (SP_OBJECT_DOCUMENT (object), value);
	if (mrk != shape->marker[key]) {
	  if (shape->marker[key]) {
	    SPItemView *v;
	    /* Detach marker */
	    sp_signal_disconnect_by_data (shape->marker[key], item);
	    /* Hide marker */
	    for (v = item->display; v != NULL; v = v->next) {
	      sp_marker_hide ((SPMarker *) (shape->marker[key]), 
			      NR_ARENA_ITEM_GET_KEY (v->arenaitem) + key);
	      /* fixme: Do we need explicit remove here? (Lauris) */
	      /* nr_arena_item_set_mask (v->arenaitem, NULL); */
	    }
	    shape->marker[key] = sp_object_hunref (shape->marker[key], object);
	  }
	  if (SP_IS_MARKER (mrk)) {
	    shape->marker[key] = sp_object_href (mrk, object);
	    g_signal_connect (G_OBJECT (shape->marker[key]), "release", 
			      G_CALLBACK (sp_shape_marker_release), shape);
	    g_signal_connect (G_OBJECT (shape->marker[key]), "modified", 
			      G_CALLBACK (sp_shape_marker_modified), shape);
	  }
	  sp_object_request_update (object, SP_OBJECT_MODIFIED_FLAG);
	}
}



/* Shape section */

void
sp_shape_set_shape (SPShape *shape)
{
	g_return_if_fail (shape != NULL);
	g_return_if_fail (SP_IS_SHAPE (shape));

	if (SP_SHAPE_CLASS (G_OBJECT_GET_CLASS (shape))->set_shape) {
	  SP_SHAPE_CLASS (G_OBJECT_GET_CLASS (shape))->set_shape (shape);
	}
}

void
sp_shape_set_curve (SPShape *shape, SPCurve *curve, unsigned int owner)
{
	if (shape->curve) {
		shape->curve = sp_curve_unref (shape->curve);
	}
	if (curve) {
		if (owner) {
			shape->curve = sp_curve_ref (curve);
		} else {
			shape->curve = sp_curve_copy (curve);
		}
	}
	sp_object_request_update (SP_OBJECT (shape), SP_OBJECT_MODIFIED_FLAG);
}

/* Return duplicate of curve or NULL */
SPCurve *
sp_shape_get_curve (SPShape *shape)
{
	if (shape->curve) {
		return sp_curve_copy (shape->curve);
	}
	return NULL;
}

/* NOT FOR GENERAL PUBLIC UNTIL SORTED OUT (Lauris) */
void
sp_shape_set_curve_insync (SPShape *shape, SPCurve *curve, unsigned int owner)
{
	if (shape->curve) {
		shape->curve = sp_curve_unref (shape->curve);
	}
	if (curve) {
		if (owner) {
			shape->curve = sp_curve_ref (curve);
		} else {
			shape->curve = sp_curve_copy (curve);
		}
	}
}


/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=c++:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :
