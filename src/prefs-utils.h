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

void prefs_set_int_attribute (gchar *path, gchar *attr, gint value);
gint prefs_get_int_attribute (gchar *path, gchar* attr, gint def);
gint prefs_get_int_attribute_limited (gchar *path, gchar* attr, gint def, gint min, gint max);

void prefs_set_double_attribute (gchar *path, gchar *attr, double value);
double prefs_get_double_attribute (gchar *path, gchar* attr, double def);
double prefs_get_double_attribute_limited (gchar *path, gchar* attr, double def, double min, double max);

