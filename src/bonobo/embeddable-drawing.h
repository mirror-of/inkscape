#ifndef SP_EMBEDDABLE_DRAWING_H
#define SP_EMBEDDABLE_DRAWING_H

#include <config.h>
#include <gnome.h>
#include <bonobo.h>
#include "embeddable-document.h"

#define SP_EMBEDDABLE_DRAWING_TYPE	(sp_embeddable_drawing_get_type ())
#define SP_EMBEDDABLE_DRAWING(o)  	(GTK_CHECK_CAST ((o), SP_EMBEDDABLE_DRAWING_TYPE, SPEmbeddableDrawing))
#define SP_EMBEDDABLE_DRAWING_CLASS(k)    (GTK_CHECK_CLASS_CAST((k), SP_EMBEDDABLE_DRAWING_TYPE, SPEmbeddableDrawingClass))
#define IS_SP_EMBEDDABLE_DRAWING(o)       (GTK_CHECK_TYPE ((o), SP_EMBEDDABLE_DRAWING_TYPE))
#define IS_SP_EMBEDDABLE_DRAWING_CLASS(k) (GTK_CHECK_CLASS_TYPE ((k), SP_EMBEDDABLE_DRAWING_TYPE))

/*
 * SPEmbeddableDrawing
 *
 */

typedef struct _SPEmbeddableDrawing SPEmbeddableDrawing;
typedef struct _SPEmbeddableDrawingClass SPEmbeddableDrawingClass;

struct _SPEmbeddableDrawing {
	BonoboCanvasComponent parent;
	SPEmbeddableDocument * edocument;
	SPDocument * spdocument;
	GnomeCanvasGroup * drawing;
};

struct _SPEmbeddableDrawingClass {
	BonoboCanvasComponentClass parent_class;
};

GtkType sp_embeddable_drawing_get_type (void);

/*
 * Constructor
 */

BonoboCanvasComponent * sp_embeddable_drawing_factory (BonoboEmbeddable * embeddable,
						       GnomeCanvas * canvas, gpointer data);

/*
 * Notify drawing, that underlying SPDocument has changed
 */

void sp_embeddable_drawing_new_doc (BonoboCanvasComponent * component, gpointer data);

#endif
