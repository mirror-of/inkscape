/*
 * Inkscape::Traits::List - traits class for list-like types
 *
 * Authors:
 *   MenTaLguY <mental@rydia.net>
 *
 * Copyright (C) 2004 MenTaLguY
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifndef SEEN_INKSCAPE_TRAITS_LIST_H
#define SEEN_INKSCAPE_TRAITS_LIST_H

namespace Inkscape {

namespace Traits {

template <typename T> struct List;

class ListSample {};
class ListSampleData;

template <>
struct List<ListSample> {
    typedef ListSampleData Data;

    static bool is_null(ListSample);
    static ListSample null();

    static ListSampleData first(ListSample);
    static ListSample rest(ListSample);
};

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
