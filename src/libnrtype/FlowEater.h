/*
 *  FlowEater.h
 */

#ifndef my_flow_eater
#define my_flow_eater

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "FlowDefs.h"
#include "FlowUtils.h"

class text_style;
class flow_src;
class flow_dest;
class flow_res;
class line_solutions;

class flow_eater {
public:
	bool          line_rtl,word_rtl;
	int						cur_letter,word_letter,next_letter;
	double        cur_length,word_length,next_length;
	bool          first_letter;
	double        line_st_x,line_st_y,line_spc;
	
	flow_res*     the_flow;
	
	flow_eater(void);
	~flow_eater(void);
	
	void          StartLine(bool rtl,double x,double y,double l_spc);
	void          StartWord(bool rtl,int nb_letter,double length);
	void          StartLetter(void);
	void          Eat(int g_id,text_style* g_font,double g_x,double g_y,double g_w);
	
	void          StartBox(bool rtl,int nb_letter,double length);
	void          Eat(char* iText,int iLen,double i_w,int i_l,text_style* i_style,double* k_x,double* k_y,int k_offset);
};

class flow_maker {
public:
	flow_src*          f_src;
	flow_dest*         f_dst;
	
	flow_tasks*        pending;
	int                nbBrk,maxBrk;
	flow_brk*          brks;
	int                elem_st_st;
	int                nbElmSt,maxElmSt;
	int*               elem_start;
	int                nbKgSt,maxKgSt;
	int*               king_start;
	typedef struct brk_king {
		int              next;
		int              index;
		double           x,y;
		int							 elem_no,elem_pos;
		double           score;
		int              brk_no;
	} brk_king;
	int                nbKing,maxKing;
	brk_king*          kings;
	
	bool               strictBefore,strictAfter;
	double             par_indent;
	bool               justify;
	double             min_scale,max_scale;
	
	int                algo;
	
	flow_maker(flow_src* i_src,flow_dest* i_dst);
	~flow_maker(void);
	
	flow_res*          Work(void);
	
	flow_res*					 StdAlgo(void);
	flow_res*          KPAlgo(void);
	
	void               KPDoPara(int &st_brk,flow_requirement &st_req,line_solutions *sols);
	
	int                AddBrk(box_sol& i_box,int i_no,int i_pos);
	
	double             KPBrkScore(int root_brk,int to_brk);
	void               ResetElemStarts(int st);
	void               AddElems(int up_to);
	int                BrkIndex(int elem_no,int elem_pos);
	
	int                KingOf(int elem_no,int elem_pos,double x,double y);
	void							 AddKing(int brk,int elem_no,int elem_pos,double x,double y,double score);
};

#endif
