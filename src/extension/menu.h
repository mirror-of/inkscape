/*
 * This code builds all of the menus for the modules.  It creates them
 * so that calling code doesn't have to know that much about the menus
 * at all.  Really kinda simple, makes good use of the db functions.
 *
 * Authors:
 *   Ted Gould <ted@gould.cx>
 *
 * Copyright (C) 2002-2003 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifndef __MODULES_MENU_H__
#define __MODULES_MENU_H__

#include <widgets/menu.h>

SPMenu *    sp_module_menu_open    (void);
SPMenu *    sp_module_menu_save    (void);
SPMenu *    sp_module_menu_about   (void);
GtkWidget * sp_module_menu_filter  (void);

#endif /* __MODULES_MENU_H__ */

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0))
  indent-tabs-mode:nil
  fill-column:99
  End:
  vim: filetype=c++:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :
*/
