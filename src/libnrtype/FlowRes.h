/*
 *  FlowRes.h
 *      by fred, 2004
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

/*
 * class to hold the result of a text flow
 * its contents are in this hierarchy:
 *  - chunks = lines
 *  - spans = runs of consistent style/directionality
 *	- letters = groups of glyphs behaving as a single letter 
 *  - glyphs
 * additionaly, glyphs are also grouped in groups of consistent style, to avoid the letter step, and to
 * make use of te NRArenaGlyphsGroup class.
 */
class flow_res {
public:
	typedef struct flow_glyph_group {
		int                  st, en; // first/ last+1 glyph of this group
		text_style*          style; // style
		NRArenaGlyphsGroup*  g_gr;  // unused
	} flow_glyph_group;
	
	typedef struct flow_glyph {
		int                  g_id; // glyph_id
		int                  let;  // letter this glyph belongs to (for the position)
		double               g_x, g_y, g_r; // position inside the letter
		font_instance*       g_font; // font
		NRArenaGlyphs*       g_gl; // unused
	} flow_glyph;
	
	typedef struct flow_styled_letter {
		int            g_st, g_en;  // first/ last+1 glyph in this letter
		int            t_st, t_en;  // first/ last+1 utf8 char in the 'chars' array
		int            utf8_offset; // utf8 offset in the source text
		int            ucs4_offset; // ucs4 offset in the source text
		int            no;         // this letter is the 'no'-th of its line from the line start
		double         x_st, x_en, y; // position (x_en < x_st => letter is rtl)
		double         kern_x, kern_y; // kerning for this letter; has already been accounted for in x_st/ y, it's here for output purposes
		double         pos_x, pos_y;   // x/y attributes for this letter (unused)
		double         rotate;      // rotation for this letter. set by the ApplyPath() for text-on-path
		bool           invisible;   // invisible letter (newline, or the one beyond the end of textPath)
	} flow_styled_letter;
	
	typedef struct flow_styled_span {
		int            g_st, g_en; // first/ last+1 glyph in this span, computed from the letters
		int            l_st, l_en; // first/ last+1 letter in this span
		bool			 rtl;       // direction
		double		 x_st, x_en, y; // dimensions, computed from the letters
		text_style*    c_style;   // style
	} flow_styled_span;
	
	typedef struct flow_styled_chunk {
		int            g_st, g_en;
		int            l_st, l_en;
		int            s_st, s_en; // first/ last+1 span in this line
		bool			 rtl;       // line rtl (may be != its spans' rtl)
		double         x_st, x_en, y, spacing; // position/ letterspacing
		double         ascent, descent, leading; // dimensions
		text_holder*   mommy;     // the paragraph that gave birth to this line (this gives access to the one_flow_src bounding this line)
	} flow_styled_chunk;
	
	// the arrays
	int               nbGroup, maxGroup;
	flow_glyph_group*   groups;
	int               nbGlyph, maxGlyph;
	flow_glyph*       glyphs;
	int               nbChar, maxChar;
	char*             chars;
	int               nbChunk, maxChunk;
	flow_styled_chunk*  chunks;
	int               nbLetter, maxLetter;
	flow_styled_letter* letters;
	int               nbSpan, maxSpan;
	flow_styled_span*   spans;
	// temporary variables
	bool              last_style_set;
	text_style*       last_c_style;
	bool              last_rtl;
	// these are set outside the Feed() functions because they are constant on the line => no need to pass them
	// down as parameters
	double            cur_ascent, cur_descent, cur_leading;
	text_holder*      cur_mommy;
	int               cur_offset;
	
	flow_res(void);
	~flow_res(void);
	
	// utility functions to fill the arrays
	void               Reset(void);
	void               AddGroup(text_style* g_s); // starts new glyph group; called automatically in StartSpan() and friends
	void               AddGlyph(int g_id, double g_x, double g_y, double g_w); // adds glyph in the last letter
	
	void               SetSourcePos(int i_pos);
	void               StartChunk(double x_st, double x_en, double y, bool rtl, double spacing);
	void               SetChunkInfo(double ascent, double descent, double leading, text_holder* mommy);
	void               StartSpan(text_style* i_style, bool rtl);
	void               EndWord(void);
	void               StartLetter(text_style* i_style,bool i_rtl, double k_x, double k_y, double p_x, double p_y, double rot, int i_no, int i_offset);
	void               AddText(char* iText, int iLen); // adds text in the last letter
		
	// polishing of the arrays:
	//  - offsets given during the construction are offset inside a paragraph; this functions translates to offset in the global text
	void               ComputeLetterOffsets(void);
	//  - computes glyph/ letter intervals for spans and chunks, and position/ dimensions
	void               ComputeIntervals(void);
	//  - adds letterspacing from the style (only for sp-text)
	void               ApplyLetterSpacing(void);
	//  - dy are transformed from incremental to absolute by the text_holder, this function reverts them to the incremental form
	void			  ComputeDY(int no);
	//  - dump all info on the terminal for debugging
	void               AfficheOutput(void);
	
	// functions for placing the text, used by sp-text; each applies to chunks[no]
	//  - make it vertical at position (to_x,to_y)
	void               Verticalize(int no, double to_x, double to_y);
	//  - put it on a path -> set the x_st/ y/ rotate values in the letter, and x_en becomes the letter length
	void               ApplyPath(int no, Path* i_path);
	//  - translate the line to position (to_x, to_y). if the_start==false, the end of the line is set to (to_x, to_y)
	void               TranslateChunk(int no, double to_x, double to_y, bool the_start);

	// utility accessors
	int                ChunkType(int no);
	SPObject*          ChunkSourceStart(int no);
	SPObject*          ChunkSourceEnd(int no);
	
	// functions for doing the showing
	//  - put glyphs on the canvas; set the paintbox on each nr_arena_glyph_group of the flow_res (used for paintserver fill)
	void               Show(NRArenaGroup* in_arena, NRRect *paintbox);
	//  - compute bbox
	void               BBox(NRRect *bbox, NR::Matrix const &transform);
	//  - print
	void               Print(SPPrintContext *ctx,
				 NRRect const *pbox, NRRect const *dbox, NRRect const *bbox,
				 NRMatrix const &ctm);
	//  - transform to a big fat sp-curve
	SPCurve*           NormalizedBPath(void);
	
	// functions to handle the on-canvas editing
	//  'c' is a chunk number, 's' is a span number, 'l' is a letter number in their respective arrays
	//  l_start==true means the position is at the beginning of the letter
	//  l_end==true means the position is at the end of the letter
	
	//  - from position 'offset' in the global utf8 text, find the letter in which it resides
	// returns: chunk number in c, span number in s, letter number in l
	void               OffsetToLetter(int offset, int &c, int &s, int &l, bool &l_start, bool &l_end);
	//  - from letter to offset
	void               LetterToOffset(int c, int s, int l, bool l_start, bool l_end, int &offset);
	//  - from position on the canvas to the nearest letter
	void               PositionToLetter(double px, double py, int &c, int &s, int &l, bool &l_start, bool &l_end);
	//  - from letter to position. return the position in (px,py), the line height in 'size', and the caret rotation in 'angle'
	void               LetterToPosition(int c, int s, int l, bool l_start, bool l_end, double &px, double &py, double &size, double &angle);
};

#endif
