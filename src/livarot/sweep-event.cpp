#include <glib/gmem.h>
#include "libnr/nr-point.h"
#include "livarot/sweep-event.h"
#include "livarot/Shape.h"

SweepEventQueue::SweepEventQueue(int size) : nbEvt(0), maxEvt(size)
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

    iLeft = e.leftSweep;
    iRight = e.rightSweep;
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
    
    iLeft = e.leftSweep;
    iRight = e.rightSweep;
    px = e.posx;
    itl = e.tl;
    itr = e.tr;
    e.SupprFromQueue (*this);
    
    return true;
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
  leftSweep = iLeft;
  rightSweep = iRight;
  leftSweep->rightEvt = this;
  rightSweep->leftEvt = this;
}

void
SweepEvent::MakeDelete (void)
{
  if (leftSweep)
    {
      if (leftSweep->src->getEdge(leftSweep->bord).st <
	  leftSweep->src->getEdge(leftSweep->bord).en)
	{
	  leftSweep->src->pData[leftSweep->src->getEdge(leftSweep->bord).en].
	    pending--;
	}
      else
	{
	  leftSweep->src->pData[leftSweep->src->getEdge(leftSweep->bord).st].
	    pending--;
	}
      leftSweep->rightEvt = NULL;
    }
  if (rightSweep)
    {
      if (rightSweep->src->getEdge(rightSweep->bord).st <
	  rightSweep->src->getEdge(rightSweep->bord).en)
	{
	  rightSweep->src->pData[rightSweep->src->getEdge(rightSweep->bord).
				 en].pending--;
	}
      else
	{
	  rightSweep->src->pData[rightSweep->src->getEdge(rightSweep->bord).
				 st].pending--;
	}
      rightSweep->leftEvt = NULL;
    }
  leftSweep = rightSweep = NULL;
}


void
SweepEvent::SupprFromQueue (SweepEventQueue & queue)
{
  if (queue.nbEvt <= 1)
    {
      MakeDelete ();
      queue.nbEvt = 0;
      return;
    }
  int n = ind;
  int to = queue.inds[n];
  MakeDelete ();
  queue.events[--queue.nbEvt].Relocate (queue, to);

  int moveInd = queue.nbEvt;
  if (moveInd == n)
    return;
  to = queue.inds[moveInd];

  queue.events[to].ind = n;
  queue.inds[n] = to;

  int curInd = n;
  NR::Point px = queue.events[to].posx;
  bool didClimb = false;
  while (curInd > 0)
    {
      int half = (curInd - 1) / 2;
      int no = queue.inds[half];
      if (px[1] < queue.events[no].posx[1]
	  || (px[1] == queue.events[no].posx[1] && px[0] < queue.events[no].posx[0]))
	{
	  queue.events[to].ind = half;
	  queue.events[no].ind = curInd;
	  queue.inds[half] = to;
	  queue.inds[curInd] = no;
	  didClimb = true;
	}
      else
	{
	  break;
	}
      curInd = half;
    }
  if (didClimb)
    return;
  while (2 * curInd + 1 < queue.nbEvt)
    {
      int son1 = 2 * curInd + 1;
      int son2 = son1 + 1;
      int no1 = queue.inds[son1];
      int no2 = queue.inds[son2];
      if (son2 < queue.nbEvt)
	{
	  if (px[1] > queue.events[no1].posx[1]
	      || (px[1] == queue.events[no1].posx[1]
		  && px[0] > queue.events[no1].posx[0]))
	    {
	      if (queue.events[no2].posx[1] > queue.events[no1].posx[1]
		  || (queue.events[no2].posx[1] == queue.events[no1].posx[1]
		      && queue.events[no2].posx[0] > queue.events[no1].posx[0]))
		{
		  queue.events[to].ind = son1;
		  queue.events[no1].ind = curInd;
		  queue.inds[son1] = to;
		  queue.inds[curInd] = no1;
		  curInd = son1;
		}
	      else
		{
		  queue.events[to].ind = son2;
		  queue.events[no2].ind = curInd;
		  queue.inds[son2] = to;
		  queue.inds[curInd] = no2;
		  curInd = son2;
		}
	    }
	  else
	    {
	      if (px[1] > queue.events[no2].posx[1]
		  || (px[1] == queue.events[no2].posx[1]
		      && px[0] > queue.events[no2].posx[0]))
		{
		  queue.events[to].ind = son2;
		  queue.events[no2].ind = curInd;
		  queue.inds[son2] = to;
		  queue.inds[curInd] = no2;
		  curInd = son2;
		}
	      else
		{
		  break;
		}
	    }
	}
      else
	{
	  if (px[1] > queue.events[no1].posx[1]
	      || (px[1] == queue.events[no1].posx[1]
		  && px[0] > queue.events[no1].posx[0]))
	    {
	      queue.events[to].ind = son1;
	      queue.events[no1].ind = curInd;
	      queue.inds[son1] = to;
	      queue.inds[curInd] = no1;
	    }
	  break;
	}
    }
}


void
SweepEvent::Relocate (SweepEventQueue & queue, int to)
{
  if (queue.inds[ind] == to)
    return;			// j'y suis deja

  queue.events[to].posx = posx;
  queue.events[to].tl = tl;
  queue.events[to].tr = tr;
  queue.events[to].leftSweep = leftSweep;
  queue.events[to].rightSweep = rightSweep;
  leftSweep->rightEvt = queue.events + to;
  rightSweep->leftEvt = queue.events + to;
  queue.events[to].ind = ind;
  queue.inds[ind] = to;
}
