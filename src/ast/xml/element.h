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
#include "ast/xml/node.h"
#include "ast/string-node.h"

namespace Inkscape {
namespace AST {
namespace XML {

class Element : public Base {
public:
    Element(Symbol name);

    static Symbol const PROPERTY=Symbol::intern("property");
    static Symbol const ATTRIBUTE=Symbol::intern("attribute");

    static Symbol const NAME=Symbol::intern("name");
    static Symbol const CHILDREN=Symbol::intern("children");

    Symbol name() const throw() { return _name.symbol(); }
    Node const *attribute(Symbol name) const throw();
    Base const *child(unsigned pos) const throw();

    Element const &setAttribute(Symbol name, CString const &value) const throw(InvalidBranch, InvalidTransformation, std::bad_alloc);
    Element const &setAttribute(Symbol name, Node const &value) const throw(InvalidBranch, InvalidTransformation, std::bad_alloc);
    Element const &unsetAttribute(Symbol name) const throw(InvalidBranch, InvalidTransformation, std::bad_alloc);

    Element const &insertChild(unsigned pos, Base const &child) const throw(InvalidBranch, InvalidTransformation, std::bad_alloc);
    Element const &replaceChild(unsigned pos, Base const &child) const throw(InvalidBranch, InvalidTransformation, std::bad_alloc);
    Element const &removeChild(unsigned pos) const throw(std::bad_alloc);
    Element const &reorderChild(unsigned old_pos, unsigned new_pos) const throw(std::bad_alloc);

private:
    Element(Symbol name, unsigned child_count, ChildList const *children, AttributeList const *attributes);

    Node const *_lookup(BranchName const &branch, unsigned pos) const throw(InvalidBranch);

    Node const &_insert(BranchName const &branch, unsigned pos, Node const &node) const throw(InvalidBranch, InvalidTransformation, std::bad_alloc);
    Node const &_replace(BranchName const &branch, unsigned pos, Node const &node) const throw(InvalidBranch, InvalidTransformation, std::bad_alloc);
    Node const &_remove(BranchName const &branch, unsigned pos) const throw(InvalidBranch, InvalidTransformation, std::bad_alloc);
    Node const &_reorder(BranchName const &branch, unsigned old_pos, unsigned new_pos) const throw(InvalidBranch, InvalidTransformation, std::bad_alloc);

    struct AttributeList {
        AttributeList(nx, nm, v) : next(nx), name(nm), value(v) {}

        AttributeList const *next;
        Symbol name;
        Node const &value;
    };

    AST::SymbolNode _name;
    unsigned _child_count;
    ChildList const *_children;
    AttributeList const *_attributes;
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
