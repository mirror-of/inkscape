/*
 *  FlowSrc.cpp
 */

#include "FlowRes.h"

#include "FlowStyle.h"
#include "FlowBoxes.h"
#include "FlowEater.h"
#include "FlowSrc.h"
	
#include "../sp-object.h"
#include "../style.h"
#include "../print.h"
#include "../svg/svg.h"

#include <math.h>

#include <pango/pango.h>

#include "livarot/Path.h"

#include "libnr/nr-rect.h"
#include "libnr/nr-point.h"
#include "libnr/nr-point-ops.h"
#include "libnr/nr-matrix.h"
#include "libnr/nr-matrix-ops.h"
#include "libnr/nr-scale.h"
#include "libnr/nr-scale-ops.h"
#include "libnr/nr-translate.h"
#include "libnr/nr-translate-ops.h"
#include "libnr/nr-rotate.h"
#include "libnr/nr-rotate-ops.h"

#include "libnrtype/FontFactory.h"
#include "libnrtype/font-instance.h"
#include "libnrtype/font-style-to-pos.h"

#include "display/curve.h"
#include "display/nr-arena-item.h"
#include "display/nr-arena-group.h"
#include "display/nr-arena-glyphs.h"

#include "extension/print.h"

/*
 *
 */
SPObject*          flow_res::ChunkSourceStart(int no)
{
	if ( no < 0 || no >= nbChunk ) return NULL;
	if ( chunks[no].mommy == NULL ) return NULL;
	if ( chunks[no].mommy->source_start ) return chunks[no].mommy->source_start->me;
	return NULL;
}
SPObject*          flow_res::ChunkSourceEnd(int no)
{
	if ( no < 0 || no >= nbChunk ) return NULL;
	if ( chunks[no].mommy == NULL ) return NULL;
	if ( chunks[no].mommy->source_end ) return chunks[no].mommy->source_end->me;
	return NULL;
}
int                flow_res::ChunkType(int no)
{
	if ( no < 0 || no >= nbChunk ) return flw_none;
	if ( chunks[no].mommy == NULL ) return flw_none;
	if ( chunks[no].mommy->source_start ) {
		return chunks[no].mommy->source_start->Type();
	}
	return flw_none;
}

void               flow_res::Verticalize(int no,double to_x,double to_y)
{
	if ( no < 0 || no >= nbChunk ) return;
	for (int i=chunks[no].l_st;i<chunks[no].l_en;i++) {
		double l_wid=letters[i].x_en-letters[i].x_st;
		double sav_y=letters[i].y;
		letters[i].y=to_y+letters[i].x_st;
		letters[i].x_st=to_x+sav_y;
		letters[i].x_en=to_x+sav_y+l_wid;
		double swap=letters[i].kern_x;letters[i].kern_x=letters[i].kern_y;letters[i].kern_y=swap;
	}
	chunks[no].y+=to_y;
	chunks[no].x_st+=to_x;
	chunks[no].x_en+=to_x;
}
void               flow_res::TranslateChunk(int no,double to_x,double to_y,bool the_start)
{
	//printf("translate %i to %f %f \n",no,to_x,to_y);
	if ( no < 0 || no >= nbChunk ) return;
	if ( the_start == false ) {
		to_x-=chunks[no].x_en;
	}
	for (int i=chunks[no].s_st;i<chunks[no].s_en;i++) {
		for (int j=spans[i].l_st;j<spans[i].l_en;j++) {
			letters[j].x_st+=to_x;
			letters[j].x_en+=to_x;
			letters[j].y+=to_y;
		}
	}
	chunks[no].x_st+=to_x;
	chunks[no].x_en+=to_x;
	chunks[no].y+=to_y;
}

void flow_res::ApplyPath(int no, Path *i_path)
{
    if ( no < 0 || no >= nbChunk ) {
        return;
    }
    
    if ( i_path == NULL ) {
        return;
    }

    for (int i = chunks[no].s_st; i < chunks[no].s_en; i++) {
        for (int j = spans[i].l_st; j < spans[i].l_en; j++) {
            // dummy
            int nb_glyph_p = 0;

            //startpoint-on-the-path is the point on the path ... which is startOffset distance
            //along the path
            
            //from the start of the path, calculated using the user agent's distance along the path algorithm.
            double startOffset = letters[j].x_st;
            Path::cut_position *startpoint_otp = i_path->CurvilignToPosition(1, &startOffset, nb_glyph_p);

            //Determine the glyph's charwidth (i.e., the amount which the current text position advances 
            //horizontally when the glyph is drawn using horizontal text layout). 
            double charwidth = letters[j].x_en - letters[j].x_st;

            //Determine the point on the curve which is charwidth distance along the path from the 
            //startpoint-on-the-path for this glyph, calculated using the user agent's distance along the path algorithm.
            double endOffset = letters[j].x_en;
            Path::cut_position *endpoint_otp = i_path->CurvilignToPosition(1, &endOffset, nb_glyph_p);

            //Determine the midpoint-on-the-path, which is the point on the path which is "halfway" (user agents can 
            //choose either a distance calculation or a parametric calculation) between the startpoint-on-the-path 
            //and the endpoint-on-the-path.
            double midOffset = letters[j].x_st + 0.5 * charwidth;
            Path::cut_position* midpoint_otp = i_path->CurvilignToPosition(1, &midOffset, nb_glyph_p);

            // only glyphs whose midpoint is on path are rendered
            if ( midpoint_otp && midpoint_otp[0].piece >= 0) {

                // find out coords and tangent at the midpoint_otp
                NR::Point midpoint;
                NR::Point mid_tangent;
                i_path->PointAndTangentAt(midpoint_otp[0].piece, midpoint_otp[0].t, midpoint, mid_tangent);

                NR::Point tangent;

                if (startpoint_otp && startpoint_otp[0].piece >= 0 && endpoint_otp && endpoint_otp[0].piece >= 0) {

                    // if both start and endpoints are also on the path,

                    // find out coords and tangent at startpoint and endpoint
                    NR::Point startpoint;
                    NR::Point start_tangent;
                    i_path->PointAndTangentAt (startpoint_otp[0].piece, startpoint_otp[0].t, startpoint, start_tangent);
                    NR::Point endpoint;
                    NR::Point end_tangent;
                    i_path->PointAndTangentAt (endpoint_otp[0].piece, endpoint_otp[0].t, endpoint, end_tangent);

                    // get the vector from start to end and normalize it so that its length is 1
                    tangent = (endpoint - startpoint);
                    tangent.normalize();
                    
                } else {
                    
                    // The spec is a bit bogus here; it says to render glyph if one of the start/end points is on path,
                    // but the algorithm for tangent (see sibling branch above) needs both of them.
                    // We work around this by just taking the path tangent at the midpoint.
                    tangent = mid_tangent;

                }

                // glyph origin: baseline of glyph must be on midpoint, so we step back half of charwidth from midpoint
                // in the direction of the tangent
                NR::Point origin = midpoint - 0.5 * charwidth * tangent;
                // from there, we apply the y displacement perpendicular to tangent (for vertical kerning)
                origin -= letters[j].y * tangent.ccw();
                letters[j].x_st = origin[NR::X];
                letters[j].y = origin[NR::Y];

                // glyph end: same as origin but to the other side of the midpoint
                NR::Point end = midpoint + 0.5 * charwidth * tangent;
                end -= letters[j].y * tangent.ccw();
                letters[j].x_en = end[NR::X];

                // rotation of the glyph
                double ang = 0;
                if ( tangent[NR::X] >= 1 ) {
                    ang = 0;
                } else if ( tangent[NR::X] <= -1 ) {
                    ang = M_PI;
                } else {
                    ang = acos(tangent[NR::X]);
                }
                if ( tangent[NR::Y] < 0 ) {
                    ang = 2*M_PI - ang;
                }
                
                letters[j].rotate += ang;
            } else {
                letters[j].invisible = true;
            }
            
            free(startpoint_otp);
            free(midpoint_otp);
            free(endpoint_otp);
            
            //letters[j].x_en=charwidth; // special case...
        }
        
        { // Now set the coords for the current tspan
            double glyph_a = 0;
            int nb_glyph_p = 0;
            Path::cut_position *glyph_p = i_path->CurvilignToPosition(1, &glyph_a, nb_glyph_p);
            if ( glyph_p ) {
                if ( glyph_p[0].piece >= 0 ) {
                    NR::Point g_pos;
                    NR::Point g_tgt;
                    NR::Point g_nor;
                    i_path->PointAndTangentAt(glyph_p[0].piece, glyph_p[0].t, g_pos, g_tgt);
                    spans[i].x_st = g_pos[0];
                    spans[i].y = g_pos[1];
                }
            }
        }
    }
}


void               flow_res::Show(NRArenaGroup* in_arena, NRRect *paintbox)
{
	if ( nbGroup <= 0 || nbGlyph <= 0 ) return;
	for (int i=0;i<nbGroup;i++) {
		groups[i].g_gr = NRArenaGlyphsGroup::create(in_arena->arena);
		nr_arena_item_add_child (in_arena, groups[i].g_gr, NULL);
		nr_arena_item_unref (groups[i].g_gr);
		
		nr_arena_glyphs_group_set_style (groups[i].g_gr, groups[i].style->with_style);
		font_instance* curF=groups[i].style->theFont;
		double size=groups[i].style->with_style->font_size.computed;
		NR::Matrix f_tr(NR::scale(size,-size));
		for (int j=groups[i].st;j<groups[i].en;j++) {
			if ( letters[glyphs[j].let].invisible == false ) {
				double  px=glyphs[j].g_x,py=glyphs[j].g_y;
				double  ang=letters[glyphs[j].let].rotate;
				px+=letters[glyphs[j].let].x_st;
				py+=letters[glyphs[j].let].y;
				//py+=letters[glyphs[j].let].kern_y;
				NRMatrix  g_mat;
				g_mat.c[0]=size*cos(ang);
				g_mat.c[1]=size*sin(ang);
				g_mat.c[2]=size*sin(ang);
				g_mat.c[3]=-size*cos(ang);
				g_mat.c[4]=px;
				g_mat.c[5]=py;
				nr_arena_glyphs_group_add_component (groups[i].g_gr, curF, glyphs[j].g_id, &g_mat);
			}
		}

		nr_arena_glyphs_group_set_paintbox (NR_ARENA_GLYPHS_GROUP (groups[i].g_gr), paintbox);
	}
	nr_arena_item_request_update (NR_ARENA_ITEM (in_arena), NR_ARENA_ITEM_STATE_ALL, FALSE);
}


void               flow_res::BBox(NRRect *bbox, NR::Matrix const &transform)
{
	if ( nbGroup <= 0 || nbGlyph <= 0 ) return;
	for (int i=0;i<nbGroup;i++) {
		double size=groups[i].style->with_style->font_size.computed;
		NR::Matrix  f_tr(NR::scale(size,-size));
		font_instance*  curF=groups[i].style->theFont;
		if ( curF ) {
			for (int j=groups[i].st;j<groups[i].en;j++) {
				if ( letters[glyphs[j].let].invisible == false ) {
					double  px=glyphs[j].g_x,py=glyphs[j].g_y;
					double  ang=letters[glyphs[j].let].rotate;
					px+=letters[glyphs[j].let].x_st;
					py+=letters[glyphs[j].let].y;
					//py+=letters[glyphs[j].let].kern_y;
					NR::Matrix  g_tr;
					g_tr[0]=size*cos(ang);
					g_tr[1]=size*sin(ang);
					g_tr[2]=size*sin(ang);
					g_tr[3]=-size*cos(ang);
					g_tr[4]=px;
					g_tr[5]=py;
					NR::Matrix   tot_tr=g_tr*transform;
					NR::Rect  res=curF->BBox(glyphs[j].g_id);
					NR::Point bmi=res.min(),bma=res.max();
					NR::Point tlp(bmi[0],bmi[1]),trp(bma[0],bmi[1]),blp(bmi[0],bma[1]),brp(bma[0],bma[1]);
					tlp=tlp*tot_tr;
					trp=trp*tot_tr;
					blp=blp*tot_tr;
					brp=brp*tot_tr;
					res=NR::Rect(tlp,trp);
					res.expandTo(blp);
					res.expandTo(brp);
					if ( (res.min())[0] < bbox->x0 ) bbox->x0=(res.min())[0];
					if ( (res.max())[0] > bbox->x1 ) bbox->x1=(res.max())[0];
					if ( (res.min())[1] < bbox->y0 ) bbox->y0=(res.min())[1];
					if ( (res.max())[1] > bbox->y1 ) bbox->y1=(res.max())[1];
				}
			}
		}
	}
}

void flow_res::Print(SPPrintContext *ctx,
		     NRRect const *pbox, NRRect const *dbox, NRRect const *bbox,
		     NRMatrix const &ctm)
{
	if ( nbGroup <= 0 || nbGlyph <= 0 || nbChar <= 0 ) return;
	bool text_to_path=ctx->module->textToPath();
	if ( text_to_path ) {
		for (int i=0;i<nbGroup;i++) {
			text_style*     curS=groups[i].style;
			font_instance*  curF=curS->theFont;
			const char*     curFam=pango_font_description_get_family(curF->descr);
			char*           savFam=curS->with_style->text->font_family.value;
			curS->with_style->text->font_family.value=(gchar*)curFam;
			double          size=groups[i].style->with_style->font_size.computed;
			for (int j=groups[i].st;j<groups[i].en;j++) {
				if ( letters[glyphs[j].let].invisible == false ) {
					double  px=glyphs[j].g_x,py=glyphs[j].g_y;
					double  ang=letters[glyphs[j].let].rotate;
					px+=letters[glyphs[j].let].x_st;
					py+=letters[glyphs[j].let].y;
					//py+=letters[glyphs[j].let].kern_y;
					NR::Matrix  g_tr;
					g_tr[0]=size*cos(ang);
					g_tr[1]=size*sin(ang);
					g_tr[2]=size*sin(ang);
					g_tr[3]=-size*cos(ang);
					g_tr[4]=px;
					g_tr[5]=py;
					NRBPath     bpath;
					if ( curF ) bpath.path=(NArtBpath*)curF->ArtBPath(glyphs[j].g_id); else bpath.path=NULL;
					if ( bpath.path ) {
						NRBPath abp;
						abp.path = nr_artpath_affine (bpath.path, g_tr);
						if (curS->with_style->fill.type != SP_PAINT_TYPE_NONE) {
							sp_print_fill (ctx, &abp, &ctm, curS->with_style, pbox, dbox, bbox);
						}
						
						if (curS->with_style->stroke.type != SP_PAINT_TYPE_NONE) {
							sp_print_stroke (ctx, &abp, &ctm, curS->with_style, pbox, dbox, bbox);
						}
						nr_free (abp.path);
					}
				}
			}
			curS->with_style->text->font_family.value=savFam;
		}
	} else {
		for (int no=0;no<nbChunk;no++) {
			for (int i=chunks[no].s_st;i<chunks[no].s_en;i++) {
				text_style*     curS=spans[i].c_style;
				font_instance*  curF=curS->theFont;
				const char*     curFam=pango_font_description_get_family(curF->descr);
				char*           savFam=curS->with_style->text->font_family.value;
				curS->with_style->text->font_family.value=(gchar*)curFam;
				for (int j=spans[i].l_st;j<spans[i].l_en;j++) {
					NR::Point   g_pos(0,0);
					double  px=letters[j].x_st,py=letters[j].y;
					double  ang=letters[j].rotate;
					NR::Matrix  g_tr;
					g_tr[0]=cos(ang);
					g_tr[1]=sin(ang);
					g_tr[2]=sin(ang);
					g_tr[3]=-cos(ang);
					g_tr[4]=px;
					g_tr[5]=py;
					char*       g_txt=chars+letters[j].t_st;
					int         g_len=letters[j].t_en-letters[j].t_st;
					char savC=g_txt[g_len];
					g_txt[g_len]=0;
					sp_print_bind(ctx, g_tr,1.0);
					sp_print_text (ctx, g_txt, g_pos, curS->with_style);
					sp_print_release(ctx);
					g_txt[g_len]=savC;
				}
				curS->with_style->text->font_family.value=savFam;
			}
		}
	}
}
SPCurve*              flow_res::NormalizedBPath(void)
{
	if ( nbGroup <= 0 || nbGlyph <= 0 ) return sp_curve_new();
	
	GSList *cc = NULL;

	for (int i=0;i<nbGroup;i++) {
		text_style*     curS=groups[i].style;
		font_instance*  curF=curS->theFont;
		double          size=curS->theSize;
		for (int j=groups[i].st;j<groups[i].en;j++) {
			if ( letters[glyphs[j].let].invisible == false ) {
				double  px=glyphs[j].g_x,py=glyphs[j].g_y;
				double  ang=letters[glyphs[j].let].rotate;
				px+=letters[glyphs[j].let].x_st;
				py+=letters[glyphs[j].let].y;
				//py+=letters[glyphs[j].let].kern_y;
				NR::Matrix  g_tr;
				g_tr[0]=size*cos(ang);
				g_tr[1]=size*sin(ang);
				g_tr[2]=size*sin(ang);
				g_tr[3]=-size*cos(ang);
				g_tr[4]=px;
				g_tr[5]=py;
				NRBPath     bpath;
				if ( curF ) bpath.path=(NArtBpath*)curF->ArtBPath(glyphs[j].g_id); else bpath.path=NULL;
				if ( bpath.path ) {
					NArtBpath *abp = nr_artpath_affine (bpath.path,g_tr);
					SPCurve *c = sp_curve_new_from_bpath (abp);
					if (c) cc = g_slist_prepend (cc, c);
				}
			}
		}
	}
		
	cc = g_slist_reverse (cc);
	
	SPCurve *curve;
	if ( cc ) {
		curve = sp_curve_concat (cc);
	} else {
		curve = sp_curve_new();
	}
  
	while (cc) {
		/* fixme: This is dangerous, as we are mixing art_alloc and g_new */
		sp_curve_unref ((SPCurve *) cc->data);
		cc = g_slist_remove (cc, cc->data);
	}
	
	return curve;
	
}

// for each letter, the text it comes from starts at letters[].utf8_offset in the text. its length is unknown, but should be
// the length of the utf8 text sufficient to describe it, that is letters[].t_en-letters[].t_st
// note that the text in the 'chars' array may be reorganized wrt the source text in case of bidirectionality, and that some 
// chars of the source may be missing from the text_holder, hence from chars, like soft hyphen.
void               flow_res::OffsetToLetter(int offset,int &c,int &s,int &l,bool &l_start,bool &l_end)
{
	c=s=l=-1;
	l_start=l_end=false;
	for (int i=0;i<nbLetter;i++) {
		// check for each letter if the requested offset if inside the letter
		int   l_st=letters[i].utf8_offset;
		int   l_en=l_st+letters[i].t_en-letters[i].t_st;
		if ( offset >= l_st && offset < l_en ) {
			l=i;
			l_end=false;
			l_start=( offset == letters[i].utf8_offset );
			break;
		}
		// test if the position is at end of the letter
		// fo not return, because starting a letter is more important than ending one.
		if ( offset == l_en ) {
			l_end=true;
			l_start=false;
			l=i;
		}
	}
	// once we have the letter no, fill the rest of the parameters
	if ( l >= 0 ) {
		for (int i=0;i<nbChunk;i++) {
			if ( l >= chunks[i].l_st && l < chunks[i].l_en ) {
				c=i;
				break;
			}
		}
		for (int i=0;i<nbSpan;i++) {
			if ( l >= spans[i].l_st && l < spans[i].l_en ) {
				s=i;
				break;
			}
		}
	} else {
		// maybe an empty line
	}
}
void               flow_res::LetterToOffset(int /*c*/,int /*s*/,int l,bool /*l_start*/,bool l_end,int &offset)
{
	offset=0;
	if ( l < 0 ) {
		return;
	}
	if ( l >= nbLetter ) {
		if ( nbLetter > 0 ) {
			offset=letters[nbLetter-1].utf8_offset+letters[nbLetter-1].t_en-letters[nbLetter-1].t_st;
		} else {
			offset=0;
		}
		return;
	}
	if ( l_end ) {
		offset = letters[l].utf8_offset + letters[l].t_en - letters[l].t_st;
	} else {
		offset = letters[l].utf8_offset;
	}
}
void               flow_res::PositionToLetter(double px,double py,int &c,int &s,int &l,bool &l_start,bool &l_end)
{
	c=s=l=-1;
	l_start=l_end=false;
	int      best_l=-1;
	double   best_dist=0;
	bool     best_start=false;
	bool     best_end=false;
	for (int i=0;i<nbChunk;i++) {
		for (int j=chunks[i].s_st;j<chunks[i].s_en;j++) {
			text_style*     curS=spans[j].c_style;
			double          size=(curS)?curS->theSize:chunks[i].ascent+chunks[i].descent;
			for (int k=spans[j].l_st;k<spans[j].l_en;k++) {
				if ( letters[k].t_st >= letters[k].t_en ) {
					// empty letter, like newline
				}
				double    ang=letters[k].rotate;
				double    ll=letters[k].x_en-letters[k].x_st;
				NR::Point lp(0.5*(letters[k].x_st+letters[k].x_en),letters[k].y);
				if ( ChunkType(i) == txt_textpath ) {
					ll=letters[k].x_en;
					lp=NR::Point(letters[k].x_st,letters[k].y);
				}
				double    dist=NR::L2(NR::Point(px,py)-lp);
				if ( best_l < 0 || dist < best_dist ) {
					best_l=k;
					best_dist=dist;
					NR::Matrix  g_tr;
					g_tr[0]=size*cos(ang);
					g_tr[1]=size*sin(ang);
					g_tr[2]=size*sin(ang);
					g_tr[3]=-size*cos(ang);
					g_tr[4]=lp[0];
					g_tr[5]=lp[1];
					g_tr=g_tr.inverse();
					NR::Point  np=NR::Point(px,py)*g_tr;
					best_start=best_end=false;
					if ( ll >= 0 ) {
						if ( np[0] < -0.5*ll ) best_start=true;
						if ( np[0] > 0.5*ll ) best_end=true;
					} else {
						if ( np[0] > -0.5*ll ) best_start=true;
						if ( np[0] < 0.5*ll ) best_end=true;
					}
				}
			}
		}
	}
	l=best_l;
	if ( l >= 0 ) {
		l_start=best_start;
		l_end=best_end;
		for (int i=0;i<nbChunk;i++) {
			if ( l >= chunks[i].l_st && l < chunks[i].l_en ) {
				c=i;
				break;
			}
		}
		for (int i=0;i<nbSpan;i++) {
			if ( l >= spans[i].l_st && l < spans[i].l_en ) {
				s=i;
				break;
			}
		}
	}
}
void               flow_res::LetterToPosition(int c,int s,int l,bool /*l_start*/,bool l_end,double &px,double &py,double &size,double &angle)
{
	px=py=0;
	size=12.0;
	angle=0;
	if ( l < 0 ) {
		if ( nbLetter > 0 ) {
			LetterToPosition(c,s,0,true,false,px,py,size,angle);
		} else {
		}
		return;
	}
	if ( l >= nbLetter ) {
		if ( nbLetter > 0 ) {
			LetterToPosition(c,s,nbLetter-1,false,true,px,py,size,angle);
		} else {
		}
		return;
	}
	angle=letters[l].rotate;
	py=letters[l].y;
	if ( s < 0 || s >= nbSpan ) s=chunks[c].s_st;
	if ( s >= 0 && s < nbSpan ) {
		text_style*     curS=spans[s].c_style;
		size=(curS)?curS->theSize:chunks[c].ascent; // just the ascent, it's nicer
	}
	if ( ChunkType(c) == txt_textpath ) {
		if ( l_end ) {
			px=letters[l].x_st+letters[l].x_en*cos(angle);
			py=letters[l].y+letters[l].x_en*sin(angle);
		} else {
			px=letters[l].x_st;
		}
	} else {
		if ( l_end ) {
			px=letters[l].x_en;
		} else {
			px=letters[l].x_st;
		}
	}
}



/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :

