/*
 *  text_holder.h
 */

#ifndef my_flow_boxes
#define my_flow_boxes

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "FlowDefs.h"
#include "FlowSrcText.h"

//#include <pango/pango.h>

class text_style;
class flow_styles;
class line_solutions;
class flow_eater;
class one_flow_src;

/* a class to hold the text of a paragraph being flowed into a shape.
 * the base directionality the paragraph is given by the directionality of its first box, ie boxes[0].rtl ; 
 * boxes are reordered to have only one direction: box i+1 is flowed right after box i
 * some information regarding the control elements in the text is stored in the ctrls array, but it
 * is mainly meant to hold the boundaries of boxes/ words/ style spans in the text
 * since pango needs it, the pango engines spans are also stored there
 */

// types of control elements in the text
enum {
	ctrl_none      = 0,
	ctrl_return    = 1, // newline
	ctrl_tab       = 2,
	ctrl_style     = 3, // style span end or start
	ctrl_bidi      = 4, // bidi span end or start
	ctrl_lang      = 5, // language span end or start
	ctrl_letter    = 6, 
	ctrl_word      = 7,
	ctrl_box       = 8,
	ctrl_shp_eng   = 9, // span of identical shape_engine for pango, end or start
	ctrl_lang_eng  = 10,
	ctrl_soft_hyph = 11 // soft hyphen
};

class text_holder {
public:
	// source text
	partial_text   raw_text;
	correspondance flow_to_me;
	int            utf8_length;    // the text 
	char*          utf8_text;      // actually, it's a pointer on the dst_text in raw_text
	int            ucs4_length;    // not really needed
	// list of non-text stuff (control chars)
	// it is an array of boundaries or positions
	typedef struct one_ctrl {
		int          pos;      // position of this boundary/ element. position i means: 'at beginning of char i in the text'
		bool         is_start; // is it a boundary that begins a span?
		bool         is_end;   // is it a boundary that ends a span?
		int					 other;    // if boundary of a span, index of the other boundary for this span
		int          typ;      // type of the control
		int          ind,inv;  // temporary variables to allow refreshing the field 'other' after sorting
		int          ucs4_offset; // offset in number of unicode codepoint wrt the beginning of the text (for kerning)
		union {
			int          i;
			text_style*  s;
			void*				 v;
		} data; // possible data
	} one_ctrl;
	// the array of controls
	int            nbCtrl,maxCtrl;
	one_ctrl*			 ctrls;
	// resulting boxes for the flow
	// these are intermediate values for the flow algorithm: boxes are flowed, and after that, each flowed box
	// is treated individually, and its glyphs are sent to the appropriate functions
	typedef struct one_flow_box {
		int          st,en,ucs4_offset; // dimensions in the source text
		bool         rtl;               // contains text right-to-left?
		bool         white;             // whitespace?
		bool         hyphenated;        // ends with an hyphen?
		box_sizes    meas;              // cached dimensions
		int          next,prev;         // next and prev box in the flow.
		                                // for an hyphenated box, next is the box corresponding to the rest of the word
		int					 first,link;        // boxes corresponding to the first possible hyphenation of the current box, 
		                                // and to the next possible hyphenation respectively
	} one_flow_box;
	// array of boxes
	int            nbBox,maxBox;
	one_flow_box*  boxes;
	// kerning info, built when the text is collected.
	// if kern_x != NULL then kern_x covers all the text in this text_holder; idem for kern_y
	int						 nb_kern_x,nb_kern_y;
	double				 *kern_x,*kern_y;	
	// temp data
	bool           paragraph_rtl;
	one_flow_src*  source_start;  // for the sp-text layout, to which tspan/ textpath does this text belong?
	one_flow_src*  source_end;  // for the sp-text layout, to which tspan/ textpath does this text belong?

	text_holder(void);
	~text_holder(void);
	
	// adding utf8 text
	void					 AppendUTF8(partial_text* iTxt);
	void           LastAddition(int &d_utf8_st,int &d_ucs4_st,int &d_utf8_en,int &d_ucs4_en);
	// final touch on the raw_text, and setting utf8_text to point on tha appropriate location
	void					 DoText(void);
	// preparation of the text, mostly chunking into boxes
	void           DoChunking(flow_styles* style_holder);
	// computing the boxes sizes
	void           ComputeBoxes(void);
	// text metrics at position from_box in this text_holder
	// if flow_rtl is not boxes[0].rtl, from_box is understood as the index from the end
	bool           MetricsAt(int from_box,double &ascent,double &descent,double &leading,bool flow_rtl);
	// pushes boxes into the line_solution instance, to prepare the possible ways of filling a region span
	// last_in_para and last_in_rgn are set up by the calling flow_src::ComputeSols, and reflect whether this
	// text_holder is the last of the para or the region (should always end a para with current settings)
	bool           ComputeSols(int from_box,line_solutions* sols,int with_no,bool last_in_para,bool last_in_rgn,bool flow_rtl);
	// send boxes st_pos to en_pos (reverse if flow_rtl is not boxes[0].rtl) to the baby for recording glyphs
	void					 Feed(int st_pos,int en_pos,bool flow_rtl,flow_eater* baby);
	
	// manipulation of the array of controls
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

	void           UpdatePangoAnalysis(int from,int p_st,int p_en,void *pan);
	void           MeasureText(int p_st,int p_en,box_sizes &a,void *pan,int b_offset,int with_hyphen);
};


#endif
