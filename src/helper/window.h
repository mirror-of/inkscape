#ifndef __SP_WINDOW_H__
#define __SP_WINDOW_H__

/*
 * Generic window implementation
 *
 * Author:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * This code is in public domain
 */

#include <gtk/gtkwidget.h>

GtkWidget *sp_window_new (const unsigned char *title, unsigned int resizeable);


#endif
