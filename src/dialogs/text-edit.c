#define __SP_TEXT_EDIT_C__

/*
 * Text editing dialog
 *
 * Author:
 *   Lauris Kaplinski <lauris@ximian.com>
 *
 * Copyright (C) 1999-2002 Lauris Kaplinski
 * Copyright (C) 2000-2001 Ximian, Inc.
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <config.h>

#include <string.h>
#include <libnrtype/nr-type-directory.h>

#include <glib.h>
#include <gtk/gtksignal.h>
#include <gtk/gtkstock.h>
#include <gtk/gtknotebook.h>
#include <gtk/gtkvbox.h>
#include <gtk/gtkhbox.h>
#include <gtk/gtklabel.h>
#include <gtk/gtkframe.h>
#include <gtk/gtktable.h>
#include <gtk/gtktextview.h>
#include <gtk/gtkradiobutton.h>
#include <gtk/gtkhseparator.h>
#include <gtk/gtkimage.h>
#include <gtk/gtkentry.h>
#include <gtk/gtkcombo.h>

#include "macros.h"
#include "helper/sp-intl.h"
#include "helper/window.h"
#include "../widgets/font-selector.h"
#include "../forward.h"
#include "../sodipodi.h"
#include "../document.h"
#include "../desktop-handles.h"
#include "../selection.h"
#include "../style.h"
#include "../sp-text.h"

#include "text-edit.h"

static void sp_text_edit_dialog_modify_selection (Sodipodi *sodipodi, SPSelection *sel, guint flags, GtkWidget *dlg);
static void sp_text_edit_dialog_change_selection (Sodipodi *sodipodi, SPSelection *sel, GtkWidget *dlg);

static void sp_text_edit_dialog_set_default (GtkButton *button, GtkWidget *dlg);
static void sp_text_edit_dialog_apply (GtkButton *button, GtkWidget *dlg);
static void sp_text_edit_dialog_close (GtkButton *button, GtkWidget *dlg);

static void sp_text_edit_dialog_read_selection (GtkWidget *dlg, gboolean style, gboolean content);

static void sp_text_edit_dialog_text_changed (GtkTextBuffer *tb, GtkWidget *dlg);
static void sp_text_edit_dialog_font_changed (SPFontSelector *fontsel, NRFont *font, GtkWidget *dlg);
static void sp_text_edit_dialog_any_toggled (GtkToggleButton *tb, GtkWidget *dlg);
static void sp_text_edit_dialog_line_spacing_changed (GtkEditable *editable, GtkWidget *dlg);

static SPText *sp_ted_get_selected_text_item (void);
static unsigned sp_ted_get_selected_text_count (void);


static const unsigned char *spacings[] = {"90%", "100%", "110%", "120%", "133%", "150%", "200%", NULL};

static GtkWidget *dlg = NULL;

static void
sp_text_edit_dialog_destroy (GtkObject *object, gpointer data)
{
	sp_signal_disconnect_by_data (SODIPODI, dlg);
	dlg = NULL;
}

static gint
sp_text_edit_dialog_delete (GtkWidget *dlg, GdkEvent *event, gpointer data)
{
	gtk_widget_destroy (GTK_WIDGET (dlg));
	return TRUE;
}

void
sp_text_edit_dialog (void)
{
	if (!dlg) {
		GtkWidget *mainvb, *nb, *vb, *hb, *txt, *fontsel, *preview, *f, *tbl, *l, *px, *c, *b, *hs;
		GtkTextBuffer *tb;
		GList *sl;
		int i;

		dlg = sp_window_new (_("Text properties"), TRUE);
		gtk_window_set_policy (GTK_WINDOW (dlg), TRUE, TRUE, FALSE);
		g_signal_connect (G_OBJECT (dlg), "destroy", G_CALLBACK (sp_text_edit_dialog_destroy), dlg);
		g_signal_connect (G_OBJECT (dlg), "delete_event", G_CALLBACK (sp_text_edit_dialog_delete), dlg);

		mainvb = gtk_vbox_new (FALSE, 0);
		gtk_container_add (GTK_CONTAINER (dlg), mainvb);

		nb = gtk_notebook_new ();
		gtk_container_set_border_width (GTK_CONTAINER (nb), 4);
		gtk_box_pack_start (GTK_BOX (mainvb), nb, TRUE, TRUE, 0);
		g_object_set_data (G_OBJECT (dlg), "notebook", nb);

		/* Vbox inside notebook */
		vb = gtk_vbox_new (FALSE, 0);

		/* Textview */
		f = gtk_frame_new (NULL);
		gtk_frame_set_shadow_type (GTK_FRAME (f), GTK_SHADOW_IN);
		tb = gtk_text_buffer_new (NULL);
		txt = gtk_text_view_new_with_buffer (tb);
		gtk_widget_set_size_request (txt, -1, 64);
		gtk_text_view_set_editable (GTK_TEXT_VIEW (txt), TRUE);
		gtk_container_add (GTK_CONTAINER (f), txt);
		gtk_box_pack_start (GTK_BOX (vb), f, TRUE, TRUE, 0);
		g_signal_connect (G_OBJECT (tb), "changed", G_CALLBACK (sp_text_edit_dialog_text_changed), dlg);
		g_object_set_data (G_OBJECT (dlg), "text", tb);
		g_object_set_data (G_OBJECT (dlg), "textw", txt);

		/* HBox containing font selection and layout */
		hb = gtk_hbox_new (FALSE, 0);
		gtk_box_pack_start (GTK_BOX (vb), hb, TRUE, TRUE, 0);

		fontsel = sp_font_selector_new ();
		g_signal_connect (G_OBJECT (fontsel), "font_set", G_CALLBACK (sp_text_edit_dialog_font_changed), dlg);
		gtk_box_pack_start (GTK_BOX (hb), fontsel, TRUE, TRUE, 0);
		g_object_set_data (G_OBJECT (dlg), "fontsel", fontsel);

		/* Layout */
		f = gtk_frame_new (_("Layout"));
		gtk_box_pack_start (GTK_BOX (hb), f, FALSE, FALSE, 4);

		tbl = gtk_table_new (3, 4, FALSE);
		gtk_table_set_row_spacings (GTK_TABLE (tbl), 4);
		gtk_table_set_col_spacings (GTK_TABLE (tbl), 4);
		gtk_container_add (GTK_CONTAINER (f), tbl);

		l = gtk_label_new (_("Alignment:"));
		gtk_misc_set_alignment (GTK_MISC (l), 1.0, 0.5);
		gtk_table_attach (GTK_TABLE (tbl), l, 0, 1, 0, 1, 0, 0, 4, 0);
		px = gtk_image_new_from_stock (GTK_STOCK_JUSTIFY_LEFT, GTK_ICON_SIZE_LARGE_TOOLBAR);
		b = gtk_radio_button_new (NULL);
		g_signal_connect (G_OBJECT (b), "toggled", G_CALLBACK (sp_text_edit_dialog_any_toggled), dlg);
		gtk_toggle_button_set_mode (GTK_TOGGLE_BUTTON (b), FALSE);
		gtk_container_add (GTK_CONTAINER (b), px);
		gtk_table_attach (GTK_TABLE (tbl), b, 1, 2, 0, 1, 0, 0, 0, 0);
		g_object_set_data (G_OBJECT (dlg), "text_anchor_start", b);
		px = gtk_image_new_from_stock (GTK_STOCK_JUSTIFY_CENTER, GTK_ICON_SIZE_LARGE_TOOLBAR);
		b = gtk_radio_button_new (gtk_radio_button_group (GTK_RADIO_BUTTON (b)));
		g_signal_connect (G_OBJECT (b), "toggled", G_CALLBACK (sp_text_edit_dialog_any_toggled), dlg);
		gtk_toggle_button_set_mode (GTK_TOGGLE_BUTTON (b), FALSE);
		gtk_container_add (GTK_CONTAINER (b), px);
		gtk_table_attach (GTK_TABLE (tbl), b, 2, 3, 0, 1, 0, 0, 0, 0);
		g_object_set_data (G_OBJECT (dlg), "text_anchor_middle", b);
		px = gtk_image_new_from_stock (GTK_STOCK_JUSTIFY_RIGHT, GTK_ICON_SIZE_LARGE_TOOLBAR);
		b = gtk_radio_button_new (gtk_radio_button_group (GTK_RADIO_BUTTON (b)));
		g_signal_connect (G_OBJECT (b), "toggled", G_CALLBACK (sp_text_edit_dialog_any_toggled), dlg);
		gtk_toggle_button_set_mode (GTK_TOGGLE_BUTTON (b), FALSE);
		gtk_container_add (GTK_CONTAINER (b), px);
		gtk_table_attach (GTK_TABLE (tbl), b, 3, 4, 0, 1, 0, 0, 0, 0);
		g_object_set_data (G_OBJECT (dlg), "text_anchor_end", b);

		l = gtk_label_new (_("Orientation:"));
		gtk_misc_set_alignment (GTK_MISC (l), 1.0, 0.5);
		gtk_table_attach (GTK_TABLE (tbl), l, 0, 1, 1, 2, 0, 0, 4, 0);
		px = gtk_image_new_from_file (SODIPODI_GLADEDIR "/writing_mode_lr.xpm");
		b = gtk_radio_button_new (NULL);
		g_signal_connect (G_OBJECT (b), "toggled", G_CALLBACK (sp_text_edit_dialog_any_toggled), dlg);
		gtk_toggle_button_set_mode (GTK_TOGGLE_BUTTON (b), FALSE);
		gtk_container_add (GTK_CONTAINER (b), px);
		gtk_table_attach (GTK_TABLE (tbl), b, 1, 2, 1, 2, 0, 0, 0, 0);
		g_object_set_data (G_OBJECT (dlg), "writing_mode_lr", b);
		px = gtk_image_new_from_file (SODIPODI_GLADEDIR "/writing_mode_tb.xpm");
		b = gtk_radio_button_new (gtk_radio_button_group (GTK_RADIO_BUTTON (b)));
		g_signal_connect (G_OBJECT (b), "toggled", G_CALLBACK (sp_text_edit_dialog_any_toggled), dlg);
		gtk_toggle_button_set_mode (GTK_TOGGLE_BUTTON (b), FALSE);
		gtk_container_add (GTK_CONTAINER (b), px);
		gtk_table_attach (GTK_TABLE (tbl), b, 2, 3, 1, 2, 0, 0, 0, 0);
		g_object_set_data (G_OBJECT (dlg), "writing_mode_tb", b);

		l = gtk_label_new (_("Line spacing:"));
		gtk_misc_set_alignment (GTK_MISC (l), 1.0, 0.5);
		gtk_table_attach (GTK_TABLE (tbl), l, 0, 1, 2, 3, GTK_EXPAND | GTK_FILL, 0, 4, 0);
		c = gtk_combo_new ();
		gtk_combo_set_value_in_list ((GtkCombo *) c, FALSE, FALSE);
		gtk_combo_set_use_arrows ((GtkCombo *) c, TRUE);
		gtk_combo_set_use_arrows_always ((GtkCombo *) c, TRUE);
		gtk_widget_set_size_request (c, 64, -1);
		/* Setup strings */
		sl = NULL;
		for (i = 0; spacings[i]; i++) {
			sl = g_list_prepend (sl, (void *) spacings[i]);
		}
		sl = g_list_reverse (sl);
		gtk_combo_set_popdown_strings ((GtkCombo *) c, sl);
		g_list_free (sl);
		g_signal_connect ((GObject *) ((GtkCombo *) c)->entry, "changed", (GCallback) sp_text_edit_dialog_line_spacing_changed, dlg);
		gtk_table_attach (GTK_TABLE (tbl), c, 1, 4, 2, 3, 0, 0, 4, 0);
		g_object_set_data (G_OBJECT (dlg), "line_spacing", c);

		/* Font preview */
		preview = sp_font_preview_new ();
		gtk_box_pack_start (GTK_BOX (vb), preview, TRUE, TRUE, 4);
		g_object_set_data (G_OBJECT (dlg), "preview", preview);

		l = gtk_label_new (_("Text and font"));
		gtk_notebook_append_page (GTK_NOTEBOOK (nb), vb, l);

		/* Buttons */
		hs = gtk_hseparator_new ();
		gtk_box_pack_start (GTK_BOX (mainvb), hs, FALSE, FALSE, 0);

		hb = gtk_hbox_new (FALSE, 4);
		gtk_container_set_border_width (GTK_CONTAINER (hb), 4);
		gtk_box_pack_start (GTK_BOX (mainvb), hb, FALSE, FALSE, 0);

		b = gtk_button_new_with_label (_("Set as default"));
		g_signal_connect (G_OBJECT (b), "clicked", G_CALLBACK (sp_text_edit_dialog_set_default), dlg);
		gtk_box_pack_start (GTK_BOX (hb), b, FALSE, FALSE, 0);
		g_object_set_data (G_OBJECT (dlg), "default", b);

		b = gtk_button_new_from_stock (GTK_STOCK_CLOSE);
		g_signal_connect (G_OBJECT (b), "clicked", G_CALLBACK (sp_text_edit_dialog_close), dlg);
		gtk_box_pack_end (GTK_BOX (hb), b, FALSE, FALSE, 0);

		b = gtk_button_new_from_stock (GTK_STOCK_APPLY);
		g_signal_connect (G_OBJECT (b), "clicked", G_CALLBACK (sp_text_edit_dialog_apply), dlg);
		gtk_box_pack_end (GTK_BOX (hb), b, FALSE, FALSE, 0);
		g_object_set_data (G_OBJECT (dlg), "apply", b);

		g_signal_connect (G_OBJECT (SODIPODI), "modify_selection", G_CALLBACK (sp_text_edit_dialog_modify_selection), dlg);
		g_signal_connect (G_OBJECT (SODIPODI), "change_selection", G_CALLBACK (sp_text_edit_dialog_change_selection), dlg);

		sp_text_edit_dialog_read_selection (dlg, TRUE, TRUE);

		gtk_widget_show_all (dlg);
	}

	gtk_window_present ((GtkWindow *) dlg);
}

static void
sp_text_edit_dialog_modify_selection (Sodipodi *sodipodi, SPSelection *sel, guint flags, GtkWidget *dlg)
{
	gboolean style, content;

	style = ((flags & (SP_OBJECT_CHILD_MODIFIED_FLAG | SP_OBJECT_STYLE_MODIFIED_FLAG )) != 0);
	content = ((flags & (SP_OBJECT_CHILD_MODIFIED_FLAG | SP_TEXT_CONTENT_MODIFIED_FLAG)) != 0);
	sp_text_edit_dialog_read_selection (dlg, style, content);
}

static void
sp_text_edit_dialog_change_selection (Sodipodi *sodipodi, SPSelection *sel, GtkWidget *dlg)
{
	sp_text_edit_dialog_read_selection (dlg, TRUE, TRUE);
}


static void
sp_text_edit_dialog_update_object (SPText *text, SPRepr *repr)
{
	g_object_set_data (G_OBJECT (dlg), "blocked", GINT_TO_POINTER (TRUE));

	if (text) {
		GtkTextBuffer *tb;
		GtkTextIter start, end;
		guchar *str;

		tb = g_object_get_data (G_OBJECT (dlg), "text");

		/* Content */
		gtk_text_buffer_get_bounds (tb, &start, &end);
		str = gtk_text_buffer_get_text (tb, &start, &end, TRUE);
		sp_text_set_repr_text_multiline (text, str);
		g_free (str);
	}

	if (repr) {
		GtkWidget *fontsel, *preview, *b, *combo;
		SPCSSAttr *css;
		NRFont *font;
		guchar c[256];
		const char *sstr;

		fontsel = g_object_get_data (G_OBJECT (dlg), "fontsel");
		preview = g_object_get_data (G_OBJECT (dlg), "preview");

		css = sp_repr_css_attr_new ();

		/* font */
		font = sp_font_selector_get_font (SP_FONT_SELECTOR (fontsel));
		nr_typeface_family_name_get (NR_FONT_TYPEFACE (font), c, 256);
		sp_repr_css_set_property (css, "font-family", c);
		nr_typeface_attribute_get (NR_FONT_TYPEFACE (font), "weight", c, 256);
		g_strdown (c);
		sp_repr_css_set_property (css, "font-weight", c);
		nr_typeface_attribute_get (NR_FONT_TYPEFACE (font), "style", c, 256);
		g_strdown (c);
		sp_repr_css_set_property (css, "font-style", c);
		g_snprintf (c, 64, "%g", NR_FONT_SIZE (font));
		sp_repr_css_set_property (css, "font-size", c);
		nr_font_unref (font);
		/* Layout */
		b = g_object_get_data (G_OBJECT (dlg), "text_anchor_start");
		if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (b))) {
			sp_repr_css_set_property (css, "text-anchor", "start");
		} else {
			b = g_object_get_data (G_OBJECT (dlg), "text_anchor_middle");
			if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (b))) {
				sp_repr_css_set_property (css, "text-anchor", "middle");
			} else {
				sp_repr_css_set_property (css, "text-anchor", "end");
			}
		}
		b = g_object_get_data (G_OBJECT (dlg), "writing_mode_lr");
		if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (b))) {
			sp_repr_css_set_property (css, "writing-mode", "lr");
		} else {
			sp_repr_css_set_property (css, "writing-mode", "tb");
		}
		combo = g_object_get_data ((GObject *) dlg, "line_spacing");
		sstr = gtk_entry_get_text ((GtkEntry *) ((GtkCombo *) (combo))->entry);
		sp_repr_set_attr (repr, "sodipodi:linespacing", sstr);

		sp_repr_css_change (repr, css, "style");
		sp_repr_css_attr_unref (css);
	}

	if (text) {
		sp_document_done (SP_DT_DOCUMENT (SP_ACTIVE_DESKTOP));
		sp_document_ensure_up_to_date (SP_OBJECT_DOCUMENT (text));
	}

	g_object_set_data (G_OBJECT (dlg), "blocked", NULL);
}

static void
sp_text_edit_dialog_set_default (GtkButton *button, GtkWidget *dlg)
{
	GtkWidget *def;
	SPRepr *repr;

	def = g_object_get_data (G_OBJECT (dlg), "default");

	repr = sodipodi_get_repr (SODIPODI, "tools.text");

	sp_text_edit_dialog_update_object (NULL, repr);

	gtk_widget_set_sensitive (def, FALSE);
}

static void
sp_text_edit_dialog_apply (GtkButton *button, GtkWidget *dlg)
{
	GtkWidget *apply, *def;
	SPText *text;
	SPRepr *repr;
	const GSList *item;
	unsigned items;
	
	g_object_set_data (G_OBJECT (dlg), "blocked", GINT_TO_POINTER (TRUE));

	apply = g_object_get_data (G_OBJECT (dlg), "apply");
	def = g_object_get_data (G_OBJECT (dlg), "default");

	text = NULL;
	items = 0;
	item = sp_selection_item_list(SP_DT_SELECTION (SP_ACTIVE_DESKTOP));
	
	for (; item != NULL; item = item->next) {
		if (!SP_IS_TEXT (item->data)) continue;
		text = SP_TEXT(item->data);
		repr = SP_OBJECT_REPR (text);
		sp_text_edit_dialog_update_object (NULL, repr);
		++items;
	}
	
	if (items == 1) {
		sp_text_edit_dialog_update_object (text, NULL);
	} else if (items == 0) {
		repr = sodipodi_get_repr (SODIPODI, "tools.text");
		sp_text_edit_dialog_update_object (NULL, repr);
		gtk_widget_set_sensitive (def, FALSE);
	}

	gtk_widget_set_sensitive (apply, FALSE);

	g_object_set_data (G_OBJECT (dlg), "blocked", NULL);
}

static void
sp_text_edit_dialog_close (GtkButton *button, GtkWidget *dlg)
{
	gtk_widget_destroy (GTK_WIDGET (dlg));
}

static guchar *
sp_text_edit_dialog_font_style_to_lookup (SPStyle *style)
{
	static guchar c[256];
	guchar *wstr, *sstr, *p;

	switch (style->font_weight.computed) {
	case SP_CSS_FONT_WEIGHT_100:
		wstr = "extra light";
		break;
	case SP_CSS_FONT_WEIGHT_200:
		wstr = "thin";
		break;
	case SP_CSS_FONT_WEIGHT_300:
		wstr = "light";
		break;
	case SP_CSS_FONT_WEIGHT_400:
	case SP_CSS_FONT_WEIGHT_NORMAL:
		wstr = NULL;
		break;
	case SP_CSS_FONT_WEIGHT_500:
		wstr = "medium";
		break;
	case SP_CSS_FONT_WEIGHT_600:
		wstr = "semi";
		break;
	case SP_CSS_FONT_WEIGHT_700:
	case SP_CSS_FONT_WEIGHT_BOLD:
		wstr = "bold";
		break;
	case SP_CSS_FONT_WEIGHT_800:
		wstr = "heavy";
		break;
	case SP_CSS_FONT_WEIGHT_900:
		wstr = "black";
		break;
	default:
		wstr = NULL;
		break;
	}

	switch (style->font_style.computed) {
	case SP_CSS_FONT_STYLE_NORMAL:
		sstr = NULL;
		break;
	case SP_CSS_FONT_STYLE_ITALIC:
		sstr = "italic";
		break;
	case SP_CSS_FONT_STYLE_OBLIQUE:
		sstr = "oblique";
		break;
	default:
		sstr = NULL;
		break;
	}

	p = c;
	if (wstr) {
		if (p != c) *p++ = ' ';
		memcpy (p, wstr, strlen (wstr));
		p += strlen (wstr);
	}
	if (sstr) {
		if (p != c) *p++ = ' ';
		memcpy (p, sstr, strlen (sstr));
		p += strlen (sstr);
	}
	*p = '\0';

	return c;
}

static void
sp_text_edit_dialog_read_selection (GtkWidget *dlg, gboolean dostyle, gboolean docontent)
{
	GtkWidget *notebook, *textw, *fontsel, *preview, *apply, *def;
	GtkTextBuffer *tb;
	SPText *text;
	SPRepr *repr;
	SPStyle *style;
	unsigned items;

	if (g_object_get_data (G_OBJECT (dlg), "blocked")) return;
	g_object_set_data (G_OBJECT (dlg), "blocked", GINT_TO_POINTER (TRUE));

	notebook = g_object_get_data (G_OBJECT (dlg), "notebook");
	textw = g_object_get_data (G_OBJECT (dlg), "textw");
	tb = g_object_get_data (G_OBJECT (dlg), "text");
	fontsel = g_object_get_data (G_OBJECT (dlg), "fontsel");
	preview = g_object_get_data (G_OBJECT (dlg), "preview");
	apply = g_object_get_data (G_OBJECT (dlg), "apply");
	def = g_object_get_data (G_OBJECT (dlg), "default");

	text = sp_ted_get_selected_text_item ();

	if (text) {
		items = sp_ted_get_selected_text_count ();
		if (items == 1) {
			gtk_widget_set_sensitive (textw, TRUE);
		} else {
			gtk_widget_set_sensitive (textw, FALSE);
		}
		gtk_widget_set_sensitive (apply, FALSE);
		gtk_widget_set_sensitive (def, TRUE);
		style = SP_OBJECT_STYLE (text);
		if (docontent) {
			guchar *str;
			str = sp_text_get_string_multiline (text);
			if (str) {
				int pos;
				pos = 0;
				if (items == 1) {
					gtk_text_buffer_set_text (tb, str, strlen (str));
				}
				sp_font_preview_set_phrase (SP_FONT_PREVIEW (preview), str);
				g_free (str);
			} else {
				gtk_text_buffer_set_text (tb, "", 0);
				sp_font_preview_set_phrase (SP_FONT_PREVIEW (preview), NULL);
			}
		}
		repr = SP_OBJECT_REPR (text);
	} else {
		gtk_widget_set_sensitive (textw, FALSE);
		gtk_widget_set_sensitive (apply, FALSE);
		gtk_widget_set_sensitive (def, FALSE);
		repr = sodipodi_get_repr (SODIPODI, "tools.text");
		if (repr) {
			gtk_widget_set_sensitive (notebook, TRUE);
			style = sp_style_new ();
			sp_style_read_from_repr (style, repr);
		} else {
			gtk_widget_set_sensitive (notebook, FALSE);
			style = sp_style_new ();
		}
	}

	if (dostyle) {
		NRTypeFace *tf;
		NRFont *font;
		GtkWidget *b, *combo;
		guchar *c;
		const unsigned char *sstr;

		c = sp_text_edit_dialog_font_style_to_lookup (style);
		tf = nr_type_directory_lookup_fuzzy (style->text->font_family.value, c);
		font = nr_font_new_default (tf, NR_TYPEFACE_METRICS_HORIZONTAL, style->font_size.computed);
		nr_typeface_unref (tf);

		if (font) {
			sp_font_selector_set_font (SP_FONT_SELECTOR (fontsel), font);
			sp_font_preview_set_font (SP_FONT_PREVIEW (preview), font);
			nr_font_unref (font);
		}

		if (style->text_anchor.computed == SP_CSS_TEXT_ANCHOR_START) {
			b = g_object_get_data (G_OBJECT (dlg), "text_anchor_start");
		} else if (style->text_anchor.computed == SP_CSS_TEXT_ANCHOR_MIDDLE) {
			b = g_object_get_data (G_OBJECT (dlg), "text_anchor_middle");
		} else {
			b = g_object_get_data (G_OBJECT (dlg), "text_anchor_end");
		}
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (b), TRUE);
		if (style->writing_mode.computed == SP_CSS_WRITING_MODE_LR) {
			b = g_object_get_data (G_OBJECT (dlg), "writing_mode_lr");
		} else {
			b = g_object_get_data (G_OBJECT (dlg), "writing_mode_tb");
		}
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (b), TRUE);
		combo = g_object_get_data (G_OBJECT (dlg), "line_spacing");
		sstr = (repr) ? sp_repr_attr (repr, "sodipodi:linespacing") : NULL;
		gtk_entry_set_text ((GtkEntry *) ((GtkCombo *) (combo))->entry, (sstr) ? sstr : (const unsigned char *) "100%");
	}

	g_object_set_data (G_OBJECT (dlg), "blocked", NULL);
}

static void
sp_text_edit_dialog_text_changed (GtkTextBuffer *tb, GtkWidget *dlg)
{
	GtkWidget *textw, *preview, *apply, *def;
	SPText *text;
	GtkTextIter start, end;
	gchar *str;

	if (g_object_get_data (G_OBJECT (dlg), "blocked")) return;

	text = sp_ted_get_selected_text_item ();

	textw = g_object_get_data (G_OBJECT (dlg), "textw");
	preview = g_object_get_data (G_OBJECT (dlg), "preview");
	apply = g_object_get_data (G_OBJECT (dlg), "apply");
	def = g_object_get_data (G_OBJECT (dlg), "default");

	gtk_text_buffer_get_bounds (tb, &start, &end);
	str = gtk_text_buffer_get_text (tb, &start, &end, TRUE);
	if (str && *str) {
		sp_font_preview_set_phrase (SP_FONT_PREVIEW (preview), str);
	} else {
		sp_font_preview_set_phrase (SP_FONT_PREVIEW (preview), NULL);
	}
	if (str) g_free (str);

	if (text) {
		gtk_widget_set_sensitive (apply, TRUE);
	}
	gtk_widget_set_sensitive (def, TRUE);
}

static void
sp_text_edit_dialog_font_changed (SPFontSelector *fsel, NRFont *font, GtkWidget *dlg)
{
	GtkWidget *preview, *apply, *def;
	SPText *text;

	if (g_object_get_data (G_OBJECT (dlg), "blocked")) return;

	text = sp_ted_get_selected_text_item ();

	preview = g_object_get_data (G_OBJECT (dlg), "preview");
	apply = g_object_get_data (G_OBJECT (dlg), "apply");
	def = g_object_get_data (G_OBJECT (dlg), "default");

	sp_font_preview_set_font (SP_FONT_PREVIEW (preview), font);

	if (text) {
		gtk_widget_set_sensitive (apply, TRUE);
	}
	gtk_widget_set_sensitive (def, TRUE);
}

static void
sp_text_edit_dialog_any_toggled (GtkToggleButton *tb, GtkWidget *dlg)
{
	GtkWidget *apply, *def;
	SPText *text;

	if (g_object_get_data (G_OBJECT (dlg), "blocked")) return;

	text = sp_ted_get_selected_text_item ();

	apply = g_object_get_data (G_OBJECT (dlg), "apply");
	def = g_object_get_data (G_OBJECT (dlg), "default");

	if (text) {
		gtk_widget_set_sensitive (apply, TRUE);
	}
	gtk_widget_set_sensitive (def, TRUE);
}

static void
sp_text_edit_dialog_line_spacing_changed (GtkEditable *editable, GtkWidget *dlg)
{
	GtkWidget *apply, *def;
	SPText *text;

	if (g_object_get_data ((GObject *) dlg, "blocked")) return;

	text = sp_ted_get_selected_text_item ();

	apply = g_object_get_data (G_OBJECT (dlg), "apply");
	def = g_object_get_data (G_OBJECT (dlg), "default");

	if (text) {
		gtk_widget_set_sensitive (apply, TRUE);
	}
	gtk_widget_set_sensitive (def, TRUE);
}

static SPText *
sp_ted_get_selected_text_item (void)
{
	const GSList *item;
	
	if (!SP_ACTIVE_DESKTOP) return NULL;
	
	item = sp_selection_item_list(SP_DT_SELECTION (SP_ACTIVE_DESKTOP));
	for (; item != NULL; item = item->next) {
		if (SP_IS_TEXT(item->data)) return SP_TEXT (item->data);
	}
	
	return NULL;
}

static unsigned
sp_ted_get_selected_text_count (void)
{
	const GSList *item;
	unsigned items;

	if (!SP_ACTIVE_DESKTOP) return 0;

	items = 0;
	item = sp_selection_item_list(SP_DT_SELECTION (SP_ACTIVE_DESKTOP));
	for (; item != NULL; item = item->next) {
		if (SP_IS_TEXT(item->data)) ++items;
	}
	
	return items;
}
