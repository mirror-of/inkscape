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

#ifndef SEEN_INKSCAPE_AST_REFERENCE_H
#define SEEN_INKSCAPE_AST_REFERENCE_H

#include "ast/gc.h"
#include "ast/branch-name.h"
#include "ast/invalid-transformation.h"
#include "ast/node.h"

namespace Inkscape {
namespace AST {

class Path;
class Pad;

class Reference : public SimpleGCObject<> {
public:
    explicit Reference(Node const &node)
    throw()
    : _path(NULL), _node(node) {}

    Path const *path() const throw() { return _path; }
    Node const &node() const throw() { return _node; }
    Reference const *parent() const throw() {
        return _path ? &_path->parent() : NULL;
    }

    Reference const *lookup(BranchName const &branch, unsigned pos) const
    throw(InvalidBranch, std::bad_alloc);

    unsigned branchSize(BranchName const &branch) const
    throw(InvalidBranch)
    {
        return _node.branchSize(branch);
    }

    Reference const &insert(BranchName const &branch, unsigned pos,
                            Node const &node) const
    throw(InvalidBranch, InvalidTransformation, std::bad_alloc)
    {
        return reseat(_node.insert(branch, pos, node));
    }

    Reference const &replace(BranchName const &branch, unsigned pos,
                             Node const &node) const
    throw(InvalidBranch, InvalidTransformation, std::bad_alloc)
    {
        return reseat(_node.replace(branch, pos, node));
    }

    Reference const &remove(BranchName const &branch, unsigned pos) const
    throw(InvalidBranch, InvalidTransformation, std::bad_alloc)
    {
        return reseat(_node.remove(branch, pos));
    }

    Reference const &reorder(BranchName const &branch,
                             unsigned old_pos, unsigned new_pos) const
    throw(InvalidBranch, InvalidTransformation, std::bad_alloc)
    {
        return reseat(_node.reorder(branch, old_pos, new_pos));
    }

    Reference const &reparent(Reference const *parent) const
    throw(InvalidBranch, InvalidTransformation, std::bad_alloc);

    Reference const &reseat(Node const &node) const
    throw(InvalidBranch, InvalidTransformation, std::bad_alloc);

private:
    Reference(Path const &path, Node const &node)
    : _path(path), _node(node) {}

    void operator=(Path const &);

    Path *_path;
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
