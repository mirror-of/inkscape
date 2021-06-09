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

static int findRightSnaps(std::vector<Geom::Rect>::iterator it, std::vector<Geom::Rect>::iterator end, Geom::Coord &dist, int level = 0)
{
   if (level > 5)   
      return 5;

   //if (it == end)
      //return level;

   if (std::next(it) == end)
      return level;

   if (it->intersects(*std::next(it)))
      return level;

   if (level == 0) {
      dist = - it->max().x() + std::next(it)->min().x();
      return findRightSnaps(++it, end, dist, ++level);
   }

   // 1e-5 for accuracy if we use equality (==), it never satisfies.
   if (-dist - it->max().x() + std::next(it)->min().x() < 1e-5) {
      return findRightSnaps(++it, end, dist, ++level);
   } else { 
      return level;
   }
}

static int findLeftSnaps(std::vector<Geom::Rect>::iterator it, std::vector<Geom::Rect>::iterator end, Geom::Coord &dist, int level = 0)
{
   if (level > 5)   
      return 5;

   //if (it == end)
      //return level;

   if (std::next(it) == end)
      return level;

   if ((it)->intersects(*std::next(it)))
      return level;

   if (level == 0) {
      dist = it->min().x() - std::next(it)->max().x();
      return findLeftSnaps(++it, end, dist, ++level);
   }

   // 1e-5 for accuracy if we use equality (==), it never satisfies.
   if (-dist + it->min().x() - std::next(it)->max().x() < 1e-5) {
      return findLeftSnaps(++it, end, dist, ++level);
   } else { 
      return level;
   }
}

static int findUpSnaps(std::vector<Geom::Rect>::iterator it, std::vector<Geom::Rect>::iterator end, Geom::Coord &dist, int level = 0)
{
   if (level > 5)   
      return 5;

   //if (it == end)
      //return level;

   if (std::next(it) == end)
      return level;

   if ((std::next(it))->intersects(*it))
      return level;

   if (level == 0) {
      dist = it->min().y() - std::next(it)->max().y();
      return findUpSnaps(++it, end, dist, ++level);
   }

   // 1e-5 for accuracy if we use equality (==), it never satisfies.
   if (dist - it->min().y() + std::next(it)->max().y() < 1e-5) {
      return findUpSnaps(++it, end, dist, ++level);
   } else { 
      return level;
   }
}

static int findDownSnaps(std::vector<Geom::Rect>::iterator it, std::vector<Geom::Rect>::iterator end, Geom::Coord &dist, int level = 0)
{
   if (level > 5)   
      return 5;

   //if (it == end)
      //return level;

   if (std::next(it) == end)
      return level;

   if ((std::next(it))->intersects(*it))
      return level;

   if (level == 0) {
      dist = - it->max().y() + std::next(it)->min().y();
      return findDownSnaps(++it, end, dist, ++level);
   }

   // 1e-5 for accuracy if we use equality (==), it never satisfies.
   if (dist + it->max().y() - std::next(it)->min().y() < 1e-5) {
      return findDownSnaps(++it, end, dist, ++level);
   } else { 
      return level;
   }
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

    // Debug log
    //std::cout<<"----------"<<std::endl;
    //std::cout<<"Right: "<<_bboxes_right->size()<<std::endl;
    //std::cout<<"Left: "<<_bboxes_left->size()<<std::endl;
    //std::cout<<"Up: "<<_bboxes_up->size()<<std::endl;
    //std::cout<<"Down: "<<_bboxes_down->size()<<std::endl;
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

    Geom::Coord dist;

    if (p.getSourceType() != SNAPSOURCE_BBOX_MIDPOINT)
       return;

    // in between snap
    Geom::Point pointR = Geom::Point(Geom::infinity(), Geom::infinity());
    if (_bboxes_right->size() > 0)
       pointR = _bboxes_right->begin()->min();

    Geom::Point pointL = Geom::Point(Geom::infinity(), Geom::infinity());
    if (_bboxes_left->size() > 0)
       pointL = _bboxes_left->begin()->max();

    Geom::Point pointU = Geom::Point(Geom::infinity(), Geom::infinity());
    if (_bboxes_up->size() > 0)
       pointU = _bboxes_up->begin()->max();

    Geom::Point pointD = Geom::Point(Geom::infinity(), Geom::infinity());
    if (_bboxes_down->size() > 0)
       pointD = _bboxes_down->begin()->min();

    Geom::Coord equal_dist;

    // horizontally in between
    auto x = Geom::Point((pointR + pointL)/2).x();
    equal_dist = x - pointL.x();
    dist = abs(x - bbox_to_snap->midpoint().x());
    if (consider_x && dist < getSnapperTolerance()) {
       std::vector<Geom::Rect> bboxes = {*_bboxes_left->begin(), *_bboxes_right->begin()};
       auto s = SnappedPoint(Geom::Point(x, bbox_to_snap->midpoint().y()), bboxes, *bbox_to_snap, equal_dist, p.getSourceType(), p.getSourceNum(), SNAPTARGET_DISTRIBUTION_X, dist, getSnapperTolerance(), getSnapperAlwaysSnap(), false, true);
       //std::cout<<"X snap to"<<Geom::Point(x, bbox_to_snap->midpoint().y())<<std::endl;
       isr.points.push_back(s);
    }

    // vertically in between
    auto y = Geom::Point((pointU + pointD)/2).y();
    equal_dist = y - pointU.y();
    dist = abs(y - bbox_to_snap->midpoint().y());
    if (consider_y && dist < getSnapperTolerance()) {
       std::vector<Geom::Rect> bboxes = {*_bboxes_up->begin(), *_bboxes_down->begin()};
       auto s = SnappedPoint(Geom::Point(bbox_to_snap->midpoint().x(), y), bboxes, *bbox_to_snap, equal_dist, p.getSourceType(), p.getSourceNum(), SNAPTARGET_DISTRIBUTION_Y, dist, getSnapperTolerance(), getSnapperAlwaysSnap(), false, true);
       //std::cout<<"Y snap to"<<<<std::endl;
       isr.points.push_back(s);
    }

    // right snaps
    if (consider_x && _bboxes_right->size() > 1) {
       int num = findRightSnaps(_bboxes_right->begin(), _bboxes_right->end(), equal_dist);
       if (num > 0) {
          Geom::Coord offset = - bbox_to_snap->max().x() + _bboxes_right->begin()->min().x() - equal_dist;
          Geom::Point target = bbox_to_snap->midpoint() + Geom::Point(offset, 0);

         if (abs(offset) < getSnapperTolerance()) {
             std::vector<Geom::Rect> bboxes(_bboxes_right->begin(), _bboxes_right->begin() + num + 1);
             auto s = SnappedPoint(target, bboxes, *bbox_to_snap, equal_dist, p.getSourceType(), p.getSourceNum(), SNAPTARGET_DISTRIBUTION_RIGHT, abs(offset), getSnapperTolerance(), getSnapperAlwaysSnap(), false, true);
             isr.points.push_back(s);

             // Debug log
             //std::cout<<"------"<<std::endl;
             //std::cout<<"Level: "<<num<<std::endl;
             //std::cout<<"R snap to"<<target<<std::endl;
             //std::cout<<"offset "<<abs(offset)<<std::endl;
             //std::cout<<"equal_dist "<<equal_dist<<std::endl;
             //std::cout<<"target"<<target<<std::endl;
         }
       }
    }

    // left snaps
    if (consider_x && _bboxes_left->size() > 1 ) {
       int num = findLeftSnaps(_bboxes_left->begin(), _bboxes_left->end(), equal_dist);
       if (num > 0) {
          Geom::Coord offset = bbox_to_snap->min().x() - _bboxes_left->begin()->max().x() - equal_dist;
          Geom::Point target = bbox_to_snap->midpoint() - Geom::Point(offset, 0);

         if (abs(offset) < getSnapperTolerance()) {
             std::vector<Geom::Rect> bboxes(_bboxes_left->begin(), _bboxes_left->begin() + num + 1);
             auto s = SnappedPoint(target, bboxes, *bbox_to_snap, equal_dist, p.getSourceType(), p.getSourceNum(), SNAPTARGET_DISTRIBUTION_LEFT, abs(offset), getSnapperTolerance(), getSnapperAlwaysSnap(), false, true);
             isr.points.push_back(s);

             // Debug log
             //std::cout<<"------"<<std::endl;
             //std::cout<<"Level: "<<num<<std::endl;
             //std::cout<<"L snap to "<<target<<std::endl;
             //std::cout<<"offset "<<abs(offset)<<std::endl;
             //std::cout<<"equal_dist "<<equal_dist<<std::endl;
         }
       }
    }

    // up snaps
    if (consider_y && _bboxes_up->size() > 1 ) {
       int num = findUpSnaps(_bboxes_up->begin(), _bboxes_up->end(), equal_dist);
       if (num > 0) {
          Geom::Coord offset = bbox_to_snap->min().y() - _bboxes_up->begin()->max().y() - equal_dist;
          Geom::Point target = bbox_to_snap->midpoint() - Geom::Point(0, offset);

         if (abs(offset) < getSnapperTolerance()) {
             std::vector<Geom::Rect> bboxes(_bboxes_up->begin(), _bboxes_up->begin() + num + 1);
             auto s = SnappedPoint(target, bboxes, *bbox_to_snap, equal_dist, p.getSourceType(), p.getSourceNum(), SNAPTARGET_DISTRIBUTION_UP, abs(offset), getSnapperTolerance(), getSnapperAlwaysSnap(), false, true);
             isr.points.push_back(s);

             // Debug log
             //std::cout<<"------"<<std::endl;
             //std::cout<<"Level: "<<num<<std::endl;
             //std::cout<<"U snap to "<<target<<std::endl;
             //std::cout<<"offset "<<abs(offset)<<std::endl;
             //std::cout<<"equal_dist "<<equal_dist<<std::endl;
         }
       }
    }

    // down snaps
    if (consider_y && _bboxes_down->size() > 1 ) {
       int num = findDownSnaps(_bboxes_down->begin(), _bboxes_down->end(), equal_dist);
       if (num > 0) {
          Geom::Coord offset = - bbox_to_snap->max().y() + _bboxes_down->begin()->min().y() - equal_dist;
          Geom::Point target = bbox_to_snap->midpoint() + Geom::Point(0, offset);

         if (abs(offset) < getSnapperTolerance()) {
             std::vector<Geom::Rect> bboxes(_bboxes_down->begin(), _bboxes_down->begin() + num + 1);
             auto s = SnappedPoint(target, bboxes, *bbox_to_snap, equal_dist, p.getSourceType(), p.getSourceNum(), SNAPTARGET_DISTRIBUTION_DOWN, abs(offset), getSnapperTolerance(), getSnapperAlwaysSnap(), false, true);
             isr.points.push_back(s);

             // Debug log
             //std::cout<<"------"<<std::endl;
             //std::cout<<"Level: "<<num<<std::endl;
             //std::cout<<"D snap to "<<target<<std::endl;
             //std::cout<<"offset "<<abs(offset)<<std::endl;
             //std::cout<<"equal_dist "<<equal_dist<<std::endl;
         }
       }
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
    return _snapmanager->snapprefs.getAlignmentTolerance() == 10000; //TODO: Replace this threshold of 10000 by a constant; see also tolerance-slider.cpp
}

Geom::Coord Inkscape::DistributionSnapper::getSnapperTolerance() const
{
    SPDesktop const *dt = _snapmanager->getDesktop();
    double const zoom =  dt ? dt->current_zoom() : 1;
    return _snapmanager->snapprefs.getDistributionTolerance() / zoom;
}
