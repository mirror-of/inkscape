// SPDX-License-Identifier: GPL-2.0-or-later
/** \file
 * Anchors implementation.
 */

/*
 * Authors:
 * Copyright (C) 2000 Lauris Kaplinski
 * Copyright (C) 2000-2001 Ximian, Inc.
 * Copyright (C) 2002 Lauris Kaplinski
 * Copyright (C) 2004 Monash University
 *
 * Released under GNU GPL v2+, read the file 'COPYING' for more information.
 */


#include "ui/draw-anchor.h"
#include "ui/tools/tool-base.h"
#include "ui/tools/lpe-tool.h"

#include "display/control/canvas-item-ctrl.h"
#include "display/curve.h"

const guint32 FILL_COLOR_NORMAL    = 0xffffff7f;
const guint32 FILL_COLOR_MOUSEOVER = 0xff0000ff;

/**
 * Creates an anchor object and initializes it.
 */
SPDrawAnchor::SPDrawAnchor(Inkscape::UI::Tools::FreehandBase *dc, SPCurve *curve, bool start, Geom::Point delta)
    : dc(dc), curve(curve->ref()), start(start), active(FALSE), dp(delta),
      ctrl(
        new Inkscape::CanvasItemCtrl(
          dc->getDesktop()->getCanvasControls(),
          Inkscape::CANVAS_ITEM_CTRL_TYPE_ANCHOR
        )
      )
{
    ctrl->set_name("CanvasItemCtrl:DrawAnchor");
    ctrl->set_fill(FILL_COLOR_NORMAL);
    ctrl->set_position(delta);
    ctrl->set_pickable(false); // We do our own checking. (TODO: Should be fixed!)
}

SPDrawAnchor::~SPDrawAnchor()
{
    if (ctrl) {
        delete (ctrl);
    }
}

/**
 * Test if point is near anchor, if so fill anchor on canvas and return
 * pointer to it or NULL.
 */
SPDrawAnchor *SPDrawAnchor::anchorTest(Geom::Point w, bool activate)
{
    if ( activate && this->ctrl->contains(w)) {
        
        if (!this->active) {
            this->ctrl->set_size_extra(4);
            this->ctrl->set_fill(FILL_COLOR_MOUSEOVER);
            this->active = TRUE;
        }
        return this;
    }

    if (this->active) {
        this->ctrl->set_size_extra(0);
        this->ctrl->set_fill(FILL_COLOR_NORMAL);
        this->active = FALSE;
    }

    return nullptr;
}

/**
 * Destroys the anchor's canvas item and frees the anchor object.
 */
SPDrawAnchor *SPDrawAnchor::anchorDestroy()
{
    delete this;
    return nullptr;
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
