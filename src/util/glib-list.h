/*
 * Inkscape::Util::glib_pointer_list - adaptor for glib lists
 *
 * Authors:
 *   MenTaLguY <mental@rydia.net>
 *
 * Copyright (C) 2004 MenTaLguY
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifndef SEEN_INKSCAPE_UTIL_GLIB_LIST_H
#define SEEN_INKSCAPE_UTIL_GLIB_LIST_H

#include <glib/glist.h>
#include <glib/gslist.h>

#include "util/list.h"

namespace Inkscape {
namespace Util {

template <typename T>
List<T *> const &glib_pointer_list(GSList * const &list) {
    return reinterpret_cast<List<T *> const &>(list);
}

template <typename T>
List<T *> const &glib_pointer_list(GList * const &list) {
    return reinterpret_cast<List<T *> const &>(list);
}

} /* namespace Util */
} /* namespace Inkscape */

#endif
/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=99 :
