#ifndef __SP_GRADIENT_IMAGE_H__
#define __SP_GRADIENT_IMAGE_H__

/*
 * A simple gradient preview
 *
 * Author:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * Copyright (C) 2001-2002 Lauris Kaplinski
 * Copyright (C) 2001 Ximian, Inc.
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <gtk/gtkwidget.h>
#include "../sp-gradient.h"

typedef struct _SPGradientImage SPGradientImage;
typedef struct _SPGradientImageClass SPGradientImageClass;

#define SP_TYPE_GRADIENT_IMAGE (sp_gradient_image_get_type ())
#define SP_GRADIENT_IMAGE(o) (GTK_CHECK_CAST ((o), SP_TYPE_GRADIENT_IMAGE, SPGradientImage))
#define SP_GRADIENT_IMAGE_CLASS(k) (GTK_CHECK_CLASS_CAST ((k), SP_TYPE_GRADIENT_IMAGE, SPGradientImageClass))
#define SP_IS_GRADIENT_IMAGE(o) (GTK_CHECK_TYPE ((o), SP_TYPE_GRADIENT_IMAGE))
#define SP_IS_GRADIENT_IMAGE_CLASS(k) (GTK_CHECK_CLASS_TYPE ((k), SP_TYPE_GRADIENT_IMAGE))

struct _SPGradientImage {
	GtkWidget widget;
	SPGradient *gradient;
	guchar *px;
};

struct _SPGradientImageClass {
	GtkWidgetClass parent_class;
};

GtkType sp_gradient_image_get_type (void);

GtkWidget *sp_gradient_image_new (SPGradient *gradient);
void sp_gradient_image_set_gradient (SPGradientImage *gi, SPGradient *gr);

#endif
