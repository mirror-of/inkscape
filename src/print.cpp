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
#include "extension/extension.h"
#include "extension/system.h"
#include "print.h"

#if 0
#include <extension/internal/ps.h>

#ifdef WIN32
#include <extension/internal/win32.h>
#endif

#ifdef WITH_GNOME_PRINT
#include <extension/internal/gnome.h>
#endif
#endif

/* Identity typedef */

struct _SPPrintContext {
	Inkscape::Extension::Print * module;
};

unsigned int sp_print_bind(SPPrintContext *ctx, NR::Matrix const &transform, float opacity)
{
	NRMatrix const ntransform(transform);
	return sp_print_bind(ctx, &ntransform, opacity);
}

unsigned int
sp_print_bind (SPPrintContext *ctx, const NRMatrix *transform, float opacity)
{
	return ctx->module->bind(transform, opacity);
}

unsigned int
sp_print_release (SPPrintContext *ctx)
{
	return ctx->module->release();
}

unsigned int
sp_print_fill (SPPrintContext *ctx, const NRBPath *bpath, const NRMatrix *ctm, const SPStyle *style,
	       const NRRect *pbox, const NRRect *dbox, const NRRect *bbox)
{
	return ctx->module->fill(bpath, ctm, style, pbox, dbox, bbox);
}

unsigned int
sp_print_stroke (SPPrintContext *ctx, const NRBPath *bpath, const NRMatrix *ctm, const SPStyle *style,
		 const NRRect *pbox, const NRRect *dbox, const NRRect *bbox)
{
	return ctx->module->stroke(bpath, ctm, style, pbox, dbox, bbox);
}

unsigned int
sp_print_image_R8G8B8A8_N (SPPrintContext *ctx,
			   guchar *px, unsigned int w, unsigned int h, unsigned int rs,
			   const NRMatrix *transform, const SPStyle *style)
{
	return ctx->module->image(px, w, h, rs, transform, style);
}


#include "display/nr-arena.h"
#include "display/nr-arena-item.h"

/* UI */

void
sp_print_preview_document (SPDocument *doc)
{
	Inkscape::Extension::Print *mod;
	unsigned int ret;

	sp_document_ensure_up_to_date (doc);

	mod = sp_module_system_get_print(SP_MODULE_KEY_PRINT_DEFAULT);

	ret = mod->set_preview ();

	if (ret) {
		SPPrintContext context;
		context.module = mod;

		/* fixme: This has to go into module constructor somehow */
		/* Create new arena */
		mod->base = SP_ITEM (sp_document_root (doc));
		mod->arena = (NRArena *) nr_object_new (NR_TYPE_ARENA);
		mod->dkey = sp_item_display_key_new (1);
		mod->root = sp_item_invoke_show (mod->base, mod->arena, mod->dkey, SP_ITEM_SHOW_PRINT);
		/* Print document */
		ret = mod->begin (doc);
		sp_item_invoke_print (mod->base, &context);
		ret = mod->finish ();
		/* Release arena */
		sp_item_invoke_hide (mod->base, mod->dkey);
		mod->base = NULL;
		nr_arena_item_unref (mod->root);
		mod->root = NULL;
		nr_object_unref ((NRObject *) mod->arena);
		mod->arena = NULL;
	}

	return;
}

void
sp_print_document (SPDocument *doc, unsigned int direct)
{
	Inkscape::Extension::Print *mod;
	unsigned int ret;

	sp_document_ensure_up_to_date (doc);

	if (direct) {
		mod = sp_module_system_get_print(SP_MODULE_KEY_PRINT_PS);
	} else {
		mod = sp_module_system_get_print(SP_MODULE_KEY_PRINT_DEFAULT);
	}

	ret = mod->setup();

	if (ret) {
		SPPrintContext context;
		context.module = mod;

		/* fixme: This has to go into module constructor somehow */
		/* Create new arena */
		mod->base = SP_ITEM (sp_document_root (doc));
		mod->arena = (NRArena *) nr_object_new (NR_TYPE_ARENA);
		mod->dkey = sp_item_display_key_new (1);
		mod->root = sp_item_invoke_show (mod->base, mod->arena, mod->dkey, SP_ITEM_SHOW_PRINT);
		/* Print document */
		ret = mod->begin (doc);
		sp_item_invoke_print (mod->base, &context);
		ret = mod->finish ();
		/* Release arena */
		sp_item_invoke_hide (mod->base, mod->dkey);
		mod->base = NULL;
		nr_arena_item_unref (mod->root);
		mod->root = NULL;
		nr_object_unref ((NRObject *) mod->arena);
		mod->arena = NULL;
	}

	return;
}

void
sp_print_document_to_file (SPDocument *doc, const gchar *filename)
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

