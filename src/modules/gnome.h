#ifndef __SP_GNOME_H__
#define __SP_GNOME_H__

/*
 * Gnome stuff
 *
 * Author:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * This code is in public domain
 */

#include <config.h>

#define SP_TYPE_MODULE_PRINT_GNOME (sp_module_print_gnome_get_type())
#define SP_MODULE_PRINT_GNOME(o) (G_TYPE_CHECK_INSTANCE_CAST ((o), SP_TYPE_MODULE_PRINT_GNOME, SPModulePrintGnome))
#define SP_IS_MODULE_PRINT_GNOME(o) (G_TYPE_CHECK_INSTANCE_TYPE ((o), SP_TYPE_MODULE_PRINT_GNOME))

typedef struct _SPModulePrintGnome SPModulePrintGnome;
typedef struct _SPModulePrintGnomeClass SPModulePrintGnomeClass;

#include <libgnomeprint/gnome-print.h>

#include <module.h>

struct _SPModulePrintGnome {
	SPModulePrint module;
	GnomePrintContext *gpc;
};

struct _SPModulePrintGnomeClass {
	SPModulePrintClass module_print_class;
};

GType sp_module_print_gnome_get_type (void);

#endif
