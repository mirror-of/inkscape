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

#include "ast/node.h"

namespace Inkscape {
namespace AST {

Node const *Node::_traverse(BranchName const &branch, unsigned pos) const {
    return NULL;
}

Node const &Node::_insertBefore(BranchName const &branch, unsigned pos,
                                Node const &node) const
throw(InvalidTransformation, std::bad_alloc)
{
    throw InvalidTransformation();
}

Node const &Node::_replaceWith(BranchName const &branch, unsigned pos,
                               Node const &node) const
throw(InvalidTransformation, std::bad_alloc)
{
    throw InvalidTransformation();
}

Node const &Node::_removeAt(BranchName const &branch, unsigned pos) const
throw(InvalidTransformation, std::bad_alloc)
{
    throw InvalidTransformation();
}

Node const &Node::_reorder(BranchName const &branch, unsigned old_pos,
                           unsigned new_pos) const
throw(InvalidTransformation, std::bad_alloc)
{
    throw InvalidTransformation();
}

void Node::_write(std::ostream &stream) const {
    stream << (char const *)toString();
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
