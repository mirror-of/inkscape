/*
 * Inkscape::Traits::Reference - traits class for dealing with reference types
 *
 * Authors:
 *   MenTaLguY <mental@rydia.net>
 *
 * Copyright (C) 2004 MenTaLguY
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifndef SEEN_INKSCAPE_TRAITS_REFERENCE_H
#define SEEN_INKSCAPE_TRAITS_REFERENCE_H

namespace Inkscape {

namespace Traits {

template <typename T>
struct Reference {
    typedef T const &Copy;
    typedef T &Assign;
};

template <typename T>
struct Reference<T &> {
    typedef T &Copy;
    typedef T &Assign;
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
