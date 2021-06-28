// SPDX-License-Identifier: GPL-2.0-or-later
#ifndef SEEN_GRADIENT_EDITOR_H
#define SEEN_GRADIENT_EDITOR_H

#include <gtkmm/box.h>
#include <gtkmm/grid.h>
#include <gtkmm/button.h>
#include <gtkmm/spinbutton.h>
#include <gtkmm/popover.h>
#include <gtkmm/image.h>
#include <gtkmm/expander.h>
#include <gtkmm/liststore.h>
#include <gtkmm/treemodelcolumn.h>
#include <gtkmm/builder.h>
#include <optional>

#include "object/sp-gradient.h"
#include "object/sp-stop.h"
#include "ui/selected-color.h"
#include "spin-scale.h"
#include "gradient-with-stops.h"
#include "gradient-selector-interface.h"
#include "ui/operation-blocker.h"

namespace Inkscape {
namespace UI {
namespace Widget {

class GradientSelector;

class GradientEditor : public Gtk::Box, public GradientSelectorInterface {
public:
    GradientEditor(const char* prefs);
    ~GradientEditor() noexcept override;

private:
    sigc::signal<void> _signal_grabbed;
    sigc::signal<void> _signal_dragged;
    sigc::signal<void> _signal_released;
    sigc::signal<void, SPGradient*> _signal_changed;

public:
    decltype(_signal_changed) signal_changed() const { return _signal_changed; }
    decltype(_signal_grabbed) signal_grabbed() const { return _signal_grabbed; }
    decltype(_signal_dragged) signal_dragged() const { return _signal_dragged; }
    decltype(_signal_released) signal_released() const { return _signal_released; }

    void setGradient(SPGradient* gradient) override;
    SPGradient* getVector() override;
    void setVector(SPDocument* doc, SPGradient* vector) override;
    void setMode(SelectorMode mode) override;
    void setUnits(SPGradientUnits units) override;
    SPGradientUnits getUnits() override;
    void setSpread(SPGradientSpread spread) override;
    SPGradientSpread getSpread() override;
    void selectStop(SPStop* selected) override;

private:
    void set_gradient(SPGradient* gradient);
    void stop_selected();
    void insert_stop_at(double offset);
    void add_stop(int index);
    void duplicate_stop();
    void delete_stop(int index);
    void show_stops(bool visible);
    void update_stops_layout();
    void set_repeat_mode(SPGradientSpread mode);
    void set_repeat_icon(SPGradientSpread mode);
    void reverse_gradient();
    void set_stop_color(SPColor color, float opacity);
    std::optional<Gtk::TreeRow> current_stop();
    SPStop* get_nth_stop(size_t index);
    SPStop* get_current_stop();
    bool select_stop(size_t index);
    void set_stop_offset(size_t index, double offset);
    SPGradient* get_gradient_vector();
    void fire_stop_selected(SPStop* stop);

    Glib::RefPtr<Gtk::Builder> _builder;
    GradientSelector* _selector;
    Inkscape::UI::SelectedColor _selected_color;
    Gtk::Popover& _popover;
    Gtk::Image& _repeat_icon;
    GradientWithStops _gradient_image;
    Glib::RefPtr<Gtk::ListStore> _stop_list_store;
    Gtk::TreeModelColumnRecord _stop_columns;
    Gtk::TreeModelColumn<SPStop*> _stopObj;
    Gtk::TreeModelColumn<size_t> _stopIdx;
    Gtk::TreeModelColumn<Glib::ustring> _stopID;
    Gtk::TreeModelColumn<Glib::RefPtr<Gdk::Pixbuf>> _stop_color;
    Gtk::TreeView& _stop_tree;
    Gtk::SpinButton& _offset_btn;
    Gtk::Button& _add_stop;
    Gtk::Button& _delete_stop;
    Gtk::Expander& _show_stops_list;
    bool _stops_list_visible = true;
    Gtk::Box& _stops_gallery;
    Gtk::Box& _colors_box;
    Gtk::ToggleButton& _linear_btn;
    Gtk::ToggleButton& _radial_btn;
    Gtk::Grid& _main_grid;
    SPGradient* _gradient = nullptr;
    SPDocument* _document = nullptr;
    OperationBlocker _update;
    OperationBlocker _notification;
    Glib::ustring _prefs;
};


} // namespace Widget
} // namespace UI
} // namespace Inkscape

#endif
