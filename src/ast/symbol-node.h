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

#include <cstring>
#include "ast/node.h"
#include "ast/symbol.h"

namespace Inkscape {
namespace AST {

class SymbolNode : public Node {
public:
    explicit SymbolNode(Symbol symbol) : _symbol(symbol) {}

    Symbol symbol() const { return _symbol; }
    String const &toString() const { return _symbol.toString(); }

private:
    void operator=(SymbolNode const &);

    Symbol _symbol;
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
