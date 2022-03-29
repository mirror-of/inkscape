// SPDX-License-Identifier: GPL-2.0-or-later
#ifndef SEEN_SP_EVENT_CONTEXT_H
#define SEEN_SP_EVENT_CONTEXT_H

/*
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   Frank Felfe <innerspace@iname.com>
 *
 * Copyright (C) 1999-2002 authors
 * Copyright (C) 2001-2002 Ximian, Inc.
 *
 * Released under GNU GPL v2+, read the file 'COPYING' for more information.
 */

#include <cstddef>
#include <string>
#include <memory>

#include <gdkmm/device.h>  // EventMask
#include <gdkmm/cursor.h>
#include <glib-object.h>
#include <sigc++/trackable.h>

#include <2geom/point.h>

#include "preferences.h"

class GrDrag;
class SPDesktop;
class SPObject;
class SPItem;
class SPGroup;
class KnotHolder;
namespace Inkscape {
    class MessageContext;
    class SelCue;
}

namespace Inkscape {
namespace UI {

class ShapeEditor;

namespace Tools {

class ToolBase;

gboolean sp_event_context_snap_watchdog_callback(gpointer data);

class DelayedSnapEvent {
public:
    enum DelayedSnapEventOrigin {
        UNDEFINED_HANDLER = 0,
        EVENTCONTEXT_ROOT_HANDLER,
        EVENTCONTEXT_ITEM_HANDLER,
        KNOT_HANDLER,
        CONTROL_POINT_HANDLER,
        GUIDE_HANDLER,
        GUIDE_HRULER,
        GUIDE_VRULER
    };

    DelayedSnapEvent(ToolBase *event_context, gpointer const dse_item, gpointer dse_item2, GdkEventMotion const *event, DelayedSnapEvent::DelayedSnapEventOrigin const origin)
        : _timer_id(0)
        , _event(nullptr)
        , _item(dse_item)
        , _item2(dse_item2)
        , _origin(origin)
        , _event_context(event_context)
    {
        Inkscape::Preferences *prefs = Inkscape::Preferences::get();
        double value = prefs->getDoubleLimited("/options/snapdelay/value", 0, 0, 1000);

        // We used to have this specified in milliseconds; this has changed to seconds now for consistency's sake
        if (value > 1) { // Apparently we have an old preference file, this value must have been in milliseconds;
            value = value / 1000.0; // now convert this value to seconds
        }

        _timer_id = g_timeout_add(value*1000.0, &sp_event_context_snap_watchdog_callback, this);
        _event = gdk_event_copy((GdkEvent*) event);

        ((GdkEventMotion *)_event)->time = GDK_CURRENT_TIME;
    }

    ~DelayedSnapEvent()    {
        if (_timer_id > 0) g_source_remove(_timer_id); // Kill the watchdog
        if (_event != nullptr) gdk_event_free(_event); // Remove the copy of the original event
    }

    ToolBase* getEventContext() {
        return _event_context;
    }

    DelayedSnapEventOrigin getOrigin() {
        return _origin;
    }

    GdkEvent* getEvent() {
        return _event;
    }

    gpointer getItem() {
        return _item;
    }

    gpointer getItem2() {
        return _item2;
    }

private:
    guint _timer_id;
    GdkEvent* _event;
    gpointer _item;
    gpointer _item2;
    DelayedSnapEventOrigin _origin;
    ToolBase* _event_context;
};

void sp_event_context_snap_delay_handler(ToolBase *ec, gpointer const dse_item, gpointer const dse_item2, GdkEventMotion *event, DelayedSnapEvent::DelayedSnapEventOrigin origin);


/**
 * Base class for Event processors.
 *
 * This is per desktop object, which (its derivatives) implements
 * different actions bound to mouse events.
 *
 * ToolBase is an abstract base class of all tools. As the name
 * indicates, event context implementations process UI events (mouse
 * movements and keypresses) and take actions (like creating or modifying
 * objects).  There is one event context implementation for each tool,
 * plus few abstract base classes. Writing a new tool involves
 * subclassing ToolBase.
 */
class ToolBase : public sigc::trackable
{
public:
    ToolBase(SPDesktop *desktop, std::string prefs_path, std::string cursor_filename, bool uses_snap = true);

    virtual ~ToolBase();

    ToolBase(const ToolBase&) = delete;
    ToolBase& operator=(const ToolBase&) = delete;

    virtual void set(const Inkscape::Preferences::Entry& val);
    virtual bool root_handler(GdkEvent *event);
    virtual bool item_handler(SPItem *item, GdkEvent *event);
    virtual void menu_popup(GdkEvent *event, SPObject *obj = nullptr);

    void set_on_buttons(GdkEvent *event);
    bool are_buttons_1_and_3_on() const;
    bool are_buttons_1_and_3_on(GdkEvent *event);

    std::string getPrefsPath() { return _prefs_path; };
    void enableSelectionCue (bool enable=true);

    Inkscape::MessageContext *defaultMessageContext() const {
        return message_context.get();
    }

    SPDesktop *getDesktop() { return _desktop; }
    SPGroup *currentLayer() const;

    // Commonly used CanvasItemCatchall grab/ungrab.
    void grabCanvasEvents(Gdk::EventMask mask =
                          Gdk::KEY_PRESS_MASK      |
                          Gdk::BUTTON_RELEASE_MASK |
                          Gdk::POINTER_MOTION_MASK |
                          Gdk::BUTTON_PRESS_MASK);
    void ungrabCanvasEvents();

    /**
     * An observer that relays pref changes to the derived classes.
     */
    class ToolPrefObserver: public Inkscape::Preferences::Observer {
    public:
        ToolPrefObserver(Glib::ustring const &path, ToolBase *ec)
            : Inkscape::Preferences::Observer(path)
            , ec(ec)
        {
        }

        void notify(Inkscape::Preferences::Entry const &val) override {
            ec->set(val);
        }

    private:
        ToolBase * const ec;
    };

private:
    Inkscape::Preferences::Observer *pref_observer = nullptr;
    std::string _prefs_path;

protected:
    Glib::RefPtr<Gdk::Cursor> _cursor;
    std::string _cursor_filename = "select.svg";
    std::string _cursor_default = "select.svg";

    gint xp = 0;           ///< where drag started
    gint yp = 0;           ///< where drag started
    gint tolerance = 0;
    bool within_tolerance = false;  ///< are we still within tolerance of origin
    bool _button1on = false;
    bool _button2on = false;
    bool _button3on = false;
    SPItem *item_to_select = nullptr; ///< the item where mouse_press occurred, to
                                      ///< be selected if this is a click not drag

    Geom::Point setup_for_drag_start(GdkEvent *ev);

private:
    enum
    {
        PANNING_NONE = 0,          //
        PANNING_SPACE_BUTTON1 = 1, // TODO is this mode relevant?
        PANNING_BUTTON2 = 2,       //
        PANNING_BUTTON3 = 3,       //
        PANNING_SPACE = 4,
    } panning = PANNING_NONE;

public:
    gint start_root_handler(GdkEvent *event);
    gint tool_root_handler(GdkEvent *event);
    gint start_item_handler(SPItem *item, GdkEvent *event);
    gint virtual_item_handler(SPItem *item, GdkEvent *event);

    /// True if we're panning with any method (space bar, middle-mouse, right-mouse+Ctrl)
    bool is_panning() const { return panning != 0; }

    /// True if we're panning with the space bar
    bool is_space_panning() const { return panning == PANNING_SPACE || panning == PANNING_SPACE_BUTTON1; }

    bool rotating_mode = false;;

    std::unique_ptr<Inkscape::MessageContext> message_context;
    Inkscape::SelCue *_selcue = nullptr;

    GrDrag *_grdrag = nullptr;

    ShapeEditor* shape_editor = nullptr;

    bool _dse_callback_in_process = false;

    bool _uses_snap = false;
    DelayedSnapEvent *_delayed_snap_event = nullptr;

    void discard_delayed_snap_event();
    void set_cursor(std::string filename);
    void use_cursor(Glib::RefPtr<Gdk::Cursor> cursor);
    Glib::RefPtr<Gdk::Cursor> get_cursor(Glib::RefPtr<Gdk::Window> window, std::string filename);
    void use_tool_cursor();

    void enableGrDrag(bool enable = true);
    bool deleteSelectedDrag(bool just_one);
    bool hasGradientDrag() const;
    GrDrag *get_drag() { return _grdrag; }

protected:
    bool sp_event_context_knot_mouseover() const;

    void set_high_motion_precision(bool high_precision = true);

    int gobble_key_events(guint keyval, guint mask) const;
    void gobble_motion_events(guint mask) const;

    SPDesktop *_desktop = nullptr;

private:

    bool _keyboardMove(GdkEventKey const &event, Geom::Point const &dir);
};

void sp_event_context_read(ToolBase *ec, gchar const *key);


void sp_event_root_menu_popup(SPDesktop *desktop, SPItem *item, GdkEvent *event);

void sp_event_show_modifier_tip(Inkscape::MessageContext *message_context, GdkEvent *event,
                                gchar const *ctrl_tip, gchar const *shift_tip, gchar const *alt_tip);

void init_latin_keys_group();
guint get_latin_keyval(GdkEventKey const *event, guint *consumed_modifiers = nullptr);

SPItem *sp_event_context_find_item (SPDesktop *desktop, Geom::Point const &p, bool select_under, bool into_groups);
SPItem *sp_event_context_over_item (SPDesktop *desktop, SPItem *item, Geom::Point const &p);

void sp_toggle_dropper(SPDesktop *dt);

bool sp_event_context_knot_mouseover(ToolBase *ec);

} // namespace Tools
} // namespace UI
} // namespace Inkscape

#endif // SEEN_SP_EVENT_CONTEXT_H


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
