// SPDX-License-Identifier: GPL-2.0-or-later
/** \file
 * LPE <slice> implementation: slices a path with respect to a given line.
 */
/*
 * Authors:
 *   Maximilian Albert
 *   Johan Engelen
 *   Abhishek Sharma
 *   Jabiertxof
 *
 * Copyright (C) Johan Engelen 2007 <j.b.c.engelen@utwente.nl>
 * Copyright (C) Maximilin Albert 2008 <maximilian.albert@gmail.com>
 *
 * Released under GNU GPL v2+, read the file 'COPYING' for more information.
 */

#include "live_effects/lpe-slice.h"

#include <gtkmm.h>

#include "2geom/affine.h"
#include "2geom/path-intersection.h"
#include "display/curve.h"
#include "helper/geom.h"
#include "live_effects/parameter/satellite-reference.h"
#include "object/sp-defs.h"
#include "object/sp-lpe-item.h"
#include "object/sp-path.h"
#include "object/sp-text.h"
#include "path-chemistry.h"
#include "path/path-boolop.h"
#include "style.h"
#include "svg/path-string.h"
#include "svg/svg.h"
#include "xml/sp-css-attr.h"

// this is only to flatten nonzero fillrule
#include "livarot/Path.h"
#include "livarot/Shape.h"

// TODO due to internal breakage in glibmm headers, this must be last:
#include <glibmm/i18n.h>

typedef FillRule FillRuleFlatten;

namespace Inkscape {
namespace LivePathEffect {
LPESlice::LPESlice(LivePathEffectObject *lpeobject) :
    Effect(lpeobject),
    // do not change name of this parameter us used in oncommit
    lpesatellites(_("lpesatellites"), _("Items satellites"), "lpesatellites", &wr, this, false),
    allow_transforms(_("Allow Transforms"), _("Allow transforms"), "allow_transforms", &wr, this, true),
    start_point(_("Slice line start"), _("Start point of slice line"), "start_point", &wr, this, _("Adjust start point of slice line")),
    end_point(_("Slice line end"), _("End point of slice line"), "end_point", &wr, this, _("Adjust end point of slice line")),
    center_point(_("Slice line mid"), _("Center point of slice line"), "center_point", &wr, this, _("Adjust center point of slice line"))
{
    registerParameter(&lpesatellites);
    registerParameter(&allow_transforms);
    registerParameter(&start_point);
    registerParameter(&end_point);
    registerParameter(&center_point);
    show_orig_path = true;
    apply_to_clippath_and_mask = false;
    previous_center = Geom::Point(0,0);
    center_point.param_widget_is_visible(false);
    reset = false;
    center_horiz = false;
    center_vert = false;
    allow_transforms_prev = allow_transforms;
    on_remove_all = false;
    container = nullptr;
    satellitestoclipboard = true;
}

LPESlice::~LPESlice()
{
    keep_paths = false;
    doOnRemove(nullptr);
};

bool
LPESlice::doOnOpen(SPLPEItem const* lpeitem) {
    bool fixed = false;
    Glib::ustring version = lpeversion.param_getSVGValue();
    if (version < "1.2") {
        std::vector<SPLPEItem *> lpeitems = getCurrrentLPEItems();
        if (lpeitems.size() >= 1) {
            sp_lpe_item_update_patheffect(lpeitems[0], false, true);
        }
        lpeversion.param_setValue("1.2", true);
        fixed = true;
        lpesatellites.write_to_SVG();
    }
    lpesatellites.start_listening();
    lpesatellites.connect_selection_changed();
    return fixed;
}

Gtk::Widget *
LPESlice::newWidget()
{
    // use manage here, because after deletion of Effect object, others might
    // still be pointing to this widget.
    Gtk::Box *vbox = Gtk::manage(new Gtk::Box(Gtk::ORIENTATION_VERTICAL));

    vbox->set_border_width(5);
    vbox->set_homogeneous(false);
    vbox->set_spacing(2);
    Gtk::Box *hbox = Gtk::manage(new Gtk::Box(Gtk::ORIENTATION_HORIZONTAL, 0));
    Gtk::Button *center_vert_button = Gtk::manage(new Gtk::Button(Glib::ustring(_("Vertical"))));
    center_vert_button->signal_clicked().connect(sigc::mem_fun(*this, &LPESlice::centerVert));
    center_vert_button->set_size_request(110, 20);
    Gtk::Button *center_horiz_button = Gtk::manage(new Gtk::Button(Glib::ustring(_("Horizontal"))));
    center_horiz_button->signal_clicked().connect(sigc::mem_fun(*this, &LPESlice::centerHoriz));
    center_horiz_button->set_size_request(110, 20);
    Gtk::Button *reset_button = Gtk::manage(new Gtk::Button(Glib::ustring(_("Reset styles"))));
    reset_button->signal_clicked().connect(sigc::mem_fun(*this, &LPESlice::resetStyles));
    reset_button->set_size_request(110, 20);

    vbox->pack_start(*hbox, true, true, 2);
    hbox->pack_start(*reset_button, false, false, 2);
    hbox->pack_start(*center_vert_button, false, false, 2);
    hbox->pack_start(*center_horiz_button, false, false, 2);
    std::vector<Parameter *>::iterator it = param_vector.begin();
    while (it != param_vector.end()) {
        if ((*it)->widget_is_visible) {
            Parameter *param = *it;
            Gtk::Widget *widg = dynamic_cast<Gtk::Widget *>(param->param_newWidget());
            Glib::ustring *tip = param->param_getTooltip();
            if (widg) {
                vbox->pack_start(*widg, true, true, 2);
                if (tip) {
                    widg->set_tooltip_text(*tip);
                } else {
                    widg->set_tooltip_text("");
                    widg->set_has_tooltip(false);
                }
            }
        }

        ++it;
    }
    if (Gtk::Widget *widg = defaultParamSet()) {
        vbox->pack_start(*widg, true, true, 2);
    }
    return dynamic_cast<Gtk::Widget *>(vbox);
}


void
LPESlice::centerVert(){
    center_vert = true;
    refresh_widgets = true;
    std::vector<SPLPEItem *> lpeitems = getCurrrentLPEItems();
    if (lpeitems.size() == 1) {
        sp_lpe_item = lpeitems[0];
        sp_lpe_item_update_patheffect(sp_lpe_item, false, false);
    }
}

void
LPESlice::centerHoriz(){
    center_horiz = true;
    refresh_widgets = true;
    std::vector<SPLPEItem *> lpeitems = getCurrrentLPEItems();
    if (lpeitems.size() == 1) {
        sp_lpe_item = lpeitems[0];
        sp_lpe_item_update_patheffect(sp_lpe_item, false, false);
    }
}

bool sp_has_path_data(SPItem *item, bool originald) 
{
    SPGroup *group = dynamic_cast<SPGroup *>(item);
    if (group) {
        std::vector<SPObject *> childs = group->childList(true);
        for (auto &child : childs) {
            SPItem *item = dynamic_cast<SPItem *>(child);
            if (sp_has_path_data(item, originald)) {
                return true;
            }
        }
    }
    SPShape *shape = dynamic_cast<SPShape *>(item);
    if (shape) {
        SPCurve const *c = shape->curve();
        if (c && !c->is_empty()) {
            return true;
        }
        if (originald) {
            if (shape->hasPathEffectRecursive()) {
                SPCurve const *c = shape->curveBeforeLPE();
                if (c && !c->is_empty()) {
                    return true;
                }
            }
        }
    }
    return false;
}
/*
 * Allow changing original-d to d to "reset" temporary the LPE
 * when the slice doesn't pass through item till sp_lpe_item is crossed
 */
void
LPESlice::originalDtoD(SPShape const *shape, SPCurve *curve)
{
    SPCurve const *c = shape->curveBeforeLPE();
    if (c && !c->is_empty()) {
        curve->set_pathvector(c->get_pathvector());
    }
}

/*
 * Allow changing original-d to d to "reset" temporary the LPE
 * when the slice doesn't pass through item till sp_lpe_item is crossed
 */
void
LPESlice::originalDtoD(SPItem *item)
{
    SPGroup *group = dynamic_cast<SPGroup *>(item);
    if (group) {
        std::vector<SPObject *> childs = group->childList(true);
        for (auto &child : childs) {
            SPItem *item = dynamic_cast<SPItem *>(child);
            originalDtoD(item);
        }
        return;
    }
    SPShape *shape = dynamic_cast<SPShape *>(item);
    if (shape) {
        SPCurve const *c = shape->curveBeforeLPE();
        if (c && !c->is_empty()) {
            shape->bbox_vis_cache_is_valid = false;
            shape->bbox_geom_cache_is_valid = false;
            shape->setCurveInsync(std::move(c));
            auto str = sp_svg_write_path(c->get_pathvector());
            shape->setAttribute("d", str);
        }
    }
}

void 
LPESlice::doAfterEffect (SPLPEItem const* lpeitem, SPCurve *curve)
{
    Glib::ustring version = lpeversion.param_getSVGValue();
    // this avoid regenerate fake satellites un undo after open a legacy LPE
    if (!is_load && version < "1.2") {
        return;
    }
    SPDocument *document = getSPDoc();
    if (!document) {
        return;
    }
    bool m_saved = DocumentUndo::getUndoSensitive(getSPDoc());
    DocumentUndo::ScopedInsensitive _no_undo(document);
    if (document->isPartial()) {
        DocumentUndo::setUndoSensitive(document, m_saved);
    } else if (document->isSeeking()) {
        return;
    }
    bool is_applied_on = false; 
    if (is_applied) {
        is_applied_on = true;
        is_applied = false;
    }
    bool write = false;
    bool active = !lpesatellites.data().size() || is_load;
    for (auto lpereference : lpesatellites.data()) {
        if (lpereference && lpereference->isAttached() && lpereference.get()->getObject() != nullptr) {
            active = true;
        }
    }
    if (!active && !is_load) {
        lpesatellites.clear();
        return;
    }

    LPESlice *nextslice = dynamic_cast<LPESlice *>(sp_lpe_item->getNextLPE(this));
    if (is_visible && (!nextslice || !nextslice->is_visible)) {   
        LPESlice *prevslice = dynamic_cast<LPESlice *>(sp_lpe_item->getPrevLPE(this));
        if (boundingbox_X.isSingular() || boundingbox_Y.isSingular()) {
            for (auto & iter : lpesatellites.data()) {
                SPObject *elemref;
                if (iter && iter->isAttached() && (elemref = iter->getObject())) {
                    if (auto *splpeitem = dynamic_cast<SPLPEItem *>(elemref)) {
                        splpeitem->setHidden(true);
                    }
                }
            }
            return;
        }    
        //ungroup
        if (!is_load && container && container != sp_lpe_item->parent && container != sp_lpe_item->parent->parent) {
            processObjects(LPE_UPDATE);
        } else if (!is_load && container && container != sp_lpe_item->parent) { // group
            processObjects(LPE_UPDATE);
        }
        std::vector<std::pair<Geom::Line, size_t> > slicer = getSplitLines();
        if (!slicer.size()) {
            return;
        }
        container = lpeitem->parent;
        objindex = 0;
        legacy = false;
        bool creation = write;
        split(sp_lpe_item, curve, slicer, 0, creation);
        bool connected = lpesatellites.is_connected();
        if (lpesatellites.data().size() && (creation || !connected)) {
            lpesatellites.write_to_SVG();
            lpesatellites.start_listening();
            lpesatellites.update_satellites(!connected);
        }
        bool maindata = sp_has_path_data(sp_lpe_item, true);
        for (auto & iter : lpesatellites.data()) {
            SPObject *elemref;
            if (iter && iter->isAttached() && (elemref = iter->getObject())) {
                SPLPEItem *splpeitem = dynamic_cast<SPLPEItem *>(elemref);
                if (splpeitem || lpeitem->isHidden()) {
                    if (!maindata || lpeitem->isHidden()) {
                        splpeitem->setHidden(true);
                    }
                    sp_lpe_item_update_patheffect(splpeitem, false, false);
                }
            }
        }
        if (!maindata) {
            if (!curve) { // group
                originalDtoD(sp_lpe_item);
            } else {
                originalDtoD(getCurrentShape(), curve);
            }
            return; 
        }
        reset = false;
        if (is_applied_on && prevslice) {
            sp_lpe_item_update_patheffect(sp_lpe_item, false, false);
            for (auto link : prevslice->lpesatellites.data()) {
                if (link && link->isAttached()) {
                    SPGroup *spgrp = dynamic_cast<SPGroup *>(link->getObject());
                    SPShape *spit = dynamic_cast<SPShape *>(link->getObject());
                    Glib::ustring transform = "";
                    Glib::ustring patheffects = "";
                    Geom::OptRect _gbbox = Geom::OptRect();
                    if (spgrp) {
                        if (spgrp->getAttribute("transform")) {
                            transform = spgrp->getAttribute("transform");
                        }
                        if (spgrp->getAttribute("inkscape:path-effect")) {
                            patheffects =  spgrp->getAttribute("inkscape:path-effect");
                        }
                        spgrp->setAttribute("transform", nullptr);   
                        spgrp->setAttribute("inkscape:path-effect", nullptr); 
                        _gbbox = spgrp->geometricBounds();
                    }
                    if (spit || spgrp) {
                        for (auto link2 : lpesatellites.data()) {
                            if (link2 && link2->isAttached()) {
                                SPGroup *spgrp2 = dynamic_cast<SPGroup *>(link2->getObject());
                                SPShape *spit2 = dynamic_cast<SPShape *>(link2->getObject());
                                if (spit && spit2) {
                                    auto edit = SPCurve::copy(spit->curveForEdit());
                                    auto edit2 = SPCurve::copy(spit2->curveForEdit());
                                    Geom::OptRect _bbox = edit->get_pathvector().boundsFast();
                                    Geom::OptRect _bbox2 = edit2->get_pathvector().boundsFast();
                                    if (_bbox && _bbox2) {
                                        (*_bbox).expandBy(1);
                                        if ((*_bbox).contains(*_bbox2)) {
                                            spit2->setAttribute("transform", spit->getAttribute("transform"));
                                            spit2->setAttribute("inkscape:path-effect", spit->getAttribute("inkscape:path-effect"));
                                            spit2->setAttribute("style", spit->getAttribute("style"));
                                        }
                                    }
                                } else if (spgrp && spgrp2) {  
                                    Geom::OptRect _gbbox2 = spgrp2->geometricBounds();
                                    if (_gbbox && _gbbox2) {
                                        (*_gbbox).expandBy(1);
                                        if ((*_gbbox).contains(*_gbbox2)) {
                                            spgrp2->setAttribute("transform", transform);
                                            spgrp2->setAttribute("inkscape:path-effect", patheffects);
                                            cloneStyle(spgrp, spgrp2);
                                        }
                                    }
                                }
                            }
                        }
                        if (spgrp) {
                            spgrp->setAttribute("transform", transform);
                            spgrp->setAttribute("inkscape:path-effect", patheffects);
                        }
                    }
                }
            }
            
        }
    } else {
        for (auto itemrf : lpesatellites.data()) {
            if (itemrf && itemrf->isAttached()) {
                SPLPEItem *splpeitem = dynamic_cast<SPLPEItem *>(itemrf->getObject());
                if (splpeitem) {
                    splpeitem->setHidden(true);
                    sp_lpe_item_update_patheffect(splpeitem, false, false);
                }
            }
        }
    }
}

bool
LPESlice::split(SPItem* item, SPCurve *curve, std::vector<std::pair<Geom::Line, size_t> > slicer, size_t splitindex, bool &creation) {
    bool splited = false;
    size_t nsplits = slicer.size();
    SPDocument *document = getSPDoc();
    if (!document) {
        return splited;
    }
    
    SPObject *elemref = nullptr;
    if (!is_load && container != sp_lpe_item->parent) {
        lpesatellites.read_from_SVG();
        return splited;
    }
    if (objindex < lpesatellites.data().size() && lpesatellites.data()[objindex]) {
        elemref = lpesatellites.data()[objindex]->getObject();
    }
    bool prevreset = reset;
    
    if (!elemref && item->getId()) {
        
        Glib::ustring elemref_id = Glib::ustring("slice-");
        elemref_id += Glib::ustring::format(slicer[splitindex].second);
        elemref_id += "-";
        Glib::ustring clean_id = item->getId();
        //First check is to allow effects on "satellittes"
        SPLPEItem *lpeitem = dynamic_cast<SPLPEItem *>(item);
        if (!lpeitem) {
            return splited;
        }
        if (!lpeitem->hasPathEffectOfType(SLICE) && clean_id.find("slice-") != Glib::ustring::npos) {
            clean_id = clean_id.replace(0,6,"");
            elemref_id += clean_id;
        } else {
            elemref_id += clean_id;
        }
        creation = true;
        if (is_load && (elemref = document->getObjectById(elemref_id))) {
            legacy = true;
            lpesatellites.link(elemref, objindex);
        } else {
            reset = true;
            Inkscape::XML::Node *phantom = createPathBase(item);
            if (!container) {
                return splited;
            }
            elemref = container->appendChildRepr(phantom);
            Inkscape::GC::release(phantom);
            lpesatellites.link(elemref, objindex);
        }
    }
    SPItem *other = dynamic_cast<SPItem *>(elemref);
    if (other) {
        objindex++;
        other->setHidden(false);
        if (nsplits) {
            cloneD(item, other, false);
            reset = prevreset;
            splited = splititem(item, curve, slicer[splitindex], true);
            splititem(other, nullptr, slicer[splitindex], false);
            if (!splited) {
                other->setHidden(true);
            }
            splitindex++;
            if (nsplits > splitindex) {
                SPLPEItem *splpeother = dynamic_cast<SPLPEItem *>(other);
                SPLPEItem *splpeitem = dynamic_cast<SPLPEItem *>(item);
                if (item == sp_lpe_item || !splpeitem->hasPathEffectOfType(SLICE)) {
                    split(item, curve, slicer, splitindex, creation);
                    if (other == sp_lpe_item || !splpeother->hasPathEffectOfType(SLICE)) {
                        split(other, nullptr, slicer, splitindex, creation);
                    }
                }
            }
        }
    }
    return splited;
}

std::vector<std::pair<Geom::Line, size_t> >
LPESlice::getSplitLines() {
    std::vector<std::pair<Geom::Line, size_t> > splitlines;
    std::vector<SPLPEItem *> lpeitems = getCurrrentLPEItems();
    if (lpeitems.size() >= 1) {
        sp_lpe_item = lpeitems[0];
    } else {
        return splitlines;
    }
    LPESlice *prevslice = dynamic_cast<LPESlice *>(sp_lpe_item->getPrevLPE(this));
    if (prevslice) {
        splitlines = prevslice->getSplitLines();
    }
    Geom::Line line_separation((Geom::Point)start_point, (Geom::Point)end_point);
    size_t index = sp_lpe_item->getLPEIndex(this);
    std::pair<Geom::Line, size_t> slice = std::make_pair(line_separation, index);
    splitlines.push_back(slice);
    return splitlines;
}

Inkscape::XML::Node *
LPESlice::createPathBase(SPObject *elemref) {
    SPDocument *document = getSPDoc();
    if (!document) {
        return nullptr;
    }
    Inkscape::XML::Document *xml_doc = getSPDoc()->getReprDoc();
    Inkscape::XML::Node *prev = elemref->getRepr();
    SPGroup *group = dynamic_cast<SPGroup *>(elemref);
    if (group) {
        Inkscape::XML::Node *container = xml_doc->createElement("svg:g");
        container->setAttribute("transform", prev->attribute("transform"));
        container->setAttribute("mask", prev->attribute("mask"));
        container->setAttribute("clip-path", prev->attribute("clip-path"));
        std::vector<SPItem*> const item_list = sp_item_group_item_list(group);
        Inkscape::XML::Node *previous = nullptr;
        for (auto sub_item : item_list) {
            Inkscape::XML::Node *resultnode = createPathBase(sub_item);
            container->addChild(resultnode, previous);
            previous = resultnode;
        }
        return container;
    }
    Inkscape::XML::Node *resultnode = xml_doc->createElement("svg:path");
    resultnode->setAttribute("transform", prev->attribute("transform"));
    resultnode->setAttribute("mask", prev->attribute("mask"));
    resultnode->setAttribute("clip-path", prev->attribute("clip-path"));
    return resultnode;
}

void
LPESlice::cloneD(SPObject *orig, SPObject *dest, bool is_original)
{
    if (!is_original && !g_strcmp0(sp_lpe_item->getId(), orig->getId())) {
        is_original = true;
    }
    SPDocument *document = getSPDoc();
    if (!document) {
        return;
    }
    SPItem *originalitem = dynamic_cast<SPItem *>(orig);
    if ( SP_IS_GROUP(orig) && SP_IS_GROUP(dest) && SP_GROUP(orig)->getItemCount() == SP_GROUP(dest)->getItemCount() ) {
        if (reset) {
            cloneStyle(orig, dest);
        }
        if (!allow_transforms) {
            auto str = sp_svg_transform_write(originalitem->transform);
            dest->setAttributeOrRemoveIfEmpty("transform", str);
        }
        std::vector< SPObject * > childs = orig->childList(true);
        size_t index = 0;
        for (auto &child : childs) {
            SPObject *dest_child = dest->nthChild(index);
            cloneD(child, dest_child, is_original);
            index++;
        }
        return;
    }

    SPShape * shape =  SP_SHAPE(orig);
    SPPath * path =  SP_PATH(dest);
    SPLPEItem *splpeitem = dynamic_cast<SPLPEItem *>(path);
    if (path && shape && splpeitem) {
        SPCurve const *c = shape->curve();
        if (c && !c->is_empty()) {
            auto str = sp_svg_write_path(c->get_pathvector());
            if (path->hasPathEffectRecursive()) {
                sp_lpe_item_enable_path_effects(path, false);
                dest->setAttribute("inkscape:original-d", str);
                sp_lpe_item_enable_path_effects(path, true);
                dest->setAttribute("d", str);
            } else {
                dest->setAttribute("d", str);
            }
            if (!allow_transforms) {
                auto str = sp_svg_transform_write(originalitem->transform);
                dest->setAttributeOrRemoveIfEmpty("transform", str);
            }
            if (reset) {
                cloneStyle(orig, dest);
            }
        }
    }
}

static fill_typ GetFillTyp(SPItem *item)
{
    SPCSSAttr *css = sp_repr_css_attr(item->getRepr(), "style");
    gchar const *val = sp_repr_css_property(css, "fill-rule", nullptr);
    if (val && strcmp(val, "nonzero") == 0) {
        return fill_nonZero;
    } else if (val && strcmp(val, "evenodd") == 0) {
        return fill_oddEven;
    } else {
        return fill_nonZero;
    }
}

bool
LPESlice::splititem(SPItem* item, SPCurve * curve, std::pair<Geom::Line, size_t> slicer, bool toggle, bool is_original) 
{
    bool splited = false;
    if (!is_original && !g_strcmp0(sp_lpe_item->getId(), item->getId())) {
        is_original = true;
    }
    Geom::Line line_separation = slicer.first;
    Geom::Point s = line_separation.initialPoint();
    Geom::Point e = line_separation.finalPoint();
    Geom::Point center = Geom::middle_point(s, e);
    SPGroup *group = dynamic_cast<SPGroup *>(item);
    if (group) {
        std::vector<SPObject *> childs = group->childList(true);
        for (auto &child : childs) {
            SPItem *dest_child = dynamic_cast<SPItem *>(child);
            // groups not need update curve
            splited = splititem(dest_child, nullptr, slicer, toggle, is_original) ? true : splited;
        }
        if (!is_original && group->hasPathEffectRecursive()) { 
            sp_lpe_item_update_patheffect(group, false, false);
        }
        return splited;
    }
    SPShape *shape = dynamic_cast<SPShape *>(item);
    SPPath *path = dynamic_cast<SPPath *>(item);
    if (shape) {
        SPCurve const *c;
        c = shape->curve();
        if (c) {
            Geom::PathVector original_pathv = pathv_to_linear_and_cubic_beziers(c->get_pathvector());
            sp_flatten(original_pathv, GetFillTyp(shape));
            Geom::PathVector path_out;
            Geom::Affine t = shape->transform;
            if (!dynamic_cast<SPGroup *>(sp_lpe_item)) {
                t = Geom::identity();
            }
            for (auto & path_it : original_pathv) {
                path_it *= t;
                if (path_it.empty()) {
                    continue;
                }
                Geom::PathVector tmp_pathvector;
                double time_start = 0.0;
                int position = 0;
                bool end_open = false;
                if (path_it.closed()) {
                    const Geom::Curve &closingline = path_it.back_closed();
                    if (!are_near(closingline.initialPoint(), closingline.finalPoint())) {
                        end_open = true;
                    }
                }
                Geom::Path original = path_it;
                if (end_open && path_it.closed()) {
                    original.close(false);
                    original.appendNew<Geom::LineSegment>( original.initialPoint() );
                    original.close(true);
                }
                double dir = line_separation.angle();
                Geom::Ray ray = line_separation.ray(0);
                double diagonal = Geom::distance(Geom::Point(boundingbox_X.min(),boundingbox_Y.min()),Geom::Point(boundingbox_X.max(),boundingbox_Y.max()));
                Geom::Rect bbox(Geom::Point(boundingbox_X.min(),boundingbox_Y.min()),Geom::Point(boundingbox_X.max(),boundingbox_Y.max()));
                double size_divider = Geom::distance(center, bbox) + diagonal;
                s = Geom::Point::polar(dir,size_divider) + center;
                e = Geom::Point::polar(dir + Geom::rad_from_deg(180),size_divider) + center;
                Geom::Path divider = Geom::Path(s);
                divider.appendNew<Geom::LineSegment>(e);
                std::vector<double> crossed;
                if (Geom::are_near(s,e)) {
                    continue;
                }
                Geom::Crossings cs = crossings(original, divider);
                for(auto & c : cs) {
                    crossed.push_back(c.ta);
                }
                double angle = Geom::deg_from_rad(ray.angle());
                bool toggleside = !(angle > 0 && angle < 180);
                std::sort(crossed.begin(), crossed.end());
                for (double time_end : crossed) {
                    if (time_start != time_end && time_end - time_start > Geom::EPSILON) {
                        Geom::Path portion = original.portion(time_start, time_end);
                        if (!portion.empty()) {
                            Geom::Point middle = portion.pointAt((double)portion.size()/2.0);
                            position = Geom::sgn(Geom::cross(e - s, middle - s));
                            if (toggleside) {
                                position *= -1;
                            }
                            if (toggle) {
                                position *= -1;
                            }
                            if (position == 1) {
                                tmp_pathvector.push_back(portion);
                            }
                            portion.clear();
                        }
                    }
                    time_start = time_end;
                }
                position = Geom::sgn(Geom::cross(e - s, original.finalPoint() - s));
                if (toggleside) {
                    position *= -1;
                }
                if (toggle) {
                    position *= -1;
                }
                if (cs.size()!=0 && (position == 1)) {
                    if (time_start != original.size() && original.size() - time_start > Geom::EPSILON) {
                        Geom::Path portion = original.portion(time_start, original.size());
                        if (!portion.empty()) {
                            if (!original.closed()) {
                                tmp_pathvector.push_back(portion);
                            } else {
                                if (cs.size() > 1 && tmp_pathvector.size() > 0 && tmp_pathvector[0].size() > 0 ) {
                                    tmp_pathvector[0] = tmp_pathvector[0].reversed();
                                    portion = portion.reversed();
                                    portion.setInitial(tmp_pathvector[0].finalPoint());
                                    tmp_pathvector[0].append(portion);
                                    tmp_pathvector[0] = tmp_pathvector[0].reversed();
                                } else {
                                    tmp_pathvector.push_back(portion);
                                }
                            }
                            portion.clear();
                        }
                    }
                }
                if (cs.size() > 0 && original.closed()) {
                    for (auto &path : tmp_pathvector) {
                        if (!path.closed()) {
                            path.close();
                        }
                    }
                }
                if (cs.size() == 0 && position == 1) {
                    splited = false;
                    tmp_pathvector.push_back(original);
                } else {
                    splited = true;
                }
                tmp_pathvector *= t.inverse();
                path_out.insert(path_out.end(), tmp_pathvector.begin(), tmp_pathvector.end());
                
                tmp_pathvector.clear();
            }
            if (curve && is_original) {
                curve->set_pathvector(path_out);
            }
            auto cpro = SPCurve::copy(shape->curve());
            if (cpro) {
                shape->bbox_vis_cache_is_valid = false;
                shape->bbox_geom_cache_is_valid = false;
                cpro->set_pathvector(path_out);
                shape->setCurveInsync(std::move(cpro));
                auto str = sp_svg_write_path(path_out);
                if (!is_original && shape->hasPathEffectRecursive()) {
                    sp_lpe_item_enable_path_effects(shape, false);
                    if (path) {
                        shape->setAttribute("inkscape:original-d", str);
                    } else {
                        shape->setAttribute("d", str);
                    }
                    sp_lpe_item_enable_path_effects(shape, true);
                } else {
                    shape->setAttribute("d", str);
                }
            }
        }
    }
    return splited;
}

void
LPESlice::doBeforeEffect (SPLPEItem const* lpeitem)
{
    SPDocument *document = getSPDoc();
    if (!document) {
        return;
    }
    if (!lpesatellites.data().size()) {
        lpesatellites.read_from_SVG();
        if (lpesatellites.data().size()) {
            lpesatellites.update_satellites();
        }
    }
    using namespace Geom;
    original_bbox(lpeitem, false, true);
    Point point_a(boundingbox_X.max(), boundingbox_Y.min());
    Point point_b(boundingbox_X.max(), boundingbox_Y.max());
    Point point_c(boundingbox_X.middle(), boundingbox_Y.middle());
    if (center_vert) {
        double dista = std::abs(end_point[Geom::Y] - boundingbox_Y.min());
        double distb = std::abs(start_point[Geom::Y] - boundingbox_Y.min());
        previous_center = Geom::Point(Geom::infinity(), g_random_double_range(0, 1000));
        end_point.param_setValue(
            Geom::Point(center_point[Geom::X], dista <= distb ? boundingbox_Y.min() : boundingbox_Y.max()), true);
        start_point.param_setValue(
            Geom::Point(center_point[Geom::X], dista > distb ? boundingbox_Y.min() : boundingbox_Y.max()), true);
        //force update
        center_vert = false;
    } else if (center_horiz) {
        double dista = std::abs(end_point[Geom::X] - boundingbox_X.min());
        double distb = std::abs(start_point[Geom::X] - boundingbox_X.min());
        previous_center = Geom::Point(Geom::infinity(), g_random_double_range(0, 1000));
        end_point.param_setValue(
            Geom::Point(dista <= distb ? boundingbox_X.min() : boundingbox_X.max(), center_point[Geom::Y]), true);
        start_point.param_setValue(
            Geom::Point(dista > distb ? boundingbox_X.min() : boundingbox_X.max(), center_point[Geom::Y]), true);
        //force update
        center_horiz = false;
    } else {
        if ((Geom::Point)start_point == (Geom::Point)end_point) {
            start_point.param_setValue(point_a);
            end_point.param_setValue(point_b);
            previous_center = Geom::middle_point((Geom::Point)start_point, (Geom::Point)end_point);
            center_point.param_setValue(previous_center);
            return;
        }
        if (are_near(previous_center, (Geom::Point)center_point, 0.001)) {
            center_point.param_setValue(Geom::middle_point((Geom::Point)start_point, (Geom::Point)end_point));
        } else {
            Geom::Point trans = center_point - Geom::middle_point((Geom::Point)start_point, (Geom::Point)end_point);
            start_point.param_setValue(start_point * trans);
            end_point.param_setValue(end_point * trans);
        }
    }
    if (allow_transforms_prev != allow_transforms) {
        LPESlice *nextslice = dynamic_cast<LPESlice *>(sp_lpe_item->getNextLPE(this));
        while (nextslice) {
            if (nextslice->allow_transforms != allow_transforms) {
                nextslice->allow_transforms_prev = allow_transforms;
                nextslice->allow_transforms.param_setValue(allow_transforms);
            }
            nextslice = dynamic_cast<LPESlice *>(sp_lpe_item->getNextLPE(nextslice));
        }
        LPESlice *prevslice = dynamic_cast<LPESlice *>(sp_lpe_item->getPrevLPE(this));
        while (prevslice) {
            if (prevslice->allow_transforms != allow_transforms) {
                prevslice->allow_transforms_prev = allow_transforms;
                prevslice->allow_transforms.param_setValue(allow_transforms);
            }
            prevslice = dynamic_cast<LPESlice *>(sp_lpe_item->getNextLPE(prevslice));
        }
    }
    allow_transforms_prev = allow_transforms;
}

void LPESlice::cloneStyle(SPObject *orig, SPObject *dest)
{
    for (auto iter : orig->style->properties()) {
        if (iter->style_src != SPStyleSrc::UNSET) {
            auto key = iter->id();
            if (key != SPAttr::FONT && key != SPAttr::D && key != SPAttr::MARKER) {
                const gchar *attr = orig->getAttribute(iter->name().c_str());
                if (attr) {
                    dest->setAttribute(iter->name(), attr);
                }
            }
        }
    }
    dest->setAttribute("style", orig->getAttribute("style"));
}

void
LPESlice::resetStyles(){
    std::vector<SPLPEItem *> lpeitems = getCurrrentLPEItems();
    if (lpeitems.size() == 1) {
        sp_lpe_item = lpeitems[0];
        LPESlice *nextslice = dynamic_cast<LPESlice *>(sp_lpe_item->getNextLPE(this));
        while (nextslice) {
            nextslice->reset = true;
            nextslice = dynamic_cast<LPESlice *>(sp_lpe_item->getNextLPE(nextslice));
        }
        reset = true;
        sp_lpe_item_update_patheffect(sp_lpe_item, false, false);
    }
}

void
LPESlice::doOnVisibilityToggled(SPLPEItem const* /*lpeitem*/)
{
    if (!is_visible) {
        for (auto itemrf : lpesatellites.data()) {
            if (itemrf && itemrf->isAttached()) {
                SPLPEItem *splpeitem = dynamic_cast<SPLPEItem *>(itemrf->getObject());
                if (splpeitem) {
                    splpeitem->setHidden(true);
                    sp_lpe_item_update_patheffect(splpeitem, false, false);
                }
            }
        }
    }
}


void
LPESlice::doOnRemove(SPLPEItem const* lpeitem)
{
    if (keep_paths) {
        processObjects(LPE_TO_OBJECTS);
        return;
    }
    processObjects(LPE_ERASE);
}

void
LPESlice::doOnApply (SPLPEItem const* lpeitem)
{
    using namespace Geom;
    original_bbox(lpeitem, false, true);
    LPESlice *prevslice = dynamic_cast<LPESlice *>(sp_lpe_item->getPrevLPE(this));
    if (prevslice) {
        allow_transforms_prev = prevslice->allow_transforms;
        allow_transforms.param_setValue(prevslice->allow_transforms);
    }
    Point point_a(boundingbox_X.middle(), boundingbox_Y.min());
    Point point_b(boundingbox_X.middle(), boundingbox_Y.max());
    Point point_c(boundingbox_X.middle(), boundingbox_Y.middle());
    start_point.param_setValue(point_a, true);
    start_point.param_update_default(point_a);
    end_point.param_setValue(point_b, true);
    end_point.param_update_default(point_b);
    center_point.param_setValue(point_c, true);
    end_point.param_update_default(point_c);
    previous_center = center_point;
    lpeversion.param_setValue("1.2", true);
    lpesatellites.update_satellites(true);
}


Geom::PathVector
LPESlice::doEffect_path (Geom::PathVector const & path_in)
{
    return path_in;
}

void
LPESlice::addCanvasIndicators(SPLPEItem const */*lpeitem*/, std::vector<Geom::PathVector> &hp_vec)
{
    using namespace Geom;
    hp_vec.clear();
    Geom::Path path;
    Geom::Point s = start_point;
    Geom::Point e = end_point;
    path.start( s );
    path.appendNew<Geom::LineSegment>( e );
    Geom::PathVector helper;
    helper.push_back(path);
    hp_vec.push_back(helper);
}

} //namespace LivePathEffect
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
