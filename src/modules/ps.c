#define __SP_PS_C__

/*
 * PostScript printing
 *
 * Author:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * Basic printing code, EXCEPT image and
 * ascii85 filter is in public domain
 *
 * Image printing and Ascii85 filter:
 *
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 * Copyright (C) 1997-98 Peter Kirchgessner
 * George White <aa056@chebucto.ns.ca>
 * Austin Donnelly <austin@gimp.org>
 *
 * Licensed under GNU GPL
 */

/* Plain Print */

#include <config.h>

#include <string.h>
#include <ctype.h>

#include <libnr/nr-macros.h>
#include <libnr/nr-matrix.h>

#include <glib.h>
#include <gtk/gtkstock.h>
#include <gtk/gtkdialog.h>
#include <gtk/gtkvbox.h>
#include <gtk/gtkhbox.h>
#include <gtk/gtkframe.h>
#include <gtk/gtkradiobutton.h>
#include <gtk/gtkcombo.h>
#include <gtk/gtklabel.h>
#include <gtk/gtkentry.h>
#include <gtk/gtktooltips.h>

#include "helper/sp-intl.h"
#include "display/nr-arena-item.h"
#include "enums.h"
#include "document.h"
#include "style.h"

#include "ps.h"

static void sp_module_print_plain_class_init (SPModulePrintPlainClass *klass);
static void sp_module_print_plain_init (SPModulePrintPlain *fmod);
static void sp_module_print_plain_finalize (GObject *object);

static unsigned int sp_module_print_plain_setup (SPModulePrint *mod);
static unsigned int sp_module_print_plain_begin (SPModulePrint *mod, SPDocument *doc);
static unsigned int sp_module_print_plain_finish (SPModulePrint *mod);
static unsigned int sp_module_print_plain_bind (SPModulePrint *mod, const NRMatrixF *transform, float opacity);
static unsigned int sp_module_print_plain_release (SPModulePrint *mod);
static unsigned int sp_module_print_plain_fill (SPModulePrint *mod, const NRBPath *bpath, const NRMatrixF *ctm, const SPStyle *style,
						const NRRectF *pbox, const NRRectF *dbox, const NRRectF *bbox);
static unsigned int sp_module_print_plain_stroke (SPModulePrint *mod, const NRBPath *bpath, const NRMatrixF *ctm, const SPStyle *style,
						  const NRRectF *pbox, const NRRectF *dbox, const NRRectF *bbox);
static unsigned int sp_module_print_plain_image (SPModulePrint *mod, unsigned char *px, unsigned int w, unsigned int h, unsigned int rs,
						 const NRMatrixF *transform, const SPStyle *style);

static void sp_print_bpath (FILE *stream, const ArtBpath *bp);
static unsigned int sp_ps_print_image (FILE *ofp, unsigned char *px, unsigned int width, unsigned int height, unsigned int rs,
				       const NRMatrixF *transform);

static SPModulePrintClass *print_plain_parent_class;

GType
sp_module_print_plain_get_type (void)
{
	static GType type = 0;
	if (!type) {
		GTypeInfo info = {
			sizeof (SPModulePrintPlainClass),
			NULL, NULL,
			(GClassInitFunc) sp_module_print_plain_class_init,
			NULL, NULL,
			sizeof (SPModulePrintPlain),
			16,
			(GInstanceInitFunc) sp_module_print_plain_init,
		};
		type = g_type_register_static (SP_TYPE_MODULE_PRINT, "SPModulePrintPlain", &info, 0);
	}
	return type;
}

static void
sp_module_print_plain_class_init (SPModulePrintPlainClass *klass)
{
	GObjectClass *g_object_class;
	SPModulePrintClass *module_print_class;

	g_object_class = (GObjectClass *)klass;
	module_print_class = (SPModulePrintClass *) klass;

	print_plain_parent_class = g_type_class_peek_parent (klass);

	g_object_class->finalize = sp_module_print_plain_finalize;

	module_print_class->setup = sp_module_print_plain_setup;
	module_print_class->begin = sp_module_print_plain_begin;
	module_print_class->finish = sp_module_print_plain_finish;
	module_print_class->bind = sp_module_print_plain_bind;
	module_print_class->release = sp_module_print_plain_release;
	module_print_class->fill = sp_module_print_plain_fill;
	module_print_class->stroke = sp_module_print_plain_stroke;
	module_print_class->image = sp_module_print_plain_image;
}

static void
sp_module_print_plain_init (SPModulePrintPlain *pmod)
{
	pmod->dpi = 72;
}

static void
sp_module_print_plain_finalize (GObject *object)
{
	SPModulePrintPlain *gpmod;

	gpmod = (SPModulePrintPlain *) object;
	
	if (gpmod->stream) fclose (gpmod->stream);

	G_OBJECT_CLASS (print_plain_parent_class)->finalize (object);
}

static unsigned int
sp_module_print_plain_setup (SPModulePrint *mod)
{
	static const guchar *pdr[] = {"72", "75", "100", "144", "150", "200", "300", "360", "600", "1200", "2400", NULL};
	SPModulePrintPlain *pmod;
	GtkWidget *dlg, *vbox, *f, *vb, *rb, *hb, *combo, *l, *e;
	GtkTooltips *tt;
	GList *sl;
	int i;
	int response;
	unsigned int ret;
	SPRepr *repr;
	unsigned int p2bm;

	pmod = (SPModulePrintPlain *) mod;
	repr = ((SPModule *) mod)->repr;

	ret = FALSE;

	/* Create dialog */
	tt = gtk_tooltips_new ();
	g_object_ref ((GObject *) tt);
	gtk_object_sink ((GtkObject *) tt);
	dlg = gtk_dialog_new_with_buttons (_("Print destination"), NULL,
					   GTK_DIALOG_MODAL,
					   GTK_STOCK_PRINT,
					   GTK_RESPONSE_OK,
					   GTK_STOCK_CANCEL,
					   GTK_RESPONSE_CANCEL,
					   NULL);

	vbox = GTK_DIALOG (dlg)->vbox;
	gtk_container_set_border_width (GTK_CONTAINER (vbox), 4);
	/* Print properties frame */
	f = gtk_frame_new (_("Print properties"));
	gtk_box_pack_start (GTK_BOX (vbox), f, FALSE, FALSE, 4);
	vb = gtk_vbox_new (FALSE, 4);
	gtk_container_add (GTK_CONTAINER (f), vb);
	gtk_container_set_border_width (GTK_CONTAINER (vb), 4);
	/* Print type */
	p2bm = FALSE;
	if (repr) sp_repr_get_boolean (repr, "bitmap", &p2bm);
	rb = gtk_radio_button_new_with_label (NULL, _("Print using PostScript operators"));
	gtk_tooltips_set_tip ((GtkTooltips *) tt, rb,
						  _("Use PostScript vector operators, resulting image will be (usually) smaller "
						    "and can be arbitrarily scaled, but alpha transparency, "
							"markers and patterns will be lost"), NULL);
	if (!p2bm) gtk_toggle_button_set_active ((GtkToggleButton *) rb, TRUE);
	gtk_box_pack_start (GTK_BOX (vb), rb, FALSE, FALSE, 0);
	rb = gtk_radio_button_new_with_label (gtk_radio_button_get_group ((GtkRadioButton *) rb), _("Print as bitmap"));
	gtk_tooltips_set_tip ((GtkTooltips *) tt, rb,
						  _("Print everything as bitmap, resulting image will be (usualy) larger "
						    "and it quality depends on zoom factor, but all graphics "
							"will be rendered identical to display"), NULL);
	if (p2bm) gtk_toggle_button_set_active ((GtkToggleButton *) rb, TRUE);
	gtk_box_pack_start (GTK_BOX (vb), rb, FALSE, FALSE, 0);
	/* Resolution */
	hb = gtk_hbox_new (FALSE, 4);
	gtk_box_pack_start (GTK_BOX (vb), hb, FALSE, FALSE, 0);
	combo = gtk_combo_new ();
	gtk_combo_set_value_in_list (GTK_COMBO (combo), FALSE, FALSE);
	gtk_combo_set_use_arrows (GTK_COMBO (combo), TRUE);
	gtk_combo_set_use_arrows_always (GTK_COMBO (combo), TRUE);
	gtk_widget_set_usize (combo, 64, -1);
	gtk_tooltips_set_tip ((GtkTooltips *) tt, GTK_COMBO (combo)->entry,
						  _("Preferred resolution (dots per inch) of bitmap"), NULL);
	/* Setup strings */
	sl = NULL;
	for (i = 0; pdr[i] != NULL; i++) {
		sl = g_list_prepend (sl, (gpointer) pdr[i]);
	}
	sl = g_list_reverse (sl);
	gtk_combo_set_popdown_strings (GTK_COMBO (combo), sl);
	g_list_free (sl);
	if (repr) {
		const unsigned char *val;
		val = sp_repr_attr (repr, "resolution");
		gtk_entry_set_text (GTK_ENTRY (GTK_COMBO (combo)->entry), val);
	}
	gtk_box_pack_end (GTK_BOX (hb), combo, FALSE, FALSE, 0);
	l = gtk_label_new (_("Resolution:"));
	gtk_box_pack_end (GTK_BOX (hb), l, FALSE, FALSE, 0);

	/* Print destination frame */
	f = gtk_frame_new (_("Print destination"));
	gtk_box_pack_start (GTK_BOX (vbox), f, FALSE, FALSE, 4);
	vb = gtk_vbox_new (FALSE, 4);
	gtk_container_add (GTK_CONTAINER (f), vb);
	gtk_container_set_border_width (GTK_CONTAINER (vb), 4);

	l = gtk_label_new (_("Enter destination lpr queue.\n"
			     "Use '> filename' to print to file.\n"
			     "Use '| prog arg...' to pipe to program"));
	gtk_box_pack_start (GTK_BOX (vb), l, FALSE, FALSE, 0);

	e = gtk_entry_new ();
	if (repr && sp_repr_attr (repr, "destination")) {
		const unsigned char *val;
		val = sp_repr_attr (repr, "destination");
		gtk_entry_set_text (GTK_ENTRY (e), val);
	} else {
		gtk_entry_set_text (GTK_ENTRY (e), "lp");
	}
	gtk_box_pack_start (GTK_BOX (vb), e, FALSE, FALSE, 0);

	gtk_widget_show_all (vbox);
	
	response = gtk_dialog_run (GTK_DIALOG (dlg));

	g_object_unref ((GObject *) tt);

	if (response == GTK_RESPONSE_OK) {
		const unsigned char *fn;
		const char *sstr;
		FILE *osf, *osp;
		pmod->bitmap = gtk_toggle_button_get_active ((GtkToggleButton *) rb);
		sstr = gtk_entry_get_text (GTK_ENTRY (GTK_COMBO (combo)->entry));
		pmod->dpi = MAX (atof (sstr), 1);
		/* Arrgh, have to do something */
		fn = gtk_entry_get_text (GTK_ENTRY (e));
		/* g_print ("Printing to %s\n", fn); */
		if (repr) {
			sp_repr_set_attr (repr, "bitmap", (pmod->bitmap) ? "true" : "false");
			sp_repr_set_attr (repr, "resolution", sstr);
			sp_repr_set_attr (repr, "destination", fn);
		}
		osf = NULL;
		osp = NULL;
		if (fn) {
			if (*fn == '|') {
				fn += 1;
				while (isspace (*fn)) fn += 1;
#ifndef WIN32
				osp = popen (fn, "w");
#else
				osp = _popen (fn, "w");
#endif
				pmod->stream = osp;
			} else if (*fn == '>') {
				fn += 1;
				while (isspace (*fn)) fn += 1;
				osf = fopen (fn, "w+");
				pmod->stream = osf;
			} else {
				unsigned char *qn;
				qn = g_strdup_printf ("lpr %s", fn);
#ifndef WIN32
				osp = popen (qn, "w");
#else
				osp = _popen (qn, "w");
#endif
				g_free (qn);
				pmod->stream = osp;
			}
		}
		if (pmod->stream) ret = TRUE;
	}

	gtk_widget_destroy (dlg);

	return ret;
}

static unsigned int
sp_module_print_plain_begin (SPModulePrint *mod, SPDocument *doc)
{
	SPModulePrintPlain *pmod;
	int res;

	pmod = (SPModulePrintPlain *) mod;

	res = fprintf (pmod->stream, "%%!\n");
	pmod->width = sp_document_width (doc);
	pmod->height = sp_document_height (doc);

	if (pmod->bitmap) return 0;

	if (res >= 0) res = fprintf (pmod->stream, "%g %g translate\n", 0.0, sp_document_height (doc));
	if (res >= 0) res = fprintf (pmod->stream, "0.8 -0.8 scale\n");

	return res;
}

static unsigned int
sp_module_print_plain_finish (SPModulePrint *mod)
{
	int res;

	SPModulePrintPlain *pmod;

	pmod = (SPModulePrintPlain *) mod;

	if (pmod->bitmap) {
		double x0, y0, x1, y1;
		int width, height;
		float scale;
		NRMatrixF affine;
		unsigned char *px;
		int y;

		scale = pmod->dpi / 72.0;

		y0 = 0.0;
		x0 = 0.0;
		x1 = pmod->width;
		y1 = pmod->height;

		width = (int) (pmod->width * scale + 0.5);
		height = (int) (pmod->height * scale + 0.5);

		affine.c[0] = width / ((x1 - x0) * 1.25);
		affine.c[1] = 0.0;
		affine.c[2] = 0.0;
		affine.c[3] = height / ((y1 - y0) * 1.25);
		affine.c[4] = -affine.c[0] * x0 * 1.25;
		affine.c[5] = -affine.c[3] * y0 * 1.25;

		nr_arena_item_set_transform (mod->root, &affine);

		px = nr_new (unsigned char, 4 * width * 64);

		for (y = 0; y < height; y += 64) {
			NRRectL bbox;
			NRGC gc;
			NRMatrixF imgt;
			NRPixBlock pb;
			/* Set area of interest */
			bbox.x0 = 0;
			bbox.y0 = y;
			bbox.x1 = width;
			bbox.y1 = MIN (height, y + 64);
			/* Update to renderable state */
			nr_matrix_d_set_identity (&gc.transform);
			nr_arena_item_invoke_update (mod->root, &bbox, &gc, NR_ARENA_ITEM_STATE_ALL, NR_ARENA_ITEM_STATE_NONE);
			/* Render */
			nr_pixblock_setup_extern (&pb, NR_PIXBLOCK_MODE_R8G8B8A8N,
						  bbox.x0, bbox.y0, bbox.x1, bbox.y1,
						  px, 4 * width, FALSE, FALSE);
			memset (px, 0xff, 4 * width * 64);
			nr_arena_item_invoke_render (mod->root, &bbox, &pb, 0);
			/* Blitter goes here */
			imgt.c[0] = (bbox.x1 - bbox.x0) / scale;
			imgt.c[1] = 0.0;
			imgt.c[2] = 0.0;
			imgt.c[3] = (bbox.y1 - bbox.y0) / scale;
			imgt.c[4] = 0.0;
			imgt.c[5] = pmod->height - y / scale - (bbox.y1 - bbox.y0) / scale;

			sp_ps_print_image (pmod->stream, px, bbox.x1 - bbox.x0, bbox.y1 - bbox.y0, 4 * width, &imgt);
		}

		nr_free (px);
	}

	res = fprintf (pmod->stream, "showpage\n");

	return res;
}

static unsigned int
sp_module_print_plain_bind (SPModulePrint *mod, const NRMatrixF *transform, float opacity)
{
	SPModulePrintPlain *pmod;

	pmod = (SPModulePrintPlain *) mod;

	if (!pmod->stream) return -1;
	if (pmod->bitmap) return 0;

	return fprintf (pmod->stream, "gsave [%g %g %g %g %g %g] concat\n",
			transform->c[0], transform->c[1],
			transform->c[2], transform->c[3],
			transform->c[4], transform->c[5]);
}

static unsigned int
sp_module_print_plain_release (SPModulePrint *mod)
{
	SPModulePrintPlain *pmod;

	pmod = (SPModulePrintPlain *) mod;

	if (!pmod->stream) return -1;
	if (pmod->bitmap) return 0;

	return fprintf (pmod->stream, "grestore\n");
}

static unsigned int
sp_module_print_plain_fill (SPModulePrint *mod, const NRBPath *bpath, const NRMatrixF *ctm, const SPStyle *style,
			    const NRRectF *pbox, const NRRectF *dbox, const NRRectF *bbox)
{
	SPModulePrintPlain *pmod;

	pmod = (SPModulePrintPlain *) mod;

	if (!pmod->stream) return -1;
	if (pmod->bitmap) return 0;

	if (style->fill.type == SP_PAINT_TYPE_COLOR) {
		float rgb[3];

		sp_color_get_rgb_floatv (&style->fill.value.color, rgb);

		fprintf (pmod->stream, "%g %g %g setrgbcolor\n", rgb[0], rgb[1], rgb[2]);

		sp_print_bpath (pmod->stream, bpath->path);

		if (style->fill_rule.value == SP_WIND_RULE_EVENODD) {
			fprintf (pmod->stream, "eofill\n");
		} else {
			fprintf (pmod->stream, "fill\n");
		}
	}

	return 0;
}

static unsigned int
sp_module_print_plain_stroke (SPModulePrint *mod, const NRBPath *bpath, const NRMatrixF *ctm, const SPStyle *style,
			      const NRRectF *pbox, const NRRectF *dbox, const NRRectF *bbox)
{
	SPModulePrintPlain *pmod;

	pmod = (SPModulePrintPlain *) mod;

	if (!pmod->stream) return -1;
	if (pmod->bitmap) return 0;

	if (style->stroke.type == SP_PAINT_TYPE_COLOR) {
		float rgb[3];

		sp_color_get_rgb_floatv (&style->stroke.value.color, rgb);

		fprintf (pmod->stream, "%g %g %g setrgbcolor\n", rgb[0], rgb[1], rgb[2]);

		sp_print_bpath (pmod->stream, bpath->path);

		if (style->stroke_dash.n_dash > 0) {
			int i;
			fprintf (pmod->stream, "[");
			for (i = 0; i < style->stroke_dash.n_dash; i++) {
				fprintf (pmod->stream, (i) ? " %g" : "%g", style->stroke_dash.dash[i]);
			}
			fprintf (pmod->stream, "] %g setdash\n", style->stroke_dash.offset);
		} else {
			fprintf (pmod->stream, "[] 0 setdash\n");
		}

		fprintf (pmod->stream, "%g setlinewidth\n", style->stroke_width.computed);
		fprintf (pmod->stream, "%d setlinejoin\n", style->stroke_linejoin.computed);
		fprintf (pmod->stream, "%d setlinecap\n", style->stroke_linecap.computed);

		fprintf (pmod->stream, "stroke\n");
	}

	return 0;
}

static unsigned int
sp_module_print_plain_image (SPModulePrint *mod, unsigned char *px, unsigned int w, unsigned int h, unsigned int rs,
			     const NRMatrixF *transform, const SPStyle *style)
{
	SPModulePrintPlain *pmod;

	pmod = (SPModulePrintPlain *) mod;

	if (!pmod->stream) return -1;
	if (pmod->bitmap) return 0;

	return sp_ps_print_image (pmod->stream, px, w, h, rs, transform);
#if 0
	fprintf (pmod->stream, "gsave\n");
	fprintf (pmod->stream, "/rowdata %d string def\n", 3 * w);
	fprintf (pmod->stream, "[%g %g %g %g %g %g] concat\n",
		 transform->c[0],
		 transform->c[1],
		 transform->c[2],
		 transform->c[3],
		 transform->c[4],
		 transform->c[5]);
	fprintf (pmod->stream, "%d %d 8 [%d 0 0 -%d 0 %d]\n", w, h, w, h, h);
	fprintf (pmod->stream, "{currentfile rowdata readhexstring pop}\n");
	fprintf (pmod->stream, "false 3 colorimage\n");

	for (r = 0; r < h; r++) {
		unsigned char *s;
		int c0, c1, c;
		s = px + r * rs;
		for (c0 = 0; c0 < w; c0 += 24) {
			c1 = MIN (w, c0 + 24);
			for (c = c0; c < c1; c++) {
				static const char xtab[] = {'0','1','2','3','4','5','6','7','8','9','a','b','c','d','e','f'};
				fputc (xtab[s[0] >> 4], pmod->stream);
				fputc (xtab[s[0] & 0xf], pmod->stream);
				fputc (xtab[s[1] >> 4], pmod->stream);
				fputc (xtab[s[1] & 0xf], pmod->stream);
				fputc (xtab[s[2] >> 4], pmod->stream);
				fputc (xtab[s[2] & 0xf], pmod->stream);
				s += 4;
			}
			fputs ("\n", pmod->stream);
		}
	}

	fprintf (pmod->stream, "grestore\n");

	return 0;
#endif
}

/* PostScript helpers */

static void
sp_print_bpath (FILE *stream, const ArtBpath *bp)
{
	unsigned int closed;

	fprintf (stream, "newpath\n");
	closed = FALSE;
	while (bp->code != ART_END) {
		switch (bp->code) {
		case ART_MOVETO:
			if (closed) {
				fprintf (stream, "closepath\n");
			}
			closed = TRUE;
			fprintf (stream, "%g %g moveto\n", bp->x3, bp->y3);
			break;
		case ART_MOVETO_OPEN:
			if (closed) {
				fprintf (stream, "closepath\n");
			}
			closed = FALSE;
			fprintf (stream, "%g %g moveto\n", bp->x3, bp->y3);
			break;
		case ART_LINETO:
			fprintf (stream, "%g %g lineto\n", bp->x3, bp->y3);
			break;
		case ART_CURVETO:
			fprintf (stream, "%g %g %g %g %g %g curveto\n", bp->x1, bp->y1, bp->x2, bp->y2, bp->x3, bp->y3);
			break;
		default:
			break;
		}
		bp += 1;
	}
	if (closed) {
		fprintf (stream, "closepath\n");
	}
}

/* The following code is licensed under GNU GPL */

static void
compress_packbits (int nin,
                   unsigned char *src,
                   int *nout,
                   unsigned char *dst)

{register unsigned char c;
 int nrepeat, nliteral;
 unsigned char *run_start;
 unsigned char *start_dst = dst;
 unsigned char *last_literal = NULL;

 for (;;)
 {
   if (nin <= 0) break;

   run_start = src;
   c = *run_start;

   /* Search repeat bytes */
   if ((nin > 1) && (c == src[1]))
   {
     nrepeat = 1;
     nin -= 2;
     src += 2;
     while ((nin > 0) && (c == *src))
     {
       nrepeat++;
       src++;
       nin--;
       if (nrepeat == 127) break; /* Maximum repeat */
     }

     /* Add two-byte repeat to last literal run ? */
     if (   (nrepeat == 1)
         && (last_literal != NULL) && (((*last_literal)+1)+2 <= 128))
     {
       *last_literal += 2;
       *(dst++) = c;
       *(dst++) = c;
       continue;
     }

     /* Add repeat run */
     *(dst++) = (unsigned char)((-nrepeat) & 0xff);
     *(dst++) = c;
     last_literal = NULL;
     continue;
   }
   /* Search literal bytes */
   nliteral = 1;
   nin--;
   src++;

   for (;;)
   {
     if (nin <= 0) break;

     if ((nin >= 2) && (src[0] == src[1])) /* A two byte repeat ? */
       break;

     nliteral++;
     nin--;
     src++;
     if (nliteral == 128) break; /* Maximum literal run */
   }

   /* Could be added to last literal run ? */
   if ((last_literal != NULL) && (((*last_literal)+1)+nliteral <= 128))
   {
     *last_literal += nliteral;
   }
   else
   {
     last_literal = dst;
     *(dst++) = (unsigned char)(nliteral-1);
   }
   while (nliteral-- > 0) *(dst++) = *(run_start++);
 }
 *nout = dst - start_dst;
}

static guint32 ascii85_buf;
static int ascii85_len = 0;
static int ascii85_linewidth = 0;

static void
ascii85_init (void)
{
  ascii85_len = 0;
  ascii85_linewidth = 0;
}

static void
ascii85_flush (FILE *ofp)
{
  char c[5];
  int i;
  gboolean zero_case = (ascii85_buf == 0);
  static int max_linewidth = 75;

  for (i=4; i >= 0; i--)
    {
      c[i] = (ascii85_buf % 85) + '!';
      ascii85_buf /= 85;
    }
  /* check for special case: "!!!!!" becomes "z", but only if not
   * at end of data. */
  if (zero_case && (ascii85_len == 4))
    {
      if (ascii85_linewidth >= max_linewidth)
      {
        putc ('\n', ofp);
        ascii85_linewidth = 0;
      }
      putc ('z', ofp);
      ascii85_linewidth++;
    }
  else
    {
      for (i=0; i < ascii85_len+1; i++)
      {
        if ((ascii85_linewidth >= max_linewidth) && (c[i] != '%'))
        {
          putc ('\n', ofp);
          ascii85_linewidth = 0;
        }
	putc (c[i], ofp);
        ascii85_linewidth++;
      }
    }

  ascii85_len = 0;
  ascii85_buf = 0;
}

static inline void
ascii85_out (unsigned char byte, FILE *ofp)
{
  if (ascii85_len == 4)
    ascii85_flush (ofp);

  ascii85_buf <<= 8;
  ascii85_buf |= byte;
  ascii85_len++;
}

static void
ascii85_nout (int n, unsigned char *uptr, FILE *ofp)
{
 while (n-- > 0)
 {
   ascii85_out (*uptr, ofp);
   uptr++;
 }
}

static void
ascii85_done (FILE *ofp)
{
  if (ascii85_len)
    {
      /* zero any unfilled buffer portion, then flush */
      ascii85_buf <<= (8 * (4-ascii85_len));
      ascii85_flush (ofp);
    }

  putc ('~', ofp);
  putc ('>', ofp);
  putc ('\n', ofp);
}

static unsigned int
sp_ps_print_image (FILE *ofp, unsigned char *px, unsigned int width, unsigned int height, unsigned int rs,
		   const NRMatrixF *transform)
{
	int i, j;
	/* guchar *data, *src; */
	guchar *packb = NULL, *plane = NULL;

	fprintf (ofp, "gsave\n");
	fprintf (ofp, "[%g %g %g %g %g %g] concat\n",
		 transform->c[0],
		 transform->c[1],
		 transform->c[2],
		 transform->c[3],
		 transform->c[4],
		 transform->c[5]);
	fprintf (ofp, "%d %d 8 [%d 0 0 -%d 0 %d]\n", width, height, width, height, height);

	/* Write read image procedure */
	fprintf (ofp, "%% Strings to hold RGB-samples per scanline\n");
	fprintf (ofp, "/rstr %d string def\n", width);
	fprintf (ofp, "/gstr %d string def\n", width);
	fprintf (ofp, "/bstr %d string def\n", width);
	fprintf (ofp, "{currentfile /ASCII85Decode filter /RunLengthDecode filter rstr readstring pop}\n");
	fprintf (ofp, "{currentfile /ASCII85Decode filter /RunLengthDecode filter gstr readstring pop}\n");
	fprintf (ofp, "{currentfile /ASCII85Decode filter /RunLengthDecode filter bstr readstring pop}\n");
	fprintf (ofp, "true 3\n");

	/* Allocate buffer for packbits data. Worst case: Less than 1% increase */
	packb = (guchar *)g_malloc ((width * 105)/100+2);
	plane = (guchar *)g_malloc (width);

	/* ps_begin_data (ofp); */
	fprintf (ofp, "colorimage\n");

#define GET_RGB_TILE(begin) \
  {int scan_lines; \
    scan_lines = (i+tile_height-1 < height) ? tile_height : (height-i); \
    gimp_pixel_rgn_get_rect (&pixel_rgn, begin, 0, i, width, scan_lines); \
    src = begin; }

	for (i = 0; i < height; i++) {
		/* if ((i % tile_height) == 0) GET_RGB_TILE (data); */ /* Get more data */
		guchar *plane_ptr, *src_ptr;
		int rgb, nout;
		unsigned char *src;

		src = px + i * rs;

		/* Iterate over RGB */
		for (rgb = 0; rgb < 3; rgb++) {
			src_ptr = src + rgb;
			plane_ptr = plane;
			for (j = 0; j < width; j++) {
				*(plane_ptr++) = *src_ptr;
				src_ptr += 4;
			}
			compress_packbits (width, plane, &nout, packb);
			ascii85_init ();
			ascii85_nout (nout, packb, ofp);
			ascii85_out (128, ofp); /* Write EOD of RunLengthDecode filter */
			ascii85_done (ofp);
		}
	}
	/* ps_end_data (ofp); */

#if 0
	fprintf (ofp, "showpage\n");
	g_free (data);
#endif

	if (packb != NULL) g_free (packb);
	if (plane != NULL) g_free (plane);

#if 0
	if (ferror (ofp))
	{
		g_message (_("write error occured"));
		return (FALSE);
	}
#endif

	fprintf (ofp, "grestore\n");

	return 0;
#undef GET_RGB_TILE
}

/* End of GNU GPL code */



