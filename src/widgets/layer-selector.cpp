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

#include "widgets/layer-selector.h"
#include "widgets/document-tree-model.h"
#include "desktop.h"

namespace Inkscape {
namespace Widgets {

LayerSelector::LayerSelector(SPDesktop *desktop)
: _layer_model(Gtk::ListStore::create(LayerModelColumns())), _desktop(NULL)
{
    pack_start(_lock_button, Gtk::PACK_SHRINK);
    pack_start(_hide_button, Gtk::PACK_SHRINK);
    pack_start(_selector, Gtk::PACK_EXPAND_WIDGET);

    _selector.set_model(_layer_model);

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
    if (desktop) {
        // TODO we need a different signal for this, really..
        g_signal_connect_after(desktop, "shutdown", GCallback(detach), this);
        _layer_changed_connection = desktop->connectCurrentLayerChanged(sigc::mem_fun(*this, &LayerSelector::_updateLayer));
    }
    _desktop = desktop;
}

void LayerSelector::_updateLayer(SPObject *layer) {
    
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
