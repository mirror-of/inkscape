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

#include <sys/types.h>
#include <cstring>
#include <new>
#include <gc/gc_cpp.h>
#include <glib/glib.h>
#include <libxml/tree.h>
#include "ast/c-array.h"

namespace Inkscape {
namespace AST {

struct CString : public CArray<char> {
public:
    static CString const &create(char const *string)
    throw(std::bad_alloc, NullPointer)
    {
        if (string) {
            return _create(string, std::strlen(string));
        } else {
            throw NullPointer();
        }
    }

    static CString const &create(gchar const *string)
    throw(std::bad_alloc, NullPointer)
    {
        return create(reinterpret_cast<char const *>(string));
    }

    static CString const &create(xmlChar const *string)
    throw(std::bad_alloc, NullPointer)
    {
        return create(reinterpret_cast<char const *>(string));
    }

    static CString const &create(char const *string, size_t bytes)
    throw(std::bad_alloc, NullPointer)
    {
        if (string) {
            return _create(string, bytes);
        } else {
            throw NullPointer();
        }
    }

    static CString const &create(gchar const *string, size_t bytes)
    throw(std::bad_alloc, NullPointer)
    {
        return create(reinterpret_cast<char const *>(string), bytes);
    }

    static CString const &create(xmlChar const *string, size_t bytes)
    throw(std::bad_alloc, NullPointer)
    {
        return create(reinterpret_cast<char const *>(string), bytes);
    }

    static CString const &create_unsafe(char const *string)
    throw(NullPointer)
    {
        if (string) {
            return _create_unsafe(string);
        } else {
            throw NullPointer();
        }
    }

    static CString const &create_unsafe(gchar const *string)
    throw(NullPointer)
    {
        return create_unsafe(reinterpret_cast<char const *>(string));
    }

    static CString const &create_unsafe(xmlChar const *string)
    throw(NullPointer)
    {
        return create_unsafe(reinterpret_cast<char const *>(string));
    }

    operator char const *() const throw() {
        return toPointer();
    }
    operator gchar const *() const throw() {
        return reinterpret_cast<gchar const *>(toPointer());
    }
    operator xmlChar const *() const throw() {
        return reinterpret_cast<xmlChar const *>(toPointer());
    }

private:
    void operator=(CString const &);

    static CString const &_create(char const *string, size_t bytes) const
    throw(std::bad_alloc)
    {
        char *copy=new (GC) char[bytes+1];
        std::memcpy(copy, string, bytes);
        copy[bytes] = '\000';
        return _create_unsafe(copy);
    }

    static CString const &_create_unsafe(char const *string) const
    throw()
    {
        CArray<char> const &array=CArray<char>::create_unsafe(string);
        return reinterpret_cast<CString const &>(array);
    }
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
