#define __TEST_WIDGETS_C__

#include <gtk/gtk.h>
#include "sp-color-selector.h"

static gboolean blocked = FALSE;
GtkWidget *s[3];

static void
sel_grabbed (SPColorSelector *sel, gpointer data)
{
	g_print ("grabbed\n");
}

static void
sel_dragged (SPColorSelector *sel, gpointer data)
{
	gint i;

	if (blocked) return;

	blocked = TRUE;

	for (i = 0; i < 3; i++) {
		if (sel != (SPColorSelector *) s[i]) sp_color_selector_set_any_rgba_float (SP_COLOR_SELECTOR (s[i]),
											   sp_color_selector_get_r (sel),
											   sp_color_selector_get_g (sel),
											   sp_color_selector_get_b (sel),
											   sp_color_selector_get_a (sel));
	}

	blocked = FALSE;

	g_print ("dragged\n");
}

static void
sel_released (SPColorSelector *sel, gpointer data)
{
	g_print ("released\n");
}

static void
sel_changed (SPColorSelector *sel, gpointer data)
{
	gint i;

	if (blocked) return;

	blocked = TRUE;

	for (i = 0; i < 3; i++) {
		if (sel != (SPColorSelector *) s[i]) sp_color_selector_set_any_rgba_float (SP_COLOR_SELECTOR (s[i]),
											   sp_color_selector_get_r (sel),
											   sp_color_selector_get_g (sel),
											   sp_color_selector_get_b (sel),
											   sp_color_selector_get_a (sel));
	}

	blocked = FALSE;

	g_print ("changed\n");
}

int main (int argc, char **argv)
{
	GtkWidget *w, *vb;
	gint i;

	gtk_init (&argc, &argv);

	w = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title (GTK_WINDOW (w), "Test Widgets");

	vb = gtk_vbox_new (FALSE, 4);
	gtk_widget_show (vb);
	gtk_container_add (GTK_CONTAINER (w), vb);

	for (i = 0; i < 3; i++) {
		s[i] = sp_color_selector_new ();
		gtk_widget_show (s[i]);
		gtk_box_pack_start (GTK_BOX (vb), s[i], TRUE, TRUE, 4);
		gtk_signal_connect (GTK_OBJECT (s[i]), "grabbed", GTK_SIGNAL_FUNC (sel_grabbed), NULL);
		gtk_signal_connect (GTK_OBJECT (s[i]), "dragged", GTK_SIGNAL_FUNC (sel_dragged), NULL);
		gtk_signal_connect (GTK_OBJECT (s[i]), "released", GTK_SIGNAL_FUNC (sel_released), NULL);
		gtk_signal_connect (GTK_OBJECT (s[i]), "changed", GTK_SIGNAL_FUNC (sel_changed), NULL);
	}

	sp_color_selector_set_mode (SP_COLOR_SELECTOR (s[1]), SP_COLOR_SELECTOR_MODE_HSV);
	sp_color_selector_set_mode (SP_COLOR_SELECTOR (s[2]), SP_COLOR_SELECTOR_MODE_CMYK);

	gtk_widget_show (w);

	gtk_main ();

	return 0;
}
