// SPDX-License-Identifier: GPL-2.0-or-later
/**
 * Authors:
 *   Jabier Arraiza <jabier.arraiza@marker.es>
 *
 * Copyright (C) 2019 Authors
 * Released under GNU GPL v2+, read the file 'COPYING' for more information.
 */


#ifndef SEEN_SP_PANNED_CANVAS__H
#define SEEN_SP_PANNED_CANVAS__H

#include <gtkmm.h>

struct SPCanvas;

namespace Inkscape {
  class SPPanedCanvas  : public Gtk::Panned  {
  public:
    SPPanedCanvas();
    ~SPPanedCanvas() = default;
    void toggleSplit();
    void toggleOrientation();
    void reload();
  private:
    SPCanvas *_canvas1;
    SPCanvas *_canvas2;
    Gtk::Orientation _orientation;
    bool _splited;
    SPPanedCanvas(SPPanedCanvas const &d);
    SPPanedCanvas& operator=(SPPanedCanvas const &d);
};

} // namespace Inkscape

#endif /* !SEEN_SP_PANNED_CANVAS__H */

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8:textwidth=99 :
