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

#include <gtkmm/box.h>
#include <gtkmm/combobox.h>
#include <gtkmm/togglebutton.h>
#include <gtkmm/cellrenderertext.h>

class SPDesktop;
class SPDocument;

namespace Inkscape {
namespace Widgets {

class DocumentTreeModel;

class LayerSelector : public Gtk::HBox {
public:
    LayerSelector(SPDesktop *desktop);
    ~LayerSelector();

    SPDesktop *desktop() { return _desktop; }
    void setDesktop(SPDesktop *desktop);

private:
    Gtk::ComboBox _selector;
    Gtk::ToggleButton _lock_button;
    Gtk::ToggleButton _hide_button;

    Gtk::CellRendererText _name_column_renderer;

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
