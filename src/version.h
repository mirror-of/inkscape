#ifndef __VERSION_H__
#define __VERSION_H__

/*
 * Authors:
 *   MenTaLguY <mental@rydia.net>
 *
 * Copyright (C) 2003 MenTaLguY
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <glib.h>

#define SVG_VERSION "1.0"
#define SODIPODI_VERSION "0.32"

typedef struct _SPVersion {
	unsigned major, minor;
} SPVersion;

#define SP_VERSION_IS_ZERO (v) (!(v).major && !(v).minor)

#ifdef __cplusplus
extern "C" {
#endif

gboolean sp_version_from_string (const gchar *string, SPVersion *version);
gchar *sp_version_to_string (SPVersion version);
gboolean sp_version_inside_range (SPVersion version,
                                  unsigned major_min, unsigned minor_min,
                                  unsigned major_max, unsigned minor_max);

#ifdef __cplusplus
};
#endif

#endif
