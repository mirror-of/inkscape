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
    _selector.pack_start(_model_columns.label);

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

    bool operator()(SPObject &object) { return _desktop->isLayer(&object); }

private:
    SPDesktop *_desktop;
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

        Gtk::ListStore::iterator iter(_layer_model->children().begin());
        while ( iter != _layer_model->children().end() ) {
            if ( iter->get_value(_model_columns.object) == layer ) {
                iter->set_value(_model_columns.is_selected, true);
                _selector.set_active(iter);
                break;
            }
            ++iter;
        }
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

void LayerSelector::_buildSiblingEntries(unsigned depth,
                                         SPObject &parent,
                                         Inkscape::Util::List<SPObject &> hierarchy)
{
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
    entry->set_value(_model_columns.is_selected, false);
    gchar *label=g_strdup_printf("%*s#%s", depth*2, "", sp_repr_attr(SP_OBJECT_REPR(&object), "id"));
    entry->set_value(_model_columns.label, Glib::ustring(label));
    g_free(label);
    entry->set_value(_model_columns.object, &object);
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
