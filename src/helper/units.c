#define __SP_PAPER_C__

/*
 * SPUnit
 *
 * Ported from libgnomeprint
 *
 * Authors:
 *   Dirk Luetjens <dirk@luedi.oche.de>
 *   Yves Arrouye <Yves.Arrouye@marin.fdn.fr>
 *   Lauris Kaplinski <lauris@ximian.com>
 *
 * Copyright 1999-2001 Ximian, Inc. and authors
 *
 */

#include <config.h>

#include <math.h>
#include <string.h>
#include <glib.h>
#include "units.h"
#include "sp-intl.h"

/* fixme: use some fancy unit program */

/*
 * WARNING! Do not mess up with that - we use hardcoded numbers for base units!
 */

enum {
	SP_UNIT_BASE_DIMENSIONLESS,
	SP_UNIT_BASE_ABSOLUTE,
	SP_UNIT_BASE_DEVICE,
	SP_UNIT_BASE_USERSPACE
};

static const SPUnit sp_units[] = {
	/* Do not insert any elements before/between first 4 */
	{0, SP_UNIT_DIMENSIONLESS, 1.0, N_("Unit"), "", N_("Units"), ""},
	{0, SP_UNIT_ABSOLUTE, 1.0, N_("Point"), N_("pt"), N_("Points"), N_("Pt")},
	{0, SP_UNIT_DEVICE, 1.0, N_("Pixel"), N_("px"), N_("Pixels"), N_("Px")},
	{0, SP_UNIT_USERSPACE, 1.0, N_("Userspace unit"), N_("User"), N_("Userspace units"), N_("User")},
	/* Volatiles do not have default, so there are none here */
	/* You can add new elements from this point forward */
	{0, SP_UNIT_DIMENSIONLESS, 0.01, N_("Percent"), N_("%"), N_("Percents"), N_("%")},
	{0, SP_UNIT_ABSOLUTE, (72.0 / 25.4), N_("Millimeter"), N_("mm"), N_("Millimeters"), N_("mm")},
	{0, SP_UNIT_ABSOLUTE, (72.0 / 2.54), N_("Centimeter"), N_("cm"), N_("Centimeters"), N_("cm")},
	{0, SP_UNIT_ABSOLUTE, (72.0 / 0.0254), N_("Meter"), N_("m"), N_("meters"), N_("m")},
	{0, SP_UNIT_ABSOLUTE, (72.0), N_("Inch"), N_("in"), N_("Inches"), N_("in")},
	{0, SP_UNIT_VOLATILE, 1.0, N_("Em square"), N_("em"), N_("Em squares"), N_("em")},
	{0, SP_UNIT_VOLATILE, 1.0, N_("Ex square"), N_("ex"), N_("Ex squares"), N_("ex")},
};

#define sp_num_units (sizeof (sp_units) / sizeof (sp_units[0]))

/* Base units are the ones used by gnome-print and paper descriptions */

const SPUnit *
sp_unit_get_identity (guint base)
{
	switch (base) {
	case SP_UNIT_DIMENSIONLESS:
		return &sp_units[SP_UNIT_BASE_DIMENSIONLESS];
		break;
	case SP_UNIT_ABSOLUTE:
		return &sp_units[SP_UNIT_BASE_ABSOLUTE];
		break;
	case SP_UNIT_DEVICE:
		return &sp_units[SP_UNIT_BASE_DEVICE];
		break;
	case SP_UNIT_USERSPACE:
		return &sp_units[SP_UNIT_BASE_USERSPACE];
		break;
	default:
		g_warning ("file %s: line %d: Illegal unit base %d", __FILE__, __LINE__, base);
		return NULL;
		break;
	}
}

/* fixme: */
const SPUnit *
sp_unit_get_default (void)
{
	return &sp_units[0];
}

const SPUnit *
sp_unit_get_by_name (const guchar *name)
{
	gint i;

	g_return_val_if_fail (name != NULL, NULL);

	for (i = 0; i < sp_num_units; i++) {
#ifndef WIN32
		if (!strcasecmp (name, sp_units[i].name)) return &sp_units[i];
		if (!strcasecmp (name, sp_units[i].plural)) return &sp_units[i];
#else
		if (!stricmp (name, sp_units[i].name)) return &sp_units[i];
		if (!stricmp (name, sp_units[i].plural)) return &sp_units[i];
#endif
	}

	return NULL;
}

const SPUnit *
sp_unit_get_by_abbreviation (const guchar *abbreviation)
{
	gint i;

	g_return_val_if_fail (abbreviation != NULL, NULL);

	for (i = 0; i < sp_num_units; i++) {
#ifndef WIN32
		if (!strcasecmp (abbreviation, sp_units[i].abbr)) return &sp_units[i];
		if (!strcasecmp (abbreviation, sp_units[i].abbr_plural)) return &sp_units[i];
#else
		if (!stricmp (abbreviation, sp_units[i].abbr)) return &sp_units[i];
		if (!stricmp (abbreviation, sp_units[i].abbr_plural)) return &sp_units[i];
#endif
	}

	return NULL;
}

GSList *
sp_unit_get_list (guint bases)
{
	GSList *units;
	gint i;

	g_return_val_if_fail ((bases & ~SP_UNITS_ALL) == 0, NULL);

	units = NULL;

	for (i = 0; i < sp_num_units; i++) {
		if (bases & sp_units[i].base) {
			units = g_slist_prepend (units, (gpointer) &sp_units[i]);
		}
	}

	units = g_slist_reverse (units);

	return units;
}

void
sp_unit_free_list (GSList *units)
{
	g_slist_free (units);
}

/* These are pure utility */
/* Return TRUE if conversion is possible */
gboolean
sp_convert_distance (gdouble *distance, const SPUnit *from, const SPUnit *to)
{
	g_return_val_if_fail (distance != NULL, FALSE);
	g_return_val_if_fail (from != NULL, FALSE);
	g_return_val_if_fail (to != NULL, FALSE);

	if (from == to) return TRUE;
	if ((from->base == SP_UNIT_DIMENSIONLESS) || (to->base == SP_UNIT_DIMENSIONLESS)) {
		*distance = *distance * from->unittobase / to->unittobase;
		return TRUE;
	}
	if (from->base != to->base) return FALSE;
	if ((from->base == SP_UNIT_VOLATILE) || (to->base == SP_UNIT_VOLATILE)) return FALSE;

	*distance = *distance * from->unittobase / to->unittobase;

	return TRUE;
}

/* ctm is for userspace, devicetransform is for device units */
gboolean
sp_convert_distance_full (gdouble *distance, const SPUnit *from, const SPUnit *to,
				   gdouble ctmscale, gdouble devicescale)
{
	gdouble absolute;

	g_return_val_if_fail (distance != NULL, FALSE);
	g_return_val_if_fail (from != NULL, FALSE);
	g_return_val_if_fail (to != NULL, FALSE);

	if (from == to) return TRUE;
	if (from->base == to->base) return sp_convert_distance (distance, from, to);
	if ((from->base == SP_UNIT_DIMENSIONLESS) || (to->base == SP_UNIT_DIMENSIONLESS)) {
		*distance = *distance * from->unittobase / to->unittobase;
		return TRUE;
	}
	if ((from->base == SP_UNIT_VOLATILE) || (to->base == SP_UNIT_VOLATILE)) return FALSE;

	switch (from->base) {
	case SP_UNIT_ABSOLUTE:
		absolute = *distance * from->unittobase;
		break;
	case SP_UNIT_DEVICE:
		if (devicescale) {
			absolute = *distance * from->unittobase * devicescale;
		} else {
			return FALSE;
		}
		break;
	case SP_UNIT_USERSPACE:
		if (ctmscale) {
			absolute = *distance * from->unittobase * ctmscale;
		} else {
			return FALSE;
		}
		break;
	default:
		g_warning ("file %s: line %d: Illegal unit (base %d)", __FILE__, __LINE__, from->base);
		return FALSE;
		break;
	}

	switch (to->base) {
	case SP_UNIT_DIMENSIONLESS:
	case SP_UNIT_ABSOLUTE:
		*distance = absolute / to->unittobase;
		break;
	case SP_UNIT_DEVICE:
		if (devicescale) {
			*distance = absolute / (to->unittobase * devicescale);
		} else {
			return FALSE;
		}
		break;
	case SP_UNIT_USERSPACE:
		if (ctmscale) {
			*distance = absolute / (to->unittobase * ctmscale);
		} else {
			return FALSE;
		}
		break;
	default:
		g_warning ("file %s: line %d: Illegal unit (base %d)", __FILE__, __LINE__, to->base);
		return FALSE;
		break;
	}

	return TRUE;
}

/* Some more convenience */
/* Be careful to not mix bases */

gdouble
sp_distance_get_units (SPDistance *distance, const SPUnit *unit)
{
	gdouble val;

	g_return_val_if_fail (distance != NULL, 0.0);
	g_return_val_if_fail (unit != NULL, 0.0);

	val = distance->distance;
	sp_convert_distance (&val, distance->unit, unit);

	return val;
}

gdouble
sp_distance_get_points (SPDistance *distance)
{
	gdouble val;

	g_return_val_if_fail (distance != NULL, 0.0);

	val = distance->distance;
	sp_convert_distance (&val, distance->unit, &sp_units[SP_UNIT_BASE_ABSOLUTE]);

	return val;
}

gdouble
sp_points_get_units (gdouble points, const SPUnit *unit)
{
	sp_convert_distance (&points, &sp_units[SP_UNIT_BASE_ABSOLUTE], unit);

	return points;
}

gdouble
sp_units_get_points (gdouble units, const SPUnit *unit)
{
	sp_convert_distance (&units, unit, &sp_units[SP_UNIT_BASE_ABSOLUTE]);

	return units;
}

