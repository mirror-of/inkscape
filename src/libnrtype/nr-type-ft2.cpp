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
#include <libnr/nr-matrix-fns.h>
#include <libnrtype/nr-type-pos-def.h>
#include <freetype/ftoutln.h>
#include <freetype/ftbbox.h>
#include "nr-type-ft2.h"

#define noNRTFFT2_DEBUG

#ifdef NRTFFT2_DEBUG
static int olcount = 0;
#endif

#define NR_SLOTS_BLOCK 32

static void nr_typeface_ft2_class_init (NRTypeFaceFT2Class *klass);
static void nr_typeface_ft2_init (NRTypeFaceFT2 *tff);
static void nr_typeface_ft2_finalize (NRObject *object);

static void nr_typeface_ft2_setup (NRTypeFace *tface, NRTypeFaceDef *def);
static unsigned int nr_typeface_ft2_attribute_get (NRTypeFace *tf, const gchar *key, gchar *str, unsigned int size);
static NRBPath *nr_typeface_ft2_glyph_outline_get (NRTypeFace *tf, unsigned int glyph, unsigned int metrics, NRBPath *d, unsigned int ref);
static void nr_typeface_ft2_glyph_outline_unref (NRTypeFace *tf, unsigned int glyph, unsigned int metrics);
static NR::Point nr_typeface_ft2_glyph_advance_get (NRTypeFace *tf, unsigned int glyph, unsigned int metrics);
static unsigned int nr_typeface_ft2_lookup (NRTypeFace *tf, unsigned int rule, unsigned int unival);

static NRFont *nr_typeface_ft2_font_new (NRTypeFace *tf, unsigned int metrics, NR::Matrix const transform);
static void nr_typeface_ft2_font_free (NRFont *font);

static FT_Library ft_library = NULL;

static NRTypeFaceClass *parent_class;

/**
 * Gets the class type for NRTypeFaceFT2
 */
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

/**
 * Initializes a freetype typeface class with its virtual functions
 */
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

/**
 * Initializes a freetype typeface object
 */
static void
nr_typeface_ft2_init (NRTypeFaceFT2 *tff)
{
	NRTypeFace *tface;

	tface = (NRTypeFace *) tff;
	tface->nglyphs = 1;

	tff->ft2ps = 0.0;

	tff->unimap = FALSE;
	tff->freelo = FALSE;

	tff->fonts = NULL;
	tff->hgidx = NULL;
	tff->vgidx = NULL;

	tff->slots = NULL;
	tff->slots_length = 0;
	tff->slots_size = 0;
}

/**
 * Finalizes the freetype typeface object to prepare it for destruction
 */
static void
nr_typeface_ft2_finalize (NRObject *object)
{
	NRTypeFaceFT2 *tff;

	tff = (NRTypeFaceFT2 *) object;

	if (tff->ft_face) {
		FT_Done_Face (tff->ft_face);
		if (tff->slots) {
			unsigned int i;
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

/**
 * Sets up the given typeface as a freetype typeface using the given 
 * typeface definition.
 */
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
			fprintf (stderr, "Error loading typeface %s from file %s:%d", dft2->name, dft2->data.file, dft2->face);
			return;
		}
	} else {
		ft_result = FT_New_Memory_Face (ft_library, (FT_Byte*)dft2->data.data, dft2->size, dft2->face, &ft_face);
		if (ft_result != FT_Err_Ok) {
			fprintf (stderr, "Error loading typeface %s from memory", dft2->name);
			return;
		}
	}

	/* fixme: Test scalability */

	tff->ft_face = ft_face;

	tff->nglyphs = tff->ft_face->num_glyphs;

	ft_result = FT_Select_Charmap (ft_face, ft_encoding_unicode);
	if (ft_result != FT_Err_Ok) {
		tff->unimap = 0;
		/* Typeface %s does not have unicode charmap", def->name); */
	} else {
		int cp, nglyphs;
		tff->unimap = 1;
		tff->freelo = 1;
		nglyphs = MIN (tff->nglyphs, 0x1900);
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

/**
 * Builds/initializes the definition of a freetype object dft2
 * using the given parameters.
 */
void
nr_type_ft2_build_def (NRTypeFaceDefFT2 *dft2,
		       const gchar *name,
		       const gchar *family,
		       const gchar *file,
		       unsigned int face)
{
	dft2->type = NR_TYPE_TYPEFACE_FT2;
	dft2->name = strdup (name);
	dft2->family = strdup (family);
	dft2->typeface = NULL;
	dft2->is_file = TRUE;
	dft2->data.file = strdup (file);
	dft2->face = face;
}

/**
 * Builds/initializes the definition of a freetype object dft2
 * using the given parameters.  This differs from nr_type_ft2_build_def
 * it takes its data directly rather than from a file.
 */
void
nr_type_ft2_build_def_data (NRTypeFaceDefFT2 *dft2,
			    const gchar *name,
			    const gchar *family,
			    const gchar *data,
			    unsigned int size,
			    unsigned int face)
{
	dft2->type = NR_TYPE_TYPEFACE_FT2;
	dft2->name = strdup (name);
	dft2->family = strdup (family);
	dft2->typeface = NULL;
	dft2->is_file = FALSE;
	dft2->data.data = data;
	dft2->size = size;
	dft2->face = face;
}

/**
 * Gets the attribute specified by key with the value given by str from the typeface
 */
static unsigned int
nr_typeface_ft2_attribute_get (NRTypeFace *tf, const gchar *key, gchar *str, unsigned int size)
{
	NRTypeFaceFT2 *tff;
	const gchar *val = "";
	int len;

	tff = (NRTypeFaceFT2 *) tf;

	if (!strcmp (key, "name")) {
		val = tf->def->name;
	} else if (!strcmp (key, "family")) {
		val = tf->def->family;
	} else if (!strcmp (key, "style")) {
		gint w = parse_name_for_style (tff->ft_face->style_name);
		val = style_to_css (w);
		//g_print ("w %d, val %s\n", w, val);

		// The only font style information that FT provides is the italic and bold bits.
		// We will use the italic bit, but for weight and stretch, the only thing we 
		// can do is parse the style name. Probably when we switch to Pango, we
		// can use its data instead.

	} else if (!strcmp (key, "weight")) {
		//		val = (tff->ft_face->style_flags & FT_STYLE_FLAG_BOLD) ? "bold" : "normal";
		gint w = parse_name_for_weight (tff->ft_face->style_name);
		val = weight_to_css (w);
	} else if (!strcmp (key, "stretch")) {
		gint w = parse_name_for_stretch (tff->ft_face->style_name);
		val = stretch_to_css (w);
	} else if (!strcmp (key, "variant")) {
		gint w = parse_name_for_variant (tff->ft_face->style_name);
		val = variant_to_css (w);
	} else {
		g_warning ("Unknown font attribute requested: %s", key);
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

/**
 * Gets the outline path for the glyph with the given metrics from the typeface
 */
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

/**
 * Dereferences the freetype glyph with the given metrics from the typeface
 */
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

/**
 * Retrieves the horizontal positional advancement for the glyph in the
 * given font.
 */
static NR::Point nr_typeface_ft2_glyph_advance_get (NRTypeFace *tf, unsigned int glyph, unsigned int metrics)
{
	NRTypeFaceGlyphFT2 *slot;

	NRTypeFaceFT2 *tff = (NRTypeFaceFT2 *) tf;

	if (metrics == NR_TYPEFACE_METRICS_VERTICAL) {
		slot = nr_typeface_ft2_ensure_slot_v (tff, glyph);
	} else {
		slot = nr_typeface_ft2_ensure_slot_h (tff, glyph);
	}

	if (slot) {
		return slot->advance;
	}
	
	// I don't understand what it means to get a null advance?
	return NR::Point(0,0);
}

/**
 * Looks up the char index for a given rule and unival of a typeface
 * 
 * Q:  What does the rule represent?
 * Q:  What does the unival represent?
 */
static unsigned int
nr_typeface_ft2_lookup (NRTypeFace *tf, unsigned int rule, unsigned int unival)
{
	NRTypeFaceFT2 *tff;

	tff = (NRTypeFaceFT2 *) tf;

	if (rule == NR_TYPEFACE_LOOKUP_RULE_DEFAULT) {
		if (unival > 0xf0000) {
			unsigned int idx;
			idx = CLAMP (unival, 0xf0000, 0x1fffff) - 0xf0000;
			/* FIXME: The above CLAMP call used to have 0x1ffff as its third argument,
			   but that's presumably unintended, as 0x1ffff < the second argument
			   0xf0000.  I've changed it to 0x1fffff, but that's only a guess as to the
			   intention; I don't know much about this code.  -- pjrm */
			return MIN (idx, tf->nglyphs - 1);
		} else if (!tff->unimap || (tff->freelo && (unival >= 0xe000) && (unival <= 0xf8ff))) {
			unsigned int idx = CLAMP (unival, 0xe000, 0xf8ff) - 0xe000;
			return MIN (idx, tf->nglyphs - 1);
		} else {
			return FT_Get_Char_Index (tff->ft_face, unival);
		}
	}

	return 0;
}

/**
 * Creates a new freetype font object with the given metrics and transform
 */
static NRFont *
nr_typeface_ft2_font_new (NRTypeFace *tf, unsigned int metrics, NR::Matrix const transform)
{
	NRTypeFaceFT2 *tff;
	NRFont *font;
	float size;

	tff = (NRTypeFaceFT2 *) tf;
	size = NR::expansion(transform);

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

/**
 * Frees a freetype font object
 */
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

/**
 * Ensures a vgidx exists for the given typeface font, creating it if
 * necessary, then creates the slots and initializes their areas.
 * This is for horizontal text.
 */
static NRTypeFaceGlyphFT2 *
nr_typeface_ft2_ensure_slot_h (NRTypeFaceFT2 *tff, unsigned int glyph)
{
	if (!tff->hgidx) {
		unsigned int i;
		tff->hgidx = nr_new (int, tff->nglyphs);
		for (i = 0; i < tff->nglyphs; i++) {
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

		if (FT_Load_Glyph (tff->ft_face, glyph, FT_LOAD_NO_SCALE | FT_LOAD_NO_HINTING | FT_LOAD_NO_BITMAP)) return NULL;

		slot->area.x0 = tff->ft_face->glyph->metrics.horiBearingX * tff->ft2ps;
		slot->area.y1 = tff->ft_face->glyph->metrics.horiBearingY * tff->ft2ps;
		slot->area.y0 = slot->area.y1 - tff->ft_face->glyph->metrics.height * tff->ft2ps;
		slot->area.x1 = slot->area.x0 + tff->ft_face->glyph->metrics.width * tff->ft2ps;
		slot->advance = NR::Point(tff->ft_face->glyph->metrics.horiAdvance * tff->ft2ps, 0.0);

		slot->olref = 0;
		slot->outline.path = NULL;
		tff->hgidx[glyph] = tff->slots_length;
		tff->slots_length += 1;
	}

	return tff->slots + tff->hgidx[glyph];
}

/**
 * Ensures a vgidx exists for the given typeface font, creating it if
 * necessary, then creates the slots and initializes their areas.
 * This is for vertical text.
 *
 * Q:  What is a vgidx?
 * Q:  What are slots?
 */
static NRTypeFaceGlyphFT2 *
nr_typeface_ft2_ensure_slot_v (NRTypeFaceFT2 *tff, unsigned int glyph)
{
	if (!tff->vgidx) {
		unsigned int i;
		tff->vgidx = nr_new (int, tff->nglyphs);
		for (i = 0; i < tff->nglyphs; i++) {
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
			slot->advance = NR::Point(0.0, -tff->ft_face->glyph->metrics.vertAdvance * tff->ft2ps);
		} else {
			FT_Load_Glyph (tff->ft_face, glyph, FT_LOAD_NO_SCALE | FT_LOAD_NO_HINTING | FT_LOAD_NO_BITMAP);
			slot->area.x0 = -0.5 * tff->ft_face->glyph->metrics.width * tff->ft2ps;
			slot->area.x1 = 0.5 * tff->ft_face->glyph->metrics.width * tff->ft2ps;
			slot->area.y1 = tff->ft_face->glyph->metrics.horiBearingY * tff->ft2ps - 1000.0;
			slot->area.y0 = slot->area.y1 - tff->ft_face->glyph->metrics.height * tff->ft2ps;
			slot->advance = NR::Point(0.0, -1000.0);
		}

		slot->olref = 0;
		slot->outline.path = NULL;
		tff->vgidx[glyph] = tff->slots_length;
		tff->slots_length += 1;
	}

	return tff->slots + tff->vgidx[glyph];
}

/* Outline conversion */

static NArtBpath *tff_ol2bp (FT_Outline *ol, float transform[]);

/**
 * Loads the glyph from freetype, does some transforming for vertical text,
 * and returns the slot outline
 */
static NRBPath *
nr_typeface_ft2_ensure_outline (NRTypeFaceFT2 *tff, NRTypeFaceGlyphFT2 *slot, unsigned int glyph, unsigned int metrics)
{
	float a[6];

	FT_Load_Glyph (tff->ft_face, glyph, FT_LOAD_NO_SCALE | FT_LOAD_NO_HINTING | FT_LOAD_NO_BITMAP);
	
	// FIXME: use matrix.
	a[0] = a[3] = tff->ft2ps;
	a[1] = a[2] = 0.0;

	if (metrics == NR_TYPEFACE_METRICS_VERTICAL) {
		a[4] = slot->area.x0 - tff->ft_face->glyph->metrics.horiBearingX * tff->ft2ps;
		a[5] = slot->area.y1 - tff->ft_face->glyph->metrics.horiBearingY * tff->ft2ps;
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

/**
 * A structure for the outline data, defining its start, end, and path.
 *
 * Q:  What is the t member?
 */
typedef struct {
	NArtBpath *bp;
	int start, end;
	float *t;
} TFFT2OutlineData;

/**
 * Moves the outline data according to a given vector (??)
 */
static int tfft2_move_to (FT_Vector * to, void * user)
{
	NRPoint p;

	TFFT2OutlineData *od = (TFFT2OutlineData *) user;

	p.x = to->x * od->t[0] + to->y * od->t[2] + od->t[4];
	p.y = to->x * od->t[1] + to->y * od->t[3] + od->t[5];

	if (od->end == 0 ||
	    p.x != od->bp[od->end - 1].x3 ||
	    p.y != od->bp[od->end - 1].y3) {
		od->bp[od->end].code = NR_MOVETO;
		od->bp[od->end].x3 = to->x * od->t[0] + to->y * od->t[2] + od->t[4];
		od->bp[od->end].y3 = to->x * od->t[1] + to->y * od->t[3] + od->t[5];
		od->end++;
	}

	return 0;
}

/**
 * Calculates a line for the outline data
 */
static int tfft2_line_to (FT_Vector * to, void * user)
{
	NRPoint p;

	TFFT2OutlineData *od = (TFFT2OutlineData *) user;

	NArtBpath *s = &od->bp[od->end - 1];

	p.x = to->x * od->t[0] + to->y * od->t[2] + od->t[4];
	p.y = to->x * od->t[1] + to->y * od->t[3] + od->t[5];

	if ((p.x != s->x3) || (p.y != s->y3)) {
		od->bp[od->end].code = NR_LINETO;
		od->bp[od->end].x3 = to->x * od->t[0] + to->y * od->t[2] + od->t[4];
		od->bp[od->end].y3 = to->x * od->t[1] + to->y * od->t[3] + od->t[5];
		od->end++;
	}

	return 0;
}

/**
 * Calculates the conic for the outline data
 */
static int tfft2_conic_to (FT_Vector * control, FT_Vector * to, void * user)
{
	TFFT2OutlineData *od;
	NArtBpath *s, *e;
	NRPoint c;

	od = (TFFT2OutlineData *) user;

	s = &od->bp[od->end - 1];
	e = &od->bp[od->end];

	e->code = NR_CURVETO;

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

/**
 * Q:  Calculates the cubic for the outline data
 */
static int tfft2_cubic_to (FT_Vector * control1, FT_Vector * control2, FT_Vector * to, void * user)
{
	TFFT2OutlineData * od;

	od = (TFFT2OutlineData *) user;

	od->bp[od->end].code = NR_CURVETO;
	od->bp[od->end].x1 = control1->x * od->t[0] + control1->y * od->t[2] + od->t[4];
	od->bp[od->end].y1 = control1->x * od->t[1] + control1->y * od->t[3] + od->t[5];
	od->bp[od->end].x2 = control2->x * od->t[0] + control2->y * od->t[2] + od->t[4];
	od->bp[od->end].y2 = control2->x * od->t[1] + control2->y * od->t[3] + od->t[5];
	od->bp[od->end].x3 = to->x * od->t[0] + to->y * od->t[2] + od->t[4];
	od->bp[od->end].y3 = to->x * od->t[1] + to->y * od->t[3] + od->t[5];
	od->end++;

	return 0;
}

/**
 * Q:  What is this?
 */
FT_Outline_Funcs tfft2_outline_funcs = {
	tfft2_move_to,
	tfft2_line_to,
	tfft2_conic_to,
	tfft2_cubic_to,
	0, 0
};

/**
 * Gets the NArtBPath for the outline object with the given transform.
 * We support only 4x4 matrix here (do you need more?)
 */
static NArtBpath *
tff_ol2bp (FT_Outline * ol, float transform[])
{
	TFFT2OutlineData od;

	od.bp = nr_new (NArtBpath, ol->n_points * 2 + ol->n_contours + 1);
	od.start = od.end = 0;
	od.t = transform;

	FT_Outline_Decompose (ol, &tfft2_outline_funcs, &od);

	od.bp[od.end].code = NR_END;

	od.bp = nr_renew (od.bp, NArtBpath, od.end + 1);

	return od.bp;
}



