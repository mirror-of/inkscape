/*
 *  ShapeSweepUtils.cpp
 *  nlivarot
 *
 *  Created by fred on Wed Jun 18 2003.
 *
 */

#include <glib/gmem.h>
#include "Shape.h"
#include "LivarotDefs.h"
#include <libnr/nr-point.h>
#include <libnr/nr-point-fns.h>

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
      if (leftSweep->src->aretes[leftSweep->bord].st <
	  leftSweep->src->aretes[leftSweep->bord].en)
	{
	  leftSweep->src->pData[leftSweep->src->aretes[leftSweep->bord].en].
	    pending--;
	}
      else
	{
	  leftSweep->src->pData[leftSweep->src->aretes[leftSweep->bord].st].
	    pending--;
	}
      leftSweep->rightEvt = NULL;
    }
  if (rightSweep)
    {
      if (rightSweep->src->aretes[rightSweep->bord].st <
	  rightSweep->src->aretes[rightSweep->bord].en)
	{
	  rightSweep->src->pData[rightSweep->src->aretes[rightSweep->bord].
				 en].pending--;
	}
      else
	{
	  rightSweep->src->pData[rightSweep->src->aretes[rightSweep->bord].
				 st].pending--;
	}
      rightSweep->leftEvt = NULL;
    }
  leftSweep = rightSweep = NULL;
}

void
SweepEvent::CreateQueue (SweepEventQueue & queue, int size)
{
  queue.nbEvt = 0;
  queue.maxEvt = size;
  queue.events = (SweepEvent *) g_malloc(queue.maxEvt * sizeof (SweepEvent));
  queue.inds = (int *) g_malloc(queue.maxEvt * sizeof (int));
}

void
SweepEvent::DestroyQueue (SweepEventQueue & queue)
{
  g_free(queue.events);
  g_free(queue.inds);
  queue.nbEvt = queue.maxEvt = 0;
  queue.inds = NULL;
  queue.events = NULL;
}

SweepEvent *
SweepEvent::AddInQueue (SweepTree * iLeft, SweepTree * iRight, NR::Point &px, double itl, double itr,
			SweepEventQueue & queue)
{
  if (queue.nbEvt >= queue.maxEvt)
    return NULL;
  int n = queue.nbEvt++;
  queue.events[n].MakeNew (iLeft, iRight, px, itl, itr);

  if (iLeft->src->aretes[iLeft->bord].st < iLeft->src->aretes[iLeft->bord].en)
    {
      iLeft->src->pData[iLeft->src->aretes[iLeft->bord].en].pending++;
    }
  else
    {
      iLeft->src->pData[iLeft->src->aretes[iLeft->bord].st].pending++;
    }
  if (iRight->src->aretes[iRight->bord].st <
      iRight->src->aretes[iRight->bord].en)
    {
      iRight->src->pData[iRight->src->aretes[iRight->bord].en].pending++;
    }
  else
    {
      iRight->src->pData[iRight->src->aretes[iRight->bord].st].pending++;
    }

  queue.events[n].ind = n;
  queue.inds[n] = n;

  int curInd = n;
  while (curInd > 0)
    {
      int half = (curInd - 1) / 2;
      int no = queue.inds[half];
      if (px[1] < queue.events[no].posx[1]
	  || (px[1] == queue.events[no].posx[1] && px[0] < queue.events[no].posx[0]))
	{
	  queue.events[n].ind = half;
	  queue.events[no].ind = curInd;
	  queue.inds[half] = n;
	  queue.inds[curInd] = no;
	}
      else
	{
	  break;
	}
      curInd = half;
    }
  return queue.events + n;
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
bool
SweepEvent::PeekInQueue (SweepTree * &iLeft, SweepTree * &iRight, NR::Point &px, double &itl, double &itr,
			 SweepEventQueue & queue)
{
  if (queue.nbEvt <= 0)
    return false;
  iLeft = queue.events[queue.inds[0]].leftSweep;
  iRight = queue.events[queue.inds[0]].rightSweep;
  px = queue.events[queue.inds[0]].posx;
  itl = queue.events[queue.inds[0]].tl;
  itr = queue.events[queue.inds[0]].tr;
  return true;
}

bool
SweepEvent::ExtractFromQueue (SweepTree * &iLeft, SweepTree * &iRight,
                              NR::Point &px, double &itl, double &itr,
			      SweepEventQueue & queue)
{
  if (queue.nbEvt <= 0)
    return false;
  iLeft = queue.events[queue.inds[0]].leftSweep;
  iRight = queue.events[queue.inds[0]].rightSweep;
  px = queue.events[queue.inds[0]].posx;
  itl = queue.events[queue.inds[0]].tl;
  itr = queue.events[queue.inds[0]].tr;
  queue.events[queue.inds[0]].SupprFromQueue (queue);
  return true;
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

/*
 * the AVL tree holding the edges intersecting the sweepline
 * that structure is very sensitive to anything
 * you have edges stored in nodes, the nodes are sorted in increasing x-order of intersection
 * with the sweepline, you have the 2 potential intersections of the edge in the node with its
 * neighbours, plus the fact that it's stored in an array that's realloc'd
 */

SweepTree::SweepTree (void)
{
  src = NULL;
  bord = -1;
  startPoint = -1;
  leftEvt = rightEvt = NULL;
  sens = true;
//      invDirLength=1;
}
SweepTree::~SweepTree (void)
{
  MakeDelete ();
}

void
SweepTree::MakeNew (Shape * iSrc, int iBord, int iWeight, int iStartPoint)
{
  AVLTree::MakeNew ();
  ConvertTo (iSrc, iBord, iWeight, iStartPoint);
}

void
SweepTree::ConvertTo (Shape * iSrc, int iBord, int iWeight, int iStartPoint)
{
  src = iSrc;
  bord = iBord;
  leftEvt = rightEvt = NULL;
  startPoint = iStartPoint;
  if (src->aretes[bord].st < src->aretes[bord].en)
    {
      if (iWeight >= 0)
	sens = true;
      else
	sens = false;
    }
  else
    {
      if (iWeight >= 0)
	sens = false;
      else
	sens = true;
    }
//      invDirLength=src->eData[bord].isqlength;
//      invDirLength=1/sqrt(src->aretes[bord].dx*src->aretes[bord].dx+src->aretes[bord].dy*src->aretes[bord].dy);
}
void
SweepTree::MakeDelete (void)
{
  if (leftEvt)
    {
      leftEvt->rightSweep = NULL;
    }
  if (rightEvt)
    {
      rightEvt->leftSweep = NULL;
    }
  leftEvt = rightEvt = NULL;
  AVLTree::MakeDelete ();
}

void
SweepTree::CreateList (SweepTreeList & list, int size)
{
  list.nbTree = 0;
  list.maxTree = size;
  list.trees = (SweepTree *) g_malloc(list.maxTree * sizeof (SweepTree));
  list.racine = NULL;
}

void
SweepTree::DestroyList (SweepTreeList & list)
{
  g_free(list.trees);
  list.trees = NULL;
  list.nbTree = list.maxTree = 0;
  list.racine = NULL;
}

SweepTree *
SweepTree::AddInList (Shape * iSrc, int iBord, int iWeight, int iStartPoint,
		      SweepTreeList & list, Shape * iDst)
{
  if (list.nbTree >= list.maxTree)
    return NULL;
  int n = list.nbTree++;
  list.trees[n].MakeNew (iSrc, iBord, iWeight, iStartPoint);

  return list.trees + n;
}

// find the position at which node "newOne" should be inserted in the subtree rooted here
// we want to order with respect to the order of intersections with the sweepline, currently 
// lying at y=px[1].
// px is the upper endpoint of newOne
int
SweepTree::Find (NR::Point &px, SweepTree * newOne, SweepTree * &insertL,
		 SweepTree * &insertR, bool sweepSens)
{
  // get the edge associated with this node: one point+one direction
  // since we're dealing with line, the direction (bNorm) is taken downwards
  NR::Point bOrig, bNorm;
  bOrig = src->pData[src->aretes[bord].st].rx;
  bNorm = src->eData[bord].rdx;
  if (src->aretes[bord].st > src->aretes[bord].en)
    {
      bNorm = -bNorm;
    }
  // rotate to get the normal to the edge
  bNorm=bNorm.ccw();

  NR::Point diff;
  diff = px - bOrig;

  // compute (px-orig)^dir to know on which side of this edge the point px lies
  double y = 0;
  //      if ( startPoint == newOne->startPoint ) {
  //             y=0;
  //     } else {
  y = dot (bNorm, diff);
  //      }
  //      y*=invDirLength;
  if (fabs(y) < 0.000001)
    {
    // that damn point px lies on me, so i need to consider to direction of the edge in
    // newOne to know if it goes toward my left side or my right side
    // sweepSens is needed (actually only used by the Scan() functions) because if the sweepline goes upward,
    // signs change
      // prendre en compte les directions
      NR::Point nNorm;
      nNorm = newOne->src->eData[newOne->bord].rdx;
      if (newOne->src->aretes[newOne->bord].st >
	  newOne->src->aretes[newOne->bord].en)
	{
	  nNorm = -nNorm;
	}
      nNorm=nNorm.ccw();

      if (sweepSens)
	{
	  y = cross (nNorm, bNorm);
	}
      else
	{
	  y = cross (bNorm, nNorm);
	}
      if (y == 0)
	{
	  y = dot (bNorm, nNorm);
	  if (y == 0)
	    {
	      insertL = this;
	      insertR = static_cast < SweepTree * >(rightElem);
	      return found_exact;
	    }
	}
    }
  if (y < 0)
    {
      if (sonL)
	{
	  return (static_cast < SweepTree * >(sonL))->Find (px, newOne,
							    insertL, insertR,
							    sweepSens);
	}
      else
	{
	  insertR = this;
	  insertL = static_cast < SweepTree * >(leftElem);
	  if (insertL)
	    {
	      return found_between;
	    }
	  else
	    {
	      return found_on_left;
	    }
	}
    }
  else
    {
      if (sonR)
	{
	  return (static_cast < SweepTree * >(sonR))->Find (px, newOne,
							    insertL, insertR,
							    sweepSens);
	}
      else
	{
	  insertL = this;
	  insertR = static_cast < SweepTree * >(rightElem);
	  if (insertR)
	    {
	      return found_between;
	    }
	  else
	    {
	      return found_on_right;
	    }
	}
    }
  return not_found;
}

// only find a point's position
int
SweepTree::Find (NR::Point &px, SweepTree * &insertL,
		 SweepTree * &insertR)
{
  NR::Point bOrig, bNorm;
  bOrig = src->pData[src->aretes[bord].st].rx;
  bNorm = src->eData[bord].rdx;
  if (src->aretes[bord].st > src->aretes[bord].en)
    {
      bNorm = -bNorm;
    }
 bNorm=bNorm.ccw();

  NR::Point diff;
  diff = px - bOrig;

  double y = 0;
  y = dot (bNorm, diff);
  if (y == 0)
    {
      insertL = this;
      insertR = static_cast < SweepTree * >(rightElem);
      return found_exact;
    }
  if (y < 0)
    {
      if (sonL)
	{
	  return (static_cast < SweepTree * >(sonL))->Find (px, insertL,
							    insertR);
	}
      else
	{
	  insertR = this;
	  insertL = static_cast < SweepTree * >(leftElem);
	  if (insertL)
	    {
	      return found_between;
	    }
	  else
	    {
	      return found_on_left;
	    }
	}
    }
  else
    {
      if (sonR)
	{
	  return (static_cast < SweepTree * >(sonR))->Find (px, insertL,
							    insertR);
	}
      else
	{
	  insertL = this;
	  insertR = static_cast < SweepTree * >(rightElem);
	  if (insertR)
	    {
	      return found_between;
	    }
	  else
	    {
	      return found_on_right;
	    }
	}
    }
  return not_found;
}

void
SweepTree::RemoveEvents (SweepEventQueue & queue)
{
  RemoveEvent (queue, true);
  RemoveEvent (queue, false);
}

void
SweepTree::RemoveEvent (SweepEventQueue & queue, bool onLeft)
{
  if (onLeft)
    {
      if (leftEvt)
	{
	  leftEvt->SupprFromQueue (queue);
//                      leftEvt->MakeDelete(); // fait dans SupprFromQueue
	}
      leftEvt = NULL;
    }
  else
    {
      if (rightEvt)
	{
	  rightEvt->SupprFromQueue (queue);
//                      rightEvt->MakeDelete(); // fait dans SupprFromQueue
	}
      rightEvt = NULL;
    }
}
int
SweepTree::Remove (SweepTreeList & list, SweepEventQueue & queue,
		   bool rebalance)
{
  RemoveEvents (queue);
  AVLTree *tempR = static_cast < AVLTree * >(list.racine);
  int err = AVLTree::Remove (tempR, rebalance);
  list.racine = static_cast < SweepTree * >(tempR);
  MakeDelete ();
  if (list.nbTree <= 1)
    {
      list.nbTree = 0;
      list.racine = NULL;
    }
  else
    {
      if (list.racine == list.trees + (list.nbTree - 1))
	list.racine = this;
      list.trees[--list.nbTree].Relocate (this);
    }
  return err;
}

int
SweepTree::Insert (SweepTreeList & list, SweepEventQueue & queue,
		   Shape * iDst, int iAtPoint, bool rebalance, bool sweepSens)
{
  if (list.racine == NULL)
    {
      list.racine = this;
      return avl_no_err;
    }
  SweepTree *insertL = NULL;
  SweepTree *insertR = NULL;
  int insertion =
    list.racine->Find (iDst->pts[iAtPoint].x, this,
		       insertL, insertR, sweepSens);
  if (insertion == found_on_left)
    {
    }
  else if (insertion == found_on_right)
    {
    }
  else if (insertion == found_exact)
    {
      if (insertR)
	insertR->RemoveEvent (queue, true);
      if (insertL)
	insertL->RemoveEvent (queue, false);
//              insertL->startPoint=startPoint;
    }
  else if (insertion == found_between)
    {
      insertR->RemoveEvent (queue, true);
      insertL->RemoveEvent (queue, false);
    }

  //      if ( insertL ) cout << insertL->bord; else cout << "-1";
  //     cout << "  <   ";
  //     cout << bord;
  //     cout << "  <   ";
  //     if ( insertR ) cout << insertR->bord; else cout << "-1";
  //     cout << endl;
  AVLTree *tempR = static_cast < AVLTree * >(list.racine);
  int err =
    AVLTree::Insert (tempR, insertion, static_cast < AVLTree * >(insertL),
		     static_cast < AVLTree * >(insertR), rebalance);
  list.racine = static_cast < SweepTree * >(tempR);
  return err;
}

// insertAt() is a speedup on the regular sweepline: if the polygon contains a point of high degree, you
// get a set of edge that are to be added in the same position. thus you insert one edge with a regular insert(),
// and then insert all the other in a doubly-linked list fashion. this avoids the Find() call, but is O(d^2) worst-case
// where d is the number of edge to add in this fashion. hopefully d remains small

int
SweepTree::InsertAt (SweepTreeList & list, SweepEventQueue & queue,
		     Shape * iDst, SweepTree * insNode, int fromPt,
		     bool rebalance, bool sweepSens)
{
  if (list.racine == NULL)
    {
      list.racine = this;
      return avl_no_err;
    }

  NR::Point fromP;
  fromP = src->pData[fromPt].rx;
  NR::Point nNorm;
  nNorm = src->aretes[bord].dx;
  if (src->aretes[bord].st > src->aretes[bord].en)
    {
      nNorm = -nNorm;
    }
  if (sweepSens == false)
    {
      nNorm = -nNorm;
    }

  NR::Point bNorm;
  bNorm = insNode->src->aretes[insNode->bord].dx;
  if (insNode->src->aretes[insNode->bord].st >
      insNode->src->aretes[insNode->bord].en)
    {
      bNorm = -bNorm;
    }

  SweepTree *insertL = NULL;
  SweepTree *insertR = NULL;
  double ang = cross (nNorm, bNorm);
  if (ang == 0)
    {
      insertL = insNode;
      insertR = static_cast < SweepTree * >(insNode->rightElem);
    }
  else if (ang > 0)
    {
      insertL = insNode;
      insertR = static_cast < SweepTree * >(insNode->rightElem);

      while (insertL)
	{
	  if (insertL->src == src)
	    {
	      if (insertL->src->aretes[insertL->bord].st != fromPt
		  && insertL->src->aretes[insertL->bord].en != fromPt)
		{
		  break;
		}
	    }
	  else
	    {
	      int ils = insertL->src->aretes[insertL->bord].st;
	      int ile = insertL->src->aretes[insertL->bord].en;
	      if ((insertL->src->pData[ils].rx[0] != fromP[0]
		   || insertL->src->pData[ils].rx[1] != fromP[1])
		  && (insertL->src->pData[ile].rx[0] != fromP[0]
		      || insertL->src->pData[ile].rx[1] != fromP[1]))
		{
		  break;
		}
	    }
	  bNorm = insertL->src->aretes[insertL->bord].dx;
	  if (insertL->src->aretes[insertL->bord].st >
	      insertL->src->aretes[insertL->bord].en)
	    {
	      bNorm = -bNorm;
	    }
	  ang = cross (nNorm, bNorm);
	  if (ang <= 0)
	    {
	      break;
	    }
	  insertR = insertL;
	  insertL = static_cast < SweepTree * >(insertR->leftElem);
	}
    }
  else if (ang < 0)
    {
      insertL = insNode;
      insertR = static_cast < SweepTree * >(insNode->rightElem);

      while (insertR)
	{
	  if (insertR->src == src)
	    {
	      if (insertR->src->aretes[insertR->bord].st != fromPt
		  && insertR->src->aretes[insertR->bord].en != fromPt)
		{
		  break;
		}
	    }
	  else
	    {
	      int ils = insertR->src->aretes[insertR->bord].st;
	      int ile = insertR->src->aretes[insertR->bord].en;
	      if ((insertR->src->pData[ils].rx[0] != fromP[0]
		   || insertR->src->pData[ils].rx[1] != fromP[1])
		  && (insertR->src->pData[ile].rx[0] != fromP[0]
		      || insertR->src->pData[ile].rx[1] != fromP[1]))
		{
		  break;
		}
	    }
	  bNorm = insertR->src->aretes[insertR->bord].dx;
	  if (insertR->src->aretes[insertR->bord].st >
	      insertR->src->aretes[insertR->bord].en)
	    {
	      bNorm = -bNorm;
	    }
	  ang = cross (nNorm, bNorm);
	  if (ang > 0)
	    {
	      break;
	    }
	  insertL = insertR;
	  insertR = static_cast < SweepTree * >(insertL->rightElem);
	}
    }

  int insertion = found_between;
  if (insertL == NULL)
    insertion = found_on_left;
  if (insertR == NULL)
    insertion = found_on_right;
  if (insertion == found_on_left)
    {
    }
  else if (insertion == found_on_right)
    {
    }
  else if (insertion == found_exact)
    {
      if (insertR)
	insertR->RemoveEvent (queue, true);
      if (insertL)
	insertL->RemoveEvent (queue, false);
//              insertL->startPoint=startPoint;
    }
  else if (insertion == found_between)
    {
      insertR->RemoveEvent (queue, true);
      insertL->RemoveEvent (queue, false);
    }

  //      if ( insertL ) cout << insertL->bord; else cout << "-1";
  //     cout << "  <   ";
  //     cout << bord;
  //     cout << "  <   ";
  //     if ( insertR ) cout << insertR->bord; else cout << "-1";
  //     cout << endl;

  AVLTree *tempR = static_cast < AVLTree * >(list.racine);
  int err =
    AVLTree::Insert (tempR, insertion, static_cast < AVLTree * >(insertL),
		     static_cast < AVLTree * >(insertR), rebalance);
  list.racine = static_cast < SweepTree * >(tempR);
  return err;
}

void
SweepTree::Relocate (SweepTree * to)
{
  if (this == to)
    return;
  AVLTree::Relocate (to);
  to->src = src;
  to->bord = bord;
  to->sens = sens;
  to->leftEvt = leftEvt;
  to->rightEvt = rightEvt;
  to->startPoint = startPoint;
  if (src->swsData)
    src->swsData[bord].misc = to;
  if (src->swrData)
    src->swrData[bord].misc = to;
  if (leftEvt)
    leftEvt->rightSweep = to;
  if (rightEvt)
    rightEvt->leftSweep = to;
}

void
SweepTree::SwapWithRight (SweepTreeList & list, SweepEventQueue & queue)
{
  SweepTree *tL = this;
  SweepTree *tR = static_cast < SweepTree * >(rightElem);

  tL->src->swsData[tL->bord].misc = tR;
  tR->src->swsData[tR->bord].misc = tL;

  {
    Shape *swap = tL->src;
    tL->src = tR->src;
    tR->src = swap;
  }
  {
    int swap = tL->bord;
    tL->bord = tR->bord;
    tR->bord = swap;
  }
  {
    int swap = tL->startPoint;
    tL->startPoint = tR->startPoint;
    tR->startPoint = swap;
  }
//      {double swap=tL->invDirLength;tL->invDirLength=tR->invDirLength;tR->invDirLength=swap;}
  {
    bool swap = tL->sens;
    tL->sens = tR->sens;
    tR->sens = swap;
  }
}
void
SweepTree::Avance (Shape * dstPts, int curPoint, Shape * a, Shape * b)
{
  return;
/*	if ( curPoint != startPoint ) {
		int nb=-1;
		if ( sens ) {
//			nb=dstPts->AddEdge(startPoint,curPoint);
		} else {
//			nb=dstPts->AddEdge(curPoint,startPoint);
		}
		if ( nb >= 0 ) {
			dstPts->swsData[nb].misc=(void*)((src==b)?1:0);
			int   wp=waitingPoint;
			dstPts->eData[nb].firstLinkedPoint=waitingPoint;
			waitingPoint=-1;
			while ( wp >= 0 ) {
				dstPts->pData[wp].edgeOnLeft=nb;
				wp=dstPts->pData[wp].nextLinkedPoint;
			}
		}
		startPoint=curPoint;
	}*/
}
