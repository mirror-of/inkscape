#include "nr-type-pos-def.h"
#include <glib.h>
#include <string.h>

int
parse_name_for_weight (char const *cc)
{
	gchar *c = g_strdup(cc);
	for (char *p = c ; *p != '\0' ; ++p) {
		*p = g_ascii_tolower(*p);
	}

	gint weight;
	if (strstr (c, "thin")) {
		weight = NR_POS_WEIGHT_THIN;
	} else if (strstr (c, "extra light")) {
		weight = NR_POS_WEIGHT_EXTRA_LIGHT;
	} else if (strstr (c, "ultra light")) {
		weight = NR_POS_WEIGHT_ULTRA_LIGHT;
	} else if (strstr (c, "light")) {
		weight = NR_POS_WEIGHT_LIGHT;
	} else if (strstr (c, "book")) {
		weight = NR_POS_WEIGHT_BOOK;
	} else if (strstr (c, "medium")) {
		weight = NR_POS_WEIGHT_MEDIUM;
	} else if (strstr (c, "semi bold")) {
		weight = NR_POS_WEIGHT_SEMIBOLD;
	} else if (strstr (c, "semibold")) {
		weight = NR_POS_WEIGHT_SEMIBOLD;
	} else if (strstr (c, "demi bold")) {
		weight = NR_POS_WEIGHT_DEMIBOLD;
	} else if (strstr (c, "demibold") || strstr (c, "demi")) {
		weight = NR_POS_WEIGHT_DEMIBOLD;
	} else if (strstr (c, "ultra bold")) {
		weight = NR_POS_WEIGHT_ULTRA_BOLD;
	} else if (strstr (c, "extra bold") || strstr (c, "xbold") || strstr (c, "xtrabold")) {
		weight = NR_POS_WEIGHT_EXTRA_BOLD;
	} else if (strstr (c, "black") || strstr (c, "heavy")) {
		weight = NR_POS_WEIGHT_BLACK;
	} else if (strstr (c, "bold")) {
		/* Must come after the checks for `blah bold'. */
		weight = NR_POS_WEIGHT_BOLD;
	} else {
		weight = NR_POS_WEIGHT_NORMAL;
	}

	g_free (c);
	return weight;
}

int
parse_name_for_stretch (char const *cc)
{
	gchar *c = g_strdup(cc);
	for (char *p = c ; *p != '\0' ; ++p) {
		*p = g_ascii_tolower(*p);
	}

	gint stretch;
	if (strstr (c, "ultra narrow") || strstr (c, "ultra condensed") || strstr (c, "extra condensed")) {
		stretch = NR_POS_STRETCH_EXTRA_CONDENSED;
	} else if (strstr (c, "ultra wide") || strstr (c, "ultra expanded") || strstr (c, "ultra extended")  || strstr (c, "extra expanded")) {
		stretch = NR_POS_STRETCH_EXTRA_EXPANDED;
	} else if (strstr (c, "semi condensed") || strstr (c, "semicondensed")) {
		stretch = NR_POS_STRETCH_SEMI_CONDENSED;
	} else if (strstr (c, "semi extended") || strstr (c, "semiextended")) {
		stretch = NR_POS_STRETCH_SEMI_EXPANDED;
	} else if (strstr (c, "narrow") || strstr (c, "condensed")) {
		stretch = NR_POS_STRETCH_CONDENSED;
	} else if (strstr (c, "wide") || strstr (c, "expanded") || strstr (c, "extended")) {
		stretch = NR_POS_STRETCH_EXPANDED;
	} else {
		stretch = NR_POS_STRETCH_NORMAL;
	}

	g_free (c);
	return stretch;
}

const char *
weight_to_css (int weight)
{
	switch (weight) {
	case NR_POS_WEIGHT_THIN:
		return "100";
		break;
  case NR_POS_WEIGHT_EXTRA_LIGHT:
		return "200";
		break;
  case NR_POS_WEIGHT_LIGHT:
		return "300";
		break;
  case NR_POS_WEIGHT_BOOK:
		return "normal";
		break;
  case NR_POS_WEIGHT_MEDIUM:
		return "500";
		break;
  case NR_POS_WEIGHT_SEMIBOLD:
		return "600";
		break;
  case NR_POS_WEIGHT_BOLD:
		return "bold";
		break;
  case NR_POS_WEIGHT_EXTRA_BOLD:
		return "800";
		break;
	case NR_POS_WEIGHT_BLACK:
		return "900";
		break;
	default:
		break;
	}
	return NULL;
}

const char *
stretch_to_css (int stretch)
{
	switch (stretch) {
	case NR_POS_STRETCH_EXTRA_CONDENSED:
		return "extra-condensed";
		break;
	case NR_POS_STRETCH_CONDENSED:
		return "condensed";
		break;
	case NR_POS_STRETCH_SEMI_CONDENSED:
		return "semi-condensed";
		break;
	case NR_POS_STRETCH_NORMAL:
		return "normal";
		break;
	case NR_POS_STRETCH_SEMI_EXPANDED:
		return "semi-expanded";
		break;
	case NR_POS_STRETCH_EXPANDED:
		return "expanded";
		break;
	case NR_POS_STRETCH_EXTRA_EXPANDED:
		return "extra-expanded";
		break;
	default:
		break;
	}
	return NULL;
}


NRTypePosDef::NRTypePosDef(char const *description) {
	// we cannot use strcasestr, it's linux only... so we must lowercase the string first
	gchar *c = g_strdup(description);
	for (char *p = c ; *p != '\0' ; ++p) {
		*p = g_ascii_tolower(*p);
	}

	/* copied from nr-type-directory.cpp:nr_type_calculate_position. */

	italic = (strstr (c, "italic") != NULL);
	oblique = (strstr (c, "oblique") != NULL);

	weight = parse_name_for_weight (c);

	stretch = parse_name_for_stretch (c);

	g_free (c);
}
