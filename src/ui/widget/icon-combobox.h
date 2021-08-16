// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef ICON_COMBO_BOX_SEEN_
#define ICON_COMBO_BOX_SEEN_

#include <gtkmm/combobox.h>
#include <gtkmm/liststore.h>

namespace Inkscape {
namespace UI {
namespace Widget {

class IconComboBox : public Gtk::ComboBox {
public:
    IconComboBox() {
        _model = Gtk::ListStore::create(_columns);
        set_model(_model);

        pack_start(_renderer, false);
        _renderer.set_property("stock_size", Gtk::ICON_SIZE_BUTTON);
        _renderer.set_padding(2, 0);
        add_attribute(_renderer, "icon_name", _columns.icon_name);

        pack_start(_columns.label);
    }

    void add_row(const Glib::ustring& icon_name, const Glib::ustring& label, int id) {
        Gtk::TreeModel::Row row = *_model->append();
        row[_columns.id] = id;
        row[_columns.icon_name] = icon_name;
        row[_columns.label] = ' ' + label;
    }

    void set_active_by_id(int id) {
        for (auto i = _model->children().begin(); i != _model->children().end(); ++i) {
            const int data = (*i)[_columns.id];
            if (data == id) {
                set_active(i);
                break;
            }
        }
    };

    int get_active_row_id() const {
        if (auto it = get_active()) {
            return (*it)[_columns.id];
        }
        return -1;
    }

private:
    class Columns : public Gtk::TreeModel::ColumnRecord
    {
    public:
        Columns() {
            add(icon_name);
            add(label);
            add(id);
        }

        Gtk::TreeModelColumn<Glib::ustring> icon_name;
        Gtk::TreeModelColumn<Glib::ustring> label;
        Gtk::TreeModelColumn<int> id;
    };

    Columns _columns;
    Glib::RefPtr<Gtk::ListStore> _model;
    Gtk::CellRendererPixbuf _renderer;
};

}}}

#endif
