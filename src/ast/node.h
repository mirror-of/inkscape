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
#include <ostream>
#include <gc/gc_cpp.h>
#include <glib/glib.h>
#include "ast/branch-name.h"
#include "ast/invalid-transformation.h"

namespace Inkscape {
namespace AST {

class Node : public gc {
public:
    virtual Node const *traverse(BranchName const &branch, unsigned pos) const;

    virtual Node const &insertBefore(BranchName const &branch, unsigned pos, Node const *node) throw(InvalidTransformation) const;
    virtual Node const &replaceWith(BranchName const &branch, unsigned pos, Node const *node) throw(InvalidTransformation) const;
    virtual Node const &removeAt(BranchName const &branch, unsigned pos) throw(InvalidTransformation) const;

    virtual Node const &deepClone() const=0;

    virtual void write(std::ostream &stream) const;
    virtual CString const &toString() const=0;
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
