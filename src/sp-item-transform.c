#define __SP_ITEM_TRANSFORM_C__

/*
 * Transformations on selected items
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   Frank Felfe <innerspace@iname.com>
 *
 * Copyright (C) 1999-2002 authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <libnr/nr-matrix.h>
#include "svg/svg.h"
#include "sp-item-transform.h"

#include <libart_lgpl/art_affine.h>

void art_affine_skew (double dst[6], double dx, double dy);

void
sp_item_rotate_rel (SPItem * item, double angle)
{
	NRRectF b;
	NRMatrixF curaff, f;
  double rotate[6], s[6], t[6], u[6], v[6], newaff[6];
  double x,y;
  char tstr[80];

  sp_item_bbox_desktop (item, &b);

  x = b.x0 + (b.x1 - b.x0)/2;
  y = b.y0 + (b.y1 - b.y0)/2;

  art_affine_rotate (rotate,angle);
  nr_matrix_d_set_translate (NR_MATRIX_D_FROM_DOUBLE (s),x,y);
  nr_matrix_d_set_translate (NR_MATRIX_D_FROM_DOUBLE (t),-x,-y);


  tstr[79] = '\0';

  // rotate item
  sp_item_i2d_affine (item, &curaff);
  nr_matrix_multiply_dfd (NR_MATRIX_D_FROM_DOUBLE (u), &curaff, NR_MATRIX_D_FROM_DOUBLE (t));
  art_affine_multiply (v, u, rotate);
  art_affine_multiply (newaff, v, s);
  nr_matrix_f_from_d (&f, NR_MATRIX_D_FROM_DOUBLE (newaff));
  sp_item_set_i2d_affine (item, &f);

  //update repr
  if (sp_svg_transform_write (tstr, 80, &item->transform)) {
	  sp_repr_set_attr (SP_OBJECT (item)->repr, "transform", tstr);
  } else {
	  sp_repr_set_attr (SP_OBJECT (item)->repr, "transform", NULL);
  }
}

void
sp_item_scale_rel (SPItem *item, double dx, double dy)
{
	NRMatrixF curaff, new;
	NRMatrixD scale, s, t, u, v;
	char tstr[80];
	double x, y;
	NRRectF b;

	sp_item_bbox_desktop (item, &b);

	x = b.x0 + (b.x1 - b.x0) / 2;
	y = b.y0 + (b.y1 - b.y0) / 2;

	nr_matrix_d_set_scale (&scale, dx, dy);
	nr_matrix_d_set_translate (&s, x, y);
	nr_matrix_d_set_translate (&t, -x, -y);

	tstr[79] = '\0';

	// scale item
	sp_item_i2d_affine (item, &curaff);
	nr_matrix_multiply_dfd (&u, &curaff, &t);
	nr_matrix_multiply_ddd (&v, &u, &scale);
	nr_matrix_multiply_ddd (&u, &v, &s);
	nr_matrix_f_from_d (&new, &u);

	sp_item_set_i2d_affine (item, &new);

	//update repr
	if (sp_svg_transform_write (tstr, 79, &item->transform)) {
		sp_repr_set_attr (SP_OBJECT (item)->repr, "transform", tstr);
	} else {
		sp_repr_set_attr (SP_OBJECT (item)->repr, "transform", NULL);
	}
} 

void
art_affine_skew (double dst[6], double dx, double dy)
{
  dst[0] = 1;
  dst[1] = dy;
  dst[2] = dx;
  dst[3] = 1;
  dst[4] = 0;
  dst[5] = 0;
}

void
sp_item_skew_rel (SPItem *item, double dx, double dy)
{
	NRMatrixF cur, new;
	double skew[6], s[6], t[6] ,u[6] ,v[6] ,newaff[6];
	char tstr[80];
	double x,y;
	NRRectF b;

	sp_item_bbox_desktop (item, &b);
	x = b.x0 + (b.x1 - b.x0) / 2;
	y = b.y0 + (b.y1 - b.y0) / 2;

	art_affine_skew (skew, dx, dy);
	art_affine_translate (s, x, y);
	art_affine_translate (t, -x, -y);

	tstr[79] = '\0';

	// skew item
	sp_item_i2d_affine (item, &cur);
	nr_matrix_multiply_dfd (NR_MATRIX_D_FROM_DOUBLE (u), &cur, NR_MATRIX_D_FROM_DOUBLE (t));
	art_affine_multiply (v, u, skew);
	art_affine_multiply (newaff, v, s);
	nr_matrix_f_from_d (&new, NR_MATRIX_D_FROM_DOUBLE (newaff));
	sp_item_set_i2d_affine (item, &new);
	//update repr
	if (sp_svg_transform_write (tstr, 79, &item->transform)) {
		sp_repr_set_attr (SP_OBJECT (item)->repr, "transform", tstr);
	} else {
		sp_repr_set_attr (SP_OBJECT (item)->repr, "transform", NULL);
	}
}

void
sp_item_move_rel (SPItem * item, double dx, double dy)
{
	NRMatrixF cur, new;
	double move[6];
	char tstr[80];

	tstr[79] = '\0';

	// move item
	art_affine_translate (move, dx, dy);
	sp_item_i2d_affine (item, &cur);
	nr_matrix_multiply_ffd (&new, &cur, NR_MATRIX_D_FROM_DOUBLE (move));
	sp_item_set_i2d_affine (item, &new);

	//update repr
	if (sp_svg_transform_write (tstr, 79, &item->transform)) {
		sp_repr_set_attr (SP_OBJECT (item)->repr, "transform", tstr);
	} else {
		sp_repr_set_attr (SP_OBJECT (item)->repr, "transform", NULL);
	}
}

