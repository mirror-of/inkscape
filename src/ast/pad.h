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

#ifndef SEEN_INKSCAPE_AST_PAD_H
#define SEEN_INKSCAPE_AST_PAD_H

#include "ast/invalid-branch.h"
#include "ast/invalid-transformation.h"
#include "ast/gc.h"

namespace Inkscape {
namespace AST {

class Reference;

class Pad : public FinalizedGCObject<::NoGC> {
public:
    Pad(Pad &parent, BranchName const &branch, unsigned pos);
    Pad(Node const &node);

    void Pad
    
private:
    Pad *_parent;
    Reference const *_ref;
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
