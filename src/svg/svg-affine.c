#define __SP_SVG_AFFINE_C__

/*
 * SVG data parser
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   Raph Levien <raph@acm.org>
 *
 * Copyright (C) 1999-2002 Lauris Kaplinski
 * Copyright (C) 1999 Raph Levien
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>

#include <libnr/nr-matrix.h>
#include <glib.h>
#include "svg.h"

#ifndef M_PI
#define M_PI 3.1415927
#endif

unsigned int
sp_svg_transform_read (const unsigned char *str, NRMatrixF *transform)
{
	NRMatrixD m, a;
	int idx;
	char keyword[32];
	double args[6];
	int n_args;
	int key_len;

	if (str == NULL) return 0;

	nr_matrix_d_set_identity (&a);

	idx = 0;
	while (str[idx]) {
		/* skip initial whitespace */
		while (isspace (str[idx])) idx++;

		/* parse keyword */
		for (key_len = 0; key_len < sizeof (keyword); key_len++) {
			char c;

			c = str[idx];
			if (isalpha (c) || c == '-') {
				keyword[key_len] = str[idx++];
			} else {
				break;
			}
		}
		if (key_len >= sizeof (keyword)) return 0;
		keyword[key_len] = '\0';

		/* skip whitespace */
		while (isspace (str[idx])) idx++;

		if (str[idx] != '(') return 0;
		idx++;

		for (n_args = 0; ; n_args++) {
			char c;
			char *end_ptr;

			/* skip whitespace */
			while (isspace (str[idx])) idx++;
			c = str[idx];
			if (isdigit (c) || c == '+' || c == '-' || c == '.') {
				if (n_args == sizeof (args) / sizeof (args[0])) return 0; /* Too many args */
				args[n_args] = strtod (str + idx, &end_ptr);
				idx = end_ptr - (char *) str;

				while (isspace (str[idx])) idx++;

				/* skip optional comma */
				if (str[idx] == ',') idx++;
			} else if (c == ')') {
				break;
			} else {
				return 0;
			}
		}
		idx++;

		/* ok, have parsed keyword and args, now modify the transform */
		if (!strcmp (keyword, "matrix")) {
			if (n_args != 6) return 0;
			nr_matrix_multiply_ddd (&a, NR_MATRIX_D_FROM_DOUBLE (args), &a);
		} else if (!strcmp (keyword, "translate")) {
			if (n_args == 1) {
				args[1] = 0;
			} else if (n_args != 2) {
				return 0;
			}
			nr_matrix_d_set_translate (&m, args[0], args[1]);
			nr_matrix_multiply_ddd (&a, &m, &a);
		} else if (!strcmp (keyword, "scale")) {
			if (n_args == 1) {
				args[1] = args[0];
			} else if (n_args != 2) {
				return 0;
			}
			nr_matrix_d_set_scale (&m, args[0], args[1]);
			nr_matrix_multiply_ddd (&a, &m, &a);
		} else if (!strcmp (keyword, "rotate")) {
			double s, c;
			if (n_args != 1) return 0;
			s = sin (args[0] * M_PI / 180.0);
			c = cos (args[0] * M_PI / 180.0);
			m.c[0] = c;
			m.c[1] = s;
			m.c[2] = -s;
			m.c[3] = c;
			m.c[4] = 0.0;
			m.c[5] = 0.0;
			nr_matrix_multiply_ddd (&a, &m, &a);
		} else if (!strcmp (keyword, "skewX")) {
			if (n_args != 1) return 0;
			m.c[0] = 1;
			m.c[1] = 0;
			m.c[2] = tan (args[0] * M_PI / 180.0);
			m.c[3] = 1;
			m.c[4] = 0.0;
			m.c[5] = 0.0;
			nr_matrix_multiply_ddd (&a, &m, &a);
		} else if (!strcmp (keyword, "skewY")) {
			if (n_args != 1) return 0;
			m.c[0] = 1;
			m.c[1] = tan (args[0] * M_PI / 180.0);
			m.c[2] = 0;
			m.c[3] = 1;
			m.c[4] = 0.0;
			m.c[5] = 0.0;
			nr_matrix_multiply_ddd (&a, &m, &a);
		} else {
			return 0; /* unknown keyword */
		}
	}

	transform->c[0] = a.c[0];
	transform->c[1] = a.c[1];
	transform->c[2] = a.c[2];
	transform->c[3] = a.c[3];
	transform->c[4] = a.c[4];
	transform->c[5] = a.c[5];

	return 1;
}

#define EQ(a,b) (fabs ((a) - (b)) < 1e-9)

unsigned int
sp_svg_transform_write (unsigned char *str, unsigned int size, NRMatrixF *transform)
{
	unsigned char c[256];
	int p;
	double e;

	if (!transform) {
		*str = 0;
		return 0;
	}

	e = 0.000001 * NR_MATRIX_DF_EXPANSION (transform);

	p = 0;
	/* fixme: We could use t1 * t1 + t2 * t2 here instead */
	if (NR_DF_TEST_CLOSE (transform->c[1], 0.0, e) && NR_DF_TEST_CLOSE (transform->c[2], 0.0, e)) {
		if (NR_DF_TEST_CLOSE (transform->c[4], 0.0, e) && NR_DF_TEST_CLOSE (transform->c[5], 0.0, e)) {
			if (NR_DF_TEST_CLOSE (transform->c[0], 1.0, e) && NR_DF_TEST_CLOSE (transform->c[3], 1.0, e)) {
				/* We are more or less identity */
				*str = 0;
				return 0;
			} else {
				/* We are more or less scale */
				strcpy (c + p, "scale(");
				p += 6;
				p += sp_svg_number_write_de (c + p, transform->c[0], 6, FALSE);
				c[p++] = ',';
				p += sp_svg_number_write_de (c + p, transform->c[3], 6, FALSE);
				c[p++] = ')';
				p = MIN (p, size - 1);
				memcpy (str, c, p);
				str[p] = 0;
				return p;
			}
		} else {
			if (NR_DF_TEST_CLOSE (transform->c[0], 1.0, e) && NR_DF_TEST_CLOSE (transform->c[3], 1.0, e)) {
				/* We are more or less translate */
				strcpy (c + p, "translate(");
				p += 10;
				p += sp_svg_number_write_de (c + p, transform->c[4], 6, FALSE);
				c[p++] = ',';
				p += sp_svg_number_write_de (c + p, transform->c[5], 6, FALSE);
				c[p++] = ')';
				p = MIN (p, size - 1);
				memcpy (str, c, p);
				str[p] = 0;
				return p;
			} else {
				strcpy (c + p, "matrix(");
				p += 7;
				p += sp_svg_number_write_de (c + p, transform->c[0], 6, FALSE);
				c[p++] = ',';
				p += sp_svg_number_write_de (c + p, transform->c[1], 6, FALSE);
				c[p++] = ',';
				p += sp_svg_number_write_de (c + p, transform->c[2], 6, FALSE);
				c[p++] = ',';
				p += sp_svg_number_write_de (c + p, transform->c[3], 6, FALSE);
				c[p++] = ',';
				p += sp_svg_number_write_de (c + p, transform->c[4], 6, FALSE);
				c[p++] = ',';
				p += sp_svg_number_write_de (c + p, transform->c[5], 6, FALSE);
				c[p++] = ')';
				p = MIN (p, size - 1);
				memcpy (str, c, p);
				str[p] = 0;
				return p;
			}
		}
	} else {
		strcpy (c + p, "matrix(");
		p += 7;
		p += sp_svg_number_write_de (c + p, transform->c[0], 6, FALSE);
		c[p++] = ',';
		p += sp_svg_number_write_de (c + p, transform->c[1], 6, FALSE);
		c[p++] = ',';
		p += sp_svg_number_write_de (c + p, transform->c[2], 6, FALSE);
		c[p++] = ',';
		p += sp_svg_number_write_de (c + p, transform->c[3], 6, FALSE);
		c[p++] = ',';
		p += sp_svg_number_write_de (c + p, transform->c[4], 6, FALSE);
		c[p++] = ',';
		p += sp_svg_number_write_de (c + p, transform->c[5], 6, FALSE);
		c[p++] = ')';
		p = MIN (p, size - 1);
		memcpy (str, c, p);
		str[p] = 0;
		return p;
	}
}

