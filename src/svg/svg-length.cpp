#define __SP_SVG_LENGTH_C__

/*
 * SVG data parser
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * Copyright (C) 1999-2002 Lauris Kaplinski
 *
 * This code is in public domain
 */
#include <config.h>

#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>

#include "svg.h"

#include <glib.h>

#ifndef MAX
#define MAX(a,b) ((a < b) ? (b) : (a))
#endif

unsigned int
sp_svg_boolean_read (const gchar *str, unsigned int *val)
{
	unsigned int v;
	char *e;
	if (!val) return 0;
	if (!g_strcasecmp (str, "true") || !g_strcasecmp (str, "yes")) {
		*val = 1;
		return 1;
	}
	if (!g_strcasecmp (str, "false") || !g_strcasecmp (str, "no")) {
		*val = 0;
		return 1;
	}
	v = strtoul (str, &e, 10);
	if ((const gchar *) e != str) {
		*val = v;
		return 1;
	}
	return 0;
}

unsigned int
sp_svg_number_read_f (const gchar *str, float *val)
{
	char *e;
	float v;

	if (!str) return 0;
	v = strtod (str, &e);
	if ((const gchar *) e == str) return 0;
	*val = v;
	return 1;
}

unsigned int
sp_svg_number_read_d (const gchar *str, double *val)
{
	char *e;
	double v;

	if (!str) return 0;
	v = strtod (str, &e);
	if ((const gchar *) e == str) return 0;
	*val = v;
	return 1;
}

unsigned int
sp_svg_number_write_i (gchar *buf, int val)
{
	char c[32];
	int p, i;
	p = 0;
	if (val < 0) {
		buf[p++] = '-';
		val = -val;
	}
	i = 0;
	do {
		c[32 - (++i)] = '0' + (val % 10);
		val /= 10;
	} while (val > 0);
	memcpy (buf + p, &c[32 - i], i);
	p += i;
	buf[p] = 0;
	return p;
}

unsigned int
sp_svg_number_write_d (gchar *buf, double val, unsigned int tprec, unsigned int fprec, unsigned int padf)
{
	double dival, fval;
	int idigits, ival, i;
	i = 0;
	/* Process sign */
	if (val < 0.0) {
		buf[i++] = '-';
		val = fabs (val);
	}
	/* Determine number of integral digits */
	if (val >= 1.0) {
		idigits = (int) floor (log10 (val));
	} else {
		idigits = 0;
	}
	/* Determine the actual number of fractional digits */
	fprec = MAX (fprec, tprec - idigits);
	/* Round value */
	val += 0.5 * pow (10.0, - ((double) fprec));
	/* Extract integral and fractional parts */
	dival = floor (val);
	ival = (int) dival;
	fval = val - dival;
	/* Write integra */
	i += sp_svg_number_write_i (buf + i, ival);
	if ((fprec > 0) && (padf || (fval > 0.0))) {
		buf[i++] = '.';
		while ((fprec > 0) && (padf || (fval > 0.0))) {
			fval *= 10.0;
			dival = floor (fval);
			fval -= dival;
			buf[i++] = '0' + (int) dival;
			fprec -= 1;
		}

	}
	buf[i] = 0;
	return i;
}

unsigned int
sp_svg_number_write_de (gchar *buf, double val, unsigned int tprec, unsigned int padf)
{
	if ((val == 0.0) || ((fabs (val) >= 0.1) && (fabs(val) < 10000000))) {
		return sp_svg_number_write_d (buf, val, tprec, 0, padf);
	} else {
		double eval;
		int p;
		eval = floor (log10 (fabs (val)));
		val = val / pow (10.0, eval);
		p = sp_svg_number_write_d (buf, val, tprec, 0, padf);
		buf[p++] = 'e';
		p += sp_svg_number_write_i (buf + p, (int) eval);
		return p;
	}
}

/* Length */

unsigned int
sp_svg_length_read (const gchar *str, SPSVGLength *length)
{
	unsigned long unit;
	float value, computed;

	if (!str) return 0;

	if (!sp_svg_length_read_lff (str, &unit, &value, &computed, NULL)) 
		return 0;

	length->set = 1;
	length->unit = unit;
	length->value = value;
	length->computed = computed;

	return 1;
}

unsigned int
sp_svg_length_read_absolute (const gchar *str, SPSVGLength *length)
{
	unsigned long unit;
	float value, computed;

	if (!str) return 0;

	if (!sp_svg_length_read_lff (str, &unit, &value, &computed, NULL)) 
		return 0;

	if ((unit == SP_SVG_UNIT_EM) || (unit == SP_SVG_UNIT_EX) || (unit == SP_SVG_UNIT_PERCENT))
		//not an absolute unit
		return 0;

	length->set = 1;
	length->unit = unit;
	length->value = value;
	length->computed = computed;

	return 1;
}


unsigned int
sp_svg_length_read_computed_absolute (const gchar *str, float *length)
{
	unsigned long unit;
	float computed;

	if (!str) return 0;

	if (!sp_svg_length_read_lff (str, &unit, NULL, &computed, NULL)) 
		// failed to read
		return 0;

	if ((unit == SP_SVG_UNIT_EM) || (unit == SP_SVG_UNIT_EX) || (unit == SP_SVG_UNIT_PERCENT))
		//not an absolute unit
		return 0;

	*length = computed;

	return 1;
}

GList *
sp_svg_length_list_read (const gchar *str)
{
	unsigned long unit;
	float value, computed;
	char *next = (char *) str;
	GList *list = NULL;

	if (!str) return NULL;

	while (sp_svg_length_read_lff (next, &unit, &value, &computed, &next) && next) {

		SPSVGLength *length = g_new (SPSVGLength, 1);

		length->set = 1;
		length->unit = unit;
		length->value = value;
		length->computed = computed;

		while (*next == ',' && next) next++;

		list = g_list_append (list, length);

		if (!*next) break;
	}

	return list;
}


#define UVAL(a,b) (((unsigned int) (a) << 8) | (unsigned int) (b))

unsigned int
sp_svg_length_read_lff (const gchar *str, unsigned long *unit, float *val, float *computed, char **next)
{
	const gchar *e;
	float v;

	if (!str) return 0;
	v = strtod (str, (char **) &e);
	if (e == str) return 0;
	if (!e[0]) {
		/* Unitless */
		if (unit) *unit = SP_SVG_UNIT_NONE;
		if (val) *val = v;
		if (computed) *computed = v;
		if (next) *next = NULL; // no more values
		return 1;
	} else if (!isalnum (e[0])) {
		/* Unitless or percent */
		if (e[0] == '%') {
			/* Percent */
			if (e[1] && isalnum (e[1])) return 0;
			if (unit) *unit = SP_SVG_UNIT_PERCENT;
			if (val) *val = v * 0.01;
			if (next) *next = (char *) e + 1; 
			return 1;
		} else {
			/* Unitless */
			if (unit) *unit = SP_SVG_UNIT_NONE;
			if (val) *val = v;
			if (computed) *computed = v;
			if (next) *next = (char *) e; 
			return 1;
		}
	} else if (e[1] && !isalnum (e[2])) {
		unsigned int uval;
		/* Units */
		uval = UVAL (e[0], e[1]);
		switch (uval) {
		case UVAL('p','x'):
			if (unit) *unit = SP_SVG_UNIT_PX;
			if (computed) *computed = v * 1.0;
			break;
		case UVAL('p','t'):
			if (unit) *unit = SP_SVG_UNIT_PT;
			if (computed) *computed = v * 1.25;
			break;
		case UVAL('p','c'):
			if (unit) *unit = SP_SVG_UNIT_PC;
			if (computed) *computed = v * 15.0;
			break;
		case UVAL('m','m'):
			if (unit) *unit = SP_SVG_UNIT_MM;
			if (computed) *computed = v * 3.543307;
			break;
		case UVAL('c','m'):
			if (unit) *unit = SP_SVG_UNIT_CM;
			if (computed) *computed = v * 35.43307;
			break;
		case UVAL('i','n'):
			if (unit) *unit = SP_SVG_UNIT_IN;
			if (computed) *computed = v * 90.0;
			break;
		case UVAL('e','m'):
			if (unit) *unit = SP_SVG_UNIT_EM;
			break;
		case UVAL('e','x'):
			if (unit) *unit = SP_SVG_UNIT_EX;
			break;
		default:
			/* Invalid */
			return 0;
			break;
		}
		if (val) *val = v;
		if (next) *next = (char *) e + 2; 
		return 1;
	}

	/* Invalid */
	return 0;
}

unsigned int sp_svg_length_read_ldd (const gchar *str, unsigned long *unit, double *value, double *computed) {
	float a, b;
	unsigned int r = sp_svg_length_read_lff (str, unit, &a, &b, NULL);
	if(value) *value = a;
	if(computed) *computed = b;
	return r;
}

void
sp_svg_length_set (SPSVGLength *length, unsigned long unit, float value, float computed)
{
	length->set = 1;
	length->unit = unit;
	length->value = value;
	length->computed = computed;
}

void
sp_svg_length_unset (SPSVGLength *length, unsigned long unit, float value, float computed)
{
	length->set = 0;
	length->unit = unit;
	length->value = value;
	length->computed = computed;
}

void
sp_svg_length_update (SPSVGLength *length, double em, double ex, double scale)
{
	if (length->unit == SP_SVG_UNIT_EM) {
		length->computed = length->value * em;
	} else if (length->unit == SP_SVG_UNIT_EX) {
		length->computed = length->value * ex;
	} else if (length->unit == SP_SVG_UNIT_PERCENT) {
		length->computed = length->value * scale;
	}
}

double
sp_svg_read_percentage (const char * str, double def)
{
	char * u;
	double v;

	if (str == NULL) return def;

	v = strtod (str, &u);
	while (isspace (*u)) {
		if (*u == '\0') return v;
		u++;
	}
	if (*u == '%') v /= 100.0;

	return v;
}

int
sp_svg_write_percentage (char * buf, int buflen, double val)
{
	return g_snprintf (buf, buflen, "%g%%", val * 100.0);
}

