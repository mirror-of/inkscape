#ifndef __SP_MODULE_H__
#define __SP_MODULE_H__

/*
 * Frontend to certain, possibly pluggable, actions
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   Ted Gould <ted@gould.cx>
 *
 * Copyright (C) 2002 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#define SP_MODULE_KEY_INPUT_SVG "modules.input.SVG"
#define SP_MODULE_KEY_INPUT_AI "modules.input.AdobeIllustrator"
#define SP_MODULE_KEY_INPUT_DEFAULT SP_MODULE_KEY_INPUT_SVG
#define SP_MODULE_KEY_OUTPUT_SVG "modules.output.SVG.plain"
#define SP_MODULE_KEY_OUTPUT_SVG_SODIPODI "modules.output.SVG.sodipodi"
#define SP_MODULE_KEY_OUTPUT_DEFAULT SP_MODULE_KEY_OUTPUT_SVG_SODIPODI

#define SP_TYPE_MODULE (sp_module_get_type ())
#define SP_MODULE(o)  (G_TYPE_CHECK_INSTANCE_CAST ((o), SP_TYPE_MODULE, SPModule))
#define SP_IS_MODULE(o) (G_TYPE_CHECK_INSTANCE_TYPE ((o), SP_TYPE_MODULE))

typedef struct _SPModule SPModule;
typedef struct _SPModuleClass SPModuleClass;

#define SP_TYPE_MODULE_HANDLER (sp_module_handler_get_type ())
#define SP_MODULE_HANDLER(o)  (G_TYPE_CHECK_INSTANCE_CAST ((o), SP_TYPE_MODULE_HANDLER, SPModuleHandler))
#define SP_IS_MODULE_HANDLER(o) (G_TYPE_CHECK_INSTANCE_TYPE ((o), SP_TYPE_MODULE_HANDLER))

typedef struct _SPModuleHandler SPModuleHandler;
typedef struct _SPModuleHandlerClass SPModuleHandlerClass;

#define SP_TYPE_MODULE_INPUT (sp_module_input_get_type ())
#define SP_MODULE_INPUT(o)  (G_TYPE_CHECK_INSTANCE_CAST ((o), SP_TYPE_MODULE_INPUT, SPModuleInput))
#define SP_IS_MODULE_INPUT(o) (G_TYPE_CHECK_INSTANCE_TYPE ((o), SP_TYPE_MODULE_INPUT))

typedef struct _SPModuleInput SPModuleInput;
typedef struct _SPModuleInputClass SPModuleInputClass;

#define SP_TYPE_MODULE_OUTPUT (sp_module_output_get_type())
#define SP_MODULE_OUTPUT(o) (G_TYPE_CHECK_INSTANCE_CAST ((o), SP_TYPE_MODULE_OUTPUT, SPModuleOutput))
#define SP_IS_MODULE_OUTPUT(o) (G_TYPE_CHECK_INSTANCE_TYPE ((o), SP_TYPE_MODULE_OUTPUT))

typedef struct _SPModuleOutput SPModuleOutput;
typedef struct _SPModuleOutputClass SPModuleOutputClass;

#define SP_TYPE_MODULE_FILTER (sp_module_filter_get_type())
#define SP_MODULE_FILTER(o) (G_TYPE_CHECK_INSTANCE_CAST ((o), SP_TYPE_MODULE_FILTER, SPModuleFilter))
#define SP_IS_MODULE_FILTER(o) (G_TYPE_CHECK_INSTANCE_TYPE ((o), SP_TYPE_MODULE_FILTER))

typedef struct _SPModuleFilter SPModuleFilter;
typedef struct _SPModuleFilterClass SPModuleFilterClass;

#define SP_TYPE_MODULE_PRINT (sp_module_print_get_type())
#define SP_MODULE_PRINT(o) (G_TYPE_CHECK_INSTANCE_CAST ((o), SP_TYPE_MODULE_PRINT, SPModulePrint))
#define SP_IS_MODULE_PRINT(o) (G_TYPE_CHECK_INSTANCE_TYPE ((o), SP_TYPE_MODULE_PRINT))

typedef struct _SPModulePrint SPModulePrint;
typedef struct _SPModulePrintClass SPModulePrintClass;

#include "widgets/menu.h"
#include "xml/repr.h"
#include "forward.h"

/* SPModule */

struct _SPModule {
	GObject object;

	SPRepr *repr;

	unsigned char *id;

	gchar *name;
	gchar *version;
	GList *authors;
	gchar *copyright;
	gchar *description;
	gboolean about;
	SPModuleHandler *exec;
	gboolean toolbox;
	gchar *icon;
};

struct _SPModuleClass {
	GObjectClass parent_class;

	void (*build) (SPModule *module, SPRepr *repr);
};

GType sp_module_get_type (void);

#define SP_MODULE_ID(m) (((SPModule *) (m))->id)

SPModule *sp_module_new (GType type, SPRepr *repr);
SPModule *sp_module_new_from_path (GType type, const unsigned char *path);

SPModule *sp_module_ref (SPModule *mod);
SPModule *sp_module_unref (SPModule *mod);

/* ModuleInput */

struct _SPModuleInput {
	SPModule module;
	gchar *mimetype;
	gchar *extention;
};

struct _SPModuleInputClass {
	SPModuleClass module_class;
};

GType sp_module_input_get_type (void);

SPDocument *sp_module_input_document_open (SPModuleInput *mod, const unsigned char *uri, unsigned int advertize, unsigned int keepalive);

/* ModuleOutput */

struct _SPModuleOutput {
	SPModule module;
	gchar *mimetype;
	gchar *extention;
};

struct _SPModuleOutputClass {
	SPModuleClass module_class;
};

GType sp_module_output_get_type (void);

void sp_module_output_document_save (SPModuleOutput *mod, SPDocument *doc, const unsigned char *uri);

/* ModuleFilter */

struct _SPModuleFilter {
	SPModule module;
};

struct _SPModuleFilterClass {
	SPModuleClass module_class;
};

GType sp_module_filter_get_type (void);

/* ModulePrint */

#include <libnr/nr-path.h>
#include <display/nr-arena-forward.h>

struct _SPModulePrint {
	SPModule module;

	/* Copy of document image */
	SPItem *base;
	NRArena *arena;
	NRArenaItem *root;
	unsigned int dkey;
};

struct _SPModulePrintClass {
	SPModuleClass module_class;

	/* FALSE means user hit cancel */
	unsigned int (* setup) (SPModulePrint *modp);
	unsigned int (* set_preview) (SPModulePrint *modp);

	unsigned int (* begin) (SPModulePrint *modp, SPDocument *doc);
	unsigned int (* finish) (SPModulePrint *modp);

	/* Rendering methods */
	unsigned int (* bind) (SPModulePrint *modp, const NRMatrixF *transform, float opacity);
	unsigned int (* release) (SPModulePrint *modp);
	unsigned int (* fill) (SPModulePrint *modp, const NRBPath *bpath, const NRMatrixF *ctm, const SPStyle *style,
			       const NRRectF *pbox, const NRRectF *dbox, const NRRectF *bbox);
	unsigned int (* stroke) (SPModulePrint *modp, const NRBPath *bpath, const NRMatrixF *transform, const SPStyle *style,
				 const NRRectF *pbox, const NRRectF *dbox, const NRRectF *bbox);
	unsigned int (* image) (SPModulePrint *modp, unsigned char *px, unsigned int w, unsigned int h, unsigned int rs,
				const NRMatrixF *transform, const SPStyle *style);
};

GType sp_module_print_get_type (void);

/* Global methods */

SPModule *sp_module_system_get (const unsigned char *key);

void sp_module_system_menu_open (SPMenu *menu);
void sp_module_system_menu_save (SPMenu *menu);

#endif
