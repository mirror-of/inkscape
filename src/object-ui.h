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
