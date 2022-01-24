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

LayerSelector::LayerSelector(SPDesktop *desktop)
    : Gtk::Box(Gtk::ORIENTATION_HORIZONTAL)
    , _desktop(nullptr)
    , _observer(new Inkscape::XML::SignalObserver)
{
    set_name("LayerSelector");

    _layer_name.signal_clicked().connect(sigc::mem_fun(*this, &LayerSelector::_layerChoose));
    _layer_name.set_relief(Gtk::RELIEF_NONE);
    _layer_name.set_tooltip_text(_("Current layer"));
    pack_start(_layer_name, Gtk::PACK_EXPAND_WIDGET);

    _eye_label = Gtk::manage(new AlternateIcons(Gtk::ICON_SIZE_MENU,
        INKSCAPE_ICON("object-visible"), INKSCAPE_ICON("object-hidden")));
    _eye_toggle.add(*_eye_label);
    _hide_layer_connection = _eye_toggle.signal_toggled().connect(sigc::mem_fun(*this, &LayerSelector::_hideLayer));

    _eye_toggle.set_relief(Gtk::RELIEF_NONE);
    _eye_toggle.set_tooltip_text(_("Toggle current layer visibility"));
    pack_start(_eye_toggle, Gtk::PACK_EXPAND_PADDING);

    _lock_label = Gtk::manage(new AlternateIcons(Gtk::ICON_SIZE_MENU,
        INKSCAPE_ICON("object-unlocked"), INKSCAPE_ICON("object-locked")));
    _lock_toggle.add(*_lock_label);
    _lock_layer_connection = _lock_toggle.signal_toggled().connect(sigc::mem_fun(*this, &LayerSelector::_lockLayer));

    _lock_toggle.set_relief(Gtk::RELIEF_NONE);
    _lock_toggle.set_tooltip_text(_("Lock or unlock current layer"));
    pack_start(_lock_toggle, Gtk::PACK_EXPAND_PADDING);

    _layer_name.add(_layer_label);
    _layer_label.set_max_width_chars(16);
    _layer_label.set_ellipsize(Pango::ELLIPSIZE_END);
    _layer_label.set_markup("<i>Unset</i>");
    _layer_label.set_valign(Gtk::ALIGN_CENTER);

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

    if (_label_style) {
        _layer_label.get_style_context()->remove_provider(_label_style);
    }
    auto color_str = std::string("white");

    if (active) {
        _layer_label.set_text(_layer->defaultLabel());
        color_str = SPColor(_layer->highlight_color()).toString();
    } else {
        _layer_label.set_markup(_layer ? "<i>[root]</i>" : "<i>nothing</i>");
    }

    Glib::RefPtr<Gtk::StyleContext> style_context = _layer_label.get_style_context();
    _label_style = Gtk::CssProvider::create();
    _label_style->load_from_data("#LayerSelector label {border-color:" + color_str + ";}");
    _layer_label.get_style_context()->add_provider(_label_style, GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);

    _hide_layer_connection.block();
    _lock_layer_connection.block();
    _eye_toggle.set_sensitive(active);
    _lock_toggle.set_sensitive(active);
    _eye_label->setState(active && _layer->isHidden());
    _eye_toggle.set_active(active && _layer->isHidden());
    _lock_label->setState(active && _layer->isLocked());
    _lock_toggle.set_active(active && _layer->isLocked());
    _hide_layer_connection.unblock();
    _lock_layer_connection.unblock();
}

void LayerSelector::_lockLayer()
{
    bool lock = _lock_toggle.get_active();
    if (auto layer = _desktop->layerManager().currentLayer()) {
        layer->setLocked(lock);
        DocumentUndo::done(_desktop->getDocument(), lock ? _("Lock layer") : _("Unlock layer"), "");
    }
}

void LayerSelector::_hideLayer()
{
    bool hide = _eye_toggle.get_active();
    if (auto layer = _desktop->layerManager().currentLayer()) {
        layer->setHidden(hide);
        DocumentUndo::done(_desktop->getDocument(), hide ? _("Hide layer") : _("Unhide layer"), "");
    }
}

void LayerSelector::_layerChoose()
{
    _desktop->getContainer()->new_dialog("Objects");
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
