// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Main event handling, and related helper functions.
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   Frank Felfe <innerspace@iname.com>
 *   bulia byak <buliabyak@users.sf.net>
 *   Jon A. Cruz <jon@joncruz.org>
 *   Kris De Gussem <Kris.DeGussem@gmail.com>
 *
 * Copyright (C) 1999-2012 authors
 * Copyright (C) 2001-2002 Ximian, Inc.
 *
 * Released under GNU GPL v2+, read the file 'COPYING' for more information.
 */

#include <gdk/gdkkeysyms.h>
#include <gdkmm/display.h>
#include <glibmm/i18n.h>

#include <set>

#include "desktop-events.h"
#include "desktop-style.h"
#include "desktop.h"
#include "file.h"
#include "gradient-drag.h"
#include "layer-manager.h"
#include "message-context.h"
#include "rubberband.h"
#include "selcue.h"
#include "selection.h"

#include "actions/actions-tools.h"

#include "display/control/canvas-item-catchall.h" // Grab/Ungrab
#include "display/control/canvas-item-rotate.h"
#include "display/control/snap-indicator.h"

#include "include/gtkmm_version.h"
#include "include/macros.h"

#include "object/sp-guide.h"

#include "ui/contextmenu.h"
#include "ui/cursor-utils.h"
#include "ui/event-debug.h"
#include "ui/interface.h"
#include "ui/knot/knot.h"
#include "ui/knot/knot-holder.h"
#include "ui/knot/knot-ptr.h"
#include "ui/modifiers.h"
#include "ui/shape-editor.h"
#include "ui/shortcuts.h"

#include "ui/tool/commit-events.h"
#include "ui/tool/control-point.h"
#include "ui/tool/event-utils.h"
#include "ui/tool/shape-record.h"
#include "ui/tools/calligraphic-tool.h"
#include "ui/tools/dropper-tool.h"
#include "ui/tools/lpe-tool.h"
#include "ui/tools/node-tool.h"
#include "ui/tools/select-tool.h"
#include "ui/tools/tool-base.h"
#include "ui/widget/canvas.h"

#include "widgets/desktop-widget.h"

#include "xml/node-event-vector.h"

// globals for temporary switching to selector by space
static bool selector_toggled = FALSE;
static Glib::ustring switch_selector_to;

// globals for temporary switching to dropper by 'D'
static bool dropper_toggled = FALSE;
static Glib::ustring switch_dropper_to;

// globals for keeping track of keyboard scroll events in order to accelerate
static guint32 scroll_event_time = 0;
static gdouble scroll_multiply = 1;
static guint scroll_keyval = 0;

// globals for key processing
static bool latin_keys_group_valid = FALSE;
static gint latin_keys_group;
static std::set<int> latin_keys_groups;

namespace Inkscape {
namespace UI {
namespace Tools {

static void set_event_location(SPDesktop * desktop, GdkEvent * event);

ToolBase::ToolBase(SPDesktop *desktop, std::string prefs_path, std::string cursor_filename, bool uses_snap)
    : _desktop(desktop)
    , _prefs_path(std::move(prefs_path))
    , _cursor_default(std::move(cursor_filename))
    , _cursor_filename("none")
    , _uses_snap(uses_snap)
{
    pref_observer = new ToolPrefObserver(_prefs_path, this);
    Inkscape::Preferences::get()->addObserver(*(this->pref_observer));
    this->set_cursor(_cursor_default);
    _desktop->getCanvas()->grab_focus();

    message_context = std::unique_ptr<Inkscape::MessageContext>(new Inkscape::MessageContext(desktop->messageStack()));

    // Make sure no delayed snapping events are carried over after switching tools
    // (this is only an additional safety measure against sloppy coding, because each
    // tool should take care of this by itself)
    discard_delayed_snap_event();
}

ToolBase::~ToolBase() {
    this->enableSelectionCue(false);

    if (this->pref_observer) {
        delete this->pref_observer;
    }

    if (this->_delayed_snap_event) {
        delete this->_delayed_snap_event;
    }
}

/**
 * Called by our pref_observer if a preference has been changed.
 */
void ToolBase::set(const Inkscape::Preferences::Entry& /*val*/) {
}


SPGroup *ToolBase::currentLayer() const
{
    return _desktop->layerManager().currentLayer();
}

/**
 * Sets the current cursor to the given filename. Does not readload if not changed.
 */
void ToolBase::set_cursor(std::string filename)
{
    if (filename != _cursor_filename) {
        _cursor_filename = filename;
        use_tool_cursor();
    }
}

/**
 * Returns the Gdk Cursor for the given filename
 *
 * WARNING: currently this changes the window cursor, see load_svg_cursor
 */
Glib::RefPtr<Gdk::Cursor> ToolBase::get_cursor(Glib::RefPtr<Gdk::Window> window, std::string filename)
{
    bool fillHasColor   = false;
    bool strokeHasColor = false;
    guint32 fillColor = sp_desktop_get_color_tool(_desktop, getPrefsPath(), true, &fillHasColor);
    guint32 strokeColor = sp_desktop_get_color_tool(_desktop, getPrefsPath(), false, &strokeHasColor);
    double fillOpacity = fillHasColor ? sp_desktop_get_opacity_tool(_desktop, getPrefsPath(), true) : 1.0;
    double strokeOpacity = strokeHasColor ? sp_desktop_get_opacity_tool(_desktop, getPrefsPath(), false) : 1.0;

    return load_svg_cursor(window->get_display(), window, filename,
                           fillColor, strokeColor, fillOpacity, strokeOpacity);
}

/**
 * Uses the saved cursor, based on the saved filename.
 */
void ToolBase::use_tool_cursor() {
    if (auto window = _desktop->getCanvas()->get_window()) {
        _cursor = get_cursor(window, _cursor_filename);
        window->set_cursor(_cursor);
    }
    _desktop->waiting_cursor = false;
}

/**
 * Set the cursor to this specific one, don't remember it.
 *
 * If RefPtr is empty, sets the remembered cursor (reverting it)
 */
void ToolBase::use_cursor(Glib::RefPtr<Gdk::Cursor> cursor)
{
    if (auto window = _desktop->getCanvas()->get_window()) {
        window->set_cursor(cursor ? cursor : _cursor);
    }
}

/**
 * Toggles current tool between active tool and selector tool.
 * Subroutine of sp_event_context_private_root_handler().
 */
static void sp_toggle_selector(SPDesktop *dt) {

    if (!dt->event_context) {
        return;
    }

    if (dynamic_cast<Inkscape::UI::Tools::SelectTool *>(dt->event_context)) {
        if (selector_toggled) {
            set_active_tool(dt, switch_selector_to);
            selector_toggled = false;
        }
    } else {
        selector_toggled = TRUE;
        switch_selector_to = get_active_tool(dt);
        set_active_tool(dt, "Select");
    }
}

/**
 * Toggles current tool between active tool and dropper tool.
 * Subroutine of sp_event_context_private_root_handler().
 */
void sp_toggle_dropper(SPDesktop *dt) {

    if (!dt->event_context) {
        return;
    }

    if (dynamic_cast<Inkscape::UI::Tools::DropperTool *>(dt->event_context)) {
        if (dropper_toggled) {
            set_active_tool(dt, switch_dropper_to);
            dropper_toggled = FALSE;
        }
    } else {
        dropper_toggled = TRUE;
        switch_dropper_to = get_active_tool(dt);
        set_active_tool(dt, "Dropper");
    }
}

/**
 * Calculates and keeps track of scroll acceleration.
 * Subroutine of sp_event_context_private_root_handler().
 */
static gdouble accelerate_scroll(GdkEvent *event, gdouble acceleration)
{
    guint32 time_diff = ((GdkEventKey *) event)->time - scroll_event_time;

    /* key pressed within 500ms ? (1/2 second) */
    if (time_diff > 500 || event->key.keyval != scroll_keyval) {
        scroll_multiply = 1; // abort acceleration
    } else {
        scroll_multiply += acceleration; // continue acceleration
    }

    scroll_event_time = ((GdkEventKey *) event)->time;
    scroll_keyval = event->key.keyval;

    return scroll_multiply;
}

/** Moves the selected points along the supplied unit vector according to
 * the modifier state of the supplied event. */
bool ToolBase::_keyboardMove(GdkEventKey const &event, Geom::Point const &dir)
{
    if (held_control(event)) return false;
    unsigned num = 1 + gobble_key_events(shortcut_key(event), 0);
    Geom::Point delta = dir * num;

    if (held_shift(event)) {
        delta *= 10;
    }

    if (held_alt(event)) {
        delta /= _desktop->current_zoom();
    } else {
        Inkscape::Preferences *prefs = Inkscape::Preferences::get();
        double nudge = prefs->getDoubleLimited("/options/nudgedistance/value", 2, 0, 1000, "px");
        delta *= nudge;
    }

    bool moved = false;
    if (shape_editor && shape_editor->has_knotholder()) {
        KnotHolder * knotholder = shape_editor->knotholder;
        if (knotholder && knotholder->knot_selected()) {
            knotholder->transform_selected(Geom::Translate(delta));
            moved = true;
        }
    } else {
        auto nt = dynamic_cast<Inkscape::UI::Tools::NodeTool *>(_desktop->event_context);
        if (nt) {
            for (auto &_shape_editor : nt->_shape_editors) {
                ShapeEditor *shape_editor = _shape_editor.second.get();
                if (shape_editor && shape_editor->has_knotholder()) {
                    KnotHolder * knotholder = shape_editor->knotholder;
                    if (knotholder && knotholder->knot_selected()) {
                        knotholder->transform_selected(Geom::Translate(delta));
                        moved = true;
                    }
                }
            }
        }
    }

    return moved;
}


bool ToolBase::root_handler(GdkEvent* event) {

#ifdef EVENT_DUMP
    ui_dump_event (event, "ToolBase::root_handler");
#endif

    static Geom::Point button_w;
    static unsigned int panning_cursor = 0;
    static unsigned int zoom_rb = 0;

    Inkscape::Preferences *prefs = Inkscape::Preferences::get();

    /// @todo REmove redundant /value in preference keys
    tolerance = prefs->getIntLimited("/options/dragtolerance/value", 0, 0, 100);
    bool allow_panning = prefs->getBool("/options/spacebarpans/value");
    gint ret = FALSE;

    switch (event->type) {
    case GDK_2BUTTON_PRESS:
        if (panning) {
            panning = PANNING_NONE;
            ungrabCanvasEvents();
            ret = TRUE;
        } else {
            /* sp_desktop_dialog(); */
        }
        break;

    case GDK_BUTTON_PRESS:
        // save drag origin
        xp = (gint) event->button.x;
        yp = (gint) event->button.y;
        within_tolerance = true;

        button_w = Geom::Point(event->button.x, event->button.y);

        switch (event->button.button) {
        case 1:
            // TODO Does this make sense? Panning starts on passive mouse motion while space
            // bar is pressed, it's not necessary to press the mouse button.
            if (this->is_space_panning()) {
                // When starting panning, make sure there are no snap events pending because these might disable the panning again
                if (_uses_snap) {
                    this->discard_delayed_snap_event();
                }
                panning = PANNING_SPACE_BUTTON1;

                grabCanvasEvents(Gdk::KEY_RELEASE_MASK    |
                                 Gdk::BUTTON_RELEASE_MASK |
                                 Gdk::POINTER_MOTION_MASK );

                ret = TRUE;
            }
            break;

        case 2:
            if ((event->button.state & GDK_CONTROL_MASK) && !_desktop->get_rotation_lock()) {
                // On screen canvas rotation preview

                // Grab background before doing anything else
                _desktop->getCanvasRotate()->start(_desktop);
                _desktop->getCanvasRotate()->show();

                // CanvasItemRotate code takes over!
                ungrabCanvasEvents();

                _desktop->getCanvasRotate()->grab(Gdk::KEY_PRESS_MASK      |
                                                 Gdk::KEY_RELEASE_MASK    |
                                                 Gdk::BUTTON_RELEASE_MASK |
                                                 Gdk::POINTER_MOTION_MASK,
                                                 nullptr);  // Cursor is null.

            } else if (event->button.state & GDK_SHIFT_MASK) {
                zoom_rb = 2;
            } else {
                // When starting panning, make sure there are no snap events pending because these might disable the panning again
                if (_uses_snap) {
                    this->discard_delayed_snap_event();
                }
                panning = PANNING_BUTTON2;

                grabCanvasEvents(Gdk::BUTTON_RELEASE_MASK |
                                 Gdk::POINTER_MOTION_MASK );
            }

            ret = TRUE;
            break;

        case 3:
            if (event->button.state & (GDK_SHIFT_MASK | GDK_CONTROL_MASK)) {
                // When starting panning, make sure there are no snap events pending because these might disable the panning again
                if (_uses_snap) {
                    this->discard_delayed_snap_event();
                }
                panning = PANNING_BUTTON3;

                grabCanvasEvents(Gdk::BUTTON_RELEASE_MASK |
                                 Gdk::POINTER_MOTION_MASK );
                ret = TRUE;
            } else if (!are_buttons_1_and_3_on(event)) {
                sp_event_root_menu_popup(_desktop, nullptr, event);
                ret = TRUE;
            }
            break;

        default:
            break;
        }
        break;

    case GDK_MOTION_NOTIFY:
        if (panning) {
            if (panning == 4 && !xp && !yp ) {
                // <Space> + mouse panning started, save location and grab canvas
                xp = event->motion.x;
                yp = event->motion.y;
                button_w = Geom::Point(event->motion.x, event->motion.y);

                grabCanvasEvents(Gdk::KEY_RELEASE_MASK    |
                                 Gdk::BUTTON_RELEASE_MASK |
                                 Gdk::POINTER_MOTION_MASK );
            }

            if ((panning == 2 && !(event->motion.state & GDK_BUTTON2_MASK))
                    || (panning == 1 && !(event->motion.state & GDK_BUTTON1_MASK))
                    || (panning == 3 && !(event->motion.state & GDK_BUTTON3_MASK))) {
                /* Gdk seems to lose button release for us sometimes :-( */
                panning = PANNING_NONE;
                ungrabCanvasEvents();
                ret = TRUE;
            } else {
                // To fix https://bugs.launchpad.net/inkscape/+bug/1458200
                // we increase the tolerance because no sensible data for panning
                if (within_tolerance && (abs((gint) event->motion.x - xp)
                        < tolerance * 3) && (abs((gint) event->motion.y - yp)
                        < tolerance * 3)) {
                    // do not drag if we're within tolerance from origin
                    break;
                }

                // Once the user has moved farther than tolerance from
                // the original location (indicating they intend to move
                // the object, not click), then always process the motion
                // notify coordinates as given (no snapping back to origin)
                within_tolerance = false;

                // gobble subsequent motion events to prevent "sticking"
                // when scrolling is slow
                gobble_motion_events(panning == 2 ? GDK_BUTTON2_MASK : (panning
                        == 1 ? GDK_BUTTON1_MASK : GDK_BUTTON3_MASK));

                if (panning_cursor == 0) {
                    panning_cursor = 1;
                    auto display = _desktop->getCanvas()->get_display();
                    auto window = _desktop->getCanvas()->get_window();
                    auto cursor = Gdk::Cursor::create(display, "move");
                    window->set_cursor(cursor);
                }

                Geom::Point const motion_w(event->motion.x, event->motion.y);
                Geom::Point const moved_w(motion_w - button_w);
                _desktop->scroll_relative(moved_w, true); // we're still scrolling, do not redraw
                ret = TRUE;
            }
        } else if (zoom_rb) {
            if (within_tolerance && (abs((gint) event->motion.x - xp)
                    < tolerance) && (abs((gint) event->motion.y - yp)
                    < tolerance)) {
                break; // do not drag if we're within tolerance from origin
            }

            // Once the user has moved farther than tolerance from the original location
            // (indicating they intend to move the object, not click), then always process the
            // motion notify coordinates as given (no snapping back to origin)
            within_tolerance = false;

            if (Inkscape::Rubberband::get(_desktop)->is_started()) {
                Geom::Point const motion_w(event->motion.x, event->motion.y);
                Geom::Point const motion_dt(_desktop->w2d(motion_w));

                Inkscape::Rubberband::get(_desktop)->move(motion_dt);
            } else {
                // Start the box where the mouse was clicked, not where it is now
                // because otherwise our box would be offset by the amount of tolerance.
                Geom::Point const motion_w(xp, yp);
                Geom::Point const motion_dt(_desktop->w2d(motion_w));

                Inkscape::Rubberband::get(_desktop)->start(_desktop, motion_dt);
            }

            if (zoom_rb == 2) {
                gobble_motion_events(GDK_BUTTON2_MASK);
            }
        }
        break;

    case GDK_BUTTON_RELEASE: {
        bool middle_mouse_zoom = prefs->getBool("/options/middlemousezoom/value");

        xp = yp = 0;

        if (panning_cursor == 1) {
            panning_cursor = 0;
            _desktop->getCanvas()->get_window()->set_cursor(_cursor);
        }

        if (middle_mouse_zoom && within_tolerance && (panning || zoom_rb)) {
            zoom_rb = 0;

            if (panning) {
                panning = PANNING_NONE;
                ungrabCanvasEvents();
            }

            Geom::Point const event_w(event->button.x, event->button.y);
            Geom::Point const event_dt(_desktop->w2d(event_w));

            double const zoom_inc = prefs->getDoubleLimited(
                    "/options/zoomincrement/value", M_SQRT2, 1.01, 10);

            _desktop->zoom_relative(event_dt, (event->button.state & GDK_SHIFT_MASK) ? 1 / zoom_inc : zoom_inc);
            ret = TRUE;
        } else if (panning == event->button.button) {
            panning = PANNING_NONE;
            ungrabCanvasEvents();

            // in slow complex drawings, some of the motion events are lost;
            // to make up for this, we scroll it once again to the button-up event coordinates
            // (i.e. canvas will always get scrolled all the way to the mouse release point,
            // even if few intermediate steps were visible)
            Geom::Point const motion_w(event->button.x, event->button.y);
            Geom::Point const moved_w(motion_w - button_w);

            _desktop->scroll_relative(moved_w);
            ret = TRUE;
        } else if (zoom_rb == event->button.button) {
            zoom_rb = 0;

            Geom::OptRect const b = Inkscape::Rubberband::get(_desktop)->getRectangle();
            Inkscape::Rubberband::get(_desktop)->stop();

            if (b && !within_tolerance) {
                _desktop->set_display_area(*b, 10);
            }

            ret = TRUE;
        }
        }
        break;

    case GDK_KEY_PRESS: {
        double const acceleration = prefs->getDoubleLimited(
                "/options/scrollingacceleration/value", 0, 0, 6);
        int const key_scroll = prefs->getIntLimited("/options/keyscroll/value",
                10, 0, 1000);

        switch (get_latin_keyval(&event->key)) {
        // GDK insists on stealing these keys (F1 for no idea what, tab for cycling widgets
        // in the editing window). So we resteal them back and run our regular shortcut
        // invoker on them.
        case GDK_KEY_Tab:
        case GDK_KEY_ISO_Left_Tab:
        case GDK_KEY_F1:
            ret = Inkscape::Shortcuts::getInstance().invoke_action(&event->key);
            break;

        case GDK_KEY_Q:
        case GDK_KEY_q:
            if (_desktop->quick_zoomed()) {
                ret = TRUE;
            }
            if (!MOD__SHIFT(event) && !MOD__CTRL(event) && !MOD__ALT(event)) {
                _desktop->zoom_quick(true);
                ret = TRUE;
            }
            break;

        case GDK_KEY_W:
        case GDK_KEY_w:
        case GDK_KEY_F4:
            /* Close view */
            if (MOD__CTRL_ONLY(event)) {
                sp_ui_close_view(nullptr);
                ret = TRUE;
            }
            break;

        case GDK_KEY_Left: // Ctrl Left
        case GDK_KEY_KP_Left:
        case GDK_KEY_KP_4:
            if (MOD__CTRL_ONLY(event)) {
                int i = (int) floor(key_scroll * accelerate_scroll(event, acceleration));

                gobble_key_events(get_latin_keyval(&event->key), GDK_CONTROL_MASK);
                _desktop->scroll_relative(Geom::Point(i, 0));
            } else if (!_keyboardMove(event->key, Geom::Point(-1, 0))) {
                Inkscape::Shortcuts::getInstance().invoke_action(&event->key);
            }
            ret = true;
            break;

        case GDK_KEY_Up: // Ctrl Up
        case GDK_KEY_KP_Up:
        case GDK_KEY_KP_8:
            if (MOD__CTRL_ONLY(event)) {
                int i = (int) floor(key_scroll * accelerate_scroll(event, acceleration));

                gobble_key_events(get_latin_keyval(&event->key), GDK_CONTROL_MASK);
                _desktop->scroll_relative(Geom::Point(0, i));
            } else if (!_keyboardMove(event->key, Geom::Point(0, -_desktop->yaxisdir()))) {
                Inkscape::Shortcuts::getInstance().invoke_action(&event->key);
            }
            ret = true;
            break;

        case GDK_KEY_Right: // Ctrl Right
        case GDK_KEY_KP_Right:
        case GDK_KEY_KP_6:
            if (MOD__CTRL_ONLY(event)) {
                int i = (int) floor(key_scroll * accelerate_scroll(event, acceleration));

                gobble_key_events(get_latin_keyval(&event->key), GDK_CONTROL_MASK);
                _desktop->scroll_relative(Geom::Point(-i, 0));
            } else if (!_keyboardMove(event->key, Geom::Point(1, 0))) {
                Inkscape::Shortcuts::getInstance().invoke_action(&event->key);
            }
            ret = true;
            break;

        case GDK_KEY_Down: // Ctrl Down
        case GDK_KEY_KP_Down:
        case GDK_KEY_KP_2:
            if (MOD__CTRL_ONLY(event)) {
                int i = (int) floor(key_scroll * accelerate_scroll(event, acceleration));

                gobble_key_events(get_latin_keyval(&event->key), GDK_CONTROL_MASK);
                _desktop->scroll_relative(Geom::Point(0, -i));
            } else if (!_keyboardMove(event->key, Geom::Point(0, _desktop->yaxisdir()))) {
                Inkscape::Shortcuts::getInstance().invoke_action(&event->key);
            }
            ret = true;
            break;

        case GDK_KEY_Menu:
            sp_event_root_menu_popup(_desktop, nullptr, event);
            ret = TRUE;
            break;

        case GDK_KEY_F10:
            if (MOD__SHIFT_ONLY(event)) {
                sp_event_root_menu_popup(_desktop, nullptr, event);
                ret = TRUE;
            }
            break;

        case GDK_KEY_space:
            within_tolerance = true;
            xp = yp = 0;
            if (!allow_panning) break;
            panning = PANNING_SPACE;
            this->message_context->set(Inkscape::INFORMATION_MESSAGE,
                    _("<b>Space+mouse move</b> to pan canvas"));

            ret = TRUE;
            break;

        case GDK_KEY_z:
        case GDK_KEY_Z:
            if (MOD__ALT_ONLY(event)) {
                _desktop->zoom_grab_focus();
                ret = TRUE;
            }
            break;

        default:
            break;
            }
        }
        break;

    case GDK_KEY_RELEASE:
        // Stop panning on any key release
        if (this->is_space_panning()) {
            this->message_context->clear();
        }

        if (panning) {
            panning = PANNING_NONE;
            xp = yp = 0;

            ungrabCanvasEvents();
        }

        if (panning_cursor == 1) {
            panning_cursor = 0;
            _desktop->getCanvas()->get_window()->set_cursor(_cursor);
        }

        switch (get_latin_keyval(&event->key)) {
        case GDK_KEY_space:
            if (within_tolerance) {
                // Space was pressed, but not panned
                sp_toggle_selector(_desktop);

                // Be careful, sp_toggle_selector will delete ourselves.
                // Thus, make sure we return immediately.
                return true;
            }

            break;

        case GDK_KEY_Q:
        case GDK_KEY_q:
            if (_desktop->quick_zoomed()) {
                _desktop->zoom_quick(false);
                ret = TRUE;
            }
            break;

        default:
            break;
        }
        break;

    case GDK_SCROLL: {
        int constexpr WHEEL_SCROLL_DEFAULT = 40;
        
        // previously we did two wheel_scrolls for each mouse scroll
        int const wheel_scroll = prefs->getIntLimited(
                "/options/wheelscroll/value", WHEEL_SCROLL_DEFAULT, 0, 1000) * 2;

        // Size of smooth-scrolls (only used in GTK+ 3)
        gdouble delta_x = 0;
        gdouble delta_y = 0;

        using Modifiers::Type;
        using Modifiers::Triggers;
        Type action = Modifiers::Modifier::which(Triggers::CANVAS | Triggers::SCROLL, event->scroll.state);

        if (action == Type::CANVAS_ROTATE && !_desktop->get_rotation_lock()) {
            double rotate_inc = prefs->getDoubleLimited(
                    "/options/rotateincrement/value", 15, 1, 90, "°" );
            rotate_inc *= M_PI/180.0;

            switch (event->scroll.direction) {
            case GDK_SCROLL_UP:
                // Do nothing
                break;

            case GDK_SCROLL_DOWN:
                rotate_inc = -rotate_inc;
                break;

            case GDK_SCROLL_SMOOTH: {
                gdk_event_get_scroll_deltas(event, &delta_x, &delta_y);
#ifdef GDK_WINDOWING_QUARTZ
                // MacBook trackpad scroll event gives pixel delta
                delta_y /= WHEEL_SCROLL_DEFAULT;
#endif
                double delta_y_clamped = CLAMP(delta_y, -1.0, 1.0); // values > 1 result in excessive rotating
                rotate_inc = rotate_inc * -delta_y_clamped;
                break;
            }

            default:
                rotate_inc = 0.0;
                break;
            }

            if (rotate_inc != 0.0) {
                Geom::Point const scroll_dt = _desktop->point();
                _desktop->rotate_relative_keep_point(scroll_dt, rotate_inc);
                ret = TRUE;
            }

        } else if (action == Type::CANVAS_PAN_X) {
           /* shift + wheel, pan left--right */

            switch (event->scroll.direction) {
            case GDK_SCROLL_UP:
            case GDK_SCROLL_LEFT:
                _desktop->scroll_relative(Geom::Point(wheel_scroll, 0));
                ret = TRUE;
                break;

            case GDK_SCROLL_DOWN:
            case GDK_SCROLL_RIGHT:
                _desktop->scroll_relative(Geom::Point(-wheel_scroll, 0));
                ret = TRUE;
                break;

            case GDK_SCROLL_SMOOTH: {
                gdk_event_get_scroll_deltas(event, &delta_x, &delta_y);
#ifdef GDK_WINDOWING_QUARTZ
                // MacBook trackpad scroll event gives pixel delta
                delta_y /= WHEEL_SCROLL_DEFAULT;
#endif
                _desktop->scroll_relative(Geom::Point(wheel_scroll * -delta_y, 0));
                ret = TRUE;
                break;
            }

            default:
                break;
            }

        } else if (action == Type::CANVAS_ZOOM) {
            /* ctrl + wheel, zoom in--out */
            double rel_zoom;
            double const zoom_inc = prefs->getDoubleLimited(
                    "/options/zoomincrement/value", M_SQRT2, 1.01, 10);

            switch (event->scroll.direction) {
            case GDK_SCROLL_UP:
                rel_zoom = zoom_inc;
                break;

            case GDK_SCROLL_DOWN:
                rel_zoom = 1 / zoom_inc;
                break;

            case GDK_SCROLL_SMOOTH: {
                gdk_event_get_scroll_deltas(event, &delta_x, &delta_y);
#ifdef GDK_WINDOWING_QUARTZ
                // MacBook trackpad scroll event gives pixel delta
                delta_y /= WHEEL_SCROLL_DEFAULT;
#endif
                double delta_y_clamped = CLAMP(std::abs(delta_y), 0.0, 1.0); // values > 1 result in excessive zooming
                double zoom_inc_scaled = (zoom_inc-1) * delta_y_clamped + 1;
                if (delta_y < 0) {
                    rel_zoom = zoom_inc_scaled;
                } else {
                    rel_zoom = 1 / zoom_inc_scaled;
                }
                break;
            }

            default:
                rel_zoom = 0.0;
                break;
            }

            if (rel_zoom != 0.0) {
                Geom::Point const scroll_dt = _desktop->point();
                _desktop->zoom_relative(scroll_dt, rel_zoom);
                ret = TRUE;
            }

            /* no modifier, pan up--down (left--right on multiwheel mice?) */
        } else if (action == Type::CANVAS_PAN_Y) {
            switch (event->scroll.direction) {
            case GDK_SCROLL_UP:
                _desktop->scroll_relative(Geom::Point(0, wheel_scroll));
                break;

            case GDK_SCROLL_DOWN:
                _desktop->scroll_relative(Geom::Point(0, -wheel_scroll));
                break;

            case GDK_SCROLL_LEFT:
                _desktop->scroll_relative(Geom::Point(wheel_scroll, 0));
                break;

            case GDK_SCROLL_RIGHT:
                _desktop->scroll_relative(Geom::Point(-wheel_scroll, 0));
                break;

            case GDK_SCROLL_SMOOTH:
                gdk_event_get_scroll_deltas(event, &delta_x, &delta_y);
#ifdef GDK_WINDOWING_QUARTZ
                // MacBook trackpad scroll event gives pixel delta
                delta_x /= WHEEL_SCROLL_DEFAULT;
                delta_y /= WHEEL_SCROLL_DEFAULT;
#endif
                _desktop->scroll_relative(Geom::Point(-wheel_scroll * delta_x, -wheel_scroll * delta_y));
                break;
            }
            ret = TRUE;
        } else {
            g_warning("unhandled scroll event with scroll.state=0x%x", event->scroll.state);
        }
        break;
    }

    default:
        break;
    }

    return ret;
}

/**
 * This function allows to handle global tool events if _pre function is not fully overridden.
 */

void ToolBase::set_on_buttons(GdkEvent *event)
{
    switch (event->type) {
        case GDK_BUTTON_PRESS:
            switch (event->button.button) {
                case 1:
                    this->_button1on = true;
                    break;
                case 2:
                    this->_button2on = true;
                    break;
                case 3:
                    this->_button3on = true;
                    break;
            }
            break;
        case GDK_BUTTON_RELEASE:
            switch (event->button.button) {
                case 1:
                    this->_button1on = false;
                    break;
                case 2:
                    this->_button2on = false;
                    break;
                case 3:
                    this->_button3on = false;
                    break;
            }
            break;
        case GDK_MOTION_NOTIFY:
            if (event->motion.state & Gdk::ModifierType::BUTTON1_MASK) {
                this->_button1on = true;
            } else {
                this->_button1on = false;
            }
            if (event->motion.state & Gdk::ModifierType::BUTTON2_MASK) {
                this->_button2on = true;
            } else {
                this->_button2on = false;
            }
            if (event->motion.state & Gdk::ModifierType::BUTTON3_MASK) {
                this->_button3on = true;
            } else {
                this->_button3on = false;
            }
    }
}

bool ToolBase::are_buttons_1_and_3_on() const
{
    return this->_button1on && this->_button3on;
}

bool ToolBase::are_buttons_1_and_3_on(GdkEvent* event)
{
    set_on_buttons(event);
    return are_buttons_1_and_3_on();
}

/**
 * Handles item specific events. Gets called from Gdk.
 *
 * Only reacts to right mouse button at the moment.
 * \todo Fixme: do context sensitive popup menu on items.
 */
bool ToolBase::item_handler(SPItem* item, GdkEvent* event) {
    int ret = FALSE;

    switch (event->type) {
    case GDK_BUTTON_PRESS:
        if (!are_buttons_1_and_3_on(event) && event->button.button == 3 &&
            !((event->button.state & GDK_SHIFT_MASK) || (event->button.state & GDK_CONTROL_MASK))) {
            sp_event_root_menu_popup(_desktop, item, event);
            ret = TRUE;
        }
        break;

    default:
        break;
    }

    return ret;
}

/**
 * Returns true if we're hovering above a knot (needed because we don't want to pre-snap in that case).
 */
bool ToolBase::sp_event_context_knot_mouseover() const {
    if (this->shape_editor) {
        return this->shape_editor->knot_mouseover();
    }

    return false;
}

/**
 * Enables/disables the ToolBase's SelCue.
 */
void ToolBase::enableSelectionCue(bool enable) {
    if (enable) {
        if (!_selcue) {
            _selcue = new Inkscape::SelCue(_desktop);
        }
    } else {
        delete _selcue;
        _selcue = nullptr;
    }
}

/*
 * Enables/disables the ToolBase's GrDrag.
 */
void ToolBase::enableGrDrag(bool enable) {
    if (enable) {
        if (!_grdrag) {
            _grdrag = new GrDrag(_desktop);
        }
    } else {
        if (_grdrag) {
            delete _grdrag;
            _grdrag = nullptr;
        }
    }
}

/**
 * Delete a selected GrDrag point
 */
bool ToolBase::deleteSelectedDrag(bool just_one)
{
    if (_grdrag && !_grdrag->selected.empty()) {
        _grdrag->deleteSelected(just_one);
        return true;
    }
    return false;
}

/**
 * Return true if there is a gradient drag.
 */
bool ToolBase::hasGradientDrag() const
{
    return _grdrag && _grdrag->isNonEmpty();
}

/**
 * Grab events from the Canvas Catchall. (Common configuration.)
 */
void ToolBase::grabCanvasEvents(Gdk::EventMask mask)
{
    _desktop->getCanvasCatchall()->grab(mask, nullptr); // Cursor is null.
}

/**
 * Ungrab events from the Canvas Catchall. (Common configuration.)
 */
void ToolBase::ungrabCanvasEvents()
{
    _desktop->snapindicator->remove_snaptarget();
    _desktop->getCanvasCatchall()->ungrab();
}

/** Enable (or disable) high precision for motion events
  *
  * This is intended to be used by drawing tools, that need to process motion events with high accuracy
  * and high update rate (for example free hand tools)
  *
  * With standard accuracy some intermediate motion events might be discarded
  *
  * Call this function when an operation that requires high accuracy is started (e.g. mouse button is pressed
  * to draw a line). Make sure to call it again and restore standard precision afterwards. **/
void ToolBase::set_high_motion_precision(bool high_precision) {
    Glib::RefPtr<Gdk::Window> window = _desktop->getToplevel()->get_window();

    if (window) {
        window->set_event_compression(!high_precision);
    }
}

Geom::Point ToolBase::setup_for_drag_start(GdkEvent *ev)
{
    this->xp = static_cast<gint>(ev->button.x);
    this->yp = static_cast<gint>(ev->button.y);
    this->within_tolerance = true;

    Geom::Point const p(ev->button.x, ev->button.y);
    item_to_select = Inkscape::UI::Tools::sp_event_context_find_item(_desktop, p, ev->button.state & GDK_MOD1_MASK, TRUE);
    return _desktop->w2d(p);
}

/**
 * Discard and count matching key events from top of event bucket.
 * Convenience function that just passes request to canvas.
 */
int ToolBase::gobble_key_events(guint keyval, guint mask) const
{
    return _desktop->canvas->gobble_key_events(keyval, mask);
}

/**
 * Discard matching motion events from top of event bucket.
 * Convenience function that just passes request to canvas.
 */
void ToolBase::gobble_motion_events(guint mask) const
{
    _desktop->canvas->gobble_motion_events(mask);
}

/**
 * Calls virtual set() function of ToolBase.
 */
void sp_event_context_read(ToolBase *ec, gchar const *key) {
    g_return_if_fail(ec != nullptr);
    g_return_if_fail(key != nullptr);

    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    Inkscape::Preferences::Entry val = prefs->getEntry(ec->getPrefsPath() + '/' + key);
    ec->set(val);
}

/**
 * Handles snapping events for all tools and then passes to tool_root_handler.
 */
gint ToolBase::start_root_handler(GdkEvent *event)
{
#ifdef EVENT_DEBUG
    ui_dump_event(reinterpret_cast<GdkEvent *>(event), "ToolBase::start_root_handler");
#endif

    if (!_uses_snap) {
        return this->tool_root_handler(event);
    }

    switch (event->type) {
    case GDK_MOTION_NOTIFY:
        sp_event_context_snap_delay_handler(this, nullptr, nullptr,
                (GdkEventMotion *) event,
                DelayedSnapEvent::EVENTCONTEXT_ROOT_HANDLER);
        break;
    case GDK_BUTTON_RELEASE:
        if (_delayed_snap_event) {
            // If we have any pending snapping action, then invoke it now
            sp_event_context_snap_watchdog_callback(_delayed_snap_event);
        }
        break;
    case GDK_BUTTON_PRESS:
    case GDK_2BUTTON_PRESS:
    case GDK_3BUTTON_PRESS:
        // Snapping will be on hold if we're moving the mouse at high speeds. When starting
        // drawing a new shape we really should snap though.
        _desktop->namedview->snap_manager.snapprefs.setSnapPostponedGlobally(false);
        break;
    default:
        break;
    }

    return this->tool_root_handler(event);
}

/**
 * Calls the right tool's event handler, depending on the selected tool and state.
 */
gint ToolBase::tool_root_handler(GdkEvent *event)
{
#ifdef EVENT_DEBUG
    ui_dump_event(reinterpret_cast<GdkEvent *>(event), "tool_root_handler");
#endif
    gint ret = false;

    // Just set the on buttons for now. later, behave as intended.
    this->set_on_buttons(event);

    // refresh coordinates UI here while 'event' is still valid
    set_event_location(_desktop, event);

    // Panning has priority over tool-specific event handling
    if (this->is_panning()) {
        ret = ToolBase::root_handler(event);
    } else {
        ret = this->root_handler(event);
    }

    // at this point 'event' could be deleted already (after ctrl+w document close)

    return ret;
}

/**
 * Starts handling item snapping and pass to virtual_item_handler afterwards.
 */
gint ToolBase::start_item_handler(SPItem *item, GdkEvent *event)
{
    if (!_uses_snap) {
        return this->virtual_item_handler(item, event);
    }

    switch (event->type) {
    case GDK_MOTION_NOTIFY:
        sp_event_context_snap_delay_handler(this, (gpointer) item, nullptr, (GdkEventMotion *) event, DelayedSnapEvent::EVENTCONTEXT_ITEM_HANDLER);
        break;
    case GDK_BUTTON_RELEASE:
        if (_delayed_snap_event) {
            // If we have any pending snapping action, then invoke it now
            sp_event_context_snap_watchdog_callback(_delayed_snap_event);
        }
        break;
    case GDK_BUTTON_PRESS:
    case GDK_2BUTTON_PRESS:
    case GDK_3BUTTON_PRESS:
        // Snapping will be on hold if we're moving the mouse at high speeds. When starting
        // drawing a new shape we really should snap though.
        _desktop->namedview->snap_manager.snapprefs.setSnapPostponedGlobally(false);
        break;
    default:
        break;
    }

    return this->virtual_item_handler(item, event);
}

gint ToolBase::virtual_item_handler(SPItem *item, GdkEvent *event)
{
    gint ret = false;

    // Just set the on buttons for now. later, behave as intended.
    this->set_on_buttons(event);

    // Panning has priority over tool-specific event handling
    if (is_panning()) {
        ret = ToolBase::item_handler(item, event);
    } else {
        ret = item_handler(item, event);
    }

    if (!ret) {
        ret = this->tool_root_handler(event);
    } else {
        set_event_location(_desktop, event);
    }

    return ret;
}

/**
 * Shows coordinates on status bar.
 */
static void set_event_location(SPDesktop *desktop, GdkEvent *event) {
    if (event->type != GDK_MOTION_NOTIFY) {
        return;
    }

    Geom::Point const button_w(event->button.x, event->button.y);
    Geom::Point const button_dt(desktop->w2d(button_w));
    desktop->set_coordinate_status(button_dt);
}

//-------------------------------------------------------------------
/**
 * Create popup menu and tell Gtk to show it.
 */
void sp_event_root_menu_popup(SPDesktop *desktop, SPItem *item, GdkEvent *event) {

    // It seems the param item is the SPItem at the bottom of the z-order
    // Using the same function call used on left click in sp_select_context_item_handler() to get top of z-order
    // fixme: sp_canvas_arena should set the top z-order object as arena->active
    item = sp_event_context_find_item (desktop,
                              Geom::Point(event->button.x, event->button.y), FALSE, FALSE);

    if (event->type == GDK_KEY_PRESS && !desktop->getSelection()->isEmpty()) {
        item = desktop->getSelection()->items().front();
    }

    ContextMenu* menu = new ContextMenu(desktop, item);
    menu->attach_to_widget(*(desktop->getCanvas())); // So actions work!
    menu->show();

    switch (event->type) {
    case GDK_BUTTON_PRESS:
    case GDK_KEY_PRESS:
        menu->popup_at_pointer(event);
        break;
    default:
        break;
    }
}

/**
 * Show tool context specific modifier tip.
 */
void sp_event_show_modifier_tip(Inkscape::MessageContext *message_context,
        GdkEvent *event, gchar const *ctrl_tip, gchar const *shift_tip,
        gchar const *alt_tip) {
    guint keyval = get_latin_keyval(&event->key);

    bool ctrl = ctrl_tip && (MOD__CTRL(event) || (keyval == GDK_KEY_Control_L) || (keyval
            == GDK_KEY_Control_R));
    bool shift = shift_tip && (MOD__SHIFT(event) || (keyval == GDK_KEY_Shift_L) || (keyval
            == GDK_KEY_Shift_R));
    bool alt = alt_tip && (MOD__ALT(event) || (keyval == GDK_KEY_Alt_L) || (keyval
            == GDK_KEY_Alt_R) || (keyval == GDK_KEY_Meta_L) || (keyval == GDK_KEY_Meta_R));

    gchar *tip = g_strdup_printf("%s%s%s%s%s", (ctrl ? ctrl_tip : ""), (ctrl
            && (shift || alt) ? "; " : ""), (shift ? shift_tip : ""), ((ctrl
            || shift) && alt ? "; " : ""), (alt ? alt_tip : ""));

    if (strlen(tip) > 0) {
        message_context->flash(Inkscape::INFORMATION_MESSAGE, tip);
    }

    g_free(tip);
}

/**
 * Try to determine the keys group of Latin layout.
 * Check available keymap entries for Latin 'a' key and find the minimal integer value.
 */
static void update_latin_keys_group() {
    GdkKeymapKey* keys;
    gint n_keys;

    latin_keys_group_valid = FALSE;
    latin_keys_groups.clear();

    if (gdk_keymap_get_entries_for_keyval(Gdk::Display::get_default()->get_keymap(), GDK_KEY_a, &keys, &n_keys)) {
        for (gint i = 0; i < n_keys; i++) {
            latin_keys_groups.insert(keys[i].group);

            if (!latin_keys_group_valid || keys[i].group < latin_keys_group) {
                latin_keys_group = keys[i].group;
                latin_keys_group_valid = TRUE;
            }
        }
        g_free(keys);
    }
}

/**
 * Initialize Latin keys group handling.
 */
void init_latin_keys_group() {
    g_signal_connect(G_OBJECT(Gdk::Display::get_default()->get_keymap()),
            "keys-changed", G_CALLBACK(update_latin_keys_group), NULL);
    update_latin_keys_group();
}

/**
 * Return the keyval corresponding to the key event in Latin group.
 *
 * Use this instead of simply event->keyval, so that your keyboard shortcuts
 * work regardless of layouts (e.g., in Cyrillic).
 */
guint get_latin_keyval(GdkEventKey const *event, guint *consumed_modifiers /*= NULL*/) {
    guint keyval = 0;
    GdkModifierType modifiers;
    gint group = latin_keys_group_valid ? latin_keys_group : event->group;

    if (latin_keys_groups.count(event->group)) {
        // Keyboard group is a latin layout, so just use it.
        group = event->group;
    }

    gdk_keymap_translate_keyboard_state(
            Gdk::Display::get_default()->get_keymap(),
            event->hardware_keycode, (GdkModifierType) event->state, group,
            &keyval, nullptr, nullptr, &modifiers);

    if (consumed_modifiers) {
        *consumed_modifiers = modifiers;
    }
    if (keyval != event->keyval) {
        std::cerr << "get_latin_keyval: OH OH OH keyval did change! "
                  << "  keyval: " << keyval << " (" << (char)keyval << ")"
                  << "  event->keyval: " << event->keyval << "(" << (char)event->keyval << ")" << std::endl;
    }
    return keyval;
}

/**
 * Returns item at point p in desktop.
 *
 * If state includes alt key mask, cyclically selects under; honors
 * into_groups.
 */
SPItem *sp_event_context_find_item(SPDesktop *desktop, Geom::Point const &p,
                                   bool select_under, bool into_groups)
{
    SPItem *item = nullptr;

    if (select_under) {
        auto tmp = desktop->selection->items();
        std::vector<SPItem *> vec(tmp.begin(), tmp.end());
        SPItem *selected_at_point = desktop->getItemFromListAtPointBottom(vec, p);
        item = desktop->getItemAtPoint(p, into_groups, selected_at_point);
        if (item == nullptr) { // we may have reached bottom, flip over to the top
            item = desktop->getItemAtPoint(p, into_groups, nullptr);
        }
    } else {
        item = desktop->getItemAtPoint(p, into_groups, nullptr);
    }

    return item;
}

/**
 * Returns item if it is under point p in desktop, at any depth; otherwise returns NULL.
 *
 * Honors into_groups.
 */
SPItem *
sp_event_context_over_item(SPDesktop *desktop, SPItem *item,
        Geom::Point const &p) {
	std::vector<SPItem*> temp;
    temp.push_back(item);
    SPItem *item_at_point = desktop->getItemFromListAtPointBottom(temp, p);
    return item_at_point;
}

ShapeEditor *
sp_event_context_get_shape_editor(ToolBase *ec) {
    return ec->shape_editor;
}


/**
 * Analyses the current event, calculates the mouse speed, turns snapping off (temporarily) if the
 * mouse speed is above a threshold, and stores the current event such that it can be re-triggered when needed
 * (re-triggering is controlled by a watchdog timer).
 *
 * @param ec Pointer to the event context.
 * @param dse_item Pointer that store a reference to a canvas or to an item.
 * @param dse_item2 Another pointer, storing a reference to a knot or controlpoint.
 * @param event Pointer to the motion event.
 * @param origin Identifier (enum) specifying where the delay (and the call to this method) were initiated.
 */
void sp_event_context_snap_delay_handler(ToolBase *ec,
        gpointer const dse_item, gpointer const dse_item2, GdkEventMotion *event,
        DelayedSnapEvent::DelayedSnapEventOrigin origin)
{
    static guint32 prev_time;
    static std::optional<Geom::Point> prev_pos;

    if (!ec->_uses_snap || ec->_dse_callback_in_process) {
        return;
    }

    // Snapping occurs when dragging with the left mouse button down, or when hovering e.g. in the pen tool with left mouse button up
    bool const c1 = event->state & GDK_BUTTON2_MASK; // We shouldn't hold back any events when other mouse buttons have been
    bool const c2 = event->state & GDK_BUTTON3_MASK; // pressed, e.g. when scrolling with the middle mouse button; if we do then
    // Inkscape will get stuck in an unresponsive state
    bool const c3 = dynamic_cast<Inkscape::UI::Tools::CalligraphicTool *>(ec);
    // The snap delay will repeat the last motion event, which will lead to
    // erroneous points in the calligraphy context. And because we don't snap
    // in this context, we might just as well disable the snap delay all together
    bool const c4 = ec->is_panning(); // Don't snap while panning

    if (c1 || c2 || c3 || c4) {
        // Make sure that we don't send any pending snap events to a context if we know in advance
        // that we're not going to snap any way (e.g. while scrolling with middle mouse button)
        // Any motion event might affect the state of the context, leading to unexpected behavior
        ec->discard_delayed_snap_event();
    } else if (ec->getDesktop() &&
               ec->getDesktop()->namedview->snap_manager.snapprefs.getSnapEnabledGlobally()) {
        // Snap when speed drops below e.g. 0.02 px/msec, or when no motion events have occurred for some period.
        // i.e. snap when we're at stand still. A speed threshold enforces snapping for tablets, which might never
        // be fully at stand still and might keep spitting out motion events.
        ec->getDesktop()->namedview->snap_manager.snapprefs.setSnapPostponedGlobally(true); // put snapping on hold

        Geom::Point event_pos(event->x, event->y);
        guint32 event_t = gdk_event_get_time((GdkEvent *) event);

        if (prev_pos) {
            Geom::Coord dist = Geom::L2(event_pos - *prev_pos);
            guint32 delta_t = event_t - prev_time;
            gdouble speed = delta_t > 0 ? dist / delta_t : 1000;
            //std::cout << "Mouse speed = " << speed << " px/msec " << std::endl;
            if (speed > 0.02) { // Jitter threshold, might be needed for tablets
                // We're moving fast, so postpone any snapping until the next GDK_MOTION_NOTIFY event. We
                // will keep on postponing the snapping as long as the speed is high.
                // We must snap at some point in time though, so set a watchdog timer at some time from
                // now, just in case there's no future motion event that drops under the speed limit (when
                // stopping abruptly)
                delete ec->_delayed_snap_event;
                ec->_delayed_snap_event = new DelayedSnapEvent(ec, dse_item, dse_item2, event, origin); // watchdog is reset, i.e. pushed forward in time
                // If the watchdog expires before a new motion event is received, we will snap (as explained
                // above). This means however that when the timer is too short, we will always snap and that the
                // speed threshold is ineffective. In the extreme case the delay is set to zero, and snapping will
                // be immediate, as it used to be in the old days ;-).
            } else { // Speed is very low, so we're virtually at stand still
                // But if we're really standing still, then we should snap now. We could use some low-pass filtering,
                // otherwise snapping occurs for each jitter movement. For this filtering we'll leave the watchdog to expire,
                // snap, and set a new watchdog again.
                if (ec->_delayed_snap_event == nullptr) { // no watchdog has been set
                    // it might have already expired, so we'll set a new one; the snapping frequency will be limited this way
                    ec->_delayed_snap_event = new DelayedSnapEvent(ec, dse_item, dse_item2, event, origin);
                } // else: watchdog has been set before and we'll wait for it to expire
            }
        } else {
            // This is the first GDK_MOTION_NOTIFY event, so postpone snapping and set the watchdog
            g_assert(ec->_delayed_snap_event == nullptr);
            ec->_delayed_snap_event = new DelayedSnapEvent(ec, dse_item, dse_item2, event, origin);
        }

        prev_pos = event_pos;
        prev_time = event_t;
    }
}

/**
 * When the snap delay watchdog timer barks, this method will be called and will re-inject the last motion
 * event in an appropriate place, with snapping being turned on again.
 */
gboolean sp_event_context_snap_watchdog_callback(gpointer data) {
    // Snap NOW! For this the "postponed" flag will be reset and the last motion event will be repeated
    DelayedSnapEvent *dse = reinterpret_cast<DelayedSnapEvent*> (data);

    if (dse == nullptr) {
        // This might occur when this method is called directly, i.e. not through the timer
        // E.g. on GDK_BUTTON_RELEASE in start_root_handler()
        return FALSE;
    }

    ToolBase *ec = dse->getEventContext();
    if (ec == nullptr) {
        delete dse;
        return false;
    }

    const SPDesktop *dt = ec->getDesktop();
    if (dt == nullptr) {
        ec->_delayed_snap_event = nullptr;
        delete dse;
        return false;
    }

    ec->_dse_callback_in_process = true;

    dt->namedview->snap_manager.snapprefs.setSnapPostponedGlobally(false);

    // Depending on where the delayed snap event originated from, we will inject it back at it's origin
    // The switch below takes care of that and prepares the relevant parameters
    switch (dse->getOrigin()) {
    case DelayedSnapEvent::EVENTCONTEXT_ROOT_HANDLER:
        ec->tool_root_handler(dse->getEvent());
        break;
    case DelayedSnapEvent::EVENTCONTEXT_ITEM_HANDLER: {
        auto item = static_cast<SPItem *>(dse->getItem());
        if (item) {
            ec->virtual_item_handler(item, dse->getEvent());
        }
    }
        break;
    case DelayedSnapEvent::KNOT_HANDLER: {
        gpointer knot = dse->getItem2();
        check_if_knot_deleted(knot);
        if (knot && SP_IS_KNOT(knot)) {
            bool was_grabbed = SP_KNOT(knot)->is_grabbed();
            SP_KNOT(knot)->setFlag(SP_KNOT_GRABBED, true); // Must be grabbed for Inkscape::SelTrans::handleRequest() to pass
            sp_knot_handler_request_position(dse->getEvent(), SP_KNOT(knot));
            SP_KNOT(knot)->setFlag(SP_KNOT_GRABBED, was_grabbed);
        }
    }
        break;
    case DelayedSnapEvent::CONTROL_POINT_HANDLER: {
        using Inkscape::UI::ControlPoint;
        gpointer pitem2 = dse->getItem2();
        if (!pitem2)
        {
            ec->_delayed_snap_event = nullptr;
            delete dse;
            return false;
        }
        ControlPoint *point = reinterpret_cast<ControlPoint*> (pitem2);
        if (point) {
            if (point->position().isFinite() && (dt == point->_desktop)) {
                point->_eventHandler(ec, dse->getEvent());
            }
            else {
                //workaround:
                //[Bug 781893] Crash after moving a Bezier node after Knot path effect?
                // --> at some time, some point with X = 0 and Y = nan (not a number) is created ...
                //     even so, the desktop pointer is invalid and equal to 0xff
                g_warning ("encountered non finite point when evaluating snapping callback");
            }
        }
    }
        break;
    case DelayedSnapEvent::GUIDE_HANDLER: {
        auto item  = static_cast<CanvasItemGuideLine *>(dse->getItem());
        auto item2 = static_cast<SPGuide *>(dse->getItem2());
        if (item && item2) {
            sp_dt_guide_event(dse->getEvent(), item, item2);
        }
    }
        break;
    case DelayedSnapEvent::GUIDE_HRULER:
    case DelayedSnapEvent::GUIDE_VRULER: {
        gpointer item = dse->getItem();
        auto item2 = static_cast<Gtk::Widget *>(dse->getItem2());
        if (item && item2) {
            g_assert(GTK_IS_WIDGET(item));
            if (dse->getOrigin() == DelayedSnapEvent::GUIDE_HRULER) {
                SPDesktopWidget::ruler_event(GTK_WIDGET(item), dse->getEvent(), SP_DESKTOP_WIDGET(item2), true);
            } else {
                SPDesktopWidget::ruler_event(GTK_WIDGET(item), dse->getEvent(), SP_DESKTOP_WIDGET(item2), false);
            }
        }
    }
        break;
    default:
        g_warning("Origin of snap-delay event has not been defined!;");
        break;
    }

    ec->_delayed_snap_event = nullptr;
    delete dse;

    ec->_dse_callback_in_process = false;

    return FALSE; //Kills the timer and stops it from executing this callback over and over again.
}

void ToolBase::discard_delayed_snap_event()
{
    delete _delayed_snap_event;
    _delayed_snap_event = nullptr;
    _desktop->namedview->snap_manager.snapprefs.setSnapPostponedGlobally(false);
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
