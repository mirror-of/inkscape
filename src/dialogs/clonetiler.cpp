#define __SP_CLONE_TILER_C__

/*
 * Clone tiling dialog
 *
 * Authors:
 *   bulia byak
 *
 * Copyright (C) 2004 Authors
 *
 */

#include <config.h>
#include <glib.h>
#include <gtk/gtk.h>

#include <glibmm/i18n.h>
#include "helper/window.h"
#include "../inkscape.h"
#include "../prefs-utils.h"
#include "dialog-events.h"
#include "../macros.h"
#include "../verbs.h"
#include "../interface.h"
#include "../seltrans.h"
#include "../selection.h"
#include "../selection-chemistry.h"
#include "../style.h"
#include "../desktop.h"
#include "../document.h"
#include "../sp-use.h"

#include "xml/repr.h"

#include "svg/svg.h"

#include "libnr/nr-point-ops.h"
#include "libnr/nr-rect.h"
#include "libnr/nr-matrix.h"
#include "libnr/nr-matrix-ops.h"
#include "libnr/nr-matrix-fns.h"

#include "libnr/nr-matrix-rotate-ops.h"
#include "libnr/nr-matrix-scale-ops.h"
#include "libnr/nr-matrix-translate-ops.h"
#include "libnr/nr-rotate-matrix-ops.h"
#include "libnr/nr-scale-matrix-ops.h"
#include "libnr/nr-rotate-fns.h"
#include "libnr/nr-rotate.h"
#include "libnr/nr-rotate-matrix-ops.h"
#include "libnr/nr-rotate-ops.h"
#include "libnr/nr-scale.h"
#include "libnr/nr-scale-matrix-ops.h"
#include "libnr/nr-scale-ops.h"
#include "libnr/nr-scale-translate-ops.h"
#include "libnr/nr-translate.h"
#include "libnr/nr-translate-matrix-ops.h"
#include "libnr/nr-translate-ops.h"
#include "libnr/nr-translate-rotate-ops.h"
#include "libnr/nr-translate-scale-ops.h"

#include "clonetiler.h"

static GtkWidget *dlg = NULL;
static win_data wd;

// impossible original values to make sure they are read from prefs
static gint x = -1000, y = -1000, w = 0, h = 0; 
static gchar *prefs_path = "dialogs.clonetiler";

#define SB_WIDTH 90
#define SB_LONG_ADJUSTMENT 20
#define SB_MARGIN 1
#define SUFFIX_WIDTH 70
#define HB_MARGIN 4
#define VB_MARGIN 4
#define VB_SKIP 1

static void
clonetiler_dialog_destroy (GtkObject *object, gpointer data)
{

    sp_signal_disconnect_by_data (INKSCAPE, dlg);
    wd.win = dlg = NULL;
    wd.stop = 0;
    
} // edn of sp_display_dialog_destroy()



static gboolean
clonetiler_dialog_delete (GtkObject *object, GdkEvent *event, gpointer data)
{
    gtk_window_get_position ((GtkWindow *) dlg, &x, &y);
    gtk_window_get_size ((GtkWindow *) dlg, &w, &h);

    prefs_set_int_attribute (prefs_path, "x", x);
    prefs_set_int_attribute (prefs_path, "y", y);
    prefs_set_int_attribute (prefs_path, "w", w);
    prefs_set_int_attribute (prefs_path, "h", h);

    return FALSE; // which means, go ahead and destroy it

} // end of sp_display_dialog_delete()


enum {
    TILE_P1,
    TILE_P2,
    TILE_PM,
    TILE_PG,
    TILE_CM,
    TILE_PMM,
    TILE_PMG,
    TILE_PGG,
    TILE_CMM,
    TILE_P4,
    TILE_P4M,
    TILE_P4G,
    TILE_P3,
    TILE_P31M,
    TILE_P3M1,
    TILE_P6,
    TILE_P6M
};


NR::Matrix 
clonetiler_get_transform ( 
    // symmetry group
    int type,
    // row, column
    int x, int y,
    // center, width, height of the tile
    double cx, double cy,
    double w, double h,

    // values from the dialog:
    double d_x_per_x, double d_y_per_x, double d_x_per_y, double d_y_per_y, int alternate_xy, double rand_xy,
    double d_rot_per_x, double d_rot_per_y, int alternate_rot, double rand_rot,
    double d_scalex_per_x, double d_scaley_per_x, double d_scalex_per_y, double d_scaley_per_y, int alternate_scale, double rand_scale
    )
{
    int eff_x, eff_y;

    // in abs units
    eff_x = (alternate_xy? (x%2) : (x));
    eff_y = (alternate_xy? (y%2) : (y));
    double dx = d_x_per_x * w * eff_x + d_x_per_y  * w * eff_y + rand_xy * w * g_random_double_range (-1, 1);
    double dy = d_y_per_x * h * eff_x + d_y_per_y  * h * eff_y + rand_xy * h * g_random_double_range (-1, 1);

    // in deg
    eff_x = (alternate_rot? (x%2) : (x));
    eff_y = (alternate_rot? (y%2) : (y));
    double drot = d_rot_per_x * eff_x + d_rot_per_y * eff_y + rand_rot * 180 * g_random_double_range (-1, 1);

    // times the original
    eff_x = (alternate_scale? (x%2) : (x));
    eff_y = (alternate_scale? (y%2) : (y));
    double rand_scale_both = rand_scale * 1 * g_random_double_range (-1, 1);
    double dscalex = 1 + d_scalex_per_x * eff_x + d_scalex_per_y * eff_y + rand_scale_both;
    if (dscalex < 0) dscalex = 0;
    double dscaley = 1 + d_scaley_per_x * eff_x + d_scaley_per_y * eff_y + rand_scale_both;
    if (dscaley < 0) dscaley = 0;

    NR::Matrix rect_translate (NR::translate (w * x + dx, h * y + dy));

    NR::Matrix drot_c = NR::translate(-cx, -cy) * NR::rotate (M_PI*drot/180) * NR::translate(cx, cy);

    NR::Matrix dscale_c = NR::translate(-cx, -cy) * NR::scale (dscalex, dscaley) * NR::translate(cx, cy);

    NR::Matrix d_s_r = dscale_c * drot_c;

    NR::Matrix rotate_180_c = NR::translate(-cx, -cy) * NR::rotate (M_PI) * NR::translate(cx, cy);

    NR::Matrix rotate_90_c = NR::translate(-cx, -cy) * NR::rotate (-M_PI/2) * NR::translate(cx, cy);
    NR::Matrix rotate_m90_c = NR::translate(-cx, -cy) * NR::rotate (M_PI/2) * NR::translate(cx, cy);

    NR::Matrix rotate_120_c = NR::translate(-cx, -cy) * NR::rotate (-2*M_PI/3) * NR::translate(cx, cy);
    NR::Matrix rotate_m120_c = NR::translate(-cx, -cy) * NR::rotate (2*M_PI/3) * NR::translate(cx, cy);

    NR::Matrix rotate_60_c = NR::translate(-cx, -cy) * NR::rotate (-M_PI/3) * NR::translate(cx, cy);
    NR::Matrix rotate_m60_c = NR::translate(-cx, -cy) * NR::rotate (M_PI/3) * NR::translate(cx, cy);

    double cos60 = cos(M_PI/3);
    double sin60 = sin(M_PI/3);
    double cos30 = cos(M_PI/6);
    double sin30 = sin(M_PI/6);

    NR::Matrix flip_x = NR::translate(-cx, -cy) * NR::scale (-1, 1) * NR::translate(cx, cy);
    NR::Matrix flip_y = NR::translate(-cx, -cy) * NR::scale (1, -1) * NR::translate(cx, cy);

    switch (type) {

    case TILE_P1:
        return d_s_r * rect_translate;
        break;

    case TILE_P2:
        if (x % 2 == 0) {
            return d_s_r * rect_translate;
        } else {
            return d_s_r * rotate_180_c * rect_translate;
        }
        break;

    case TILE_PM:
        if (x % 2 == 0) {
            return d_s_r * rect_translate;
        } else {
            return d_s_r * flip_x * rect_translate;
        }
        break;

    case TILE_PG:
        if (y % 2 == 0) {
            return d_s_r * rect_translate;
        } else {
            return d_s_r * flip_x * rect_translate;
        }
        break;

    case TILE_CM:
        if ((x + y) % 2 == 0) {
            return d_s_r * rect_translate;
        } else {
            return d_s_r * flip_x * rect_translate;
        }
        break;

    case TILE_PMM:
        if (y % 2 == 0) {
            if (x % 2 == 0) {
                return rect_translate;
            } else {
                return d_s_r * flip_x * rect_translate;
            }
        } else {
            if (x % 2 == 0) {
                return d_s_r * flip_y * rect_translate;
            } else {
                return d_s_r * flip_x * flip_y * rect_translate;
            }
        }
        break;

    case TILE_PMG:
        if (y % 4 == 0) {
            return d_s_r * rect_translate;
        } else if (y % 4 == 1) {
            return d_s_r * flip_y * rect_translate;
        } else if (y % 4 == 2) {
            return d_s_r * flip_x * rect_translate;
        } else if (y % 4 == 3) {
            return d_s_r * flip_x * flip_y * rect_translate;
        }
        break;

    case TILE_PGG:
        if (y % 2 == 0) {
            if (x % 2 == 0) {
                return d_s_r * rect_translate;
            } else {
                return d_s_r * flip_y * rect_translate;
            }
        } else {
            if (x % 2 == 0) {
                return d_s_r * rotate_180_c * rect_translate;
            } else {
                return d_s_r * rotate_180_c * flip_y * rect_translate;
            }
        }
        break;

    case TILE_CMM:
        if (y % 4 == 0) {
            if (x % 2 == 0) {
                return d_s_r * rect_translate;
            } else {
                return d_s_r * flip_x * rect_translate;
            }
        } else if (y % 4 == 1) {
            if (x % 2 == 0) {
                return d_s_r * flip_y * rect_translate;
            } else {
                return d_s_r * flip_x * flip_y * rect_translate;
            }
        } else if (y % 4 == 2) {
            if (x % 2 == 1) {
                return d_s_r * rect_translate;
            } else {
                return d_s_r * flip_x * rect_translate;
            }
        } else {
            if (x % 2 == 1) {
                return d_s_r * flip_y * rect_translate;
            } else {
                return d_s_r * flip_x * flip_y * rect_translate;
            }
        }
        break;

    case TILE_P4:
    {
        NR::Matrix ori (NR::translate ((w + h) * (x/2) + dx,  (h + w) * (y/2) + dy));
        NR::Matrix dia1 (NR::translate (w/2 + h/2, -h/2 + w/2));
        NR::Matrix dia2 (NR::translate (-w/2 + h/2, h/2 + w/2));
        if (y % 2 == 0) {
            if (x % 2 == 0) {
                return d_s_r * ori;
            } else {
                return d_s_r * rotate_m90_c * dia1 * ori;
            }
        } else {
            if (x % 2 == 0) {
                return d_s_r * rotate_90_c * dia2 * ori;
            } else {
                return d_s_r * rotate_180_c * dia1 * dia2 * ori;
            }
        }
    }
    break;

    case TILE_P4M:
    {
        double max = MAX(w, h);
        NR::Matrix ori (NR::translate ((max + max) * (x/4) + dx,  (max + max) * (y/2) + dy));
        NR::Matrix dia1 (NR::translate (w/2 - h/2, h/2 - w/2));
        NR::Matrix dia2 (NR::translate (-h/2 + w/2, w/2 - h/2));
        if (y % 2 == 0) {
            if (x % 4 == 0) {
                return d_s_r * ori;
            } else if (x % 4 == 1) {
                return d_s_r * flip_y * rotate_m90_c * dia1 * ori;
            } else if (x % 4 == 2) {
                return d_s_r * rotate_m90_c * dia1 * NR::translate (h, 0) * ori;
            } else if (x % 4 == 3) {
                return d_s_r * flip_x * NR::translate (w, 0) * ori;
            }
        } else {
            if (x % 4 == 0) {
                return d_s_r * flip_y * NR::translate(0, h) * ori;
            } else if (x % 4 == 1) {
                return d_s_r * rotate_90_c * dia2 * NR::translate(0, h) * ori;
            } else if (x % 4 == 2) {
                return d_s_r * flip_y * rotate_90_c * dia2 * NR::translate(h, 0) * NR::translate(0, h) * ori;
            } else if (x % 4 == 3) {
                return d_s_r * flip_y * flip_x * NR::translate(w, 0) * NR::translate(0, h) * ori;
            }
        }
    }
    break;

    case TILE_P4G:
    {
        double max = MAX(w, h);
        NR::Matrix ori (NR::translate ((max + max) * (x/4) + dx,  (max + max) * y + dy));
        NR::Matrix dia1 (NR::translate (w/2 + h/2, h/2 - w/2));
        NR::Matrix dia2 (NR::translate (-h/2 + w/2, w/2 + h/2));
        if (((x/4) + y) % 2 == 0) {
            if (x % 4 == 0) {
                return d_s_r * ori;
            } else if (x % 4 == 1) {
                return d_s_r * rotate_m90_c * dia1 * ori;
            } else if (x % 4 == 2) {
                return d_s_r * rotate_90_c * dia2 * ori;
            } else if (x % 4 == 3) {
                return d_s_r * rotate_180_c * dia1 * dia2 * ori;
            }
        } else {
            if (x % 4 == 0) {
                return d_s_r * flip_y * NR::translate (0, h) * ori;
            } else if (x % 4 == 1) {
                return d_s_r * flip_y * rotate_m90_c * dia1 * NR::translate (-h, 0) * ori;
            } else if (x % 4 == 2) {
                return d_s_r * flip_y * rotate_90_c * dia2 * NR::translate (h, 0) * ori;
            } else if (x % 4 == 3) {
                return d_s_r * flip_x * NR::translate (w, 0) * ori;
            }
        }
    }
    break;

    case TILE_P3:
    {
        double width;
        double height;
        NR::Matrix dia1;
        NR::Matrix dia2;
        if (w > h) {
            width = w + w * cos60;
            height = 2 * w * sin60;
            dia1 = NR::Matrix (NR::translate (w/2 + w/2 * cos60, -(w/2 * sin60)));
            dia2 = dia1 * NR::Matrix (NR::translate (0, 2 * (w/2 * sin60)));
        } else {
            width = h * cos (M_PI/6);
            height = h;
            dia1 = NR::Matrix (NR::translate (h/2 * cos30, -(h/2 * sin30)));
            dia2 = dia1 * NR::Matrix (NR::translate (0, h/2));
        }
        NR::Matrix ori (NR::translate (width * (2*(x/3) + y%2) + dx,  (height/2) * y + dy));
        if (x % 3 == 0) {
            return d_s_r * ori; 
        } else if (x % 3 == 1) {
            return d_s_r * rotate_m120_c * dia1 * ori; 
        } else if (x % 3 == 2) {
            return d_s_r * rotate_120_c * dia2 * ori; 
        }
    }
    break;

    case TILE_P31M:
    {
        NR::Matrix ori;
        NR::Matrix dia1;
        NR::Matrix dia2;
        NR::Matrix dia3;
        NR::Matrix dia4;
        if (w > h) {
            ori = NR::Matrix(NR::translate (w * (x/6) + w/2 * (y%2) + dx,  (w * cos30) * y + dy));
            dia1 = NR::Matrix (NR::translate (0, h/2) * NR::translate (w/2, 0) * NR::translate (w/2 * cos60, -w/2 * sin60) * NR::translate (-h/2 * cos30, -h/2 * sin30) );
            dia2 = dia1 * NR::Matrix (NR::translate (h * cos30, h * sin30));
            dia3 = dia2 * NR::Matrix (NR::translate (0, 2 * (w/2 * sin60 - h/2 * sin30)));
            dia4 = dia3 * NR::Matrix (NR::translate (-h * cos30, h * sin30));
        } else {
            ori  = NR::Matrix (NR::translate (2*h * cos30  * (x/6 + 0.5*(y%2)) + dx,  (2*h - h * sin30) * y + dy));
            dia1 = NR::Matrix (NR::translate (0, -h/2) * NR::translate (h/2 * cos30, h/2 * sin30));
            dia2 = dia1 * NR::Matrix (NR::translate (h * cos30, h * sin30));
            dia3 = dia2 * NR::Matrix (NR::translate (0, h/2));
            dia4 = dia3 * NR::Matrix (NR::translate (-h * cos30, h * sin30));
        }
        if (x % 6 == 0) {
            return d_s_r * ori;
        } else if (x % 6 == 1) {
            return d_s_r * flip_y * rotate_m120_c * dia1 * ori;
        } else if (x % 6 == 2) {
            return d_s_r * rotate_m120_c * dia2 * ori;
        } else if (x % 6 == 3) {
            return d_s_r * flip_y * rotate_120_c * dia3 * ori;
        } else if (x % 6 == 4) {
            return d_s_r * rotate_120_c * dia4 * ori;
        } else if (x % 6 == 5) {
            return d_s_r * flip_y * NR::translate(0, h) * ori;
        }
    }
    break;

    case TILE_P3M1:
    {
        double width;
        double height;
        NR::Matrix dia1;
        NR::Matrix dia2;
        NR::Matrix dia3;
        NR::Matrix dia4;
        if (w > h) {
            width = w + w * cos60;
            height = 2 * w * sin60;
            dia1 = NR::Matrix (NR::translate (0, h/2) * NR::translate (w/2, 0) * NR::translate (w/2 * cos60, -w/2 * sin60) * NR::translate (-h/2 * cos30, -h/2 * sin30) );
            dia2 = dia1 * NR::Matrix (NR::translate (h * cos30, h * sin30));
            dia3 = dia2 * NR::Matrix (NR::translate (0, 2 * (w/2 * sin60 - h/2 * sin30)));
            dia4 = dia3 * NR::Matrix (NR::translate (-h * cos30, h * sin30));
        } else {
            width = 2 * h * cos (M_PI/6);
            height = 2 * h;
            dia1 = NR::Matrix (NR::translate (0, -h/2) * NR::translate (h/2 * cos30, h/2 * sin30));
            dia2 = dia1 * NR::Matrix (NR::translate (h * cos30, h * sin30));
            dia3 = dia2 * NR::Matrix (NR::translate (0, h/2));
            dia4 = dia3 * NR::Matrix (NR::translate (-h * cos30, h * sin30));
        }
        NR::Matrix ori (NR::translate (width * (2*(x/6) + y%2) + dx,  (height/2) * y + dy));
        if (x % 6 == 0) {
            return d_s_r * ori;
        } else if (x % 6 == 1) {
            return d_s_r * flip_y * rotate_m120_c * dia1 * ori;
        } else if (x % 6 == 2) {
            return d_s_r * rotate_m120_c * dia2 * ori;
        } else if (x % 6 == 3) {
            return d_s_r * flip_y * rotate_120_c * dia3 * ori;
        } else if (x % 6 == 4) {
            return d_s_r * rotate_120_c * dia4 * ori;
        } else if (x % 6 == 5) {
            return d_s_r * flip_y * NR::translate(0, h) * ori;
        }
    }
    break;

    case TILE_P6:
    {
        NR::Matrix ori;
        NR::Matrix dia1;
        NR::Matrix dia2;
        NR::Matrix dia3;
        NR::Matrix dia4;
        NR::Matrix dia5;
        if (w > h) {
            ori = NR::Matrix(NR::translate (2*w * (x/6) + w * (y%2) + dx,  (2*w * sin60) * y + dy));
            dia1 = NR::Matrix (NR::translate (w/2 * cos60, -w/2 * sin60));
            dia2 = dia1 * NR::Matrix (NR::translate (w/2, 0));
            dia3 = dia2 * NR::Matrix (NR::translate (w/2 * cos60, w/2 * sin60));
            dia4 = dia3 * NR::Matrix (NR::translate (-w/2 * cos60, w/2 * sin60));
            dia5 = dia4 * NR::Matrix (NR::translate (-w/2, 0));
        } else {
            ori = NR::Matrix(NR::translate (2*h * cos30 * (x/6 + 0.5*(y%2)) + dx,  (h + h * sin30) * y + dy));
            dia1 = NR::Matrix (NR::translate (-w/2, -h/2) * NR::translate (h/2 * cos30, -h/2 * sin30) * NR::translate (w/2 * cos60, w/2 * sin60));
            dia2 = dia1 * NR::Matrix (NR::translate (-w/2 * cos60, -w/2 * sin60) * NR::translate (h/2 * cos30, -h/2 * sin30) * NR::translate (h/2 * cos30, h/2 * sin30) * NR::translate (-w/2 * cos60, w/2 * sin60));
            dia3 = dia2 * NR::Matrix (NR::translate (w/2 * cos60, -w/2 * sin60) * NR::translate (h/2 * cos30, h/2 * sin30) * NR::translate (-w/2, h/2));
            dia4 = dia3 * dia1.inverse();
            dia5 = dia3 * dia2.inverse();
        }
        if (x % 6 == 0) {
            return d_s_r * ori;
        } else if (x % 6 == 1) {
            return d_s_r * rotate_m60_c * dia1 * ori;
        } else if (x % 6 == 2) {
            return d_s_r * rotate_m120_c * dia2 * ori;
        } else if (x % 6 == 3) {
            return d_s_r * rotate_180_c * dia3 * ori;
        } else if (x % 6 == 4) {
            return d_s_r * rotate_120_c * dia4 * ori;
        } else if (x % 6 == 5) {
            return d_s_r * rotate_60_c * dia5 * ori;
        }
    }
    break;

    case TILE_P6M:
    {

        NR::Matrix ori;
        NR::Matrix dia1, dia2, dia3, dia4, dia5, dia6, dia7, dia8, dia9, dia10;
        if (w > h) {
            ori = NR::Matrix(NR::translate (2*w * (x/12) + w * (y%2) + dx,  (2*w * sin60) * y + dy));
            dia1 = NR::Matrix (NR::translate (w/2, h/2) * NR::translate (-w/2 * cos60, -w/2 * sin60) * NR::translate (-h/2 * cos30, h/2 * sin30));
            dia2 = dia1 * NR::Matrix (NR::translate (h * cos30, -h * sin30));
            dia3 = dia2 * NR::Matrix (NR::translate (-h/2 * cos30, h/2 * sin30) * NR::translate (w * cos60, 0) * NR::translate (-h/2 * cos30, -h/2 * sin30));
            dia4 = dia3 * NR::Matrix (NR::translate (h * cos30, h * sin30));
            dia5 = dia4 * NR::Matrix (NR::translate (-h/2 * cos30, -h/2 * sin30) * NR::translate (-w/2 * cos60, w/2 * sin60) * NR::translate (w/2, -h/2));
            dia6 = dia5 * NR::Matrix (NR::translate (0, h));
            dia7 = dia6 * dia1.inverse();
            dia8 = dia6 * dia2.inverse();
            dia9 = dia6 * dia3.inverse();
            dia10 = dia6 * dia4.inverse();
        } else {
            ori = NR::Matrix(NR::translate (4*h * cos30 * (x/12 + 0.5*(y%2)) + dx,  (2*h  + 2*h * sin30) * y + dy));
            dia1 = NR::Matrix (NR::translate (-w/2, -h/2) * NR::translate (h/2 * cos30, -h/2 * sin30) * NR::translate (w/2 * cos60, w/2 * sin60));
            dia2 = dia1 * NR::Matrix (NR::translate (h * cos30, -h * sin30));
            dia3 = dia2 * NR::Matrix (NR::translate (-w/2 * cos60, -w/2 * sin60) * NR::translate (h * cos30, 0) * NR::translate (-w/2 * cos60, w/2 * sin60));
            dia4 = dia3 * NR::Matrix (NR::translate (h * cos30, h * sin30));
            dia5 = dia4 * NR::Matrix (NR::translate (w/2 * cos60, -w/2 * sin60) * NR::translate (h/2 * cos30, h/2 * sin30) * NR::translate (-w/2, h/2));
            dia6 = dia5 * NR::Matrix (NR::translate (0, h));
            dia7 = dia6 * dia1.inverse();
            dia8 = dia6 * dia2.inverse();
            dia9 = dia6 * dia3.inverse();
            dia10 = dia6 * dia4.inverse();
        }
        if (x % 12 == 0) {
            return d_s_r * ori;
        } else if (x % 12 == 1) {
            return d_s_r * flip_y * rotate_m60_c * dia1 * ori;
        } else if (x % 12 == 2) {
            return d_s_r * rotate_m60_c * dia2 * ori;
        } else if (x % 12 == 3) {
            return d_s_r * flip_y * rotate_m120_c * dia3 * ori;
        } else if (x % 12 == 4) {
            return d_s_r * rotate_m120_c * dia4 * ori;
        } else if (x % 12 == 5) {
            return d_s_r * flip_x * dia5 * ori;
        } else if (x % 12 == 6) {
            return d_s_r * flip_x * flip_y * dia6 * ori;
        } else if (x % 12 == 7) {
            return d_s_r * flip_y * rotate_120_c * dia7 * ori;
        } else if (x % 12 == 8) {
            return d_s_r * rotate_120_c * dia8 * ori;
        } else if (x % 12 == 9) {
            return d_s_r * flip_y * rotate_60_c * dia9 * ori;
        } else if (x % 12 == 10) {
            return d_s_r * rotate_60_c * dia10 * ori;
        } else if (x % 12 == 11) {
            return d_s_r * flip_y * NR::translate (0, h) * ori;
        }
    }
    break;

    default: 
        break;
    }

    return NR::identity();
}

static void
clonetiler_remove (GtkWidget *widget, void *)
{
    SPDesktop *desktop = SP_ACTIVE_DESKTOP;
    if (desktop == NULL)
        return;

    SPSelection *selection = SP_DT_SELECTION(desktop);

    // check if something is selected
    if (selection->isEmpty() || g_slist_length((GSList *) selection->itemList()) > 1) {
        desktop->messageStack()->flash(Inkscape::WARNING_MESSAGE, _("Select <b>one object</b> whose tiled clones to remove."));
        return;
    }

    SPObject *obj = SP_OBJECT(selection->singleItem());
    Inkscape::XML::Node *obj_repr = SP_OBJECT_REPR(obj);
    const char *id_href = g_strdup_printf("#%s", obj_repr->attribute("id"));
    SPObject *parent = SP_OBJECT_PARENT (obj);

// remove old tiling
    GSList *to_delete = NULL;
    for (SPObject *child = sp_object_first_child(parent); child != NULL; child = SP_OBJECT_NEXT(child)) {
        if (SP_IS_USE(child) && 
            SP_OBJECT_REPR(child)->attribute("xlink:href") &&
            !strcmp(id_href, SP_OBJECT_REPR(child)->attribute("xlink:href")) && 
            SP_OBJECT_REPR(child)->attribute("inkscape:tiled-clone-of") &&
            !strcmp(id_href, SP_OBJECT_REPR(child)->attribute("inkscape:tiled-clone-of"))) {
            to_delete = g_slist_prepend (to_delete, child);
        }
    }
    for (GSList *i = to_delete; i; i = i->next) {
        SP_OBJECT(i->data)->deleteObject();
    }
    g_slist_free (to_delete);
}


static void
clonetiler_apply (GtkWidget *widget, void *)
{

    SPDesktop *desktop = SP_ACTIVE_DESKTOP;
    if (desktop == NULL)
        return;

    SPSelection *selection = SP_DT_SELECTION(desktop);

    // check if something is selected
    if (selection->isEmpty()) {
        desktop->messageStack()->flash(Inkscape::WARNING_MESSAGE, _("Select an <b>object</b> to clone."));
        return;
    }

    // Check if more than one object is selected.
    if (g_slist_length((GSList *) selection->itemList()) > 1) {
        desktop->messageStack()->flash(Inkscape::ERROR_MESSAGE, _("If you want to clone several objects, <b>group</b> them and <b>clone the group</b>."));
        return;
    }

    SPObject *obj = SP_OBJECT(selection->singleItem());
    Inkscape::XML::Node *obj_repr = SP_OBJECT_REPR(obj);
    const char *id_href = g_strdup_printf("#%s", obj_repr->attribute("id"));
    SPObject *parent = SP_OBJECT_PARENT (obj);

    clonetiler_remove (NULL, NULL);

    double d_x_per_x = prefs_get_double_attribute_limited ("dialogs.clonetiler", "d_x_per_x", 0, -1, 1);
    double d_y_per_x = prefs_get_double_attribute_limited ("dialogs.clonetiler", "d_y_per_x", 0, -1, 1);
    double d_x_per_y = prefs_get_double_attribute_limited ("dialogs.clonetiler", "d_x_per_y", 0, -1, 1);
    double d_y_per_y = prefs_get_double_attribute_limited ("dialogs.clonetiler", "d_y_per_y", 0, -1, 1);
    int alternate_xy = prefs_get_int_attribute ("dialogs.clonetiler", "alternate_xy", 0);
    double rand_xy = prefs_get_double_attribute_limited ("dialogs.clonetiler", "rand_xy", 0, 0, 1);
    double d_rot_per_x = prefs_get_double_attribute_limited ("dialogs.clonetiler", "d_rot_per_x", 0, -180, 180);
    double d_rot_per_y = prefs_get_double_attribute_limited ("dialogs.clonetiler", "d_rot_per_y", 0, -180, 180);
    int alternate_rot = prefs_get_int_attribute ("dialogs.clonetiler", "alternate_rot", 0);
    double rand_rot = prefs_get_double_attribute_limited ("dialogs.clonetiler", "rand_rot", 0, 0, 1);
    double d_scalex_per_x = prefs_get_double_attribute_limited ("dialogs.clonetiler", "d_scalex_per_x", 0, -1, 1);
    double d_scaley_per_x = prefs_get_double_attribute_limited ("dialogs.clonetiler", "d_scaley_per_x", 0, -1, 1);
    double d_scalex_per_y = prefs_get_double_attribute_limited ("dialogs.clonetiler", "d_scalex_per_y", 0, -1, 1);
    double d_scaley_per_y = prefs_get_double_attribute_limited ("dialogs.clonetiler", "d_scaley_per_y", 0, -1, 1);
    int alternate_scale = prefs_get_int_attribute ("dialogs.clonetiler", "alternate_scale", 0);
    double rand_scale = prefs_get_double_attribute_limited ("dialogs.clonetiler", "rand_scale", 0, 0, 1);

    double d_opacity_per_y = prefs_get_double_attribute_limited ("dialogs.clonetiler", "d_opacity_per_y", 0, 0, 1);
    double d_opacity_per_x = prefs_get_double_attribute_limited ("dialogs.clonetiler", "d_opacity_per_x", 0, 0, 1);
    int alternate_opacity = prefs_get_int_attribute ("dialogs.clonetiler", "alternate_opacity", 0);
    double rand_opacity = prefs_get_double_attribute_limited ("dialogs.clonetiler", "rand_opacity", 0, 0, 1);

    int xmax = prefs_get_int_attribute ("dialogs.clonetiler", "xmax", 2);
    int ymax = prefs_get_int_attribute ("dialogs.clonetiler", "ymax", 2);

    int type = prefs_get_int_attribute ("dialogs.clonetiler", "symmetrygroup", 0);

    int keepbbox = prefs_get_int_attribute ("dialogs.clonetiler", "keepbbox", 1);

    NR::Point c;
    double w;
    double h;

    if (keepbbox && 
        obj_repr->attribute("inkscape:tile-w") &&
        obj_repr->attribute("inkscape:tile-h") &&
        obj_repr->attribute("inkscape:tile-cx") &&
        obj_repr->attribute("inkscape:tile-cy")) {

        double cx = sp_repr_get_double_attribute (obj_repr, "inkscape:tile-cx", 0);
        double cy = sp_repr_get_double_attribute (obj_repr, "inkscape:tile-cy", 0);

        c = NR::Point (cx, cy);

        w = sp_repr_get_double_attribute (obj_repr, "inkscape:tile-w", 0);
        h = sp_repr_get_double_attribute (obj_repr, "inkscape:tile-h", 0);
    } else {
        NRRect r;
        sp_item_invoke_bbox(SP_ITEM(obj), &r, sp_item_i2doc_affine(SP_ITEM(obj)), TRUE);
        c = NR::Point ((r.x0 + r.x1)/2, (r.y0 + r.y1)/2); 
        w = fabs (r.x1 - r.x0); 
        h = fabs (r.y1 - r.y0); 

        sp_repr_set_double (obj_repr, "inkscape:tile-w", w);
        sp_repr_set_double (obj_repr, "inkscape:tile-h", h);
        sp_repr_set_double (obj_repr, "inkscape:tile-cx", c[NR::X]);
        sp_repr_set_double (obj_repr, "inkscape:tile-cy", c[NR::Y]);
    }

    for (int x = 0; x < xmax; x ++) {
        for (int y = 0; y < ymax; y ++) {

            if (y == 0 && x == 0) // original
                continue;

            Inkscape::XML::Node *clone = sp_repr_new("svg:use");
            sp_repr_set_attr(clone, "x", "0");
            sp_repr_set_attr(clone, "y", "0");
            sp_repr_set_attr(clone, "inkscape:tiled-clone-of", id_href);
            sp_repr_set_attr(clone, "xlink:href", id_href);

            NR::Matrix t = clonetiler_get_transform (type, x, y, c[NR::X], c[NR::Y], w, h,
                                                     d_x_per_x, d_y_per_x, d_x_per_y, d_y_per_y, alternate_xy, rand_xy, 
                                                     d_rot_per_x, d_rot_per_y, alternate_rot, rand_rot,
                                                     d_scalex_per_x, d_scaley_per_x, d_scalex_per_y, d_scaley_per_y, alternate_scale, rand_scale);
            gchar affinestr[80];

            if (sp_svg_transform_write(affinestr, 79, t)) {
                sp_repr_set_attr (clone, "transform", affinestr);
            } else {
                sp_repr_set_attr (clone, "transform", NULL);
            }

            int eff_x = (alternate_opacity? (x%2) : (x));
            int eff_y = (alternate_opacity? (y%2) : (y));
            double d_opacity = 1 - (d_opacity_per_x * eff_x + d_opacity_per_y * eff_y + rand_opacity * g_random_double_range (-1, 1));
            d_opacity = CLAMP (d_opacity, 0, 1);
            if (d_opacity < 1.0) {
                sp_repr_set_double (clone, "opacity", d_opacity);
            }

            // add the new clone to the top of the original's parent
            SP_OBJECT_REPR(parent)->appendChild(clone);
        }
    }

    sp_document_done(SP_DT_DOCUMENT(desktop));
}

GtkWidget *
clonetiler_new_tab (GtkWidget *nb, const gchar *label)
{
    GtkWidget *l = gtk_label_new_with_mnemonic (label);
    GtkWidget *vb = gtk_vbox_new (FALSE, VB_MARGIN);
    gtk_container_set_border_width (GTK_CONTAINER (vb), VB_MARGIN);
    gtk_notebook_append_page (GTK_NOTEBOOK (nb), vb, l);
    return vb;
}

static void
clonetiler_checkbox_toggled (GtkToggleButton *tb, gpointer *data)
{
    const gchar *attr = (const gchar *) data;
    prefs_set_int_attribute ("dialogs.clonetiler", attr, gtk_toggle_button_get_active (tb));
}

GtkWidget *
clonetiler_checkbox (const char *label, GtkTooltips *tt, const char *tip, const char *attr)
{
    GtkWidget *hb = gtk_hbox_new(FALSE, VB_MARGIN);

    GtkWidget *b = gtk_check_button_new_with_label (label);
    gtk_tooltips_set_tip (GTK_TOOLTIPS (tt), b, tip, NULL);
 
    int value = prefs_get_int_attribute ("dialogs.clonetiler", attr, 0);
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(b), value);

    gtk_box_pack_end (GTK_BOX (hb), b, FALSE, TRUE, 0);
    gtk_signal_connect ( GTK_OBJECT (b), "clicked", 
                         GTK_SIGNAL_FUNC (clonetiler_checkbox_toggled), (gpointer) attr);

    g_object_set_data (G_OBJECT(b), "uncheckable", GINT_TO_POINTER(TRUE));

    return hb;
}


static void
clonetiler_percent_changed (GtkAdjustment *adj, gpointer data)
{
    const gchar *pref = (const gchar *) data;
    prefs_set_double_attribute ("dialogs.clonetiler", pref, 0.01 * adj->value);
}

static void
clonetiler_angle_changed (GtkAdjustment *adj, gpointer data)
{
    const gchar *pref = (const gchar *) data;
    prefs_set_double_attribute ("dialogs.clonetiler", pref, adj->value);
}

GtkWidget *
clonetiler_percent_spinbox (const char *label, GtkTooltips *tt, const char *tip, const char *attr, bool onlypos = false, bool angle = false)
{
    GtkWidget *hb = gtk_hbox_new(FALSE, VB_MARGIN);

    {
        GtkWidget *l = gtk_label_new (label);
        gtk_misc_set_alignment (GTK_MISC (l), 1.0, 0.5);
        gtk_box_pack_start (GTK_BOX (hb), l, TRUE, TRUE, 0);
    }

    {
        GtkWidget *l = gtk_label_new ("");
        gtk_label_set_markup (GTK_LABEL(l), angle? "&#176;" : "%");
        gtk_misc_set_alignment (GTK_MISC (l), 1.0, 0);
        gtk_box_pack_end (GTK_BOX (hb), l, FALSE, FALSE, 0);
    }

    {
        GtkObject *a;
        if (angle) {
            a = gtk_adjustment_new(0.0, -180, 180, 1, 10, 10);
        } else {
            if (onlypos)
                a = gtk_adjustment_new(0.0, 0, 100, 1, 10, 10);
            else 
                a = gtk_adjustment_new(0.0, -100, 100, 1, 10, 10);
        }

        GtkWidget *sb = gtk_spin_button_new (GTK_ADJUSTMENT (a), 1.0, 0);
        gtk_tooltips_set_tip (GTK_TOOLTIPS (tt), sb, tip, NULL);
        gtk_entry_set_width_chars (GTK_ENTRY (sb), 3);
        gtk_box_pack_end (GTK_BOX (hb), sb, FALSE, FALSE, SB_MARGIN);

        if (angle) {
            double value = prefs_get_double_attribute_limited ("dialogs.clonetiler", attr, 0, -180, 180);
            gtk_adjustment_set_value (GTK_ADJUSTMENT (a), value);
            gtk_signal_connect(GTK_OBJECT(a), "value_changed",
                           GTK_SIGNAL_FUNC(clonetiler_angle_changed), (gpointer) attr);
        } else {
            double value = prefs_get_double_attribute_limited ("dialogs.clonetiler", attr, 0, -1, 1);
            gtk_adjustment_set_value (GTK_ADJUSTMENT (a), (value * 100));
            gtk_signal_connect(GTK_OBJECT(a), "value_changed",
                           GTK_SIGNAL_FUNC(clonetiler_percent_changed), (gpointer) attr);
        }

        g_object_set_data (G_OBJECT(sb), "zeroable", GINT_TO_POINTER(TRUE));
    }

    return hb;
}

static void
clonetiler_symgroup_changed (GtkMenuItem *item, gpointer data)
{
    gint group_new = GPOINTER_TO_INT (data);
    prefs_set_int_attribute ( "dialogs.clonetiler", "symmetrygroup", group_new );
} 

static void
clonetiler_xy_changed (GtkAdjustment *adj, gpointer data)
{
    const gchar *pref = (const gchar *) data;
    prefs_set_int_attribute ("dialogs.clonetiler", pref, (int) round(adj->value));
}

static void
clonetiler_keep_bbox_toggled (GtkToggleButton *tb, gpointer data)
{
    prefs_set_int_attribute ("dialogs.clonetiler", "keepbbox", gtk_toggle_button_get_active (tb));
}

static void
clonetiler_reset_recursive (GtkWidget *w)
{
    if (w && GTK_IS_OBJECT(w)) {
        {
            int r = GPOINTER_TO_INT (gtk_object_get_data (GTK_OBJECT(w), "zeroable"));
            if (r && GTK_IS_SPIN_BUTTON(w)) { // spinbutton
                GtkAdjustment *a = gtk_spin_button_get_adjustment (GTK_SPIN_BUTTON(w));
                gtk_adjustment_set_value (a, 0);
            }
        }
        {
            int r = GPOINTER_TO_INT (gtk_object_get_data (GTK_OBJECT(w), "uncheckable"));
            if (r && GTK_IS_TOGGLE_BUTTON(w)) { // checkbox
                gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(w), FALSE);
            }
        }
    }
	
    if (GTK_IS_CONTAINER(w)) {
        GList *ch = gtk_container_get_children (GTK_CONTAINER(w));
        for (GList *i = ch; i != NULL; i = i->next) {
            clonetiler_reset_recursive (GTK_WIDGET(i->data));
        }
        g_list_free (ch);
    }
}

static void
clonetiler_reset (GtkWidget *widget, void *)
{
    clonetiler_reset_recursive (dlg);
}

void
clonetiler_dialog (void)
{
    if (!dlg)
    {
        gchar title[500];
        sp_ui_dialog_title_string (Inkscape::Verb::get(SP_VERB_DIALOG_CLONETILER), title);

        dlg = sp_window_new (title, TRUE);
        if (x == -1000 || y == -1000) {
            x = prefs_get_int_attribute (prefs_path, "x", 0);
            y = prefs_get_int_attribute (prefs_path, "y", 0);
        }
        
        if (w ==0 || h == 0) {
            w = prefs_get_int_attribute (prefs_path, "w", 0);
            h = prefs_get_int_attribute (prefs_path, "h", 0);
        }
        
        if (x != 0 || y != 0) {
            gtk_window_move ((GtkWindow *) dlg, x, y);
        
        } else {
            gtk_window_set_position(GTK_WINDOW(dlg), GTK_WIN_POS_CENTER);
        }
        
        if (w && h) {
            gtk_window_resize ((GtkWindow *) dlg, w, h);
        }
        
        sp_transientize (dlg);
        wd.win = dlg;
        wd.stop = 0;
        
        g_signal_connect   ( G_OBJECT (INKSCAPE), "activate_desktop", G_CALLBACK (sp_transientize_callback), &wd);
                             
        gtk_signal_connect ( GTK_OBJECT (dlg), "event", GTK_SIGNAL_FUNC (sp_dialog_event_handler), dlg);
        
        gtk_signal_connect ( GTK_OBJECT (dlg), "destroy", G_CALLBACK (clonetiler_dialog_destroy), dlg);
        gtk_signal_connect ( GTK_OBJECT (dlg), "delete_event", G_CALLBACK (clonetiler_dialog_delete), dlg);
        g_signal_connect   ( G_OBJECT (INKSCAPE), "shut_down", G_CALLBACK (clonetiler_dialog_delete), dlg);
        
        g_signal_connect   ( G_OBJECT (INKSCAPE), "dialogs_hide", G_CALLBACK (sp_dialog_hide), dlg);
        g_signal_connect   ( G_OBJECT (INKSCAPE), "dialogs_unhide", G_CALLBACK (sp_dialog_unhide), dlg);

        GtkTooltips *tt = gtk_tooltips_new();

        GtkWidget *mainbox = gtk_vbox_new(FALSE, 4);
        gtk_container_set_border_width (GTK_CONTAINER (mainbox), 6);
        gtk_container_add (GTK_CONTAINER (dlg), mainbox);
                            
        GtkWidget *nb = gtk_notebook_new ();
        gtk_box_pack_start (GTK_BOX (mainbox), nb, FALSE, FALSE, 0);

        {
            GtkWidget *vb = clonetiler_new_tab (nb, _("_Symmetry"));

            GtkWidget *om = gtk_option_menu_new ();
            gtk_tooltips_set_tip (GTK_TOOLTIPS (tt), om, _("Select one of the 17 symmetry groups for the tiling"), NULL);
            gtk_box_pack_start (GTK_BOX (vb), om, FALSE, FALSE, SB_MARGIN);

            GtkWidget *m = gtk_menu_new ();
            int current = prefs_get_int_attribute ("dialogs.clonetiler", "symmetrygroup", 0);

            struct SymGroups {
                int group;
                gchar const *label;
            } const sym_groups[] = {
                // TRANSLATORS: "translation" means "shift" here
                {TILE_P1, _("<b>P1</b>: simple translation")},
                {TILE_P2, _("<b>P2</b>: 180&#176; rotation")},
                {TILE_PM, _("<b>PM</b>: reflection")},
                // TRANSLATORS: "glide reflection" is a reflection and a translation combined
                //  For more info, see http://mathforum.org/sum95/suzanne/symsusan.html
                {TILE_PG, _("<b>PG</b>: glide reflection")},
                {TILE_CM, _("<b>CM</b>: reflection + glide reflection")},
                {TILE_PMM, _("<b>PMM</b>: reflection + reflection")},
                {TILE_PMG, _("<b>PMG</b>: reflection + 180&#176; rotation")},
                {TILE_PGG, _("<b>PGG</b>: glide reflection + 180&#176; rotation")},
                {TILE_CMM, _("<b>CMM</b>: reflection + reflection + 180&#176; rotation")},
                {TILE_P4, _("<b>P4</b>: 90&#176; rotation")},
                {TILE_P4M, _("<b>P4M</b>: 90&#176; rotation + 45&#176; reflection")},
                {TILE_P4G, _("<b>P4G</b>: 90&#176; rotation + 90&#176; reflection")},
                {TILE_P3, _("<b>P3</b>: 120&#176; rotation")},
                {TILE_P31M, _("<b>P31M</b>: reflection + 120&#176; rotation, dense")},
                {TILE_P3M1, _("<b>P3M1</b>: reflection + 120&#176; rotation, sparse")},
                {TILE_P6, _("<b>P6</b>: 60&#176; rotation")},
                {TILE_P6M, _("<b>P6M</b>: reflection + 60&#176; rotation")},
            };

            for (unsigned j = 0; j < G_N_ELEMENTS(sym_groups); ++j) {
                SymGroups const &sg = sym_groups[j];

                GtkWidget *l = gtk_label_new ("");
                gtk_label_set_markup (GTK_LABEL(l), sg.label);
                gtk_misc_set_alignment (GTK_MISC(l), 0, 0.5);

                GtkWidget *item = gtk_menu_item_new ();
                gtk_container_add (GTK_CONTAINER (item), l);

                gtk_signal_connect ( GTK_OBJECT (item), "activate", 
                                     GTK_SIGNAL_FUNC (clonetiler_symgroup_changed),
                                     GINT_TO_POINTER (sg.group) );

                gtk_menu_append (GTK_MENU (m), item);
            }

            gtk_option_menu_set_menu (GTK_OPTION_MENU (om), m);
            gtk_option_menu_set_history ( GTK_OPTION_MENU (om), current);
        }

        {
            GtkWidget *vb = clonetiler_new_tab (nb, _("S_hift"));

            GtkWidget *table = gtk_hbox_new (TRUE, 4);
            gtk_container_set_border_width (GTK_CONTAINER (table), 4);
            gtk_box_pack_start (GTK_BOX (vb), table, FALSE, FALSE, 0);

            {
                GtkWidget *f = gtk_frame_new (_("Per row:"));
                gtk_box_pack_start (GTK_BOX (table), f, TRUE, TRUE, 0);

                GtkWidget *fvbox = gtk_vbox_new (FALSE, 0);
                gtk_container_set_border_width (GTK_CONTAINER (fvbox), 4);
                gtk_container_add(GTK_CONTAINER(f), fvbox);

                {
                    GtkWidget *l = clonetiler_percent_spinbox (_("X:"), tt,
                    // TRANSLATORS: "shift" means: in each row of the pattern, the tiles will be shifted (offset) by this amount
                    // xgettext:no-c-format
                    _("Horizontal shift per each row (in % of tile width)"), "d_x_per_y");
                    gtk_box_pack_start (GTK_BOX (fvbox), l, FALSE, FALSE, VB_SKIP);
                }

                {
                    GtkWidget *l = clonetiler_percent_spinbox (_("Y:"), tt,
                    // TRANSLATORS: "shift" means: in each row of the pattern, the tiles will be shifted (offset) by this amount
                    // xgettext:no-c-format
                    _("Vertical shift per each row (in % of tile height)"), "d_y_per_y");
                    gtk_box_pack_start (GTK_BOX (fvbox), l, FALSE, FALSE, VB_SKIP);
                }
            }

            {
                GtkWidget *f = gtk_frame_new (_("Per column:"));
                gtk_box_pack_start (GTK_BOX (table), f, TRUE, TRUE, 0);


                GtkWidget *fvbox = gtk_vbox_new (FALSE, 0);
                gtk_container_set_border_width (GTK_CONTAINER (fvbox), 4);
                gtk_container_add(GTK_CONTAINER(f), fvbox);

                {
                    // xgettext:no-c-format
                    GtkWidget *l = clonetiler_percent_spinbox (_("X:"), tt, _("Horizontal shift per each column (in % of tile width)"), "d_x_per_x");
                    gtk_box_pack_start (GTK_BOX (fvbox), l, FALSE, FALSE, VB_SKIP);
                }


                {
                    // xgettext:no-c-format
                    GtkWidget *l = clonetiler_percent_spinbox (_("Y:"), tt, _("Vertical shift per each column (in % of tile height)"), "d_y_per_x");
                    gtk_box_pack_start (GTK_BOX (fvbox), l, FALSE, FALSE, VB_SKIP);
                }
            }


            {
                // TRANSLATORS: "Alternate" is a verb here
                GtkWidget *l = clonetiler_checkbox (_("Alternate sign"), tt, _("Alternate the sign of the shifts for each row or column"), "alternate_xy");
                gtk_container_set_border_width (GTK_CONTAINER (l), 4);
                gtk_box_pack_start (GTK_BOX (vb), l, FALSE, FALSE, 0);
            }

            {
                GtkWidget *l = clonetiler_percent_spinbox (_("Randomize:"), tt, _("How much to randomize tile positions"),"rand_xy", true);
                gtk_container_set_border_width (GTK_CONTAINER (l), 4);
                gtk_box_pack_start (GTK_BOX (vb), l, FALSE, FALSE, 0);
            }
        }



        {
            GtkWidget *vb = clonetiler_new_tab (nb, _("Sc_ale"));

            GtkWidget *table = gtk_hbox_new (TRUE, 4);
            gtk_container_set_border_width (GTK_CONTAINER (table), 4);
            gtk_box_pack_start (GTK_BOX (vb), table, FALSE, FALSE, 0);

            {
                GtkWidget *f = gtk_frame_new (_("Per row:"));
                gtk_box_pack_start (GTK_BOX (table), f, TRUE, TRUE, 0);

                GtkWidget *fvbox = gtk_vbox_new (FALSE, 0);
                gtk_container_set_border_width (GTK_CONTAINER (fvbox), 4);
                gtk_container_add(GTK_CONTAINER(f), fvbox);

                {
                    GtkWidget *l = clonetiler_percent_spinbox (_("X:"), tt, _("Horizontal scale per each row"), "d_scalex_per_y");
                    gtk_box_pack_start (GTK_BOX (fvbox), l, FALSE, FALSE, VB_SKIP);
                }

                {
                    GtkWidget *l = clonetiler_percent_spinbox (_("Y:"), tt, _("Vertical scale per each row"), "d_scaley_per_y");
                    gtk_box_pack_start (GTK_BOX (fvbox), l, FALSE, FALSE, VB_SKIP);
                }
            }

            {
                GtkWidget *f = gtk_frame_new (_("Per column:"));
                gtk_box_pack_start (GTK_BOX (table), f, TRUE, TRUE, 0);


                GtkWidget *fvbox = gtk_vbox_new (FALSE, 0);
                gtk_container_set_border_width (GTK_CONTAINER (fvbox), 4);
                gtk_container_add(GTK_CONTAINER(f), fvbox);

                {
                    GtkWidget *l = clonetiler_percent_spinbox (_("X:"), tt, _("Horizontal scale per each column"), "d_scalex_per_x");
                    gtk_box_pack_start (GTK_BOX (fvbox), l, FALSE, FALSE, VB_SKIP);
                }


                {
                    GtkWidget *l = clonetiler_percent_spinbox (_("Y:"), tt, _("Vertical scale per each column"), "d_scaley_per_x");
                    gtk_box_pack_start (GTK_BOX (fvbox), l, FALSE, FALSE, VB_SKIP);
                }
            }


            {
                // TRANSLATORS: "Alternate" is a verb here
                GtkWidget *l = clonetiler_checkbox (_("Alternate sign"), tt, _("Alternate the sign of the scale increment for each row or column"), "alternate_scale");
                gtk_container_set_border_width (GTK_CONTAINER (l), 4);
                gtk_box_pack_start (GTK_BOX (vb), l, FALSE, FALSE, 0);
            }

            {
                GtkWidget *l = clonetiler_percent_spinbox (_("Randomize:"), tt, _("How much to randomize tile sizes"),"rand_scale", true);
                gtk_container_set_border_width (GTK_CONTAINER (l), 4);
                gtk_box_pack_start (GTK_BOX (vb), l, FALSE, FALSE, 0);
            }
        }


        {
            GtkWidget *vb = clonetiler_new_tab (nb, _("_Rotation"));

            GtkWidget *table = gtk_hbox_new (TRUE, 4);
            gtk_container_set_border_width (GTK_CONTAINER (table), 4);
            gtk_box_pack_start (GTK_BOX (vb), table, FALSE, FALSE, 0);

            {
                GtkWidget *f = gtk_frame_new (_("Per row:"));
                gtk_box_pack_start (GTK_BOX (table), f, TRUE, TRUE, 0);

                GtkWidget *fvbox = gtk_vbox_new (FALSE, 0);
                gtk_container_set_border_width (GTK_CONTAINER (fvbox), 4);
                gtk_container_add(GTK_CONTAINER(f), fvbox);

                {
                    GtkWidget *l = clonetiler_percent_spinbox (_("Angle:"), tt, _("Rotate tiles by this angle for each row"), "d_rot_per_y", false, true);
                    gtk_box_pack_start (GTK_BOX (fvbox), l, FALSE, FALSE, VB_SKIP);
                }
            }

            {
                GtkWidget *f = gtk_frame_new (_("Per column:"));
                gtk_box_pack_start (GTK_BOX (table), f, TRUE, TRUE, 0);


                GtkWidget *fvbox = gtk_vbox_new (FALSE, 0);
                gtk_container_set_border_width (GTK_CONTAINER (fvbox), 4);
                gtk_container_add(GTK_CONTAINER(f), fvbox);

                {
                    GtkWidget *l = clonetiler_percent_spinbox (_("Angle:"), tt, _("Rotate tiles by this angle for each column"), "d_rot_per_x", false, true);
                    gtk_box_pack_start (GTK_BOX (fvbox), l, FALSE, FALSE, VB_SKIP);
                }
            }


            {
                // TRANSLATORS: "Alternate" is a verb here
                GtkWidget *l = clonetiler_checkbox (_("Alternate sign"), tt, _("Alternate the rotation direction for each row or column"), "alternate_rot");
                gtk_container_set_border_width (GTK_CONTAINER (l), 4);
                gtk_box_pack_start (GTK_BOX (vb), l, FALSE, FALSE, 0);
            }

            {
                GtkWidget *l = clonetiler_percent_spinbox (_("Randomize:"), tt, _("How much to randomize tile rotation"),"rand_rot", true);
                gtk_container_set_border_width (GTK_CONTAINER (l), 4);
                gtk_box_pack_start (GTK_BOX (vb), l, FALSE, FALSE, 0);
            }
        }


        {
            GtkWidget *vb = clonetiler_new_tab (nb, _("_Opacity"));

            GtkWidget *table = gtk_hbox_new (TRUE, 4);
            gtk_container_set_border_width (GTK_CONTAINER (table), 4);
            gtk_box_pack_start (GTK_BOX (vb), table, FALSE, FALSE, 0);

            {
                GtkWidget *f = gtk_frame_new (_("Per row:"));
                gtk_box_pack_start (GTK_BOX (table), f, TRUE, TRUE, 0);

                GtkWidget *fvbox = gtk_vbox_new (FALSE, 0);
                gtk_container_set_border_width (GTK_CONTAINER (fvbox), 4);
                gtk_container_add(GTK_CONTAINER(f), fvbox);

                {
                    GtkWidget *l = clonetiler_percent_spinbox (_("Dissolve:"), tt, _("Decrease tile opacity by this percentage for each row"), "d_opacity_per_y", true);
                    gtk_box_pack_start (GTK_BOX (fvbox), l, FALSE, FALSE, VB_SKIP);
                }
            }

            {
                GtkWidget *f = gtk_frame_new (_("Per column:"));
                gtk_box_pack_start (GTK_BOX (table), f, TRUE, TRUE, 0);


                GtkWidget *fvbox = gtk_vbox_new (FALSE, 0);
                gtk_container_set_border_width (GTK_CONTAINER (fvbox), 4);
                gtk_container_add(GTK_CONTAINER(f), fvbox);

                {
                    GtkWidget *l = clonetiler_percent_spinbox (_("Dissolve:"), tt, _("Decrease tile opacity by this percentage for each column"), "d_opacity_per_x", true);
                    gtk_box_pack_start (GTK_BOX (fvbox), l, FALSE, FALSE, VB_SKIP);
                }
            }


            {
                // TRANSLATORS: "Alternate" is a verb here
                GtkWidget *l = clonetiler_checkbox (_("Alternate sign"), tt, _("Alternate the sign of opacity changes for each row or column"), "alternate_opacity");
                gtk_container_set_border_width (GTK_CONTAINER (l), 4);
                gtk_box_pack_start (GTK_BOX (vb), l, FALSE, FALSE, 0);
            }

            {
                GtkWidget *l = clonetiler_percent_spinbox (_("Randomize:"), tt, _("How much to randomize tile opacity"),"rand_opacity", true);
                gtk_container_set_border_width (GTK_CONTAINER (l), 4);
                gtk_box_pack_start (GTK_BOX (vb), l, FALSE, FALSE, 0);
            }
        }


        {
            GtkWidget *table = gtk_hbox_new (TRUE, 4);
            gtk_container_set_border_width (GTK_CONTAINER (table), 4);
            gtk_box_pack_start (GTK_BOX (mainbox), table, FALSE, FALSE, VB_SKIP);

            {
                GtkWidget *hb = gtk_hbox_new(FALSE, VB_MARGIN);

                {
                    GtkWidget *l = gtk_label_new ("");
                    gtk_label_set_markup (GTK_LABEL(l), _("<b>Rows</b>:"));
                    gtk_misc_set_alignment (GTK_MISC (l), 1.0, 0.5);
                    gtk_box_pack_start (GTK_BOX (hb), l, TRUE, TRUE, 0);
                }

                {
                    GtkObject *a = gtk_adjustment_new(0.0, 1, 500, 1, 10, 10);
                    int value = prefs_get_int_attribute ("dialogs.clonetiler", "ymax", 2);
                    gtk_adjustment_set_value (GTK_ADJUSTMENT (a), value);
                    GtkWidget *sb = gtk_spin_button_new (GTK_ADJUSTMENT (a), 1.0, 0);
                    gtk_tooltips_set_tip (GTK_TOOLTIPS (tt), sb, _("How many rows in the tiling"), NULL);
                    gtk_entry_set_width_chars (GTK_ENTRY (sb), 6);
                    gtk_box_pack_end (GTK_BOX (hb), sb, FALSE, FALSE, SB_MARGIN);

                    gtk_signal_connect(GTK_OBJECT(a), "value_changed",
                                       GTK_SIGNAL_FUNC(clonetiler_xy_changed), (gpointer) "ymax");
                }

                gtk_box_pack_start (GTK_BOX (table), hb, FALSE, FALSE, 0);
            }

            {
                GtkWidget *hb = gtk_hbox_new(FALSE, 4);

                {
                    GtkWidget *l = gtk_label_new ("");
                    gtk_label_set_markup (GTK_LABEL(l), _("<b>Columns</b>:"));
                    gtk_misc_set_alignment (GTK_MISC (l), 1.0, 0.5);
                    gtk_box_pack_start (GTK_BOX (hb), l, TRUE, TRUE, 0);
                }

                {
                    GtkObject *a = gtk_adjustment_new(0.0, 1, 500, 1, 10, 10);
                    int value = prefs_get_int_attribute ("dialogs.clonetiler", "xmax", 2);
                    gtk_adjustment_set_value (GTK_ADJUSTMENT (a), value);
                    GtkWidget *sb = gtk_spin_button_new (GTK_ADJUSTMENT (a), 1.0, 0);
                    gtk_tooltips_set_tip (GTK_TOOLTIPS (tt), sb, _("How many columns in the tiling"), NULL);
                    gtk_entry_set_width_chars (GTK_ENTRY (sb), 6);
                    gtk_box_pack_end (GTK_BOX (hb), sb, FALSE, FALSE, SB_MARGIN);

                    gtk_signal_connect(GTK_OBJECT(a), "value_changed",
                                       GTK_SIGNAL_FUNC(clonetiler_xy_changed), (gpointer) "xmax");
                }

                gtk_box_pack_start (GTK_BOX (table), hb, FALSE, FALSE, 0);
            }
        }

        {
            GtkWidget *hb = gtk_hbox_new(FALSE, VB_MARGIN);
            gtk_box_pack_start (GTK_BOX (mainbox), hb, FALSE, FALSE, 0);

            GtkWidget *b  = gtk_check_button_new_with_label (_("Use saved size and position of the tile"));
            gint keepbbox = prefs_get_int_attribute ("dialogs.clonetiler", "keepbbox", 1);
            gtk_toggle_button_set_active ((GtkToggleButton *) b, keepbbox != 0);
            gtk_tooltips_set_tip (GTK_TOOLTIPS (tt), b, _("Pretend that the size and position of the tile are the same as the last time you tiled it (if any), instead of using the current size"), NULL);
            gtk_box_pack_start (GTK_BOX (hb), b, FALSE, FALSE, 0);

            gtk_signal_connect(GTK_OBJECT(b), "toggled",
                               GTK_SIGNAL_FUNC(clonetiler_keep_bbox_toggled), NULL);

        }

        {
            GtkWidget *hb = gtk_hbox_new(FALSE, VB_MARGIN);
            gtk_box_pack_start (GTK_BOX (mainbox), hb, FALSE, FALSE, 0);

            {
                GtkWidget *b = gtk_button_new ();
                GtkWidget *l = gtk_label_new ("");
                gtk_label_set_markup_with_mnemonic (GTK_LABEL(l), _(" <b>_Create</b> "));
                gtk_container_add (GTK_CONTAINER(b), l);
                gtk_tooltips_set_tip (tt, b, _("Create and tile the clones of the selection"), NULL);
                gtk_signal_connect (GTK_OBJECT (b), "clicked", GTK_SIGNAL_FUNC (clonetiler_apply), NULL);
                gtk_box_pack_end (GTK_BOX (hb), b, FALSE, FALSE, 0);
            }

            {
                GtkWidget *b = gtk_button_new_with_mnemonic (_(" Re_move "));
                gtk_tooltips_set_tip (tt, b, _("Remove existing tiled clones of the selected object (siblings only)"), NULL);
                gtk_signal_connect (GTK_OBJECT (b), "clicked", GTK_SIGNAL_FUNC (clonetiler_remove), NULL);
                gtk_box_pack_end (GTK_BOX (hb), b, FALSE, FALSE, 0);
            }

            {
                GtkWidget *b = gtk_button_new_with_mnemonic (_(" R_eset "));
                // TRANSLATORS: "change" is a noun here
                gtk_tooltips_set_tip (tt, b, _("Reset all shifts, scales, rotates, and opacity changes to zero"), NULL);
                gtk_signal_connect (GTK_OBJECT (b), "clicked", GTK_SIGNAL_FUNC (clonetiler_reset), NULL);
                gtk_box_pack_end (GTK_BOX (hb), b, FALSE, FALSE, 0);
            }
        }

        gtk_widget_show_all (mainbox);

    } // end of if (!dlg)
    
    gtk_window_present ((GtkWindow *) dlg);
}



/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=c++:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :
