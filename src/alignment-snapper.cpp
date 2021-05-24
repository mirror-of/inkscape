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
#include "style.h"
#include "svg/svg.h"
#include "text-editing.h"

Inkscape::AlignmentSnapper::AlignmentSnapper(SnapManager *sm, Geom::Coord const d)
    : Snapper(sm, d)
{
    _candidates = new std::vector<SnapCandidateItem>;
}

Inkscape::AlignmentSnapper::~AlignmentSnapper()
{
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


void Inkscape::AlignmentSnapper::freeSnap(IntermSnapResults &isr,
                                          Inkscape::SnapCandidatePoint const &p,
                                          Geom::OptRect const &bbox_to_snap,
                                          std::vector<SPItem const *> const *it,
                                          std::vector<SnapCandidatePoint> *unselected_nodes) const
{
    // TODO: add toggle checks here
    if (p.getSourceNum() <= 0){
        _candidates->clear();
        _findCandidates(_snapmanager->getDocument()->getRoot(), it, true, false);
    }
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



