#define __SP_BUTTON_C__

/*
 * Generic button widget
 *
 * Author:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * Copyright (C) 2002 Lauris Kaplinski
 *
 * This code is in public domain
 */

#include <string.h>

#include <libnr/nr-macros.h>
#include <libnr/nr-rect.h>
#include <libnr/nr-matrix.h>
#include <libnr/nr-pixblock.h>
#include <libnr/nr-pixblock-pattern.h>
#include <libnr/nr-pixops.h>

#include <gdk/gdkkeys.h>
#include <gtk/gtkmain.h>
#include <gtk/gtkmenu.h>
#include <gtk/gtkmenuitem.h>

#include "helper/sp-marshal.h"

#include "icon.h"
#include "button.h"

#include "shortcuts.h"

enum {PRESSED, RELEASED, CLICKED, TOGGLED, LAST_SIGNAL};

static void sp_button_class_init (SPButtonClass *klass);
static void sp_button_init (SPButton *button);
static void sp_button_destroy (GtkObject *object);

static void sp_button_realize (GtkWidget *widget);
static void sp_button_unrealize (GtkWidget *widget);
static void sp_button_map (GtkWidget *widget);
static void sp_button_unmap (GtkWidget *widget);
static void sp_button_size_request (GtkWidget *widget, GtkRequisition *requisition);
static void sp_button_size_allocate (GtkWidget *widget, GtkAllocation *allocation);
static int sp_button_expose (GtkWidget *widget, GdkEventExpose *event);
static int sp_button_leave_notify (GtkWidget *widget, GdkEventCrossing *event);
static int sp_button_enter_notify (GtkWidget *widget, GdkEventCrossing *event);
static int sp_button_button_release (GtkWidget *widget, GdkEventButton *event);
static int sp_button_button_press (GtkWidget *widget, GdkEventButton *event);

static void sp_button_paint (SPButton *button, GdkRectangle *area);
static void sp_button_paint_arrow (NRRectL *iarea, int x0, int y0, int x1, int y1, unsigned char *px, unsigned int rs);

static void sp_button_action_set_active (SPAction *action, unsigned int active, void *data);
static void sp_button_action_set_sensitive (SPAction *action, unsigned int sensitive, void *data);
static void sp_button_action_set_shortcut (SPAction *action, unsigned int shortcut, void *data);

static void sp_button_set_composed_tooltip (SPButton *button, GtkWidget *widget, SPAction *action);

static GtkWidgetClass *parent_class;
static guint button_signals[LAST_SIGNAL];
SPActionEventVector button_event_vector = {
	{NULL},
	 NULL,
	 sp_button_action_set_active,
	 sp_button_action_set_sensitive,
	 sp_button_action_set_shortcut
};

GtkType
sp_button_get_type (void)
{
	static GtkType type = 0;
	if (!type) {
		GtkTypeInfo info = {
			"SPButton",
			sizeof (SPButton),
			sizeof (SPButtonClass),
			(GtkClassInitFunc) sp_button_class_init,
			(GtkObjectInitFunc) sp_button_init,
			NULL, NULL, NULL
		};
		type = gtk_type_unique (GTK_TYPE_WIDGET, &info);
	}
	return type;
}

static void
sp_button_class_init (SPButtonClass *klass)
{
	GtkObjectClass *object_class;
	GtkWidgetClass *widget_class;

	object_class = (GtkObjectClass *) klass;
	widget_class = (GtkWidgetClass *) klass;

	parent_class = g_type_class_peek_parent (klass);

	object_class->destroy = sp_button_destroy;

	widget_class->realize = sp_button_realize;
	widget_class->unrealize = sp_button_unrealize;
	widget_class->map = sp_button_map;
	widget_class->unmap = sp_button_unmap;
	widget_class->size_request = sp_button_size_request;
	widget_class->size_allocate = sp_button_size_allocate;
	widget_class->expose_event = sp_button_expose;
	widget_class->button_press_event = sp_button_button_press;
	widget_class->button_release_event = sp_button_button_release;
	widget_class->enter_notify_event = sp_button_enter_notify;
	widget_class->leave_notify_event = sp_button_leave_notify;

	button_signals[PRESSED] = g_signal_new ("pressed",
						G_OBJECT_CLASS_TYPE (object_class),
						G_SIGNAL_RUN_FIRST,
						G_STRUCT_OFFSET (SPButtonClass, pressed),
						NULL, NULL,
						sp_marshal_VOID__VOID,
						G_TYPE_NONE, 0);
	button_signals[RELEASED] = g_signal_new ("released",
						G_OBJECT_CLASS_TYPE (object_class),
						G_SIGNAL_RUN_FIRST,
						G_STRUCT_OFFSET (SPButtonClass, released),
						NULL, NULL,
						sp_marshal_VOID__VOID,
						G_TYPE_NONE, 0);
	button_signals[CLICKED] = g_signal_new ("clicked",
						G_OBJECT_CLASS_TYPE (object_class),
						G_SIGNAL_RUN_FIRST,
						G_STRUCT_OFFSET (SPButtonClass, clicked),
						NULL, NULL,
						sp_marshal_VOID__VOID,
						G_TYPE_NONE, 0);
	button_signals[TOGGLED] = g_signal_new ("toggled",
						G_OBJECT_CLASS_TYPE (object_class),
						G_SIGNAL_RUN_FIRST,
						G_STRUCT_OFFSET (SPButtonClass, toggled),
						NULL, NULL,
						sp_marshal_VOID__VOID,
						G_TYPE_NONE, 0);
}

static void
sp_button_init (SPButton *button)
{
	GTK_WIDGET_SET_FLAGS (button, GTK_NO_WINDOW);
}

static void
sp_button_destroy (GtkObject *object)
{
	SPButton *button;

	button = SP_BUTTON (object);

	if (button->timeout) {
		gtk_timeout_remove (button->timeout);
		button->timeout = 0;
	}

	if (button->menu) {
		gtk_widget_destroy (button->menu);
		button->menu = NULL;
	}

	if (button->options) {
		int i;
		for (i = 0; i < button->noptions; i++) {
			nr_active_object_remove_listener_by_data ((NRActiveObject *) button->options[i].action, button);
			nr_free (button->options[i].px);
			nr_object_unref ((NRObject *) button->options[i].action);
		}
		g_free (button->options);
		button->options = NULL;
	}

	if (button->tooltips) {
		g_object_unref (G_OBJECT (button->tooltips));
		button->tooltips = NULL;
	}

	((GtkObjectClass *) (parent_class))->destroy (object);
}

static void
sp_button_realize (GtkWidget *widget)
{
	SPButton *button;
	GdkWindowAttr attributes;
	gint attributes_mask;

	button = SP_BUTTON (widget);

	GTK_WIDGET_SET_FLAGS (widget, GTK_REALIZED);

	attributes.window_type = GDK_WINDOW_CHILD;
	attributes.x = widget->allocation.x;
	attributes.y = widget->allocation.y;
	attributes.width = widget->allocation.width;
	attributes.height = widget->allocation.height;
	attributes.wclass = GDK_INPUT_ONLY;
	attributes.event_mask = gtk_widget_get_events (widget);
	attributes.event_mask |= (GDK_BUTTON_PRESS_MASK |
				  GDK_BUTTON_RELEASE_MASK |
				  GDK_ENTER_NOTIFY_MASK |
				  GDK_LEAVE_NOTIFY_MASK);
	attributes_mask = GDK_WA_X | GDK_WA_Y;

	widget->window = gtk_widget_get_parent_window (widget);
	g_object_ref (widget->window);

	button->event_window = gdk_window_new (widget->window,
					       &attributes,
					       attributes_mask);
	gdk_window_set_user_data (button->event_window, button);

	widget->style = gtk_style_attach (widget->style, widget->window);
}

static void
sp_button_unrealize (GtkWidget *widget)
{
	SPButton *button;

	button = SP_BUTTON (widget);

	if (button->event_window) {
		gdk_window_set_user_data (button->event_window, NULL);
		gdk_window_destroy (button->event_window);
		button->event_window = NULL;
	}

	GTK_WIDGET_CLASS (parent_class)->unrealize (widget);
}

static void
sp_button_map (GtkWidget *widget)
{
	SPButton *button;

	button = SP_BUTTON (widget);

	GTK_WIDGET_CLASS (parent_class)->map (widget);

	if (button->event_window) {
		gdk_window_show (button->event_window);
	}
}

static void
sp_button_unmap (GtkWidget *widget)
{
	SPButton *button;

	button = SP_BUTTON (widget);

	if (button->event_window) {
		gdk_window_hide (button->event_window);
	}

	GTK_WIDGET_CLASS (parent_class)->unmap (widget);
}

static void
sp_button_size_request (GtkWidget *widget, GtkRequisition *requisition)
{
	SPButton *button;

	button = SP_BUTTON (widget);

	requisition->width = button->size + 2 * MAX (2, widget->style->xthickness);
	requisition->height = button->size + 2 * MAX (2, widget->style->ythickness);
}

static void
sp_button_size_allocate (GtkWidget *widget, GtkAllocation *allocation)
{
	SPButton *button;

	button = SP_BUTTON (widget);

	widget->allocation = *allocation;

	if (GTK_WIDGET_REALIZED (widget)) {
		gdk_window_move_resize (button->event_window,
					widget->allocation.x,
					widget->allocation.y,
					widget->allocation.width,
					widget->allocation.height);
	}

	if (GTK_WIDGET_DRAWABLE (widget)) {
		gtk_widget_queue_draw (widget);
	}
}

static int
sp_button_expose (GtkWidget *widget, GdkEventExpose *event)
{
	if (GTK_WIDGET_DRAWABLE (widget)) {
		sp_button_paint (SP_BUTTON (widget), &event->area);
	}

	return TRUE;
}

static void
sp_button_update_state (SPButton *button)
{
	GtkStateType state;

	if (button->pressed) {
		state = GTK_STATE_ACTIVE;
	} else if (button->inside) {
		state = GTK_STATE_PRELIGHT;
	} else {
		state = GTK_STATE_NORMAL;
	}

	if (state != GTK_WIDGET (button)->state) {
		gtk_widget_set_state (GTK_WIDGET (button), state);
	}
}

static void
sp_button_menu_activate (GObject *object, SPButton *button)
{
	button->option = GPOINTER_TO_INT (g_object_get_data (object, "option"));
	gtk_widget_queue_draw (GTK_WIDGET (button));
}

static void
sp_button_menu_selection_done (GObject *object, SPButton *button)
{
	SPAction *action;
	action = button->options[button->option].action;
	/* Emulate button released */
	switch (button->type) {
	case SP_BUTTON_TYPE_NORMAL:
		button->pressed = 0;
		button->down = 0;
		gdk_pointer_ungrab (GDK_CURRENT_TIME);
		sp_button_update_state (button);
		g_signal_emit (button, button_signals[RELEASED], 0);
		g_signal_emit (button, button_signals[CLICKED], 0);
		sp_action_perform (action);
		break;
	case SP_BUTTON_TYPE_TOGGLE:
		button->pressed = 0;
		button->down = !button->initial;
		gdk_pointer_ungrab (GDK_CURRENT_TIME);
		sp_button_update_state (button);
		g_signal_emit (button, button_signals[RELEASED], 0);
		g_signal_emit (button, button_signals[TOGGLED], 0);
		sp_action_set_active (action, button->down);
		if (button->down) sp_action_perform (action);
		break;
	default:
		break;
	}
	if (button->tooltips) {
		sp_button_set_composed_tooltip (button, (GtkWidget *) button, action);
	}
}

static int
sp_button_timeout (gpointer data)
{
	SPButton *button;
	int i;

	button = SP_BUTTON (data);

	button->timeout = 0;

	if (button->menu) {
		gtk_widget_destroy (button->menu);
		button->menu = NULL;
	}
	button->menu = gtk_menu_new ();
	gtk_widget_show (button->menu);
	for (i = 0; i < button->noptions; i++) {
		GtkWidget *icon, *mi;
		icon = sp_icon_new_from_data (button->size, button->options[i].px);
		gtk_widget_show (icon);
		mi = gtk_menu_item_new ();
		gtk_widget_show (mi);
		gtk_container_add (GTK_CONTAINER (mi), icon);
		gtk_menu_append (GTK_MENU (button->menu), mi);
		g_object_set_data (G_OBJECT (mi), "option", GINT_TO_POINTER (i));
		if (button->tooltips) {
			sp_button_set_composed_tooltip (button, mi, button->options[i].action);
		}
		g_signal_connect (G_OBJECT (mi), "activate", G_CALLBACK (sp_button_menu_activate), button);
	}
	g_signal_connect (G_OBJECT (button->menu), "selection_done", G_CALLBACK (sp_button_menu_selection_done), button);

	gtk_menu_popup (GTK_MENU (button->menu), NULL, NULL, NULL, NULL, 1, GDK_CURRENT_TIME);

	return FALSE;
}

static int
sp_button_button_press (GtkWidget *widget, GdkEventButton *event)
{
	SPButton *button;

	button = SP_BUTTON (widget);

	if (event->button == 1) {
		if (button->timeout) {
			gtk_timeout_remove (button->timeout);
			button->timeout = 0;
		}
		if (button->noptions > 1) {
			button->timeout = gtk_timeout_add (200, sp_button_timeout, button);
		}
		switch (button->type) {
		case SP_BUTTON_TYPE_NORMAL:
			button->grabbed = 1;
			button->pressed = 1;
			button->down = 1;
			sp_button_update_state (button);
			g_signal_emit (button, button_signals[PRESSED], 0);
			break;
		case SP_BUTTON_TYPE_TOGGLE:
			button->grabbed = 1;
			button->pressed = 1;
			button->initial = button->down;
			button->down = 1;
			sp_button_update_state (button);
			g_signal_emit (button, button_signals[PRESSED], 0);
			break;
		default:
			break;
		}
	}

	return TRUE;
}

static int
sp_button_button_release (GtkWidget *widget, GdkEventButton *event)
{
	SPButton *button;
	SPAction *action;

	button = SP_BUTTON (widget);
	action = button->options[button->option].action;

	if (event->button == 1) {
		if (button->timeout) {
			gtk_timeout_remove (button->timeout);
			button->timeout = 0;
		}
		switch (button->type) {
		case SP_BUTTON_TYPE_NORMAL:
			button->pressed = 0;
			button->down = 0;
			button->grabbed = 0;
			sp_button_update_state (button);
			g_signal_emit (button, button_signals[RELEASED], 0);
			g_signal_emit (button, button_signals[CLICKED], 0);
			sp_action_perform (action);
			break;
		case SP_BUTTON_TYPE_TOGGLE:
			button->pressed = 0;
			button->down = !button->initial;
			button->grabbed = 0;
			sp_button_update_state (button);
			g_signal_emit (button, button_signals[RELEASED], 0);
			g_signal_emit (button, button_signals[TOGGLED], 0);
			sp_action_set_active (action, button->down);
			if (button->down) sp_action_perform (action);
			break;
		default:
			break;
		}
	}

	return TRUE;
}

static int
sp_button_enter_notify (GtkWidget *widget, GdkEventCrossing *event)
{
	SPButton *button;

	button = SP_BUTTON (widget);

	button->inside = 1;
	sp_button_update_state (button);

	return FALSE;
}

static int
sp_button_leave_notify (GtkWidget *widget, GdkEventCrossing *event)
{
	SPButton *button;

	button = SP_BUTTON (widget);

	button->inside = 0;
	sp_button_update_state (button);

	return FALSE;
}

GtkWidget *
sp_button_new (unsigned int size, unsigned int type, SPAction *action, GtkTooltips *tooltips)
{
	SPButton *button;

	button = g_object_new (SP_TYPE_BUTTON, NULL);

	button->noptions = 1;
	button->type = type;
	button->size = CLAMP (size, 4, 64);
	button->options = g_new (SPBChoiceData, 1);
	button->tooltips = tooltips;
	if (tooltips) g_object_ref ((GObject *) tooltips);
	sp_button_add_option (button, 0, action);

	return (GtkWidget *) button;
}

GtkWidget *
sp_button_menu_new (unsigned int size, unsigned int type, unsigned int noptions, GtkTooltips *tooltips)
{
	SPButton *button;

	button = g_object_new (SP_TYPE_BUTTON, NULL);

	button->noptions = CLAMP (noptions, 1, 16);
	button->type = type;
	button->size = CLAMP (size, 4, 64);
	button->options = g_new0 (SPBChoiceData, button->noptions);
	if (tooltips) {
		g_object_ref (G_OBJECT (tooltips));
		button->tooltips = tooltips;
	}

	return (GtkWidget *) button;
}

void
sp_button_toggle_set_down (SPButton *button, unsigned int down, unsigned int signal)
{
	SPAction *action;

	down = (down != 0);

	action = button->options[button->option].action;

	if (button->down != down) {
		button->down = down;
		if (signal) {
			g_signal_emit (button, button_signals[TOGGLED], 0);
			sp_action_set_active (action, down);
		}
		gtk_widget_queue_draw (GTK_WIDGET (button));
	}
}

void
sp_button_add_option (SPButton *button, unsigned int option, SPAction *action)
{
	button->options[option].px = sp_icon_image_load_gtk ((GtkWidget *) button, action->image, button->size, button->size);
	button->options[option].action = (SPAction *) nr_object_ref ((NRObject *) action);
	nr_active_object_add_listener ((NRActiveObject *) action,
				       (NRObjectEventVector *) &button_event_vector,
				       sizeof (SPActionEventVector),
				       button);

	if ((option == button->option) && button->tooltips) {
		sp_button_set_composed_tooltip (button, (GtkWidget *) button, action);
	}
}

unsigned int
sp_button_get_option (SPButton *button)
{
	return button->option;
}

void
sp_button_set_option (SPButton *button, unsigned int option)
{
	if (option != button->option) {
		SPAction *action;
		button->option = option;
		action = button->options[button->option].action;
		gtk_widget_queue_draw (GTK_WIDGET (button));
		if (button->tooltips) {
			sp_button_set_composed_tooltip (button, (GtkWidget *) button, action);
		}
	}

}

static void
sp_button_action_set_active (SPAction *action, unsigned int active, void *data)
{
	SPButton *button;
	button = (SPButton *) data;
	if (button->type == SP_BUTTON_TYPE_TOGGLE) {
		int aidx;
		for (aidx = 0; aidx < button->noptions; aidx++) {
			if (action == button->options[aidx].action) break;
		}
		if ((active) && (aidx != button->option)) sp_button_set_option (button, aidx);
		if ((aidx == button->option) && (active != button->down)) sp_button_toggle_set_down (button, active, FALSE);
	}
}

static void
sp_button_action_set_sensitive (SPAction *action, unsigned int sensitive, void *data)
{
	SPButton *button;
	SPAction *ba;
	button = (SPButton *) data;
	ba = button->options[button->option].action;
	if (action == ba) {
		gtk_widget_set_sensitive ((GtkWidget *) button, sensitive);
	}
}

static void
sp_button_action_set_shortcut (SPAction *action, unsigned int shortcut, void *data)
{
	SPButton *button;
	SPAction *ba;
	button = (SPButton *) data;
	ba = button->options[button->option].action;
	if (button->tooltips && (action == ba)) {
		sp_button_set_composed_tooltip (button, (GtkWidget *) button, action);
	}
}

static void
sp_button_set_composed_tooltip (SPButton *button, GtkWidget *widget, SPAction *action)
{
	if (action->shortcut) {
		unsigned char c[16384];
		unsigned char *as, *cs, *ss;
		as = (action->shortcut & SP_SHORTCUT_ALT_MASK) ? "Alt+" : "";
		cs = (action->shortcut & SP_SHORTCUT_CONTROL_MASK) ? "Ctrl+" : "";
		ss = (action->shortcut & SP_SHORTCUT_SHIFT_MASK) ? "Shift+" : "";
		g_snprintf (c, 16384, "%s [%s%s%s%s]", action->tip, as, cs, ss, gdk_keyval_name (action->shortcut & 0xffffff));
		gtk_tooltips_set_tip (button->tooltips, widget, c, NULL);
	} else {
		gtk_tooltips_set_tip (button->tooltips, widget, action->tip, NULL);
	}
}

static void
sp_button_paint (SPButton *button, GdkRectangle *area)
{
	GtkWidget *widget;
	NRRectL parea, iarea;
	int padx, pady;
	int x0, y0, x1, y1, x, y;

	widget = GTK_WIDGET (button);

	parea.x0 = widget->allocation.x + MAX (2, widget->style->xthickness);
	parea.y0 = widget->allocation.y + MAX (2, widget->style->ythickness);
	parea.x1 = widget->allocation.x + widget->allocation.width - MAX (2, widget->style->xthickness);
	parea.y1 = widget->allocation.y + widget->allocation.height - MAX (2, widget->style->ythickness);

	padx = (parea.x1 - parea.x0 - button->size) / 2;
	pady = (parea.y1 - parea.y0 - button->size) / 2;

	iarea.x0 = parea.x0 + padx;
	iarea.y0 = parea.y0 + pady;
	iarea.x1 = iarea.x0 + button->size;
	iarea.y1 = iarea.y0 + button->size;

	x0 = MAX (area->x, iarea.x0);
	y0 = MAX (area->y, iarea.y0);
	x1 = MIN (area->x + area->width, iarea.x1);
	y1 = MIN (area->y + area->height, iarea.y1);

	gtk_paint_box (widget->style, widget->window, widget->state,
		       (button->down) ? GTK_SHADOW_IN : GTK_SHADOW_OUT,
		       area, widget, "button",
		       widget->allocation.x,
		       widget->allocation.y,
		       widget->allocation.width,
		       widget->allocation.height);

	for (y = y0; y < y1; y += 64) {
		for (x = x0; x < x1; x += 64) {
			NRPixBlock bpb;
			unsigned char *px;
			int xe, ye;
			xe = MIN (x + 64, x1);
			ye = MIN (y + 64, y1);
			nr_pixblock_setup_fast (&bpb, NR_PIXBLOCK_MODE_R8G8B8, x, y, xe, ye, FALSE);

			px = button->options[button->option].px;
			if (px) {
				GdkColor *color;
				unsigned int br, bg, bb;
				int xx, yy;

				/* fixme: We support only plain-color themes */
				/* fixme: But who needs other ones anyways? (Lauris) */
				color = &widget->style->bg[widget->state];
				br = (color->red & 0xff00) >> 8;
				bg = (color->green & 0xff00) >> 8;
				bb = (color->blue & 0xff00) >> 8;

				if (GTK_WIDGET_SENSITIVE (button) && GTK_WIDGET_PARENT_SENSITIVE (button)) {
					for (yy = y; yy < ye; yy++) {
						const unsigned char *s;
						unsigned char *d;
						d = NR_PIXBLOCK_PX (&bpb) + (yy - y) * bpb.rs;
						s = px + 4 * (yy - iarea.y0) * button->size + 4 * (x - iarea.x0);
						for (xx = x; xx < xe; xx++) {
							d[0] = NR_COMPOSEN11 (s[0], s[3], br);
							d[1] = NR_COMPOSEN11 (s[1], s[3], bg);
							d[2] = NR_COMPOSEN11 (s[2], s[3], bb);
							s += 4;
							d += 3;
						}
					}
				} else {
					for (yy = y; yy < ye; yy++) {
						const unsigned char *s;
						unsigned char *d;
						unsigned int r, g, b;
						d = NR_PIXBLOCK_PX (&bpb) + (yy - y) * bpb.rs;
						s = px + 4 * (yy - iarea.y0) * button->size + 4 * (x - iarea.x0);
						for (xx = x; xx < xe; xx++) {
							r = br + ((int) s[0] - (int) br) / 2;
							g = bg + ((int) s[1] - (int) bg) / 2;
							b = bb + ((int) s[2] - (int) bb) / 2;
							d[0] = NR_COMPOSEN11 (r, s[3], br);
							d[1] = NR_COMPOSEN11 (g, s[3], bg);
							d[2] = NR_COMPOSEN11 (b, s[3], bb);
							s += 4;
							d += 3;
						}
					}
				}
			} else {
				nr_pixblock_render_gray_noise (&bpb, NULL);
			}

			if (button->noptions > 1) {
				/* Render arrow */
				sp_button_paint_arrow (&iarea, x, y, xe, ye, NR_PIXBLOCK_PX (&bpb), bpb.rs);
			}

			gdk_draw_rgb_image (widget->window, widget->style->black_gc,
					    x, y,
					    xe - x, ye - y,
					    GDK_RGB_DITHER_MAX,
					    NR_PIXBLOCK_PX (&bpb), bpb.rs);

			nr_pixblock_release (&bpb);
		}
	}
}

#define ARROW_SIZE 7

static void
sp_button_paint_arrow (NRRectL *parea, int x0, int y0, int x1, int y1, unsigned char *px, unsigned int rs)
{
	int sx, sy, width, height, x, y;
	unsigned char *d;

	/* Upper left corner */
	sx = parea->x1 - x0 - 1 - ARROW_SIZE;
	sy = parea->y1 - y0 - 1 - ARROW_SIZE;
	width = x1 - x0;
	height = y1 - y0;

	/* Draw top row */
	y = 0;
	if (((sy + y) >= 0) && ((sy + y) < height)) {
		d = px + sy * rs + 3 * sx;
		for (x = 0; x < ARROW_SIZE; x++) {
			if (((sx + x) >= 0) && ((sx + x) < width)) {
				/* Shade */
				d[0] >>= 1;
				d[1] >>= 1;
				d[2] >>= 1;
				d += 3;
			}
		}
	}
	/* Draw the rest */
	for (y = 1; y < ARROW_SIZE; y++) {
		if (((sy + y) >= 0) && ((sy + y) < height)) {
			int xa, xb, xc, xd;
			xa = y / 2;
			xb = xa + 1;
			xd = ARROW_SIZE - xa;
			xc = xd - 1;
			d = px + (sy + y) * rs + 3 * (sx + xa);
			/* Draw XA */
			if (((sx + xa) >= 0) && ((sx + xa) < width)) {
				/* Shade */
				d[0] >>= 1;
				d[1] >>= 1;
				d[2] >>= 1;
				d += 3;
			}
#if 0
			/* Draw XB-XC */
			if (xc > xb) {
				for (x = xb; x < xc; x++) {
					if (((sx + x) >= 0) && ((sx + x) < width)) {
						d[0] = 0x70;
						d[1] = 0x70;
						d[2] = 0x70;
						d += 3;
					}
				}
			}
#else
			d = px + (sy + y) * rs + 3 * (sx + xc);
#endif
			/* Draw XC */
			if ((xc > xa) && ((sx + xc) >= 0) && ((sx + xc) < width)) {
				/* Lighten */
				d[0] = 255 - ((255 - d[0]) >> 2);
				d[1] = 255 - ((255 - d[1]) >> 2);
				d[2] = 255 - ((255 - d[2]) >> 2);
				d += 3;
			}
		}
	}
}

GtkWidget *
sp_button_new_from_data (unsigned int size,
			 unsigned int type,
			 const unsigned char *name,
			 const unsigned char *tip,
			 GtkTooltips *tooltips)
{
	GtkWidget *button;
	SPAction *action;
	action = (SPAction *) nr_object_new (SP_TYPE_ACTION);
	sp_action_setup (action, name, name, tip, name);
	button = sp_button_new (size, type, action, tooltips);
	nr_object_unref ((NRObject *) action);
	return button;
}


