/*
 * Utility functions for reading and setting preferences
 *
 * Authors:
 *   bulia byak <bulia@dr.com>
 *
 * Copyright (C) 2003 authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <string.h>

#include "select-context.h"
#include "node-context.h"
#include "rect-context.h"
#include "arc-context.h"
#include "star-context.h"
#include "spiral-context.h"
#include "draw-context.h"
#include "dyna-draw-context.h"
#include "text-context.h"
#include "zoom-context.h"
#include "dropper-context.h"

#include "inkscape-private.h"
#include "file.h"
#include "document.h"
#include "desktop.h"
#include "selection.h"
#include "selection-chemistry.h"
#include "path-chemistry.h"
#include "shortcuts.h"
#include "verbs.h"

#include "tools-switch.h"

void
prefs_set_int_attribute (gchar const *path, gchar const *attr, gint value)
{
	SPRepr *repr;
	repr = inkscape_get_repr (INKSCAPE, path);
	if (repr) {
		sp_repr_set_int_attribute (repr, attr, value);
	}
}

gint
prefs_get_int_attribute (gchar const *path, gchar const *attr, gint def)
{
	SPRepr *repr;
	repr = inkscape_get_repr (INKSCAPE, path);
	if (repr) {
		return sp_repr_get_int_attribute (repr, attr, def);
	} else 
		return def;
}

/**
\brief Retrieves an int attribute guarding against screwed-up data; if the value is beyond limits, default is returned
*/
gint
prefs_get_int_attribute_limited (gchar const *path, gchar const *attr, gint def, gint min, gint max)
{
	gint v; 

	SPRepr *repr;
	repr = inkscape_get_repr (INKSCAPE, path);
	if (repr) {
		v = sp_repr_get_int_attribute (repr, attr, def);
		if (v >= min && v <= max)
			return v;
		else
			return def;
	} else 
		return def;
}

void
prefs_set_double_attribute (gchar const *path, gchar const *attr, double value)
{
	SPRepr *repr;
	repr = inkscape_get_repr (INKSCAPE, path);
	if (repr) {
		sp_repr_set_double_attribute (repr, attr, value);
	}
}

double
prefs_get_double_attribute (gchar const *path, gchar const *attr, double def)
{
	SPRepr *repr;
	repr = inkscape_get_repr (INKSCAPE, path);
	if (repr) {
		return sp_repr_get_double_attribute (repr, attr, def);
	} else 
		return def;
}

/**
\brief Retrieves an int attribute guarding against screwed-up data; if the value is beyond limits, default is returned
*/
double
prefs_get_double_attribute_limited (gchar const *path, gchar const *attr, double def, double min, double max)
{
	double v; 

	SPRepr *repr;
	repr = inkscape_get_repr (INKSCAPE, path);
	if (repr) {
		v = sp_repr_get_double_attribute (repr, attr, def);
		if (v >= min && v <= max)
			return v;
		else
			return def;
	} else 
		return def;
}

gchar const *
prefs_get_string_attribute (gchar const *path, gchar const *attr)
{
	SPRepr *repr;
	repr = inkscape_get_repr (INKSCAPE, path);
	if (repr) {
		return (char *) sp_repr_attr (repr, attr);
	}
	return NULL;
}

void
prefs_set_string_attribute (gchar const *path, gchar const *attr, gchar const *value)
{
	SPRepr *repr;
	repr = inkscape_get_repr (INKSCAPE, path);
	if (repr) {
		sp_repr_set_attr (repr, attr, value);
	}
}
