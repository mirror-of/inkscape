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

#include <cstdlib>
#include <iosfwd>
#include <glib/glib.h>
#include <sigc++/sigc++.h>

#include "ast/gc.h"
#include "ast/list.h"
#include "ast/branch-name.h"
#include "ast/invalid-branch.h"
#include "ast/invalid-transformation.h"

namespace Inkscape {
namespace AST {

class String;

class Node : public SimpleGCObject<> {
public:
    typedef SigC::Slot2<bool, Node const &, BranchName const &> BranchSelector;

    Node const *lookup(BranchName const &branch, unsigned pos) const
    throw(InvalidBranch)
    {
        return _lookup(branch, pos);
    }

    List<BranchName> const *selectBranches() const
    throw(std::bad_alloc)
    {
        return _selectBranches(NULL);
    }

    List<BranchName> const *selectBranches(BranchSelector selector) const
    throw(std::bad_alloc)
    {
        return _selectBranches(&selector);
    }

    unsigned branchSize(BranchName const &branch) const
    throw(InvalidBranch)
    {
        return _branchSize(branch);
    }

    Node const &insert(BranchName const &branch, unsigned pos,
                       Node const &node) const
    throw(InvalidBranch, InvalidTransformation, std::bad_alloc)
    {
        return _insert(branch, pos, node);
    }

    Node const &replace(BranchName const &branch, unsigned pos,
                        Node const &node) const
    throw(InvalidBranch, InvalidTransformation, std::bad_alloc)
    {
        return _replace(branch, pos, node);
    }

    Node const &remove(BranchName const &branch, unsigned pos) const
    throw(InvalidBranch, InvalidTransformation, std::bad_alloc)
    {
        return _remove(branch, pos);
    }

    Node const &reorder(BranchName const &branch,
                        unsigned old_pos, unsigned new_pos) const
    throw(InvalidBranch, InvalidTransformation, std::bad_alloc)
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
    virtual Node const *_lookup(BranchName const &branch, unsigned pos) const
    throw(InvalidBranch)=0;

    virtual List<BranchName> const *_selectBranches(BranchSelector *selector)
    const
    throw(std::bad_alloc)=0;

    virtual unsigned _branchSize(BranchName const &branch) const
    throw(InvalidBranch)=0;

    virtual Node const &_insert(BranchName const &branch,
                                unsigned pos, Node const &node) const
    throw(InvalidBranch, InvalidTransformation, std::bad_alloc)=0;

    virtual Node const &_replace(BranchName const &branch,
                                 unsigned pos, Node const &node) const
    throw(InvalidBranch, InvalidTransformation, std::bad_alloc)=0;

    virtual Node const &_remove(BranchName const &branch,
                                unsigned pos, Node const &node) const
    throw(InvalidBranch, InvalidTransformation, std::bad_alloc)=0;

    virtual Node const &_reorder(BranchName const &branch,
                                 unsigned old_pos, unsigned new_pos) const
    throw(InvalidBranch, InvalidTransformation, std::bad_alloc)=0;

    virtual void _write(std::ostream &stream) const=0;
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
