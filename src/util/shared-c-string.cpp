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

#include <cstring>
#include <glib/gmessages.h>
#include <sys/types.h>
#include "gc-core.h"
#include "util/shared-c-string.h"

namespace Inkscape {

namespace Util {

SharedCString SharedCString::copy(char const *string) {
    g_return_val_if_fail(string != NULL, SharedCString::coerce(NULL));

    return SharedCString::copy(string, std::strlen(string));
}

SharedCString SharedCString::copy(char const *string, size_t len) {
    g_return_val_if_fail(string != NULL, SharedCString::coerce(NULL));

    char *dup=new (GC::ATOMIC) gchar[len+1];
    std::memcpy(dup, string, len);
    dup[len] = '\000';

    return SharedCString::coerce(dup);
}

}

}

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
