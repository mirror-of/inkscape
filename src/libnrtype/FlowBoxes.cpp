/*
 *  text_holder.cpp
 */

#include "FlowBoxes.h"
#include "FlowStyle.h"
#include "FlowSrc.h"
#include "FlowRes.h"
#include "FlowSols.h"
#include "FlowEater.h"

#include <config.h>
#include <libnrtype/font-instance.h>
#include <libnrtype/FontFactory.h>

#include <pango/pango.h>
#include <glib.h>

#define ntext_holder_verbose

text_holder::text_holder(void):raw_text(),flow_to_me()
{
	raw_text.t_owner=this;
	utf8_length=0;
	utf8_text=NULL;
	ucs4_length=0;
	nbCtrl=maxCtrl=0;
	ctrls=NULL;
	nbBox=maxBox=0;
	boxes=NULL;
	kern_x=kern_y=NULL;
	nb_kern_x=nb_kern_y=0;
	source_start=source_end=NULL;
	paragraph_rtl=false;
}
text_holder::~text_holder(void)
{
	if ( kern_x ) free(kern_x);
	if ( kern_y ) free(kern_y);
	if ( boxes ) free(boxes);
	if ( ctrls ) free(ctrls);
	utf8_length=0;
	utf8_text=NULL;
	nbCtrl=maxCtrl=0;
	ctrls=NULL;
	nbBox=maxBox=0;
	boxes=NULL;
	kern_x=kern_y=NULL;
	nb_kern_x=nb_kern_y=0;
}

void         text_holder::AppendUTF8(partial_text* iTxt)
{
	flow_to_me.AddSource(iTxt);
	DoText();
}
void           text_holder::LastAddition(int &d_utf8_st,int &d_ucs4_st,int &d_utf8_en,int &d_ucs4_en)
{
	if ( flow_to_me.nbSrc <= 0 ) {
		d_utf8_st=d_utf8_en=0;
		d_ucs4_st=d_ucs4_en=0;
		return;
	}
	correspondance::corresp_src* last=flow_to_me.src+(flow_to_me.nbSrc-1);
	flow_to_me.SourceToDest(0, 0, last->txt, d_utf8_st, d_ucs4_st, false);
	flow_to_me.SourceToDest(last->txt->utf8_length, last->txt->ucs4_length, last->txt, d_utf8_en, d_ucs4_en, true);
}
void					 text_holder::DoText(void)
{
	// we should only be passed the 'true' text, so no need to worry with xml-space
	// we can eat soft hyphen at will, if this is in a sp-text, they have been already stripped out
	//nbCtrl=0;
	flow_to_me.SetDestination(&raw_text);
	flow_to_me.StartAdding();
	for (int i=0;i<flow_to_me.nbSrc;i++) {
		char* src_text=flow_to_me.src[i].txt->utf8_text;
		int   src_length=flow_to_me.src[i].txt->utf8_length;
		int   src_offset=flow_to_me.src[i].utf8_offset;
		for (char* p=src_text;p&&*p;p=g_utf8_next_char(p)) {
			int     d=((int)p)-((int)src_text);
			gunichar nc=g_utf8_get_char(p);
			if ( *p == '\n' ) {
				AddCtrl(raw_text.utf8_length,ctrl_return); // nooo, not a newline!
				flow_to_me.TreatChar(src_offset+d,false,i);
			} else if ( *p == '\r' ) {
				AddCtrl(raw_text.utf8_length,ctrl_return); // nooo, not a newline!
				flow_to_me.TreatChar(src_offset+d,false,i);
			} else if ( *p == '\t' ) {			
				AddCtrl(raw_text.utf8_length,ctrl_tab); // nooo, not a tab!
				flow_to_me.TreatChar(src_offset+d,false,i);
			} else if ( nc == 0x00AD ) {
				AddCtrl(raw_text.utf8_length,ctrl_soft_hyph); // soft hyphen-> we can break 'after' it
				flow_to_me.TreatChar(src_offset+d,false,i);
			} else {
				flow_to_me.TreatChar(src_offset+d,true,i);
			}
		}
		flow_to_me.FlushAdding(src_offset+src_length,i);
	}
	flow_to_me.EndAdding();

	utf8_length=raw_text.utf8_length;
	utf8_text=raw_text.utf8_text;
	ucs4_length=raw_text.ucs4_length;
}
void           text_holder::SubCtrl(int no)
{
	if ( no < 0 || no >= nbCtrl ) return;
	int   np=ctrls[no].other;
	if ( np >= 0 ) {
		if ( np < no ) {int swap=no;no=np;np=swap;}
		ctrls[np]=ctrls[--nbCtrl];
		if ( ctrls[np].other >= 0 ) ctrls[ctrls[np].other].other=np;
	}
	ctrls[no]=ctrls[--nbCtrl];
	if ( ctrls[no].other >= 0 ) ctrls[ctrls[no].other].other=no;
}
void           text_holder::AddCtrl(int pos,int typ)
{
	if ( nbCtrl >= maxCtrl ) {
		maxCtrl=2*nbCtrl+1;
		ctrls=(one_ctrl*)realloc(ctrls,maxCtrl*sizeof(one_ctrl));
	}
	ctrls[nbCtrl].pos=pos;
	ctrls[nbCtrl].is_start=false;
	ctrls[nbCtrl].is_end=false;
	ctrls[nbCtrl].other=-1;
	ctrls[nbCtrl].typ=typ;
	ctrls[nbCtrl].ucs4_offset=0;
	nbCtrl++;
}
void           text_holder::AddSpan(int st,int en,int typ)
{
	if ( nbCtrl+1 >= maxCtrl ) {
		maxCtrl=2*nbCtrl+2;
		ctrls=(one_ctrl*)realloc(ctrls,maxCtrl*sizeof(one_ctrl));
	}
	ctrls[nbCtrl].pos=st;
	ctrls[nbCtrl].is_start=true;
	ctrls[nbCtrl].is_end=false;
	ctrls[nbCtrl].other=nbCtrl+1;
	ctrls[nbCtrl].typ=typ;
	ctrls[nbCtrl].ucs4_offset=0;
	nbCtrl++;
	ctrls[nbCtrl].pos=en;
	ctrls[nbCtrl].is_start=false;
	ctrls[nbCtrl].is_end=true;
	ctrls[nbCtrl].other=nbCtrl-1;
	ctrls[nbCtrl].typ=typ;
	ctrls[nbCtrl].ucs4_offset=0;
	nbCtrl++;
}
void           text_holder::AddStyleSpan(int st,int en,text_style* i_s)
{
	if ( nbCtrl+1 >= maxCtrl ) {
		maxCtrl=2*nbCtrl+2;
		ctrls=(one_ctrl*)realloc(ctrls,maxCtrl*sizeof(one_ctrl));
	}
	ctrls[nbCtrl].pos=st;
	ctrls[nbCtrl].is_start=true;
	ctrls[nbCtrl].is_end=false;
	ctrls[nbCtrl].other=nbCtrl+1;
	ctrls[nbCtrl].typ=ctrl_style;
	ctrls[nbCtrl].ucs4_offset=0;
	ctrls[nbCtrl].data.s=i_s;
	nbCtrl++;
	ctrls[nbCtrl].pos=en;
	ctrls[nbCtrl].is_start=false;
	ctrls[nbCtrl].is_end=true;
	ctrls[nbCtrl].other=nbCtrl-1;
	ctrls[nbCtrl].typ=ctrl_style;
	ctrls[nbCtrl].ucs4_offset=0;
	ctrls[nbCtrl].data.s=i_s;
	nbCtrl++;
}
void           text_holder::AddBidiSpan(int st,int en,bool rtl)
{
	if ( nbCtrl+1 >= maxCtrl ) {
		maxCtrl=2*nbCtrl+2;
		ctrls=(one_ctrl*)realloc(ctrls,maxCtrl*sizeof(one_ctrl));
	}
	ctrls[nbCtrl].pos=st;
	ctrls[nbCtrl].is_start=true;
	ctrls[nbCtrl].is_end=false;
	ctrls[nbCtrl].other=nbCtrl+1;
	ctrls[nbCtrl].typ=ctrl_bidi;
	ctrls[nbCtrl].data.i=(rtl)?1:0;
	nbCtrl++;
	ctrls[nbCtrl].pos=en;
	ctrls[nbCtrl].is_start=false;
	ctrls[nbCtrl].is_end=true;
	ctrls[nbCtrl].other=nbCtrl-1;
	ctrls[nbCtrl].typ=ctrl_bidi;
	ctrls[nbCtrl].data.i=(rtl)?1:0;
	nbCtrl++;
}
void           text_holder::AddVoidSpan(int st,int en,int typ,void* i_l)
{
	if ( nbCtrl+1 >= maxCtrl ) {
		maxCtrl=2*nbCtrl+2;
		ctrls=(one_ctrl*)realloc(ctrls,maxCtrl*sizeof(one_ctrl));
	}
	ctrls[nbCtrl].pos=st;
	ctrls[nbCtrl].is_start=true;
	ctrls[nbCtrl].is_end=false;
	ctrls[nbCtrl].other=nbCtrl+1;
	ctrls[nbCtrl].typ=typ;
	ctrls[nbCtrl].data.v=i_l;
	nbCtrl++;
	ctrls[nbCtrl].pos=en;
	ctrls[nbCtrl].is_start=false;
	ctrls[nbCtrl].is_end=true;
	ctrls[nbCtrl].other=nbCtrl-1;
	ctrls[nbCtrl].typ=typ;
	ctrls[nbCtrl].data.v=i_l;
	nbCtrl++;
}
bool           text_holder::NextStop(int typ,int &n_st)
{
	do {
		n_st++;
	} while ( n_st < nbCtrl && ctrls[n_st].typ != typ );
	return ( n_st < nbCtrl );
}
bool           text_holder::NextStart(int typ,int &n_st)
{
	do {
		NextStop(typ,n_st);
	} while ( n_st < nbCtrl && ctrls[n_st].is_start == false );
	return ( n_st < nbCtrl );
}
bool           text_holder::NextEnd(int typ,int &n_st)
{
	do {
		NextStop(typ,n_st);
	} while ( n_st < nbCtrl && ctrls[n_st].is_end == false );
	return ( n_st < nbCtrl );
}
void           text_holder::NextSpanOfTyp(int typ,int &s_st,int &s_en)
{
	if ( NextStart(typ,s_st) ) {
		if ( s_st < nbCtrl ) {
			s_en=ctrls[s_st].other;
		} else {
			s_en=s_st;
		}
	}
}
void           text_holder::NextSpanOfTyp(int typ,int pos,int &s_st,int &s_en)
{
	do {
		NextSpanOfTyp(typ,s_st,s_en);
		if ( s_st < nbCtrl ) {
			if ( s_en >= 0 ) {
				if ( ctrls[s_st].pos <= pos && ctrls[s_en].pos > pos ) break;
			} else {
				s_st=nbCtrl;
			}
		}
	} while ( s_st < nbCtrl );
}
void           text_holder::SplitSpan(int a_t,int b_t)
{
	int   cur_pos=0;
	do {
		int   c_bd=-1;
		while ( NextStop(b_t,c_bd) && ctrls[c_bd].pos < cur_pos ) {}
		if ( c_bd >= nbCtrl ) break;
		cur_pos=ctrls[c_bd].pos;
		
		int   c_st=0,c_en=0;
		int   p_st=0,p_en=0;
		while ( c_st < nbCtrl ) {
			if ( ctrls[c_st].typ == a_t && ctrls[c_st].is_start ) {
				c_en=ctrls[c_st].other;
				if ( c_en < 0 ) {
					c_st=c_en=-1;
					cur_pos=utf8_length;
					break;
				}
				p_st=ctrls[c_st].pos;
				p_en=ctrls[c_en].pos;
				if ( p_st < cur_pos && p_en > cur_pos ) break;
				if ( p_st >= cur_pos ) {
					c_st=c_en=-1;
					break;
				}
			}
			c_st++;
		}
		if ( c_st >= 0 && c_st < nbCtrl && c_en >= 0 && c_en < nbCtrl ) {
			if ( nbCtrl+3 >= maxCtrl ) {
				maxCtrl=2*nbCtrl+4;
				ctrls=(one_ctrl*)realloc(ctrls,maxCtrl*sizeof(one_ctrl));
			}
			one_ctrl  sav=ctrls[c_st];
			SubCtrl(c_st);
			sav.pos=p_st;
			sav.is_start=true;
			sav.is_end=false;
			sav.other=nbCtrl+1;
			ctrls[nbCtrl++]=sav;
			sav.pos=cur_pos;
			sav.is_start=false;
			sav.is_end=true;
			sav.other=nbCtrl-1;
			ctrls[nbCtrl++]=sav;
			sav.pos=cur_pos;
			sav.is_start=true;
			sav.is_end=false;
			sav.other=nbCtrl+1;
			ctrls[nbCtrl++]=sav;
			sav.pos=p_en;
			sav.is_start=false;
			sav.is_end=true;
			sav.other=nbCtrl-1;
			ctrls[nbCtrl++]=sav;
			SortCtrl();
		}
		cur_pos++;
	} while ( cur_pos < utf8_length );
}


int   CmpCtrl(const void* ia,const void* ib)
{
	text_holder::one_ctrl* a=(text_holder::one_ctrl*)ia;
	text_holder::one_ctrl* b=(text_holder::one_ctrl*)ib;
	if ( a->pos < b->pos ) return -1;
	if ( a->pos > b->pos ) return 1;
	if ( a->is_start && b->is_start == false ) return 1;
	if ( b->is_start && a->is_start == false ) return -1;
	if ( a->is_end && b->is_end == false ) return -1;
	if ( b->is_end && a->is_end == false ) return 1;
	return 0;
}
void           text_holder::SortCtrl(void)
{
	if ( nbCtrl <= 0 ) return;
	for (int i=0;i<nbCtrl;i++) ctrls[i].ind=i;
	qsort(ctrls,nbCtrl,sizeof(one_ctrl),CmpCtrl);
	for (int i=0;i<nbCtrl;i++) ctrls[ctrls[i].ind].inv=i;
	for (int i=0;i<nbCtrl;i++) {
		if ( ctrls[i].other >= 0 ) ctrls[i].other=ctrls[ctrls[i].other].inv;
	}
}
void           text_holder::AfficheCtrl(void)
{
	printf("%i ctrls:\n",nbCtrl);
	for (int i=0;i<nbCtrl;i++) {
		printf("  %i: typ=%i pos=%i s=%i e=%i ot=%i\n",i,ctrls[i].typ,ctrls[i].pos,(ctrls[i].is_start)?1:0,(ctrls[i].is_end)?1:0,ctrls[i].other);
	}
	printf("\n");
}

int            text_holder::AddBox(one_flow_box &i_b)
{
	if ( nbBox >= maxBox ) {
		maxBox=2*nbBox+1;
		boxes=(one_flow_box*)realloc(boxes,maxBox*sizeof(one_flow_box));
	}
	boxes[nbBox++]=i_b;
	return nbBox-1;
}
void           text_holder::AfficheBoxes(void)
{
	printf("%i boites\n",nbBox);
	for (int i=0;i<nbBox;i++) {
		printf("%i: [%i -> %i] = [%f %f %f / %f] %i ; n=%i p=%i h=%i l=%i rtl=%i\n",i,boxes[i].st,boxes[i].en,boxes[i].meas.ascent,boxes[i].meas.descent
					 ,boxes[i].meas.leading,boxes[i].meas.width,boxes[i].meas.nb_letter,boxes[i].next,boxes[i].prev,boxes[i].first,boxes[i].link
					 ,(boxes[i].rtl)?1:0);
	}
}

/*
 */
void           text_holder::DoChunking(flow_styles* style_holder)
{
	//DoText();
#ifdef text_holder_verbose
	printf("chunk %s\n",utf8_text);
#endif
	PangoContext*  pContext=(font_factory::Default())->fontContext;
	PangoAttrList* pAttr=pango_attr_list_new();
	GList* pItems=pango_itemize(pContext,utf8_text,0,utf8_length,pAttr,NULL);
	int               cur_bidi=-1;
	int               last_bidi_st=0;
	PangoLanguage*    cur_lang=NULL;
	int               last_lang_st=0;
	PangoEngineShape* cur_shp=NULL;
	int               last_shp_st=0;
	PangoEngineLang*  cur_elg=NULL;
	int               last_elg_st=0;
	for (GList* cur=pItems;cur;cur=cur->next) {
		PangoItem* item=(PangoItem*)cur->data;
		
		int   n_bidi=(item->analysis.level==1)?1:0;
		if ( cur_bidi < 0 ) {
			last_bidi_st=item->offset;
		} else if ( cur_bidi != n_bidi ) {
			AddBidiSpan(last_bidi_st,item->offset,(cur_bidi==1));
			last_bidi_st=item->offset;
		}
		cur_bidi=n_bidi;
		
		PangoLanguage*   n_lang=item->analysis.language;
		if ( cur_lang == NULL ) {
			last_lang_st=item->offset;
		} else if ( cur_lang != n_lang ) {
			AddVoidSpan(last_lang_st,item->offset,ctrl_lang,cur_lang);
			last_lang_st=item->offset;
		}
		cur_lang=n_lang;

		PangoEngineShape*   n_shp=item->analysis.shape_engine;
		if ( cur_shp == NULL ) {
			last_shp_st=item->offset;
		} else if ( cur_shp != n_shp ) {
			AddVoidSpan(last_shp_st,item->offset,ctrl_shp_eng,cur_shp);
			last_shp_st=item->offset;
		}
		cur_shp=n_shp;

		PangoEngineLang*    n_elg=item->analysis.lang_engine;
		if ( cur_elg == NULL ) {
			last_elg_st=item->offset;
		} else if ( cur_elg != n_elg ) {
			AddVoidSpan(last_elg_st,item->offset,ctrl_lang_eng,cur_elg);
			last_elg_st=item->offset;
		}
		cur_elg=n_elg;
	}
	if ( cur_bidi >= 0 && last_bidi_st < utf8_length ) {
		AddBidiSpan(last_bidi_st,utf8_length,(cur_bidi==1));
	}
	if ( cur_lang && last_lang_st < utf8_length ) {
		AddVoidSpan(last_lang_st,utf8_length,ctrl_lang,cur_lang);
	}
	if ( cur_shp && last_shp_st < utf8_length ) {
		AddVoidSpan(last_shp_st,utf8_length,ctrl_shp_eng,cur_shp);
	}
	if ( cur_elg && last_elg_st < utf8_length ) {
		AddVoidSpan(last_elg_st,utf8_length,ctrl_lang_eng,cur_elg);
	}
	
	for (GList* cur=pItems;cur;cur=cur->next) {
		PangoItem* item=(PangoItem*)cur->data;
		pango_item_free(item);
	}
	g_list_free(pItems);
	
	SortCtrl();
	SplitSpan(ctrl_style,ctrl_lang);
	SplitSpan(ctrl_style,ctrl_bidi);

	int    cur_pos=0;
	do {
		int     cur_ctrl=0;
		while ( cur_ctrl < nbCtrl ) {
			if ( ctrls[cur_ctrl].is_start == true && ctrls[cur_ctrl].typ == ctrl_style ) {
				if ( ctrls[cur_ctrl].pos >= cur_pos ) break;
			}
			cur_ctrl++;
		}
		if ( cur_ctrl < nbCtrl ) {
			int         c_st=cur_ctrl,c_en=ctrls[cur_ctrl].other;
			cur_pos=ctrls[c_en].pos;
			text_style* c_style=ctrls[c_st].data.s;
			pango_context_set_font_description(pContext,c_style->theFont->descr);
			pItems=pango_itemize(pContext,utf8_text,ctrls[c_st].pos,ctrls[c_en].pos-ctrls[c_st].pos,pAttr,NULL);
			
			int         old_offset=ctrls[c_st].pos;
			bool        ctrl_changed=false;
			for (GList* cur=pItems;cur;cur=cur->next) {
				PangoItem*            item=(PangoItem*)cur->data;
				PangoFont*            font=item->analysis.font;
				PangoFontDescription* pfd=pango_font_describe(font);
				font_instance*        t_font=(font_factory::Default())->Face(pfd);
				text_style*           n_style=c_style;
				if ( t_font != c_style->theFont ) {
					n_style=new text_style(c_style);
					n_style->SetFont(t_font,c_style->theSize,c_style->baseline_shift);
					style_holder->AddStyle(n_style);
					if ( ctrl_changed == false ) {
						SubCtrl(c_st);
						if ( item->offset > old_offset ) AddStyleSpan(old_offset,item->offset,c_style);
					}
					ctrl_changed=true;
				}
				if ( ctrl_changed ) {
					AddStyleSpan(item->offset,item->offset+item->length,n_style);
				}
				t_font->Unref();
				pango_font_description_free(pfd);
			}
			if ( ctrl_changed ) SortCtrl();
			
			for (GList* cur=pItems;cur;cur=cur->next) {
				PangoItem* item=(PangoItem*)cur->data;
				pango_item_free(item);
			}
			g_list_free(pItems);
		} else {
			break;
		}
	} while ( cur_pos < utf8_length );
	
	pango_attr_list_unref(pAttr);
	
	// word splitting
	
	cur_pos=0;
	do {
		int   c_bd=0;
		while ( c_bd < nbCtrl ) {
			if ( ctrls[c_bd].typ == ctrl_lang && ctrls[c_bd].is_start ) {
				if ( ctrls[c_bd].pos >= cur_pos ) break;
			}
			c_bd++;
		}
		if ( c_bd >= nbCtrl ) break;
		cur_pos=ctrls[c_bd].pos;
		if ( ctrls[c_bd].other < 0 ) break;
		int  l_st=ctrls[c_bd].pos;
		int  l_en=ctrls[ctrls[c_bd].other].pos;
		if ( l_en > l_st ) {
			int						 l_len=l_en-l_st;
			char*          t_st=utf8_text+l_st;
			PangoLogAttr*  lAttrs=(PangoLogAttr*)malloc((l_len+1)*sizeof(PangoLogAttr));
			pango_get_log_attrs(t_st,l_len,-1,(PangoLanguage*)ctrls[c_bd].data.v,lAttrs,l_len+1);
			int    last_word_st=l_st;
			int    last_box_st=l_st;
			int    last_letter_st=l_st;
			char*  p=t_st;
			int    ucs4_i=0;
			for (int i=0;i<=l_len;) {
				if ( i >= l_len || lAttrs[ucs4_i].is_cursor_position ) {
//					if ( last_letter_st < l_st+i ) AddSpan(last_letter_st,l_st+i,ctrl_letter);
					last_letter_st=l_st+i;
				}
				if ( i >= l_len || lAttrs[ucs4_i].is_line_break || lAttrs[ucs4_i].is_white ) {
					if ( last_box_st < l_st+i ) AddSpan(last_box_st,l_st+i,ctrl_box);
					last_box_st=l_st+i;
				}
				if ( i >= l_len || lAttrs[ucs4_i].is_word_start || lAttrs[ucs4_i+1].is_word_end || lAttrs[ucs4_i].is_white ) {
					if ( last_word_st < l_st+i ) AddSpan(last_word_st,l_st+i,ctrl_word);
					last_word_st=l_st+i;
				}
				if ( *p == 0 ) break;
				p=g_utf8_next_char(p);
				i=((int)p)-((int)t_st);
				ucs4_i++;
			}
			free(lAttrs);
		}
		cur_pos++;
	} while ( cur_pos < utf8_length );
	// ya never know
	SortCtrl();
	SplitSpan(ctrl_box,ctrl_bidi);
	SplitSpan(ctrl_box,ctrl_lang);
	// compute ucs4 offsets
	for (int i=0;i<nbCtrl;i++) ctrls[i].ucs4_offset=UCS4Offset(ctrls[i].pos);
}

void           text_holder::UpdatePangoAnalysis(int from,int p_st,int p_en,void *i_pan)
{
	PangoAnalysis* pan=(PangoAnalysis*)i_pan;
	for (int i=from;i<nbCtrl;i++) {
		if ( ctrls[i].pos > p_st && ctrls[i].is_start ) break;
		
		if ( ctrls[i].typ == ctrl_lang && ctrls[i].is_start ) {
			int j=ctrls[i].other;
			if ( ctrls[i].pos <= p_st && ctrls[j].pos >= p_en ) {
				pan->language=(PangoLanguage*)ctrls[i].data.v;
			}
		}
		if ( ctrls[i].typ == ctrl_bidi && ctrls[i].is_start ) {
			int j=ctrls[i].other;
			if ( ctrls[i].pos <= p_st && ctrls[j].pos >= p_en ) {
				pan->level=ctrls[i].data.i;
			}
		}
		if ( ctrls[i].typ == ctrl_shp_eng && ctrls[i].is_start ) {
			int j=ctrls[i].other;
			if ( ctrls[i].pos <= p_st && ctrls[j].pos >= p_en ) {
				pan->shape_engine=(PangoEngineShape*)ctrls[i].data.v;
			}
		}
		if ( ctrls[i].typ == ctrl_lang_eng && ctrls[i].is_start ) {
			int j=ctrls[i].other;
			if ( ctrls[i].pos <= p_st && ctrls[j].pos >= p_en ) {
				pan->lang_engine=(PangoEngineLang*)ctrls[i].data.v;
			}
		}
	}
}
void           text_holder::MeasureText(int p_st,int p_en,box_sizes &n_a,void *i_pan,int b_offset,int with_hyphen)
{
	PangoAnalysis* b_pan=(PangoAnalysis*)i_pan;
	n_a.ascent=n_a.descent=n_a.leading=0;
	n_a.width=0;
	n_a.nb_letter=0;
	int    s_st=-1,s_en=0;
	int    s_p_st=p_st,s_p_en=p_st;
	do {
		NextSpanOfTyp(ctrl_style,s_p_st,s_st,s_en);
		//printf("measure style %i %i =(%i -> %i)\n",s_st,s_en,(s_st>=0&&s_st<nbCtrl)?ctrls[s_st].pos,(s_en>=0&&s_en<nbCtrl)?ctrls[s_en].pos);
		if ( s_st < nbCtrl && ( ctrls[s_st].pos < p_en && ctrls[s_en].pos > p_st ) ) {
			s_p_st=ctrls[s_st].pos;
			s_p_en=ctrls[s_en].pos;
			int   i_p_st=(p_st>s_p_st)?p_st:s_p_st;
			int   i_p_en=(p_en<s_p_en)?p_en:s_p_en;
			if ( i_p_st < i_p_en ) {
				text_style* c_style=ctrls[s_st].data.s;
				box_sizes   a;
				int         offset=(p_st>ctrls[s_st].pos)?b_offset:ctrls[s_st].ucs4_offset;
				c_style->Measure(utf8_text+i_p_st,i_p_en-i_p_st,&a,with_hyphen,b_pan,(kern_x)?kern_x+offset:NULL,(kern_y)?kern_y+offset:NULL);
				n_a.Add(a);
			}
		} else {
			break;
		}
		s_p_st=s_p_en;
	} while ( s_p_st < p_en );
}
void           text_holder::ComputeBoxes(void)
{
	if ( utf8_length <= 0 ) return;
	
	// flatten the kerning for simpler use
	if ( kern_x ) {
		if ( nb_kern_x < ucs4_length ) {
			kern_x=(double*)realloc(kern_x,ucs4_length*sizeof(double));
			for (int i=nb_kern_x;i<ucs4_length;i++) kern_x[i]=0;
			nb_kern_x=ucs4_length;
		}
	}
	if ( kern_y ) {
		if ( nb_kern_y < ucs4_length ) {
			kern_y=(double*)realloc(kern_y,ucs4_length*sizeof(double));
			for (int i=nb_kern_y;i<ucs4_length;i++) kern_y[i]=0;
			nb_kern_y=ucs4_length;
		}
		double   sum=0;
		for (int i=0;i<nb_kern_y;i++) {
			sum+=kern_y[i];
			kern_y[i]=sum;
		}
	}
	
	int		*first_box_at=(int*)malloc((utf8_length+1)*sizeof(int));
	for (int i=0;i<=utf8_length;i++) first_box_at[i]=-1;
	
	SortCtrl();
	int           b_st=-1,b_en=0;   
	PangoAnalysis b_pan;
	b_pan.language=NULL;
	b_pan.level=0;
	b_pan.shape_engine=NULL;
	b_pan.lang_engine=NULL;
	b_pan.extra_attrs=NULL;
	b_pan.font=NULL;
	bool   needed=false;
	do {
		NextSpanOfTyp(ctrl_box,b_st,b_en);
		if ( b_st >= nbCtrl || b_en < 0 ) break;
		int    p_st=ctrls[b_st].pos;
		int    p_en=ctrls[b_en].pos;
		//printf("do box %i %i = (%i -> %i)\n",b_st,b_en,p_st,p_en);
		if ( p_st < p_en ) {
			// collect shaper info
			int    cur_b_st=b_st;
			do {
				int    cur_p_st=ctrls[cur_b_st].pos;
				UpdatePangoAnalysis(0,cur_p_st,p_en,&b_pan);
				// go through the successive styles of this box
				one_flow_box  n_a;
				n_a.st=cur_p_st;
				n_a.en=p_en;
				n_a.white=g_unichar_isspace(g_utf8_get_char(utf8_text+cur_p_st));
				n_a.rtl=(b_pan.level==1);
				if ( nbBox <= 0 ) needed=n_a.rtl;
				n_a.ucs4_offset=ctrls[cur_b_st].ucs4_offset;
				MeasureText(cur_p_st,p_en,n_a.meas,&b_pan,n_a.ucs4_offset,0);
				n_a.hyphenated=false;
				n_a.next=-1;
				n_a.first=-1;
				n_a.link=-1;
				int   c_first=AddBox(n_a);
				int   c_last=-1;
//				first_box_at[cur_p_st]=c_first;
				if ( n_a.rtl == needed ) {
					int   cur_b_en=cur_b_st;
					n_a.hyphenated=true;
					while ( NextStop(ctrl_soft_hyph,cur_b_en) ) {
						int  cur_p_en=ctrls[cur_b_en].pos;
						if ( cur_p_en >= p_en ) break;
						
						n_a.en=cur_p_en;
						MeasureText(cur_p_st,cur_p_en,n_a.meas,&b_pan,n_a.ucs4_offset,1);
						int  c_cur=AddBox(n_a);
						if ( boxes[c_first].first < 0 ) boxes[c_first].first=c_cur;
						if ( c_last >= 0 ) boxes[c_last].link=c_cur;
						c_last=c_cur;
					}
				} else {
					// no hyphenation of counterdirectional text
				}
				if ( n_a.rtl != needed ) break;
				if ( NextStop(ctrl_soft_hyph,cur_b_st) == false ) break; // no more hyphen
			} while ( cur_b_st < nbCtrl && ctrls[cur_b_st].pos < p_en );
		}
	} while ( b_st < nbCtrl );
	// set the appropriate 'next' value
	for (int i=0;i<nbBox;i++) {
		boxes[i].next=nbBox;
		boxes[i].prev=-1;
	}
	for (int i=0;i<nbBox;i++) if ( boxes[i].hyphenated == false ) first_box_at[boxes[i].st]=i;
	for (int i=0;i<nbBox;i++) {
		boxes[i].next=first_box_at[boxes[i].en];
		if ( boxes[i].next >= 0 ) {
			boxes[boxes[i].next].prev=i;
		} else {
			boxes[i].next=nbBox;
		}
	}
	// reorder to get a unique bidi direction (ok since we ensured counterdirectional boxes are not hyphenated)
	paragraph_rtl=false; // ltr prefered
	if ( nbBox > 0 ) {
		paragraph_rtl=boxes[0].rtl;
		bool   needed=paragraph_rtl;
		for (int i=0;i>=0&&i<nbBox;) {
			if ( boxes[i].rtl != needed ) {
				int j=i;
				for (;j>=0&&j<nbBox;j=boxes[j].next) {
					if ( boxes[j].rtl == needed ) break;
				}
				// reverse [i..j-1]
				int    pb=boxes[i].prev;
				int    nb=boxes[j-1].next;
				int   r_s=i,r_e=j-1;
				while ( r_s < r_e ) {
					one_flow_box  swap=boxes[r_s];
					boxes[r_s]=boxes[r_e];
					boxes[r_e]=swap;
					r_s++;
					r_e--;
				}
				for (int k=i;k<j;k++) {
					boxes[k].next=k+1;
					boxes[k].prev=k-1;
				}
				if ( pb >= 0 && pb < nbBox ) boxes[pb].next=i;
				if ( nb >= 0 && nb < nbBox ) boxes[nb].prev=j-1;
				boxes[i].prev=pb;
				boxes[j-1].next=nb;
				i=j;
			} else {
				i=boxes[i].next;
			}
		}
	}
	
	free(first_box_at);
#ifdef text_holder_verbose
	AfficheBoxes();
#endif
}
int            text_holder::UCS4Offset(int pos)
{
	int n_o=0;
	for (char* p=utf8_text;p&&*p;p=g_utf8_next_char(p)) {
		int  d=((int)p)-((int)utf8_text);
		if ( d >= pos ) break;
		n_o++;
	}
	return n_o;
}
bool           text_holder::MetricsAt(int i_from_box,double &ascent,double &descent,double &leading,bool flow_rtl)
{
	if ( nbBox <= 0 ) return false;
	if ( flow_rtl == paragraph_rtl ) {
		int  from_box=i_from_box;
		if ( from_box < 0 ) from_box=0;
		if ( from_box >= nbBox ) from_box=nbBox-1;
		ascent=boxes[from_box].meas.ascent;
		descent=boxes[from_box].meas.descent;
		leading=boxes[from_box].meas.leading;
	} else {
		int  from_box=nbBox-1-i_from_box;
		if ( from_box < 0 ) from_box=0;
		if ( from_box >= nbBox ) from_box=nbBox-1;
		ascent=boxes[from_box].meas.ascent;
		descent=boxes[from_box].meas.descent;
		leading=boxes[from_box].meas.leading;
	}
	return true;
}
bool           text_holder::ComputeSols(int i_from_box,line_solutions* sols,int with_no,bool last_in_para,bool last_in_rgn,bool flow_rtl)
{
	if ( nbBox <= 0 ) return false;
	if ( flow_rtl == paragraph_rtl ) {
		int  from_box=i_from_box;
		if ( from_box < 0 ) from_box=0;
		if ( from_box >= nbBox ) return false;
		for (int i=from_box;i>=0 && i<nbBox;i=boxes[i].next) {
			sols->StartWord();
			int   end_pos=boxes[i].next;
			if ( sols->PushBox(boxes[i].meas,with_no,boxes[i].next,boxes[i].white,(end_pos<nbBox)?false:last_in_para,(end_pos<nbBox)?false:last_in_rgn,true) ) {
				for (int h=boxes[i].first;h>=0&&h<nbBox;h=boxes[h].link) {
					end_pos=boxes[h].next;
					sols->PushBox(boxes[h].meas,with_no,end_pos,boxes[h].white,(end_pos<nbBox)?false:last_in_para,(end_pos<nbBox)?false:last_in_rgn,false);
				}
				return true; // maybe try to hyphenate a bit
			}
			for (int h=boxes[i].first;h>=0&&h<nbBox;h=boxes[h].link) {
				end_pos=boxes[h].next;
				sols->PushBox(boxes[h].meas,with_no,end_pos,boxes[h].white,(end_pos<nbBox)?false:last_in_para,(end_pos<nbBox)?false:last_in_rgn,false);
			}
		}
	} else {
		int  from_box=nbBox-1-i_from_box;
		if ( from_box >= nbBox ) from_box=nbBox-1;
		if ( from_box < 0 ) return false;
		for (int i=from_box;i>=0&&i<nbBox;i=boxes[i].prev) {
			sols->StartWord();
			int   end_pos=boxes[i].prev;
			end_pos=nbBox-1-end_pos;
			if ( sols->PushBox(boxes[i].meas,with_no,end_pos,boxes[i].white,(end_pos<nbBox)?false:last_in_para,(end_pos<nbBox)?false:last_in_rgn,true) ) {
				return true; // maybe try to hyphenate a bit
			}
		}
	}
	return false;
}
void					 text_holder::Feed(int i_st_pos,int i_en_pos,bool flow_rtl,flow_eater* baby)
{
	if ( baby && baby->the_flow ) baby->the_flow->SetSourcePos(0);
	if ( nbBox <= 0 ) return;
	if ( flow_rtl == paragraph_rtl ) {
		int st_pos=i_st_pos;
		int en_pos=i_en_pos;
		if ( st_pos >= nbBox ) return;
		if ( en_pos > nbBox ) en_pos=nbBox;
		PangoAnalysis b_pan;
		b_pan.language=NULL;
		b_pan.level=0;
		b_pan.shape_engine=NULL;
		b_pan.lang_engine=NULL;
		b_pan.extra_attrs=NULL;
		b_pan.font=NULL;
		int        utf8_end=(en_pos<nbBox)?boxes[en_pos].st:utf8_length;
		for (int i=st_pos;i>=0&&i<en_pos&&i<nbBox;i=boxes[i].next) {
			int    p_st=boxes[i].st;
			int    p_en=boxes[i].en;
			//printf("feed box %i (%i -> %i)\n",i,p_st,p_en);
			if ( p_st < p_en ) {
				if ( boxes[i].rtl == flow_rtl ) {
					if ( p_en > utf8_end ) {
						i=boxes[i].first;
						for (;i>=0&&i<nbBox;i=boxes[i].link) {
							if ( flow_rtl == true && boxes[i].st == utf8_end ) break;
							if ( flow_rtl == false && boxes[i].en == utf8_end ) break;
						}
						if ( i < 0 || i >= nbBox ) break;
						p_st=boxes[i].st;
						p_en=boxes[i].en;
					}
				} else {
					// do not hyphenate counterdiractional boxes
				}
				baby->StartWord(boxes[i].rtl,boxes[i].meas.nb_letter,boxes[i].meas.width);
				// collect shaper info
				UpdatePangoAnalysis(0,p_st,p_en,&b_pan);
				// go through the successive styles of this box
				int    s_st=-1,s_en=0;
				int    s_p_st=p_st,s_p_en=p_st;
				do {
					NextSpanOfTyp(ctrl_style,s_p_st,s_st,s_en);
					//printf("span style %i %i =(%i -> %i)\n",s_st,s_en,(s_st>=0&&s_st<nbCtrl)?ctrls[s_st].pos:-1,(s_en>=0&&s_en<nbCtrl)?ctrls[s_en].pos:-1);
					if ( s_st < nbCtrl && ( ctrls[s_st].pos < p_en && ctrls[s_en].pos > p_st ) ) {
						s_p_st=ctrls[s_st].pos;
						s_p_en=ctrls[s_en].pos;
						int   i_p_st=(p_st>s_p_st)?p_st:s_p_st;
						int   i_p_en=(p_en<s_p_en)?p_en:s_p_en;
						if ( i_p_st < i_p_en ) {
							text_style* c_style=ctrls[s_st].data.s;
							int         offset=(boxes[i].st>ctrls[s_st].pos)?boxes[i].ucs4_offset:ctrls[s_st].ucs4_offset;
							if ( baby->the_flow ) baby->the_flow->SetSourcePos(i_p_st);
							c_style->Feed(utf8_text+i_p_st,i_p_en-i_p_st,(boxes[i].hyphenated)?1:0,&b_pan,baby,(kern_x)?kern_x+offset:NULL,(kern_y)?kern_y+offset:NULL);
						}
					} else {
						break;
					}
					s_p_st=s_p_en;
				} while ( s_p_st < p_en );
				if ( baby->the_flow ) baby->the_flow->EndWord();
			}
		}
	} else {
		// no hyphenation in this backward case
		int st_pos=nbBox-1-i_st_pos;
		int en_pos=nbBox-1-i_en_pos;
		if ( st_pos >= nbBox ) st_pos=nbBox-1;
		if ( st_pos < 0 ) return;
		if ( en_pos >= nbBox ) return;
		PangoAnalysis b_pan;
		b_pan.language=NULL;
		b_pan.level=0;
		b_pan.shape_engine=NULL;
		b_pan.lang_engine=NULL;
		b_pan.extra_attrs=NULL;
		b_pan.font=NULL;
		for (int i=st_pos;i>=0&&i>en_pos&&i<nbBox;i=boxes[i].prev) {
			int    p_st=boxes[i].st;
			int    p_en=boxes[i].en;
			if ( p_st < p_en ) {
				baby->StartWord(boxes[i].rtl,boxes[i].meas.nb_letter,boxes[i].meas.width);
				// collect shaper info
				UpdatePangoAnalysis(0,p_st,p_en,&b_pan);
				// go through the successive styles of this box
				int    s_st=-1,s_en=0;
				int    s_p_st=p_st,s_p_en=p_st;
				do {
					NextSpanOfTyp(ctrl_style,s_p_st,s_st,s_en);
					if ( s_st < nbCtrl && ( ctrls[s_st].pos < p_en && ctrls[s_en].pos > p_st ) ) {
						s_p_st=ctrls[s_st].pos;
						s_p_en=ctrls[s_en].pos;
						int   i_p_st=(p_st>s_p_st)?p_st:s_p_st;
						int   i_p_en=(p_en<s_p_en)?p_en:s_p_en;
						if ( i_p_st < i_p_en ) {
							text_style* c_style=ctrls[s_st].data.s;
							int         offset=(boxes[i].st>ctrls[s_st].pos)?boxes[i].ucs4_offset:ctrls[s_st].ucs4_offset;
							if ( baby->the_flow ) baby->the_flow->SetSourcePos(i_p_st);
							c_style->Feed(utf8_text+i_p_st,i_p_en-i_p_st,(boxes[i].hyphenated)?1:0,&b_pan,baby,(kern_x)?kern_x+offset:NULL,(kern_y)?kern_y+offset:NULL);
						}
					} else {
						break;
					}
					s_p_st=s_p_en;
				} while ( s_p_st < p_en );
				if ( baby->the_flow ) baby->the_flow->EndWord();
			}
		}
	}
}

void             text_holder::AddKerning(double* i_kern,int i_st,int i_en,bool is_x)
{
	if ( i_st >= i_en ) return;
	if ( is_x ) {
		if ( i_en > nb_kern_x ) {
			kern_x=(double*)realloc(kern_x,i_en*sizeof(double));
			for (int i=nb_kern_x;i<i_en;i++) kern_x[i]=0;
			nb_kern_x=i_en;
		}
	} else {
		if ( i_en > nb_kern_y ) {
			kern_y=(double*)realloc(kern_y,i_en*sizeof(double));
			for (int i=nb_kern_y;i<i_en;i++) kern_y[i]=0;
			nb_kern_y=i_en;
		}
	}
	if ( is_x ) {
		for (int i=i_st;i<i_en;i++) {
			kern_x[i]+=i_kern[i-i_st];
			i_kern[i-i_st]=0;
		}
	} else {
		for (int i=i_st;i<i_en;i++) {
			kern_y[i]+=i_kern[i-i_st];
			i_kern[i-i_st]=0;
		}
	}
}

