#ifndef __SP_MARSHAL_H__
#define __SP_MARSHAL_H__

/*
 * Gtk+ signal marshallers
 *
 * Author:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * Copyright (C) 2001-2002 Lauris Kaplinski
 * Copyright (C) 2001 Ximian, Inc.
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <gtk/gtkobject.h>

void sp_marshal_NONE__DOUBLE_DOUBLE (GtkObject *object, GtkSignalFunc func, gpointer func_data, GtkArg *args);
void sp_marshal_NONE__STRING_BOOL (GtkObject *object, GtkSignalFunc func, gpointer func_data, GtkArg *args);

#endif
