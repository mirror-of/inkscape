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

#include <glib/glib.h>
#include <gc/gc_cpp.h>
#include "ast/branch-name.h"

namespace Inkscape {
namespace AST {

class Node;

class Path : public gc {
public:
    Path(Path const *parent, BranchName const &branch, unsigned pos)
    : _parent(parent), _branch(branch), _pos(pos) {}

    Path const *parent() const { return _parent; }
    BranchName const &branch() const { return _branch; }
    unsigned pos() const { return _pos; }

    bool operator==(Path const &path) const {
        return _parent == path._parent &&
               _branch == path._branch &&
                  _pos == path._pos;
    }

private:
    void operator=(Path const &);

    Path const *_parent;
    BranchName _branch;
    unsigned _pos;
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
