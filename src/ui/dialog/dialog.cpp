/**
 * \brief Base class for dialogs in Inkscape.  This class provides certain
 *        common behaviors and styles wanted of all dialogs in the application.
 *
 * Author:
 *   Bryce W. Harrington <bryce@bryceharrington.org>
 *
 * Copyright (C) 2004 Bryce Harrington
 *
 * Released under GNU GPL.  Read the file 'COPYING' for more information.
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <gtkmm/stock.h>

#include "dialog.h"

namespace Inkscape {
namespace UI {
namespace Dialog {

/**
 * UI::Dialog::Dialog is a base class for all dialogs in Inkscape.  The
 * purpose of this class is to provide a unified place for ensuring
 * style and behavior.  Specifically, this class provides functionality
 * for saving and restoring the size and position of dialogs (through
 * the user's preferences file).
 *
 * It also provides some general purpose signal handlers for things like
 * showing and hiding all dialogs.
 */
Dialog::Dialog()
{
    set_has_separator(false);

    add_button(Gtk::Stock::APPLY, Gtk::RESPONSE_APPLY);
    add_button(Gtk::Stock::CLOSE, Gtk::RESPONSE_CLOSE);
    set_default_response(Gtk::RESPONSE_APPLY);

    // TODO:  Perhaps this '-1000' should be a constant instead of a magic number?
    int x = -1000;
    int y = -1000;
    int w = 0;
    int h = 0;

    /* TODO:  Hook to preferences...
    x = prefs_get_int_attribute (prefs_path, "x", 0);
    y = prefs_get_int_attribute (prefs_path, "y", 0);
    w = prefs_get_int_attribute (prefs_path, "w", 0);
    h = prefs_get_int_attribute (prefs_path, "h", 0);
    */

    // If there are stored values for where the dialog should be
    // located, then restore the dialog to that position.
    if (x != -1000 && y != -1000) {
        move(x, y);
    } else {
        // ...otherwise just put it in the middle of the screen
/* TODO        set_position(GTK_WIN_POS_CENTER); 
*/
    }

    // If there are stored height and width values for the dialog,
    // resize the window to match; otherwise we leave it at its default
    if (w != 0 && h != 0) {
        resize(w, h);
    }
}

Dialog::Dialog(BaseObjectType *gobj)
    : Gtk::Dialog(gobj)
{
}

Dialog::~Dialog() 
{
    // TODO:  Should this be invoked prior to the destructor stage?
    onDestroy();
}

void
Dialog::onDestroy()
{
    int y, x, w, h;

    get_position(x, y);
    get_size(w, h);

/* TODO:  Hook up preferences storing
    prefs_set_int_attribute (prefs_path, "x", x);
    prefs_set_int_attribute (prefs_path, "y", y);
    prefs_set_int_attribute (prefs_path, "w", w);
    prefs_set_int_attribute (prefs_path, "h", h);
*/

    _setDesktop(NULL);

}

void
Dialog::onHideDialogs()
{
    _user_hidden = true;
    hide();
}

void
Dialog::onHideF12()
{
    hide();
}

void
Dialog::onShowDialogs()
{
    _user_hidden = false;
/* TODO:  Gtkmmify
    gtk_window_set_transient_for(GTK_WINDOW(gobj()), 
                                 GTK_WINDOW(gtk_widget_get_toplevel(GTK_WIDGET(owner)))
        );
*/
    property_destroy_with_parent() = true;

    show();
    raise();
    present();
}


void
Dialog::onShowF12()
{
    if (_user_hidden) {
        return;
    }
    show();
    raise();
    present();
}

// TODO:  Temporary define until this code is hooked into Inkscape
#ifndef SP_ACTIVE_DESKTOP
#define SP_ACTIVE_DESKTOP (false)
#endif

Selection*
Dialog::_getSelection()
{
    if (!_desktop) {
        return NULL;
    }

//    return _desktop->getSelection();
    return NULL;  // TODO
}

void
Dialog::_setDesktop(SPDesktop *desktop) {
    if (desktop) {
        g_object_ref(desktop);
    }
    if (_desktop) {
        g_object_unref(_desktop);
    }
    _desktop = desktop;
}

void
Dialog::transientize()
{
    if (get_modal()) {
        set_type_hint(Gdk::WINDOW_TYPE_HINT_UTILITY);
    }

/* TODO:  Gtkmmify
    if (prefs_get_int_attribute( "options.dialogsskiptaskbar", "value", 0)) {
        set_skip_taskbar_hint(TRUE);
    }

    gint transient_policy = prefs_get_int_attribute_limited( "options.transientpolicy", "value", 1, 0, 2);

    if (transient_policy) {
        // transientzing does not work on windows; when you minimize a document
        // and then open it back, only its transient emerges and you cannot access
        // the document window.
        //
#ifndef WIN32
        // if there's an active document window, attach dialog to it as a transient:
        if ( SP_ACTIVE_DESKTOP &&
             g_object_get_data( G_OBJECT(SP_ACTIVE_DESKTOP), "window"))
        {
            set_transient_for( this, get_data(G_OBJECT(SP_ACTIVE_DESKTOP), "window"));
        }
#endif
    }
*/
}

void
Dialog::on_response(int response_id)
{
    switch (response_id) {
        case Gtk::RESPONSE_APPLY: {
            _apply();
            break;
        }
        case Gtk::RESPONSE_CLOSE: {
            _close();
            break;
        }
    }
}

void
Dialog::_apply()
{
    g_warning("Apply button clicked for dialog [Dialog::_apply()]");
}

void
Dialog::_close()
{
    hide();
}

} // namespace Dialog
} // namespace UI
} // namespace Inkscape

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=c++:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :
