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

#include <glib.h>

#include <pango/pango.h>

class font_instance;

enum {
	bnd_none = 0,
	bnd_char,
	bnd_word,
	bnd_sent,
	bnd_para
};

typedef struct text_boundary {
	int							uni_pos; // premier char si start, suivant si end
	int             type;
	bool            start;
	int             other;
	int             ind;
	int             inv_ind;
	union {
		int           i;
		double        f;
		void*         p;
	} data; 
} text_boundary;

typedef struct one_glyph {
	int               gl;
	double            x,y;
	bool              char_start;
	bool              word_start;
	bool              para_start;
	char							uni_dir;
	int               uni_st,uni_en;
	PangoFont*        font;
} one_glyph;

typedef struct one_box {
	int               g_st,g_en;
	double            ascent,descent,leading;
	double            width;
	bool              word_start,word_end;
} one_box;

typedef struct one_para {
	int               b_st,b_en;
} one_para;

class text_wrapper {
public:
	char*           utf8_text;
	gunichar*				uni32_text;
	one_glyph*			glyph_text;
		
	// maps between the 2
	int             utf8_length;
	int             uni32_length;
	int             glyph_length;
	int*            uni32_codepoint;
	int*            utf8_codepoint;  // start of the ith unicode char
		
	// layout
	font_instance*  default_font;
	PangoLayout*    pLayout;
	
	// kerning additions
	int             last_addition;
	double*         kern_x;
	double*         kern_y;
		
	// boundaries
	int							nbBound,maxBound;
	text_boundary*  bounds;
	
	// text organization
	int							nbBox,maxBox;
	one_box*        boxes;
	int             nbPara,maxPara;
	one_para*       paras;

	text_wrapper(void);
	~text_wrapper(void);
	
	void            SetDefaultFont(font_instance* iFont);
	void            AppendUTF8(char* text,int len);
	
	void            DoLayout(void);
	
	void            ChunkText(void);
	
	bool            NextChar(int &st,int &en);
	bool            NextWord(int &st,int &en);
	bool            NextPara(int &st,int &en);
	
	void            MergeWhiteSpace(void);
	void            MakeVertical(void);
	void            AddLetterSpacing(double dx,double dy,int g_st=-1,int g_en=-1);
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

	void            KernXForLastAddition(double *i_kern_x,int i_len,double scale=1.0);
	void            KernYForLastAddition(double *i_kern_y,int i_len,double scale=1.0);
	void            KernXForLastAddition(GList *i_kern_x,double scale=1.0);
	void            KernYForLastAddition(GList *i_kern_y,double scale=1.0);
};

#endif

