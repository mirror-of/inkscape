/*
    Author:  Ted Gould <ted@gould.cx>
    Copyright (c) 2003-2004

    This code is licensed under the GNU GPL.  See COPYING for details.
 
    This file is the backend to the extensions system.  These are
    the parts of the system that most users will never see, but are
    important for implementing the extensions themselves.  This file
    contains the base class for all of that.
*/
#ifndef __INKSCAPE_EXTENSION_IMPLEMENTATION_H__
#define __INKSCAPE_EXTENSION_IMPLEMENTATION_H__

#include <gtk/gtkdialog.h>

namespace Inkscape {
namespace Extension {
namespace Implementation {
class Implementation;
};};};

#include <extension/extension.h>

namespace Inkscape {
namespace Extension {
namespace Implementation {

class Implementation {
    /** The implementation class is the base class for all implementations
        of modules.  This is whether they are done systematically by having
        something like the scripting system, or they are implemented internally
        they all derive from this class */
public:
    /* Basic functions for all Extension */
    virtual bool          load        (Inkscape::Extension::Extension * module);    /**< The function that should be called to load the module */
    virtual void          unload      (Inkscape::Extension::Extension * module);    /**< The function that should be called to unload the module */

    /* Input functions */
    virtual GtkDialog *   prefs       (Inkscape::Extension::Input * module,
                                       const gchar * filename);
                                       /**< The function to find out information about the file */
    virtual SPDocument *  open        (Inkscape::Extension::Input * module,
                                       const gchar * filename);
                                       /**< Hey, there needs to be some function to do the work! */

    /* Output functions */
    virtual GtkDialog *   prefs       (Inkscape::Extension::Output * module);
                                       /**< The function to find out information about the file */
    virtual void          save        (Inkscape::Extension::Output * module, SPDocument * doc, const gchar * filename);
                                       /**< Hey, there needs to be some function to do the work! */

    /* Effect functions */
    virtual GtkDialog *   prefs       (Inkscape::Extension::Effect * module);
                                       /**< The function to find out information about the file */
    /* TODO: need to figure out what we need here */
    virtual void          effect      (Inkscape::Extension::Effect * module,
                                       SPDocument * document);
                                       /**< Hey, there needs to be some function to do the work! */

    /* Print functions */
    virtual unsigned int  setup       (Inkscape::Extension::Print * module);
    virtual unsigned int  set_preview (Inkscape::Extension::Print * module);

    virtual unsigned int  begin       (Inkscape::Extension::Print * module,
                                       SPDocument *doc);
    virtual unsigned int  finish      (Inkscape::Extension::Print * module);

    /* Rendering methods */
    virtual unsigned int  bind        (Inkscape::Extension::Print * module,
                                       const NRMatrix *transform,
                                       float opacity);
    virtual unsigned int  release     (Inkscape::Extension::Print * module);
    virtual unsigned int  fill        (Inkscape::Extension::Print * module,
                                       const NRBPath *bpath,
                                       const NRMatrix *ctm,
                                       const SPStyle *style,
                                       const NRRect *pbox,
                                       const NRRect *dbox,
                                       const NRRect *bbox);
    virtual unsigned int  stroke      (Inkscape::Extension::Print * module,
                                       const NRBPath *bpath,
                                       const NRMatrix *transform,
                                       const SPStyle *style,
                                       const NRRect *pbox,
                                       const NRRect *dbox,
                                       const NRRect *bbox);
    virtual unsigned int  image       (Inkscape::Extension::Print * module,
                                       unsigned char *px,
                                       unsigned int w,
                                       unsigned int h,
                                       unsigned int rs,
                                       const NRMatrix *transform,
                                       const SPStyle *style);
};

}; /* namespace Implementation */
}; /* namespace Extension */
}; /* namespace Inkscape */

#endif /* __INKSCAPE_EXTENSION_IMPLEMENTATION_H__ */

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
