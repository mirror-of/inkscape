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

#ifndef SEEN_INKSCAPE_AST_PATH_H
#define SEEN_INKSCAPE_AST_PATH_H

#include "ast/gc.h"
#include "ast/branch-name.h"

namespace Inkscape {
namespace AST {

class Reference;
class Node;

class Path : public SimpleGCObject<> {
public:
    Path(Reference const &parent, BranchName const &branch, unsigned pos)
    throw()
    : _parent(parent), _branch(branch), _pos(pos) {}

    Node const *lookup() const throw();

    Reference const &parent() const throw() { return _reference; }
    BranchName const &branch() cnost throw() { return _branch; }
    unsigned pos() const throw() { return _pos; }

private:
    void operator=(Path const &);

    Reference const &_parent;
    BranchName const _branch;
    unsigned _pos;
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
