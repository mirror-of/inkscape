/*
 * Inkscape::AST - Abstract Syntax Tree in a Database
 *
 * Authors:
 *   MenTaLguY <mental@rydia.net>
 *
 * Copyright (C) 2003 MenTaLguY
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <cstdlib>
#include "ast/xml/element.h"

namespace Inkscape {
namespace AST {
namespace XML {

XML::Node const *Element::child(unsigned pos) const {
    if ( pos < _child_count ) {
        ChildList const *iter;
        pos = _child_count - pos;
        for ( iter = _children ; iter && pos ; iter = iter->prev, pos-- );
        return iter ? iter->child : NULL;
    } else {
        return NULL;
    }
};

AST::Node const *_lookup(BranchName const &branch, unsigned pos) const
throw()
{
    if ( branch.axis() == PROPERTY ) {
        if ( branch.name() == NAME ) {
            return pos == 0 ? &_name : NULL;
        } else if ( branch.name() == CHILDREN ) {
            return NULL;
        } else {
            return NULL;
        }
    } else if ( branch.axis() == ATTRIBUTE ) {
        return NULL;
    } else {
        return NULL;
    }
}

};
};
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
