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

static const SPUnit sp_units[] = {
	/* Do not insert any elements before/between first 3 */
	{SP_UNIT_SCALE, SP_UNIT_DIMENSIONLESS, 1.0, N_("Unit"), "", N_("Units"), ""},
	{SP_UNIT_PT, SP_UNIT_ABSOLUTE, 1.0, N_("Point"), N_("pt"), N_("Points"), N_("Pt")},
	{SP_UNIT_PX, SP_UNIT_DEVICE, 1.0, N_("Pixel"), N_("px"), N_("Pixels"), N_("Px")},
	/* Volatiles do not have default, so there are none here */
	/* You can add new elements from this point forward */
	{SP_UNIT_PERCENT, SP_UNIT_DIMENSIONLESS, 0.01, N_("Percent"), N_("%"), N_("Percents"), N_("%")},
	{SP_UNIT_MM, SP_UNIT_ABSOLUTE, (72.0 / 25.4), N_("Millimeter"), N_("mm"), N_("Millimeters"), N_("mm")},
	{SP_UNIT_CM, SP_UNIT_ABSOLUTE, (72.0 / 2.54), N_("Centimeter"), N_("cm"), N_("Centimeters"), N_("cm")},
	{SP_UNIT_M, SP_UNIT_ABSOLUTE, (72.0 / 0.0254), N_("Meter"), N_("m"), N_("meters"), N_("m")},
	{SP_UNIT_IN, SP_UNIT_ABSOLUTE, (72.0), N_("Inch"), N_("in"), N_("Inches"), N_("in")},
	// TRANSLATORS: for info, see http://www.w3.org/TR/REC-CSS2/syndata.html#length-units
	{SP_UNIT_EM, SP_UNIT_VOLATILE, 1.0, N_("Em square"), N_("em"), N_("Em squares"), N_("em")},
	// TRANSLATORS: for info, see http://www.w3.org/TR/REC-CSS2/syndata.html#length-units
	{SP_UNIT_EX, SP_UNIT_VOLATILE, 1.0, N_("Ex square"), N_("ex"), N_("Ex squares"), N_("ex")},
};

#define sp_num_units G_N_ELEMENTS(sp_units)

/* Base units are the ones used by gnome-print and paper descriptions */

/* todo: Change param to SPUnitBase. */
const SPUnit *
sp_unit_get_identity (guint base)
{
	SPUnitId ret_id;
	switch (base) {
	case SP_UNIT_DIMENSIONLESS:
		ret_id = SP_UNIT_SCALE;
		break;
	case SP_UNIT_ABSOLUTE:
		ret_id = SP_UNIT_PT;
		break;
	case SP_UNIT_DEVICE:
		ret_id = SP_UNIT_PX;
		break;
	default:
		g_warning ("file %s: line %d: Illegal unit base 0x%x", __FILE__, __LINE__, base);
		return NULL;
		break;
	}
	SPUnit const &ret = sp_units[ret_id];
	g_assert(guint(ret.base) == base);
	g_assert(ret.unittobase == 1.0);
	return &ret;
}

/* fixme: */
const SPUnit *
sp_unit_get_default (void)
{
	return &sp_units[0];
}

const SPUnit *
sp_unit_get_by_name (const gchar *name)
{
	g_return_val_if_fail (name != NULL, NULL);

	for (unsigned i = 0 ; i < sp_num_units ; i++) {
		if (!g_strcasecmp (name, sp_units[i].name)) return &sp_units[i];
		if (!g_strcasecmp (name, sp_units[i].plural)) return &sp_units[i];
	}

	return NULL;
}

const SPUnit *
sp_unit_get_by_abbreviation (const gchar *abbreviation)
{
	g_return_val_if_fail (abbreviation != NULL, NULL);

	for (unsigned i = 0 ; i < sp_num_units ; i++) {
		if (!g_strcasecmp (abbreviation, sp_units[i].abbr)) return &sp_units[i];
		if (!g_strcasecmp (abbreviation, sp_units[i].abbr_plural)) return &sp_units[i];
	}

	return NULL;
}

GSList *
sp_unit_get_list (guint bases)
{
	g_return_val_if_fail ((bases & ~SP_UNITS_ALL) == 0, NULL);

	GSList *units = NULL;
	for (unsigned i = sp_num_units ; i--; ) {
		if (bases & sp_units[i].base) {
			units = g_slist_prepend (units, (gpointer) &sp_units[i]);
		}
	}

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

/** @param devicetransform for device units. */
/* TODO: Remove the ctmscale parameter given that we no longer have SP_UNIT_USERSPACE. */
gboolean
sp_convert_distance_full (gdouble *distance, const SPUnit *from, const SPUnit *to,
				   gdouble /*ctmscale*/, gdouble devicescale)
{
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

	gdouble absolute;
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
	sp_convert_distance (&val, distance->unit, &sp_units[SP_UNIT_PT]);

	return val;
}

gdouble
sp_units_get_points(gdouble const units, SPUnit const &unit)
{
	gdouble const ret = units * unit.unittobase;
	if (unit.base != SP_UNIT_ABSOLUTE) {
		g_warning("no exact unit conversion available");
	}
	return ret;
}

gdouble
sp_points_get_units(gdouble const points, SPUnit const &unit)
{
	gdouble const ret = points / unit.unittobase;
	if (unit.base != SP_UNIT_ABSOLUTE) {
		g_warning("no exact unit conversion available");
	}
	return ret;
}

bool
sp_units_table_sane()
{
	for (unsigned i = 0; i < G_N_ELEMENTS(sp_units); ++i) {
		if (unsigned(sp_units[i].unit_id) != i) {
			return false;
		}
	}
	return true;
}
