/*
 *  text_style.cpp
 */

#include "text_style.h"
#include "FlowEater.h"

#include "../style.h"
#include "../sp-flowtext.h"

#include <config.h>
#include <libnrtype/font-instance.h>
#include <libnrtype/font-style-to-pos.h>
#include <libnrtype/FontFactory.h>

#include <pango/pango.h>
#include <glib.h>

text_style::text_style(void)
{
	theFont=NULL;
	theSize=0.0;
	baseline_shift=0.0;
	with_style=NULL;
}
text_style::text_style(text_style* modele)
{
	theFont=NULL;
	theSize=modele->theSize;
	baseline_shift=modele->baseline_shift;
	with_style=modele->with_style;
	if ( with_style ) sp_style_ref(with_style);
}
text_style::~text_style(void)
{
	if ( theFont ) theFont->Unref();
	theFont=NULL;
	theSize=0.0;
	baseline_shift=0.0;
	if ( with_style ) sp_style_unref(with_style);
	with_style=NULL;
}

void             text_style::SetStyle(SPStyle* i_style)
{
	if ( with_style ) sp_style_unref(with_style);
	with_style=i_style;
	if ( with_style ) {
		sp_style_ref(with_style);
		const double     c_size = with_style->font_size.computed;
		font_instance*   c_face = (font_factory::Default())->Face(with_style->text->font_family.value, font_style_to_pos(*with_style));
//		printf("n style %s at %f\n",i_style->text->font_family.value,i_style->font_size.computed);
		if ( c_face ) {
			SetFont(c_face,c_size,0.0);
			c_face->Unref();
		}
	}
}
void             text_style::SetFont(font_instance* iFont,double iSize,double iShift)
{
	if ( theFont ) theFont->Unref();
	theFont=iFont;
	if ( theFont ) theFont->Ref();
	theSize=iSize;
	baseline_shift=iShift;
}

void             text_style::Measure(char* iText,int iLen,box_sizes *sizes,int hyphen,PangoAnalysis pan,double *kern_x,double *kern_y)
{
	sizes->ascent=sizes->descent=sizes->leading=0.0;
	sizes->width=0.0;
	sizes->nb_letter=0;
	if ( iLen < 0 ) iLen=strlen(iText);
	if ( iLen <= 0 ) return;
	int    uLen=iLen;
	char  savC=iText[iLen];
	if ( hyphen ) {
		iText[iLen]='-';
		uLen++;
	}
	
	PangoContext*     pContext=(font_factory::Default())->fontContext;
	pango_context_set_font_description(pContext,theFont->descr);
	PangoGlyphString* pGlyphs=pango_glyph_string_new();
	
	pan.font=theFont->pFont;
	pango_shape(iText,uLen,&pan,pGlyphs);
	sizes->width=0;
	sizes->nb_letter=0;
	double pango_scale=1.0/((double)PANGO_SCALE);
	for (int i=0;i<pGlyphs->num_glyphs;i++) {
		sizes->width+=pango_scale*((double)pGlyphs->glyphs[i].geometry.width);
		if ( pGlyphs->glyphs[i].attr.is_cluster_start ) sizes->nb_letter++;
	}
	sizes->width/=512;
	sizes->width*=theSize;
	if ( kern_x ) {
		double   sum=0;
		int      k_pos=0;
		for (char* p=iText;*p;p=g_utf8_next_char(p)) {
			int d=((int)p)-((int)iText);
			if ( d >= iLen ) break;
			sum+=kern_x[k_pos];
			k_pos++;
		}
		sizes->width+=sum;
	}
	theFont->FontMetrics(sizes->ascent,sizes->descent,sizes->leading);
	sizes->ascent*=theSize;
	sizes->descent*=theSize;
	sizes->leading*=theSize;
	sizes->ascent-=baseline_shift;
	sizes->descent+=baseline_shift;
	if ( kern_y ) {
		double   min_y=0,max_y=0;
		int      k_pos=0;
		for (char* p=iText;*p;p=g_utf8_next_char(p)) {
			int d=((int)p)-((int)iText);
			if ( d >= iLen ) break;
			if ( kern_y[k_pos] < min_y ) min_y=kern_y[k_pos];
			if ( kern_y[k_pos] > max_y ) max_y=kern_y[k_pos];
			k_pos++;
		}
		sizes->ascent+=-min_y;
		sizes->descent+=max_y;
	}
	
	pango_glyph_string_free(pGlyphs);
	
	if ( hyphen ) iText[iLen]=savC;
}
void             text_style::Feed(char* iText,int iLen,int hyphen,PangoAnalysis pan,flow_eater* baby,double *kern_x,double *kern_y)
{
	if ( iLen < 0 ) iLen=strlen(iText);
	if ( iLen <= 0 ) return;
	int    uLen=iLen;
	char  savC=iText[iLen];
	if ( hyphen ) {
		iText[iLen]='-';
		uLen++;
	}
	
	//printf("fed %i chars\n",iLen);
	
	PangoContext*     pContext=(font_factory::Default())->fontContext;
	pango_context_set_font_description(pContext,theFont->descr);
	PangoGlyphString* pGlyphs=pango_glyph_string_new();
	
	pan.font=theFont->pFont;
	pango_shape(iText,uLen,&pan,pGlyphs);
	double pango_scale=1.0/((double)PANGO_SCALE);
	for (int i=0;i<pGlyphs->num_glyphs;i++) {
		double x=pango_scale*((double)pGlyphs->glyphs[i].geometry.x_offset);
		double y=pango_scale*((double)pGlyphs->glyphs[i].geometry.y_offset);
		double w=pango_scale*((double)pGlyphs->glyphs[i].geometry.width);
		w/=512;
		w*=theSize;
		x/=512;
		x*=theSize;
		y/=512;
		y*=theSize;
		y+=baseline_shift;
		int  g_st=pGlyphs->log_clusters[i];
		int  g_en=g_st;
		if ( pan.level ) {
			g_en=(i>0)?pGlyphs->log_clusters[i-1]:pGlyphs->num_glyphs;
		} else {
			g_en=(i<pGlyphs->num_glyphs-1)?pGlyphs->log_clusters[i+1]:pGlyphs->num_glyphs;
		}
		if ( kern_x ) {
			int l_kx_st=(i>0)?pGlyphs->log_clusters[i-1]:0;
			int kx_st=pGlyphs->log_clusters[i];
			if ( l_kx_st < kx_st ) {
				int l_kx_pos=0;
				for (char* p=iText;*p;p=g_utf8_next_char(p)) {
					int d=((int)p)-((int)iText);
					if ( d >= l_kx_st ) break;
					l_kx_pos++;
				}
				int kx_pos=l_kx_pos;
				for (char* p=iText+l_kx_st;*p;p=g_utf8_next_char(p)) {
					int d=((int)p)-((int)iText);
					w+=kern_x[kx_pos];
					if ( d >= kx_st ) break;
					kx_pos++;
				}
			} else if ( l_kx_st > kx_st ) {
				int l_kx_pos=0;
				for (char* p=iText;*p;p=g_utf8_next_char(p)) {
					int d=((int)p)-((int)iText);
					if ( d >= kx_st ) break;
					l_kx_pos++;
				}
				int kx_pos=l_kx_pos;
				for (char* p=iText+kx_st;*p;p=g_utf8_next_char(p)) {
					int d=((int)p)-((int)iText);
					w+=kern_x[kx_pos];
					if ( d >= l_kx_st ) break;
					kx_pos++;
				}
			}
		}
		if ( kern_y ) {
			int gLen=pGlyphs->log_clusters[i];
			int k_pos=0;
			double last_k=0;
			for (char* p=iText;*p;p=g_utf8_next_char(p)) {
				int d=((int)p)-((int)iText);
				last_k=kern_y[k_pos];
				if ( d >= gLen ) break;
				k_pos++;
			}
			y+=last_k;
		}
		if ( pGlyphs->glyphs[i].attr.is_cluster_start ) baby->StartLetter();
		baby->Eat(pGlyphs->glyphs[i].glyph,this,x,y,w);
		if ( baby->the_flow ) baby->the_flow->SetLastText(iText+g_st,g_en-g_st);
		
	}
		
	pango_glyph_string_free(pGlyphs);
	
	if ( hyphen ) iText[iLen]=savC;
}
void             text_style::Construct(char* iText,int iLen,int hyphen,PangoAnalysis pan,flow_eater* baby,double *kern_x,double *kern_y)
{
	if ( iLen < 0 ) iLen=strlen(iText);
	if ( iLen <= 0 ) return;
	int    uLen=iLen;
	char  savC=iText[iLen];
	if ( hyphen ) {
		iText[iLen]='-';
		uLen++;
	}
	
	PangoContext*     pContext=(font_factory::Default())->fontContext;
	pango_context_set_font_description(pContext,theFont->descr);
	PangoGlyphString* pGlyphs=pango_glyph_string_new();
	
	pan.font=theFont->pFont;
	pango_shape(iText,uLen,&pan,pGlyphs);
	double pango_scale=1.0/((double)PANGO_SCALE);
	double  tot_width=0;
	int     tot_letter=0;
	for (int i=0;i<pGlyphs->num_glyphs;i++) {
		double x=pango_scale*((double)pGlyphs->glyphs[i].geometry.x_offset);
		double y=pango_scale*((double)pGlyphs->glyphs[i].geometry.y_offset);
		double w=pango_scale*((double)pGlyphs->glyphs[i].geometry.width);
		w/=512;
		w*=theSize;
		x/=512;
		x*=theSize;
		y/=512;
		y*=theSize;
		y+=baseline_shift;
		int  g_st=pGlyphs->log_clusters[i];
		int  g_en=g_st;
		if ( pan.level ) {
			g_en=(i>0)?pGlyphs->log_clusters[i-1]:pGlyphs->num_glyphs;
		} else {
			g_en=(i<pGlyphs->num_glyphs-1)?pGlyphs->log_clusters[i+1]:pGlyphs->num_glyphs;
		}
		if ( kern_x ) {
			int l_kx_st=(i>0)?pGlyphs->log_clusters[i-1]:0;
			int kx_st=pGlyphs->log_clusters[i];
			if ( l_kx_st < kx_st ) {
				int l_kx_pos=0;
				for (char* p=iText;*p;p=g_utf8_next_char(p)) {
					int d=((int)p)-((int)iText);
					if ( d >= l_kx_st ) break;
					l_kx_pos++;
				}
				int kx_pos=l_kx_pos;
				for (char* p=iText+l_kx_st;*p;p=g_utf8_next_char(p)) {
					int d=((int)p)-((int)iText);
					w+=kern_x[kx_pos];
					if ( d >= kx_st ) break;
					kx_pos++;
				}
			} else if ( l_kx_st > kx_st ) {
				int l_kx_pos=0;
				for (char* p=iText;*p;p=g_utf8_next_char(p)) {
					int d=((int)p)-((int)iText);
					if ( d >= kx_st ) break;
					l_kx_pos++;
				}
				int kx_pos=l_kx_pos;
				for (char* p=iText+kx_st;*p;p=g_utf8_next_char(p)) {
					int d=((int)p)-((int)iText);
					w+=kern_x[kx_pos];
					if ( d >= l_kx_st ) break;
					kx_pos++;
				}
			}
		}
		tot_width+=w;
		if ( pGlyphs->glyphs[i].attr.is_cluster_start ) tot_letter++;
	}
	baby->Eat(iText,uLen,tot_width,tot_letter,this,NULL,NULL,0);
		
	pango_glyph_string_free(pGlyphs);
	
	if ( hyphen ) iText[iLen]=savC;
}

/*
 *
 */

flow_styles::flow_styles(void)
{
	nbStyle=maxStyle=0;
	styles=NULL;
}
flow_styles::~flow_styles(void)
{
	for (int i=0;i<nbStyle;i++) delete styles[i];
	if ( styles ) free(styles);
	nbStyle=maxStyle=0;
	styles=NULL;
}

void             flow_styles::AddStyle(text_style* who)
{
	if ( nbStyle >= maxStyle ) {
		maxStyle=2*nbStyle+1;
		styles=(text_style**)realloc(styles,maxStyle*sizeof(text_style*));
	}
	styles[nbStyle]=who;
	nbStyle++;
}

