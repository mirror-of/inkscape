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
#include <gtkmm/treemodel.h>
#include <gtkmm/liststore.h>
#include "util/list.h"

class SPDesktop;
class SPDocument;
class SPObject;

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

    class LayerModelColumns : public Gtk::TreeModel::ColumnRecord {
    public:
        Gtk::TreeModelColumn<bool> is_selected;
        Gtk::TreeModelColumn<Glib::ustring> label;
        Gtk::TreeModelColumn<SPObject *> object;

        LayerModelColumns() { add(is_selected); add(label); add(object); }
    };

    LayerModelColumns _model_columns;
    Gtk::CellRendererToggle _selected_column_renderer;
    Gtk::CellRendererText _name_column_renderer;
    Glib::RefPtr<Gtk::ListStore> _layer_model;

    sigc::connection _layer_changed_connection;

    SPDesktop *_desktop;

    void _updateLayer(SPObject *layer);
    void _buildEntry(unsigned depth, SPObject &object);
    void _buildEntries(unsigned depth,
                       Inkscape::Util::List<SPObject &> hierarchy);
    void _buildSiblingEntries(unsigned depth,
                              SPObject &parent,
                              Inkscape::Util::List<SPObject &> hierarchy);
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
