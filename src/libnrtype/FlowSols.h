/*
 *  FlowSrc.h
 */

#ifndef my_flow_sols
#define my_flow_sols

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "FlowDefs.h"

class text_holder;
class text_style;
class flow_eater;

/** \brief internal libnrtype class

internal libnrtype class. Stores potential solutions for the line wrapping
algorithms and somewhat assists in their calculation (with the aid of
text_holder).

Terminology:
 - solution: the complete bounding box of a line as stored by a box_sizes
   class.
*/
class line_solutions {
public:
	int            elem_st,pos_st;
	
	typedef struct one_sol {
		int          elem_no;    ///index into flow_src::elems
		int          pos;        ///index into text_holder::boxes
		box_sizes    meas;
		bool         para_end;
		bool         rgn_end;
	} one_sol;
	int						 nbSol,maxSol;
	one_sol*       sols;
	
    /** The longest solution seen to date. */
	one_sol				 style_end_sol;
    /** Whether #style_end_sol contains valid data. */
	bool           style_end_set;
	int						 style_end_no,style_end_pos;
	double         style_end_ascent,style_end_descent,style_end_leading;

    /** The last call to PushBox() contained a font change such that the
    baseline of the line needs to be changed and the whole line
    recalculated. */
	bool           style_ending;

    /** Set when the last entry in #sols is sufficiently long that
    justification (that is within the bounds set by NewLine()) would
    require squeezing the characters together. */
    bool           no_style_ending;
	
	bool           in_leading_white;
	int            min_line_no,min_line_pos;
	
	
	line_solutions(void);
	~line_solutions(void);
	
    /** TODO. Resets a load of variables in preparation. Empties #sols.
     \param min  The minimum total length available for the line, in pixels
     \param max  The maximum total length available for the line, in pixels.
                 These are different to allow for justification, which can
                 squish or stretch character spacing by the given amount.
                 \a max is therefore the longest line, unadjusted for
                 justification, which can be used and thus indicates the
                 greatest possible squishing. Similarly, \a min indicates
                 the greatest possible expansion.
     \param typ  The actual line length available, in pixels, with none of
                 this justification nonsense.
     \param strict_bef  If true, then only solutions which are longer than
                        \a min are stored, otherwise all possible lengths
                        are stored.
     \param strict_aft  If true, then only solutions which are shorter than
                        \a max are stored, otherwise all possible lengths
                        are stored (until you stop calling PushBox()).
    */
	void            NewLine(double min,double max,double typ,bool strict_bef,bool strict_aft);

    /** Set the position of the baseline of the current line. This is used by
    PushBox() to detect when larger text comes in that would necessitate
    moving the baseline.
    */
	void            SetLineSizes(double ascent,double descent,double leading);

    /** Clears out nearly everything, including #sols, but not what was
    set by NewLine(). */
	void            StartLine(int at_elem,int at_pos);

    /** TODO. Short. No clue */
	void            EndLine(void);
	
	/** TODO. Sets #cur_line = #last_word */
    void            StartWord(void);

    /** Processes the given box and creates any new solutions in #sols and
    #style_end_sol that fit the criteria specified in NewLine(). 
    A box is the text_holder definition of a box.
     \param s        the box to add.
     \param end_no   ?An index into a flow_src::elems array.
     \param end_pos  An index into a text_holder::boxes array.
     \param is_white Set if \a end_pos points to a box containing only
                     whitespace.
     \param last_in_para This is the last box in a paragraph.
     \param last_in_rgn  This is the last box in a region. A region is a
                         single flow_dest class.
     \param is_word  Update #last_word.
     \return  true if no more data will fit on the current line, given the
           parameters set with NewLine().
    */
	bool            PushBox(box_sizes const &s,int end_no,int end_pos,bool is_white,bool last_in_para,bool last_in_rgn,bool is_word);

    /** explicitly adds an item to #sols with the given parameters. The size
    is taken from the current cumulative size of the line (#cur_line). */
	void            ForceSol(int end_no,int end_pos,bool last_in_para,bool last_in_rgn);
	
    /** debug method. Dumps the contents of the #sols array to stdout. */
	void            Affiche(void);

private:
	double         min_length,max_length,typ_length;
	bool           noBefore,noAfter;
	double         l_ascent,l_descent,l_leading;
	
    /** the size of the line up to the last call to StartWord(), ie usually
    the size excluding the last word added. */
	box_sizes			 cur_line;
    /** the sum of all the boxes passed to PushBox() (see box_sizes::Add()). */
    box_sizes      last_line;
    /** the size of the line up to the last call to PushBox() with \a is_word true. */
    box_sizes      last_word;
    /** TODO: what are these for? they seem evil. */
	int            before_state,after_state;
};

#endif
