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
#include <stdint.h>
#include <string.h>
//#include <iostream.h>

class DblLinked
{
public:
  DblLinked * leftElem;
  DblLinked *rightElem;

    DblLinked (void);
   ~DblLinked (void);

  void MakeNew (void);
  void MakeDelete (void);

  void InsertOnLeft (DblLinked * of);
  void InsertOnRight (DblLinked * of);
  void InsertBetween (DblLinked * l, DblLinked * r);
  void Extract (void);

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
