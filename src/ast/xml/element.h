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

#ifndef SEEN_INKSCAPE_AST_XML_ELEMENT_H
#define SEEN_INKSCAPE_AST_XML_ELEMENT_H

#include <cstring>
#include "ast/xml/base.h"
#include "ast/symbol-node.h"
#include "ast/length-list.h"

namespace Inkscape {
namespace AST {
namespace XML {

class Element : public Base {
public:
    explicit Element(Symbol name)
    : _name(name), _attributes(NULL), _children(NULL) {}

    static Symbol const PROPERTY(CString::create_unsafe("property"));
    static Symbol const ATTRIBUTE(CString::create_unsafe("attribute"));

    static Symbol const NAME(CString::create_unsafe("name"));
    static Symbol const CHILDREN(CString::create_unsafe("children"));

    Symbol name() const throw() { return _name.symbol(); }
    Node const *attribute(Symbol name) const throw();
    String const *attributeString(Symbol name) const throw(std::bad_alloc);
    Base const *child(unsigned pos) const throw();

    Element const &setAttribute(Symbol name, CString const &value) const throw(std::bad_alloc);
    Element const &setAttribute(Symbol name, Node const &value) const throw(std::bad_alloc);
    Element const &unsetAttribute(Symbol name) const throw(std::bad_alloc);

    Element const &insertChild(unsigned pos, Base const &child) const throw(std::bad_alloc);
    Element const &replaceChild(unsigned pos, Base const &child) const throw(std::bad_alloc);
    Element const &removeChild(unsigned pos) const throw(std::bad_alloc);
    Element const &reorderChild(unsigned old_pos, unsigned new_pos) const throw(std::bad_alloc);

private:
    Element(Symbol name, List<Attribute> const *attributes,
            LengthList<Base> const *children)
    : _name(name), _attributes(attributes), _children(children) {}

    static BranchName const NAME_BRANCH(PROPERTY, NAME);
    static BranchName const CHILDREN_BRANCH(PROPERTY, CHILDREN);

    Node const *_lookup(BranchName const &branch, unsigned pos) const
    throw(InvalidBranch);

    List<BranchName> const *_selectBranches(BranchSelector *selector)
    throw(std::bad_alloc);

    Node const &_insert(BranchName const &branch, unsigned pos, Node const &node) const throw(InvalidBranch, InvalidTransformation, std::bad_alloc);
    Node const &_replace(BranchName const &branch, unsigned pos, Node const &node) const throw(InvalidBranch, InvalidTransformation, std::bad_alloc);
    Node const &_remove(BranchName const &branch, unsigned pos) const throw(InvalidBranch, InvalidTransformation, std::bad_alloc);
    Node const &_reorder(BranchName const &branch, unsigned old_pos, unsigned new_pos) const throw(InvalidBranch, InvalidTransformation, std::bad_alloc);

    struct Attribute {
        Attribute(name, value) : _name(name), _value(value) {}

        Symbol name() const { return _name; }
        Node const &value() const { return _value; }

    private:
        void operator=(Attribute const &);

        Symbol _name;
        Node const &_value;
    };

    SymbolNode _name;
    LengthList<Base> const *_children;
    List<Attribute> const *_attributes;
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
