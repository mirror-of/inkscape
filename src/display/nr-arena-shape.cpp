#define __NR_ARENA_SHAPE_C__

/*
 * RGBA display list system for inkscape
 *
 * Author:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * Copyright (C) 2001-2002 Lauris Kaplinski
 * Copyright (C) 2001 Ximian, Inc.
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */


#include <math.h>
#include <string.h>
#include <glib.h>
#include <libnr/nr-rect.h>
#include <libnr/nr-matrix.h>
#include <libnr/nr-path.h>
#include <libnr/nr-pixops.h>
#include <libnr/nr-blit.h>
#include <libnr/nr-stroke.h>
#include <libnr/nr-svp-render.h>

#include <libnr/nr-svp-private.h>

#include <libart_lgpl/art_misc.h>
#include <libart_lgpl/art_bpath.h>
#include <libart_lgpl/art_vpath_bpath.h>
#include <libart_lgpl/art_vpath.h>
#include <libart_lgpl/art_svp.h>
#include <libart_lgpl/art_svp_vpath_stroke.h>

#include "../style.h"
#include "nr-arena.h"
#include "nr-arena-shape.h"

#ifdef test_liv
#include "../livarot/Shape.h"
#include "../livarot/Path.h"
#include "../livarot/AlphaLigne.h"
#include "../livarot/Ligne.h"
#include "../livarot/BitLigne.h"
#include "../libnr/nr-matrix-ops.h"

//int  showRuns=0;
void nr_pixblock_render_shape_mask_or (NRPixBlock &m,Shape* theS);
#endif

static void nr_arena_shape_class_init (NRArenaShapeClass *klass);
static void nr_arena_shape_init (NRArenaShape *shape);
static void nr_arena_shape_finalize (NRObject *object);

static NRArenaItem *nr_arena_shape_children (NRArenaItem *item);
static void nr_arena_shape_add_child (NRArenaItem *item, NRArenaItem *child, NRArenaItem *ref);
static void nr_arena_shape_remove_child (NRArenaItem *item, NRArenaItem *child);
static void nr_arena_shape_set_child_position (NRArenaItem *item, NRArenaItem *child, NRArenaItem *ref);

static guint nr_arena_shape_update (NRArenaItem *item, NRRectL *area, NRGC *gc, guint state, guint reset);
static unsigned int nr_arena_shape_render (NRArenaItem *item, NRRectL *area, NRPixBlock *pb, unsigned int flags);
static guint nr_arena_shape_clip (NRArenaItem *item, NRRectL *area, NRPixBlock *pb);
static NRArenaItem *nr_arena_shape_pick (NRArenaItem *item, NR::Point p, double delta, unsigned int sticky);

static NRArenaItemClass *shape_parent_class;

NRType
nr_arena_shape_get_type (void)
{
	static NRType type = 0;
	if (!type) {
		type = nr_object_register_type (NR_TYPE_ARENA_ITEM,
						"NRArenaShape",
						sizeof (NRArenaShapeClass),
						sizeof (NRArenaShape),
						(void (*) (NRObjectClass *)) nr_arena_shape_class_init,
						(void (*) (NRObject *)) nr_arena_shape_init);
	}
	return type;
}

static void
nr_arena_shape_class_init (NRArenaShapeClass *klass)
{
	NRObjectClass *object_class;
	NRArenaItemClass *item_class;

	object_class = (NRObjectClass *) klass;
	item_class = (NRArenaItemClass *) klass;

	shape_parent_class = (NRArenaItemClass *)  ((NRObjectClass *) klass)->parent;

	object_class->finalize = nr_arena_shape_finalize;

	item_class->children = nr_arena_shape_children;
	item_class->add_child = nr_arena_shape_add_child;
	item_class->set_child_position = nr_arena_shape_set_child_position;
	item_class->remove_child = nr_arena_shape_remove_child;
	item_class->update = nr_arena_shape_update;
	item_class->render = nr_arena_shape_render;
	item_class->clip = nr_arena_shape_clip;
	item_class->pick = nr_arena_shape_pick;
}

static void
nr_arena_shape_init (NRArenaShape *shape)
{
	shape->curve = NULL;
	shape->style = NULL;
	shape->paintbox.x0 = shape->paintbox.y0 = 0.0F;
	shape->paintbox.x1 = shape->paintbox.y1 = 256.0F;

	nr_matrix_set_identity (&shape->ctm);
	shape->fill_painter = NULL;
	shape->stroke_painter = NULL;
#ifndef test_liv
	shape->fill_svp = NULL;
	shape->stroke_svp = NULL;
#else
	shape->cached_fill = NULL;
	shape->cached_stroke = NULL;
	shape->fill_shp = NULL;
	shape->stroke_shp = NULL;
#endif
}

static void
nr_arena_shape_finalize (NRObject *object)
{
	NRArenaItem *item = (NRArenaItem *) object;
	NRArenaShape *shape = (NRArenaShape *) (object);

	while (shape->markers) {
		shape->markers = nr_arena_item_detach_unref (item, shape->markers);
	}

#ifndef test_liv
	if (shape->fill_svp) nr_svp_free (shape->fill_svp);
	if (shape->stroke_svp) nr_svp_free (shape->stroke_svp);
#else
	if (shape->fill_shp) delete shape->fill_shp;
	if (shape->stroke_shp) delete shape->stroke_shp;
	if (shape->cached_fill) delete shape->cached_fill;
	if (shape->cached_stroke) delete shape->cached_stroke;
#endif
	if (shape->fill_painter) sp_painter_free (shape->fill_painter);
	if (shape->stroke_painter) sp_painter_free (shape->stroke_painter);
	if (shape->style) sp_style_unref (shape->style);
	if (shape->curve) sp_curve_unref (shape->curve);

	((NRObjectClass *) shape_parent_class)->finalize (object);
}

static NRArenaItem *
nr_arena_shape_children (NRArenaItem *item)
{
	NRArenaShape *shape = (NRArenaShape *) item;

	return shape->markers;
}

static void
nr_arena_shape_add_child (NRArenaItem *item, NRArenaItem *child, NRArenaItem *ref)
{
	NRArenaShape *shape = (NRArenaShape *) item;

	if (!ref) {
		shape->markers = nr_arena_item_attach_ref (item, child, NULL, shape->markers);
	} else {
		ref->next = nr_arena_item_attach_ref (item, child, ref, ref->next);
	}

	nr_arena_item_request_update (item, NR_ARENA_ITEM_STATE_ALL, FALSE);
}

static void
nr_arena_shape_remove_child (NRArenaItem *item, NRArenaItem *child)
{
	NRArenaShape *shape = (NRArenaShape *) item;

	if (child->prev) {
		nr_arena_item_detach_unref (item, child);
	} else {
		shape->markers = nr_arena_item_detach_unref (item, child);
	}

	nr_arena_item_request_update (item, NR_ARENA_ITEM_STATE_ALL, FALSE);
}

static void
nr_arena_shape_set_child_position (NRArenaItem *item, NRArenaItem *child, NRArenaItem *ref)
{
	NRArenaShape *shape = (NRArenaShape *) item;

	nr_arena_item_ref (child);

	if (child->prev) {
		nr_arena_item_detach_unref (item, child);
	} else {
		shape->markers = nr_arena_item_detach_unref (item, child);
	}

	if (!ref) {
		shape->markers = nr_arena_item_attach_ref (item, child, NULL, shape->markers);
	} else {
		ref->next = nr_arena_item_attach_ref (item, child, ref, ref->next);
	}

	nr_arena_item_unref (child);

	nr_arena_item_request_render (child);
}

#include "enums.h"
void nr_arena_shape_update_stroke(NRArenaShape *shape,NRGC* gc);
void nr_arena_shape_update_fill(NRArenaShape *shape,NRGC *gc);
void nr_arena_shape_add_bboxes(NRArenaShape* shape,NRRect &bbox);

static guint
nr_arena_shape_update (NRArenaItem *item, NRRectL *area, NRGC *gc, guint state, guint reset)
{
	NRRect bbox;

	NRArenaShape *shape = NR_ARENA_SHAPE (item);
	SPStyle *style = shape->style;

	unsigned int beststate = NR_ARENA_ITEM_STATE_ALL;

	unsigned int newstate;
	for (NRArenaItem *child = shape->markers; child != NULL; child = child->next) {
		newstate = nr_arena_item_invoke_update (child, area, gc, state, reset);
		beststate = beststate & newstate;
	}

	if (!(state & NR_ARENA_ITEM_STATE_RENDER)) {
		/* We do not have to create rendering structures */
		shape->ctm = gc->transform;
		if (state & NR_ARENA_ITEM_STATE_BBOX) {
			if (shape->curve) {
				NRBPath bp;
				/* fixme: */
				bbox.x0 = bbox.y0 = NR_HUGE;
				bbox.x1 = bbox.y1 = -NR_HUGE;
				bp.path = shape->curve->bpath;
				nr_path_matrix_f_bbox_f_union(&bp, &gc->transform, &bbox, 1.0);
				item->bbox.x0 = (gint32)(bbox.x0 - 1.0F);
				item->bbox.y0 = (gint32)(bbox.y0 - 1.0F);
				item->bbox.x1 = (gint32)(bbox.x1 + 1.9999F);
				item->bbox.y1 = (gint32)(bbox.y1 + 1.9999F);
			}
			if (beststate & NR_ARENA_ITEM_STATE_BBOX) {
				for (NRArenaItem *child = shape->markers; child != NULL; child = child->next) {
					nr_rect_l_union (&item->bbox, &item->bbox, &child->bbox);
				}
			}
		}
		return (state | item->state);
	}

	/* Request repaint old area if needed */
	/* fixme: Think about it a bit (Lauris) */
	/* fixme: Thios is only needed, if actually rendered/had svp (Lauris) */
	if (!nr_rect_l_test_empty (&item->bbox)) {
		nr_arena_request_render_rect (item->arena, &item->bbox);
		nr_rect_l_set_empty (&item->bbox);
	}
  
#ifdef test_liv
  shape->delayed_shp=true;
  shape->ctm = gc->transform;
  bbox.x0 = bbox.y0 = NR_HUGE;
  bbox.x1 = bbox.y1 = -NR_HUGE;
  
  if (shape->curve) {
    NRBPath bp;
    /* fixme: */
    bbox.x0 = bbox.y0 = NR_HUGE;
    bbox.x1 = bbox.y1 = -NR_HUGE;
    bp.path = shape->curve->bpath;
    nr_path_matrix_f_bbox_f_union(&bp, &gc->transform, &bbox, 1.0);
    if (style->stroke.type != SP_PAINT_TYPE_NONE) {
      float width, scale;
      scale = NR_MATRIX_DF_EXPANSION (&gc->transform);
      width = MAX (0.125, style->stroke_width.computed * scale);
      if ( fabsf(style->stroke_width.computed * scale) > 0.01 ) { // sinon c'est 0=oon veut pas de bord
        bbox.x0-=width;
        bbox.x1+=width;
        bbox.y0-=width;
        bbox.y1+=width;
      }      
      // those pesky miters, now
      float miterMax=width*shape->style->stroke_miterlimit.value;
      if ( miterMax > 0.01 ) {
        // grunt mode. we should compute the various miters instead (one for each point on the curve)
        bbox.x0-=miterMax;
        bbox.x1+=miterMax;
        bbox.y0-=miterMax;
        bbox.y1+=miterMax;        
      }
    }
  } else {
  }
  shape->approx_bbox.x0 = (gint32)(bbox.x0 - 1.0F);
  shape->approx_bbox.y0 = (gint32)(bbox.y0 - 1.0F);
  shape->approx_bbox.x1 = (gint32)(bbox.x1 + 1.9999F);
  shape->approx_bbox.y1 = (gint32)(bbox.y1 + 1.9999F);
  if ( area && nr_rect_l_test_intersect (area, &shape->approx_bbox) ) shape->delayed_shp=false;
#endif

	/* Release state data */
	if (TRUE || !nr_matrix_test_transform_equal (&gc->transform, &shape->ctm, NR_EPSILON)) {
		/* Concept test */
#ifndef test_liv
		if (shape->fill_svp) {
			nr_svp_free (shape->fill_svp);
			shape->fill_svp = NULL;
		}
#else
		if (shape->fill_shp) {
			delete shape->fill_shp;
			shape->fill_shp = NULL;
		}
#endif
	}
#ifndef test_liv
	if (shape->stroke_svp) {
		nr_svp_free (shape->stroke_svp);
		shape->stroke_svp = NULL;
	}
#else
	if (shape->stroke_shp) {
		delete shape->stroke_shp;
		shape->stroke_shp = NULL;
	}
#endif
	if (shape->fill_painter) {
		sp_painter_free (shape->fill_painter);
		shape->fill_painter = NULL;
	}
	if (shape->stroke_painter) {
		sp_painter_free (shape->stroke_painter);
		shape->stroke_painter = NULL;
	}

	if (!shape->curve || !shape->style) return NR_ARENA_ITEM_STATE_ALL;
	if (sp_curve_is_empty (shape->curve)) return NR_ARENA_ITEM_STATE_ALL;
	if ((shape->style->fill.type == SP_PAINT_TYPE_NONE) && (shape->style->stroke.type == SP_PAINT_TYPE_NONE)) return NR_ARENA_ITEM_STATE_ALL;

	/* Build state data */
#ifdef test_liv
  if ( shape->delayed_shp ) {
    item->bbox=shape->approx_bbox;
  } else {
    nr_arena_shape_update_stroke(shape,gc);
    nr_arena_shape_update_fill(shape,gc);

  	bbox.x0 = bbox.y0 = bbox.x1 = bbox.y1 = 0.0;
    nr_arena_shape_add_bboxes(shape,bbox);
    
    shape->approx_bbox.x0 = (gint32)(bbox.x0 - 1.0F);
    shape->approx_bbox.y0 = (gint32)(bbox.y0 - 1.0F);
    shape->approx_bbox.x1 = (gint32)(bbox.x1 + 1.9999F);
    shape->approx_bbox.y1 = (gint32)(bbox.y1 + 1.9999F);
  } 
#else 
	bbox.x0 = bbox.y0 = bbox.x1 = bbox.y1 = 0.0;
 	if (shape->stroke_svp && shape->stroke_svp->length > 0) {
		nr_svp_bbox (shape->stroke_svp, &bbox, FALSE);
	}
	if (shape->fill_svp && shape->fill_svp->length > 0) {
		nr_svp_bbox (shape->fill_svp, &bbox, FALSE);
	}
#endif
  
	if (nr_rect_f_test_empty (&bbox)) return NR_ARENA_ITEM_STATE_ALL;

	item->bbox.x0 = (gint32)(bbox.x0 - 1.0F);
	item->bbox.y0 = (gint32)(bbox.y0 - 1.0F);
	item->bbox.x1 = (gint32)(bbox.x1 + 1.0F);
	item->bbox.y1 = (gint32)(bbox.y1 + 1.0F);
	nr_arena_request_render_rect (item->arena, &item->bbox);

	item->render_opacity = TRUE;
	if (shape->style->fill.type == SP_PAINT_TYPE_PAINTSERVER) {
		shape->fill_painter = sp_paint_server_painter_new (SP_STYLE_FILL_SERVER (shape->style),
														   NR_MATRIX_D_TO_DOUBLE (&gc->transform), 
														   &shape->paintbox);
		item->render_opacity = FALSE;
	}
	if (shape->style->stroke.type == SP_PAINT_TYPE_PAINTSERVER) {
		shape->stroke_painter = sp_paint_server_painter_new (SP_STYLE_STROKE_SERVER (shape->style),
															 NR_MATRIX_D_TO_DOUBLE (&gc->transform), 
															 &shape->paintbox);
		item->render_opacity = FALSE;
	}
	if ( item->render_opacity == TRUE && style->fill.type != SP_PAINT_TYPE_NONE && style->stroke.type != SP_PAINT_TYPE_NONE ) {
		// don't merge item opacity with paint opacity if there is a stroke on the fill
		item->render_opacity = FALSE;    
	}
	
	if (beststate & NR_ARENA_ITEM_STATE_BBOX) {
		for (NRArenaItem *child = shape->markers; child != NULL; child = child->next) {
			nr_rect_l_union (&item->bbox, &item->bbox, &child->bbox);
		}
	}

	return NR_ARENA_ITEM_STATE_ALL;
}

void
nr_arena_shape_update_fill(NRArenaShape *shape,NRGC *gc)
{
#ifdef test_liv
  shape->delayed_shp=false;
#endif
  if (shape->style->fill.type != SP_PAINT_TYPE_NONE) {
		if ((shape->curve->end > 2) || (shape->curve->bpath[1].code == ART_CURVETO)) {
#ifndef test_liv
			if (TRUE || !shape->fill_svp) {
#else
        if (TRUE || !shape->fill_shp) {
#endif
          unsigned int windrule;
          windrule = (shape->style->fill_rule.value == SP_WIND_RULE_EVENODD) ? NR_WIND_RULE_EVENODD : NR_WIND_RULE_NONZERO;
#ifndef test_liv
          NRSVL *svl = nr_svl_from_art_bpath(shape->curve->bpath, &gc->transform, windrule, TRUE, 0.25);
          shape->fill_svp = nr_svp_from_svl (svl, NULL);
          nr_svl_free_list (svl);
#else
          NRMatrix  cached_to_new;
          int       isometry=0;
          if ( shape->cached_fill ) {
            NR::Matrix   oMat(shape->cached_fctm);
            NR::Matrix   nMat(gc->transform);
            oMat=oMat.inverse();
            NR::Matrix   p=oMat*nMat;
            cached_to_new=p;
            NR::Matrix   tp;
            // trasnposition
            tp[0]=p[0];
            tp[1]=p[2];
            tp[2]=p[1];
            tp[3]=p[3];
            NR::Matrix   isom=tp*p;
            if ( fabs(isom[0]-1.0) < 0.01 && fabs(isom[3]-1.0) < 0.01 &&
                 fabs(isom[1]) < 0.01 && fabs(isom[2]) < 0.01 ) {
              // the changed wrt the cached version is an isometry-> no need to recompute
              // the uncrossed polygon
              if ( p.det() < 0 ) isometry=-1; else isometry=1;
            } else {
              isom=0;
            }
          }
          if ( isometry == 0 ) {
            if ( shape->cached_fill == NULL ) shape->cached_fill=new Shape;
            shape->cached_fill->Reset();
            
            Path*  thePath=new Path;
            Shape* theShape=new Shape;
            {
              NR::Matrix   tempMat(gc->transform);
              thePath->LoadArtBPath(shape->curve->bpath,tempMat,true);
            }
            thePath->Convert(1.0);
            thePath->Fill(theShape,0);
            if ( shape->style->fill_rule.value == SP_WIND_RULE_EVENODD ) {
              if ( shape->cached_fill->ConvertToShape(theShape,fill_oddEven) ) {
              }
            } else {
              if ( shape->cached_fill->ConvertToShape(theShape,fill_nonZero) ) {
              }
            }
            shape->cached_fctm=gc->transform;
            delete theShape;
            delete thePath;
            if ( shape->fill_shp == NULL )
				shape->fill_shp = new Shape;
            shape->fill_shp->Copy(shape->cached_fill);
          } else if ( isometry != 0 ) {
			  if ( shape->fill_shp == NULL )
				  shape->fill_shp=new Shape;
			  shape->fill_shp->Reset(shape->cached_fill->nbPt,
									 shape->cached_fill->nbAr);
			  for (int i=0;i<shape->cached_fill->nbPt;i++)
				  shape->fill_shp->AddPoint(shape->cached_fill->pts[i].x * cached_to_new);
			  if ( isometry == 1 ) {
				  for (int i=0;i<shape->cached_fill->nbAr;i++) 
					  shape->fill_shp->AddEdge(shape->cached_fill->aretes[i].st,
											   shape->cached_fill->aretes[i].en);
			  } else if ( isometry == -1 ) { // need to flip poly.
				  for (int i=0;i<shape->cached_fill->nbAr;i++)
					  shape->fill_shp->AddEdge(shape->cached_fill->aretes[i].en,
											   shape->cached_fill->aretes[i].st);
			  }
			  shape->fill_shp->ForceToPolygon();
			  shape->fill_shp->flags|=need_points_sorting | need_edges_sorting;
		  }
#endif
#ifndef test_liv
        }
#else
      }
#endif
//			shape->ctm = gc->transform;
		}
  }
}
static int       CmpFloat(const void * p1, const void * p2) {
	float* d1 = (float*)p1;
	float* d2 = (float*)p2;
	if ( fabsf(*d2 - *d1) < 0.0001) return 0;
	return (( *d1 < *d2 )?-1:1);
};

void
nr_arena_shape_update_stroke(NRArenaShape *shape,NRGC* gc)
{
	SPStyle* style = shape->style;
  
#ifdef test_liv
	shape->delayed_shp=false;
#endif

  if (style->stroke.type != SP_PAINT_TYPE_NONE) {
		float width, scale;
		scale = NR_MATRIX_DF_EXPANSION (&gc->transform);
    if ( fabsf(style->stroke_width.computed * scale) > 0.01 ) { // sinon c'est 0=oon veut pas de bord
      width = MAX (0.125, style->stroke_width.computed * scale);
#ifndef test_liv
      NRBPath bp;
      NRSVL *svl;
      bp.path = art_bpath_affine_transform (shape->curve->bpath, NR_MATRIX_D_TO_DOUBLE (&gc->transform));
      if (!style->stroke_dash.n_dash) {      
        svl = nr_bpath_stroke (&bp, NULL, width,
                               shape->style->stroke_linecap.value,
                               shape->style->stroke_linejoin.value,
                               shape->style->stroke_miterlimit.value * M_PI / 180.0,
                               0.25);
      } else {
        double dlen;
        ArtVpath *vp, *pvp;
        ArtSVP *asvp;
        vp = art_bez_path_to_vec (bp.path, 0.25);
        pvp = art_vpath_perturb (vp);
        art_free (vp);
        dlen = 0.0;
        for (int i = 0; i < style->stroke_dash.n_dash; i++) {
          dlen += style->stroke_dash.dash[i] * scale;
        }
        if (dlen >= 1.0) {
          ArtVpathDash dash;
          dash.offset = style->stroke_dash.offset * scale;
          dash.n_dash = style->stroke_dash.n_dash;
          dash.dash = g_new (double, dash.n_dash);
          for (int i = 0; i < dash.n_dash; i++) {
            dash.dash[i] = style->stroke_dash.dash[i] * scale;
          }
          vp = art_vpath_dash (pvp, &dash);
          art_free (pvp);
          pvp = vp;
          g_free (dash.dash);
        }
        asvp = art_svp_vpath_stroke (pvp,
                                     (ArtPathStrokeJoinType)shape->style->stroke_linejoin.value,
                                     (ArtPathStrokeCapType)shape->style->stroke_linecap.value,
                                     width,
                                     shape->style->stroke_miterlimit.value, 0.25);
        art_free (pvp);
        svl = nr_svl_from_art_svp (asvp);
        art_svp_free (asvp);
      }
      shape->stroke_svp = nr_svp_from_svl (svl, NULL);
      nr_svl_free_list (svl);
      art_free (bp.path);
#else
      NRMatrix  cached_to_new;
      int       isometry=0;
      if ( shape->cached_stroke ) {
        NR::Matrix   oMat(shape->cached_sctm);
        NR::Matrix   nMat(gc->transform);
        oMat=oMat.inverse();
        NR::Matrix   p=oMat*nMat;
        cached_to_new=p;
        NR::Matrix   tp;
        // transposition
        tp[0]=p[0];
        tp[1]=p[2];
        tp[2]=p[1];
        tp[3]=p[3];
        NR::Matrix   isom=tp*p;
        if ( fabs(isom[0]-1.0) < 0.01 && fabs(isom[3]-1.0) < 0.01 &&
             fabs(isom[1]) < 0.01 && fabs(isom[2]) < 0.01 ) {
          // the changed wrt the cached version is an isometry-> no need to recompute
          // the uncrossed polygon
          if ( p.det() < 0 ) isometry=-1; else isometry=1;
        } else {
          isom=0;
        }
      }
      if ( isometry == 0 ) {
        if ( shape->cached_stroke == NULL ) shape->cached_stroke=new Shape;
        shape->cached_stroke->Reset();
        Path*  thePath=new Path;
        Shape* theShape=new Shape;    
        {
          NR::Matrix   tempMat(gc->transform);
          thePath->LoadArtBPath(shape->curve->bpath,tempMat,true);
        }
        thePath->Convert(1.0);
        if (style->stroke_dash.n_dash) {      
          double dlen = 0.0;
          for (int i = 0; i < style->stroke_dash.n_dash; i++) {
            dlen += style->stroke_dash.dash[i] * scale;
          }
          if (dlen >= 1.0) {
            ArtVpathDash dash;
            dash.offset = style->stroke_dash.offset * scale;
            dash.n_dash = style->stroke_dash.n_dash;
            dash.dash = g_new (double, dash.n_dash);
            for (int i = 0; i < dash.n_dash; i++) {
              dash.dash[i] = style->stroke_dash.dash[i] * scale;
            }
            int    nbD=dash.n_dash;
            float  *dashs=(float*)malloc((nbD+1)*sizeof(float));
            while ( dash.offset >= dlen ) dash.offset-=dlen;
            dashs[0]=dash.dash[0];
            for (int i=1;i<nbD;i++) {
              dashs[i]=dashs[i-1]+dash.dash[i];
            }
            // modulo dlen
            thePath->DashPolyline(0.0,0.0,dlen,nbD,dashs,true,dash.offset);
            free(dashs);
            g_free (dash.dash);
          }
        }
		JoinType join=join_straight;
		ButtType butt=butt_straight;
		if ( shape->style->stroke_linecap.value == SP_STROKE_LINECAP_BUTT ) butt=butt_straight;
		if ( shape->style->stroke_linecap.value == SP_STROKE_LINECAP_ROUND ) butt=butt_round;
		if ( shape->style->stroke_linecap.value == SP_STROKE_LINECAP_SQUARE ) butt=butt_square;
		if ( shape->style->stroke_linejoin.value == SP_STROKE_LINEJOIN_MITER ) join=join_pointy;
		if ( shape->style->stroke_linejoin.value == SP_STROKE_LINEJOIN_ROUND ) join=join_round;
		if ( shape->style->stroke_linejoin.value == SP_STROKE_LINEJOIN_BEVEL ) join=join_straight;
		thePath->Stroke(theShape, false, 0.5*width, join, butt, 
						0.5*width*shape->style->stroke_miterlimit.value );
		
        if ( shape->cached_stroke->ConvertToShape(theShape,fill_nonZero) ) {
        }
        shape->cached_sctm=gc->transform;
        delete thePath;
        delete theShape;
        if ( shape->stroke_shp == NULL ) shape->stroke_shp=new Shape;
        shape->stroke_shp->Copy(shape->cached_stroke);
	  } else if ( isometry != 0 ) {
		  if ( shape->stroke_shp == NULL )
			  shape->stroke_shp=new Shape;
		  shape->stroke_shp->Reset(shape->cached_stroke->nbPt, shape->cached_stroke->nbAr);
		  for (int i=0;i<shape->cached_stroke->nbPt;i++)
			  shape->stroke_shp->AddPoint(shape->cached_stroke->pts[i].x*cached_to_new);
		  if ( isometry == 1 ) {
			  for (int i=0;i<shape->cached_stroke->nbAr;i++)
				  shape->stroke_shp->AddEdge(shape->cached_stroke->aretes[i].st,
											 shape->cached_stroke->aretes[i].en);
		  } else if ( isometry == -1 ) {
			  for (int i=0;i<shape->cached_stroke->nbAr;i++)
				  shape->stroke_shp->AddEdge(shape->cached_stroke->aretes[i].en,
											 shape->cached_stroke->aretes[i].st);
		  }
		  shape->stroke_shp->ForceToPolygon();
		  shape->stroke_shp->flags|=need_points_sorting | need_edges_sorting;
	  }
#endif
    }
  }
}
void
nr_arena_shape_add_bboxes(NRArenaShape* shape,NRRect &bbox)
{
#ifdef test_liv
  if ( shape->stroke_shp ) {
    shape->stroke_shp->CalcBBox();
    shape->stroke_shp->leftX=floorf(shape->stroke_shp->leftX);
    shape->stroke_shp->rightX=ceilf(shape->stroke_shp->rightX);
    shape->stroke_shp->topY=floorf(shape->stroke_shp->topY);
    shape->stroke_shp->bottomY=ceilf(shape->stroke_shp->bottomY);
    if ( bbox.x0 >= bbox.x1 ) {
      if ( shape->stroke_shp->leftX < shape->stroke_shp->rightX ) {
        bbox.x0=shape->stroke_shp->leftX;
        bbox.x1=shape->stroke_shp->rightX;
      }
    } else {
      if ( shape->stroke_shp->leftX < bbox.x0 )
		  bbox.x0=shape->stroke_shp->leftX;
      if ( shape->stroke_shp->rightX > bbox.x1 )
		  bbox.x1=shape->stroke_shp->rightX;
    }
    if ( bbox.y0 >= bbox.y1 ) {
      if ( shape->stroke_shp->topY < shape->stroke_shp->bottomY ) {
        bbox.y0=shape->stroke_shp->topY;
        bbox.y1=shape->stroke_shp->bottomY;
      }
    } else {
		if ( shape->stroke_shp->topY < bbox.y0 )
			bbox.y0=shape->stroke_shp->topY;
		if ( shape->stroke_shp->bottomY > bbox.y1 )
			bbox.y1=shape->stroke_shp->bottomY;
    }
  }
  if ( shape->fill_shp ) {
    shape->fill_shp->CalcBBox();
    shape->fill_shp->leftX=floorf(shape->fill_shp->leftX);
    shape->fill_shp->rightX=ceilf(shape->fill_shp->rightX);
    shape->fill_shp->topY=floorf(shape->fill_shp->topY);
    shape->fill_shp->bottomY=ceilf(shape->fill_shp->bottomY);
    if ( bbox.x0 >= bbox.x1 ) {
      if ( shape->fill_shp->leftX < shape->fill_shp->rightX ) {
        bbox.x0=shape->fill_shp->leftX;
        bbox.x1=shape->fill_shp->rightX;
      }
    } else {
      if ( shape->fill_shp->leftX < bbox.x0 ) bbox.x0=shape->fill_shp->leftX;
      if ( shape->fill_shp->rightX > bbox.x1 ) bbox.x1=shape->fill_shp->rightX;
    }
    if ( bbox.y0 >= bbox.y1 ) {
      if ( shape->fill_shp->topY < shape->fill_shp->bottomY ) {
        bbox.y0=shape->fill_shp->topY;
        bbox.y1=shape->fill_shp->bottomY;
      }
    } else {
      if ( shape->fill_shp->topY < bbox.y0 ) bbox.y0=shape->fill_shp->topY;
      if ( shape->fill_shp->bottomY > bbox.y1 ) bbox.y1=shape->fill_shp->bottomY;
    }
  }
#else
#endif
}
static unsigned int
nr_arena_shape_render (NRArenaItem *item, NRRectL *area, NRPixBlock *pb, unsigned int flags)
{
	NRArenaItem *child;

	NRArenaShape *shape = NR_ARENA_SHAPE (item);

	if (!shape->curve) return item->state;
	if (!shape->style) return item->state;
  
#ifdef test_liv
  if ( shape->delayed_shp ) {
    if ( nr_rect_l_test_intersect (area, &item->bbox) ) {
      NRGC   tempGC;
      tempGC.transform=shape->ctm;
      nr_arena_shape_update_stroke(shape,&tempGC);
      nr_arena_shape_update_fill(shape,&tempGC);
/*      NRRect bbox;
      bbox.x0 = bbox.y0 = bbox.x1 = bbox.y1 = 0.0;
      nr_arena_shape_add_bboxes(shape,bbox);
      item->bbox.x0 = (gint32)(bbox.x0 - 1.0F);
      item->bbox.y0 = (gint32)(bbox.y0 - 1.0F);
      item->bbox.x1 = (gint32)(bbox.x1 + 1.0F);
      item->bbox.y1 = (gint32)(bbox.y1 + 1.0F);
      shape->approx_bbox=item->bbox;*/
    }
  }
#endif

	SPStyle const *style = shape->style;
#ifndef test_liv
	if (shape->fill_svp) {
#else
  if ( shape->fill_shp ) {
#endif
		NRPixBlock m;
		guint32 rgba;

		nr_pixblock_setup_fast (&m, NR_PIXBLOCK_MODE_A8, area->x0, area->y0, area->x1, area->y1, TRUE);
#ifndef test_liv
		nr_pixblock_render_svp_mask_or (&m, shape->fill_svp);
#else
		nr_pixblock_render_shape_mask_or (m,shape->fill_shp);
#endif
		m.empty = FALSE;

		switch (style->fill.type) {
		case SP_PAINT_TYPE_COLOR:
			if ( item->render_opacity ) {
				rgba = sp_color_get_rgba32_falpha (&style->fill.value.color,
												   SP_SCALE24_TO_FLOAT (style->fill_opacity.value) *
												   SP_SCALE24_TO_FLOAT (style->opacity.value));
			} else {
				rgba = sp_color_get_rgba32_falpha (&style->fill.value.color,
												   SP_SCALE24_TO_FLOAT (style->fill_opacity.value) );
			}
			nr_blit_pixblock_mask_rgba32 (pb, &m, rgba);
			pb->empty = FALSE;
			break;
		case SP_PAINT_TYPE_PAINTSERVER:
			if (shape->fill_painter) {
				NRPixBlock cb;
				/* Need separate gradient buffer */
				nr_pixblock_setup_fast (&cb, NR_PIXBLOCK_MODE_R8G8B8A8N, area->x0, area->y0, area->x1, area->y1, TRUE);
				shape->fill_painter->fill (shape->fill_painter, &cb);
				cb.empty = FALSE;
				/* Composite */
				nr_blit_pixblock_pixblock_mask (pb, &cb, &m);
				pb->empty = FALSE;
				nr_pixblock_release (&cb);
			}
			break;
		default:
			break;
		}
		nr_pixblock_release (&m);
#ifndef test_liv
	}
#else
  }
#endif
  
#ifndef test_liv
	if (shape->stroke_svp) {
#else
  if ( shape->stroke_shp ) {
#endif
		NRPixBlock m;
		guint32 rgba;

		nr_pixblock_setup_fast (&m, NR_PIXBLOCK_MODE_A8, area->x0, area->y0, area->x1, area->y1, TRUE);
#ifndef test_liv
		nr_pixblock_render_svp_mask_or (&m, shape->stroke_svp);
#else
		nr_pixblock_render_shape_mask_or (m,shape->stroke_shp);
#endif
		m.empty = FALSE;

		switch (style->stroke.type) {
		case SP_PAINT_TYPE_COLOR:
      if ( item->render_opacity ) {
        rgba = sp_color_get_rgba32_falpha (&style->stroke.value.color,
                                           SP_SCALE24_TO_FLOAT (style->stroke_opacity.value) *
                                           SP_SCALE24_TO_FLOAT (style->opacity.value));
      } else {
        rgba = sp_color_get_rgba32_falpha (&style->stroke.value.color,
                                           SP_SCALE24_TO_FLOAT (style->stroke_opacity.value));
      }
			nr_blit_pixblock_mask_rgba32 (pb, &m, rgba);
			pb->empty = FALSE;
			break;
		case SP_PAINT_TYPE_PAINTSERVER:
			if (shape->stroke_painter) {
				NRPixBlock cb;
				/* Need separate gradient buffer */
				nr_pixblock_setup_fast (&cb, NR_PIXBLOCK_MODE_R8G8B8A8N, area->x0, area->y0, area->x1, area->y1, TRUE);
				shape->stroke_painter->fill (shape->stroke_painter, &cb);
				cb.empty = FALSE;
				/* Composite */
				nr_blit_pixblock_pixblock_mask (pb, &cb, &m);
				pb->empty = FALSE;
				nr_pixblock_release (&cb);
			}
			break;
		default:
			break;
		}
		nr_pixblock_release (&m);
#ifndef test_liv
	}
#else
  }
#endif
  
	/* Just compose children into parent buffer */
	for (child = shape->markers; child != NULL; child = child->next) {
		unsigned int ret;
		ret = nr_arena_item_invoke_render (child, area, pb, flags);
		if (ret & NR_ARENA_ITEM_STATE_INVALID) return ret;
	}

	return item->state;
}

static guint
nr_arena_shape_clip (NRArenaItem *item, NRRectL *area, NRPixBlock *pb)
{
	NRArenaShape *shape = NR_ARENA_SHAPE (item);

	if (!shape->curve) return item->state;

#ifdef test_liv
  if ( shape->delayed_shp ) {
    if ( nr_rect_l_test_intersect (area, &item->bbox) ) {
      NRGC   tempGC;
      tempGC.transform=shape->ctm;
      nr_arena_shape_update_stroke(shape,&tempGC);
      nr_arena_shape_update_fill(shape,&tempGC);
      /*      NRRect bbox;
      bbox.x0 = bbox.y0 = bbox.x1 = bbox.y1 = 0.0;
      nr_arena_shape_add_bboxes(shape,bbox);
      item->bbox.x0 = (gint32)(bbox.x0 - 1.0F);
      item->bbox.y0 = (gint32)(bbox.y0 - 1.0F);
      item->bbox.x1 = (gint32)(bbox.x1 + 1.0F);
      item->bbox.y1 = (gint32)(bbox.y1 + 1.0F);
      shape->approx_bbox=item->bbox;*/
    }
  }
#endif

#ifndef test_liv
	if (shape->fill_svp) {
#else
  if ( shape->fill_shp ) {
#endif
		NRPixBlock m;

		/* fixme: We can OR in one step (Lauris) */
		nr_pixblock_setup_fast (&m, NR_PIXBLOCK_MODE_A8, area->x0, area->y0, area->x1, area->y1, TRUE);
#ifndef test_liv
		nr_pixblock_render_svp_mask_or (&m, shape->fill_svp);
#else
    nr_pixblock_render_shape_mask_or (m,shape->fill_shp);
#endif
    
		for (int y = area->y0; y < area->y1; y++) {
			unsigned char *s, *d;
			s = NR_PIXBLOCK_PX (&m) + (y - area->y0) * m.rs;
			d = NR_PIXBLOCK_PX (pb) + (y - area->y0) * pb->rs;
			for (int x = area->x0; x < area->x1; x++) {
				*d = (NR_A7 (*s, *d) + 127) / 255;
				d += 1;
				s += 1;
			}
		}
		nr_pixblock_release (&m);
		pb->empty = FALSE;
#ifndef test_liv
	}
#else
  }
#endif
  
	return item->state;
}

static NRArenaItem *
nr_arena_shape_pick (NRArenaItem *item, NR::Point p, double delta, unsigned int /*sticky*/)
{
	NRArenaShape *shape = NR_ARENA_SHAPE (item);

	if (!shape->curve) return NULL;
	if (!shape->style) return NULL;
	const double x = p[NR::X];
	const double y = p[NR::Y];
#ifdef test_liv
  if ( shape->delayed_shp ) {
    NRRectL  area;
    area.x0=(int)floor(x);
    area.x1=(int)ceil(x);
    area.y0=(int)floor(y);
    area.y1=(int)ceil(y);
    area.x0-=(int)ceil(delta);
    area.x1+=(int)ceil(delta);
    area.y0-=(int)ceil(delta);
    area.y1+=(int)ceil(delta);
    area.x0-=1;
    area.x1+=1;
    area.y0-=1;
    area.y1+=1;
    if ( nr_rect_l_test_intersect (&area, &item->bbox) ) {
      NRGC   tempGC;
      tempGC.transform=shape->ctm;
      nr_arena_shape_update_stroke(shape,&tempGC);
      nr_arena_shape_update_fill(shape,&tempGC);
      /*      NRRect bbox;
      bbox.x0 = bbox.y0 = bbox.x1 = bbox.y1 = 0.0;
      nr_arena_shape_add_bboxes(shape,bbox);
      item->bbox.x0 = (gint32)(bbox.x0 - 1.0F);
      item->bbox.y0 = (gint32)(bbox.y0 - 1.0F);
      item->bbox.x1 = (gint32)(bbox.x1 + 1.0F);
      item->bbox.y1 = (gint32)(bbox.y1 + 1.0F);
      shape->approx_bbox=item->bbox;*/
    }
  }
#endif

	if (item->state & NR_ARENA_ITEM_STATE_RENDER) {
#ifndef test_liv
		if (shape->fill_svp && (shape->style->fill.type != SP_PAINT_TYPE_NONE)) {
			if (nr_svp_point_wind (shape->fill_svp, (float) x, (float) y)) return item;
		}
		if (shape->stroke_svp && (shape->style->stroke.type != SP_PAINT_TYPE_NONE)) {
			if (nr_svp_point_wind (shape->stroke_svp, (float) x, (float) y)) return item;
		}
		if (delta > 1e-3) {
			if (shape->fill_svp && (shape->style->fill.type != SP_PAINT_TYPE_NONE)) {
				if (nr_svp_point_distance (shape->fill_svp, (float) x, (float) y) <= delta) return item;
			}
			if (shape->stroke_svp && (shape->style->stroke.type != SP_PAINT_TYPE_NONE)) {
				if (nr_svp_point_distance (shape->stroke_svp, (float) x, (float) y) <= delta) return item;
			}
		}
#else 
    NR::Point const thePt(x, y);
		if (shape->fill_shp && (shape->style->fill.type != SP_PAINT_TYPE_NONE)) {
			if (shape->fill_shp->PtWinding(thePt) > 0 ) return item;
		}
		if (shape->stroke_shp && (shape->style->stroke.type != SP_PAINT_TYPE_NONE)) {
			if (shape->stroke_shp->PtWinding(thePt) > 0 ) return item;
		}
		if (delta > 1e-3) {
			if (shape->fill_shp && (shape->style->fill.type != SP_PAINT_TYPE_NONE)) {
				if (shape->fill_shp->DistanceLE(thePt, delta)) return item;
			}
			if (shape->stroke_shp && (shape->style->stroke.type != SP_PAINT_TYPE_NONE)) {
				if ( shape->stroke_shp->DistanceLE(thePt, delta)) return item;
			}
		}
#endif
	} else {
		/* todo: These float casts may be left over from when pt was a NRPointF (with float
		   coords). They can probably be removed.  Similarly, dist can probably be changed
		   to double and change ...wind_distance accordingly. */
		NRPoint pt;
		pt.x = (float) x;
		pt.y = (float) y;
		NRBPath bp;
		bp.path = shape->curve->bpath;
		float dist = NR_HUGE;
		int wind = 0;
		nr_path_matrix_f_point_f_bbox_wind_distance(&bp, &shape->ctm, &pt, NULL, &wind, &dist, NR_EPSILON);
		if (shape->style->fill.type != SP_PAINT_TYPE_NONE) {
			if (!shape->style->fill_rule.value) {
				if (wind != 0) return item;
			} else {
				if (wind & 0x1) return item;
			}
		}
		if (shape->style->stroke.type != SP_PAINT_TYPE_NONE) {
			/* fixme: We do not take stroke width into account here (Lauris) */
			if (dist < delta) return item;
		}
	}

	return NULL;
}

/** 
 *
 *  Requests a render of the shape, then if the shape is already a curve it
 *  unrefs the old curve; if the new curve is valid it creates a copy of the
 *  curve and adds it to the shape.  Finally, it requests an update of the
 *  arena for the shape.
 */
void nr_arena_shape_set_path(NRArenaShape *shape, SPCurve *curve,bool justTrans)
{
	g_return_if_fail (shape != NULL);
	g_return_if_fail (NR_IS_ARENA_SHAPE (shape));

#ifdef test_liv
  if ( justTrans == false ) {
    // dirty cached versions
    if ( shape->cached_fill ) {
      delete shape->cached_fill;
      shape->cached_fill=NULL;
    }
    if ( shape->cached_stroke ) {
      delete shape->cached_stroke;
      shape->cached_stroke=NULL;
    }
  }
#endif

	nr_arena_item_request_render (NR_ARENA_ITEM (shape));

	if (shape->curve) {
		sp_curve_unref (shape->curve);
		shape->curve = NULL;
	}

	if (curve) {
		shape->curve = curve;
		sp_curve_ref (curve);
	}

	nr_arena_item_request_update (NR_ARENA_ITEM (shape), NR_ARENA_ITEM_STATE_ALL, FALSE);
}

/** nr_arena_shape_set_style
 *
 * Unrefs any existing style and ref's to the given one, then requests an update of the arena
 */
void
nr_arena_shape_set_style (NRArenaShape *shape, SPStyle *style)
{
	g_return_if_fail (shape != NULL);
	g_return_if_fail (NR_IS_ARENA_SHAPE (shape));

	if (style) sp_style_ref (style);
	if (shape->style) sp_style_unref (shape->style);
	shape->style = style;
  
#ifdef test_liv
  // dirty cached versions
  if ( shape->cached_fill ) {
    delete shape->cached_fill;
    shape->cached_fill=NULL;
  }
  if ( shape->cached_stroke ) {
    delete shape->cached_stroke;
    shape->cached_stroke=NULL;
  }
#endif

	nr_arena_item_request_update (NR_ARENA_ITEM (shape), NR_ARENA_ITEM_STATE_ALL, FALSE);
}

void
nr_arena_shape_set_paintbox (NRArenaShape *shape, const NRRect *pbox)
{
	g_return_if_fail (shape != NULL);
	g_return_if_fail (NR_IS_ARENA_SHAPE (shape));
	g_return_if_fail (pbox != NULL);

	if ((pbox->x0 < pbox->x1) && (pbox->y0 < pbox->y1)) {
		shape->paintbox = *pbox;
	} else {
		/* fixme: We kill warning, although not sure what to do here (Lauris) */
		shape->paintbox.x0 = shape->paintbox.y0 = 0.0F;
		shape->paintbox.x1 = shape->paintbox.y1 = 256.0F;
	}

	nr_arena_item_request_update (NR_ARENA_ITEM (shape), NR_ARENA_ITEM_STATE_ALL, FALSE);
}

#ifdef test_liv

static void
shape_run_A8_OR (raster_info &dest,void */*data*/,int st,float vst,int en,float ven)
{
  if ( st >= en ) return;
  if ( vst < 0 ) vst=0;
  if ( vst > 1 ) vst=1;
  if ( ven < 0 ) ven=0;
  if ( ven > 1 ) ven=1;
  float   sv=vst;
  float   dv=ven-vst;
  int     len=en-st;
  unsigned char*   d=(unsigned char*)dest.buffer;
  d+=(st-dest.startPix);
  if ( fabsf(dv) < 0.001 ) {
    if ( vst > 0.999 ) {
      /* Simple copy */
      while (len > 0) {
        d[0] = 255;
        d += 1;
        len -= 1;
      }
    } else {
      sv*=256;
      unsigned int c0_24=(int)sv;
      c0_24&=0xFF;
      while (len > 0) {
        unsigned int da;
        /* Draw */
        da = 65025 - (255 - c0_24) * (255 - d[0]);
        d[0] = (da + 127) / 255;
        d += 1;
        len -= 1;
      }
    }
  } else {
    if ( en <= st+1 ) {
      sv=0.5*(vst+ven);
      sv*=256;
      unsigned int c0_24=(int)sv;
      c0_24&=0xFF;
      unsigned int da;
      /* Draw */
      da = 65025 - (255 - c0_24) * (255 - d[0]);
      d[0] = (da + 127) / 255;
    } else {
      dv/=len;
      sv+=0.5*dv; // correction trapezoidale
      sv*=16777216;
      dv*=16777216;
      int c0_24 = static_cast<int>(CLAMP(sv, 0, 16777216));
      int s0_24 = static_cast<int>(dv);
      while (len > 0) {
        unsigned int ca, da;
        /* Draw */
        ca = c0_24 >> 16;
        if ( ca > 255 ) ca=255;
        da = 65025 - (255 - ca) * (255 - d[0]);
        d[0] = (da + 127) / 255;
        d += 1;
        c0_24 += s0_24;
        c0_24 = CLAMP (c0_24, 0, 16777216);
        len -= 1;
      }
    }
  }
}

void nr_pixblock_render_shape_mask_or (NRPixBlock &m,Shape* theS)
{

  theS->CalcBBox();
  float  l=theS->leftX,r=theS->rightX,t=theS->topY,b=theS->bottomY;
  int    il,ir,it,ib;
  il=(int)floorf(l);
  ir=(int)ceilf(r);
  it=(int)floorf(t);
  ib=(int)ceilf(b);

  if ( il >= m.area.x1 || ir <= m.area.x0 || it >= m.area.y1 || ib <= m.area.y0 ) return;
  if ( il < m.area.x0 ) il=m.area.x0;
  if ( it < m.area.y0 ) it=m.area.y0;
  if ( ir > m.area.x1 ) ir=m.area.x1;
  if ( ib > m.area.y1 ) ib=m.area.y1;
  
  // version par FloatLigne
  int    curPt;
  float  curY;
  theS->BeginQuickRaster(curY,curPt,1.0);
  
  FloatLigne* theI=new FloatLigne();
  IntLigne*   theIL=new IntLigne();
  
  theS->DirectQuickScan(curY,curPt,(float)(it),true,1.0);
  
  char* mdata=(char*)m.data.px;
  if ( m.size == NR_PIXBLOCK_SIZE_TINY ) mdata=(char*)m.data.p;
  uint32_t* ligStart=((uint32_t*)(mdata+((il-m.area.x0)+m.rs*(it-m.area.y0))));
  for (int y=it;y<ib;y++) {
    theI->Reset();
    if ( y&0x00000003 ) {
      theS->QuickScan(curY,curPt,((float)(y+1)),theI,false,1.0);
    } else {
      theS->QuickScan(curY,curPt,((float)(y+1)),theI,true,1.0);
    }
    theI->Flatten();
    theIL->Copy(theI);
    
    raster_info  dest;
    dest.startPix=il;
    dest.endPix=ir;
    dest.sth=il;
    dest.stv=y;
    dest.buffer=ligStart;
    theIL->Raster(dest,NULL,shape_run_A8_OR);
    ligStart=((uint32_t*)(((char*)ligStart)+m.rs));
  }
  theS->EndQuickRaster();
  delete theI;
  delete theIL;
  
  /*  // version par BitLigne
    int    curPt;
  float  curY;
  theS->BeginRaster(curY,curPt,1.0);
  
  BitLigne*   theI[4];
  for (int i=0;i<4;i++) theI[i]=new BitLigne(il,ir);
  IntLigne*   theIL=new IntLigne();
  
  theS->Scan(curY,curPt,(float)(it),0.25);
  
  char* mdata=(char*)m.data.px;
  if ( m.size == NR_PIXBLOCK_SIZE_TINY ) mdata=(char*)m.data.p;
  uint32_t* ligStart=((uint32_t*)(mdata+((il-m.area.x0)+m.rs*(it-m.area.y0))));
  for (int y=it;y<ib;y++) {
    for (int i=0;i<4;i++) theI[i]->Reset();
    if ( y&0x00000003 ) {
      theS->Scan(curY,curPt,((float)(y+0.25)),fill_oddEven,theI[0],false,0.25);
      theS->Scan(curY,curPt,((float)(y+0.5)),fill_oddEven,theI[1],false,0.25);
      theS->Scan(curY,curPt,((float)(y+0.75)),fill_oddEven,theI[2],false,0.25);
      theS->Scan(curY,curPt,((float)(y+1.0)),fill_oddEven,theI[3],false,0.25);
    } else {
      theS->Scan(curY,curPt,((float)(y+0.25)),fill_oddEven,theI[0],true,0.25);
      theS->Scan(curY,curPt,((float)(y+0.5)),fill_oddEven,theI[1],true,0.25);
      theS->Scan(curY,curPt,((float)(y+0.75)),fill_oddEven,theI[2],true,0.25);
      theS->Scan(curY,curPt,((float)(y+1.0)),fill_oddEven,theI[3],true,0.25);
    }
    theIL->Copy(4,theI);
    
    raster_info  dest;
    dest.startPix=il;
    dest.endPix=ir;
    dest.sth=il;
    dest.stv=y;
    dest.buffer=ligStart;
    theIL->Raster(dest,NULL,shape_run_A8_OR);
    ligStart=((uint32_t*)(((char*)ligStart)+m.rs));
  }
  theS->EndRaster();
  for (int i=0;i<4;i++) delete theI[i];
  delete theIL;*/
  
/*  // version par BitLigne directe
  int    curPt;
  float  curY;
  theS->BeginQuickRaster(curY,curPt,1.0);
  
  BitLigne*   theI[4];
  for (int i=0;i<4;i++) theI[i]=new BitLigne(il,ir);
  IntLigne*   theIL=new IntLigne();
  
  theS->DirectQuickScan(curY,curPt,(float)(it),true,0.25);
  
  char* mdata=(char*)m.data.px;
  if ( m.size == NR_PIXBLOCK_SIZE_TINY ) mdata=(char*)m.data.p;
  uint32_t* ligStart=((uint32_t*)(mdata+((il-m.area.x0)+m.rs*(it-m.area.y0))));
  for (int y=it;y<ib;y++) {
    for (int i=0;i<4;i++) theI[i]->Reset();
    if ( y&0x00000003 ) {
      theS->QuickScan(curY,curPt,((float)(y+0.25)),fill_oddEven,theI[0],false,0.25);
      theS->QuickScan(curY,curPt,((float)(y+0.5)),fill_oddEven,theI[1],false,0.25);
      theS->QuickScan(curY,curPt,((float)(y+0.75)),fill_oddEven,theI[2],false,0.25);
      theS->QuickScan(curY,curPt,((float)(y+1.0)),fill_oddEven,theI[3],false,0.25);
    } else {
      theS->QuickScan(curY,curPt,((float)(y+0.25)),fill_oddEven,theI[0],true,0.25);
      theS->QuickScan(curY,curPt,((float)(y+0.5)),fill_oddEven,theI[1],true,0.25);
      theS->QuickScan(curY,curPt,((float)(y+0.75)),fill_oddEven,theI[2],true,0.25);
      theS->QuickScan(curY,curPt,((float)(y+1.0)),fill_oddEven,theI[3],true,0.25);
    }
    theIL->Copy(4,theI);
    
    raster_info  dest;
    dest.startPix=il;
    dest.endPix=ir;
    dest.sth=il;
    dest.stv=y;
    dest.buffer=ligStart;
    theIL->Raster(dest,NULL,shape_run_A8_OR);
    ligStart=((uint32_t*)(((char*)ligStart)+m.rs));
  }
  theS->EndQuickRaster();
  for (int i=0;i<4;i++) delete theI[i];
  delete theIL;*/
}

#endif

