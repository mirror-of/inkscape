#ifndef __SP_SLIDESHOW_H__
#define __SP_SLIDESHOW_H__

/*
 * Slideshow/About window
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * Copyright (C) 1999-2003 authors
 * Copyright (C) 2000-2002 Ximian, Inc.
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <gtk/gtkwidget.h>

GtkWidget *sp_slideshow_new (const GSList *files);

#endif
