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

#ifndef SEEN_INKSCAPE_AST_XML_ELEMENT_NODE_H
#define SEEN_INKSCAPE_AST_XML_ELEMENT_NODE_H

#include <cstring>
#include "ast/node.h"

namespace Inkscape {
namespace AST {

class XMLElementNode : public Node {
public:
    Node const *traverse(BranchName const &branch, unsigned pos) const;

    Node const &insertBefore(BranchName const &branch, unsigned pos,
                             Node const *node) const
    throw(InvalidTransformation);

    Node const &replaceWith(BranchName const &branch, unsigned pos,
                            Node const *node) const
    throw(InvalidTransformation);

    Node const &removeAt(BranchName const &branch, unsigned pos) const
    throw(InvalidTransformation);

    void write(std::ostream &os) const;
    CString const &toString() const;

private:
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
