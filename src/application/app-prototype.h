/**
 * \brief  Base class for different application modes
 *
 * Author:
 *   Bryce W. Harrington <bryce@bryceharrington.org>
 *
 * Copyright (C) 2005 Bryce Harrington
 *
 * Released under GNU GPL.  Read the file 'COPYING' for more information.
 */

#ifndef INKSCAPE_APPLICATION_APP_PROTOTYPE_H
#define INKSCAPE_APPLICATION_APP_PROTOTYPE_H

class Gtk::Window;


namespace Inkscape {
namespace NSApplication {

class AppPrototype
{
public:
    AppPrototype(int argc, const char **argv);
    virtual ~AppPrototype();

    Gtk::Window* getWindow();
    
protected:
    AppPrototype(AppPrototype const &);
    AppPrototype& operator=(AppPrototype const &);

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
  fill-column:75
  End:
*/
// vim: filetype=c++:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :
