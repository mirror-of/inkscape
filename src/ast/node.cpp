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
#include "ast/path.h"

namespace Inkscape {
namespace AST {

Path const &Node::traverse(Path const *from,
                           BranchName const &branch, unsigned pos) const
throw(Node::NotFound, std::bad_alloc)
{
}

Node const *Node::_lookup(BranchName const &branch, unsigned pos) const throw() {
    return NULL;
}

Node const &Node::_insertBefore(BranchName const &branch, unsigned pos,
                                Node const &node) const
throw(Node::InvalidTransformation, std::bad_alloc)
{
    throw InvalidTransformation();
}

Node const &Node::_replaceWith(BranchName const &branch, unsigned pos,
                               Node const &node) const
throw(Node::InvalidTransformation, std::bad_alloc)
{
    throw InvalidTransformation();
}

Node const &Node::_removeAt(BranchName const &branch, unsigned pos) const
throw(Node::InvalidTransformation, std::bad_alloc)
{
    throw InvalidTransformation();
}

Node const &Node::_reorder(BranchName const &branch, unsigned old_pos,
                           unsigned new_pos) const
throw(Node::InvalidTransformation, std::bad_alloc)
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
