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
  elem[LEFT] = elem[RIGHT] = NULL;
}

void
DblLinked::MakeDelete (void)
{
  Extract ();
}

void
DblLinked::InsertOnLeft (DblLinked * of)
{
  elem[RIGHT] = of;
  if (of)
    of->elem[LEFT] = this;
}

void
DblLinked::InsertOnRight (DblLinked * of)
{
  elem[LEFT] = of;
  if (of)
    of->elem[RIGHT] = this;
}

void
DblLinked::InsertBetween (DblLinked * l, DblLinked * r)
{
  if (l)
    l->elem[RIGHT] = this;
  if (r)
    r->elem[LEFT] = this;
  elem[LEFT] = l;
  elem[RIGHT] = r;
}

void
DblLinked::Extract (void)
{
  if (elem[LEFT])
    elem[LEFT]->elem[RIGHT] = elem[RIGHT];
  if (elem[RIGHT])
    elem[RIGHT]->elem[LEFT] = elem[LEFT];
  elem[LEFT] = elem[RIGHT] = NULL;
}

// the only possible links to this element are in elem[LEFT] or elem[RIGHT]
// the object retaining pointers to the list (like, say, first or last element) has to take care of the
// relocation himself. in practice, only the sweep code has to deal with DblLinked lists
void
DblLinked::Relocate (DblLinked * to)
{
  if (elem[LEFT])
    elem[LEFT]->elem[RIGHT] = to;
  if (elem[RIGHT])
    elem[RIGHT]->elem[LEFT] = to;
  to->elem[LEFT] = elem[LEFT];
  to->elem[RIGHT] = elem[RIGHT];
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
