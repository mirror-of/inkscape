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

#ifndef SEEN_INKSCAPE_AST_NODE_H
#define SEEN_INKSCAPE_AST_NODE_H

#include <new>
#include <cstdlib>
#include <iosfwd>
#include <gc/gc_cpp.h>
#include <glib/glib.h>
#include "ast/branch-name.h"
#include "ast/invalid-transformation.h"

namespace Inkscape {
namespace AST {

class String;

class Node : public gc {
public:
    virtual ~Node();

    class InvalidTransformation : public std::runtime_error {
    public:
        InvalidTransformation() : runtime_error("invalid transformation") {}
    };

    Node const *traverse(BranchName const &branch, unsigned pos) const
    throw()
    {
        return _traverse(branch, pos);
    }

    Node const &insertBefore(BranchName const &branch, unsigned pos,
                             Node const &node) const
    throw(InvalidTransformation, std::bad_alloc)
    {
        return _insertBefore(branch, pos, node);
    }

    Node const &replaceWith(BranchName const &branch, unsigned pos,
                            Node const &node) const
    throw(InvalidTransformation, std::bad_alloc)
    {
        return _replaceWith(branch, pos, node);
    }

    Node const &removeAt(BranchName const &branch, unsigned pos) const
    throw(InvalidTransformation, std::bad_alloc)
    {
        return _removeAt(branch, pos);
    }

    Node const &reorder(BranchName const &branch, unsigned old_pos, unsigned new_pos) const
    throw(InvalidTransformation, std::bad_alloc)
    {
        return _reorder(branch, old_pos, new_pos);
    }

    void write(std::ostream &stream) const {
        _write(stream);
    }
    String const &toString() const {
        return _toString();
    }

protected:
    virtual Node const *_traverse(BranchName const &branch, unsigned pos) const
    throw();

    virtual Node const &_insertBefore(BranchName const &branch,
                                      unsigned pos, Node const &node) const
    throw(InvalidTransformation, std::bad_alloc);

    virtual Node const &_replaceWith(BranchName const &branch,
                                     unsigned pos, Node const &node) const
    throw(InvalidTransformation, std::bad_alloc);

    virtual Node const &_removeAt(BranchName const &branch,
                                  unsigned pos, Node const &node) const
    throw(InvalidTransformation, std::bad_alloc);

    virtual Node const &_reorder(BranchName const &branch, unsigned old_pos,
                                 unsigned new_pos) const
    throw(InvalidTransformation, std::bad_alloc);

    virtual void _write(std::ostream &stream) const;
    virtual String const &_toString() const=0;

private:
    void operator=(Node const &);
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
