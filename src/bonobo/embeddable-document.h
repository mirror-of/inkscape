#ifndef SP_EMBEDDABLE_DOCUMENT_H
#define SP_EMBEDDABLE_DOCUMENT_H

#include <config.h>
#include <gnome.h>
#include <bonobo.h>
#include "../forward.h"

#define SP_EMBEDDABLE_DOCUMENT_TYPE	(sp_embeddable_document_get_type ())
#define SP_EMBEDDABLE_DOCUMENT(o)  	(GTK_CHECK_CAST ((o), SP_EMBEDDABLE_DOCUMENT_TYPE, SPEmbeddableDocument))
#define SP_EMBEDDABLE_DOCUMENT_CLASS(k)    (GTK_CHECK_CLASS_CAST((k), SP_EMBEDDABLE_DOCUMENT_TYPE, SPEmbeddableDocumentClass))
#define IS_SP_EMBEDDABLE_DOCUMENT(o)       (GTK_CHECK_TYPE ((o), SP_EMBEDDABLE_DOCUMENT_TYPE))
#define IS_SP_EMBEDDABLE_DOCUMENT_CLASS(k) (GTK_CHECK_CLASS_TYPE ((k), SP_EMBEDDABLE_DOCUMENT_TYPE))

/*
 * SPEmbeddableDocument
 *
 */

typedef struct _SPEmbeddableDocument SPEmbeddableDocument;
typedef struct _SPEmbeddableDocumentClass SPEmbeddableDocumentClass;

struct _SPEmbeddableDocument {
	BonoboEmbeddable embeddable;
	SPDocument * document;
};

struct _SPEmbeddableDocumentClass {
	BonoboEmbeddableClass parent_class;
};

GtkType sp_embeddable_document_get_type (void);

/*
 * Constructor
 */

BonoboObject * sp_embeddable_document_new (void);

#endif
