/**
 * \brief Editor Implementation class declaration for Inkscape.  This
 *        class implements the functionality of the window layout, menus,
 *        and signals.
 *
 * Authors:
 *   Bryce W. Harrington <bryce@bryceharrington.org>
 *   Derek P. Moore <derekm@hackunix.org>
 *
 * Copyright (C) 2004 Authors
 *
 * Released under GNU GPL.  Read the file 'COPYING' for more information.
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

/*
  TODO:  Replace SPDocument with the new Inkscape::Document
  TODO:  use_gui, args, and prefs move up to the 
         Inkscape::Application::Application level
  TODO:  Where is the best place for stock.h, stock-items.h, 
         and icons.h to live? 
  TODO:  A lot of the GUI bits in here need to be broken out
         to Inkscape::UI::View::Edit.
  TODO:  Change 'desktop's to 'view*'s
  TODO:  Add derivation from Inkscape::Application::RunMode
*/

#include "editor.h"
#include "editor-impl.h"

namespace Inkscape {
namespace NSApplication {

Editor::Editor(gint argc, char **argv, gboolean use_gui)
    : _preferences(NULL),
      _documents(NULL),
      _desktops(NULL),
      _argv0(NULL),
      _dialogs_toggle(true),
      _save_preferences(true),
      _use_gui(use_gui)
{
    g_warning("In Editor::Editor");
    rep = new EditorImpl();
    g_warning("Created a new EditorImpl object");

    // Store the arguments
//    if (_argv != NULL) {
//        _argv = g_strdup(argv);
//    }

    // TODO:  Initialize _preferences with the preferences skeleton
/*    _save_preferences = loadPreferences(); */

    // Signals
    /* TODO
       modify_selection
       change_selection
       set_selection
       set_eventcontext
       new_desktop
       destroy_desktop
       activate_desktop
       deactivate_desktop
       new_document
       destroy_document
       shut_down
       dialogs_hide
       dialogs_unhide
     */
}

Editor::~Editor()
{
    if (rep != NULL) {
        delete rep;
    }
}

/// Returns the Gtk::Window representation of this application object
Gtk::Window*
Editor::getWindow()
{
    return rep;
}

/// Returns the active document
SPDocument*
Editor::getActiveDocument()
{
/* TODO
    if (SP_ACTIVE_DESKTOP) {
        return SP_DT_DOCUMENT (SP_ACTIVE_DESKTOP);
    }
*/

    return NULL;
}

/// Returns the currently active desktop
SPDesktop*
Editor::getActiveDesktop()
{
    if (_desktops == NULL) {
        return NULL;
    }

//    return (SPDesktop *) _desktops->data;
    return NULL;
}

/// Returns the event context
SPEventContext*
Editor::getEventContext()
{
/*
    if (SP_ACTIVE_DESKTOP) {
        return SP_DT_EVENTCONTEXT (SP_ACTIVE_DESKTOP);
    }
*/

    return NULL;
}

/// Returns the name of the application
Glib::ustring
Editor::getName() const
{
    return "Inkscape";
}

int
Editor::loadPreferences()
{
    // TODO
    return 0;
}

int
Editor::savePreferences()
{
    // TODO
    return 0;
}

void
Editor::hideDialogs()
{
/* TODO
    g_signal_emit(G_OBJECT (inkscape), inkscape_signals[DIALOGS_HIDE], 0);
 */
    _dialogs_toggle = false;
}

void
Editor::unhideDialogs()
{
/* TODO
    g_signal_emit (G_OBJECT (inkscape), inkscape_signals[DIALOGS_UNHIDE], 0);
*/
    _dialogs_toggle = true;
}

void
Editor::toggleDialogs()
{
    if (_dialogs_toggle) {
//        dialogs_hide();
    } else {
//        dialogs_unhide();
    }
}

void
Editor::refreshDisplay()
{
    // TODO
}

void
Editor::exit()
{
    //emit shutdown signal so that dialogs could remember layout
/* TODO
    g_signal_emit (G_OBJECT (INKSCAPE), inkscape_signals[SHUTDOWN_SIGNAL], 0);

    if (inkscape->preferences && inkscape->save_preferences) {
        inkscape_save_preferences (INKSCAPE);
    }

    gtk_main_quit();
*/

}

} // namespace NSApplication
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
