#define __VERSION_C__

/*
 * Versions
 *
 * Authors:
 *   MenTaLguY <mental@rydia.net>
 *
 * Copyright (C) 2003 MenTaLguY
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "version.h"

gboolean
sp_version_from_string(const gchar *string, SPVersion *version)
{
	int result;
	if (!string) return FALSE;

	version->major = 0;
	version->minor = 0;

	return sscanf((const char *)string, "%u.%u",
	              &version->major, &version->minor) ||
	       sscanf((const char *)string, "%u", &version->major);
}

gchar *
sp_version_to_string(SPVersion version)
{
	return g_strdup_printf("%u.%u", version.major, version.minor);
}

gboolean
sp_version_inside_range(SPVersion version,
                        unsigned major_min, unsigned minor_min,
                        unsigned major_max, unsigned minor_max)
{
	if ( version.major < major_min || version.major > major_max ) {
		return FALSE;
	} else if ( version.major == major_min &&
	            version.minor <= minor_min )
	{
		return FALSE;
	} else if ( version.major == major_max &&
	            version.minor >= minor_max )
	{
		return FALSE;
	} else {
		return TRUE;
	}
}

