#ifndef __SP_CHARS_H__
#define __SP_CHARS_H__

/*
 * SPChars - parent class for text objects
 *
 * Author:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * Copyright (C) 1999-2002 Lauris Kaplinski
 * Copyright (C) 2000-2001 Ximian, Inc.
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <libnrtype/FontInstance.h>
#include "display/curve.h"
#include "sp-item.h"

#define SP_TYPE_CHARS (sp_chars_get_type ())
#define SP_CHARS(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), SP_TYPE_CHARS, SPChars))
#define SP_CHARS_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), SP_TYPE_CHARS, SPCharsClass))
#define SP_IS_CHARS(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), SP_TYPE_CHARS))
#define SP_IS_CHARS_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), SP_TYPE_CHARS))

class SPCharElement;

struct SPCharElement {
	SPCharElement *next;
	guint glyph;
	font_instance *font;
	NRMatrix transform;
};

struct SPChars {
	SPItem item;
	SPCharElement *elements;
	NRRect paintbox;
};

struct SPCharsClass {
	SPItemClass parent_class;
};

GType sp_chars_get_type ();

void sp_chars_clear (SPChars *chars);

void sp_chars_add_element(SPChars *chars, guint glyph, font_instance *font, NR::Matrix const &transform);
void sp_chars_add_element(SPChars *chars, guint glyph, font_instance *font, NRMatrix const *transform);

SPCurve *sp_chars_normalized_bpath (SPChars *chars);

/* This is completely unrelated to SPItem::print */
void sp_chars_do_print (SPChars *chars, SPPrintContext *ctx, const NRMatrix *ctm,
			const NRRect *pbox, const NRRect *dbox, const NRRect *bbox);
void sp_chars_set_paintbox (SPChars *chars, NRRect *paintbox);

#endif


