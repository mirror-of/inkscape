/*
 * The reference corresponding to href of <use> element.
 *
 * Copyright (C) 2004 Bulia Byak
 * Copyright (C) 2004 Monash University
 *
 * Released under GNU GPL, read the file 'COPYING' for more information.
 */

#include <forward.h>
#include <sp-object.h>
#include <sp-use-reference.h>

bool SPUseReference::_acceptObject(SPObject * const obj) const
{
    if (SP_IS_ITEM(obj)) {
        SPObject * const owner = getOwner();
        /* Refuse references to us or to an ancestor. */
        for ( SPObject *iter = owner ; iter ; iter = SP_OBJECT_PARENT(iter) ) {
            if ( iter == obj ) {
                return false;
            }
        }
        return true;
    } else {
        return false;
    }
}


/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :
