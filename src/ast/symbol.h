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

#ifndef SEEN_INKSCAPE_AST_SYMBOL_H
#define SEEN_INKSCAPE_AST_SYMBOL_H

#include <new>
#include "ast/null-pointer.h"

namespace Inkscape {
namespace AST {

class CString;
class String;

class Symbol {
public:
    explicit Symbol(char const *string) throw(NullPointer, std::bad_alloc);
    explicit Symbol(CString const &string) throw(std::bad_alloc);
    explicit Symbol(String const &string) throw(std::bad_alloc);

    bool operator==(Symbol const &symbol) const {
        return _string == symbol._string;
    }
    bool operator!=(Symbol const &symbol) const {
        return _string != symbol._string;
    }

    String const &toString() const throw() { return *_string; }

private:
    String const *_string;
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
