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

#include "gtk/gtkiconfactory.h"

#include "inkscape-stock.h"

static struct StockIcon
{
	const char *name;
	const char *dir;
	const char *filename;

} const stock_icons[] =
{
  /*stroke style*/
  { INKSCAPE_STOCK_JOIN_MITER, INKSCAPE_PIXMAPDIR, "join_miter.xpm"},
	{ INKSCAPE_STOCK_JOIN_ROUND, INKSCAPE_PIXMAPDIR, "join_round.xpm"},
	{ INKSCAPE_STOCK_JOIN_BEVEL, INKSCAPE_PIXMAPDIR, "join_bevel.xpm"},
	{ INKSCAPE_STOCK_CAP_BUTT, INKSCAPE_PIXMAPDIR, "cap_butt.xpm"},
	{ INKSCAPE_STOCK_CAP_ROUND, INKSCAPE_PIXMAPDIR, "cap_round.xpm"},
	{ INKSCAPE_STOCK_CAP_SQUARE, INKSCAPE_PIXMAPDIR, "cap_square.xpm"},
  /*object properties*/
	{ INKSCAPE_STOCK_ARROWS_HOR, INKSCAPE_PIXMAPDIR, "arrows_hor.xpm"},
	{ INKSCAPE_STOCK_ARROWS_VER, INKSCAPE_PIXMAPDIR, "arrows_ver.xpm"},
	{ INKSCAPE_STOCK_DIMENSION_HOR, INKSCAPE_PIXMAPDIR, "dimension_hor.xpm"},

  { INKSCAPE_STOCK_WRITING_MODE_LR, INKSCAPE_PIXMAPDIR, "writing_mode_lr.xpm"},
  { INKSCAPE_STOCK_WRITING_MODE_TB, INKSCAPE_PIXMAPDIR, "writing_mode_tb.xpm"},
  /*xml-tree*/
  { INKSCAPE_STOCK_ADD_XML_ELEMENT_NODE, INKSCAPE_PIXMAPDIR, "add_xml_element_node.xpm"},
  { INKSCAPE_STOCK_ADD_XML_TEXT_NODE, INKSCAPE_PIXMAPDIR, "add_xml_text_node.xpm"},
  { INKSCAPE_STOCK_DUPLICATE_XML_NODE, INKSCAPE_PIXMAPDIR, "duplicate_xml_node.xpm"},
  { INKSCAPE_STOCK_DELETE_XML_NODE, INKSCAPE_PIXMAPDIR, "delete_xml_node.xpm"},
  { INKSCAPE_STOCK_DELETE_XML_ATTRIBUTE, INKSCAPE_PIXMAPDIR, "delete_xml_attribute.xpm"},
  { INKSCAPE_STOCK_SET, INKSCAPE_PIXMAPDIR, "set.xpm"},

  { INKSCAPE_STOCK_FILL_NONE, INKSCAPE_PIXMAPDIR, "fill_none.xpm"},
  { INKSCAPE_STOCK_FILL_SOLID, INKSCAPE_PIXMAPDIR, "fill_solid.xpm"},
  { INKSCAPE_STOCK_FILL_GRADIENT, INKSCAPE_PIXMAPDIR, "fill_gradient.xpm"},
  { INKSCAPE_STOCK_FILL_RADIAL, INKSCAPE_PIXMAPDIR, "fill_radial.xpm"},
  { INKSCAPE_STOCK_FILL_PATTERN, INKSCAPE_PIXMAPDIR, "fill_pattern.xpm"},
  { INKSCAPE_STOCK_FILL_FRACTAL, INKSCAPE_PIXMAPDIR, "fill_fractal.xpm"},
};

static gint stock_icon_count = sizeof(stock_icons) / sizeof(*stock_icons);
static gboolean stock_initialized = FALSE;

void
inkscape_gtk_stock_init(void)
{
	GtkIconFactory *icon_factory;
	int i;

  if (stock_initialized)
	return;

  icon_factory = gtk_icon_factory_new();

  for (i = 0; i < stock_icon_count; i++) {
    GtkIconSet *icon_set;
    GdkPixbuf *pixbuf;
    gchar *filename;

    filename = (gchar *) g_strdup_printf ("%s/%s", stock_icons[i].dir, stock_icons[i].filename);
    if (!g_file_test(filename, G_FILE_TEST_EXISTS)) {
      g_critical("Unable to load stock pixmap %s\n", filename);
      g_free(filename);
      return;
    }

    pixbuf = gdk_pixbuf_new_from_file(filename, NULL);

    g_free(filename);

    icon_set = gtk_icon_set_new_from_pixbuf(pixbuf);

    gtk_icon_factory_add(icon_factory, stock_icons[i].name, icon_set);

    gtk_icon_set_unref(icon_set);
    g_object_unref(pixbuf);
  }

	gtk_icon_factory_add_default(icon_factory);

  stock_initialized = TRUE;
}

