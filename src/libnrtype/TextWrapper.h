/*
 *  TextWrapper.h
 *  testICU
 *
 */

#ifndef my_text_wrapper
#define my_text_wrapper

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <pango/pango.h>

#include <libnrtype/nrtype-forward.h>

// miscanellous but useful data for a given text: chunking into logical pieces
// pieces include sentence/word, needed for example for the word-spacing property,
// and more important stuff like letter (ie visual letters)

// internal: different kinds of boundaries in a text
// the algo to determine these is very buggy...
enum {
	bnd_none = 0,
	bnd_char,
	bnd_word,
	bnd_sent,
	bnd_para
};
// one boundary
// boundaries are paired
typedef struct text_boundary {
	int							uni_pos; // index of the boundary in the text =
	                  // first char of the text chunk if 'start' boundary, char right after the boundary otherwise
	int             type;    // kind of boundary
	bool            start;   // is it the beginning of a chunk or its end
	int             other;   // index of the boundary this one is paired with in the array
	int             ind;     // utility
	int             inv_ind;
	union {   // data for this boundary; usually, one int is enough
		int           i;
		double        f;
		void*         p;
	} data; 
} text_boundary;

// pango converts the text into glyphs, but scatters the info for a given glyph
// here is a structure holding what inkscape needs to know
typedef struct one_glyph {
	int               gl;  // glyph_id
	double            x,y; // glyph position in the layout (nominal sizes, in the [0..1] range)
	bool              char_start; // is this glyph the beginning of a letter (rtl is taken in account)
	bool              word_start; // is this glyph the beginning of a word
	bool              para_start; // is this glyph the beginning of a paragraph (for indentation)
	char							uni_dir;    // bidi orientation of the run containing this glyph
	int               uni_st,uni_en; // start and end positions of the text corresponding to this glyph
	                     // you always have uni_st < uni_en
	PangoFont*        font;  // font this glyph uses (for bidi text, you need several fonts)
	                     // when rendering glyphs, check is this font is the one you're using
} one_glyph;

// text chunking 2, the comeback
// this time for sp-typeset
typedef struct one_box {
	int               g_st,g_en; // first and last glyph of this word
	double            ascent,descent,leading; // measurements 
	double            width;
	bool              word_start,word_end; // 
} one_box;

typedef struct one_para {
	int               b_st,b_en;
} one_para;

class text_wrapper {
public:
	char*           utf8_text;  // source text
	gunichar*				uni32_text; // ucs4 text computed from utf8_text
	one_glyph*			glyph_text; // glyph string computed for uni32_text
		
	// maps between the 2
	int             utf8_length; // utf8_text length
	int             uni32_length; // uni32_text length
	int             glyph_length; // number of glyph in the glyph_text array
	                   // the size of the array is (glyph_length+1) in fact; the last glyph is kind of a '0' char
	int*            uni32_codepoint; // uni32_codepoint[i] is the index in uni32_text corresponding to utf8_text[i]
	int*            utf8_codepoint;  // utf8_codepoint[i] is the index in utf8_text of the beginning of uni32_text[i]
		
	// layout
	font_instance*  default_font; // font set as the default font (would need at least one alternate per language)
	PangoLayout*    pLayout;      // private structure
	
	// kerning additions
	int             last_addition; // index in uni32_text of the beginning of the text added by the last AppendUTF8 call
	double*         kern_x;        // dx[i] is the dx for the ith unicode char
	double*         kern_y;
		
	// boundaries, in an array
	int							nbBound,maxBound;
	text_boundary*  bounds;
	
	// text organization
	int							nbBox,maxBox;
	one_box*        boxes;
	int             nbPara,maxPara;
	one_para*       paras;

	text_wrapper(void);
	~text_wrapper(void);
	
	// filling the structure with input data
	void            SetDefaultFont(font_instance* iFont);

	/**
	 * Append the specified text to utf8_text and uni32_codepoint.
	 *
	 * Note: Despite the name, the current implementation is primarily suited for a single
	 * call to set the text, rather than repeated calls to AppendUTF8: the implementation is
	 * Omega(n) in the new total length of the string, rather than just in the length of the
	 * text being appended.  This can probably be addressed fairly easily (see comments in
	 * code) if this is an issue for new callers.
	 *
	 * Requires: text is valid UTF8, or null.
	 *           Formally: text==NULL || g_utf8_validate(text,len,NULL).
	 *
	 * @param len Our sole existing caller (widgets/font_selector.cpp) uses len=-1.  N.B. The current
	 *   implementation may be buggy for len >= 0, especially for len==0.
	 */
	void            AppendUTF8(char const *text, int len);

	// adds dx or dy for the text added by the last AppendUTF8() call
	void            KernXForLastAddition(double *i_kern_x,int i_len,double scale=1.0);
	void            KernYForLastAddition(double *i_kern_y,int i_len,double scale=1.0);
	void            KernXForLastAddition(GList *i_kern_x,double scale=1.0);
	void            KernYForLastAddition(GList *i_kern_y,double scale=1.0);
	// compute the layout and stuff
	void            DoLayout(void);
	// semi-private: computes boundaries in the input text
	void            ChunkText(void);
	// utility function to move to the next element
	bool            NextChar(int &st,int &en);
	bool            NextWord(int &st,int &en);
	bool            NextPara(int &st,int &en);
	
	// post-processing after the initial layout
	// for the xml-space property: merges consecutive whitespace, and eats leading whitespace in the text
	void            MergeWhiteSpace(void);
	// makes vertical 'x' and 'y' fields in the glyph_text based on the computed positions
	void            MakeVertical(void);
	// as the names says...
	void            AddLetterSpacing(double dx,double dy,int g_st=-1,int g_en=-1);
	// adds the kerning specified by the KernXForLastAddition call to the layout
	void            AddDxDy(void);
	
	// boundary handling
	int             AddBoundary(text_boundary &ib);
	void            AddTwinBoundaries(text_boundary &is,text_boundary &ie);
	void            SortBoundaries(void);
	void            MakeTextBoundaries(PangoLogAttr* pAttrs,int nAttr);
	bool            Contains(int type,int g_st,int g_en,int &c_st,int &c_en);
	bool            IsBound(int type,int g_st,int &c_st);
	
	void            MeasureBoxes(void);
	int             NbLetter(int g_st,int g_en);
};

#endif

