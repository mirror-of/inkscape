#define __RUBBERBAND_C__

/*
 * Rubberbanding selector
 *
 * Author:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * Copyright (C) 1999-2002 Lauris Kaplinski
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */


#include "helper/sodipodi-ctrlrect.h"
#include "desktop.h"
#include "desktop-handles.h"
#include "rubberband.h"

SPDesktop * sp_rb_desktop = NULL;
NRRect sp_rb_dbox;
SPCtrlRect * sp_rb = NULL;
gboolean sp_rb_dragging = FALSE;
ArtDRect sp_rb_rect;

void sp_rubberband_start (SPDesktop * desktop, NR::Point const &p)
{
	using NR::X;
	using NR::Y;

	g_return_if_fail (desktop != NULL);
	g_return_if_fail (SP_IS_DESKTOP (desktop));

	if (sp_rb) sp_rubberband_stop ();

	sp_rb_desktop = desktop;
	sp_rb_rect.x0 = p[X];
	sp_rb_rect.y0 = p[Y];
	sp_rb_dragging = TRUE;

	sp_desktop_get_display_area (sp_rb_desktop, &sp_rb_dbox);
}

void sp_rubberband_move(NR::Point const &p)
{
	using NR::X; using NR::Y;
	sp_rubberband_move(p[X], p[Y]);
}

void
sp_rubberband_move (double x, double y)
{
	// no need to loudly compain if !sp_rb_dragging, because it could have been nullified by escape without mouse release; just return
	if (!sp_rb_dragging) return;
	g_return_if_fail (sp_rb_desktop != NULL);

	if (sp_rb == NULL) {
		sp_rb = (SPCtrlRect *) sp_canvas_item_new (SP_DT_CONTROLS (sp_rb_desktop), SP_TYPE_CTRLRECT, NULL);
	}

	if (!(x > sp_rb_dbox.x0 && x < sp_rb_dbox.x1) || !(y > sp_rb_dbox.y0 && y < sp_rb_dbox.y1)) {
		NR::Point const s_dt(x, y);
		NR::Point const s_w( s_dt * sp_rb_desktop->d2w );

		gdouble x_to;
		if (x < sp_rb_dbox.x0)
			x_to = sp_rb_dbox.x0;
		else if (x > sp_rb_dbox.x1)
			x_to = sp_rb_dbox.x1;
		else 
			x_to = x;

		gdouble y_to;
		if (y < sp_rb_dbox.y0)
			y_to = sp_rb_dbox.y0;
		else if (y > sp_rb_dbox.y1)
			y_to = sp_rb_dbox.y1;
		else 
			y_to = y;

		NR::Point const d_dt(x_to, y_to);
		NR::Point const d_w( d_dt * sp_rb_desktop->d2w );
		NR::Point const moved_w( d_w - s_w );
		gint const dx = (gint) moved_w[NR::X];
		gint const dy = (gint) moved_w[NR::Y];
		sp_desktop_scroll_world(sp_rb_desktop, dx, dy);

		sp_desktop_get_display_area (sp_rb_desktop, &sp_rb_dbox);
	}

	sp_rb_rect.x1 = x;
	sp_rb_rect.y1 = y;
	sp_ctrlrect_set_rect (sp_rb, &sp_rb_rect);
}

void
sp_rubberband_stop (void)
{
	if (sp_rb) gtk_object_destroy ((GtkObject *) sp_rb);

	sp_rb = NULL;
	sp_rb_desktop = NULL;
	sp_rb_dragging = FALSE;
}

gboolean
sp_rubberband_rect (NRRect *rect)
{
	if (sp_rb == NULL) return FALSE;
	rect->x0 = MIN (sp_rb_rect.x0, sp_rb_rect.x1);
	rect->y0 = MIN (sp_rb_rect.y0, sp_rb_rect.y1);
	rect->x1 = MAX (sp_rb_rect.x0, sp_rb_rect.x1);
	rect->y1 = MAX (sp_rb_rect.y0, sp_rb_rect.y1);
	return TRUE;
}

