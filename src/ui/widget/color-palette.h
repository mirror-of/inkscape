// SPDX-License-Identifier: GPL-2.0-or-later
/** @file
 * @brief Color palette widget
 */
/* Authors:
 *   Michael Kowalski
 *
 * Copyright (C) 2021 Michael Kowalski
 * Released under GNU GPL v2+, read the file 'COPYING' for more information.
 */

#ifndef SEEN_COLOR_PALETTE_H
#define SEEN_COLOR_PALETTE_H

#include <gtkmm/bin.h>
#include <gtkmm/builder.h>
#include <gtkmm/button.h>
#include <gtkmm/flowbox.h>
#include <gtkmm/scrolledwindow.h>
#include <gtkmm/menu.h>
#include <vector>

namespace Inkscape {
namespace UI {
namespace Widget {

class ColorPalette : public Gtk::Bin {
public:
    ColorPalette();
    ~ColorPalette() override;

    struct rgb_t { double r; double g; double b; };
    struct palette_t { Glib::ustring name; std::vector<rgb_t> colors; };

    // set colors presented in a palette
    void set_colors(const std::vector<Gtk::Widget*>& swatches);
    // list of palettes to present in the menu
    void set_palettes(const std::vector<palette_t>& palettes);
    // enable compact mode (true) with mini-scroll buttons, or normal mode (false) with regular scrollbars
    void set_compact(bool compact);

    void set_tile_size(int size_px);
    void set_tile_border(int border_px);
    void set_rows(int rows);
    void set_aspect(double aspect);
    // show horizontal scrollbar when only 1 row is set
    void enable_scrollbar(bool show);
    // allow tile stretching (horizontally)
    void enable_stretch(bool enable);

    int get_tile_size() const;
    int get_tile_border() const;
    int get_rows() const;
    double get_aspect() const;
    bool is_scrollbar_enabled() const;
    bool is_stretch_enabled() const;

    void set_selected(const Glib::ustring& name);

    sigc::signal<void, Glib::ustring>& get_palette_selected_signal();
    sigc::signal<void>& get_settings_changed_signal();

private:
    void resize();
    void set_up_scrolling();
    void free();
    void scroll(int dx, int dy, double snap, bool smooth);
    void do_scroll(int dx, int dy);
    static gboolean scroll_cb(gpointer self);
    void _set_tile_size(int size_px);
    void _set_tile_border(int border_px);
    void _set_rows(int rows);
    void _set_aspect(double aspect);
    void _enable_scrollbar(bool show);
    void _enable_stretch(bool enable);
    static gboolean check_scrollbar(gpointer self);
    void update_checkbox();
    void update_stretch();
    int get_tile_size(bool horz) const;
    int get_tile_width() const;
    int get_tile_height() const;
    int get_palette_height() const;

    Glib::RefPtr<Gtk::Builder> _builder;
    Gtk::FlowBox& _flowbox;
    Gtk::ScrolledWindow& _scroll;
    Gtk::FlowBox& _scroll_btn;
    Gtk::Button& _scroll_up;
    Gtk::Button& _scroll_down;
    Gtk::Button& _scroll_left;
    Gtk::Button& _scroll_right;
    Gtk::Menu& _menu;
    int _size = 10;
    int _border = 0;
    int _rows = 1;
    double _aspect = 0.0;
    int _count = 1;
    bool _compact = true;
    sigc::signal<void, Glib::ustring> _signal_palette_selected;
    sigc::signal<void> _signal_settings_changed;
    bool _in_update = false;
    guint _active_timeout = 0;
    bool _force_scrollbar = false;
    bool _stretch_tiles = false;
    double _scroll_step = 0.0; // smooth scrolling step
    double _scroll_final = 0.0; // smooth scroll final value
};

}}} // namespace

#endif // SEEN_COLOR_PALETTE_H
