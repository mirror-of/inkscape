/*
 *  FlowEater.h
 */

#ifndef my_flow_eater
#define my_flow_eater

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "FlowDefs.h"
#include "FlowUtils.h"

class text_style;
class flow_src;
class flow_dest;
class flow_res;
class line_solutions;

/** \brief internal libnrtype class

internal libnrtype class. A flow_eater is the connection between the
output of the text positioning algorithm (in flow_maker) and a
flow_res. Once the final position and length of a line has been
calculated (which takes a bit of trial and error), the layout algorithm
will go back to the beginning of the line and use a flow_eater to send the
data glyph-by-glyph to the flow_res. The primary cleverness of this class
is in reordering rtl glyphs in ltr lines, and vice-versa, to be in strictly
increasing (decreasing) pixel order, rather than in character order.
Secondary cleverness is that it can do (not very clever) justification. See
StartLine() for details.

#the_flow must be set before anything else is done. After that, the
methods must be called in a sane order (ie don't call StartWord() before
StartLine()). The Eat() method should more logically be named StartGlyph.
*/
class flow_eater {
public:
    /** set once, right after construction. Used by flow_maker for no reason later on.
    Shouldn't be public and should be a required parameter to the ctor. */
	flow_res*     the_flow;
	
	flow_eater(void);
	~flow_eater(void);
	
	/** Begin a new line. A new chunk is added to #the_flow using 
    flow_res::StartChunk() ready to store the data which will follow.
      \param rtl   the text directionality
      \param x     the starting x point, ie the left for ltr and right for rtl
      \param ox    the 'other' x: right for ltr and left for rtl
      \param y     the y coordinate of the glyphs on this line
      \param l_spc the number of extra pixels to insert between every
                   character on the line. A simplistic implementation of
                   justification.
    */
	void          StartLine(bool rtl,double x,double ox,double y,double l_spc);
    
    /** Begin a new 'word'. A 'word' is the largest continous run of
    characters all with the same direction and thus can, and usually will,
    contain many linguistic words.  A word has no comparable flow_res
    array and its letters need not all have the same font/style.
      \param rtl   the text directionality
      \param nb_letter  the number of letters in the word
      \param length     the width of the word in pixels
    */
	void          StartWord(bool rtl,int nb_letter,double length);

    /** no implementation, never called */
	void          EndWord(void);

    /** Begins a new letter in #the_flow by calling flow_res::StartLetter().
    The x and y coordinates and letter number have been kept track of by the
    class. Rotation is always zero.
    */
	void          StartLetter(text_style* g_style,double k_x,double k_y,int utf8_offset);

    /** Adds the glyph g_id, which represents the character string iText, to
    the end of #the_flow. iLen is measured in bytes. g_x,g_y is the
    position of this glyph within its letter and g_w is the width by which
    to extend the letter. */
	void          Eat(int g_id,double g_x,double g_y,double g_w,char* iText,int iLen);

private:
    /** the rtl value passed to the most recent calls to StartLine() and
    StartWord() respectively. */
	bool          line_rtl,word_rtl;
    /**
     \*_letter define the number of letters since the start of the line
     \*_length define the number of pixels since the start of the line
     cur_* define the count at the start of the current word
     next_* define what the count will be at the end of the current word
     word_* define the count at the start of the current letter. For rtl
            words on an ltr line, or vice-versa, this counts backwards.
    */
	int	          cur_letter,word_letter,next_letter;
	double        cur_length,word_length,next_length;
    /** true for the first letter on a line */
	bool          first_letter;
    /** the location passed to the most recent call to StartLine() */
	double        line_st_x,line_st_y;
    /** the intra-letter spacing set by the most recent call to StartLine() */
    double        line_spc;
    /** the cumulative widths of all the glyphs in the current letter, plus
    any manual kerning. Reset to zero by StartLetter(). It is negative for
    rtl letters. */
	double        letter_length;
};

/** \brief convert a string of characters with formatting codes into a layout of glyphs

This class converts a flow_src into a flow_res by interpreting the
formatting codes in the former, laying out the text and then storing it as
a pattern of glyphs in the latter.

It has two modes of operation: flowed with text wrapping, and plain.

Usage:
-# Set up your flow_src with the text and formatting codes you wish to
   render.
-# If you are doing wrapping, set up a flow_dest containing the fully
   defined shape inside which to wrap. You may also use flow_dest::next_in_flow
   to chain addition shapes.
-# Construct a flow_maker. If you aren't doing wrapping then the second
   parameter to the constructor should be NULL.
-# Configure any of the public variables you wish to, or leave them as
   the defaults.
-# If you want to wrap inside a shape and set a flow_dest in the
   constructor call Work(), otherwise call TextWork().
-# Don't forget to deconstruct the flow_maker when you're done.
*/
class flow_maker {
public:
    /* Set by the caller. TODO. See flow_sol::NewLine() */
	bool               strictBefore,strictAfter;
    /* Set by the caller. TODO: ?the number of pixels to add as the left
    margin of every line of text (right margin for rtl paragraphs). */
	double             par_indent;
    /** Set by the caller. Enable full justification when wrapping inside
    a shape. */
	bool               justify;
    /** Set by the caller. TODO: ?the maximum and minimum amounts of padding to use during
    justification. 1.0 is no padding. Less than this and lines that are
    slightly too long will have their letter spacing reduced to fit the
    available space. Greater than this and lines that are too short will
    have letter spacing added. If the line is still not long enough to
    fill the space then the right margin will be ragged. If the line is
    comprised of one word and cannot be compressed small enough it will
    be wrapped at a character boundary. ?None of this checked. */
	double             min_scale,max_scale;
	
    /** set this to control which algorithm the flow_maker will use to flow
    text inside a shape. Algorithm 0 is described in StdAlgo(), algorithm
    1 is described in KPAlgo(). Other values will cause a NULL return from
    Work(). */
	int                algo;
	
	flow_maker(flow_src* i_src,flow_dest* i_dst = NULL);
	~flow_maker(void);
	
    /** for #algo == 0 call StdAlgo(), for #algo == 1 call KpAlgo() */
	flow_res*          Work(void);

    /** Layout the text in the #flow_src disregarding line wrapping. All
    lines will continue until either there is a line break explicitly
    specified or there is no more text. */
	flow_res*          TextWork(void);
	
	/** internal libnrtype only. TODO*/
    flow_brk*          brks;
private:
    /** the flow_src (the source of the formatted text) set by the constructor */
	flow_src*          f_src;
    /** the flow_dest (the shape inside which to wrap) set by the constructor */
	flow_dest*         f_dst;
	
	flow_tasks*        pending;
	int                nbBrk,maxBrk;
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
