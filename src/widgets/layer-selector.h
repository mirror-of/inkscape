/*
 * Inkscape::Widgets::LayerSelector - layer selector widget
 *
 * Authors:
 *   MenTaLguY <mental@rydia.net>
 *
 * Copyright (C) 2004 MenTaLguY
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifndef SEEN_INKSCAPE_WIDGETS_LAYER_SELECTOR
#define SEEN_INKSCAPE_WIDGETS_LAYER_SELECTOR

#include <gtkmm/optionmenu.h>

class SPDesktop;
class SPDocument;

namespace Inkscape {
namespace Widgets {

class LayerSelector : public Gtk::OptionMenu {
public:
    LayerSelector(SPDesktop *desktop);
    ~LayerSelector();

private:
    SPDesktop *_desktop;
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
