#include <glib/gmem.h>
#include "libnr/nr-point.h"
#include "livarot/sweep-event.h"
#include "livarot/sweep-event-queue.h"
#include "livarot/Shape.h"

SweepEventQueue::SweepEventQueue(int s) : nbEvt(0), maxEvt(s)
{
    /* FIXME: use new[] for this, but this causes problems when delete[]
    ** calls the SweepEvent destructors.
    */
    events = (SweepEvent *) g_malloc(maxEvt * sizeof(SweepEvent));
    inds = new int[maxEvt];
}

SweepEventQueue::~SweepEventQueue()
{
    g_free(events);
    delete []inds;
}

SweepEvent *SweepEventQueue::add(SweepTree *iLeft, SweepTree *iRight, NR::Point &px, double itl, double itr)
{
    if (nbEvt > maxEvt) {
	return NULL;
    }
    
    int const n = nbEvt++;
    events[n].MakeNew (iLeft, iRight, px, itl, itr);

    if (iLeft->src->getEdge(iLeft->bord).st < iLeft->src->getEdge(iLeft->bord).en) {
	iLeft->src->pData[iLeft->src->getEdge(iLeft->bord).en].pending++;
    } else {
	iLeft->src->pData[iLeft->src->getEdge(iLeft->bord).st].pending++;
    }
    
    if (iRight->src->getEdge(iRight->bord).st < iRight->src->getEdge(iRight->bord).en) {
	iRight->src->pData[iRight->src->getEdge(iRight->bord).en].pending++;
    } else {
	iRight->src->pData[iRight->src->getEdge(iRight->bord).st].pending++;
    }

    events[n].ind = n;
    inds[n] = n;

    int curInd = n;
    while (curInd > 0) {
	int const half = (curInd - 1) / 2;
	int const no = inds[half];
	if (px[1] < events[no].posx[1]
	    || (px[1] == events[no].posx[1] && px[0] < events[no].posx[0]))
	{
	    events[n].ind = half;
	    events[no].ind = curInd;
	    inds[half] = n;
	    inds[curInd] = no;
	} else {
	    break;
	}
	
	curInd = half;
    }
  
    return events + n;
}



bool SweepEventQueue::peek(SweepTree * &iLeft, SweepTree * &iRight, NR::Point &px, double &itl, double &itr)
{
    if (nbEvt <= 0) {
	return false;
    }
    
    SweepEvent const &e = events[inds[0]];

    iLeft = e.sweep[LEFT];
    iRight = e.sweep[RIGHT];
    px = e.posx;
    itl = e.tl;
    itr = e.tr;
    
    return true;
}

bool SweepEventQueue::extract(SweepTree * &iLeft, SweepTree * &iRight, NR::Point &px, double &itl, double &itr)
{
    if (nbEvt <= 0) {
	return false;
    }

    SweepEvent &e = events[inds[0]];
    
    iLeft = e.sweep[LEFT];
    iRight = e.sweep[RIGHT];
    px = e.posx;
    itl = e.tl;
    itr = e.tr;
    remove(&e);
    
    return true;
}


void SweepEventQueue::remove(SweepEvent *e)
{
    if (nbEvt <= 1) {
	e->MakeDelete ();
	nbEvt = 0;
	return;
    }
    
    int const n = e->ind;
    int to = inds[n];
    e->MakeDelete();
    relocate(&events[--nbEvt], to);

    int const moveInd = nbEvt;
    if (moveInd == n) {
	return;
    }
    
    to = inds[moveInd];

    events[to].ind = n;
    inds[n] = to;

    int curInd = n;
    NR::Point const px = events[to].posx;
    bool didClimb = false;
    while (curInd > 0) {
	int const half = (curInd - 1) / 2;
	int const no = inds[half];
	if (px[1] < events[no].posx[1]
	    || (px[1] == events[no].posx[1] && px[0] < events[no].posx[0]))
	{
	  events[to].ind = half;
	  events[no].ind = curInd;
	  inds[half] = to;
	  inds[curInd] = no;
	  didClimb = true;
	} else {
	    break;
	}
	curInd = half;
    }
    
    if (didClimb) {
	return;
    }
    
    while (2 * curInd + 1 < nbEvt) {
	int const son1 = 2 * curInd + 1;
	int const son2 = son1 + 1;
	int const no1 = inds[son1];
	int const no2 = inds[son2];
	if (son2 < nbEvt) {
	    if (px[1] > events[no1].posx[1]
		|| (px[1] == events[no1].posx[1]
		    && px[0] > events[no1].posx[0]))
	    {
		if (events[no2].posx[1] > events[no1].posx[1]
		    || (events[no2].posx[1] == events[no1].posx[1]
			&& events[no2].posx[0] > events[no1].posx[0]))
		{
		    events[to].ind = son1;
		    events[no1].ind = curInd;
		    inds[son1] = to;
		    inds[curInd] = no1;
		    curInd = son1;
		} else {
		    events[to].ind = son2;
		    events[no2].ind = curInd;
		    inds[son2] = to;
		    inds[curInd] = no2;
		    curInd = son2;
		}
	    } else {
		if (px[1] > events[no2].posx[1]
		    || (px[1] == events[no2].posx[1]
			&& px[0] > events[no2].posx[0]))
		{
		    events[to].ind = son2;
		    events[no2].ind = curInd;
		    inds[son2] = to;
		    inds[curInd] = no2;
		    curInd = son2;
		} else {
		    break;
		}
	    }
	} else {
	    if (px[1] > events[no1].posx[1]
		|| (px[1] == events[no1].posx[1]
		    && px[0] > events[no1].posx[0]))
	    {
		events[to].ind = son1;
		events[no1].ind = curInd;
		inds[son1] = to;
		inds[curInd] = no1;
	    }
	    
	    break;
	}
    }
}




void SweepEventQueue::relocate(SweepEvent *e, int to)
{
    if (inds[e->ind] == to) {
	return;			// j'y suis deja
    }

    events[to] = *e;

    e->sweep[LEFT]->evt[RIGHT] = events + to;
    e->sweep[RIGHT]->evt[LEFT] = events + to;
    inds[e->ind] = to;
}


/*
 * a simple binary heap
 * it only contains intersection events
 * the regular benley-ottman stuffs the segment ends in it too, but that not needed here since theses points
 * are already sorted. and the binary heap is much faster with only intersections...
 * the code sample on which this code is based comes from purists.org
 */
SweepEvent::SweepEvent ()
{
  NR::Point dummy(0,0);
  MakeNew (NULL, NULL,dummy, 0, 0);
}
SweepEvent::~SweepEvent (void)
{
  MakeDelete ();
}

void
SweepEvent::MakeNew (SweepTree * iLeft, SweepTree * iRight, NR::Point &px, double itl, double itr)
{
  ind = -1;
  posx = px;
  tl = itl;
  tr = itr;
  sweep[LEFT] = iLeft;
  sweep[RIGHT] = iRight;
  sweep[LEFT]->evt[RIGHT] = this;
  sweep[RIGHT]->evt[LEFT] = this;
}

void
SweepEvent::MakeDelete (void)
{
  if (sweep[LEFT])
    {
      if (sweep[LEFT]->src->getEdge(sweep[LEFT]->bord).st <
	  sweep[LEFT]->src->getEdge(sweep[LEFT]->bord).en)
	{
	  sweep[LEFT]->src->pData[sweep[LEFT]->src->getEdge(sweep[LEFT]->bord).en].
	    pending--;
	}
      else
	{
	  sweep[LEFT]->src->pData[sweep[LEFT]->src->getEdge(sweep[LEFT]->bord).st].
	    pending--;
	}
      sweep[LEFT]->evt[RIGHT] = NULL;
    }
  if (sweep[RIGHT])
    {
      if (sweep[RIGHT]->src->getEdge(sweep[RIGHT]->bord).st <
	  sweep[RIGHT]->src->getEdge(sweep[RIGHT]->bord).en)
	{
	  sweep[RIGHT]->src->pData[sweep[RIGHT]->src->getEdge(sweep[RIGHT]->bord).
				 en].pending--;
	}
      else
	{
	  sweep[RIGHT]->src->pData[sweep[RIGHT]->src->getEdge(sweep[RIGHT]->bord).
				 st].pending--;
	}
      sweep[RIGHT]->evt[LEFT] = NULL;
    }
  sweep[LEFT] = sweep[RIGHT] = NULL;
}
