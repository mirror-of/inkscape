#include <stdlib.h>
#include <glib.h>
#include "sp-module-doc.h"

/* Local Prototypes */
static void sp_module_doc_class_init  (SPModuleDocClass    * klass);
static void sp_module_doc_init        (SPModuleDoc         * object);
static void sp_module_doc_finalize     (GObject           * object);

/* Global variables */
static GObjectClass * base_parent_class;

GType
sp_module_doc_get_type (void)
{
	static GType type = 0;
	if (!type) {
		GTypeInfo info = {
			sizeof (SPModuleDocClass),
			NULL, NULL,
			(GClassInitFunc) sp_module_doc_class_init,
			NULL, NULL,
			sizeof (SPModuleDoc),
			16,
			(GInstanceInitFunc) sp_module_doc_init,
		};
		type = g_type_register_static (G_TYPE_OBJECT, "SPModuleDoc", &info, 0);
	}
	return type;
}

static void sp_module_doc_class_init (SPModuleDocClass * klass) {
	GObjectClass * g_object_class;

	g_object_class = (GObjectClass *)klass;

	base_parent_class = g_type_class_peek_parent (klass);

	g_object_class->finalize = sp_module_doc_finalize;
}

static void sp_module_doc_init (SPModuleDoc * object) {
	object->filename = NULL;
	object->doc      = NULL;
	return;
}

static void sp_module_doc_finalize (GObject * object) {
	SPModuleDoc * pdoc;

	pdoc = (SPModuleDoc *)object;

	if (pdoc->filename != NULL) {
		g_free(pdoc->filename);
	}

	if (pdoc->doc != NULL) {
		sp_document_unref(pdoc->doc);
	}

	G_OBJECT_CLASS (base_parent_class)->finalize (object);
}

SPModuleDoc * sp_module_doc_new (void) {
	SPModuleDoc * retval;

	retval = (SPModuleDoc *) g_object_new (SP_TYPE_MODULE_DOC, NULL);

	return retval;
}

gchar * sp_module_doc_set_filename (SPModuleDoc * doc,
                                    const gchar * filename) {
	g_return_val_if_fail(SP_IS_MODULE_DOC(doc), NULL);
	g_return_val_if_fail(filename != NULL, NULL);

	doc->filename = g_strdup(filename);

	return doc->filename;
}

gchar * sp_module_doc_get_filename (SPModuleDoc * doc) {
	g_return_val_if_fail(SP_IS_MODULE_DOC(doc), NULL);

	return doc->filename;
}

SPDocument * sp_module_doc_set_document (SPModuleDoc * doc,
                                         SPDocument * sp_doc) {
	g_return_val_if_fail(SP_IS_MODULE_DOC(doc), NULL);
	g_return_val_if_fail(sp_doc != NULL, NULL);

	sp_document_ref(sp_doc);
	doc->doc = sp_doc;
	
	return doc->doc;
}

SPDocument * sp_module_doc_get_document (SPModuleDoc * doc) {
	g_return_val_if_fail(SP_IS_MODULE_DOC(doc), NULL);
	return doc->doc;
}

