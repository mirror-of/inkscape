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

namespace Inkscape {
namespace XML {
class Document;
}
}


namespace Inkscape {
namespace NSApplication {

class AppPrototype;

class Application
{
public:
    Application(int argc, char **argv, bool use_gui=true, bool new_gui=false);
    virtual ~Application();

    bool        loadPreferences();
    bool        savePreferences();

    const gchar*    homedir() const;

    gint            run();
    void            exit();

protected:
    Application(Application const &);
    Application& operator=(Application const &);

    Gtk::Main      *_gtk_main;
    gint            _argc;
    char           **_argv;
    Inkscape::XML::Document      *_preferences;
    AppPrototype   *_app_impl;

    mutable gchar  *_path_home;

    bool        _save_preferences;
    bool        _use_gui;
};

} // namespace NSApplication
} // namespace Inkscape

#endif /* !INKSCAPE_APPLICATION_APPLICATION_H */

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=99 :
