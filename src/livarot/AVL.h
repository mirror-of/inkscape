/*
 *  AVL.h
 *  nlivarot
 *
 *  Created by fred on Mon Jun 16 2003.
 *
 */

#ifndef my_avl
#define my_avl

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
//#include <iostream.h>

#include "DblLinked.h"

/*
 * base class providing AVL tree functionnality, that is binary balanced tree
 * there is no Find() function because the class only deal with topological info
 * subclasses of this class have to implement a Find(), and most certainly to 
 * override the Insert() function
 */

class AVLTree:public DblLinked
{
public:
  AVLTree * dad;
  AVLTree *sonL;
  AVLTree *sonR;

  int balance;

//      AVLTree*         leftElem;
//      AVLTree*         rightElem;

    AVLTree (void);
   ~AVLTree (void);

   // constructor/destructor meant to be called for an array of AVLTree created by malloc
  void MakeNew (void);
  void MakeDelete (void);

  // removal of the present element
  // racine is the tree's root; it's a reference because if the node is the root, removal of the node will change the root
  // rebalance==true if rebalancing is needed
  int Remove (AVLTree * &racine, bool rebalance = true);
  // called internally (should be private)
  // startNode is the node where the rebalancing starts; rebalancing then moves up the tree to the root
  // diff is the change in "subtree height", as needed for the rebalancing
  // racine is the reference to the root, since rebalancing can change it too
  int Remove (AVLTree * &racine, AVLTree * &startNode, int &diff);

  // insertion of the present node in the tree
  // insertType is the insertion type (defined in LivarotDefs.h: not_found, found_exact, found_on_left, etc)
  // insertL is the node in the tree that is immediatly before the current one, NULL is the present node goes to the 
  // leftmost position. if insertType == found_exact, insertL should be the node with ak key equal to that of the present node
  int Insert (AVLTree * &racine, int insertType, AVLTree * insertL,
	      AVLTree * insertR, bool rebalance);
  // insertion gruntwork.
  int Insert (AVLTree * &racine, int insertType, AVLTree * insertL,
	      AVLTree * insertR);

  // should return the leaf immediatly after this one. in practice, since elements are in a doubly-linked list,
  // this function returns leftElem.
  AVLTree *LeftLeaf (AVLTree * from, bool from_dad);
  // node immediatly after this one
  AVLTree *RightLeaf (AVLTree * from, bool from_dad);

  // left most node (ie, with smallest key) in the subtree of this node
  AVLTree *Leftmost (void);

  // rebalancing functions. both are recursive, but the depth of the trees we'll use should not be a problem
  // this one is for rebalancing after insertions
  int RestoreBalances (AVLTree * from, AVLTree * &racine);
  // this one is for removals
  int RestoreBalances (int diff, AVLTree * &racine);

  // called when this node is relocated to a new position in memory, to update pointers to him
  void Relocate (AVLTree * to);
};

#endif
