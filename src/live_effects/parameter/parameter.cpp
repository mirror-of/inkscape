// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) Johan Engelen 2007 <j.b.c.engelen@utwente.nl>
 *
 * Released under GNU GPL v2+, read the file 'COPYING' for more information.
 */

#include "parameter.h"

#include <glibmm/i18n.h>
#include <utility>

#include "display/control/canvas-item-bpath.h"
#include "display/curve.h"
#include "live_effects/effect.h"
#include "live_effects/effect-enum.h"
#include "svg/stringstream.h"
#include "svg/svg.h"
#include "ui/icon-names.h"
#include "xml/repr.h"

#define noLPEREALPARAM_DEBUG

namespace Inkscape {

namespace LivePathEffect {


Parameter::Parameter(Glib::ustring label, Glib::ustring tip, Glib::ustring key, Inkscape::UI::Widget::Registry *wr,
                     Effect *effect)
    : param_key(std::move(key))
    , param_wr(wr)
    , param_label(std::move(label))
    , oncanvas_editable(false)
    , widget_is_visible(true)
    , widget_is_enabled(true)
    , param_tooltip(std::move(tip))
    , param_effect(effect)
{
}

Parameter::~Parameter()
{
    SPDesktop *desktop = SP_ACTIVE_DESKTOP;
    if (desktop && ownerlocator) {
        desktop->remove_temporary_canvasitem(ownerlocator);
    }
    if (selection_changed_connection) {
        selection_changed_connection->disconnect();
        delete selection_changed_connection;
        selection_changed_connection = nullptr;
    }
}

void Parameter::param_write_to_repr(const char *svgd)
{
    param_effect->getRepr()->setAttribute(param_key, svgd);
}

void Parameter::write_to_SVG()
{
    param_write_to_repr(param_getSVGValue().c_str());
}

EffectType Parameter::effectType() const 
{ 
    if (param_effect) {
        return param_effect->effectType(); 
    }
    return INVALID_LPE;
};

ParamType Parameter::paramType() const 
{ 
    return INVALID_PARAM;
};

void
sp_add_class(SPObject *item, Glib::ustring classglib) {
    gchar const *classlpe = item->getAttribute("class");
    if (classlpe) {
        classglib = classlpe;
        if (classglib.find("UnoptimicedTransforms") == Glib::ustring::npos) {
            classglib += " UnoptimicedTransforms";
            item->setAttribute("class",classglib.c_str());
        }
    } else {
        item->setAttribute("class","UnoptimicedTransforms");
    }
}

/*
 * sometimes for example on ungrouping or loading documents we need to relay in stored value instead the volatile
 * version in the parameter
 */
void Parameter::read_from_SVG()
{
    const gchar *val = param_effect->getRepr()->attribute(param_key.c_str());
    if (val) {
        param_readSVGValue(val);
    }
}

void Parameter::param_higlight(bool highlight, bool select)
{
    SPDesktop *desktop = SP_ACTIVE_DESKTOP;
    if (desktop) {
        std::vector<SPLPEItem *> lpeitems = param_effect->getCurrrentLPEItems();
        if (lpeitems.size()) {
            sp_add_class(lpeitems[0], "UnoptimicedTransforms");
        }
        if (!highlight && ownerlocator) {
            desktop->remove_temporary_canvasitem(ownerlocator);
            ownerlocator = nullptr;
        }
        if (highlight) {
            if (lpeitems.size() == 1 && param_effect->is_visible) {
                if (select && !lpeitems[0]->isHidden()) {
                    desktop->selection->clear();
                    desktop->selection->add(lpeitems[0]);
                    return;
                }
                auto c = std::make_unique<SPCurve>();
                std::vector<Geom::PathVector> cs; // = param_effect->getCanvasIndicators(lpeitems[0]);
                Geom::OptRect bbox = lpeitems[0]->documentVisualBounds();

                if (param_effect->helperLineSatellites) {
                    std::vector<SPObject *> satellites = param_get_satellites();
                    for (auto iter : satellites) {
                        SPItem *satelliteitem = dynamic_cast<SPItem *>(iter);
                        if (satelliteitem) {
                            bbox.unionWith(satelliteitem->documentVisualBounds());
                        }
                    }
                }
                Geom::PathVector out;
                if (bbox) {
                    Geom::Path p = Geom::Path(*bbox);
                    out.push_back(p);
                }
                cs.push_back(out);
                for (auto &p2 : cs) {
                    p2 *= desktop->dt2doc();
                    c->append(p2);
                }
                if (!c->is_empty()) {
                    desktop->remove_temporary_canvasitem(ownerlocator);
                    auto tmpitem = new Inkscape::CanvasItemBpath(desktop->getCanvasTemp(), c.get(), true);
                    tmpitem->set_stroke(0x0000ff9a);
                    tmpitem->set_fill(0x0, SP_WIND_RULE_NONZERO); // No fill
                    ownerlocator = desktop->add_temporary_canvasitem(tmpitem, 0);
                }
            }
        }
    }
}

void Parameter::change_selection(Inkscape::Selection *selection)
{
    update_satellites(false);
}

void Parameter::connect_selection_changed()
{
    SPDesktop *desktop = SP_ACTIVE_DESKTOP;
    if (desktop) {
        Inkscape::Selection *selection = desktop->selection;
        if (selection) {
            std::vector<SPObject *> satellites = param_get_satellites();
            if (!selection_changed_connection) {
                selection_changed_connection = new sigc::connection(
                    selection->connectChanged(sigc::mem_fun(*this, &Parameter::change_selection)));
            }
        }
    }
}

void Parameter::update_satellites(bool updatelpe)
{
    if (paramType() == ParamType::SATELLITE || paramType() == ParamType::SATELLITE_ARRAY || paramType() == ParamType::PATH ||
        paramType() == ParamType::PATH_ARRAY || paramType() == ParamType::ORIGINAL_PATH || paramType() == ParamType::ORIGINAL_SATELLITE) {
        SPDesktop *desktop = SP_ACTIVE_DESKTOP;
        if (desktop) {
            DocumentUndo::ScopedInsensitive _no_undo(desktop->getDocument());
            param_higlight(false, false);
            Inkscape::Selection *selection = desktop->selection;
            if (selection) {
                std::vector<SPObject *> satellites = param_get_satellites();
                connect_selection_changed();
                if (selection->singleItem()) {
                    if (param_effect->isOnClipboard()) {
                        return;
                    }
                    // we always start hidding helper path
                    for (auto iter : satellites) {
                        sp_add_class(iter, "UnoptimicedTransforms");
                        // if selection is current ref we highlight original sp_lpe_item to
                        // give visual feedback to the user to know whats the LPE item that generate the selection
                        if (iter && selection->includes(iter)) {
                            const gchar *classtoparentchar = iter->getAttribute("class");
                            if (classtoparentchar) {
                                Glib::ustring classtoparent = classtoparentchar;
                                if (classtoparent.find("lpeselectparent ") != Glib::ustring::npos) {
                                    param_higlight(true, true);
                                } else {
                                    param_higlight(true, false);
                                }
                            } else {
                                param_higlight(true, false);
                            }
                            break;
                        }
                    }
                }
            }
        }
        if (updatelpe) {
            std::vector<SPLPEItem *> lpeitems = param_effect->getCurrrentLPEItems();
            if (lpeitems.size() == 1 && param_effect->is_visible) {
                sp_lpe_item_update_patheffect(lpeitems[0], false, false);
            }
        }
    }
}

/*
 * we get satellites of parameter, virtual function overided by some parameter with linked satellites
 */
std::vector<SPObject *> Parameter::param_get_satellites()
{
    std::vector<SPObject *> objs;
    return objs;
};

/*###########################################
 *   REAL PARAM
 */
ScalarParam::ScalarParam(const Glib::ustring &label, const Glib::ustring &tip, const Glib::ustring &key,
                         Inkscape::UI::Widget::Registry *wr, Effect *effect, gdouble default_value)
    : Parameter(label, tip, key, wr, effect)
    , value(default_value)
    , min(-SCALARPARAM_G_MAXDOUBLE)
    , max(SCALARPARAM_G_MAXDOUBLE)
    , integer(false)
    , defvalue(default_value)
    , digits(2)
    , inc_step(0.1)
    , inc_page(1)
    , add_slider(false)
    , _set_undo(true)
{
}

ScalarParam::~ScalarParam() = default;

bool ScalarParam::param_readSVGValue(const gchar *strvalue)
{
    double newval;
    unsigned int success = sp_svg_number_read_d(strvalue, &newval);
    if (success == 1) {
        param_set_value(newval);
        return true;
    }
    return false;
}

Glib::ustring ScalarParam::param_getSVGValue() const
{
    Inkscape::SVGOStringStream os;
    os << value;
    return os.str();
}

Glib::ustring ScalarParam::param_getDefaultSVGValue() const
{
    Inkscape::SVGOStringStream os;
    os << defvalue;
    return os.str();
}

void ScalarParam::param_set_default() { param_set_value(defvalue); }

void ScalarParam::param_update_default(gdouble default_value) { defvalue = default_value; }

void ScalarParam::param_update_default(const gchar *default_value)
{
    double newval;
    unsigned int success = sp_svg_number_read_d(default_value, &newval);
    if (success == 1) {
        param_update_default(newval);
    }
}

void ScalarParam::param_transform_multiply(Geom::Affine const &postmul, bool set)
{
    // Check if proportional stroke-width scaling is on
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    bool transform_stroke = prefs ? prefs->getBool("/options/transform/stroke", true) : true;
    if (transform_stroke || set) {
        param_set_value(value * postmul.descrim());
        write_to_SVG();
    }
}

void ScalarParam::param_set_value(gdouble val)
{
    value = val;
    if (integer)
        value = round(value);
    if (value > max)
        value = max;
    if (value < min)
        value = min;
}

void ScalarParam::param_set_range(gdouble min, gdouble max)
{
    // if you look at client code, you'll see that many effects
    // has a tendency to set an upper range of Geom::infinity().
    // Once again, in gtk2, this is not a problem. But in gtk3,
    // widgets get allocated the amount of size they ask for,
    // leading to excessively long widgets.
    if (min >= -SCALARPARAM_G_MAXDOUBLE) {
        this->min = min;
    } else {
        this->min = -SCALARPARAM_G_MAXDOUBLE;
    }
    if (max <= SCALARPARAM_G_MAXDOUBLE) {
        this->max = max;
    } else {
        this->max = SCALARPARAM_G_MAXDOUBLE;
    }
    param_set_value(value); // reset value to see whether it is in ranges
}

void ScalarParam::param_make_integer(bool yes)
{
    integer = yes;
    digits = 0;
    inc_step = 1;
    inc_page = 10;
}

void ScalarParam::param_set_undo(bool set_undo) { _set_undo = set_undo; }

Gtk::Widget *ScalarParam::param_newWidget()
{
    if (widget_is_visible) {
        Inkscape::UI::Widget::RegisteredScalar *rsu = Gtk::manage(new Inkscape::UI::Widget::RegisteredScalar(
            param_label, param_tooltip, param_key, *param_wr, param_effect->getRepr(), param_effect->getSPDoc()));

        rsu->setValue(value);
        rsu->setDigits(digits);
        rsu->setIncrements(inc_step, inc_page);
        rsu->setRange(min, max);
        rsu->setProgrammatically = false;
        if (add_slider) {
            rsu->addSlider();
        }
        if (_set_undo) {
            rsu->set_undo_parameters(_("Change scalar parameter"), INKSCAPE_ICON("dialog-path-effects"));
        }
        return dynamic_cast<Gtk::Widget *>(rsu);
    } else {
        return nullptr;
    }
}

void ScalarParam::param_set_digits(unsigned digits) { this->digits = digits; }

void ScalarParam::param_set_increments(double step, double page)
{
    inc_step = step;
    inc_page = page;
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
