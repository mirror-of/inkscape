/*
 *  TextWrapper.cpp
 *  testICU
 *
 */

#include "TextWrapper.h"

#include "FontFactory.h"
#include "FontInstance.h"

#include <svg/svg.h>

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
	boxes=NULL;
	nbBox=maxBox=0;
	paras=NULL;
	nbPara=maxPara=0;
	kern_x=kern_y=NULL;
	last_addition=-1;
	
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
	if ( boxes ) free(boxes);
	if ( paras ) free(paras);
	if ( kern_x ) free(kern_x);
	if ( kern_y ) free(kern_y);
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
	if ( utf8_length <= 0 ) {
		if ( text[0] == '\n' || text[0] == '\r' ) {
			printf("string starts with a return: gonna eat it\n");
			if ( len > 0 ) {
				while ( len > 0 && ( *text == '\n' || *text == '\r' ) ) {text++;len--;}
			} else {
				while ( *text == '\n' || *text == '\r' ) text++;
			}
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
	
	last_addition=uni32_length;
	
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
	if ( uni32_length > last_addition ) {
		if ( kern_x ) {
			kern_x=(double*)realloc(kern_x,(uni32_length+1)*sizeof(double));
			for (int i=last_addition;i<=uni32_length;i++) kern_x[i]=0;
		}
		if ( kern_y ) {
			kern_y=(double*)realloc(kern_y,(uni32_length+1)*sizeof(double));
			for (int i=last_addition;i<=uni32_length;i++) kern_y[i]=0;
		}
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
	ChunkText();
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
void            text_wrapper::ChunkText(void)
{
	int    c_st=-1,c_en=-1;
	for (int i=0;i<glyph_length;i++) {
		int   g_st=glyph_text[i].uni_st,g_en=glyph_text[i].uni_en;
		glyph_text[i].char_start=false;
		glyph_text[i].word_start=false;
		glyph_text[i].para_start=false;
		if ( glyph_text[i].uni_dir == 0 ) {
			if ( IsBound(bnd_char,g_st,c_st) ) {
				if ( g_st == bounds[c_st].uni_pos ) glyph_text[i].char_start=true;
			}
			if ( IsBound(bnd_word,g_st,c_st) ) {
				if ( g_st == bounds[c_st].uni_pos ) glyph_text[i].word_start=true;
			}
			if ( IsBound(bnd_para,g_st,c_st) ) {
				if ( g_st == bounds[c_st].uni_pos ) glyph_text[i].para_start=true;
			}
		} else {
			if ( IsBound(bnd_char,g_en,c_en) ) {
				if ( g_en == bounds[c_en].uni_pos ) glyph_text[i].char_start=true;
			}
			if ( IsBound(bnd_word,g_en,c_en) ) {
				if ( g_en == bounds[c_en].uni_pos ) glyph_text[i].word_start=true;
			}
			if ( IsBound(bnd_para,g_en,c_en) ) {
				if ( g_en == bounds[c_en].uni_pos ) glyph_text[i].para_start=true;
			}
		}
	}

	if ( glyph_length > 0 ) {
		glyph_text[glyph_length].char_start=true;
		glyph_text[glyph_length].word_start=true;
		glyph_text[glyph_length].para_start=true;
	}
	{
		// doing little boxes
		int    g_st=-1,g_en=-1;
		while ( NextWord(g_st,g_en) ) {
			// check uniformity of fonts
			if ( g_st < g_en ) {
				int  n_st=g_st;
				int  n_en=g_st;
				bool first=true;
				do {
					n_st=n_en;
					PangoFont* curPF=glyph_text[n_st].font;
					do {
						n_en++;
					} while ( n_en < g_en && glyph_text[n_en].font == curPF );
					if ( nbBox >= maxBox ) {
						maxBox=2*nbBox+1;
						boxes=(one_box*)realloc(boxes,maxBox*sizeof(one_box));
					}
					boxes[nbBox].g_st=n_st;
					boxes[nbBox].g_en=n_en;
					boxes[nbBox].word_start=first;
					boxes[nbBox].word_end=(n_en>=g_en);
					nbBox++;
					first=false;
				} while ( n_en < g_en );
			}
		}
	}
	{
		// doing little paras
		int    g_st=-1,g_en=-1;
		while ( NextPara(g_st,g_en) ) {
			int b_st=0;
			while ( b_st < nbBox && boxes[b_st].g_st < g_st ) b_st++;
			if ( b_st < nbBox && boxes[b_st].g_st == g_st ) {
				int  b_en=b_st;
				while ( b_en < nbBox && boxes[b_en].g_en < g_en ) b_en++;
				if ( b_en < nbBox && boxes[b_en].g_en == g_en ) {
					if ( nbPara >= maxPara ) {
						maxPara=2*nbPara+1;
						paras=(one_para*)realloc(paras,maxPara*sizeof(one_para));
					}
					paras[nbPara].b_st=b_st;
					paras[nbPara].b_en=b_en;
					nbPara++;
				}
			}
		}
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
int             text_wrapper::NbLetter(int g_st,int g_en)
{
	if ( glyph_length <= 0 ) return 0;
	if ( g_st < 0 || g_st >= g_en ) {
		g_st=0;
		g_en=glyph_length;
	}
	int   nbLetter=0;	
	for (int i=g_st;i<g_en;i++) {
		if ( glyph_text[i].char_start ) nbLetter++;
	}
	return nbLetter;
}
void            text_wrapper::AddLetterSpacing(double dx,double dy,int g_st,int g_en)
{
	if ( glyph_length <= 0 ) return;
	if ( g_st < 0 || g_st >= g_en ) {
		g_st=0;
		g_en=glyph_length;
	}
	int   nbLetter=0;

	for (int i=g_st;i<g_en;i++) {
		if ( i > g_st && glyph_text[i].char_start ) nbLetter++;
		glyph_text[i].x+=dx*nbLetter;
		glyph_text[i].y+=dy*nbLetter;
	}
	if ( glyph_text[g_en].char_start ) nbLetter++;
	glyph_text[g_en].x+=dx*nbLetter;
	glyph_text[g_en].y+=dy*nbLetter;
}
bool            text_wrapper::NextChar(int &st,int &en)
{
	if ( st < 0 || en < 0 ) {st=0;en=0;}
	if ( st >= en ) en=st;	
	if ( st >= glyph_length || en >= glyph_length ) return false; // finished
	st=en;
	do {
		en++;
	} while ( en < glyph_length && glyph_text[en].char_start == false );
	return true;
}
bool            text_wrapper::NextWord(int &st,int &en)
{
	if ( st < 0 || en < 0 ) {st=0;en=0;}
	if ( st >= en ) en=st;	
	if ( st >= glyph_length || en >= glyph_length ) return false; // finished
	st=en;
	do {
		en++;
	} while ( en < glyph_length && glyph_text[en].word_start == false );
	return true;
}
bool            text_wrapper::NextPara(int &st,int &en)
{
	if ( st < 0 || en < 0 ) {st=0;en=0;}
	if ( st >= en ) en=st;	
	if ( st >= glyph_length || en >= glyph_length ) return false; // finished
	st=en;
	do {
		en++;
	} while ( en < glyph_length && glyph_text[en].para_start == false );
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
		if ( i == nAttr || pAttrs[i].is_cursor_position ) {
			if ( last_c_st >= 0 ) {
				nbs.type=nbe.type=bnd_char;
				nbs.uni_pos=last_c_st;
				nbe.uni_pos=i;
				AddTwinBoundaries(nbs,nbe);
			}
			last_c_st=i;
		}
		// words
		if ( i == nAttr || pAttrs[i].is_word_start ) {
			if ( last_w_st >= 0 ) {
				nbs.type=nbe.type=bnd_word;
				nbs.uni_pos=last_w_st;
				nbe.uni_pos=i;
				nbs.data.i=nbe.data.i=(pAttrs[last_w_st].is_white)?1:0;
				AddTwinBoundaries(nbs,nbe);
			}
			last_w_st=i;
		}
		if ( i < nAttr && pAttrs[i].is_word_end ) {
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
		if ( i == nAttr || pAttrs[i].is_sentence_boundary ) {
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
void            text_wrapper::MeasureBoxes(void)
{
	font_factory* f_src=font_factory::Default();
	for (int i=0;i<nbBox;i++) {
		boxes[i].ascent=0;
		boxes[i].descent=0;
		boxes[i].leading=0;
		boxes[i].width=0;
		
		PangoFont*            curPF=glyph_text[boxes[i].g_st].font;
		PangoFontDescription* pfd=pango_font_describe(curPF);
		font_instance*        curF=f_src->Face(pfd);
		if ( curF ) {
			curF->FontMetrics(boxes[i].ascent,boxes[i].descent,boxes[i].leading);
			curF->Unref();
		}
		boxes[i].width=glyph_text[boxes[i].g_en].x-glyph_text[boxes[i].g_st].x;
	}
}

void            text_wrapper::KernXForLastAddition(double *i_kern_x,int i_len,double scale)
{
  if ( i_kern_x == NULL || i_len <= 0 || last_addition < 0 || last_addition >= uni32_length || uni32_length <= 0 ) return;
  if ( kern_x == NULL ) {
    kern_x=(double*)malloc((uni32_length+1)*sizeof(double));
    for (int i=0;i<=uni32_length;i++) kern_x[i]=0;
  }
	int last_len=uni32_length-last_addition;
  if ( i_len > last_len ) i_len=last_len;
  for (int i=0;i<i_len;i++) kern_x[last_addition+i]=i_kern_x[i]*scale;
}
void            text_wrapper::KernYForLastAddition(double *i_kern_y,int i_len,double scale)
{
  if ( i_kern_y == NULL || i_len <= 0 || last_addition < 0 || last_addition >= uni32_length || uni32_length <= 0 ) return;
  if ( kern_y == NULL ) {
    kern_y=(double*)malloc((uni32_length+1)*sizeof(double));
    for (int i=0;i<=uni32_length;i++) kern_y[i]=0;
  }
	int last_len=uni32_length-last_addition;
  if ( i_len > last_len ) i_len=last_len;
  for (int i=0;i<i_len;i++) kern_y[last_addition+i]=i_kern_y[i]*scale;
}
void            text_wrapper::KernXForLastAddition(GList *i_kern_x,double scale)
{
  if ( i_kern_x == NULL || last_addition < 0 || last_addition >= uni32_length || uni32_length <= 0 ) return;
  if ( kern_x == NULL ) {
    kern_x=(double*)malloc((uni32_length+1)*sizeof(double));
    for (int i=0;i<=uni32_length;i++) kern_x[i]=0;
  }
	int last_len=uni32_length-last_addition;
	GList* l=i_kern_x;
  for (int i=0;i<last_len&&l&&l->data;i++,l=l->next) {
		kern_x[last_addition+i]=((SPSVGLength *) l->data)->computed*scale;
	}
}
void            text_wrapper::KernYForLastAddition(GList *i_kern_y,double scale)
{
  if ( i_kern_y == NULL || last_addition < 0 || last_addition >= uni32_length || uni32_length <= 0 ) return;
  if ( kern_y == NULL ) {
    kern_y=(double*)malloc((uni32_length+1)*sizeof(double));
    for (int i=0;i<=uni32_length;i++) kern_y[i]=0;
  }
	int last_len=uni32_length-last_addition;
	GList* l=i_kern_y;
  for (int i=0;i<last_len&&l&&l->data;i++,l=l->next) {
		kern_y[last_addition+i]=((SPSVGLength *) l->data)->computed*scale;
	}
}

void            text_wrapper::AddDxDy(void)
{
	if ( glyph_length <= 0 ) return;
	if ( kern_x ) {
		double  sum=0;
		int     l_pos=-1;
		for (int i=0;i<glyph_length;i++) {
			int   n_pos=glyph_text[i].uni_st;
			if ( l_pos < n_pos ) {
				for (int j=l_pos+1;j<=n_pos;j++) sum+=kern_x[j];
			} else if ( l_pos > n_pos ) {
				for (int j=l_pos;j>n_pos;j--) sum-=kern_x[j];
			}
			l_pos=n_pos;
						
			glyph_text[i].x+=sum;
		}
		{
			int   n_pos=uni32_length;
			if ( l_pos < n_pos ) {
				for (int j=l_pos+1;j<=n_pos;j++) sum+=kern_x[j];
			} else if ( l_pos > n_pos ) {
				for (int j=l_pos;j>n_pos;j--) sum-=kern_x[j];
			}
			l_pos=n_pos;
			glyph_text[glyph_length].x+=sum;
		}
	}
	if ( kern_y ) {
		double  sum=0;
		int     l_pos=-1;
		for (int i=0;i<glyph_length;i++) {
			int   n_pos=glyph_text[i].uni_st;
			if ( l_pos < n_pos ) {
				for (int j=l_pos+1;j<=n_pos;j++) sum+=kern_y[j];
			} else if ( l_pos > n_pos ) {
				for (int j=l_pos;j>n_pos;j--) sum-=kern_y[j];
			}
			l_pos=n_pos;
						
			glyph_text[i].y+=sum;
		}
		{
			int   n_pos=uni32_length;
			if ( l_pos < n_pos ) {
				for (int j=l_pos+1;j<=n_pos;j++) sum+=kern_y[j];
			} else if ( l_pos > n_pos ) {
				for (int j=l_pos;j>n_pos;j--) sum-=kern_y[j];
			}
			l_pos=n_pos;
			glyph_text[glyph_length].y+=sum;
		}
	}
}

