/*
 *  FontInstance.cpp
 *  testICU
 *
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif
#include "FontFactory.h"
#include <libnr/nr-rect.h>
#include <libnrtype/font-glyph.h>
#include <libnrtype/font-instance.h>

/* #include <layout/LEGlyphStorage.h> */

#include <livarot/Path.h>
#include <livarot/Shape.h>

#include "RasterFont.h"
#ifdef WITH_XFT
# include <freetype/ftoutln.h>
# include <freetype/ftbbox.h>
# include <freetype/internal/tttypes.h>
# include <freetype/internal/ftstream.h>
# include <freetype/tttags.h>
# include <pango/pangoft2.h>
#endif

#ifdef WIN32
# include <pango/pangowin32.h>
# include <windows.h>
#endif

size_t  font_style_hash::operator()(const font_style &x) const {
	int      h=0,n;
	for (int i=0;i<6;i++) {
		n=(int)floor(100*x.transform[i]);
		h*=12186;
		h+=n;			
	}
	n=(int)floor(100*x.stroke_width);
	h*=12186;
	h+=n;
	n=(x.vertical)?1:0;
	h*=12186;
	h+=n;
	if ( x.stroke_width >= 0.01 ) {
		n=x.stroke_cap*10+x.stroke_join;
		h*=12186;
		h+=n;
		if ( x.nbDash > 0 ) {
			n=x.nbDash;
			h*=12186;
			h+=n;
			n=(int)floor(100*x.dash_offset);
			h*=12186;
			h+=n;
			for (int i=0;i<x.nbDash;i++) {
				n=(int)floor(100*x.dashes[i]);
				h*=12186;
				h+=n;
			}
		}
	}
	return h;
}

bool  font_style_equal::operator()(const font_style &a,const font_style &b) {
	NR::Matrix  diff=a.transform.inverse();
	diff*=b.transform;
	if ( diff.is_translation(0.01) ) {
		if ( fabs(diff[4]) < 0.01 && fabs(diff[5]) < 0.01 ) {
		} else {
			return false;
		}
	} else {
		return false;
	}
	if ( a.vertical && b.vertical == false ) return false;
	if ( a.vertical == false && b.vertical ) return false;
	if ( a.stroke_width > 0.01 && b.stroke_width <= 0.01 ) return false;
	if ( a.stroke_width <= 0.01 && b.stroke_width > 0.01 ) return false;
	if ( a.stroke_width <= 0.01 && b.stroke_width <= 0.01 ) return true;
	
	if ( a.stroke_cap != b.stroke_cap ) return false;
	if ( a.stroke_join != b.stroke_join ) return false;
	if ( a.nbDash != b.nbDash ) return false;
	if ( a.nbDash <= 0 ) return true;
	if ( fabs(a.dash_offset-b.dash_offset) < 0.01 ) {
		for (int i=0;i<a.nbDash;i++) {
			if ( fabs(a.dashes[i]-b.dashes[i]) >= 0.01 ) return false;
		}
	} else {
		return false;
	}
	return true;
}

/*
 * Outline extraction
 */

#ifdef WITH_XFT
typedef struct ft2_to_liv {
	Path*        theP;
	double       scale;
	NR::Point    last;
} ft2_to_liv;

// outline as returned by freetype -> livarot Path
// see nr-type-ft2.cpp for the freetype -> artBPath on which this code is based
static int ft2_move_to (FT_Vector * to, void * i_user) {
	ft2_to_liv* user=(ft2_to_liv*)i_user;
	NR::Point   p(user->scale*to->x,user->scale*to->y);
	//	printf("m  t=%f %f\n",p[0],p[1]);
	user->theP->MoveTo(p);
	user->last=p;
	return 0;
}

static int ft2_line_to (FT_Vector * to, void * i_user)
{
	ft2_to_liv* user=(ft2_to_liv*)i_user;
	NR::Point   p(user->scale*to->x,user->scale*to->y);
	//	printf("l  t=%f %f\n",p[0],p[1]);
	user->theP->LineTo(p);
	user->last=p;
	return 0;
}

static int ft2_conic_to (FT_Vector * control, FT_Vector * to, void * i_user)
{
	ft2_to_liv* user=(ft2_to_liv*)i_user;
	NR::Point   p(user->scale*to->x,user->scale*to->y),c(user->scale*control->x,user->scale*control->y);  
	//	printf("b c=%f %f  t=%f %f\n",c[0],c[1],p[0],p[1]);
	user->theP->BezierTo(p);
	user->theP->IntermBezierTo(c);
	user->theP->EndBezierTo();
	user->last=p;
	return 0;
}

static int ft2_cubic_to (FT_Vector * control1, FT_Vector * control2, FT_Vector * to, void * i_user)
{
	ft2_to_liv* user=(ft2_to_liv*)i_user;
	NR::Point   p(user->scale*to->x,user->scale*to->y),
	c1(user->scale*control1->x,user->scale*control1->y),
	c2(user->scale*control2->x,user->scale*control2->y);  
	//	printf("c c1=%f %f  c2=%f %f   t=%f %f\n",c1[0],c1[1],c2[0],c2[1],p[0],p[1]);
	user->theP->CubicTo(p,3*(c1-user->last),3*(p-c2)); 
	user->last=p;
	return 0;
}
#endif

/*
 *
 */

font_instance::font_instance(void)
{
	// printf("font instance born\n");
	descr=NULL;
	pFont=NULL;
	refCount=0;
	daddy=NULL;
	nbGlyph=maxGlyph=0;
	glyphs=NULL;
#ifdef WITH_XFT
	theFace=NULL;
#endif
#ifdef WIN32
	theLogFont=NULL;
	wFont=NULL;
#endif
}

font_instance::~font_instance(void)
{
	if ( daddy ) daddy->UnrefFace(this);
	// printf("font instance death\n");
	if ( pFont ) g_object_unref(pFont);
	pFont=NULL;
	if ( descr ) pango_font_description_free(descr);
	descr=NULL;
#ifdef WITH_XFT
	//	if ( theFace ) FT_Done_Face(theFace); // owned by pFont. don't touch
	theFace=NULL;
#endif
#ifdef WIN32
	if ( wFont ) {
		font_factory* f_src=daddy;
		if ( f_src == NULL ) f_src=font_factory::Default();
		if ( f_src ) pango_win32_font_cache_unload(f_src->wCache,wFont);
	}
#endif
	for (int i=0;i<nbGlyph;i++) {
		if ( glyphs[i].outline ) delete glyphs[i].outline;
		if ( glyphs[i].artbpath ) free(glyphs[i].artbpath);
	}
	if ( glyphs ) free(glyphs);
	nbGlyph=maxGlyph=0;
	glyphs=NULL;
}

void font_instance::Ref(void)
{
	refCount++;
	// printf("font %x ref'd %i\n",this,refCount);
	//	if ( refCount > 1000 || refCount < 0 ) {
	//		printf("o");
	//	}
}

void font_instance::Unref(void)
{
	refCount--;
	// printf("font %x unref'd %i\n",this,refCount);
	//	if ( refCount > 1000 || refCount < 0 ) {
	//		printf("o");
	//	}
	if ( refCount <= 0 ) {
		if ( daddy ) daddy->UnrefFace(this);
		daddy=NULL;
		delete this;
	}
}

unsigned int font_instance::Name(gchar *str, unsigned int size)
{
	return Attribute("name", str, size);
}

unsigned int font_instance::Family(gchar *str, unsigned int size)
{
	return Attribute("family", str, size);
}

unsigned int font_instance::Attribute(const gchar *key, gchar *str, unsigned int size)
{
	if ( descr == NULL ) {
		if ( size > 0 ) str[0]=0;
		return 0;
	}
	char*   res=NULL;
	bool    free_res=false;
	
	if ( strcmp(key,"name") == 0 ) {
		PangoFontDescription* td=pango_font_description_copy(descr);
		pango_font_description_unset_fields (td, PANGO_FONT_MASK_SIZE);
		res=pango_font_description_to_string (td);
		pango_font_description_free(td);
		free_res=true;
	} else if ( strcmp(key,"family") == 0 ) {
		res=(char*)pango_font_description_get_family(descr);
		free_res=false;
	} else if ( strcmp(key,"style") == 0 ) {
		PangoStyle v=pango_font_description_get_style(descr);
		if ( v == PANGO_STYLE_ITALIC ) {
			res="italic";
		} else if ( v == PANGO_STYLE_OBLIQUE ) {
			res="oblique";
		} else {
			res="normal";
		}
		free_res=false;
	} else if ( strcmp(key,"weight") == 0 ) {
		PangoWeight v=pango_font_description_get_weight(descr);
		if ( v <= PANGO_WEIGHT_ULTRALIGHT ) {
			res="200";
		} else if ( v <= PANGO_WEIGHT_LIGHT ) {
			res="300";
		} else if ( v <= PANGO_WEIGHT_NORMAL ) {
			res="normal";
		} else if ( v <= PANGO_WEIGHT_BOLD ) {
			res="bold";
		} else {
			res="800";
		}
		free_res=false;
	} else if ( strcmp(key,"stretch") == 0 ) {
		PangoStretch v=pango_font_description_get_stretch(descr);
		if ( v <= PANGO_STRETCH_EXTRA_CONDENSED ) {
			res="extra-condensed";
		} else if ( v <= PANGO_STRETCH_CONDENSED ) {
			res="condensed";
		} else if ( v <= PANGO_STRETCH_SEMI_CONDENSED ) {
			res="semi-condensed";
		} else if ( v <= PANGO_STRETCH_NORMAL ) {
			res="normal";
		} else if ( v <= PANGO_STRETCH_SEMI_EXPANDED ) {
			res="semi-expanded";
		} else if ( v <= PANGO_STRETCH_EXPANDED ) {
			res="expanded";
		} else {
			res="extra-expanded";
		}
		free_res=false;
	} else if ( strcmp(key,"variant") == 0 ) {
		PangoVariant v=pango_font_description_get_variant(descr);
		if ( v == PANGO_VARIANT_SMALL_CAPS ) {
			res="small-caps";
		} else {
			res="normal";
		}
		free_res=false;
	} else {
		res = NULL;
		free_res=false;
	}
	if ( res == NULL ) {
		if ( size > 0 ) str[0]=0;
		return 0;
	}
	
	if (res) {
		unsigned int len=strlen(res);
		unsigned int rlen=(size-1<len)?size-1:len;
		if ( str ) {
			if ( rlen > 0 ) memcpy(str,res,rlen);
			if ( size > 0 ) str[rlen]=0;
		}
		if (free_res) free(res);
		return len;
	}
	return 0;
}

void font_instance::InstallFace(PangoFont* iFace)
{
	pFont=iFace;
#ifdef WITH_XFT
	theFace=pango_ft2_font_get_face(pFont);
	FT_Error ftresult=FT_Select_Charmap(theFace,ft_encoding_unicode);
	if ( ftresult ) {
		printf("failed to load unicode charmap\n");
		ftresult=FT_Select_Charmap(theFace,ft_encoding_symbol);
		if ( ftresult ) {
			printf("and failed to load symbol charmap\n");
			if ( pFont ) g_object_unref(pFont);
			pFont=NULL;
			theFace=NULL;
		}
	}
#endif
#ifdef WIN32
	HDC  wDev=NULL;
	if ( daddy ) {
		wDev=daddy->wDevice;
		if ( theLogFont == NULL ) {
			theLogFont=pango_win32_font_logfont(pFont);
			wFont=pango_win32_font_cache_load(daddy->wCache,theLogFont);
			if ( wFont ) {
				SelectFont (wDev,wFont);
				GetOutlineTextMetrics(wDev,sizeof(OUTLINETEXTMETRIC),&otm);
			} else {
				theLogFont=NULL;
				if ( pFont ) g_object_unref(pFont);
				pFont=NULL;
			}
		}
	}
#endif
	if ( pFont && IsOutlineFont() == false ) {
#ifdef WITH_XFT
		theFace=NULL;
#endif
#ifdef WIN32
		if ( wFont && daddy ) {
			pango_win32_font_cache_unload(daddy->wCache,wFont);
			wFont=NULL;
		}
#endif
		if ( pFont ) g_object_unref(pFont);
		pFont=NULL;
	}
}

bool	font_instance::IsOutlineFont(void)
{
	if ( pFont == NULL ) return false;
#ifdef WITH_XFT
	theFace=pango_ft2_font_get_face(pFont);
	return FT_IS_SCALABLE(theFace);
#endif
#ifdef WIN32
	HDC  wDev=NULL;
	if ( daddy ) {
		wDev=daddy->wDevice;
		if ( theLogFont == NULL ) {
			theLogFont=pango_win32_font_logfont(pFont);
			wFont=pango_win32_font_cache_load(daddy->wCache,theLogFont);
		}
	}
	if ( wFont ) {
		TEXTMETRIC  test;
		SelectFont (wDev,wFont);
		if ( GetTextMetrics(wDev,&test) ) {
			if ( test.tmPitchAndFamily&TMPF_VECTOR || test.tmPitchAndFamily&TMPF_TRUETYPE ) return true;
		}
	}
#endif
	return false;
}

int font_instance::MapUnicodeChar(gunichar c)
{
	if ( pFont == NULL ) return 0;
	int res=0;
#ifdef WITH_XFT
	theFace=pango_ft2_font_get_face(pFont);
	if ( c > 0xf0000 ) {
		res=CLAMP(c,0xf0000,0x1fffff)-0xf0000;
	} else {
		res=FT_Get_Char_Index(theFace, c);
	}
#endif
#ifdef WIN32
	if ( c > 0xf0000 ) {
		res=CLAMP(c,0xf0000,0x1fffff)-0xf0000;
	} else {
		res=pango_win32_font_get_glyph_index(pFont,c);
	}
#endif
	return res;
}

#ifdef WIN32
#define FIXED_TO_FLOAT(p) ((p)->value + (double) (p)->fract / 65536.0)
#endif

void font_instance::LoadGlyph(int glyph_id)
{
	if ( pFont == NULL ) return;
#ifdef WITH_XFT
	theFace=pango_ft2_font_get_face(pFont);
	if ( theFace->units_per_EM == 0 ) return; // bitmap font
#endif
#ifdef WIN32
	HDC  wDev=NULL;
	if ( daddy ) {
		wDev=daddy->wDevice;
		if ( theLogFont == NULL ) {
			theLogFont=pango_win32_font_logfont(pFont);
			wFont=pango_win32_font_cache_load(daddy->wCache,theLogFont);
		}
	}
#endif
	if ( id_to_no.find(glyph_id) == id_to_no.end() ) {
		if ( nbGlyph >= maxGlyph ) {
			maxGlyph=2*nbGlyph+1;
			glyphs=(font_glyph*)realloc(glyphs,maxGlyph*sizeof(font_glyph));
		}
		font_glyph  n_g;
		n_g.outline=NULL;
		n_g.artbpath=NULL;
		n_g.bbox[0]=n_g.bbox[1]=n_g.bbox[2]=n_g.bbox[3]=0;
		bool   doAdd=false;
#ifdef WITH_XFT
		if (FT_Load_Glyph (theFace, glyph_id, FT_LOAD_NO_SCALE | FT_LOAD_NO_HINTING | FT_LOAD_NO_BITMAP)) {
			// shit happened
		} else {
			if ( FT_HAS_HORIZONTAL(theFace) ) {
				n_g.h_advance=((double)theFace->glyph->metrics.horiAdvance)/((double)theFace->units_per_EM);
				n_g.h_width=((double)theFace->glyph->metrics.width)/((double)theFace->units_per_EM);
			} else {
				n_g.h_width=n_g.h_advance=((double)(theFace->bbox.xMax-theFace->bbox.xMin))/((double)theFace->units_per_EM);
			}
			if ( FT_HAS_VERTICAL(theFace) ) {
				n_g.v_advance=((double)theFace->glyph->metrics.vertAdvance)/((double)theFace->units_per_EM);
				n_g.v_width=((double)theFace->glyph->metrics.height)/((double)theFace->units_per_EM);
			} else {
				n_g.v_width=n_g.v_advance=((double)theFace->height)/((double)theFace->units_per_EM);
			}
			if ( theFace->glyph->format == ft_glyph_format_outline ) {
				FT_Outline_Funcs ft2_outline_funcs = {
					ft2_move_to,
					ft2_line_to,
					ft2_conic_to,
					ft2_cubic_to,
					0, 0
				};
				n_g.outline=new Path;
				ft2_to_liv   tData;
				tData.theP=n_g.outline;
				tData.scale=1.0/((double)theFace->units_per_EM);
				tData.last=NR::Point(0,0);
				FT_Outline_Decompose (&theFace->glyph->outline, &ft2_outline_funcs, &tData);
			}
			doAdd=true;
		}
#endif
#ifdef WIN32
		if ( wFont ) {
			MAT2          mat = {{0, 1}, {0, 0}, {0, 0}, {0, 1}};
			GLYPHMETRICS  gmetrics;
			double        g_scale=1.0;
			
			SelectFont (wDev,wFont);
			
			g_scale=1.0/512;
			
			int   outl_size=GetGlyphOutline(wDev,glyph_id,GGO_NATIVE | GGO_GLYPH_INDEX,&gmetrics,0,NULL,&mat);
			char* buffer=(char*)malloc(outl_size*sizeof(char));
			GetGlyphOutline(wDev,glyph_id,GGO_NATIVE | GGO_GLYPH_INDEX,&gmetrics,outl_size,buffer,&mat);
			
			n_g.outline=new Path;
			for (int start=0;start<outl_size;) {	
				// one LPTTPOLYGONHEADER with several LPTTPOLYCURVE in it
				LPTTPOLYGONHEADER     pgh=(LPTTPOLYGONHEADER) (buffer+start);
				int         end=start+pgh->cb;
				NR::Point   lastP(FIXED_TO_FLOAT(&pgh->pfxStart.x),FIXED_TO_FLOAT(&pgh->pfxStart.y));
				lastP*=g_scale;
				n_g.outline->MoveTo(lastP);
				
				start+=sizeof(TTPOLYGONHEADER);
				for (;start<end;) {
					LPTTPOLYCURVE    pc=(LPTTPOLYCURVE) (buffer+start);
					if ( pc->wType == TT_PRIM_LINE ) { // simple polyline
						for (int i=0;i<pc->cpfx;i++) {
							lastP=NR::Point(FIXED_TO_FLOAT (&pc->apfx[i].x),FIXED_TO_FLOAT (&pc->apfx[i].y));
							lastP*=g_scale;
							n_g.outline->LineTo(lastP);
						}
					} else if ( pc->wType == TT_PRIM_QSPLINE ) { // quadratic bezier segments
						for (int i=0;i<pc->cpfx-1;i++) {
							NR::Point  p(FIXED_TO_FLOAT (&pc->apfx[i].x),FIXED_TO_FLOAT (&pc->apfx[i].y));
							NR::Point  np(FIXED_TO_FLOAT (&pc->apfx[i+1].x),FIXED_TO_FLOAT (&pc->apfx[i+1].y));
							np*=g_scale;
							p*=g_scale;
							if ( i+1 < pc->cpfx-1 ) {
								np=0.5*(p+np);
							} else {
							}
							n_g.outline->BezierTo(np);
							n_g.outline->IntermBezierTo(p);
							n_g.outline->EndBezierTo();
							lastP=np;
						}
						//					} else if ( pc->wType == TT_PRIM_CSPLINE ) { // cubic bezier segments
						// needs to be done (tho fonts are usually quadratic bezier
                    }
					start+=sizeof(TTPOLYCURVE)+(pc->cpfx-1)*sizeof(POINTFX);
                }	
				n_g.outline->Close();
            }
			{
				static MAT2 mat = {{0, 1}, {0, 0}, {0, 0}, {0, 1}};
				GetGlyphOutline(wDev,glyph_id,GGO_NATIVE | GGO_METRICS,&gmetrics,0,NULL,&mat);
				n_g.h_advance=g_scale*((double)gmetrics.gmCellIncX);
				n_g.h_width=g_scale*((double)gmetrics.gmBlackBoxX);
				n_g.v_advance=g_scale*((double)gmetrics.gmCellIncY);
				n_g.v_width=g_scale*((double)gmetrics.gmBlackBoxY);
			}
			
			free(buffer);
			doAdd=true;
        }
#endif
		if ( doAdd ) {
			if ( n_g.outline ) {
				n_g.outline->FastBBox(n_g.bbox[0],n_g.bbox[1],n_g.bbox[2],n_g.bbox[3]);
				n_g.artbpath=n_g.outline->MakeArtBPath();
			}
			glyphs[nbGlyph]=n_g;
			id_to_no[glyph_id]=nbGlyph;
			nbGlyph++;
		}
    } else {
    }
}

bool font_instance::FontMetrics(double &ascent,double &descent,double &leading)
{
#ifdef WITH_XFT
	theFace=pango_ft2_font_get_face(pFont);
	if ( theFace->units_per_EM == 0 ) return false; // bitmap font
#endif
#ifdef WIN32
	HDC  wDev=NULL;
	if ( daddy ) {
		wDev=daddy->wDevice;
		if ( theLogFont == NULL ) {
			theLogFont=pango_win32_font_logfont(pFont);
			wFont=pango_win32_font_cache_load(daddy->wCache,theLogFont);
		} else {
			return false;
		}
	} else {
		return false;
	}
#endif
	
	if ( pFont == NULL ) return false;
	
#ifdef WITH_XFT
	if ( theFace == NULL ) return false;
	ascent=fabs(((double)theFace->ascender)/((double)theFace->units_per_EM));
	descent=fabs(((double)theFace->descender)/((double)theFace->units_per_EM));
	leading=fabs(((double)theFace->height)/((double)theFace->units_per_EM));
	leading-=ascent+descent;
#endif
#ifdef WIN32
	if ( wFont == NULL ) return false;
	ascent=fabs(((double)otm.otmAscent)/((double)otm.otmEMSquare));
	descent=fabs(((double)otm.otmDescent)/((double)otm.otmEMSquare));
	leading=fabs(((double)otm.otmLineGap)/((double)otm.otmEMSquare));
#endif
	return true;
}

NR::Rect font_instance::BBox(int glyph_id)
{
	int no=-1;
	if ( id_to_no.find(glyph_id) == id_to_no.end() ) {
		LoadGlyph(glyph_id);
		if ( id_to_no.find(glyph_id) == id_to_no.end() ) {
			// didn't load
		} else {
			no=id_to_no[glyph_id];
		}
	} else {
		no=id_to_no[glyph_id];
	}
	if ( no < 0 ) return NR::Rect(NR::Point(0,0),NR::Point(0,0));
	NR::Point rmin(glyphs[no].bbox[0],glyphs[no].bbox[1]);
	NR::Point rmax(glyphs[no].bbox[2],glyphs[no].bbox[3]);
	NR::Rect  res(rmin,rmax);
	return res;
}

Path* font_instance::Outline(int glyph_id,Path* copyInto)
{
	int no=-1;
	if ( id_to_no.find(glyph_id) == id_to_no.end() ) {
		LoadGlyph(glyph_id);
		if ( id_to_no.find(glyph_id) == id_to_no.end() ) {
			// didn't load
		} else {
			no=id_to_no[glyph_id];
		}
	} else {
		no=id_to_no[glyph_id];
	}
	if ( no < 0 ) return NULL;
	Path*    src_o=glyphs[no].outline;
	if ( copyInto ) {
		copyInto->Reset();
		copyInto->Copy(src_o);
		return copyInto;
	}
	return src_o;
}

void* font_instance::ArtBPath(int glyph_id)
{
	int no=-1;
	if ( id_to_no.find(glyph_id) == id_to_no.end() ) {
		LoadGlyph(glyph_id);
		if ( id_to_no.find(glyph_id) == id_to_no.end() ) {
			// didn't load
		} else {
			no=id_to_no[glyph_id];
		}
	} else {
		no=id_to_no[glyph_id];
	}
	if ( no < 0 ) return NULL;
	return glyphs[no].artbpath;
}

double font_instance::Advance(int glyph_id,bool vertical)
{
	int no=-1;
	if ( id_to_no.find(glyph_id) == id_to_no.end() ) {
		LoadGlyph(glyph_id);
		if ( id_to_no.find(glyph_id) == id_to_no.end() ) {
			// didn't load
		} else {
			no=id_to_no[glyph_id];
		}
	} else {
		no=id_to_no[glyph_id];
	}
	if ( no >= 0 ) {
		if ( vertical ) {
			return glyphs[no].v_advance;
		} else {
			return glyphs[no].h_advance;
		}
	}
	return 0;
}	


raster_font* font_instance::RasterFont(const NR::Matrix &trs,double stroke_width,bool vertical,JoinType stroke_join,ButtType stroke_cap)
{
	font_style  nStyle;
	nStyle.transform=trs;
	nStyle.vertical=vertical;
	nStyle.stroke_width=stroke_width;
	nStyle.stroke_cap=stroke_cap;
	nStyle.stroke_join=stroke_join;
	nStyle.nbDash=0;
	nStyle.dash_offset=0;
	nStyle.dashes=NULL;
	return RasterFont(nStyle);
}

raster_font* font_instance::RasterFont(const font_style &nStyle)
{
	raster_font  *res=NULL;
	if ( loadedStyles.find(nStyle) == loadedStyles.end() ) {
		raster_font  *nR=new raster_font;
		nR->style=nStyle;
		if ( nStyle.stroke_width > 0 && nStyle.nbDash > 0 && nStyle.dashes ) {
			nR->style.dashes=(double*)malloc(nStyle.nbDash*sizeof(double));
			memcpy(nR->style.dashes,nStyle.dashes,nStyle.nbDash*sizeof(double));
		}
		nR->Ref();
		nR->daddy=this;
		loadedStyles[nStyle]=nR;
		res=nR;
	} else {
		res=loadedStyles[nStyle];
		res->Ref();
	}
	if ( res ) Ref();
	return res;
}

void font_instance::RemoveRasterFont(raster_font* who)
{
	if ( who == NULL ) return;
	if ( loadedStyles.find(who->style) == loadedStyles.end() ) {
		// not found
	} else {
		loadedStyles.erase(loadedStyles.find(who->style));
		//	printf("RemoveRasterFont ");
		Unref();
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
