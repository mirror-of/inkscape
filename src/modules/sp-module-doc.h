
#ifndef __SODIPODI_MODULE_DOC_H__
#define __SODIPODI_MODULE_DOC_H__

#include "document.h"
#include "sp-module.h"

typedef struct _SPModuleDoc      SPModuleDoc;
typedef struct _SPModuleDocClass SPModuleDocClass;

struct _SPModuleDoc {
	GObject    object;
	gchar *      filename;
	SPDocument * doc;
};

struct _SPModuleDocClass {
	GObjectClass parent_class;
};

#define SP_TYPE_MODULE_DOC (sp_module_doc_get_type())
#define SP_MODULE_DOC(o)  (G_TYPE_CHECK_INSTANCE_CAST ((o), SP_TYPE_MODULE_DOC, SPModuleDoc))
#define SP_IS_MODULE_DOC(o) (G_TYPE_CHECK_INSTANCE_TYPE ((o), SP_TYPE_MODULE_DOC))

GType           sp_module_doc_get_type     (void);
SPModuleDoc *   sp_module_doc_new          (void);
gchar *         sp_module_doc_set_filename (SPModuleDoc * doc,
                                            const gchar * filename);
gchar *         sp_module_doc_get_filename (SPModuleDoc * doc);
SPDocument *    sp_module_doc_set_document (SPModuleDoc * doc,
                                            SPDocument * sp_doc);
SPDocument *    sp_module_doc_get_document (SPModuleDoc * doc);

#endif /* __SODIPODI_MODULE_DOC_H__ */
