#define SP_EMBEDDABLE_DRAWING_C

#include "../sp-item.h"
#include "../document.h"
#include "canvas-translator.h"
#include "embeddable-drawing.h"

static void sp_embeddable_drawing_class_init (GtkObjectClass * klass);
static void sp_embeddable_drawing_init (GtkObject * object);

GtkType
sp_embeddable_drawing_get_type (void)
{
	static GtkType type = 0;
	if (!type) {
		GtkTypeInfo info = {
			"SPEmbeddableDrawing",
			sizeof (SPEmbeddableDrawing),
			sizeof (SPEmbeddableDrawingClass),
			(GtkClassInitFunc) sp_embeddable_drawing_class_init,
			(GtkObjectInitFunc) sp_embeddable_drawing_init,
			NULL, NULL, NULL
		};
		type = bonobo_x_type_unique (BONOBO_CANVAS_COMPONENT_TYPE, NULL, NULL, 0, &info);
	}
	return type;
}

static void
sp_embeddable_drawing_class_init (GtkObjectClass * klass)
{
}

static void
sp_embeddable_drawing_init (GtkObject * object)
{
	SPEmbeddableDrawing * drawing;

	drawing = SP_EMBEDDABLE_DRAWING (object);

	drawing->edocument = NULL;
	drawing->spdocument = NULL;
	drawing->drawing = NULL;
}

/*
 * fixme: Write extensible desktop class
 */

BonoboCanvasComponent *
sp_embeddable_drawing_factory (BonoboEmbeddable * embeddable,
	GnomeCanvas * canvas, gpointer data)
{
	SPEmbeddableDrawing * drawing;
	SPEmbeddableDocument * document;

	document = SP_EMBEDDABLE_DOCUMENT (embeddable);

	drawing = gtk_type_new (SP_EMBEDDABLE_DRAWING_TYPE);

	drawing->edocument = document;
	drawing->spdocument = document->document;
	gtk_object_ref (GTK_OBJECT (drawing->spdocument));

	drawing->drawing = (GnomeCanvasGroup *) gnome_canvas_item_new (gnome_canvas_root (canvas),
		SP_TYPE_CANVAS_TRANSLATOR,
		NULL);

#if 0
	gnome_canvas_item_new (drawing->drawing,
		GNOME_TYPE_CANVAS_RECT,
		"x1", 10.0,
		"y1", 10.0,
		"x2", 100.0,
		"y2", 100.0,
		"fill_color", "red", NULL);
#endif
	bonobo_canvas_component_construct (BONOBO_CANVAS_COMPONENT (drawing),
		GNOME_CANVAS_ITEM (drawing->drawing));

#if 0
	drawing->drawing = (GnomeCanvasGroup *) gnome_canvas_item_new (drawing->drawing,
		SP_TYPE_CANVAS_TRANSLATOR,
		NULL);
	art_affine_scale (affine, 0.1, 0.1);
	affine[4] = 100.0;
	affine[5] = 100.0;
	gnome_canvas_item_affine_absolute (GNOME_CANVAS_ITEM (drawing->drawing), affine);
#endif

	/* fixme: */
#if 0
	sp_item_show (SP_ITEM (sp_document_root (drawing->spdocument)), NULL, drawing->drawing);
#endif

	return BONOBO_CANVAS_COMPONENT (drawing);
}

/* fixme: Give better names to documents :) */

void
sp_embeddable_drawing_new_doc (BonoboCanvasComponent * component, gpointer data)
{
	SPEmbeddableDrawing * drawing;

	drawing = SP_EMBEDDABLE_DRAWING (component);

	/* fixme: */
	sp_item_hide (SP_ITEM (sp_document_root (drawing->spdocument)), NULL);

	gtk_object_ref (GTK_OBJECT (drawing->edocument->document));
	gtk_object_unref (GTK_OBJECT (drawing->spdocument));

	drawing->spdocument = drawing->edocument->document;

	/* fixme: */
#if 0
	sp_item_show (SP_ITEM (sp_document_root (drawing->spdocument)), NULL, drawing->drawing);
#endif
}

