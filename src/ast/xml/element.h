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

class Element : public XML::Node {
public:
    Element(Symbol name);

    static Symbol const PROPERTY=Symbol::intern("property");
    static Symbol const ATTRIBUTE=Symbol::intern("attribute");

    static Symbol const NAME=Symbol::intern("name");
    static Symbol const CHILDREN=Symbol::intern("children");

    Symbol name() const throw() { return _name.symbol(); }
    AST::Node const *attribute(Symbol name) const throw();
    XML::Node const *child(unsigned pos) const throw();

    Element const &setAttribute(Symbol name, CString const &value) const throw(InvalidTransformation, std::bad_alloc);
    Element const &setAttribute(Symbol name, AST::Node const &value) const throw(InvalidTransformation, std::bad_alloc);
    Element const &unsetAttribute(Symbol name) const throw(InvalidTransformation, std::bad_alloc);

    Element const &insertChildBefore(unsigned pos, XML::Node const &child) const throw(InvalidTransformation, std::bad_alloc);
    Element const &replaceChildWith(unsigned pos, XML::Node const &child) const throw(InvalidTransformation, std::bad_alloc);
    Element const &removeChildAt(unsigned pos, XML::Node const &child) const throw(std::bad_alloc);
    Element const &reorderChild(unsigned old_pos, unsigned new_pos) const throw(std::bad_alloc);

private:
    Element(Symbol name, unsigned child_count, ChildList const *children, AttributeList const *attributes);

    AST::Node const *_traverse(BranchName const &branch, unsigned pos) const throw();

    AST::Node const *_insertBefore(BranchName const &branch, unsigned pos, AST::Node const &node) const throw(InvalidTransformation, std::bad_alloc);
    AST::Node const *_replaceWith(BranchName const &branch, unsigned pos, AST::Node const &node) const throw(InvalidTransformation, std::bad_alloc);
    AST::Node const *_removeAt(BranchName const &branch, unsigned pos) const throw(InvalidTransformation, std::bad_alloc);

    struct ChildList {
        ChildList(p, c) : prev(p), child(c) {}

        ChildList const *prev;
        XML::Node const &child;
    };
    struct AttributeList {
        AttributeList(nx, nm, v) : next(nx), name(nm), value(v) {}

        AttributeList const *next;
        Symbol name;
        AST::Node const &value;
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
