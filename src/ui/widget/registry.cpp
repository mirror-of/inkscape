/** \file
 *
 *
 * Authors:
 *   Ralf Stephan <ralf@ark.in-berlin.de> 
 *
 * Copyright (C) 2005 Authors
 *
 * Released under GNU GPL.  Read the file 'COPYING' for more information
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "registry.h"

namespace Inkscape {
namespace UI {
namespace Widget {

//===================================================

//---------------------------------------------------

Registry::Registry() : _updating(false) {}

Registry::~Registry() {}

bool
Registry::isUpdating()
{
    return _updating;
}

void
Registry::setUpdating (bool upd)
{
    _updating = upd;
}

void
Registry::add (const Glib::ustring& key, const Gtk::Object *o)
{
    _wmap[key] = o;
}

const Gtk::Object *
Registry::operator[] (const Glib::ustring& key)
{
    return _wmap[key];
}

//====================================================


} // namespace Dialog
} // namespace UI
} // namespace Inkscape

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
