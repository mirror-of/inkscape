/*
 *  FlowSrc.h
 */

#ifndef my_flow_res
#define my_flow_res

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "FlowDefs.h"

#include "../display/nr-arena-forward.h"
#include "../libnr/nr-rect.h"
#include "../libnr/nr-matrix.h"
#include "../display/curve.h"

class text_holder;
class flow_styles;
class text_style;
class flow_eater;
class font_instance;
class SPObject;
class Path;
class div_flow_src;

struct SPPrintContext;

class flow_res {
public:
	typedef struct flow_glyph_group {
		int                  st,en;
		text_style*          style;
		NRArenaGlyphsGroup*  g_gr;
	} flow_glyph_group;
	
	typedef struct flow_glyph {
		int                  g_id;
		int                  let;
		double               g_x,g_y,g_r;
		font_instance*       g_font;
		NRArenaGlyphs*       g_gl;
	} flow_glyph;
	
	typedef struct flow_styled_letter {
		int            g_st,g_en;
		int            t_st,t_en;
		int            utf8_offset;
		int            ucs4_offset;
		int            no;
		double         x_st,x_en,y;
		double         kern_x,kern_y;
		double         pos_x,pos_y;
		double         rotate;
		bool           invisible;
	} flow_styled_letter;
	
	typedef struct flow_styled_span {
		int            g_st,g_en;
		int            l_st,l_en;
		bool					 rtl;
		double				 x_st,x_en,y;
		text_style*    c_style;
	} flow_styled_span;
	
	typedef struct flow_styled_chunk {
		int            g_st,g_en;
		int            l_st,l_en;
		int            s_st,s_en;
		bool					 rtl;
		double         x_st,x_en,y,spacing;
		double         ascent,descent,leading;
		text_holder*   mommy;
	} flow_styled_chunk;
	
	int                 nbGroup,maxGroup;
	flow_glyph_group*   groups;
	int                 nbGlyph,maxGlyph;
	flow_glyph*         glyphs;
	
	int                 nbChar,maxChar;
	char*               chars;
	int                 nbChunk,maxChunk;
	flow_styled_chunk*  chunks;
	int                 nbLetter,maxLetter;
	flow_styled_letter* letters;
	int                 nbSpan,maxSpan;
	flow_styled_span*   spans;
	
	bool                last_style_set;
	text_style*         last_c_style;
	bool                last_rtl;
	double              cur_ascent,cur_descent,cur_leading;
	text_holder*        cur_mommy;
	int                 cur_offset;
	
	flow_res(void);
	~flow_res(void);
	
	void               Reset(void);
	void               AddGroup(text_style* g_s);
	void               AddGlyph(int g_id,double g_x,double g_y,double g_w);
	
	void               SetSourcePos(int i_pos);
	void               StartChunk(double x_st,double x_en,double y,bool rtl,double spacing);
	void               SetChunkInfo(double ascent,double descent,double leading,text_holder* mommy);
	void               StartSpan(text_style* i_style,bool rtl);
	void               EndWord(void);
	void               StartLetter(text_style* i_style,bool i_rtl,double k_x,double k_y,double p_x,double p_y,double rot,int i_no,int i_offset);
	void               AddText(char* iText,int iLen);
		
	void               ComputeLetterOffsets(void);
	void               ComputeIntervals(void);
	void               ApplyLetterSpacing(void);
	void							 ComputeDY(int no);
	void               AfficheOutput(void);
	
	void               Verticalize(int no,double to_x,double to_y);
	void               ApplyPath(int no,Path* i_path);
	void               TranslateChunk(int no,double to_x,double to_y,bool the_start);

	int                ChunkType(int no);
	SPObject*          ChunkSourceStart(int no);
	SPObject*          ChunkSourceEnd(int no);
	
	void               Show(NRArenaGroup* in_arena);
	void               BBox(NRRect *bbox, NR::Matrix const &transform);
	void               Print(SPPrintContext* ctx,NRRect *pbox,NRRect *dbox,NRRect *bbox,NRMatrix &ctm);
	SPCurve*           NormalizedBPath(void);
	void               OffsetToLetter(int offset,int &c,int &s,int &l,bool &l_start,bool &l_end);
	void               LetterToOffset(int c,int s,int l,bool l_start,bool l_end,int &offset);
	void               PositionToLetter(double px,double py,int &c,int &s,int &l,bool &l_start,bool &l_end);
	void               LetterToPosition(int c,int s,int l,bool l_start,bool l_end,double &px,double &py,double &size,double &angle);
};

#endif
