/*
 * Inkscape::Util::SharedCString - shared and immutable strings
 *
 * Authors:
 *   MenTaLguY <mental@rydia.net>
 *
 * Copyright (C) 2004 MenTaLguY
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifndef SEEN_INKSCAPE_UTIL_SHARED_C_STRING_H
#define SEEN_INKSCAPE_UTIL_SHARED_C_STRING_H

#include <sys/types.h>
#include <glib/gtypes.h>

namespace Inkscape {

namespace Util {

class SharedCString {
public:
    SharedCString() : _str(NULL) {}

    operator char const *() const { return cString(); }

    char operator[](size_t i) const { return cString()[i]; }

    char const *cString() const { return _str; }

    static SharedCString coerce(char const *s) { return SharedCString(s); }
    static SharedCString copy(char const *s);
    static SharedCString copy(char const *s, size_t len);

    operator bool() const { return _str; }

private:
    SharedCString(char const *s) : _str(s) {}

    char const *_str;
};

inline bool operator==(SharedCString const &ss, char const *s) {
    return ss.cString() == s;
}

inline bool operator==(char const *s, SharedCString const &ss) {
    return operator==(ss, s);
}

inline bool operator!=(SharedCString const &ss, char const *s) {
    return !operator==(ss, s);
}

inline bool operator!=(char const *s, SharedCString const &ss) {
    return !operator==(s, ss);
}

}

}

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
