#define __NR_TYPE_W32_C__

/*
 * Wrapper around Win32 font subsystem
 *
 * Author:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * This code is in public domain
 */

#include <windows.h>
#include <windowsx.h>

/* fixme: */
#include <glib.h>
#include <libart_lgpl/art_misc.h>
#include <libart_lgpl/art_affine.h>
#include <libart_lgpl/art_bpath.h>
#include <libnr/nr-macros.h>
#include <libnr/nr-matrix.h>

#include "codepages.h"

#include "nr-type-directory.h"
#include "nr-type-w32.h"

#define NR_SLOTS_BLOCK 32

static void nr_typeface_w32_class_init (NRTypeFaceW32Class *klass);
static void nr_typeface_w32_init (NRTypeFaceW32 *tfw32);
static void nr_typeface_w32_finalize (NRObject *object);

static void nr_typeface_w32_setup (NRTypeFace *tface, NRTypeFaceDef *def);

static unsigned int nr_typeface_w32_attribute_get (NRTypeFace *tf, const unsigned char *key, unsigned char *str, unsigned int size);
static NRBPath *nr_typeface_w32_glyph_outline_get (NRTypeFace *tf, unsigned int glyph, unsigned int metrics, NRBPath *d, unsigned int ref);
static void nr_typeface_w32_glyph_outline_unref (NRTypeFace *tf, unsigned int glyph, unsigned int metrics);
static NRPointF *nr_typeface_w32_glyph_advance_get (NRTypeFace *tf, unsigned int glyph, unsigned int metrics, NRPointF *adv);
static unsigned int nr_typeface_w32_lookup (NRTypeFace *tf, unsigned int rule, unsigned int unival);

static NRFont *nr_typeface_w32_font_new (NRTypeFace *tf, unsigned int metrics, NRMatrixF *transform);
static void nr_typeface_w32_font_free (NRFont *font);

static NRTypeFaceClass *parent_class;

NRType
nr_typeface_w32_get_type (void)
{
	static NRType type = 0;
	if (!type) {
		type = nr_object_register_type (NR_TYPE_TYPEFACE,
						"NRTypeFaceW32",
						sizeof (NRTypeFaceW32Class),
						sizeof (NRTypeFaceW32),
						(void (*) (NRObjectClass *)) nr_typeface_w32_class_init,
						(void (*) (NRObject *)) nr_typeface_w32_init);
	}
	return type;
}

static void
nr_typeface_w32_class_init (NRTypeFaceW32Class *klass)
{
	NRObjectClass *object_class;
	NRTypeFaceClass *tface_class;

	object_class = (NRObjectClass *) klass;
	tface_class = (NRTypeFaceClass *) klass;

	parent_class = (NRTypeFaceClass *) (((NRObjectClass *) klass)->parent);

	object_class->finalize = nr_typeface_w32_finalize;

	tface_class->setup = nr_typeface_w32_setup;
	tface_class->attribute_get = nr_typeface_w32_attribute_get;
	tface_class->glyph_outline_get = nr_typeface_w32_glyph_outline_get;
	tface_class->glyph_outline_unref = nr_typeface_w32_glyph_outline_unref;
	tface_class->glyph_advance_get = nr_typeface_w32_glyph_advance_get;
	tface_class->lookup = nr_typeface_w32_lookup;

	tface_class->font_new = nr_typeface_w32_font_new;
	tface_class->font_free = nr_typeface_w32_font_free;
}

static void
nr_typeface_w32_init (NRTypeFaceW32 *tfw32)
{
	NRTypeFace *tface;

	tface = (NRTypeFace *) tfw32;

	tface->nglyphs = 1;
}

static unsigned int w32i = FALSE;

static HDC hdc = NULL;

static GHashTable *namedict = NULL;
static GSList *namelist = NULL;
static GHashTable *familydict = NULL;
static GSList *familylist = NULL;

static NRNameList NRW32Typefaces = {0, NULL, NULL};
static NRNameList NRW32Families = {0, NULL, NULL};

static void nr_type_w32_init (void);
static NRTypeFaceGlyphW32 *nr_typeface_w32_ensure_slot (NRTypeFaceW32 *tfw32, unsigned int glyph, unsigned int metrics);

static NRBPath *nr_typeface_w32_ensure_outline (NRTypeFaceW32 *tfw32, NRTypeFaceGlyphW32 *slot, unsigned int glyph, unsigned int metrics);


void
nr_type_w32_typefaces_get (NRNameList *names)
{
    if (!w32i) nr_type_w32_init ();

    *names = NRW32Typefaces;
}

void
nr_type_w32_families_get (NRNameList *names)
{
    if (!w32i) nr_type_w32_init ();

    *names = NRW32Families;
}

void
nr_type_w32_build_def (NRTypeFaceDef *def, const unsigned char *name, const unsigned char *family)
{
    def->type = NR_TYPE_TYPEFACE_W32;
    def->name = g_strdup (name);
    def->family = g_strdup (family);
    def->typeface = NULL;
}

static void
nr_type_read_w32_list (void)
{
	NRNameList wnames, wfamilies;
	int i, j;

	nr_type_w32_typefaces_get (&wnames);
	nr_type_w32_families_get (&wfamilies);

	for (i = wnames.length - 1; i >= 0; i--) {
		NRTypeFaceDef *tdef;
		const unsigned char *family;
		family = NULL;
		for (j = wfamilies.length - 1; j >= 0; j--) {
			int len;
			len = strlen (wfamilies.names[j]);
			if (!strncmp (wfamilies.names[j], wnames.names[i], len)) {
				family = wfamilies.names[j];
				break;
			}
		}
		if (family) {
			tdef = nr_new (NRTypeFaceDef, 1);
			tdef->next = NULL;
			tdef->pdef = NULL;
			nr_type_w32_build_def (tdef, wnames.names[i], family);
			nr_type_register (tdef);
		}
	}

	nr_name_list_release (&wfamilies);
	nr_name_list_release (&wnames);
}

static void
nr_typeface_w32_setup (NRTypeFace *tface, NRTypeFaceDef *def)
{ 
    NRTypeFaceW32 *tfw32;
    unsigned int otmsize;

    tfw32 = (NRTypeFaceW32 *) tface;

	((NRTypeFaceClass *) (parent_class))->setup (tface, def);

	tfw32->fonts = NULL;
	tfw32->logfont = g_hash_table_lookup (namedict, def->name);
	tfw32->logfont->lfHeight = 1000;
	tfw32->logfont->lfWidth = 0;
	tfw32->hfont = CreateFontIndirect (tfw32->logfont);

	/* Have to select font to get metrics etc. */
	SelectFont (hdc, tfw32->hfont);

	otmsize = GetOutlineTextMetrics (hdc, 0, NULL);
	tfw32->otm = (LPOUTLINETEXTMETRIC) nr_new (unsigned char, otmsize);
	GetOutlineTextMetrics (hdc, otmsize, tfw32->otm);

	tfw32->typeface.nglyphs = tfw32->otm->otmTextMetrics.tmLastChar - tfw32->otm->otmTextMetrics.tmFirstChar + 1;

	tfw32->hgidx = NULL;
	tfw32->vgidx = NULL;
	tfw32->slots = NULL;
	tfw32->slots_length = 0;
	tfw32->slots_size = 0;
}

static void
nr_typeface_w32_finalize (NRObject *object)
{
    NRTypeFaceW32 *tfw32;

    tfw32 = (NRTypeFaceW32 *) object;

    nr_free (tfw32->otm);
    DeleteFont (tfw32->hfont);


    if (tfw32->slots) {
        unsigned int i;
        for (i = 0; i < tfw32->slots_length; i++) {
            if (tfw32->slots[i].outline.path > 0) {
				art_free (tfw32->slots[i].outline.path);
            }
        }
		nr_free (tfw32->slots);
    }
    if (tfw32->hgidx) nr_free (tfw32->hgidx);
    if (tfw32->vgidx) nr_free (tfw32->vgidx);

	((NRObjectClass *) (parent_class))->finalize (object);
}

static unsigned int
nr_typeface_w32_attribute_get (NRTypeFace *tf, const unsigned char *key, unsigned char *str, unsigned int size)
{
	NRTypeFaceW32 *tfw32;
	const unsigned char *val;
	int len;

	tfw32 = (NRTypeFaceW32 *) tf;

	if (!strcmp (key, "name")) {
		val = tf->def->name;
	} else if (!strcmp (key, "family")) {
		val = tf->def->family;
	} else if (!strcmp (key, "weight")) {
        switch (tfw32->logfont->lfWeight) {

        case FW_THIN:

                val = "thin";

                break;

        case FW_ULTRALIGHT:

                val = "ultra light";

                break;

        case FW_LIGHT:

                val = "light";

                break;

        case FW_NORMAL:

                val = "normal";

                break;

        case FW_MEDIUM:

                val = "medium";

                break;

        case FW_DEMIBOLD:

                val = "demi bold";

                break;

        case FW_BOLD:

                val = "bold";

                break;

        case FW_ULTRABOLD:

                val = "ultra bold";

                break;

        case FW_BLACK:

                val = "black";

                break;

        default:

  		val = "normal";
  		        break;

  		}

	} else if (!strcmp (key, "style")) {
	    if (tfw32->logfont->lfItalic) {

                val = "italic";

        } else {

		/* fixme: */
		val = "normal";
        }

	} else {
		val = "";
	}

	len = MIN (size - 1, strlen (val));
	if (len > 0) memcpy (str, val, len);

	if (size > 0) str[len] = '\0';


	return strlen (val);
}

static NRBPath *
nr_typeface_w32_glyph_outline_get (NRTypeFace *tf, unsigned int glyph, unsigned int metrics, NRBPath *d, unsigned int ref)
{
	NRTypeFaceW32 *tfw32;
	NRTypeFaceGlyphW32 *slot;


	tfw32 = (NRTypeFaceW32 *) tf;

	slot = nr_typeface_w32_ensure_slot (tfw32, glyph, metrics);


	if (slot) {

		nr_typeface_w32_ensure_outline (tfw32, slot, glyph, metrics);

		if (slot->olref >= 0) {

			if (ref) {

				slot->olref += 1;

			} else {

				slot->olref = -1;

			}

		}

		*d = slot->outline;

	} else {

		d->path = NULL;

	}


	return d;

}

static void
nr_typeface_w32_glyph_outline_unref (NRTypeFace *tf, unsigned int glyph, unsigned int metrics)
{
	NRTypeFaceW32 *tfw32;

	NRTypeFaceGlyphW32 *slot;



	tfw32 = (NRTypeFaceW32 *) tf;



	slot = nr_typeface_w32_ensure_slot (tfw32, glyph, metrics);



	if (slot && slot->olref > 0) {

		slot->olref -= 1;

		if (slot->olref < 1) {

			nr_free (slot->outline.path);

			slot->outline.path = NULL;

		}

	}

}

static NRPointF *
nr_typeface_w32_glyph_advance_get (NRTypeFace *tf, unsigned int glyph, unsigned int metrics, NRPointF *adv)
{
	NRTypeFaceW32 *tfw32;
	NRTypeFaceGlyphW32 *slot;


	tfw32 = (NRTypeFaceW32 *) tf;

	slot = nr_typeface_w32_ensure_slot (tfw32, glyph, metrics);


	if (slot) {

		*adv = slot->advance;

    return adv;
	}



	return NULL;

}

static unsigned int
nr_typeface_w32_lookup (NRTypeFace *tf, unsigned int rule, unsigned int unival)
{
	NRTypeFaceW32 *tfw32;
	const unsigned short *uc2cp;

	unsigned int uc2cp_size;

	unsigned int vval;


	tfw32 = (NRTypeFaceW32 *) tf;

	uc2cp = tt_cp1252;

	uc2cp_size = tt_cp1252_size;



	switch (tfw32->logfont->lfCharSet) {

	    case ANSI_CHARSET:

	         uc2cp = tt_cp1252;

	         uc2cp_size = tt_cp1252_size;

	         break;

	    case BALTIC_CHARSET:

	         uc2cp = tt_cp1257;

	         uc2cp_size = tt_cp1257_size;

	         break;

	    case CHINESEBIG5_CHARSET:

	         uc2cp = tt_cp950;

	         uc2cp_size = tt_cp950_size;

	         break;

	    case DEFAULT_CHARSET:

	         break;

	    case EASTEUROPE_CHARSET:

	         uc2cp = tt_cp1250;

	         uc2cp_size = tt_cp1250_size;

	         break;

	    case GB2312_CHARSET:

	         uc2cp = tt_cp936;

	         uc2cp_size = tt_cp936_size;

	         break;

	    case GREEK_CHARSET:

	         uc2cp = tt_cp1253;

	         uc2cp_size = tt_cp1253_size;

	         break;
#ifdef HANGUL_CHARSET
	    case HANGUL_CHARSET:

	         uc2cp = tt_cp949;

	         uc2cp_size = tt_cp949_size;

	         break;
#endif
	    case MAC_CHARSET:

	         break;

	    case OEM_CHARSET:

	         break;

	    case RUSSIAN_CHARSET:

	         uc2cp = tt_cp1251;

	         uc2cp_size = tt_cp1251_size;

	         break;

	    case SHIFTJIS_CHARSET:

	         uc2cp = tt_cp932;

	         uc2cp_size = tt_cp932_size;

	         break;

	    case SYMBOL_CHARSET:

	         break;

	    case TURKISH_CHARSET:

	         uc2cp = tt_cp1254;

	         uc2cp_size = tt_cp1254_size;

	         break;

	    case VIETNAMESE_CHARSET:

	         uc2cp = tt_cp1258;

	         uc2cp_size = tt_cp1258_size;

	         break;

	    default:

	         break;

	}



	if (unival >= uc2cp_size) unival = 0;

	vval = uc2cp[unival];

	vval = CLAMP (vval, tfw32->otm->otmTextMetrics.tmFirstChar, tfw32->otm->otmTextMetrics.tmLastChar);

	/* printf ("unival %x is vendor %x\n", unival, vval); */

	/* fixme: Use real lookup tables etc. */

    return vval - tfw32->otm->otmTextMetrics.tmFirstChar;

}


static NRFont *
nr_typeface_w32_font_new (NRTypeFace *tf, unsigned int metrics, NRMatrixF *transform)
{
	NRTypeFaceW32 *tfw32;
	NRFont *font;
	float size;

	tfw32 = (NRTypeFaceW32 *) tf;
	size = (float) NR_MATRIX_DF_EXPANSION (transform);

	font = tfw32->fonts;
	while (font != NULL) {
		if (NR_DF_TEST_CLOSE (size, font->size, 0.001 * size) && (font->metrics == metrics)) {
			return nr_font_ref (font);
		}
		font = font->next;
	}
	
	font = nr_font_generic_new (tf, metrics, transform);

	font->next = tfw32->fonts;
	tfw32->fonts = font;

	return font;
}

static void
nr_typeface_w32_font_free (NRFont *font)
{
	NRTypeFaceW32 *tfw32;

	tfw32 = (NRTypeFaceW32 *) font->face;

	if (tfw32->fonts == font) {
		tfw32->fonts = font->next;
	} else {
		NRFont *ref;
		ref = tfw32->fonts;
		while (ref->next != font) ref = ref->next;
		ref->next = font->next;
	}

	font->next = NULL;

	nr_font_generic_free (font);
}

/* W32 initialization */

static int CALLBACK
nr_type_w32_inner_enum_proc (ENUMLOGFONTEX *elfex, NEWTEXTMETRICEX *tmex, DWORD fontType, LPARAM lParam)
{
    unsigned char *name;

	switch (elfex->elfLogFont.lfCharSet) {

	    case MAC_CHARSET:

	    case OEM_CHARSET:

	    case SYMBOL_CHARSET:

	         return 1;

	         break;

	    default:

	         break;

	}



    if (!g_hash_table_lookup (familydict, elfex->elfLogFont.lfFaceName)) {
        unsigned char *s;
        /* Register family */
        s = g_strdup (elfex->elfLogFont.lfFaceName);
        familylist = g_slist_prepend (familylist, s);
        g_hash_table_insert (familydict, s, GUINT_TO_POINTER (TRUE));
    }

    name = g_strdup_printf ("%s %s", elfex->elfLogFont.lfFaceName, elfex->elfStyle);
    if (!g_hash_table_lookup (namedict, name)) {
        LOGFONT *plf;
        plf = g_new (LOGFONT, 1);
        *plf = elfex->elfLogFont;
        namelist = g_slist_prepend (namelist, name);
        g_hash_table_insert (namedict, name, plf);

        g_print ("%s | ", name);
    } else {
        g_free (name);
    }

    return 1;
}

static int CALLBACK
nr_type_w32_typefaces_enum_proc (LOGFONT *lfp, TEXTMETRIC *metrics, DWORD fontType, LPARAM lParam)
{
    if (fontType == TRUETYPE_FONTTYPE) {
        LOGFONT lf;

        lf = *lfp;

        EnumFontFamiliesExA (hdc, &lf, (FONTENUMPROC) nr_type_w32_inner_enum_proc, lParam, 0);
    }

    return 1;
}

static void
nr_type_w32_init (void)
{
    LOGFONT logfont;
    GSList *l;
    int pos;

    g_print ("Loading W32 type directory...\n");

    hdc = CreateDC ("DISPLAY", NULL, NULL, NULL);

    familydict = g_hash_table_new (g_str_hash, g_str_equal);
    namedict = g_hash_table_new (g_str_hash, g_str_equal);

    /* read system font directory */
    memset (&logfont, 0, sizeof (LOGFONT));
    logfont.lfCharSet = DEFAULT_CHARSET;
    EnumFontFamiliesExA (hdc, &logfont, (FONTENUMPROC) nr_type_w32_typefaces_enum_proc, 0, 0);

    /* Fill in lists */
    NRW32Families.length = g_slist_length (familylist);
    NRW32Families.names = g_new (unsigned char *, NRW32Families.length);
    pos = 0;
    for (l = familylist; l != NULL; l = l->next) {
        NRW32Families.names[pos] = (unsigned char *) l->data;
        pos += 1;
    }
    NRW32Typefaces.length = g_slist_length (namelist);
    NRW32Typefaces.names = g_new (unsigned char *, NRW32Typefaces.length);
    pos = 0;
    for (l = namelist; l != NULL; l = l->next) {
        NRW32Typefaces.names[pos] = (unsigned char *) l->data;
        pos += 1;
    }

    w32i = TRUE;
}

static NRTypeFaceGlyphW32 *
nr_typeface_w32_ensure_slot (NRTypeFaceW32 *tfw32, unsigned int glyph, unsigned int metrics)
{
	int gidx;

	if (metrics == NR_TYPEFACE_METRICS_VERTICAL) {
		if (!tfw32->vgidx) {
		unsigned int i;
			tfw32->vgidx = nr_new (int, tfw32->typeface.nglyphs);

		for (i = 0; i < tfw32->typeface.nglyphs; i++) {

				tfw32->vgidx[i] = -1;

		}

	}

		gidx = tfw32->vgidx[glyph];

	} else {

		if (!tfw32->hgidx) {

			unsigned int i;

			tfw32->hgidx = nr_new (int, tfw32->typeface.nglyphs);

			for (i = 0; i < tfw32->typeface.nglyphs; i++) {

				tfw32->hgidx[i] = -1;

			}

		}

		gidx = tfw32->hgidx[glyph];

	}



	if (gidx < 0) {

		NRTypeFaceGlyphW32 *slot;

		static MAT2 mat = {{0, 1}, {0, 0}, {0, 0}, {0, 1}};

		GLYPHMETRICS gmetrics;



		if (!tfw32->slots) {

			tfw32->slots = nr_new (NRTypeFaceGlyphW32, 8);

			tfw32->slots_size = 8;

		} else if (tfw32->slots_length >= tfw32->slots_size) {

			tfw32->slots_size += NR_SLOTS_BLOCK;

			tfw32->slots = nr_renew (tfw32->slots, NRTypeFaceGlyphW32, tfw32->slots_size);

		}



		slot = tfw32->slots + tfw32->slots_length;



		/* Have to select font */

		SelectFont (hdc, tfw32->hfont);

		GetGlyphOutline (hdc, glyph + tfw32->otm->otmTextMetrics.tmFirstChar, GGO_METRICS, &gmetrics, 0, NULL, &mat);



		if (metrics == NR_TYPEFACE_METRICS_VERTICAL) {

			slot->area.x0 = -0.5 * gmetrics.gmBlackBoxX;

			slot->area.x1 =  0.5 * gmetrics.gmBlackBoxX;

			slot->area.y1 = gmetrics.gmptGlyphOrigin.y - 1000.0;

			slot->area.y0 = slot->area.y1 - gmetrics.gmBlackBoxY;

			slot->advance.x = 0.0;

        		slot->advance.y = -1000.0;

		} else {
			slot->area.x0 = (float) gmetrics.gmptGlyphOrigin.x;
			slot->area.y1 = (float) gmetrics.gmptGlyphOrigin.y;
			slot->area.x1 = slot->area.x0 + gmetrics.gmBlackBoxX;
			slot->area.y0 = slot->area.y1 - gmetrics.gmBlackBoxY;
			slot->advance.x = gmetrics.gmCellIncX;
			slot->advance.y = gmetrics.gmCellIncY;

		}



		slot->olref = 0;

		slot->outline.path = NULL;



		if (metrics == NR_TYPEFACE_METRICS_VERTICAL) {

			tfw32->vgidx[glyph] = tfw32->slots_length;

		} else {

			tfw32->hgidx[glyph] = tfw32->slots_length;

		}

		tfw32->slots_length += 1;

	}



	if (metrics == NR_TYPEFACE_METRICS_VERTICAL) {

		return tfw32->slots + tfw32->vgidx[glyph];

	} else {

		return tfw32->slots + tfw32->hgidx[glyph];

	}

}



#define FIXED_TO_FLOAT(p) ((p)->value + (double) (p)->fract / 65536.0)



static NRBPath *
nr_typeface_w32_ensure_outline (NRTypeFaceW32 *tfw32, NRTypeFaceGlyphW32 *slot, unsigned int glyph, unsigned int metrics)

{

	MAT2 mat = {{0, 1}, {0, 0}, {0, 0}, {0, 1}};

	GLYPHMETRICS gmetrics;

	int golsize;

	unsigned char *gol;

	LPTTPOLYGONHEADER pgh;

    LPTTPOLYCURVE pc;

	ArtBpath bpath[8192];

    ArtBpath *bp;

	int pos, stop;

    double Ax, Ay, Bx, By, Cx, Cy;



	/* Have to select font */

	SelectFont (hdc, tfw32->hfont);



    golsize = GetGlyphOutline (hdc, glyph + tfw32->otm->otmTextMetrics.tmFirstChar, GGO_NATIVE, &gmetrics, 0, NULL, &mat);

    gol = nr_new (unsigned char, golsize);

    GetGlyphOutline (hdc, glyph + tfw32->otm->otmTextMetrics.tmFirstChar, GGO_NATIVE, &gmetrics, golsize, gol, &mat);



    bp = bpath;

    pos = 0;

    while (pos < golsize) {

        double Sx, Sy;

        pgh = (LPTTPOLYGONHEADER) (gol + pos);

        stop = pos + pgh->cb;

        /* Initialize current position */

        Ax = FIXED_TO_FLOAT (&pgh->pfxStart.x);

        Ay = FIXED_TO_FLOAT (&pgh->pfxStart.y);

        /* Always starts with moveto */

        bp->code = ART_MOVETO;

        bp->x3 = Ax;

        bp->y3 = Ay;

        bp += 1;

        Sx = Ax;

        Sy = Ay;

        pos = pos + sizeof (TTPOLYGONHEADER);

        while (pos < stop) {

            pc = (LPTTPOLYCURVE) (gol + pos);

            if (pc->wType == TT_PRIM_LINE) {

                int i;

                for (i = 0; i < pc->cpfx; i++) {

                    Cx = FIXED_TO_FLOAT (&pc->apfx[i].x);

                    Cy = FIXED_TO_FLOAT (&pc->apfx[i].y);

                    bp->code = ART_LINETO;

                    bp->x3 = Cx;

                    bp->y3 = Cy;

                    bp += 1;

                    Ax = Cx;

                    Ay = Cy;

                }

            } else if (pc->wType == TT_PRIM_QSPLINE) {

                int i;

                for (i = 0; i < (pc->cpfx - 1); i++) {

                    Bx = FIXED_TO_FLOAT (&pc->apfx[i].x);

                    By = FIXED_TO_FLOAT (&pc->apfx[i].y);

                    if (i < (pc->cpfx - 2)) {

                        Cx = (Bx + FIXED_TO_FLOAT (&pc->apfx[i + 1].x)) / 2;

                        Cy = (By + FIXED_TO_FLOAT (&pc->apfx[i + 1].y)) / 2;

                    } else {

                        Cx = FIXED_TO_FLOAT (&pc->apfx[i + 1].x);

                        Cy = FIXED_TO_FLOAT (&pc->apfx[i + 1].y);

                    }

                    bp->code = ART_CURVETO;

                    bp->x1 = Bx - (Bx - Ax) / 3;

                    bp->y1 = By - (By - Ay) / 3;

                    bp->x2 = Bx + (Cx - Bx) / 3;

                    bp->y2 = By + (Cy - By) / 3;

                    bp->x3 = Cx;

                    bp->y3 = Cy;

                    bp += 1;

                    Ax = Cx;

                    Ay = Cy;

                }

            }

            pos += sizeof (TTPOLYCURVE) + (pc->cpfx - 1) * sizeof (POINTFX);

        }

        if ((Cx != Sx) || (Cy != Sy)) {

            bp->code = ART_LINETO;

            bp->x3 = Sx;

            bp->y3 = Sy;

            bp += 1;

        }

    }



    bp->code = ART_END;



	if (metrics == NR_TYPEFACE_METRICS_VERTICAL) {

		double a[6];

		static MAT2 mat = {{0, 1}, {0, 0}, {0, 0}, {0, 1}};

		GLYPHMETRICS gmetrics;

		/* Have to select font */

		SelectFont (hdc, tfw32->hfont);

		GetGlyphOutline (hdc, glyph + tfw32->otm->otmTextMetrics.tmFirstChar, GGO_METRICS, &gmetrics, 0, NULL, &mat);

		art_affine_translate (a, -0.5 * gmetrics.gmBlackBoxX - gmetrics.gmptGlyphOrigin.x,

			gmetrics.gmptGlyphOrigin.y - 1000.0 - gmetrics.gmptGlyphOrigin.y);

		slot->outline.path = art_bpath_affine_transform (bpath, a);

	} else {

		slot->outline.path = art_new (ArtBpath, bp - bpath + 1);

    memcpy (slot->outline.path, bpath, (bp - bpath + 1) * sizeof (ArtBpath));

	}


    nr_free (gol);



    return &slot->outline;

}


