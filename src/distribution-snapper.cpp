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
#include "snap-enums.h"
#include "style.h"
#include "svg/svg.h"

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

bool Inkscape::DistributionSnapper::findSidewaysSnaps(Geom::Coord first_dist,
                          std::vector<Geom::Rect>::iterator it,
                          std::vector<Geom::Rect>::iterator end,
                          std::vector<Geom::Rect> &vec,
                          Geom::Coord &dist,
                          Geom::Coord tol,
                          std::function<Geom::Coord(Geom::Rect const&, Geom::Rect const&)> const & distance_func,
                          int level) const
{
   Geom::Rect curr_bbox = *it;

   if (it == end)
      return level != 0;

   // TODO: check if rect1.instersects(rect2) gives the same result at rect2.intersects(rect1)
   while (std::next(it) != end && (it->intersects(*std::next(it)) || std::next(it)->intersects(*it))) {
      curr_bbox.unionWith(Geom::OptRect(*++it));
   }

   vec.push_back(curr_bbox);

   if (level == 0) {
      // just add the first bbox to the vector and return if there are no more
      // objects this is used later to find in-between snaps
      if (it + 1 == end) {
         return false;
      }

      dist = distance_func(curr_bbox, *std::next(it));
      if (abs(first_dist - dist) > tol) {
         return false;
      }

      return findSidewaysSnaps(first_dist, ++it, end, vec, dist, tol, distance_func, ++level);
   }

   // TODO: investige how does this tollerance affect the number of equidistant
   // objects that are found? also does multiplying with level help (error propagation)
   if (abs(distance_func(curr_bbox, *std::next(it)) - dist) < level*1e-5) {
      return findSidewaysSnaps(first_dist, ++it, end, vec, dist, tol, distance_func, ++level);
   }

   return true;
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
   //if (!first_point)
      //return;

    _bboxes_right->clear();
    _bboxes_left->clear();
    _bboxes_down->clear();
    _bboxes_up->clear();

    SPItem::BBoxType bbox_type = SPItem::GEOMETRIC_BBOX;

    Preferences *prefs = Preferences::get();
    bool prefs_bbox = prefs->getBool("/tools/bounding_box");
    bbox_type = !prefs_bbox ?
        SPItem::VISUAL_BBOX : SPItem::GEOMETRIC_BBOX;

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
           consider_x = false; // consider horizontl snapping if moving vertically
       else
           consider_y = false; // consider vertical snapping if moving horizontally 
   }

   _collectBBoxes(bbox_to_snap, p.getSourceNum() <= 0);

   Geom::Coord offset;

   if (p.getSourceType() != SNAPSOURCE_BBOX_MIDPOINT)
      return;

   Geom::Coord equal_dist;

   SnappedPoint sr, sl, sx, su, sd, sy;
   Geom::Coord dist_r, dist_l, dist_u, dist_d;
   bool snap_r = false, snap_l = false, snap_u = false, snap_d = false, snap_x = false, snap_y = false;

   // right snaps
   std::vector<Geom::Rect> vecRight;
   if (consider_x && _bboxes_right->size() > 0) {
      auto first_dist = distRight(*bbox_to_snap, _bboxes_right->front());

      if (findSidewaysSnaps(first_dist,
                        _bboxes_right->begin(),
                        _bboxes_right->end(),
                        vecRight,
                        equal_dist,
                        getSnapperTolerance(),
                        &DistributionSnapper::distRight)) {

         Geom::Coord offset = first_dist - equal_dist;
         Geom::Point target = bbox_to_snap->midpoint() + Geom::Point(offset, 0);

         Geom::Affine translation = Geom::Translate(target - bbox_to_snap->midpoint());
         Geom::Rect bbox = *bbox_to_snap * translation;
         vecRight.insert(vecRight.begin(), bbox);
           
         _correctSelectionBBox(target, p.getPoint(), *bbox_to_snap);

         dist_r = abs(offset);
         sr = SnappedPoint(target, vecRight, bbox, equal_dist, p.getSourceType(), p.getSourceNum(), SNAPTARGET_DISTRIBUTION_RIGHT, dist_r, getSnapperTolerance(), getSnapperAlwaysSnap(), false, true);
         snap_r = true;
      }
   }

   // left snaps
   std::vector<Geom::Rect> vecLeft;
   if (consider_x && _bboxes_left->size() > 0) {
      auto first_dist = distLeft(*bbox_to_snap, _bboxes_left->front());

      if (findSidewaysSnaps(first_dist,
                        _bboxes_left->begin(),
                        _bboxes_left->end(),
                        vecLeft,
                        equal_dist,
                        getSnapperTolerance(),
                        &DistributionSnapper::distLeft)) {

         Geom::Coord offset = first_dist - equal_dist;
         Geom::Point target = bbox_to_snap->midpoint() - Geom::Point(offset, 0);

         // translate the source bbox to the snap position
         Geom::Affine translation = Geom::Translate(target - bbox_to_snap->midpoint());
         Geom::Rect bbox = *bbox_to_snap * translation;
         std::reverse(vecLeft.begin(), vecLeft.end());
         vecLeft.push_back(bbox);

         _correctSelectionBBox(target, p.getPoint(), *bbox_to_snap);

         dist_l = abs(offset);
         sl = SnappedPoint(target, vecLeft, bbox, equal_dist, p.getSourceType(), p.getSourceNum(), SNAPTARGET_DISTRIBUTION_LEFT, dist_l, getSnapperTolerance(), getSnapperAlwaysSnap(), false, true);
         snap_l = true;
      }
   }

   // up snaps
   std::vector<Geom::Rect> vecUp;
   if (consider_y && _bboxes_up->size() > 0) {
      auto first_dist = distUp(*bbox_to_snap, _bboxes_up->front());

      if (findSidewaysSnaps(first_dist,
                        _bboxes_up->begin(),
                        _bboxes_up->end(),
                        vecUp,
                        equal_dist,
                        getSnapperTolerance(),
                        &DistributionSnapper::distUp)) {

         Geom::Coord offset = first_dist - equal_dist;
         Geom::Point target = bbox_to_snap->midpoint() - Geom::Point(0, offset);

         // translate the source bbox to the snap position
         Geom::Affine translation = Geom::Translate(target - bbox_to_snap->midpoint());
         Geom::Rect bbox = *bbox_to_snap * translation;
         std::reverse(vecUp.begin(), vecUp.end());
         vecUp.push_back(bbox);

         _correctSelectionBBox(target, p.getPoint(), *bbox_to_snap);

         dist_u = abs(offset);
         su = SnappedPoint(target, vecUp, bbox, equal_dist, p.getSourceType(), p.getSourceNum(), SNAPTARGET_DISTRIBUTION_UP, dist_u, getSnapperTolerance(), getSnapperAlwaysSnap(), false, true);
         snap_u = true;
      }
   }

   // down snaps
   std::vector<Geom::Rect> vecDown;
   if (consider_y && _bboxes_down->size() > 0) {
      auto first_dist = distDown(*bbox_to_snap, _bboxes_down->front());

      if (findSidewaysSnaps(first_dist,
                        _bboxes_down->begin(),
                        _bboxes_down->end(),
                        vecDown,
                        equal_dist,
                        getSnapperTolerance(),
                        &DistributionSnapper::distDown)) {

         Geom::Coord offset = first_dist - equal_dist;
         Geom::Point target = bbox_to_snap->midpoint() + Geom::Point(0, offset);

         // translate the source bbox to the snap position
         Geom::Affine translation = Geom::Translate(target - bbox_to_snap->midpoint());
         Geom::Rect bbox = *bbox_to_snap * translation;
         vecDown.insert(vecDown.begin(), bbox);

         _correctSelectionBBox(target, p.getPoint(), *bbox_to_snap);

         dist_d = abs(offset);
         sd = SnappedPoint(target, vecDown, bbox, equal_dist, p.getSourceType(), p.getSourceNum(), SNAPTARGET_DISTRIBUTION_DOWN, dist_d, getSnapperTolerance(), getSnapperAlwaysSnap(), false, true);
         snap_d = true;
      }
   }

   // in between snap
   Geom::Point pointR = Geom::Point(Geom::infinity(), Geom::infinity());
   if (vecRight.size() > 0)
      pointR = vecRight.front().min();

   Geom::Point pointL = Geom::Point(Geom::infinity(), Geom::infinity());
   if (vecLeft.size() > 0)
      pointL = vecLeft.front().max();

   Geom::Point pointU = Geom::Point(Geom::infinity(), Geom::infinity());
   if (vecUp.size() > 0)
      pointU = vecUp.front().max();

   Geom::Point pointD = Geom::Point(Geom::infinity(), Geom::infinity());
   if (vecDown.size() > 0)
      pointD = vecDown.front().min();


   // horizontally in between
   auto x = Geom::Point((pointR + pointL)/2).x();
   offset = abs(x - bbox_to_snap->midpoint().x());
   if (consider_x && offset < getSnapperTolerance()) {
      std::vector<Geom::Rect> bboxes = {vecLeft.front(), vecRight.front()};
      Geom::Point target = Geom::Point(x, bbox_to_snap->midpoint().y());
      // translate the source bbox to the snap position
      Geom::Affine translation = Geom::Translate(target - bbox_to_snap->midpoint());
      Geom::Rect bbox = *bbox_to_snap * translation;

      _correctSelectionBBox(target, p.getPoint(), *bbox_to_snap);

      equal_dist = bbox.min().x() - pointL.x();
      sx = SnappedPoint(target, bboxes, bbox, equal_dist, p.getSourceType(), p.getSourceNum(), SNAPTARGET_DISTRIBUTION_X, offset, getSnapperTolerance(), getSnapperAlwaysSnap(), false, true);
      snap_x = true;
   }

   // vertically in between
   auto y = Geom::Point((pointU + pointD)/2).y();
   equal_dist = bbox_to_snap->min().y() - pointU.y();
   offset = abs(y - bbox_to_snap->midpoint().y());
   if (consider_y && offset < getSnapperTolerance()) {
      std::vector<Geom::Rect> bboxes = {vecUp.front(), vecDown.front()};
      Geom::Point target = Geom::Point(bbox_to_snap->midpoint().x(), y);
      // translate the source bbox to the snap position
      Geom::Affine translation = Geom::Translate(target - bbox_to_snap->midpoint());
      Geom::Rect bbox = *bbox_to_snap * translation;

      _correctSelectionBBox(target, p.getPoint(), *bbox_to_snap);

      equal_dist = bbox.min().y() - pointU.y();
      sy = SnappedPoint(target, bboxes, bbox, equal_dist, p.getSourceType(), p.getSourceNum(), SNAPTARGET_DISTRIBUTION_Y, offset, getSnapperTolerance(), getSnapperAlwaysSnap(), false, true);
      snap_y = true;
   }

   if (snap_x) {
      isr.points.push_back(sx);
      std::cout<<"center"<<std::endl;
      return;
   }

   if (snap_y) {
      isr.points.push_back(sy);
   }

   if (snap_r) {
      isr.points.push_back(sr);
   }

   if (snap_l) {
      isr.points.push_back(sl);
   }

   if (snap_u) {
      isr.points.push_back(su);
   }

   if (snap_d) {
      isr.points.push_back(sd);
   }
}

void Inkscape::DistributionSnapper::_correctSelectionBBox(Geom::Point &target, Geom::Point const &p, Geom::Rect const &bbox_to_snap) const
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
    return _snapmanager->snapprefs.getDistributionTolerance() == 10000; //TODO: Replace this threshold of 10000 by a constant; see also tolerance-slider.cpp
}

Geom::Coord Inkscape::DistributionSnapper::getSnapperTolerance() const
{
    SPDesktop const *dt = _snapmanager->getDesktop();
    double const zoom =  dt ? dt->current_zoom() : 1;
    return _snapmanager->snapprefs.getDistributionTolerance() / zoom;
}
