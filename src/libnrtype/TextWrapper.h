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
	char							uni_dir;
	int               uni_st,uni_en;
	PangoFont*        font;
} one_glyph;

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
	
	// boundaries
	int							nbBound,maxBound;
	text_boundary*  bounds;

	text_wrapper(void);
	~text_wrapper(void);
	
	void            SetDefaultFont(font_instance* iFont);
	void            AppendUTF8(char* text,int len);
	
	void            DoLayout(void);
	
	bool            NextCharacter(int &st,int &en);
	
	void            MergeWhiteSpace(void);
	void            MakeVertical(void);
	void            AddLetterSpacing(double dx,double dy);
	
	// boundary handling
	int             AddBoundary(text_boundary &ib);
	void            AddTwinBoundaries(text_boundary &is,text_boundary &ie);
	void            SortBoundaries(void);
	void            MakeTextBoundaries(PangoLogAttr* pAttrs,int nAttr);
	bool            Contains(int type,int g_st,int g_en,int &c_st,int &c_en);
};

#endif

