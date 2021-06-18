// SPDX-License-Identifier: GPL-2.0-or-later
#ifndef SEEN_GRADIENT_WITH_STOPS_H
#define SEEN_GRADIENT_WITH_STOPS_H

#include <gtkmm/widget.h>
#include <gdkmm/color.h>
#include "ui/svg-renderer.h"
#include "helper/auto-connection.h"

class SPGradient;

namespace Inkscape {
namespace UI {
namespace Widget {

class GradientWithStops : public Gtk::DrawingArea {
public:
    GradientWithStops();

    // gradient to draw or nullptr
    void set_gradient(SPGradient* gradient);

    // set selected stop handle (or pass -1 to deselect)
    void set_focused_stop(int index);

    // stop has been selected
    sigc::signal<void (size_t)>& signal_stop_selected() {
        return _signal_stop_selected;
    }

    // request to change stop's offset
    sigc::signal<void (size_t, double)>& signal_stop_offset_changed() {
        return _signal_stop_offset_changed;
    }

    sigc::signal<void (double)>& signal_add_stop_at() {
        return _signal_add_stop_at;
    }

    sigc::signal<void (size_t)>& signal_delete_stop() {
        return _signal_delete_stop;
    }

private:
    void get_preferred_width_vfunc(int& minimum_width, int& natural_width) const override;
    void get_preferred_height_vfunc(int& minimum_height, int& natural_height) const override;
    bool on_draw(const Cairo::RefPtr<Cairo::Context>& cr) override;
    void on_style_updated() override;
    bool on_button_press_event(GdkEventButton* event) override;
    bool on_button_release_event(GdkEventButton* event) override;
    bool on_motion_notify_event(GdkEventMotion* event) override;
    bool on_key_press_event(GdkEventKey* key_event) override;
    bool on_focus_in_event(GdkEventFocus* event) override;
    bool on_focus_out_event(GdkEventFocus* event) override;
    bool on_focus(Gtk::DirectionType direction) override;
    void size_request(GtkRequisition* requisition) const;
    void modified();
    // repaint widget
    void update();
    // index of gradient stop handle under (x, y) or -1
    int find_stop_at(double x, double y) const;
    // request stop move
    void move_stop(int stop_index, double offset_shift);

    // layout of gradient image/editor
    struct layout_t {
        double x, y, width, height;
    };
    layout_t get_layout() const;

    // position of single gradient stop handle
    struct stop_pos_t {
        double left, tip, right, top, bottom;
    };
    stop_pos_t get_stop_position(size_t index, const layout_t& layout) const;

    struct limits_t {
        double min_offset, max_offset, offset;
    };
    limits_t get_stop_limits(int index) const;
    GdkCursor* get_cursor(double x, double y) const;

    SPGradient* _gradient = nullptr;
    struct stop_t {
        double offset;
        SPColor color;
        double opacity;
    };
    std::vector<stop_t> _stops;
    // handle stop SVG template
    svg_renderer _template;
    // selected handle indicator
    svg_renderer _tip_template;
    auto_connection _release;
    auto_connection _modified;
    Gdk::RGBA _background_color;
    sigc::signal<void (size_t)> _signal_stop_selected;
    sigc::signal<void (size_t, double)> _signal_stop_offset_changed;
    sigc::signal<void (double)> _signal_add_stop_at;
    sigc::signal<void (size_t)> _signal_delete_stop;
    bool _dragging = false;
    // index of handle stop that user clicked; may be out of range
    int _focused_stop = -1;
    double _pointer_x = 0;
    double _stop_offset = 0;
    Glib::RefPtr<Gdk::Cursor> _cursor_mouseover;
    Glib::RefPtr<Gdk::Cursor> _cursor_dragging;
    Glib::RefPtr<Gdk::Cursor> _cursor_insert;
    // TODO: customize this amount or read prefs
    double _stop_move_increment = 0.01;
};

} // namespace Widget
} // namespace UI
} // namespace Inkscape

#endif
