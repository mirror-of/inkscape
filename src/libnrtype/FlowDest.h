/*
 *  FlowDest.h
 */

#ifndef my_flow_dest
#define my_flow_dest

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "FlowDefs.h"

class Path;
class Shape;
class flow_dest;
class FloatLigne;

/**
  \brief  Defines a sequence of shapes used for flowing text

  This class is used an optional parameter to the constructor of \a flow_maker
  in order to specify the exact shapes in which to wrap the text.
  To use it, first set up \a rgn_flow and \a next_in_flow then, for each flow you
  wish to make, call \a Prepare, construct and use a \a flow_maker, then call
  \a UnPrepare.

  It implements a stateful object used for retrieving sequential scanlines
  within the shapes.

  Consider a flow shape comprised of a circle with a rectangular 'hole'
  in the middle. When \a flow_maker is attempting to flow text inside this
  object it uses a loop which creates progressively longer lines of text
  until it locates one that needs to be wrapped. For each attempted line
  the ascender, descender and leading are calculated and passed to
  \a NextBox which will calculate the location of the next available box
  that is capable of displaying a line with those properties.

  In our example, this will mean that for successive calls to \a NextBox the
  following boxes will be returned:
  - boxes which steadily increase in length
  - one box to the left of the rectangular hole, one box to the right (this
    order is reversed in rtl mode) for each line around the rectangle
  - shortening boxes below the hole
  - boxes from \a next_in_flow
*/
class flow_dest {
public:
    /** the convex hull of the flow shape. This member is provided solely
    for the convenience of the caller, it is not read from or written to
    except by \a AddShape. The storage is allocated by flow_dest */
	Shape*       rgn_dest;

    /** the fully defined shape to use for flowing. The storage is allocated
    by flow_dest */
	Shape*       rgn_flow;

    /** text can be flowed sequentially in a series of shapes, like linked
    frames in a DTP package. next_in_flow specifies the next flow_dest after
    this one. It can be NULL. Storage is allocated by the caller */
    flow_dest*   next_in_flow;
	
	flow_dest(void);
	~flow_dest(void);
	
    /** clear the contents of \a rgn_dest and \a rgn_flow and reset them to
    empty shapes */
	void           Reset(void);

    /** effectively evaluates rgn_dest = rgn_dest <union> i_shape */
	void           AddShape(Shape const * i_shape);

    /** sets up the internal state to begin returning scanlines. Must be
    called before this structure is passed to \a flow_maker */
	void           Prepare(void);

    /** frees internal storage created for scanline processing. Must be
    called after \a flow_maker is complete */
	void           UnPrepare(void);
	
    /** internal libnrtype method. Returns the next logical box that can be
    used to store text with the given ascender, descender and leading. ltr
    version.*/
    box_sol        NextBox(box_sol const & after,double asc,double desc,double lead,bool skip,bool &stillSameLine,double min_length);
    
    /** internal libnrtype method. rtl version of \a NextBox. ('txen' is
    'next' backwards) */
    box_sol        TxenBox(box_sol const & after,double asc,double desc,double lead,bool skip,bool &stillSameLine,double min_length);
    
    /** internal libnrtype method. Returns the total number of pixels
    remaining in all the possible boxes on this logical line. ltr version */
    double         RemainingOnLine(box_sol const & after);

    /** internal libnrtype method. rtl version of \a RemainingOnLine */
    double         RemainingOnEnil(box_sol const & after);
	
private:
	double       l,t,r,b;
	int          curPt;
	float        curY;

	typedef struct cached_line {
		int          date;
		FloatLigne*  theLine;
		double       y,a,d;
	} cached_line;
	int                   lastDate; // date for the cache
	int                   maxCache,nbCache;
	cached_line*          caches;  
	FloatLigne*           tempLine;
	FloatLigne*           tempLine2;

    /** uses shaping operations to return a list of all runs in \a rgn_flow
    at the given y coordinate. They are stored in tempLine. Note that
    results are cached because this method will frequently be called many
    times with the same parameters. */
	void					 ComputeLine(float y,float a,float d);
};

#endif
