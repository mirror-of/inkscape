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

#include <algorithm>
#include <functional>
#include <gtkmm/liststore.h>
#include "widgets/layer-selector.h"
#include "widgets/document-tree-model.h"
#include "util/list.h"
#include "util/reverse-list.h"
#include "util/filter-list.h"
#include "sp-object.h"
#include "desktop.h"
#include "xml/repr.h"

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

    setDesktop(desktop);
}

LayerSelector::~LayerSelector() {
    setDesktop(NULL);
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
        _layer_changed_connection = _desktop->connectCurrentLayerChanged(sigc::mem_fun(*this, &LayerSelector::_updateLayer));
        _updateLayer(_desktop->currentLayer());
    }
}

using Inkscape::Util::List;

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
    column_matches_object(Gtk::TreeModelColumn<SPObject *> &column,
                          SPObject *object)
    : _column(column), _object(object) {}
    bool operator()(Gtk::TreeModel::const_iterator const &iter) const {
        return (*iter)[_column] == _object;
    }
private:
    Gtk::TreeModelColumn<SPObject *> &_column;
    SPObject *_object;
};

}

void LayerSelector::_updateLayer(SPObject *layer) {
    using Inkscape::Util::cons;
    using Inkscape::Util::reverse_list;

    _layer_model->clear();

    if (layer) {
        SPObject *root(_desktop->currentRoot());

        _buildEntries(0, cons(*root,
            reverse_list<SPObject &, SPObject::ParentIterator>(layer, root)
        ));

        _selector.set_active(
            std::find_if(_layer_model->children().begin(),
                         _layer_model->children().end(),
                         column_matches_object(_model_columns.object, layer))
        );
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
            filter_list<SPObject &, SPObject::SiblingIterator>(
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

void LayerSelector::_buildEntry(unsigned depth, SPObject &object) {
    Gtk::ListStore::iterator entry(_layer_model->append());

    entry->set_value(_model_columns.depth, depth);
    entry->set_value(_model_columns.object, &object);
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

        gchar const *format;
        if ( layer && SP_OBJECT_PARENT(object) == SP_OBJECT_PARENT(layer) ) {
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
            prefix = "(";
            label = "root";
            suffix = ")";
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
