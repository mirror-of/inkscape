#define SP_SVG_DOC_FACTORY_C

#include <config.h>
#include "embeddable-document.h"
#include "svg-doc-factory.h"

static BonoboObject *
sp_svg_factory (BonoboGenericFactory *factory,
		const char           *component_id,
		gpointer              closure)
{
	if (!strcmp (component_id, "OAFIID:GNOME_Sodipodi_Embeddable"))
		return sp_embeddable_document_new ();

	else if (!strcmp (component_id, "OAFIID:GNOME_Sodipodi_CanvasItem"))
		g_warning ("SodiPodi's canvas item code is dysfunctional");
	
	else
		g_warning ("Sodipodi cannot activate unknown id '%s'",
			   component_id);

	return NULL;
}

void
sp_svg_doc_factory_init (void)
{
	static BonoboGenericFactory *doc_factory = NULL;

	doc_factory = bonobo_generic_factory_new_multi (
		"OAFIID:GNOME_Sodipodi_ComponentFactory",
		sp_svg_factory, NULL);

	if (doc_factory == NULL)
		g_error (_("Could not create sodipodi-svg-doc factory"));
}

