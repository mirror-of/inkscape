#ifndef INKSCAPE_LIVAROT_SWEEP_TREE_LIST_H
#define INKSCAPE_LIVAROT_SWEEP_TREE_LIST_H

#include "libnr/nr-point.h"
#include "livarot/AVL.h"

class SweepTree;
class Shape;
class SweepEvent;
class SweepEventQueue;

// the sweepline: a set of edges intersecting the current sweepline
// stored as an AVL tree
class SweepTreeList
{
public:
  int nbTree, maxTree;		// number of nodes in the tree, max number of nodes
  SweepTree *trees;		// the array of nodes
  SweepTree *racine;		// root of the tree

  SweepTreeList(int s);
  ~SweepTreeList();
  
  SweepTree *add(Shape *iSrc, int iBord, int iWeight, int iStartPoint, Shape *iDst);
};

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

  // the find function that was missing in the AVLTrree class
  // the return values are defined in LivarotDefs.h
  int Find (NR::Point const &iPt, SweepTree * newOne, SweepTree * &insertL,
	    SweepTree * &insertR, bool sweepSens = true);
  int Find (NR::Point const &iPt, SweepTree * &insertL, SweepTree * &insertR);
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
