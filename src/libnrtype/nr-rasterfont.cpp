#define __NR_RASTERFONT_C__

/*
 * Typeface and script library
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * This code is in public domain
 */

#define noRFDEBUG

#include <string.h>

#include <libnr/nr-macros.h>
#include <libnr/nr-point-l.h>
#include <libnr/nr-rect.h>
#include <libnr/nr-rect-l.h>
#include <libnr/nr-matrix.h>
#include <libnr/nr-matrix-ops.h>
#include <libnr/nr-pixops.h>
#include <libnr/nr-svp.h>
#include <libnr/nr-svp-render.h>

#include <livarot/Shape.h>
#include <livarot/Path.h>
#include <livarot/Ligne.h>
#include <livarot/LivarotDefs.h>

void nrrf_pixblock_render_shape_mask_or (NRPixBlock &m,Shape* theS);

#include <libnr/nr-svp-private.h>

#include "nr-rasterfont.h"

NRRasterFont *
nr_rasterfont_ref (NRRasterFont *rf)
{
    rf->refcount += 1;

    return rf;
}

NRRasterFont *
nr_rasterfont_unref (NRRasterFont *rf)
{
    rf->refcount -= 1;

    if (rf->refcount < 1) {
        ((NRTypeFaceClass *) ((NRObject *) rf->font->face)->klass)->rasterfont_free (rf);
    }

    return NULL;
}

NR::Point nr_rasterfont_glyph_advance_get (NRRasterFont *rf, int glyph)
{
    return ((NRTypeFaceClass *) ((NRObject *) rf->font->face)->klass)->rasterfont_glyph_advance_get (rf, glyph);
}

NRRect *
nr_rasterfont_glyph_area_get (NRRasterFont *rf, int glyph, NRRect *area)
{
    return ((NRTypeFaceClass *) ((NRObject *) rf->font->face)->klass)->rasterfont_glyph_area_get (rf, glyph, area);
}

void
nr_rasterfont_glyph_mask_render (NRRasterFont *rf, int glyph, NRPixBlock *mask, float x, float y)
{
    ((NRTypeFaceClass *) ((NRObject *) rf->font->face)->klass)->rasterfont_glyph_mask_render (rf, glyph, mask, x, y);
}

/* Generic implementation */

#define NRRF_PAGEBITS 6
#define NRRF_PAGE_SIZE (1 << NRRF_PAGEBITS)
#define NRRF_PAGE_MASK ((1 << NRRF_PAGEBITS) - 1)

#define NR_RASTERFONT_ADVANCE_FLAG (1 << 0)
#define NR_RASTERFONT_BBOX_FLAG (1 << 0)
#define NR_RASTERFONT_GMAP_FLAG (1 << 0)

/* Maximum image size for tiny */
#define NRRF_TINY_MAX_SIZE 16

#define NRRF_MAX_GLYPH_DIMENSION 256
#define NRRF_MAX_GLYPH_SIZE 32 * 32

#define NRRF_COORD_INT_LOWER(i) ((i) >> 6)
#define NRRF_COORD_INT_UPPER(i) (((i) + 63) >> 6)
#define NRRF_COORD_INT_SIZE(i0,i1) (NRRF_COORD_INT_UPPER (i1) - NRRF_COORD_INT_LOWER (i0))
#define NRRF_COORD_TO_FLOAT(i) ((double) (i) / 64.0)
#define NRRF_COORD_FROM_FLOAT_LOWER(f) ((int) (f * 64.0))
#define NRRF_COORD_FROM_FLOAT_UPPER(f) ((int) (f * 64.0 + 63.999999))

enum {
    NRRF_TYPE_NONE,
    NRRF_TYPE_TINY,
    NRRF_TYPE_IMAGE,
    NRRF_TYPE_SVP,
    NRRF_TYPE_LIV
};

struct NRRFGlyphTiny {
    /* 26.6 fixed point */
    NRPointL advance;
    /* 26.6 fixed point */
    NRRectL bbox;
    /* Image */
    unsigned char px[16];
};

struct NRRFGlyphImage {
    /* 26.6 fixed point */
    NRPointL advance;
    /* 26.6 fixed point */
    NRRectL bbox;
    /* Image */
    unsigned char *px;
};

struct NRRFGlyphSVP {
    /* 26.6 fixed point */
    NRPointL advance;
    /* 26.6 fixed point */
    NRRectL bbox;
    /* Image */
    NRSVP *svp;
};

struct NRRFGlyphLIV {
    /* 26.6 fixed point */
    NRPointL advance;
    /* 26.6 fixed point */
    NRRectL bbox;
    /* Image */
    Shape *shp;
    Path  *delayed;
    NRRect shbbox;
};

struct NRRFGlyphSlot {
    unsigned int type : 3;
    unsigned int has_advance : 1;
    unsigned int has_bbox : 1;
    unsigned int has_gmap : 1;
    union {
        struct NRRFGlyphTiny tg;
        struct NRRFGlyphImage ig;
        struct NRRFGlyphSVP sg;
        struct NRRFGlyphLIV lg;
    } glyph;
};


static NRRFGlyphSlot *nr_rasterfont_ensure_glyph_slot (NRRasterFont *rf, unsigned int glyph, unsigned int flags);

NRRasterFont *
nr_rasterfont_generic_new (NRFont *font, NR::Matrix transform)
{
    NRRasterFont *rf;

    rf = nr_new (NRRasterFont, 1);

    rf->refcount = 1;
    rf->next = NULL;
    rf->font = nr_font_ref (font);
    rf->transform = transform;
    /* fixme: How about subpixel positioning */
    rf->transform[4] = 0.0;
    rf->transform[5] = 0.0;
    rf->nglyphs = NR_FONT_NUM_GLYPHS (font);
    rf->pages = NULL;

    return rf;
}

void
nr_rasterfont_generic_free (NRRasterFont *rf)
{
    if (rf->pages) {
        int npages, p;
        npages = rf->nglyphs / NRRF_PAGE_SIZE;
        for (p = 0; p < npages; p++) {
            if (rf->pages[p]) {
                NRRFGlyphSlot *slots;
                int s;
                slots = rf->pages[p];
                for (s = 0; s < NRRF_PAGE_SIZE; s++) {
                    if (slots[s].type == NRRF_TYPE_IMAGE) {
                        nr_free (slots[s].glyph.ig.px);
                    } else if (slots[s].type == NRRF_TYPE_SVP) {
                        nr_svp_free (slots[s].glyph.sg.svp);
                    } else if (slots[s].type == NRRF_TYPE_LIV) {
                        delete slots[s].glyph.lg.shp;
                        if ( slots[s].glyph.lg.delayed ) delete slots[s].glyph.lg.delayed;
                    }
                }
                nr_free (rf->pages[p]);
            }
        }
        nr_free (rf->pages);
    }
    nr_font_unref (rf->font);
    nr_free (rf);
}

NR::Point nr_rasterfont_generic_glyph_advance_get (NRRasterFont *rf, unsigned int glyph)
{
    return nr_font_glyph_advance_get(rf->font, glyph) * rf->transform;
}

NRRect *
nr_rasterfont_generic_glyph_area_get (NRRasterFont *rf, unsigned int glyph, NRRect *area)
{
    NRRFGlyphSlot *slot;

    glyph = MIN(glyph, rf->nglyphs);

    slot = nr_rasterfont_ensure_glyph_slot (rf, glyph, NR_RASTERFONT_BBOX_FLAG | NR_RASTERFONT_GMAP_FLAG);

    switch (slot->type) {
    case NRRF_TYPE_TINY:
    case NRRF_TYPE_IMAGE:
        area->x0 = NRRF_COORD_TO_FLOAT (slot->glyph.tg.bbox.x0);
        area->y0 = NRRF_COORD_TO_FLOAT (slot->glyph.tg.bbox.y0);
        area->x1 = NRRF_COORD_TO_FLOAT (slot->glyph.tg.bbox.x1);
        area->y1 = NRRF_COORD_TO_FLOAT (slot->glyph.tg.bbox.y1);
        break;
    case NRRF_TYPE_SVP:
        nr_svp_bbox (slot->glyph.sg.svp, area, TRUE);
        break;
    case NRRF_TYPE_LIV:
    {
        if ( slot->glyph.lg.delayed ) {
            *area=slot->glyph.lg.shbbox;
        } else {
            slot->glyph.lg.shp->CalcBBox();
            area->x0=slot->glyph.lg.shp->leftX;
            area->x1=slot->glyph.lg.shp->rightX;
            area->y0=slot->glyph.lg.shp->topY;
            area->y1=slot->glyph.lg.shp->bottomY;
        }
    }
    break;
    default:
        break;
    }

    return area;
}

void
nr_rasterfont_generic_glyph_mask_render (NRRasterFont *rf, unsigned int glyph, NRPixBlock *m, float x, float y)
{
    NRRFGlyphSlot *slot;
    NRRectL area;
    int sx, sy;
    unsigned char *spx = NULL;
    int srs = 0;
    NRPixBlock spb;

    glyph = MIN(glyph, rf->nglyphs);

    slot = nr_rasterfont_ensure_glyph_slot (rf, glyph, NR_RASTERFONT_BBOX_FLAG | NR_RASTERFONT_GMAP_FLAG);

    sx = (int) floor (x + 0.5);
    sy = (int) floor (y + 0.5);

    spb.empty = TRUE;

    switch (slot->type) {
    case NRRF_TYPE_TINY:
        if (nr_rect_l_test_empty (&slot->glyph.tg.bbox)) return;
        spx = slot->glyph.tg.px;
        srs = NRRF_COORD_INT_SIZE (slot->glyph.tg.bbox.x0, slot->glyph.tg.bbox.x1);
        area.x0 = NRRF_COORD_INT_LOWER (slot->glyph.tg.bbox.x0) + sx;
        area.y0 = NRRF_COORD_INT_LOWER (slot->glyph.tg.bbox.y0) + sy;
        area.x1 = NRRF_COORD_INT_UPPER (slot->glyph.tg.bbox.x1) + sx;
        area.y1 = NRRF_COORD_INT_UPPER (slot->glyph.tg.bbox.y1) + sy;
        break;
    case NRRF_TYPE_IMAGE:
        spx = slot->glyph.ig.px;
        srs = NRRF_COORD_INT_SIZE (slot->glyph.ig.bbox.x0, slot->glyph.ig.bbox.x1);
        area.x0 = NRRF_COORD_INT_LOWER (slot->glyph.ig.bbox.x0) + sx;
        area.y0 = NRRF_COORD_INT_LOWER (slot->glyph.ig.bbox.y0) + sy;
        area.x1 = NRRF_COORD_INT_UPPER (slot->glyph.ig.bbox.x1) + sx;
        area.y1 = NRRF_COORD_INT_UPPER (slot->glyph.ig.bbox.y1) + sy;
        break;
    case NRRF_TYPE_SVP:
        nr_pixblock_setup_extern (&spb, NR_PIXBLOCK_MODE_A8,
                                  m->area.x0 - sx, m->area.y0 - sy, m->area.x1 - sx, m->area.y1 - sy,
                                  NR_PIXBLOCK_PX (m), m->rs, FALSE, FALSE);
        nr_pixblock_render_svp_mask_or (&spb, slot->glyph.sg.svp);
        nr_pixblock_release (&spb);
        return;
    case NRRF_TYPE_LIV:
        // rasterization is position independent? wtf?
        // maybe translating/transforming the shape prior rendering would be more clever
        // or are each glyph given a slot? (very inefficient)
        if (slot->glyph.lg.delayed ) {
            Shape* theShape=new Shape;
            slot->glyph.lg.delayed->Convert(0.25);
            slot->glyph.lg.delayed->Fill(theShape,0);
            slot->glyph.lg.shp->ConvertToShape(theShape,fill_nonZero);
            delete theShape;
            delete slot->glyph.lg.delayed;
            slot->glyph.lg.delayed=NULL;
        }
        nr_pixblock_setup_extern (&spb, NR_PIXBLOCK_MODE_A8,
                                  m->area.x0 - sx, m->area.y0 - sy, m->area.x1 - sx, m->area.y1 - sy,
                                  NR_PIXBLOCK_PX (m), m->rs, FALSE, FALSE);
        nrrf_pixblock_render_shape_mask_or (spb, slot->glyph.lg.shp);
        nr_pixblock_release (&spb);
        break;
    default:
        break;
    }

    if (nr_rect_l_test_intersect (&area, &m->area)) {
        // seems to be only for the case where the glyph is an image (otherwise spx=NULL)
        // so we add that check
        // the code was maybe relying on bounding boxes to handle this segregation
        if ( slot->type == NRRF_TYPE_TINY || slot->type == NRRF_TYPE_IMAGE ) {
            NRRectL clip;
            int x, y;
            nr_rect_l_intersect (&clip, &area, &m->area);
            for (y = clip.y0; y < clip.y1; y++) {
                unsigned char *d, *s;
                s = spx + (y - area.y0) * srs + (clip.x0 - area.x0);
                d = NR_PIXBLOCK_PX (m) + (y - m->area.y0) * m->rs + (clip.x0 - m->area.x0);
                for (x = clip.x0; x < clip.x1; x++) {
                    *d = (NR_A7 (*s, *d) + 127) / 255;
                    s += 1;
                    d += 1;
                }
            }
        }
    }

    if (!spb.empty) nr_pixblock_release (&spb);
}

static NRRFGlyphSlot *
nr_rasterfont_ensure_glyph_slot (NRRasterFont *rf, unsigned int glyph, unsigned int flags)
{
    NRRFGlyphSlot *slot;
    unsigned int page, code;

    page = glyph / NRRF_PAGE_SIZE;
    code = glyph % NRRF_PAGE_SIZE;

    if (!rf->pages) {
        rf->pages = nr_new (NRRFGlyphSlot *, rf->nglyphs / NRRF_PAGE_SIZE + 1);
        memset (rf->pages, 0x0, (rf->nglyphs / NRRF_PAGE_SIZE + 1) * sizeof (NRRFGlyphSlot *));
    }

    if (!rf->pages[page]) {
        rf->pages[page] = nr_new (NRRFGlyphSlot, NRRF_PAGE_SIZE);
        memset (rf->pages[page], 0x0, NRRF_PAGE_SIZE * sizeof (NRRFGlyphSlot));
    }

    slot = rf->pages[page] + code;

    if ((flags & NR_RASTERFONT_ADVANCE_FLAG) && !slot->has_advance) {
        NR::Point const a(nr_font_glyph_advance_get(rf->font, glyph));
        NR::Point const tp(a * rf->transform);
        NRPointL ip;
        ip.x = static_cast<NR::ICoord>(tp[NR::X]);
        ip.y = static_cast<NR::ICoord>(tp[NR::Y]);
        switch (slot->type) {
        case NRRF_TYPE_TINY:
            slot->glyph.tg.advance = ip;
            break;
        case NRRF_TYPE_IMAGE:
            slot->glyph.ig.advance = ip;
            break;
        case NRRF_TYPE_SVP:
            slot->glyph.sg.advance = ip;
            break;
        case NRRF_TYPE_LIV:
            slot->glyph.lg.advance = ip;
            break;
        default:
            break;
        }
        slot->has_advance = 1;
    }

    if (((flags & NR_RASTERFONT_BBOX_FLAG) && !slot->has_bbox) ||
        ((flags & NR_RASTERFONT_GMAP_FLAG) && !slot->has_gmap)) {
        NRBPath gbp;
        slot->glyph.tg.bbox.x0 = 0;
        slot->glyph.tg.bbox.y0 = 0;
        slot->glyph.tg.bbox.x1 = 0;
        slot->glyph.tg.bbox.y1 = 0;
        slot->glyph.tg.px[0] = 0;
        slot->type = NRRF_TYPE_TINY;
        if (nr_font_glyph_outline_get (rf->font, glyph, &gbp, 0) && (gbp.path && (gbp.path->code == NR_MOVETO))) {
            NRRect bbox;
            int x0, y0, x1, y1, w, h;

            NR::Matrix const a(NR::transform(rf->transform));

            {
                NRBPath bp;
                /* fixme: */
                bbox.x0 = bbox.y0 = NR_HUGE;
                bbox.x1 = bbox.y1 = -NR_HUGE;
                bp.path = gbp.path;
                nr_path_matrix_bbox_union(&bp, a, &bbox);
            }
      
            if (!nr_rect_d_test_empty (&bbox)) {
                x0 = NRRF_COORD_FROM_FLOAT_LOWER (bbox.x0);
                y0 = NRRF_COORD_FROM_FLOAT_LOWER (bbox.y0);
                x1 = NRRF_COORD_FROM_FLOAT_UPPER (bbox.x1);
                y1 = NRRF_COORD_FROM_FLOAT_UPPER (bbox.y1);
                w = NRRF_COORD_INT_SIZE (x0, x1);
                h = NRRF_COORD_INT_SIZE (y0, y1);
                if ((w >= NRRF_MAX_GLYPH_DIMENSION) ||
                    (h >= NRRF_MAX_GLYPH_DIMENSION) ||
                    ((w * h) > NRRF_MAX_GLYPH_SIZE)) {
                    // we should not compute the svp, but use the ConvertToShape() instead
                    // using the svp ensures we keep the exact same behavior
                    // 
                    slot->glyph.lg.bbox.x0 = MAX (x0, -32768);
                    slot->glyph.lg.bbox.y0 = MAX (y0, -32768);
                    slot->glyph.lg.bbox.x1 = MIN (x1, 32767);
                    slot->glyph.lg.bbox.y1 = MIN (y1, 32767);
                    slot->type = NRRF_TYPE_LIV;
                    slot->glyph.lg.shp = new Shape;
                    slot->glyph.lg.delayed=NULL;
                    slot->glyph.lg.shbbox=bbox;
          
                    slot->glyph.lg.delayed=new Path;
                    slot->glyph.lg.delayed->LoadArtBPath(gbp.path, a, true);
 
/*          Path*  thePath=new Path;
	    Shape* theShape=new Shape;
            thePath->LoadArtBPath(gbp.path, a, true);
	    thePath->Convert(0.25);
	    thePath->Fill(theShape,0);
	    slot->glyph.lg.shp->ConvertToShape(theShape,fill_nonZero);
	    delete theShape;
	    delete thePath;*/
                } else {
                    NRPixBlock spb;
                    slot->glyph.ig.bbox.x0 = MAX (x0, -32768);
                    slot->glyph.ig.bbox.y0 = MAX (y0, -32768);
                    slot->glyph.ig.bbox.x1 = MIN (x1, 32767);
                    slot->glyph.ig.bbox.y1 = MIN (y1, 32767);
                    slot->glyph.ig.px = nr_new (unsigned char, w * h);
                    nr_pixblock_setup_extern (&spb, NR_PIXBLOCK_MODE_A8,
                                              NRRF_COORD_INT_LOWER (x0),
                                              NRRF_COORD_INT_LOWER (y0),
                                              NRRF_COORD_INT_UPPER (x1),
                                              NRRF_COORD_INT_UPPER (y1),
                                              slot->glyph.ig.px, w,
                                              TRUE, TRUE);

                    Path*  thePath=new Path;
                    Shape* theShape=new Shape;
                    thePath->LoadArtBPath(gbp.path, a, true);
                    thePath->Convert(0.25);
                    thePath->Fill(theShape,0);
          
                    Shape* temp=new Shape;
                    temp->ConvertToShape(theShape,fill_nonZero);
                    delete theShape;
                    delete thePath;
          
                    nrrf_pixblock_render_shape_mask_or (spb, temp);

                    delete temp;
          
                    nr_pixblock_release (&spb);
                    slot->type = NRRF_TYPE_IMAGE;
                }
            }
        }
        slot->has_bbox = TRUE;
        slot->has_gmap = TRUE;
    }

    return slot;
}

// duplicate of the one in nr-arena-shape.cpp

static void
shape_run_A8_OR (raster_info &dest,void */*data*/,int st,float vst,int en,float ven)
{
    //	printf("%i %f -> %i %f\n",st,vst,en,ven);
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

void nrrf_pixblock_render_shape_mask_or (NRPixBlock &m,Shape* theS)
{
    //  printf("bbox %i %i %i %i \n",m.area.x0,m.area.y0,m.area.x1,m.area.y1);
  
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
  
    int    curPt;
    float  curY;
    theS->BeginRaster(curY,curPt,1.0);
  
    FloatLigne* theI=new FloatLigne();
    IntLigne*   theIL=new IntLigne();
    //  AlphaLigne*   theI=new AlphaLigne(il,ir);
  
    theS->Scan(curY,curPt,(float)(it),1.0);
  
    char* mdata=(char*)m.data.px;
    if ( m.size == NR_PIXBLOCK_SIZE_TINY ) mdata=(char*)m.data.p;
    uint32_t* ligStart=((uint32_t*)(mdata+((il-m.area.x0)+m.rs*(it-m.area.y0))));
    for (int y=it;y<ib;y++) {
        theI->Reset();
        //    theIL->Reset();
        /*    if ( y == -1661 && il == 5424 ) {
              printf("o");
              }*/
        if ( y&0x00000003 ) {
            theS->Scan(curY,curPt,((float)(y+1)),theI,false,1.0);
        } else {
            theS->Scan(curY,curPt,((float)(y+1)),theI,true,1.0);
        }
        theI->Flatten();
        theIL->Copy(theI);
        /*    {
              bool   bug=false;
              for (int i=1;i<theI->nbRun;i++) {
              if ( theI->runs[i].st < theI->runs[i-1].en-0.1 ) bug=true;
              }
              if ( bug ) {
              //        theI->Affiche();
              }
              }
              if ( showRuns ) theIL->Affiche();*/
    
        raster_info  dest;
        dest.startPix=il;
        dest.endPix=ir;
        dest.sth=il;
        dest.stv=y;
        dest.buffer=ligStart;
        //    theI->Raster(dest,NULL,shape_run_A8_OR);
        theIL->Raster(dest,NULL,shape_run_A8_OR);
        ligStart=((uint32_t*)(((char*)ligStart)+m.rs));
    }
    theS->EndRaster();
    delete theI;
    delete theIL;
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
