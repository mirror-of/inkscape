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
#include <memory>

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
    _points_to_snap_to = std::make_unique<std::vector<Inkscape::SnapCandidatePoint>>();
}

Inkscape::AlignmentSnapper::~AlignmentSnapper()
{
    _points_to_snap_to->clear();
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
    for (const auto & candidate : *(_snapmanager->align_snapper_candidates)) {
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
            getBBoxPoints(b, _points_to_snap_to.get(), true, true, false, true, true);
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
    bool success_x = false;
    bool success_y = false;
    bool intersection = false;
    //bool strict_snapping = _snapmanager->snapprefs.getStrictSnapping();

    for (const auto & k : *_points_to_snap_to) {
        // TODO:  add strict snapping checks from ObjectSnapper::_allowSourceToSnapToTarget(...)
        if (true) {
            Geom::Point target_pt = k.getPoint();
            // (unconstrained) distance from HORIZONTAL guide 
            Geom::Point point_on_x(p.getPoint().x(), target_pt.y());
            Geom::Coord distX = Geom::L2(point_on_x - p.getPoint()); 

            // (unconstrained) distance from VERTICAL guide 
            Geom::Point point_on_y(target_pt.x(), p.getPoint().y());
            Geom::Coord distY = Geom::L2(point_on_y - p.getPoint()); 

            if (!c.isUndefined() && c.isLinear()) {
                if (c.getDirection().x() == 0)
                    consider_y = false; // consider vertical snapping if moving vertically
                else
                    consider_x = false; // consider horizontal snapping if moving horizontally 
            }

            bool is_target_node = k.getTargetType() & SNAPTARGET_NODE_CATEGORY;
            if (consider_x && distX < getSnapperTolerance() && Geom::L2(target_pt - point_on_x) < sx.getDistanceToAlignTarget()) {
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
                success_x = true;
            }

            if (consider_y && distY < getSnapperTolerance() && Geom::L2(target_pt - point_on_y) < sy.getDistanceToAlignTarget()) {
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
                success_y = true;
            }

            if (consider_x && consider_y && success_x && success_y) {
                Geom::Point intersection_p = Geom::Point(sy.getPoint().x(), sx.getPoint().y());
                Geom::Coord d =  Geom::L2(intersection_p - p.getPoint());

                if (d < sqrt(2)*getSnapperTolerance()) {
                    si = SnappedPoint(intersection_p,
                                     *sx.getAlignmentTarget(),
                                     *sy.getAlignmentTarget(),
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

    if (success_x || success_y) {
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



