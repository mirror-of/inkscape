/*
 *  text_style.h
 */

#ifndef my_flow_style
#define my_flow_style

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//#include <pango/pango.h>

#include "FlowDefs.h"

class font_instance;
class flow_eater;
//struct PangoAnalysis;

class text_style {
public:
	bool             vertical_layout;
	font_instance*   theFont;
	double           theSize;
	double           baseline_shift;
	struct SPStyle*  with_style;
	
	text_style(void);
	text_style(text_style* modele);
	~text_style(void);
	
	void             SetStyle(struct SPStyle* i_style);
	void             SetFont(font_instance* iFont,double iSize,double iShift);
	
	void             Measure(char* iText,int iLen,box_sizes *sizes,int hyphen,void *pan,double *kern_x,double *kern_y);
	void             Feed(char* iText,int iLen,int hyphen,void *pan,flow_eater* baby,double *kern_x,double *kern_y);
};

class flow_styles {
public:
	int              nbStyle,maxStyle;
	text_style*      *styles;
	
	flow_styles(void);
	~flow_styles(void);
	
	void             AddStyle(text_style* who);
};

#endif


