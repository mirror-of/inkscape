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

#include <new>
#include <glib/glib.h>
#include <gc/gc_cpp.h>
#include "ast/branch-name.h"

namespace Inkscape {
namespace AST {

class Node;

class Path : public gc {
public:
    Path(Path const *parent, BranchName const &branch, unsigned pos,
         Node const &node)
    : _parent(parent), _branch(branch), _pos(pos), _node(node) {}

    Path const &reparent(Path const *parent) const throw(std::bad_alloc) {
        return *(new Path(parent, _branch, _pos, _node));
    }
    Path const &reseat(Node const &node) const throw(std::bad_alloc) {
        return *(new Path(_parent, _branch, _pos, node));
    }

    Path const *parent() const { return _parent; }
    Node const &node() const { return _node; }
    BranchName const &branch() const { return _branch; }
    unsigned pos() const { return _pos; }

private:
    void operator=(Path const &);

    Path const *_parent;
    BranchName _branch;
    unsigned _pos;
    Node const &_node;
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
