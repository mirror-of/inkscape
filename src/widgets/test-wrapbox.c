
#include <gtk/gtk.h>
#include "sp-hwrap-box.h"
#include "sp-vwrap-box.h"

int main (int argc, char **argv)
{
	GtkWidget *window;
	GtkWidget *vbox;
	GtkWidget *wrapbox;
	GtkWidget *button;
	gint i;
	
	gtk_init (&argc, &argv);

	window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	vbox = gtk_vbox_new(FALSE, 5);

	wrapbox = sp_vwrap_box_new (FALSE);
/*  	sp_wrap_box_set_justify (SP_WRAP_BOX(wrapbox), GTK_JUSTIFY_BOTTOM); */
	for (i = 0; i < 30; i++)
	{
		button = gtk_button_new_from_stock (GTK_STOCK_OPEN);
		sp_wrap_box_pack (SP_WRAP_BOX(wrapbox),
				  button,
				  FALSE, TRUE, FALSE, TRUE);
	}
	gtk_box_pack_start_defaults (GTK_BOX(vbox), wrapbox);

	wrapbox = sp_hwrap_box_new (FALSE);
/*  	sp_wrap_box_set_justify (SP_WRAP_BOX(wrapbox), GTK_JUSTIFY_BOTTOM); */
	for (i = 0; i < 30; i++)
	{
		button = gtk_button_new_from_stock (GTK_STOCK_OPEN);
		sp_wrap_box_pack (SP_WRAP_BOX(wrapbox),
				  button,
				  FALSE, TRUE, FALSE, TRUE);
	}
	gtk_box_pack_start_defaults (GTK_BOX(vbox), wrapbox);

	gtk_container_add (GTK_CONTAINER(window), vbox);
	gtk_widget_show_all (window);
	gtk_main ();

	return 0;
}
