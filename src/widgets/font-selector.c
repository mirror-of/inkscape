#define __SP_FONT_SELECTOR_C__

/*
 * Font selection widgets
 *
 * Authors:
 *   Chris Lahey <clahey@ximian.com>
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * Copyright (C) 1999-2001 Ximian, Inc.
 * Copyright (C) 2002 Lauris Kaplinski
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "config.h"

#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#include <libnr/nr-matrix.h>
#include <libnr/nr-blit.h>

#include <glib.h>

#include <libnrtype/nr-type-directory.h>
#include <libnrtype/nr-rasterfont.h>

#include <gtk/gtksignal.h>
#include <gtk/gtkhbox.h>
#include <gtk/gtkframe.h>
#include <gtk/gtkscrolledwindow.h>
#include <gtk/gtkclist.h>
#include <gtk/gtkvbox.h>
#include <gtk/gtkcombo.h>
#include <gtk/gtkentry.h>
#include <gtk/gtkdrawingarea.h>

#include "../helper/nr-plain-stuff-gdk.h"
#include "../helper/sp-intl.h"

#include "font-selector.h"

/* SPFontSelector */

struct _SPFontSelector
{
	GtkHBox hbox;
  
	unsigned int block_emit : 1;

	GtkWidget *family;
	GtkWidget *style;
	GtkWidget *size;

	NRNameList families;
	NRNameList styles;
	int familyidx;
	int styleidx;
	gfloat fontsize;
	NRFont *font;
};


struct _SPFontSelectorClass
{
	GtkHBoxClass parent_class;

	void (* font_set) (SPFontSelector *fsel, NRFont *font);
};

enum {FONT_SET, LAST_SIGNAL};

static void sp_font_selector_class_init (SPFontSelectorClass *klass);
static void sp_font_selector_init (SPFontSelector *fsel);
static void sp_font_selector_destroy (GtkObject *object);

static void sp_font_selector_family_select_row (GtkCList *clist, gint row, gint column, GdkEvent *event, SPFontSelector *fsel);
static void sp_font_selector_style_select_row (GtkCList *clist, gint row, gint column, GdkEvent *event, SPFontSelector *fsel);
static void sp_font_selector_size_changed (GtkEditable *editable, SPFontSelector *fsel);

static void sp_font_selector_emit_set (SPFontSelector *fsel);

static const guchar *sizes[] = {
	"8", "9", "10", "11", "12", "13", "14",
	"16", "18", "20", "22", "24", "26", "28",
	"32", "36", "40", "48", "56", "64", "72",
	NULL
};

static GtkHBoxClass *fs_parent_class = NULL;
static guint fs_signals[LAST_SIGNAL] = {0};

GtkType
sp_font_selector_get_type ()
{
	static GtkType type = 0;
	if (!type) {
		static const GtkTypeInfo info = {
			"SPFontSelector",
			sizeof (SPFontSelector),
			sizeof (SPFontSelectorClass),
			(GtkClassInitFunc) sp_font_selector_class_init,
			(GtkObjectInitFunc) sp_font_selector_init,
			NULL, NULL, NULL
		};
		type = gtk_type_unique (GTK_TYPE_HBOX, &info);
	}
	return type;
}

static void
sp_font_selector_class_init (SPFontSelectorClass *klass)
{
	GtkObjectClass *object_class;
  
	object_class = (GtkObjectClass *) klass;
  
	fs_parent_class = gtk_type_class (GTK_TYPE_HBOX);

	fs_signals[FONT_SET] = gtk_signal_new ("font_set",
					       GTK_RUN_FIRST,
					       GTK_CLASS_TYPE(object_class),
					       GTK_SIGNAL_OFFSET (SPFontSelectorClass, font_set),
					       gtk_marshal_NONE__POINTER,
					       GTK_TYPE_NONE,
					       1, GTK_TYPE_POINTER);

	object_class->destroy = sp_font_selector_destroy;
}

static void
sp_font_selector_init (SPFontSelector *fsel)
{
	GtkWidget *f, *sw, *vb, *hb, *l;
	GList *sl;
	int i;

	gtk_box_set_homogeneous (GTK_BOX (fsel), TRUE);
	gtk_box_set_spacing (GTK_BOX (fsel), 4);

	/* Family frame */
	f = gtk_frame_new (_("Font family"));
	gtk_widget_show (f);
	gtk_box_pack_start (GTK_BOX (fsel), f, TRUE, TRUE, 0);

	sw = gtk_scrolled_window_new (NULL, NULL);
	gtk_widget_show (sw);
	gtk_container_set_border_width (GTK_CONTAINER (sw), 4);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (sw), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
	gtk_container_add (GTK_CONTAINER (f), sw);

	fsel->family = gtk_clist_new (1);
	gtk_widget_show (fsel->family);
	gtk_clist_set_selection_mode (GTK_CLIST (fsel->family), GTK_SELECTION_SINGLE);
	gtk_clist_column_titles_hide (GTK_CLIST (fsel->family));
	gtk_signal_connect (GTK_OBJECT (fsel->family), "select_row", GTK_SIGNAL_FUNC (sp_font_selector_family_select_row), fsel);
	gtk_container_add (GTK_CONTAINER (sw), fsel->family);

	if (nr_type_directory_family_list_get (&fsel->families)) {
		gint i;
		gtk_clist_freeze (GTK_CLIST (fsel->family));
		for (i = 0; i < fsel->families.length; i++) {
			gtk_clist_append (GTK_CLIST (fsel->family), (gchar **) fsel->families.names + i);
			gtk_clist_set_row_data (GTK_CLIST (fsel->family), i, GUINT_TO_POINTER (i));
		}
		gtk_clist_thaw (GTK_CLIST (fsel->family));
	}

	/* Style frame */
	f = gtk_frame_new (_("Style"));
	gtk_widget_show (f);
	gtk_box_pack_start (GTK_BOX (fsel), f, TRUE, TRUE, 0);

	vb = gtk_vbox_new (FALSE, 4);
	gtk_widget_show (vb);
	gtk_container_set_border_width (GTK_CONTAINER (vb), 4);
	gtk_container_add (GTK_CONTAINER (f), vb);

	sw = gtk_scrolled_window_new (NULL, NULL);
	gtk_widget_show (sw);
	gtk_container_set_border_width (GTK_CONTAINER (sw), 4);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (sw), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
	gtk_box_pack_start (GTK_BOX (vb), sw, TRUE, TRUE, 0);

	fsel->style = gtk_clist_new (1);
	gtk_widget_show (fsel->style);
	gtk_clist_set_selection_mode (GTK_CLIST (fsel->style), GTK_SELECTION_SINGLE);
	gtk_clist_column_titles_hide (GTK_CLIST (fsel->style));
	gtk_signal_connect (GTK_OBJECT (fsel->style), "select_row", GTK_SIGNAL_FUNC (sp_font_selector_style_select_row), fsel);
	gtk_container_add (GTK_CONTAINER (sw), fsel->style);

	hb = gtk_hbox_new (FALSE, 4);
	gtk_widget_show (hb);
	gtk_box_pack_start (GTK_BOX (vb), hb, FALSE, FALSE, 0);

	fsel->size = gtk_combo_new ();
	gtk_widget_show (fsel->size);
	gtk_combo_set_value_in_list (GTK_COMBO (fsel->size), FALSE, FALSE);
	gtk_combo_set_use_arrows (GTK_COMBO (fsel->size), TRUE);
	gtk_combo_set_use_arrows_always (GTK_COMBO (fsel->size), TRUE);
	gtk_widget_set_size_request (fsel->size, 64, -1);
	gtk_signal_connect (GTK_OBJECT (GTK_COMBO (fsel->size)->entry), "changed", GTK_SIGNAL_FUNC (sp_font_selector_size_changed), fsel);
	gtk_box_pack_end (GTK_BOX (hb), fsel->size, FALSE, FALSE, 0);

	l = gtk_label_new (_("Font size:"));
	gtk_widget_show (l);
	gtk_box_pack_end (GTK_BOX (hb), l, FALSE, FALSE, 0);

	/* Setup strings */
	sl = NULL;
	for (i = 0; sizes[i] != NULL; i++) {
		sl = g_list_prepend (sl, (gpointer) sizes[i]);
	}
	sl = g_list_reverse (sl);
	gtk_combo_set_popdown_strings (GTK_COMBO (fsel->size), sl);
	g_list_free (sl);

	fsel->familyidx = 0;
	fsel->styleidx = 0;
	fsel->fontsize = 10.0;
	fsel->font = NULL;
}

static void
sp_font_selector_destroy (GtkObject *object)
{
	SPFontSelector *fsel;

	fsel = SP_FONT_SELECTOR (object);

	if (fsel->font) {
		fsel->font = nr_font_unref (fsel->font);
	}

	if (fsel->families.length > 0) {
		nr_name_list_release (&fsel->families);
		fsel->families.length = 0;
	}

	if (fsel->styles.length > 0) {
		nr_name_list_release (&fsel->styles);
		fsel->styles.length = 0;
	}

  	if (GTK_OBJECT_CLASS (fs_parent_class)->destroy)
		GTK_OBJECT_CLASS (fs_parent_class)->destroy (object);
}

static void
sp_font_selector_family_select_row (GtkCList *clist, gint row, gint column, GdkEvent *event, SPFontSelector *fsel)
{
	fsel->familyidx = GPOINTER_TO_UINT (gtk_clist_get_row_data (clist, row));

	if (fsel->styles.length > 0) {
		nr_name_list_release (&fsel->styles);
		fsel->styles.length = 0;
		fsel->styleidx = 0;
	}
	gtk_clist_clear (GTK_CLIST (fsel->style));

	if (fsel->familyidx < fsel->families.length) {
		const unsigned char *family;
		family = fsel->families.names[fsel->familyidx];
		if (nr_type_directory_style_list_get (family, &fsel->styles)) {
			gint i;
			gtk_clist_freeze (GTK_CLIST (fsel->style));
			for (i = 0; i < fsel->styles.length; i++) {
				const unsigned char *p;

				p = fsel->styles.names[i] + strlen (family);
				while (*p && isspace (*p)) p += 1;
				if (!*p) p = "Normal";

				gtk_clist_append (GTK_CLIST (fsel->style), (gchar **) &p);
				gtk_clist_set_row_data (GTK_CLIST (fsel->style), i, GUINT_TO_POINTER (i));
			}
			gtk_clist_thaw (GTK_CLIST (fsel->style));
			gtk_clist_select_row (GTK_CLIST (fsel->style), 0, 0);
		}
	}
}

static void
sp_font_selector_style_select_row (GtkCList *clist, gint row, gint column, GdkEvent *event, SPFontSelector *fsel)
{
	fsel->styleidx = GPOINTER_TO_UINT (gtk_clist_get_row_data (clist, row));

	if (!fsel->block_emit) {
		sp_font_selector_emit_set (fsel);
	}
}

static void
sp_font_selector_size_changed (GtkEditable *editable, SPFontSelector *fsel)
{
	const gchar *sstr;

	sstr = gtk_entry_get_text (GTK_ENTRY (GTK_COMBO (fsel->size)->entry));
	fsel->fontsize = MAX (atof (sstr), 0.1);

	sp_font_selector_emit_set (fsel);
}

static void
sp_font_selector_emit_set (SPFontSelector *fsel)
{
	NRTypeFace *tf;
	NRFont *font;

	if (fsel->styleidx < fsel->styles.length) {
		tf = nr_type_directory_lookup (fsel->styles.names[fsel->styleidx]);
		font = nr_font_new_default (tf, NR_TYPEFACE_METRICS_DEFAULT, fsel->fontsize);
		nr_typeface_unref (tf);
	} else {
		font = NULL;
	}

	if (font != fsel->font) {
		if (fsel->font) fsel->font = nr_font_unref (fsel->font);
		if (font) fsel->font = nr_font_ref (font);
		gtk_signal_emit (GTK_OBJECT (fsel), fs_signals[FONT_SET], fsel->font);
	}

	if (font) nr_font_unref (font);
}

GtkWidget *
sp_font_selector_new (void)
{
	SPFontSelector *fsel;
  
	fsel = gtk_type_new (SP_TYPE_FONT_SELECTOR);
  
	gtk_clist_select_row (GTK_CLIST (fsel->family), 0, 0);

	return (GtkWidget *) fsel;
}

void
sp_font_selector_set_font (SPFontSelector *fsel, NRFont *font)
{
	GtkCList *fcl, *scl;

	fcl = GTK_CLIST (fsel->family);
	scl = GTK_CLIST (fsel->style);

	if (font) {
		unsigned char n[256], s[8];
		int i;
		nr_typeface_family_name_get (NR_FONT_TYPEFACE (font), n, 256);
		for (i = 0; i < fsel->families.length; i++) {
			if (!strcmp (n, fsel->families.names[i])) break;
		}
		if (i >= fsel->families.length) return;
		fsel->block_emit = TRUE;
		gtk_clist_select_row (GTK_CLIST (fsel->family), i, 0);
		fsel->block_emit = FALSE;

		nr_typeface_name_get (NR_FONT_TYPEFACE (font), n, 256);
		for (i = 0; i < fsel->styles.length; i++) {
			if (!strcmp (n, fsel->styles.names[i])) break;
		}
		if (i >= fsel->styles.length) return;
		gtk_clist_select_row (scl, i, 0);

		g_snprintf (s, 8, "%.2g", NR_FONT_SIZE (font));
		gtk_entry_set_text (GTK_ENTRY (GTK_COMBO (fsel->size)->entry), s);
	}
}

void
sp_font_selector_set_font_fuzzy (SPFontSelector *fsel, const guchar *family, const guchar *style)
{
	NRTypeFace *tf;
	NRFont *font;

	tf = nr_type_directory_lookup_fuzzy (family, style);
	font = nr_font_new_default (tf, NR_TYPEFACE_METRICS_DEFAULT, fsel->fontsize);
	nr_typeface_unref (tf);
	sp_font_selector_set_font (fsel, font);
	if (font) nr_font_unref (font);
}

NRFont*
sp_font_selector_get_font (SPFontSelector *fsel)
{
	if (fsel->font) nr_font_ref (fsel->font);

	return fsel->font;
}

/* SPFontPreview */

struct _SPFontPreview
{
	GtkDrawingArea darea;

	NRFont *font;
	NRRasterFont *rfont;
	unsigned char *phrase;
	unsigned long rgba;
};


struct _SPFontPreviewClass
{
	GtkDrawingAreaClass parent_class;
};

static void sp_font_preview_class_init (SPFontPreviewClass *klass);
static void sp_font_preview_init (SPFontPreview *fsel);
static void sp_font_preview_destroy (GtkObject *object);

void sp_font_preview_size_request (GtkWidget *widget, GtkRequisition *req);
static gint sp_font_preview_expose (GtkWidget *widget, GdkEventExpose *event);

static GtkDrawingAreaClass *fp_parent_class = NULL;

GtkType
sp_font_preview_get_type ()
{
	static GtkType type = 0;
	if (!type) {
		static const GtkTypeInfo info = {
			"SPFontPreview",
			sizeof (SPFontPreview),
			sizeof (SPFontPreviewClass),
			(GtkClassInitFunc) sp_font_preview_class_init,
			(GtkObjectInitFunc) sp_font_preview_init,
			NULL, NULL, NULL
		};
		type = gtk_type_unique (GTK_TYPE_DRAWING_AREA, &info);
	}
	return type;
}

static void
sp_font_preview_class_init (SPFontPreviewClass *klass)
{
	GtkObjectClass *object_class;
	GtkWidgetClass *widget_class;

	object_class = (GtkObjectClass *) klass;
	widget_class = (GtkWidgetClass *) klass;
  
	fp_parent_class = gtk_type_class (GTK_TYPE_DRAWING_AREA);

	object_class->destroy = sp_font_preview_destroy;

	widget_class->size_request = sp_font_preview_size_request;
	widget_class->expose_event = sp_font_preview_expose;
}

static void
sp_font_preview_init (SPFontPreview *fprev)
{
	fprev->rgba = 0x000000ff;
}

static void
sp_font_preview_destroy (GtkObject *object)
{
	SPFontPreview *fprev;

	fprev = SP_FONT_PREVIEW (object);

	if (fprev->rfont) {
		fprev->rfont = nr_rasterfont_unref (fprev->rfont);
	}

	if (fprev->font) {
		fprev->font = nr_font_unref (fprev->font);
	}

	if (fprev->phrase) {
		g_free (fprev->phrase);
		fprev->phrase = NULL;
	}

  	if (GTK_OBJECT_CLASS (fp_parent_class)->destroy)
		GTK_OBJECT_CLASS (fp_parent_class)->destroy (object);
}

void
sp_font_preview_size_request (GtkWidget *widget, GtkRequisition *req)
{
	req->width = 256;
	req->height = 32;
}

#define SPFP_MAX_LEN 64

static gint
sp_font_preview_expose (GtkWidget *widget, GdkEventExpose *event)
{
	SPFontPreview *fprev;

	fprev = SP_FONT_PREVIEW (widget);

	if (GTK_WIDGET_DRAWABLE (widget)) {
		if (fprev->rfont) {
			NRTypeFace *tface;
			unsigned char *p;
			int glyphs[SPFP_MAX_LEN];
			int hpos[SPFP_MAX_LEN];
			float px, py;
			int len;
			NRRectF bbox;
			float startx, starty;
			int x, y;
			tface = NR_RASTERFONT_TYPEFACE (fprev->rfont);
			if (fprev->phrase) {
				p = fprev->phrase;
			} else {
				p = _("AaBbCcIiPpQq12368.;/()");
			}
			px = 0.0;
			py = 0.0;
			len = 0;
			bbox.x0 = bbox.y0 = bbox.x1 = bbox.y1 = 0.0;
			while (p && *p && (len < SPFP_MAX_LEN)) {
				unsigned int unival;
				NRPointF adv;
				NRRectF gbox;
				unival = g_utf8_get_char (p);
				glyphs[len] = nr_typeface_lookup_default (tface, unival);
				hpos[len] = px;
				nr_rasterfont_glyph_advance_get (fprev->rfont, glyphs[len], &adv);
				nr_rasterfont_glyph_area_get (fprev->rfont, glyphs[len], &gbox);
				bbox.x0 = MIN (px + gbox.x0, bbox.x0);
				bbox.y0 = MIN (py + gbox.y0, bbox.y0);
				bbox.x1 = MAX (px + gbox.x1, bbox.x1);
				bbox.y1 = MAX (py + gbox.y1, bbox.y1);
				px += adv.x;
				len += 1;
				p = g_utf8_next_char (p);
			}
			startx = (widget->allocation.width - (bbox.x1 - bbox.x0)) / 2;
			starty = widget->allocation.height - (widget->allocation.height - (bbox.y1 - bbox.y0)) / 2 - bbox.y1;
			for (y = event->area.y; y < event->area.y + event->area.height; y += 64) {
				for (x = event->area.x; x < event->area.x + event->area.width; x += 64) {
					unsigned char *ps;
					NRPixBlock pb, m;
					int x0, y0, x1, y1;
					int i;
					x0 = x;
					y0 = y;
					x1 = MIN (x0 + 64, event->area.x + event->area.width);
					y1 = MIN (y0 + 64, event->area.y + event->area.height);
					ps = nr_pixelstore_16K_new (TRUE, 0xff);
					nr_pixblock_setup_extern (&pb, NR_PIXBLOCK_MODE_R8G8B8, x0, y0, x1, y1, ps, 3 * (x1 - x0), FALSE, FALSE);
					nr_pixblock_setup_fast (&m, NR_PIXBLOCK_MODE_A8, x0, y0, x1, y1, TRUE);
					pb.empty = FALSE;
					for (i = 0; i < len; i++) {
						nr_rasterfont_glyph_mask_render (fprev->rfont, glyphs[i], &m, hpos[i] + startx, starty);
					}
					nr_blit_pixblock_mask_rgba32 (&pb, &m, fprev->rgba);
					gdk_draw_rgb_image (widget->window, widget->style->black_gc,
							    x0, y0, x1 - x0, y1 - y0,
							    GDK_RGB_DITHER_NONE, NR_PIXBLOCK_PX (&pb), pb.rs);
					nr_pixblock_release (&m);
					nr_pixblock_release (&pb);
					nr_pixelstore_16K_free (ps);
				}
			}
		} else {
			nr_gdk_draw_gray_garbage (widget->window, widget->style->black_gc,
						  event->area.x, event->area.y,
						  event->area.width, event->area.height);
		}
	}

	return TRUE;
}

GtkWidget *
sp_font_preview_new (void)
{
	GtkWidget *w;

	w = gtk_type_new (SP_TYPE_FONT_PREVIEW);

	return w;
}

void
sp_font_preview_set_font (SPFontPreview *fprev, NRFont *font)
{
	if (font) nr_font_ref (font);
	if (fprev->font) nr_font_unref (fprev->font);
	fprev->font = font;

	if (fprev->rfont) {
		fprev->rfont = nr_rasterfont_unref (fprev->rfont);
	}
	if (fprev->font) {
		NRMatrixF flip;
		nr_matrix_f_set_scale (&flip, 1.0, -1.0);
		fprev->rfont = nr_rasterfont_new (fprev->font, &flip);
	}
	if (GTK_WIDGET_DRAWABLE (fprev)) gtk_widget_queue_draw (GTK_WIDGET (fprev));
}

void
sp_font_preview_set_rgba32 (SPFontPreview *fprev, guint32 rgba)
{
	fprev->rgba = rgba;
	if (GTK_WIDGET_DRAWABLE (fprev)) gtk_widget_queue_draw (GTK_WIDGET (fprev));
}

void
sp_font_preview_set_phrase (SPFontPreview *fprev, const guchar *phrase)
{
	if (fprev->phrase) g_free (fprev->phrase);
	if (phrase) {
		fprev->phrase = g_strdup (phrase);
	} else {
		fprev->phrase = NULL;
	}
	if (GTK_WIDGET_DRAWABLE (fprev)) gtk_widget_queue_draw (GTK_WIDGET (fprev));
}

