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

  This class is used an optional parameter to the constructor of flow_maker
  in order to specify the exact shapes in which to wrap the text.
  To use it, first set up #rgn_flow and #next_in_flow then, for each flow you
  wish to make, call Prepare(), construct and use a flow_maker, then call
  UnPrepare().

  It implements a stateful object used for retrieving sequential scanlines
  within the shapes.

  Consider a flow shape comprised of a circle with a rectangular 'hole'
  in the middle. When flow_maker is attempting to flow text inside this
  object it uses a loop which creates progressively longer lines of text
  until it locates one that needs to be wrapped. For each attempted line
  the ascender, descender and leading are calculated and passed to
  NextBox() which will calculate the location of the next available box
  that is capable of displaying a line with those properties.

  In our example, this will mean that for successive calls to NextBox() the
  following boxes will be returned:
  -# boxes which steadily increase in length
  -# one box to the left of the rectangular hole, one box to the right (this
    order is reversed in rtl mode) for each line around the rectangle
  -# shortening boxes below the hole
  -# boxes from #next_in_flow
*/
class flow_dest {
public:
    /** the convex hull of the flow shape. This member is provided solely
    for the convenience of the caller, it is not read from or written to
    except by AddShape(). The storage is allocated by flow_dest */
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
	
    /** clear the contents of #rgn_dest and #rgn_flow and reset them to
    empty shapes */
	void           Reset(void);

    /** effectively evaluates #rgn_dest = #rgn_dest (union) i_shape */
	void           AddShape(Shape const * i_shape);

    /** sets up the internal state to begin returning scanlines. Must be
    called before this structure is passed to flow_maker */
	void           Prepare(void);

    /** frees internal storage created for scanline processing. Must be
    called after flow_maker is complete */
	void           UnPrepare(void);
	
    /** internal libnrtype method. Returns the next logical box that can be
    used to store text with the given ascender, descender and leading. ltr
    version.
     \param after   A previous box_sol from this method, or an empty box_sol
                    for the first call
     \param asc     The ascender that needs to fit in the box
     \param desc    The descender that needs to fit in the box
     \param lead    The leading that needs to fit in the box
     \param skip    Move on to the next line. If this is false it means that
                    a font change later in the line has caused its size to
                    change and hence everything needs to be recalculated.
     \param stillSameLine (out) Will be set true if skip is false and there
                          was sufficient space on the current line for
                          \a min_length pixels
     \param min_length  The minimum width in pixels of the returned box.
    */
    box_sol        NextBox(box_sol const & after,double asc,double desc,double lead,bool skip,bool &stillSameLine,double min_length);
    
    /** internal libnrtype method. rtl version of NextBox(). ('txen' is
    'next' backwards) */
    box_sol        TxenBox(box_sol const & after,double asc,double desc,double lead,bool skip,bool &stillSameLine,double min_length);
    
    /** internal libnrtype method. Returns the total number of pixels
    remaining in all the possible boxes on this logical line. ltr version */
    double         RemainingOnLine(box_sol const & after);

    /** internal libnrtype method. rtl version of RemainingOnLine() */
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

    /** uses shaping operations to return a list of all runs in #rgn_flow
    at the given y coordinate. They are stored in tempLine. Note that
    results are cached because this method will frequently be called many
    times with the same parameters. */
	void					 ComputeLine(float y,float a,float d);
};

#endif
