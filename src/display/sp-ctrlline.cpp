#define __INKSCAPE_CTRLLINE_C__

/*
 * Simple straight line
 *
 * Author:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * Copyright (C) 1999-2002 Lauris Kaplinski
 *
 * Released under GNU GPL
 */

/*
 * TODO:
 * Draw it by hand - we really do not need aa stuff for it
 *
 */

#include <math.h>
#include "sp-canvas.h"
#include "sp-canvas-util.h"
#include "sp-ctrlline.h"

#include <libart_lgpl/art_affine.h>
#include <libart_lgpl/art_vpath.h>
#include <libart_lgpl/art_svp.h>
#include <libart_lgpl/art_svp_vpath.h>
#include <libart_lgpl/art_svp_vpath_stroke.h>
#include <libart_lgpl/art_rgb_svp.h>
#include <libart_lgpl/art_rect.h>
#include <libart_lgpl/art_rect_svp.h>

#define ctrl_liv

#ifdef ctrl_liv
#include <config.h>
#include <livarot/Shape.h>
#include <livarot/Path.h>
#include <livarot/AlphaLigne.h>
#include <livarot/Ligne.h>
#include <livarot/BitLigne.h>
#include <libnr/nr-point.h>
#include <libnr/nr-matrix.h>
#include <libnr/nr-point-ops.h>
#include <libnr/nr-matrix-ops.h>
#include <libnr/nr-rect-l.h>
#include <libnr/nr-pixops.h>
#include <libnr/nr-compose.h>

void nr_pixblock_render_ctrl_rgba (Shape* theS,uint32_t color,NRRectL &area,char* destBuf,int stride);
#endif

struct _SPCtrlLine {
	SPCanvasItem item;

	guint32 rgba;
	ArtPoint s, e;
#ifdef ctrl_liv
	Shape* shp;
#else
  ArtSVP *svp;
#endif
};

struct _SPCtrlLineClass {
	SPCanvasItemClass parent_class;
};

static void sp_ctrlline_class_init (SPCtrlLineClass *klass);
static void sp_ctrlline_init (SPCtrlLine *ctrlline);
static void sp_ctrlline_destroy (GtkObject *object);

static void sp_ctrlline_update (SPCanvasItem *item, NR::Matrix const &affine, unsigned int flags);
static void sp_ctrlline_render (SPCanvasItem *item, SPCanvasBuf *buf);

static SPCanvasItemClass *parent_class;

GtkType
sp_ctrlline_get_type (void)
{
	static GtkType type = 0;

	if (!type) {
		GtkTypeInfo info = {
			"SPCtrlLine",
			sizeof (SPCtrlLine),
			sizeof (SPCtrlLineClass),
			(GtkClassInitFunc) sp_ctrlline_class_init,
			(GtkObjectInitFunc) sp_ctrlline_init,
			NULL, NULL, NULL
		};
		type = gtk_type_unique (SP_TYPE_CANVAS_ITEM, &info);
	}
	return type;
}

static void
sp_ctrlline_class_init (SPCtrlLineClass *klass)
{
	GtkObjectClass *object_class;
	SPCanvasItemClass *item_class;

	object_class = (GtkObjectClass *) klass;
	item_class = (SPCanvasItemClass *) klass;

	parent_class = (SPCanvasItemClass*)gtk_type_class (SP_TYPE_CANVAS_ITEM);

	object_class->destroy = sp_ctrlline_destroy;

	item_class->update = sp_ctrlline_update;
	item_class->render = sp_ctrlline_render;
}

static void
sp_ctrlline_init (SPCtrlLine *ctrlline)
{
	ctrlline->rgba = 0x0000ff7f;
	ctrlline->s.x = ctrlline->s.y = ctrlline->e.x = ctrlline->e.y = 0.0;
#ifdef ctrl_liv
  ctrlline->shp=NULL;
#else
	ctrlline->svp = NULL;
#endif
}

static void
sp_ctrlline_destroy (GtkObject *object)
{
	SPCtrlLine *ctrlline;

	g_return_if_fail (object != NULL);
	g_return_if_fail (SP_IS_CTRLLINE (object));

	ctrlline = SP_CTRLLINE (object);

#ifdef ctrl_liv
	if (ctrlline->shp) {
		delete ctrlline->shp;
		ctrlline->shp = NULL;
	}
#else
	if (ctrlline->svp) {
		art_svp_free (ctrlline->svp);
		ctrlline->svp = NULL;
	}
#endif
  
	if (GTK_OBJECT_CLASS (parent_class)->destroy)
		(* GTK_OBJECT_CLASS (parent_class)->destroy) (object);
}

static void
sp_ctrlline_render (SPCanvasItem *item, SPCanvasBuf *buf)
{
	SPCtrlLine *ctrlline;

	ctrlline = SP_CTRLLINE (item);

	if (buf->is_bg) {
		sp_canvas_clear_buffer (buf);
		buf->is_bg = FALSE;
		buf->is_buf = TRUE;
	}
#ifdef ctrl_liv
  NRRectL  area;
  area.x0=buf->rect.x0;
  area.x1=buf->rect.x1;
  area.y0=buf->rect.y0;
  area.y1=buf->rect.y1;
	if (ctrlline->shp) {
		sp_canvas_buf_ensure_buf (buf);
    nr_pixblock_render_ctrl_rgba (ctrlline->shp,ctrlline->rgba,area,(char*)buf->buf, buf->buf_rowstride);
	}
#else
	if (ctrlline->svp) {
		sp_canvas_buf_ensure_buf (buf);
		art_rgb_svp_alpha (ctrlline->svp, buf->rect.x0, buf->rect.y0, buf->rect.x1, buf->rect.y1, ctrlline->rgba,
				   buf->buf, buf->buf_rowstride,
				   NULL);
	}
#endif
}

static void
sp_ctrlline_update (SPCanvasItem *item, NR::Matrix const &affine, unsigned int flags)
{
	NRRect dbox;

	SPCtrlLine *cl = SP_CTRLLINE (item);

	sp_canvas_request_redraw (item->canvas, (int)item->x1, (int)item->y1, (int)item->x2, (int)item->y2);

	if (parent_class->update)
		(* parent_class->update) (item, affine, flags);

	sp_canvas_item_reset_bounds (item);

#ifdef ctrl_liv
	dbox.x0=dbox.x1=dbox.y0=dbox.y1=0;
	if (cl->shp) {
		delete cl->shp;
		cl->shp = NULL;
	}
	Path* thePath = new Path;
	thePath->MoveTo(NR::Point(cl->s.x, cl->s.y) * affine);
	thePath->LineTo(NR::Point(cl->e.x, cl->e.y) * affine);
  
	thePath->Convert(1.0);
	if ( cl->shp == NULL ) cl->shp=new Shape;
	thePath->Stroke(cl->shp,false,0.5,join_straight,butt_straight,20.0,false);
	cl->shp->CalcBBox();
	if ( cl->shp->leftX < cl->shp->rightX ) {
		if ( dbox.x0 >= dbox.x1 ) {
			dbox.x0=cl->shp->leftX;dbox.x1=cl->shp->rightX;
			dbox.y0=cl->shp->topY;dbox.y1=cl->shp->bottomY;
		} else {
			if ( cl->shp->leftX < dbox.x0 ) dbox.x0=cl->shp->leftX;
			if ( cl->shp->rightX > dbox.x1 ) dbox.x1=cl->shp->rightX;
			if ( cl->shp->topY < dbox.y0 ) dbox.y0=cl->shp->topY;
			if ( cl->shp->bottomY > dbox.y1 ) dbox.y1=cl->shp->bottomY;
		}
	}
	delete thePath;
#else
// clearly wrong.
#endif
  
	item->x1 = (int)dbox.x0;
	item->y1 = (int)dbox.y0;
	item->x2 = (int)dbox.x1;
	item->y2 = (int)dbox.y1;

	sp_canvas_request_redraw (item->canvas, (int)item->x1, (int)item->y1, (int)item->x2, (int)item->y2);
}

void
sp_ctrlline_set_rgba32 (SPCtrlLine *cl, guint32 rgba)
{
	g_return_if_fail (cl != NULL);
	g_return_if_fail (SP_IS_CTRLLINE (cl));

	if (rgba != cl->rgba) {
		SPCanvasItem *item;
		cl->rgba = rgba;
		item = SP_CANVAS_ITEM (cl);
		sp_canvas_request_redraw (item->canvas, (int)item->x1, (int)item->y1, (int)item->x2, (int)item->y2);
	}
}

#define EPSILON 1e-6
#define DIFFER(a,b) (fabs ((a) - (b)) > EPSILON)

void
sp_ctrlline_set_coords (SPCtrlLine *cl, gdouble x0, gdouble y0, gdouble x1, gdouble y1)
{
	g_return_if_fail (cl != NULL);
	g_return_if_fail (SP_IS_CTRLLINE (cl));

	if (DIFFER (x0, cl->s.x) || DIFFER (y0, cl->s.y) || DIFFER (x1, cl->e.x) || DIFFER (y1, cl->e.y)) {
		cl->s.x = x0;
		cl->s.y = y0;
		cl->e.x = x1;
		cl->e.y = y1;
		sp_canvas_item_request_update (SP_CANVAS_ITEM (cl));
	}
}

void
sp_ctrlline_set_coords (SPCtrlLine *cl, const NR::Point start, const NR::Point end)
{
	sp_ctrlline_set_coords(cl, start[0], start[1], end[0], end[1]);
}

#ifdef ctrl_liv

static void
ctrl_run_A8_OR (raster_info &dest,void *data,int st,float vst,int en,float ven)
{
  union {
    uint8_t  comp[4];
    uint32_t col;
  } tempCol;
  if ( st >= en ) return;
  tempCol.col=*(uint32_t*)data;
  
	unsigned int r, g, b, a;
	r = NR_RGBA32_R (tempCol.col);
	g = NR_RGBA32_G (tempCol.col);
	b = NR_RGBA32_B (tempCol.col);
	a = NR_RGBA32_A (tempCol.col);
	if (a == 0) return;
  
  vst*=a;
  ven*=a;
  
  if ( vst < 0 ) vst=0;
  if ( vst > 255 ) vst=255;
  if ( ven < 0 ) ven=0;
  if ( ven > 255 ) ven=255;
  float      sv=vst;
  float      dv=ven-vst;
  int        len=en-st;
  uint8_t*   d=(uint8_t*)dest.buffer;
  
  d+=3*(st-dest.startPix);
  if ( fabsf(dv) < 0.001 ) {
    if ( sv > 249.999 ) {
      /* Simple copy */
      while (len > 0) {
        d[0] = INK_COMPOSE (r, 255, d[0]);
        d[1] = INK_COMPOSE (g, 255, d[1]);
        d[2] = INK_COMPOSE (b, 255, d[2]);
        d += 3;
        len -= 1;
      }
    } else {
      unsigned int c0_24=(int)sv;
      c0_24&=0xFF;
      while (len > 0) {
        d[0] = INK_COMPOSE (r, c0_24, d[0]);
        d[1] = INK_COMPOSE (g, c0_24, d[1]);
        d[2] = INK_COMPOSE (b, c0_24, d[2]);
        d += 3;
        len -= 1;
      }
    }
  } else {
    if ( en <= st+1 ) {
      sv=0.5*(vst+ven);
      unsigned int c0_24=(int)sv;
      c0_24&=0xFF;
      d[0] = INK_COMPOSE (r, c0_24, d[0]);
      d[1] = INK_COMPOSE (g, c0_24, d[1]);
      d[2] = INK_COMPOSE (b, c0_24, d[2]);
    } else {
      dv/=len;
      sv+=0.5*dv; // correction trapezoidale
      sv*=65536;
      dv*=65536;
      int c0_24 = static_cast<int>(CLAMP(sv, 0, 16777216));
      int s0_24 = static_cast<int>(dv);
      while (len > 0) {
        unsigned int ca;
        /* Draw */
        ca = c0_24 >> 16;
        if ( ca > 255 ) ca=255;
        d[0] = INK_COMPOSE (r, ca, d[0]);
        d[1] = INK_COMPOSE (g, ca, d[1]);
        d[2] = INK_COMPOSE (b, ca, d[2]);
        d += 3;
        c0_24 += s0_24;
        c0_24 = CLAMP (c0_24, 0, 16777216);
        len -= 1;
      }
    }
  }
}

void nr_pixblock_render_ctrl_rgba (Shape* theS,uint32_t color,NRRectL &area,char* destBuf,int stride)
{
  
  theS->CalcBBox();
  float  l=theS->leftX,r=theS->rightX,t=theS->topY,b=theS->bottomY;
  int    il,ir,it,ib;
  il=(int)floorf(l);
  ir=(int)ceilf(r);
  it=(int)floorf(t);
  ib=(int)ceilf(b);
  
//  printf("bbox %i %i %i %i  render %i %i %i %i\n",il,it,ir,ib,area.x0,area.y0,area.x1,area.y1);
  
  if ( il >= area.x1 || ir <= area.x0 || it >= area.y1 || ib <= area.y0 ) return;
  if ( il < area.x0 ) il=area.x0;
  if ( it < area.y0 ) it=area.y0;
  if ( ir > area.x1 ) ir=area.x1;
  if ( ib > area.y1 ) ib=area.y1;
  
/*  // version par FloatLigne
  int    curPt;
  float  curY;
  theS->BeginRaster(curY,curPt,1.0);
  
  FloatLigne* theI=new FloatLigne();
  IntLigne*   theIL=new IntLigne();
  
  theS->Scan(curY,curPt,(float)(it),1.0);
  
  char* mdata=(char*)destBuf;
  uint32_t* ligStart=((uint32_t*)(mdata+(3*(il-area.x0)+stride*(it-area.y0))));
  for (int y=it;y<ib;y++) {
    theI->Reset();
    if ( y&0x00000003 ) {
      theS->Scan(curY,curPt,((float)(y+1)),theI,false,1.0);
    } else {
      theS->Scan(curY,curPt,((float)(y+1)),theI,true,1.0);
    }
    theI->Flatten();
    theIL->Copy(theI);
    
    raster_info  dest;
    dest.startPix=il;
    dest.endPix=ir;
    dest.sth=il;
    dest.stv=y;
    dest.buffer=ligStart;
    theIL->Raster(dest,&color,bpath_run_A8_OR);
    ligStart=((uint32_t*)(((char*)ligStart)+stride));
  }
  theS->EndRaster();
  delete theI;
  delete theIL;  */
  
    // version par BitLigne directe
    int    curPt;
  float  curY;
  theS->BeginQuickRaster(curY,curPt,1.0);
  
  BitLigne*   theI[4];
  for (int i=0;i<4;i++) theI[i]=new BitLigne(il,ir);
  IntLigne*   theIL=new IntLigne();
  
  theS->QuickScan(curY,curPt,(float)(it),true,0.25);
  
  char* mdata=(char*)destBuf;
  uint32_t* ligStart=((uint32_t*)(mdata+(3*(il-area.x0)+stride*(it-area.y0))));
  for (int y=it;y<ib;y++) {
    for (int i=0;i<4;i++) theI[i]->Reset();
    if ( y&3 ) {
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
    theIL->Raster(dest,&color,ctrl_run_A8_OR);
    ligStart=((uint32_t*)(((char*)ligStart)+stride));
  }
  theS->EndQuickRaster();
  for (int i=0;i<4;i++) delete theI[i];
  delete theIL;    
}

#endif

