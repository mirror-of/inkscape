/*
 * Authors:
 *   Ted Gould <ted@gould.cx>
 *
 * Copyright (C) 2004 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifndef INKSCAPE_EXTENSION_DEPENDENCY_H__
#define INKSCAPE_EXTENSION_DEPENDENCY_H__

#include <glibmm/ustring.h>
#include "xml/repr.h"

namespace Inkscape {
namespace Extension {

class Dependency {
    bool _check;


public:
    Dependency (SPRepr * in_repr);
    bool check (void) const;
    Glib::ustring &get_help (void) const;
    Glib::ustring &get_link (void) const;

}; /* class Dependency */

};}; /* namespace Extension, Inkscape */

#endif /* INKSCAPE_EXTENSION_DEPENDENCY_H__ */

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :
