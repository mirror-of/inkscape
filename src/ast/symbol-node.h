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

#ifndef SEEN_INKSCAPE_AST_STRING_NODE_H
#define SEEN_INKSCAPE_AST_STRING_NODE_H

#include "ast/symbol.h"
#include "ast/leaf-node.h"

namespace Inkscape {
namespace AST {

class SymbolNode : public LeafNode<Symbol> {
public:
    explicit SymbolNode(Symbol const &symbol) : LeafNode<Symbol>(symbol) {}

protected:
    void _write(std::ostream &os) const {
        os << value().toString();
    }
    String const &_toString() const { return value().toString(); }
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
