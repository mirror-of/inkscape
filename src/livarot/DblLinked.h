/*
 *  DblLinked.h
 *  nlivarot
 *
 *  Created by fred on Mon Jun 16 2003.
 *
 */

#ifndef my_dbl_linked
#define my_dbl_linked

#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <string.h>
//#include <iostream.h>
#include "LivarotDefs.h"

/*
 * doubly-linked list class created in a "every atomic piece is a class" frenzy
 * it's only used in the AVL class, and doesn't provide any interesting functionality
 */

class DblLinked
{
public:
    // previous (left) and next (right) elements of the list
    DblLinked *elem[2];

    DblLinked (void);
   ~DblLinked (void);

   // constructor/destructor called when initializing an array created by a malloc()
   // this is because we'll use dynamically sized arrays, so new DblLinked[n] is not possible
  void MakeNew (void);
  void MakeDelete (void);

  // insertion in the list
  void InsertOnLeft (DblLinked * of);
  void InsertOnRight (DblLinked * of);
  void InsertBetween (DblLinked * l, DblLinked * r);
  // removal from the list
  void Extract (void);

  // changing the item position in the memory
  // called when a realloc is done on an array of elements derived from dbllinked
  // each element is relocated to a new pos, so this function updates the field that point to this class instance
  void Relocate (DblLinked * to);
};

#endif

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=c++:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :
