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

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

GtkWidget *sp_window_new (const gchar *title, unsigned int resizeable);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif
