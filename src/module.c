#define __SP_MODULE_C__

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

#include <config.h>

#include <string.h>
#include <stdlib.h>

#include "helper/sp-intl.h"
#include "dir-util.h"
#include "sodipodi.h"
#include "sp-object.h"
#include "document.h"
#include "module.h"

/* SPModule */

static void sp_module_class_init (SPModuleClass *klass);
static void sp_module_init (SPModule *module);
static void sp_module_finalize (GObject *object);

static void sp_module_private_build (SPModule *module, SPRepr *repr);

static const unsigned char *sp_module_get_unique_id (unsigned char *c, int len, const unsigned char *val);
static void sp_module_register (SPModule *module);
static void sp_module_unregister (SPModule *module);

static GObjectClass *module_parent_class;

GType
sp_module_get_type (void)
{
	static GType type = 0;
	if (!type) {
		GTypeInfo info = {
			sizeof (SPModuleClass),
			NULL, NULL,
			(GClassInitFunc) sp_module_class_init,
			NULL, NULL,
			sizeof (SPModule),
			16,
			(GInstanceInitFunc) sp_module_init,
		};
		type = g_type_register_static (G_TYPE_OBJECT, "SPModule", &info, 0);
	}
	return type;
}

static void
sp_module_class_init (SPModuleClass *klass)
{
	GObjectClass *g_object_class;

	g_object_class = (GObjectClass *)klass;

	module_parent_class = g_type_class_peek_parent (klass);

	g_object_class->finalize = sp_module_finalize;

	klass->build = sp_module_private_build;
}

static void
sp_module_init (SPModule *module)
{
	module->about = TRUE;
}

static void
sp_module_finalize (GObject *object)
{
	SPModule *module;

	module = SP_MODULE (object);

	sp_module_unregister (module);

	if (module->repr) sp_repr_unref (module->repr);

	/* fixme: Free everything */
	if (module->name) {
		g_free (module->name);
	}

	G_OBJECT_CLASS (module_parent_class)->finalize (object);
}

static void
sp_module_private_build (SPModule *module, SPRepr *repr)
{
	if (repr) {
		const unsigned char *val;
		unsigned char c[256];
		val = sp_repr_attr (repr, "id");
		val = sp_module_get_unique_id (c, 256, val);
		module->id = g_strdup (val);
		val = sp_repr_attr (repr, "name");
		if (val) {
			module->name = g_strdup (val);
		}
		sp_repr_get_boolean (repr, "about", &module->about);
		val = sp_repr_attr (repr, "icon");
		if (val) {
			module->icon = g_strdup (val);
		}
		sp_repr_get_boolean (repr, "toolbox", &module->toolbox);
		sp_module_register (module);
	}
}

SPModule *
sp_module_new (GType type, SPRepr *repr)
{
	SPModule *module;

	module = g_object_new (type, NULL);

	if (module) {
		if (repr) sp_repr_ref (repr);
		module->repr = repr;
		if (((SPModuleClass *) G_OBJECT_GET_CLASS (module))->build)
			((SPModuleClass *) G_OBJECT_GET_CLASS (module))->build (module, repr);
	}

	return module;
}

SPModule *
sp_module_new_from_path (GType type, const unsigned char *path)
{
	SPRepr *repr;

	repr = sodipodi_get_repr (SODIPODI, path);

	return sp_module_new (type, repr);
}

SPModule *
sp_module_ref (SPModule *mod)
{
	g_object_ref (G_OBJECT (mod));
	return mod;
}

SPModule *
sp_module_unref (SPModule *mod)
{
	g_object_unref (G_OBJECT (mod));
	return NULL;
}

static GHashTable *moduledict = NULL;

static const unsigned char *
sp_module_get_unique_id (unsigned char *c, int len, const unsigned char *val)
{
	static int mnumber = 0;
	if (!moduledict) moduledict = g_hash_table_new (g_str_hash, g_str_equal);
	while (!val || g_hash_table_lookup (moduledict, val)) {
		g_snprintf (c, len, "Module_%d", ++mnumber);
		val = c;
	}
	return val;
}

static void
sp_module_register (SPModule *module)
{
	if (module->id) g_hash_table_insert (moduledict, module->id, module);
}

static void
sp_module_unregister (SPModule *module)
{
	if (module->id) g_hash_table_remove (moduledict, module->id);
}

/* ModuleInput */

static void sp_module_input_class_init (SPModuleInputClass *klass);
static void sp_module_input_init (SPModuleInput *object);
static void sp_module_input_finalize (GObject *object);

static void sp_module_input_build (SPModule *module, SPRepr *repr);

static SPModuleClass *input_parent_class;

GType
sp_module_input_get_type (void)
{
	static GType type = 0;
	if (!type) {
		GTypeInfo info = {
			sizeof (SPModuleInputClass),
			NULL, NULL,
			(GClassInitFunc) sp_module_input_class_init,
			NULL, NULL,
			sizeof (SPModuleInput),
			16,
			(GInstanceInitFunc) sp_module_input_init,
		};
		type = g_type_register_static (SP_TYPE_MODULE, "SPModuleInput", &info, 0);
	}
	return type;
}

static void
sp_module_input_class_init (SPModuleInputClass *klass)
{
	GObjectClass *g_object_class;
	SPModuleClass *module_class;

	g_object_class = (GObjectClass *) klass;
	module_class = (SPModuleClass *) klass;

	input_parent_class = g_type_class_peek_parent (klass);

	g_object_class->finalize = sp_module_input_finalize;

	module_class->build = sp_module_input_build;
}

static void
sp_module_input_init (SPModuleInput *imod)
{
	/* Nothing here by now */
}

static void
sp_module_input_finalize (GObject *object)
{
	SPModuleInput *imod;

	imod = (SPModuleInput *) object;
	
	if (imod->mimetype) {
		g_free (imod->mimetype);
	}

	if (imod->extention) {
		g_free (imod->extention);
	}

	G_OBJECT_CLASS (input_parent_class)->finalize (object);
}

static void
sp_module_input_build (SPModule *module, SPRepr *repr)
{
	SPModuleInput *imod;

	imod = (SPModuleInput *) module;

	if (((SPModuleClass *) input_parent_class)->build)
		((SPModuleClass *) input_parent_class)->build (module, repr);

	if (repr) {
		const unsigned char *val;
		val = sp_repr_attr (repr, "mimetype");
		if (val) {
			imod->mimetype = g_strdup (val);
		}
		val = sp_repr_attr (repr, "extension");
		if (val) {
			imod->extention = g_strdup (val);
		}
	}
}

SPDocument *
sp_module_input_document_open (SPModuleInput *mod, const unsigned char *uri, unsigned int advertize, unsigned int keepalive)
{
	return sp_document_new (uri, advertize, keepalive);
}

/* ModuleOutput */

static void sp_module_output_class_init (SPModuleOutputClass *klass);
static void sp_module_output_init (SPModuleOutput *omod);
static void sp_module_output_finalize (GObject *object);

static void sp_module_output_build (SPModule *module, SPRepr *repr);

static SPModuleClass *output_parent_class;

GType sp_module_output_get_type (void) {
	static GType type = 0;
	if (!type) {
		GTypeInfo info = {
			sizeof (SPModuleOutputClass),
			NULL, NULL,
			(GClassInitFunc) sp_module_output_class_init,
			NULL, NULL,
			sizeof (SPModuleOutput),
			16,
			(GInstanceInitFunc) sp_module_output_init,
		};
		type = g_type_register_static (SP_TYPE_MODULE, "SPModuleOutput", &info, 0);
	}
	return type;
}

static void
sp_module_output_class_init (SPModuleOutputClass *klass)
{
	GObjectClass *g_object_class;
	SPModuleClass *module_class;

	g_object_class = (GObjectClass *)klass;
	module_class = (SPModuleClass *) klass;

	output_parent_class = g_type_class_peek_parent (klass);

	g_object_class->finalize = sp_module_output_finalize;

	module_class->build = sp_module_output_build;
}

static void
sp_module_output_init (SPModuleOutput *omod)
{
	/* Nothing here */
}

static void
sp_module_output_finalize (GObject *object)
{
	SPModuleOutput *omod;

	omod = (SPModuleOutput *) object;
	
	G_OBJECT_CLASS (output_parent_class)->finalize (object);
}

static void
sp_module_output_build (SPModule *module, SPRepr *repr)
{
	SPModuleOutput *mo;

	mo = (SPModuleOutput *) module;

	if (((SPModuleClass *) output_parent_class)->build)
		((SPModuleClass *) output_parent_class)->build (module, repr);

	if (repr) {
		const unsigned char *val;
		val = sp_repr_attr (repr, "mimetype");
		if (val) {
			mo->mimetype = g_strdup (val);
		}
		val = sp_repr_attr (repr, "extension");
		if (val) {
			mo->extention = g_strdup (val);
		}
	}
}

void
sp_module_output_document_save (SPModuleOutput *mod, SPDocument *doc, const unsigned char *uri)
{
	SPRepr *repr;
	gboolean spns;
	const GSList *images, *l;
	SPReprDoc *rdoc;
	const gchar *save_path;

	if (!doc) return;
	if (!uri) return;

	save_path = g_dirname (uri);

	spns = (!SP_MODULE_ID (mod) || !strcmp (SP_MODULE_ID (mod), SP_MODULE_KEY_OUTPUT_SVG_SODIPODI));
	if (spns) {
		rdoc = NULL;
		repr = sp_document_repr_root (doc);
		sp_repr_set_attr (repr, "sodipodi:docbase", save_path);
		sp_repr_set_attr (repr, "sodipodi:docname", uri);
	} else {
		rdoc = sp_repr_document_new ("svg");
		repr = sp_repr_document_root (rdoc);
		repr = sp_object_invoke_write (sp_document_root (doc), repr, SP_OBJECT_WRITE_BUILD);
	}

	images = sp_document_get_resource_list (doc, "image");
	for (l = images; l != NULL; l = l->next) {
		SPRepr *ir;
		const guchar *href, *relname;
		ir = SP_OBJECT_REPR (l->data);
		href = sp_repr_attr (ir, "xlink:href");
		if (spns && !g_path_is_absolute (href)) {
			href = sp_repr_attr (ir, "sodipodi:absref");
		}
		if (href && g_path_is_absolute (href)) {
			relname = sp_relative_path_from_path (href, save_path);
			sp_repr_set_attr (ir, "xlink:href", relname);
		}
	}

	/* fixme: */
	sp_document_set_undo_sensitive (doc, FALSE);
	sp_repr_set_attr (repr, "sodipodi:modified", NULL);
	sp_document_set_undo_sensitive (doc, TRUE);

	sp_repr_save_file (sp_repr_document (repr), uri);
	sp_document_set_uri (doc, uri);

	if (!spns) sp_repr_document_unref (rdoc);
}

/* ModuleFilter */

static void sp_module_filter_class_init (SPModuleFilterClass *klass);
static void sp_module_filter_init (SPModuleFilter *fmod);
static void sp_module_filter_finalize (GObject *object);

static SPModuleClass *filter_parent_class;

GType
sp_module_filter_get_type (void)
{
	static GType type = 0;
	if (!type) {
		GTypeInfo info = {
			sizeof (SPModuleFilterClass),
			NULL, NULL,
			(GClassInitFunc) sp_module_filter_class_init,
			NULL, NULL,
			sizeof (SPModuleFilter),
			16,
			(GInstanceInitFunc) sp_module_filter_init,
		};
		type = g_type_register_static (SP_TYPE_MODULE, "SPModuleFilter", &info, 0);
	}
	return type;
}

static void
sp_module_filter_class_init (SPModuleFilterClass *klass)
{
	GObjectClass *g_object_class;

	g_object_class = (GObjectClass *)klass;

	filter_parent_class = g_type_class_peek_parent (klass);

	g_object_class->finalize = sp_module_filter_finalize;
}

static void
sp_module_filter_init (SPModuleFilter *fmod)
{
	/* Nothing here */
}

static void
sp_module_filter_finalize (GObject *object)
{
	SPModuleFilter *fmod;

	fmod = (SPModuleFilter *) object;
	
	G_OBJECT_CLASS (filter_parent_class)->finalize (object);
}

/* ModulePrint */

static void sp_module_print_class_init (SPModulePrintClass *klass);
static void sp_module_print_init (SPModulePrint *fmod);
static void sp_module_print_finalize (GObject *object);

static SPModuleClass *print_parent_class;

GType
sp_module_print_get_type (void)
{
	static GType type = 0;
	if (!type) {
		GTypeInfo info = {
			sizeof (SPModulePrintClass),
			NULL, NULL,
			(GClassInitFunc) sp_module_print_class_init,
			NULL, NULL,
			sizeof (SPModulePrint),
			16,
			(GInstanceInitFunc) sp_module_print_init,
		};
		type = g_type_register_static (SP_TYPE_MODULE, "SPModulePrint", &info, 0);
	}
	return type;
}

static void
sp_module_print_class_init (SPModulePrintClass *klass)
{
	GObjectClass *g_object_class;

	g_object_class = (GObjectClass *)klass;

	print_parent_class = g_type_class_peek_parent (klass);

	g_object_class->finalize = sp_module_print_finalize;
}

static void
sp_module_print_init (SPModulePrint *fmod)
{
	/* Nothing here */
}

static void
sp_module_print_finalize (GObject *object)
{
	SPModulePrint *fmod;

	fmod = (SPModulePrint *) object;
	
	G_OBJECT_CLASS (print_parent_class)->finalize (object);
}

/* Global methods */

#include "modules/sp-module-sys.h"

SPModule *
sp_module_system_get (const unsigned char *key)
{
	SPModule *mod;
	if (!moduledict) moduledict = g_hash_table_new (g_str_hash, g_str_equal);
	mod = g_hash_table_lookup (moduledict, key);
	if (mod) sp_module_ref (mod);
	return mod;
}

void
sp_module_system_menu_open (SPMenu *menu)
{
	sp_menu_append (menu, _("Scalable Vector Graphic (SVG)"), _("Sodipodi native file format and W3C standard"),
			SP_MODULE_KEY_INPUT_SVG);
}

void
sp_module_system_menu_save (SPMenu *menu)
{
	sp_menu_append (menu,
			_("SVG with \"xmlns:sodipodi\" namespace"),
			_("Scalable Vector Graphics format with sodipodi extensions"),
			SP_MODULE_KEY_OUTPUT_SVG_SODIPODI);
	sp_menu_append (menu,
			_("Plain SVG"),
			_("Scalable Vector Graphics format"),
			SP_MODULE_KEY_OUTPUT_SVG);
}


