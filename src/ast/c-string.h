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

#ifndef SEEN_INKSCAPE_AST_C_STRING_H
#define SEEN_INKSCAPE_AST_C_STRING_H

#include <new>
#include <gc/gc_cpp.h>
#include <stdexcept>
#include <cstdlib>
#include <cstring>
#include <glib/glib.h>
#include <libxml/tree.h>

namespace Inkscape {
namespace AST {

struct CString {
public:
    class NullString : std::runtime_error {
    public:
        char const *what() const { return _("String is null"); }
    };

    static CString const &create(char const *string) throw(std::bad_alloc, NullString) {
        if (string) {
            return reinterpret_cast<CString const &>(*strcpy(new (GC) char[strlen(string)+1]));
        } else {
            throw NullString();
        }
    }
    static CString const &create(gchar const *string) throw(std::bad_alloc, NullString) {
        return create(reinterpret_cast<char const *>(string));
    }
    static CString const &create(xmlChar const *string) throw(std::bad_alloc, NullString) {
        return create(reinterpret_cast<char const *>(string));
    }

    static CString const &create_unsafe(char const *string) throw(NullString) {
        if (string) {
            return reinterpret_cast<CString const &>(*string);
        } else {
            throw NullString();
        }
    }
    static CString const &create_unsafe(gchar const *string) throw(NullString) {
        return create_unsafe(reinterpret_cast<char const *>(string));
    }
    static CString const &create_unsafe(xmlChar const *string) throw(NullString) {
        return create_unsafe(reinterpret_cast<char const *>(string));
    }

    operator char const *() const throw() {
        return reinterpret_cast<char const *>(this);
    }
    operator gchar const *() const throw() {
        return reinterpret_cast<gchar const *>(this);
    }
    operator xmlChar const *() const throw() {
        return reinterpret_cast<xmlChar const *>(this);
    }

private:
    CString();
    void operator=(CString const &);
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
