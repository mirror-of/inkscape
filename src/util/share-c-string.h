/*
 * Inkscape::Util::share_c_string - create an immutable GCed copy of a string
 *
 * Authors:
 *   MenTaLguY <mental@rydia.net>
 *
 * Copyright (C) 2004 MenTaLguY
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifndef SEEN_INKSCAPE_UTIL_SHARE_C_STRING_H
#define SEEN_INKSCAPE_UTIL_SHARE_C_STRING_H

#include <glib/gtypes.h>

namespace Inkscape {

namespace Util {

gchar const *share_c_string(gchar const *string);

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
