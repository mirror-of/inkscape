// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Inkscape::UI::Widget::LayerSelector - layer selector widget
 *
 * Authors:
 *   MenTaLguY <mental@rydia.net>
 *
 * Copyright (C) 2004 MenTaLguY
 *
 * Released under GNU GPL v2+, read the file 'COPYING' for more information.
 */

#ifndef SEEN_INKSCAPE_WIDGETS_LAYER_SELECTOR
#define SEEN_INKSCAPE_WIDGETS_LAYER_SELECTOR

#include <gtkmm/box.h>
#include <gtkmm/combobox.h>
#include <gtkmm/togglebutton.h>
#include <gtkmm/cellrenderertext.h>
#include <gtkmm/treemodel.h>
#include <gtkmm/liststore.h>
#include <gtkmm/cssprovider.h>
#include <sigc++/slot.h>

#include "xml/helper-observer.h"

class SPDesktop;
class SPDocument;
class SPGroup;

namespace Inkscape {
namespace UI {
namespace Widget {

class AlternateIcons;

class LayerSelector : public Gtk::Box {
public:
    LayerSelector(SPDesktop *desktop = nullptr);
    ~LayerSelector() override;

    void setDesktop(SPDesktop *desktop);
private:
    SPDesktop *_desktop;
    SPGroup *_layer;

    Gtk::ToggleButton _eye_toggle;
    Gtk::ToggleButton _lock_toggle;
    Gtk::Button _layer_name;
    Gtk::Label _layer_label;
    Glib::RefPtr<Gtk::CssProvider> _label_style;
    AlternateIcons * _eye_label;
    AlternateIcons * _lock_label;

    sigc::connection _layer_changed;
    sigc::connection _hide_layer_connection;
    sigc::connection _lock_layer_connection;
    std::unique_ptr<Inkscape::XML::SignalObserver> _observer;

    void _layerChanged(SPGroup *layer);
    void _layerModified();
    void _selectLayer();
    void _hideLayer();
    void _lockLayer();
    void _layerChoose();
};

} // namespace Widget
} // namespace UI
} // namespace Inkscape

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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8:textwidth=99 :
