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
#include "FlowSrcText.h"

#include <glib.h>
#include "../svg/svg.h"

#include "FlowStyle.h"

class text_holder;
class flow_styles;
class text_style;
class flow_eater;
class line_solutions;
class SPStyle;
class flow_src;

class SPObject;


class div_flow_src;
// lightweight class to be included in those filling the flow source
class one_flow_src {
public:
	SPObject*        me;
	// the text interval held by this object
	int              ucs4_st,ucs4_en;
	int              utf8_st,utf8_en;
	// linking in the flow; prev=NULL means it's a flow start, a paragraph or a sodipodi:role=line tspan
	one_flow_src     *next,*prev;
	one_flow_src		 *dad,*chunk;
	
	one_flow_src(SPObject* i_me);
	virtual ~one_flow_src(void);
	
	void              Link(one_flow_src* after,one_flow_src* inside);
	void              DoPositions(bool for_text);
	void              DoFill(flow_src* what);
	one_flow_src*     Locate(int utf8_pos,int &ucs4_pos,bool src_start,bool src_end,bool must_be_text);
	
	virtual int       Type(void) {return flw_none;};
	virtual void      PushInfo(int st,int en,int offset,text_holder* into);
	virtual void      DeleteInfo(int i_utf8_st,int i_utf8_en,int i_ucs4_st,int i_ucs4_en);
	virtual text_style*  GetStyle(void);
	virtual void      Fill(flow_src* what);
	virtual void      SetPositions(bool for_text,int &last_utf8,int &last_ucs4,bool &in_white); // computes the ucs4_st and ucs4_en vals, and does the linking of chunk
	virtual void      Insert(int utf8_pos,int ucs4_pos,const char* n_text,int n_utf8_len,int n_ucs4_len,bool &done); 
	virtual void      Delete(int i_utf8_st,int i_utf8_en);
	virtual void      AddValue(int utf8_pos,SPSVGLength &val,int v_type,bool increment);
};
// text variant
class text_flow_src : public one_flow_src {
public:
	partial_text      cleaned_up;
	correspondance    string_to_me;
	
	text_flow_src(SPObject* i_me);
	virtual ~text_flow_src(void);
	
	void              SetStringText(partial_text* iTxt);
			
	virtual int       Type(void) {return flw_text;};
	virtual void      Fill(flow_src* what);
	virtual void      SetPositions(bool for_text,int &last_utf8,int &last_ucs4,bool &in_white);
	virtual void      Insert(int utf8_pos,int ucs4_pos,const char* n_text,int n_utf8_len,int n_ucs4_len,bool &done); 
	virtual void      Delete(int i_utf8_st,int i_utf8_en);
	virtual void      AddValue(int utf8_pos,SPSVGLength &val,int v_type,bool increment);
};
// control stuff in the flow, like line and region breaks
class control_flow_src : public one_flow_src {
public:
	int               type;
	
	control_flow_src(SPObject* i_me,int i_type);
	virtual ~control_flow_src(void);
	
	virtual int       Type(void) {return type;};
	virtual void      Fill(flow_src* what);
	virtual void      SetPositions(bool for_text,int &last_utf8,int &last_ucs4,bool &in_white);
};
// object variant, to hold placement info
class div_flow_src : public one_flow_src {
public:
	int 							type;
	bool              is_chunk_start;
	bool              is_chunk_end;
	bool              vertical_layout;
	SPStyle           *style; // only for simplicity
	                          // this has to last as long as the flow_res we're going to derive from it
	                          // hence the style_holder class
	int               nb_x,nb_y,nb_rot,nb_dx,nb_dy;
	SPSVGLength       *x_s,*y_s,*rot_s,*dx_s,*dy_s;
	
	div_flow_src(SPObject* i_me,int i_type);
	virtual ~div_flow_src(void);
	
	static void       ReadArray(int &nb,SPSVGLength* &array,const char* from);
	static char*      WriteArray(int nb,SPSVGLength* array);
	static void       InsertArray(int l,int at,int &nb,SPSVGLength* &array,bool is_delta);
	static void       SuppressArray(int l,int at,int &nb,SPSVGLength* &array);
	static void       ForceVal(int at,SPSVGLength &val,int &nb,SPSVGLength* &array,bool increment);
	static void       UpdateArray(double size,double scale,int &nb,SPSVGLength* &array);
	void              UpdateLength(double size,double scale);
	void              SetStyle(SPStyle* i_style);
	void							SetX(const char* val);
	void							SetY(const char* val);
	void							SetDX(const char* val);
	void							SetDY(const char* val);
	void							SetRot(const char* val);
	char*							GetX(int st=-1,int en=-1);
	char*							GetY(int st=-1,int en=-1);
	char*							GetDX(int st=-1,int en=-1);
	char*							GetDY(int st=-1,int en=-1);
	char*							GetRot(int st=-1,int en=-1);
	void              DoAddValue(int utf8_pos,int ucs4_pos,SPSVGLength &val,int v_type,bool increment);
	
	int               UCS4Pos(int i_utf8_pos);
	
	virtual int       Type(void) {return type;};
	virtual void      PushInfo(int st,int en,int offset,text_holder* into);
	virtual void      DeleteInfo(int i_utf8_st,int i_utf8_en,int i_ucs4_st,int i_ucs4_en);
	virtual text_style*  GetStyle(void);
	virtual void      Fill(flow_src* what);
	virtual void      SetPositions(bool for_text,int &last_utf8,int &last_ucs4,bool &in_white);
	virtual void      Insert(int utf8_pos,int ucs4_pos,const char* n_text,int n_utf8_len,int n_ucs4_len,bool &done); 
	virtual void      Delete(int i_utf8_st,int i_utf8_en);
};

class flow_src : public flow_styles {
public:
	typedef struct one_elem {
		int               type;
		text_holder*      text;
		one_flow_src*     obj;
	} one_elem;
	int                 nbElem,maxElem;
	one_elem*           elems;
	
	text_holder*        cur_holder;
	
	bool                min_mode;

	flow_src(void);
	~flow_src(void);
		
	void                AddElement(int i_type,text_holder* i_text,one_flow_src* i_obj);

	char*               Summary(void);
	
	void                Clean(int &no,int &pos);
	
	void                Prepare(void);
	
	void                MetricsAt(int from_no,int from_pos,double &ascent,double &descent,double &leading,bool &flow_rtl);
	void                ComputeSol(int from_no,int from_pos,line_solutions *sols,bool &flow_rtl);
	void                Feed(int st_no,int st_pos,int en_no,int en_pos,bool flow_rtl,flow_eater* baby);
	text_holder*        ParagraphBetween(int st_no,int st_pos,int en_no,int en_pos);

	void                Affiche(void);
};


#endif
