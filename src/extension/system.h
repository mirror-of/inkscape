/*
 * This is file is kind of the junk file.  Basically everything that
 * didn't fit in one of the other well defined areas, well, it's now
 * here.  Which is good in someways, but this file really needs some
 * definition.  Hopefully that will come ASAP.
 *
 * Authors:
 *   Ted Gould <ted@gould.cx>
 *
 * Copyright (C) 2002-2004 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifndef INKSCAPE_EXTENSION_SYSTEM_H__
#define INKSCAPE_EXTENSION_SYSTEM_H__

#include <gtk/gtk.h>
#include <document.h>
#include <extension/extension.h>

namespace Inkscape {
namespace Extension {

SPDocument *  open                (Extension * key,
                                   const gchar *  filename);
void          save                (Extension * key,
                                   SPDocument *   doc,
                                   const gchar *  filename,
                                   bool setextension,
                                   bool check_overwrite,
                                   bool official);
void          filter              (GtkObject * object,
                                   const gchar *  key);
Print *       get_print           (const gchar * key);
Extension *   build_from_file     (const gchar  * filename,
                                   Implementation::Implementation * in_imp);
Extension *   build_from_mem      (const gchar *  buffer,
                                   Implementation::Implementation * in_imp);

}; }; /* namespace Inkscape::Extension */

#endif /* INKSCAPE_EXTENSION_SYSTEM_H__ */

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
