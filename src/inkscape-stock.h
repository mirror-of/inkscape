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

/*stroke style*/
#define INKSCAPE_STOCK_ABOUT           "inkscape-about"
#define INKSCAPE_STOCK_JOIN_MITER      "join_miter"
#define INKSCAPE_STOCK_JOIN_ROUND      "join_round"
#define INKSCAPE_STOCK_JOIN_BEVEL      "join_bevel"
#define INKSCAPE_STOCK_CAP_BUTT        "cap_butt"
#define INKSCAPE_STOCK_CAP_ROUND       "cap_round"
#define INKSCAPE_STOCK_CAP_SQUARE      "cap_square"
/*object properties*/
#define INKSCAPE_STOCK_ARROWS_HOR      "arrows_hor"
#define INKSCAPE_STOCK_ARROWS_VER      "arrows_hor"
#define INKSCAPE_STOCK_DIMENSION_HOR      "dimension_hor"
#define INKSCAPE_STOCK_DIMENSION_VER      "dimension_hor"

#define INKSCAPE_STOCK_WRITING_MODE_LR      "writing_mode_lr"
#define INKSCAPE_STOCK_WRITING_MODE_TB      "writing_mode_tb"
/*xml-tree*/
#define INKSCAPE_STOCK_ADD_XML_ELEMENT_NODE      "add_xml_element_node"
#define INKSCAPE_STOCK_ADD_XML_TEXT_NODE      "add_xml_text_node"
#define INKSCAPE_STOCK_DUPLICATE_XML_NODE      "duplicate_xml_node"
#define INKSCAPE_STOCK_DELETE_XML_NODE      "delete_xml_node"
#define INKSCAPE_STOCK_DELETE_XML_ATTRIBUTE      "delete_xml_attribute"
#define INKSCAPE_STOCK_SET      "set"
/*paint-selector*/
#define INKSCAPE_STOCK_FILL_NONE      "fill_none"
#define INKSCAPE_STOCK_FILL_SOLID      "fill_solid"
#define INKSCAPE_STOCK_FILL_GRADIENT      "fill_gradient"
#define INKSCAPE_STOCK_FILL_RADIAL      "fill_radial"
#define INKSCAPE_STOCK_FILL_PATTERN      "fill_pattern"
#define INKSCAPE_STOCK_FILL_FRACTAL      "fill_fractal"

/**
 * Sets up the inkscape stock repository.
 */
#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

void inkscape_gtk_stock_init(void);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif /* _INKSCAPE_STOCK_H_ */
