/*
 * This is file is kind of the junk file.  Basically everything that
 * didn't fit in one of the other well defined areas, well, it's now
 * here.  Which is good in someways, but this file really needs some
 * definition.  Hopefully that will come ASAP.
 *
 * Authors:
 *   Ted Gould <ted@gould.cx>
 *
 * Copyright (C) 2002-2003 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifndef __MODULES_SYSTEM_H__
#define __MODULES_SYSTEM_H__

#include <gtk/gtk.h>
#include <document.h>
#include <extension/extension.h>


SPDocument * sp_module_system_open                (const gchar * key,
                                                   const gchar *  filename);
void         sp_module_system_save                (const gchar * key,
                                                   SPDocument *   doc,
                                                   const gchar *  filename);
void         sp_module_system_filter              (GtkObject * object,
                                                   const gchar *  key);
Inkscape::Extension::Print * sp_module_system_get_print (const gchar * key);

Inkscape::Extension::Extension *   sp_module_system_build_from_file     (const gchar  * filename, Inkscape::Extension::Implementation::Implementation * in_imp);
Inkscape::Extension::Extension *   sp_module_system_build_from_mem      (const gchar *  buffer, Inkscape::Extension::Implementation::Implementation * in_imp);

Inkscape::Extension::Extension *sp_module_system_get (const unsigned char *key);

void sp_module_system_menu_open (SPMenu *menu);
void sp_module_system_menu_save (SPMenu *menu);

#endif /* __MODULES_SYSTEM_H__ */

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
