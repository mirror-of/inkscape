#include "nr-type-pos-def.h"
#include <glib.h>
#include <string.h>

int
parse_name_for_weight (char const *c)
{
	gint weight;

	if (strcasestr (c, "thin")) {
		weight = NR_POS_WEIGHT_THIN;
	} else if (strcasestr (c, "extra light")) {
		weight = NR_POS_WEIGHT_EXTRA_LIGHT;
	} else if (strcasestr (c, "ultra light")) {
		weight = NR_POS_WEIGHT_ULTRA_LIGHT;
	} else if (strcasestr (c, "light")) {
		weight = NR_POS_WEIGHT_LIGHT;
	} else if (strcasestr (c, "book")) {
		weight = NR_POS_WEIGHT_BOOK;
	} else if (strcasestr (c, "medium")) {
		weight = NR_POS_WEIGHT_MEDIUM;
	} else if (strcasestr (c, "semi bold")) {
		weight = NR_POS_WEIGHT_SEMIBOLD;
	} else if (strcasestr (c, "semibold")) {
		weight = NR_POS_WEIGHT_SEMIBOLD;
	} else if (strcasestr (c, "demi bold")) {
		weight = NR_POS_WEIGHT_DEMIBOLD;
	} else if (strcasestr (c, "demibold") || strcasestr (c, "demi")) {
		weight = NR_POS_WEIGHT_DEMIBOLD;
	} else if (strcasestr (c, "ultra bold")) {
		weight = NR_POS_WEIGHT_ULTRA_BOLD;
	} else if (strcasestr (c, "extra bold") || strcasestr (c, "xbold") || strcasestr (c, "xtrabold")) {
		weight = NR_POS_WEIGHT_EXTRA_BOLD;
	} else if (strcasestr (c, "black") || strcasestr (c, "heavy")) {
		weight = NR_POS_WEIGHT_BLACK;
	} else if (strcasestr (c, "bold")) {
		/* Must come after the checks for `blah bold'. */
		weight = NR_POS_WEIGHT_BOLD;
	} else {
		weight = NR_POS_WEIGHT_NORMAL;
	}
	return weight;
}

int
parse_name_for_stretch (char const *c)
{
	gint stretch;

	if (strcasestr (c, "ultra narrow") || strcasestr (c, "ultra condensed") || strcasestr (c, "extra condensed")) {
		stretch = NR_POS_STRETCH_EXTRA_CONDENSED;
	} else if (strcasestr (c, "ultra wide") || strcasestr (c, "ultra expanded") || strcasestr (c, "ultra extended")  || strcasestr (c, "extra expanded")) {
		stretch = NR_POS_STRETCH_EXTRA_EXPANDED;
	} else if (strcasestr (c, "semi condensed") || strcasestr (c, "semicondensed")) {
		stretch = NR_POS_STRETCH_SEMI_CONDENSED;
	} else if (strcasestr (c, "semi extended") || strcasestr (c, "semiextended")) {
		stretch = NR_POS_STRETCH_SEMI_EXPANDED;
	} else if (strcasestr (c, "narrow") || strcasestr (c, "condensed")) {
		stretch = NR_POS_STRETCH_CONDENSED;
	} else if (strcasestr (c, "wide") || strcasestr (c, "expanded") || strcasestr (c, "extended")) {
		stretch = NR_POS_STRETCH_EXPANDED;
	} else {
		stretch = NR_POS_STRETCH_NORMAL;
	}
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
	/* copied from nr-type-directory.cpp:nr_type_calculate_position. */

	italic = (strcasestr (description, "italic") != NULL);
	oblique = (strcasestr (description, "oblique") != NULL);

	weight = parse_name_for_weight (description);

	stretch = parse_name_for_stretch (description);
}
