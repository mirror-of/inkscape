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

namespace Inkscape {
namespace AST {

class StringNode : public Node {
public:
    StringNode(CString const &value) : _value(value) {}
    StringNode(StringNode const &node) : _value(node._value) {}

    Node const &deepClone() const { return *( new StringNode(*this) ); }

    CString const &toString() const { return _value; }

private:
    CString const &_value;
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
