#define __NR_ARENA_GLYPHS_C__

/*
 * RGBA display list system for inkscape
 *
 * Author:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * Copyright (C) 2002 Lauris Kaplinski
 *
 * Released under GNU GPL
 *
 */


#include <math.h>
#include <string.h>
#include <glib.h>
#include <libnr/nr-rect.h>
#include <libnr/nr-matrix.h>
#include <libnr/nr-blit.h>
#include <libnr/nr-path.h>
#include "../style.h"
#include "nr-arena.h"
#include "nr-arena-glyphs.h"

#ifdef test_glyph_liv
#include "../display/canvas-bpath.h"
#include "../livarot/LivarotDefs.h"
#include "../livarot/Path.h"
#include "../livarot/Shape.h"
#include "../livarot/Ligne.h"
#include <libnr/nr-matrix-ops.h>

// defined in nr-arena-shape.cpp
void nr_pixblock_render_shape_mask_or (NRPixBlock &m,Shape* theS);
#endif

static void nr_arena_glyphs_class_init (NRArenaGlyphsClass *klass);
static void nr_arena_glyphs_init (NRArenaGlyphs *glyphs);
static void nr_arena_glyphs_finalize (NRObject *object);

static guint nr_arena_glyphs_update (NRArenaItem *item, NRRectL *area, NRGC *gc, guint state, guint reset);
static guint nr_arena_glyphs_clip (NRArenaItem *item, NRRectL *area, NRPixBlock *pb);
static NRArenaItem *nr_arena_glyphs_pick (NRArenaItem *item, NR::Point p, double delta, unsigned int sticky);

static NRArenaItemClass *glyphs_parent_class;

NRType
nr_arena_glyphs_get_type (void)
{
	static NRType type = 0;
	if (!type) {
		type = nr_object_register_type (NR_TYPE_ARENA_ITEM,
						"NRArenaGlyphs",
						sizeof (NRArenaGlyphsClass),
						sizeof (NRArenaGlyphs),
						(void (*) (NRObjectClass *)) nr_arena_glyphs_class_init,
						(void (*) (NRObject *)) nr_arena_glyphs_init);
	}
	return type;
}

static void
nr_arena_glyphs_class_init (NRArenaGlyphsClass *klass)
{
	NRObjectClass *object_class;
	NRArenaItemClass *item_class;

	object_class = (NRObjectClass *) klass;
	item_class = (NRArenaItemClass *) klass;

	glyphs_parent_class = (NRArenaItemClass *) ((NRObjectClass *) klass)->parent;

	object_class->finalize = nr_arena_glyphs_finalize;

	item_class->update = nr_arena_glyphs_update;
	item_class->clip = nr_arena_glyphs_clip;
	item_class->pick = nr_arena_glyphs_pick;
}

static void
nr_arena_glyphs_init (NRArenaGlyphs *glyphs)
{
	glyphs->curve = NULL;
	glyphs->style = NULL;
	nr_matrix_set_identity(&glyphs->transform);
	glyphs->font = NULL;
	glyphs->glyph = 0;

	glyphs->rfont = NULL;
	glyphs->x = glyphs->y = 0.0;

	nr_matrix_set_identity(&glyphs->cached_tr);
	glyphs->cached_shp=NULL;
	glyphs->cached_shp_dirty=false;
	glyphs->cached_style_dirty=false;

	glyphs->stroke_shp=NULL;
}

static void
nr_arena_glyphs_finalize (NRObject *object)
{
	NRArenaGlyphs *glyphs;

	glyphs = NR_ARENA_GLYPHS (object);

	if (glyphs->cached_shp) {
		delete glyphs->cached_shp;
		glyphs->cached_shp = NULL;
	}
	if (glyphs->stroke_shp) {
		delete glyphs->stroke_shp;
		glyphs->stroke_shp = NULL;
	}
  
	if (glyphs->rfont) {
		glyphs->rfont = nr_rasterfont_unref (glyphs->rfont);
	}

	if (glyphs->font) {
		glyphs->font = nr_font_unref (glyphs->font);
	}

	if (glyphs->style) {
		sp_style_unref (glyphs->style);
		glyphs->style = NULL;
	}

	if (glyphs->curve) {
		glyphs->curve = sp_curve_unref (glyphs->curve);
	}

	((NRObjectClass *) glyphs_parent_class)->finalize (object);
}

static guint
nr_arena_glyphs_update (NRArenaItem *item, NRRectL *area, NRGC *gc, guint state, guint reset)
{
	NRArenaGlyphs *glyphs;
	NRRasterFont *rfont;
	NRMatrix t;
	NRRect bbox;

	glyphs = NR_ARENA_GLYPHS (item);

	/* Request repaint old area if needed */
	/* fixme: Think about it a bit (Lauris) */
	if (!nr_rect_l_test_empty (&item->bbox)) {
		nr_arena_request_render_rect (item->arena, &item->bbox);
		nr_rect_l_set_empty (&item->bbox);
	}

	/* Release state data */
	if (glyphs->stroke_shp) {
    delete glyphs->stroke_shp;
		glyphs->stroke_shp = NULL;
	}

	if (!glyphs->font || !glyphs->curve || !glyphs->style) return NR_ARENA_ITEM_STATE_ALL;
	if ((glyphs->style->fill.type == SP_PAINT_TYPE_NONE) && (glyphs->style->stroke.type == SP_PAINT_TYPE_NONE)) return NR_ARENA_ITEM_STATE_ALL;

	bbox.x0 = bbox.y0 = bbox.x1 = bbox.y1 = 0.0;

	if (glyphs->style->fill.type != SP_PAINT_TYPE_NONE) {
		NRRect area;
		nr_matrix_multiply (&t, &glyphs->transform, &gc->transform);
		rfont = nr_rasterfont_new (glyphs->font, &t);
		if (glyphs->rfont) glyphs->rfont = nr_rasterfont_unref (glyphs->rfont);
		glyphs->rfont = rfont;
		glyphs->x = t.c[4];
		glyphs->y = t.c[5];
		nr_rasterfont_glyph_area_get (rfont, glyphs->glyph, &area);
		bbox.x0 = area.x0 + glyphs->x;
		bbox.y0 = area.y0 + glyphs->y;
		bbox.x1 = area.x1 + glyphs->x;
		bbox.y1 = area.y1 + glyphs->y;
	}

	if (glyphs->style->stroke.type != SP_PAINT_TYPE_NONE) {
		/* Build state data */
    bool   recache=false;
    bool   antiIsometry=false;
    if ( glyphs->cached_shp == NULL || glyphs->cached_shp_dirty || glyphs->cached_style_dirty ) {
      recache=true;
    } else {
      NR::Matrix   oMat(glyphs->cached_tr);
      NR::Matrix   nMat(gc->transform);
      oMat=oMat.inverse();
      NR::Matrix   p=oMat*nMat;
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
        antiIsometry=(p.det()<0)?true:false;
        recache=false;
      } else {
        recache=true;
      }
    }
    if ( recache ) {
//      printf("r: %i %i\n",(glyphs->cached_shp_dirty)?1:0,(glyphs->cached_style_dirty)?1:0);
      glyphs->cached_shp_dirty=glyphs->cached_style_dirty=false;
      Path*  thePath=new Path;
      Shape* theShape=new Shape;    
      if ( glyphs->cached_shp == NULL ) glyphs->cached_shp=new Shape;
      {
        NR::Matrix   tempMat(gc->transform);
        thePath->LoadArtBPath(glyphs->curve->bpath,tempMat,true);
      }
      thePath->Convert(0.25);
      
      gdouble width;
      width = glyphs->style->stroke_width.computed * NR_MATRIX_DF_EXPANSION (&gc->transform);
      glyphs->cached_tr=gc->transform;
      width = MAX (0.125, width);
      
      JoinType join=join_straight;
      ButtType butt=butt_straight;
      if ( glyphs->style->stroke_linecap.value == SP_STROKE_LINECAP_BUTT ) butt=butt_straight;
      if ( glyphs->style->stroke_linecap.value == SP_STROKE_LINECAP_ROUND ) butt=butt_round;
      if ( glyphs->style->stroke_linecap.value == SP_STROKE_LINECAP_SQUARE ) butt=butt_square;
      if ( glyphs->style->stroke_linejoin.value == SP_STROKE_LINEJOIN_MITER ) join=join_pointy;
      if ( glyphs->style->stroke_linejoin.value == SP_STROKE_LINEJOIN_ROUND ) join=join_round;
      if ( glyphs->style->stroke_linejoin.value == SP_STROKE_LINEJOIN_BEVEL ) join=join_straight;
      thePath->Stroke(theShape,false,0.5*width, join,butt,width*glyphs->style->stroke_miterlimit.value);
      
      glyphs->cached_shp->ConvertToShape(theShape,fill_nonZero);
      delete thePath;
      delete theShape;
    }
    if ( glyphs->cached_shp == NULL ) {
      if ( glyphs->stroke_shp == NULL ) glyphs->stroke_shp=new Shape;
      glyphs->stroke_shp->Reset();
    } else {
      if ( glyphs->stroke_shp == NULL ) glyphs->stroke_shp=new Shape;
      NR::Matrix   oMat(glyphs->cached_tr);
      NR::Matrix   nMat(gc->transform);
      oMat=oMat.inverse();
      NR::Matrix   p=oMat*nMat; // attention a pas le mettre dans le sens inverse
      bool antiIsometry=(p.det()<0)?true:false;
      glyphs->stroke_shp->Reset(glyphs->cached_shp->nbPt,glyphs->cached_shp->nbAr);
      for (int i=0;i<glyphs->cached_shp->nbPt;i++) {
        glyphs->stroke_shp->AddPoint(glyphs->cached_shp->pts[i].x*p); // matrix * point multiplication is in reverse order?
      }
      if ( antiIsometry ) {
        for (int i=0;i<glyphs->cached_shp->nbAr;i++) {
          if ( glyphs->cached_shp->aretes[i].st < 0 || glyphs->cached_shp->aretes[i].en < 0 ) {
            printf("inv\n");
          }
          glyphs->stroke_shp->AddEdge(glyphs->cached_shp->aretes[i].en,glyphs->cached_shp->aretes[i].st);
        }
      } else {
        for (int i=0;i<glyphs->cached_shp->nbAr;i++) {
          if ( glyphs->cached_shp->aretes[i].st < 0 || glyphs->cached_shp->aretes[i].en < 0 ) {
            printf("inv\n");
          }
          glyphs->stroke_shp->AddEdge(glyphs->cached_shp->aretes[i].st,glyphs->cached_shp->aretes[i].en);
        }
      }
      glyphs->stroke_shp->ForceToPolygon();
      glyphs->stroke_shp->flags|=need_points_sorting;
      glyphs->stroke_shp->flags|=need_edges_sorting;
    }
	}

  if ( glyphs->stroke_shp ) {
    glyphs->stroke_shp->CalcBBox();
    if ( bbox.x0 >= bbox.x1 || bbox.y0 >= bbox.y1 ) {
      bbox.x0=glyphs->stroke_shp->leftX;
      bbox.x1=glyphs->stroke_shp->rightX;
      bbox.y0=glyphs->stroke_shp->topY;
      bbox.y1=glyphs->stroke_shp->bottomY;
    } else {
      if ( glyphs->stroke_shp->leftX < bbox.x0 ) bbox.x0=glyphs->stroke_shp->leftX;
      if ( glyphs->stroke_shp->rightX > bbox.x1 ) bbox.x1=glyphs->stroke_shp->rightX;
      if ( glyphs->stroke_shp->topY < bbox.y0 ) bbox.y0=glyphs->stroke_shp->topY;
      if ( glyphs->stroke_shp->bottomY > bbox.y1 ) bbox.y1=glyphs->stroke_shp->bottomY;
    }
  }
	if (nr_rect_d_test_empty(&bbox)) return NR_ARENA_ITEM_STATE_ALL;

	item->bbox.x0 = (gint32)(bbox.x0 - 1.0);
	item->bbox.y0 = (gint32)(bbox.y0 - 1.0);
	item->bbox.x1 = (gint32)(bbox.x1 + 1.0);
	item->bbox.y1 = (gint32)(bbox.y1 + 1.0);
	nr_arena_request_render_rect (item->arena, &item->bbox);

	return NR_ARENA_ITEM_STATE_ALL;
}

static guint
nr_arena_glyphs_clip (NRArenaItem *item, NRRectL *area, NRPixBlock *pb)
{
	NRArenaGlyphs *glyphs;

	glyphs = NR_ARENA_GLYPHS (item);

	if (!glyphs->font || !glyphs->curve) return item->state;

	/* TODO : render to greyscale pixblock provided for clipping */

	return item->state;
}

static NRArenaItem *
nr_arena_glyphs_pick (NRArenaItem *item, NR::Point p, gdouble delta, unsigned int sticky)
{
	NRArenaGlyphs *glyphs;

	glyphs = NR_ARENA_GLYPHS (item);

	if (!glyphs->font || !glyphs->curve) return NULL;
	if (!glyphs->style) return NULL;
	
	const double x = p[NR::X];
	const double y = p[NR::Y];
	/* fixme: pt in rect*/
	if ((x >= item->bbox.x0) && (y >= item->bbox.y0) && (x < item->bbox.x1) && (y < item->bbox.y1)) return item;

	NR::Point const thePt = p;
		if (glyphs->stroke_shp && (glyphs->style->stroke.type != SP_PAINT_TYPE_NONE)) {
			if (glyphs->stroke_shp->PtWinding(thePt) > 0 ) return item;
		}
		if (delta > 1e-3) {
			if (glyphs->stroke_shp && (glyphs->style->stroke.type != SP_PAINT_TYPE_NONE)) {
				if ( glyphs->stroke_shp->DistanceLE(thePt, delta)) return item;
			}
		}
  
	return NULL;
}

void
nr_arena_glyphs_set_path (NRArenaGlyphs *glyphs, SPCurve *curve, unsigned int lieutenant, NRFont *font, gint glyph, const NRMatrix *transform)
{
	nr_return_if_fail (glyphs != NULL);
	nr_return_if_fail (NR_IS_ARENA_GLYPHS (glyphs));

	nr_arena_item_request_render (NR_ARENA_ITEM (glyphs));
  
  glyphs->cached_shp_dirty=true;

	if (glyphs->curve) {
		sp_curve_unref (glyphs->curve);
		glyphs->curve = NULL;
	}

	if (curve) {
		if (transform) {
			NRBPath abp, s;
			s.path = curve->bpath;
			nr_path_duplicate_transform(&abp, &s, transform);
			//abp = art_bpath_affine_transform (curve->bpath, a);
			curve = sp_curve_new_from_bpath (abp.path);
			g_assert (curve != NULL);
			glyphs->curve = curve;
			glyphs->transform = *transform;
		} else {
			glyphs->curve = curve;
			sp_curve_ref (curve);
			nr_matrix_set_identity (&glyphs->transform);
		}
	}

	if (glyphs->font) glyphs->font = nr_font_unref (glyphs->font);
	if (font) glyphs->font = nr_font_ref (font);
	glyphs->glyph = glyph;

	nr_arena_item_request_update (NR_ARENA_ITEM (glyphs), NR_ARENA_ITEM_STATE_ALL, FALSE);
}

void
nr_arena_glyphs_set_style (NRArenaGlyphs *glyphs, SPStyle *style)
{
	nr_return_if_fail (glyphs != NULL);
	nr_return_if_fail (NR_IS_ARENA_GLYPHS (glyphs));

  glyphs->cached_style_dirty=true;
  
	if (style) sp_style_ref (style);
	if (glyphs->style) sp_style_unref (glyphs->style);
	glyphs->style = style;

	nr_arena_item_request_update (NR_ARENA_ITEM (glyphs), NR_ARENA_ITEM_STATE_ALL, FALSE);
}

static guint
nr_arena_glyphs_fill_mask (NRArenaGlyphs *glyphs, NRRectL *area, NRPixBlock *m)
{
	NRArenaItem *item;

	/* fixme: area == m->area, so merge these */

	item = NR_ARENA_ITEM (glyphs);

	if (glyphs->rfont && nr_rect_l_test_intersect (area, &item->bbox)) {
		nr_rasterfont_glyph_mask_render (glyphs->rfont, glyphs->glyph, m, glyphs->x, glyphs->y);
	}

	return item->state;
}

static guint
nr_arena_glyphs_stroke_mask (NRArenaGlyphs *glyphs, NRRectL *area, NRPixBlock *m)
{
	NRArenaItem *item;

	item = NR_ARENA_ITEM (glyphs);
	if (glyphs->stroke_shp && nr_rect_l_test_intersect (area, &item->bbox)) {
		NRPixBlock gb;
		gint x, y;
		nr_pixblock_setup_fast (&gb, NR_PIXBLOCK_MODE_A8, area->x0, area->y0, area->x1, area->y1, TRUE);
    // art_gray_svp_aa is just fillung apparently
    // dunno why it's used here instead of its libnr counterpart
    nr_pixblock_render_shape_mask_or (gb,glyphs->stroke_shp);    
		for (y = area->y0; y < area->y1; y++) {
			guchar *d, *s;
			d = NR_PIXBLOCK_PX (m) + (y - area->y0) * m->rs;
			s = NR_PIXBLOCK_PX (&gb) + (y - area->y0) * gb.rs;
			for (x = area->x0; x < area->x1; x++) {
				*d = (*d) + ((255 - *d) * (*s) / 255);
				d += 1;
				s += 1;
			}
		}
		nr_pixblock_release (&gb);
		m->empty = FALSE;
	}
  
	return item->state;
}

static void nr_arena_glyphs_group_class_init (NRArenaGlyphsGroupClass *klass);
static void nr_arena_glyphs_group_init (NRArenaGlyphsGroup *group);
static void nr_arena_glyphs_group_finalize (NRObject *object);

static guint nr_arena_glyphs_group_update (NRArenaItem *item, NRRectL *area, NRGC *gc, guint state, guint reset);
static unsigned int nr_arena_glyphs_group_render (NRArenaItem *item, NRRectL *area, NRPixBlock *pb, unsigned int flags);
static unsigned int nr_arena_glyphs_group_clip (NRArenaItem *item, NRRectL *area, NRPixBlock *pb);
static NRArenaItem *nr_arena_glyphs_group_pick (NRArenaItem *item, NR::Point p, gdouble delta, unsigned int sticky);

static NRArenaGroupClass *group_parent_class;

NRType
nr_arena_glyphs_group_get_type (void)
{
	static NRType type = 0;
	if (!type) {
		type = nr_object_register_type (NR_TYPE_ARENA_GROUP,
						"NRArenaGlyphsGroup",
						sizeof (NRArenaGlyphsGroupClass),
						sizeof (NRArenaGlyphsGroup),
						(void (*) (NRObjectClass *)) nr_arena_glyphs_group_class_init,
						(void (*) (NRObject *)) nr_arena_glyphs_group_init);
	}
	return type;
}

static void
nr_arena_glyphs_group_class_init (NRArenaGlyphsGroupClass *klass)
{
	NRObjectClass *object_class;
	NRArenaItemClass *item_class;

	object_class = (NRObjectClass *) klass;
	item_class = (NRArenaItemClass *) klass;

	group_parent_class = (NRArenaGroupClass *) ((NRObjectClass *) klass)->parent;

	object_class->finalize = nr_arena_glyphs_group_finalize;

	item_class->update = nr_arena_glyphs_group_update;
	item_class->render = nr_arena_glyphs_group_render;
	item_class->clip = nr_arena_glyphs_group_clip;
	item_class->pick = nr_arena_glyphs_group_pick;
}

static void
nr_arena_glyphs_group_init (NRArenaGlyphsGroup *group)
{
	group->style = NULL;
	group->paintbox.x0 = group->paintbox.y0 = 0.0F;
	group->paintbox.x1 = group->paintbox.y1 = 1.0F;

	group->fill_painter = NULL;
	group->stroke_painter = NULL;
}

static void
nr_arena_glyphs_group_finalize (NRObject *object)
{
	NRArenaGlyphsGroup *group;

	group = NR_ARENA_GLYPHS_GROUP (object);

	if (group->fill_painter) {
		sp_painter_free (group->fill_painter);
		group->fill_painter = NULL;
	}

	if (group->stroke_painter) {
		sp_painter_free (group->stroke_painter);
		group->stroke_painter = NULL;
	}

	if (group->style) {
		sp_style_unref (group->style);
		group->style = NULL;
	}

		((NRObjectClass *) group_parent_class)->finalize (object);
}

static guint
nr_arena_glyphs_group_update (NRArenaItem *item, NRRectL *area, NRGC *gc, guint state, guint reset)
{
	NRArenaGlyphsGroup *group;

	group = NR_ARENA_GLYPHS_GROUP (item);

	if (group->fill_painter) {
		sp_painter_free (group->fill_painter);
		group->fill_painter = NULL;
	}

	if (group->stroke_painter) {
		sp_painter_free (group->stroke_painter);
		group->stroke_painter = NULL;
	}

	item->render_opacity = TRUE;
	if (group->style->fill.type == SP_PAINT_TYPE_PAINTSERVER) {
		group->fill_painter = sp_paint_server_painter_new (SP_STYLE_FILL_SERVER (group->style),
							 NR::Matrix (&gc->transform), NR::Matrix (&gc->parent->transform),
							 &group->paintbox);
	item->render_opacity = FALSE;
	}

	if (group->style->stroke.type == SP_PAINT_TYPE_PAINTSERVER) {
		group->stroke_painter = sp_paint_server_painter_new (SP_STYLE_STROKE_SERVER (group->style),
							 NR::Matrix (&gc->transform), NR::Matrix (&gc->parent->transform),
							 &group->paintbox);
	item->render_opacity = FALSE;
	}
  if ( item->render_opacity == TRUE && group->style->stroke.type != SP_PAINT_TYPE_NONE && group->style->fill.type != SP_PAINT_TYPE_NONE ) {
    item->render_opacity=FALSE;
  }

	if (((NRArenaItemClass *) group_parent_class)->update)
		return ((NRArenaItemClass *) group_parent_class)->update (item, area, gc, state, reset);

	return NR_ARENA_ITEM_STATE_ALL;
}

/* This sucks - as soon, as we have inheritable renderprops, do something with that opacity */

static unsigned int
nr_arena_glyphs_group_render (NRArenaItem *item, NRRectL *area, NRPixBlock *pb, unsigned int flags)
{
	NRArenaGroup *group;
	NRArenaGlyphsGroup *ggroup;
	NRArenaItem *child;
	SPStyle *style;
	guint ret;

	group = NR_ARENA_GROUP (item);
	ggroup = NR_ARENA_GLYPHS_GROUP (item);
	style = ggroup->style;

	ret = item->state;

	/* Fill */
	if (style->fill.type != SP_PAINT_TYPE_NONE) {
		NRPixBlock mb;
		guint32 rgba;
		nr_pixblock_setup_fast (&mb, NR_PIXBLOCK_MODE_A8, area->x0, area->y0, area->x1, area->y1, TRUE);
		/* Render children fill mask */
		for (child = group->children; child != NULL; child = child->next) {
			ret = nr_arena_glyphs_fill_mask (NR_ARENA_GLYPHS (child), area, &mb);
			if (!(ret & NR_ARENA_ITEM_STATE_RENDER)) {
				nr_pixblock_release (&mb);
				return ret;
			}
		}
		/* Composite into buffer */
		switch (style->fill.type) {
		case SP_PAINT_TYPE_COLOR:
      if ( item->render_opacity ) {
        rgba = sp_color_get_rgba32_falpha (&style->fill.value.color,
                                           SP_SCALE24_TO_FLOAT (style->fill_opacity.value) *
                                           SP_SCALE24_TO_FLOAT (style->opacity.value));
      } else {
        rgba = sp_color_get_rgba32_falpha (&style->fill.value.color,
							   SP_SCALE24_TO_FLOAT (style->fill_opacity.value));
      }
			nr_blit_pixblock_mask_rgba32 (pb, &mb, rgba);
			pb->empty = FALSE;
			break;
		case SP_PAINT_TYPE_PAINTSERVER:
			if (ggroup->fill_painter) {
				NRPixBlock cb;
				/* Need separate gradient buffer */
				nr_pixblock_setup_fast (&cb, NR_PIXBLOCK_MODE_R8G8B8A8N, area->x0, area->y0, area->x1, area->y1, TRUE);
				ggroup->fill_painter->fill (ggroup->fill_painter, &cb);
				cb.empty = FALSE;
				/* Composite */
				nr_blit_pixblock_pixblock_mask (pb, &cb, &mb);
				pb->empty = FALSE;
				nr_pixblock_release (&cb);
			}
			break;
		default:
			break;
		}
		nr_pixblock_release (&mb);
	}

	/* Stroke */
	if (style->stroke.type != SP_PAINT_TYPE_NONE) {
		NRPixBlock m;
		guint32 rgba;
		nr_pixblock_setup_fast (&m, NR_PIXBLOCK_MODE_A8, area->x0, area->y0, area->x1, area->y1, TRUE);
		/* Render children stroke mask */
		for (child = group->children; child != NULL; child = child->next) {
			ret = nr_arena_glyphs_stroke_mask (NR_ARENA_GLYPHS (child), area, &m);
			if (!(ret & NR_ARENA_ITEM_STATE_RENDER)) {
				nr_pixblock_release (&m);
				return ret;
			}
		}
		/* Composite into buffer */
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
			if (ggroup->stroke_painter) {
				NRPixBlock cb;
				/* Need separate gradient buffer */
				nr_pixblock_setup_fast (&cb, NR_PIXBLOCK_MODE_R8G8B8A8N, area->x0, area->y0, area->x1, area->y1, TRUE);
				ggroup->stroke_painter->fill (ggroup->stroke_painter, &cb);
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
	}

	return ret;
}

static unsigned int
nr_arena_glyphs_group_clip (NRArenaItem *item, NRRectL *area, NRPixBlock *pb)
{
	NRArenaGroup *group = NR_ARENA_GROUP (item);

	guint ret = item->state;

	/* Render children fill mask */
	for (NRArenaItem *child = group->children; child != NULL; child = child->next) {
		ret = nr_arena_glyphs_fill_mask (NR_ARENA_GLYPHS (child), area, pb);
		if (!(ret & NR_ARENA_ITEM_STATE_RENDER)) return ret;
	}

	return ret;
}

static NRArenaItem *
nr_arena_glyphs_group_pick (NRArenaItem *item, NR::Point p, gdouble delta, unsigned int sticky)
{
	NRArenaItem *picked = NULL;

	if (((NRArenaItemClass *) group_parent_class)->pick)
		picked = ((NRArenaItemClass *) group_parent_class)->pick (item, p, delta, sticky);

	if (picked) picked = item;

	return picked;
}

void
nr_arena_glyphs_group_clear (NRArenaGlyphsGroup *sg)
{
	NRArenaGroup *group = NR_ARENA_GROUP (sg);

	nr_arena_item_request_render (NR_ARENA_ITEM (group));

	while (group->children) {
		nr_arena_item_remove_child (NR_ARENA_ITEM (group), group->children);
	}
}

void
nr_arena_glyphs_group_add_component (NRArenaGlyphsGroup *sg, NRFont *font, int glyph, const NRMatrix *transform)
{
	NRArenaGroup *group;
	NRBPath bpath;

	group = NR_ARENA_GROUP (sg);

	if (nr_font_glyph_outline_get (font, glyph, &bpath, FALSE) && bpath.path) {
		SPCurve *curve;

		nr_arena_item_request_render (NR_ARENA_ITEM (group));

		curve = sp_curve_new_from_foreign_bpath (bpath.path);
		if (curve) {
			NRArenaItem *new_arena;
			new_arena = NRArenaGlyphs::create(group->arena);
			nr_arena_item_append_child (NR_ARENA_ITEM (group), new_arena);
			nr_arena_item_unref (new_arena);
			nr_arena_glyphs_set_path (NR_ARENA_GLYPHS (new_arena), curve, FALSE, font, glyph, transform);
			nr_arena_glyphs_set_style (NR_ARENA_GLYPHS (new_arena), sg->style);
			sp_curve_unref (curve);
		}
	}

}

void
nr_arena_glyphs_group_set_style (NRArenaGlyphsGroup *sg, SPStyle *style)
{
	NRArenaGroup *group;
	NRArenaItem *child;

	nr_return_if_fail (sg != NULL);
	nr_return_if_fail (NR_IS_ARENA_GLYPHS_GROUP (sg));

	group = NR_ARENA_GROUP (sg);

	if (style) sp_style_ref (style);
	if (sg->style) sp_style_unref (sg->style);
	sg->style = style;

	for (child = group->children; child != NULL; child = child->next) {
		nr_return_if_fail (NR_IS_ARENA_GLYPHS (child));
		nr_arena_glyphs_set_style (NR_ARENA_GLYPHS (child), sg->style);
	}

	nr_arena_item_request_update (NR_ARENA_ITEM (sg), NR_ARENA_ITEM_STATE_ALL, FALSE);
}

void
nr_arena_glyphs_group_set_paintbox (NRArenaGlyphsGroup *gg, const NRRect *pbox)
{
	nr_return_if_fail (gg != NULL);
	nr_return_if_fail (NR_IS_ARENA_GLYPHS_GROUP (gg));
	nr_return_if_fail (pbox != NULL);

	if ((pbox->x0 < pbox->x1) && (pbox->y0 < pbox->y1)) {
		gg->paintbox.x0 = pbox->x0;
		gg->paintbox.y0 = pbox->y0;
		gg->paintbox.x1 = pbox->x1;
		gg->paintbox.y1 = pbox->y1;
	} else {
		/* fixme: We kill warning, although not sure what to do here (Lauris) */
		gg->paintbox.x0 = gg->paintbox.y0 = 0.0F;
		gg->paintbox.x1 = gg->paintbox.y1 = 256.0F;
	}

	nr_arena_item_request_update (NR_ARENA_ITEM (gg), NR_ARENA_ITEM_STATE_ALL, FALSE);
}

