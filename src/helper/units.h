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

#include <glib.h>

G_BEGIN_DECLS

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

typedef struct _SPUnit SPUnit;
typedef struct _SPDistance SPDistance;

/*
 * The base absolute unit is 1/72th of an inch (we are gnome PRINT, so sorry SI)
 */

enum {
	SP_UNIT_DIMENSIONLESS = (1 << 0), /* For percentages and like */
	SP_UNIT_ABSOLUTE = (1 << 1), /* Real world distances - i.e. mm, cm... */
	SP_UNIT_DEVICE = (1 << 2), /* Semi-real device-dependent distances i.e. pixels */
	SP_UNIT_USERSPACE = (1 << 3), /* Mathematical coordinates */
	SP_UNIT_VOLATILE = (1 << 4) /* em and ex */
} SPUnitBase;

/*
 * Notice, that for correct menus etc. you have to use
 * ngettext method family yourself. For that reason we
 * do not provide translations in unit names.
 * I also do not know, whether to allow user-created units,
 * because this would certainly confuse textdomain.
 */

struct _SPUnit {
	guint version : 8; /* Has to be 0 at moment */
	guint base : 8; /* Base */
	gdouble unittobase;
	/* I am not absolutely sure, but seems that gettext can do the magic */
	guchar *name;
	guchar *abbr;
	guchar *plural;
	guchar *abbr_plural;
};

struct _SPDistance {
	const SPUnit *unit;
	gdouble distance;
};

/* Base units are the ones used by gnome-print and paper descriptions */

#define SP_PS_UNIT (sp_unit_get_identity (SP_UNIT_ABSOLUTE))

const SPUnit *sp_unit_get_identity (guint base);
const SPUnit *sp_unit_get_default (void);
const SPUnit *sp_unit_get_by_name (const guchar *name);
const SPUnit *sp_unit_get_by_abbreviation (const guchar *abbreviation);

#define SP_UNITS_ALL (SP_UNIT_DIMENSIONLESS | SP_UNIT_ABSOLUTE | SP_UNIT_DEVICE | SP_UNIT_USERSPACE | SP_UNIT_VOLATILE)

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

G_END_DECLS

#endif 
