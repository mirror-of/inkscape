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
#include "xml/xml-forward.h"

namespace Inkscape {
namespace Extension {
namespace Implementation {

class Script : public Implementation {
private:
    gchar *       command;     /**< The command that has been dirived from
                                    the configuration file with appropriate
                                    directories */
    gchar *       helper_extension;
                               /**< This is the extension that will be used
                                    as the helper to read in or write out the
                                    data */
    /** This function actually does the work, everything else is preparing
        for this function.  It is the core here */
    void          execute      (gchar const *command,
                                gchar const *filein,
                                gchar const *fileout);
    /** Just a quick function to find and resolve relative paths for
        the incoming scripts */
    gchar *       solve_reldir (SPRepr *reprin);
    bool          check_existance (gchar const *command);

public:
                          Script       (void);
    virtual bool          load         (Inkscape::Extension::Extension *module);
    virtual void          unload       (Inkscape::Extension::Extension *module);
    virtual bool          check        (Inkscape::Extension::Extension *module);
    virtual GtkDialog *   prefs        (Inkscape::Extension::Input *module,
                                        gchar const *filename);
    virtual SPDocument *  open         (Inkscape::Extension::Input *module,
                                        gchar const *filename);
    virtual GtkDialog *   prefs        (Inkscape::Extension::Output *module);
    virtual void          save         (Inkscape::Extension::Output *module,
                                        SPDocument *doc,
                                        gchar const *filename);
    virtual GtkDialog *   prefs        (Inkscape::Extension::Effect *module);
    virtual void          effect       (Inkscape::Extension::Effect *module,
                                        SPDocument *doc);

};

}; /* Inkscape  */
}; /* Extension  */
}; /* Implementation  */
#endif /* __INKSCAPE_EXTENSION_IMPEMENTATION_SCRIPT_H__ */

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :
