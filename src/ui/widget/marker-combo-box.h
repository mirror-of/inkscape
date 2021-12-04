// SPDX-License-Identifier: GPL-2.0-or-later
#ifndef SEEN_SP_MARKER_SELECTOR_NEW_H
#define SEEN_SP_MARKER_SELECTOR_NEW_H

/* Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   Maximilian Albert <maximilian.albert> (gtkmm-ification)
 *
 * Copyright (C) 2002 Lauris Kaplinski
 *
 * Released under GNU GPL v2+, read the file 'COPYING' for more information.
 */
#include <vector>

#include <gtkmm/bin.h>
#include <gtkmm/box.h>
#include <gtkmm/builder.h>
#include <gtkmm/checkbutton.h>
#include <gtkmm/flowbox.h>
#include <gtkmm/image.h>
#include <gtkmm/liststore.h>
#include <gtkmm/menubutton.h>
#include <gtkmm/spinbutton.h>
#include <gio/gliststore.h>

#include <sigc++/signal.h>

#include "document.h"
#include "inkscape.h"
#include "scrollprotected.h"
#include "display/drawing.h"
#include "ui/operation-blocker.h"

class SPMarker;

namespace Gtk {

class Container;
class Adjustment;
}

namespace Inkscape {
namespace UI {
namespace Widget {

/**
 * ComboBox-like class for selecting stroke markers.
 */
class MarkerComboBox : public Gtk::Bin {
    using parent_type = Gtk::Bin;

public:
    MarkerComboBox(Glib::ustring id, int loc);
    ~MarkerComboBox() override;

    void setDocument(SPDocument *);

    sigc::signal<void> changed_signal;
    sigc::signal<void> edit_signal;

    void set_current(SPObject *marker);
    std::string get_active_marker_uri();
    bool in_update() { return _update.pending(); };
    const char* get_id() { return _combo_id.c_str(); };
    int get_loc() { return _loc; };

    sigc::signal<void()> signal_changed() { return _signal_changed; }

private:
    struct MarkerItem : Glib::Object {
        Cairo::RefPtr<Cairo::Surface> pix;
        SPDocument* source = nullptr;
        std::string id;
        std::string label;
        bool stock = false;
        bool history = false;
        bool separator = false;
        int width = 0;
        int height = 0;

        bool operator == (const MarkerItem& item) const;
    };

    SPMarker* get_current() const;
    Glib::ustring _current_marker_id;
    // SPMarker* _current_marker = nullptr;
    sigc::signal<void()> _signal_changed;
    Glib::RefPtr<Gtk::Builder> _builder;
    Gtk::FlowBox& _marker_list;
    Gtk::Label& _marker_name;
    Glib::RefPtr<Gio::ListStore<MarkerItem>> _marker_store;
    std::vector<Glib::RefPtr<MarkerItem>> _stock_items;
    std::vector<Glib::RefPtr<MarkerItem>> _history_items;
    std::map<Gtk::Widget*, Glib::RefPtr<MarkerItem>> _widgets_to_markers;
    Gtk::Image& _preview;
    bool _preview_no_alloc = true;
    Gtk::Button& _link_scale;
    Gtk::SpinButton& _angle_btn;
    Gtk::MenuButton& _menu_btn;
    Gtk::SpinButton& _scale_x;
    Gtk::SpinButton& _scale_y;
    Gtk::CheckButton& _scale_with_stroke;
    Gtk::SpinButton& _offset_x;
    Gtk::SpinButton& _offset_y;
    Gtk::Widget& _input_grid;
    Gtk::RadioButton& _orient_auto_rev;
    Gtk::RadioButton& _orient_auto;
    Gtk::RadioButton& _orient_angle;
    Gtk::Button& _orient_flip_horz;
    Gtk::Image& _current_img;
    Gtk::Button& _edit_marker;
    bool _scale_linked = true;
    guint32 _background_color;
    guint32 _foreground_color;
    Glib::ustring _combo_id;
    int _loc;
    OperationBlocker _update;
    SPDocument *_document = nullptr;
    std::unique_ptr<SPDocument> _sandbox;
    Gtk::CellRendererPixbuf _image_renderer;

    class MarkerColumns : public Gtk::TreeModel::ColumnRecord {
    public:
        Gtk::TreeModelColumn<Glib::ustring> label;
        Gtk::TreeModelColumn<const gchar *> marker;   // ustring doesn't work here on windows due to unicode
        Gtk::TreeModelColumn<gboolean> stock;
        Gtk::TreeModelColumn<Glib::RefPtr<Gdk::Pixbuf>> pixbuf;
        Gtk::TreeModelColumn<gboolean> history;
        Gtk::TreeModelColumn<gboolean> separator;

        MarkerColumns() {
            add(label); add(stock);  add(marker);  add(history); add(separator); add(pixbuf);
        }
    };
    MarkerColumns marker_columns;

    void update_ui(SPMarker* marker, bool select);
    void update_widgets_from_marker(SPMarker* marker);
    void update_store();
    Glib::RefPtr<MarkerItem> add_separator(bool filler);
    void update_scale_link();
    Glib::RefPtr<MarkerItem> get_active();
    Glib::RefPtr<MarkerItem> find_marker_item(SPMarker* marker);
    void on_style_updated() override;
    void update_preview(Glib::RefPtr<MarkerItem> marker_item);
    void update_menu_btn(Glib::RefPtr<MarkerItem> marker_item);
    void set_active(Glib::RefPtr<MarkerItem> item);
    void init_combo();
    void set_history(Gtk::TreeModel::Row match_row);
    void marker_list_from_doc(SPDocument* source, bool history);
    std::vector<SPMarker*> get_marker_list(SPDocument* source);
    void add_markers (std::vector<SPMarker *> const& marker_list, SPDocument *source,  gboolean history);
    void remove_markers (gboolean history);
    std::unique_ptr<SPDocument> ink_markers_preview_doc(const Glib::ustring& group_id);
    Cairo::RefPtr<Cairo::Surface> create_marker_image(Geom::IntPoint pixel_size, gchar const *mname,
        SPDocument *source, Inkscape::Drawing &drawing, unsigned /*visionkey*/, bool checkerboard, bool no_clip, double scale);
    void refresh_after_markers_modified();
    sigc::connection modified_connection;
};

} // namespace Widget
} // namespace UI
} // namespace Inkscape
#endif // SEEN_SP_MARKER_SELECTOR_NEW_H

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
