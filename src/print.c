#define __SP_PRINT_C__

/*
 * Frontend to printing
 *
 * Author:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * This code is in public domain
 */

#include <config.h>

#include <string.h>
#include <ctype.h>

#include <libnr/nr-rect.h>
#include <libnr/nr-matrix.h>
#include <libnr/nr-pixblock.h>

#include "helper/sp-intl.h"
#include "enums.h"
#include "document.h"
#include "sp-item.h"
#include "style.h"
#include "sp-paint-server.h"
#include "module.h"
#include "print.h"

#include <modules/ps.h>

#ifdef WIN32
#include <modules/win32.h>
#endif

#ifdef WITH_GNOME_PRINT
#include <modules/gnome.h>
#endif

#ifdef WITH_KDE
#include <modules/kde.h>
#endif

/* Identity typedef */

struct _SPPrintContext {
	SPModulePrint module;
};

unsigned int
sp_print_bind (SPPrintContext *ctx, const NRMatrixF *transform, float opacity)
{
	if (((SPModulePrintClass *) G_OBJECT_GET_CLASS (ctx))->bind)
		return ((SPModulePrintClass *) G_OBJECT_GET_CLASS (ctx))->bind (SP_MODULE_PRINT (ctx), transform, opacity);

	return 0;
}

unsigned int
sp_print_release (SPPrintContext *ctx)
{
	if (((SPModulePrintClass *) G_OBJECT_GET_CLASS (ctx))->release)
		return ((SPModulePrintClass *) G_OBJECT_GET_CLASS (ctx))->release (SP_MODULE_PRINT (ctx));

	return 0;
}

unsigned int
sp_print_fill (SPPrintContext *ctx, const NRBPath *bpath, const NRMatrixF *ctm, const SPStyle *style,
	       const NRRectF *pbox, const NRRectF *dbox, const NRRectF *bbox)
{
	if (((SPModulePrintClass *) G_OBJECT_GET_CLASS (ctx))->fill)
		return ((SPModulePrintClass *) G_OBJECT_GET_CLASS (ctx))->fill (SP_MODULE_PRINT (ctx), bpath, ctm, style, pbox, dbox, bbox);

	return 0;
}

unsigned int
sp_print_stroke (SPPrintContext *ctx, const NRBPath *bpath, const NRMatrixF *ctm, const SPStyle *style,
		 const NRRectF *pbox, const NRRectF *dbox, const NRRectF *bbox)
{
	if (((SPModulePrintClass *) G_OBJECT_GET_CLASS (ctx))->stroke)
		return ((SPModulePrintClass *) G_OBJECT_GET_CLASS (ctx))->stroke (SP_MODULE_PRINT (ctx), bpath, ctm, style, pbox, dbox, bbox);

	return 0;
}

unsigned int
sp_print_image_R8G8B8A8_N (SPPrintContext *ctx,
			   unsigned char *px, unsigned int w, unsigned int h, unsigned int rs,
			   const NRMatrixF *transform, const SPStyle *style)
{
	if (((SPModulePrintClass *) G_OBJECT_GET_CLASS (ctx))->image)
		return ((SPModulePrintClass *) G_OBJECT_GET_CLASS (ctx))->image (SP_MODULE_PRINT (ctx), px, w, h, rs, transform, style);

	return 0;
}


#include "display/nr-arena.h"
#include "display/nr-arena-item.h"

/* UI */

void
sp_print_preview_document (SPDocument *doc)
{
	SPModulePrint *mod;
	unsigned int ret;

	sp_document_ensure_up_to_date (doc);

#ifdef WIN32
	mod = g_object_new (SP_TYPE_MODULE_PRINT_WIN32, NULL);
#else
// Some unix probably
#ifdef WITH_KDE
	mod = sp_kde_get_module_print ();
#else
#ifdef WITH_GNOME_PRINT
	mod = (SPModulePrint *) sp_module_new (SP_TYPE_MODULE_PRINT_GNOME, NULL);
#else
	mod = (SPModulePrint *) sp_module_new_from_path (SP_TYPE_MODULE_PRINT_PLAIN, "printing.ps");
#endif
#endif
#endif

	ret = FALSE;
	if (((SPModulePrintClass *) G_OBJECT_GET_CLASS (mod))->set_preview)
		ret = ((SPModulePrintClass *) G_OBJECT_GET_CLASS (mod))->set_preview (mod);

	if (ret) {
		/* fixme: This has to go into module constructor somehow */
		/* Create new arena */
		mod->base = SP_ITEM (sp_document_root (doc));
		mod->arena = (NRArena *) nr_object_new (NR_TYPE_ARENA);
		mod->dkey = sp_item_display_key_new (1);
		mod->root = sp_item_invoke_show (mod->base, mod->arena, mod->dkey, SP_ITEM_SHOW_PRINT);
		/* Print document */
		if (((SPModulePrintClass *) G_OBJECT_GET_CLASS (mod))->begin)
			ret = ((SPModulePrintClass *) G_OBJECT_GET_CLASS (mod))->begin (mod, doc);
		sp_item_invoke_print (mod->base, (SPPrintContext *) mod);
		if (((SPModulePrintClass *) G_OBJECT_GET_CLASS (mod))->finish)
			ret = ((SPModulePrintClass *) G_OBJECT_GET_CLASS (mod))->finish (mod);
		/* Release arena */
		sp_item_invoke_hide (mod->base, mod->dkey);
		mod->base = NULL;
		nr_arena_item_unref (mod->root);
		mod->root = NULL;
		nr_object_unref ((NRObject *) mod->arena);
		mod->arena = NULL;
	}

	g_object_unref (G_OBJECT (mod));
}

void
sp_print_document (SPDocument *doc, unsigned int direct)
{
	SPModulePrint *mod;
	unsigned int ret;

	sp_document_ensure_up_to_date (doc);

	mod = NULL;
	if (direct) mod = (SPModulePrint *) sp_module_new_from_path (SP_TYPE_MODULE_PRINT_PLAIN, "printing.ps");
#ifdef WIN32
	if (!direct) mod = g_object_new (SP_TYPE_MODULE_PRINT_WIN32, NULL);
#endif
#ifdef WITH_KDE
	if (!direct) mod = sp_kde_get_module_print ();
#endif
#ifdef WITH_GNOME_PRINT
	if (!direct) mod = g_object_new (SP_TYPE_MODULE_PRINT_GNOME, NULL);
#endif
	if (!mod) mod = (SPModulePrint *) sp_module_new_from_path (SP_TYPE_MODULE_PRINT_PLAIN, "printing.ps");

	ret = FALSE;
	if (((SPModulePrintClass *) G_OBJECT_GET_CLASS (mod))->setup)
		ret = ((SPModulePrintClass *) G_OBJECT_GET_CLASS (mod))->setup (mod);

	if (ret) {
		/* fixme: This has to go into module constructor somehow */
		/* Create new arena */
		mod->base = SP_ITEM (sp_document_root (doc));
		mod->arena = (NRArena *) nr_object_new (NR_TYPE_ARENA);
		mod->dkey = sp_item_display_key_new (1);
		mod->root = sp_item_invoke_show (mod->base, mod->arena, mod->dkey, SP_ITEM_SHOW_PRINT);
		/* Print document */
		if (((SPModulePrintClass *) G_OBJECT_GET_CLASS (mod))->begin)
			ret = ((SPModulePrintClass *) G_OBJECT_GET_CLASS (mod))->begin (mod, doc);
		sp_item_invoke_print (mod->base, (SPPrintContext *) mod);
		if (((SPModulePrintClass *) G_OBJECT_GET_CLASS (mod))->finish)
			ret = ((SPModulePrintClass *) G_OBJECT_GET_CLASS (mod))->finish (mod);
		/* Release arena */
		sp_item_invoke_hide (mod->base, mod->dkey);
		mod->base = NULL;
		nr_arena_item_unref (mod->root);
		mod->root = NULL;
		nr_object_unref ((NRObject *) mod->arena);
		mod->arena = NULL;
	}

	g_object_unref (G_OBJECT (mod));
}

void
sp_print_document_to_file (SPDocument *doc, const unsigned char *filename)
{
#ifdef lalala
#ifdef WITH_GNOME_PRINT
        GnomePrintConfig *config;
	SPPrintContext ctx;
        GnomePrintContext *gpc;

	config = gnome_print_config_default ();
        if (!gnome_print_config_set (config, "Settings.Engine.Backend.Driver", "gnome-print-ps")) return;
        if (!gnome_print_config_set (config, "Settings.Transport.Backend", "file")) return;
        if (!gnome_print_config_set (config, GNOME_PRINT_KEY_OUTPUT_FILENAME, filename)) return;

	sp_document_ensure_up_to_date (doc);

	gpc = gnome_print_context_new (config);
	ctx.gpc = gpc;

	g_return_if_fail (gpc != NULL);

	/* Print document */
	gnome_print_beginpage (gpc, SP_DOCUMENT_NAME (doc));
	gnome_print_translate (gpc, 0.0, sp_document_height (doc));
	/* From desktop points to document pixels */
	gnome_print_scale (gpc, 0.8, -0.8);
	sp_item_invoke_print (SP_ITEM (sp_document_root (doc)), &ctx);
        gnome_print_showpage (gpc);
        gnome_print_context_close (gpc);
#else
	SPPrintContext ctx;

	ctx.stream = fopen (filename, "w");
	if (ctx.stream) {
		sp_document_ensure_up_to_date (doc);
		fprintf (ctx.stream, "%g %g translate\n", 0.0, sp_document_height (doc));
		fprintf (ctx.stream, "0.8 -0.8 scale\n");
		sp_item_invoke_print (SP_ITEM (sp_document_root (doc)), &ctx);
		fprintf (ctx.stream, "showpage\n");
		fclose (ctx.stream);
	}
#endif
#endif
}

