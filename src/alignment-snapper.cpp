// SPDX-License-Identifier: GPL-2.0-or-later
/** \file
 *  Snapping things to on-canvas alignment guides.
 *
 * Authors:
 *   Parth Pant <parthpant4@gmail.com>
 *
 * Copyright (C) 2021 Authors
 *
 * Released under GNU GPL v2+, read the file 'COPYING' for more information.
 */

#include <2geom/circle.h>
#include <2geom/line.h>
#include <2geom/path-intersection.h>
#include <2geom/path-sink.h>

#include "desktop.h"
#include "display/curve.h"
#include "document.h"
#include "inkscape.h"
#include "live_effects/effect-enum.h"
#include "object/sp-clippath.h"
#include "object/sp-flowtext.h"
#include "object/sp-image.h"
#include "object/sp-item-group.h"
#include "object/sp-mask.h"
#include "object/sp-namedview.h"
#include "object/sp-path.h"
#include "object/sp-root.h"
#include "object/sp-shape.h"
#include "object/sp-text.h"
#include "object/sp-use.h"
#include "path/path-util.h" // curve_for_item
#include "preferences.h"
#include "snap-enums.h"
#include "style.h"
#include "svg/svg.h"
#include "text-editing.h"

Inkscape::AlignmentSnapper::AlignmentSnapper(SnapManager *sm, Geom::Coord const d)
    : Snapper(sm, d)
{
    _candidates = new std::vector<SnapCandidateItem>;
    _points_to_snap_to = new std::vector<Inkscape::SnapCandidatePoint>;
}

Inkscape::AlignmentSnapper::~AlignmentSnapper()
{
    _candidates->clear();
    delete _candidates;

    _points_to_snap_to->clear();
    delete _points_to_snap_to;
}

void Inkscape::AlignmentSnapper::_findCandidates(SPObject* parent,
                      std::vector<SPItem const*> const *it,
                      bool const &first_point,
                      bool const clip_or_mask,
                      Geom::Affine const additional_affine) const
{
    SPDesktop const *dt = _snapmanager->getDesktop();
    if (dt == nullptr) {
        g_warning("desktop == NULL, so we cannot snap; please inform the developers of this bug");
        // Apparently the setup() method from the SnapManager class hasn't been called before trying to snap.
    }

    if (first_point) {
        _candidates->clear();
    }

    for (auto& o: parent->children) {
        g_assert(dt != nullptr);
        SPItem *item = dynamic_cast<SPItem *>(&o);
        if (item && !(dt->itemIsHidden(item) && !clip_or_mask)) {
            // Fix LPE boolops selfsnaping
            bool stop = false;
            if (item->style) {
                SPFilter *filt = item->style->getFilter();
                if (filt && filt->getId() && strcmp(filt->getId(), "selectable_hidder_filter") == 0) {
                    stop = true;
                }
                SPLPEItem *lpeitem = dynamic_cast<SPLPEItem *>(item);
                if (lpeitem && lpeitem->hasPathEffectOfType(Inkscape::LivePathEffect::EffectType::BOOL_OP)) {
                    stop = true;
                }
            }
            if (stop) {
                stop = false;
                for (auto skipitem : *it) {
                    if (skipitem && skipitem->style) {
                        SPItem *toskip = const_cast<SPItem *>(skipitem);
                        if (toskip) {
                            SPFilter *filt = toskip->style->getFilter();
                            if (filt && filt->getId() && strcmp(filt->getId(), "selectable_hidder_filter") == 0) {
                                stop = true;
                                break;
                            }

                            SPLPEItem *lpeitem = dynamic_cast<SPLPEItem *>(toskip);
                            if (!stop && lpeitem &&
                                lpeitem->hasPathEffectOfType(Inkscape::LivePathEffect::EffectType::BOOL_OP)) {
                                stop = true;
                                break;
                            }
                        }
                    }
                }
                if (stop) {
                    continue;
                }
            }
            // Snapping to items in a locked layer is allowed
            // Don't snap to hidden objects, unless they're a clipped path or a mask
            /* See if this item is on the ignore list */
            std::vector<SPItem const *>::const_iterator i;
            if (it != nullptr) {
                i = it->begin();
                while (i != it->end() && *i != &o) {
                    ++i;
                }
            }

            if (it == nullptr || i == it->end()) {
                if (item) {
                    if (!clip_or_mask) { // cannot clip or mask more than once
                        // The current item is not a clipping path or a mask, but might
                        // still be the subject of clipping or masking itself ; if so, then
                        // we should also consider that path or mask for snapping to
                        SPObject *obj = item->getClipObject();
                        if (obj && _snapmanager->snapprefs.isTargetSnappable(SNAPTARGET_PATH_CLIP)) {
                            _findCandidates(obj, it, false, true, item->i2doc_affine());
                        }
                        obj = item->getMaskObject();
                        if (obj && _snapmanager->snapprefs.isTargetSnappable(SNAPTARGET_PATH_MASK)) {
                            _findCandidates(obj, it, false, true, item->i2doc_affine());
                        }
                    }

                    if (dynamic_cast<SPGroup *>(item)) {
                        _findCandidates(&o, it, false, clip_or_mask, additional_affine);
                    } else {
                        Geom::OptRect bbox_of_item;
                        Preferences *prefs = Preferences::get();
                        int prefs_bbox = prefs->getBool("/tools/bounding_box", false);
                        // We'll only need to obtain the visual bounding box if the user preferences tell
                        // us to, AND if we are snapping to the bounding box itself. If we're snapping to
                        // paths only, then we can just as well use the geometric bounding box (which is faster)
                        SPItem::BBoxType bbox_type = (!prefs_bbox && _snapmanager->snapprefs.isTargetSnappable(SNAPTARGET_BBOX_CATEGORY)) ?
                            SPItem::VISUAL_BBOX : SPItem::GEOMETRIC_BBOX;
                        if (clip_or_mask) {
                            // Oh oh, this will get ugly. We cannot use sp_item_i2d_affine directly because we need to
                            // insert an additional transformation in document coordinates (code copied from sp_item_i2d_affine)
                            bbox_of_item = item->bounds(bbox_type, item->i2doc_affine() * additional_affine * dt->doc2dt());
                        } else {
                            bbox_of_item = item->desktopBounds(bbox_type);
                        }
                        if (bbox_of_item) {
                            if (_snapmanager->getDesktop()->get_display_area().contains(bbox_of_item->midpoint()))
                            // Finally add the object to _candidates.
                            _candidates->push_back(SnapCandidateItem(item, clip_or_mask, additional_affine));
                            // For debugging: print the id of the candidate to the console
                            // SPObject *obj = (SPObject*)item;
                            // std::cout << "Snap candidate added: " << obj->getId() << std::endl;
                            if (_candidates->size() > 200) { // This makes Inkscape crawl already
                                static Glib::Timer timer;
                                if (timer.elapsed() > 1.0) {
                                    timer.reset();
                                    std::cout << "Warning: limit of 200 snap target paths reached, some will be ignored" << std::endl;
                                }
                                break;
                            }
                        }
                    }
                }
            }
        }
    }
}


void Inkscape::AlignmentSnapper::_collectBBoxPoints(bool const &first_point) const
{
    if (!first_point)
        return;

    _points_to_snap_to->clear();
    SPItem::BBoxType bbox_type = SPItem::GEOMETRIC_BBOX;

    Preferences *prefs = Preferences::get();
    bool prefs_bbox = prefs->getBool("/tools/bounding_box");
    bbox_type = !prefs_bbox ?
        SPItem::VISUAL_BBOX : SPItem::GEOMETRIC_BBOX;

    // collect page corners and center
    if (_snapmanager->snapprefs.isTargetSnappable(SNAPTARGET_PAGE_CORNER)) {
        Geom::Coord w = (_snapmanager->getDocument())->getWidth().value("px");
        Geom::Coord h = (_snapmanager->getDocument())->getHeight().value("px");
        _points_to_snap_to->push_back(SnapCandidatePoint(Geom::Point(0,0), SNAPSOURCE_ALIGNMENT_PAGE_CORNER, SNAPTARGET_ALIGNMENT_PAGE_CORNER));
        _points_to_snap_to->push_back(SnapCandidatePoint(Geom::Point(0,h), SNAPSOURCE_ALIGNMENT_PAGE_CORNER, SNAPTARGET_ALIGNMENT_PAGE_CORNER));
        _points_to_snap_to->push_back(SnapCandidatePoint(Geom::Point(w,h), SNAPSOURCE_ALIGNMENT_PAGE_CORNER, SNAPTARGET_ALIGNMENT_PAGE_CORNER));
        _points_to_snap_to->push_back(SnapCandidatePoint(Geom::Point(w,0), SNAPSOURCE_ALIGNMENT_PAGE_CORNER, SNAPTARGET_ALIGNMENT_PAGE_CORNER));
        _points_to_snap_to->push_back(SnapCandidatePoint(Geom::Point(w/2.0f,h/2.0f), SNAPSOURCE_ALIGNMENT_PAGE_CENTER, SNAPTARGET_ALIGNMENT_PAGE_CENTER));
    }

    // collect bounding boxes of other objects
    for (const auto & candidate : *_candidates) {
        SPItem *root_item = candidate.item; 

        // get the root item in case we have a duplicate at hand
        SPUse *use = dynamic_cast<SPUse *>(candidate.item);
        if (use) {
            root_item = use->root();
        }
        g_return_if_fail(root_item);

        // if candidate is not a clip or a mask object then extract its BBox points
        if (!candidate.clip_or_mask) {
            Geom::OptRect b = root_item->desktopBounds(bbox_type);
            getBBoxPoints(b, _points_to_snap_to, true, true, false, true, true);
        }
    }

    // Debug log
    //std::cout<<"----------"<<std::endl;
    //for (auto point : *_points_to_snap_to)
        //std::cout<<point.getPoint().x()<<","<<point.getPoint().y()<<std::endl;
}

void Inkscape::AlignmentSnapper::_snapBBoxPoints(IntermSnapResults &isr,
                                                 SnapCandidatePoint const &p,
                                                 std::vector<SnapCandidatePoint> *unselected_nodes,
                                                 SnapConstraint const &c,
                                                 Geom::Point const &p_proj_on_constraint) const
{

    _collectBBoxPoints(p.getSourceNum() <= 0);

    if (unselected_nodes != nullptr &&
        unselected_nodes->size() > 0 &&
        _snapmanager->snapprefs.isTargetSnappable(Inkscape::SNAPTARGET_ALIGNMENT_HANDLE)) {
        g_assert(_points_to_snap_to != nullptr);
        _points_to_snap_to->insert(_points_to_snap_to->end(), unselected_nodes->begin(), unselected_nodes->end());
    }

    SnappedPoint sx;
    SnappedPoint sy;
    SnappedPoint si;

    bool consider_x = true;
    bool consider_y = true;
    bool success = false;
    bool intersection = false;
    //bool strict_snapping = _snapmanager->snapprefs.getStrictSnapping();

    for (const auto & k : *_points_to_snap_to) {
        // TODO:  add strict snpping checks from ObjectSnapper::_allowSourceToSnapToTarget(...)
        if (true) {
            Geom::Point target_pt = k.getPoint();
            // (unconstrained) distace from HORIZONTAL guide 
            Geom::Point point_on_x(p.getPoint().x(), target_pt.y());
            Geom::Coord distX = Geom::L2(point_on_x - p.getPoint()); 

            // (unconstrained) distace from VERTICAL guide 
            Geom::Point point_on_y(target_pt.x(), p.getPoint().y());
            Geom::Coord distY = Geom::L2(point_on_y - p.getPoint()); 

            if (!c.isUndefined() && c.isLinear()) {
                if (c.getDirection().x() == 0)
                    consider_y = false; // consider vertical snapping if moving vertically
                else
                    consider_x = false; // consider horizontal snapping if moving horizontally 
            }

            bool is_target_node = k.getTargetType() & SNAPTARGET_NODE_CATEGORY;
            if (consider_x && distX < getSnapperTolerance()) {
                sx = SnappedPoint(point_on_x,
                                 k.getPoint(),
                                 source2alignment(p.getSourceType()),
                                 p.getSourceNum(),
                                 is_target_node ? SNAPTARGET_ALIGNMENT_HANDLE : k.getTargetType(),
                                 distX,
                                 getSnapperTolerance(),
                                 getSnapperAlwaysSnap(),
                                 false,
                                 true,
                                 k.getTargetBBox());
                success = true;
            }

            if (consider_y && distY < getSnapperTolerance()) {
                sy = SnappedPoint(point_on_y,
                                 k.getPoint(),
                                 source2alignment(p.getSourceType()),
                                 p.getSourceNum(),
                                 is_target_node ? SNAPTARGET_ALIGNMENT_HANDLE : k.getTargetType(),
                                 distY,
                                 getSnapperTolerance(),
                                 getSnapperAlwaysSnap(),
                                 false,
                                 true,
                                 k.getTargetBBox());
                success = true;
            }

            if (consider_x && consider_y) {
                Geom::Point intersection_p = Geom::Point(sy.getPoint().x(), sx.getPoint().y());
                Geom::Coord d =  Geom::L2(intersection_p - p.getPoint());

                if (d < sqrt(2)*getSnapperTolerance()) {
                    si = SnappedPoint(intersection_p,
                                     sy.getAlignmentTarget(),
                                     sx.getAlignmentTarget(),
                                     source2alignment(p.getSourceType()),
                                     p.getSourceNum(),
                                     SNAPTARGET_ALIGNMENT_INTERSECTION,
                                     d,
                                     getSnapperTolerance(),
                                     getSnapperAlwaysSnap(),
                                     false,
                                     true,
                                     k.getTargetBBox());
                    intersection = true;
                }
            }
        }
    }

    if (intersection) {
       isr.points.push_back(si); 
       return;
    }

    if (success) {
        if (sx.getSnapDistance() < sy.getSnapDistance()) {
            isr.points.push_back(sx);
        } else {
            isr.points.push_back(sy);
        }
    }

}

void Inkscape::AlignmentSnapper::freeSnap(IntermSnapResults &isr,
                                          Inkscape::SnapCandidatePoint const &p,
                                          Geom::OptRect const &bbox_to_snap,
                                          std::vector<SPItem const *> const *it,
                                          std::vector<SnapCandidatePoint> *unselected_nodes) const
{
    bool p_is_bbox = p.getSourceType() & SNAPSOURCE_BBOX_CATEGORY;
    bool p_is_node = p.getSourceType() & SNAPSOURCE_NODE_HANDLE;
    
    // toggle checks 
    if (!_snap_enabled || !_snapmanager->snapprefs.isTargetSnappable(SNAPTARGET_ALIGNMENT_CATEGORY))
        return;

    unsigned n = (unselected_nodes == nullptr) ? 0 : unselected_nodes->size();

    // n > 0 : node tool is active
    if (!(p_is_bbox || (n > 0 && p_is_node) || (p.considerForAlignment() && p_is_node)))
        return;

    if (p.getSourceNum() <= 0){
        _candidates->clear();
        _findCandidates(_snapmanager->getDocument()->getRoot(), it, true, false);
    }

    _snapBBoxPoints(isr, p, unselected_nodes);
}

void Inkscape::AlignmentSnapper::constrainedSnap(IntermSnapResults &isr,
              Inkscape::SnapCandidatePoint const &p,
              Geom::OptRect const &bbox_to_snap,
              SnapConstraint const &c,
              std::vector<SPItem const *> const *it,
              std::vector<SnapCandidatePoint> *unselected_nodes) const
{
    bool p_is_bbox = p.getSourceType() & SNAPSOURCE_BBOX_CATEGORY;
    bool p_is_node = p.getSourceType() & SNAPSOURCE_NODE_HANDLE;
    
    // project the mouse pointer onto the constraint. Only the projected point will be considered for snapping
    Geom::Point pp = c.projection(p.getPoint());
    
    // toggle checks 
    if (!_snap_enabled || !_snapmanager->snapprefs.isTargetSnappable(SNAPTARGET_ALIGNMENT_CATEGORY))
        return;

    unsigned n = (unselected_nodes == nullptr) ? 0 : unselected_nodes->size();

    // n > 0 : node tool is active
    if (!(p_is_bbox || (n > 0 && p_is_node) || (p.considerForAlignment() && p_is_node)))
        return;

    if (p.getSourceNum() <= 0){
        _candidates->clear();
        _findCandidates(_snapmanager->getDocument()->getRoot(), it, true, false);
    }

    _snapBBoxPoints(isr, p, unselected_nodes, c, pp);
}

bool Inkscape::AlignmentSnapper::ThisSnapperMightSnap() const
{
    return true;
}

bool Inkscape::AlignmentSnapper::getSnapperAlwaysSnap() const
{
    return _snapmanager->snapprefs.getAlignmentTolerance() == 10000; //TODO: Replace this threshold of 10000 by a constant; see also tolerance-slider.cpp
}

Geom::Coord Inkscape::AlignmentSnapper::getSnapperTolerance() const
{
    SPDesktop const *dt = _snapmanager->getDesktop();
    double const zoom =  dt ? dt->current_zoom() : 1;
    return _snapmanager->snapprefs.getAlignmentTolerance() / zoom;
}

Inkscape::SnapSourceType Inkscape::AlignmentSnapper::source2alignment(SnapSourceType s) const
{
    switch (s) {
        case SNAPSOURCE_BBOX_CATEGORY:
            return SNAPSOURCE_ALIGNMENT_CATEGORY;
        case SNAPSOURCE_BBOX_CORNER:
            return SNAPSOURCE_ALIGNMENT_BBOX_CORNER;
        case SNAPSOURCE_BBOX_MIDPOINT:
            return SNAPSOURCE_ALIGNMENT_BBOX_MIDPOINT;
        case SNAPSOURCE_BBOX_EDGE_MIDPOINT:
            return SNAPSOURCE_ALIGNMENT_BBOX_EDGE_MIDPOINT;
        case SNAPSOURCE_NODE_CATEGORY:
        case SNAPSOURCE_OTHER_HANDLE:
            return SNAPSOURCE_ALIGNMENT_HANDLE;
        default:
            return SNAPSOURCE_UNDEFINED;
    }
}



