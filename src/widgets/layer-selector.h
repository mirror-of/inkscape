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
#include <gtkmm/tooltips.h>
#include <gtkmm/cellrenderertext.h>
#include <gtkmm/treemodel.h>
#include <gtkmm/liststore.h>
#include <sigc++/slot.h>
#include "util/list.h"

class SPDesktop;
class SPDocument;
class SPObject;
class SPRepr;

namespace Inkscape {
namespace Widgets {

class DocumentTreeModel;

class LayerSelector : public Gtk::HBox {
public:
    LayerSelector(SPDesktop *desktop);
    ~LayerSelector();

    SPDesktop *desktop() { return _desktop; }
    void setDesktop(SPDesktop *desktop);

    void startRenameLayer();

private:
    class LayerModelColumns : public Gtk::TreeModel::ColumnRecord {
    public:
        Gtk::TreeModelColumn<unsigned> depth;
        Gtk::TreeModelColumn<SPObject *> object;
        Gtk::TreeModelColumn<SPRepr *> repr;
        Gtk::TreeModelColumn<void *> callbacks;

        LayerModelColumns() {
            add(depth); add(object); add(repr); add(callbacks);
        }
    };

    SPDesktop *_desktop;

    Gtk::Tooltips _tooltips;
    Gtk::ComboBox _selector;
    Gtk::ToggleButton _visibility_toggle;
    Gtk::ToggleButton _lock_toggle;

    LayerModelColumns _model_columns;
    Gtk::CellRendererText _label_renderer;
    Glib::RefPtr<Gtk::ListStore> _layer_model;

    sigc::connection _layer_changed_connection;
    sigc::connection _selection_changed_connection;

    void _selectLayer(SPObject *layer);
    void _setDesktopLayer();

    void _buildEntry(unsigned depth, SPObject &object);
    void _buildEntries(unsigned depth,
                       Inkscape::Util::List<SPObject &> hierarchy);
    void _buildSiblingEntries(unsigned depth,
                              SPObject &parent,
                              Inkscape::Util::List<SPObject &> hierarchy);
    void _destroyEntry(Gtk::ListStore::iterator const &row);

    void _prepareLabelRenderer(Gtk::TreeModel::const_iterator const &row);
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
