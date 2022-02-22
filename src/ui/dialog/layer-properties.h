// SPDX-License-Identifier: GPL-2.0-or-later
/** @file
 * @brief  Dialog for renaming layers
 */
/* Author:
 *   Bryce W. Harrington <bryce@bryceharrington.org>
 *
 * Copyright (C) 2004 Bryce Harrington
 *
 * Released under GNU GPL v2+, read the file 'COPYING' for more information.
 */

#ifndef INKSCAPE_DIALOG_LAYER_PROPERTIES_H
#define INKSCAPE_DIALOG_LAYER_PROPERTIES_H

#include <gtkmm/dialog.h>
#include <gtkmm/entry.h>
#include <gtkmm/label.h>
#include <gtkmm/grid.h>

#include <gtkmm/combobox.h>
#include <gtkmm/liststore.h>
#include <gtkmm/treeview.h>
#include <gtkmm/treestore.h>
#include <gtkmm/scrolledwindow.h>

#include "layer-manager.h"

class SPDesktop;

namespace Inkscape {
namespace UI {
namespace Dialogs {

/* FIXME: split the LayerPropertiesDialog class into three separate dialogs */
enum class LayerPropertiesDialogType
{
    NONE,
    CREATE,
    MOVE,
    RENAME
};

class LayerPropertiesDialog : public Gtk::Dialog {
public:
    LayerPropertiesDialog(LayerPropertiesDialogType type);
    ~LayerPropertiesDialog() override;
    LayerPropertiesDialog(LayerPropertiesDialog const &) = delete; // no copy
    LayerPropertiesDialog &operator=(LayerPropertiesDialog const &) = delete; // no assign

    Glib::ustring getName() const { return "LayerPropertiesDialog"; }

    static void showRename(SPDesktop *desktop, SPObject *layer) {
        _showDialog(LayerPropertiesDialogType::RENAME, desktop, layer);
    }
    static void showCreate(SPDesktop *desktop, SPObject *layer) {
        _showDialog(LayerPropertiesDialogType::CREATE, desktop, layer);
    }
    static void showMove(SPDesktop *desktop, SPObject *layer) {
        _showDialog(LayerPropertiesDialogType::MOVE, desktop, layer);
    }

private:
    LayerPropertiesDialogType _type = LayerPropertiesDialogType::NONE;
    SPDesktop *_desktop = nullptr;
    SPObject *_layer = nullptr;

    class PositionDropdownColumns : public Gtk::TreeModel::ColumnRecord {
    public:
        Gtk::TreeModelColumn<LayerRelativePosition> position;
        Gtk::TreeModelColumn<Glib::ustring> name;

        PositionDropdownColumns() {
            add(position); add(name);
        }
    };

    Gtk::Label        _layer_name_label;
    Gtk::Entry        _layer_name_entry;
    Gtk::Label        _layer_position_label;
    Gtk::ComboBox     _layer_position_combo;
    Gtk::Grid         _layout_table;

    bool              _position_visible = false;

    class ModelColumns : public Gtk::TreeModel::ColumnRecord
    {
    public:

        ModelColumns()
        {
            add(_colObject);
            add(_colVisible);
            add(_colLocked);
            add(_colLabel);
        }
        ~ModelColumns() override = default;

        Gtk::TreeModelColumn<SPObject*> _colObject;
        Gtk::TreeModelColumn<Glib::ustring> _colLabel;
        Gtk::TreeModelColumn<bool> _colVisible;
        Gtk::TreeModelColumn<bool> _colLocked;
    };

    Gtk::TreeView _tree;
    ModelColumns* _model;
    Glib::RefPtr<Gtk::TreeStore> _store;
    Gtk::ScrolledWindow _scroller;


    PositionDropdownColumns _dropdown_columns;
    Gtk::CellRendererText _label_renderer;
    Glib::RefPtr<Gtk::ListStore> _dropdown_list;

    Gtk::Button _close_button;
    Gtk::Button _apply_button;

    sigc::connection _destroy_connection;

    void _setDesktop(SPDesktop *desktop) { _desktop = desktop; };
    void _setLayer(SPObject *layer);

    static void _showDialog(LayerPropertiesDialogType type, SPDesktop *desktop, SPObject *layer);
    void _apply();
    void _close();

    void _setup_position_controls();
    void _setup_layers_controls();
    void _prepareLabelRenderer(Gtk::TreeModel::const_iterator const &row);

    void _addLayer(SPObject* layer, Gtk::TreeModel::Row* parentRow, SPObject* target, int level);
    SPObject* _selectedLayer();
    bool _handleKeyEvent(GdkEventKey *event);
    void _handleButtonEvent(GdkEventButton* event);

    void _doCreate();
    void _doMove();
    void _doRename();
    void _setup();
};

} // namespace
} // namespace
} // namespace


#endif //INKSCAPE_DIALOG_LAYER_PROPERTIES_H

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
