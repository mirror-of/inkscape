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

#include "ast/node.h"
#include "ast/functional-traits.h"

namespace Inkscape {
namespace AST {

class LeafNodeBase : public Node {
protected:
    LeafNodeBase() {}

    Node const *_lookup(BranchName const &branch) const
    throw(InvalidBranch)
    {
        throw InvalidBranch();
    }

    List<BranchName> const *_selectBranches(BranchSelector selector) const
    throw()
    {
        return NULL;
    }

    Node const &_branchSize(BranchName const &branch) const
    throw(InvalidBranch)
    {
        throw InvalidBranch();
    }

    Node const &_insert(BranchName const &branch, unsigned pos,
                        Node const &node) const
    throw(InvalidBranch)
    {
        throw InvalidBranch();
    }

    Node const &_replace(BranchName const &branch, unsigned pos,
                         Node const &node) const
    throw(InvalidBranch)
    {
        throw InvalidBranch();
    }

    Node const &_remove(BranchName const &branch, unsigned pos) const
    throw(InvalidBranch)
    {
        throw InvalidBranch();
    }

    String const &_toString() const;
};

template <typename T>
class LeafNode : public LeafNodeBase {
public:
    typedef FunctionalTraits<T> type_traits;
    typedef typename type_traits::value_type value_type;
    typedef typename type_traits::reference_type reference_type;

    LeafNode(reference_type value) : _value(value) {}

protected:
    reference_type value() const throw() { return _value; }

    void _write(std::ostream &os) const { os << _value; }

private:
    value_type _value;
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
