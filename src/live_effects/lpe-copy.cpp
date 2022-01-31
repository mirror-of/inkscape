// SPDX-License-Identifier: GPL-2.0-or-later
/** \file
 * LPE <copy_rotate> implementation
 */
/*
 * Authors:
 *   Maximilian Albert <maximilian.albert@gmail.com>
 *   Johan Engelen <j.b.c.engelen@alumnus.utwente.nl>
 *   Jabiertxo Arraiza Cenoz <jabier.arraiza@marker.es>
 * Copyright (C) Authors 2007-2012
 *
 * Released under GNU GPL v2+, read the file 'COPYING' for more information.
 */

#include "live_effects/lpe-copy.h"

#include <2geom/intersection-graph.h>
#include <2geom/path-intersection.h>
#include <2geom/sbasis-to-bezier.h>
#include <gdk/gdk.h>
#include <gtkmm.h>

#include "display/curve.h"
#include "helper/geom.h"
#include "live_effects/lpeobject.h"
#include "live_effects/parameter/satellite-reference.h"
#include "object/sp-object.h"
#include "object/sp-path.h"
#include "object/sp-shape.h"
#include "object/sp-text.h"
#include "path-chemistry.h"
#include "path/path-boolop.h"
#include "style.h"
#include "svg/path-string.h"
#include "svg/svg.h"
#include "xml/sp-css-attr.h"
#include "ui/knot/knot-holder.h"
#include "ui/knot/knot-holder-entity.h"
#include "ui/icon-loader.h"
#include "ui/icon-names.h"
// TODO due to internal breakage in glibmm headers, this must be last:
#include <glibmm/i18n.h>

namespace Inkscape {
namespace LivePathEffect {

namespace CoS {
    class KnotHolderEntityCopyGapX : public LPEKnotHolderEntity {
    public:
        KnotHolderEntityCopyGapX(LPECopy * effect) : LPEKnotHolderEntity(effect) {};
        ~KnotHolderEntityCopyGapX() override;
        void knot_set(Geom::Point const &p, Geom::Point const &origin, guint state) override;
        void knot_click(guint state) override;
        void knot_ungrabbed(Geom::Point const &p, Geom::Point const &origin, guint state) override;
        Geom::Point knot_get() const override;
        double startpos = dynamic_cast<LPECopy const*> (_effect)->gapx_unit;
    };
    class KnotHolderEntityCopyGapY : public LPEKnotHolderEntity {
    public:
        KnotHolderEntityCopyGapY(LPECopy * effect) : LPEKnotHolderEntity(effect) {};
        ~KnotHolderEntityCopyGapY() override;
        void knot_set(Geom::Point const &p, Geom::Point const &origin, guint state) override;
        void knot_click(guint state) override;
        void knot_ungrabbed(Geom::Point const &p, Geom::Point const &origin, guint state) override;
        Geom::Point knot_get() const override;
        double startpos = dynamic_cast<LPECopy const*> (_effect)->gapy_unit;
    };
} // CoS

LPECopy::LPECopy(LivePathEffectObject *lpeobject) :
    Effect(lpeobject),
    // do not change name of this parameter us used in oncommit
    unit(_("Unit:"), _("Unit"), "unit", &wr, this, "px"),
    lpesatellites(_("lpesatellites"), _("Items satellites"), "lpesatellites", &wr, this, false),
    num_cols(_("Columns"), _("Number of columns of copies"), "num_cols", &wr, this, 3),
    num_rows(_("Rows"), _("Number of rows of copies"), "num_rows", &wr, this, 3),
    gapx(_("Gap X"), _("Gap space between copies in X"), "gapx", &wr, this, 0.0),
    gapy(_("Gap Y"), _("Gap space between copies in Y"), "gapy", &wr, this, 0.0),
    scale(_("Scale %"), _("Scale copies"), "scale", &wr, this, 100.0),
    rotate(_("Rotate°"), _("Rotate copies"), "rotate", &wr, this, 0.0),
    offset(_("Offset %"), _("Offset %"), "offset", &wr, this, 0.0),
    offset_type(_("Offset type"), _("Offset Type (rows/cols)"), "offset_type", &wr, this, false),
    interpolate_scalex(_("Interpolate scale X"), _("Interpolate rotate X"), "interpolate_scalex", &wr, this, false),
    interpolate_scaley(_("Interpolate scale Y"), _("Interpolate rotate Y"), "interpolate_scaley", &wr, this, false),
    shrink_interp(_("Shirnk gap"), _("Minimice offsets on scale interpolated, not works with rotations"), "shrink_interp", &wr, this, false),
    interpolate_rotatex(_("Interpolate rotate X"), _("Interpolate rotate X"), "interpolate_rotatex", &wr, this, false),
    interpolate_rotatey(_("Interpolate rotate Y"), _("Interpolate rotate Y"), "interpolate_rotatey", &wr, this, false),
    split_items(_("Split elements"), _("Split elements, so each can have its own style"), "split_items", &wr, this, false),
    mirrorrowsx(_("Mirror rows in X"), _("Mirror rows in X"), "mirrorrowsx", &wr, this, false),
    mirrorrowsy(_("Mirror rows in Y"), _("Mirror rows in Y"), "mirrorrowsy", &wr, this, false),
    mirrorcolsx(_("Mirror cols in X"), _("Mirror cols in X"), "mirrorcolsx", &wr, this, false),
    mirrorcolsy(_("Mirror cols in Y"), _("Mirror cols in Y"), "mirrorcolsy", &wr, this, false),
    mirrortrans(_("Mirror transforms"), _("Mirror transforms"), "mirrortrans", &wr, this, false),
    link_styles(_("Link styles"), _("Link styles on split mode"), "link_styles", &wr, this, false)
    
{
    show_orig_path = true;
    _provides_knotholder_entities = true;
    // register all your parameters here, so Inkscape knows which parameters this effect has:
    registerParameter(&unit);
    registerParameter(&lpesatellites);
    registerParameter(&num_rows);
    registerParameter(&num_cols);
    registerParameter(&gapx);
    registerParameter(&gapy);
    registerParameter(&offset);
    registerParameter(&offset_type);
    registerParameter(&scale);
    registerParameter(&rotate);
    registerParameter(&mirrorrowsx);
    registerParameter(&mirrorrowsy);
    registerParameter(&mirrorcolsx);
    registerParameter(&mirrorcolsy);
    registerParameter(&mirrortrans);
    registerParameter(&shrink_interp);
    registerParameter(&split_items);
    registerParameter(&link_styles);
    registerParameter(&interpolate_scalex);
    registerParameter(&interpolate_scaley);
    registerParameter(&interpolate_rotatex);
    registerParameter(&interpolate_rotatey);
    
    num_cols.param_set_range(1, std::numeric_limits<gint>::max());
    num_cols.param_make_integer();
    num_cols.param_set_increments(1.0, 10.0);
    num_rows.param_set_range(1, std::numeric_limits<gint>::max());
    num_rows.param_make_integer();
    num_rows.param_set_increments(1.0, 10.0);
    scale.param_set_range(-9999,9999); // we neeed the input a bit tiny so this seems enought
    scale.param_make_integer();
    scale.param_set_increments(1.0, 10.0);
    rotate.param_set_increments(1.0, 10.0);
    rotate.param_set_range(-900, 900);
    offset.param_set_range(-300, 300);
    offset.param_set_increments(1.0, 10.0);

    apply_to_clippath_and_mask = true;
    _provides_knotholder_entities = true;
    prev_num_cols = num_cols;
    prev_num_rows = num_rows;
    _knotholder = nullptr;
    reset = link_styles;
}

LPECopy::~LPECopy()
{
    keep_paths = false;
    doOnRemove(nullptr);
};

bool LPECopy::doOnOpen(SPLPEItem const *lpeitem)
{
    bool fixed = false;
    if (!is_load || is_applied) {
        return fixed;
    }
    if (!split_items) {
        return fixed;
    }
    lpesatellites.update_satellites();
    container = lpeitem->parent;
    return fixed;
}

void
LPECopy::doAfterEffect (SPLPEItem const* lpeitem, SPCurve *curve)
{
    if (split_items) {
        SPDocument *document = getSPDoc();
        if (!document) {
            return;
        }
        bool write = false;
        bool active = !lpesatellites.data().size();
        for (auto lpereference : lpesatellites.data()) {
            if (lpereference && lpereference->isAttached() && lpereference.get()->getObject() != nullptr) {
                active = true;
            }
        }
        if (!active && !is_load && prev_split) {
            lpesatellites.clear();
            prev_num_cols = 0;
            prev_num_rows = 0;
        }
        prev_split = split_items;

        container = sp_lpe_item->parent;
        if (prev_num_cols * prev_num_rows != num_cols * num_rows) {
            write = true;
            size_t pos = 0;
            for (auto lpereference : lpesatellites.data()) {
                if (lpereference && lpereference->isAttached()) {
                    SPItem *copies = dynamic_cast<SPItem *>(lpereference->getObject());
                    if (copies) {
                        if (pos > num_cols * num_rows - 2) {
                            copies->setHidden(true);
                        } else if (copies->isHidden()) {
                            copies->setHidden(false);
                        }
                    }
                }
                pos++;
            }
            prev_num_cols = num_cols;
            prev_num_rows = num_rows;
        }
        if (!gap_bbox) {
            return;
        }
        Geom::Point center = (*gap_bbox).midpoint();
        bool forcewrite = false;
        Geom::Affine origin = Geom::Translate(center).inverse();
        if (!interpolate_rotatex && !interpolate_rotatey) {
            origin *= Geom::Rotate::from_degrees(rotate);
        }
        if (!interpolate_scalex && !interpolate_scaley) {
            origin *= Geom::Scale(scale/100.0,scale/100.0);
        }
        origin *= Geom::Translate(center);
        origin = origin.inverse();
        size_t counter = 0;
        size_t total = num_rows * num_cols;
        double gapscalex = 0;
        double maxheight = 0;
        double maxwidth = 0;
        double minheight = std::numeric_limits<double>::max();
        double y[(int)num_cols] = {}; 
        double ygap[(int)num_cols] = {}; 
        double yset = 0;
        Geom::OptRect prev_bbox;
        Geom::OptRect bbox = sp_lpe_item->geometricBounds();
        if (!bbox) {
            return;
        }
        for (int i = 0; i < num_rows; ++i) {
            double fracy = 1;
            if (num_rows != 1) {
                fracy = i/(double)(num_rows - 1);
            }
            for (int j = 0; j < num_cols; ++j) {
                double x = 0;
                double fracx = 1;
                if (num_cols != 1) {
                    fracx = j/(double)(num_cols - 1);
                }
                double fract = counter / (double)total;
                Geom::Affine r = Geom::identity();
                if(mirrorrowsx || mirrorrowsy || mirrorcolsx || mirrorcolsy) {
                    gint mx = 1;
                    gint my = 1;
                    if (mirrorrowsx && mirrorcolsx) {
                        mx = (j+i)%2 != 0 ? -1 : 1;
                    } else {
                        if (mirrorrowsx) {
                            mx = i%2 != 0 ? -1 : 1;
                        } else if (mirrorcolsx) {
                            mx = j%2 != 0 ? -1 : 1;
                        }
                    }
                    if (mirrorrowsy && mirrorcolsy) {
                        my = (j+i)%2 != 0 ? -1 : 1;
                    } else {                     
                        if (mirrorrowsy) {
                            my = i%2 != 0 ? -1 : 1; 
                        } else if (mirrorcolsy) {
                            my = j%2 != 0 ? -1 : 1;
                        }
                    }
                    r *= Geom::Translate(center).inverse();
                    r *= Geom::Scale(mx, my);
                    r *= Geom::Translate(center);
                }
                if (mirrortrans && interpolate_scalex && i%2 != 0) {
                    fracx = 1-fracx;
                }
                double fracyin = fracy;
                if (mirrortrans && interpolate_scaley && j%2 != 0) {
                    fracyin = 1-fracyin;
                }
                /* if (mirrortrans && interpolate_scaley && interpolate_scalex) {
                    fract = 1-fract;
                } */
                double rotatein = rotate;
                if (interpolate_rotatex && interpolate_rotatey) {
                    rotatein = rotatein*fract;
                } else if (interpolate_rotatex) {
                    rotatein = rotatein*fracx;
                } else if (interpolate_rotatey) {
                    rotatein = rotatein*fracyin;
                }
                if (mirrortrans && 
                    ((interpolate_rotatex && i%2 != 0) ||
                    (interpolate_rotatey && j%2 != 0) ||
                    (interpolate_rotatex && interpolate_rotatey))) 
                {
                    rotatein *=-1;
                }
                double scalein = 1;
                double scalegap = scale/100.0 - scalein;
                if (interpolate_scalex && interpolate_scaley) {
                    scalein = (scalegap*fract) + 1;
                } else if (interpolate_scalex) {
                    scalein = (scalegap*fracx) + 1;
                } else if (interpolate_scaley) {
                    scalein = (scalegap*fracyin) + 1;
                } else {
                    //scalein = scale/100.0;
                }
                r *= Geom::Translate(center).inverse();
                if (!interpolate_rotatex && !interpolate_rotatey) {
                    r *= Geom::Rotate::from_degrees(rotatein).inverse();
                }
                r *= Geom::Rotate::from_degrees(rotatein);
                r *= Geom::Scale(scalein, scalein);
                r *= Geom::Translate(center);
                double heightrows = original_height * (scale/100.0);
                double widthcols = original_width * (scale/100.0);
                double fixed_heightrows = heightrows;
                double fixed_widthcols = widthcols;
                bool shrink_interpove = shrink_interp;
                if (rotatein) {
                    shrink_interpove = false;
                }
                if (scale != 100 && (interpolate_scalex || interpolate_scaley)) {// && !interpolate_rotatex && !interpolate_rotatey) {
                    maxheight = std::max(maxheight,(*bbox).height() * scalein);
                    maxwidth = std::max(maxwidth,(*bbox).width() * scalein);
                    minheight = std::min(minheight,(*bbox).height() * scalein);
                    widthcols = std::max(original_width * (scale/100.0),original_width);
                    heightrows = std::max(original_height * (scale/100.0),original_height);
                    fixed_widthcols = widthcols;
                    fixed_heightrows = heightrows;
                    double cx = (*bbox).width() * scalein;
                    double cy = (*bbox).height() * scalein; 
                    cx += gapx_unit;
                    cy += gapy_unit;
                    if (shrink_interpove && (!interpolate_scalex || !interpolate_scaley)) {
                        double px = 0;
                        double py = 0; 
                        if (prev_bbox) {                    
                            px = (*prev_bbox).width();
                            py = (*prev_bbox).height();
                            px += gapx_unit;
                            py += gapy_unit;
                        }
                        if (interpolate_scalex) {
                            if (j) {
                                x = cx - ((cx-px)/2.0);
                                gapscalex += x;
                                x = gapscalex;
                            } else {
                                x = 0;
                                gapscalex = 0;
                            }
                            widthcols = 0;
                        } else if (interpolate_scaley) { 
                            x = 0;
                            if (i == 1) {
                                ygap[j] = ((cy-y[j])/2.0);
                                y[j] += ygap[j];
                            }
                            yset = y[j];
                            y[j] += cy + ygap[j];
                            heightrows = 0;
                        }                        
                    }
                    prev_bbox = bbox;
                } else {
                    y[j] = 0;
                }
                if (!counter) {
                    counter++;
                    continue;
                }
                double xset = x;
                xset += widthcols * j;
                if (heightrows) {
                    yset = heightrows * i; 
                }

                toItem(counter -1, reset, write);
                SPItem * item = dynamic_cast<SPItem *>(lpesatellites.data()[counter-1]->getObject());
                if (item) {
                    prev_bbox = item->geometricBounds();
                    (*prev_bbox) *= r;
                    double offset_x = 0;
                    double offset_y = 0;
                    if (offset != 0) {
                        if (offset_type && j%2) {
                            offset_y = fixed_heightrows/(100.0/(double)offset);
                        }
                        if (!offset_type && i%2) {
                            offset_x = fixed_widthcols/(100.0/(double)offset);
                        }
                    }
                    r *= Geom::Translate(Geom::Point(xset + offset_x,yset + offset_y));
                    item->transform = r * sp_item_transform_repr(sp_lpe_item);
                    item->doWriteTransform(item->transform);
                    item->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
                }
                forcewrite = forcewrite || write;
                counter++;
            }
        }
        //we keep satelites connected and actived if write needed
        bool connected = lpesatellites.is_connected();
        if (forcewrite || !connected) {
            lpesatellites.write_to_SVG();
            lpesatellites.start_listening();
            lpesatellites.update_satellites(!connected);
        }
        reset = link_styles;
    }
}

void LPECopy::cloneStyle(SPObject *orig, SPObject *dest)
{
    dest->setAttribute("transform", orig->getAttribute("transform"));
    dest->setAttribute("style", orig->getAttribute("style"));
    dest->setAttribute("mask", orig->getAttribute("mask"));
    dest->setAttribute("clip-path", orig->getAttribute("clip-path"));
    dest->setAttribute("class", orig->getAttribute("class"));
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
}

void LPECopy::cloneD(SPObject *orig, SPObject *dest)
{
    SPDocument *document = getSPDoc();
    if (!document) {
        return;
    }
    if ( SP_IS_GROUP(orig) && SP_IS_GROUP(dest) && SP_GROUP(orig)->getItemCount() == SP_GROUP(dest)->getItemCount() ) {
        if (reset) {
            cloneStyle(orig, dest);
        }
        std::vector< SPObject * > childs = orig->childList(true);
        size_t index = 0;
        for (auto & child : childs) {
            SPObject *dest_child = dest->nthChild(index);
            cloneD(child, dest_child);
            index++;
        }
        return;
    }  else if( SP_IS_GROUP(orig) && SP_IS_GROUP(dest) && SP_GROUP(orig)->getItemCount() != SP_GROUP(dest)->getItemCount()) {
        split_items.param_setValue(false);
        return;
    }

    if ( SP_IS_TEXT(orig) && SP_IS_TEXT(dest) && SP_TEXT(orig)->children.size() == SP_TEXT(dest)->children.size()) {
        if (reset) {
            cloneStyle(orig, dest);
        }
        size_t index = 0;
        for (auto & child : SP_TEXT(orig)->children) {
            SPObject *dest_child = dest->nthChild(index);
            cloneD(&child, dest_child);
            index++;
        }
    }
    
    SPShape * shape =  SP_SHAPE(orig);
    SPPath * path =  SP_PATH(dest);
    if (shape) {
        SPCurve const *c = shape->curve();
        if (c) {
            auto str = sp_svg_write_path(c->get_pathvector());
            if (shape && !path) {
                const char *id = dest->getAttribute("id");
                const char *style = dest->getAttribute("style");
                Inkscape::XML::Document *xml_doc = dest->document->getReprDoc();
                Inkscape::XML::Node *dest_node = xml_doc->createElement("svg:path");
                dest_node->setAttribute("id", id);
                dest_node->setAttribute("style", style);
                dest->updateRepr(xml_doc, dest_node, SP_OBJECT_WRITE_ALL);
                path =  SP_PATH(dest);
            }
            path->setAttribute("d", str);
        } else {
            path->removeAttribute("d");
        }
        
    }
    if (reset) {
        cloneStyle(orig, dest);
    } 
}

Inkscape::XML::Node *
LPECopy::createPathBase(SPObject *elemref) {
    SPDocument *document = getSPDoc();
    if (!document) {
        return nullptr;
    }
    Inkscape::XML::Document *xml_doc = document->getReprDoc();
    Inkscape::XML::Node *prev = elemref->getRepr();
    SPGroup *group = dynamic_cast<SPGroup *>(elemref);
    if (group) {
        Inkscape::XML::Node *container = xml_doc->createElement("svg:g");
        container->setAttribute("transform", prev->attribute("transform"));
        container->setAttribute("mask", prev->attribute("mask"));
        container->setAttribute("clip-path", prev->attribute("clip-path"));
        container->setAttribute("class", prev->attribute("class"));
        container->setAttribute("style", prev->attribute("style"));
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
    resultnode->setAttribute("style", prev->attribute("style"));
    resultnode->setAttribute("mask", prev->attribute("mask"));
    resultnode->setAttribute("clip-path", prev->attribute("clip-path"));
    resultnode->setAttribute("class", prev->attribute("class"));
    return resultnode;
}

void
LPECopy::toItem(size_t i, bool reset, bool &write)
{
    SPDocument *document = getSPDoc();
    if (!document) {
        return;
    }
    
    SPObject *elemref = nullptr;
    if (container != sp_lpe_item->parent) {
        lpesatellites.read_from_SVG();
        return;
    }
    if (lpesatellites.data().size() > i && lpesatellites.data()[i]) {
        elemref = lpesatellites.data()[i]->getObject();
    }
    Inkscape::XML::Node *phantom = nullptr;
    bool creation = false;
    if (elemref) {
        phantom = elemref->getRepr();
    } else {
        creation = true;
        phantom = createPathBase(sp_lpe_item);
        reset = true;
        elemref = container->appendChildRepr(phantom);

        Inkscape::GC::release(phantom);
    }
    cloneD(sp_lpe_item, elemref);
    reset = link_styles;
    if (creation) {
        write = true;
        lpesatellites.link(elemref, i);
    } 
}

Gtk::Widget * LPECopy::newWidget()
{
    // use manage here, because after deletion of Effect object, others might
    // still be pointing to this widget.
    Gtk::Box *vbox = Gtk::manage(new Gtk::Box(Gtk::ORIENTATION_VERTICAL));

    vbox->set_border_width(5);
    vbox->set_homogeneous(false);
    vbox->set_spacing(2);
    Gtk::Widget *combo = nullptr;
    std::vector<Parameter *>::iterator it = param_vector.begin();
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    bool usemirroricons = prefs->getBool("/live_effects/copy/mirroricons",true);
    while (it != param_vector.end()) {
        if ((*it)->widget_is_visible) {
            Parameter *param = *it;
            Gtk::Widget *widg = dynamic_cast<Gtk::Widget *>(param->param_newWidget());
            Glib::ustring *tip = param->param_getTooltip();
            if (widg) {
                if (param->param_key == "unit") {
                    auto widgcombo = dynamic_cast<Inkscape::UI::Widget::RegisteredUnitMenu*>(widg);
                    delete widgcombo->get_children()[0];
                    combo = dynamic_cast<Gtk::Widget*>(widgcombo);
                    if (usemirroricons) {
                        Gtk::RadioButton::Group group;
                        Gtk::Frame * frame  = Gtk::manage(new Gtk::Frame("Mirroring mode"));
                        Gtk::Box * cbox  = Gtk::manage(new Gtk::Box(Gtk::ORIENTATION_HORIZONTAL,8));
                        Gtk::Box * vbox1 = Gtk::manage(new Gtk::Box(Gtk::ORIENTATION_VERTICAL,0));
                        Gtk::Box * hbox1 = Gtk::manage(new Gtk::Box(Gtk::ORIENTATION_HORIZONTAL,0));
                        Gtk::Box * hbox2 = Gtk::manage(new Gtk::Box(Gtk::ORIENTATION_HORIZONTAL,0));
                        Gtk::Box * vbox2 = Gtk::manage(new Gtk::Box(Gtk::ORIENTATION_VERTICAL,0));
                        Gtk::Box * hbox3 = Gtk::manage(new Gtk::Box(Gtk::ORIENTATION_HORIZONTAL,0));
                        Gtk::Box * hbox4 = Gtk::manage(new Gtk::Box(Gtk::ORIENTATION_HORIZONTAL,0));
                        vbox2->set_margin_left(5);
                        cbox->pack_start(*vbox1, false, false, 0);
                        cbox->pack_start(*vbox2, false, false, 0);
                        cbox->set_margin_left(3);
                        cbox->set_margin_right(3);
                        cbox->set_margin_bottom(3);
                        frame->add(*cbox);
                        vbox->pack_start(*frame, false, false, 0);
                        vbox1->pack_start(*hbox1, false, false, 0);
                        vbox1->pack_start(*hbox2, false, false, 0);
                        vbox2->pack_start(*hbox3, false, false, 0);
                        vbox2->pack_start(*hbox4, false, false, 0);
                        generate_buttons(hbox1, group, 0);
                        generate_buttons(hbox2, group, 1);
                        generate_buttons(hbox3, group, 2);
                        generate_buttons(hbox4, group, 3);
                    }
                    ++it;
                    continue;
                }
                if (param->param_key == "offset_type" || 
                    param->param_key == "mirrorrowsx" && usemirroricons ||
                    param->param_key == "mirrorrowsy" && usemirroricons ||
                    param->param_key == "mirrorcolsx" && usemirroricons ||
                    param->param_key == "mirrorcolsy" && usemirroricons ||
                    param->param_key == "interpolate_rotatex" ||
                    param->param_key == "interpolate_rotatey" ||
                    param->param_key == "interpolate_scalex" ||
                    param->param_key == "interpolate_scaley")
                {
                    ++it;
                    continue;
                }
                if (param->param_key == "offset") {
                    Gtk::Box *bwidg = dynamic_cast<Gtk::Box *>(widg);
                    Gtk::Box *container = Gtk::manage(new Gtk::Box(Gtk::ORIENTATION_HORIZONTAL,0));
                    Gtk::RadioButton::Group group;
                    Gtk::RadioToolButton * rows = Gtk::manage(new Gtk::RadioToolButton(group, _("Offset rows")));
                    rows->set_icon_name(INKSCAPE_ICON("rows"));
                    Gtk::RadioToolButton * cols = Gtk::manage(new Gtk::RadioToolButton(group, _("Offset cols")));
                    cols->set_icon_name(INKSCAPE_ICON("cols"));
                    if (offset_type) {
                        cols->set_active();
                    } else {
                        rows->set_active();
                    }
                    container->pack_start(*rows, false, false, 0.5);
                    container->pack_start(*cols, false, false, 0.5);
                    cols->signal_clicked().connect(sigc::mem_fun (*this, &LPECopy::setOffsetCols));
                    rows->signal_clicked().connect(sigc::mem_fun (*this, &LPECopy::setOffsetRows));
                    bwidg->pack_start(*container, false, false, 0);
                    vbox->pack_start(*bwidg, true, true, 2);
                } else if (param->param_key == "scale") {
                    Gtk::Box *bwidg = dynamic_cast<Gtk::Box *>(widg);
                    Gtk::Box *container = Gtk::manage(new Gtk::Box(Gtk::ORIENTATION_HORIZONTAL,0));
                    Gtk::RadioButton::Group group;
                    Gtk::RadioToolButton * cols = Gtk::manage(new Gtk::RadioToolButton(group, _("Interpolate X")));
                    cols->set_icon_name(INKSCAPE_ICON("interpolate-scale-x"));
                    Gtk::RadioToolButton * rows = Gtk::manage(new Gtk::RadioToolButton(group, _("Interpolate Y")));
                    rows->set_icon_name(INKSCAPE_ICON("interpolate-scale-y"));
                    Gtk::RadioToolButton * both = Gtk::manage(new Gtk::RadioToolButton(group, _("Interpolate both")));
                    both->set_icon_name(INKSCAPE_ICON("interpolate-scale-both"));
                    Gtk::RadioToolButton * none = Gtk::manage(new Gtk::RadioToolButton(group, _("interpolate none")));
                    none->set_icon_name(INKSCAPE_ICON("interpolate-scale-none"));
                    if (interpolate_scalex && interpolate_scaley) {
                        both->set_active();
                    } else if (interpolate_scalex) {
                        cols->set_active();
                    } else if (interpolate_scaley) {
                        rows->set_active();
                    } else {
                        none->set_active();
                    }
                    container->pack_start(*rows, false, false, 0.5);
                    container->pack_start(*cols, false, false, 0.5);
                    container->pack_start(*both, false, false, 0.5);
                    container->pack_start(*none, false, false, 0.5);
                    none->signal_clicked().connect(sigc::bind<bool,bool>(sigc::mem_fun(*this, &LPECopy::setScaleInterpolate), false, false));
                    cols->signal_clicked().connect(sigc::bind<bool,bool>(sigc::mem_fun(*this, &LPECopy::setScaleInterpolate), true, false));
                    rows->signal_clicked().connect(sigc::bind<bool,bool>(sigc::mem_fun(*this, &LPECopy::setScaleInterpolate), false, true));
                    both->signal_clicked().connect(sigc::bind<bool,bool>(sigc::mem_fun(*this, &LPECopy::setScaleInterpolate), true, true));
                    bwidg->pack_start(*container, false, false, 0);
                    vbox->pack_start(*bwidg, true, true, 2);
                } else if (param->param_key == "rotate") {
                    Gtk::Box *bwidg = dynamic_cast<Gtk::Box *>(widg);
                    Gtk::Box *container = Gtk::manage(new Gtk::Box(Gtk::ORIENTATION_HORIZONTAL,0));
                    Gtk::RadioButton::Group group;
                    Gtk::RadioToolButton * cols = Gtk::manage(new Gtk::RadioToolButton(group, _("Interpolate X")));
                    cols->set_icon_name(INKSCAPE_ICON("interpolate-rotate-x"));
                    Gtk::RadioToolButton * rows = Gtk::manage(new Gtk::RadioToolButton(group, _("Interpolate Y")));
                    rows->set_icon_name(INKSCAPE_ICON("interpolate-rotate-y"));
                    Gtk::RadioToolButton * both = Gtk::manage(new Gtk::RadioToolButton(group, _("Interpolate both")));
                    both->set_icon_name(INKSCAPE_ICON("interpolate-rotate-both"));
                    Gtk::RadioToolButton * none = Gtk::manage(new Gtk::RadioToolButton(group, _("Interpolate none")));
                    none->set_icon_name(INKSCAPE_ICON("interpolate-rotate-none"));
                    if (interpolate_rotatex && interpolate_rotatey) {
                        both->set_active();
                    } else if (interpolate_rotatex) {
                        cols->set_active();
                    } else if (interpolate_rotatey) {
                        rows->set_active();
                    } else {
                        none->set_active();
                    }
                    container->pack_start(*rows, false, false, 0.5);
                    container->pack_start(*cols, false, false, 0.5);
                    container->pack_start(*both, false, false, 0.5);
                    container->pack_start(*none, false, false, 0.5);
                    none->signal_clicked().connect(sigc::bind<bool,bool>(sigc::mem_fun(*this, &LPECopy::setRotateInterpolate), false, false));
                    cols->signal_clicked().connect(sigc::bind<bool,bool>(sigc::mem_fun(*this, &LPECopy::setRotateInterpolate), true, false));
                    rows->signal_clicked().connect(sigc::bind<bool,bool>(sigc::mem_fun(*this, &LPECopy::setRotateInterpolate), false, true));
                    both->signal_clicked().connect(sigc::bind<bool,bool>(sigc::mem_fun(*this, &LPECopy::setRotateInterpolate), true, true));
                    bwidg->pack_start(*container, false, false, 0);
                    vbox->pack_start(*bwidg, true, true, 2);
                } else if (param->param_key == "gapx") {
                    Gtk::Box *bwidg = dynamic_cast<Gtk::Box *>(widg);
                    bwidg->pack_start(*combo, true, true, 2);
                    vbox->pack_start(*bwidg, true, true, 2);
                } else {
                    vbox->pack_start(*widg, true, true, 2);
                }
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
    if(Gtk::Widget* widg = defaultParamSet()) {
        vbox->pack_start(*widg, true, true, 2);
    }
    return dynamic_cast<Gtk::Widget *>(vbox);
}

void
LPECopy::generate_buttons(Gtk::Box *container, Gtk::RadioButton::Group &group, gint pos)
{
    for (int i = 0; i < 4; i++) {
        gint position = (pos * 4) + i;
        Glib::ustring result = getMirrorMap(position);
        Gtk::RadioToolButton * button = Gtk::manage(new Gtk::RadioToolButton(group));
        Glib::ustring iconname = "mirroring";
        iconname += "-";
        iconname += result;
        button->set_icon_name(INKSCAPE_ICON(iconname));
        if (getActiveMirror(position)) {
            _updating = true;
            button->set_active();
            _updating = false;
        }
        button->signal_clicked().connect(sigc::bind<gint>(sigc::mem_fun(*this, &LPECopy::setMirroring),position));
        gint zero = Glib::ustring("0")[0];
        Glib::ustring tooltip = result[0] == zero ? "" : "rx+";
        tooltip += result[1] == zero ? "" : "ry+";
        tooltip += result[2] == zero ? "" : "cx+";
        tooltip += result[3] == zero ? "" : "cy+";
        if (tooltip.size()) {
            tooltip.erase(tooltip.size()-1);
        }
        button->set_tooltip_text(tooltip);
        button->set_margin_left(1);
        container->pack_start(*button, false, false, 0.5);
    }
}

Glib::ustring 
LPECopy::getMirrorMap(gint index)
{
    Glib::ustring result = "0000";
    if (index == 1) {
        result = "1000";
    } else if (index == 2) {
        result = "1100";
    } else if (index == 3) {
        result = "0100";
    } else if (index == 4) {
        result = "0011";
    } else if (index == 5) {
        result = "1011";
    } else if (index == 6) {
        result = "1111";
    } else if (index == 7) {
        result = "0111";
    } else if (index == 8) {
        result = "0010";
    } else if (index == 9) {
        result = "1010";
    } else if (index == 10) {
        result = "1110";
    } else if (index == 11) {
        result = "0110";
    } else if (index == 12) {
        result = "0001";
    } else if (index == 13) {
        result = "1001";
    } else if (index == 14) {
        result = "1101";
    } else if (index == 15) {
        result = "0101";
    }
    return result;
}

bool
LPECopy::getActiveMirror(gint index)
{
    Glib::ustring result = getMirrorMap(index);
    return result[0] == Glib::ustring::format(mirrorrowsx)[0] && 
           result[1] == Glib::ustring::format(mirrorrowsy)[0] && 
           result[2] == Glib::ustring::format(mirrorcolsx)[0] && 
           result[3] == Glib::ustring::format(mirrorcolsy)[0];
}

void 
LPECopy::setMirroring(gint index)
{
    if (_updating) {
        return;
    }
    _updating = true;
    Glib::ustring result = getMirrorMap(index);
    gint zero = Glib::ustring("0")[0];
    mirrorrowsx.param_setValue(result[0] == zero ? false : true);
    mirrorrowsy.param_setValue(result[1] == zero ? false : true);
    mirrorcolsx.param_setValue(result[2] == zero ? false : true);
    mirrorcolsy.param_setValue(result[3] == zero ? false : true);
    writeParamsToSVG();
    _updating = false;
}

void
LPECopy::setOffsetCols(){
    offset_type.param_setValue(true);
    offset_type.write_to_SVG();
}
void
LPECopy::setOffsetRows(){
    offset_type.param_setValue(false);
    offset_type.write_to_SVG();
}

void
LPECopy::setRotateInterpolate(bool x, bool y){
    interpolate_rotatex.param_setValue(x);
    interpolate_rotatey.param_setValue(y);
    writeParamsToSVG();
}

void
LPECopy::setScaleInterpolate(bool x, bool y){
    interpolate_scalex.param_setValue(x);
    interpolate_scaley.param_setValue(y);
    writeParamsToSVG();
}

void
LPECopy::doOnApply(SPLPEItem const* lpeitem)
{
    using namespace Geom;
    original_bbox(lpeitem, false, true);
    Glib::ustring display_unit = lpeitem->document->getDisplayUnit()->abbr.c_str();
    gapx_unit = Inkscape::Util::Quantity::convert(gapx, unit.get_abbreviation(), display_unit.c_str());
    gapy_unit = Inkscape::Util::Quantity::convert(gapy, unit.get_abbreviation(), display_unit.c_str());
    originalbbox = Geom::OptRect(boundingbox_X,boundingbox_Y);
    Geom::Point A = Point(boundingbox_X.min() - (gapx_unit / 2.0), boundingbox_Y.min() - (gapy_unit / 2.0));
    Geom::Point B = Point(boundingbox_X.max() + (gapx_unit / 2.0), boundingbox_Y.max() + (gapy_unit / 2.0));
    gap_bbox = Geom::OptRect(A,B);
    if (!gap_bbox) {
        return;
    }
    (*originalbbox) *= Geom::Translate((*originalbbox).midpoint()).inverse() * Geom::Scale(scale/100) * Geom::Translate((*originalbbox).midpoint());
    original_width = (*gap_bbox).width();
    original_height = (*gap_bbox).height();
}

void
LPECopy::doBeforeEffect (SPLPEItem const* lpeitem)
{
    using namespace Geom;
    if (!split_items && lpesatellites.data().size()) {
        processObjects(LPE_ERASE);
    }
    if (link_styles) {
        reset = true;
    }
    if (split_items && !lpesatellites.data().size()) {
        lpesatellites.read_from_SVG();
        if (lpesatellites.data().size()) {
            lpesatellites.update_satellites();
        }
    }
    Glib::ustring display_unit = lpeitem->document->getDisplayUnit()->abbr.c_str();
    gapx_unit = Inkscape::Util::Quantity::convert(gapx, unit.get_abbreviation(), display_unit.c_str());
    gapy_unit = Inkscape::Util::Quantity::convert(gapy, unit.get_abbreviation(), display_unit.c_str());
    original_bbox(lpeitem, false, true);
    originalbbox = Geom::OptRect(boundingbox_X,boundingbox_Y);
    Geom::Point A = Point(boundingbox_X.min() - (gapx_unit / 2.0), boundingbox_Y.min() - (gapy_unit / 2.0));
    Geom::Point B = Point(boundingbox_X.max() + (gapx_unit / 2.0), boundingbox_Y.max() + (gapy_unit / 2.0));
    gap_bbox = Geom::OptRect(A,B);
    if (!gap_bbox) {
        return;
    }
    (*originalbbox) *= Geom::Translate((*originalbbox).midpoint()).inverse() * Geom::Scale(scale/100) * Geom::Translate((*originalbbox).midpoint());
    original_width = (*gap_bbox).width();
    original_height = (*gap_bbox).height();
}


Geom::PathVector
LPECopy::doEffect_path (Geom::PathVector const & path_in)
{    
    Geom::PathVector path_out;
    FillRuleBool fillrule = fill_nonZero;
    if (current_shape->style && 
        current_shape->style->fill_rule.set &&
        current_shape->style->fill_rule.computed == SP_WIND_RULE_EVENODD) 
    {
        fillrule = (FillRuleBool)fill_oddEven;
    }
    path_out = doEffect_path_post(path_in, fillrule);
    if (_knotholder) {
        _knotholder->update_knots();
    }
    return path_out;
}

Geom::PathVector
LPECopy::doEffect_path_post (Geom::PathVector const & path_in, FillRuleBool fillrule)
{
    if (!gap_bbox) {
        return path_in;
    }
    Geom::Point center = (*gap_bbox).midpoint();
    if (split_items) {
        Geom::PathVector output_pv = pathv_to_linear_and_cubic_beziers(path_in);
        output_pv *= Geom::Translate(center).inverse();
        if (!interpolate_rotatex && !interpolate_rotatey) {
            output_pv *= Geom::Rotate::from_degrees(rotate);
        }
        if (!interpolate_scalex && !interpolate_scaley) {
            output_pv *= Geom::Scale(scale/100.0,scale/100.0);
        }
        output_pv *= Geom::Translate(center); 
        return output_pv;
    }
    Geom::PathVector output;
    size_t counter = 0;
    size_t total = num_rows * num_cols;
    Geom::OptRect prev_bbox;
    double gapscalex = 0;
    double maxheight = 0;
    double maxwidth = 0;
    double minheight = std::numeric_limits<double>::max();
    Geom::OptRect bbox = path_in.boundsFast();
    if (!bbox) {
        return path_in;
    }

    double posx = ((*gap_bbox).left() - (*bbox).left()) / (*gap_bbox).width();
    //double posy = ((*gap_bbox).top() - (*bbox).top()) / (*gap_bbox).height() ;
    double factorx = original_width/(*bbox).width();
    double factory = original_height/(*bbox).height();
    double y[(int)num_cols] = {}; 
    double yset = 0;
    double gap[(int)num_cols] = {}; 
    for (int i = 0; i < num_rows; ++i) {
        double fracy = 1;
        if (num_rows != 1) {
            fracy = i/(double)(num_rows - 1);
        }
        for (int j = 0; j < num_cols; ++j) {
            double x = 0;
            double fracx = 1;
            if (num_cols != 1) {
                fracx = j/(double)(num_cols - 1);
            }
            double fract = counter / (double)total;
            Geom::Affine r = Geom::identity();
            if(mirrorrowsx || mirrorrowsy || mirrorcolsx || mirrorcolsy) {
                gint mx = 1;
                gint my = 1;
                if (mirrorrowsx && mirrorcolsx) {
                    mx = (j+i)%2 != 0 ? -1 : 1;
                } else {
                    if (mirrorrowsx) {
                        mx = i%2 != 0 ? -1 : 1;
                    } else if (mirrorcolsx) {
                        mx = j%2 != 0 ? -1 : 1;
                    }
                }
                if (mirrorrowsy && mirrorcolsy) {
                    my = (j+i)%2 != 0 ? -1 : 1;
                } else {                     
                    if (mirrorrowsy) {
                        my = i%2 != 0 ? -1 : 1; 
                    } else if (mirrorcolsy) {
                        my = j%2 != 0 ? -1 : 1;
                    }
                }
                r *= Geom::Translate(center).inverse();
                r *= Geom::Scale(mx, my);
                r *= Geom::Translate(center);
            }
            if (mirrortrans && interpolate_scalex && i%2 != 0) {
                fracx = 1-fracx;
            }
            double fracyin = fracy;
            if (mirrortrans && interpolate_scaley && j%2 != 0) {
                fracyin = 1-fracyin;
            }
            /* if (mirrortrans && interpolate_scaley && interpolate_scalex) {
                fract = 1-fract;
            } */
            double rotatein = rotate;
            if (interpolate_rotatex && interpolate_rotatey) {
                rotatein = rotatein*fract;
            } else if (interpolate_rotatex) {
                rotatein = rotatein*fracx;
            } else if (interpolate_rotatey) {
                rotatein = rotatein*fracyin;
            }
            if (mirrortrans && 
                ((interpolate_rotatex && i%2 != 0) ||
                 (interpolate_rotatey && j%2 != 0) ||
                 (interpolate_rotatex && interpolate_rotatey))) 
            {
                rotatein *=-1;
            }
            double scalein = 1;
            double scalegap = scale/100.0 - scalein;
            if (interpolate_scalex && interpolate_scaley) {
                scalein = (scalegap*fract) + 1;
            } else if (interpolate_scalex) {
                scalein = (scalegap*fracx) + 1;
            } else if (interpolate_scaley) {
                scalein = (scalegap*fracyin) + 1;
            } else {
                scalein = scale/100.0;
            }
            r *= Geom::Translate(center).inverse();
            r *= Geom::Scale(scalein, scalein);
            r *= Geom::Rotate::from_degrees(rotatein);
            r *= Geom::Translate(center);
            Geom::PathVector output_pv = pathv_to_linear_and_cubic_beziers(path_in);
            output_pv *= r;
            double heightrows = original_height * (scale/100.0);
            double widthcols = original_width * (scale/100.0);
            double fixed_heightrows = heightrows;
            double fixed_widthcols = widthcols;

            if (rotatein && shrink_interp) {
                shrink_interp.param_setValue(false);
                shrink_interp.write_to_SVG();
                return path_in;
            }
            if (scale != 100 && (interpolate_scalex || interpolate_scaley )) {//&& !interpolate_rotatex && !interpolate_rotatey) {
                Geom::OptRect bbox = output_pv.boundsFast();
                if (bbox) {
                    maxheight = std::max(maxheight,(*bbox).height());
                    maxwidth = std::max(maxwidth,(*bbox).width());
                    minheight = std::min(minheight,(*bbox).height());
                    widthcols = std::max(original_width * (scale/100.0),original_width);
                    heightrows = std::max(original_height * (scale/100.0),original_height);
                    fixed_widthcols = widthcols;
                    fixed_heightrows = heightrows;
                    double cx = (*bbox).width();
                    double cy = (*bbox).height(); 
                    if (shrink_interp && (!interpolate_scalex || !interpolate_scaley)) {
                        heightrows = 0;
                        widthcols = 0;
                        double px = 0;
                        double py = 0; 
                        if (prev_bbox) {                    
                            px = (*prev_bbox).width();
                            py = (*prev_bbox).height();
                        }
                        if (interpolate_scalex) {
                            if (j) {
                                x = ((cx - ((cx - px) / 2.0))) * factorx;
                                gapscalex += x;
                                x = gapscalex;
                            } else {
                                x = 0;
                                gapscalex = 0;
                            }
                        } else {
                            x = (std::max(original_width * (scale/100.0),original_width) + posx) * j;
                        }
                        if (interpolate_scalex && i == 1) {
                            y[j] = maxheight * factory;
                        }
                        if (i == 1 && !interpolate_scalex) {
                            gap[j] = ((cy * factory) - y[j])/2.0;
                        }
                        yset = y[j] + (gap[j] * i);
                        if (interpolate_scaley) {
                            y[j] += cy * factory;
                        } else {
                            y[j] += maxheight * factory;
                        }
                    }
                    prev_bbox = bbox;
                }
            } else {
                y[j] = 0;
            }
            double xset = x;
            xset += widthcols * j;
            if (heightrows) {
                yset = heightrows * i; 
            }
            double offset_x = 0;
            double offset_y = 0;
            if (offset != 0) {
                if (offset_type && j%2) {
                    offset_y = fixed_heightrows/(100.0/(double)offset);
                }
                if (!offset_type && i%2) {
                    offset_x = fixed_widthcols/(100.0/(double)offset);
                }
            }
            output_pv *= Geom::Translate(Geom::Point(xset + offset_x,yset + offset_y));
            output.insert(output.end(), output_pv.begin(), output_pv.end());
            counter++;
        }
    }
    return output;
}

void
LPECopy::addCanvasIndicators(SPLPEItem const *lpeitem, std::vector<Geom::PathVector> &hp_vec)
{
    if (!gap_bbox) {
        return;
    }
    using namespace Geom;
    hp_vec.clear();
    Geom::Path hp = Geom::Path(*gap_bbox);
    hp *= Geom::Translate((*gap_bbox).midpoint()).inverse() * Geom::Scale(scale/100) * Geom::Translate((*gap_bbox).midpoint());
    Geom::PathVector pathv;
    pathv.push_back(hp);
    hp_vec.push_back(pathv);
}

void
LPECopy::resetDefaults(SPItem const* item)
{
    Effect::resetDefaults(item);
    original_bbox(SP_LPE_ITEM(item), false, true);
}

void
LPECopy::doOnVisibilityToggled(SPLPEItem const* /*lpeitem*/)
{
    processObjects(LPE_VISIBILITY);
}

void 
LPECopy::doOnRemove (SPLPEItem const* lpeitem)
{
    if (keep_paths) {
        processObjects(LPE_TO_OBJECTS);
        return;
    }
    processObjects(LPE_ERASE);
}

void LPECopy::addKnotHolderEntities(KnotHolder *knotholder, SPItem *item)
{
    _knotholder = knotholder;
    KnotHolderEntity *e = new CoS::KnotHolderEntityCopyGapX(this);
    e->create(nullptr, item, knotholder, Inkscape::CANVAS_ITEM_CTRL_TYPE_LPE, "LPE:CopiesGapX",
              _("<b>Gap Y for the copies</b>: drag to gap all copies, <b>Shift+click</b> reset o origin"));
    knotholder->add(e);

    KnotHolderEntity *f = new CoS::KnotHolderEntityCopyGapY(this);
    f->create(nullptr, item, knotholder, Inkscape::CANVAS_ITEM_CTRL_TYPE_LPE, "LPE:CopiesGapY",
              _("<b>Gap Y for the copies</b>: drag to gap all copies, <b>Shift+click</b> reset o origin"));
    knotholder->add(f);
}

namespace CoS {

KnotHolderEntityCopyGapX::~KnotHolderEntityCopyGapX()
{
    LPECopy* lpe = dynamic_cast<LPECopy *>(_effect);
    if (lpe) {
        lpe->_knotholder = nullptr;
    }
}

KnotHolderEntityCopyGapY::~KnotHolderEntityCopyGapY()
{
    LPECopy* lpe = dynamic_cast<LPECopy *>(_effect);
    if (lpe) {
        lpe->_knotholder = nullptr;
    }
}

void KnotHolderEntityCopyGapX::knot_ungrabbed(Geom::Point const &p, Geom::Point const &origin, guint state)
{
    LPECopy* lpe = dynamic_cast<LPECopy *>(_effect);
    startpos = lpe->gapx_unit;
    lpe->gapx.write_to_SVG();
}

void KnotHolderEntityCopyGapY::knot_ungrabbed(Geom::Point const &p, Geom::Point const &origin, guint state)
{
    LPECopy* lpe = dynamic_cast<LPECopy *>(_effect);
    startpos = lpe->gapy_unit;
    lpe->gapy.write_to_SVG();
}

void KnotHolderEntityCopyGapX::knot_click(guint state)
{
    if (!(state & GDK_SHIFT_MASK)) {
        return;
    }

    LPECopy* lpe = dynamic_cast<LPECopy *>(_effect);

    lpe->gapx.param_set_value(0);
    startpos = 0;
    sp_lpe_item_update_patheffect(SP_LPE_ITEM(item), false, false);
}

void KnotHolderEntityCopyGapY::knot_click(guint state)
{
    if (!(state & GDK_SHIFT_MASK)) {
        return;
    }

    LPECopy* lpe = dynamic_cast<LPECopy *>(_effect);

    lpe->gapy.param_set_value(0);
    startpos = 0;
    sp_lpe_item_update_patheffect(SP_LPE_ITEM(item), false, false);
}

void KnotHolderEntityCopyGapX::knot_set(Geom::Point const &p, Geom::Point const&/*origin*/, guint state)
{
    LPECopy* lpe = dynamic_cast<LPECopy *>(_effect);

    Geom::Point const s = snap_knot_position(p, state);
    if (lpe->originalbbox) {
        double value = (((*lpe->originalbbox).corner(1)[Geom::X] - s[Geom::X]) * -1);
        Glib::ustring display_unit = SP_ACTIVE_DOCUMENT->getDisplayUnit()->abbr.c_str();
        value = Inkscape::Util::Quantity::convert((value/(lpe->scale/100.0)) * 2, display_unit.c_str(),lpe->unit.get_abbreviation());
        lpe->gapx.param_set_value(value);
        lpe->refresh_widgets = true;
        lpe->gapx.write_to_SVG();
    }
}

void KnotHolderEntityCopyGapY::knot_set(Geom::Point const &p, Geom::Point const& /*origin*/, guint state)
{
    LPECopy* lpe = dynamic_cast<LPECopy *>(_effect);

    Geom::Point const s = snap_knot_position(p, state);
    if (lpe->originalbbox) {
        double value = (((*lpe->originalbbox).corner(3)[Geom::Y] - s[Geom::Y]) * -1);
        Glib::ustring display_unit = SP_ACTIVE_DOCUMENT->getDisplayUnit()->abbr.c_str();
        value = Inkscape::Util::Quantity::convert((value/(lpe->scale/100.0)) * 2, display_unit.c_str(),lpe->unit.get_abbreviation());
        lpe->gapy.param_set_value(value);
        lpe->refresh_widgets = true;
        lpe->gapy.write_to_SVG();
    }
}

Geom::Point KnotHolderEntityCopyGapX::knot_get() const
{
    LPECopy const * lpe = dynamic_cast<LPECopy const*> (_effect);
    if (lpe->originalbbox) {
        Glib::ustring display_unit = SP_ACTIVE_DOCUMENT->getDisplayUnit()->abbr.c_str();
        double value = Inkscape::Util::Quantity::convert(lpe->gapx, lpe->unit.get_abbreviation(), display_unit.c_str());
        return (*lpe->originalbbox).corner(1) + Geom::Point((value*(lpe->scale/100.0))/2.0,0);
    }
    return Geom::Point(Geom::infinity(),Geom::infinity());
}

Geom::Point KnotHolderEntityCopyGapY::knot_get() const
{
    LPECopy const * lpe = dynamic_cast<LPECopy const*> (_effect);
    if (lpe->originalbbox) {
        Glib::ustring display_unit = SP_ACTIVE_DOCUMENT->getDisplayUnit()->abbr.c_str();
        double value = Inkscape::Util::Quantity::convert(lpe->gapy, lpe->unit.get_abbreviation(), display_unit.c_str());
        return (*lpe->originalbbox).corner(3) + Geom::Point(0,(value*(lpe->scale/100.0))/2.0);
    }
    return Geom::Point(Geom::infinity(),Geom::infinity());
}

} // namespace CoS
} // namespace LivePathEffect
} // namespace Inkscape

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-gaps:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :
