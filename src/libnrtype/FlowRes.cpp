/*
 *  FlowSrc.cpp
 */

#include <math.h>

#include "FlowRes.h"

#include "FlowStyle.h"
#include "FlowBoxes.h"
#include "FlowEater.h"
#include "FlowSrc.h"
	
#include "../style.h"

#include <pango/pango.h>

#include "livarot/Path.h"

#include "libnrtype/FontFactory.h"
#include "libnrtype/font-instance.h"
#include "libnrtype/font-style-to-pos.h"

/*
 *
 */

flow_res::flow_res(void)
{
	nbGlyph=maxGlyph=0;
	glyphs=NULL;
	nbGroup=maxGroup=0;
	groups=NULL;
	nbChar=maxChar=0;
	chars=NULL;
	nbChunk=maxChunk=0;
	chunks=NULL;
	nbSpan=maxSpan=0;
	spans=NULL;
	nbLetter=maxLetter=0;
	letters=NULL;

	last_style_set=false;
	last_c_style=NULL;
	last_rtl=false;
	cur_ascent=cur_descent=cur_leading=0;
	cur_offset=0;
}
flow_res::~flow_res(void)
{
	if ( letters ) free(letters);
	if ( spans ) free(spans);
	if ( chunks ) free(chunks);
	if ( chars ) free(chars);
	if ( glyphs ) free(glyphs);
	if ( groups ) free(groups);
	nbGlyph=maxGlyph=0;
	glyphs=NULL;
	nbGroup=maxGroup=0;
	groups=NULL;
	nbChar=maxChar=0;
	chars=NULL;
	nbChunk=maxChunk=0;
	chunks=NULL;
	nbSpan=maxSpan=0;
	spans=NULL;
	nbLetter=maxLetter=0;
	letters=NULL;
}
void               flow_res::Reset(void)
{
	nbGlyph=nbGroup=0;
	nbChar=0;
	last_style_set=false;
	last_c_style=NULL;
	last_rtl=false;
	cur_ascent=cur_descent=cur_leading=0;
	cur_offset=0;
}
void               flow_res::AfficheOutput(void)
{
/*	printf("%i groups\n",nbGroup);
	for (int i=0;i<nbGroup;i++) {
		printf(" group %i : glyphs=(%i -> %i) font=%s\n",i,groups[i].st,groups[i].en,pango_font_description_get_family(groups->style->theFont->descr));
		for (int j=groups[i].st;j<groups[i].en;j++) {
			printf("  glyph %i : id=%i pos=(%f %f)\n",j,glyphs[j].g_id,glyphs[j].g_x,glyphs[j].g_y);
		}
	}*/
	printf("%i chunks\n",nbChunk);
	for (int i=0;i<nbChunk;i++) {
		printf(" chunk %i : span=(%i -> %i) rtl=%i y=%f\n",i,chunks[i].s_st,chunks[i].s_en,(chunks[i].rtl)?1:0,chunks[i].y);
		for (int j=chunks[i].s_st;j<chunks[i].s_en;j++) {
			printf("  span %i : letter=(%i -> %i) rtl=%i style=%p\n",j,spans[j].l_st,spans[j].l_en,(spans[j].rtl)?1:0,spans[j].c_style);
			for (int k=spans[j].l_st;k<spans[j].l_en;k++) {
				char  savC=chars[letters[k].t_en];
				chars[letters[k].t_en]=0;
				printf("   letter %i : no=%i text=(%i -> %i)",k,letters[k].no,letters[k].t_st,letters[k].t_en);
				if ( chars[letters[k].t_st] == '\n' ) printf("\\n "); else printf("%s ",chars+letters[k].t_st);
				printf("p=(%f:%f/%f) k=(%f:%f)\n",letters[k].x_st,letters[k].x_en,letters[k].y,letters[k].kern_x,letters[k].kern_y);
				chars[letters[k].t_en]=savC;
			}
		}
	}
}
void               flow_res::AddGroup(text_style* g_s)
{
	if ( nbGroup >= maxGroup ) {
		maxGroup=2*nbGroup+1;
		groups=(flow_glyph_group*)realloc(groups,maxGroup*sizeof(flow_glyph_group));
	}
	groups[nbGroup].st=nbGlyph;
	groups[nbGroup].en=nbGlyph;
	groups[nbGroup].style=g_s;
	groups[nbGroup].g_gr=NULL;
	nbGroup++;
}
void               flow_res::AddGlyph(int g_id,double g_x,double g_y,double g_w)
{
	if ( nbGlyph >= maxGlyph ) {
		maxGlyph=2*nbGlyph+1;
		glyphs=(flow_glyph*)realloc(glyphs,maxGlyph*sizeof(flow_glyph));
	}
	if ( nbGroup <= 0 || groups[nbGroup-1].style != last_c_style ) AddGroup(last_c_style);
	glyphs[nbGlyph].g_id=g_id;
	glyphs[nbGlyph].let=nbLetter-1;
	glyphs[nbGlyph].g_x=g_x;
	glyphs[nbGlyph].g_y=g_y;
	glyphs[nbGlyph].g_font=NULL;
	glyphs[nbGlyph].g_gl=NULL;
	nbGlyph++;
	groups[nbGroup-1].en=nbGlyph;
	if ( nbLetter > 0 ) {
		letters[nbLetter-1].g_en=nbGlyph;
		if ( spans[nbSpan-1].rtl ) {
			letters[nbLetter-1].x_en-=g_w;
		} else {
			letters[nbLetter-1].x_en+=g_w;
		}
	}
}


void               flow_res::StartChunk(double i_x_st,double i_x_en,double i_y,bool i_rtl,double spacing)
{
	//printf("start_chunk %f %f\n",i_x_st,i_y);
	if ( nbChunk >= maxChunk ) {
		maxChunk=2*nbChunk+1;
		chunks=(flow_styled_chunk*)realloc(chunks,maxChunk*sizeof(flow_styled_chunk));
	}
	chunks[nbChunk].s_st=chunks[nbChunk].s_en=nbSpan;
	chunks[nbChunk].l_st=chunks[nbChunk].l_en=nbLetter;
	chunks[nbChunk].rtl=i_rtl;
	chunks[nbChunk].x_st=i_x_st;
	chunks[nbChunk].x_en=i_x_en;
	chunks[nbChunk].y=i_y;
	chunks[nbChunk].spacing=spacing;
	chunks[nbChunk].ascent=cur_ascent;
	chunks[nbChunk].descent=cur_descent;
	chunks[nbChunk].leading=cur_leading;
	chunks[nbChunk].mommy=cur_mommy;
	nbChunk++;
	last_rtl=i_rtl;
	last_style_set=false;
	last_c_style=NULL;
}
void               flow_res::SetChunkInfo(double ascent,double descent,double leading,text_holder* mommy)
{
	cur_mommy=mommy;
	cur_ascent=ascent;
	cur_descent=descent;
	cur_leading=leading;
}
void               flow_res::SetSourcePos(int i_pos)
{
	cur_offset=i_pos;
}

void               flow_res::StartSpan(text_style* i_style,bool i_rtl)
{
	//printf("start_style %x %i\n",i_style,(i_rtl)?1:0);
	if ( nbSpan >= maxSpan ) {
		maxSpan=2*nbSpan+1;
		spans=(flow_styled_span*)realloc(spans,maxSpan*sizeof(flow_styled_span));
	}
	spans[nbSpan].l_st=spans[nbSpan].l_en=nbLetter;
	spans[nbSpan].rtl=i_rtl;
	spans[nbSpan].c_style=i_style;
	nbSpan++;
	if ( nbChunk > 0 ) chunks[nbChunk-1].s_en=nbSpan;
	last_style_set=true;
	last_c_style=i_style;
	last_rtl=i_rtl;
}
void               flow_res::EndWord(void)
{
}
void               flow_res::StartLetter(text_style* i_style,bool i_rtl,double k_x,double k_y,double p_x,double p_y,double rot,int i_no,int i_utf8_offset)
{
	//printf("start_letter %x %i o=%i\n",i_style,(i_rtl)?1:0,i_utf8_offset);
	//if ( i_style == NULL ) i_style=last_c_style; // no adding letters without style
	if ( last_style_set == false || i_style != last_c_style || i_rtl != last_rtl) {
		StartSpan(i_style,i_rtl);
	}
	last_c_style=i_style;
	
	if ( nbLetter >= maxLetter ) {
		maxLetter=2*nbLetter+1;
		letters=(flow_styled_letter*)realloc(letters,maxLetter*sizeof(flow_styled_letter));
	}
	letters[nbLetter].t_st=letters[nbLetter].t_en=nbChar;
	letters[nbLetter].g_st=letters[nbLetter].g_en=nbGlyph;
	letters[nbLetter].ucs4_offset=0;
	letters[nbLetter].kern_x=k_x;
	letters[nbLetter].kern_y=k_y;
	letters[nbLetter].pos_x=p_x;
	letters[nbLetter].pos_y=p_y;
	letters[nbLetter].x_st=letters[nbLetter].x_en=p_x;
	letters[nbLetter].y=p_y;
	letters[nbLetter].no=i_no;
	letters[nbLetter].rotate=rot;
	letters[nbLetter].invisible=false;
	letters[nbLetter].utf8_offset=cur_offset+i_utf8_offset;
	nbLetter++;
	if ( nbSpan > 0 ) spans[nbSpan-1].l_en=nbLetter;
}
void               flow_res::AddText(char* iText,int iLen)
{
	if ( iLen <= 0 ) return;
	if ( nbChunk <= 0 || nbSpan <= 0 || nbLetter <= 0 ) return;
	
	if ( nbChar+iLen >= maxChar ) {
		maxChar=2*nbChar+iLen+1;
		chars=(char*)realloc(chars,maxChar*sizeof(char));
	}
	
	//bool      line_rtl=chunks[nbChunk-1].rtl;
	//bool      word_rtl=spans[nbSpan-1].rtl;
	memcpy(chars+nbChar,iText,iLen*sizeof(char));
	if ( nbLetter > 0 ) letters[nbLetter-1].t_en=nbChar+iLen;
	nbChar+=iLen;
	chars[nbChar]=0;
}

void               flow_res::ComputeIntervals(void)
{
	for (int i=0;i<nbSpan;i++) {
		if ( spans[i].l_st < spans[i].l_en ) {
			spans[i].g_st=letters[spans[i].l_st].g_st;
			spans[i].g_en=letters[spans[i].l_en-1].g_en;
			if ( spans[i].rtl ) {
				spans[i].x_st=letters[spans[i].l_en-1].x_st;
				if ( letters[spans[i].l_en-1].x_en < spans[i].x_st ) spans[i].x_st=letters[spans[i].l_en-1].x_en;
				for (int j=spans[i].l_st;j<spans[i].l_en;j++) {
					spans[i].x_en=letters[j].x_en;
					if ( letters[j].x_st > spans[i].x_en ) spans[i].x_en=letters[j].x_st;
					if ( letters[j].t_st < letters[j].t_en ) {
						if ( chars[letters[j].t_st] != ' '  ) break;
					}
				}
			} else {
				for (int j=spans[i].l_st;j<spans[i].l_en;j++) {
					spans[i].x_st=letters[j].x_st;
					if ( letters[j].x_en < spans[i].x_st ) spans[i].x_st=letters[j].x_en;
					if ( letters[j].t_st < letters[j].t_en ) {
						if ( chars[letters[j].t_st] != ' '  ) break;
					}
				}
				spans[i].x_en=letters[spans[i].l_en-1].x_en;
				if ( letters[spans[i].l_en-1].x_st > spans[i].x_en ) spans[i].x_en=letters[spans[i].l_en-1].x_st;
			}
		} else {
			spans[i].g_st=spans[i].g_en=0;
			spans[i].x_st=spans[i].x_en=0;
		}
	}
	for (int i=0;i<nbChunk;i++) {
		if ( chunks[i].s_st < chunks[i].s_en ) {
			chunks[i].g_st=spans[chunks[i].s_st].g_st;
			chunks[i].g_en=spans[chunks[i].s_en-1].g_en;
			chunks[i].l_st=spans[chunks[i].s_st].l_st;
			chunks[i].l_en=spans[chunks[i].s_en-1].l_en;
			chunks[i].x_st=spans[chunks[i].s_st].x_st;
			if ( spans[chunks[i].s_en-1].x_st < chunks[i].x_st ) chunks[i].x_st=spans[chunks[i].s_en-1].x_st;
			chunks[i].x_en=spans[chunks[i].s_en-1].x_en;
			if ( spans[chunks[i].s_st].x_en > chunks[i].x_en ) chunks[i].x_en=spans[chunks[i].s_st].x_en;
		} else {
			//chunks[i].g_st=chunks[i].g_en=0;
			//chunks[i].l_st=chunks[i].l_en=0;
		}
	}
}
void							 flow_res::ComputeDY(int no)
{
	if ( no < 0 || no >= nbChunk ) return;
	double   last_y=0;
	for (int i=chunks[no].l_st;i<chunks[no].l_en;i++) {
		double   n_y=letters[i].kern_y;
		letters[i].kern_y=n_y-last_y;
		last_y=n_y;
	}
}

void               flow_res::ApplyLetterSpacing(void)
{
	// for each line:
	for (int i=0;i<nbChunk;i++) {
		// first collect all letterspacing
		int max=0;
		for (int j=chunks[i].l_st;j<chunks[i].l_en;j++) {
			if ( letters[j].no > max ) max=letters[j].no;
		}
		double    *lspc=(double*)malloc((max+1)*sizeof(double));
		for (int j=0;j<=max;j++) lspc[j]=0;
		double spc=chunks[i].spacing;
		for (int j=chunks[i].s_st;j<chunks[i].s_en;j++) {
			if ( spans[j].c_style == NULL ) continue;
			//			if ( spans[j].c_style->with_style->text->letterspacing.set ) {
				double sspc=spans[j].c_style->with_style->text->letterspacing.computed;
				for (int k=spans[j].l_st;k<spans[j].l_en;k++) {
					if ( letters[k].no >= 0 ) lspc[letters[k].no]=spc+sspc;
				}
				//			}
		}
		// do a running sum to get the actual displacements
		double cumul=0;
		for (int j=0;j<=max;j++) {
			double nspc=lspc[j];
			lspc[j]=cumul;
			cumul+=nspc;
		}
		// displace each letter
		for (int j=chunks[i].s_st;j<chunks[i].s_en;j++) {
			for (int k=spans[j].l_st;k<spans[j].l_en;k++) {
				if ( letters[k].no >= 0 ) {
					letters[k].x_st+=lspc[letters[k].no];
					letters[k].x_en+=lspc[letters[k].no];
				}
			}
		}

		free(lspc);
	}
}
void               flow_res::ComputeLetterOffsets(void)
{
	for (int i=0;i<nbChunk;i++) {
		text_holder* th=chunks[i].mommy;
		if ( th == NULL ) continue;
		for (int j=chunks[i].l_st;j<chunks[i].l_en;j++) {
			// fill missing fields
			letters[j].ucs4_offset=th->raw_text.UTF8_2_UCS4(letters[j].utf8_offset);
			// from the internal text used by the text_holder to the one it was given
			int u8p=letters[j].utf8_offset;
			int u4p=letters[j].ucs4_offset;
			partial_text* l_owner=NULL;
			th->flow_to_me.DestToSource(u8p,u4p,letters[j].utf8_offset,letters[j].ucs4_offset,l_owner,false);
			if ( l_owner && l_owner->f_owner ) {
				// and from the text given to the text_holder to the position in the whole layout
				letters[j].utf8_offset+=l_owner->f_owner->utf8_st;
				letters[j].ucs4_offset+=l_owner->f_owner->ucs4_st;
			}
		}
	}
}


