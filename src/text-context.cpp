#define __SP_TEXT_CONTEXT_C__

/*
 * SPTextContext
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * Copyright (C) 1999-2002 Lauris Kaplinski
 * Copyright (C) 2001 Ximian, Inc.
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <math.h>

#include <ctype.h>
#include <libart_lgpl/art_affine.h>
#include <glib-object.h>
#include <gdk/gdkkeysyms.h>
#include <gtk/gtkmain.h>
#include <helper/sp-ctrlline.h>
#include <helper/sodipodi-ctrlrect.h>
#include <gtk/gtkimmulticontext.h>
#include "macros.h"
#include "sp-text.h"
#include "inkscape.h"
#include "document.h"
#include "style.h"
#include "selection.h"
#include "desktop.h"
#include "desktop-handles.h"
#include "desktop-affine.h"
#include "pixmaps/cursor-text.xpm"
#include "text-context.h"
#include "view.h"

static void sp_text_context_class_init (SPTextContextClass * klass);
static void sp_text_context_init (SPTextContext * text_context);

static void sp_text_context_setup (SPEventContext *ec);
static void sp_text_context_finish (SPEventContext *ec);
static gint sp_text_context_root_handler (SPEventContext * event_context, GdkEvent * event);
static gint sp_text_context_item_handler (SPEventContext * event_context, SPItem * item, GdkEvent * event);

static void sp_text_context_selection_changed (SPSelection *selection, SPTextContext *tc);
static void sp_text_context_selection_modified (SPSelection *selection, guint flags, SPTextContext *tc);

static void sp_text_context_update_cursor (SPTextContext *tc);
static gint sp_text_context_timeout (SPTextContext *tc);
static void sp_text_context_forget_text (SPTextContext *tc);

static gint sptc_focus_in (GtkWidget *widget, GdkEventFocus *event, SPTextContext *tc);
static gint sptc_focus_out (GtkWidget *widget, GdkEventFocus *event, SPTextContext *tc);
static void sptc_commit (GtkIMContext *imc, gchar *string, SPTextContext *tc);
static void sptc_preedit_changed (GtkIMContext *imc, SPTextContext *tc);
static void sp_text_context_preedit_reset (SPTextContext *tc);

static SPEventContextClass * parent_class;

GType
sp_text_context_get_type (void)
{
	static GType type = 0;
	if (!type) {
		GTypeInfo info = {
			sizeof (SPTextContextClass),
			NULL, NULL,
			(GClassInitFunc) sp_text_context_class_init,
			NULL, NULL,
			sizeof (SPTextContext),
			4,
			(GInstanceInitFunc) sp_text_context_init,
			NULL,	/* value_table */
		};
		type = g_type_register_static (SP_TYPE_EVENT_CONTEXT, "SPTextContext", &info, (GTypeFlags)0);
	}
	return type;
}

static void
sp_text_context_class_init (SPTextContextClass * klass)
{
	GObjectClass *object_class = (GObjectClass *) klass;
	SPEventContextClass *event_context_class = (SPEventContextClass *) klass;

	parent_class = (SPEventContextClass*)g_type_class_peek_parent (klass);

	event_context_class->setup = sp_text_context_setup;
	event_context_class->finish = sp_text_context_finish;
	event_context_class->root_handler = sp_text_context_root_handler;
	event_context_class->item_handler = sp_text_context_item_handler;
}

static void
sp_text_context_init (SPTextContext *tc)
{
	SPEventContext *event_context = SP_EVENT_CONTEXT (tc);

	event_context->cursor_shape = cursor_text_xpm;
	event_context->hot_x = 0;
	event_context->hot_y = 0;

	tc->imc = NULL;

	tc->text = NULL;
	tc->pdoc = NR::Point(0, 0);
	tc->ipos = 0;

	tc->unimode = FALSE;

	tc->cursor = NULL;
	tc->indicator = NULL;
	tc->timeout = 0;
	tc->show = FALSE;
	tc->phase = 0;
	tc->nascent_object = 0;

	tc->preedit_string = NULL;
}

static void
sp_text_context_setup (SPEventContext *ec)
{
	SPTextContext *tc = SP_TEXT_CONTEXT (ec);
	SPDesktop *desktop = ec->desktop;

	tc->cursor = sp_canvas_item_new (SP_DT_CONTROLS (desktop), SP_TYPE_CTRLLINE, NULL);
	sp_ctrlline_set_coords (SP_CTRLLINE (tc->cursor), 100, 0, 100, 100);
	sp_canvas_item_hide (tc->cursor);

	tc->indicator = sp_canvas_item_new (SP_DT_CONTROLS (desktop), SP_TYPE_CTRLRECT, NULL);
	sp_ctrlrect_set_area (SP_CTRLRECT (tc->indicator), 0, 0, 100, 100);
	sp_ctrlrect_set_color(SP_CTRLRECT (tc->indicator), 0x0000ff7f, FALSE, 0);
	sp_canvas_item_hide (tc->indicator);

	tc->timeout = gtk_timeout_add (250, (GtkFunction) sp_text_context_timeout, ec);

	tc->imc = gtk_im_multicontext_new();
	if (tc->imc) {
		GtkWidget *canvas = GTK_WIDGET (SP_DT_CANVAS (desktop));

		gtk_im_context_set_use_preedit (tc->imc, TRUE);
		gtk_im_context_set_client_window (tc->imc, canvas->window);

		g_signal_connect (G_OBJECT (canvas), "focus_in_event", G_CALLBACK (sptc_focus_in), tc);
		g_signal_connect (G_OBJECT (canvas), "focus_out_event", G_CALLBACK (sptc_focus_out), tc);
		g_signal_connect (G_OBJECT (tc->imc), "commit", G_CALLBACK (sptc_commit), tc);
		g_signal_connect (G_OBJECT (tc->imc), "preedit_changed", G_CALLBACK (sptc_preedit_changed), tc);

		if (GTK_WIDGET_HAS_FOCUS (canvas)) {
			sptc_focus_in (canvas, NULL, tc);
		}

	}

	if (((SPEventContextClass *) parent_class)->setup)
		((SPEventContextClass *) parent_class)->setup (ec);

	g_signal_connect (G_OBJECT (SP_DT_SELECTION (desktop)), "changed", G_CALLBACK (sp_text_context_selection_changed), tc);
	g_signal_connect (G_OBJECT (SP_DT_SELECTION (desktop)), "modified", G_CALLBACK (sp_text_context_selection_modified), tc);

	sp_text_context_selection_changed (SP_DT_SELECTION (desktop), tc);
}

static void
sp_text_context_finish (SPEventContext *ec)
{
	SPTextContext *tc = SP_TEXT_CONTEXT (ec);

	sp_text_context_forget_text (SP_TEXT_CONTEXT (ec));

	if (tc->imc) {
		g_object_unref (G_OBJECT (tc->imc));
		tc->imc = NULL;
	}
	
	if (tc->timeout) {
		gtk_timeout_remove (tc->timeout);
		tc->timeout = 0;
	}

	if (tc->cursor) {
		gtk_object_destroy (GTK_OBJECT (tc->cursor));
		tc->cursor = NULL;
	}

	if (tc->indicator) {
		gtk_object_destroy (GTK_OBJECT (tc->indicator));
		tc->indicator = NULL;
	}

	if (ec->desktop) {
  		sp_signal_disconnect_by_data (SP_DT_CANVAS (ec->desktop), tc);
  		sp_signal_disconnect_by_data (SP_DT_SELECTION (ec->desktop), ec);
	}
}

static gint
sp_text_context_item_handler (SPEventContext *ec, SPItem *item, GdkEvent *event)
{
	SPTextContext *tc = SP_TEXT_CONTEXT (ec);

	gint ret = FALSE;

	switch (event->type) {
	case GDK_BUTTON_PRESS:
		if (event->button.button == 1) {
			if (SP_IS_TEXT (item)) {
				sp_selection_set_item (SP_DT_SELECTION (ec->desktop), item);
				ret = TRUE;
			}
		}
		break;
	case GDK_MOTION_NOTIFY:
		if (SP_IS_TEXT (item)) {
			NRRect bbox;
			sp_item_bbox_desktop(item, &bbox);
			sp_canvas_item_show (tc->indicator);
			sp_ctrlrect_set_area (SP_CTRLRECT (tc->indicator), 
					      bbox.x0, bbox.y0, 
					      bbox.x1, bbox.y1);
			ret = TRUE;
		}
		break;
	default:
		break;
	}

	if (!ret) {
		if (((SPEventContextClass *) parent_class)->item_handler)
			ret = ((SPEventContextClass *) parent_class)->item_handler (ec, item, event);
	}

	return ret;
}

static void
sp_text_context_setup_text (SPTextContext *tc)
{
	SPEventContext *ec = SP_EVENT_CONTEXT (tc);

	/* Create <text> */
	SPRepr *rtext = sp_repr_new ("text");
	sp_repr_set_attr (rtext, "xml:space", "preserve"); // we preserve spaces in the text objects we create

	/* Set style */
	SPRepr *style = inkscape_get_repr (INKSCAPE, "tools.text");
	if (style) {
		SPCSSAttr *css;
		css = sp_repr_css_attr_inherited (style, "style");
		sp_repr_css_set (rtext, css, "style");
		sp_repr_css_attr_unref (css);
	}
	sp_repr_set_double_attribute (rtext, "x", tc->pdoc[NR::X]);
	sp_repr_set_double_attribute (rtext, "y", tc->pdoc[NR::Y]);

	/* Create <tspan> */
	SPRepr *rtspan = sp_repr_new ("tspan");
	sp_repr_add_child (rtext, rtspan, NULL);
	sp_repr_unref (rtspan);

	/* Create TEXT */
	SPRepr *rstring = sp_xml_document_createTextNode (sp_repr_document (rtext), "");
	sp_repr_add_child (rtspan, rstring, NULL);
	sp_repr_unref (rstring);
	sp_document_add_repr (SP_DT_DOCUMENT (ec->desktop), rtext);
	/* fixme: Is selection::changed really immediate? */
	sp_selection_set_repr (SP_DT_SELECTION (ec->desktop), rtext);
	sp_repr_unref (rtext);
	sp_document_done (SP_DT_DOCUMENT (ec->desktop));
}

static gint
sp_text_context_root_handler (SPEventContext *ec, GdkEvent *event)
{
	SPTextContext *tc = SP_TEXT_CONTEXT (ec);

	sp_canvas_item_hide (tc->indicator);

	switch (event->type) {
	case GDK_BUTTON_PRESS:
		if (event->button.button == 1) {
			/* Button 1, set X & Y & new item */
			sp_selection_empty (SP_DT_SELECTION (ec->desktop));
			NR::Point dtp = sp_desktop_w2d_xy_point (ec->desktop, NR::Point(event->button.x, event->button.y));
			tc->pdoc = sp_desktop_dt2root_xy_point (ec->desktop, dtp);
			/* Cursor */
			tc->show = TRUE;
			tc->phase = 1;
			tc->nascent_object = 1; // new object was just created
			sp_canvas_item_show (tc->cursor);
			//NR::Point dtp = sp_desktop_w2d_xy_point (ec->desktop, NR::Point(event->button.x, event->button.y));
			sp_ctrlline_set_coords (SP_CTRLLINE (tc->cursor), dtp, dtp + NR::Point(32, 0));
			/* Processed */
			return TRUE;
		}
		break;
	case GDK_KEY_PRESS:
		if (!sp_selection_is_empty (SP_DT_SELECTION (ec->desktop)) || (tc->nascent_object)) {
			// there is a selection, or a new object was just created

			tc->nascent_object = 0; // we don't need it anymore, because now sp_selection_is_empty != NULL

			if (tc->unimode || !tc->imc || !gtk_im_context_filter_keypress (tc->imc, (GdkEventKey*) event)) {
				//IM did not consumed the key, or we're in unimode

				if (!tc->text) sp_text_context_setup_text (tc);
				else sp_text_context_preedit_reset (tc);
				g_assert (tc->text != NULL);
				SPStyle *style = SP_OBJECT_STYLE (tc->text);

				if (MOD__CTRL_ONLY) {
					switch (event->key.keyval) {
					case GDK_space:
						/* No-break space */
						tc->ipos = sp_text_insert (SP_TEXT (tc->text), tc->ipos, "\302\240");
						sp_view_set_statusf_flash (SP_VIEW(ec->desktop), "No-break space");
						sp_document_done (SP_DT_DOCUMENT (ec->desktop));
						return TRUE;
					case GDK_U:
					case GDK_u:
						if (tc->unimode) {
							tc->unimode = FALSE;
							sp_view_clear_status (SP_VIEW(ec->desktop));
						} else {
							tc->unimode = TRUE;
							tc->unipos = 0;
							sp_view_set_statusf (SP_VIEW(ec->desktop), "Unicode: ");
						}
						if (tc->imc) {
							gtk_im_context_reset (tc->imc);
						}
						return TRUE;
					default:
						break;
					}
				} else {
					if (tc->unimode) {
						if (isxdigit ((guchar) event->key.keyval)) {
							tc->uni[tc->unipos] = event->key.keyval;
							sp_view_set_statusf (SP_VIEW(ec->desktop), 
                                                        "Unicode: %c%c%c%c", 
                                                        tc->uni[0], 
                                                        tc->unipos > 0 ? tc->uni[1] : ' ', 
                                                        tc->unipos > 1 ? tc->uni[2] : ' ', 
                                                        tc->unipos > 2 ? tc->uni[3] : ' ');
							if (tc->unipos == 3) {
								gchar u[7];
								guint uv, len;
								sscanf (tc->uni, "%x", &uv);
								len = g_unichar_to_utf8 (uv, u);
								u[len] = '\0';
								tc->unipos = 0;
								if (!g_unichar_isprint ((gunichar) uv)) {
									sp_view_set_statusf_error (SP_VIEW(ec->desktop), "Non-printable character"); // this may be due to bad input, so it goes to statusbar
								} else {
									tc->ipos = sp_text_insert (SP_TEXT (tc->text), tc->ipos, u);
									sp_document_done (SP_DT_DOCUMENT (ec->desktop));
								}
								return TRUE;
							} else {
								tc->unipos += 1;
								return TRUE;
							}
						} else { // non-hex-digit, canceling unimode
							tc->unimode = FALSE;
							gtk_im_context_reset (tc->imc);
							sp_view_clear_status (SP_VIEW(ec->desktop));
							return TRUE;
						}
					}

					/* Neither unimode nor IM consumed key */
					switch (event->key.keyval) {
					case GDK_Return:
						sp_text_insert_line (SP_TEXT (tc->text), tc->ipos);
						tc->ipos += 1;
						sp_document_done (SP_DT_DOCUMENT (ec->desktop));
						return TRUE;
					case GDK_BackSpace:
						tc->ipos = sp_text_delete (SP_TEXT (tc->text), MAX (tc->ipos - 1, 0), tc->ipos);
						sp_document_done (SP_DT_DOCUMENT (ec->desktop));
						return TRUE;
					case GDK_Delete:
						tc->ipos = sp_text_delete (SP_TEXT (tc->text), tc->ipos, MIN (tc->ipos + 1, sp_text_get_length (SP_TEXT (tc->text))));
						sp_document_done (SP_DT_DOCUMENT (ec->desktop));
						return TRUE;
					case GDK_Left:
						if (style->writing_mode.computed == SP_CSS_WRITING_MODE_TB) {
							tc->ipos = sp_text_down (SP_TEXT (tc->text), tc->ipos);
						} else {
							tc->ipos = MAX (tc->ipos - 1, 0);
						}
						sp_text_context_update_cursor (tc);
						return TRUE;
					case GDK_Right:
						if (style->writing_mode.computed == SP_CSS_WRITING_MODE_TB) {
							tc->ipos = sp_text_up (SP_TEXT (tc->text), tc->ipos);
						} else {
							tc->ipos = MIN (tc->ipos + 1, sp_text_get_length (SP_TEXT (tc->text)));
						}
						sp_text_context_update_cursor (tc);
						return TRUE;
					case GDK_Up:
						if (style->writing_mode.computed == SP_CSS_WRITING_MODE_TB) {
							tc->ipos = MAX (tc->ipos - 1, 0);
						} else {
							tc->ipos = sp_text_up (SP_TEXT (tc->text), tc->ipos);
						}
						sp_text_context_update_cursor (tc);
						return TRUE;
					case GDK_Down:
						if (style->writing_mode.computed == SP_CSS_WRITING_MODE_TB) {
							tc->ipos = MIN (tc->ipos + 1, sp_text_get_length (SP_TEXT (tc->text)));
						} else {
							tc->ipos = sp_text_down (SP_TEXT (tc->text), tc->ipos);
						}
						sp_text_context_update_cursor (tc);
						return TRUE;
					case GDK_Home:
						tc->ipos = sp_text_start_of_line (SP_TEXT (tc->text), tc->ipos);
						sp_text_context_update_cursor (tc);
						return TRUE;
					case GDK_End:
						tc->ipos = sp_text_end_of_line (SP_TEXT (tc->text), tc->ipos);
						sp_text_context_update_cursor (tc);
						return TRUE;
					case GDK_Escape:
						sp_selection_empty (SP_DT_SELECTION (ec->desktop));
						return TRUE;
					case GDK_less:
					case GDK_comma:
						if (MOD__ALT) { 
							if (MOD__SHIFT)
								sp_adjust_tspan_letterspacing_screen (SP_TEXT (tc->text), tc->ipos, ec->desktop, -10);
							else 
								sp_adjust_tspan_letterspacing_screen (SP_TEXT (tc->text), tc->ipos, ec->desktop, -1);
							sp_document_done (SP_DT_DOCUMENT (ec->desktop));
							sp_text_context_update_cursor (tc);
							return TRUE;
						}
						break;
					case GDK_greater:
					case GDK_period:
						if (MOD__ALT) { 
							if (MOD__SHIFT)
								sp_adjust_tspan_letterspacing_screen (SP_TEXT (tc->text), tc->ipos, ec->desktop, 10);
							else 
								sp_adjust_tspan_letterspacing_screen (SP_TEXT (tc->text), tc->ipos, ec->desktop, 1);
							sp_document_done (SP_DT_DOCUMENT (ec->desktop));
							sp_text_context_update_cursor (tc);
							return TRUE;
						}
						break;
					default:
						break;
					}
				}
			} else return TRUE; // return the "I took care of it" value if it was consumed by the IM
		} else { // do nothing if there's no object to type in - the key will be sent to parent context, 
			    // except up/down that are swallowed to prevent the zoom field from activation
			if ((event->key.keyval == GDK_Up || event->key.keyval == GDK_Down) && !MOD__CTRL_ONLY) 
				return TRUE;
		}
		break;
	case GDK_KEY_RELEASE:
		if (!tc->unimode && tc->imc && gtk_im_context_filter_keypress (tc->imc, (GdkEventKey*) event)) {
			return TRUE;
		}
		break;
	default:
		break;
	}

	// if nobody consumed it so far
	if (((SPEventContextClass *) parent_class)->root_handler) { // and there's a handler in parent context,
		return ((SPEventContextClass *) parent_class)->root_handler (ec, event); // send event to parent
	} else {
		return FALSE; // return "I did nothing" value so that global shortcuts can be activated
	}
}

static void
sp_text_context_selection_changed (SPSelection *selection, SPTextContext *tc)
{
	SPItem *item = sp_selection_item (selection);

	if (tc->text && (item != tc->text)) {
		sp_text_context_forget_text (tc);
	}
	tc->text = NULL;

	if (SP_IS_TEXT (item)) {
		tc->text = item;
		tc->ipos = sp_text_get_length (SP_TEXT (tc->text));
	} else {
		tc->text = NULL;
	}

	sp_text_context_update_cursor (tc);
}

static void
sp_text_context_selection_modified (SPSelection *selection, guint flags, SPTextContext *tc)
{
	sp_text_context_update_cursor (tc);
}

static void
sp_text_context_update_cursor (SPTextContext *tc)
{
	GdkRectangle im_cursor = { 0, 0, 1, 1 };

	if (tc->text) {
		NR::Point p0, p1;
		sp_text_get_cursor_coords (SP_TEXT (tc->text), tc->ipos, p0, p1);
		NR::Point d0 = sp_item_i2d_affine (SP_ITEM (tc->text)) * p0;
		NR::Point d1 = sp_item_i2d_affine (SP_ITEM (tc->text)) * p1;
		sp_canvas_item_show (tc->cursor);
		sp_ctrlline_set_coords (SP_CTRLLINE (tc->cursor), d0, d1);
		/* fixme: ... need another transformation to get canvas widget coordinate space? */
		im_cursor.x = (int) floor (d0[NR::X]);
		im_cursor.y = (int) floor (d0[NR::Y]);
		im_cursor.width = (int) floor (d1[NR::X]) - im_cursor.x;
		im_cursor.height = (int) floor (d1[NR::Y]) - im_cursor.y;

		tc->show = TRUE;
		tc->phase = 1;
	} else {
		
		sp_canvas_item_hide (tc->cursor);
		tc->show = FALSE;
	}

	if (tc->imc) {
		gtk_im_context_set_cursor_location (tc->imc, &im_cursor);
	}
}

static gint
sp_text_context_timeout (SPTextContext *tc)
{
	if (tc->show) {
		if (tc->phase) {
			tc->phase = 0;
			sp_canvas_item_hide (tc->cursor);
		} else {
			tc->phase = 1;
			sp_canvas_item_show (tc->cursor);
		}
	}

	return TRUE;
}

static void
sp_text_context_forget_text (SPTextContext *tc)
{
	if (! tc->text) return;
	if( tc->preedit_string ) sp_text_context_preedit_reset (tc);
	SPItem *ti = tc->text;
	/* We have to set it to zero,
	 * or selection changed signal messes everything up */
	tc->text = NULL;
	if (sp_text_is_empty (SP_TEXT (ti))) {
		sp_repr_unparent (SP_OBJECT_REPR (ti));
	}
}

gint
sptc_focus_in (GtkWidget *widget, GdkEventFocus *event, SPTextContext *tc)
{
	gtk_im_context_focus_in (tc->imc);
	return FALSE;
}

gint
sptc_focus_out (GtkWidget *widget, GdkEventFocus *event, SPTextContext *tc)
{
	gtk_im_context_focus_out (tc->imc);
	return FALSE;
}

void
sptc_commit (GtkIMContext *imc, gchar *string, SPTextContext *tc)
{
	if (!tc->text) sp_text_context_setup_text (tc);

	if (!tc->preedit_string ) sp_text_context_preedit_reset (tc);

	tc->ipos = sp_text_insert (SP_TEXT (tc->text), tc->ipos, string);

	sp_document_done (SP_OBJECT_DOCUMENT (tc->text));
}

void
sptc_preedit_changed (GtkIMContext *imc, SPTextContext *tc)
{
	gint cursor_pos;

	sp_text_context_preedit_reset (tc);

	gtk_im_context_get_preedit_string (tc->imc,
					   &tc->preedit_string, NULL,
					   &cursor_pos);
	if(tc->preedit_string != NULL) {
		sp_text_insert (SP_TEXT (tc->text), tc->ipos, tc->preedit_string);
	}
	sp_document_done (SP_OBJECT_DOCUMENT (tc->text));
}

static void
sp_text_context_preedit_reset (SPTextContext *tc)
{
       if( tc->preedit_string != NULL ) {
               sp_text_delete (SP_TEXT (tc->text), tc->ipos, tc->ipos + g_utf8_strlen(tc->preedit_string, -1));

               g_free(tc->preedit_string);
               tc->preedit_string = NULL;
       }
}
