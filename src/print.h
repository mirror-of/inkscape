#ifndef __SP_PRINT_H__
#define __SP_PRINT_H__

/*
 * Frontend to printing
 *
 * Author:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * This code is in public domain
 */

#include <libnr/nr-path.h>
#include "forward.h"

unsigned int sp_print_bind(SPPrintContext *ctx, NR::Matrix const &transform, float opacity);
unsigned int sp_print_bind(SPPrintContext *ctx, NRMatrix const *transform, float opacity);
unsigned int sp_print_release (SPPrintContext *ctx);
unsigned int sp_print_fill (SPPrintContext *ctx, const NRBPath *bpath, const NRMatrix *ctm, const SPStyle *style,
			    const NRRect *pbox, const NRRect *dbox, const NRRect *bbox);
unsigned int sp_print_stroke (SPPrintContext *ctx, const NRBPath *bpath, const NRMatrix *transform, const SPStyle *style,
			      const NRRect *pbox, const NRRect *dbox, const NRRect *bbox);

unsigned int sp_print_image_R8G8B8A8_N (SPPrintContext *ctx,
					guchar *px, unsigned int w, unsigned int h, unsigned int rs,
					const NRMatrix *transform, const SPStyle *style);

unsigned int sp_print_text (SPPrintContext* ctx, const char* text, NR::Point p,
			    const SPStyle* style);

void sp_print_get_param(SPPrintContext *ctx, gchar *name, bool *value);


/* UI */
void sp_print_preview_document (SPDocument *doc);
void sp_print_document (SPDocument *doc, unsigned int direct);
void sp_print_document_to_file (SPDocument *doc, const gchar *filename);

#endif
