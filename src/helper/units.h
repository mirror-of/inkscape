#ifndef __SP_UNIT_H__
#define __SP_UNIT_H__

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

#include <glib/gslist.h>
#include <glib/gtypes.h>



/*
 * Units and conversion methods used by libgnomeprint.
 *
 * You need those for certain config keys (like paper size), if you are
 * interested in using these (look at gnome-print-config.h for discussion,
 * why you may NOT be interested in paper size).
 *
 * Unit bases define set of mutually unrelated measuring systems (numbers,
 * paper, screen and dimesionless user coordinates). Still, you can convert
 * between those, specifying scaling factors explicitly.
 *
 * Paper (i.e. output) coordinates are taken as absolute real world units.
 * It has some justification, because screen unit (pixel) size changes,
 * if you change screen resolution, while you cannot change output on paper
 * as easily (unless you have thermally contracting paper, of course).
 *
 */

struct SPUnit;
struct SPDistance;

/*
 * The base linear ("absolute") unit is 1/72th of an inch, i.e. the base unit of postscript.
 */

/*
 * Unit bases
 */
enum SPUnitBase {
	SP_UNIT_DIMENSIONLESS = (1 << 0), /* For percentages and like */
	SP_UNIT_ABSOLUTE = (1 << 1), /* Real world distances - i.e. mm, cm... */
	SP_UNIT_DEVICE = (1 << 2), /* Pixels in the SVG/CSS sense. */
	SP_UNIT_VOLATILE = (1 << 3) /* em and ex */
};

/*
 * Units: indexes into sp_units.
 */
enum SPUnitId {
	SP_UNIT_SCALE,	// 1.0 == 100%
	SP_UNIT_PT,	// Postscript points: exactly 72 per inch
	SP_UNIT_PX,	// "Pixels" in the CSS sense; though Inkscape assumes a constant 90 per inch.
	SP_UNIT_PERCENT,  /* Note: In Inkscape this often means "relative to current value" (for
			     users to edit a value), rather than the SVG/CSS use of percentages. */
	SP_UNIT_MM,	// millimetres
	SP_UNIT_CM,	// centimetres
	SP_UNIT_M,	// metres
	SP_UNIT_IN,	// inches
	SP_UNIT_EM,	// em of current font
	SP_UNIT_EX	// x-height of current font
};

/*
 * Notice, that for correct menus etc. you have to use
 * ngettext method family yourself. For that reason we
 * do not provide translations in unit names.
 * I also do not know, whether to allow user-created units,
 * because this would certainly confuse textdomain.
 */

struct SPUnit {
	SPUnitId unit_id; /* used as sanity check */
	SPUnitBase base;
	gdouble unittobase; /* how many base units in this unit */
	/* I am not absolutely sure, but seems that gettext can do the magic */
	gchar const *name;
	gchar const *abbr;
	gchar const *plural;
	gchar const *abbr_plural;
};

struct SPDistance {
	const SPUnit *unit;
	gdouble distance;
};

/* Base units are the ones used by gnome-print and paper descriptions */

#define SP_PS_UNIT (sp_unit_get_identity (SP_UNIT_ABSOLUTE))

const SPUnit *sp_unit_get_identity (guint base);
const SPUnit *sp_unit_get_default (void);
const SPUnit *sp_unit_get_by_name (const gchar *name);
const SPUnit *sp_unit_get_by_abbreviation (const gchar *abbreviation);

#define SP_UNITS_ALL (SP_UNIT_DIMENSIONLESS | SP_UNIT_ABSOLUTE | SP_UNIT_DEVICE | SP_UNIT_VOLATILE)

GSList *sp_unit_get_list (guint bases);
void sp_unit_free_list (GSList *units);

/* These are pure utility */
/* Return TRUE if conversion is possible, FALSE if unit bases differ */
gboolean sp_convert_distance (gdouble *distance, const SPUnit *from, const SPUnit *to);

/* ctmscale is userspace->absolute, devicescale is device->absolute */
/* If either one is NULL, transconverting to/from that base fails */
/* Generic conversion between volatile units would be useless anyways */
gboolean sp_convert_distance_full (gdouble *distance, const SPUnit *from, const SPUnit *to, gdouble ctmscale, gdouble devicescale);

/* Some more convenience */
/* Be careful to not mix bases */

gdouble sp_distance_get_units (SPDistance *distance, const SPUnit *unit);
gdouble sp_distance_get_points (SPDistance *distance);

gdouble sp_points_get_units (gdouble points, const SPUnit *unit);
gdouble sp_units_get_points (gdouble units, const SPUnit *unit);



#endif 
