#define __SP_MARSHAL_C__

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

#include "marshal.h"

typedef void (* SPSignal_NONE__DOUBLE_DOUBLE) (GtkObject *object, gdouble arg1, gdouble arg2, gpointer user_data);
typedef void (* SPSignal_NONE__STRING_BOOL) (GtkObject *object, gchar *arg1, gboolean arg2, gpointer user_data);

void
sp_marshal_NONE__DOUBLE_DOUBLE (GtkObject *object, GtkSignalFunc func, gpointer func_data, GtkArg *args)
{
	SPSignal_NONE__DOUBLE_DOUBLE rfunc;

	rfunc = (SPSignal_NONE__DOUBLE_DOUBLE) func;

	(* rfunc) (object, GTK_VALUE_DOUBLE (args[0]), GTK_VALUE_DOUBLE (args[1]), func_data);
}

void
sp_marshal_NONE__STRING_BOOL (GtkObject *object, GtkSignalFunc func, gpointer func_data, GtkArg *args)
{
	SPSignal_NONE__STRING_BOOL rfunc;

	rfunc = (SPSignal_NONE__STRING_BOOL) func;

	(* rfunc) (object, GTK_VALUE_STRING (args[0]), GTK_VALUE_BOOL (args[1]), func_data);
}


