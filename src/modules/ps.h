#ifndef __SP_PS_H__
#define __SP_PS_H__

/*
 * PostScript printing
 *
 * Author:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * This code is in public domain
 */

#include <config.h>

#define SP_TYPE_MODULE_PRINT_PLAIN (sp_module_print_plain_get_type())
#define SP_MODULE_PRINT_PLAIN(o) (G_TYPE_CHECK_INSTANCE_CAST ((o), SP_TYPE_MODULE_PRINT_PLAIN, SPModulePrintPlain))
#define SP_IS_MODULE_PRINT_PLAIN(o) (G_TYPE_CHECK_INSTANCE_TYPE ((o), SP_TYPE_MODULE_PRINT_PLAIN))

typedef struct _SPModulePrintPlain SPModulePrintPlain;
typedef struct _SPModulePrintPlainClass SPModulePrintPlainClass;

#include <module.h>

struct _SPModulePrintPlain {
	SPModulePrint module;
	unsigned int bitmap : 1;
	unsigned int dpi : 15;
	float width;
	float height;
	FILE *stream;
};

struct _SPModulePrintPlainClass {
	SPModulePrintClass module_print_class;
};

GType sp_module_print_plain_get_type (void);

#endif
