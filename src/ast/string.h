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

#ifndef SEEN_INKSCAPE_AST_STRING_H
#define SEEN_INKSCAPE_AST_STRING_H

#include <sys/types.h>
#include <new>
#include "ast/c-string.h"

namespace Inkscape {
namespace AST {

struct String {
public:
    static String const &create(CString const &string) throw(std::bad_alloc) {
        return *(new (GC) String(string));
    }

    size_t byteLength() const {
        return _bytes ? _bytes : ( _bytes = std::strlen(_string) );
    }
    size_t charLength() const {
        return _chars ? _chars : ( _chars = g_utf8_strlen(_string) );
    }

    operator CString const &() const throw() { return _string; }
    operator char const *() const throw() { return _string; }
    operator gchar const *() const throw() { return _string; }
    operator xmlChar const *() const throw() { return _string; }

private:
    String(CString const &string, size_t bytes=0, size_t chars=0)
    : _string(string), _bytes(bytes), _chars(chars) {}

    CString const &_string;
    mutable size_t _bytes;
    mutable size_t _chars;
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
