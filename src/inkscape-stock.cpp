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

#include "config.h"
#include "inkscape-stock.h"

static struct StockIcon
{
  const char *name;
  const char *filename;

} const stock_icons[] = {
    /*stroke style */
    {INKSCAPE_STOCK_JOIN_MITER,                    "join_miter.xpm"          },
    {INKSCAPE_STOCK_JOIN_ROUND,                    "join_round.xpm"          },
    {INKSCAPE_STOCK_JOIN_BEVEL,                    "join_bevel.xpm"          },
    {INKSCAPE_STOCK_CAP_BUTT,                      "cap_butt.xpm"            },
    {INKSCAPE_STOCK_CAP_ROUND,                     "cap_round.xpm"           },
    {INKSCAPE_STOCK_CAP_SQUARE,                    "cap_square.xpm"          },

    /*object properties */
    {INKSCAPE_STOCK_ROTATE_LEFT,                   "rotate_left.xpm"         },
    {INKSCAPE_STOCK_SCALE_HOR,                     "scale_hor.xpm"           },
    {INKSCAPE_STOCK_SCALE_VER,                     "scale_ver.xpm"           },
    {INKSCAPE_STOCK_ARROWS_HOR,                    "arrows_hor.xpm"          },
    {INKSCAPE_STOCK_ARROWS_VER,                    "arrows_ver.xpm"          },
    {INKSCAPE_STOCK_DIMENSION_HOR,                 "dimension_hor.xpm"       },

    {INKSCAPE_STOCK_WRITING_MODE_LR,               "writing_mode_lr.xpm"     },
    {INKSCAPE_STOCK_WRITING_MODE_TB,               "writing_mode_tb.xpm"     },

    /*xml-tree */
    {INKSCAPE_STOCK_ADD_XML_ELEMENT_NODE,          "add_xml_element_node.xpm"},
    {INKSCAPE_STOCK_ADD_XML_TEXT_NODE,             "add_xml_text_node.xpm"   },
    {INKSCAPE_STOCK_DUPLICATE_XML_NODE,            "duplicate_xml_node.xpm"  },
    {INKSCAPE_STOCK_DELETE_XML_NODE,               "delete_xml_node.xpm"     },
    {INKSCAPE_STOCK_DELETE_XML_ATTRIBUTE,          "delete_xml_attribute.xpm"},
    {INKSCAPE_STOCK_SET,                           "set.xpm"                 },

    {INKSCAPE_STOCK_FILL_NONE,                     "fill_none.xpm"           },
    {INKSCAPE_STOCK_FILL_SOLID,                    "fill_solid.xpm"          },
    {INKSCAPE_STOCK_FILL_GRADIENT,                 "fill_gradient.xpm"       },
    {INKSCAPE_STOCK_FILL_RADIAL,                   "fill_radial.xpm"         },
    {INKSCAPE_STOCK_FILL_PATTERN,                  "fill_pattern.xpm"        },
    {INKSCAPE_STOCK_FILL_FRACTAL,                  "fill_fractal.xpm"        },

  //{INKSCAPE_STOCK_GUIDE_DIALOG,                  "guide_dialog.xpm"        },

    {INKSCAPE_STOCK_TOOLBOX_FILE,                  "toolbox_file.xpm"        },
    {INKSCAPE_STOCK_TOOLBOX_EDIT,                  "toolbox_edit.xpm"        },
    {INKSCAPE_STOCK_TOOLBOX_OBJECT,                "toolbox_object.xpm"      },
    {INKSCAPE_STOCK_TOOLBOX_SELECT,                "toolbox_select.xpm"      },
    {INKSCAPE_STOCK_TOOLBOX_DRAW,                  "toolbox_draw.xpm"        },
    {INKSCAPE_STOCK_TOOLBOX_ZOOM,                  "toolbox_zoom.xpm"        },
    {INKSCAPE_STOCK_TOOLBOX_NODE,                  "toolbox_node.xpm"        },
  //{INKSCAPE_STOCK_SEPARATE_TOOL,                 "separate_tool.xpm"       },

    {INKSCAPE_STOCK_EDIT_DUPLICATE,                "edit_duplicate.xpm"      },

    {INKSCAPE_STOCK_SELECTION_TOP,                 "selection_top.xpm"       },
    {INKSCAPE_STOCK_SELECTION_BOT,                 "selection_bot.xpm"       },
    {INKSCAPE_STOCK_SELECTION_UP,                  "selection_up.xpm"        },
    {INKSCAPE_STOCK_SELECTION_DOWN,                "selection_down.xpm"      },
    {INKSCAPE_STOCK_SELECTION_GROUP,               "selection_group.xpm"     },
    {INKSCAPE_STOCK_SELECTION_UNGROUP,             "selection_ungroup.xpm"   },
    {INKSCAPE_STOCK_SELECTION_COMBINE,             "selection_combine.xpm"   },
    {INKSCAPE_STOCK_SELECTION_BREAK,               "selection_break.xpm"     },

    {INKSCAPE_STOCK_OBJECT_ROTATE,                 "object_rotate.xpm"       },
    {INKSCAPE_STOCK_OBJECT_RESET,                  "object_reset.xpm"        },
    {INKSCAPE_STOCK_OBJECT_TOCURVE,                "object_tocurve.xpm"      },

    {INKSCAPE_STOCK_DRAW_SELECT,                   "draw_select.xpm"         },
    {INKSCAPE_STOCK_DRAW_NODE,                     "draw_node.xpm"           },
    {INKSCAPE_STOCK_DRAW_RECT,                     "draw_rect.xpm"           },
    {INKSCAPE_STOCK_DRAW_ARC,                      "draw_arc.xpm"            },
    {INKSCAPE_STOCK_DRAW_STAR,                     "draw_star.xpm"           },
    {INKSCAPE_STOCK_DRAW_SPIRAL,                   "draw_spiral.xpm"         },
    {INKSCAPE_STOCK_DRAW_FREEHAND,                 "draw_freehand.xpm"       },
    {INKSCAPE_STOCK_DRAW_PEN,                      "draw_pen.xpm"            },
    {INKSCAPE_STOCK_DRAW_DYNAHAND,                 "draw_dynahand.xpm"       },
    {INKSCAPE_STOCK_DRAW_TEXT,                     "draw_text.xpm"           },
    {INKSCAPE_STOCK_DRAW_ZOOM,                     "draw_zoom.xpm"           },
  //{INKSCAPE_STOCK_DRAW_DROPPER,                  "draw_dropper.xpm"        },

    {INKSCAPE_STOCK_ZOOM_IN,                       "zoom_in.xpm"             },
    {INKSCAPE_STOCK_ZOOM_OUT,                      "zoom_out.xpm"            },
  //{INKSCAPE_STOCK_TOGGLE_GRID,                   "toggle_grid.xpm"         },
  //{INKSCAPE_STOCK_TOGGLE_GUIDES,                 "toggle_guides"           },
    {INKSCAPE_STOCK_ZOOM_PAGE,                     "zoom_page.xpm"           },
    {INKSCAPE_STOCK_ZOOM_DRAW,                     "zoom_draw.xpm"           },
    {INKSCAPE_STOCK_ZOOM_SELECT,                   "zoom_select.xpm"         },

    {INKSCAPE_STOCK_OBJECT_LAYOUT,                 "object_layout.xpm"       },
    {INKSCAPE_STOCK_OBJECT_TRANS,                  "object_trans.xpm"        },
    {INKSCAPE_STOCK_OBJECT_ALIGN,                  "object_align.xpm"        },
    {INKSCAPE_STOCK_OBJECT_FONT,                   "object_font.xpm"         },

    {INKSCAPE_STOCK_PROPERTIES_FILL_PAGE,          "properties_fill.xpm"     },
    {INKSCAPE_STOCK_PROPERTIES_STROKE_PAINT_PAGE,  "properties_stroke.xpm"   },
    {INKSCAPE_STOCK_PROPERTIES_STROKE_PAGE,        "properties_stroke.xpm"   },

};

static gint stock_icon_count = sizeof (stock_icons) / sizeof (*stock_icons);
static gboolean stock_initialized = FALSE;

void
inkscape_gtk_stock_init (void) {

    if (stock_initialized)
        return;

    GtkIconFactory *icon_factory = gtk_icon_factory_new ();

    for (int i = 0; i < stock_icon_count; i++) {

        gchar *filename = (gchar *) g_build_filename (INKSCAPE_PIXMAPDIR,
                      stock_icons[i].filename, NULL);
        if (!g_file_test (filename, G_FILE_TEST_EXISTS)) {
            // testing
            g_critical ("Unable to load stock pixmap %s\n", filename);
            // g_critical ("Unable to load stock pixmap %s\n", 
            //             INKSCAPE_PIXMAPDIR);
            // g_critical ("Unable to load stock pixmap %s\n", 
            //             stock_icons[i].filename);
            g_free (filename);
            continue;
        }

        GdkPixbuf *pixbuf = gdk_pixbuf_new_from_file (filename, NULL);

        g_free (filename);

        GtkIconSet *icon_set = gtk_icon_set_new_from_pixbuf (pixbuf);

        if (icon_set)
            gtk_icon_factory_add (icon_factory, stock_icons[i].name, icon_set);

        gtk_icon_set_unref (icon_set);
        g_object_unref (pixbuf);
    }

    gtk_icon_factory_add_default (icon_factory);

    stock_initialized = TRUE;
}
