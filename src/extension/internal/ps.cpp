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
#include <stdlib.h>
#include <signal.h>
#include <errno.h>
#include <math.h>

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
#include "display/canvas-bpath.h"
#include "enums.h"
#include "document.h"
#include "inkscape.h"
#include "style.h"

#include "ps.h"
#include <extension/system.h>


namespace Inkscape {
namespace Extension {
namespace Internal {

PrintPS::PrintPS (void)
{
	_dpi = 72;

	return;
}

PrintPS::~PrintPS (void)
{
	/* fixme: should really use pclose for popen'd streams */
	if (_stream) fclose (_stream);

	/* restore default signal handling for SIGPIPE */
#if !defined(_WIN32) && !defined(__WIN32__)
	(void) signal(SIGPIPE, SIG_DFL);
#endif

	return;
}

unsigned int
PrintPS::setup (Inkscape::Extension::Print * mod)
{
	static const gchar *pdr[] = {"72", "75", "100", "144", "150", "200", "300", "360", "600", "1200", "2400", NULL};
	GtkWidget *dlg, *vbox, *f, *vb, *rb, *hb, *combo, *l, *e;
	GtkTooltips *tt;
	GList *sl;
	int i;
	int response;
	unsigned int ret;
#ifdef TED
	SPRepr *repr;
#endif
	bool p2bm;

#ifdef TED
	repr = ((SPModule *) mod)->repr;
#endif

	ret = FALSE;

	/* Create dialog */
	tt = gtk_tooltips_new ();
	g_object_ref ((GObject *) tt);
	gtk_object_sink ((GtkObject *) tt);
  
     dlg = gtk_dialog_new_with_buttons (_("Print Destination"), 
						 (GtkWindow *) g_object_get_data (G_OBJECT (SP_ACTIVE_DESKTOP), "window"),
                                       (GtkDialogFlags) (GTK_DIALOG_MODAL | GTK_DIALOG_NO_SEPARATOR | GTK_DIALOG_DESTROY_WITH_PARENT),
                                       GTK_STOCK_CANCEL,
                                       GTK_RESPONSE_CANCEL,
                                       GTK_STOCK_PRINT,
                                       GTK_RESPONSE_OK,
                                       NULL);

	gtk_dialog_set_default_response (GTK_DIALOG (dlg), GTK_RESPONSE_OK);

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
	mod->get_param("bitmap", &p2bm);
	rb = gtk_radio_button_new_with_label (NULL, _("Print using PostScript operators"));
	gtk_tooltips_set_tip ((GtkTooltips *) tt, rb,
						  _("Use PostScript vector operators. The resulting image is usually smaller "
						    "in file size and can be arbitrarily scaled, but alpha transparency, "
							"gradients and patterns will be lost."), NULL);
	if (!p2bm) gtk_toggle_button_set_active ((GtkToggleButton *) rb, TRUE);
	gtk_box_pack_start (GTK_BOX (vb), rb, FALSE, FALSE, 0);
	rb = gtk_radio_button_new_with_label (gtk_radio_button_get_group ((GtkRadioButton *) rb), _("Print as bitmap"));
	gtk_tooltips_set_tip ((GtkTooltips *) tt, rb,
						  _("Print everything as bitmap. The resulting image is usually larger "
						    "in file size and cannot be arbitrarily scaled without quality loss, "
							"but all objects will be rendered exactly as displayed."), NULL);
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
	if (1) {
		gchar * val = NULL;
		mod->get_param("resolution", &val);
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

	l = gtk_label_new (_("Use '> filename' to print to file.\n"
			     "Use '| prog arg...' to pipe to a program."));
	gtk_box_pack_start (GTK_BOX (vb), l, FALSE, FALSE, 0);

	e = gtk_entry_new ();
	if (1) {
		gchar *val;
		mod->get_param("destination", &val);
		gtk_entry_set_text (GTK_ENTRY (e), val);
	}
	gtk_box_pack_start (GTK_BOX (vb), e, FALSE, FALSE, 0);

	// pressing enter in the destination field is the same as clicking Print:
	gtk_entry_set_activates_default (GTK_ENTRY(e), TRUE);

	gtk_widget_show_all (vbox);
	
	response = gtk_dialog_run (GTK_DIALOG (dlg));

	g_object_unref ((GObject *) tt);

	if (response == GTK_RESPONSE_OK) {
		const gchar *fn;
		const char *sstr;

		_bitmap = gtk_toggle_button_get_active ((GtkToggleButton *) rb);
		sstr = gtk_entry_get_text (GTK_ENTRY (GTK_COMBO (combo)->entry));
		_dpi = (unsigned int) MAX ((int)(atof (sstr)), 1);
		/* Arrgh, have to do something */
		fn = gtk_entry_get_text (GTK_ENTRY (e));
		/* g_print ("Printing to %s\n", fn); */

		mod->set_param("bitmap", (_bitmap) ? (bool)TRUE : (bool)FALSE);
		mod->set_param("resolution", (gchar *)sstr);
		mod->set_param("destination", (gchar *)fn);
		ret = TRUE;
	}

	gtk_widget_destroy (dlg);

	return ret;
}

unsigned int
PrintPS::begin (Inkscape::Extension::Print *mod, SPDocument *doc)
{
	Inkscape::SVGOStringStream os;
	int res;
	FILE *osf, *osp;
	gchar * fn;

	mod->get_param("destination", (gchar **)&fn);

	osf = NULL;
	osp = NULL;

	gsize bytesRead = 0;
	gsize bytesWritten = 0;
	GError* error = NULL;
	gchar* local_fn = g_filename_from_utf8 ( fn,
                                 -1,  &bytesRead,  &bytesWritten, &error);
	fn = local_fn;

	if (fn != NULL) {
		if (*fn == '|') {
			fn += 1;
			while (isspace (*fn)) fn += 1;
#ifndef WIN32
			osp = popen (fn, "w");
#else
			osp = _popen (fn, "w");
#endif
			_stream = osp;
		} else if (*fn == '>') {
			fn += 1;
			while (isspace (*fn)) fn += 1;
			osf = fopen (fn, "w+");
			_stream = osf;
		} else {
			gchar *qn;
			/* put cwd stuff in here */
			qn = g_strdup_printf ("lpr -P %s", fn);
#ifndef WIN32
			osp = popen (qn, "w");
#else
			osp = _popen (qn, "w");
#endif
			g_free (qn);
			_stream = osp;
		}
	}

	g_free(local_fn);

	if (_stream) {
		/* fixme: this is kinda icky */
#if !defined(_WIN32) && !defined(__WIN32__)
		(void) signal(SIGPIPE, SIG_IGN);
#endif
	}

	res = fprintf (_stream, "%%!PS-Adobe-2.0\n");
	/* flush this to test output stream as early as possible */
	if (fflush(_stream)) {
/*		g_print("caught error in sp_module_print_plain_begin\n");*/
		if (ferror(_stream)) {
			g_print("Error %d on output stream: %s\n", errno,
				g_strerror(errno));
		}
		g_print("Printing failed\n");
		/* fixme: should use pclose() for pipes */
		fclose(_stream);
		_stream = NULL;
		fflush(stdout);
		return 0;
	}
	_width = sp_document_width (doc);
	_height = sp_document_height (doc);

	if (_bitmap) return 0;

	NRRect d;
	bool   pageBoundingBox;
	mod->get_param("pageBoundingBox", &pageBoundingBox);
	// printf("Page Bounding Box: %s\n", pageBoundingBox ? "TRUE" : "FALSE");
	if (pageBoundingBox)
	{
		d.x0 = d.y0 = 0;
		d.x1 = ceil (sp_document_width (doc));
		d.y1 = ceil (sp_document_height (doc));
	}
	else
	{
		SPItem* doc_item = SP_ITEM (sp_document_root (doc));
		sp_item_bbox_desktop (doc_item, &d);
	}

	if (res >= 0) {
		os << "%%BoundingBox: " << (int) d.x0 << " "
					<< (int) d.y0 << " "
					<< (int) ceil (d.x1) << " "
					<< (int) ceil (d.y1) << "\n";
	}
	if (res >= 0) {
		os << "%%HiResBoundingBox: " << d.x0 << " "
					<< d.y0 << " "
					<< d.x1 << " "
					<< d.y1 << "\n";
	}

	os << "0.0 " << sp_document_height (doc) << " translate\n";
	os << "0.8 -0.8 scale\n";

	return fprintf (_stream, "%s", os.str().c_str());
}

unsigned int
PrintPS::finish (Inkscape::Extension::Print *mod)
{
	int res;

	if (!_stream) return 0;

	if (_bitmap) {
		double x0, y0, x1, y1;
		int width, height;
		float scale;
		NRMatrix affine;
		guchar *px;
		int y;

		scale = _dpi / 72.0;

		y0 = 0.0;
		x0 = 0.0;
		x1 = _width;
		y1 = _height;

		width = (int) (_width * scale + 0.5);
		height = (int) (_height * scale + 0.5);

		affine.c[0] = width / ((x1 - x0) * 1.25);
		affine.c[1] = 0.0;
		affine.c[2] = 0.0;
		affine.c[3] = height / ((y1 - y0) * 1.25);
		affine.c[4] = -affine.c[0] * x0 * 1.25;
		affine.c[5] = -affine.c[3] * y0 * 1.25;

		nr_arena_item_set_transform (mod->root, &affine);

		px = nr_new (guchar, 4 * width * 64);

		for (y = 0; y < height; y += 64) {
			NRRectL bbox;
			NRGC gc(NULL);
			NRMatrix imgt;
			NRPixBlock pb;
			/* Set area of interest */
			bbox.x0 = 0;
			bbox.y0 = y;
			bbox.x1 = width;
			bbox.y1 = MIN (height, y + 64);
			/* Update to renderable state */
			nr_matrix_set_identity (&gc.transform);
			nr_arena_item_invoke_update (mod->root, &bbox, &gc, NR_ARENA_ITEM_STATE_ALL, NR_ARENA_ITEM_STATE_NONE);
			/* Render */
			/* This should take guchar* instead of unsigned char*) */
			nr_pixblock_setup_extern (&pb, NR_PIXBLOCK_MODE_R8G8B8A8N,
						  bbox.x0, bbox.y0, bbox.x1, bbox.y1,
						  (guchar*)px, 4 * width, FALSE, FALSE);
			memset (px, 0xff, 4 * width * 64);
			nr_arena_item_invoke_render (mod->root, &bbox, &pb, 0);
			/* Blitter goes here */
			imgt.c[0] = (bbox.x1 - bbox.x0) / scale;
			imgt.c[1] = 0.0;
			imgt.c[2] = 0.0;
			imgt.c[3] = (bbox.y1 - bbox.y0) / scale;
			imgt.c[4] = 0.0;
			imgt.c[5] = _height - y / scale - (bbox.y1 - bbox.y0) / scale;

			print_image (_stream, px, bbox.x1 - bbox.x0, bbox.y1 - bbox.y0, 4 * width, &imgt);
		}

		nr_free (px);
	}

	res = fprintf (_stream, "showpage\n");

	/* Flush stream to be sure */
	(void) fflush (_stream);

	/* fixme: should really use pclose for popen'd streams */
	fclose (_stream);
	_stream = 0;

	return res;
}

unsigned int
PrintPS::bind (Inkscape::Extension::Print *mod, const NRMatrix *transform, float opacity)
{
	if (!_stream) return 0;  // XXX: fixme, returning -1 as unsigned.
	if (_bitmap) return 0;

        Inkscape::SVGOStringStream os;
	os << "gsave [" << transform->c[0] << " "
			<< transform->c[1] << " "
			<< transform->c[2] << " "
			<< transform->c[3] << " "
			<< transform->c[4] << " "
			<< transform->c[5] << "] concat\n";

	return fprintf (_stream, "%s", os.str().c_str());
}

unsigned int
PrintPS::release (Inkscape::Extension::Print *mod)
{
	if (!_stream) return 0; // XXX: fixme, returning -1 as unsigned.
	if (_bitmap) return 0;

	return fprintf (_stream, "grestore\n");
}

unsigned int
PrintPS::fill (Inkscape::Extension::Print *mod, const NRBPath *bpath, const NRMatrix *ctm, const SPStyle *style,
			    const NRRect *pbox, const NRRect *dbox, const NRRect *bbox)
{
	if (!_stream) return 0; // XXX: fixme, returning -1 as unsigned.
	if (_bitmap) return 0;

	if (style->fill.type == SP_PAINT_TYPE_COLOR) {
		float rgb[3];
        	Inkscape::SVGOStringStream os;

		sp_color_get_rgb_floatv (&style->fill.value.color, rgb);

		os << rgb[0] << " " << rgb[1] << " " << rgb[2] << " setrgbcolor\n";

		print_bpath (os, bpath->path);

		if (style->fill_rule.value == SP_WIND_RULE_EVENODD) {
			os << "eofill\n";
		} else {
			os << "fill\n";
		}
		fprintf (_stream, "%s", os.str().c_str());
	}

	return 0;
}

unsigned int
PrintPS::stroke (Inkscape::Extension::Print *mod, const NRBPath *bpath, const NRMatrix *ctm, const SPStyle *style,
			      const NRRect *pbox, const NRRect *dbox, const NRRect *bbox)
{
	if (!_stream) return 0; // XXX: fixme, returning -1 as unsigned.
	if (_bitmap) return 0;

	if (style->stroke.type == SP_PAINT_TYPE_COLOR) {
		float rgb[3];
        	Inkscape::SVGOStringStream os;

		sp_color_get_rgb_floatv (&style->stroke.value.color, rgb);

		os << rgb[0] << " " << rgb[1] << " " << rgb[2] << " setrgbcolor\n";

		print_bpath (os, bpath->path);

		if (style->stroke_dash.n_dash > 0) {
			int i;
			os << "[";
			for (i = 0; i < style->stroke_dash.n_dash; i++) {
				if ((i)) {
					os << " ";
				}
				os << style->stroke_dash.dash[i];
			}
			os << "] " << style->stroke_dash.offset << " setdash\n";
		} else {
			os << "[] 0 setdash\n";
		}

		os << style->stroke_width.computed << " setlinewidth\n";
		os << style->stroke_linejoin.computed << " setlinejoin\n";
		os << style->stroke_linecap.computed << " setlinecap\n";
		os << "stroke\n";

		fprintf (_stream, "%s", os.str().c_str());
	}

	return 0;
}

unsigned int
PrintPS::image (Inkscape::Extension::Print *mod, guchar *px, unsigned int w, unsigned int h, unsigned int rs,
			     const NRMatrix *transform, const SPStyle *style)
{
	if (!_stream) return 0; // XXX: fixme, returning -1 as unsigned.
	if (_bitmap) return 0;

	return print_image (_stream, px, w, h, rs, transform);
#if 0
	fprintf (_stream, "gsave\n");
	fprintf (_stream, "/rowdata %d string def\n", 3 * w);
	fprintf (_stream, "[%g %g %g %g %g %g] concat\n",
		 transform->c[0],
		 transform->c[1],
		 transform->c[2],
		 transform->c[3],
		 transform->c[4],
		 transform->c[5]);
	fprintf (_stream, "%d %d 8 [%d 0 0 -%d 0 %d]\n", w, h, w, h, h);
	fprintf (_stream, "{currentfile rowdata readhexstring pop}\n");
	fprintf (_stream, "false 3 colorimage\n");

	for (unsigned int r = 0; r < h; r++) {
		guchar *s;
		unsigned int c0, c1, c;
		s = px + r * rs;
		for (c0 = 0; c0 < w; c0 += 24) {
			c1 = MIN (w, c0 + 24);
			for (c = c0; c < c1; c++) {
				static const char xtab[] = {'0','1','2','3','4','5','6','7','8','9','a','b','c','d','e','f'};
				fputc (xtab[s[0] >> 4], _stream);
				fputc (xtab[s[0] & 0xf], _stream);
				fputc (xtab[s[1] >> 4], _stream);
				fputc (xtab[s[1] & 0xf], _stream);
				fputc (xtab[s[2] >> 4], _stream);
				fputc (xtab[s[2] & 0xf], _stream);
				s += 4;
			}
			fputs ("\n", _stream);
		}
	}

	fprintf (_stream, "grestore\n");

	return 0;
#endif
}

unsigned int
PrintPS::text (Inkscape::Extension::Print *mod, const char *text, NR::Point p,
	       const SPStyle* style)
{
  if (!_stream) return 0; // XXX: fixme, returning -1 as unsigned.
  if (_bitmap) return 0;

  NRMatrix m;
  nr_matrix_set_scale (&m, 1, -1);
  m.c[5] = 2 * p[NR::Y];
  bind(mod, &m, 0);
  
  Inkscape::SVGOStringStream os;

  os << "/Times-Roman findfont\n";
  os << style->font_size.computed << " scalefont\n";
  os << "setfont newpath\n";
  os << p[NR::X] << " " << p[NR::Y] << " moveto\n";
  os << "(" << text << ") show\n";

  fprintf (_stream, "%s", os.str().c_str());

  release(mod);

  return 0;
}
	


/* PostScript helpers */

void
PrintPS::print_bpath (SVGOStringStream &os, const NArtBpath *bp)
{
	unsigned int closed;

	os << "newpath\n";
	closed = FALSE;
	while (bp->code != NR_END) {
		switch (bp->code) {
		case NR_MOVETO:
			if (closed) {
				os << "closepath\n";
			}
			closed = TRUE;
			os << bp->x3 << " " << bp->y3 << " moveto\n";
			break;
		case NR_MOVETO_OPEN:
			if (closed) {
				os << "closepath\n";
			}
			closed = FALSE;
			os << bp->x3 << " " << bp->y3 << " moveto\n";
			break;
		case NR_LINETO:
			os << bp->x3 << " " << bp->y3 << " lineto\n";
			break;
		case NR_CURVETO:
			os << bp->x1 << " " << bp->y1 << " "
			   << bp->x2 << " " << bp->y2 << " "
			   << bp->x3 << " " << bp->y3 << " curveto\n";
			break;
		default:
			break;
		}
		bp += 1;
	}
	if (closed) {
		os << "closepath\n";
	}
}

/* The following code is licensed under GNU GPL.
** The packbits, ascii85 and imaging printing code
** is from the gimp's postscript.c.
*/

/**
* \param nin Number of bytes of source data.
* \param src Source data.
* \param nout Number of output bytes.
* \param dst Buffer for output.
*/
void
PrintPS::compress_packbits (int nin,
                            guchar *src,
                            int *nout,
                            guchar *dst)

{
  register guchar c;
  int nrepeat, nliteral;
  guchar *run_start;
  guchar *start_dst = dst;
  guchar *last_literal = NULL;

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
     *(dst++) = (guchar)((-nrepeat) & 0xff);
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
     *(dst++) = (guchar)(nliteral-1);
   }
   while (nliteral-- > 0) *(dst++) = *(run_start++);
 }
 *nout = dst - start_dst;
}

void
PrintPS::ascii85_init (void)
{
  ascii85_len = 0;
  ascii85_linewidth = 0;
}

void
PrintPS::ascii85_flush (SVGOStringStream &os)
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
	os << '\n';
        ascii85_linewidth = 0;
      }
      os << 'z';
      ascii85_linewidth++;
    }
  else
    {
      for (i=0; i < ascii85_len+1; i++)
      {
        if ((ascii85_linewidth >= max_linewidth) && (c[i] != '%'))
        {
	  os << '\n';
          ascii85_linewidth = 0;
        }
	os << c[i];
        ascii85_linewidth++;
      }
    }

  ascii85_len = 0;
  ascii85_buf = 0;
}

inline void
PrintPS::ascii85_out (guchar byte, SVGOStringStream &os)
{
  if (ascii85_len == 4)
    ascii85_flush (os);

  ascii85_buf <<= 8;
  ascii85_buf |= byte;
  ascii85_len++;
}

void
PrintPS::ascii85_nout (int n, guchar *uptr, SVGOStringStream &os)
{
 while (n-- > 0)
 {
   ascii85_out (*uptr, os);
   uptr++;
 }
}

void
PrintPS::ascii85_done (SVGOStringStream &os)
{
  if (ascii85_len)
    {
      /* zero any unfilled buffer portion, then flush */
      ascii85_buf <<= (8 * (4-ascii85_len));
      ascii85_flush (os);
    }

  os << "~>\n";
}

unsigned int
PrintPS::print_image (FILE *ofp, guchar *px, unsigned int width, unsigned int height, unsigned int rs,
		   const NRMatrix *transform)
{
	unsigned int i, j;
	/* gchar *data, *src; */
	guchar *packb = NULL, *plane = NULL;
        Inkscape::SVGOStringStream os;

	os << "gsave\n";
	os << "[" << transform->c[0] << " "
		<< transform->c[1] << " "
		<< transform->c[2] << " "
		<< transform->c[3] << " "
		<< transform->c[4] << " "
		<< transform->c[5] << "] concat\n";
	os << width << " " << height << " 8 ["
		<< width << " 0 0 -" << height << " 0 " << height << "]\n";


	/* Write read image procedure */
	os << "% Strings to hold RGB-samples per scanline\n";
	os << "/rstr " << width << " string def\n";
	os << "/gstr " << width << " string def\n";
	os << "/bstr " << width << " string def\n";
	os << "{currentfile /ASCII85Decode filter /RunLengthDecode filter rstr readstring pop}\n";
	os << "{currentfile /ASCII85Decode filter /RunLengthDecode filter gstr readstring pop}\n";
	os << "{currentfile /ASCII85Decode filter /RunLengthDecode filter bstr readstring pop}\n";
	os << "true 3\n";

	/* Allocate buffer for packbits data. Worst case: Less than 1% increase */
	packb = (guchar *)g_malloc ((width * 105)/100+2);
	plane = (guchar *)g_malloc (width);

	/* ps_begin_data (ofp); */
	os << "colorimage\n";

#define GET_RGB_TILE(begin) \
  {int scan_lines; \
    scan_lines = (i+tile_height-1 < height) ? tile_height : (height-i); \
    gimp_pixel_rgn_get_rect (&pixel_rgn, begin, 0, i, width, scan_lines); \
    src = begin; }

	for (i = 0; i < height; i++) {
		/* if ((i % tile_height) == 0) GET_RGB_TILE (data); */ /* Get more data */
		guchar *plane_ptr, *src_ptr;
		int rgb, nout;
		guchar *src;

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
			ascii85_nout (nout, packb, os);
			ascii85_out (128, os); /* Write EOD of RunLengthDecode filter */
			ascii85_done (os);
		}
	}
	/* ps_end_data (ofp); */

#if 0
	fprintf (ofp, "showpage\n");
	g_free (data);
#endif

	g_free (packb);
	g_free (plane);

#if 0
	if (ferror (ofp))
	{
		g_message (_("write error occurred"));
		return (FALSE);
	}
#endif

	os << "grestore\n";

	fprintf (ofp, "%s", os.str().c_str());

	return 0;
#undef GET_RGB_TILE
}

bool
PrintPS::textToPath (Inkscape::Extension::Print * ext)
{
	bool param;
	ext->get_param("textToPath", &param);
	return param;
}

void
PrintPS::init (void)
{
	Inkscape::Extension::Extension * ext;
	
	/* SVG in */
    ext = Inkscape::Extension::build_from_mem(
		"<spmodule>\n"
			"<name>Postscript Print</name>\n"
			"<id>" SP_MODULE_KEY_PRINT_PS "</id>\n"
			"<param name=\"bitmap\" type=\"boolean\">FALSE</param>\n"
			"<param name=\"resolution\" type=\"string\">72</param>\n"
			"<param name=\"destination\" type=\"string\">lp</param>\n"
			"<param name=\"pageBoundingBox\" type=\"boolean\">TRUE</param>\n"
			"<param name=\"textToPath\" type=\"boolean\">TRUE</param>\n"
			"<print/>\n"
		"</spmodule>", new PrintPS());

	return;
}


}; /* namespace Internal */
}; /* namespace Extension */
}; /* namespace Inkscape */

/* End of GNU GPL code */
