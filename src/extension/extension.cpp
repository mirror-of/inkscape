#define __SP_MODULE_C__
/**
	\file extension.cpp
 
	Frontend to certain, possibly pluggable, actions
*/

/*
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   Ted Gould <ted@gould.cx>
 *
 * Copyright (C) 2002-2003 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <config.h>

#include <string.h>
#include <stdlib.h>

#include "helper/sp-intl.h"
#include "dir-util.h"
#include "inkscape.h"
#include "sp-object.h"
#include "document.h"
#include "extension.h"

#include "db.h"

/* SPModule */

static void sp_module_class_init (SPModuleClass *klass);
static void sp_module_init (SPModule *module);
static void sp_module_finalize (GObject *object);

static void sp_module_private_build (SPModule *module, SPRepr *repr);
static void sp_module_load_default (SPModule * module);
static void sp_module_unload_default (SPModule * module);


/** 
	\var module_parent_class

	This is the parent class for the modules.  It should be
	GObject - but no promises */
static GObjectClass * module_parent_class;

/**
	\return  The type value for a SP Module
	\brief   A quick way to get the type value for a SPModule

	This function sets up a static for the type of an SPModule.  If
	that value has not been initialized yet, then it is gotten by
	using the Glib register function with a internally defined
	configuration.  This is similar to how most GModule based objects
	implement this function.
*/
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
			NULL,	/* value_table */
		};
		type = g_type_register_static (G_TYPE_OBJECT, "SPModule", &info, (GTypeFlags)0);
	}
	return type;
}

/**
	\return  none
	\brief   A function to initalize the SPModule class
	\param   klass  This is the SPModuleClass that needs to be
	                initialized.

	This function goes and finds the parent class, and puts that into
	the global parent class for the modules.  It also sets up the 
	build function in the class to sp_module_private_build.
*/
static void
sp_module_class_init (SPModuleClass *klass)
{
	GObjectClass *g_object_class;

	g_object_class = (GObjectClass *)klass;

	module_parent_class = (GObjectClass *)g_type_class_peek_parent ((void *)klass);

	g_object_class->finalize = sp_module_finalize;

	klass->build = sp_module_private_build;

	return;
}

/**
	\return none
	\brief  A function to initialize all of the fields in a SPModule object
	\param  module  The module that needs to be initalized.

	This function quite simply initializes all of the fields is a
	given SPModule structure.  For most of them they are set up
	to NULL, there are a few that are interesting though.  The function
	pointers (load and unload) are set to the default handlers.  This
	makes it easier for those who want to use the default handlers, they
	can just assume that they are already assigned.
*/
static void
sp_module_init (SPModule *module)
{
	module->repr        = NULL;
	module->id          = NULL;
	module->name        = NULL;

	module->state       = SP_MODULE_UNLOADED;
	module->load        = sp_module_load_default;
	module->unload      = sp_module_unload_default;

	return;
}

/**
	\return none
	\brief  The module is dead, clean it up.
	\param  object  The data that should be cleaned up.

	This function is the last to be called in the life cycle of a
	SPModule.  This cleans up all of the malloced memory that could
	have been used for all the values.  It also removes the reference
	for the Repr that was referenced in the module.
*/
static void
sp_module_finalize (GObject *object)
{
	SPModule *module;

	module = SP_MODULE (object);

	sp_module_db_unregister (module);

	if (module->repr) sp_repr_unref (module->repr);

	g_free (module->name);
	g_free (module->id);

	G_OBJECT_CLASS (module_parent_class)->finalize (object);

	return;
}

/**
	\return  none
	\brief   Builds a SPModule object from a SPRepr
	\param   module  The module to be built.
	\param   repr    The repr that should be used to build it

	This function is the basis of building a module for Sodipodi.  It
	currently extracts the fields from the Repr that are used in the
	module.  The Repr will likely include other children that are
	not related to the module directly.  If the Repr does not include
	a name and an ID the module will be left in an errored state.
*/
static void
sp_module_private_build (SPModule *module, SPRepr *repr)
{
	if (repr != NULL) {
		const gchar *val;
		gchar c[256];
		SPRepr * child_repr;

		child_repr = sp_repr_children(repr);
		/* TODO: Handle what happens if we don't have these two */
		while (child_repr != NULL) {
			if (!strcmp(sp_repr_name(child_repr), "id")) {
				val = sp_repr_content (sp_repr_children(child_repr));
				val = sp_module_db_get_unique_id (c, 256, val);
				module->id = g_strdup (val);
			} /* id */
			if (!strcmp(sp_repr_name(child_repr), "name")) {
				module->name = g_strdup (sp_repr_content(sp_repr_children(child_repr)));
			} /* name */
			child_repr = sp_repr_next(child_repr);
		}

		sp_module_db_register (module);
	}

	return;
}

SPModule *
sp_module_new_from_path (GType type, const char *path)
{
	SPRepr *repr;

	repr = inkscape_get_repr (INKSCAPE, path);

	return sp_module_new (type, repr);
}

/**
	\return  Returns the module that was passed in
	\brief   Increases the reference count of a SPModule object
	\param   mod   The module that should have its reference count
	               increased.
	
	This function uses the GObject reference counting mechanizms on
	the SPModule object.  This is a tried and true way of doing
	refeerence counting.
*/
SPModule *
sp_module_ref (SPModule *mod)
{
	g_object_ref (G_OBJECT (mod));
	return mod;
}

/**
	\return  Returns NULL.
	\brief   This function lowers the reference count by one.
	\param   mod  The module that your no longer interested in

	This function uses the GLib reference counting to lower the
	reference count.  This could cause the object to finalize if the
	count goes to zero (basically meaning that no one cares any
	more)
*/
SPModule *
sp_module_unref (SPModule *mod)
{
	g_object_unref (G_OBJECT (mod));
	return NULL;
}

/**
	\return   none
	\brief    A default function to handle the load pointer, this function
	          just changes the state to loaded.
	\param    module The module to load.
*/
static void
sp_module_load_default (SPModule * module)
{
	module->state = SP_MODULE_LOADED;
	return;
}

/**
	\return   none
	\brief    A default function to handle the unload pointer, this
	          function just changes the state to unloaded.
    \param    module  The module to unload.
*/
static void
sp_module_unload_default (SPModule * module)
{
	module->state = SP_MODULE_UNLOADED;
	return;
}

/**
	\return  A new module object.
	\brief   This is the function that creates a module object genericly,
	         most subclasses implement this on their own (non-staicly).
	\param   type  The Glib type indentifier of which SPModule subclass
	               should be created.
	\param   repr  The XML structure of the module definition.

	This function is used to build an SPModule (really a subclass of that
	though) from an XML description stored in a SPRepr tree.  If the class
	has its own build function, then that function is used.  Otherwise
	the generic 'sp_module_private_build' function is used.

	A reference to the SPRepr structure is added in this function so it
	should not need to be added by each individual subclass.
*/
SPModule *
sp_module_new (GType type, SPRepr *repr)
{
	SPModule *module;

	module = (SPModule *)g_object_new (type, NULL);

	g_return_val_if_fail(module != NULL, NULL);
	if (repr == NULL) {
		g_object_unref(module);
		g_return_val_if_fail(repr != NULL, NULL);
	}

	sp_repr_ref (repr);
	module->repr = repr;
	if (((SPModuleClass *) G_OBJECT_GET_CLASS (module))->build != NULL) {
		((SPModuleClass *) G_OBJECT_GET_CLASS (module))->build (module, repr);
	} else {
		sp_module_private_build (module, repr);
	}

	return module;
}

/* ModuleInput */

static void sp_module_input_class_init (SPModuleInputClass *klass);
static void sp_module_input_init (SPModuleInput *object);
static void sp_module_input_finalize (GObject *object);

static void sp_module_input_build (SPModule *module, SPRepr *repr);

/** The parent class of the SPModuleInput class, this should be a
    SPModule.  */
static SPModuleClass *input_parent_class;

/**
	\return   The type identifier for the SPModuleInput object
	\brief    This function stores, and creates on first calling, the
	          type identifier for the SPModuleInput object.

	This is a function that is standard in Glib object type definitions.
	It keeps a static for the type identifier that is create for the
	SPModuleInput object.  If that identifier has not yet been defined,
	it creates it using a constant structure that is also included in
	the function.
*/
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
			NULL,	/* value_table */
		};
		type = g_type_register_static (SP_TYPE_MODULE, "SPModuleInput", &info, (GTypeFlags)0);
	}
	return type;
}

/**
	\return   none
	\brief    A function to initialize the SPModuleInput class
	\param    klass  The class to be initialized

	This function fills in the data fields that are in the SPModuleInput
	class.  The only one that is not in the standard Glib object system
	is the build routine.  This is used to build the object from a
	SPRepr later.  Also, the global variable: input_parent_class is
	set in this function.
*/
static void
sp_module_input_class_init (SPModuleInputClass *klass)
{
	GObjectClass *g_object_class;
	SPModuleClass *module_class;

	g_object_class = (GObjectClass *) klass;
	module_class = (SPModuleClass *) klass;

	input_parent_class = (SPModuleClass *)g_type_class_peek_parent ((void *)klass);

	g_object_class->finalize = sp_module_input_finalize;

	module_class->build = sp_module_input_build;
	return;
}

/**
	\return  none
	\brief   Initialize a SPModuleInput object
	\param   imod  The object to be initialized

	This function basically sets the entire structure to zero.  Because
	NULL is not always defined as zero, the pointers are set to NULL
	expicitly in this function.
*/
static void
sp_module_input_init (SPModuleInput *imod)
{
	imod->mimetype =  NULL;
	imod->extension = NULL;
	imod->filetypename = NULL;
	imod->filetypetooltip = NULL;
	imod->prefs =     NULL;
	imod->open =      NULL;

	return;
}

/**
	\return   none
	\brief    It's the end my friend
	\param    object  The object that has lost all of its friends

	This is the function that removes all of the data associated with
	a SPModuleInput object.  It frees all allocated memory and then
	calls the parent class' finalize routine to finish that part of the
	clean up.  At the end, there should be no memory leaks.
*/
static void
sp_module_input_finalize (GObject *object)
{
	SPModuleInput *imod;

	imod = (SPModuleInput *) object;
	
	g_free (imod->mimetype);
	g_free (imod->extension);
	g_free (imod->filetypename);
	g_free (imod->filetypetooltip);

	G_OBJECT_CLASS (input_parent_class)->finalize (object);

	return;
}

/**
	\return   None
	\brief    Builds a SPModuleInput object from a XML description
	\param    module  The module to be initialized
	\param    repr    The XML description in a SPRepr tree

	Okay, so you want to build a SPModuleInput object.

	This function first takes and does the build of the parent class,
	which is SPModule.  Then, it looks for the <input> section of the
	XML description.  Under there should be several fields which
	describe the input module to excruciating detail.  Those are parsed,
	copied, and put into the structure that is passed in as module.
	Overall, there are many levels of indentation, just to handle the
	levels of indentation in the XML file.
*/
static void
sp_module_input_build (SPModule *module, SPRepr *repr)
{
	SPModuleInput *imod;

	imod = (SPModuleInput *) module;

	if (((SPModuleClass *) input_parent_class)->build)
		((SPModuleClass *) input_parent_class)->build (module, repr);

	if (repr != NULL) {
		SPRepr * child_repr;

		child_repr = sp_repr_children(repr);

		while (child_repr != NULL) {
			if (!strcmp(sp_repr_name(child_repr), "input")) {
				child_repr = sp_repr_children(child_repr);
				while (child_repr != NULL) {
					if (!strcmp(sp_repr_name(child_repr), "extension")) {
						g_free (imod->extension);
						imod->extension = g_strdup(sp_repr_content(sp_repr_children(child_repr)));
					}
					if (!strcmp(sp_repr_name(child_repr), "mimetype")) {
						g_free (imod->mimetype);
						imod->mimetype = g_strdup(sp_repr_content(sp_repr_children(child_repr)));
					}
					if (!strcmp(sp_repr_name(child_repr), "filetypename")) {
						g_free (imod->filetypename);
						imod->filetypename = g_strdup(sp_repr_content(sp_repr_children(child_repr)));
					}
					if (!strcmp(sp_repr_name(child_repr), "filetypetooltip")) {
						g_free (imod->filetypetooltip);
						imod->filetypetooltip = g_strdup(sp_repr_content(sp_repr_children(child_repr)));
					}

					child_repr = sp_repr_next(child_repr);
				}

				break;
			}

			child_repr = sp_repr_next(child_repr);
		}

	}

	return;
}

/**
	\return   A brand new SPModuleInput object
	\brief    This function creates a SPModuleInput object from an
	          XML description as stored in a SPRepr.
	\param    in_repr   The description of the SPModuleInput object

	This function just calls sp_module_new with the type parameter
	being hard coded to SP_TYPE_MODULE_INPUT.
*/
SPModuleInput *
sp_module_input_new (SPRepr * in_repr)
{
	return SP_MODULE_INPUT(sp_module_new(SP_TYPE_MODULE_INPUT, in_repr));
}


/* ModuleOutput */

static void sp_module_output_class_init (SPModuleOutputClass *klass);
static void sp_module_output_init (SPModuleOutput *omod);
static void sp_module_output_finalize (GObject *object);

static void sp_module_output_build (SPModule *module, SPRepr *repr);

/** The parent class of the SPModuleOuptut class, this should be a
    SPModule.  */
static SPModuleClass *output_parent_class;

/**
	\return   The type identifier for the SPModuleOutput object
	\brief    This function stores, and creates on first calling, the
	          type identifier for the SPModuleOutput object.

	This is a function that is standard in Glib object type definitions.
	It keeps a static for the type identifier that is create for the
	SPModuleOutput object.  If that identifier has not yet been defined,
	it creates it using a constant structure that is also included in
	the function.
*/
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
			NULL,	/* value_table */
		};
		type = g_type_register_static (SP_TYPE_MODULE, "SPModuleOutput", &info, (GTypeFlags)0);
	}
	return type;
}

/**
	\return   none
	\brief    A function to initialize the SPModuleOutput class
	\param    klass  The class to be initialized

	This function fills in the data fields that are in the SPModuleOutput
	class.  The only one that is not in the standard Glib object system
	is the build routine.  This is used to build the object from a
	SPRepr later.  Also, the global variable: output_parent_class is
	set in this function.
*/
static void
sp_module_output_class_init (SPModuleOutputClass *klass)
{
	GObjectClass *g_object_class;
	SPModuleClass *module_class;

	g_object_class = (GObjectClass *)klass;
	module_class = (SPModuleClass *) klass;

	output_parent_class = (SPModuleClass *)g_type_class_peek_parent ((void *)klass);

	g_object_class->finalize = sp_module_output_finalize;

	module_class->build = sp_module_output_build;
}

/**
	\return  none
	\brief   Initialize a SPModuleOutput object
	\param   omod  The object to be initialized

	This function basically sets the entire structure to zero.  Because
	NULL is not always defined as zero, the pointers are set to NULL
	expicitly in this function.
*/
static void
sp_module_output_init (SPModuleOutput *omod)
{
	omod->mimetype  = NULL;
	omod->extension = NULL;
	omod->filetypename = NULL;
	omod->filetypetooltip = NULL;

	omod->prefs     = NULL;
	omod->save      = NULL;

	return;
}

/**
	\return   none
	\brief    It's the end my friend
	\param    object  The object that has lost all of its friends

	This is the function that removes all of the data associated with
	a SPModuleOutput object.  It frees all allocated memory and then
	calls the parent class' finalize routine to finish that part of the
	clean up.  At the end, there should be no memory leaks.
*/
static void
sp_module_output_finalize (GObject *object)
{
	SPModuleOutput *omod;

	omod = (SPModuleOutput *) object;

	g_free (omod->mimetype);
	g_free (omod->extension);
	g_free (omod->filetypename);
	g_free (omod->filetypetooltip);
	
	G_OBJECT_CLASS (output_parent_class)->finalize (object);
}

/**
	\return   None
	\brief    Builds a SPModuleOutput object from a XML description
	\param    module  The module to be initialized
	\param    repr    The XML description in a SPRepr tree

	Okay, so you want to build a SPModuleOutput object.

	This function first takes and does the build of the parent class,
	which is SPModule.  Then, it looks for the <output> section of the
	XML description.  Under there should be several fields which
	describe the output module to excruciating detail.  Those are parsed,
	copied, and put into the structure that is passed in as module.
	Overall, there are many levels of indentation, just to handle the
	levels of indentation in the XML file.
*/
static void
sp_module_output_build (SPModule *module, SPRepr *repr)
{
	SPModuleOutput *omod;

	omod = (SPModuleOutput *) module;

	if (((SPModuleClass *) output_parent_class)->build)
		((SPModuleClass *) output_parent_class)->build (module, repr);

	if (repr != NULL) {
		SPRepr * child_repr;

		child_repr = sp_repr_children(repr);

		while (child_repr != NULL) {
			if (!strcmp(sp_repr_name(child_repr), "output")) {
				child_repr = sp_repr_children(child_repr);
				while (child_repr != NULL) {
					if (!strcmp(sp_repr_name(child_repr), "extension")) {
						g_free (omod->extension);
						omod->extension = g_strdup(sp_repr_content(sp_repr_children(child_repr)));
					}
					if (!strcmp(sp_repr_name(child_repr), "mimetype")) {
						g_free (omod->mimetype);
						omod->mimetype = g_strdup(sp_repr_content(sp_repr_children(child_repr)));
					}
					if (!strcmp(sp_repr_name(child_repr), "filetypename")) {
						g_free (omod->filetypename);
						omod->filetypename = g_strdup(sp_repr_content(sp_repr_children(child_repr)));
					}
					if (!strcmp(sp_repr_name(child_repr), "filetypetooltip")) {
						g_free (omod->filetypetooltip);
						omod->filetypetooltip = g_strdup(sp_repr_content(sp_repr_children(child_repr)));
					}

					child_repr = sp_repr_next(child_repr);
				}

				break;
			}

			child_repr = sp_repr_next(child_repr);
		}

	}
}

/**
	\return   A brand new SPModuleOutput object
	\brief    This function creates a SPModuleOutput object from an
	          XML description as stored in a SPRepr.
	\param    in_repr   The description of the SPModuleOutput object

	This function just calls sp_module_new with the type parameter
	being hard coded to SP_TYPE_MODULE_OUTPUT.
*/
SPModuleOutput *
sp_module_output_new (SPRepr * in_repr)
{
	return SP_MODULE_OUTPUT(sp_module_new(SP_TYPE_MODULE_OUTPUT, in_repr));
}


/* ModuleFilter */

static void sp_module_filter_class_init (SPModuleFilterClass *klass);
static void sp_module_filter_init (SPModuleFilter *fmod);
static void sp_module_filter_finalize (GObject *object);

/** The parent class of the SPModuleFilter class, this should be a
    SPModule.  */
static SPModuleClass *filter_parent_class;

/**
	\return   The type identifier for the SPModuleFilter object
	\brief    This function stores, and creates on first calling, the
	          type identifier for the SPModuleFilter object.

	This is a function that is standard in Glib object type definitions.
	It keeps a static for the type identifier that is create for the
	SPModuleFilter object.  If that identifier has not yet been defined,
	it creates it using a constant structure that is also included in
	the function.
*/
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
			NULL,	/* value_table */
		};
		type = g_type_register_static (SP_TYPE_MODULE, "SPModuleFilter", &info, (GTypeFlags)0);
	}
	return type;
}

/**
	\return   none
	\brief    A function to initialize the SPModuleFilter class
	\param    klass  The class to be initialized

	This function fills in the data fields that are in the SPModuleFilter
	class.  The only one that is not in the standard Glib object system
	is the build routine.  This is used to build the object from a
	SPRepr later.  Also, the global variable: filter_parent_class is
	set in this function.
*/
static void
sp_module_filter_class_init (SPModuleFilterClass *klass)
{
	GObjectClass *g_object_class;

	g_object_class = (GObjectClass *)klass;

	filter_parent_class = (SPModuleClass *)g_type_class_peek_parent ((void *)klass);

	g_object_class->finalize = sp_module_filter_finalize;

	return;
}

/**
	\return   none
	\brief    Initialize the SPModuleFilter data structure
	\param    fmod  The filter to be initialized.

	This is a function that will initialize all of the values that
	are specific to the SPModuleFilter structure.  There are none
	right now.
*/
static void
sp_module_filter_init (SPModuleFilter *fmod)
{
	/* Nothing here */
	return;
}

/**
	\return   none
	\brief    It's the end my friend
	\param    object  The object that has lost all of its friends

	This is the function that removes all of the data associated with
	a SPModuleFilter object.  It frees all allocated memory and then
	calls the parent class' finalize routine to finish that part of the
	clean up.  At the end, there should be no memory leaks.
*/
static void
sp_module_filter_finalize (GObject *object)
{
	SPModuleFilter *fmod;

	fmod = (SPModuleFilter *) object;
	
	G_OBJECT_CLASS (filter_parent_class)->finalize (object);

	return;
}

/**
	\return   A brand new SPModuleFilter object
	\brief    This function creates a SPModuleFilter object from an
	          XML description as stored in a SPRepr.
	\param    in_repr   The description of the SPModuleFilter object

	This function just calls sp_module_new with the type parameter
	being hard coded to SP_TYPE_MODULE_FILTER.
*/
SPModuleFilter *
sp_module_filter_new (SPRepr * in_repr)
{
	return SP_MODULE_FILTER(sp_module_new(SP_TYPE_MODULE_FILTER, in_repr));
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
			NULL,	/* value_table */
		};
		type = g_type_register_static (SP_TYPE_MODULE, "SPModulePrint", &info, (GTypeFlags)0);
	}
	return type;
}

static void
sp_module_print_class_init (SPModulePrintClass *klass)
{
	GObjectClass *g_object_class;

	g_object_class = (GObjectClass *)klass;

	print_parent_class = (SPModuleClass *)g_type_class_peek_parent ((void *)klass);

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

SPModulePrint *
sp_module_print_new (SPRepr * in_repr)
{
	return SP_MODULE_PRINT(sp_module_new(SP_TYPE_MODULE_PRINT, in_repr));
}

