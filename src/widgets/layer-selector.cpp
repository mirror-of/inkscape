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

#include <cstring>
#include <algorithm>
#include <functional>
#include <gtkmm/liststore.h>
#include "desktop-handles.h"
#include "selection.h"
#include "widgets/layer-selector.h"
#include "widgets/document-tree-model.h"
#include "util/list.h"
#include "util/reverse-list.h"
#include "util/filter-list.h"
#include "sp-object.h"
#include "desktop.h"
#include "xml/repr.h"
#include "xml/sp-repr-event-vector.h"

namespace Inkscape {
namespace Widgets {

LayerSelector::LayerSelector(SPDesktop *desktop)
: _desktop(NULL)
{
    pack_start(_lock_button, Gtk::PACK_SHRINK);
    pack_start(_hide_button, Gtk::PACK_SHRINK);
    pack_start(_selector, Gtk::PACK_EXPAND_WIDGET);

    _layer_model = Gtk::ListStore::create(_model_columns);
    _selector.set_model(_layer_model);
    _selector.pack_start(_label_renderer);
    _selector.set_cell_data_func(
        _label_renderer,
        sigc::mem_fun(*this, &LayerSelector::_prepareLabelRenderer)
    );

    _selection_changed_connection = _selector.signal_changed().connect(
        sigc::mem_fun(*this, &LayerSelector::_setDesktopLayer)
    );
    setDesktop(desktop);
}

LayerSelector::~LayerSelector() {
    setDesktop(NULL);
    _selection_changed_connection.disconnect();
}

namespace {

gboolean detach(SPView *view, LayerSelector *selector) {
    selector->setDesktop(NULL);
    return FALSE;
}

}

void LayerSelector::setDesktop(SPDesktop *desktop) {
    if ( desktop == _desktop ) {
        return;
    }

    if (_desktop) {
        _layer_changed_connection.disconnect();
        g_signal_handlers_disconnect_by_func(_desktop, (gpointer)&detach, this);
    }
    _desktop = desktop;
    if (_desktop) {
        // TODO we need a different signal for this, really..
        g_signal_connect_after(_desktop, "shutdown", GCallback(detach), this);

        _layer_changed_connection = _desktop->connectCurrentLayerChanged(
            sigc::mem_fun(*this, &LayerSelector::_selectLayer)
        );
        _selectLayer(_desktop->currentLayer());
    }
}

namespace {

class is_layer {
public:
    is_layer(SPDesktop *desktop) : _desktop(desktop) {}
    bool operator()(SPObject &object) const {
        return _desktop->isLayer(&object);
    }
private:
    SPDesktop *_desktop;
};

class column_matches_object {
public:
    column_matches_object(Gtk::TreeModelColumn<SPObject *> const &column,
                          SPObject &object)
    : _column(column), _object(object) {}
    bool operator()(Gtk::TreeModel::const_iterator const &iter) const {
        return (*iter)[_column] == &_object;
    }
private:
    Gtk::TreeModelColumn<SPObject *> const &_column;
    SPObject &_object;
};

}

void LayerSelector::_selectLayer(SPObject *layer) {
    using Inkscape::Util::cons;
    using Inkscape::Util::reverse_list;

    _selection_changed_connection.block();

    while (!_layer_model->children().empty()) {
        Gtk::ListStore::iterator first_row(_layer_model->children().begin());
        _destroyEntry(first_row);
        _layer_model->erase(first_row);
    }

    if (layer) {
        SPObject *root(_desktop->currentRoot());

        _buildEntries(0, cons(*root,
            reverse_list<SPObject::ParentIterator>(layer, root)
        ));

        _selector.set_active(
            std::find_if(
                _layer_model->children().begin(),
                _layer_model->children().end(),
                column_matches_object(_model_columns.object, *layer)
            )
        );
            
    }

    _selection_changed_connection.unblock();
}

void LayerSelector::_setDesktopLayer() {
    Gtk::ListStore::iterator selected(_selector.get_active());
    SPObject *layer=_selector.get_active()->get_value(_model_columns.object);
    if ( _desktop && layer ) {
        _layer_changed_connection.block();
        _desktop->setCurrentLayer(layer);
        _layer_changed_connection.unblock();
        SP_DT_SELECTION(_desktop)->clear();
        _selectLayer(_desktop->currentLayer());
    }
}

void LayerSelector::_buildEntries(unsigned depth,
                                  Inkscape::Util::List<SPObject &> hierarchy)
{
    using Inkscape::Util::List;
    using Inkscape::Util::rest;

    _buildEntry(depth, *hierarchy);

    List<SPObject &> remainder(rest(hierarchy));
    if ( remainder && rest(remainder) ) {
        _buildEntries(depth+1, remainder);
    } else {
        _buildSiblingEntries(depth+1, *hierarchy, remainder);
    }
}

void LayerSelector::_buildSiblingEntries(
    unsigned depth, SPObject &parent,
    Inkscape::Util::List<SPObject &> hierarchy
) {
    using Inkscape::Util::List;
    using Inkscape::Util::rest;
    using Inkscape::Util::reverse_list_in_place;
    using Inkscape::Util::filter_list;

    Inkscape::Util::List<SPObject &> siblings(
        reverse_list_in_place(
            filter_list<SPObject::SiblingIterator>(
                is_layer(_desktop), parent.firstChild(), NULL
            )
        )
    );

    SPObject *layer( hierarchy ? &*hierarchy : NULL );

    while (siblings) {
        _buildEntry(depth, *siblings);
        if ( &*siblings == layer ) {
            _buildSiblingEntries(depth+1, *layer, rest(hierarchy));
        }
        ++siblings;
    }
}

namespace {

void attribute_changed(SPRepr *repr, gchar const *name,
                       gchar const *old_value, gchar const *new_value,
                       bool is_interactive, void *data) 
{
    if ( !std::strcmp(name, "id") || !std::strcmp(name, "inkscape:label") ) {
        (*reinterpret_cast<sigc::slot<void> *>(data))();
    }
}

void update_row_for_object(SPObject &object,
                           Gtk::TreeModelColumn<SPObject *> const &column,
                           Glib::RefPtr<Gtk::ListStore> const &model)
{
    Gtk::TreeIter row(
        std::find_if(
            model->children().begin(),
            model->children().end(),
            column_matches_object(column, object)
        )
    );
    model->row_changed(model->get_path(row), row);
}

}

void LayerSelector::_buildEntry(unsigned depth, SPObject &object) {
    static const SPReprEventVector events = {
        NULL, NULL,
        NULL, NULL,
        NULL, &attribute_changed,
        NULL, NULL,
        NULL, NULL
    };

    Gtk::ListStore::iterator row(_layer_model->append());

    SPRepr *repr=SP_OBJECT_REPR(&object);

    sigc::slot<void> *update_slot = new sigc::slot<void>(
        sigc::bind(
            sigc::ptr_fun(&update_row_for_object),
            object, _model_columns.object, _layer_model
        )
    );

    row->set_value(_model_columns.depth, depth);
    row->set_value(_model_columns.object, &object);
    row->set_value(_model_columns.repr, repr);
    row->set_value(_model_columns.update_slot, update_slot);

    sp_repr_add_listener(repr, &events, reinterpret_cast<void *>(update_slot));
}

void LayerSelector::_destroyEntry(Gtk::ListStore::iterator const &row) {
    SPRepr *repr=row->get_value(_model_columns.repr);
    sigc::slot<void> *update_slot=row->get_value(_model_columns.update_slot);
    if (repr) {
        sp_repr_remove_listener_by_data(repr, reinterpret_cast<void *>(update_slot));
        delete update_slot;
    }
}

void LayerSelector::_prepareLabelRenderer(
    Gtk::TreeModel::const_iterator const &row
) {
    unsigned depth=(*row)[_model_columns.depth];
    SPObject *object=(*row)[_model_columns.object];

    // TODO: when the currently selected row is removed,
    //       (or before one has been selected) something appears to
    //       "invent" an iterator with null data and try to render it;
    //       where does it come from, and how can we avoid it?
    if (object) {
        SPObject *layer=( _desktop ? _desktop->currentLayer() : NULL );
        SPObject *root=( _desktop ? _desktop->currentRoot() : NULL );

        gchar const *format;
        if ( layer && SP_OBJECT_PARENT(object) == SP_OBJECT_PARENT(layer) ||
             layer == root && SP_OBJECT_PARENT(object) == root
        ) {
            format="%*s<small>%s%s%s</small>";
        } else {
            format="%*s<small><small>%s%s%s</small></small>";
        }

        gchar const *prefix="";
        gchar const *suffix="";

        gchar const *label;
        if (depth) {
            if (object->label()) {
                label = object->label();
            } else {
                prefix = "(#";
                label = SP_OBJECT_ID(object);
                suffix = ")";
            }
        } else {
            prefix = "<";
            label = "root";
            suffix = ">";
        }

        gchar *text=g_markup_printf_escaped(format, depth*3, "",
                                            prefix, label, suffix);
        _label_renderer.property_markup() = text;
        g_free(text);
    } else {
        _label_renderer.property_markup() = "<small></small>";
    }

    _label_renderer.property_ypad() = 1;
    _label_renderer.property_yalign() = 0.25;
}

}
}

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
