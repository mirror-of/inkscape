#define __SP_GNOME_C__

/*
 * Gnome stuff
 *
 * Author:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * This code is in public domain
 */

/* Gnome Print */

#include <config.h>

#include <string.h>

#include <libnr/nr-rect.h>
#include <libnr/nr-matrix.h>
#include <libnr/nr-pixblock.h>

#include <glib.h>
#include <gtk/gtkdialog.h>
#include <gtk/gtkbox.h>
#include <gtk/gtkstock.h>

#if 0
#include <libgnomeprint/gnome-print-master.h>
#include <libgnomeprintui/gnome-print-master-preview.h>
#endif
#include <libgnomeprintui/gnome-print-dialog.h>

#include "helper/sp-intl.h"
#include "enums.h"
#include "document.h"
#include "style.h"
#include "sp-paint-server.h"

#include "gnome.h"

static void sp_module_print_gnome_class_init (SPModulePrintClass *klass);
static void sp_module_print_gnome_init (SPModulePrintGnome *gpmod);
static void sp_module_print_gnome_finalize (GObject *object);

static unsigned int sp_module_print_gnome_setup (SPModulePrint *mod);
static unsigned int sp_module_print_gnome_set_preview (SPModulePrint *mod);

static unsigned int sp_module_print_gnome_begin (SPModulePrint *mod, SPDocument *doc);
static unsigned int sp_module_print_gnome_finish (SPModulePrint *mod);

static unsigned int sp_module_print_gnome_bind (SPModulePrint *mod, const NRMatrixF *transform, float opacity);
static unsigned int sp_module_print_gnome_release (SPModulePrint *mod);
static unsigned int sp_module_print_gnome_fill (SPModulePrint *mod, const NRBPath *bpath, const NRMatrixF *ctm, const SPStyle *style,
						const NRRectF *pbox, const NRRectF *dbox, const NRRectF *bbox);
static unsigned int sp_module_print_gnome_stroke (SPModulePrint *mod, const NRBPath *bpath, const NRMatrixF *ctm, const SPStyle *style,
						  const NRRectF *pbox, const NRRectF *dbox, const NRRectF *bbox);
static unsigned int sp_module_print_gnome_image (SPModulePrint *mod, unsigned char *px, unsigned int w, unsigned int h, unsigned int rs,
						 const NRMatrixF *transform, const SPStyle *style);

static SPModulePrintClass *print_gnome_parent_class;

GType
sp_module_print_gnome_get_type (void)
{
	static GType type = 0;
	if (!type) {
		GTypeInfo info = {
			sizeof (SPModulePrintGnomeClass),
			NULL, NULL,
			(GClassInitFunc) sp_module_print_gnome_class_init,
			NULL, NULL,
			sizeof (SPModulePrintGnome),
			16,
			(GInstanceInitFunc) sp_module_print_gnome_init,
		};
		type = g_type_register_static (SP_TYPE_MODULE_PRINT, "SPModulePrintGnome", &info, 0);
	}
	return type;
}

static void
sp_module_print_gnome_class_init (SPModulePrintClass *klass)
{
	GObjectClass *g_object_class;
	SPModulePrintClass *module_print_class;

	g_object_class = (GObjectClass *)klass;
	module_print_class = (SPModulePrintClass *) klass;

	print_gnome_parent_class = g_type_class_peek_parent (klass);

	g_object_class->finalize = sp_module_print_gnome_finalize;

	module_print_class->setup = sp_module_print_gnome_setup;
	module_print_class->set_preview = sp_module_print_gnome_set_preview;
	module_print_class->begin = sp_module_print_gnome_begin;
	module_print_class->finish = sp_module_print_gnome_finish;
	module_print_class->bind = sp_module_print_gnome_bind;
	module_print_class->release = sp_module_print_gnome_release;
	module_print_class->fill = sp_module_print_gnome_fill;
	module_print_class->stroke = sp_module_print_gnome_stroke;
	module_print_class->image = sp_module_print_gnome_image;
}

static void
sp_module_print_gnome_init (SPModulePrintGnome *fmod)
{
	/* Nothing here */
}

static void
sp_module_print_gnome_finalize (GObject *object)
{
	SPModulePrintGnome *gpmod;

	gpmod = (SPModulePrintGnome *) object;
	
	G_OBJECT_CLASS (print_gnome_parent_class)->finalize (object);
}

static unsigned int
sp_module_print_gnome_setup (SPModulePrint *mod)
{
	SPModulePrintGnome *gpmod;
        GnomePrintConfig *config;
	GtkWidget *dlg, *vbox, *sel;
	int btn;

	gpmod = (SPModulePrintGnome *) mod;

	config = gnome_print_config_default ();

	dlg = gtk_dialog_new_with_buttons (_("Select printer"), NULL,
					   GTK_DIALOG_MODAL,
					   GTK_STOCK_PRINT,
					   GTK_RESPONSE_OK,
					   GTK_STOCK_CANCEL,
					   GTK_RESPONSE_CANCEL,
					   NULL);

	vbox = GTK_DIALOG (dlg)->vbox;
	gtk_container_set_border_width (GTK_CONTAINER (vbox), 4);

        sel = gnome_printer_selector_new (config);
	gtk_widget_show (sel);
	gtk_box_pack_start (GTK_BOX (vbox), sel, TRUE, TRUE, 0);

	btn = gtk_dialog_run (GTK_DIALOG (dlg));
	gtk_widget_destroy (dlg);
        if (btn != GTK_RESPONSE_OK) return FALSE;

	gpmod->gpc = gnome_print_context_new (config);

	gnome_print_config_unref (config);

	return TRUE;
}

static unsigned int
sp_module_print_gnome_set_preview (SPModulePrint *mod)
{
#if 0
	SPModulePrintGnome *gpmod;
	SPPrintContext ctx;
        GnomePrintContext *gpc;
        GnomePrintMaster *gpm;
	GtkWidget *gpmp;
	gchar *title;

	gpmod = (SPModulePrintGnome *) mod;

	gpm = gnome_print_master_new();
	gpmod->gpc = gnome_print_master_get_context (gpm);

	g_return_if_fail (gpm != NULL);
	g_return_if_fail (gpc != NULL);

	/* Print document */
	gnome_print_beginpage (gpc, SP_DOCUMENT_NAME (doc));
	gnome_print_translate (gpc, 0.0, sp_document_height (doc));
	/* From desktop points to document pixels */
	gnome_print_scale (gpc, 0.8, -0.8);
	sp_item_invoke_print (SP_ITEM (sp_document_root (doc)), &ctx);
        gnome_print_showpage (gpc);
        gnome_print_context_close (gpc);

	title = g_strdup_printf (_("Sodipodi (doc name %s..): Print Preview"),"");
	gpmp = gnome_print_master_preview_new (gpm, title);

	gtk_widget_show (GTK_WIDGET(gpmp));

	gnome_print_master_close (gpm);

	g_free (title);
#endif
	return 0;
}

static unsigned int
sp_module_print_gnome_begin (SPModulePrint *mod, SPDocument *doc)
{
	SPModulePrintGnome *gpmod;

	gpmod = (SPModulePrintGnome *) mod;

	gnome_print_beginpage (gpmod->gpc, SP_DOCUMENT_NAME (doc));
	gnome_print_translate (gpmod->gpc, 0.0, sp_document_height (doc));
	/* From desktop points to document pixels */
	gnome_print_scale (gpmod->gpc, 0.8, -0.8);

	return 0;
}

static unsigned int
sp_module_print_gnome_finish (SPModulePrint *mod)
{
	SPModulePrintGnome *gpmod;

	gpmod = (SPModulePrintGnome *) mod;

	gnome_print_showpage (gpmod->gpc);
	gnome_print_context_close (gpmod->gpc);

	return 0;
}

static unsigned int
sp_module_print_gnome_bind (SPModulePrint *mod, const NRMatrixF *transform, float opacity)
{
	SPModulePrintGnome *gpmod;
	gdouble t[6];

	gpmod = (SPModulePrintGnome *) mod;

	gnome_print_gsave (gpmod->gpc);

	t[0] = transform->c[0];
	t[1] = transform->c[1];
	t[2] = transform->c[2];
	t[3] = transform->c[3];
	t[4] = transform->c[4];
	t[5] = transform->c[5];

	gnome_print_concat (gpmod->gpc, t);

	/* fixme: Opacity? (lauris) */

	return 0;
}

static unsigned int
sp_module_print_gnome_release (SPModulePrint *mod)
{
	SPModulePrintGnome *gpmod;

	gpmod = (SPModulePrintGnome *) mod;

	gnome_print_grestore (gpmod->gpc);

	return 0;
}

static unsigned int
sp_module_print_gnome_fill (SPModulePrint *mod, const NRBPath *bpath, const NRMatrixF *ctm, const SPStyle *style,
			    const NRRectF *pbox, const NRRectF *dbox, const NRRectF *bbox)
{
	SPModulePrintGnome *gpmod;
	gdouble t[6];

	gpmod = (SPModulePrintGnome *) mod;

	/* CTM is for information purposes only */
	/* We expect user coordinate system to be set up already */

	t[0] = ctm->c[0];
	t[1] = ctm->c[1];
	t[2] = ctm->c[2];
	t[3] = ctm->c[3];
	t[4] = ctm->c[4];
	t[5] = ctm->c[5];

	if (style->fill.type == SP_PAINT_TYPE_COLOR) {
		float rgb[3], opacity;
		sp_color_get_rgb_floatv (&style->fill.value.color, rgb);
		gnome_print_setrgbcolor (gpmod->gpc, rgb[0], rgb[1], rgb[2]);

		/* fixme: */
		opacity = SP_SCALE24_TO_FLOAT (style->fill_opacity.value) * SP_SCALE24_TO_FLOAT (style->opacity.value);
		gnome_print_setopacity (gpmod->gpc, opacity);

		gnome_print_bpath (gpmod->gpc, bpath->path, FALSE);

		if (style->fill_rule.value == SP_WIND_RULE_EVENODD) {
			gnome_print_eofill (gpmod->gpc);
		} else {
			gnome_print_fill (gpmod->gpc);
		}
	} else if (style->fill.type == SP_PAINT_TYPE_PAINTSERVER) {
		SPPainter *painter;
		NRMatrixD dctm;
		NRRectD dpbox;

		/* fixme: */
		nr_matrix_d_from_f (&dctm, ctm);
		dpbox.x0 = pbox->x0;
		dpbox.y0 = pbox->y0;
		dpbox.x1 = pbox->x1;
		dpbox.y1 = pbox->y1;
		painter = sp_paint_server_painter_new (SP_STYLE_FILL_SERVER (style), NR_MATRIX_D_TO_DOUBLE (&dctm), &dpbox);
		if (painter) {
			NRRectF cbox;
			NRRectL ibox;
			NRMatrixF d2i;
			double dd2i[6];
			int x, y;

			nr_rect_f_intersect (&cbox, dbox, bbox);
			ibox.x0 = (long) cbox.x0;
			ibox.y0 = (long) cbox.y0;
			ibox.x1 = (long) (cbox.x1 + 0.9999);
			ibox.y1 = (long) (cbox.y1 + 0.9999);

			nr_matrix_f_invert (&d2i, ctm);

			gnome_print_gsave (gpmod->gpc);

			gnome_print_bpath (gpmod->gpc, bpath->path, FALSE);

			if (style->fill_rule.value == SP_WIND_RULE_EVENODD) {
				gnome_print_eoclip (gpmod->gpc);
			} else {
				gnome_print_clip (gpmod->gpc);
			}
			dd2i[0] = d2i.c[0];
			dd2i[1] = d2i.c[1];
			dd2i[2] = d2i.c[2];
			dd2i[3] = d2i.c[3];
			dd2i[4] = d2i.c[4];
			dd2i[5] = d2i.c[5];
			gnome_print_concat (gpmod->gpc, dd2i);
			/* Now we are in desktop coordinates */
			for (y = ibox.y0; y < ibox.y1; y+= 64) {
				for (x = ibox.x0; x < ibox.x1; x+= 64) {
					NRPixBlock pb;
					nr_pixblock_setup_fast (&pb, NR_PIXBLOCK_MODE_R8G8B8A8N, x, y, x + 64, y + 64, TRUE);
					painter->fill (painter, &pb);
					gnome_print_gsave (gpmod->gpc);
					gnome_print_translate (gpmod->gpc, x, y + 64);
					gnome_print_scale (gpmod->gpc, 64, -64);
					gnome_print_rgbaimage (gpmod->gpc, NR_PIXBLOCK_PX (&pb), 64, 64, pb.rs);
					gnome_print_grestore (gpmod->gpc);
					nr_pixblock_release (&pb);
				}
			}
			gnome_print_grestore (gpmod->gpc);
			sp_painter_free (painter);
		}
	}

	return 0;
}

static unsigned int
sp_module_print_gnome_stroke (SPModulePrint *mod, const NRBPath *bpath, const NRMatrixF *ctm, const SPStyle *style,
			      const NRRectF *pbox, const NRRectF *dbox, const NRRectF *bbox)
{
	SPModulePrintGnome *gpmod;
	gdouble t[6];

	gpmod = (SPModulePrintGnome *) mod;

	/* CTM is for information purposes only */
	/* We expect user coordinate system to be set up already */

	t[0] = ctm->c[0];
	t[1] = ctm->c[1];
	t[2] = ctm->c[2];
	t[3] = ctm->c[3];
	t[4] = ctm->c[4];
	t[5] = ctm->c[5];

	if (style->stroke.type == SP_PAINT_TYPE_COLOR) {
		float rgb[3], opacity;
		sp_color_get_rgb_floatv (&style->stroke.value.color, rgb);
		gnome_print_setrgbcolor (gpmod->gpc, rgb[0], rgb[1], rgb[2]);

		/* fixme: */
		opacity = SP_SCALE24_TO_FLOAT (style->stroke_opacity.value) * SP_SCALE24_TO_FLOAT (style->opacity.value);
		gnome_print_setopacity (gpmod->gpc, opacity);

		if (style->stroke_dash.n_dash > 0) {
			gnome_print_setdash (gpmod->gpc, style->stroke_dash.n_dash, style->stroke_dash.dash, style->stroke_dash.offset);
		} else {
			gnome_print_setdash (gpmod->gpc, 0, NULL, 0.0);
		}

		gnome_print_setlinewidth (gpmod->gpc, style->stroke_width.computed);
		gnome_print_setlinejoin (gpmod->gpc, style->stroke_linejoin.computed);
		gnome_print_setlinecap (gpmod->gpc, style->stroke_linecap.computed);

		gnome_print_bpath (gpmod->gpc, bpath->path, FALSE);

		gnome_print_stroke (gpmod->gpc);
	}

	return 0;
}

static unsigned int
sp_module_print_gnome_image (SPModulePrint *mod, unsigned char *px, unsigned int w, unsigned int h, unsigned int rs,
			     const NRMatrixF *transform, const SPStyle *style)
{
	SPModulePrintGnome *gpmod;
	gdouble t[6];

	gpmod = (SPModulePrintGnome *) mod;

	t[0] = transform->c[0];
	t[1] = transform->c[1];
	t[2] = transform->c[2];
	t[3] = transform->c[3];
	t[4] = transform->c[4];
	t[5] = transform->c[5];

	gnome_print_gsave (gpmod->gpc);

	gnome_print_concat (gpmod->gpc, t);

	if (style->opacity.value != SP_SCALE24_MAX) {
		guchar *dpx, *d, *s;
		gint x, y;
		guint32 alpha;
		alpha = (guint32) floor (SP_SCALE24_TO_FLOAT (style->opacity.value) * 255.9999);
		dpx = g_new (guchar, w * h * 4);
		for (y = 0; y < h; y++) {
			s = px + y * rs;
			d = dpx + y * w * 4;
			memcpy (d, s, w * 4);
			for (x = 0; x < w; x++) {
				d[3] = (s[3] * alpha) / 255;
				s += 4;
				d += 4;
			}
		}
		gnome_print_rgbaimage (gpmod->gpc, dpx, w, h, w * 4);
		g_free (dpx);
	} else {
		gnome_print_rgbaimage (gpmod->gpc, px, w, h, rs);
	}

	gnome_print_grestore (gpmod->gpc);

	return 0;
}

