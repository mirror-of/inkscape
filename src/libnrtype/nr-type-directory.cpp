#define __NR_TYPE_DIRECTORY_C__

/*
 * Typeface and script library
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * This code is in public domain
 */

#define noTFDEBUG

#include <config.h>

#include <math.h>

#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#ifndef WIN32
#include <unistd.h>
#endif
#include <stdio.h>
#include <ctype.h>
#include <libnr/nr-macros.h>
#include <libnr/nr-values.h>
#include "nr-type-primitives.h"
#include "nr-type-ft2.h"
#ifdef WITH_XFT
#include "nr-type-xft.h"
#endif
#ifdef WIN32
#include "nr-type-w32.h"
#endif

#ifndef S_ISDIR
#define S_ISDIR(m) (((m) & _S_IFMT) == _S_IFDIR)
#define S_ISREG(m) (((m) & _S_IFMT) == _S_IFREG)
#endif

#include "nr-type-directory.h"
#include "nr-type-pos-def.h"

/**
 * 
 */
struct NRFamilyDef {
	NRFamilyDef *next;
	gchar *name;
	NRTypeFaceDef *faces;
};

static void nr_type_directory_build (void);
static unsigned nr_type_distance_family (gchar const *ask, gchar const *bid);
static double nr_type_distance_position (NRTypePosDef const *ask, NRTypePosDef const *bid);

#ifdef WIN32
void nr_type_read_w32_list (void);
#endif

/**
 * Global dictionaries of font types and families
 */
static NRTypeDict *typedict = NULL;
static NRTypeDict *familydict = NULL;

static NRFamilyDef *families = NULL;

// Store the list of families and styles for which we have already complained, so as to avoid duplication
GSList *family_warnings = NULL;
GSList *style_warnings = NULL;

/**
 * Looks up the typeface for the given name from the typeface directory.
 * If the directory doesn't exist yet, it first builds it.  If the typeface
 * hasn't been created, it creates it.  
 *
 * Returns NULL if it wasn't able to find the typeface.
 */
NRTypeFace *
nr_type_directory_lookup (const gchar *name)
{
	NRTypeFaceDef *tdef;

	if (!typedict) nr_type_directory_build ();

	tdef = (NRTypeFaceDef *)nr_type_dict_lookup (typedict, name);

	if (tdef) {
		if (!tdef->typeface) {
			tdef->typeface = nr_typeface_new (tdef);
		} else {
			nr_typeface_ref (tdef->typeface);
		}
		return tdef->typeface;
	}

	return NULL;
}

/**
 * Does a strcmp(a, b) and returns result
 */
gint 
compare_warnings (const void *a, const void *b)
{
	return ((gint) strcmp ((char *) a, (char *) b));
}

// A simple cache implementation, to speed up fuzzy lookup for documents with lots of text objects

struct cache_unit{
	gchar *fam;
	guint spec;
	NRTypeFaceDef *face;
};

/**
 * Returns an int made up of the font properties (italic, oblique, weight, etc.)
 * for the given font type pos definition.
 */
guint 
spec_from_def (NRTypePosDef a)
{
	return a.italic + a.oblique * 10 + a.variant * 100 + a.weight * 1000 + a.stretch * 1000000;
}

/**
 * A global cache list
 */
static GSList *cache = NULL;

/**
 * 
 */
gint 
compare_cache (const void *a, const void *b)
{
	if ((((cache_unit *) a)->spec == ((cache_unit *) b)->spec) 
		&& (!strcmp(((cache_unit *) a)->fam, ((cache_unit *) b)->fam))) {
		return 0;
	} else {
		return 1;
	}
}

/**
 * Searches the cache for the typeface definition matching the given
 * family and typeface pos definition.
 */
NRTypeFaceDef *
search_cache (gchar *fam, NRTypePosDef a)
{
	cache_unit c;
	c.fam = fam;
	c.spec = spec_from_def (a);
	GSList *found = g_slist_find_custom (cache, (gpointer) &c, compare_cache);
	if (found) 
		return ((cache_unit *) found->data)->face;
	else 
		return NULL;
}

/**
 * Adds the given typeface definition to the cache
 */
void
add_to_cache (gchar *fam, NRTypePosDef a, NRTypeFaceDef *face)
{
	cache_unit *p = g_new (cache_unit, 1);
	p->fam = g_strdup (fam);
	p->spec = spec_from_def (a);
	p->face = face;
	cache = g_slist_prepend (cache, p);
}


/**
 * Performs a fuzzy lookup of a typeface for the family and typeface pos definition
 */
NRTypeFace *
nr_type_directory_lookup_fuzzy(gchar const *family, NRTypePosDef apos)
{
	/* At the time of writing, all callers form their apos from a call to font_style_to_pos, so
	   we could take a SPStyle as argument instead. */
	NRFamilyDef *fdef, *bestfdef;
	NRTypeFaceDef *tdef, *besttdef;

	if (!typedict) nr_type_directory_build ();

	NRTypeFaceDef *from_cache = search_cache ((gchar *) family, apos);
	if (from_cache) {
		if (!from_cache->typeface) {
			from_cache->typeface = nr_typeface_new (from_cache);
		} else {
			nr_typeface_ref (from_cache->typeface);
		}
		return from_cache->typeface;
	}

	unsigned fbest = ~0u;
	bestfdef = NULL;

	char **familytokens = g_strsplit(family, ",", 0);

	for (int i = 0; familytokens[i] != NULL; i++) {
		char *familytoken = g_strstrip (familytokens[i]);
		for (fdef = families; fdef; fdef = fdef->next) {
			unsigned const dist = nr_type_distance_family (familytoken, fdef->name);
			if (dist < fbest) {
				fbest = dist;
				bestfdef = fdef;
				if (dist == 0) {
					break;
				}
			}
		}
	}

	if (!bestfdef) return NULL;

	// Fixme: modal box?
	if (fbest != 0 && !g_slist_find_custom (family_warnings, (gpointer) family, compare_warnings)) {
		g_warning ("font-family: No exact match for '%s', using '%s'", family, bestfdef->name);
		family_warnings = g_slist_prepend (family_warnings, (gpointer) g_strdup (family));
	}

	double best = NR_HUGE;
	besttdef = NULL;

	/* fixme: In reality the latter method reqires full qualified name (lauris) */

	for (tdef = bestfdef->faces; tdef; tdef = tdef->next) {
		double dist = nr_type_distance_position (&apos, tdef->pdef);
		if (dist < best) {
			best = dist;
			besttdef = tdef;
			if (best == 0.0) {
				break;
			}
		}
	}

	// Fixme: modal box?
	if (best != 0 && !g_slist_find_custom (style_warnings, (gpointer) besttdef->name, compare_warnings)) {
		g_warning ("In family '%s', required style not found, using '%s'", bestfdef->name, besttdef->name);
		style_warnings = g_slist_prepend (style_warnings, (gpointer) g_strdup (besttdef->name));
	}

	add_to_cache ((gchar *) family, apos, besttdef);

	if (!besttdef->typeface) {
		besttdef->typeface = nr_typeface_new (besttdef);
	} else {
		nr_typeface_ref (besttdef->typeface);
	}

	return besttdef->typeface;
}

/**
 * Removes the typeface definition from the typeface object
 * 
 * Q:  Why is this needed?
 * Q:  Will this cause a memory leak?
 */
void
nr_type_directory_forget_face (NRTypeFace *tf)
{
	tf->def->typeface = NULL;
}

/**
 * Retrieves a name list of the typeface families.
 * Builds the typeface directory if it hasn't been built yet.
 * 
 * Q:  Why is this needed?
 */
NRNameList *
nr_type_directory_family_list_get (NRNameList *flist)
{
	static int flen = 0;
	static gchar **fnames = NULL;

	if (!typedict) nr_type_directory_build ();

	if (!fnames) {
		NRFamilyDef *fdef;
		int pos;
		for (fdef = families; fdef; fdef = fdef->next) flen += 1;
		fnames = nr_new (gchar *, flen);
		pos = 0;
		for (fdef = families; fdef; fdef = fdef->next) {
			fnames[pos] = fdef->name;
			pos += 1;
		}
	}

	flist->length = flen;
	flist->names = (guchar **)fnames;
	flist->destructor = NULL;

	return flist;
}

/**
 * Registers a typeface definition.  Creates the family def as
 * well if appropriate, and inserts it into the dictionary prior
 * to inserting it into the typeface dictionary.
 *
 * Returns 0 if it's already in the dictionary, 1 otherwise.
 */
unsigned int
nr_type_register (NRTypeFaceDef *def)
{
	NRFamilyDef *fdef;

	if (nr_type_dict_lookup (typedict, def->name)) return 0;

	fdef = (NRFamilyDef *)nr_type_dict_lookup (familydict, def->family);
	if (!fdef) {
		fdef = nr_new (NRFamilyDef, 1);
		fdef->name = strdup (def->family);
		fdef->faces = NULL;
		fdef->next = families;
		families = fdef;
		nr_type_dict_insert (familydict, fdef->name, fdef);
	}

	def->next = fdef->faces;
	fdef->faces = def;

	nr_type_dict_insert (typedict, def->name, def);

	return 1;
}

/**
 * Frees any typeface names in the given named list
 */
static void
nr_type_directory_style_list_destructor (NRNameList *list)
{
	if (list->names) nr_free (list->names);
}

/**
 * Provides an implementation of strcasestr() for Win32.
 */
bool
ink_strstr (const char *haystack, const char *pneedle)
{
#ifndef WIN32
	return (strcasestr(haystack, pneedle) != NULL);
#else
	// windows has no strcasestr implementation, so here is ours...
	// stolen from nmap
	char buf[512];
	register const char *p;
	char *needle, *q, *foundto;
	if (!*pneedle) return true;
	if (!haystack) return false;

	needle = buf;
	p = pneedle; q = needle;
	while((*q++ = tolower(*p++)))
		;
	p = haystack - 1; foundto = needle;
	while(*++p) {
		if(tolower(*p) == *foundto) {
			if(!*++foundto) {
				/* Yeah, we found it */
				return true;
			}
		} else foundto = needle;
	}
	return false;
#endif
}

/**
 * Regular fonts are 'Regular', 'Roman', 'Normal', or 'Plain'
 */
// FIXME: make this UTF8, add non-English style names
bool
is_regular (const char *s)
{
	if (ink_strstr(s, "Regular")) return true;
	if (ink_strstr(s, "Roman")) return true;
	if (ink_strstr(s, "Normal")) return true;
	if (ink_strstr(s, "Plain")) return true;
	return false;
}

/**
 * Non-bold fonts are 'Medium' or 'Book'
 */
bool
is_nonbold (const char *s)
{
	if (ink_strstr(s, "Medium")) return true;
	if (ink_strstr(s, "Book")) return true;
	return false;
}

/**
 * Italic fonts are 'Italic', 'Oblique', or 'Slanted'
 */
bool
is_italic (const char *s)
{
	if (ink_strstr(s, "Italic")) return true;
	if (ink_strstr(s, "Oblique")) return true;
	if (ink_strstr(s, "Slanted")) return true;
	return false;
}

/**
 * Bold fonts are 'Bold'
 */
bool
is_bold (const char *s)
{
	if (ink_strstr(s, "Bold")) return true;
	return false;
}

/**
 * Caps fonts are 'Caps'
 */
bool
is_caps (const char *s)
{
	if (ink_strstr(s, "Caps")) return true;
	return false;
}

/**
 * Monospaced fonts are 'Mono'
 */
bool
is_mono (const char *s)
{
	if (ink_strstr(s, "Mono")) return true;
	return false;
}

/**
 * Rounded fonts are 'Round'
 */
bool
is_round (const char *s)
{
	if (ink_strstr(s, "Round")) return true;
	return false;
}

/**
 * Outline fonts are 'Outline'
 */
bool
is_outline (const char *s)
{
	if (ink_strstr(s, "Outline")) return true;
	return false;
}

/**
 * Swash fonts are 'Swash'
 */
bool
is_swash (const char *s)
{
	if (ink_strstr(s, "Swash")) return true;
	return false;
}

/**
 * Determines if two style names match.  This allows us to match
 * based on the type of style rather than simply doing string matching,
 * because for instance 'Plain' and 'Normal' mean the same thing.
 * 
 * Q:  Shouldn't this include the other tests such as is_outline, etc.?
 * Q:  Is there a problem with strcasecmp on Win32?  Should it use stricmp?
 */
static int
style_name_compare (const void *aa, const void *bb)
{
	const char *a = *((char **) aa);
	const char *b = *((char **) bb);

 if (is_regular(a) && !is_regular(b)) return -1;
 if (is_regular(b) && !is_regular(a)) return 1;

 if (is_bold(a) && !is_bold(b)) return 1;
 if (is_bold(b) && !is_bold(a)) return -1;

 if (is_italic(a) && !is_italic(b)) return 1;
 if (is_italic(b) && !is_italic(a)) return -1;

 if (is_nonbold(a) && !is_nonbold(b)) return 1;
 if (is_nonbold(b) && !is_nonbold(a)) return -1;

 if (is_caps(a) && !is_caps(b)) return 1;
 if (is_caps(b) && !is_caps(a)) return -1;

 return strcasecmp (a, b);
}


/**
 * Builds the type directory if it hasn't been built already.
 * Looks up the font definition for the given font family.
 * Assigns the directory destructor to the styles->destructor.
 * If a definition exists for the font, it gets the fonts of
 * that family and adds them to the styles->names array.
 * Otherwise, it sets styles->names to null. 
 *
 * Returns the styles object.
 */
NRNameList *
nr_type_directory_style_list_get (const gchar *family, NRNameList *styles)
{
	NRFamilyDef *fdef;

	if (!typedict) nr_type_directory_build ();

	fdef = (NRFamilyDef*)nr_type_dict_lookup (familydict, family);

	styles->destructor = nr_type_directory_style_list_destructor;

	if (fdef) {
		NRTypeFaceDef *tdef;
		int tlen, pos;
		tlen = 0;
		for (tdef = fdef->faces; tdef; tdef = tdef->next) tlen += 1;
		styles->length = tlen;
		styles->names = nr_new (guchar *, styles->length);
		pos = 0;
		for (tdef = fdef->faces; tdef; tdef = tdef->next) {
			styles->names[pos] = (guchar *)tdef->name;
			pos += 1;
		}
		qsort (styles->names, pos, sizeof (guchar *), style_name_compare);
	} else {
		styles->length = 0;
		styles->names = NULL;
	}

	return styles;
}

/**
 * On Win32 performs a stricmp(a,b), otherwise does a strcasecmp(a,b)
 */
static int
nr_type_family_def_compare (const void *a, const void *b)
{
#ifndef WIN32
	return strcasecmp ((*((NRFamilyDef **) a))->name, (*((NRFamilyDef **) b))->name);
#else
	return stricmp ((*((NRFamilyDef **) a))->name, (*((NRFamilyDef **) b))->name);
#endif
}

/**
 * Builds the typeface directory.  On Win32 it uses nr_type_read_w32_list(),
 * whereas for systems using Xft it uses nr_type_read_xft_list().
 */
static void
nr_type_directory_build (void)
{
	NRFamilyDef *fdef, **ffdef;
	NRTypePosDef *pdefs;
	int fnum, tnum, pos, i;

	typedict = nr_type_dict_new ();
	familydict = nr_type_dict_new ();

#ifdef WIN32
	nr_type_read_w32_list ();
#endif

#ifdef WITH_XFT
	nr_type_read_xft_list ();
#endif

	if (!families) {
		NRTypeFaceDef *def;
		/* No families, register empty typeface */
		def = nr_new (NRTypeFaceDef, 1);
		def->next = NULL;
		def->pdef = NULL;
		nr_type_empty_build_def (def, "empty", "Empty");
		nr_type_register (def);
	}

	/* Sort families */
	fnum = 0;
	for (fdef = families; fdef; fdef = fdef->next) fnum += 1;
	ffdef = nr_new (NRFamilyDef *, fnum);
	pos = 0;
	for (fdef = families; fdef; fdef = fdef->next) {
		ffdef[pos] = fdef;
		pos += 1;
	}
	qsort (ffdef, fnum, sizeof (NRFamilyDef *), nr_type_family_def_compare);
	for (i = 0; i < fnum - 1; i++) {
		ffdef[i]->next = ffdef[i + 1];
	}
	ffdef[i]->next = NULL;
	families = ffdef[0];
	nr_free (ffdef);

	/* Build posdefs */
	tnum = 0;
	for (fdef = families; fdef; fdef = fdef->next) {
		NRTypeFaceDef *tdef;
		for (tdef = fdef->faces; tdef; tdef = tdef->next) tnum += 1;
	}

	pdefs = nr_new (NRTypePosDef, tnum);
	pos = 0;
	for (fdef = families; fdef; fdef = fdef->next) {
		NRTypeFaceDef *tdef;
		for (tdef = fdef->faces; tdef; tdef = tdef->next) {
			tdef->pdef = pdefs + pos;
			gchar *style;
			if (!strncmp (tdef->name, tdef->family, strlen(tdef->family))) { // name starts with family, cut it off
				style = tdef->name + strlen(tdef->family);
			} else {
				style = tdef->name;
			}
			// we need the pdef to reflect the font style only, not including family name
			*tdef->pdef = NRTypePosDef(style);
			++pos;
		}
	}
}

/**
 * 
 */
/**
Return "distance" between two font family names, allowing to choose the closest match or a sensible alternative if there's no match
 */
static unsigned
nr_type_distance_family (const gchar *ask_c, const gchar *bid_c)
{
	// perfect match (modulo case), distance 0
	if (!g_ascii_strcasecmp (ask_c, bid_c)) {
		return 0;
	}

	unsigned int ret = 0;

	// lowercase so we can use strstr everywhere
	gchar *ask = g_ascii_strdown (ask_c, -1);
	gchar *bid = g_ascii_strdown (bid_c, -1);

	// take care of the generic families

	if (!strcmp (ask, "serif")) {
		if (!strcmp (bid, "bitstream vera serif") || !strcmp (bid, "luxi serif") || !strcmp (bid, "times new roman")) {
			return 0;
		} else if (strstr (bid, "times")) {
			return (strlen(bid) - strlen("times")); // give the shortest one a chance to win
		} else {
			return 100;
		}
	}

	if (!strcmp (ask, "sans-serif")) {
		if (!strcmp (bid, "bitstream vera sans") || !strcmp (bid, "luxi sans") || !strcmp (bid, "arial") || !strcmp (bid, "verdana")) {
			return 0;
		} else if (strstr (bid, "arial")) {
			return (strlen(bid) - strlen("arial")); 
		} else if (strstr (bid, "helvetica")) {
			return (strlen(bid) - strlen("helvetica")); 
		} else {
			return 100;
		}
	}

	if (!strcmp (ask, "monospace")) {
		if (!strcmp (bid, "bitstream vera sans mono") || !strcmp (bid, "luxi mono") || !strcmp (bid, "courier new") || !strcmp (bid, "andale mono")) {
			return 0;
		} else if (strstr (bid, "courier")) {
			return (strlen(bid) - strlen("courier")); 
		} else {
			return 100;
		}
	}

	size_t const alen = strlen (ask);
	size_t const blen = strlen (bid);

	if ( ( blen < alen ) && !g_ascii_strncasecmp(ask, bid, blen) ) {
	      // bid (available family) is the beginning of ask (the required family), let the closest-length match win
		ret = alen - blen;
	} else if ( ( alen < blen ) && !g_ascii_strncasecmp(bid, ask, alen) ) {
	      // aks is the beginning of bid, let the closest-length match win, but this is less desirable than vice versa
		ret = 2 * (blen - alen);
	} else if (strstr (ask, bid)) {
	      // bid is inside ask, let the closest-length match win
		ret = 4 + alen - blen;
	} else if (strstr (bid, ask)) {
	      // ask is inside bid, let the closest-length match win, but this is less desirable than vice versa
		ret = 8 + 2 * (blen - alen);
	} else if (strstr (bid, "bitstream vera sans")) {
		ret = 40;
	} else if (strstr (bid, "luxi sans")) {
		ret = 40;
	} else if (strstr (bid, "verdana")) {
		ret = 40;
	} else if (strstr (bid, "helvetica")) {
		ret = 80;
	} else if (strstr (bid, "arial")) {
		ret = 80;
	} else if (strstr (bid, "times")) {
		ret = 80;
	} else {
		ret = 1000;
	}

	// using monotype fonts instead of normal or vice versa sucks, discourage that
	if (is_mono (ask) && !is_mono (bid) || is_mono (bid) && !is_mono (ask)) {
		ret *= 2;
	}

	// same for caps
	if (is_caps (ask) && !is_caps (bid) || is_caps (bid) && !is_caps (ask)) {
		ret *= 2;
	}

	// same for swash
	if (is_swash (ask) && !is_swash (bid) || is_swash (bid) && !is_swash (ask)) {
		ret *= 2;
	}

	// same for outline
	if (is_outline (ask) && !is_outline (bid) || is_outline (bid) && !is_outline (ask)) {
		ret *= 2;
	}

	// same for round
	if (is_round (ask) && !is_round (bid) || is_round (bid) && !is_round (ask)) {
		ret *= 2;
	}

	g_free (ask);
	g_free (bid);

	return ret;
}

// These weights are defined inverse proportionally to the range of the corresponding parameters,
// so that all parameters get the same weight
#define NR_TYPE_ITALIC_SCALE 10000.0F
#define NR_TYPE_OBLIQUE_SCALE 1000.0F
#define NR_TYPE_WEIGHT_SCALE 100.0F
#define NR_TYPE_STRETCH_SCALE 2000.0F
#define NR_TYPE_VARIANT_SCALE 10000.0F

/**
 * 
 */
static double
nr_type_distance_position (NRTypePosDef const *ask, NRTypePosDef const *bid)
{
	double ditalic = 0, doblique = 0, dweight = 0, dstretch = 0, dvariant = 0;
	double dist;

	//g_print ("Ask: %d %d %d %d %d  Bid: %d %d %d %d %d\n", ask->italic, ask->oblique, ask->weight, ask->stretch, ask->variant,    bid->italic, bid->oblique, bid->weight, bid->stretch, bid->variant);

	// For oblique, match italic if oblique not found, and vice versa
      if (ask->italic || bid->italic)
				ditalic = NR_TYPE_ITALIC_SCALE * ((int) ask->italic - (int) bid->italic - 0.5 * bid->oblique);
      if (ask->oblique || bid->oblique)
				doblique = NR_TYPE_OBLIQUE_SCALE * ((int) ask->oblique - (int) bid->oblique - 0.5 * bid->italic);
      if (ask->weight || bid->weight)
				dweight = NR_TYPE_WEIGHT_SCALE * ((int) ask->weight - (int) bid->weight);
      if (ask->stretch || bid->stretch)
				dstretch = NR_TYPE_STRETCH_SCALE * ((int) ask->stretch - (int) bid->stretch);
      if (ask->variant || bid->variant)
				dvariant = NR_TYPE_VARIANT_SCALE * ((int) ask->variant - (int) bid->variant);

	dist = sqrt (ditalic * ditalic  +
		     doblique * doblique  +
		     dweight * dweight  +
		     dstretch * dstretch  +
                  dvariant * dvariant);

	return dist;
}

/**
 * 
 */
NRTypeFace *
nr_type_build (const gchar *name, const gchar *family,
	       const gchar *data, unsigned int size, unsigned int face)
{
	NRTypeFaceDefFT2 *dft2;

	if (!typedict) nr_type_directory_build ();

	dft2 = nr_new (NRTypeFaceDefFT2, 1);
	dft2->next = NULL;
	dft2->pdef = NULL;
	nr_type_ft2_build_def_data (dft2, name, family, data, size, face);
	nr_type_register ((NRTypeFaceDef *) dft2);

	return nr_type_directory_lookup (name);
}

