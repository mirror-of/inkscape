#define __SP_QUICK_ALIGN_C__

/*
 * Object align dialog
 *
 * Authors:
 *   Frank Felfe <innerspace@iname.com>
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * Copyright (C) 1999-2002 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <config.h>

#include <math.h>
#include <stdlib.h>
#include <libnr/nr-macros.h>
#include <gtk/gtksignal.h>
#include <gtk/gtknotebook.h>
#include <gtk/gtkvbox.h>
#include <gtk/gtktable.h>
#include <gtk/gtkoptionmenu.h>
#include <gtk/gtkmenuitem.h>
#include <gtk/gtklabel.h>

#include "helper/sp-intl.h"
#include "helper/window.h"
#include "widgets/button.h"
#include "sodipodi.h"
#include "document.h"
#include "desktop-handles.h"
#include "sp-item-transform.h"
#include "selection.h"

#include "align.h"

/*
 * handler functions for quick align dialog
 *
 * todo: dialog with more aligns
 * - more aligns (30 % left from center ...)
 * - aligns for nodes
 *
 */ 

enum {
	SP_ALIGN_LAST,
	SP_ALIGN_FIRST,
	SP_ALIGN_BIGGEST,
	SP_ALIGN_SMALLEST,
	SP_ALIGN_PAGE,
	SP_ALIGN_DRAWING,
	SP_ALIGN_SELECTION,
};

enum {
	SP_ALIGN_TOP_IN,
	SP_ALIGN_TOP_OUT,
	SP_ALIGN_RIGHT_IN,
	SP_ALIGN_RIGHT_OUT,
	SP_ALIGN_BOTTOM_IN,
	SP_ALIGN_BOTTOM_OUT,
	SP_ALIGN_LEFT_IN,
	SP_ALIGN_LEFT_OUT,
	SP_ALIGN_CENTER_HOR,
	SP_ALIGN_CENTER_VER,
};

enum {
	SP_DISTRIBUTE_LEFT,
	SP_DISTRIBUTE_HCENTRE,
	SP_DISTRIBUTE_RIGHT,
	SP_DISTRIBUTE_HDIST
};
	
enum {
	SP_DISTRIBUTE_TOP,
	SP_DISTRIBUTE_VCENTRE,
	SP_DISTRIBUTE_BOTTOM,
	SP_DISTRIBUTE_VDIST
};

static const unsigned char aligns[10][8] = {
	{0, 0, 0, 2, 0, 0, 0, 2},
	{0, 0, 0, 2, 0, 0, 2, 0},
	{0, 2, 0, 0, 0, 2, 0, 0},
	{0, 2, 0, 0, 2, 0, 0, 0},
	{0, 0, 2, 0, 0, 0, 2, 0},
	{0, 0, 2, 0, 0, 0, 0, 2},
	{2, 0, 0, 0, 2, 0, 0, 0},
	{2, 0, 0, 0, 0, 2, 0, 0},
	{1, 1, 0, 0, 1, 1, 0, 0},
	{0, 0, 1, 1, 0, 0, 1, 1}
};

static const unsigned char hdist[4][3] = {
	{2, 0, 0},
	{1, 1, 0},
	{0, 2, 0},
	{1, 1, 1}
};

static const unsigned char vdist[4][3] = {
	{0, 2, 0},
	{1, 1, 0},
	{2, 0, 0},
	{1, 1, 1}
};

void sp_align_arrange_clicked (GtkWidget *widget, const unsigned char *aligns);
void sp_align_distribute_h_clicked (GtkWidget *widget, const unsigned char *layout);
void sp_align_distribute_v_clicked (GtkWidget *widget, const unsigned char *layout);

void sp_quick_align_dialog_close (void);

static GtkWidget *sp_align_dialog_create_base_menu (void);
static void set_base (GtkMenuItem * menuitem, gpointer data);
static SPItem * sp_quick_align_find_master (const GSList * slist, gboolean horizontal);

static GtkWidget *dlg = NULL;

static unsigned int base = SP_ALIGN_LAST;

static int
sp_quick_align_dialog_delete (void)
{
	if (GTK_WIDGET_VISIBLE (dlg)) gtk_widget_hide (dlg);

	return TRUE;
}

static void
sp_align_add_button (GtkWidget *t, int col, int row, GCallback handler, gconstpointer data, 
		     const unsigned char *px, const unsigned char *tip,
		     GtkTooltips * tt)
{
	GtkWidget *b;
	b = sp_button_new_from_data (24, SP_BUTTON_TYPE_NORMAL, px, tip, tt);
	gtk_widget_show (b);
	if (handler) g_signal_connect (G_OBJECT (b), "clicked", handler, (gpointer) data);
	gtk_table_attach (GTK_TABLE (t), b, col, col + 1, row, row + 1, 0, 0, 0, 0);
}

void
sp_quick_align_dialog (void)
{
	if (!dlg) {
		GtkWidget *nb, *vb, *om, *t, *l;
		GtkTooltips * tt = gtk_tooltips_new ();

		dlg = sp_window_new (_("Align objects"), FALSE);
		g_signal_connect (G_OBJECT (dlg), "delete_event", G_CALLBACK (sp_quick_align_dialog_delete), NULL);

		nb = gtk_notebook_new ();
		gtk_container_add (GTK_CONTAINER (dlg), nb);

		/* Align */

		vb = gtk_vbox_new (FALSE, 4);
		gtk_container_set_border_width (GTK_CONTAINER (vb), 4);

		om = gtk_option_menu_new ();
		gtk_box_pack_start (GTK_BOX (vb), om, FALSE, FALSE, 0);
		gtk_option_menu_set_menu (GTK_OPTION_MENU (om), sp_align_dialog_create_base_menu ());

		t = gtk_table_new (2, 5, TRUE);
		gtk_box_pack_start (GTK_BOX (vb), t, FALSE, FALSE, 0);

		sp_align_add_button (t, 0, 0, G_CALLBACK (sp_align_arrange_clicked), aligns[SP_ALIGN_LEFT_OUT], "al_left_out",
				     _("Right side of aligned objects to left side of anchor"),
				     tt);
		sp_align_add_button (t, 1, 0, G_CALLBACK (sp_align_arrange_clicked), aligns[SP_ALIGN_LEFT_IN], "al_left_in",
				     _("Left side of aligned objects to left side of anchor"),
				     tt);
		sp_align_add_button (t, 2, 0, G_CALLBACK (sp_align_arrange_clicked), aligns[SP_ALIGN_CENTER_HOR], "al_center_hor",
				     _("Center horizontally"),
				     tt);
		sp_align_add_button (t, 3, 0, G_CALLBACK (sp_align_arrange_clicked), aligns[SP_ALIGN_RIGHT_IN], "al_right_in",
				     _("Right side of aligned objects to right side of anchor"),
				     tt);
		sp_align_add_button (t, 4, 0, G_CALLBACK (sp_align_arrange_clicked), aligns[SP_ALIGN_RIGHT_OUT], "al_right_out",
				     _("Left side of aligned objects to right side of anchor"),
				     tt);

		sp_align_add_button (t, 0, 1, G_CALLBACK (sp_align_arrange_clicked), aligns[SP_ALIGN_TOP_OUT], "al_top_out",
				     _("Bottom of aligned objects to top of anchor"),
				     tt);
		sp_align_add_button (t, 1, 1, G_CALLBACK (sp_align_arrange_clicked), aligns[SP_ALIGN_TOP_IN], "al_top_in",
				     _("Top of aligned objects to top of anchor"),
				     tt);
		sp_align_add_button (t, 2, 1, G_CALLBACK (sp_align_arrange_clicked), aligns[SP_ALIGN_CENTER_VER], "al_center_ver",
				     _("Center vertically"),
				     tt);
		sp_align_add_button (t, 3, 1, G_CALLBACK (sp_align_arrange_clicked), aligns[SP_ALIGN_BOTTOM_IN], "al_bottom_in",
				     _("Bottom of aligned objects to bottom of anchor"),
				     tt);
		sp_align_add_button (t, 4, 1, G_CALLBACK (sp_align_arrange_clicked), aligns[SP_ALIGN_BOTTOM_OUT], "al_bottom_out",
				     _("Top of aligned objects to top of anchor"),
				     tt);

		l = gtk_label_new (_("Align"));
		gtk_widget_show (l);
		gtk_notebook_append_page (GTK_NOTEBOOK (nb), vb, l);

		/* Distribute */

		vb = gtk_vbox_new (FALSE, 4);
		gtk_container_set_border_width (GTK_CONTAINER (vb), 4);

		om = gtk_option_menu_new ();
		gtk_box_pack_start (GTK_BOX (vb), om, FALSE, FALSE, 0);
		gtk_option_menu_set_menu (GTK_OPTION_MENU (om), sp_align_dialog_create_base_menu ());
		gtk_widget_set_sensitive (om, FALSE);

		t = gtk_table_new (2, 4, TRUE);
		gtk_box_pack_start (GTK_BOX (vb), t, FALSE, FALSE, 0);

		sp_align_add_button (t, 0, 0, G_CALLBACK (sp_align_distribute_h_clicked), hdist[SP_DISTRIBUTE_LEFT], "distribute_left",
				     _("Distribute left sides of objects at even distances"),
				     tt);
		sp_align_add_button (t, 1, 0, G_CALLBACK (sp_align_distribute_h_clicked), hdist[SP_DISTRIBUTE_HCENTRE], "distribute_hcentre",
				     _("Distribute centres of objects at even distances horizontally"),
				     tt);
		sp_align_add_button (t, 2, 0, G_CALLBACK (sp_align_distribute_h_clicked), hdist[SP_DISTRIBUTE_RIGHT], "distribute_right",
				     _("Distribute right sides of objects at even distances"),
				     tt);
		sp_align_add_button (t, 3, 0, G_CALLBACK (sp_align_distribute_h_clicked), hdist[SP_DISTRIBUTE_HDIST], "distribute_hdist",
				     _("Distribute horizontal distance between objects equally"),
				     tt);

		sp_align_add_button (t, 0, 1, G_CALLBACK (sp_align_distribute_v_clicked), vdist[SP_DISTRIBUTE_TOP], "distribute_top",
				     _("Distribute top sides of objects at even distances"),
				     tt);
		sp_align_add_button (t, 1, 1, G_CALLBACK (sp_align_distribute_v_clicked), vdist[SP_DISTRIBUTE_VCENTRE], "distribute_vcentre",
				     _("Distribute centres of objects at even distances vertically"),
				     tt);
		sp_align_add_button (t, 2, 1, G_CALLBACK (sp_align_distribute_v_clicked), vdist[SP_DISTRIBUTE_BOTTOM], "distribute_bottom",
				     _("Distribute bottom sides of objects at even distances"),
				     tt);
		sp_align_add_button (t, 3, 1, G_CALLBACK (sp_align_distribute_v_clicked), vdist[SP_DISTRIBUTE_VDIST], "distribute_vdist",
				     _("Distribute vertical distance between objects equally"),
				     tt);

		l = gtk_label_new (_("Distribute"));
		gtk_widget_show (l);
		gtk_notebook_append_page (GTK_NOTEBOOK (nb), vb, l);

		gtk_widget_show_all (nb);
	}

	if (!GTK_WIDGET_VISIBLE (dlg)) gtk_widget_show (dlg);
}

void
sp_quick_align_dialog_close (void)
{
	g_assert (dlg != NULL);

	if (GTK_WIDGET_VISIBLE (dlg)) gtk_widget_hide (dlg);
}

static void
sp_align_add_menuitem (GtkWidget *menu, const unsigned char *label, GCallback handler, int value)
{
	GtkWidget *menuitem;

	menuitem = gtk_menu_item_new_with_label (label);
	gtk_widget_show (menuitem);
	if (handler) g_signal_connect (G_OBJECT (menuitem), "activate", handler, GINT_TO_POINTER (value));
	gtk_menu_append (GTK_MENU (menu), menuitem);
}

static GtkWidget *
sp_align_dialog_create_base_menu (void)
{
	GtkWidget *menu;

	menu = gtk_menu_new ();

	sp_align_add_menuitem (menu, _("Last selected"), G_CALLBACK (set_base), SP_ALIGN_LAST);
	sp_align_add_menuitem (menu, _("First selected"), G_CALLBACK (set_base), SP_ALIGN_FIRST);
	sp_align_add_menuitem (menu, _("Biggest item"), G_CALLBACK (set_base), SP_ALIGN_BIGGEST);
	sp_align_add_menuitem (menu, _("Smallest item"), G_CALLBACK (set_base), SP_ALIGN_SMALLEST);
	sp_align_add_menuitem (menu, _("Page"), G_CALLBACK (set_base), SP_ALIGN_PAGE);
	sp_align_add_menuitem (menu, _("Drawing"), G_CALLBACK (set_base), SP_ALIGN_DRAWING);
	sp_align_add_menuitem (menu, _("Selection"), G_CALLBACK (set_base), SP_ALIGN_SELECTION);

	gtk_widget_show (menu);

	return menu;
}

static void
set_base (GtkMenuItem *menuitem, gpointer data)
{
	base = GPOINTER_TO_UINT (data);
}


void
sp_align_arrange_clicked (GtkWidget *widget, const unsigned char *aligns)
{
	float mx0, mx1, my0, my1;
	float sx0, sx1, sy0, sy1;
	SPDesktop * desktop;
	SPSelection * selection;
	GSList * slist;
	SPItem * master, * item;
	NRRectF b;
	NRPointF mp, sp;
	GSList * l;
	gboolean changed;

	mx0 = 0.5 * aligns[0];
	mx1 = 0.5 * aligns[1];
	my0 = 0.5 * aligns[2];
	my1 = 0.5 * aligns[3];
	sx0 = 0.5 * aligns[4];
	sx1 = 0.5 * aligns[5];
	sy0 = 0.5 * aligns[6];
	sy1 = 0.5 * aligns[7];

	desktop = SP_ACTIVE_DESKTOP;
	if (!desktop) return;

	selection = SP_DT_SELECTION (desktop);
	slist = (GSList *) sp_selection_item_list (selection);
	if (!slist) return;

	switch (base) {
	case SP_ALIGN_LAST:
	case SP_ALIGN_FIRST:
	case SP_ALIGN_BIGGEST:
	case SP_ALIGN_SMALLEST:
		if (!slist->next) return;
		slist = g_slist_copy (slist);
		master = sp_quick_align_find_master (slist, (mx0 != 0.0) || (mx1 != 0.0));
		slist = g_slist_remove (slist, master);
		sp_item_bbox_desktop (master, &b);
		mp.x = mx0 * b.x0 + mx1 * b.x1;
		mp.y = my0 * b.y0 + my1 * b.y1;
		break;
	case SP_ALIGN_PAGE:
		slist = g_slist_copy (slist);
		mp.x = mx1 * sp_document_width (SP_DT_DOCUMENT (desktop));
		mp.y = my1 * sp_document_height (SP_DT_DOCUMENT (desktop));
		break;
	case SP_ALIGN_DRAWING:
		slist = g_slist_copy (slist);
		sp_item_bbox_desktop ((SPItem *) sp_document_root (SP_DT_DOCUMENT (desktop)), &b);
		mp.x = mx0 * b.x0 + mx1 * b.x1;
		mp.y = my0 * b.y0 + my1 * b.y1;
		break;
	case SP_ALIGN_SELECTION:
		slist = g_slist_copy (slist);
		sp_selection_bbox (selection, &b);
		mp.x = mx0 * b.x0 + mx1 * b.x1;
		mp.y = my0 * b.y0 + my1 * b.y1;
		break;
	default:
		g_assert_not_reached ();
		break;
	};

	changed = FALSE;

	for (l = slist; l != NULL; l = l->next) {
		item = (SPItem *) l->data;
		sp_item_bbox_desktop (item, &b);
		sp.x = sx0 * b.x0 + sx1 * b.x1;
		sp.y = sy0 * b.y0 + sy1 * b.y1;

		if ((fabs (mp.x - sp.x) > 1e-9) || (fabs (mp.y - sp.y) > 1e-9)) {
			sp_item_move_rel (item, mp.x - sp.x, mp.y - sp.y);
			changed = TRUE;
		}
	}

	g_slist_free (slist);

	if (changed) {
		sp_selection_changed (selection);
		sp_document_done (SP_DT_DOCUMENT (desktop));
	}
}

static SPItem *
sp_quick_align_find_master (const GSList *slist, gboolean horizontal)
{
	NRRectF b;
	const GSList * l;
	SPItem * master, * item;
	gdouble dim, max;

	master = NULL;

	switch (base) {
	case SP_ALIGN_LAST:
		return (SPItem *) slist->data;
		break;
	case SP_ALIGN_FIRST: 
		return (SPItem *) g_slist_last ((GSList *) slist)->data;
		break;
	case SP_ALIGN_BIGGEST:
		max = -1e18;
		for (l = slist; l != NULL; l = l->next) {
			item = (SPItem *) l->data;
			sp_item_bbox_desktop (item, &b);
			if (horizontal) {
				dim = b.x1 - b.x0; 
			} else {
				dim = b.y1 - b.y0;
			}
			if (dim > max) {
				max = dim;
				master = item;
			}
		}
		return master;
		break;
	case SP_ALIGN_SMALLEST:
		max = 1e18;
		for (l = slist; l != NULL; l = l->next) {
			item = (SPItem *) l->data;
			sp_item_bbox_desktop (item, &b);
			if (horizontal) {
				dim = b.x1 - b.x0;
			} else {
				dim = b.y1 - b.y0;
			}
			if (dim < max) {
				max = dim;
				master = item;
			}
		}
		return master;
		break;
	default:
		g_assert_not_reached ();
		break;
	}

	return NULL;
}

struct _SPBBoxSort {
	SPItem *item;
	NRRectF bbox;
	float anchor;
};

static int
sp_align_bbox_sort (const void *a, const void *b)
{
	const struct _SPBBoxSort *bbsa, *bbsb;
	bbsa = (struct _SPBBoxSort *) a;
	bbsb = (struct _SPBBoxSort *) b;
	if (bbsa->anchor < bbsb->anchor) return -1;
	if (bbsa->anchor > bbsb->anchor) return 1;
	return 0;
}

void
sp_align_distribute_h_clicked (GtkWidget *widget, const unsigned char *layout)
{
	SPDesktop *desktop;
	SPSelection *selection;
	const GSList *slist, *l;
	struct _SPBBoxSort *bbs;
	int len, pos;
	unsigned int changed;

	desktop = SP_ACTIVE_DESKTOP;
	if (!desktop) return;

	selection = SP_DT_SELECTION (desktop);
	slist = sp_selection_item_list (selection);
	if (!slist) return;
	if (!slist->next) return;

	len = g_slist_length ((GSList *) slist);
	bbs = g_new (struct _SPBBoxSort, len);
	pos = 0;
	for (l = slist; l != NULL; l = l->next) {
		bbs[pos].item = SP_ITEM (l->data);
		sp_item_bbox_desktop (bbs[pos].item, &bbs[pos].bbox);
		bbs[pos].anchor = 0.5 * layout[0] * bbs[pos].bbox.x0 + 0.5 * layout[1] * bbs[pos].bbox.x1;
		pos += 1;
	}

	qsort (bbs, len, sizeof (struct _SPBBoxSort), sp_align_bbox_sort);

	changed = FALSE;

	if (!layout[2]) {
		float dist, step;
		int i;
		dist = bbs[len - 1].anchor - bbs[0].anchor;
		step = dist / (len - 1);
		for (i = 0; i < len; i++) {
			float pos;
			pos = bbs[0].anchor + i * step;
			if (!NR_DF_TEST_CLOSE (pos, bbs[i].anchor, 1e-6)) {
				sp_item_move_rel (bbs[i].item, pos - bbs[i].anchor, 0.0);
				changed = TRUE;
			}
		}
	} else {
		/* Damn I am not sure, how to order them initially (Lauris) */
		float dist, span, step, pos;
		int i;
		dist = bbs[len - 1].bbox.x1 - bbs[0].bbox.x0;
		span = 0;
		for (i = 0; i < len; i++) span += (bbs[i].bbox.x1 - bbs[i].bbox.x0);
		step = (dist - span) / (len - 1);
		pos = bbs[0].bbox.x0;
		for (i = 0; i < len; i++) {
			if (!NR_DF_TEST_CLOSE (pos, bbs[i].bbox.x0, 1e-6)) {
				sp_item_move_rel (bbs[i].item, pos - bbs[i].bbox.x0, 0.0);
				changed = TRUE;
			}
			pos += (bbs[i].bbox.x1 - bbs[i].bbox.x0);
			pos += step;
		}
	}

	g_free (bbs);

	if (changed) {
		sp_selection_changed (selection);
		sp_document_done (SP_DT_DOCUMENT (desktop));
	}
}

void
sp_align_distribute_v_clicked (GtkWidget *widget, const unsigned char *layout)
{
	SPDesktop *desktop;
	SPSelection *selection;
	const GSList *slist, *l;
	struct _SPBBoxSort *bbs;
	int len, pos;
	unsigned int changed;

	desktop = SP_ACTIVE_DESKTOP;
	if (!desktop) return;

	selection = SP_DT_SELECTION (desktop);
	slist = sp_selection_item_list (selection);
	if (!slist) return;
	if (!slist->next) return;

	len = g_slist_length ((GSList *) slist);
	bbs = g_new (struct _SPBBoxSort, len);
	pos = 0;
	for (l = slist; l != NULL; l = l->next) {
		bbs[pos].item = SP_ITEM (l->data);
		sp_item_bbox_desktop (bbs[pos].item, &bbs[pos].bbox);
		bbs[pos].anchor = 0.5 * layout[0] * bbs[pos].bbox.y0 + 0.5 * layout[1] * bbs[pos].bbox.y1;
		pos += 1;
	}

	qsort (bbs, len, sizeof (struct _SPBBoxSort), sp_align_bbox_sort);

	changed = FALSE;

	if (!layout[2]) {
		float dist, step;
		int i;
		dist = bbs[len - 1].anchor - bbs[0].anchor;
		step = dist / (len - 1);
		for (i = 0; i < len; i++) {
			float pos;
			pos = bbs[0].anchor + i * step;
			if (!NR_DF_TEST_CLOSE (pos, bbs[i].anchor, 1e-6)) {
				sp_item_move_rel (bbs[i].item, 0.0, pos - bbs[i].anchor);
				changed = TRUE;
			}
		}
	} else {
		/* Damn I am not sure, how to order them initially (Lauris) */
		float dist, span, step, pos;
		int i;
		dist = bbs[len - 1].bbox.y1 - bbs[0].bbox.y0;
		span = 0;
		for (i = 0; i < len; i++) span += (bbs[i].bbox.y1 - bbs[i].bbox.y0);
		step = (dist - span) / (len - 1);
		pos = bbs[0].bbox.y0;
		for (i = 0; i < len; i++) {
			if (!NR_DF_TEST_CLOSE (pos, bbs[i].bbox.y0, 1e-6)) {
				sp_item_move_rel (bbs[i].item, 0.0, pos - bbs[i].bbox.y0);
				changed = TRUE;
			}
			pos += (bbs[i].bbox.y1 - bbs[i].bbox.y0);
			pos += step;
		}
	}

	g_free (bbs);

	if (changed) {
		sp_selection_changed (selection);
		sp_document_done (SP_DT_DOCUMENT (desktop));
	}
}





