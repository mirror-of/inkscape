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
#include "helper/units.h"
#include "helper/sp-intl.h"
#include "sp-metrics.h"
#include "unit-constants.h"

/* todo: use some fancy unit program */

/* The order determines the order of the list returned by sp_unit_get_list.
 * (It can also affect string lookups if there are any duplicates in the
 * current locale... hopefully none.)  If you re-order this list, then you must
 * also re-order the SPUnitId enum values accordingly.  Run `make check' (which
 * calls sp_unit_table_sane) to ensure that the two are in sync.
 */
SPUnit const sp_units[] = {
    {SP_UNIT_SCALE, SP_UNIT_DIMENSIONLESS, 1.0, NONE, N_("Unit"), "", N_("Units"), ""},
    {SP_UNIT_PT, SP_UNIT_ABSOLUTE, PT_PER_PT, SP_PT, N_("Point"), N_("pt"), N_("Points"), N_("Pt")},
    {SP_UNIT_PX, SP_UNIT_DEVICE, PT_PER_PX, SP_PX, N_("Pixel"), N_("px"), N_("Pixels"), N_("Px")}, 
    /* You can add new elements from this point forward */
    {SP_UNIT_PERCENT, SP_UNIT_DIMENSIONLESS, 0.01, NONE, N_("Percent"), N_("%"), N_("Percents"), N_("%")},
    {SP_UNIT_MM, SP_UNIT_ABSOLUTE, PT_PER_MM, SP_MM, N_("Millimeter"), N_("mm"), N_("Millimeters"), N_("mm")},
    {SP_UNIT_CM, SP_UNIT_ABSOLUTE, PT_PER_CM, SP_CM, N_("Centimeter"), N_("cm"), N_("Centimeters"), N_("cm")},
    {SP_UNIT_M, SP_UNIT_ABSOLUTE, PT_PER_M, SP_M, N_("Meter"), N_("m"), N_("Meters"), N_("m")},
    {SP_UNIT_IN, SP_UNIT_ABSOLUTE, PT_PER_IN, SP_IN, N_("Inch"), N_("in"), N_("Inches"), N_("in")},
    /* Volatiles do not have default, so there are none here */
    // TRANSLATORS: for info, see http://www.w3.org/TR/REC-CSS2/syndata.html#length-units
    {SP_UNIT_EM, SP_UNIT_VOLATILE, 1.0, NONE, N_("Em square"), N_("em"), N_("Em squares"), N_("em")},
    // TRANSLATORS: for info, see http://www.w3.org/TR/REC-CSS2/syndata.html#length-units
    {SP_UNIT_EX, SP_UNIT_VOLATILE, 1.0, NONE, N_("Ex square"), N_("ex"), N_("Ex squares"), N_("ex")},
};

#define sp_num_units G_N_ELEMENTS(sp_units)

SPUnit const *
sp_unit_get_by_abbreviation(gchar const *abbreviation)
{
    g_return_val_if_fail(abbreviation != NULL, NULL);

    for (unsigned i = 0 ; i < sp_num_units ; i++) {
        if (!g_strcasecmp(abbreviation, sp_units[i].abbr)) return &sp_units[i];
        if (!g_strcasecmp(abbreviation, sp_units[i].abbr_plural)) return &sp_units[i];
    }

    return NULL;
}

gchar const *
sp_unit_get_abbreviation(SPUnit const *unit)
{
    g_return_val_if_fail(unit != NULL, NULL);

    return unit->abbr;
}

gchar const *
sp_unit_get_plural (SPUnit const *unit)
{
    g_return_val_if_fail(unit != NULL, NULL);

    return unit->plural;
}

SPMetric
sp_unit_get_metric(SPUnit const *unit)
{
    g_return_val_if_fail(unit != NULL, NONE);

    return unit->metric;
}

GSList *
sp_unit_get_list(guint bases)
{
    g_return_val_if_fail((bases & ~SP_UNITS_ALL) == 0, NULL);

    GSList *units = NULL;
    for (unsigned i = sp_num_units ; i--; ) {
        if (bases & sp_units[i].base) {
            units = g_slist_prepend(units, (gpointer) &sp_units[i]);
        }
    }

    return units;
}

void
sp_unit_free_list(GSList *units)
{
    g_slist_free(units);
}

/* These are pure utility */
/* Return TRUE if conversion is possible */
gboolean
sp_convert_distance(gdouble *distance, SPUnit const *from, SPUnit const *to)
{
    g_return_val_if_fail(distance != NULL, FALSE);
    g_return_val_if_fail(from != NULL, FALSE);
    g_return_val_if_fail(to != NULL, FALSE);

    if (from == to) return TRUE;
    if ((from->base == SP_UNIT_DIMENSIONLESS) || (to->base == SP_UNIT_DIMENSIONLESS)) {
        *distance = *distance * from->unittobase / to->unittobase;
        return TRUE;
    }
    if ((from->base == SP_UNIT_VOLATILE) || (to->base == SP_UNIT_VOLATILE)) return FALSE;

    if ((from->base == to->base)
        || (from->base == SP_UNIT_DEVICE) && (to->base == SP_UNIT_ABSOLUTE)
        || (from->base == SP_UNIT_ABSOLUTE) && (to->base == SP_UNIT_DEVICE))
    {
        *distance = *distance * from->unittobase / to->unittobase;
        return TRUE;
    }

    return FALSE;
}

/** @param devicetransform for device units. */
/* TODO: Remove the ctmscale parameter given that we no longer have SP_UNIT_USERSPACE. */
gdouble
sp_convert_distance_full(gdouble const from_dist, SPUnit const &from, SPUnit const &to,
                         gdouble const devicescale)
{
    if (&from == &to) {
        return from_dist;
    }
    if (from.base == to.base) {
        gdouble ret = from_dist;
        bool const succ = sp_convert_distance(&ret, &from, &to);
        g_assert(succ);
        return ret;
    }
    if ((from.base == SP_UNIT_DIMENSIONLESS)
        || (to.base == SP_UNIT_DIMENSIONLESS))
    {
        return from_dist * from.unittobase / to.unittobase;
    }
    g_return_val_if_fail(((from.base != SP_UNIT_VOLATILE)
                          && (to.base != SP_UNIT_VOLATILE)),
                         from_dist);

    gdouble absolute;
    switch (from.base) {
        case SP_UNIT_ABSOLUTE:
        case SP_UNIT_DEVICE:
            absolute = from_dist * from.unittobase;
            break;
        default:
            g_warning("file %s: line %d: Illegal unit (base 0x%x)", __FILE__, __LINE__, from.base);
            return from_dist;
    }

    gdouble ret;
    switch (to.base) {
        default:
            g_warning("file %s: line %d: Illegal unit (base 0x%x)", __FILE__, __LINE__, to.base);
            /* FALL-THROUGH */
        case SP_UNIT_ABSOLUTE:
        case SP_UNIT_DEVICE:
            ret = absolute / to.unittobase;
            break;
    }

    return ret;
}

/* Some more convenience */
/* Be careful to not mix bases */

gdouble
sp_units_get_points(gdouble const units, SPUnit const &unit)
{
    if (unit.base == SP_UNIT_ABSOLUTE || unit.base == SP_UNIT_DEVICE) {
        return units * unit.unittobase;
    } else {
        g_warning("Different unit bases: No exact unit conversion available");
        return units * unit.unittobase;
    }
}

gdouble
sp_points_get_units(gdouble const points, SPUnit const &unit)
{
    if (unit.base == SP_UNIT_ABSOLUTE || unit.base == SP_UNIT_DEVICE) {
        return points / unit.unittobase;
    } else {
        g_warning("Different unit bases: No exact unit conversion available");
        return points / unit.unittobase;
    }
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


/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=99 :
