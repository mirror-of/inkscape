#include "nr-type-pos-def.h"
#include <glib.h>
#include <string.h>

NRTypePosDef::NRTypePosDef(char const *description) {
	/* copied from nr-type-directory.cpp:nr_type_calculate_position. */
	gchar *c = g_strdup(description);
	for (char *p = c ; *p != '\0' ; ++p) {
		*p = g_ascii_tolower(*p);
	}

	italic = (strstr (c, "italic") != NULL);
	oblique = (strstr (c, "oblique") != NULL);
	if (strstr (c, "thin")) {
		weight = 32;
	} else if (strstr (c, "extra light")) {
		weight = 64;
	} else if (strstr (c, "ultra light")) {
		weight = 64;
	} else if (strstr (c, "light")) {
		weight = 96;
	} else if (strstr (c, "book")) {
		weight = 128;
	} else if (strstr (c, "medium")) {
		weight = 144;
	} else if (strstr (c, "semi bold")) {
		weight = 160;
	} else if (strstr (c, "semibold")) {
		weight = 160;
	} else if (strstr (c, "demi bold")) {
		weight = 160;
	} else if (strstr (c, "demibold")) {
		weight = 160;
	} else if (strstr (c, "ultra bold")) {
		weight = 224;
	} else if (strstr (c, "extra bold")) {
		weight = 224;
	} else if (strstr (c, "black")) {
		weight = 255;
	} else if (strstr (c, "bold")) {
		/* Must come after the checks for `blah bold'. */
		weight = 192;
	} else {
		weight = 128;
	}

	if (strstr (c, "ultra narrow") || strstr (c, "ultra condensed") || strstr (c, "extra condensed")) {
		stretch = 48;
	} else if (strstr (c, "ultra wide") || strstr (c, "ultra expanded") || strstr (c, "ultra extended")  || strstr (c, "extra expanded")) {
		stretch = 228;
	} else if (strstr (c, "semi condensed") || strstr (c, "semicondensed")) {
		stretch = 108;
	} else if (strstr (c, "semi extended") || strstr (c, "semiextended")) {
		stretch = 148;
	} else if (strstr (c, "narrow") || strstr (c, "condensed")) {
		stretch = 88;
	} else if (strstr (c, "wide") || strstr (c, "expanded") || strstr (c, "extended")) {
		stretch = 168;
	} else {
		stretch = 128;
	}

	g_free(c);
}
