/*
 *  DblLinked.cpp
 *  nlivarot
 *
 *  Created by fred on Mon Jun 16 2003.
 *
 */

#include "DblLinked.h"

DblLinked::DblLinked (void)
{
  MakeNew ();
}
DblLinked::~DblLinked (void)
{
  MakeDelete ();
}

void
DblLinked::MakeNew (void)
{
  leftElem = rightElem = NULL;
}

void
DblLinked::MakeDelete (void)
{
  Extract ();
}

void
DblLinked::InsertOnLeft (DblLinked * of)
{
  rightElem = of;
  if (of)
    of->leftElem = this;
}

void
DblLinked::InsertOnRight (DblLinked * of)
{
  leftElem = of;
  if (of)
    of->rightElem = this;
}

void
DblLinked::InsertBetween (DblLinked * l, DblLinked * r)
{
  if (l)
    l->rightElem = this;
  if (r)
    r->leftElem = this;
  leftElem = l;
  rightElem = r;
}

void
DblLinked::Extract (void)
{
  if (leftElem)
    leftElem->rightElem = rightElem;
  if (rightElem)
    rightElem->leftElem = leftElem;
  leftElem = rightElem = NULL;
}

void
DblLinked::Relocate (DblLinked * to)
{
  if (leftElem)
    leftElem->rightElem = to;
  if (rightElem)
    rightElem->leftElem = to;
  to->leftElem = leftElem;
  to->rightElem = rightElem;
}

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
