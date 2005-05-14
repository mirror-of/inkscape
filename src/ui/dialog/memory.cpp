/**
 * \brief Memory statistics dialog
 *
 * Copyright 2005 MenTaLguY <mental@rydia.net>
 *
 * Released under GNU GPL.  Read the file 'COPYING' for more information.
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "ui/dialog/memory.h"

namespace Inkscape {
namespace UI {
namespace Dialog {

Memory::Memory() {
    set_title(_("Memory Info"));
    set_default_size(200, 200);

    transientize();

    show_all_children();
}

Memory::~Memory() {
}

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
