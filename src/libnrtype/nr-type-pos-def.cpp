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

	if (strstr (c, "narrow")) {
		stretch = 64;
	} else if (strstr (c, "condensed")) {
		stretch = 64;
	} else if (strstr (c, "wide")) {
		stretch = 192;
	} else {
		stretch = 128;
	}

	g_free(c);
}
