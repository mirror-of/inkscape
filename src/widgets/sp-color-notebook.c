#define __SP_COLOR_NOTEBOOK_C__

/*
 * A block of 3 color sliders plus spinbuttons
 *
 * Author:
 *	 Lauris Kaplinski <lauris@kaplinski.com>
 *
 * Copyright (C) 2001-2002 Lauris Kaplinski
 *
 * This code is in public domain
 */

#undef SPCS_PREVIEW

#include <config.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <gtk/gtksignal.h>
#include <gtk/gtkhbox.h>
#include <gtk/gtklabel.h>
#include <gtk/gtktable.h>
#include <gtk/gtkspinbutton.h>
#include <gtk/gtknotebook.h>
#include <gtk/gtkcolorsel.h>
#include <gtk/gtkmenu.h>
#include <gtk/gtkmenuitem.h>
#include <gtk/gtkcheckmenuitem.h>
#include <gtk/gtkoptionmenu.h>
#include "../color.h"
#include "../helper/sp-intl.h"
#include "../dialogs/dialog-events.h"
#include "sp-color-preview.h"
#include "sp-color-notebook.h"

#include "sp-color-scales.h"
#include "sp-color-gtkselector.h"
#include "gtk/gtkarrow.h"

enum {
	GRABBED,
	DRAGGED,
	RELEASED,
	CHANGED,
	LAST_SIGNAL
};

struct _SPColorNotebookTracker {
	const gchar* name;
	const gchar* className;
	GType type;
	guint submode;
	gboolean enabledFull;
	gboolean enabledBrief;
	SPColorNotebook *backPointer;
};

static void sp_color_notebook_class_init (SPColorNotebookClass *klass);
static void sp_color_notebook_init (SPColorNotebook *slider);
static void sp_color_notebook_destroy (GtkObject *object);

static void sp_color_notebook_show_all (GtkWidget *widget);
static void sp_color_notebook_hide_all (GtkWidget *widget);

static void sp_color_notebook_rgba_entry_changed (GtkEntry *entry, SPColorNotebook *colorbook);
static void sp_color_notebook_entry_changed (SPColorSelector *csel, SPColorNotebook *colorbook);
static void sp_color_notebook_entry_dragged (SPColorSelector *csel, SPColorNotebook *colorbook);
static void sp_color_notebook_entry_modified (SPColorSelector *csel, SPColorNotebook *colorbook);

static GtkWidget *sp_color_notebook_add_page (SPColorNotebook *colorbook, GType page_type, guint submode);
static GtkWidget *sp_color_notebook_get_page (SPColorNotebook *colorbook, GType page_type, guint submode);
static void sp_color_notebook_remove_page (SPColorNotebook *colorbook, GType page_type, guint submode);

static SPColorSelector* sp_color_notebook_get_current_selector(SPColorNotebook* csel);

static SPColorSelectorClass *parent_class;
static guint csel_signals[LAST_SIGNAL] = {0};

#define XPAD 4
#define YPAD 1

GtkType
sp_color_notebook_get_type (void)
{
	static GtkType type = 0;
	if (!type) {
		GtkTypeInfo info = {
			"SPColorNotebook",
			sizeof (SPColorNotebook),
			sizeof (SPColorNotebookClass),
			(GtkClassInitFunc) sp_color_notebook_class_init,
			(GtkObjectInitFunc) sp_color_notebook_init,
			NULL, NULL, NULL
		};
		type = gtk_type_unique (SP_TYPE_COLOR_SELECTOR, &info);
	}
	return type;
}

static void
sp_color_notebook_class_init (SPColorNotebookClass *klass)
{
	GtkObjectClass *object_class;
	GtkWidgetClass *widget_class;
	SPColorSelectorClass *selector_class;

	object_class = (GtkObjectClass *) klass;
	widget_class = (GtkWidgetClass *) klass;
	selector_class = SP_COLOR_SELECTOR_CLASS (klass);

	parent_class = SP_COLOR_SELECTOR_CLASS (g_type_class_peek_parent (klass));


	csel_signals[GRABBED] =	 gtk_signal_lookup ("grabbed", SP_TYPE_COLOR_SELECTOR);
	csel_signals[DRAGGED] =	 gtk_signal_lookup ("dragged", SP_TYPE_COLOR_SELECTOR);
	csel_signals[RELEASED] = gtk_signal_lookup ("released", SP_TYPE_COLOR_SELECTOR);
	csel_signals[CHANGED] =	 gtk_signal_lookup ("changed", SP_TYPE_COLOR_SELECTOR);

	selector_class->set_color_alpha = sp_color_notebook_set_color_alpha;
	selector_class->get_color_alpha = NULL; //sp_color_notebook_get_color_alpha;

	object_class->destroy = sp_color_notebook_destroy;

	widget_class->show_all = sp_color_notebook_show_all;
	widget_class->hide_all = sp_color_notebook_hide_all;
}

static void
sp_color_notebook_switch_page(GtkNotebook *notebook,
							  GtkNotebookPage *page,
							  guint page_num,
							  SPColorNotebook *colorbook)
{
	if ( colorbook )
	{
		SPColorSelector* csel;
		SPColorSelector* parent_csel;
		GtkWidget* widget;

		parent_csel = SP_COLOR_SELECTOR (colorbook);
		if ( gtk_notebook_get_current_page (GTK_NOTEBOOK (colorbook->book)) >= 0 )
		{
			csel = sp_color_notebook_get_current_selector (colorbook);
			sp_color_selector_get_color_alpha(csel, &csel->color, &csel->alpha);
		}
		widget = gtk_notebook_get_nth_page (GTK_NOTEBOOK (colorbook->book), page_num);
		if ( widget && SP_IS_COLOR_SELECTOR (widget) )
		{
			csel = SP_COLOR_SELECTOR (widget);
			sp_color_selector_set_color_alpha (csel, &parent_csel->color, parent_csel->alpha);
		}
	}
}

static gint sp_color_notebook_menu_handler( GtkWidget *widget, GdkEvent *event )
{
	if (event->type == GDK_BUTTON_PRESS)
	{
	  GdkEventButton *bevent = (GdkEventButton *) event;
	  SPColorNotebook *colorbook = SP_COLOR_NOTEBOOK (widget);
		gtk_menu_popup (GTK_MENU(colorbook->popup), NULL, NULL, NULL, NULL,
						bevent->button, bevent->time);
		/* Tell calling code that we have handled this event; the buck
		 * stops here. */
		return TRUE;
	}

	/* Tell calling code that we have not handled this event; pass it on. */
	return FALSE;
}

static void sp_color_notebook_menuitem_response (GtkMenuItem *menuitem, gpointer user_data)
{
	gboolean active = FALSE;

	active = gtk_check_menu_item_get_active (GTK_CHECK_MENU_ITEM (menuitem));
	SPColorNotebookTracker *entry = INK_REINTERPRET_CAST( SPColorNotebookTracker*, user_data );
	if ( entry )
	{
		if ( active )
		{
			sp_color_notebook_add_page (entry->backPointer, entry->type, entry->submode);
		}
		else
		{
			sp_color_notebook_remove_page (entry->backPointer, entry->type, entry->submode);
		}
	}
}

static void
sp_color_notebook_init (SPColorNotebook *colorbook)
{
	GtkWidget* table = 0;
	guint row = 0;
	guint i = 0;
	guint j = 0;
	GType *selector_types = 0;
	guint	selector_type_count = 0;

	/* tempory hardcoding to get types loaded */
	SP_TYPE_COLOR_SCALES;
	SP_TYPE_COLOR_GTKSELECTOR;

	colorbook->updating = FALSE;
	colorbook->btn = 0;
	colorbook->popup = 0;
	colorbook->trackerList = g_ptr_array_new ();

	colorbook->book = gtk_notebook_new ();
	gtk_widget_show (colorbook->book);

	colorbook->id = g_signal_connect(GTK_OBJECT (colorbook->book), "switch-page",
								GTK_SIGNAL_FUNC (sp_color_notebook_switch_page), colorbook);

	selector_types = g_type_children (SP_TYPE_COLOR_SELECTOR, &selector_type_count);

	for ( i = 0; i < selector_type_count; i++ )
	{
		if (!g_type_is_a (selector_types[i], SP_TYPE_COLOR_NOTEBOOK))
		{
			guint howmany = 1;
			gpointer klass = gtk_type_class (selector_types[i]);
			if ( klass && SP_IS_COLOR_SELECTOR_CLASS (klass) )
			{
				SPColorSelectorClass *ck = SP_COLOR_SELECTOR_CLASS (klass);
				howmany = MAX (1, ck->submode_count);
				for ( j = 0; j < howmany; j++ )
				{
					SPColorNotebookTracker *entry = INK_REINTERPRET_CAST (SPColorNotebookTracker*, malloc(sizeof(SPColorNotebookTracker)));
					if ( entry )
					{
						memset( entry, 0, sizeof(SPColorNotebookTracker) );
						entry->name = ck->name[j];
						entry->type = selector_types[i];
						entry->submode = j;
						entry->enabledFull = TRUE;
						entry->enabledBrief = TRUE;
						entry->backPointer = colorbook;

						g_ptr_array_add (colorbook->trackerList, entry);
					}
				}
			}
		}
	}

	for ( i = 0; i < colorbook->trackerList->len; i++ )
	{
		SPColorNotebookTracker *entry = INK_REINTERPRET_CAST (SPColorNotebookTracker*, g_ptr_array_index (colorbook->trackerList, i));
		if ( entry )
		{
			sp_color_notebook_add_page (colorbook, entry->type, entry->submode);
		}
	}

	table = gtk_table_new (2, 3, FALSE);
	gtk_widget_show (table);

	gtk_box_pack_start (GTK_BOX (colorbook), table, TRUE, TRUE, 0);


	gtk_table_attach (GTK_TABLE (table), colorbook->book, 0, 2, row, row + 1, GTK_FILL, GTK_FILL, XPAD, YPAD);
	row++;

	/* Create RGBA entry and color preview */
	colorbook->rgbal = gtk_label_new ("RGBA:");
/*	   gtk_misc_set_alignment (GTK_MISC (colorbook->l[i]), 1.0, 0.5); */
	gtk_widget_show (colorbook->rgbal);
	gtk_table_attach (GTK_TABLE (table), colorbook->rgbal, 0, 1, row, row + 1, GTK_FILL, GTK_FILL, XPAD, YPAD);
	colorbook->rgbae = gtk_entry_new ();
	sp_dialog_defocus_on_enter (colorbook->rgbae);
	gtk_entry_set_max_length (GTK_ENTRY (colorbook->rgbae), 16);
	gtk_entry_set_width_chars (GTK_ENTRY (colorbook->rgbae), 10);
	gtk_widget_show (colorbook->rgbae);
	gtk_table_attach (GTK_TABLE (table), colorbook->rgbae, 1, 2, row, row + 1, GTK_FILL, GTK_FILL, XPAD, YPAD);

	{
		gboolean found = FALSE;
		GtkWidget* arrow;
		GtkMenu* menu;
		GtkWidget* item;

		colorbook->popup = gtk_menu_new();
		menu = GTK_MENU (colorbook->popup);

		for ( i = 0; i < colorbook->trackerList->len; i++ )
		{
			SPColorNotebookTracker *entry = INK_REINTERPRET_CAST (SPColorNotebookTracker*, g_ptr_array_index (colorbook->trackerList, i));
			if ( entry )
			{
				item = gtk_check_menu_item_new_with_label (entry->name);
				gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (item), entry->enabledFull);
				gtk_widget_show (item);
				gtk_menu_append (menu, item);

				g_signal_connect (G_OBJECT (item), "activate",
								  G_CALLBACK (sp_color_notebook_menuitem_response),
								  INK_REINTERPRET_CAST(gpointer, entry));
				found = TRUE;
			}
		}

		colorbook->btn = gtk_button_new ();
		gtk_widget_show (colorbook->btn);

		arrow = gtk_arrow_new(GTK_ARROW_RIGHT, GTK_SHADOW_ETCHED_OUT);
		gtk_widget_show (arrow);

		gtk_container_add (GTK_CONTAINER (colorbook->btn), arrow);

		gtk_table_attach (GTK_TABLE (table), colorbook->btn, 2, 3, row, row + 1, GTK_FILL, GTK_FILL, XPAD, YPAD);

		gtk_signal_connect_object(GTK_OBJECT(colorbook->btn), "event", GTK_SIGNAL_FUNC (sp_color_notebook_menu_handler), GTK_OBJECT(colorbook));
		if ( !found )
		{
			gtk_widget_set_sensitive (colorbook->btn, FALSE);
		}
	}

#ifdef SPCS_PREVIEW
	colorbook->p = sp_color_preview_new (0xffffffff);
	gtk_widget_show (colorbook->p);
	gtk_table_attach (GTK_TABLE (table), colorbook->p, 2, 3, row, row + 1, GTK_FILL, GTK_FILL, XPAD, YPAD);
#endif

	gtk_signal_connect (GTK_OBJECT (colorbook->rgbae), "changed", GTK_SIGNAL_FUNC (sp_color_notebook_rgba_entry_changed), colorbook);
}

static void
sp_color_notebook_destroy (GtkObject *object)
{
	SPColorNotebook *colorbook;

	colorbook = SP_COLOR_NOTEBOOK (object);
	if ( colorbook && colorbook->trackerList )
	{
		g_ptr_array_free (colorbook->trackerList, TRUE);
		colorbook->trackerList = 0;
	}

	if ( colorbook && colorbook->id )
	{
		if ( colorbook->book )
		{
			g_signal_handler_disconnect (colorbook->book, colorbook->id);
			colorbook->id = 0;
		}
	}

	if (((GtkObjectClass *) (parent_class))->destroy)
		(* ((GtkObjectClass *) (parent_class))->destroy) (object);
}

static void
sp_color_notebook_show_all (GtkWidget *widget)
{
	gtk_widget_show (widget);
}

static void
sp_color_notebook_hide_all (GtkWidget *widget)
{
	gtk_widget_hide (widget);
}

GtkWidget *
sp_color_notebook_new (void)
{
	SPColorNotebook *colorbook;
	SPColor color;

	colorbook = (SPColorNotebook*)gtk_type_new (SP_TYPE_COLOR_NOTEBOOK);

	sp_color_set_rgb_rgba32 (&color, 0);
	sp_color_notebook_set_color_alpha (SP_COLOR_SELECTOR(colorbook), &color, 1.0);

	return GTK_WIDGET (colorbook);
}



static SPColorSelector* sp_color_notebook_get_current_selector(SPColorNotebook* colorbook)
{
	SPColorSelector* csel = NULL;
	guint current_page;

	if ( colorbook && SP_IS_COLOR_NOTEBOOK (colorbook) )
	{
		current_page = gtk_notebook_get_current_page (GTK_NOTEBOOK (colorbook->book));
		if ( current_page >= 0 )
		{
			GtkWidget* widget = gtk_notebook_get_nth_page (GTK_NOTEBOOK (colorbook->book), current_page);
			if ( SP_IS_COLOR_SELECTOR (widget) )
			{
				csel = SP_COLOR_SELECTOR (widget);
			}
		}
	}

	return csel;
}

void sp_color_notebook_set_color_alpha (SPColorSelector *csel, const SPColor *color, gfloat alpha)
{
	SPColorNotebook* colorbook;
	SPColorSelector* cselPage;

	g_return_if_fail (csel != NULL);
	g_return_if_fail (SP_IS_COLOR_NOTEBOOK (csel));
	g_return_if_fail (color != NULL);

	if ( sp_color_is_close (&csel->color, color, 1e-4)
		 && (fabs ((alpha) - (csel->alpha)) < 1e-4) )
	{
		return;
	}

	sp_color_copy (&csel->color, color);
	csel->alpha = alpha;

	colorbook = SP_COLOR_NOTEBOOK (csel);
	cselPage = sp_color_notebook_get_current_selector(colorbook);
	if ( cselPage )
	{
		sp_color_selector_set_color_alpha (cselPage, color, alpha);
	}

	if ( !colorbook->updatingrgba )
	{
		gchar s[32];
		guint32 rgba;

		/* Update RGBA entry */
		rgba = sp_color_get_rgba32_falpha (&csel->color, csel->alpha);

		g_snprintf (s, 32, "%08x", rgba);
		gtk_entry_set_text (GTK_ENTRY (colorbook->rgbae), s);
		gtk_signal_emit (GTK_OBJECT (csel), csel_signals[colorbook->dragging ? DRAGGED:CHANGED]);
	}
}

void sp_color_notebook_get_color_alpha (SPColorSelector *csel, SPColor *color, gfloat *alpha)
{
	g_return_if_fail (csel != NULL);
	g_return_if_fail (SP_IS_COLOR_NOTEBOOK (csel));
	g_return_if_fail (color != NULL);

	sp_color_copy (color, &csel->color);
	if (alpha)
		*alpha = csel->alpha;
}

static void
sp_color_notebook_rgba_entry_changed (GtkEntry *entry, SPColorNotebook *colorbook)
{
	const gchar *t;
	gchar *e;
	SPColor color;
	guint rgba;

	if (colorbook->updating) return;
	if (colorbook->updatingrgba) return;

	t = gtk_entry_get_text (entry);

	if (t) {
		rgba = strtoul (t, &e, 16);
		if (e && e != t) {
			if (strlen (t) < 5) {
				/* treat as rgba instead of rrggbbaa */
				rgba = ((rgba << 16) & 0xf0000000) |
					((rgba << 12) & 0xff00000) |
					((rgba << 8) & 0xff000) |
					((rgba << 4) & 0xff0) |
					(rgba & 0xf);
			}
			colorbook->updatingrgba = TRUE;
			sp_color_set_rgb_rgba32 (&color, rgba);
			sp_color_notebook_set_color_alpha (SP_COLOR_SELECTOR (colorbook), &color, SP_RGBA32_A_F (rgba));
			colorbook->updatingrgba = FALSE;
		}
	}
}

static void
sp_color_notebook_entry_changed (SPColorSelector *csel, SPColorNotebook *colorbook)
{
	gboolean oldState;

	oldState = colorbook->dragging;

	colorbook->dragging = FALSE;
	sp_color_notebook_entry_modified (csel, colorbook);

	colorbook->dragging = oldState;
}

static void
sp_color_notebook_entry_dragged (SPColorSelector *csel, SPColorNotebook *colorbook)
{
	gboolean oldState;

	oldState = colorbook->dragging;

	colorbook->dragging = TRUE;
	sp_color_notebook_entry_modified (csel, colorbook);

	colorbook->dragging = oldState;
}

static void sp_color_notebook_entry_modified (SPColorSelector *csel, SPColorNotebook *colorbook)
{
	SPColorSelector *parent_csel;
	g_return_if_fail (colorbook != NULL);
	g_return_if_fail (SP_IS_COLOR_NOTEBOOK (colorbook));
	g_return_if_fail (csel != NULL);
	g_return_if_fail (SP_IS_COLOR_SELECTOR (csel));

	parent_csel = SP_COLOR_SELECTOR (colorbook);
	if ( !sp_color_is_close (&parent_csel->color, &csel->color, 1e-4)
		 || (fabs ((csel->alpha) - (parent_csel->alpha)) >= 1e-4) )
	{
		sp_color_selector_set_color_alpha (parent_csel, &csel->color, csel->alpha);
	}
}

static GtkWidget *sp_color_notebook_add_page (SPColorNotebook *colorbook, GType page_type, guint submode)
{
	GtkWidget *page;

	page = sp_color_selector_new (page_type, SP_COLORSPACE_TYPE_NONE);
	if ( page )
	{
		GtkWidget* tab_label = 0;
		SPColorSelector* csel;

		csel = SP_COLOR_SELECTOR (page);
		if ( submode > 0 )
		{
			sp_color_selector_set_submode (csel, submode);
		}
		gtk_widget_show (page);
		tab_label = gtk_label_new (SP_COLOR_SELECTOR_GET_CLASS (csel)->name[sp_color_selector_get_submode (csel)]);
		gtk_notebook_append_page( GTK_NOTEBOOK (colorbook->book), page, tab_label );
		gtk_signal_connect (GTK_OBJECT (page), "changed", GTK_SIGNAL_FUNC (sp_color_notebook_entry_changed), colorbook);
		gtk_signal_connect (GTK_OBJECT (page), "dragged", GTK_SIGNAL_FUNC (sp_color_notebook_entry_dragged), colorbook);
	}

	return page;
}

static GtkWidget *sp_color_notebook_get_page (SPColorNotebook *colorbook, GType page_type, guint submode)
{
	gint count = 0;
	gint i = 0;
	GtkWidget* page = 0;

//	  count = gtk_notebook_get_n_pages (colorbook->book);
	count = 200;
	for ( i = 0; i < count && !page; i++ )
	{
		page = gtk_notebook_get_nth_page (GTK_NOTEBOOK (colorbook->book), i);
		if ( page )
		{
			SPColorSelector* csel;
			guint pagemode;
			csel = SP_COLOR_SELECTOR (page);
			pagemode = sp_color_selector_get_submode (csel);
			if ( G_TYPE_FROM_INSTANCE (page) == page_type
				 && pagemode == submode )
			{
				// found it.
				break;
			}
			else
			{
				page = 0;
			}
		}
		else
		{
			break;
		}
	}
	return page;
}

static void sp_color_notebook_remove_page (SPColorNotebook *colorbook, GType page_type, guint submode)
{
	GtkWidget *page = 0;

	page = sp_color_notebook_get_page (colorbook, page_type, submode);
	if ( page )
	{
		gint where = gtk_notebook_page_num (GTK_NOTEBOOK (colorbook->book), page);
		if ( where >= 0 )
		{
			if ( gtk_notebook_get_current_page (GTK_NOTEBOOK (colorbook->book)) == where )
			{
				SPColorSelector *csel = SP_COLOR_SELECTOR(colorbook);
				sp_color_selector_get_color_alpha(csel, &csel->color, &csel->alpha);
			}
			gtk_notebook_remove_page (GTK_NOTEBOOK (colorbook->book), where);
		}
	}
}
