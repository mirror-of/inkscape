#ifndef __SP_NAMEDVIEW_H__
#define __SP_NAMEDVIEW_H__

/*
 * <sodipodi:namedview> implementation
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * Copyright (C) Lauris Kaplinski 2000-2002
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#define SP_TYPE_NAMEDVIEW (sp_namedview_get_type ())
#define SP_NAMEDVIEW(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), SP_TYPE_NAMEDVIEW, SPNamedView))
#define SP_NAMEDVIEW_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), SP_TYPE_NAMEDVIEW, SPNamedViewClass))
#define SP_IS_NAMEDVIEW(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), SP_TYPE_NAMEDVIEW))
#define SP_IS_NAMEDVIEW_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), SP_TYPE_NAMEDVIEW))

#include "helper/units.h"
#include "sp-object-group.h"

enum {
	SP_BORDER_LAYER_BOTTOM,
	SP_BORDER_LAYER_TOP
};

struct _SPNamedView {
	SPObjectGroup objectgroup;
	unsigned int editable : 1;
	unsigned int showgrid : 1;
	unsigned int snaptogrid : 1;
	unsigned int showguides : 1;
	unsigned int snaptoguides : 1;
	unsigned int showborder : 1;
	unsigned int borderlayer : 2;

	const SPUnit *gridunit;
	/* Grid data is in points regardless of unit */
	gdouble gridoriginx;
	gdouble gridoriginy;
	gdouble gridspacingx;
	gdouble gridspacingy;

	const SPUnit *gridtoleranceunit;
	gdouble gridtolerance;

	const SPUnit *guidetoleranceunit;
	gdouble guidetolerance;

	guint32 gridcolor;
	guint32 guidecolor;
	guint32 guidehicolor;
	GSList * hguides;
	GSList * vguides;
	GSList * views;
	GSList * gridviews;
        gint viewcount;
};

struct _SPNamedViewClass {
	SPObjectGroupClass parent_class;
};

GType sp_namedview_get_type (void);

void sp_namedview_show (SPNamedView * namedview, gpointer desktop);
void sp_namedview_hide (SPNamedView * namedview, gpointer desktop);

void sp_namedview_activate_guides (SPNamedView * nv, gpointer desktop, gboolean active);
guint sp_namedview_viewcount (SPNamedView * nv);
const gchar * sp_namedview_get_name (SPNamedView * nv);
const GSList * sp_namedview_view_list (SPNamedView * nv);

SPNamedView *sp_document_namedview (SPDocument * document, const gchar * name);

#endif




