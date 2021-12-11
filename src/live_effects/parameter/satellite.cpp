// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) Johan Engelen 2007 <j.b.c.engelen@utwente.nl>
 *   Abhishek Sharma
 *
 * Released under GNU GPL v2+, read the file 'COPYING' for more information.
 */

#include "live_effects/parameter/satellite.h"

#include "bad-uri-exception.h"
#include "desktop.h"
#include "enums.h"
#include "inkscape.h"
#include "live_effects/effect.h"
#include "live_effects/lpe-clone-original.h"
#include "live_effects/lpeobject.h"
#include "message-stack.h"
#include "selection-chemistry.h"
#include "svg/svg.h"
#include "ui/icon-loader.h"
#include "ui/widget/point.h"
#include "xml/repr.h"
// clipboard support
#include "ui/clipboard.h"
// required for linking to other paths
#include <glibmm/i18n.h>

#include "bad-uri-exception.h"
#include "object/sp-item.h"
#include "object/uri.h"
#include "ui/icon-names.h"
#include "verbs.h"

namespace Inkscape {

namespace LivePathEffect {

SatelliteParam::SatelliteParam(const Glib::ustring &label, const Glib::ustring &tip, const Glib::ustring &key,
                               Inkscape::UI::Widget::Registry *wr, Effect *effect)
    : Parameter(label, tip, key, wr, effect)
    , lperef(std::make_shared<SatelliteReference>(param_effect->getLPEObj(), false))
    , last_transform(Geom::identity())
{}

SatelliteParam::~SatelliteParam()
{
    quit_listening();
}

std::vector<SPObject *> SatelliteParam::param_get_satellites()
{
    std::vector<SPObject *> objs;
    // we reload connexions in case are lost for example item recreation on ungroup
    if (!linked_transformed_connection) {
        write_to_SVG();
    }

    SPObject * linked_obj = lperef->getObject();
    if (linked_obj) {
        objs.push_back(linked_obj);
    }
    return objs;
}

bool SatelliteParam::param_readSVGValue(const gchar *strvalue)
{
    if (strvalue) {
        SPObject *old_ref = lperef->getObject();
        if (old_ref) {
            unlink();
        }
        if (strvalue[0] == '#') {
            try {
                lperef->attach(Inkscape::URI(g_strdup(strvalue)));
                // lp:1299948
                SPObject *new_ref = lperef->getObject();
                if (new_ref) {
                    linked_changed(old_ref, new_ref);
                    // linked_modified(new_ref, SP_OBJECT_STYLESHEET_MODIFIED_FLAG);
                } // else: document still processing new events. Repr of the linked object not created yet.
            } catch (Inkscape::BadURIException &e) {
                g_warning("%s", e.what());
                lperef->detach();
            }
        }
        return true;
    }

    return false;
}

bool SatelliteParam::linksToItem() const
{
    return lperef->isAttached();
}

SPObject *SatelliteParam::getObject() const
{
    return lperef->isAttached() ? lperef->getObject() : nullptr;
}

Glib::ustring SatelliteParam::param_getSVGValue() const
{
    if (lperef->getURI()) {
        return lperef->getURI()->str();
    }
    return "";
}

Glib::ustring SatelliteParam::param_getDefaultSVGValue() const
{
    return "";
}

void SatelliteParam::param_set_default()
{
    param_readSVGValue("");
}

void SatelliteParam::unlink()
{
    quit_listening();
    if (linksToItem()) {
        lperef->detach();
    }
}

void SatelliteParam::link(Glib::ustring itemid)
{
    if (itemid.empty()) {
        return;
    }
    auto *document = param_effect->getSPDoc();
    SPObject *object = document->getObjectById(itemid);

    if (object && object != getObject()) {
        itemid.insert(itemid.begin(), '#');
        param_write_to_repr(itemid.c_str());
    } else {
        param_write_to_repr("");
    }
    DocumentUndo::done(document, _("Link item parameter to path"), "");
}

// SIGNALS

void SatelliteParam::start_listening(SPObject *to)
{
    if (!to) {
        return;
    }
    quit_listening();
    linked_changed_connection = lperef->changedSignal().connect(sigc::mem_fun(*this, &SatelliteParam::linked_changed));
    SPItem *item = dynamic_cast<SPItem *>(to);
    if (item) {
        linked_released_connection = item->connectRelease(sigc::mem_fun(*this, &SatelliteParam::linked_released));
        linked_modified_connection = item->connectModified(sigc::mem_fun(*this, &SatelliteParam::linked_modified));
        linked_transformed_connection =
            item->connectTransformed(sigc::mem_fun(*this, &SatelliteParam::linked_transformed));
        linked_modified(item, SP_OBJECT_MODIFIED_FLAG);
    }
}

void SatelliteParam::quit_listening()
{
    return;
    if (linked_changed_connection) {
        linked_changed_connection.disconnect();
    }
    if (linked_released_connection) {
        linked_released_connection.disconnect();
    }
    if (linked_modified_connection) {
        linked_modified_connection.disconnect();
    }
    if (linked_transformed_connection) {
        linked_transformed_connection.disconnect();
    }
}

void SatelliteParam::linked_changed(SPObject *old_obj, SPObject *new_obj)
{
    quit_listening();
    if (new_obj) {
        start_listening(new_obj);
    }
}

void SatelliteParam::linked_released(SPObject *released)
{
    unlink();
    param_effect->processObjects(LPE_UPDATE);
}


void SatelliteParam::linked_modified(SPObject *linked_obj, guint flags)
{
    param_effect->getLPEObj()->requestModified(SP_OBJECT_MODIFIED_FLAG);
    last_transform = Geom::identity();
    update_satellites();
}

void SatelliteParam::linked_transformed(Geom::Affine const *rel_transf, SPItem *moved_item)
{
    last_transform = *rel_transf;
    param_effect->getLPEObj()->requestModified(SP_OBJECT_MODIFIED_FLAG);
    SPDesktop *desktop = SP_ACTIVE_DESKTOP;
    if (desktop) {
        Inkscape::Selection *selection = desktop->getSelection();
        if (dynamic_cast<Inkscape::LivePathEffect::LPECloneOriginal *>(param_effect->getLPEObj()->get_lpe())) {
            auto hreflist = param_effect->getLPEObj()->hrefList;
            if (hreflist.size()) {
                SPLPEItem *sp_lpe_item = dynamic_cast<SPLPEItem *>(*hreflist.begin());
                if (sp_lpe_item) {
                    if (selection->includes(moved_item)) {
                        SPItem *item = sp_lpe_item;
                        Geom::Affine m(*rel_transf);
                        Inkscape::Preferences *prefs = Inkscape::Preferences::get();
                        guint mode = prefs->getInt("/options/clonecompensation/value", SP_CLONE_COMPENSATION_PARALLEL);
                        if (!selection->includes(item) &&
                            (!(m.isTranslation()) || mode == SP_CLONE_COMPENSATION_NONE)) {
                            item->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
                            update_satellites();
                            return;
                        }
                        // calculate the compensation matrix and the advertized movement matrix
                        item->readAttr(SPAttr::TRANSFORM);

                        Geom::Affine t = item->transform;
                        Geom::Affine offset_move = t.inverse() * m * t;

                        Geom::Affine advertized_move;
                        if (mode == SP_CLONE_COMPENSATION_PARALLEL) {
                            offset_move = offset_move.inverse() * m;
                            advertized_move = m;
                        } else if (mode == SP_CLONE_COMPENSATION_UNMOVED) {
                            offset_move = offset_move.inverse();
                            advertized_move.setIdentity();
                        } else {
                            g_assert_not_reached();
                        }
                        if (selection->includes(item)) {
                            advertized_move = m;
                        }
                        item->transform *= offset_move;
                        item->doWriteTransform(item->transform, &advertized_move);
                        item->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
                    }
                }
            }
        }
    }
    update_satellites();
}

// UI

void SatelliteParam::addCanvasIndicators(SPLPEItem const * /*lpeitem*/, std::vector<Geom::PathVector> &hp_vec) {}

void SatelliteParam::on_link_button_click()
{
    Inkscape::UI::ClipboardManager *cm = Inkscape::UI::ClipboardManager::get();
    auto itemid = cm->getFirstObjectID();
    if (itemid.empty()) {
        return;
    }

    link(itemid);
}

Gtk::Widget *SatelliteParam::param_newWidget()
{
    Gtk::Box *_widget = Gtk::manage(new Gtk::Box(Gtk::ORIENTATION_HORIZONTAL));
    Gtk::Image *pIcon = Gtk::manage(sp_get_icon_image("edit-clone", Gtk::ICON_SIZE_BUTTON));
    Gtk::Button *pButton = Gtk::manage(new Gtk::Button());
    Gtk::Label *pLabel = Gtk::manage(new Gtk::Label(param_label));
    _widget->pack_start(*pLabel, true, true);
    pLabel->set_tooltip_text(param_tooltip);
    pButton->set_relief(Gtk::RELIEF_NONE);
    pIcon->show();
    pButton->add(*pIcon);
    pButton->show();
    pButton->signal_clicked().connect(sigc::mem_fun(*this, &SatelliteParam::on_link_button_click));
    _widget->pack_start(*pButton, true, true);
    pButton->set_tooltip_text(_("Link to item on clipboard"));

    _widget->show_all_children();

    return dynamic_cast<Gtk::Widget *>(_widget);
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
