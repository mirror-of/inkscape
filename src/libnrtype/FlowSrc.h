/*
 *  FlowSrc.h
 */

#ifndef my_flow_src
#define my_flow_src

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "FlowDefs.h"

class text_holder;
class flow_styles;
class text_style;
class flow_eater;

enum {
	flw_none     = 0,
	flw_text     = 1,
	flw_line_brk = 2,
	flw_rgn_brk  = 3
};

class line_solutions {
public:
	int            elem_st,pos_st;
	
	typedef struct one_sol {
		int          elem_no;
		int          pos;
		box_sizes    meas;
		bool         para_end;
		bool         rgn_end;
	} one_sol;
	int						 nbSol,maxSol;
	one_sol*       sols;
	
	one_sol				 style_end_sol;
	bool           style_end_set;
	int						 style_end_no,style_end_pos;
	double         style_end_ascent,style_end_descent,style_end_leading;
	bool           style_ending,no_style_ending;
	
	double         min_length,max_length,typ_length;
	bool           noBefore,noAfter;
	double         l_ascent,l_descent,l_leading;
	
	box_sizes			 cur_line,last_line,last_word;
	int            before_state,after_state;
	bool           in_leading_white;
	int            min_line_no,min_line_pos;
	
	
	line_solutions(void);
	~line_solutions(void);
	
	void            NewLine(double min,double max,double typ,bool strict_bef,bool strict_aft);
	void						SetLineSizes(double ascent,double descent,double leading);
	void            StartLine(int at_elem,int at_pos);
	void            EndLine(void);
	
	void            StartWord(void);
	bool						PushBox(box_sizes &s,int end_no,int end_pos,bool is_white,bool last_in_para,bool last_in_rgn,bool is_word);
	void            ForceSol(int end_no,int end_pos,bool last_in_para,bool last_in_rgn);
	
	void            Affiche(void);
};

class flow_src {
public:
	typedef struct one_elem {
		int               type;
		text_holder*      text;
	} one_elem;
	int                 nbElem,maxElem;
	one_elem*           elems;
	
	typedef struct stack_elem {
		int               elem;
		int               ucs4_st,ucs4_en;
		text_style*       style;
		int               nb_kern_x,nb_kern_y;
		double            *kern_x,*kern_y;
	} stack_elem;
	flow_styles*        styles;
	int                 nbStack,maxStack;
	stack_elem*         stacks;
	
	flow_src(void);
	~flow_src(void);
	
	void                Push(text_style* i_style);
	void                SetKern(double *i_kern,int i_nb,bool is_x);
	void                Pop(void);
	void                AddUTF8(char* iText,int iLen,bool force=false);
	void                AddControl(int type);
	
	void                Clean(int &no,int &pos);
	
	void                Prepare(void);
	
	void                MetricsAt(int from_no,int from_pos,double &ascent,double &descent,double &leading,bool &flow_rtl);
	void                ComputeSol(int from_no,int from_pos,line_solutions *sols,bool &flow_rtl);
	void                Feed(int st_no,int st_pos,int en_no,int en_pos,bool flow_rtl,flow_eater* baby);
	void                Construct(int st_no,int st_pos,int en_no,int en_pos,bool flow_rtl,flow_eater* baby);

	void                Affiche(void);
};


#endif
