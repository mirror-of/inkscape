/*
 *  FlowDefs.h
 */

#ifndef my_flow_defs
#define my_flow_defs

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

class flow_dest;

/** \brief stores the outer dimensions of a set of characters

  internal to libnrtype. Holds all the information necessary to specify
  the bounding region of a complete line of text.

  Notice that a complete line of text (even across gaps) should have a
  common baseline. From there you then have an ascender, descender and
  leading, as normal, which are the maxima across all the fonts used on
  that line. The width is simply the cumulative width of all the
  characters.
*/
typedef struct box_sizes {
	double        ascent,descent,leading;
	double        width;
    /** number of letters in the box */
	int           nb_letter;
	
    /** append another box to this one to create a box large enough to
    completely enclose all the characters */
	void          Add(box_sizes const &x) {
		if ( x.ascent > ascent ) ascent=x.ascent;
		if ( x.descent > descent ) descent=x.descent;
		if ( x.ascent+x.leading > ascent+leading ) leading=x.ascent+x.leading-ascent;
		width+=x.width;
		nb_letter+=x.nb_letter;
	};
} box_sizes;

// sructure for one span of the region to fill
typedef struct box_sol {
  flow_dest    *frame;                    // region this span comes from
  double       y,ascent,descent,leading;  // info relating to vertical dimensions/position of the span
  double       x_start,x_end;             // info relating to the beginning/end of the span
  bool         before_rgn,after_rgn;			// before= before the first span; after= after the region
} box_sol;

// enum for the element's types in the flow routines
enum {
	flw_none     = 0,
	flw_text     = 1,   // text (content, actual text)
	flw_line_brk = 2,   // line break, or newline
	flw_rgn_brk  = 3,   // region break
	flw_div      = 4,   // flowDiv element
	flw_para		 = 5,
	flw_span     = 6,   // flowSpan element
	txt_text     = 7,   // text element (ie sp-text)
	txt_tline    = 8,   // tspan with sodipodi:role=line
	txt_firstline= 9,   // first tspan with sodipodi:role=line
	txt_span     = 10,  // normal tspan
	txt_textpath = 11   // textpath element
};


#endif
