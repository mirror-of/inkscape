/**
 * \brief  The top level class for managing the application.
 *
 * Author:
 *   Bryce W. Harrington <bryce@bryceharrington.org>
 *
 * Copyright (C) 2005 Bryce Harrington
 *
 * Released under GNU GPL.  Read the file 'COPYING' for more information.
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <gtkmm/main.h>
#include <glibmm/i18n.h>

#include "application.h"
#include "app-prototype.h"
#include "editor.h"

int sp_main_gui(int argc, char const **argv);
int sp_main_console(int argc, char const **argv);

namespace Inkscape {
namespace NSApplication {

class SPReprDoc;
class GSList;

Application::Application(int argc, char **argv, gboolean use_gui, gboolean new_gui)
    : _gtk_main(NULL),
      _argc(argc),
      _argv(NULL),
      _preferences(NULL),
      _save_preferences(false),
      _use_gui(use_gui)
{
    if (argv != NULL) {
        _argv = argv;   // TODO:  Is this correct?
    }

    // TODO:  Initialize _preferences with the preferences skeleton
    _save_preferences = loadPreferences();

    // TODO:  Determine class by arguments
//    _app_impl = (AppPrototype*)new Editor(_argc, _argv);

    // TODO:  Set up paths
/*
    _path_home  = "/home/bryce";
    _path_etc   = "/etc/inkscape";
    _path_share = "/usr/share/inkscape";
*/

    if (new_gui) {
        g_warning("Creating Gtk::Main object\n");
        _gtk_main = new Gtk::Main(argc, argv, true);
    }

}

Application::~Application()
{
/*
    delete _app_impl;
    delete _preferences;
*/
}

gboolean
Application::loadPreferences()
{
    // TODO
    return true;
}

gboolean
Application::savePreferences()
{
    // TODO
    return true;
}

char const*
Application::etcdir() const
{
    return _path_etc;
}

char const*
Application::homedir() const
{
    return _path_home;
}

char const*
Application::sharedir() const
{
    return _path_share;
}

gint
Application::run()
{
    gint result = 0;

    g_assert(_app_impl != NULL);

    /* Note:  This if loop should be replaced by calls to the
     * various subclasses of I::A::AppPrototype.
     */
    if (_gtk_main != NULL) {
        g_warning("Running new gtkmm interface\n");
//        result = _gtk_main->run(_app_impl->getWindow());
        _gtk_main->run();
        result = 0;

    } else if (_use_gui) {
//        g_warning("Running old gtk interface\n");
        result = sp_main_gui(_argc, (const char**)_argv);

    } else {
        g_warning("Running commandline\n");
        result = sp_main_console(_argc, (const char**)_argv);
    }

    return result;
}

void
Application::exit()
{
/* TODO
    // Emit shutdown signal
    g_signal_emit(G_OBJECT(INKSCAPE), inkscape_signals[SHUTDOWN_SIGNAL], 0);

    if (inkscape->preferences && inkscape->save_preferences) {
        inkscape_save_preferences(INKSCAPE);
    }
*/
    if (_gtk_main) {
        _gtk_main->quit();
    }

}

} // namespace NSApplication
} // namespace Inkscape

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0))
  indent-tabs-mode:nil
  fill-column:75
  End:
*/
// vim: filetype=c++:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :
