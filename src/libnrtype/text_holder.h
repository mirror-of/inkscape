/*
 *  text_holder.h
 */

#ifndef my_text_holder
#define my_text_holder

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "FlowDefs.h"

#include <pango/pango.h>

class text_style;
class flow_styles;
class line_solutions;
class flow_eater;

enum {
	ctrl_none      = 0,
	ctrl_return    = 1,
	ctrl_tab       = 2,
	ctrl_style     = 3,
	ctrl_bidi      = 4,
	ctrl_lang      = 5,
	ctrl_letter    = 6,
	ctrl_word      = 7,
	ctrl_box       = 8,
	ctrl_shp_eng   = 9,
	ctrl_lang_eng  = 10,
	ctrl_soft_hyph = 11
};

class text_holder {
public:
	// source text
	int            utf8_length;
	char*          utf8_text;
	int            ucs4_length;
	// list of non-text stuff (control chars)
	typedef struct one_ctrl {
		int          pos;
		bool         is_start;
		bool         is_end;
		int					 other;
		int          typ;
		int          ind,inv;
		int          ucs4_offset;
		union {
			int          i;
			text_style*  s;
			void*				 v;
		} data;
	} one_ctrl;
	int            nbCtrl,maxCtrl;
	one_ctrl*			 ctrls;
	// resulting boxes for the flow
	typedef struct one_flow_box {
		int          st,en,ucs4_offset;
		bool         rtl;
		bool         white;
		bool         hyphenated;
		box_sizes    meas;
		int          next,prev;
		int					 first,link;
	} one_flow_box;
	int            nbBox,maxBox;
	one_flow_box*  boxes;
	
	int						 nb_kern_x,nb_kern_y;
	double				 *kern_x,*kern_y;	

	text_holder(void);
	~text_holder(void);
	
	int            AppendUTF8(char* iText,int iLen,int &rLen,bool preserve=false);
	void           DoChunking(flow_styles* style_holder);
	void           ComputeBoxes(void);
	bool           MetricsAt(int from_box,double &ascent,double &descent,double &leading,bool flow_rtl);
	bool           ComputeSols(int from_box,line_solutions* sols,int with_no,bool last_in_para,bool last_in_rgn,bool flow_rtl);
	void					 Feed(int st_pos,int en_pos,bool flow_rtl,flow_eater* baby);
	
	void           AddCtrl(int pos,int typ);
	void           AddSpan(int st,int en,int typ);
	void           AddStyleSpan(int st,int en,text_style* i_s);
	void           AddBidiSpan(int st,int en,bool rtl);
	void           AddVoidSpan(int st,int en,int typ,void* i_l);
	void           SplitSpan(int a_t,int b_t);
	void           SubCtrl(int no);

	void           SortCtrl(void);
	void           AfficheCtrl(void);
	
	int            AddBox(one_flow_box &i_b);
	
	void           AfficheBoxes(void);
	
	// utility ctrl position functions
	bool           NextStop(int typ,int &n_st);
	bool           NextStart(int typ,int &n_st);
	bool           NextEnd(int typ,int &n_st);

	void           AddKerning(double* i_kern,int i_st,int i_en,bool is_x);
	int            UCS4Offset(int pos);
	void           NextSpanOfTyp(int typ,int &s_st,int &s_en);
	void           NextSpanOfTyp(int typ,int after,int &s_st,int &s_en);

	void           UpdatePangoAnalysis(int from,int p_st,int p_en,PangoAnalysis &pan);
	void           MeasureText(int p_st,int p_en,box_sizes &a,PangoAnalysis &pan,int b_offset,int with_hyphen);
};


#endif
