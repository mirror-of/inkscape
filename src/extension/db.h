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

#include <glib.h>
#include <extension/extension.h>

namespace Inkscape {
namespace Extension {

class DB {
private:
	/** This is the actual database.  It has all of the modules in it,
		indexed by their ids.  It's a hash table for faster lookups */
    GHashTable *moduledict;

	static void foreach_internal (gpointer in_key, gpointer in_value, gpointer in_data);

public:
	DB (void);
	Extension * get (const gchar *key);
    void register_ext (Extension *module);
    void unregister_ext (Extension *module);
    const gchar * get_unique_id (gchar *c, int len, const gchar *val);
    void foreach (void (*in_func)(Extension * in_plug, gpointer in_data), gpointer in_data);
};

extern DB db;

}; }; /* namespace Extension, Inkscape */

#endif /* __MODULES_DB_H__ */

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
