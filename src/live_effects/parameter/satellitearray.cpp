// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) Jabiertxof <jabier.arraiza@marker.es>
 * this class handle satellites of a lpe as a parameter
 * Released under GNU GPL v2+, read the file 'COPYING' for more information.
 */

#include "live_effects/parameter/satellitearray.h"
#include "live_effects/effect.h"
#include "live_effects/lpeobject.h"
#include "inkscape.h"
#include "ui/clipboard.h"
#include "ui/icon-loader.h"
#include <glibmm/i18n.h>

namespace Inkscape {

namespace LivePathEffect {

class SatelliteArrayParam::ModelColumns : public Gtk::TreeModel::ColumnRecord
{
public:
    ModelColumns()
    {
        add(_colObject);
        add(_colLabel);
        add(_colActive);
    }
    ~ModelColumns() override = default;

    Gtk::TreeModelColumn<Glib::ustring> _colObject;
    Gtk::TreeModelColumn<Glib::ustring> _colLabel;
    Gtk::TreeModelColumn<bool> _colActive;
};
SatelliteArrayParam::SatelliteArrayParam(const Glib::ustring &label, const Glib::ustring &tip, const Glib::ustring &key,
                                         Inkscape::UI::Widget::Registry *wr, Effect *effect, bool visible)
    : ArrayParam<std::shared_ptr<SatelliteReference>>(label, tip, key, wr, effect)
    , _visible(visible)
{
    param_widget_is_visible(_visible);
    if (_visible) {
        _tree = nullptr;
        _scroller = nullptr;
        _model = nullptr;
        initui();
        oncanvas_editable = true;
    }
}

SatelliteArrayParam::~SatelliteArrayParam()
{
    _vector.clear();
    if (_store.get() && _model) {
        delete _model;
    }
    quit_listening();
}

void SatelliteArrayParam::initui()
{
    SPDesktop *desktop = SP_ACTIVE_DESKTOP;
    if (!desktop) {
        return;
    }
    if (!_tree) {
        _tree = manage(new Gtk::TreeView());
        _model = new ModelColumns();
        _store = Gtk::TreeStore::create(*_model);
        _tree->set_model(_store);

        _tree->set_reorderable(true);
        _tree->enable_model_drag_dest(Gdk::ACTION_MOVE);
        Gtk::CellRendererToggle *_toggle_active = manage(new Gtk::CellRendererToggle());
        int activeColNum = _tree->append_column(_("Active"), *_toggle_active) - 1;
        Gtk::TreeViewColumn *col_active = _tree->get_column(activeColNum);
        _toggle_active->set_activatable(true);
        _toggle_active->signal_toggled().connect(sigc::mem_fun(*this, &SatelliteArrayParam::on_active_toggled));
        col_active->add_attribute(_toggle_active->property_active(), _model->_colActive);

        _text_renderer = manage(new Gtk::CellRendererText());
        int nameColNum = _tree->append_column(_("Name"), *_text_renderer) - 1;
        _name_column = _tree->get_column(nameColNum);
        _name_column->add_attribute(_text_renderer->property_text(), _model->_colLabel);

        _tree->set_expander_column(*_tree->get_column(nameColNum));
        _tree->set_search_column(_model->_colLabel);

        // quick little hack -- newer versions of gtk gave the item zero space allotment
        _scroller = manage(new Gtk::ScrolledWindow());
        _scroller->set_size_request(-1, 120);

        _scroller->add(*_tree);
        _scroller->set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
        //_scroller->set_shadow_type(Gtk::SHADOW_IN);
    }
    param_readSVGValue(param_getSVGValue().c_str());
}

void SatelliteArrayParam::start_listening()
{
    quit_listening();
    for (auto ref : _vector) {
        if (ref && ref->isAttached()) {
            SPItem *item = dynamic_cast<SPItem *>(ref->getObject());
            if (item) {
                linked_connections.emplace_back(item->connectRelease(
                    sigc::hide(sigc::mem_fun(*this, &SatelliteArrayParam::updatesignal))));
                linked_connections.emplace_back(item->connectModified(
                    sigc::hide(sigc::hide(sigc::mem_fun(*this, &SatelliteArrayParam::updatesignal)))));
                linked_connections.emplace_back(item->connectTransformed(
                    sigc::hide(sigc::hide(sigc::mem_fun(*this, &SatelliteArrayParam::updatesignal)))));
                linked_connections.emplace_back(ref->changedSignal().connect(
                    sigc::hide(sigc::hide(sigc::mem_fun(*this, &SatelliteArrayParam::updatesignal)))));
            }
        }
    }
}

void SatelliteArrayParam::updatesignal()
{
    if (param_effect->_lpe_action == LPE_NONE) {
        param_effect->processObjects(LPE_UPDATE);
    }
}

void SatelliteArrayParam::quit_listening()
{
    for (auto connexion : linked_connections) {
        if (connexion) {
            connexion.disconnect();
        }
    }
    linked_connections.clear();
};

void SatelliteArrayParam::on_active_toggled(const Glib::ustring &item)
{
    int i = 0;
    for (auto w : _vector) {
        if (w && w->isAttached() && w->getObject()) {
            Gtk::TreeModel::Row row =  *_store->get_iter(Glib::ustring::format(i));
            Glib::ustring id = w->getObject()->getId() ? w->getObject()->getId() : "";
            if (id == row[_model->_colObject]) {
                row[_model->_colActive] = !row[_model->_colActive];
                w->setActive(row[_model->_colActive]);
                i++;
                break;
            }
        }
    }
    auto full = param_getSVGValue();
    param_write_to_repr(full.c_str());
    DocumentUndo::done(param_effect->getSPDoc(), _("Actived switched"), "");
}

bool SatelliteArrayParam::param_readSVGValue(const gchar *strvalue)
{
    if (strvalue) {
        bool changed =
            g_strcmp0(strvalue, param_effect->getRepr()->attribute(param_key.c_str())) || !linked_connections.size();
        if (!ArrayParam::param_readSVGValue(strvalue)) {
            return false;
        }
        if (_store.get()) {
            _store->clear();
            for (auto w : _vector) {
                if (w) {
                    Gtk::TreeModel::iterator iter = _store->append();
                    Gtk::TreeModel::Row row = *iter;
                    if (auto obj = w->getObject()) {
                        row[_model->_colObject] = Glib::ustring(obj->getId());
                        row[_model->_colLabel]  = obj->label() ? obj->label() : obj->getId();
                        row[_model->_colActive] = w->getActive();
                    }
                }
            }
        }
        if (changed) {
            start_listening();
        }
        return true;
    }
    return false;
}

bool SatelliteArrayParam::_selectIndex(const Gtk::TreeIter &iter, int *i)
{
    if ((*i)-- <= 0) {
        _tree->get_selection()->select(iter);
        return true;
    }
    return false;
}

void SatelliteArrayParam::on_up_button_click()
{
    Gtk::TreeModel::iterator iter = _tree->get_selection()->get_selected();
    if (iter) {
        Gtk::TreeModel::Row rowselected = *iter;
        int i = 0;
        for (auto w : _vector) {
            if (w && w->isAttached() && w->getObject()) {
                Gtk::TreeModel::Row row =  *_store->get_iter(Glib::ustring::format(i));
                if (rowselected == row && i > 0) {
                    std::swap(_vector[i],_vector[i-1]);
                    i--;
                    break;
                }
                i++;
            }
        }
        auto full = param_getSVGValue();
        param_write_to_repr(full.c_str());

        DocumentUndo::done(param_effect->getSPDoc(), _("Move item up"), "");

        _store->foreach_iter(sigc::bind<int *>(sigc::mem_fun(*this, &SatelliteArrayParam::_selectIndex), &i));
    }
}

void SatelliteArrayParam::on_down_button_click()
{
    Gtk::TreeModel::iterator iter = _tree->get_selection()->get_selected();
    if (iter) {
        Gtk::TreeModel::Row rowselected = *iter;
        int i = 0;
        for (auto w : _vector) {
            if (w && w->isAttached() && w->getObject()) {
                Gtk::TreeModel::Row row =  *_store->get_iter(Glib::ustring::format(i));
                if (rowselected == row && i < _vector.size() - 1) {
                    std::swap(_vector[i],_vector[i+1]);
                    i++;
                    break;
                }
                i++;
            }
        }
        auto full = param_getSVGValue();
        param_write_to_repr(full.c_str());

        DocumentUndo::done(param_effect->getSPDoc(), _("Move item down"), "");

        _store->foreach_iter(sigc::bind<int *>(sigc::mem_fun(*this, &SatelliteArrayParam::_selectIndex), &i));
    }
}

void SatelliteArrayParam::on_remove_button_click()
{
    Gtk::TreeModel::iterator iter = _tree->get_selection()->get_selected();
    if (iter) {
        Gtk::TreeModel::Row row = *iter;
        unlink(param_effect->getSPDoc()->getObjectById(row[_model->_colObject]));

        auto full = param_getSVGValue();
        param_write_to_repr(full.c_str());

        DocumentUndo::done(param_effect->getSPDoc(), _("Remove item"), "");
    }
}

void SatelliteArrayParam::on_link_button_click()
{
    Inkscape::UI::ClipboardManager *cm = Inkscape::UI::ClipboardManager::get();
    std::vector<Glib::ustring> itemsid;
    // Here we ignore auto clipboard group wrapper
    std::vector<Glib::ustring> itemsids = cm->getElementsOfType(SP_ACTIVE_DESKTOP, "*", 2);
    std::vector<Glib::ustring> containers = cm->getElementsOfType(SP_ACTIVE_DESKTOP, "*", 1);
    for (auto item : itemsids) {
        bool cont = false;
        for (auto citems : containers) {
            if (citems == item) {
                cont = true;
            }
        }
        if (cont == false) {
            itemsid.push_back(item);
        }
    }
    if (itemsid.empty()) {
        return;
    }
    auto hreflist = param_effect->getLPEObj()->hrefList;
    if (hreflist.size()) {
        SPLPEItem *sp_lpe_item = dynamic_cast<SPLPEItem *>(*hreflist.begin());
        if (sp_lpe_item) {
            for (auto itemid : itemsid) {
                SPObject *added = param_effect->getSPDoc()->getObjectById(itemid);
                if (added && sp_lpe_item != added) {
                    itemid.insert(itemid.begin(), '#');
                    std::shared_ptr<SatelliteReference> satellitereference =
                        std::make_shared<SatelliteReference>(param_effect->getLPEObj(), _visible);
                    try {
                        satellitereference->attach(Inkscape::URI(itemid.c_str()));
                        satellitereference->setActive(true);
                        _vector.push_back(satellitereference);
                    } catch (Inkscape::BadURIException &e) {
                        g_warning("%s", e.what());
                        satellitereference->detach();
                    }
                }
            }
        }
    }
    write_to_SVG();
    DocumentUndo::done(param_effect->getSPDoc(), _("Link itemarray parameter to item"), "");
}

Gtk::Widget *SatelliteArrayParam::param_newWidget()
{
    if (!_visible) {
        return nullptr;
    }
    Gtk::Box *vbox = Gtk::manage(new Gtk::Box(Gtk::ORIENTATION_VERTICAL));
    Gtk::Box *hbox = Gtk::manage(new Gtk::Box(Gtk::ORIENTATION_HORIZONTAL));
    _tree = nullptr;
    _scroller = nullptr;
    _model = nullptr;
    initui();
    vbox->pack_start(*_scroller, Gtk::PACK_EXPAND_WIDGET);

    { // Paste item to link button
        Gtk::Image *pIcon = Gtk::manage(sp_get_icon_image("edit-clone", Gtk::ICON_SIZE_BUTTON));
        Gtk::Button *pButton = Gtk::manage(new Gtk::Button());
        pButton->set_relief(Gtk::RELIEF_NONE);
        pIcon->show();
        pButton->add(*pIcon);
        pButton->show();
        pButton->signal_clicked().connect(sigc::mem_fun(*this, &SatelliteArrayParam::on_link_button_click));
        hbox->pack_start(*pButton, Gtk::PACK_SHRINK);
        pButton->set_tooltip_text(_("Link to item"));
    }

    { // Remove linked item
        Gtk::Image *pIcon = Gtk::manage(sp_get_icon_image("list-remove", Gtk::ICON_SIZE_BUTTON));
        Gtk::Button *pButton = Gtk::manage(new Gtk::Button());
        pButton->set_relief(Gtk::RELIEF_NONE);
        pIcon->show();
        pButton->add(*pIcon);
        pButton->show();
        pButton->signal_clicked().connect(sigc::mem_fun(*this, &SatelliteArrayParam::on_remove_button_click));
        hbox->pack_start(*pButton, Gtk::PACK_SHRINK);
        pButton->set_tooltip_text(_("Remove Item"));
    }

    { // Move Down
        Gtk::Image *pIcon = Gtk::manage(sp_get_icon_image("go-down", Gtk::ICON_SIZE_BUTTON));
        Gtk::Button *pButton = Gtk::manage(new Gtk::Button());
        pButton->set_relief(Gtk::RELIEF_NONE);
        pIcon->show();
        pButton->add(*pIcon);
        pButton->show();
        pButton->signal_clicked().connect(sigc::mem_fun(*this, &SatelliteArrayParam::on_down_button_click));
        hbox->pack_end(*pButton, Gtk::PACK_SHRINK);
        pButton->set_tooltip_text(_("Move Down"));
    }

    { // Move Down
        Gtk::Image *pIcon = Gtk::manage(sp_get_icon_image("go-up", Gtk::ICON_SIZE_BUTTON));
        Gtk::Button *pButton = Gtk::manage(new Gtk::Button());
        pButton->set_relief(Gtk::RELIEF_NONE);
        pIcon->show();
        pButton->add(*pIcon);
        pButton->show();
        pButton->signal_clicked().connect(sigc::mem_fun(*this, &SatelliteArrayParam::on_up_button_click));
        hbox->pack_end(*pButton, Gtk::PACK_SHRINK);
        pButton->set_tooltip_text(_("Move Up"));
    }

    vbox->pack_end(*hbox, Gtk::PACK_SHRINK);

    vbox->show_all_children(true);

    return vbox;
}

std::vector<SPObject *> SatelliteArrayParam::param_get_satellites()
{
    std::vector<SPObject *> objs;
    for (auto &iter : _vector) {
        if (iter && iter->isAttached()) {
            SPObject *obj = iter->getObject();
            if (obj) {
                objs.push_back(obj);
            }
        }
    }
    return objs;
}

/*
 * This function link a satellite writing into XML directly
 * @param obj: object to link
 * @param obj: position in vector
 */
void SatelliteArrayParam::link(SPObject *obj, size_t pos)
{
    if (obj && obj->getId()) {
        Glib::ustring itemid = "#";
        itemid += obj->getId();
        std::shared_ptr<SatelliteReference> satellitereference =
            std::make_shared<SatelliteReference>(param_effect->getLPEObj(), _visible);
        try {
            satellitereference->attach(Inkscape::URI(itemid.c_str()));
            if (_visible) {
                satellitereference->setActive(true);
            }
            if (_vector.size() == pos || pos == Glib::ustring::npos) {
                _vector.push_back(satellitereference);
            } else {
                _vector[pos] = satellitereference;
            }
        } catch (Inkscape::BadURIException &e) {
            g_warning("%s", e.what());
            satellitereference->detach();
        }
    }
}

void SatelliteArrayParam::unlink(SPObject *obj)
{
    if (!obj) {
        return;
    }
    gint pos = -1;
    for (auto w : _vector) {
        pos++;
        if (w) {
            if (w->getObject() == obj) {
                break;
            }
        }
    }
    if (pos != -1) {
        _vector.erase(_vector.begin() + pos);
        _vector.insert(_vector.begin() + pos, nullptr);
    }
}

void SatelliteArrayParam::unlink(std::shared_ptr<SatelliteReference> to)
{
    if (!to) {
        return;
    }
    gint pos = -1;
    for (auto w : _vector) {
        pos++;
        if (w) {
            if (w->getObject() == to->getObject()) {
                break;
            }
            
        }
    }
    if (pos != -1) {
        _vector.erase(_vector.begin() + pos);
        _vector.insert(_vector.begin() + pos, nullptr);
    }
}

void SatelliteArrayParam::clear()
{
    _vector.clear();
}

} /* namespace LivePathEffect */

} /* namespace Inkscape */

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :
