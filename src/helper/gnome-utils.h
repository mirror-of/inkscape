/*
 * GNOME Utils - Migration helper
 *
 * Author:
 *   GNOME Developer
 *   Mitsuru Oka <oka326@parkcity.ne.jp>
 *   Lauris Kaplinski <lauris@ximian.com>
 *
 * Copyright (C) 2001 Lauris Kaplinski and Ximian, Inc.
 *
 * Released under GNU GPL
 */


#ifndef __GNOME_UTILS_H__
#define __GNOME_UTILS_H__

GList *gnome_uri_list_extract_uris (const gchar* uri_list);

GList *gnome_uri_list_extract_filenames (const gchar* uri_list);

#endif /* __GNOME_UTILS_H__ */

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
