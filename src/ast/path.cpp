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

#include "ast/node.h"
#include "ast/path.h"
#include "ast/path-fns.h"

namespace Inkscape {
namespace AST {

Node const *resolve(Path const *path, Node const *node) {
    if (path) {
        if (node) {
            return traverse(path, resolve(path->parent(), node));
        } else {
            return NULL;
        }
    } else {
        return node;
    }
}

Node const *traverse(Path const *path, Node const *node) {
    if (path) {
        if (node) {
            return node->traverse(path->branch(), path->pos());
        } else {
            return NULL;
        }
    } else {
        return node;
    }
}

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
