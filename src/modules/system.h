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
#include <module.h>


SPDocument * sp_module_system_open                (const guchar * key,
                                                   const gchar *  filename);
void         sp_module_system_save                (const guchar * key,
                                                   SPDocument *   doc,
                                                   const gchar *  filename);
void         sp_module_system_filter              (GtkObject * object,
                                                   const gchar *  key);
SPModule *   sp_module_system_build_from_file     (const guchar * filename);
SPModule *   sp_module_system_build_from_mem      (const guchar * buffer);

#endif /* __MODULES_SYSTEM_H__ */
