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

#include <typeinfo>
#include <cstdlib>
#include "ast/xml/element.h"
#include "ast/length-list-fns.h"
#include "ast/string-node.h"

namespace Inkscape {
namespace AST {
namespace XML {

namespace {

class AttributeNameMatchesP {
public:
    AttributeNameMatchesP(Symbol name) : _name(name) {}
    bool operator(List<Attribute> const &list) {
        return &list->value().name() == _name );
    }
private:
    Symbol _name;
};

Node const *Element::attribute(Symbol name) const
throw()
{
    return findLastValue(_attributes, AttributeNameMatchesP(name));
}

String const *Element::attributeString(Symbol name) const
throw(std::bad_alloc)
{
    Node const *value=attribute(name);
    return value ? &value.toString() : NULL;
}

Base const *Element::child(unsigned pos) const
throw()
{
    return get(_children, pos);
};

Element const &Element::setAttribute(Symbol name, CString const &value) const
throw(std::bad_alloc)
{
    return setAttribute(name, *(new StringNode(value)));
}

Element const &Element::setAttribute(Symbol name, Node const &value) const
throw(std::bad_alloc)
{
    List<Attribute> const *attributes=new List<Attribute>(removeLast(_attributes, AttributeNameMatchesP(name)), value);
    return *(new Element(_name, attributes, _children));
}

Element const &Element::unsetAttribute(Symbol name, CString const &value) const
throw(std::bad_alloc)
{
    List<Attribute> const *attributes=removeLast(_attributes, AttributeNameMatchesP(name));
    if ( attributes != _attributes ) {
        return *(new Element(_name, attributes, _children));
    } else {
        return *this;
    }
}

Element const &Element::insertChild(unsigned pos, Base const &child) const
throw(std::bad_alloc)
{
    LengthList<Base &> const *children=insert(_children, pos, child);
    return *(new Element(_name, _attributes, children));
}

Element const &Element::replaceChild(unsigned pos, Base const &child) const
throw(std::bad_alloc)
{
    LengthList<Base &> const *children=replace(_children, pos, child);
    return *(new Element(_name, _attributes, children));
}

Element const &Element::removeChild(unsigned pos) const
throw(std::bad_alloc)
{
    LengthList<Base &> const *children=remove(_children, pos);
    if ( children != _children ) {
        return *(new Element(_name, _attributes, children));
    } else {
        return *this;
    }
}

Element const &Element::reorderChild(unsigned old_pos, unsigned new_pos) const
throw(std::bad_alloc)
{
    LengthList<Base &> const *children=reorder(_children, old_pos, new_pos);
    if ( children != _children ) {
        return *(new Element(_name, _attributes, children));
    } else {
        return *this;
    }
}

Node const *Element::_lookup(BranchName const &branch, unsigned pos) const
throw(InvalidBranch)
{
    if ( branch.axis() == PROPERTY ) {
        if ( branch.name() == NAME ) {
            return pos == 0 ? &_name : NULL;
        } else if ( branch.name() == CHILDREN ) {
            return child(pos);
        } else {
            throw InvalidBranch();
        }
    } else if ( branch.axis() == ATTRIBUTE ) {
        return attribute(branch.name());
    } else {
        throw InvalidBranch();
    }
}

List<BranchName> const *Element::_selectBranches(Node::BranchSelector *selector)
const
throw(std::bad_alloc)
{
    List<BranchName> const *branches=NULL;    

    for ( List<Attribute> const *iter=_attributes ; iter ; iter = iter->prev() )
    {
        BranchName branch(Element::ATTRIBUTE, iter->value().name());

        if ( selector && (*selector)(*this, branch) ) {
            branches = new List<BranchName>(branches, branch);
        }
    }

    if ( selector && (*selector)(*this, NAME_BRANCH)) {
        branches = new List<BranchName>(branches, NAME_BRANCH);
    }
    if ( _children && selector && (*selector)(*this, CHILDREN_BRANCH) ) {
        branches = new List<BranchName>(branches, CHILDREN_BRANCH);
    }

    return branches;
}

Node const &Element::_insert(BranchName const &branch, unsigned pos, Node const &node) const
throw(InvalidBranch, InvalidTransformation, std::bad_alloc)
{
    if ( branch.axis() == ATTRIBUTE ) {
        if (!attribute(branch.name())) {
            return setAttribute(branch.name(), node);
        } else {
            throw InvalidTransformation();
        }
    } else if ( branch.axis() == PROPERTY ) {
        if ( branch.name() == CHILDREN ) {
            try {
                return insertChild(pos, dynamic_cast<Base const &>(node));
            } catch (std::bad_cast &e) {
                throw InvalidTransformation();
            }
        } else if ( branch.name() == NAME ) {
            throw InvalidTransformation();
        } else {
            throw InvalidBranch();
        }
    } else {
        throw InvalidBranch();
    }
}

Node const &Element::_replace(BranchName const &branch, unsigned pos, Node const &node) const
throw(InvalidBranch, InvalidTransformation, std::bad_alloc)
{
    if ( branch.axis() == ATTRIBUTE ) {
        return setAttribute(branch.name(), node);
    } else if ( branch.axis() == PROPERTY ) {
        if ( branch.name() == CHILDREN ) {
            try {
                return replaceChild(pos, dynamic_cast<Base const &>(node));
            } catch (std::bad_cast &e) {
                throw InvalidTransformation();
            }
        } else if ( branch.name() == NAME ) {
            throw InvalidTransformation();
        } else {
            throw InvalidBranch();
        }
    } else {
        throw InvalidBranch();
    }
}

Node const &Element::_remove(BranchName const &branch, unsigned pos) const
throw(InvalidBranch, InvalidTransformation, std::bad_alloc)
{
    if ( branch.axis() == ATTRIBUTE ) {
        return unsetAttribute(branch.name());
    } else if ( branch.axis() == PROPERTY ) {
        if ( branch.name() == CHILDREN ) {
            return removeChild(pos);
        } else if ( branch.name() == NAME ) {
            throw InvalidTransformation();
        } else {
            throw InvalidBranch();
        }
    } else {
        throw InvalidBranch();
    }
}

Node const &Element::_reorder(BranchName const &branch, unsigned old_pos, unsigned new_pos) const
throw(InvalidBranch, InvalidTransformation, std::bad_alloc)
{
    if ( branch.axis() == ATTRIBUTE ) {
        // reordering on an attribute branch is always a noop,
        // since they never exceed one node long
        return *this;
    } else if ( branch.axis() == PROPERTY ) {
        if ( branch.name() == CHILDREN ) {
            return reorderChild(old_pos, new_pos);
        } else if ( branch.name() == NAME ) {
            // safe noop
            return *this;
        } else {
            throw InvalidBranch();
        }
    } else {
        throw InvalidBranch();
    }
}

};

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
