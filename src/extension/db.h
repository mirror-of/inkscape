/*
 * Functions to keep a listing of all modules in the system.  Has its
 * own file mostly for abstraction reasons, but is pretty simple
 * otherwise.
 *
 * Authors:
 *   Ted Gould <ted@gould.cx>
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * Copyright (C) 2002-2003 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifndef __MODULES_DB_H__
#define __MODULES_DB_H__

#include <extension/extension.h>

SPModule * sp_module_db_get (const gchar *key);
void sp_module_db_register (SPModule *module);
void sp_module_db_unregister (SPModule *module);
const gchar *sp_module_db_get_unique_id (gchar *c, int len, const gchar *val);
void sp_module_db_foreach (void (*in_func)(SPModule * in_plug, gpointer in_data), gpointer in_data);

#endif /* __MODULES_DB_H__ */

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
