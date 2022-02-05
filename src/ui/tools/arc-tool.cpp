// SPDX-License-Identifier: GPL-2.0-or-later
/**
 * @file
 * Ellipse drawing context.
 */
/* Authors:
 *   Mitsuru Oka
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   bulia byak <buliabyak@users.sf.net>
 *   Johan Engelen <johan@shouraizou.nl>
 *   Abhishek Sharma
 *   Jon A. Cruz <jon@joncruz.org>
 *
 * Copyright (C) 2000-2006 Authors
 * Copyright (C) 2000-2001 Ximian, Inc.
 *
 * Released under GNU GPL v2+, read the file 'COPYING' for more information.
 */
#include <glibmm/i18n.h>
#include <gdk/gdkkeysyms.h>

#include "context-fns.h"
#include "desktop-style.h"
#include "desktop.h"
#include "document-undo.h"
#include "document.h"
#include "message-context.h"
#include "preferences.h"
#include "selection.h"
#include "snap.h"

#include "include/macros.h"

#include "object/sp-ellipse.h"
#include "object/sp-namedview.h"

#include "ui/icon-names.h"
#include "ui/modifiers.h"
#include "ui/tools/arc-tool.h"
#include "ui/shape-editor.h"
#include "ui/tools/tool-base.h"

#include "xml/repr.h"
#include "xml/node-event-vector.h"

using Inkscape::DocumentUndo;

namespace Inkscape {
namespace UI {
namespace Tools {

ArcTool::ArcTool(SPDesktop *desktop)
    : ToolBase(desktop, "/tools/shapes/arc", "arc.svg")
    , arc(nullptr)
{
    Inkscape::Selection *selection = desktop->getSelection();

    this->shape_editor = new ShapeEditor(desktop);

    SPItem *item = desktop->getSelection()->singleItem();
    if (item) {
        this->shape_editor->set_item(item);
    }

    this->sel_changed_connection.disconnect();
    this->sel_changed_connection = selection->connectChanged(
        sigc::mem_fun(this, &ArcTool::selection_changed)
    );

    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    if (prefs->getBool("/tools/shapes/selcue")) {
        this->enableSelectionCue();
    }

    if (prefs->getBool("/tools/shapes/gradientdrag")) {
        this->enableGrDrag();
    }
}

ArcTool::~ArcTool()
{
    ungrabCanvasEvents();
    this->finishItem();
    this->sel_changed_connection.disconnect();

    this->enableGrDrag(false);

    this->sel_changed_connection.disconnect();

    delete this->shape_editor;
    this->shape_editor = nullptr;

    /* fixme: This is necessary because we do not grab */
    if (this->arc) {
        this->finishItem();
    }
}

/**
 * Callback that processes the "changed" signal on the selection;
 * destroys old and creates new knotholder.
 */
void ArcTool::selection_changed(Inkscape::Selection* selection) {
    this->shape_editor->unset_item();
    this->shape_editor->set_item(selection->singleItem());
}


bool ArcTool::item_handler(SPItem* item, GdkEvent* event) {
    switch (event->type) {
        case GDK_BUTTON_PRESS:
            if (event->button.button == 1) {
                this->setup_for_drag_start(event);
            }
            break;
            // motion and release are always on root (why?)
        default:
            break;
    }

    return ToolBase::item_handler(item, event);
}

bool ArcTool::root_handler(GdkEvent* event) {
    static bool dragging;

    Inkscape::Selection *selection = _desktop->getSelection();
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();

    this->tolerance = prefs->getIntLimited("/options/dragtolerance/value", 0, 0, 100);

    bool handled = false;

    switch (event->type) {
        case GDK_BUTTON_PRESS:
            if (event->button.button == 1) {
                dragging = true;

                this->center = this->setup_for_drag_start(event);

                /* Snap center */
                SnapManager &m = _desktop->namedview->snap_manager;
                m.setup(_desktop);
                m.freeSnapReturnByRef(this->center, Inkscape::SNAPSOURCE_NODE_HANDLE);

                grabCanvasEvents();

                handled = true;
                m.unSetup();
            }
            break;
        case GDK_MOTION_NOTIFY:
            if (dragging && (event->motion.state & GDK_BUTTON1_MASK)) {
                if ( this->within_tolerance
                     && ( abs( (gint) event->motion.x - this->xp ) < this->tolerance )
                     && ( abs( (gint) event->motion.y - this->yp ) < this->tolerance ) ) {
                    break; // do not drag if we're within tolerance from origin
                }
                // Once the user has moved farther than tolerance from the original location
                // (indicating they intend to draw, not click), then always process the
                // motion notify coordinates as given (no snapping back to origin)
                this->within_tolerance = false;

                Geom::Point const motion_w(event->motion.x, event->motion.y);
                Geom::Point motion_dt(_desktop->w2d(motion_w));

                this->drag(motion_dt, event->motion.state);

                gobble_motion_events(GDK_BUTTON1_MASK);

                handled = true;
            } else if (!this->sp_event_context_knot_mouseover()){
                SnapManager &m = _desktop->namedview->snap_manager;
                m.setup(_desktop);

                Geom::Point const motion_w(event->motion.x, event->motion.y);
                Geom::Point motion_dt(_desktop->w2d(motion_w));
                m.preSnap(Inkscape::SnapCandidatePoint(motion_dt, Inkscape::SNAPSOURCE_NODE_HANDLE));
                m.unSetup();
            }
            break;
        case GDK_BUTTON_RELEASE:
            this->xp = this->yp = 0;
            if (event->button.button == 1) {
                dragging = false;
                this->discard_delayed_snap_event();

                if (!this->within_tolerance) {
                    // we've been dragging, finish the arc
                    this->finishItem();
                } else if (this->item_to_select) {
                    // no dragging, select clicked item if any
                    if (event->button.state & GDK_SHIFT_MASK) {
                        selection->toggle(this->item_to_select);
                    } else {
                        selection->set(this->item_to_select);
                    }
                } else {
                    // click in an empty space
                    selection->clear();
                }

                this->xp = 0;
                this->yp = 0;
                this->item_to_select = nullptr;
                handled = true;
            }
            ungrabCanvasEvents();
            break;

        case GDK_KEY_PRESS:
            switch (get_latin_keyval (&event->key)) {
                case GDK_KEY_Alt_L:
                case GDK_KEY_Alt_R:
                case GDK_KEY_Control_L:
                case GDK_KEY_Control_R:
                case GDK_KEY_Shift_L:
                case GDK_KEY_Shift_R:
                case GDK_KEY_Meta_L:  // Meta is when you press Shift+Alt (at least on my machine)
                case GDK_KEY_Meta_R:
                    if (!dragging) {
                        sp_event_show_modifier_tip(this->defaultMessageContext(), event,
                                                   _("<b>Ctrl</b>: make circle or integer-ratio ellipse, snap arc/segment angle"),
                                                   _("<b>Shift</b>: draw around the starting point"),
                                                   nullptr);
                    }
                    break;

                case GDK_KEY_x:
                case GDK_KEY_X:
                    if (MOD__ALT_ONLY(event)) {
                        _desktop->setToolboxFocusTo("arc-rx");
                        handled = true;
                    }
                    break;

                case GDK_KEY_Escape:
                    if (dragging) {
                        dragging = false;
                        this->discard_delayed_snap_event();
                        // if drawing, cancel, otherwise pass it up for deselecting
                        this->cancel();
                        handled = true;
                    }
                    break;

                case GDK_KEY_space:
                    if (dragging) {
                        ungrabCanvasEvents();
                        dragging = false;
                        this->discard_delayed_snap_event();

                        if (!this->within_tolerance) {
                            // we've been dragging, finish the arc
                            this->finishItem();
                        }
                        // do not return true, so that space would work switching to selector
                    }
                    break;

                case GDK_KEY_Delete:
                case GDK_KEY_KP_Delete:
                case GDK_KEY_BackSpace:
                    handled = this->deleteSelectedDrag(MOD__CTRL_ONLY(event));
                    break;

                default:
                    break;
            }
            break;

        case GDK_KEY_RELEASE:
            switch (event->key.keyval) {
                case GDK_KEY_Alt_L:
                case GDK_KEY_Alt_R:
                case GDK_KEY_Control_L:
                case GDK_KEY_Control_R:
                case GDK_KEY_Shift_L:
                case GDK_KEY_Shift_R:
                case GDK_KEY_Meta_L:  // Meta is when you press Shift+Alt
                case GDK_KEY_Meta_R:
                    this->defaultMessageContext()->clear();
                    break;

                default:
                    break;
            }
            break;

        default:
            break;
    }

    if (!handled) {
    	handled = ToolBase::root_handler(event);
    }

    return handled;
}

void ArcTool::drag(Geom::Point pt, guint state) {
    if (!this->arc) {
        if (Inkscape::have_viable_layer(_desktop, defaultMessageContext()) == false) {
            return;
        }

        // Create object
        Inkscape::XML::Document *xml_doc = _desktop->doc()->getReprDoc();
        Inkscape::XML::Node *repr = xml_doc->createElement("svg:path");
        repr->setAttribute("sodipodi:type", "arc");

        // Set style
        sp_desktop_apply_style_tool(_desktop, repr, "/tools/shapes/arc", false);

        auto layer = currentLayer();
        this->arc = SP_GENERICELLIPSE(layer->appendChildRepr(repr));
        Inkscape::GC::release(repr);
        this->arc->transform = layer->i2doc_affine().inverse();
        this->arc->updateRepr();
    }

    auto confine = Modifiers::Modifier::get(Modifiers::Type::TRANS_CONFINE)->active(state);
    // Third is weirdly wrong, surely incrememnts should do something else.
    auto circle_edge = Modifiers::Modifier::get(Modifiers::Type::TRANS_INCREMENT)->active(state);

    Geom::Rect r = Inkscape::snap_rectangular_box(_desktop, this->arc, pt, this->center, state);

    Geom::Point dir = r.dimensions() / 2;


    if (circle_edge) {
        /* With Alt let the ellipse pass through the mouse pointer */
        Geom::Point c = r.midpoint();

        if (!confine) {
            if (fabs(dir[Geom::X]) > 1E-6 && fabs(dir[Geom::Y]) > 1E-6) {
                Geom::Affine const i2d ( (this->arc)->i2dt_affine() );
                Geom::Point new_dir = pt * i2d - c;
                new_dir[Geom::X] *= dir[Geom::Y] / dir[Geom::X];
                double lambda = new_dir.length() / dir[Geom::Y];
                r = Geom::Rect (c - lambda*dir, c + lambda*dir);
            }
        } else {
            /* with Alt+Ctrl (without Shift) we generate a perfect circle
               with diameter click point <--> mouse pointer */
                double l = dir.length();
                Geom::Point d (l, l);
                r = Geom::Rect (c - d, c + d);
        }
    }

    this->arc->position_set(
        r.midpoint()[Geom::X], r.midpoint()[Geom::Y],
        r.dimensions()[Geom::X] / 2, r.dimensions()[Geom::Y] / 2);

    double rdimx = r.dimensions()[Geom::X];
    double rdimy = r.dimensions()[Geom::Y];

    Inkscape::Util::Quantity rdimx_q = Inkscape::Util::Quantity(rdimx, "px");
    Inkscape::Util::Quantity rdimy_q = Inkscape::Util::Quantity(rdimy, "px");
    Glib::ustring xs = rdimx_q.string(_desktop->namedview->display_units);
    Glib::ustring ys = rdimy_q.string(_desktop->namedview->display_units);

    if (state & GDK_CONTROL_MASK) {
        int ratio_x, ratio_y;
        bool is_golden_ratio = false;

        if (fabs (rdimx) > fabs (rdimy)) {
            if (fabs(rdimx / rdimy - goldenratio) < 1e-6) {
                is_golden_ratio = true;
            }

            ratio_x = (int) rint (rdimx / rdimy);
            ratio_y = 1;
        } else {
            if (fabs(rdimy / rdimx - goldenratio) < 1e-6) {
                is_golden_ratio = true;
            }

            ratio_x = 1;
            ratio_y = (int) rint (rdimy / rdimx);
        }

        if (!is_golden_ratio) {
            this->message_context->setF(Inkscape::IMMEDIATE_MESSAGE,
                    _("<b>Ellipse</b>: %s &#215; %s (constrained to ratio %d:%d); with <b>Shift</b> to draw around the starting point"),
                    xs.c_str(), ys.c_str(), ratio_x, ratio_y);
        } else {
            if (ratio_y == 1) {
                this->message_context->setF(Inkscape::IMMEDIATE_MESSAGE,
                        _("<b>Ellipse</b>: %s &#215; %s (constrained to golden ratio 1.618 : 1); with <b>Shift</b> to draw around the starting point"),
                        xs.c_str(), ys.c_str());
            } else {
                this->message_context->setF(Inkscape::IMMEDIATE_MESSAGE,
                        _("<b>Ellipse</b>: %s &#215; %s (constrained to golden ratio 1 : 1.618); with <b>Shift</b> to draw around the starting point"),
                        xs.c_str(), ys.c_str());
            }
        }
    } else {
        this->message_context->setF(Inkscape::IMMEDIATE_MESSAGE, _("<b>Ellipse</b>: %s &#215; %s; with <b>Ctrl</b> to make circle, integer-ratio, or golden-ratio ellipse; with <b>Shift</b> to draw around the starting point"), xs.c_str(), ys.c_str());
    }
}

void ArcTool::finishItem() {
    this->message_context->clear();

    if (this->arc != nullptr) {
        if (this->arc->rx.computed == 0 || this->arc->ry.computed == 0) {
            this->cancel(); // Don't allow the creating of zero sized arc, for example when the start and and point snap to the snap grid point
            return;
        }

        this->arc->updateRepr();
        this->arc->doWriteTransform(this->arc->transform, nullptr, true);

        _desktop->getSelection()->set(this->arc);

        DocumentUndo::done(_desktop->getDocument(), _("Create ellipse"), INKSCAPE_ICON("draw-ellipse"));

        this->arc = nullptr;
    }
}

void ArcTool::cancel() {
    _desktop->getSelection()->clear();
    ungrabCanvasEvents();

    if (this->arc != nullptr) {
        this->arc->deleteObject();
        this->arc = nullptr;
    }

    this->within_tolerance = false;
    this->xp = 0;
    this->yp = 0;
    this->item_to_select = nullptr;

    DocumentUndo::cancel(_desktop->getDocument());
}

}
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
