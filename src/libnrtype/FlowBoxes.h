/*
 *  text_holder.h
 */

#ifndef my_flow_boxes
#define my_flow_boxes

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "FlowDefs.h"
#include "FlowSrcText.h"
#include "FlowStyle.h"

//#include <pango/pango.h>

class text_style;
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

/** \brief internal libnrtype method. Does the actual meat of deciding where to wrap text

This class does a huge amount of stuff. It takes the text stream all the way
from a set of partial_text's and a load of font styles to a flow_res and
hence could be described at the main driver behind text layout.

Usage:
-# Construct
-# Call AppendUTF8() lots of times
-# Call AddStyleSpan() lots of times (note that confusion may arise because
   the parameters to AddStyleSpan() represent offsets in the combined text,
   not in the original sources.
-# (optional) Call AddKerning().
-# Call DoChunking()
-# Call ComputeBoxes()
-# Work out the area in which you want to render the text (usually using
   flow_dest).
-# Set up a line_solutions
-# Call ComputeSols()
-# Once you are happy with the solution, call Feed().

Terminology:
- box: a continuous run of non-whitespace characters (ie a word) or a
  continuous run of whitespace (ie the gaps between words).
- control: an item of metadata in the text stream, such as a tab.
  Some controls are spans.
- span: a pair of items in the #ctrls array which together specify the
  start and end of a control. Styles are stored as spans (like in
  svg).

Major flaws that I've spotted so far (NB: these don't apply to any particular
method - the whole implementation is broken) :
- lines that wrap in the middle of counterdirectional text will have the
  wrong word ordering
- word-wrapping is defined using whitespace. Some languages use no whitespace
  at all (Pango has custom language engines in these cases).
- data is passed to Pango in a very piecemeal manner. As well as being slow,
  there are some languages for which this is a very bad idea. It doesn't
  even save any memory because all the tracking needed to manage this is
  huge.
- important data from Pango is lost, such as the available cursor positions.
- all the tests of Pango's rtl results are wrong and will break for
  triple-nested directionality.
- the whole concept of spans is flawed, given the effort that goes in to
  keeping both the start and end points maintained. Single control points
  marking where the changes happen would be a much better implementation.
*/
class text_holder {
public:
	// source text
    /** the output storage for #flow_to_me */
	partial_text   raw_text;
	correspondance flow_to_me;

    /** maintained as a copy of #raw_text .utf8_length */
	int            utf8_length;    // the text 
    /** maintained as a copy of #raw_text .utf8_text */
	char*          utf8_text;      // actually, it's a pointer on the dst_text in raw_text
    /** maintained as a copy of #raw_text .ucs4_length */
	int            ucs4_length;    // not really needed

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
	int            nbBox,maxBox;
	/// array of boxes, see ComputeBoxes()
	one_flow_box*  boxes;

	bool           paragraph_rtl;   ///the base directionality of the paragraph
    /// not used by this class
	one_flow_src*  source_start;  // for the sp-text layout, to which tspan/ textpath does this text belong?
    /// not used by this class
	one_flow_src*  source_end;  // for the sp-text layout, to which tspan/ textpath does this text belong?

	text_holder(void);
	~text_holder(void);
	
	// adding utf8 text
    /** Adds iTxt as a source for #flow_to_me then calls DoText().
    TODO: bug? This is called more than once, but the details of DoText()
    (particularly that commented-out nbCtrl=0) mean that control characters
    will be added multiple times in that case. It's likely that this has
    never been tested because nobody tried tabs or soft hyphenation. */
	void					 AppendUTF8(partial_text* iTxt);

    /** returns the start and end characters/bytes of the last continuous
    run of text added to #flow_to_me, ie the last block of text that didn't
    require any extraneous characters removed. */
	void           LastAddition(int &d_utf8_st,int &d_ucs4_st,int &d_utf8_en,int &d_ucs4_en);

    /** Fills the #ctrls array using the text added with AppendUTF8().
    #ctrls is private so nothing can be done with this information until
    after ComputeBoxes() has been called.
    
    Uses Pango facilities to perform a three-stage process on #raw_text
    (so DoText() must have already been called for it to be filled):
     -# add to #ctrls spans describing the changes in directionality,
        language, shaping engine and language engines.
     -# detect when the font selected for a piece of text does not have the
        glyphs required to render some of that text and breaks the span in
        the #ctrls array that specifies the font style into several separate
        spans that use the necessary fonts.
     -# creates new spans in the #ctrls array with types ctrl_boxes and
        ctrl_word. Both of these spans have full coverage, ie every byte of
        text belongs to exactly one span of type ctrl_boxes \em and exactly
        one span of type ctrl_word.
        - Words are delimited by Pango's definition of a word (see
          PangoLogAttr). To ensure full coverage, the whitespace between
          words have their own span. These spans are never again referenced.
          TODO: bug? The Pango structure seems to be misinterpreted such
          that the last character of every word is also given its own span.
        - Boxes are delimited by Pango's definition of whitespace. The
          whitespace is also granted its own span.

        You will have noticed that this is a lot of entries in #ctrls for
        any reasonable size paragraph.

    On output the #ctrls array will be sorted, will have no overlapping
    spans (see SplitSpan()) and its ucs4_offset values will be filled.
    TODO: why are both boxes and words needed? Is whitespace not rendered?
    Why are they needed at all?

    \param style_holder  a list of all the text_style elements used in the
                         text which is added to if it was necessary to
                         substitute a font because a needed glyph was not
                         found.
    */
	void           DoChunking(flow_styles* style_holder);

    /** Fills the #boxes array with the widths of each word of text (and all
    the other fields, but it's mainly used to store widths).
    It is assumed that DoChunking() has already been called.
    Remember that a 'word' is also a continuous run of whitespace, as well as
    a continuous run of non-whitespace.
     -# Uses the ctrl_box members of the #ctrls array (which were created by
        DoChunking()) to fill the #boxes array, which primarily stores the
        total pixel width of the glyphs in that word. When a word contains a
        soft hyphen, multiple boxes are created for that word: firstly one
        specifying the total width, then one for every soft hyphen in the
        word specifying the cumulative width for the word up until that
        hyphen.
     -# The elements of #boxes are chained together so that the \a next and
        \a prev members index the sequential words, ignoring the soft hyphen
        entries. For a word containing soft hyphens, \a link is set to the
        first shorter version and the \a next member of subsequent entries
        is set to the next one. These values are only used internally.
     -# The elements of #boxes which represent counterdirectional text (ltr
        in an rtl paragraph or vice-versa) are re-ordered to be in the same
        direction as the glyph order of the paragraph. This is a bad thing
        to do (see list of flaws above).
    */
	void           ComputeBoxes(void);

    /** Retrieves the sizes of the ascender, descender and leading from the
    element of #boxes with index \a from_box. If flow_rtl is different to
    the directionality of the stored paragraph (from #paragraph_rtl) then
    \a from_box is an index from the \em end of the #boxes array. TODO: why?

    Returns false if you haven't called ComputeBoxes() yet or the paragraph
    is empty.
    */
	bool           MetricsAt(int from_box,double &ascent,double &descent,double &leading,bool flow_rtl);

	// pushes boxes into the line_solution instance, to prepare the possible ways of filling a region span
	// last_in_para and last_in_rgn are set up by the calling flow_src::ComputeSols, and reflect whether this
	// text_holder is the last of the para or the region (should always end a para with current settings)
    /** ComputeBoxes() must have already been called.
    Adds the #boxes starting at \a from_box to \a sols up until the
    pixel width specified in \a sols. The caller should have already
    called line_solutions::NewLine() and line_solutions::StartLine() to
    set up the available space on the line. If \a flow_rtl is true then
    \a from_box indexes from the end of #boxes. \a with_no is an index in
    to a flow_src::elems array to use for marking the outputted solutions.
     \ret  true if the end of the allocated width was reached before all
           the boxes were consumed (ie true if we wrapped). */
	bool           ComputeSols(int from_box,line_solutions* sols,int with_no,bool last_in_para,bool last_in_rgn,bool flow_rtl);

	// send boxes st_pos to en_pos (reverse if flow_rtl is not boxes[0].rtl) to the baby for recording glyphs
    /** Converts all the characters between the given byte ranges into the
    correct glyphs and uses the flow_eater to pass them into the flow_res.
    text_style::Feed() does the actual glyph conversion.
    flow_eater::StartLine() must have been called on \a baby prior to calling
    this method.
    */
	void					 Feed(int st_pos,int en_pos,bool flow_rtl,flow_eater* baby);
	
    /** Assigns a font style to the section of #raw_text between the two
    byte values given.
    
    Effectively calls AddSpan() with typ=ctrl_style then sets the
    \a s of one_ctrl::data of both new elements to \a i_s */
	void           AddStyleSpan(int st,int en,text_style* i_s);

    /** sorts the #ctrls array into numerically increasing order of the \a pos
    element. Also, any non-spans which sit exactly on the start or end of a
    span are considered to be 'outside' the span. */
	void           SortCtrl(void);

    /** debug method. Dumps to stdout the contents of the #ctrls array.
    (From the French 'afficher', to display, in case you were wondering. */
	void           AfficheCtrl(void);
	
    /** debug method. Dumps to stdout the contents of the #boxes array. */
	void           AfficheBoxes(void);
	
	/** assigns kerning values for character numbers \a i_st to \a i_en.
    If \a is_x is true these are x kerns, otherwise they're y kerns. The
    input kerns are added to any existing kerns and then the entire
    input array is zeroed. */
    void           AddKerning(double * i_kern,int i_st,int i_en,bool is_x);

private:
    // types of control elements in the text
    enum control_type {
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

	// final touch on the raw_text, and setting utf8_text to point on tha appropriate location
    /** Assumes #flow_to_me's sources have already been added and uses the
    correspondance class's methods to create its destination string in
    #raw_text which omits control characters (cr, lf, tab and soft hyphens
    but not spaces). These control characters are instead added to the #ctrls
    array. Thus a fairly standard plain text/metadata separation is
    implemented. */
	void					 DoText(void);

    /** adds a new element representing a control character to the #ctrls
    array. */
	void           AddCtrl(int pos,control_type typ);

    /** adds two new elements to the #ctrls array which represent a span
    of text with the given start and end positions. The relationship between
    the two elements is recording using the other, is_start and is_end
    fields. */
	void           AddSpan(int st,int en,control_type typ);

    /** effectively calls AddSpan() with typ=ctrl_bidi then sets the
    \a i of one_ctrl::data of both new elements to \a rtl */
	void           AddBidiSpan(int st,int en,bool rtl);

    /** effectively calls AddSpan() then sets the \a v of one_ctrl::data of
    both new elements to \a i_l */
	void           AddVoidSpan(int st,int en,control_type typ,void* i_l);

    /** adds a new element to the end of the #boxes array and returns the
    index of this new element. */
	int            AddBox(one_flow_box &i_b);
	
    /** Any spans in the #ctrls array which are of type \a a_t and enclose
    a control of type \a b_t will be split into two spans, one on either
    side of the control, so that the control is no longer enclosed.
    Although it will work where \a b_t is any control, it is actually
    used to disentangle overlapping spans. */
	void           SplitSpan(control_type a_t,control_type b_t);

    /** removes the element of the #ctrls array with index \a no and its
    pair, if one exists. Note that no attempt is made to maintain any
    particular ordering within the array, ie the last element is copied into
    the hole created. See SortCtrl(). */
	void           SubCtrl(int no);
	
	// utility ctrl position functions
    /** \a n_st is an index into #ctrls. This method increments \a n_st
    until it indexes an item with the given \a typ. To search from the
    beginning set \a n_st = -1. Returns false and
    \a n_st == #nbCtrl if no items are found. NB NextStop() is \em not the
    opposite of NextStart(). */
	bool           NextStop(control_type typ,int &n_st);

    /** Returns the NextStop() which is also marked as the beginning of a
    span (see AddSpan() and friends). */
	bool           NextStart(control_type typ,int &n_st);

    /** Returns the NextStop() which is also marked as the end of a
    span (see AddSpan() and friends). */
	bool           NextEnd(control_type typ,int &n_st);

    /** Calls NextStart() and then returns the indexes of both that element
    and its pair. \a s_en is output-only. */
	void           NextSpanOfTyp(control_type typ,int &s_st,int &s_en);

    /** Returns the next span after s_st of the given \a typ which encloses
    the character position \a pos. \a s_st == #nbCtrl if none such is found. */
	void           NextSpanOfTyp(control_type typ,int pos,int &s_st,int &s_en);

    /** Returns in \a a the cumulative size in pixels of all the text in
    #raw_text from byte \a p_st to byte \a p_en (excluding the latter), as
    it would be rendered (all on one line) using the fonts assigned.
    If \a with_hyphen is true then the width of a hyphen is also added to
    the result. The kerning information is accounted for. \a b_offset is the
    character index of p_st, used to save time when looking into the kerning
    arrays. This method is used to fill the entries of the #boxes array - it
    does not use them to calculate the size.
    */
	void           MeasureText(int p_st,int p_en,box_sizes &a,void *pan,int b_offset,bool with_hyphen);

    /** computes the number of characters from the beginning of #raw_text to
    the byte index \a pos. */
	int            UCS4Offset(int pos);

    /** Alters the PangoAnalysis \a pan to contain the shaping engine,
    language engine, bidi level and language of the text between the given
    start and end bytes. If one or more of these is not constant over the
    whole range specified then that field is not updated (but the others are).
    \a from is probably an optimisation because the #ctrls array is only
    scanned starting at this index, but no callers ever pass anything but
    zero for this parameter. */
	void           UpdatePangoAnalysis(int from,int p_st,int p_en,void *pan);

	// kerning info, built when the text is collected.
	// if kern_x != NULL then kern_x covers all the text in this text_holder; idem for kern_y
	int						 nb_kern_x,nb_kern_y;
	double				 *kern_x,*kern_y;	

	// list of non-text stuff (control chars)
	// it is an array of boundaries or positions
	typedef struct one_ctrl {
		int          pos;      // position of this boundary/ element. position i means: 'at beginning of char i in #raw_text'
		bool         is_start; // is it a boundary that begins a span? (#other points to the end)
		bool         is_end;   // is it a boundary that ends a span? (#other points to the start)
		int					 other;    // if boundary of a span, index within #ctrls of the other boundary for this span
		control_type typ;      // type of the control: member of that enum at the top
		int          ind,inv;  // temporary variables to allow refreshing the field 'other' after sorting (see SortCtrl())
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

    friend int   CmpCtrl(const void* ia,const void* ib);
};


#endif
