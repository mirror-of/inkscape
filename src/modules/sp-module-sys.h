#ifndef __SODIPODI_MODULE_SYS_H__
#define __SODIPODI_MODULE_SYS_H__

#include <gtk/gtk.h>

#include "document.h"
#include "sp-module-doc.h"

void         sp_modulesys_init        (void);

SPModule *   sp_modulesys_list_add    (SPModule *     in_module);
GtkMenu *    sp_modulesys_menu_about  (void);
GtkMenu *    sp_modulesys_menu_save   (void);

/* GtkMenu *    sp_modulesys_menu_open   (void); */

GtkMenu *    sp_modulesys_menu_filter (void);
SPDocument * sp_modulesys_do_open     (SPModule *     object,
                                       SPModuleDoc *  doc);
void         sp_modulesys_do_save     (SPModule *     object,
                                       SPModuleDoc *  doc);
void         sp_modulesys_do_filter   (gpointer       object,
                                       gpointer       in_module);
SPModule *   sp_modulesys_get_open_module (void);
SPModule *   sp_modulesys_get_save_module (void);

void sp_module_sys_prefs_complete (SPModule * object, SPModuleDoc * doc, gboolean success);


#endif /* __SODIPODI_MODULE_SYS_H__ */
