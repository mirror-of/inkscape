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
#include "ast/gc.h"
#include "ast/c-string.h"

namespace Inkscape {
namespace AST {

struct String : public SimpleGCObject<> {
public:
    explicit String(CString const &string)
    : _string(string), _bytes(0), _chars(0) {}

    size_t byteLength() const throw() {
        return _bytes ? _bytes : ( _bytes = std::strlen(_string) );
    }
    size_t charLength() const throw() {
        return _chars ? _chars : ( _chars = g_utf8_strlen(_string) );
    }

    operator CString const &() const throw() {
        return _string;
    }
    operator char const *() const throw() {
        return _string.toPointer();
    }
    operator gchar const *() const throw() {
        return reinterpret_cast<gchar const *>(_string.toPointer());
    }
    operator xmlChar const *() const throw() {
        return reinterpret_cast<xmlChar const *>(_string.toPointer());
    }

private:
    void operator=(CString const &);

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
