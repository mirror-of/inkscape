#ifndef INKSCAPE_LIVAROT_SWEEP_EVENT_H
#define INKSCAPE_LIVAROT_SWEEP_EVENT_H
/** \file */

#include <libnr/nr-point.h>
#include <livarot/sweep-event-queue.h>  /* TODO: Remove this #include. */
class SweepTree;


/** One intersection event. */
class SweepEvent
{
public:
  SweepTree * leftSweep;	// sweep element associated with the left edge of the intersection
  SweepTree *rightSweep;	// sweep element associated with the right edge 

  NR::Point posx;		// coordinates of the intersection
  double tl, tr;			// coordinates of the intersection on the left edge (tl) and on the right edge (tr)

  int ind;			// index in the binary heap

    SweepEvent (void);		// not used
   ~SweepEvent (void);		// not used

  // inits a SweepEvent structure
   void MakeNew (SweepTree * iLeft, SweepTree * iRight, NR::Point &iPt,
		double itl, double itr);
  // voids a SweepEvent structure
  void MakeDelete (void);
};


#endif /* !INKSCAPE_LIVAROT_SWEEP_EVENT_H */

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=99 :
