/*
 *  ShapeUtils.h
 *  nlivarot
 *
 *  Created by fred on Sun Jul 20 2003.
 *
 */

#ifndef my_shape_utils
#define my_shape_utils

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
//#include <iostream.h>

#include "LivarotDefs.h"
#include "AVL.h"

#include <libnr/nr-point.h>

/*
 * utiliies for the sweepline algorithms used on polygons 
 */


class SweepTree;
class SweepEvent;
class Shape;

// the structure to hold the intersections events encountered during the sweep
// it's an array of SweepEvent (not allocated with "new SweepEvent[n]" but with a malloc)
// there's a list of indices because it's a binary heap: inds[i] tell that events[inds[i]] has position i in the
// heap
// each SweepEvent has a field to store its index in the heap, too
typedef struct SweepEventQueue
{
  int nbEvt, maxEvt;		// number of events currently in the heap, allocated size of the heap
  int *inds;			// indices
  SweepEvent *events;		// events
}
SweepEventQueue;

// one intersection event
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

  // create the structure to store the binary heap
  static void CreateQueue (SweepEventQueue & queue, int size);
  // destroy the structure
  static void DestroyQueue (SweepEventQueue & queue);
  // add one intersection in the binary heap
  static SweepEvent *AddInQueue (SweepTree * iLeft, SweepTree * iRight,
				 NR::Point &iPt, double itl, double itr,
				 SweepEventQueue & queue);
  // the calling SweepEvent removes itself from the binary heap
  void SupprFromQueue (SweepEventQueue & queue);
  // look for the topmost intersection in the heap
  static bool PeekInQueue (SweepTree * &iLeft, SweepTree * &iRight, NR::Point &oPt, double &itl, double &itr,
			   SweepEventQueue & queue);
  // extract the topmost intersection from the heap
  static bool ExtractFromQueue (SweepTree * &iLeft, SweepTree * &iRight,
				NR::Point &oPt, double &itl, double &itr,
				SweepEventQueue & queue);

  // misc: change a SweepEvent structure's postion in the heap
  void Relocate (SweepEventQueue & queue, int to);
};

// the sweepline: a set of edges intersecting the current sweepline
// stored as an AVL tree
typedef struct SweepTreeList
{
  int nbTree, maxTree;		// number of nodes in the tree, max number of nodes
  SweepTree *trees;		// the array of nodes
  SweepTree *racine;		// root of the tree
}
SweepTreeList;

// one node in the AVL tree of edges
// note that these nodes will be stored in a dynamically allocated array, hence the Relocate() function
class SweepTree:public AVLTree
{
public:
  SweepEvent * leftEvt;		// intersection with the edge on the left (if any)
  SweepEvent *rightEvt;		// intersection with the edge on the right (if any)

  Shape *src;			// Shape from which the edge comes (when doing boolean operation on polygons, edges can come
  // from 2 different polygons)
  int bord;			// edge index in the Shape
  bool sens;			// true= top->bottom; false= bottom->top
  int startPoint;		// point index in the result Shape associated with the upper end of the edge

    SweepTree (void);
   ~SweepTree (void);

   // inits a brand new node
  void MakeNew (Shape * iSrc, int iBord, int iWeight, int iStartPoint);
  // changes the edge associated with this node
  // goal: reuse the node when an edge follows another, which is the most common case
  void ConvertTo (Shape * iSrc, int iBord, int iWeight, int iStartPoint);
  // delete the contents of node
  void MakeDelete (void);

  // utilites
  static void CreateList (SweepTreeList & list, int size);
  static void DestroyList (SweepTreeList & list);
  static SweepTree *AddInList (Shape * iSrc, int iBord, int iWeight,
			       int iStartPoint, SweepTreeList & list,
			       Shape * iDst);

  // the find function that was missing in the AVLTrree class
  // the return values are defined in LivarotDefs.h
  int Find (NR::Point &iPt, SweepTree * newOne, SweepTree * &insertL,
	    SweepTree * &insertR, bool sweepSens = true);
  int Find (NR::Point &iPt, SweepTree * &insertL, SweepTree * &insertR);
  // removes sweepevents attached to this node
  void RemoveEvents (SweepEventQueue & queue);
  // onLeft=true: only remove left sweepevent
  // onLeft=false: only remove right sweepevent
  void RemoveEvent (SweepEventQueue & queue, bool onLeft);
  // overrides of the AVLTree functions, to account for the sorting in the tree
  // and some other stuff
  int Remove (SweepTreeList & list, SweepEventQueue & queue, bool rebalance =
	      true);
  int Insert (SweepTreeList & list, SweepEventQueue & queue, Shape * iDst,
	      int iAtPoint, bool rebalance = true, bool sweepSens = true);
  int InsertAt (SweepTreeList & list, SweepEventQueue & queue, Shape * iDst,
		SweepTree * insNode, int fromPt, bool rebalance =
		true, bool sweepSens = true);
  // swap nodes, or more exactly, swap the edges in them
  void SwapWithRight (SweepTreeList & list, SweepEventQueue & queue);

  void Avance (Shape * dst, int nPt, Shape * a, Shape * b);

  void Relocate (SweepTree * to);
};

#endif
