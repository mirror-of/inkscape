#ifndef __SP_OBJECT_UI_H__
#define __SP_OBJECT_UI_H__

/*
 * Unser-interface related object extension
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * This code is in public domain
 */

#include <gtk/gtkmenu.h>

#include "forward.h"

/* Append object-specific part to context menu */

void sp_object_menu (SPObject *object, SPDesktop *desktop, GtkMenu *menu);

#endif

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
