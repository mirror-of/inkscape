/*
 *  FlowDefs.h
 */

#ifndef my_flow_defs
#define my_flow_defs

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

class flow_dest;

typedef struct box_sizes {
	double        ascent,descent,leading;
	double        width;
	int           nb_letter;
	
	void          Add(box_sizes &x) {
		if ( x.ascent > ascent ) ascent=x.ascent;
		if ( x.descent > descent ) descent=x.descent;
		if ( x.ascent+x.leading > ascent+leading ) leading=x.ascent+x.leading-ascent;
		width+=x.width;
		nb_letter+=x.nb_letter;
	};
} box_sizes;

// sructure for holding the spans to fill with text
typedef struct box_sol {
  flow_dest    *frame;                    // region this span comes from
  double       y,ascent,descent,leading;  // info relating to vertical dimensions/position of the span
  double       x_start,x_end;             // info relating to the beginning/end of the span
  bool         before_rgn,after_rgn;			// before= before the first span; after= after the region
} box_sol;


#endif
