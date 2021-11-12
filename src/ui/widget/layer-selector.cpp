// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Inkscape::Widgets::LayerSelector - layer selector widget
 *
 * Authors:
 *   MenTaLguY <mental@rydia.net>
 *   Abhishek Sharma
 *
 * Copyright (C) 2004 MenTaLguY
 *
 * Released under GNU GPL v2+, read the file 'COPYING' for more information.
 */

#include <cstring>
#include <string>

#include <boost/range/adaptor/filtered.hpp>
#include <boost/range/adaptor/reversed.hpp>

#include <glibmm/i18n.h>

#include "desktop.h"
#include "document-undo.h"
#include "document.h"
#include "layer-manager.h"
#include "verbs.h"

#include "ui/widget/layer-selector.h"
#include "ui/dialog/dialog-container.h"
#include "ui/dialog/objects.h"
#include "ui/icon-loader.h"
#include "ui/icon-names.h"
#include "ui/util.h"

#include "object/sp-root.h"
#include "object/sp-item-group.h"

namespace Inkscape {
namespace UI {
namespace Widget {

namespace {

class AlternateIcons : public Gtk::Box {
public:
    AlternateIcons(Gtk::BuiltinIconSize size, Glib::ustring const &a, Glib::ustring const &b)
    : Gtk::Box(Gtk::ORIENTATION_HORIZONTAL)
    , _a(nullptr)
    , _b(nullptr)
    {
        set_name("AlternateIcons");
        if (!a.empty()) {
            _a = Gtk::manage(sp_get_icon_image(a, size));
            _a->set_no_show_all(true);
            add(*_a);
        }
        if (!b.empty()) {
            _b = Gtk::manage(sp_get_icon_image(b, size));
            _b->set_no_show_all(true);
            add(*_b);
        }
        setState(false);
    }

    bool state() const { return _state; }
    void setState(bool state) {
        _state = state;
        if (_state) {
            if (_a) _a->hide();
            if (_b) _b->show();
        } else {
            if (_a) _a->show();
            if (_b) _b->hide();
        }
    }
private:
    Gtk::Image *_a;
    Gtk::Image *_b;
    bool _state;
};
}

LayerSelector::LayerSelector(SPDesktop *desktop)
    : Gtk::Box(Gtk::ORIENTATION_HORIZONTAL)
    , _desktop(nullptr)
    , _observer(new Inkscape::XML::SignalObserver)
{
    set_name("LayerSelector");
    AlternateIcons *label;

    label = Gtk::manage(new AlternateIcons(Gtk::ICON_SIZE_MENU,
        INKSCAPE_ICON("object-visible"), INKSCAPE_ICON("object-hidden")));
    _visibility_toggle.add(*label);
    _visibility_toggle.signal_toggled().connect(sigc::mem_fun(*this, &LayerSelector::_hideLayer));

    _visibility_toggle.set_relief(Gtk::RELIEF_NONE);
    _visibility_toggle.set_tooltip_text(_("Toggle current layer visibility"));
    pack_start(_visibility_toggle, Gtk::PACK_EXPAND_PADDING);

    label = Gtk::manage(new AlternateIcons(Gtk::ICON_SIZE_MENU,
        INKSCAPE_ICON("object-unlocked"), INKSCAPE_ICON("object-locked")));
    _lock_toggle.add(*label);
    _lock_toggle.signal_toggled().connect(sigc::mem_fun(*this, &LayerSelector::_lockLayer));

    _lock_toggle.set_relief(Gtk::RELIEF_NONE);
    _lock_toggle.set_tooltip_text(_("Lock or unlock current layer"));
    pack_start(_lock_toggle, Gtk::PACK_EXPAND_PADDING);

    _layer_name.signal_clicked().connect(
        sigc::mem_fun(*this, &LayerSelector::_layerChoose));
    _layer_name.set_relief(Gtk::RELIEF_NONE);
    _layer_name.set_tooltip_text(_("Current layer"));
    pack_start(_layer_name, Gtk::PACK_EXPAND_WIDGET);

    _observer->signal_changed().connect(sigc::mem_fun(*this, &LayerSelector::_layerModified));
    setDesktop(desktop);
}

LayerSelector::~LayerSelector() {
    setDesktop(nullptr);
}

void LayerSelector::setDesktop(SPDesktop *desktop) {
    if ( desktop == _desktop )
        return;

    _layer_changed.disconnect();
    _desktop = desktop;

    if (_desktop) {
        _layer_changed = _desktop->layerManager().connectCurrentLayerChanged(sigc::mem_fun(*this, &LayerSelector::_layerChanged));
        _layerChanged(_desktop->layerManager().currentLayer());
    }
}

/**
 * Selects the given layer in the widget.
 */
void LayerSelector::_layerChanged(SPGroup *layer)
{
    _layer = layer;
    _observer->set(layer);
    _layerModified();
}

/**
 * If anything happens to the layer, refresh it.
 */
void LayerSelector::_layerModified()
{
    auto root = _desktop->layerManager().currentRoot();
    bool active = _layer && _layer != root;

    if (active) {
        _layer_name.set_label(_layer->defaultLabel());
    } else {
        _layer_name.set_label(_layer ? "[root]" : "~nothing~");
    }
    _visibility_toggle.set_sensitive(active);
    _lock_toggle.set_sensitive(active);
    _visibility_toggle.set_active(active && _layer->isHidden());
    _lock_toggle.set_active(active && _layer->isLocked());
}

void LayerSelector::_lockLayer()
{
    bool lock = _lock_toggle.get_active();
    if (auto layer = _desktop->layerManager().currentLayer()) {
        if (auto child = dynamic_cast<AlternateIcons *>(_lock_toggle.get_child())) {
            child->setState(lock);
        }
        layer->setLocked(lock);
        DocumentUndo::done(_desktop->getDocument(), SP_VERB_NONE,
                           lock ? _("Lock layer") : _("Unlock layer"));
    }
}

void LayerSelector::_hideLayer()
{
    bool hide = _lock_toggle.get_active();
    if (auto layer = _desktop->layerManager().currentLayer()) {
        if (auto child = dynamic_cast<AlternateIcons *>(_lock_toggle.get_child())) {
            child->setState(hide);
        }
        layer->setHidden(hide);
        DocumentUndo::done(_desktop->getDocument(), SP_VERB_NONE,
                           hide ? _("Hide layer") : _("Unhide layer"));
    }
}

void LayerSelector::_layerChoose()
{
    auto prefs = Inkscape::Preferences::get();
    prefs->setBool("/dialogs/objects/layers_only", true);
    _desktop->getContainer()->new_dialog(SP_VERB_DIALOG_OBJECTS);
}

} // namespace Widget
} // namespace UI
} // namespace Inkscape

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
