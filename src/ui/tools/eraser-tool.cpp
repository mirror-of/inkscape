// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Eraser drawing mode
 *
 * Authors:
 *   Mitsuru Oka <oka326@parkcity.ne.jp>
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   bulia byak <buliabyak@users.sf.net>
 *   MenTaLguY <mental@rydia.net>
 *   Jon A. Cruz <jon@joncruz.org>
 *   Abhishek Sharma
 *
 * The original dynadraw code:
 *   Paul Haeberli <paul@sgi.com>
 *
 * Copyright (C) 1998 The Free Software Foundation
 * Copyright (C) 1999-2005 authors
 * Copyright (C) 2001-2002 Ximian, Inc.
 * Copyright (C) 2005-2007 bulia byak
 * Copyright (C) 2006 MenTaLguY
 * Copyright (C) 2008 Jon A. Cruz
 *
 * Released under GNU GPL v2+, read the file 'COPYING' for more information.
 */

#define noERASER_VERBOSE

#include "eraser-tool.h"

#include <string>
#include <cstring>
#include <numeric>

#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>
#include <glibmm/i18n.h>

#include <2geom/bezier-utils.h>
#include <2geom/pathvector.h>

#include "context-fns.h"
#include "desktop-events.h"
#include "desktop-style.h"
#include "desktop.h"
#include "document-undo.h"
#include "document.h"
#include "layer-manager.h"
#include "message-context.h"
#include "path-chemistry.h"
#include "rubberband.h"
#include "selection-chemistry.h"
#include "selection.h"

#include "display/curve.h"
#include "display/control/canvas-item-bpath.h"

#include "include/macros.h"

#include "object/sp-clippath.h"
#include "object/sp-item-group.h"
#include "object/sp-path.h"
#include "object/sp-rect.h"
#include "object/sp-root.h"
#include "object/sp-shape.h"
#include "object/sp-text.h"
#include "object/sp-use.h"

#include "ui/icon-names.h"

#include "svg/svg.h"


using Inkscape::DocumentUndo;

namespace Inkscape {
namespace UI {
namespace Tools {

extern EraserToolMode const DEFAULT_ERASER_MODE = EraserToolMode::CUT;

EraserTool::EraserTool(SPDesktop *desktop)
    : DynamicBase(desktop, "/tools/eraser", "eraser.svg")
{
    accumulated.reset(new SPCurve());
    currentcurve.reset(new SPCurve());

    cal1.reset(new SPCurve());
    cal2.reset(new SPCurve());

    currentshape = new Inkscape::CanvasItemBpath(desktop->getCanvasSketch());
    currentshape->set_stroke(0x0);
    currentshape->set_fill(trace_color_rgba, trace_wind_rule);

    /* fixme: Cannot we cascade it to root more clearly? */
    currentshape->connect_event(sigc::bind(sigc::ptr_fun(sp_desktop_root_handler), desktop));

    sp_event_context_read(this, "mass");
    sp_event_context_read(this, "wiggle");
    sp_event_context_read(this, "angle");
    sp_event_context_read(this, "width");
    sp_event_context_read(this, "thinning");
    sp_event_context_read(this, "tremor");
    sp_event_context_read(this, "flatness");
    sp_event_context_read(this, "tracebackground");
    sp_event_context_read(this, "usepressure");
    sp_event_context_read(this, "usetilt");
    sp_event_context_read(this, "abs_width");
    sp_event_context_read(this, "cap_rounding");

    is_drawing = false;
    //TODO not sure why get 0.01 if slider width == 0, maybe a double/int problem

    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    if (prefs->getBool("/tools/eraser/selcue", false) != 0) {
        enableSelectionCue();
    }
    _updateMode();

    // TODO temp force:
    enableSelectionCue();
}

EraserTool::~EraserTool()
{
    delete currentshape;
    currentshape = nullptr;
}

/**  Reads the current Eraser mode from Preferences and sets `mode` accordingly. */
void EraserTool::_updateMode()
{
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    if (!prefs) {
        return;
    }

    int mode_idx = prefs->getInt("/tools/eraser/mode", 1); // Cut mode is default

    // Note: the integer indices must agree with those in EraserToolbar::_modeAsInt()
    if (mode_idx == 0) {
        mode = EraserToolMode::DELETE;
    } else if (mode_idx == 1) {
        mode = EraserToolMode::CUT;
    } else if (mode_idx == 2) {
        mode = EraserToolMode::CLIP;
    } else {
        g_printerr("Error: invalid mode setting \"%d\" for Eraser tool!", mode_idx);
        mode = DEFAULT_ERASER_MODE;
    }
}

// TODO: After switch to C++20, replace this with std::lerp
inline double flerp(double const f0, double const f1, double const p)
{
    return f0 + (f1 - f0) * p;
}

inline double square(double const x)
{
    return x * x;
}

void EraserTool::_reset(Geom::Point p)
{
    last = cur = getNormalizedPoint(p);
    vel = Geom::Point(0, 0);
    vel_max = 0;
    acc = Geom::Point(0, 0);
    ang = Geom::Point(0, 0);
    del = Geom::Point(0, 0);
}

void EraserTool::_extinput(GdkEvent *event)
{
    if (gdk_event_get_axis(event, GDK_AXIS_PRESSURE, &pressure)) {
        pressure = CLAMP(pressure, min_pressure, max_pressure);
    } else {
        pressure = default_pressure;
    }

    if (gdk_event_get_axis(event, GDK_AXIS_XTILT, &xtilt)) {
        xtilt = CLAMP(xtilt, min_tilt, max_tilt);
    } else {
        xtilt = default_tilt;
    }

    if (gdk_event_get_axis(event, GDK_AXIS_YTILT, &ytilt)) {
        ytilt = CLAMP(ytilt, min_tilt, max_tilt);
    } else {
        ytilt = default_tilt;
    }
}

bool EraserTool::_apply(Geom::Point p)
{
    /* Calculate force and acceleration */
    Geom::Point n = getNormalizedPoint(p);
    Geom::Point force = n - cur;

    // If force is below the absolute threshold `epsilon`,
    // or we haven't yet reached `vel_start` (i.e. at the beginning of stroke)
    // _and_ the force is below the (higher) `epsilon_start` threshold,
    // discard this move.
    // This prevents flips, blobs, and jerks caused by microscopic tremor of the tablet pen,
    // especially bothersome at the start of the stroke where we don't yet have the inertia to
    // smooth them out.
    if (Geom::L2(force) < epsilon || (vel_max < vel_start && Geom::L2(force) < epsilon_start)) {
        return false;
    }

    // Calculate mass
    double const m = flerp(1.0, 160.0, mass);
    acc = force / m;
    vel += acc; // Calculate new velocity
    double const speed = Geom::L2(vel);

    if (speed > vel_max) {
        vel_max = speed;
    } else if (speed < epsilon) {
        return false; // return early if movement is insignificant
    }

    /* Calculate angle of eraser tool */
    double angle_fixed{0.0};
    if (usetilt) {
        // 1a. calculate nib angle from input device tilt:
        Geom::Point normal{ytilt, xtilt};
        if (!Geom::is_zero(normal)) {
            angle_fixed = Geom::atan2(normal);
        }
    } else {
        // 1b. fixed angle (absolutely flat nib):
        angle_fixed = angle * M_PI / 180.0; // convert to radians
    }
    if (flatness < 0.0) {
        // flips direction. Useful when usetilt is true
        // allows simulating both pen/charcoal and broad-nibbed pen
        angle_fixed *= -1;
    }

    // 2. Angle perpendicular to vel (absolutely non-flat nib):
    double angle_dynamic = Geom::atan2(Geom::rot90(vel));
    // flip angle_dynamic to force it to be in the same half-circle as angle_fixed
    bool flipped = false;
    if (fabs(angle_dynamic - angle_fixed) > M_PI_2) {
        angle_dynamic += M_PI;
        flipped = true;
    }
    // normalize angle_dynamic
    if (angle_dynamic > M_PI) {
        angle_dynamic -= 2 * M_PI;
    }
    if (angle_dynamic < -M_PI) {
        angle_dynamic += 2 * M_PI;
    }

    // 3. Average them using flatness parameter:
    // find the flatness-weighted bisector angle, unflip if angle_dynamic was flipped
    // FIXME: when `vel` is oscillating around the fixed angle, the new_ang flips back and forth.
    // How to avoid this?
    double new_ang = flerp(angle_dynamic, angle_fixed, fabs(flatness)) - (flipped ? M_PI : 0);

    // Try to detect a sudden flip when the new angle differs too much from the previous for the
    // current velocity; in that case discard this move
    double angle_delta = Geom::L2(Geom::Point(cos(new_ang), sin(new_ang)) - ang);
    if (angle_delta / speed > 4000) {
        return false;
    }

    // convert to point
    ang = Geom::Point(cos(new_ang), sin(new_ang));

    /* Apply drag */
    double const d = flerp(0.0, 0.5, square(drag));
    vel *= 1.0 - d;

    /* Update position */
    last = cur;
    cur += vel;

    return true;
}

void EraserTool::_brush()
{
    g_assert(npoints >= 0 && npoints < SAMPLING_SIZE);

    // How much velocity thins strokestyle
    double const vel_thinning = flerp(0, 160, vel_thin);

    // Influence of pressure on thickness
    double const pressure_thick = (usepressure ? pressure : 1.0);

    // get the real brush point, not the same as pointer (affected by mass drag)
    Geom::Point brush = getViewPoint(cur);

    double const trace_thick = 1;
    double const speed = Geom::L2(vel);
    double effective_width = (pressure_thick * trace_thick - vel_thinning * speed) * width;

    double tremble_left = 0, tremble_right = 0;
    if (tremor > 0) {
        // obtain two normally distributed random variables, using polar Box-Muller transform
        double y1, y2;
        _generateNormalDist2(y1, y2);

        // deflect both left and right edges randomly and independently, so that:
        // (1) tremor=1 corresponds to sigma=1, decreasing tremor narrows the bell curve;
        // (2) deflection depends on width, but is upped for small widths for better visual uniformity across widths;
        // (3) deflection somewhat depends on speed, to prevent fast strokes looking
        //     comparatively smooth and slow ones excessively jittery
        double const width_coefficient = 0.15 + 0.8 * effective_width;
        double const speed_coefficient = 0.35 + 14 * speed;
        double const total_coefficient = tremor * width_coefficient * speed_coefficient;

        tremble_left  = y1 * total_coefficient;
        tremble_right = y2 * total_coefficient;
    }

    double const min_width = 0.02 * width;
    if (effective_width < min_width) {
        effective_width = min_width;
    }

    double dezoomify_factor = 0.05 * 1000;
    if (!abs_width) {
        dezoomify_factor /= _desktop->current_zoom();
    }

    Geom::Point del_left  = dezoomify_factor * (effective_width + tremble_left)  * ang;
    Geom::Point del_right = dezoomify_factor * (effective_width + tremble_right) * ang;

    point1[npoints] = brush + del_left;
    point2[npoints] = brush - del_right;

    if (nowidth) {
        point1[npoints] = Geom::middle_point(point1[npoints], point2[npoints]);
    }
    del = Geom::middle_point(del_left, del_right);

    npoints++;
}

void EraserTool::_generateNormalDist2(double &r1, double &r2)
{
    // obtain two normally distributed random variables, using polar Box-Muller transform
    double x1, x2, w;
    do {
        x1 = 2.0 * g_random_double_range(0, 1) - 1.0;
        x2 = 2.0 * g_random_double_range(0, 1) - 1.0;
        w = square(x1) + square(x2);
    } while (w >= 1.0);
    w = sqrt(-2.0 * log(w) / w);
    r1 = x1 * w;
    r2 = x2 * w;
}

void EraserTool::_cancel()
{
    dragging = false;
    is_drawing = false;
    ungrabCanvasEvents();

    _removeTemporarySegments();

    /* reset accumulated curve */
    accumulated->reset();
    _clearCurrent();
    repr = nullptr;
}

/** Removes all temporary line segments */
void EraserTool::_removeTemporarySegments()
{
    for (auto segment : segments) {
        delete segment;
    }
    segments.clear();
}

bool EraserTool::root_handler(GdkEvent* event)
{
    bool ret = false;
    _updateMode();
    switch (event->type) {
        case GDK_BUTTON_PRESS:
            if (event->button.button == 1) {
                if (!Inkscape::have_viable_layer(_desktop, defaultMessageContext())) {
                    return true;
                }

                Geom::Point const button_w(event->button.x, event->button.y);
                Geom::Point const button_dt(_desktop->w2d(button_w));

                _reset(button_dt);
                _extinput(event);
                _apply(button_dt);
                accumulated->reset();

                repr = nullptr;

                if (mode == EraserToolMode::DELETE) {
                    auto rubberband = Inkscape::Rubberband::get(_desktop);
                    rubberband->start(_desktop, button_dt);
                    rubberband->setMode(RUBBERBAND_MODE_TOUCHPATH);
                }
                /* initialize first point */
                npoints = 0;

                grabCanvasEvents();
                is_drawing = true;
                ret = true;
            }
            break;

        case GDK_MOTION_NOTIFY: {
            Geom::Point const motion_w(event->motion.x, event->motion.y);
            Geom::Point motion_dt(_desktop->w2d(motion_w));
            _extinput(event);

            message_context->clear();

            if (is_drawing && (event->motion.state & GDK_BUTTON1_MASK)) {
                dragging = true;

                message_context->set(Inkscape::NORMAL_MESSAGE, _("<b>Drawing</b> an eraser stroke"));

                if (!_apply(motion_dt)) {
                    ret = true;
                    break;
                }

                if (cur != last) {
                    _brush();
                    g_assert(npoints > 0);
                    _fitAndSplit(false);
                }

                ret = true;
            }
            if (mode == EraserToolMode::DELETE) {
                accumulated->reset();
                Inkscape::Rubberband::get(_desktop)->move(motion_dt);
            }
            break;
        }
        case GDK_BUTTON_RELEASE: {
            if (event->button.button != 1) {
                break;
            }

            Geom::Point const motion_w(event->button.x, event->button.y);
            Geom::Point const motion_dt(_desktop->w2d(motion_w));

            ungrabCanvasEvents();

            is_drawing = false;

            if (dragging) {
                dragging = false;

                _apply(motion_dt);
                _removeTemporarySegments();

                /* Create object */
                _fitAndSplit(true);
                _accumulate();
                _setToAccumulated(); // performs document_done

                /* reset accumulated curve */
                accumulated->reset();

                _clearCurrent();
                repr = nullptr;

                message_context->clear();
                ret = true;
            }

            if (mode == EraserToolMode::DELETE) {
                auto r = Inkscape::Rubberband::get(_desktop);
                if (r->is_started()) {
                    r->stop();
                }
            }

            break;
        }
        case GDK_KEY_PRESS:
            ret = _handleKeypress(&event->key);
            break;

        case GDK_KEY_RELEASE:
            switch (get_latin_keyval(&event->key)) {
                case GDK_KEY_Control_L:
                case GDK_KEY_Control_R:
                    message_context->clear();
                    break;

                default:
                    break;
            }
            break;

        default:
            break;
    }

    if (!ret) {
        ret = DynamicBase::root_handler(event);
    }
    return ret;
}

/** Analyses and handles a key press event, returns true if processed, false if not. */
bool EraserTool::_handleKeypress(const GdkEventKey *key)
{
    bool ret = false;
    bool just_ctrl = (key->state & GDK_CONTROL_MASK)                      // Ctrl key is down
                     && !(key->state & (GDK_MOD1_MASK | GDK_SHIFT_MASK)); // but not Alt or Shift

    bool just_alt = (key->state & GDK_MOD1_MASK)                            // Alt is down
                    && !(key->state & (GDK_CONTROL_MASK | GDK_SHIFT_MASK)); // but not Ctrl or Shift

    switch (get_latin_keyval(key)) {
        case GDK_KEY_Right:
        case GDK_KEY_KP_Right:
            if (!just_ctrl) {
                width += 0.01;
                if (width > 1.0) {
                    width = 1.0;
                }
                // Alt+X sets focus to this spinbutton as well
                _desktop->setToolboxAdjustmentValue("eraser-width", width * 100);
                ret = true;
            }
            break;

        case GDK_KEY_Left:
        case GDK_KEY_KP_Left:
            if (!just_ctrl) {
                width -= 0.01;
                if (width < 0.01) {
                    width = 0.01;
                }
                _desktop->setToolboxAdjustmentValue("eraser-width", width * 100);
                ret = true;
            }
            break;

        case GDK_KEY_Home:
        case GDK_KEY_KP_Home:
            width = 0.01;
            _desktop->setToolboxAdjustmentValue("eraser-width", width * 100);
            ret = true;
            break;

        case GDK_KEY_End:
        case GDK_KEY_KP_End:
            width = 1.0;
            _desktop->setToolboxAdjustmentValue("eraser-width", width * 100);
            ret = true;
            break;

        case GDK_KEY_x:
        case GDK_KEY_X:
            if (just_alt) {
                _desktop->setToolboxFocusTo("eraser-width");
                ret = true;
            }
            break;

        case GDK_KEY_Escape:
            if (mode == EraserToolMode::DELETE) {
                Inkscape::Rubberband::get(_desktop)->stop();
            }
            if (is_drawing) {
                // if drawing, cancel, otherwise pass it up for deselecting
                _cancel();
                ret = true;
            }
            break;

        case GDK_KEY_z:
        case GDK_KEY_Z:
            if (just_ctrl && is_drawing) { // Ctrl+Z pressed while drawing
                _cancel();
                ret = true;
            } // if not drawing, pass it up for undo
            break;

        default:
            break;
    }
    return ret;
}

void EraserTool::_clearCurrent()
{
    // reset bpath
    currentshape->set_bpath(nullptr);

    // reset curve
    currentcurve->reset();
    cal1->reset();
    cal2->reset();

    // reset points
    npoints = 0;
}

void EraserTool::_setToAccumulated()
{
    bool work_done = false;
    SPDocument *document = _desktop->doc();

    if (!accumulated->is_empty()) {
        if (!repr) {
            /* Create object */
            Inkscape::XML::Document *xml_doc = _desktop->doc()->getReprDoc();
            Inkscape::XML::Node *eraser_repr = xml_doc->createElement("svg:path");

            /* Set style */
            sp_desktop_apply_style_tool(_desktop, eraser_repr, "/tools/eraser", false);

            repr = eraser_repr;
        }
        SPObject *top_layer = _desktop->layerManager().currentRoot();
        SPItem *item_repr = SP_ITEM(top_layer->appendChildRepr(repr));
        Inkscape::GC::release(repr);
        item_repr->updateRepr();
        Geom::PathVector pathv = accumulated->get_pathvector() * _desktop->dt2doc();
        pathv *= item_repr->i2doc_affine().inverse();
        repr->setAttribute("d", sp_svg_write_path(pathv));
        Geom::OptRect eraser_bbox;
        if (repr) {
            bool was_selection = false;
            Inkscape::Selection *selection = _desktop->getSelection();
            _updateMode();
            Inkscape::XML::Document *xml_doc = _desktop->doc()->getReprDoc();
            SPItem *acid = SP_ITEM(_desktop->doc()->getObjectByRepr(repr));
            eraser_bbox = acid->documentVisualBounds();
            std::vector<SPItem *> remaining_items;
            std::vector<SPItem *> to_work_on;
            if (selection->isEmpty()) {
                if (mode == EraserToolMode::CUT || mode == EraserToolMode::CLIP) {
                    to_work_on = document->getItemsPartiallyInBox(_desktop->dkey, *eraser_bbox,
                                                                  false, false, false, true);
                } else {
                    Inkscape::Rubberband *r = Inkscape::Rubberband::get(_desktop);
                    to_work_on = document->getItemsAtPoints(_desktop->dkey, r->getPoints());
                }
                to_work_on.erase(std::remove(to_work_on.begin(), to_work_on.end(), acid), to_work_on.end());
            } else {
                if (mode == EraserToolMode::DELETE) {
                    Inkscape::Rubberband *r = Inkscape::Rubberband::get(_desktop);
                    std::vector<SPItem *> touched = document->getItemsAtPoints(_desktop->dkey, r->getPoints());
                    for (auto i : touched) {
                        if (selection->includes(i)) {
                            to_work_on.push_back(i);
                        }
                    }
                } else {
                    to_work_on.insert(to_work_on.end(), selection->items().begin(), selection->items().end());
                }
                was_selection = true;
            }

            if (!to_work_on.empty()) {
                if (mode == EraserToolMode::CUT) {
                    for (auto i : to_work_on) { // TODO: Move the loop body to a separate function
                        SPItem *item = i;

                        // Unlink a clone before cutting it
                        if (SPUse *use = dynamic_cast<SPUse *>(item)) {
                            item = use->unlink();
                            if (!item) {
                                continue;
                            }
                        }
                        if (!SP_IS_GROUP(item)) {
                            Geom::OptRect bbox = item->documentVisualBounds();
                            if (bbox && bbox->intersects(*eraser_bbox)) {
                                Inkscape::XML::Node *dup = repr->duplicate(xml_doc);
                                repr->parent()->appendChild(dup);
                                Inkscape::GC::release(dup); // parent takes over
                                Inkscape::ObjectSet w_selection(_desktop);
                                w_selection.set(dup);
                                if (!nowidth) {
                                    w_selection.pathUnion(true, true);
                                }
                                w_selection.add(item);
                                if (item->style->fill_rule.value == SP_WIND_RULE_EVENODD) {
                                    SPCSSAttr *css = sp_repr_css_attr_new();
                                    sp_repr_css_set_property(css, "fill-rule", "evenodd");
                                    sp_desktop_set_style(_desktop, css);
                                    sp_repr_css_attr_unref(css);
                                    css = nullptr;
                                }

                                bool success = (nowidth ? w_selection.pathCut(true, true)
                                                        : w_selection.pathDiff(true, true));
                                if (success) {
                                    work_done = true;
                                }

                                Inkscape::Preferences *prefs = Inkscape::Preferences::get();
                                bool break_apart = prefs->getBool("/tools/eraser/break_apart", false);
                                if (!break_apart) {
                                    w_selection.combine(true, true);
                                } else if (!nowidth) {
                                    w_selection.breakApart(true, false, true);
                                }
                                if (!w_selection.isEmpty()) {
                                    // If the item was not completely erased, track the new remainder.
                                    std::vector<SPItem*> nowSel(w_selection.items().begin(), w_selection.items().end());
                                    for (auto i2 : nowSel) {
                                        remaining_items.push_back(i2);
                                    }
                                }
                            } else {
                                remaining_items.push_back(item);
                            }
                        }
                    }
                } else if (mode == EraserToolMode::CLIP) {
                    if (!nowidth) {
                        remaining_items.clear();
                        for (auto item : to_work_on) {
                            _clipErase(item, item_repr->parent, eraser_bbox);
                            if (was_selection) {
                                remaining_items.push_back(item);
                            }
                        }
                        work_done = true;
                    }
                } else if (mode == EraserToolMode::DELETE) {
                    for (auto item : to_work_on) {
                        item->deleteObject(true);
                    }
                    selection->deleteItems();
                    remaining_items.clear();
                    work_done = true;
                }

                selection->clear();
                if (was_selection && !remaining_items.empty()) {
                    selection->add(remaining_items.begin(), remaining_items.end());
                }
            }
            // Remove the eraser stroke itself:
            sp_repr_unparent(repr);
            repr = nullptr;
        }
    } else if (repr) {
        sp_repr_unparent(repr);
        repr = nullptr;
    }

    if (work_done) {
        DocumentUndo::done(document, _("Draw eraser stroke"), INKSCAPE_ICON("draw-eraser"));
    } else {
        DocumentUndo::cancel(document);
    }
}

/** Clips through an item */
void EraserTool::_clipErase(SPItem *item, SPObject *parent, Geom::OptRect &eraser_box)
{
    Inkscape::ObjectSet w_selection(_desktop);
    Geom::OptRect bbox = item->documentVisualBounds();
    Inkscape::XML::Document *xml_doc = _desktop->doc()->getReprDoc();
    Inkscape::XML::Node *dup = repr->duplicate(xml_doc);
    repr->parent()->appendChild(dup);
    Inkscape::GC::release(dup); // parent takes over
    w_selection.set(dup);
    w_selection.pathUnion(true);
    bool delete_old_clip_path = false;
    if (bbox && bbox->intersects(*eraser_box)) {
        SPClipPath *clip_path = item->getClipObject();
        if (clip_path) {
            std::vector<SPItem *> selected;
            selected.push_back(SP_ITEM(clip_path->firstChild()));
            std::vector<Inkscape::XML::Node *> to_select;
            std::vector<SPItem *> items(selected);
            sp_item_list_to_curves(items, selected, to_select);
            Inkscape::XML::Node *clip_data = SP_ITEM(clip_path->firstChild())->getRepr();
            if (!clip_data && !to_select.empty()) {
                clip_data = *(to_select.begin());
            }
            if (clip_data) {
                Inkscape::XML::Node *dup_clip = clip_data->duplicate(xml_doc);
                if (dup_clip) {
                    SPItem *dup_clip_obj = SP_ITEM(parent->appendChildRepr(dup_clip));
                    Inkscape::GC::release(dup_clip);
                    if (dup_clip_obj) {
                        dup_clip_obj->transform *= item->getRelativeTransform(SP_ITEM(parent));
                        dup_clip_obj->updateRepr();
                        delete_old_clip_path = true;
                        w_selection.raiseToTop(true);
                        w_selection.add(dup_clip);
                        w_selection.pathDiff(true, true);
                    }
                }
            }
        } else {
            Inkscape::XML::Node *rect_repr = xml_doc->createElement("svg:rect");
            sp_desktop_apply_style_tool(_desktop, rect_repr, "/tools/eraser", false);
            SPRect *rect = SP_RECT(parent->appendChildRepr(rect_repr));
            Inkscape::GC::release(rect_repr);
            rect->setPosition(bbox->left(), bbox->top(), bbox->width(), bbox->height());
            rect->transform = SP_ITEM(rect->parent)->i2doc_affine().inverse();

            rect->updateRepr();
            rect->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
            w_selection.raiseToTop(true);
            w_selection.add(rect);
            w_selection.pathDiff(true, true);
        }
        w_selection.raiseToTop(true);
        w_selection.add(item);
        w_selection.setMask(true, false, true);
        if (delete_old_clip_path) {
            clip_path->deleteObject(true);
        }
    } else {
        SPItem *erase_clip = w_selection.singleItem();
        if (erase_clip) {
            erase_clip->deleteObject(true);
        }
    }
}

void EraserTool::_addCap(SPCurve &curve, Geom::Point const &pre, Geom::Point const &from, Geom::Point const &to,
                         Geom::Point const &post, double rounding)
{
    Geom::Point vel = rounding * Geom::rot90(to - from) / M_SQRT2;
    double mag = Geom::L2(vel);

    Geom::Point v_in = from - pre;
    double mag_in = Geom::L2(v_in);

    if (mag_in > epsilon) {
        v_in = mag * v_in / mag_in;
    } else {
        v_in = Geom::Point(0, 0);
    }

    Geom::Point v_out = to - post;
    double mag_out = Geom::L2(v_out);

    if (mag_out > epsilon) {
        v_out = mag * v_out / mag_out;
    } else {
        v_out = Geom::Point(0, 0);
    }

    if (Geom::L2(v_in) > epsilon || Geom::L2(v_out) > epsilon) {
        curve.curveto(from + v_in, to + v_out, to);
    }
}

void EraserTool::_accumulate()
{
    // construct a crude outline of the eraser's path.
    // this desperately needs to be rewritten to use the path outliner...
    if (cal1->is_empty() || cal2->is_empty()) {
        return;
    }
    accumulated->reset(); /*  Is this required ?? */
    auto rev_cal2 = cal2->create_reverse();

    g_assert(cal1->get_segment_count() > 0);
    g_assert(rev_cal2->get_segment_count() > 0);
    g_assert(!cal1->first_path()->closed());
    g_assert(!rev_cal2->first_path()->closed());

    Geom::BezierCurve const *dc_cal1_firstseg  = dynamic_cast<Geom::BezierCurve const *>(cal1->first_segment());
    Geom::BezierCurve const *rev_cal2_firstseg = dynamic_cast<Geom::BezierCurve const *>(rev_cal2->first_segment());
    Geom::BezierCurve const *dc_cal1_lastseg   = dynamic_cast<Geom::BezierCurve const *>(cal1->last_segment());
    Geom::BezierCurve const *rev_cal2_lastseg  = dynamic_cast<Geom::BezierCurve const *>(rev_cal2->last_segment());

    g_assert(dc_cal1_firstseg);
    g_assert(rev_cal2_firstseg);
    g_assert(dc_cal1_lastseg);
    g_assert(rev_cal2_lastseg);

    accumulated->append(*cal1);
    if (!nowidth) {
        _addCap(*accumulated,
                dc_cal1_lastseg->finalPoint() - dc_cal1_lastseg->unitTangentAt(1),
                dc_cal1_lastseg->finalPoint(),
                rev_cal2_firstseg->initialPoint(),
                rev_cal2_firstseg->initialPoint() + rev_cal2_firstseg->unitTangentAt(0),
                cap_rounding);

        accumulated->append(*rev_cal2, true);

        _addCap(*accumulated,
                rev_cal2_lastseg->finalPoint() - rev_cal2_lastseg->unitTangentAt(1),
                rev_cal2_lastseg->finalPoint(),
                dc_cal1_firstseg->initialPoint(),
                dc_cal1_firstseg->initialPoint() + dc_cal1_firstseg->unitTangentAt(0),
                cap_rounding);

        accumulated->closepath();
    }
    cal1->reset();
    cal2->reset();
}

void EraserTool::_fitAndSplit(bool releasing)
{
    double const tolerance_sq = square(_desktop->w2d().descrim() * tolerance);
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    nowidth = (prefs->getDouble("/tools/eraser/width", 1) == 0);

#ifdef ERASER_VERBOSE
    g_print("[F&S:R=%c]", releasing ? 'T' : 'F');
#endif
    if (npoints >= SAMPLING_SIZE || npoints <= 0) {
        return; // just clicked
    }

    if (npoints == SAMPLING_SIZE - 1 || releasing) {
        _completeBezier(tolerance_sq, releasing);

#ifdef ERASER_VERBOSE
        g_print("[%d]Yup\n", npoints);
#endif
        if (!releasing) {
            _fitDrawLastPoint();
        }

        // Copy last point
        point1[0] = point1[npoints - 1];
        point2[0] = point2[npoints - 1];
        npoints = 1;
    } else {
        _drawTemporaryBox();
    }
}

void EraserTool::_completeBezier(double tolerance_sq, bool releasing)
{
    /* Current eraser */
    if (cal1->is_empty() || cal2->is_empty()) {
        /* dc->npoints > 0 */
        cal1->reset();
        cal2->reset();

        cal1->moveto(point1[0]);
        cal2->moveto(point2[0]);
    }
#ifdef ERASER_VERBOSE
    g_print("[F&S:#] npoints:%d, releasing:%s\n", npoints, releasing ? "TRUE" : "FALSE");
#endif

    unsigned const bezier_size = 4;
    unsigned const max_beziers = 8;
    size_t const bezier_max_length = bezier_size * max_beziers;

    Geom::Point b1[bezier_max_length];
    gint const nb1 = Geom::bezier_fit_cubic_r(b1, point1, npoints, tolerance_sq, max_beziers);
    g_assert(nb1 * bezier_size <= gint(G_N_ELEMENTS(b1)));

    Geom::Point b2[bezier_max_length];
    gint const nb2 = Geom::bezier_fit_cubic_r(b2, point2, npoints, tolerance_sq, max_beziers);
    g_assert(nb2 * bezier_size <= gint(G_N_ELEMENTS(b2)));

    if (nb1 == -1 || nb2 == -1) {
        _failedBezierFallback(); // TODO: do we ever need this?
        return;
    }

    /* Fit and draw and reset state */
#ifdef ERASER_VERBOSE
    g_print("nb1:%d nb2:%d\n", nb1, nb2);
#endif

    /* CanvasShape */
    if (!releasing) {
        currentcurve->reset();
        currentcurve->moveto(b1[0]);

        for (Geom::Point *bp1 = b1; bp1 < b1 + bezier_size * nb1; bp1 += bezier_size) {
            currentcurve->curveto(bp1[1], bp1[2], bp1[3]);
        }

        currentcurve->lineto(b2[bezier_size * (nb2 - 1) + 3]);

        for (Geom::Point *bp2 = b2 + bezier_size * (nb2 - 1); bp2 >= b2; bp2 -= bezier_size) {
            currentcurve->curveto(bp2[2], bp2[1], bp2[0]);
        }

        // FIXME: segments is always NULL at this point??
        if (segments.empty()) { // first segment
            _addCap(*currentcurve, b2[1], b2[0], b1[0], b1[1], cap_rounding);
        }

        currentcurve->closepath();
        currentshape->set_bpath(currentcurve.get(), true);
    }

    /* Current eraser */
    for (Geom::Point *bp1 = b1; bp1 < b1 + bezier_size * nb1; bp1 += bezier_size) {
        cal1->curveto(bp1[1], bp1[2], bp1[3]);
    }

    for (Geom::Point *bp2 = b2; bp2 < b2 + bezier_size * nb2; bp2 += bezier_size) {
        cal2->curveto(bp2[1], bp2[2], bp2[3]);
    }
}

void EraserTool::_failedBezierFallback()
{
    /* fixme: ??? */
#ifdef ERASER_VERBOSE
    g_print("[_failedBezierFallback] - failed to fit cubic.\n");
#endif
    _drawTemporaryBox();

    for (gint i = 1; i < npoints; i++) {
        cal1->lineto(point1[i]);
    }

    for (gint i = 1; i < npoints; i++) {
        cal2->lineto(point2[i]);
    }
}

void EraserTool::_fitDrawLastPoint()
{
    g_assert(!currentcurve->is_empty());

    guint32 fillColor = sp_desktop_get_color_tool(_desktop, "/tools/eraser", true);
    double opacity = sp_desktop_get_master_opacity_tool(_desktop, "/tools/eraser");
    double fillOpacity = sp_desktop_get_opacity_tool(_desktop, "/tools/eraser", true);

    guint fill = (fillColor & 0xffffff00) | SP_COLOR_F_TO_U(opacity * fillOpacity);

    auto cbp = new Inkscape::CanvasItemBpath(_desktop->getCanvasSketch(), currentcurve.get(), true);
    cbp->set_fill(fill, trace_wind_rule);
    cbp->set_stroke(0x0);

    /* fixme: Cannot we cascade it to root more clearly? */
    cbp->connect_event(sigc::bind(sigc::ptr_fun(sp_desktop_root_handler), _desktop));
    segments.push_back(cbp);

    if (mode == EraserToolMode::DELETE) {
        cbp->hide();
        currentshape->hide();
    }
}

void EraserTool::_drawTemporaryBox()
{
    currentcurve->reset();

    currentcurve->moveto(point1[npoints - 1]);

    for (gint i = npoints - 2; i >= 0; i--) {
        currentcurve->lineto(point1[i]);
    }

    for (gint i = 0; i < npoints; i++) {
        currentcurve->lineto(point2[i]);
    }

    if (npoints >= 2) {
        _addCap(*currentcurve,
                point2[npoints - 2], point2[npoints - 1],
                point1[npoints - 1], point1[npoints - 2], cap_rounding);
    }

    currentcurve->closepath();
    currentshape->set_bpath(currentcurve.get(), true);
}

} // namespace Tools
} // namespace UI
} // namespace Inkscape

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
