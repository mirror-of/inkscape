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

    /* Lists for UI stuff */
    /** A class that holds all the data to create a fun menu of
        the IO extensions.  It only has a constructor and destructor
        to make it clean itself up nicely */
    class IOExtensionDescription {
    public:
        const gchar *     name;           /**< Name of the extension */
        const gchar *     file_extension; /**< Extension of file for this extension */
        const gchar *     mimetype;       /**< MIME type of file */
        const Extension * extension;      /**< Key used to pass back to the extension system */

        IOExtensionDescription   (const gchar *      in_name,
                                  const gchar *      in_file_extension,
                                  const gchar *      in_mime,
                                  const Extension *  in_extension);
        ~IOExtensionDescription(void);
    };

private:
    static void input_internal (Extension * in_plug, gpointer data);
    static void output_internal (Extension * in_plug, gpointer data);

public:
    GSList *  get_input_list (void);
    GSList *  get_output_list (void);
    void      free_list (GSList * in_list);

}; /* class DB */

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
