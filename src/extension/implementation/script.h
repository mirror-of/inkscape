/*
 * Code for handling extensions (i.e., scripts)
 *
 * Authors:
 *   Bryce Harrington <bryce@osdl.org>
 *   Ted Gould <ted@gould.cx>
 *
 * Copyright (C) 2002-2004 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifndef __INKSCAPE_EXTENSION_IMPEMENTATION_SCRIPT_H__
#define __INKSCAPE_EXTENSION_IMPEMENTATION_SCRIPT_H__

#include "implementation.h"

namespace Inkscape {
namespace Extension {
namespace Implementation {

class Script : public Implementation {
private:
    gchar *       command;     /**< The command that has been dirived from
                                    the configuration file with appropriate
                                    directories */
    /** This function actually does the work, everything else is preparing
        for this function.  It is the core here */
    void          execute      (const gchar * command,
                                const gchar * filein,
                                const gchar * fileout);
    /** Just a quick function to find and resolve relative paths for
        the incoming scripts */
    gchar *       solve_reldir (SPRepr * reprin);

public:
                          Script       (void);
    virtual bool          load         (Inkscape::Extension::Extension * module);
    virtual void          unload       (Inkscape::Extension::Extension * module);
    virtual GtkDialog *   prefs        (Inkscape::Extension::Input * module,
                                        const gchar * filename);
    virtual SPDocument *  open         (Inkscape::Extension::Input * module,
                                        const gchar * filename);
    virtual GtkDialog *   prefs        (Inkscape::Extension::Output * module);
    virtual void          save         (Inkscape::Extension::Output * module,
                                        SPDocument * doc,
                                        const gchar * filename);
    virtual GtkDialog *   prefs        (Inkscape::Extension::Filter * module);
    virtual void          filter       (Inkscape::Extension::Filter * module,
                                        SPDocument * doc);

};

}; /* Inkscape  */
}; /* Extension  */
}; /* Implementation  */
#endif /* __INKSCAPE_EXTENSION_IMPEMENTATION_SCRIPT_H__ */

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
