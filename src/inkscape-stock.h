/*
 * @file inkscape-stock.h GTK+ Stock resources
 *
 * Authors:
 *   Robert Crosbie
 *
 * Copyright (C) 1999-2002 Authors
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef _INKSCAPE_STOCK_H_
#define _INKSCAPE_STOCK_H_

/**************************************************************************/
/** @name Inkscape Stock images                                           */
/**************************************************************************/
/*@{*/
#define INKSCAPE_STOCK_ABOUT           "inkscape-about"
#define INKSCAPE_STOCK_JOIN_MITER      "join_miter"
#define INKSCAPE_STOCK_JOIN_ROUND      "join_round"
#define INKSCAPE_STOCK_JOIN_BEVEL      "join_bevel"
#define INKSCAPE_STOCK_CAP_BUTT        "cap_butt"
#define INKSCAPE_STOCK_CAP_ROUND       "cap_round"
#define INKSCAPE_STOCK_CAP_SQUARE      "cap_square"

/**
 * Sets up the inkscape stock repository.
 */
void inkscape_gtk_stock_init(void);

#endif /* _INKSCAPE_STOCK_H_ */
