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

#ifndef INKSCAPE_APPLICATION_APPLICATION_H
#define INKSCAPE_APPLICATION_APPLICATION_H

#include <gtkmm/main.h>

class SPReprDoc;

namespace Inkscape {
namespace NSApplication {

class AppPrototype;

class Application
{
public:
    Application(int argc, char **argv, gboolean use_gui=true, gboolean new_gui=false);
    virtual ~Application();

    gboolean        loadPreferences();
    gboolean        savePreferences();

    const gchar*    etcdir() const;
    const gchar*    homedir() const;
    const gchar*    sharedir() const;

    gint            run();
    void            exit();
    
protected:
    Application(Application const &);
    Application& operator=(Application const &);

    Gtk::Main      *_gtk_main;
    gint            _argc;
    char           **_argv;
    SPReprDoc      *_preferences;
    AppPrototype   *_app_impl;

    gchar const    *_path_home;
    gchar const    *_path_etc;
    gchar const    *_path_share;

    gboolean        _save_preferences;
    gboolean        _use_gui;
};

} // namespace NSApplication
} // namespace Inkscape

#endif // INKSCAPE_APPLICATION_EDITOR_H

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
