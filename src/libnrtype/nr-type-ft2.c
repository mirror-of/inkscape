#define __NR_TYPE_FT2_C__

/*
 * Typeface and script library
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * This code is in public domain
 */

#include <string.h>
#include <stdio.h>
#include <libnr/nr-macros.h>
#include <libnr/nr-matrix.h>
#include <freetype/ftoutln.h>
#include <freetype/ftbbox.h>
#include "nr-type-ft2.h"

#define NRTFFT2_DEBUG

#ifdef NRTFFT2_DEBUG
static int olcount = 0;
#endif

#define NR_SLOTS_BLOCK 32

static void nr_typeface_ft2_class_init (NRTypeFaceFT2Class *klass);
static void nr_typeface_ft2_init (NRTypeFaceFT2 *tff);
static void nr_typeface_ft2_finalize (NRObject *object);

static void nr_typeface_ft2_setup (NRTypeFace *tface, NRTypeFaceDef *def);
static unsigned int nr_typeface_ft2_attribute_get (NRTypeFace *tf, const unsigned char *key, unsigned char *str, unsigned int size);
static NRBPath *nr_typeface_ft2_glyph_outline_get (NRTypeFace *tf, unsigned int glyph, unsigned int metrics, NRBPath *d, unsigned int ref);
static void nr_typeface_ft2_glyph_outline_unref (NRTypeFace *tf, unsigned int glyph, unsigned int metrics);
static NRPointF *nr_typeface_ft2_glyph_advance_get (NRTypeFace *tf, unsigned int glyph, unsigned int metrics, NRPointF *adv);
static unsigned int nr_typeface_ft2_lookup (NRTypeFace *tf, unsigned int rule, unsigned int unival);

static NRFont *nr_typeface_ft2_font_new (NRTypeFace *tf, unsigned int metrics, NRMatrixF *transform);
static void nr_typeface_ft2_font_free (NRFont *font);

static FT_Library ft_library = NULL;

static NRTypeFaceClass *parent_class;

unsigned int
nr_typeface_ft2_get_type (void)
{
	static unsigned int type = 0;
	if (!type) {
		type = nr_object_register_type (NR_TYPE_TYPEFACE,
						"NRTypeFaceFT2",
						sizeof (NRTypeFaceFT2Class),
						sizeof (NRTypeFaceFT2),
						(void (*) (NRObjectClass *)) nr_typeface_ft2_class_init,
						(void (*) (NRObject *)) nr_typeface_ft2_init);
	}
	return type;
}

static void
nr_typeface_ft2_class_init (NRTypeFaceFT2Class *klass)
{
	NRObjectClass *object_class;
	NRTypeFaceClass *tface_class;

	object_class = (NRObjectClass *) klass;
	tface_class = (NRTypeFaceClass *) klass;

	parent_class = (NRTypeFaceClass *) (((NRObjectClass *) klass)->parent);

	object_class->finalize = nr_typeface_ft2_finalize;

	tface_class->setup = nr_typeface_ft2_setup;
	tface_class->attribute_get = nr_typeface_ft2_attribute_get;
	tface_class->glyph_outline_get = nr_typeface_ft2_glyph_outline_get;
	tface_class->glyph_outline_unref = nr_typeface_ft2_glyph_outline_unref;
	tface_class->glyph_advance_get = nr_typeface_ft2_glyph_advance_get;
	tface_class->lookup = nr_typeface_ft2_lookup;

	tface_class->font_new = nr_typeface_ft2_font_new;
	tface_class->font_free = nr_typeface_ft2_font_free;
}

static void
nr_typeface_ft2_init (NRTypeFaceFT2 *tff)
{
	NRTypeFace *tface;

	tface = (NRTypeFace *) tff;

	tface->nglyphs = 1;
}

static void
nr_typeface_ft2_finalize (NRObject *object)
{
	NRTypeFaceFT2 *tff;

	tff = (NRTypeFaceFT2 *) object;

	if (tff->ft_face) {
		FT_Done_Face (tff->ft_face);
		if (tff->slots) {
			int i;
			for (i = 0; i < tff->slots_length; i++) {
				if (tff->slots[i].outline.path) {
					nr_free (tff->slots[i].outline.path);
#ifdef NRTFFT2_DEBUG
					olcount -= 1;
					printf ("finalize - outlines %d\n", olcount);
#endif
				}
			}
			nr_free (tff->slots);
		}
		if (tff->hgidx) nr_free (tff->hgidx);
		if (tff->vgidx) nr_free (tff->vgidx);
	}

	((NRObjectClass *) (parent_class))->finalize (object);
}

static void
nr_typeface_ft2_setup (NRTypeFace *tface, NRTypeFaceDef *def)
{
	NRTypeFaceFT2 *tff;
	NRTypeFaceDefFT2 *dft2;
	FT_Face ft_face;
	FT_Error ft_result;

	tff = (NRTypeFaceFT2 *) tface;
	dft2 = (NRTypeFaceDefFT2 *) def;

	((NRTypeFaceClass *) (parent_class))->setup (tface, def);

	if (!ft_library) {
		ft_result = FT_Init_FreeType (&ft_library);
		if (ft_result != FT_Err_Ok) {
			fprintf (stderr, "Error initializing FreeType2 library");
			return;
		}
	}

	if (dft2->is_file) {
		ft_result = FT_New_Face (ft_library, dft2->data.file, dft2->face, &ft_face);
		if (ft_result != FT_Err_Ok) {
			fprintf (stderr, "Error loading typeface %s from file %s:%d", dft2->def.name, dft2->data.file, dft2->face);
			return;
		}
	} else {
		ft_result = FT_New_Memory_Face (ft_library, dft2->data.data, dft2->size, dft2->face, &ft_face);
		if (ft_result != FT_Err_Ok) {
			fprintf (stderr, "Error loading typeface %s from memory", dft2->def.name);
			return;
		}
	}

	/* fixme: Test scalability */

	tff->ft_face = ft_face;

	tff->typeface.nglyphs = tff->ft_face->num_glyphs;

	ft_result = FT_Select_Charmap (ft_face, ft_encoding_unicode);
	if (ft_result != FT_Err_Ok) {
		tff->unimap = 0;
#if 0
		fprintf (stderr, "Typeface %s does not have unicode charmap", def->name);
#endif
	} else {
		int cp, nglyphs;
		tff->unimap = 1;
		tff->freelo = 1;
		nglyphs = MIN (tff->typeface.nglyphs, 0x1900);
		/* Check whether we have free U+E000 - U+F8FF */
		for (cp = 0; cp < nglyphs; cp++) {
			if (FT_Get_Char_Index (tff->ft_face, 0xe000 + cp)) {
				tff->freelo = 0;
				break;
			}
		}
	}

	tff->ft2ps = 1000.0 / tff->ft_face->units_per_EM;
	tff->fonts = NULL;

	tff->hgidx = NULL;
	tff->vgidx = NULL;
	tff->slots = NULL;
	tff->slots_length = 0;
	tff->slots_size = 0;
}

static NRTypeFaceGlyphFT2 *nr_typeface_ft2_ensure_slot_h (NRTypeFaceFT2 *tff, unsigned int glyph);
static NRTypeFaceGlyphFT2 *nr_typeface_ft2_ensure_slot_v (NRTypeFaceFT2 *tff, unsigned int glyph);
static NRBPath *nr_typeface_ft2_ensure_outline (NRTypeFaceFT2 *tff, NRTypeFaceGlyphFT2 *slot, unsigned int glyph, unsigned int metrics);

void
nr_type_ft2_build_def (NRTypeFaceDefFT2 *dft2,
		       const unsigned char *name,
		       const unsigned char *family,
		       const unsigned char *file,
		       unsigned int face)
{
	dft2->def.type = NR_TYPE_TYPEFACE_FT2;
	dft2->def.name = strdup (name);
	dft2->def.family = strdup (family);
	dft2->def.typeface = NULL;
	dft2->is_file = TRUE;
	dft2->data.file = strdup (file);
	dft2->face = face;
}

void
nr_type_ft2_build_def_data (NRTypeFaceDefFT2 *dft2,
			    const unsigned char *name,
			    const unsigned char *family,
			    const unsigned char *data,
			    unsigned int size,
			    unsigned int face)
{
	dft2->def.type = NR_TYPE_TYPEFACE_FT2;
	dft2->def.name = strdup (name);
	dft2->def.family = strdup (family);
	dft2->def.typeface = NULL;
	dft2->is_file = FALSE;
	dft2->data.data = data;
	dft2->size = size;
	dft2->face = face;
}

static unsigned int
nr_typeface_ft2_attribute_get (NRTypeFace *tf, const unsigned char *key, unsigned char *str, unsigned int size)
{
	NRTypeFaceFT2 *tff;
	const unsigned char *val;
	int len;

	tff = (NRTypeFaceFT2 *) tf;

	if (!strcmp (key, "name")) {
		val = tf->def->name;
	} else if (!strcmp (key, "family")) {
		val = tf->def->family;
	} else if (!strcmp (key, "weight")) {
		/* fixme: This is just wrong */
		val = (tff->ft_face->style_flags & FT_STYLE_FLAG_BOLD) ? "bold" : "normal";
	} else if (!strcmp (key, "style")) {
		/* fixme: This is just wrong */
		val = (tff->ft_face->style_flags & FT_STYLE_FLAG_ITALIC) ? "italic" : "normal";
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
nr_typeface_ft2_glyph_outline_get (NRTypeFace *tf, unsigned int glyph, unsigned int metrics, NRBPath *d, unsigned int ref)
{
	NRTypeFaceFT2 *tff;
	NRTypeFaceGlyphFT2 *slot;

	tff = (NRTypeFaceFT2 *) tf;

	if (metrics == NR_TYPEFACE_METRICS_VERTICAL) {
		slot = nr_typeface_ft2_ensure_slot_v (tff, glyph);
	} else {
		slot = nr_typeface_ft2_ensure_slot_h (tff, glyph);
	}

	if (slot) {
		if (!slot->olref) nr_typeface_ft2_ensure_outline (tff, slot, glyph, metrics);
		if (ref) slot->olref += 1;
		*d = slot->outline;
	} else {
		d->path = NULL;
	}

	return d;
}

static void
nr_typeface_ft2_glyph_outline_unref (NRTypeFace *tf, unsigned int glyph, unsigned int metrics)
{
	NRTypeFaceFT2 *tff;
	NRTypeFaceGlyphFT2 *slot;

	tff = (NRTypeFaceFT2 *) tf;

	if (metrics == NR_TYPEFACE_METRICS_VERTICAL) {
		slot = nr_typeface_ft2_ensure_slot_v (tff, glyph);
	} else {
		slot = nr_typeface_ft2_ensure_slot_h (tff, glyph);
	}

	if (slot && slot->olref > 0) {
		slot->olref -= 1;
		if (slot->olref < 1) {
			nr_free (slot->outline.path);
			slot->outline.path = NULL;
#ifdef NRTFFT2_DEBUG
			olcount -= 1;
			printf ("outline unref - outlines %d\n", olcount);
#endif
		}
	}
}

static NRPointF *
nr_typeface_ft2_glyph_advance_get (NRTypeFace *tf, unsigned int glyph, unsigned int metrics, NRPointF *adv)
{
	NRTypeFaceFT2 *tff;
	NRTypeFaceGlyphFT2 *slot;

	tff = (NRTypeFaceFT2 *) tf;

	if (metrics == NR_TYPEFACE_METRICS_VERTICAL) {
		slot = nr_typeface_ft2_ensure_slot_v (tff, glyph);
	} else {
		slot = nr_typeface_ft2_ensure_slot_h (tff, glyph);
	}

	if (slot) {
		*adv = slot->advance;
		return adv;
	}

	return NULL;
}

static unsigned int
nr_typeface_ft2_lookup (NRTypeFace *tf, unsigned int rule, unsigned int unival)
{
	NRTypeFaceFT2 *tff;

	tff = (NRTypeFaceFT2 *) tf;

	if (rule == NR_TYPEFACE_LOOKUP_RULE_DEFAULT) {
		if (unival > 0xf0000) {
			unsigned int idx;
			idx = CLAMP (unival, 0xf0000, 0x1ffff) - 0xf0000;
			return MIN (idx, tf->nglyphs - 1);
		} else if (!tff->unimap || (tff->freelo && (unival >= 0xe000) && (unival <= 0xf8ff))) {
			unsigned int idx;
			idx = CLAMP (unival, 0xe000, 0xf8ff) - 0xe000;
			return MIN (idx, tf->nglyphs - 1);
		} else {
			return FT_Get_Char_Index (tff->ft_face, unival);
		}
	}

	return 0;
}

static NRFont *
nr_typeface_ft2_font_new (NRTypeFace *tf, unsigned int metrics, NRMatrixF *transform)
{
	NRTypeFaceFT2 *tff;
	NRFont *font;
	float size;

	tff = (NRTypeFaceFT2 *) tf;
	size = NR_MATRIX_DF_EXPANSION (transform);

	font = tff->fonts;
	while (font != NULL) {
		if (NR_DF_TEST_CLOSE (size, font->size, 0.001 * size) && (font->metrics == metrics)) {
			return nr_font_ref (font);
		}
		font = font->next;
	}
	
	font = nr_font_generic_new (tf, metrics, transform);

	font->next = tff->fonts;
	tff->fonts = font;

	return font;
}

static void
nr_typeface_ft2_font_free (NRFont *font)
{
	NRTypeFaceFT2 *tff;

	tff = (NRTypeFaceFT2 *) font->face;

	if (tff->fonts == font) {
		tff->fonts = font->next;
	} else {
		NRFont *ref;
		ref = tff->fonts;
		while (ref->next != font) ref = ref->next;
		ref->next = font->next;
	}

	font->next = NULL;

	nr_font_generic_free (font);
}

static NRTypeFaceGlyphFT2 *
nr_typeface_ft2_ensure_slot_h (NRTypeFaceFT2 *tff, unsigned int glyph)
{
	if (!tff->hgidx) {
		int i;
		tff->hgidx = nr_new (int, tff->typeface.nglyphs);
		for (i = 0; i < tff->typeface.nglyphs; i++) {
			tff->hgidx[i] = -1;
		}
	}

	if (tff->hgidx[glyph] < 0) {
		NRTypeFaceGlyphFT2 *slot;
		if (!tff->slots) {
			tff->slots = nr_new (NRTypeFaceGlyphFT2, 8);
			tff->slots_size = 8;
		} else if (tff->slots_length >= tff->slots_size) {
			tff->slots_size += NR_SLOTS_BLOCK;
			tff->slots = nr_renew (tff->slots, NRTypeFaceGlyphFT2, tff->slots_size);
		}
		slot = tff->slots + tff->slots_length;

		FT_Load_Glyph (tff->ft_face, glyph, FT_LOAD_NO_SCALE | FT_LOAD_NO_HINTING | FT_LOAD_NO_BITMAP);

		slot->area.x0 = tff->ft_face->glyph->metrics.horiBearingX * tff->ft2ps;
		slot->area.y1 = tff->ft_face->glyph->metrics.horiBearingY * tff->ft2ps;
		slot->area.y0 = slot->area.y1 - tff->ft_face->glyph->metrics.height * tff->ft2ps;
		slot->area.x1 = slot->area.x0 + tff->ft_face->glyph->metrics.width * tff->ft2ps;
		slot->advance.x = tff->ft_face->glyph->metrics.horiAdvance * tff->ft2ps;
		slot->advance.y = 0.0;

		slot->olref = 0;
		slot->outline.path = NULL;
		tff->hgidx[glyph] = tff->slots_length;
		tff->slots_length += 1;
	}

	return tff->slots + tff->hgidx[glyph];
}

static NRTypeFaceGlyphFT2 *
nr_typeface_ft2_ensure_slot_v (NRTypeFaceFT2 *tff, unsigned int glyph)
{
	if (!tff->vgidx) {
		int i;
		tff->vgidx = nr_new (int, tff->typeface.nglyphs);
		for (i = 0; i < tff->typeface.nglyphs; i++) {
			tff->vgidx[i] = -1;
		}
	}

	if (tff->vgidx[glyph] < 0) {
		NRTypeFaceGlyphFT2 *slot;
		if (!tff->slots) {
			tff->slots = nr_new (NRTypeFaceGlyphFT2, 8);
			tff->slots_size = 8;
		} else if (tff->slots_length >= tff->slots_size) {
			tff->slots_size += NR_SLOTS_BLOCK;
			tff->slots = nr_renew (tff->slots, NRTypeFaceGlyphFT2, tff->slots_size);
		}
		slot = tff->slots + tff->slots_length;

		if (FT_HAS_VERTICAL (tff->ft_face)) {
			FT_Load_Glyph (tff->ft_face, glyph, FT_LOAD_VERTICAL_LAYOUT | FT_LOAD_NO_SCALE | FT_LOAD_NO_HINTING | FT_LOAD_NO_BITMAP);
			slot->area.x0 = tff->ft_face->glyph->metrics.vertBearingX * tff->ft2ps;
			slot->area.x1 = slot->area.x0 + tff->ft_face->glyph->metrics.width * tff->ft2ps;
			slot->area.y1 = -tff->ft_face->glyph->metrics.vertBearingY * tff->ft2ps;
			slot->area.y0 = slot->area.y1 - tff->ft_face->glyph->metrics.height * tff->ft2ps;
			slot->advance.x = 0.0;
			slot->advance.y = -tff->ft_face->glyph->metrics.vertAdvance * tff->ft2ps;
#if 0
			printf ("VM %d - %f %f %f %f - %f %f\n", glyph,
				slot->area.x0, slot->area.y0, slot->area.x1, slot->area.y1,
				slot->advance.x, slot->advance.y);
#endif
		} else {
			FT_Load_Glyph (tff->ft_face, glyph, FT_LOAD_NO_SCALE | FT_LOAD_NO_HINTING | FT_LOAD_NO_BITMAP);
			slot->area.x0 = -0.5 * tff->ft_face->glyph->metrics.width * tff->ft2ps;
			slot->area.x1 = 0.5 * tff->ft_face->glyph->metrics.width * tff->ft2ps;
			slot->area.y1 = tff->ft_face->glyph->metrics.horiBearingY * tff->ft2ps - 1000.0;
			slot->area.y0 = slot->area.y1 - tff->ft_face->glyph->metrics.height * tff->ft2ps;
			slot->advance.x = 0.0;
			slot->advance.y = -1000.0;
		}

		slot->olref = 0;
		slot->outline.path = NULL;
		tff->vgidx[glyph] = tff->slots_length;
		tff->slots_length += 1;
	}

	return tff->slots + tff->vgidx[glyph];
}

/* Outline conversion */

static ArtBpath *tff_ol2bp (FT_Outline *ol, float transform[]);

static NRBPath *
nr_typeface_ft2_ensure_outline (NRTypeFaceFT2 *tff, NRTypeFaceGlyphFT2 *slot, unsigned int glyph, unsigned int metrics)
{
	float a[6];

	FT_Load_Glyph (tff->ft_face, glyph, FT_LOAD_NO_SCALE | FT_LOAD_NO_HINTING | FT_LOAD_NO_BITMAP);

	a[0] = a[3] = tff->ft2ps;
	a[1] = a[2] = 0.0;

	if (metrics == NR_TYPEFACE_METRICS_VERTICAL) {
#if 0
		FT_BBox bbox;
		/* Metrics are always loaded if we have slot */
		FT_Outline_Get_BBox (&tff->ft_face->glyph->outline, &bbox);
		a[4] = slot->area.x0 - bbox.xMin * tff->ft2ps;
		a[5] = slot->area.y0 - bbox.yMin * tff->ft2ps;
#else
		a[4] = slot->area.x0 - tff->ft_face->glyph->metrics.horiBearingX * tff->ft2ps;
		a[5] = slot->area.y1 - tff->ft_face->glyph->metrics.horiBearingY * tff->ft2ps;
#endif
	} else {
		a[4] = 0.0;
		a[5] = 0.0;
	}

	slot->outline.path = tff_ol2bp (&tff->ft_face->glyph->outline, a);
	slot->olref = 1;
#ifdef NRTFFT2_DEBUG
	if (slot->outline.path) {
		olcount += 1;
		printf ("ensure outline - outlines %d\n", olcount);
	}
#endif

	return &slot->outline;
}

/* Bpath methods */

typedef struct {
	ArtBpath *bp;
	int start, end;
	float *t;
} TFFT2OutlineData;

static int tfft2_move_to (FT_Vector * to, void * user)
{
	TFFT2OutlineData * od;
	NRPointF p;

	od = (TFFT2OutlineData *) user;

	p.x = to->x * od->t[0] + to->y * od->t[2] + od->t[4];
	p.y = to->x * od->t[1] + to->y * od->t[3] + od->t[5];

	if (od->end == 0 ||
	    p.x != od->bp[od->end - 1].x3 ||
	    p.y != od->bp[od->end - 1].y3) {
		od->bp[od->end].code = ART_MOVETO;
		od->bp[od->end].x3 = to->x * od->t[0] + to->y * od->t[2] + od->t[4];
		od->bp[od->end].y3 = to->x * od->t[1] + to->y * od->t[3] + od->t[5];
		od->end++;
	}

	return 0;
}

static int tfft2_line_to (FT_Vector * to, void * user)
{
	TFFT2OutlineData * od;
	ArtBpath *s;
	NRPointF p;

	od = (TFFT2OutlineData *) user;

	s = &od->bp[od->end - 1];

	p.x = to->x * od->t[0] + to->y * od->t[2] + od->t[4];
	p.y = to->x * od->t[1] + to->y * od->t[3] + od->t[5];

	if ((p.x != s->x3) || (p.y != s->y3)) {
		od->bp[od->end].code = ART_LINETO;
		od->bp[od->end].x3 = to->x * od->t[0] + to->y * od->t[2] + od->t[4];
		od->bp[od->end].y3 = to->x * od->t[1] + to->y * od->t[3] + od->t[5];
		od->end++;
	}

	return 0;
}

static int tfft2_conic_to (FT_Vector * control, FT_Vector * to, void * user)
{
	TFFT2OutlineData *od;
	ArtBpath *s, *e;
	NRPointF c;

	od = (TFFT2OutlineData *) user;

	s = &od->bp[od->end - 1];
	e = &od->bp[od->end];

	e->code = ART_CURVETO;

	c.x = control->x * od->t[0] + control->y * od->t[2] + od->t[4];
	c.y = control->x * od->t[1] + control->y * od->t[3] + od->t[5];
	e->x3 = to->x * od->t[0] + to->y * od->t[2] + od->t[4];
	e->y3 = to->x * od->t[1] + to->y * od->t[3] + od->t[5];

	od->bp[od->end].x1 = c.x - (c.x - s->x3) / 3;
	od->bp[od->end].y1 = c.y - (c.y - s->y3) / 3;
	od->bp[od->end].x2 = c.x + (e->x3 - c.x) / 3;
	od->bp[od->end].y2 = c.y + (e->y3 - c.y) / 3;
	od->end++;

	return 0;
}

static int tfft2_cubic_to (FT_Vector * control1, FT_Vector * control2, FT_Vector * to, void * user)
{
	TFFT2OutlineData * od;

	od = (TFFT2OutlineData *) user;

	od->bp[od->end].code = ART_CURVETO;
	od->bp[od->end].x1 = control1->x * od->t[0] + control1->y * od->t[2] + od->t[4];
	od->bp[od->end].y1 = control1->x * od->t[1] + control1->y * od->t[3] + od->t[5];
	od->bp[od->end].x2 = control2->x * od->t[0] + control2->y * od->t[2] + od->t[4];
	od->bp[od->end].y2 = control2->x * od->t[1] + control2->y * od->t[3] + od->t[5];
	od->bp[od->end].x3 = to->x * od->t[0] + to->y * od->t[2] + od->t[4];
	od->bp[od->end].y3 = to->x * od->t[1] + to->y * od->t[3] + od->t[5];
	od->end++;

	return 0;
}

FT_Outline_Funcs tfft2_outline_funcs = {
	tfft2_move_to,
	tfft2_line_to,
	tfft2_conic_to,
	tfft2_cubic_to,
	0, 0
};

/*
 * We support only 4x4 matrix here (do you need more?)
 */

static ArtBpath *
tff_ol2bp (FT_Outline * ol, float transform[])
{
	TFFT2OutlineData od;

	od.bp = nr_new (ArtBpath, ol->n_points * 2 + ol->n_contours + 1);
	od.start = od.end = 0;
	od.t = transform;

	FT_Outline_Decompose (ol, &tfft2_outline_funcs, &od);

	od.bp[od.end].code = ART_END;

	od.bp = nr_renew (od.bp, ArtBpath, od.end + 1);

	return od.bp;
}



