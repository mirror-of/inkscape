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

#ifndef SEEN_INKSCAPE_AST_PAD_H
#define SEEN_INKSCAPE_AST_PAD_H

#include "ast/invalid-branch.h"
#include "ast/invalid-transformation.h"
#include "ast/gc.h"

namespace Inkscape {
namespace AST {

class Reference;

class Pad : public FinalizedGCObject<::NoGC> {
public:
    explicit Pad(Node const &node)
    : _parent(NULL), _ref(*(new Reference(node))), _refcount(1) {}
    virtual ~Pad();

    void reference() const { _refcount++; }
    void unreference() const { if (!--_refcount) delete this; }

    Reference const *nodeRef() const { return _ref; }

protected:
    Reference const *_getRef() const {
        Reference const *parent_ref=( _parent ? _parent->_getRef() : NULL );
        if ( _ref->parent() != parent_ref ) {
            _ref = _ref->reparent_unsafe(parent_ref);
        }
        return _ref;
    }
    
private:
    Pad *_parent;
    Reference const *_ref;
    mutable unsigned _refcount;
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
