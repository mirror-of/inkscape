// SPDX-License-Identifier: GPL-2.0-or-later
/** \file
 *  Snapping equidistant objects
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
#include "style.h"
#include "svg/svg.h"

#define DISTRIBUTION_SNAPPING_EPSILON 0.5e-4f

static bool compare_double(double x, double y, double epsilon = DISTRIBUTION_SNAPPING_EPSILON)
{
    if (abs(x - y) < epsilon)
        return true;
    return false;
}

static int sortBoxesRight(Geom::Rect const &a, Geom::Rect const &b)
{
    if (a.midpoint().x() < b.midpoint().x())
        return 1;
    return 0;
}

static int sortBoxesLeft(Geom::Rect const &a, Geom::Rect const &b)
{
    if (a.midpoint().x() > b.midpoint().x())
        return 1;
    return 0;
}

static int sortBoxesUp(Geom::Rect const &a, Geom::Rect const &b)
{
    if (a.midpoint().y() > b.midpoint().y())
        return 1;
    return 0;
}

static int sortBoxesDown(Geom::Rect const &a, Geom::Rect const &b)
{
    if (a.midpoint().y() < b.midpoint().y())
        return 1;
    return 0;
}

Geom::Coord Inkscape::DistributionSnapper::distRight(Geom::Rect const &a, Geom::Rect const &b)
{
    return -a.max().x() + b.min().x();
}

Geom::Coord Inkscape::DistributionSnapper::distLeft(Geom::Rect const &a, Geom::Rect const &b)
{
    return a.min().x() - b.max().x();
}

Geom::Coord Inkscape::DistributionSnapper::distUp(Geom::Rect const &a, Geom::Rect const &b)
{
    return a.min().y() - b.max().y();
}

Geom::Coord Inkscape::DistributionSnapper::distDown(Geom::Rect const &a, Geom::Rect const &b)
{
    return -a.max().y() + b.min().y();
}

bool Inkscape::DistributionSnapper::_findSidewaysSnaps(
                                    Geom::Rect const &source_bbox,
                                    std::vector<Geom::Rect>::iterator it,
                                    std::vector<Geom::Rect>::iterator end,
                                    std::vector<Geom::Rect> &vec,
                                    Geom::Coord &dist,
                                    Geom::Coord tol,
                                    std::function<Geom::Coord(Geom::Rect const &, Geom::Rect const &)> const &distance_func,
                                    int level) const
{
    std::vector<Geom::Rect>::iterator next_bbox = it;
    std::vector<Geom::Rect>::iterator _next_bbox = it;

    if (level == 0) {
        int max_length = 0;

        // check each consecutive box for a snap
        while (std::next(next_bbox) != end) {
            auto first_dist = distance_func(source_bbox, *next_bbox);
            level = 0;

            // temporary result for this particular item
            auto result = new std::vector<Geom::Rect>;
            if (_findSidewaysSnaps(*next_bbox, ++it, end, *result, first_dist, tol, distance_func, ++level)) {
                if (result->size() > max_length) {
                    // if this item has the most number of items equidistant form each other
                    // then make this the final result
                    max_length = result->size();
                    vec = *result;
                    dist = first_dist;
                } else {
                    // delete the result
                    result->clear();
                    delete result;
                }
            } else {
                // delete the result
                result->clear();
                delete result;
            }

            ++next_bbox;
        }

        // if there is no snap, just add the first item and return false
        // this is useful to find in-between snaps (see _snapEquidistantPoints())
        if (max_length == 0) {
            vec.push_back(*_next_bbox);
            return false;
        }
        return true;
    }

    // if not the zeroth level
    if (level != 1)
        vec.push_back(source_bbox); 

    if (it == end)
        return true;

    while (next_bbox != end) {
        Geom::Coord next_dist = distance_func(source_bbox, *next_bbox);

        if (level == 1 && compare_double(dist, next_dist, tol)){
            // if this is the first level, check if the snap is within tolerance
            // we cancel here if the possible snap in not whithing tolerance, saves us some time!
            dist = next_dist;
            vec.push_back(source_bbox);
            return _findSidewaysSnaps(*next_bbox, ++it, end, vec, dist, tol, distance_func, ++level);
        } else if (compare_double(dist, next_dist)) {
            return _findSidewaysSnaps(*next_bbox, ++it, end, vec, dist, tol, distance_func, ++level);
        }

        ++next_bbox;
    }

    // once reach the end, return false if level == 1, as there is just one(or more in case of overlap) item to that side.
    return level != 1;
}

Inkscape::DistributionSnapper::DistributionSnapper(SnapManager *sm, Geom::Coord const d)
    : Snapper(sm, d)
{
    _bboxes_right = new std::vector<Geom::Rect>;
    _bboxes_left = new std::vector<Geom::Rect>;
    _bboxes_up = new std::vector<Geom::Rect>;
    _bboxes_down = new std::vector<Geom::Rect>;
}

Inkscape::DistributionSnapper::~DistributionSnapper()
{
    _bboxes_right->clear();
    delete _bboxes_right;

    _bboxes_left->clear();
    delete _bboxes_left;

    _bboxes_up->clear();
    delete _bboxes_up;

    _bboxes_down->clear();
    delete _bboxes_down;
}

void Inkscape::DistributionSnapper::_collectBBoxes(Geom::OptRect const &bbox_to_snap, bool const &first_point) const
{
    if (!first_point)
        return;

    _bboxes_right->clear();
    _bboxes_left->clear();
    _bboxes_down->clear();
    _bboxes_up->clear();

    SPItem::BBoxType bbox_type = SPItem::GEOMETRIC_BBOX;

    Preferences *prefs = Preferences::get();
    bool prefs_bbox = prefs->getBool("/tools/bounding_box");
    bbox_type = !prefs_bbox ? SPItem::VISUAL_BBOX : SPItem::GEOMETRIC_BBOX;

    // collect bounding boxes of other objects
    for (const auto &candidate : *(_snapmanager->align_snapper_candidates)) {
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
            if (!b.intersects(bbox_to_snap)) {
                auto diff_vec = b->midpoint() - bbox_to_snap->midpoint();

                Geom::Rect Xbounds = *bbox_to_snap;
                Xbounds.expandBy(_snapmanager->_desktop->get_display_area().maxExtent(), 0);

                Geom::Rect Ybounds = *bbox_to_snap;
                Ybounds.expandBy(0, _snapmanager->_desktop->get_display_area().maxExtent());

                if (Xbounds.intersects(b)) {
                    if (diff_vec.x() > 0) {
                        _bboxes_right->push_back(*b);
                    } else {
                        _bboxes_left->push_back(*b);
                    }
                } else if (Ybounds.intersects(b)) {
                    if (diff_vec.y() < 0) {
                        _bboxes_up->push_back(*b);
                    } else {
                        _bboxes_down->push_back(*b);
                    }
                }
            }
        }
    }

    std::stable_sort(_bboxes_right->begin(), _bboxes_right->end(), sortBoxesRight);
    std::stable_sort(_bboxes_left->begin(), _bboxes_left->end(), sortBoxesLeft);
    std::stable_sort(_bboxes_up->begin(), _bboxes_up->end(), sortBoxesUp);
    std::stable_sort(_bboxes_down->begin(), _bboxes_down->end(), sortBoxesDown);

    _addBBoxForIntersectingBoxes();
}

void Inkscape::DistributionSnapper::_addBBoxForIntersectingBoxes() const {
    if (_bboxes_right->size() > 0) {
        for (auto it = _bboxes_right->begin(); std::next(it) != _bboxes_right->end(); it++) {
            Geom::Rect comb(*it);
            int num = 0;
            auto start = it;
            auto insertPos = it;

            while (std::next(it) != _bboxes_right->end() && it->intersects(*std::next(it))) {
                comb.unionWith(*std::next(it));
                if (comb.midpoint().x() > it->midpoint().x()) {
                    insertPos = std::next(it);
                } else {
                    ++num;
                }
                ++it;
            }

            if (it != start) {
                it = _bboxes_right->insert(insertPos, comb);
            }

            it += num;
        }
    }

    if (_bboxes_left->size() > 0) {
        for (auto it = _bboxes_left->begin(); std::next(it) != _bboxes_left->end(); it++) {
            Geom::Rect comb(*it);
            int num = 0;
            auto start = it;
            auto insertPos = it;

            while (std::next(it) != _bboxes_left->end() && it->intersects(*std::next(it))) {
                comb.unionWith(*std::next(it));
                if (comb.midpoint().x() < it->midpoint().x()) {
                    insertPos = std::next(it);
                } else {
                    ++num;
                }
                ++it;
            }

            if (it != start) {
                it = _bboxes_left->insert(insertPos, comb);
            }

            it += num;
        }
    }

    if (_bboxes_up->size() > 0) {
        for (auto it = _bboxes_up->begin(); std::next(it) != _bboxes_up->end(); it++) {
            Geom::Rect comb(*it);
            int num = 0;
            auto start = it;
            auto insertPos = it;

            while (std::next(it) != _bboxes_up->end() && it->intersects(*std::next(it))) {
                comb.unionWith(*std::next(it));
                if (comb.midpoint().y() > it->midpoint().y()) {
                    insertPos = std::next(it);
                } else {
                    ++num;
                }
                ++it;
            }

            if (it != start) {
                it = _bboxes_up->insert(insertPos, comb);
            }

            it += num;
        }
    }

    if (_bboxes_down->size() > 0) {
        for (auto it = _bboxes_down->begin(); std::next(it) != _bboxes_down->end(); it++) {
            Geom::Rect comb(*it);
            int num = 0;
            auto start = it;
            auto insertPos = it;

            while (std::next(it) != _bboxes_down->end() && it->intersects(*std::next(it))) {
                comb.unionWith(*std::next(it));
                if (comb.midpoint().y() < it->midpoint().y()) {
                    insertPos = std::next(it);
                } else {
                    ++num;
                }
                ++it;
            }

            if (it != start) {
                it = _bboxes_down->insert(insertPos, comb);
            }

            it += num;
        }
    }
}

void Inkscape::DistributionSnapper::_snapEquidistantPoints(IntermSnapResults &isr,
                                                           SnapCandidatePoint const &p,
                                                           Geom::OptRect const &bbox_to_snap,
                                                           std::vector<SnapCandidatePoint> *unselected_nodes,
                                                           SnapConstraint const &c,
                                                           Geom::Point const &p_proj_on_constraint) const
{
    bool consider_x = true;
    bool consider_y = true;
    if (!c.isUndefined() && c.isLinear()) {
        if (c.getDirection().x() == 0)
            consider_x = false; // consider horizontal snapping if moving vertically
        else
            consider_y = false; // consider vertical snapping if moving horizontally
    }

    _collectBBoxes(bbox_to_snap, p.getSourceNum() <= 0);

    Geom::Coord offset;

    if (p.getSourceType() != SNAPSOURCE_BBOX_MIDPOINT)
        return;

    Geom::Coord equal_dist;

    SnappedPoint sr, sl, sx, su, sd, sy;
    Geom::Coord dist_x, dist_y;
    bool snap_x = false, snap_y = false;

    // 1. look right
    // if there is a snap then add right bboxes and look left, if there is a snap to the left then
    // add those bboxes too
    std::vector<Geom::Rect> vecRight;
    std::vector<Geom::Rect> vecLeft;
    if (consider_x && _bboxes_right->size() > 0) {
        if (_findSidewaysSnaps(*bbox_to_snap, _bboxes_right->begin(), _bboxes_right->end(), vecRight, equal_dist, getSnapperTolerance(), &DistributionSnapper::distRight)) {
            auto first_dist = distRight(*bbox_to_snap, vecRight.front());
            Geom::Coord offset = first_dist - equal_dist;
            Geom::Point target = bbox_to_snap->midpoint() + Geom::Point(offset, 0);

            Geom::Affine translation = Geom::Translate(target - bbox_to_snap->midpoint());
            Geom::Rect bbox = *bbox_to_snap * translation;
            vecRight.insert(vecRight.begin(), bbox);

            _correctSelectionBBox(target, p.getPoint(), *bbox_to_snap);

            if (_bboxes_left->size() > 0) {
                first_dist = distLeft(bbox, _bboxes_left->front());
                Geom::Coord left_dist;
                vecLeft.clear();
                if (_findSidewaysSnaps(*bbox_to_snap, _bboxes_left->begin(), _bboxes_left->end(), vecLeft, left_dist, getSnapperTolerance(), &DistributionSnapper::distLeft)) {
                    if (compare_double(left_dist, equal_dist)) {
                        std::reverse(vecLeft.begin(), vecLeft.end());
                        vecRight.insert(vecRight.begin(), vecLeft.begin(), vecLeft.end());
                    }

                } else if (compare_double(first_dist, equal_dist)) {
                    vecRight.insert(vecRight.begin(), vecLeft.front());
                }
            }

            dist_x = abs(offset);
            sx = SnappedPoint(target, vecRight, bbox, equal_dist, p.getSourceType(), p.getSourceNum(), SNAPTARGET_DISTRIBUTION_RIGHT, dist_x, getSnapperTolerance(), getSnapperAlwaysSnap(), false, true);
            snap_x = true;
        }
    }

    // 2. if no snap to right, look left
    // if there is a snap then add left bboxes and right left, if there is a snap to the right then
    // add those bboxes too
    if (consider_x && !snap_x && _bboxes_left->size() > 0) {
        vecLeft.clear();
        if (_findSidewaysSnaps(*bbox_to_snap, _bboxes_left->begin(), _bboxes_left->end(), vecLeft, equal_dist, getSnapperTolerance(), &DistributionSnapper::distLeft)) {
            auto first_dist = distLeft(*bbox_to_snap, vecLeft.front());
            Geom::Coord offset = first_dist - equal_dist;
            Geom::Point target = bbox_to_snap->midpoint() - Geom::Point(offset, 0);

            // translate the source bbox to the snap position
            Geom::Affine translation = Geom::Translate(target - bbox_to_snap->midpoint());
            Geom::Rect bbox = *bbox_to_snap * translation;
            std::reverse(vecLeft.begin(), vecLeft.end());
            vecLeft.push_back(bbox);

            _correctSelectionBBox(target, p.getPoint(), *bbox_to_snap);

            if (_bboxes_right->size() > 0) {
                first_dist = distRight(bbox, _bboxes_right->front());
                Geom::Coord right_dist;
                vecRight.clear();
                if (_findSidewaysSnaps(*bbox_to_snap, _bboxes_right->begin(), _bboxes_right->end(), vecRight, right_dist, getSnapperTolerance(), &DistributionSnapper::distRight)) {
                    if (compare_double(right_dist, equal_dist)) {
                        vecLeft.insert(vecLeft.end(), vecRight.begin(), vecRight.end());
                    }

                } else if (compare_double(first_dist, equal_dist)) {
                    vecLeft.push_back(vecRight.front());
                }
            }

            dist_x = abs(offset);
            sx = SnappedPoint(target, vecLeft, bbox, equal_dist, p.getSourceType(), p.getSourceNum(), SNAPTARGET_DISTRIBUTION_LEFT, dist_x, getSnapperTolerance(), getSnapperAlwaysSnap(), false, true);
            snap_x = true;
        }
    }

    // 3. if no snap to right or left just add the center snap
    if (consider_x && !snap_x && vecRight.size() > 0 && vecLeft.size() > 0) {
        auto x = Geom::Point((vecRight.front().min() + vecLeft.front().max()) / 2).x();
        offset = abs(x - bbox_to_snap->midpoint().x());
        if (offset < getSnapperTolerance()) {
            Geom::Point target = Geom::Point(x, bbox_to_snap->midpoint().y());
            // translate the source bbox to the snap position
            Geom::Affine translation = Geom::Translate(target - bbox_to_snap->midpoint());
            Geom::Rect bbox = *bbox_to_snap * translation;
            std::vector<Geom::Rect> bboxes = {vecLeft.front(), bbox, vecRight.front()};

            _correctSelectionBBox(target, p.getPoint(), *bbox_to_snap);

            equal_dist = bbox.min().x() - vecLeft.front().max().x();
            sx = SnappedPoint(target, bboxes, bbox, equal_dist, p.getSourceType(), p.getSourceNum(), SNAPTARGET_DISTRIBUTION_X, offset, getSnapperTolerance(), getSnapperAlwaysSnap(), false, true);
            snap_x = true;
        }
    }

    // 1. look Up
    // if there is a snap then add top bboxes and look down, if there is a snap at the bottom then
    // add those bboxes too
    std::vector<Geom::Rect> vecUp;
    std::vector<Geom::Rect> vecDown;
    if (consider_y && _bboxes_up->size() > 0) {
        if (_findSidewaysSnaps(*bbox_to_snap, _bboxes_up->begin(), _bboxes_up->end(), vecUp, equal_dist, getSnapperTolerance(), &DistributionSnapper::distUp)) {
            auto first_dist = distUp(*bbox_to_snap, vecUp.front());
            Geom::Coord offset = first_dist - equal_dist;
            Geom::Point target = bbox_to_snap->midpoint() - Geom::Point(0, offset);

            // translate the source bbox to the snap position
            Geom::Affine translation = Geom::Translate(target - bbox_to_snap->midpoint());
            Geom::Rect bbox = *bbox_to_snap * translation;
            std::reverse(vecUp.begin(), vecUp.end());
            vecUp.push_back(bbox);

            _correctSelectionBBox(target, p.getPoint(), *bbox_to_snap);

            if (_bboxes_down->size() > 0) {
                first_dist = distDown(bbox, _bboxes_down->front());
                Geom::Coord down_dist;
                vecDown.clear();
                if (_findSidewaysSnaps(*bbox_to_snap, _bboxes_down->begin(), _bboxes_down->end(), vecDown, down_dist,
                                      getSnapperTolerance(), &DistributionSnapper::distDown)) {
                    if (abs(down_dist - equal_dist) < 1e-4) {
                        vecUp.insert(vecUp.end(), vecDown.begin(), vecDown.end());
                    }

                } else if (abs(first_dist - equal_dist) < 1e-4) {
                    vecUp.insert(vecUp.end(), vecDown.front());
                }
            }

            dist_y = abs(offset);
            sy = SnappedPoint(target, vecUp, bbox, equal_dist, p.getSourceType(), p.getSourceNum(), SNAPTARGET_DISTRIBUTION_UP, dist_y, getSnapperTolerance(), getSnapperAlwaysSnap(), false, true);
            snap_y = true;
        }
    }

    // 2. if no snaps on top, look Down
    // if there is a snap then add bottom bboxes and look Up, if there is a snap above then
    // add those bboxes too
    if (consider_y && !snap_y && _bboxes_down->size() > 0) {
        vecDown.clear();
        if (_findSidewaysSnaps(*bbox_to_snap, _bboxes_down->begin(), _bboxes_down->end(), vecDown, equal_dist, getSnapperTolerance(), &DistributionSnapper::distDown)) {
            auto first_dist = distDown(*bbox_to_snap, vecDown.front());
            Geom::Coord offset = first_dist - equal_dist;
            Geom::Point target = bbox_to_snap->midpoint() + Geom::Point(0, offset);

            // translate the source bbox to the snap position
            Geom::Affine translation = Geom::Translate(target - bbox_to_snap->midpoint());
            Geom::Rect bbox = *bbox_to_snap * translation;
            vecDown.insert(vecDown.begin(), bbox);

            _correctSelectionBBox(target, p.getPoint(), *bbox_to_snap);

            if (_bboxes_up->size() > 0) {
                first_dist = distUp(bbox, _bboxes_up->front());
                Geom::Coord up_dist;
                vecUp.clear();

                if (_findSidewaysSnaps(*bbox_to_snap, _bboxes_up->begin(), _bboxes_up->end(), vecUp, up_dist, getSnapperTolerance(), &DistributionSnapper::distUp)) {
                    if (compare_double(up_dist, equal_dist)) {
                        std::reverse(vecUp.begin(), vecUp.end());
                        vecDown.insert(vecDown.begin(), vecUp.begin(), vecUp.end());
                    }
                } else if (compare_double(first_dist, equal_dist)) {
                    vecDown.insert(vecDown.begin(), vecUp.front());
                }
            }

            dist_y = abs(offset);
            sy = SnappedPoint(target, vecDown, bbox, equal_dist, p.getSourceType(), p.getSourceNum(), SNAPTARGET_DISTRIBUTION_DOWN, dist_y, getSnapperTolerance(), getSnapperAlwaysSnap(), false, true);
            snap_y = true;
        }
    }

    // 3. if no snap to right or left just add the center snap
    if (consider_y && !snap_y && vecUp.size() > 0 && vecDown.size() > 0) {
        auto y = Geom::Point((vecUp.front().max() + vecDown.front().min()) / 2).y();
        offset = abs(y - bbox_to_snap->midpoint().y());
        if (consider_y && offset < getSnapperTolerance()) {
            Geom::Point target = Geom::Point(bbox_to_snap->midpoint().x(), y);
            // translate the source bbox to the snap position
            Geom::Affine translation = Geom::Translate(target - bbox_to_snap->midpoint());
            Geom::Rect bbox = *bbox_to_snap * translation;
            std::vector<Geom::Rect> bboxes = {vecUp.front(), bbox, vecDown.front()};

            _correctSelectionBBox(target, p.getPoint(), *bbox_to_snap);

            equal_dist = bbox.min().y() - vecUp.front().max().y();
            sy = SnappedPoint(target, bboxes, bbox, equal_dist, p.getSourceType(), p.getSourceNum(), SNAPTARGET_DISTRIBUTION_Y, offset, getSnapperTolerance(), getSnapperAlwaysSnap(), false, true);
            snap_y = true;
        }
    }

    if (snap_x && snap_y) {
        Geom::Point target = Geom::Point(sx.getPoint().x(), sy.getPoint().y());
        Geom::Affine translation = Geom::Translate(target - bbox_to_snap->midpoint());
        Geom::Rect bbox = *bbox_to_snap * translation;
        std::vector<Geom::Rect> bboxes_x = sx.getBBoxes();
        std::vector<Geom::Rect> bboxes_y = sy.getBBoxes();

        _correctSelectionBBox(target, p.getPoint(), *bbox_to_snap);
        auto si = SnappedPoint(target, bboxes_x, bboxes_y, bbox, sx.getDistributionDistance(), sy.getDistributionDistance(), p.getSourceType(), p.getSourceNum(), SNAPTARGET_DISTRIBUTION_XY, offset, getSnapperTolerance(), getSnapperAlwaysSnap(), false, true);
        isr.points.push_back(si);
        return;
    }

    if (snap_x) {
        isr.points.push_back(sx);
    }

    if (snap_y) {
        isr.points.push_back(sy);
    }
}

void Inkscape::DistributionSnapper::_correctSelectionBBox(Geom::Point &target,
                                                          Geom::Point const &p,
                                                          Geom::Rect const &bbox_to_snap) const
{
    if (_snapmanager->_desktop->selection->size() > 1) {
        auto correction = bbox_to_snap.midpoint() - p;
        target -= correction;
    }
}

void Inkscape::DistributionSnapper::freeSnap(IntermSnapResults &isr,
                                             Inkscape::SnapCandidatePoint const &p,
                                             Geom::OptRect const &bbox_to_snap,
                                             std::vector<SPItem const *> const *it,
                                             std::vector<SnapCandidatePoint> *unselected_nodes) const
{
    if (bbox_to_snap.empty())
        return;

    if (!(p.getSourceType() & SNAPSOURCE_BBOX_CATEGORY)) {
        return;
    }

    // toggle checks
    if (!_snap_enabled || !_snapmanager->snapprefs.isTargetSnappable(SNAPTARGET_DISTRIBUTION_CATEGORY))
        return;

    _snapEquidistantPoints(isr, p, bbox_to_snap, unselected_nodes);
}

void Inkscape::DistributionSnapper::constrainedSnap(IntermSnapResults &isr,
                                                    Inkscape::SnapCandidatePoint const &p,
                                                    Geom::OptRect const &bbox_to_snap,
                                                    SnapConstraint const &c,
                                                    std::vector<SPItem const *> const *it,
                                                    std::vector<SnapCandidatePoint> *unselected_nodes) const
{
    if (bbox_to_snap.empty())
        return;

    // project the mouse pointer onto the constraint. Only the projected point will be considered for snapping
    Geom::Point pp = c.projection(p.getPoint());

    // toggle checks
    if (!_snap_enabled || !_snapmanager->snapprefs.isTargetSnappable(SNAPTARGET_DISTRIBUTION_CATEGORY))
        return;

    _snapEquidistantPoints(isr, p, bbox_to_snap, unselected_nodes, c, pp);
}

bool Inkscape::DistributionSnapper::ThisSnapperMightSnap() const
{
    return true;
}

bool Inkscape::DistributionSnapper::getSnapperAlwaysSnap() const
{
    // TODO: Replace this threshold of 10000 by a constant; see also tolerance-slider.cpp
    return _snapmanager->snapprefs.getDistributionTolerance() == 10000;
}

Geom::Coord Inkscape::DistributionSnapper::getSnapperTolerance() const
{
    SPDesktop const *dt = _snapmanager->getDesktop();
    double const zoom = dt ? dt->current_zoom() : 1;
    return _snapmanager->snapprefs.getDistributionTolerance() / zoom;
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
