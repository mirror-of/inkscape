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

#include "ast/reference.h"

namespace Inkscape {
namespace AST {

class ChildReference : public Reference {
protected:
    ChildReference(Reference const &parent, BranchName const &branch,
                   unsigned pos, Node const &node) throw()
    : _private_path(parent, branch, pos), Reference(&_private_path, node) {}

private:
    Path _private_path;

    friend class Reference;
};

Reference const *Reference::lookup(BranchName const &branch, unsigned pos)
const
throw(InvalidBranch, std::bad_alloc)
{
    Node const *node=_node.lookup(branch, pos);
    if (node) {
        return *(new ChildReference(*this, branch, pos, *node));
    } else {
        return NULL;
    }
}

Reference const &Reference::reseat(Node const &node) const
throw(InvalidBranch, InvalidTransformation, std::bad_alloc)
{
    if (_path) {
        Reference const &parent=_path->parent();

        Node const &new_parent_node=parent._node.replace(_path->branch(), _path->pos(), node);
        Reference const &new_parent=parent.reseat(new_parent_node);

        return *(new ChildReference(new_parent, _path->branch(), _path->pos(), node));
    } else {
        return *(new Reference(node));
    }
}


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
