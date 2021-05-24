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

#include "inkscape.h"

Inkscape::AlignmentSnapper::AlignmentSnapper(SnapManager *sm, Geom::Coord const d)
    : Snapper(sm, d)
{
}

Inkscape::AlignmentSnapper::~AlignmentSnapper()
{
}

void Inkscape::AlignmentSnapper::freeSnap(IntermSnapResults &isr,
                                          Inkscape::SnapCandidatePoint const &p,
                                          Geom::OptRect const &bbox_to_snap,
                                          std::vector<SPItem const *> const *it,
                                          std::vector<SnapCandidatePoint> *unselected_nodes) const
{
}

void Inkscape::AlignmentSnapper::constrainedSnap(IntermSnapResults &isr,
              Inkscape::SnapCandidatePoint const &p,
              Geom::OptRect const &bbox_to_snap,
              SnapConstraint const &c,
              std::vector<SPItem const *> const *it,
              std::vector<SnapCandidatePoint> *unselected_nodes) const
{
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



