#define __NR_TYPE_XFT_C__

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

#include <X11/Xft/Xft.h>

#include <glib.h>
#include <gdk/gdkx.h>

#include "nr-type-directory.h"
#include "nr-type-xft.h"

static void nr_type_xft_init (void);

static unsigned int nrxfti = FALSE;

static NRNameList NRXftTypefaces = {0, NULL, NULL, NULL};
static NRNameList NRXftFamilies = {0, NULL, NULL, NULL};

static XftFontSet *NRXftPatterns = NULL;
static GHashTable *NRXftNamedict = NULL;
static GHashTable *NRXftFamilydict = NULL;

void
nr_type_xft_typefaces_get (NRNameList *names)
{
	if (!nrxfti) nr_type_xft_init ();

	*names = NRXftTypefaces;
}

void
nr_type_xft_families_get (NRNameList *names)
{
	if (!nrxfti) nr_type_xft_init ();

	*names = NRXftFamilies;
}

void
nr_type_xft_build_def (NRTypeFaceDefFT2 *dft2, const gchar *name, const gchar *family)
{
	XftPattern *pat;

	pat = (XftPattern *)g_hash_table_lookup (NRXftNamedict, name);
	if (pat) {
		char *file;
		int index;
		XftPatternGetString (pat, XFT_FILE, 0, &file);
		XftPatternGetInteger (pat, XFT_INDEX, 0, &index);
		if (file) {
			nr_type_ft2_build_def (dft2, name, family, file, index);
		}
	}
}

void
nr_type_read_xft_list (void)
{
	NRNameList gnames, gfamilies;
	const char *debugenv;
	int debug;

	debugenv = getenv ("INKSCAPE_DEBUG_XFT");
	debug = (debugenv && *debugenv && (*debugenv != '0'));

	nr_type_xft_typefaces_get (&gnames);
	nr_type_xft_families_get (&gfamilies);

	if (debug) {
		fprintf (stderr, "Number of usable Xft familes: %lu\n", gfamilies.length);
		fprintf (stderr, "Number of usable Xft typefaces: %lu\n", gnames.length);
	}

	for (int i = gnames.length - 1; i >= 0; i--) {
		const gchar *family = (gchar *) (gnames.families[i]);

		NRTypeFaceDefFT2 *tdef;
		if (family) {
			tdef = nr_new (NRTypeFaceDefFT2, 1);
			tdef->def.next = NULL;
			tdef->def.pdef = NULL;
			nr_type_xft_build_def (tdef, (gchar *)gnames.names[i], (gchar *)family);
			nr_type_register ((NRTypeFaceDef *) tdef);
		}
	}

	nr_name_list_release (&gfamilies);
	nr_name_list_release (&gnames);
}

static void
nr_type_xft_init (void)
{
	XftFontSet *fs;
	const char *debugenv;
	int debug, ret;
	int i, pos, fpos;

	debugenv = getenv ("INKSCAPE_DEBUG_XFT");
	debug = (debugenv && *debugenv && (*debugenv != '0'));

	ret = XftInit (NULL);

	if (debug) {
		fprintf (stderr, "XftInit result %d\n", ret);
		fprintf (stderr, "Reading Xft font database...\n");
	}

	/* Get family list */
	fs = XftListFonts (GDK_DISPLAY (), 0,
			   XFT_SCALABLE, XftTypeBool, 1, XFT_OUTLINE, XftTypeBool, 1, 0,
			   XFT_FAMILY, 0);
	NRXftFamilies.length = fs->nfont;
	NRXftFamilies.names = nr_new (guchar *, NRXftFamilies.length);
	NRXftFamilies.destructor = NULL;
	XftFontSetDestroy (fs);

	if (debug) {
		fprintf (stderr, "Read %lu families\n", NRXftFamilies.length);
	}

	/* Get typeface list */
	NRXftPatterns = XftListFonts (GDK_DISPLAY (), 0,
				      XFT_SCALABLE, XftTypeBool, 1, XFT_OUTLINE, XftTypeBool, 1, 0,
				      XFT_FAMILY, XFT_STYLE, XFT_WEIGHT, XFT_SLANT, XFT_FILE, XFT_INDEX, 0);
	NRXftTypefaces.length = NRXftPatterns->nfont * 2;
	NRXftTypefaces.names = nr_new (guchar *, NRXftPatterns->nfont);
	NRXftTypefaces.families = nr_new (guchar *, NRXftPatterns->nfont);
	NRXftTypefaces.destructor = NULL;
	NRXftNamedict = g_hash_table_new (g_str_hash, g_str_equal);
	NRXftFamilydict = g_hash_table_new (g_str_hash, g_str_equal);

	if (debug) {
		fprintf (stderr, "Read %lu fonts\n", NRXftTypefaces.length);
	}

	pos = 0;
	fpos = 0;
	for (i = 0; i < NRXftPatterns->nfont; i++) {
		{
			char const *name;
			XftPatternGetString (NRXftPatterns->fonts[i], XFT_FAMILY, 0, &name);
			if (debug) {
				fprintf (stderr, "%d\t Typeface %s\n", i, name);
			}
		}
		char const *file;
		XftPatternGetString (NRXftPatterns->fonts[i], XFT_FILE, 0, &file);
		if (file) {
			int len;
			if (debug) {
				fprintf (stderr, "%d\t Got filename %s\n", i, file);
			}
			len = strlen (file);
			/* fixme: This is silly and evil */
			/* But Freetype just does not load pfa reliably (Lauris) */
			/* Changed to exclude pfa, better for OSX */
			if ((len > 4) //&&
			/*
			    (!strcmp (file + len - 4, ".ttf") ||
			     !strcmp (file + len - 4, ".TTF") ||
			     !strcmp (file + len - 4, ".ttc") ||
			     !strcmp (file + len - 4, ".TTC") ||
			     !strcmp (file + len - 4, ".otf") ||
			     !strcmp (file + len - 4, ".OTF") ||
			     !strcmp (file + len - 4, ".pfb") ||
			     !strcmp (file + len - 4, ".PFB"))) {
			  */
					// Commented out; needs to be investigated whether there are really any problems with pfa! --bb
					//  strcmp (file + len - 4, ".pfa") &&
					//			     strcmp (file + len - 4, ".PFA")
					) {
				char const *fn = NULL, *styn = NULL, *wn = NULL, *sn = NULL;
				int weight;
				int slant;
				if (debug) {
					fprintf (stderr, "%d\t Seems valid\n", i);
				}
				XftPatternGetString (NRXftPatterns->fonts[i], XFT_FAMILY, 0, &fn);
				XftPatternGetString (NRXftPatterns->fonts[i], XFT_STYLE, 0, &styn);
				XftPatternGetInteger (NRXftPatterns->fonts[i], XFT_WEIGHT, 0, &weight);
				XftPatternGetInteger (NRXftPatterns->fonts[i], XFT_SLANT, 0, &slant);
				switch (weight) {
				case XFT_WEIGHT_LIGHT:
					wn = "Light";
					break;
				case XFT_WEIGHT_MEDIUM:
					wn = "Book";
					break;
				case XFT_WEIGHT_DEMIBOLD:
					wn = "Demibold";
					break;
				case XFT_WEIGHT_BOLD:
					wn = "Bold";
					break;
				case XFT_WEIGHT_BLACK:
					wn = "Black";
					break;
				default:
					wn = "";
					break;
				}
				switch (slant) {
				case XFT_SLANT_ROMAN:
					sn = "Roman";
					break;
				case XFT_SLANT_ITALIC:
					sn = "Italic";
					break;
				case XFT_SLANT_OBLIQUE:
					sn = "Oblique";
					break;
				default:
					sn = "";
					break;
				}

				char *name = g_strdup_printf (
                                    "%s%s%s%s%s%s%s", 
                                    fn, 
                                    styn ? " " : "", styn ? styn : "", 
                                    (wn && !styn) ? " " : "", (wn && !styn) ? wn : "", 
                                    (sn  && !styn) ? " " : "", (sn  && !styn) ? sn : "");

				if (!g_hash_table_lookup (NRXftNamedict, name)) {
					if (!g_hash_table_lookup (NRXftFamilydict, fn)) {
						NRXftFamilies.names[fpos] = (guchar *)g_strdup (fn);
						g_hash_table_insert (NRXftFamilydict, NRXftFamilies.names[fpos], (void *) TRUE);
						fpos++;
					}
					NRXftTypefaces.names[pos] = (guchar *) name;
					NRXftTypefaces.families[pos] = (guchar *) g_strdup (fn);
					pos++;
					g_hash_table_insert (NRXftNamedict, name, NRXftPatterns->fonts[i]);
				} else {
					// Not inserted, dupe; most likely the same font is available from different locations
					g_free (name);
				}
			}
		}
	}
	NRXftTypefaces.length = pos;
	NRXftFamilies.length = fpos;

	nrxfti = TRUE;
}
