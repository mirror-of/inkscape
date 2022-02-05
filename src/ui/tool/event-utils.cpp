// SPDX-License-Identifier: GPL-2.0-or-later
/** @file
 * Collection of shorthands to deal with GDK events.
 */
/* Authors:
 *   Krzysztof Kosiński <tweenk.pl@gmail.com>
 *
 * Copyright (C) 2009 Authors
 * Released under GNU GPL v2+, read the file 'COPYING' for more information.
 */

#include <cstring>
#include <gdk/gdk.h>
#include <gdk/gdkkeysyms.h>
#include <gdkmm/display.h>
#include "ui/tool/event-utils.h"

namespace Inkscape {
namespace UI {


guint shortcut_key(GdkEventKey const &event)
{
    guint shortcut_key = 0;
    gdk_keymap_translate_keyboard_state(
            Gdk::Display::get_default()->get_keymap(),
            event.hardware_keycode,
            (GdkModifierType) event.state,
            0   /*event->key.group*/,
            &shortcut_key, nullptr, nullptr, nullptr);
    return shortcut_key;
}

/** Returns the modifier state valid after this event. Use this when you process events
 * that change the modifier state. Currently handles only Shift, Ctrl, Alt. */
unsigned state_after_event(GdkEvent *event)
{
    unsigned state = 0;
    switch (event->type) {
    case GDK_KEY_PRESS:
        state = event->key.state;
        switch(shortcut_key(event->key)) {
        case GDK_KEY_Shift_L:
        case GDK_KEY_Shift_R:
            state |= GDK_SHIFT_MASK;
            break;
        case GDK_KEY_Control_L:
        case GDK_KEY_Control_R:
            state |= GDK_CONTROL_MASK;
            break;
        case GDK_KEY_Alt_L:
        case GDK_KEY_Alt_R:
            state |= GDK_MOD1_MASK;
            break;
        default: break;
        }
        break;
    case GDK_KEY_RELEASE:
        state = event->key.state;
        switch(shortcut_key(event->key)) {
        case GDK_KEY_Shift_L:
        case GDK_KEY_Shift_R:
            state &= ~GDK_SHIFT_MASK;
            break;
        case GDK_KEY_Control_L:
        case GDK_KEY_Control_R:
            state &= ~GDK_CONTROL_MASK;
            break;
        case GDK_KEY_Alt_L:
        case GDK_KEY_Alt_R:
            state &= ~GDK_MOD1_MASK;
            break;
        default: break;
        }
        break;
    default: break;
    }
    return state;
}

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
