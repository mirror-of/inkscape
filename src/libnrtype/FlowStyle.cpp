/*
 *  text_style.cpp
 */

#include "FlowStyle.h"
#include "FlowEater.h"
#include "FlowRes.h"
#include "FlowSols.h"

#include "../style.h"

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
	vertical_layout=false;
}
text_style::text_style(text_style* modele)
{
	theFont=NULL;
	theSize=modele->theSize;
	vertical_layout=modele->vertical_layout;
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

void             text_style::Measure(char* iText,int iLen,box_sizes *sizes,int hyphen,void *i_pan,double *kern_x,double *kern_y)
{
	PangoAnalysis* pan=(PangoAnalysis*)i_pan;
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
	
	pan->font=theFont->pFont;
	pango_shape(iText,uLen,pan,pGlyphs);
	sizes->width=0;
	sizes->nb_letter=0;
	double pango_scale=1.0/((double)PANGO_SCALE);
	if ( vertical_layout == false ) {
		for (int i=0;i<pGlyphs->num_glyphs;i++) {
			sizes->width+=pango_scale*((double)pGlyphs->glyphs[i].geometry.width);
			if ( pGlyphs->glyphs[i].attr.is_cluster_start ) sizes->nb_letter++;
		}
		sizes->width/=512;
		sizes->width*=theSize;
		theFont->FontMetrics(sizes->ascent,sizes->descent,sizes->leading);
		sizes->ascent*=theSize;
		sizes->descent*=theSize;
		sizes->leading*=theSize;
	} else {
		double n_let_w=0;
		double n_let_a=0,n_let_d=0,n_let_l=0;
		theFont->FontMetrics(n_let_a,n_let_d,n_let_l);
		for (int i=0;i<pGlyphs->num_glyphs;i++) {
			if ( pGlyphs->glyphs[i].attr.is_cluster_start ) {
				sizes->nb_letter++;
				if ( n_let_w > sizes->ascent ) sizes->ascent=n_let_w;
				n_let_w=0;
			}
			n_let_w+=pango_scale*((double)pGlyphs->glyphs[i].geometry.width);
		}
		if ( n_let_w > sizes->ascent ) sizes->ascent=n_let_w;
		sizes->width=theSize*(n_let_a+n_let_d+n_let_l)*((double)sizes->nb_letter);
		sizes->ascent/=512;
		sizes->ascent*=theSize;
	}
	sizes->ascent-=baseline_shift;
	sizes->descent+=baseline_shift;
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
void             text_style::Feed(char* iText,int iLen,int hyphen,void *i_pan,flow_eater* baby,double *kern_x,double *kern_y)
{
	PangoAnalysis* pan=(PangoAnalysis*)i_pan;
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
	
	pan->font=theFont->pFont;
	pango_shape(iText,uLen,pan,pGlyphs);
	double pango_scale=1.0/((double)PANGO_SCALE);
	if ( vertical_layout == false ) {
		if ( pan->level ) {
			int kx_st=0;
			int kx_pos=0;
			for (int i=pGlyphs->num_glyphs-1;i>=0;i--) {
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
				int  g_en=(i>0)?pGlyphs->log_clusters[i-1]:uLen;
				if ( i >= pGlyphs->num_glyphs-1 || pGlyphs->glyphs[i].attr.is_cluster_start ) {
					double a_k_x=0,a_k_y=0;
					if ( kern_x ) {
						for (char* p=iText+kx_st;*p;p=g_utf8_next_char(p)) {
							int d=((int)p)-((int)iText);
							if ( d >= g_en ) break;
							a_k_x+=kern_x[kx_pos];
							kx_pos++;
						}
						kx_st=g_en;
					}
					if ( kern_y ) {
						//int gLen=pGlyphs->log_clusters[i];
						int k_pos=0;
						for (char* p=iText;*p;p=g_utf8_next_char(p)) {
							int d=((int)p)-((int)iText);
							a_k_y=kern_y[k_pos];
							if ( d >= g_st ) break;
							k_pos++;
						}
					}				
					baby->StartLetter(this,a_k_x,a_k_y,g_st);
				}
				baby->Eat(pGlyphs->glyphs[i].glyph,x,y,w,iText+g_st,g_en-g_st);
			}
		} else {
			int kx_st=0;
			int kx_pos=0;
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
				int  g_en=(i<pGlyphs->num_glyphs-1)?pGlyphs->log_clusters[i+1]:uLen;
				if ( i <= 0 || pGlyphs->glyphs[i].attr.is_cluster_start ) {
					double a_k_x=0,a_k_y=0;
					if ( kern_x ) {
						for (char* p=iText+kx_st;*p;p=g_utf8_next_char(p)) {
							int d=((int)p)-((int)iText);
							if ( d >= g_en ) break;
							a_k_x+=kern_x[kx_pos];
							kx_pos++;
						}
						kx_st=g_en;
					}
					if ( kern_y ) {
						int k_pos=0;
						for (char* p=iText;*p;p=g_utf8_next_char(p)) {
							int d=((int)p)-((int)iText);
							a_k_y=kern_y[k_pos];
							if ( d >= g_st ) break;
							k_pos++;
						}
					}
					baby->StartLetter(this,a_k_x,a_k_y,g_st);
				}
				baby->Eat(pGlyphs->glyphs[i].glyph,x,y,w,iText+g_st,g_en-g_st);
			}
		}
	} else {
		//double n_let_w=0;
		double n_let_a=0,n_let_d=0,n_let_l=0;
		theFont->FontMetrics(n_let_a,n_let_d,n_let_l);
		n_let_a*=theSize;
		n_let_d*=theSize;
		n_let_l*=theSize;
		if ( pan->level ) {
			int kx_st=0;
			int kx_pos=0;
			for (int i=pGlyphs->num_glyphs-1;i>=0;i--) {
				double x=pango_scale*((double)pGlyphs->glyphs[i].geometry.x_offset);
				double y=pango_scale*((double)pGlyphs->glyphs[i].geometry.y_offset);
				double w=pango_scale*((double)pGlyphs->glyphs[i].geometry.width);
				w/=512;
				w*=theSize;
				x/=512;
				x*=theSize;
				y/=512;
				y*=theSize;
				x+=baseline_shift;
				int  g_st=pGlyphs->log_clusters[i];
				int  g_en=(i>0)?pGlyphs->log_clusters[i-1]:uLen;
				double pretend_width=0;
				if ( i >= pGlyphs->num_glyphs-1 || pGlyphs->glyphs[i].attr.is_cluster_start ) {
					double a_k_x=0,a_k_y=0;
					if ( kern_x ) {
						for (char* p=iText+kx_st;*p;p=g_utf8_next_char(p)) {
							int d=((int)p)-((int)iText);
							if ( d >= g_en ) break;
							a_k_x+=kern_x[kx_pos];
							kx_pos++;
						}
						kx_st=g_en;
					}
					if ( kern_y ) {
						//int gLen=pGlyphs->log_clusters[i];
						int k_pos=0;
						for (char* p=iText;*p;p=g_utf8_next_char(p)) {
							int d=((int)p)-((int)iText);
							a_k_y=kern_y[k_pos];
							if ( d >= g_st ) break;
							k_pos++;
						}
					}				
					pretend_width=n_let_a+n_let_d+n_let_l;
					baby->StartLetter(this,a_k_x,a_k_y,g_st);
				}
				baby->Eat(pGlyphs->glyphs[i].glyph,-n_let_d+y,x,pretend_width,iText+g_st,g_en-g_st);
			}
		} else {
			int kx_st=0;
			int kx_pos=0;
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
				x+=baseline_shift;
				int  g_st=pGlyphs->log_clusters[i];
				int  g_en=(i<pGlyphs->num_glyphs-1)?pGlyphs->log_clusters[i+1]:uLen;
				double pretend_width=0;
				if ( i <= 0 || pGlyphs->glyphs[i].attr.is_cluster_start ) {
					double a_k_x=0,a_k_y=0;
					if ( kern_x ) {
						for (char* p=iText+kx_st;*p;p=g_utf8_next_char(p)) {
							int d=((int)p)-((int)iText);
							if ( d >= g_en ) break;
							a_k_x+=kern_x[kx_pos];
							kx_pos++;
						}
						kx_st=g_en;
					}
					if ( kern_y ) {
						int k_pos=0;
						for (char* p=iText;*p;p=g_utf8_next_char(p)) {
							int d=((int)p)-((int)iText);
							a_k_y=kern_y[k_pos];
							if ( d >= g_st ) break;
							k_pos++;
						}
					}
					pretend_width=n_let_a+n_let_d+n_let_l;
					baby->StartLetter(this,a_k_x,a_k_y,g_st);
				}
				baby->Eat(pGlyphs->glyphs[i].glyph,-n_let_d+y,x,pretend_width,iText+g_st,g_en-g_st);
			}
		}
	}
	
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

