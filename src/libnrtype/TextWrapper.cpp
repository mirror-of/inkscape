/*
 *  TextWrapper.cpp
 *  testICU
 *
 */

#include "TextWrapper.h"

#include "FontFactory.h"
#include "FontInstance.h"

text_wrapper::text_wrapper(void)
{
	utf8_text=NULL;
	uni32_text=NULL;
	glyph_text=NULL;
	utf8_length=0;
	uni32_length=0;
	glyph_length=0;
	utf8_codepoint=NULL;
	uni32_codepoint=NULL;
	default_font=NULL;
	bounds=NULL;
	nbBound=maxBound=0;
	
	font_factory* font_src=font_factory::Default();
	pLayout=pango_layout_new(font_src->fontContext);
	pango_layout_set_single_paragraph_mode(pLayout,true);
	pango_layout_set_width(pLayout,-1);
}
text_wrapper::~text_wrapper(void)
{
	if ( utf8_text ) free(utf8_text);
	if ( uni32_text ) free(uni32_text);
	if ( glyph_text ) free(glyph_text);
	if ( utf8_codepoint ) free(utf8_codepoint);
	if ( uni32_codepoint ) free(uni32_codepoint);
	if ( default_font ) default_font->Unref();
	for (int i=0;i<nbBound;i++) {
		switch ( bounds[i].type ) {
			default:
				break;
		}
	}
	if ( bounds ) free(bounds);
	default_font=NULL;

	g_object_unref(pLayout);
}

void            text_wrapper::SetDefaultFont(font_instance* iFont)
{
	if ( iFont ) iFont->Ref();
	if ( default_font ) default_font->Unref();
	default_font=iFont;
}
void            text_wrapper::AppendUTF8(char* text,int len)
{
	if ( text[0] == '\n' || text[0] == '\r' ) {
		printf("string starts with a return: gonna eat it\n");
		if ( len > 0 ) {
			while ( len > 0 && ( *text == '\n' || *text == '\r' ) ) {text++;len--;}
		} else {
			while ( *text == '\n' || *text == '\r' ) text++;
		}
	}
	if ( len == 0 || text == NULL || *text == 0 ) return;
	if ( g_utf8_validate(text,len,NULL) ) {
	} else {
		printf("invalid utf8\n");
		return;
	}
	int nlen=len;
	if ( nlen < 0 ) {
		char* cur=text;
		nlen=0;
		while ( cur[nlen] != 0 ) nlen++;		
	}
	utf8_text=(char*)realloc(utf8_text,(utf8_length+nlen+1)*sizeof(char));
	uni32_codepoint=(int*)realloc(uni32_codepoint,(utf8_length+nlen+1)*sizeof(int));
	memcpy(utf8_text+utf8_length,text,nlen*sizeof(char));
	utf8_length+=nlen;
	utf8_text[utf8_length]=0;
	
	if ( uni32_text ) free(uni32_text);
	if ( utf8_codepoint ) free(utf8_codepoint);
	uni32_text=NULL;
	utf8_codepoint=NULL;
	uni32_length=0;
	{
		char*  p=utf8_text;
		while ( *p ) {
			p=g_utf8_next_char(p);
			uni32_length++;
		}
	}
	uni32_text=(gunichar*)malloc((uni32_length+1)*sizeof(gunichar));
	utf8_codepoint=(int*)malloc((uni32_length+1)*sizeof(int));
	{
		char*  p=utf8_text;
		int    i=0;
		int    l_o=0;
		while ( *p ) {
			uni32_text[i]=g_utf8_get_char(p);
			int n_o=((int)p)-((int)utf8_text);
			utf8_codepoint[i]=n_o;
			for (int j=l_o;j<n_o;j++) uni32_codepoint[j]=i-1;
			l_o=n_o;
			p=g_utf8_next_char(p);
			i++;
		}
		for (int j=l_o;j<utf8_length;j++) uni32_codepoint[j]=uni32_length-1;

		uni32_codepoint[utf8_length]=uni32_length;
		uni32_text[uni32_length]=0;	
		utf8_codepoint[uni32_length]=utf8_length;	
	}
}


void            text_wrapper::DoLayout(void)
{	
	if ( default_font == NULL ) return;
	if ( uni32_length <= 0 || utf8_length <= 0 ) return;
	
	pango_layout_set_font_description(pLayout,default_font->descr);
	pango_layout_set_text(pLayout,utf8_text,utf8_length);
	
	if ( glyph_text ) free(glyph_text);
	glyph_text=NULL;
	glyph_length=0;
	
	double pango_to_ink=(1.0/((double)PANGO_SCALE));
	int max_g=0;
	PangoLayoutIter* pIter=pango_layout_get_iter(pLayout);
	do {
		PangoLayoutLine* pLine=pango_layout_iter_get_line(pIter); // no need for unref
		int              plOffset=pLine->start_index;
		PangoRectangle   ink_r,log_r;
		pango_layout_iter_get_line_extents(pIter,&ink_r,&log_r);
		double           plY=(1.0/((double)PANGO_SCALE))*((double)log_r.y);
		double           plX=(1.0/((double)PANGO_SCALE))*((double)log_r.x);
		GSList* curR=pLine->runs;
		while ( curR ) {
			PangoLayoutRun* pRun=(PangoLayoutRun*)curR->data;
			int             prOffset=pRun->item->offset;
			if ( pRun ) {
				int o_g_l=glyph_length;
				for (int i=0;i<pRun->glyphs->num_glyphs;i++) {
					if ( glyph_length >= max_g ) {
						max_g=2*glyph_length+1;
						glyph_text=(one_glyph*)realloc(glyph_text,(max_g+1)*sizeof(one_glyph));
					}
					glyph_text[glyph_length].font=pRun->item->analysis.font;
					glyph_text[glyph_length].gl=pRun->glyphs->glyphs[i].glyph;
					glyph_text[glyph_length].uni_st=plOffset+prOffset+pRun->glyphs->log_clusters[i];
					if ( pRun->item->analysis.level == 1 ) {
						// rtl
						if ( i < pRun->glyphs->num_glyphs-1 ) {
							glyph_text[glyph_length+1].uni_en=glyph_text[glyph_length].uni_st;
						}
						glyph_text[glyph_length].uni_dir=1;
						glyph_text[glyph_length+1].uni_dir=1;
					} else {
						// ltr
						if ( i > 0 ) {
							glyph_text[glyph_length-1].uni_en=glyph_text[glyph_length].uni_st;
						}
						glyph_text[glyph_length].uni_dir=0;
						glyph_text[glyph_length+1].uni_dir=0;
					}
					glyph_text[glyph_length].x=plX+pango_to_ink*((double)pRun->glyphs->glyphs[i].geometry.x_offset);
					glyph_text[glyph_length].y=plY+pango_to_ink*((double)pRun->glyphs->glyphs[i].geometry.y_offset);
					plX+=pango_to_ink*((double)pRun->glyphs->glyphs[i].geometry.width);
					glyph_text[glyph_length+1].x=plX;
					glyph_text[glyph_length+1].y=plY;
					glyph_length++;
				}
				if ( pRun->item->analysis.level == 1 ) {
					// rtl
					if ( glyph_length > o_g_l ) glyph_text[o_g_l].uni_en=plOffset+prOffset+pRun->item->length;
				} else {
					if ( glyph_length > 0 ) glyph_text[glyph_length-1].uni_en=plOffset+prOffset+pRun->item->length;
				}
				glyph_text[glyph_length].gl=0;
				glyph_text[glyph_length].uni_st=glyph_text[glyph_length].uni_en=plOffset+prOffset+pRun->item->length;
			}
			curR=curR->next;
		}
		
	} while ( pango_layout_iter_next_line(pIter) );
	pango_layout_iter_free(pIter);
	
	PangoLogAttr*  pAttrs=NULL;
	int            nbAttr=0;
	pango_layout_get_log_attrs(pLayout,&pAttrs,&nbAttr);
	MakeTextBoundaries(pAttrs,nbAttr);
	SortBoundaries();
	{
		int    c_st=-1,c_en=-1;
		for (int i=0;i<glyph_length;i++) {
			int   g_st=glyph_text[i].uni_st,g_en=glyph_text[i].uni_en;
			glyph_text[i].char_start=false;
			if ( glyph_text[i].uni_dir == 0 ) {
				if ( IsBound(bnd_char,g_st,c_st) ) {
					if ( g_st == bounds[c_st].uni_pos ) glyph_text[i].char_start=true;
				}
			} else {
				if ( IsBound(bnd_char,g_en,c_en) ) {
					if ( g_en == bounds[c_en].uni_pos ) glyph_text[i].char_start=true;
				}
			}
		}
	}
	if ( glyph_length > 0 ) {
		glyph_text[glyph_length].char_start=true;
	}
	if ( pAttrs ) g_free(pAttrs);
	
	for (int i=0;i<glyph_length;i++) {
		glyph_text[i].uni_st=uni32_codepoint[glyph_text[i].uni_st];
		glyph_text[i].uni_en=uni32_codepoint[glyph_text[i].uni_en];
		glyph_text[i].x/=512;
		glyph_text[i].y/=512;
	}
	if ( glyph_length > 0 ) {
		glyph_text[glyph_length].x/=512;
		glyph_text[glyph_length].y/=512;
	}
}
void            text_wrapper::MakeVertical(void)
{
	if ( glyph_length <= 0 ) return;
	font_factory* font_src=font_factory::Default();

	double   baseY=glyph_text[0].y;
	double   lastY=baseY;
	int      g_st=0,g_en=0;
	int      nbLetter=0;
	PangoFont*     curPF=NULL;
	font_instance* curF=NULL;
	do {
		g_st=g_en;
		do {
			g_en++;
		} while ( g_en < glyph_length && glyph_text[g_en].char_start == false );
		if ( g_st < g_en && g_en <= glyph_length ) {
			double n_adv=0;
			double  minX=glyph_text[g_st].x,maxX=glyph_text[g_st].x;
			for (int i=g_st;i<g_en;i++) {
				if ( glyph_text[i].font != curPF ) {
					if ( curF ) curF->Unref();
					curF=NULL;
					curPF=glyph_text[i].font;
					if ( curPF ) {
						PangoFontDescription* pfd=pango_font_describe(curPF);
						curF=font_src->Face(pfd);
						pango_font_description_free(pfd);
					}
				}
				double  x=(curF)?curF->Advance(glyph_text[i].gl,true):0;
				if ( x > n_adv ) n_adv=x;
				if ( glyph_text[i].x < minX ) minX=glyph_text[i].x;
				if ( glyph_text[i].x > maxX ) maxX=glyph_text[i].x;
			}
			lastY+=n_adv;
			for (int i=g_st;i<g_en;i++) {
				glyph_text[i].x-=minX;
				glyph_text[i].y+=lastY;
			}
			g_st=g_en;
		}
		nbLetter++;
	} while ( g_st < glyph_length );
	if ( curF ) curF->Unref();
}
void            text_wrapper::MergeWhiteSpace(void)
{
	if ( glyph_length <= 0 ) return;
	
	double      delta_x=0,delta_y=0;
	bool        inWhite=true;
	int         wpos=0,rpos=0;
	for (rpos=0;rpos<glyph_length;rpos++) {
		glyph_text[wpos].gl=glyph_text[rpos].gl;
		glyph_text[wpos].uni_st=glyph_text[rpos].uni_st;
		glyph_text[wpos].uni_en=glyph_text[rpos].uni_en;
		glyph_text[wpos].font=glyph_text[rpos].font;
		glyph_text[wpos].x=glyph_text[rpos].x-delta_x;
		glyph_text[wpos].y=glyph_text[rpos].y-delta_y;
		wpos++;
		if ( g_unichar_isspace(uni32_text[glyph_text[rpos].uni_st]) ) {
			if ( inWhite ) { 
				// eat me
				delta_x+=glyph_text[rpos+1].x-glyph_text[rpos].x;
				delta_y+=glyph_text[rpos+1].y-glyph_text[rpos].y;
				wpos--;
			}
			inWhite=true;
		} else {
			inWhite=false;
		}
	}
	glyph_text[wpos].x=glyph_text[rpos].x-delta_x;
	glyph_text[wpos].y=glyph_text[rpos].y-delta_y;
	glyph_length=wpos;
}
void            text_wrapper::AddLetterSpacing(double dx,double dy)
{
	if ( glyph_length <= 0 ) return;
	int   nbLetter=0;

	for (int i=0;i<glyph_length;i++) {
		glyph_text[i].x+=dx*nbLetter;
		glyph_text[i].y+=dy*nbLetter;
		if ( glyph_text[i].char_start ) nbLetter++;
	}
	glyph_text[glyph_length].x+=dx*nbLetter;
	glyph_text[glyph_length].y+=dy*nbLetter;
}
bool            text_wrapper::NextCharacter(int &st,int &en)
{
	if ( st < 0 || en < 0 ) {
		st=0;en=0;
	}
	if ( st >= en ) {
		en=st;
	}
	
	if ( st >= glyph_length || en >= glyph_length ) return false; // finished
	
	st=en;
	en++;
	while ( en < glyph_length ) {
		if ( glyph_text[en].char_start ) return true;
		en++;
	}
	return true;
}

// boundary handling
int             text_wrapper::AddBoundary(text_boundary &ib)
{
	if ( nbBound >= maxBound ) {
		maxBound=2*nbBound+1;
		bounds=(text_boundary*)realloc(bounds,maxBound*sizeof(text_boundary));
	}
	int n=nbBound++;
	bounds[n]=ib;
	bounds[n].ind=bounds[n].inv_ind=n;
	bounds[n].other=-1;
	return n;
}
void            text_wrapper::AddTwinBoundaries(text_boundary &is,text_boundary &ie)
{
	int  ns=AddBoundary(is);
	int  ne=AddBoundary(ie);
	bounds[ns].start=true;
	bounds[ns].other=ne;
	bounds[ne].start=false;
	bounds[ne].other=ns;
}
static int    CmpBound(const void* a,const void* b) {
	text_boundary* ta=(text_boundary*)a;
	text_boundary* tb=(text_boundary*)b;
	if ( ta->uni_pos < tb->uni_pos ) return -1;
	if ( ta->uni_pos > tb->uni_pos ) return 1;
	if ( ta->start && tb->start == false ) return -1;
	if ( ta->start == false && tb->start ) return 1;
	return 0;
}
void            text_wrapper::SortBoundaries(void)
{
	for (int i=0;i<nbBound;i++) {
		bounds[i].ind=i;
		bounds[i].inv_ind=i;
	}
	qsort(bounds,nbBound,sizeof(text_boundary),CmpBound);
	for (int i=0;i<nbBound;i++) {
		bounds[bounds[i].ind].inv_ind=i;
	}
	for (int i=0;i<nbBound;i++) {
		if ( bounds[i].other >= 0 ) {
			bounds[i].other=bounds[bounds[i].other].inv_ind;
		}
	}
}
void            text_wrapper::MakeTextBoundaries(PangoLogAttr* pAttrs,int nAttr)
{
	if ( pAttrs == NULL || nAttr <= 0 || uni32_length <= 0 ) return;
	if ( nAttr > uni32_length+1 ) nAttr=uni32_length+1;
	int   last_c_st=-1;
	int   last_w_st=-1;
	int   last_s_st=-1;
	int   last_p_st=0;
	for (int i=0;i<=nAttr;i++) {
		text_boundary nbs;
		text_boundary nbe;
		nbs.uni_pos=i;
		nbs.start=true;
		nbe.uni_pos=i;
		nbe.start=false;
		// letters
		if ( pAttrs[i].is_cursor_position || i == nAttr ) {
			if ( last_c_st >= 0 ) {
				nbs.type=nbe.type=bnd_char;
				nbs.uni_pos=last_c_st;
				nbe.uni_pos=i;
				AddTwinBoundaries(nbs,nbe);
			}
			last_c_st=i;
		}
		// words
		if ( pAttrs[i].is_word_start || i == nAttr ) {
			if ( last_w_st >= 0 ) {
				nbs.type=nbe.type=bnd_word;
				nbs.uni_pos=last_w_st;
				nbe.uni_pos=i;
				nbs.data.i=nbe.data.i=(pAttrs[last_w_st].is_white)?1:0;
				AddTwinBoundaries(nbs,nbe);
			}
			last_w_st=i;
		}
		if ( pAttrs[i].is_word_end ) {
			if ( last_w_st >= 0 ) {
				nbs.type=nbe.type=bnd_word;
				nbs.uni_pos=last_w_st;
				nbe.uni_pos=i;
				nbs.data.i=nbe.data.i=(pAttrs[last_w_st].is_white)?1:0;
				AddTwinBoundaries(nbs,nbe);
			}
			last_w_st=i;
		}
		// sentences
		if ( pAttrs[i].is_sentence_boundary || i == nAttr ) {
			if ( last_s_st >= 0 ) {
				nbs.type=nbe.type=bnd_sent;
				nbs.uni_pos=last_s_st;
				nbe.uni_pos=i;
				AddTwinBoundaries(nbs,nbe);
			}
			last_s_st=i;
		}
		// paragraphs
		if ( uni32_text[i] == '\n' || uni32_text[i] == '\r' || i == nAttr ) { // too simple to be true?
			nbs.type=nbe.type=bnd_para;
			nbs.uni_pos=last_p_st;
			nbe.uni_pos=i+1;
			AddTwinBoundaries(nbs,nbe);
			last_p_st=i+1;
		}
	}
}
bool            text_wrapper::IsBound(int bnd_type,int g_st,int &c_st)
{
	if ( c_st < 0 ) c_st=0;
	int  scan_dir=0;
	while ( c_st >= 0 && c_st < nbBound ) {
		if ( bounds[c_st].uni_pos == g_st && bounds[c_st].type == bnd_type ) {
			return true;
		}
		if ( bounds[c_st].uni_pos < g_st ) {
			if ( scan_dir < 0 ) break;
			c_st++;
			scan_dir=1; 
		} else if ( bounds[c_st].uni_pos > g_st ) {
			if ( scan_dir > 0 ) break;
			c_st--;
			scan_dir=-1; 
		} else {
			// good pos, wrong type
			while ( c_st > 0 && bounds[c_st].uni_pos == g_st ) {
				c_st--;
			}
			if ( bounds[c_st].uni_pos < g_st ) c_st++;
			while ( c_st < nbBound && bounds[c_st].uni_pos == g_st ) {
				if ( bounds[c_st].type == bnd_type ) {
					return true;
				}
				c_st++;
			}
			break;
		}
	}
	return false;
}
bool            text_wrapper::Contains(int bnd_type,int g_st,int g_en,int &c_st,int &c_en)
{
	if ( c_st < 0 ) c_st=0;
	bool found=false;
	int  scan_dir=0;
	while ( c_st >= 0 && c_st < nbBound ) {
		if ( bounds[c_st].type == bnd_type ) {
			if ( bounds[c_st].start ) {
				c_en=bounds[c_st].other;
			} else {
			}
		}
		if ( bounds[c_st].type == bnd_type && c_en == bounds[c_st].other ) {
			if ( g_st >= bounds[c_st].uni_pos && g_en <= bounds[c_en].uni_pos ) {
				// character found
				found=true;
				break;
			}
		}
		if ( bounds[c_st].uni_pos < g_st ) {
			if ( scan_dir < 0 ) break;
			c_st++;
			scan_dir=1; 
		} else if ( bounds[c_st].uni_pos > g_st ) {
			if ( scan_dir > 0 ) break;
			c_st--;
			scan_dir=-1; 
		} else {
			// good pos, wrong type
			while ( c_st > 0 && bounds[c_st].uni_pos == g_st ) {
				c_st--;
			}
			if ( bounds[c_st].uni_pos < g_st ) c_st++;
			while ( c_st < nbBound && bounds[c_st].uni_pos == g_st ) {
				if ( bounds[c_st].type == bnd_type ) {
					if ( bounds[c_st].start ) {
						c_en=bounds[c_st].other;
					} else {
					}
				}
				if ( bounds[c_st].type == bnd_type && c_en == bounds[c_st].other ) {
					if ( g_st >= bounds[c_st].uni_pos && g_en <= bounds[c_en].uni_pos ) {
						// character found
						return true;
					}
				}
				c_st++;
			}
			
			break;
		}
	}
	return found;
}


