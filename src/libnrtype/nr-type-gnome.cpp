#define __NR_TYPE_GNOME_C__

/*
 * Typeface and script library
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * This code is in public domain
 */

#include <string.h>
#include <libnr/nr-macros.h>
#include <libnr/nr-rect.h>
#include <libnr/nr-matrix.h>
#include <libart_lgpl/art_misc.h>

#include "nr-type-directory.h"
#include "nr-type-gnome.h"

static void nr_typeface_gnome_class_init (NRTypeFaceGnomeClass *klass);
static void nr_typeface_gnome_init (NRTypeFaceGnome *tfg);
static void nr_typeface_gnome_finalize (NRObject *object);

static void nr_typeface_gnome_setup (NRTypeFace *tface, NRTypeFaceDef *def);
static unsigned int nr_typeface_gnome_attribute_get (NRTypeFace *tf, const unsigned char *key, unsigned char *str, unsigned int size);
static NRBPath *nr_typeface_gnome_glyph_outline_get (NRTypeFace *tf, unsigned int glyph, unsigned int metrics, NRBPath *d, unsigned int ref);
static void nr_typeface_gnome_glyph_outline_unref (NRTypeFace *tf, unsigned int glyph, unsigned int metrics);
static NRPointF *nr_typeface_gnome_glyph_advance_get (NRTypeFace *tf, unsigned int glyph, unsigned int metrics, NRPointF *adv);
static unsigned int nr_typeface_gnome_lookup (NRTypeFace *tf, unsigned int rule, unsigned int unival);

static NRFont *nr_typeface_gnome_font_new (NRTypeFace *tf, unsigned int metrics, NRMatrixF *transform);
static void nr_typeface_gnome_font_free (NRFont *font);

static NRTypeFaceClass *parent_class;

NRType
nr_typeface_gnome_get_type (void)
{
	static NRType type = 0;
	if (!type) {
		type = nr_object_register_type (NR_TYPE_TYPEFACE,
						"NRTypeFaceGnome",
						sizeof (NRTypeFaceGnomeClass),
						sizeof (NRTypeFaceGnome),
						(void (*) (NRObjectClass *)) nr_typeface_gnome_class_init,
						(void (*) (NRObject *)) nr_typeface_gnome_init);
	}
	return type;
}

static void
nr_typeface_gnome_class_init (NRTypeFaceGnomeClass *klass)
{
	NRObjectClass *object_class;
	NRTypeFaceClass *tface_class;

	object_class = (NRObjectClass *) klass;
	tface_class = (NRTypeFaceClass *) klass;

	parent_class = (NRTypeFaceClass *) (((NRObjectClass *) klass)->parent);

	object_class->finalize = nr_typeface_gnome_finalize;

	tface_class->setup = nr_typeface_gnome_setup;
	tface_class->attribute_get = nr_typeface_gnome_attribute_get;
	tface_class->glyph_outline_get = nr_typeface_gnome_glyph_outline_get;
	tface_class->glyph_outline_unref = nr_typeface_gnome_glyph_outline_unref;
	tface_class->glyph_advance_get = nr_typeface_gnome_glyph_advance_get;
	tface_class->lookup = nr_typeface_gnome_lookup;

	tface_class->font_new = nr_typeface_gnome_font_new;
	tface_class->font_free = nr_typeface_gnome_font_free;
}

static void
nr_typeface_gnome_init (NRTypeFaceGnome *tfg)
{
	NRTypeFace *tface;

	tface = (NRTypeFace *) tfg;
}

static void
nr_typeface_gnome_finalize (NRObject *object)
{
	NRTypeFace *tf;
	NRTypeFaceGnome *tfg;

	tf = (NRTypeFace *) object;
	tfg = (NRTypeFaceGnome *) object;

	if (tfg->voutlines) {
		int i;
		for (i = 0; i < tf->nglyphs; i++) {
			if (tfg->voutlines[i].path) art_free (tfg->voutlines[i].path);
		}
		nr_free (tfg->voutlines);
	}

	gnome_font_face_unref (tfg->face);

	((NRObjectClass *) (parent_class))->finalize (object);
}

static void
nr_typeface_gnome_setup (NRTypeFace *tface, NRTypeFaceDef *def)
{
	NRTypeFaceGnome *tfg;

	tfg = (NRTypeFaceGnome *) tface;

	((NRTypeFaceClass *) (parent_class))->setup (tface, def);

	tfg->face = gnome_font_face_find (def->name);
	tfg->fonts = NULL;

	tfg->typeface.nglyphs = gnome_font_face_get_num_glyphs (tfg->face);

	tfg->voutlines = NULL;
}

static void
nr_type_gnome_typefaces_destructor (NRNameList *list)
{
	nr_free (list->names);
}

NRNameList *
nr_type_gnome_typefaces_get (NRNameList *typefaces)
{
	static GList *fl = NULL;

	typefaces->destructor = nr_type_gnome_typefaces_destructor;

	if (!fl) fl = gnome_font_list ();

	if (fl) {
		GList *l;
		int pos;
		typefaces->length = g_list_length (fl);
		typefaces->names = nr_new (unsigned char *, typefaces->length);
		pos = 0;
		for (l = fl; l; l = l->next) {
			typefaces->names[pos++] = (unsigned char *) l->data;
		}
	} else {
		typefaces->length = 0;
		typefaces->names = NULL;
	}

	return typefaces;
}

static void
nr_type_gnome_families_destructor (NRNameList *list)
{
	nr_free (list->names);
}

NRNameList *
nr_type_gnome_families_get (NRNameList *families)
{
	static GList *fl = NULL;

	families->destructor = nr_type_gnome_families_destructor;

	if (!fl) fl = gnome_font_family_list ();

	if (fl) {
		GList *l;
		int pos;
		families->length = g_list_length (fl);
		families->names = nr_new (unsigned char *, families->length);
		pos = 0;
		for (l = fl; l; l = l->next) {
			families->names[pos++] = (unsigned char *) l->data;
		}
	} else {
		families->length = 0;
		families->names = NULL;
	}

	return families;
}

void
nr_type_gnome_build_def (NRTypeFaceDef *def, const unsigned char *name, const unsigned char *family)
{
	def->type = NR_TYPE_TYPEFACE_GNOME;
	def->name = g_strdup (name);
	def->family = g_strdup (family);
	def->typeface = NULL;
}

void
nr_type_read_gnome_list (void)
{
	NRNameList gnames, gfamilies;
	int i, j;

	nr_type_gnome_typefaces_get (&gnames);
	nr_type_gnome_families_get (&gfamilies);

	for (i = gnames.length - 1; i >= 0; i--) {
		NRTypeFaceDef *tdef;
		const unsigned char *family;
		family = NULL;
		for (j = gfamilies.length - 1; j >= 0; j--) {
			int len;
			len = strlen (gfamilies.names[j]);
			if (!strncmp (gfamilies.names[j], gnames.names[i], len)) {
				family = gfamilies.names[j];
				break;
			}
		}
		if (family) {
			tdef = nr_new (NRTypeFaceDef, 1);
			tdef->next = NULL;
			tdef->pdef = NULL;
			nr_type_gnome_build_def (tdef, gnames.names[i], family);
			nr_type_register (tdef);
		}
	}

	nr_name_list_release (&gfamilies);
	nr_name_list_release (&gnames);
}

static unsigned int
nr_typeface_gnome_attribute_get (NRTypeFace *tf, const unsigned char *key, unsigned char *str, unsigned int size)
{
	NRTypeFaceGnome *tfg;
	const unsigned char *val;
	int len;

	tfg = (NRTypeFaceGnome *) tf;

	if (!strcmp (key, "name")) {
		val = tf->def->name;
	} else if (!strcmp (key, "family")) {
		val = tf->def->family;
	} else if (!strcmp (key, "weight")) {
		guint wc;
		wc = gnome_font_face_get_weight_code (tfg->face);
		val = (wc >= GNOME_FONT_BOLD) ? "bold" : "normal";
	} else if (!strcmp (key, "style")) {
		if (gnome_font_face_is_italic (tfg->face)) {
			const unsigned char *name;
			name = gnome_font_face_get_name (tfg->face);
			if (strstr (name, "oblique") || strstr (name, "Oblique") || (strstr (name, "OBLIQUE"))) {
				val = "oblique";
			} else {
				val = "italic";
			}
		} else {
			val = "normal";
		}
	} else {
		val = "";
	}

	len = MIN (size - 1, strlen (val));
	if (len > 0) {
		memcpy (str, val, len);
	}
	if (size > 0) {
		str[len] = '\0';
	}

	return strlen (val);
}

static NRBPath *
nr_typeface_gnome_glyph_outline_get (NRTypeFace *tf, unsigned int glyph, unsigned int metrics, NRBPath *d, unsigned int ref)
{
	NRTypeFaceGnome *tfg;

	tfg = (NRTypeFaceGnome *) tf;

	if (metrics == NR_TYPEFACE_METRICS_VERTICAL) {
		if (!tfg->voutlines) {
			int i;
			tfg->voutlines = nr_new (NRBPath, tf->nglyphs);
			for (i = 0; i < tf->nglyphs; i++) {
				tfg->voutlines[i].path = NULL;
			}
		}
		if (!tfg->voutlines[glyph].path) {
			NRBPath bpath;
			NRRectF bbox;
			double t[6];
			bpath.path = (ArtBpath *) gnome_font_face_get_glyph_stdoutline (tfg->face, glyph);
			bbox.x0 = bbox.y0 = 1e18;
			bbox.x1 = bbox.y1 = -1e18;
			nr_path_matrix_f_bbox_f_union (&bpath, NULL, &bbox, 0.25);
#if 0
			printf ("BBOX %d - %f %f %f %f\n", glyph, bbox.x0, bbox.y0, bbox.x1, bbox.y1);
#endif
			if (!nr_rect_f_test_empty (&bbox)) {
				t[0] = 1.0;
				t[1] = 0.0;
				t[2] = 0.0;
				t[3] = 1.0;
				t[4] = 0.0 - (bbox.x1 - bbox.x0) / 2;
				t[5] = -1000.0;
				tfg->voutlines[glyph].path = art_bpath_affine_transform (bpath.path, t);
			}
		}
		*d = tfg->voutlines[glyph];
	} else {
		d->path = (ArtBpath *) gnome_font_face_get_glyph_stdoutline (tfg->face, glyph);
	}

	return d;
}

static void
nr_typeface_gnome_glyph_outline_unref (NRTypeFace *tf, unsigned int glyph, unsigned int metrics)
{
	NRTypeFaceGnome *tfg;

	tfg = (NRTypeFaceGnome *) tf;
}

static NRPointF *
nr_typeface_gnome_glyph_advance_get (NRTypeFace *tf, unsigned int glyph, unsigned int metrics, NRPointF *adv)
{
	NRTypeFaceGnome *tfg;

	tfg = (NRTypeFaceGnome *) tf;

	if (metrics == NR_TYPEFACE_METRICS_VERTICAL) {
		adv->x = 0.0;
		adv->y = -1000.0;
	} else {
		ArtPoint gfa;
		gnome_font_face_get_glyph_stdadvance (tfg->face, glyph, &gfa);
		adv->x = gfa.x;
		adv->y = gfa.y;
	}

	return adv;
}

static unsigned int
nr_typeface_gnome_lookup (NRTypeFace *tf, unsigned int rule, unsigned int unival)
{
	NRTypeFaceGnome *tfg;

	tfg = (NRTypeFaceGnome *) tf;

	if (rule == NR_TYPEFACE_LOOKUP_RULE_DEFAULT) {
		if ((unival >= 0xe000) && (unival <= 0xf8ff)) {
			unsigned int idx;
			idx = CLAMP (unival, 0xe000, 0xf8ff) - 0xe000;
			return MIN (idx, tf->nglyphs - 1);
		} else {
			return gnome_font_face_lookup_default (tfg->face, unival);
		}
	}

	return 0;
}

static NRFont *
nr_typeface_gnome_font_new (NRTypeFace *tf, unsigned int metrics, NRMatrixF *transform)
{
	NRTypeFaceGnome *tfg;
	NRFont *font;
	float size;

	tfg = (NRTypeFaceGnome *) tf;
	size = NR_MATRIX_DF_EXPANSION (transform);

	font = tfg->fonts;
	while (font != NULL) {
		if (NR_DF_TEST_CLOSE (size, font->size, 0.001 * size) && (font->metrics == metrics)) {
			return nr_font_ref (font);
		}
		font = font->next;
	}
	
	font = nr_font_generic_new (tf, metrics, transform);

	font->next = tfg->fonts;
	tfg->fonts = font;

	return font;
}

static void
nr_typeface_gnome_font_free (NRFont *font)
{
	NRTypeFaceGnome *tfg;

	tfg = (NRTypeFaceGnome *) font->face;

	if (tfg->fonts == font) {
		tfg->fonts = font->next;
	} else {
		NRFont *ref;
		ref = tfg->fonts;
		while (ref->next != font) ref = ref->next;
		ref->next = font->next;
	}

	font->next = NULL;

	nr_font_generic_free (font);
}
