/*
 * Inkscape::Util::share_c_string - creates a GCed, immutable, duplicate
 *                                  of a string
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
#include "util/share-c-string.h"

namespace Inkscape {

namespace Util {

gchar const *share_c_string(gchar const *string) {
    g_return_val_if_fail(string != NULL, NULL);

    size_t n_bytes=std::strlen(string);
    gchar *copy=new (GC::ATOMIC) gchar[n_bytes+1];
    std::memcpy(copy, string, n_bytes);
    copy[n_bytes] = '\000';

    return copy;
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
