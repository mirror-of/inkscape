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

#include <glib/glib.h>
#include <gc/gc_cpp.h>

#include "ast/symbol.h"
#include "ast/c-string.h"
#include "ast/string.h"

namespace Inkscape {
namespace AST {

// anonymous namespace
namespace {

class SymbolEntry {
    SymbolEntry(CString const &string) : _string(string) {}
    SymbolEntry(String const &string) : _string(string) {}

    gchar const *key() const { return _string; }
    String const &string() const { return _string; }
    
    static guint hash(gconstpointer v) {
        return g_str_hash(pointer_to_key(v));
    }
    static gboolean equal(gconstpointer v, gconstpointer v2) {
        return g_str_equal(pointer_to_key(v), pointer_to_key(v2));
    }
private:
    static const gchar *pointer_to_key(gconstpointer v) {
        return reinterpret_cast<SymbolEntry const *>(v)->key();
    }

    String _string;
};

GHashTable *symbol_table=NULL;

GHashTable *get_table() {
    if (!symbol_table) {
        symbol_table = g_hash_table_new(&SymbolEntry::hash, &SymbolEntry::equal);
    }
    return symbol_table;
}

template <typename T>
String const &lookup_symbol(T const &string) throw(std::bad_alloc) {
    GHashTable *table = get_table();
    gchar const *chars = string;
    SymbolEntry *entry = reinterpret_cast<SymbolEntry *>(g_hash_table_lookup(table, chars));
    if (!entry) {
        entry = new (NoGC) SymbolEntry(string);
        g_hash_table_insert(table, entry->key(), entry);
    }
    return entry->string();
}

};

Symbol::Symbol(CString const &string) throw(std::bad_alloc) {
    _string = &lookup_symbol(string);
}

Symbol::Symbol(String const &string) throw(std::bad_alloc) {
    _string = &lookup_symbol(string);
}

Symbol Symbol::intern(char const *string) throw(NullPointer, std::bad_alloc) {
    return Symbol(CString::create(string));
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
