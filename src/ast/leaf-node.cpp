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

#include <osstream>
#include "ast/cstring.h"
#include "ast/string.h"
#include "ast/leaf-node.h"

namespace Inkscape {
namespace AST {

String const &LeafNodeBase::_toString() const {
    std::ostringstream os;
    write(os);
    return *(new String(CString::create(os.str().c_str())));
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
