#ifndef __SP_MODULE_H__
#define __SP_MODULE_H__

/*
 * Frontend to certain, possibly pluggable, actions
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   Ted Gould <ted@gould.cx>
 *
 * Copyright (C) 2002-2003 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

/** The key that is used to identify that the I/O should be autodetected */
#define SP_MODULE_KEY_AUTODETECT "autodetect"
/** This is the key for the SVG input module */
#define SP_MODULE_KEY_INPUT_SVG "modules.input.SVG"
/** Specifies the input module that should be used if none are selected */
#define SP_MODULE_KEY_INPUT_DEFAULT SP_MODULE_KEY_AUTODETECT
/** The key for outputing standard W3C SVG */
#define SP_MODULE_KEY_OUTPUT_SVG "modules.output.SVG.plain"
/** This is an output file that has SVG data with the Sodipodi namespace extensions */
#define SP_MODULE_KEY_OUTPUT_SVG_INKSCAPE "modules.output.SVG.sodipodi"
/** Which output module should be used? */
#define SP_MODULE_KEY_OUTPUT_DEFAULT SP_MODULE_KEY_AUTODETECT

/** A quick identifier to for the GtkType of an SPModule */
#define SP_TYPE_MODULE (sp_module_get_type ())
/** A macro to cast something to an SPModule */
#define SP_MODULE(o)  (G_TYPE_CHECK_INSTANCE_CAST ((o), SP_TYPE_MODULE, SPModule))
/** A macro to check if something is a SPModule (or subclass) */
#define SP_IS_MODULE(o) (G_TYPE_CHECK_INSTANCE_TYPE ((o), SP_TYPE_MODULE))

typedef struct _SPModule SPModule;
typedef struct _SPModuleClass SPModuleClass;

/** A quick identifier to for the GtkType of an SPModuleInput */
#define SP_TYPE_MODULE_INPUT (sp_module_input_get_type ())
/** A macro to cast something to an SPModuleInput */
#define SP_MODULE_INPUT(o)  (G_TYPE_CHECK_INSTANCE_CAST ((o), SP_TYPE_MODULE_INPUT, SPModuleInput))
/** A macro to check if something is a SPModuleInput (or subclass) */
#define SP_IS_MODULE_INPUT(o) (G_TYPE_CHECK_INSTANCE_TYPE ((o), SP_TYPE_MODULE_INPUT))

typedef struct _SPModuleInput SPModuleInput;
typedef struct _SPModuleInputClass SPModuleInputClass;

/** A quick identifier to for the GtkType of an SPModuleOutput */
#define SP_TYPE_MODULE_OUTPUT (sp_module_output_get_type())
/** A macro to cast something to an SPModuleOutput */
#define SP_MODULE_OUTPUT(o) (G_TYPE_CHECK_INSTANCE_CAST ((o), SP_TYPE_MODULE_OUTPUT, SPModuleOutput))
/** A macro to check if something is a SPModuleOutput (or subclass) */
#define SP_IS_MODULE_OUTPUT(o) (G_TYPE_CHECK_INSTANCE_TYPE ((o), SP_TYPE_MODULE_OUTPUT))

typedef struct _SPModuleOutput SPModuleOutput;
typedef struct _SPModuleOutputClass SPModuleOutputClass;

/** A quick identifier to for the GtkType of an SPModuleFilter */
#define SP_TYPE_MODULE_FILTER (sp_module_filter_get_type())
/** A macro to cast something to an SPModuleFilter */
#define SP_MODULE_FILTER(o) (G_TYPE_CHECK_INSTANCE_CAST ((o), SP_TYPE_MODULE_FILTER, SPModuleFilter))
/** A macro to check if something is a SPModuleFilter (or subclass) */
#define SP_IS_MODULE_FILTER(o) (G_TYPE_CHECK_INSTANCE_TYPE ((o), SP_TYPE_MODULE_FILTER))

typedef struct _SPModuleFilter SPModuleFilter;
typedef struct _SPModuleFilterClass SPModuleFilterClass;

/** A quick identifier to for the GtkType of an SPModulePrint */
#define SP_TYPE_MODULE_PRINT (sp_module_print_get_type())
/** A macro to cast something to an SPModulePrint */
#define SP_MODULE_PRINT(o) (G_TYPE_CHECK_INSTANCE_CAST ((o), SP_TYPE_MODULE_PRINT, SPModulePrint))
/** A macro to check if something is a SPModulePrint (or subclass) */
#define SP_IS_MODULE_PRINT(o) (G_TYPE_CHECK_INSTANCE_TYPE ((o), SP_TYPE_MODULE_PRINT))

typedef struct _SPModulePrint SPModulePrint;
typedef struct _SPModulePrintClass SPModulePrintClass;

#include <gtk/gtk.h>
#include "widgets/menu.h"
#include "xml/repr.h"
#include "forward.h"

/** An enumeration to identify if the module has been loaded or not. */
typedef enum {
	SP_MODULE_LOADED,
	SP_MODULE_UNLOADED
} sp_module_state_t;

/* SPModule */

/** The object that is the basis for the module system.  This object
    contains all of the information that all modules have.  The
	individual items are detailed within. */
struct _SPModule {
	GObject object;                     /**< Parent class */

	SPRepr *repr;                       /**< The XML description of the module */

	gchar *id;                          /**< The unique identifier for the module */

	gchar *name;                        /**< A user friendly name for the module */
	sp_module_state_t state;            /**< Which state the module is currently in */

	void (*load) (SPModule *module);    /**< The function that should be called to load the module */
	void (*unload) (SPModule *module);  /**< The function that should be called to unload the module */
};

/** All of the information that is global for all SPModules */
struct _SPModuleClass {
	GObjectClass parent_class;          /**< Parent class */

	void (*build) (SPModule *module, SPRepr *repr);
	                                    /**< The function that is used to build SPModules */
};

GType sp_module_get_type (void);

/** A quick way to get the ID of a module */
#define SP_MODULE_ID(m) (((SPModule *) (m))->id)

SPModule * sp_module_new (GType type, SPRepr *repr);
SPModule *sp_module_new_from_path (GType type, const char *path);

SPModule *sp_module_ref (SPModule *mod);
SPModule *sp_module_unref (SPModule *mod);

/* ModuleInput */

/** Now there are some things that make an Input module unique.  And
    they are stored in this structure. */
struct _SPModuleInput {
	SPModule module;             /**< The data in the parent class */
	gchar *mimetype;             /**< What is the mime type this inputs? */
	gchar *extension;            /**< The extension of the input files */
	gchar *filetypename;         /**< A userfriendly name for the file type */
	gchar *filetypetooltip;      /**< A more detailed description of the filetype */

	GtkDialog * (*prefs) (SPModule * module, const gchar * filename);
	                             /**< The function to find out information about the file */
	SPDocument * (*open) (SPModule * module, const gchar * filename);
	                             /**< Hey, there needs to be some function to do the work! */
};

/** More of a place holder for the Glib Object system */
struct _SPModuleInputClass {
	SPModuleClass module_class; /**< Parent class */
};

GType sp_module_input_get_type (void);

SPModuleInput * sp_module_input_new (SPRepr * in_repr);
SPDocument *sp_module_input_document_open (SPModuleInput *mod, const unsigned char *uri, unsigned int advertize, unsigned int keepalive);

/* ModuleOutput */

/** Now there are some things that make an Output module unique.  And
    they are stored in this structure. */
struct _SPModuleOutput {
	SPModule module;             /**< The data in the parent class */
	gchar *mimetype;             /**< What is the mime type this inputs? */
	gchar *extension;            /**< The extension of the input files */
	gchar *filetypename;         /**< A userfriendly name for the file type */
	gchar *filetypetooltip;      /**< A more detailed description of the filetype */

	GtkDialog * (*prefs) (SPModule * module);
	                             /**< The function to find out information about the file */
	void (*save) (SPModule * module, SPDocument * doc, const gchar * filename);
	                             /**< Hey, there needs to be some function to do the work! */
};

/** More of a place holder for the Glib Object system */
struct _SPModuleOutputClass {
	SPModuleClass module_class; /**< Parent class */
};

SPModuleOutput * sp_module_output_new (SPRepr * in_repr);
GType sp_module_output_get_type (void);

void sp_module_output_document_save (SPModuleOutput *mod, SPDocument *doc, const unsigned char *uri);

/* ModuleFilter */

/** All of the data that is needed for every Filter module is stored
    in this structure. */
struct _SPModuleFilter {
	SPModule module;  /**< Parent class */

	GtkDialog * (*prefs) (SPModule * module);
	                  /**< The function to find out information about the file */
	/* TODO: need to figure out what we need here */
	void (*filter) (SPModule * module, SPDocument * document);
	                  /**< Hey, there needs to be some function to do the work! */
};

/** More of a place holder for the Glib Object system */
struct _SPModuleFilterClass {
	SPModuleClass module_class; /**< Parent class */
};

SPModuleFilter * sp_module_filter_new (SPRepr * in_repr);
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
	unsigned int (* bind) (SPModulePrint *modp, const NRMatrix *transform, float opacity);
	unsigned int (* release) (SPModulePrint *modp);
	unsigned int (* fill) (SPModulePrint *modp, const NRBPath *bpath, const NRMatrix *ctm, const SPStyle *style,
			       const NRRect *pbox, const NRRect *dbox, const NRRect *bbox);
	unsigned int (* stroke) (SPModulePrint *modp, const NRBPath *bpath, const NRMatrix *transform, const SPStyle *style,
				 const NRRect *pbox, const NRRect *dbox, const NRRect *bbox);
	unsigned int (* image) (SPModulePrint *modp, unsigned char *px, unsigned int w, unsigned int h, unsigned int rs,
				const NRMatrix *transform, const SPStyle *style);
};

SPModulePrint * sp_module_print_new (SPRepr * in_repr);
GType sp_module_print_get_type (void);

/* Global methods */

SPModule *sp_module_system_get (const unsigned char *key);

void sp_module_system_menu_open (SPMenu *menu);
void sp_module_system_menu_save (SPMenu *menu);

#endif
