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

#include <config.h>

#include "inkscape.h"
#include "xml/repr.h"
#include "xml/repr-private.h"

#include "prefs-utils.h"

void
prefs_set_int_attribute (gchar const *path, gchar const *attr, gint value)
{
	SPRepr *repr;
	repr = inkscape_get_repr (INKSCAPE, path);
	if (repr) {
		sp_repr_set_int (repr, attr, value);
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
		sp_repr_set_double (repr, attr, value);
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

void
prefs_set_recent_file (const gchar * uri, const gchar * name) {
	int i;
    int max_documents = prefs_get_int_attribute ("options.maxrecentdocuments", "value", 20);

    if (uri != NULL) {
        SPRepr *recent;
        recent = inkscape_get_repr (INKSCAPE, "documents.recent");
        if (recent) {
            SPRepr *child;
            child = sp_repr_lookup_child (recent, "uri", uri);
            if (child) {
                sp_repr_change_order (recent, child, NULL);
            } else {
                if (sp_repr_n_children (recent) >= max_documents) {
                    child = recent->children;
                    // count to the last
                    for (i = 0; i < max_documents - 2; i ++) child = child->next;
                    // remove all after the last
                    while (child->next) sp_repr_unparent (child->next);
                }
                child = sp_repr_new ("document");
                sp_repr_set_attr (child, "uri", uri);
                sp_repr_add_child (recent, child, NULL);
            }
            sp_repr_set_attr (child, "name", name);
        }
    }

	return;
}

const gchar **
prefs_get_recent_files(void) {
	SPRepr *recent;

	recent = inkscape_get_repr (INKSCAPE, "documents.recent");

	if (recent) {
		SPRepr *child;
		const gchar ** datalst;
		gint i;
		gint docs = sp_repr_n_children (recent);

		datalst = (const gchar **)g_malloc(sizeof(gchar *) * (docs * 2) + 1);

		for (i = 0, child = recent->children;
				child != NULL;
				child = child->next, i += 2) {
			const gchar *uri, *name;
			uri = sp_repr_attr (child, "uri");
			name = sp_repr_attr (child, "name");

			datalst[i]     = uri;
			datalst[i + 1] = name;
		}

		datalst[i] = NULL;
		return datalst;
	}

	return NULL;
}
