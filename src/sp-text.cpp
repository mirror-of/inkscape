#define __SP_TEXT_C__

/*
 * SVG <text> and <tspan> implementation
 *
 * Author:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * Copyright (C) 1999-2002 Lauris Kaplinski
 * Copyright (C) 2000-2001 Ximian, Inc.
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

/*
 * fixme:
 *
 * These subcomponents should not be items, or alternately
 * we have to invent set of flags to mark, whether standard
 * attributes are applicable to given item (I even like this
 * idea somewhat - Lauris)
 *
 */

#include <config.h>

#include <string.h>

#include <libnr/nr-rect.h>
#include <libnr/nr-matrix.h>
#include <libnrtype/nr-type-directory.h>
#include <libnrtype/nr-font.h>
#include <libnrtype/font-style-to-pos.h>

#include <glib.h>
#include <gtk/gtk.h>

#include "helper/sp-intl.h"
#include "xml/repr-private.h"
#include "svg/svg.h"
#include "display/nr-arena-group.h"
#include "display/nr-arena-glyphs.h"
#include "attributes.h"
#include "document.h"
#include "style.h"
#include "version.h"
#include "inkscape.h"
#include "view.h"

#include "sp-text.h"

static void sp_text_update_length (SPSVGLength *length, gdouble em, gdouble ex, gdouble scale);

/* SPString */

static void sp_string_class_init (SPStringClass *classname);
static void sp_string_init (SPString *string);

static void sp_string_build (SPObject *object, SPDocument *document, SPRepr *repr);
static void sp_string_release (SPObject *object);
static void sp_string_read_content (SPObject *object);
static void sp_string_update (SPObject *object, SPCtx *ctx, unsigned int flags);

static void sp_string_calculate_dimensions (SPString *string);
static void sp_string_set_shape (SPString *string, SPLayoutData *ly, NR::Point &cp, gboolean *inspace);

static SPCharsClass *string_parent_class;

GType
sp_string_get_type ()
{
	static GType type = 0;
	if (!type) {
		GTypeInfo info = {
			sizeof (SPStringClass),
			NULL,	/* base_init */
			NULL,	/* base_finalize */
			(GClassInitFunc) sp_string_class_init,
			NULL,	/* class_finalize */
			NULL,	/* class_data */
			sizeof (SPString),
			16,	/* n_preallocs */
			(GInstanceInitFunc) sp_string_init,
			NULL,	/* value_table */
		};
		type = g_type_register_static (SP_TYPE_CHARS, "SPString", &info, (GTypeFlags)0);
	}
	return type;
}

static void
sp_string_class_init (SPStringClass *classname)
{
	SPObjectClass *sp_object_class;
	SPItemClass *item_class;

	sp_object_class = (SPObjectClass *) classname;
	item_class = (SPItemClass *) classname;

	string_parent_class = (SPCharsClass*)g_type_class_ref (SP_TYPE_CHARS);

	sp_object_class->build = sp_string_build;
	sp_object_class->release = sp_string_release;
	sp_object_class->read_content = sp_string_read_content;
	sp_object_class->update = sp_string_update;
}

static void
sp_string_init (SPString *string)
{
	/*	string->text = NULL;
	string->p = NULL;
	string->start = 0;
	string->length = 0;
	string->bbox.x0 = string->bbox.y0 = 0.0;
	string->bbox.x1 = string->bbox.y1 = 0.0;
	string->advance = NR::Point(0, 0);*/
}

static void
sp_string_build (SPObject *object, SPDocument *doc, SPRepr *repr)
{
	if (((SPObjectClass *) string_parent_class)->build)
		((SPObjectClass *) string_parent_class)->build (object, doc, repr);

	sp_string_read_content (object);

	/* fixme: This can be waste here, but ensures loaded documents are up-to-date */
	sp_string_calculate_dimensions (SP_STRING (object));
}

static void
sp_string_release (SPObject *object)
{
	SPString *string;

	string = SP_STRING (object);

	g_free (string->p);
	g_free (string->text);

	if (((SPObjectClass *) string_parent_class)->release)
		((SPObjectClass *) string_parent_class)->release (object);
}

/* fixme: We have to notify parents that we changed */

static void
sp_string_read_content (SPObject *object)
{
	SPString *string;
	const gchar *t;

	string = SP_STRING (object);

	g_free (string->p);
	string->p = NULL;
	g_free (string->text);
	t = sp_repr_content (object->repr);
	string->text = (t) ? g_strdup (t) : NULL;

	/* Is this correct? I think so (Lauris) */
	/* Virtual method will be invoked BEFORE signal, so we can update there */
	sp_object_request_update (object, SP_OBJECT_MODIFIED_FLAG);
}

/* This happen before parent does layouting but after styles have been set */
/* So it is the right place to calculate untransformed string dimensions */

static void
sp_string_update (SPObject *object, SPCtx *ctx, unsigned int flags)
{
	if (((SPObjectClass *) string_parent_class)->update)
		((SPObjectClass *) string_parent_class)->update (object, ctx, flags);

	if (flags & (SP_OBJECT_STYLE_MODIFIED_FLAG | SP_OBJECT_MODIFIED_FLAG)) {
		/* Parent style or we ourselves changed, so recalculate */
		sp_string_calculate_dimensions (SP_STRING (object));
	}
}

NR::Point
sp_letterspacing_advance (const SPStyle *style)
{
	NR::Point letterspacing_adv;
	if (style->text->letterspacing.value != 0 && style->text->letterspacing.computed == 0) { // set in em or ex
		if (style->text->letterspacing.unit == SP_CSS_UNIT_EM) {
			letterspacing_adv = NR::Point(style->font_size.computed * style->text->letterspacing.value, 0.0);
		} else if (style->text->letterspacing.unit == SP_CSS_UNIT_EX) {
			// I did not invent this 0.5 multiplier; it's what lauris uses in style.cpp
			// Someone knowledgeable must find out how to extract the real em and ex values from the font!
			letterspacing_adv = NR::Point(style->font_size.computed * style->text->letterspacing.value * 0.5, 0.0);
		} 
	} else { // there's a real value in .computed, or it's zero
		letterspacing_adv = NR::Point(style->text->letterspacing.computed, 0.0);
	} 
	return letterspacing_adv;
}

/* Vertical metric simulator */

static void
sp_string_calculate_dimensions (SPString *string)
{
	const SPStyle *style;
	NRTypeFace *face;
	NRFont *font;
	gdouble size;
	gint spglyph;
	unsigned int metrics;
	NR::Point spadv;

	string->bbox.x0 = string->bbox.y0 = 1e18;
	string->bbox.x1 = string->bbox.y1 = -1e18;
	string->advance = NR::Point(0, 0);

	style = SP_OBJECT_STYLE (SP_OBJECT_PARENT (string));
	/* fixme: Adjusted value (Lauris) */
	size = style->font_size.computed;
	face = nr_type_directory_lookup_fuzzy(style->text->font_family.value,
					      font_style_to_pos(*style));
	if (style->writing_mode.computed == SP_CSS_WRITING_MODE_TB) {
		metrics = NR_TYPEFACE_METRICS_VERTICAL;
	} else {
		metrics = NR_TYPEFACE_METRICS_HORIZONTAL;
	}
	font = nr_font_new_default (face, metrics, size);

	// calculating letterspacing advance
	NR::Point letterspacing_adv = sp_letterspacing_advance (style);

	if (style->writing_mode.computed == SP_CSS_WRITING_MODE_TB) {
		spadv = NR::Point(0.0, size);
	} else {
		spadv = NR::Point(size, 0.0);
	}
	spglyph = nr_typeface_lookup_default (face, ' ');
	spadv = nr_font_glyph_advance_get (font, spglyph) + letterspacing_adv;

	if (string->text) {
		const gchar *p;
		gboolean preserve, inspace, intext;

		preserve = (((SPObject*)string)->xml_space.value == SP_XML_SPACE_PRESERVE);
		inspace = FALSE;
		intext = FALSE;

		for (p = string->text; p && *p; p = g_utf8_next_char (p)) {
			gunichar unival;
			
			unival = g_utf8_get_char (p);

			if (g_unichar_isspace (unival) && (unival != g_utf8_get_char ("\302\240"))) { // space but not non-break space
				if (preserve) {
					string->advance += spadv;
				}
				if (unival != '\n' && unival != '\r') inspace = TRUE;
			} else {
				NRRect bbox;
				NR::Point adv;
				gint glyph;

				glyph = nr_typeface_lookup_default (face, unival);

				if (!preserve && inspace && intext) {
					string->advance += spadv;
				}

				if (nr_font_glyph_area_get (font, glyph, &bbox)) {
					string->bbox.x0 = MIN (string->bbox.x0, string->advance[NR::X] + bbox.x0);
					string->bbox.y0 = MIN (string->bbox.y0, string->advance[NR::Y] - bbox.y1);
					string->bbox.x1 = MAX (string->bbox.x1, string->advance[NR::X] + bbox.x1);
					string->bbox.y1 = MAX (string->bbox.y1, string->advance[NR::Y] - bbox.y0);
				}
				adv = nr_font_glyph_advance_get (font, glyph ) + letterspacing_adv;
				string->advance += adv;
				
				inspace = FALSE;
				intext = TRUE;
			}
		}
	}

	nr_font_unref (font);
	nr_typeface_unref (face);

	if (nr_rect_f_test_empty (&string->bbox)) {
		string->bbox.x0 = string->bbox.y0 = 0.0;
		string->bbox.x1 = string->bbox.y1 = 0.0;
	}

}

/* fixme: Should values be parsed by parent? */

static void
sp_string_set_shape (SPString *string, SPLayoutData *ly, NR::Point &cp, gboolean *pinspace)
{

	SPChars *chars = SP_CHARS (string);
	const SPStyle *style = SP_OBJECT_STYLE (SP_OBJECT_PARENT (string));

	sp_chars_clear (chars);

	if (!string->text || !*string->text) return;
	const gint len = g_utf8_strlen (string->text, -1);
	if (!len) return;
	g_free (string->p);
	string->p = g_new (NR::Point, len + 1);

	/* fixme: Adjusted value (Lauris) */
	const gdouble size = style->font_size.computed;
	NRTypeFace *face = nr_type_directory_lookup_fuzzy(style->text->font_family.value,
								font_style_to_pos(*style));
	
	unsigned int metrics;
	if (style->writing_mode.computed == SP_CSS_WRITING_MODE_TB) {
		metrics = NR_TYPEFACE_METRICS_VERTICAL;
	} else {
		metrics = NR_TYPEFACE_METRICS_HORIZONTAL;
	}
	NRFont *font = nr_font_new_default (face, metrics, size);

	// calculating letterspacing advance
	NR::Point letterspacing_adv = sp_letterspacing_advance (style);

	NR::Point spadv;
	if (style->writing_mode.computed == SP_CSS_WRITING_MODE_TB) {
		spadv = NR::Point(0.0, size);
	} else {
		spadv = NR::Point(size, 0);
	}
	gint spglyph = nr_typeface_lookup_default (face, ' ');
	spadv = nr_font_glyph_advance_get (font, spglyph) + letterspacing_adv;

	/* fixme: Find a way how to manipulate these */
	NR::Point pt = cp;

	/* fixme: SPChars should do this upright instead */
	NR::Matrix a = NR::scale (NR::Point(1.0, -1.0));

	gboolean intext = FALSE;
	gboolean preserve = (((SPObject*)string)->xml_space.value == SP_XML_SPACE_PRESERVE);
	gboolean inspace = pinspace ? *pinspace : FALSE;
	gint pos = 0;
	for (gchar const* cur_char = string->text; cur_char && *cur_char; cur_char = g_utf8_next_char (cur_char)) {
		gunichar unival;
               if (!preserve && inspace && intext) {
                       /* SP_XML_SPACE_DEFAULT */
			string->p[pos] = pt + NR::Point(spadv[NR::X], -spadv[NR::Y]);
		} else {
			string->p[pos] = pt;
		}
		unival = g_utf8_get_char (cur_char);
             if (g_unichar_isspace(unival) && (unival != g_utf8_get_char ("\302\240"))) { // space but not non-break space
                       if (preserve) {
                               pt += NR::Point(spadv[NR::X], -spadv[NR::Y]);
                       }
                       if (unival != '\n' && unival != '\r') inspace = TRUE;
		} else {
			gint glyph = nr_typeface_lookup_default (face, unival);

			if (!preserve && inspace && intext) {
				pt = pt + NR::Point(spadv[NR::X], -spadv[NR::Y]);
			}
			
			// NR::translate?
			a.c[4] = pt[NR::X];
			a.c[5] = pt[NR::Y];

			sp_chars_add_element (chars, glyph, font, a);
			NR::Point adv = nr_font_glyph_advance_get (font, glyph) + letterspacing_adv;

			pt = pt + NR::Point(adv[NR::X], -adv[NR::Y]);
			
			inspace = FALSE;
			intext = TRUE;
		}
		pos += 1;
	}

	nr_font_unref (font);
	nr_typeface_unref (face);

	cp = string->p[pos] = pt;

	if (pinspace)
		*pinspace = inspace;
}

/* SPTSpan */

static void sp_tspan_class_init (SPTSpanClass *classname);
static void sp_tspan_init (SPTSpan *tspan);

static void sp_tspan_build (SPObject * object, SPDocument * document, SPRepr * repr);
static void sp_tspan_release (SPObject *object);
static void sp_tspan_set (SPObject *object, unsigned int key, const gchar *value);
static void sp_tspan_child_added (SPObject *object, SPRepr *rch, SPRepr *ref);
static void sp_tspan_remove_child (SPObject *object, SPRepr *rch);
static void sp_tspan_update (SPObject *object, SPCtx *ctx, guint flags);
static void sp_tspan_modified (SPObject *object, unsigned int flags);
static SPRepr *sp_tspan_write (SPObject *object, SPRepr *repr, guint flags);

static void sp_tspan_bbox (SPItem *item, NRRect *bbox, const NRMatrix *transform, unsigned int flags);
static NRArenaItem *sp_tspan_show (SPItem *item, NRArena *arena, unsigned int key, unsigned int flags);
static void sp_tspan_hide (SPItem *item, unsigned int key);

static void sp_tspan_set_shape (SPTSpan *tspan, SPLayoutData *ly, NR::Point &cp, gboolean firstline, gboolean *inspace);

static SPItemClass *tspan_parent_class;

GType
sp_tspan_get_type ()
{
	static GType type = 0;
	if (!type) {
		GTypeInfo info = {
			sizeof (SPTSpanClass),
			NULL,	/* base_init */
			NULL,	/* base_finalize */
			(GClassInitFunc) sp_tspan_class_init,
			NULL,	/* class_finalize */
			NULL,	/* class_data */
			sizeof (SPTSpan),
			16,	/* n_preallocs */
			(GInstanceInitFunc) sp_tspan_init,
			NULL,	/* value_table */
		};
		type = g_type_register_static (SP_TYPE_ITEM, "SPTSpan", &info, (GTypeFlags)0);
	}
	return type;
}

static void
sp_tspan_class_init (SPTSpanClass *classname)
{
	SPObjectClass * sp_object_class;
	SPItemClass * item_class;

	sp_object_class = (SPObjectClass *) classname;
	item_class = (SPItemClass *) classname;

	tspan_parent_class = (SPItemClass*)g_type_class_ref (SP_TYPE_ITEM);

	sp_object_class->build = sp_tspan_build;
	sp_object_class->release = sp_tspan_release;
	sp_object_class->set = sp_tspan_set;
	sp_object_class->child_added = sp_tspan_child_added;
	sp_object_class->remove_child = sp_tspan_remove_child;
	sp_object_class->update = sp_tspan_update;
	sp_object_class->modified = sp_tspan_modified;
	sp_object_class->write = sp_tspan_write;

	item_class->bbox = sp_tspan_bbox;
	item_class->show = sp_tspan_show;
	item_class->hide = sp_tspan_hide;
}

static void
sp_tspan_init (SPTSpan *tspan)
{
	/* fixme: Initialize layout */
	sp_svg_length_unset (&tspan->ly.x, SP_SVG_UNIT_NONE, 0.0, 0.0);
	sp_svg_length_unset (&tspan->ly.y, SP_SVG_UNIT_NONE, 0.0, 0.0);
	sp_svg_length_unset (&tspan->ly.dx, SP_SVG_UNIT_NONE, 0.0, 0.0);
	sp_svg_length_unset (&tspan->ly.dy, SP_SVG_UNIT_NONE, 0.0, 0.0);
	tspan->ly.linespacing = 1.0;
	tspan->string = NULL;
}

static void
sp_tspan_build (SPObject *object, SPDocument *doc, SPRepr *repr)
{
	SPTSpan *tspan;
	SPString *string;
	SPRepr *rch;

	tspan = SP_TSPAN (object);

	if (((SPObjectClass *) tspan_parent_class)->build)
		((SPObjectClass *) tspan_parent_class)->build (object, doc, repr);

	sp_object_read_attr (object, "x");
	sp_object_read_attr (object, "y");
	sp_object_read_attr (object, "dx");
	sp_object_read_attr (object, "dy");
	sp_object_read_attr (object, "rotate");
	sp_object_read_attr (object, "sodipodi:role");

	for (rch = repr->children; rch != NULL; rch = rch->next) {
		if (rch->type == SP_XML_TEXT_NODE) break;
	}

	if (!rch) {
		rch = sp_xml_document_createTextNode (sp_repr_document (repr), "");
		sp_repr_add_child (repr, rch, NULL);
	}

	/* fixme: We should really pick up first child always */
	string = (SPString*)g_object_new (SP_TYPE_STRING, NULL);
	tspan->string = sp_object_attach_reref (object, SP_OBJECT (string), NULL);
	string->ly = &tspan->ly;
	sp_object_invoke_build (tspan->string, doc, rch, SP_OBJECT_IS_CLONED (object));
}

static void
sp_tspan_release (SPObject *object)
{
	SPTSpan *tspan;

	tspan = SP_TSPAN (object);

	if (tspan->string) {
		tspan->string = sp_object_detach_unref (SP_OBJECT (object), tspan->string);
	} else {
		g_print ("NULL tspan content\n");
	}

	if (((SPObjectClass *) tspan_parent_class)->release)
		((SPObjectClass *) tspan_parent_class)->release (object);
}

static void
sp_tspan_set (SPObject *object, unsigned int key, const gchar *value)
{
	SPTSpan *tspan;

	tspan = SP_TSPAN (object);

	/* fixme: Vectors */
	switch (key) {
	case SP_ATTR_X:
		if (!sp_svg_length_read (value, &tspan->ly.x)) {
			sp_svg_length_unset (&tspan->ly.x, SP_SVG_UNIT_NONE, 0.0, 0.0);
		}
		/* fixme: Re-layout it */
		if (tspan->role != SP_TSPAN_ROLE_LINE) sp_object_request_update (object, SP_OBJECT_MODIFIED_FLAG);
		break;
	case SP_ATTR_Y:
		if (!sp_svg_length_read (value, &tspan->ly.y)) {
			sp_svg_length_unset (&tspan->ly.y, SP_SVG_UNIT_NONE, 0.0, 0.0);
		}
		/* fixme: Re-layout it */
		if (tspan->role != SP_TSPAN_ROLE_LINE) sp_object_request_update (object, SP_OBJECT_MODIFIED_FLAG);
		break;
	case SP_ATTR_DX:
		if (!sp_svg_length_read (value, &tspan->ly.dx)) {
			sp_svg_length_unset (&tspan->ly.dx, SP_SVG_UNIT_NONE, 0.0, 0.0);
		}
		/* fixme: Re-layout it */
		break;
	case SP_ATTR_DY:
		if (!sp_svg_length_read (value, &tspan->ly.dy)) {
			sp_svg_length_unset (&tspan->ly.dy, SP_SVG_UNIT_NONE, 0.0, 0.0);
		}
		/* fixme: Re-layout it */
		break;
	case SP_ATTR_ROTATE:
		/* fixme: Implement SVGNumber or something similar (Lauris) */
		tspan->ly.rotate = (value) ? atof (value) : 0.0;
		tspan->ly.rotate_set = (value != NULL);
		/* fixme: Re-layout it */
		break;
	case SP_ATTR_SODIPODI_ROLE:
		if (value && (!strcmp (value, "line") || !strcmp (value, "paragraph"))) {
			tspan->role = SP_TSPAN_ROLE_LINE;
		} else {
			tspan->role = SP_TSPAN_ROLE_UNSPECIFIED;
		}
		break;
	default:
		if (((SPObjectClass *) tspan_parent_class)->set)
			(((SPObjectClass *) tspan_parent_class)->set) (object, key, value);
		break;
	}
}

static void
sp_tspan_child_added (SPObject *object, SPRepr *rch, SPRepr *ref)
{
	SPTSpan *tspan;

	tspan = SP_TSPAN (object);

	if (((SPObjectClass *) tspan_parent_class)->child_added)
		((SPObjectClass *) tspan_parent_class)->child_added (object, rch, ref);

	if (!tspan->string && rch->type == SP_XML_TEXT_NODE) {
		SPString *string;
		/* fixme: We should really pick up first child always */
		string = (SPString*)g_object_new (SP_TYPE_STRING, 0);
		tspan->string = sp_object_attach_reref (object, SP_OBJECT (string), NULL);
		string->ly = &tspan->ly;
		sp_object_invoke_build (tspan->string, SP_OBJECT_DOCUMENT (object), rch, SP_OBJECT_IS_CLONED (object));
	}
}

static void
sp_tspan_remove_child (SPObject *object, SPRepr *rch)
{
	SPTSpan *tspan;

	tspan = SP_TSPAN (object);

	if (((SPObjectClass *) tspan_parent_class)->remove_child)
		((SPObjectClass *) tspan_parent_class)->remove_child (object, rch);

	if (tspan->string && (SP_OBJECT_REPR (tspan->string) == rch)) {
		tspan->string = sp_object_detach_unref (object, tspan->string);
	}
}

static void
sp_tspan_update (SPObject *object, SPCtx *ctx, guint flags)
{
	SPTSpan *tspan;
	SPItemCtx *ictx;
	SPStyle *style;
	double d;

	tspan = SP_TSPAN (object);
	style = SP_OBJECT_STYLE (object);
	ictx = (SPItemCtx *) ctx;

	if (((SPObjectClass *) tspan_parent_class)->update)
		((SPObjectClass *) tspan_parent_class)->update (object, ctx, flags);

	if (flags & SP_OBJECT_MODIFIED_FLAG) flags |= SP_OBJECT_PARENT_MODIFIED_FLAG;
	flags &= SP_OBJECT_MODIFIED_CASCADE;

	/* Update relative distances */
	d = 1.0 / NR_MATRIX_DF_EXPANSION (&ictx->i2vp);
	sp_text_update_length (&tspan->ly.x, style->font_size.computed, style->font_size.computed * 0.5, d);
	sp_text_update_length (&tspan->ly.y, style->font_size.computed, style->font_size.computed * 0.5, d);
	sp_text_update_length (&tspan->ly.dx, style->font_size.computed, style->font_size.computed * 0.5, d);
	sp_text_update_length (&tspan->ly.dy, style->font_size.computed, style->font_size.computed * 0.5, d);

	if (tspan->string) {
		if (flags || (tspan->string->uflags & SP_OBJECT_MODIFIED_FLAG)) {
			sp_object_invoke_update (tspan->string, ctx, flags);
		}
	}
}

static void
sp_tspan_modified (SPObject *object, unsigned int flags)
{
	SPTSpan *tspan;
	SPStyle *style;

	tspan = SP_TSPAN (object);
	style = SP_OBJECT_STYLE (object);

	if (((SPObjectClass *) tspan_parent_class)->modified)
		((SPObjectClass *) tspan_parent_class)->modified (object, flags);

	if (flags & SP_OBJECT_MODIFIED_FLAG) flags |= SP_OBJECT_PARENT_MODIFIED_FLAG;
	flags &= SP_OBJECT_MODIFIED_CASCADE;

	if (tspan->string) {
		if (flags || (tspan->string->mflags & SP_OBJECT_MODIFIED_FLAG)) {
			sp_object_invoke_modified (tspan->string, flags);
		}
	}
}

static SPRepr *
sp_tspan_write (SPObject *object, SPRepr *repr, guint flags)
{
	SPTSpan *tspan;

	tspan = SP_TSPAN (object);

	if ((flags & SP_OBJECT_WRITE_BUILD) && !repr) {
		repr = sp_repr_new ("tspan");
	}

	if (tspan->ly.x.set) sp_repr_set_double_attribute (repr, "x", tspan->ly.x.computed);
	if (tspan->ly.y.set) sp_repr_set_double_attribute (repr, "y", tspan->ly.y.computed);
	if (tspan->ly.dx.set) sp_repr_set_double_attribute (repr, "dx", tspan->ly.dx.computed);
	if (tspan->ly.dy.set) sp_repr_set_double_attribute (repr, "dy", tspan->ly.dy.computed);
	if (tspan->ly.rotate_set) sp_repr_set_double_attribute (repr, "rotate", tspan->ly.rotate);
	if (flags & SP_OBJECT_WRITE_EXT) {
		sp_repr_set_attr (repr, "sodipodi:role", (tspan->role != SP_TSPAN_ROLE_UNSPECIFIED) ? "line" : NULL);
	}

	if (flags & SP_OBJECT_WRITE_BUILD) {
		SPRepr *rstr;
		/* TEXT element */
		rstr = sp_xml_document_createTextNode (sp_repr_document (repr), SP_STRING_TEXT (tspan->string));
		sp_repr_append_child (repr, rstr);
		sp_repr_unref (rstr);
	} else {
		sp_repr_set_content (SP_OBJECT_REPR (tspan->string), SP_STRING_TEXT (tspan->string));
	}

	/* fixme: Strictly speaking, item class write 'transform' too */
	/* fixme: This is harmless as long as tspan affine is identity (lauris) */
	if (((SPObjectClass *) tspan_parent_class)->write)
		((SPObjectClass *) tspan_parent_class)->write (object, repr, flags);

	return repr;
}

static void
sp_tspan_bbox (SPItem *item, NRRect *bbox, const NRMatrix *transform, unsigned int flags)
{
	SPTSpan *tspan;

	tspan = SP_TSPAN (item);

	if (tspan->string) {
		sp_item_invoke_bbox_full (SP_ITEM (tspan->string), bbox, transform, flags, FALSE);
	}
}

static NRArenaItem *
sp_tspan_show (SPItem *item, NRArena *arena, unsigned int key, unsigned int flags)
{
	SPTSpan *tspan;

	tspan = SP_TSPAN (item);

	if (tspan->string) {
		NRArenaItem *ai, *ac;
		ai = nr_arena_item_new (arena, NR_TYPE_ARENA_GROUP);
		nr_arena_group_set_transparent (NR_ARENA_GROUP (ai), FALSE);
		ac = sp_item_invoke_show (SP_ITEM (tspan->string), arena, key, flags);
		if (ac) {
			nr_arena_item_add_child (ai, ac, NULL);
			nr_arena_item_unref (ac);
		}
		return ai;
	}

	return NULL;
}

static void
sp_tspan_hide (SPItem *item, unsigned int key)
{
	SPTSpan *tspan;

	tspan = SP_TSPAN (item);

	if (tspan->string) sp_item_invoke_hide (SP_ITEM (tspan->string), key);

	if (((SPItemClass *) tspan_parent_class)->hide)
		((SPItemClass *) tspan_parent_class)->hide (item, key);
}

static void
sp_tspan_set_shape (SPTSpan *tspan, SPLayoutData *ly, NR::Point &cp, gboolean firstline, gboolean *inspace)
{
	sp_string_set_shape (SP_STRING (tspan->string), &tspan->ly, cp, inspace);
}

/* SPText */

static void sp_text_class_init (SPTextClass *classname);
static void sp_text_init (SPText *text);

static void sp_text_build (SPObject * object, SPDocument * document, SPRepr * repr);
static void sp_text_release (SPObject *object);
static void sp_text_set (SPObject *object, unsigned int key, const gchar *value);
static void sp_text_child_added (SPObject *object, SPRepr *rch, SPRepr *ref);
static void sp_text_remove_child (SPObject *object, SPRepr *rch);
static void sp_text_update (SPObject *object, SPCtx *ctx, guint flags);
static void sp_text_modified (SPObject *object, guint flags);
static SPRepr *sp_text_write (SPObject *object, SPRepr *repr, guint flags);

static void sp_text_bbox (SPItem *item, NRRect *bbox, const NRMatrix *transform, unsigned int flags);
static NRArenaItem *sp_text_show (SPItem *item, NRArena *arena, unsigned int key, unsigned int flags);
static void sp_text_hide (SPItem *item, unsigned int key);
static char * sp_text_description (SPItem *item);
static int sp_text_snappoints (SPItem *item, NRPoint *p, int size);
static void sp_text_write_transform (SPItem *item, SPRepr *repr, NRMatrix *transform);
static void sp_text_print (SPItem *item, SPPrintContext *gpc);

static void sp_text_request_relayout (SPText *text, guint flags);
static void sp_text_update_immediate_state (SPText *text);
static void sp_text_set_shape (SPText *text);

static SPObject *sp_text_get_child_by_position (SPText *text, gint pos);

static SPItemClass *text_parent_class;

GType
sp_text_get_type ()
{
	static GType type = 0;
	if (!type) {
		GTypeInfo info = {
			sizeof (SPTextClass),
			NULL,	/* base_init */
			NULL,	/* base_finalize */
			(GClassInitFunc) sp_text_class_init,
			NULL,	/* class_finalize */
			NULL,	/* class_data */
			sizeof (SPText),
			16,	/* n_preallocs */
			(GInstanceInitFunc) sp_text_init,
			NULL,	/* value_table */
		};
		type = g_type_register_static (SP_TYPE_ITEM, "SPText", &info, (GTypeFlags)0);
	}
	return type;
}

static void
sp_text_class_init (SPTextClass *classname)
{
	SPObjectClass * sp_object_class;
	SPItemClass * item_class;

	sp_object_class = (SPObjectClass *) classname;
	item_class = (SPItemClass *) classname;

	text_parent_class = (SPItemClass*)g_type_class_ref (SP_TYPE_ITEM);

	sp_object_class->build = sp_text_build;
	sp_object_class->release = sp_text_release;
	sp_object_class->set = sp_text_set;
	sp_object_class->child_added = sp_text_child_added;
	sp_object_class->remove_child = sp_text_remove_child;
	sp_object_class->update = sp_text_update;
	sp_object_class->modified = sp_text_modified;
	sp_object_class->write = sp_text_write;

	item_class->bbox = sp_text_bbox;
	item_class->show = sp_text_show;
	item_class->hide = sp_text_hide;
	item_class->description = sp_text_description;
	item_class->snappoints = sp_text_snappoints;
	item_class->write_transform = sp_text_write_transform;
	item_class->print = sp_text_print;
}

static void
sp_text_init (SPText *text)
{
	/* fixme: Initialize layout */
	sp_svg_length_unset (&text->ly.x, SP_SVG_UNIT_NONE, 0.0, 0.0);
	sp_svg_length_unset (&text->ly.y, SP_SVG_UNIT_NONE, 0.0, 0.0);
	sp_svg_length_unset (&text->ly.dx, SP_SVG_UNIT_NONE, 0.0, 0.0);
	sp_svg_length_unset (&text->ly.dy, SP_SVG_UNIT_NONE, 0.0, 0.0);
	text->ly.linespacing = 1.0;
	text->children = NULL;
}

static void
sp_text_build (SPObject *object, SPDocument *doc, SPRepr *repr)
{
	SPText *text;
	SPObject *ref;
	SPRepr *rch;
	SPVersion version;

	text = SP_TEXT (object);

	if (((SPObjectClass *) text_parent_class)->build)
		((SPObjectClass *) text_parent_class)->build (object, doc, repr);
	sp_object_read_attr (object, "x");
	sp_object_read_attr (object, "y");
	sp_object_read_attr (object, "dx");
	sp_object_read_attr (object, "dy");
	sp_object_read_attr (object, "rotate");
	sp_object_read_attr (object, "sodipodi:linespacing");

	version = sp_object_get_sodipodi_version (object);

	if (sp_version_inside_range (version, 0, 0, 0, 25)) {
		const gchar *content;
		for (rch = repr->children; rch != NULL; rch = rch->next) {
			if (rch->type == SP_XML_TEXT_NODE) {
				content = sp_repr_content (rch);
				sp_text_set_repr_text_multiline (text, content);
				break;
			}
		}
	}

	ref = NULL;

	for (rch = repr->children; rch != NULL; rch = rch->next) {
		if (rch->type == SP_XML_TEXT_NODE) {
			SPString *string;
			string = (SPString*)g_object_new (SP_TYPE_STRING, 0);
			if (ref) {
				ref->next = sp_object_attach_reref (object, SP_OBJECT (string), NULL);
			} else {
				text->children = sp_object_attach_reref (object, SP_OBJECT (string), NULL);
			}
			string->ly = &text->ly;
			sp_object_invoke_build (SP_OBJECT (string), doc, rch, SP_OBJECT_IS_CLONED (object));
			ref = SP_OBJECT (string);
		} else if ((rch->type == SP_XML_ELEMENT_NODE) && !strcmp (sp_repr_name (rch), "tspan")) {
			SPObject *child;
			child = (SPObject*)g_object_new (SP_TYPE_TSPAN, 0);
			if (ref) {
				ref->next = sp_object_attach_reref (object, child, NULL);
			} else {
				text->children = sp_object_attach_reref (object, child, NULL);
			}
			sp_object_invoke_build (child, doc, rch, SP_OBJECT_IS_CLONED (object));
			ref = child;
		} else {
			continue;
		}
	}

	sp_text_update_immediate_state (text);
}

static void
sp_text_release (SPObject *object)
{
	SPText *text;

	text = SP_TEXT (object);

	while (text->children) {
		text->children = sp_object_detach_unref (SP_OBJECT (object), text->children);
	}

	if (((SPObjectClass *) text_parent_class)->release)
		((SPObjectClass *) text_parent_class)->release (object);
}

static void
sp_text_set (SPObject *object, unsigned int key, const gchar *value)
{
	SPText *text;

	text = SP_TEXT (object);

	/* fixme: Vectors (Lauris) */
	switch (key) {
	case SP_ATTR_X:
		if (!sp_svg_length_read (value, &text->ly.x)) {
			sp_svg_length_unset (&text->ly.x, SP_SVG_UNIT_NONE, 0.0, 0.0);
		}
		sp_object_request_update (object, SP_OBJECT_MODIFIED_FLAG | SP_TEXT_LAYOUT_MODIFIED_FLAG);
		break;
	case SP_ATTR_Y:
		if (!sp_svg_length_read (value, &text->ly.y)) {
			sp_svg_length_unset (&text->ly.y, SP_SVG_UNIT_NONE, 0.0, 0.0);
		}
		sp_object_request_update (object, SP_OBJECT_MODIFIED_FLAG | SP_TEXT_LAYOUT_MODIFIED_FLAG);
		break;
	case SP_ATTR_DX:
		if (!sp_svg_length_read (value, &text->ly.dx)) {
			sp_svg_length_unset (&text->ly.dx, SP_SVG_UNIT_NONE, 0.0, 0.0);
		}
		/* fixme: Re-layout it */
		break;
	case SP_ATTR_DY:
		if (!sp_svg_length_read (value, &text->ly.dy)) {
			sp_svg_length_unset (&text->ly.dy, SP_SVG_UNIT_NONE, 0.0, 0.0);
		}
		/* fixme: Re-layout it */
		break;
	case SP_ATTR_ROTATE:
		/* fixme: Implement SVGNumber or something similar (Lauris) */
		text->ly.rotate = (value) ? atof (value) : 0.0;
		text->ly.rotate_set = (value != NULL);
		/* fixme: Re-layout it */
		break;
	case SP_ATTR_SODIPODI_LINESPACING:
		text->ly.linespacing = 1.0;
		if (value) {
			text->ly.linespacing = sp_svg_read_percentage (value, 1.0);
			text->ly.linespacing = CLAMP (text->ly.linespacing, 0.01, 10.0);
		}
		sp_object_request_update (object, SP_OBJECT_MODIFIED_FLAG | SP_TEXT_LAYOUT_MODIFIED_FLAG);
		break;
	default:
		if (((SPObjectClass *) text_parent_class)->set)
			((SPObjectClass *) text_parent_class)->set (object, key, value);
		break;
	}
}

static void
sp_text_child_added (SPObject *object, SPRepr *rch, SPRepr *ref)
{
	SPText *text;
	SPItem *item;
	SPObject *och, *prev;

	item = SP_ITEM (object);
	text = SP_TEXT (object);

	if (((SPObjectClass *) text_parent_class)->child_added)
		((SPObjectClass *) text_parent_class)->child_added (object, rch, ref);

	/* Search for position reference */
	prev = NULL;

	if (ref != NULL) {
		prev = text->children;
		while (prev && (prev->repr != ref)) prev = prev->next;
	}

	if (rch->type == SP_XML_TEXT_NODE) {
		SPString *string;
		string = (SPString*)g_object_new (SP_TYPE_STRING, 0);
		if (prev) {
			prev->next = sp_object_attach_reref (object, SP_OBJECT (string), prev->next);
		} else {
			text->children = sp_object_attach_reref (object, SP_OBJECT (string), text->children);
		}
		string->ly = &text->ly;
		sp_object_invoke_build (SP_OBJECT (string), SP_OBJECT_DOCUMENT (object), rch, SP_OBJECT_IS_CLONED (object));
		och = SP_OBJECT (string);
	} else if ((rch->type == SP_XML_ELEMENT_NODE) && !strcmp (sp_repr_name (rch), "tspan")) {
		SPObject *child;
		child = (SPObject*)g_object_new (SP_TYPE_TSPAN, 0);
		if (prev) {
			prev->next = sp_object_attach_reref (object, child, prev->next);
		} else {
			text->children = sp_object_attach_reref (object, child, text->children);
		}
		sp_object_invoke_build (child, SP_OBJECT_DOCUMENT (object), rch, SP_OBJECT_IS_CLONED (object));
		och = child;
	} else {
		och = NULL;
	}

	if (och) {
		SPItemView *v;
		NRArenaItem *ac;

		for (v = item->display; v != NULL; v = v->next) {
			ac = sp_item_invoke_show (SP_ITEM (och), NR_ARENA_ITEM_ARENA (v->arenaitem), v->key, v->flags);
			if (ac) {
				nr_arena_item_add_child (v->arenaitem, ac, NULL);
				nr_arena_item_unref (ac);
			}
		}
	}

	sp_text_request_relayout (text, SP_OBJECT_MODIFIED_FLAG | SP_TEXT_CONTENT_MODIFIED_FLAG);
	/* fixme: Instead of forcing it, do it when needed */
	sp_text_update_immediate_state (text);
}

static void
sp_text_remove_child (SPObject *object, SPRepr *rch)
{
	SPText *text;
	SPObject *prev, *och;

	text = SP_TEXT (object);

	if (((SPObjectClass *) text_parent_class)->remove_child)
		((SPObjectClass *) text_parent_class)->remove_child (object, rch);

	prev = NULL;
	och = text->children;
	while (och->repr != rch) {
		prev = och;
		och = och->next;
	}

	if (prev) {
		prev->next = sp_object_detach_unref (object, och);
	} else {
		text->children = sp_object_detach_unref (object, och);
	}

	sp_text_request_relayout (text, SP_OBJECT_MODIFIED_FLAG | SP_TEXT_CONTENT_MODIFIED_FLAG);
	sp_text_update_immediate_state (text);
}

/* fixme: This is wrong, as we schedule relayout every time something changes */

static void
sp_text_update (SPObject *object, SPCtx *ctx, guint flags)
{
	SPItemCtx *ictx;
	SPObject *child;
	GSList *l;

	SPText *text = SP_TEXT (object);
	SPStyle *style = SP_OBJECT_STYLE (text);
	ictx = (SPItemCtx *) ctx;

	if (((SPObjectClass *) text_parent_class)->update)
		((SPObjectClass *) text_parent_class)->update (object, ctx, flags);

	guint cflags = (flags & SP_OBJECT_MODIFIED_CASCADE);
	if (flags & SP_OBJECT_MODIFIED_FLAG) cflags |= SP_OBJECT_PARENT_MODIFIED_FLAG;

	/* Update relative distances */
	double d = 1.0 / NR_MATRIX_DF_EXPANSION (&ictx->i2vp);
	sp_text_update_length (&text->ly.x, style->font_size.computed, style->font_size.computed * 0.5, d);
	sp_text_update_length (&text->ly.y, style->font_size.computed, style->font_size.computed * 0.5, d);
	sp_text_update_length (&text->ly.dx, style->font_size.computed, style->font_size.computed * 0.5, d);
	sp_text_update_length (&text->ly.dy, style->font_size.computed, style->font_size.computed * 0.5, d);

	/* Create temporary list of children */
	l = NULL;
	for (child = text->children; child != NULL; child = child->next) {
		sp_object_ref (SP_OBJECT (child), object);
		l = g_slist_prepend (l, child);
	}
	l = g_slist_reverse (l);
	while (l) {
		child = SP_OBJECT (l->data);
		l = g_slist_remove (l, child);
		if (cflags || (child->uflags & (SP_OBJECT_MODIFIED_FLAG | SP_OBJECT_CHILD_MODIFIED_FLAG))) {
			/* fixme: Do we need transform? */
			sp_object_invoke_update (child, ctx, cflags);
		}
		sp_object_unref (SP_OBJECT (child), object);
	}
	if (text->relayout || (flags & (SP_OBJECT_STYLE_MODIFIED_FLAG | SP_OBJECT_CHILD_MODIFIED_FLAG | SP_TEXT_LAYOUT_MODIFIED_FLAG))) {
		/* fixme: It is not nice to have it here, but otherwise children content changes does not work */
		/* fixme: Even now it may not work, as we are delayed */
		/* fixme: So check modification flag everywhere immediate state is used */
		sp_text_update_immediate_state (text);
		sp_text_set_shape (text);
		text->relayout = FALSE;
	}
}

static void
sp_text_modified (SPObject *object, guint flags)
{
	SPText *text;
	SPObject *child;
	GSList *l;
	guint cflags;

	text = SP_TEXT (object);

	if (((SPObjectClass *) text_parent_class)->modified)
		((SPObjectClass *) text_parent_class)->modified (object, flags);

	cflags = (flags & SP_OBJECT_MODIFIED_CASCADE);
	if (flags & SP_OBJECT_MODIFIED_FLAG) cflags |= SP_OBJECT_PARENT_MODIFIED_FLAG;

	/* Create temporary list of children */
	l = NULL;
	for (child = text->children; child != NULL; child = child->next) {
		sp_object_ref (SP_OBJECT (child), object);
		l = g_slist_prepend (l, child);
	}
	l = g_slist_reverse (l);
	while (l) {
		child = SP_OBJECT (l->data);
		l = g_slist_remove (l, child);
		if (cflags || (child->mflags & (SP_OBJECT_MODIFIED_FLAG | SP_OBJECT_CHILD_MODIFIED_FLAG))) {
			sp_object_invoke_modified (child, cflags);
		}
		sp_object_unref (SP_OBJECT (child), object);
	}
}

static SPRepr *
sp_text_write (SPObject *object, SPRepr *repr, guint flags)
{
	SPText *text;
	SPObject *child;
	SPRepr *crepr;

	text = SP_TEXT (object);

	if (flags & SP_OBJECT_WRITE_BUILD) {
		GSList *l;
		if (!repr) repr = sp_repr_new ("text");
		l = NULL;
		for (child = text->children; child != NULL; child = child->next) {
			if (SP_IS_TSPAN (child)) {
				crepr = sp_object_invoke_write (child, NULL, flags);
				if (crepr) l = g_slist_prepend (l, crepr);
			} else {
				crepr = sp_xml_document_createTextNode (sp_repr_document (repr), SP_STRING_TEXT (child));
			}
		}
		while (l) {
			sp_repr_add_child (repr, (SPRepr *) l->data, NULL);
			sp_repr_unref ((SPRepr *) l->data);
			l = g_slist_remove (l, l->data);
		}
	} else {
		for (child = text->children; child != NULL; child = child->next) {
			if (SP_IS_TSPAN (child)) {
				sp_object_invoke_write (child, SP_OBJECT_REPR (child), flags);
			} else {
				sp_repr_set_content (SP_OBJECT_REPR (child), SP_STRING_TEXT (child));
			}
		}
	}

	if (text->ly.x.set) sp_repr_set_double_attribute (repr, "x", text->ly.x.computed);
	if (text->ly.y.set) sp_repr_set_double_attribute (repr, "y", text->ly.y.computed);
	if (text->ly.dx.set) sp_repr_set_double_attribute (repr, "dx", text->ly.dx.computed);
	if (text->ly.dy.set) sp_repr_set_double_attribute (repr, "dy", text->ly.dy.computed);
	if (text->ly.rotate_set) sp_repr_set_double_attribute (repr, "rotate", text->ly.rotate);

	if (((SPObjectClass *) (text_parent_class))->write)
		((SPObjectClass *) (text_parent_class))->write (object, repr, flags);

	return repr;
}

static void
sp_text_bbox (SPItem *item, NRRect *bbox, const NRMatrix *transform, unsigned int flags)
{
	SPText *text;
	SPItem *child;
	SPObject *o;

	text = SP_TEXT (item);

	for (o = text->children; o != NULL; o = o->next) {
		NRMatrix a;
		child = SP_ITEM (o);
		nr_matrix_multiply (&a, &child->transform, transform);
		sp_item_invoke_bbox_full (child, bbox, &a, flags, FALSE);
	}
}

static NRArenaItem *
sp_text_show (SPItem *item, NRArena *arena, unsigned int key, unsigned int flags)
{
	SPText *text;
	NRArenaItem *ai, *ac, *ar;
	SPItem * child;
	SPObject * o;

	text = SP_TEXT (item);

	ai = nr_arena_item_new (arena, NR_TYPE_ARENA_GROUP);
	nr_arena_group_set_transparent (NR_ARENA_GROUP (ai), FALSE);

	ar = NULL;
	for (o = text->children; o != NULL; o = o->next) {
		if (SP_IS_ITEM (o)) {
			child = SP_ITEM (o);
			ac = sp_item_invoke_show (child, arena, key, flags);
			if (ac) {
				nr_arena_item_add_child (ai, ac, ar);
				ar = ac;
				nr_arena_item_unref (ac);
			}
		}
	}

	return ai;
}

static void
sp_text_hide (SPItem *item, unsigned int key)
{
	SPText *text;
	SPItem * child;
	SPObject * o;

	text = SP_TEXT (item);

	for (o = text->children; o != NULL; o = o->next) {
		if (SP_IS_ITEM (o)) {
			child = SP_ITEM (o);
			sp_item_invoke_hide (child, key);
		}
	}

	if (((SPItemClass *) text_parent_class)->hide)
		((SPItemClass *) text_parent_class)->hide (item, key);
}

static char *
sp_text_description (SPItem * item)
{
	SPText *text;
	SPStyle *style;
	NRTypeFace *tf;
	NRFont *font;
	gchar n[256];

	text = (SPText *) item;
	style = SP_OBJECT_STYLE (text);

	tf = nr_type_directory_lookup_fuzzy(style->text->font_family.value,
						    font_style_to_pos(*style));
	font = nr_font_new_default (tf, NR_TYPEFACE_METRICS_HORIZONTAL, style->font_size.computed);
	nr_typeface_name_get (NR_FONT_TYPEFACE (font), n, 256);
	nr_typeface_unref (tf);

	return g_strdup_printf (_("Text (%s, %.5gpt)"), n, NR_FONT_SIZE (font));
}


/* fixme: Do text chunks here (Lauris) */
/* fixme: We'll remove string bbox adjustment and bring it here for the whole chunk (Lauris) */

static void
sp_text_set_shape (SPText *text)
{
	NR::Point cp;
	SPObject *child;
	gboolean isfirstline, inspace;
	NRRect paintbox;

	/* The logic should be: */
	/* 1. Calculate attributes */
	/* 2. Iterate through children asking them to set shape */

	cp[NR::X] = text->ly.x.computed;
	cp[NR::Y] = text->ly.y.computed;

	isfirstline = TRUE;
	inspace = FALSE;

	child = text->children;
	while (child != NULL) {
		SPObject *next, *spnew;
		SPString *string;
		ArtDRect bbox;
		if (SP_IS_TSPAN (child)) {
			SPTSpan *tspan;
			/* fixme: Maybe break this up into 2 pieces - relayout and set shape (Lauris) */
			tspan = SP_TSPAN (child);
			string = SP_TSPAN_STRING (tspan);
			switch (tspan->role) {
			case SP_TSPAN_ROLE_PARAGRAPH:
			case SP_TSPAN_ROLE_LINE:
				if (!isfirstline) {
					if (child->style->writing_mode.computed == SP_CSS_WRITING_MODE_TB) {
						cp[NR::X] -= child->style->font_size.computed * text->ly.linespacing;
						cp[NR::Y] = text->ly.y.computed;
					} else {
						cp[NR::X] = text->ly.x.computed;
						cp[NR::Y] += child->style->font_size.computed * text->ly.linespacing;
					}
				}
				/* fixme: This is extremely (EXTREMELY) dangerous (Lauris) */
				/* fixme: Our only hope is to ensure LINE tspans do not request ::modified */
				sp_document_set_undo_sensitive (SP_OBJECT_DOCUMENT (tspan), FALSE);
				sp_repr_set_double (SP_OBJECT_REPR (tspan), "x", cp[NR::X]);
				sp_repr_set_double (SP_OBJECT_REPR (tspan), "y", cp[NR::Y]);
				sp_document_set_undo_sensitive (SP_OBJECT_DOCUMENT (tspan), TRUE);
				break;
			case SP_TSPAN_ROLE_UNSPECIFIED:
				if (tspan->ly.x.set) {
					cp[NR::X] = tspan->ly.x.computed;
				} else {
					tspan->ly.x.computed = cp[NR::X];
				}
				if (tspan->ly.y.set) {
					cp[NR::Y] = tspan->ly.y.computed;
				} else {
					tspan->ly.y.computed = cp[NR::Y];
				}
				if (tspan->ly.dx.set) {
					cp[NR::X] += tspan->ly.dx.computed;
				}
				if (tspan->ly.dy.set) {
					cp[NR::Y] += tspan->ly.dy.computed;
				}
				break;
			default:
				/* Error */
				break;
			}
		} else {
			string = SP_STRING (child);
		}
		/* Calculate block bbox */
		NR::Point advance = NR::Point((string->ly->dx.set) ? string->ly->dx.computed : 0.0,
					      (string->ly->dy.set) ? string->ly->dy.computed : 0.0);
		bbox.x0 = string->bbox.x0 + advance[NR::X];
		bbox.y0 = string->bbox.y0 + advance[NR::Y];
		bbox.x1 = string->bbox.x1 + advance[NR::X];
		bbox.y1 = string->bbox.y1 + advance[NR::Y];
		advance[NR::X] += string->advance[NR::X];
		advance[NR::Y] += string->advance[NR::Y];
		for (next = child->next; next != NULL; next = next->next) {
			SPString *string;
			if (SP_IS_TSPAN (next)) {
				SPTSpan *tspan;
				tspan = SP_TSPAN (next);
				if (tspan->role != SP_TSPAN_ROLE_UNSPECIFIED) break;
				if ((tspan->ly.x.set) || (tspan->ly.y.set)) break;
				if (tspan->ly.dx.set) advance[NR::X] += tspan->ly.dx.computed;
				if (tspan->ly.dy.set) advance[NR::Y] += tspan->ly.dy.computed;
				string = SP_TSPAN_STRING (tspan);
			} else {
				string = SP_STRING (next);
			}
			bbox.x0 = MIN (bbox.x0, string->bbox.x0 + advance[NR::X]);
			bbox.y0 = MIN (bbox.y0, string->bbox.y0 + advance[NR::Y]);
			bbox.x1 = MAX (bbox.x1, string->bbox.x1 + advance[NR::X]);
			bbox.y1 = MAX (bbox.y1, string->bbox.y1 + advance[NR::Y]);
			advance[NR::X] += string->advance[NR::X];
			advance[NR::Y] += string->advance[NR::Y];
		}
		spnew = next;
		/* Calculate starting position */
		switch (child->style->text_anchor.computed) {
		case SP_CSS_TEXT_ANCHOR_START:
			break;
		case SP_CSS_TEXT_ANCHOR_MIDDLE:
			/* Ink midpoint */
			if (child->style->writing_mode.computed == SP_CSS_WRITING_MODE_TB) {
				cp[NR::Y] -= (bbox.y0 + bbox.y1) / 2;
			} else {
				cp[NR::X] -= (bbox.x0 + bbox.x1) / 2;
			}
			break;
		case SP_CSS_TEXT_ANCHOR_END:
			/* Ink endpoint */
			if (child->style->writing_mode.computed == SP_CSS_WRITING_MODE_TB) {
				cp[NR::Y] -= bbox.y1;
			} else {
				cp[NR::X] -= bbox.x1;
			}
			break;
		default:
			break;
		}
		/* Set child shapes */
		for (next = child; (next != NULL) && (next != spnew); next = next->next) {
			if (SP_IS_STRING (next)) {
				sp_string_set_shape (SP_STRING (next), &text->ly, cp, &inspace);
			} else {
				SPTSpan *tspan;
				tspan = SP_TSPAN (next);
				if (tspan->ly.dx.set) cp[NR::X] += tspan->ly.dx.computed;
				if (tspan->ly.dy.set) cp[NR::Y] += tspan->ly.dy.computed;
				sp_tspan_set_shape (tspan, &text->ly, cp, isfirstline, &inspace);
			}
		}
		child = next;
		isfirstline = FALSE;
	}

	sp_item_invoke_bbox (SP_ITEM (text), &paintbox, NULL, TRUE);

	for (child = text->children; child != NULL; child = child->next) {
		SPString *string;
		if (SP_IS_TSPAN (child)) {
			string = SP_TSPAN_STRING (child);
		} else {
			string = SP_STRING (child);
		}
		sp_chars_set_paintbox (SP_CHARS (string), &paintbox);
	}
}

static int
sp_text_snappoints (SPItem *item, NRPoint *p, int size)
{
	/* we use corners of item and x,y coordinates of ellipse */
	int pos = 0;
	if (((SPItemClass *) text_parent_class)->snappoints)
		pos = ((SPItemClass *) text_parent_class)->snappoints (item, p, size);

	if (pos < size) {
		SPLayoutData *ly = &SP_TEXT (item)->ly;

		p[pos] = sp_item_i2d_affine (item)* NR::Point(ly->x.computed, ly->y.computed);
	}

	return pos;
}

/*
 * Initially we'll do:
 * Transform x, y, set x, y, clear translation
 */

static void
sp_text_write_transform (SPItem *item, SPRepr *repr, NRMatrix *t)
{
	SPText *text;
	NRMatrix i2p, p2i;
	gdouble px, py, x, y;
	SPObject *child;
	gchar c[120];

	text = SP_TEXT (item);

	i2p = *t;
	// translate?
	i2p.c[4] = 0.0;
	i2p.c[5] = 0.0;
	nr_matrix_invert (&p2i, &i2p);

	px = NR_MATRIX_DF_TRANSFORM_X (t, text->ly.x.computed, text->ly.y.computed);
	py = NR_MATRIX_DF_TRANSFORM_Y (t, text->ly.x.computed, text->ly.y.computed);
	x = NR_MATRIX_DF_TRANSFORM_X (&p2i, px, py);
	y = NR_MATRIX_DF_TRANSFORM_Y (&p2i, px, py);
	sp_repr_set_double (repr, "x", x);
	sp_repr_set_double (repr, "y", y);
	for (child = text->children; child != NULL; child = child->next) {
		if (SP_IS_TSPAN (child)) {
			SPTSpan *tspan;
			tspan = SP_TSPAN (child);
			if (tspan->ly.x.set || tspan->ly.y.set) {
				x = (tspan->ly.x.set) ? tspan->ly.x.computed : text->ly.x.computed;
				y = (tspan->ly.y.set) ? tspan->ly.y.computed : text->ly.y.computed;
				px = NR_MATRIX_DF_TRANSFORM_X (t, x, y);
				py = NR_MATRIX_DF_TRANSFORM_Y (t, x, y);
				x = NR_MATRIX_DF_TRANSFORM_X (&p2i, px, py);
				y = NR_MATRIX_DF_TRANSFORM_Y (&p2i, px, py);
				/* fixme: This is wrong - what if repr != SP_OBJECT_REPR (text) */
				sp_repr_set_double (SP_OBJECT_REPR (tspan), "x", x);
				sp_repr_set_double (SP_OBJECT_REPR (tspan), "y", y);
			}
		}
	}

	if (sp_svg_transform_write (c, 80, &i2p)) {
		sp_repr_set_attr (repr, "transform", c);
	} else {
		sp_repr_set_attr (repr, "transform", NULL);
	}
}

static void
sp_text_print (SPItem *item, SPPrintContext *ctx)
{
	SPText *text;
	SPObject *ch;
	NRMatrix ctm;
	NRRect pbox, dbox, bbox;

	text = SP_TEXT (item);

	/* fixme: Think (Lauris) */
	sp_item_invoke_bbox (item, &pbox, NULL, TRUE);
	sp_item_bbox_desktop (item, &bbox);
	dbox.x0 = 0.0;
	dbox.y0 = 0.0;
	dbox.x1 = sp_document_width (SP_OBJECT_DOCUMENT (item));
	dbox.y1 = sp_document_height (SP_OBJECT_DOCUMENT (item));
	sp_item_i2d_affine (item, &ctm);

	for (ch = text->children; ch != NULL; ch = ch->next) {
		if (SP_IS_TSPAN (ch)) {
			sp_chars_do_print (SP_CHARS (SP_TSPAN (ch)->string), ctx, &ctm, &pbox, &dbox, &bbox);
		} else if (SP_IS_STRING (ch)) {
			sp_chars_do_print (SP_CHARS (ch), ctx, &ctm, &pbox, &dbox, &bbox);
		}
	}
}

int
sp_text_is_empty (SPText *text)
{
	SPObject *ch;

	for (ch = text->children; ch != NULL; ch = ch->next) {
		SPString *str;
		gchar *p;
		str = SP_TEXT_CHILD_STRING (ch);
		for (p = str->text; p && *p; p = g_utf8_next_char (p)) {
			gunichar unival;
			unival = g_utf8_get_char (p);
			if ((unival > 0xe000) && (unival <= 0xf8ff)) return FALSE;
			if (g_unichar_isgraph (unival)) return FALSE;
		}
	}

	return TRUE;
}

gchar *
sp_text_get_string_multiline (SPText *text)
{
	SPObject *ch;
	GSList *strs, *l;
	gint len;
	gchar *str, *p;

	strs = NULL;
	for (ch = text->children; ch != NULL; ch = ch->next) {
		if (SP_IS_TSPAN (ch)) {
			SPTSpan *tspan;
			tspan = SP_TSPAN (ch);
			if (tspan->string && SP_STRING (tspan->string)->text) {
				strs = g_slist_prepend (strs, SP_STRING (tspan->string)->text);
			}
		} else if (SP_IS_STRING (ch) && SP_STRING (ch)->text) {
			strs = g_slist_prepend (strs, SP_STRING (ch)->text);
		} else {
			continue;
		}
	}

	len = 0;
	for (l = strs; l != NULL; l = l->next) {
		len += strlen ((const gchar*)l->data);
		len += strlen ("\n");
	}

	len += 1;

	strs = g_slist_reverse (strs);

	str = g_new (gchar, len);
	p = str;
	while (strs) {
		memcpy (p, strs->data, strlen ((const gchar*)strs->data));
		p += strlen ((const gchar*)strs->data);
		strs = g_slist_remove (strs, strs->data);
		if (strs) *p++ = '\n';
	}
	*p++ = '\0';

	return str;
}

void
sp_text_set_repr_text_multiline (SPText *text, const gchar *str)
{
	SPRepr *repr;
	SPStyle *style;
	gchar *content, *p;
	NR::Point cp;

	g_return_if_fail (text != NULL);
	g_return_if_fail (SP_IS_TEXT (text));

	repr = SP_OBJECT_REPR (text);
	style = SP_OBJECT_STYLE (text);

	if (!str) str = "";
	content = g_strdup (str);

	sp_repr_set_content (SP_OBJECT_REPR (text), NULL);
	while (repr->children) {
		sp_repr_remove_child (repr, repr->children);
	}

	p = content;

	cp[NR::X] = text->ly.x.computed;
	cp[NR::Y] = text->ly.y.computed;

	while (p) {
		SPRepr *rtspan, *rstr;
		gchar *e;
		e = strchr (p, '\n');
		if (e) *e = '\0';
		rtspan = sp_repr_new ("tspan");
		sp_repr_set_double (rtspan, "x", cp[NR::X]);
		sp_repr_set_double (rtspan, "y", cp[NR::Y]);
		if (style->writing_mode.computed == SP_CSS_WRITING_MODE_TB) {
			/* fixme: real line height */
			/* fixme: What to do with mixed direction tspans? */
			cp[NR::X] -= style->font_size.computed;
		} else {
			cp[NR::Y] += style->font_size.computed;
		}
		sp_repr_set_attr (rtspan, "sodipodi:role", "line");
		rstr = sp_xml_document_createTextNode (sp_repr_document (repr), p);
		sp_repr_add_child (rtspan, rstr, NULL);
		sp_repr_append_child (repr, rtspan);
		p = (e) ? e + 1 : NULL;
	}

	g_free (content);

	/* fixme: Calculate line positions (Lauris) */
}

SPCurve *
sp_text_normalized_bpath (SPText *text)
{
	SPObject *child;
	GSList *cc;
	SPCurve *curve;

	g_return_val_if_fail (text != NULL, NULL);
	g_return_val_if_fail (SP_IS_TEXT (text), NULL);

	cc = NULL;
	for (child = text->children; child != NULL; child = child->next) {
		SPCurve *c;
		if (SP_IS_STRING (child)) {
			c = sp_chars_normalized_bpath (SP_CHARS (child));
		} else if (SP_IS_TSPAN (child)) {
			SPTSpan *tspan;
			tspan = SP_TSPAN (child);
			c = sp_chars_normalized_bpath (SP_CHARS (tspan->string));
		} else {
			c = NULL;
		}
		if (c) cc = g_slist_prepend (cc, c);
	}

	cc = g_slist_reverse (cc);

  if ( cc ) {
    curve = sp_curve_concat (cc);
  } else {
    curve=sp_curve_new();
  }
  
	while (cc) {
		sp_curve_unref ((SPCurve *) cc->data);
		cc = g_slist_remove (cc, cc->data);
	}

	return curve;
}

static void
sp_text_update_immediate_state (SPText *text)
{
	SPObject *child;
	guint start;

	start = 0;
	for (child = text->children; child != NULL; child = child->next) {
		SPString *string;
		if (SP_IS_TSPAN (child)) {
			string = SP_TSPAN_STRING (child);
		} else {
			string = SP_STRING (child);
		}
		string->start = start;
		string->length = (string->text) ? g_utf8_strlen (string->text, -1) : 0;
		start += string->length;
		/* Count newlines as well */
		if (child->next) start += 1;
	}
}

static void
sp_text_request_relayout (SPText *text, guint flags)
{
	text->relayout = TRUE;

	sp_object_request_update (SP_OBJECT (text), flags);
}

/* fixme: Think about these (Lauris) */

gint
sp_text_get_length (SPText *text)
{
	SPObject *child;
	SPString *string;

	g_return_val_if_fail (text != NULL, 0);
	g_return_val_if_fail (SP_IS_TEXT (text), 0);

	if (!text->children) return 0;

	child = text->children;
	while (child->next) child = child->next;

	if (SP_IS_STRING (child)) {
		string = SP_STRING (child);
	} else {
		string = SP_TSPAN_STRING (child);
	}

	return string->start + string->length;
}

SPTSpan *
sp_text_append_line (SPText *text)
{
	SPRepr *rtspan, *rstring;
	SPObject *child;
	SPStyle *style;
	NR::Point cp;

	g_return_val_if_fail (text != NULL, NULL);
	g_return_val_if_fail (SP_IS_TEXT (text), NULL);

	style = SP_OBJECT_STYLE (text);

	cp[NR::X] = text->ly.x.computed;
	cp[NR::Y] = text->ly.y.computed;

	for (child = text->children; child != NULL; child = child->next) {
		if (SP_IS_TSPAN (child)) {
			SPTSpan *tspan;
			tspan = SP_TSPAN (child);
			if (tspan->role == SP_TSPAN_ROLE_LINE) {
				cp[NR::X] = tspan->ly.x.computed;
				cp[NR::Y] = tspan->ly.y.computed;
			}
		}
	}

	/* Create <tspan> */
	rtspan = sp_repr_new ("tspan");
	if (style->writing_mode.computed == SP_CSS_WRITING_MODE_TB) {
		/* fixme: real line height */
		/* fixme: What to do with mixed direction tspans? */
		sp_repr_set_double (rtspan, "x", cp[NR::X] - style->font_size.computed);
		sp_repr_set_double (rtspan, "y", cp[NR::Y]);
	} else {
		sp_repr_set_double (rtspan, "x", cp[NR::X]);
		sp_repr_set_double (rtspan, "y", cp[NR::Y] + style->font_size.computed);
	}
	sp_repr_set_attr (rtspan, "sodipodi:role", "line");

	/* Create TEXT */
	rstring = sp_xml_document_createTextNode (sp_repr_document (rtspan), "");
	sp_repr_add_child (rtspan, rstring, NULL);
	sp_repr_unref (rstring);
	/* Append to text */
	sp_repr_append_child (SP_OBJECT_REPR (text), rtspan);
	sp_repr_unref (rtspan);

	return (SPTSpan *) sp_document_lookup_id (SP_OBJECT_DOCUMENT (text), sp_repr_attr (rtspan, "id"));
}

SPTSpan *
sp_text_insert_line (SPText *text, gint pos)
{
	SPObject *child;
	SPString *string;
	SPRepr *rtspan, *rstring;
	gchar *ip;

	g_return_val_if_fail (text != NULL, NULL);
	g_return_val_if_fail (SP_IS_TEXT (text), NULL);
	g_return_val_if_fail (pos >= 0, NULL);

	child = sp_text_get_child_by_position (text, pos);
	string = SP_TEXT_CHILD_STRING (child);

	/* Create <tspan> */
	rtspan = sp_repr_new ("tspan");
	sp_repr_set_attr (rtspan, "sodipodi:role", "line");
	/* Create TEXT */
	rstring = sp_xml_document_createTextNode (sp_repr_document (rtspan), "");
	sp_repr_add_child (rtspan, rstring, NULL);
	sp_repr_unref (rstring);

	sp_repr_add_child (SP_OBJECT_REPR (text), rtspan, SP_OBJECT_REPR (child));
	sp_repr_unref (rtspan);

	if (string->text) {
		ip = g_utf8_offset_to_pointer (string->text, pos - string->start);
		sp_repr_set_content (rstring, ip);
		*ip = '\0';
		sp_repr_set_content (SP_OBJECT_REPR (string), string->text);
	}

	return (SPTSpan *) sp_document_lookup_id (SP_OBJECT_DOCUMENT (text), sp_repr_attr (rtspan, "id"));
}

gint
sp_text_append (SPText *text, const gchar *utf8)
{
	SPObject *child;
	SPString *string;
	const gchar *content;
	gint clen, ulen, cchars, uchars;
	gchar b[1024], *p;

	g_return_val_if_fail (text != NULL, -1);
	g_return_val_if_fail (SP_IS_TEXT (text), -1);
	g_return_val_if_fail (utf8 != NULL, -1);

	if (!text->children) {
		SPRepr *rtspan, *rstring;
		/* Create <tspan> */
		rtspan = sp_repr_new ("tspan");
		/* Create TEXT */
		rstring = sp_xml_document_createTextNode (sp_repr_document (rtspan), "");
		sp_repr_add_child (rtspan, rstring, NULL);
		sp_repr_unref (rstring);
		/* Add string */
		sp_repr_add_child (SP_OBJECT_REPR (text), rtspan, NULL);
		sp_repr_unref (rtspan);
		g_assert (text->children);
	}

	child = text->children;
	while (child->next) child = child->next;
	if (SP_IS_STRING (child)) {
		string = SP_STRING (child);
	} else {
		string = SP_TSPAN_STRING (child);
	}
		
	content = sp_repr_content (SP_OBJECT_REPR (string));

	clen = (content) ? strlen (content) : 0;
	cchars = (content) ? g_utf8_strlen (content, clen) : 0;

	ulen = (utf8) ? strlen (utf8) : 0;
	uchars = (utf8) ? g_utf8_strlen (utf8, ulen) : 0;

	if (ulen < 1) return cchars;

	if ((clen + ulen) < 1024) {
		p = b;
	} else {
		p = g_new (gchar, clen + ulen + 1);
	}

	if (clen > 0) memcpy (p, content, clen);
	if (ulen > 0) memcpy (p + clen, utf8, ulen);
	*(p + clen + ulen) = '\0';

	sp_repr_set_content (SP_OBJECT_REPR (string), p);

	if (p != b) g_free (p);

	return string->start + cchars + uchars;
}

/* Returns position after inserted */

gint
sp_text_insert (SPText *text, gint pos, const gchar *utf8)
{
	SPObject *child;
	SPString *string;
	gchar *spnew, *ip;
	int slen, ulen;
	gunichar u; 

	g_return_val_if_fail (text != NULL, -1);
	g_return_val_if_fail (SP_IS_TEXT (text), -1);
	g_return_val_if_fail (pos >= 0, -1);

	if (!utf8) return pos;
	if (!*utf8) return pos;

	u = g_utf8_get_char (utf8);
	if (u == (gunichar) -1) {
		g_warning ("Bad UTF-8 character"); // this can only happen due to a bug, so it goes to the console
		return pos;
	}

	child = sp_text_get_child_by_position (text, pos);
	if (!child) return sp_text_append (text, utf8);
	string = SP_TEXT_CHILD_STRING (child);

	ip = g_utf8_offset_to_pointer (string->text, pos - string->start);

	slen = ip - string->text;
	ulen = strlen (utf8);

	spnew = g_new (gchar, strlen (string->text) + ulen + 1);
	/* Copy start */
	memcpy (spnew, string->text, slen);
	/* Copy insertion */
	memcpy (spnew + slen, utf8, ulen);
	/* Copy end */
	strcpy (spnew + slen + ulen, ip);
	sp_repr_set_content (SP_OBJECT_REPR (string), spnew);
	g_free (spnew);

	return pos + g_utf8_strlen (utf8, -1);
}

/* Returns start position */

gint 
sp_text_delete (SPText *text, gint start, gint end)
{
	SPObject *schild, *echild;

	g_return_val_if_fail (text != NULL, -1);
	g_return_val_if_fail (SP_IS_TEXT (text), -1);
	g_return_val_if_fail (start >= 0, -1);
	g_return_val_if_fail (end >= start, -1);

	if (!text->children) return 0;
	if (start == end) return start;

	schild = sp_text_get_child_by_position (text, start);
	echild = sp_text_get_child_by_position (text, end);

	if (schild != echild) {
		SPString *sstring, *estring;
		SPObject *child;
		gchar *utf8, *sp, *ep;
		GSList *cl;
		/* Easy case */
		sstring = SP_TEXT_CHILD_STRING (schild);
		estring = SP_TEXT_CHILD_STRING (echild);
		sp = g_utf8_offset_to_pointer (sstring->text, start - sstring->start);
		ep = g_utf8_offset_to_pointer (estring->text, end - estring->start);
		utf8 = g_new (gchar, (sp - sstring->text) + strlen (ep) + 1);
		if (sp > sstring->text) memcpy (utf8, sstring->text, sp - sstring->text);
		memcpy (utf8 + (sp - sstring->text), ep, strlen (ep) + 1);
		sp_repr_set_content (SP_OBJECT_REPR (sstring), utf8);
		g_free (utf8);
		/* Delete nodes */
		cl = NULL;
		for (child = schild->next; child != echild; child = child->next) {
			cl = g_slist_prepend (cl, SP_OBJECT_REPR (child));
		}
		cl = g_slist_prepend (cl, SP_OBJECT_REPR (child));
		while (cl) {
			sp_repr_unparent ((SPRepr *) cl->data);
			cl = g_slist_remove (cl, cl->data);
		}
	} else {
		SPString *string;
		gchar *sp, *ep;
		/* Easy case */
		string = SP_TEXT_CHILD_STRING (schild);
		sp = g_utf8_offset_to_pointer (string->text, start - string->start);
		ep = g_utf8_offset_to_pointer (string->text, end - string->start);
		memmove (sp, ep, strlen (ep) + 1);
		sp_repr_set_content (SP_OBJECT_REPR (string), string->text);
	}

	return start;
}

/* fixme: Should look roles here */

gint
sp_text_up (SPText *text, gint pos)
{
	SPObject *child, *up;
	SPString *string;
	gint col;

	child = sp_text_get_child_by_position (text, pos);
	if (!child || child == text->children) return pos;
	string = SP_TEXT_CHILD_STRING (child);
	col = pos - string->start;

	up = text->children;
	while (up->next != child) up = up->next;
	string = SP_TEXT_CHILD_STRING (up);
	col = MIN (col, static_cast< gint > (string->length) );

	return string->start + col;
}

gint
sp_text_down (SPText *text, gint pos)
{
	SPObject *child;
	SPString *string;
	gint col;

	child = sp_text_get_child_by_position (text, pos);
	if (!child || !child->next) return pos;
	string = SP_TEXT_CHILD_STRING (child);
	col = pos - string->start;

	child = child->next;
	string = SP_TEXT_CHILD_STRING (child);
	col = MIN (col, static_cast< gint > (string->length) );

	return string->start + col;
}

gint
sp_text_start_of_line (SPText *text, gint pos)
{
        SPObject *child;
        SPString *string;

        child = sp_text_get_child_by_position (text, pos);
        if (!child) return 0;
        string = SP_TEXT_CHILD_STRING (child);

        return string->start;
}

gint
sp_text_end_of_line (SPText *text, gint pos)
{
        SPObject *child;
        SPString *string;

        child = sp_text_get_child_by_position (text, pos);
        if (!child) return sp_text_get_length (text);
        string = SP_TEXT_CHILD_STRING (child);

        return string->start + string->length;
}

void
sp_text_get_cursor_coords (SPText *text, gint position, NR::Point &p0, NR::Point &p1)
{
	SPObject *child = sp_text_get_child_by_position (text, position);
	SPString *str = SP_TEXT_CHILD_STRING (child);

	NR::Point p;
	if (!str->p) {
		p = NR::Point(str->ly->x.computed, str->ly->y.computed);
	} else {
		p = str->p[position - str->start];
	}

	if (child->style->writing_mode.computed == SP_CSS_WRITING_MODE_TB) {
		NR::Point dp(child->style->font_size.computed / 2.0, 0);
		p0 = p - dp;
		p1 = p + dp;
	} else {
		// XXX: FIXME: why is this different to above?
		p0 = p - NR::Point(0, child->style->font_size.computed);
		p1 = p;
	}
}

static SPObject *
sp_text_get_child_by_position (SPText *text, gint pos)
{
	SPObject *child;

	for (child = text->children; child && child->next; child = child->next) {
		SPString *str;
		if (SP_IS_STRING (child)) {
			str = SP_STRING (child);
		} else {
			str = SP_TSPAN_STRING (child);
		}
		if (pos <= static_cast< gint > (str->start + str->length))
		{
			return child;
		}
	}

	return child;
}

static void
sp_text_update_length (SPSVGLength *length, gdouble em, gdouble ex, gdouble scale)
{
	if (length->unit == SP_SVG_UNIT_EM) {
		length->computed = length->value * em;
	} else if (length->unit == SP_SVG_UNIT_EX) {
		length->computed = length->value * ex;
	} else if (length->unit == SP_SVG_UNIT_PERCENT) {
		length->computed = length->value * scale;
	}
}

