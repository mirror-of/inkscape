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
#include <libarikkei/arikkei-token.h>
#include <libnr/nr-macros.h>
#include <libnr/nr-values.h>
#include "nr-type-primitives.h"
#include "nr-type-ft2.h"
#ifdef WITH_GNOME_PRINT
#include "nr-type-gnome.h"
#endif
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

typedef struct _NRFamilyDef NRFamilyDef;

struct _NRFamilyDef {
	NRFamilyDef *next;
	gchar *name;
	NRTypeFaceDef *faces;
};

static void nr_type_directory_build (void);
static unsigned nr_type_distance_family (gchar const *ask, gchar const *bid);
static double nr_type_distance_position (NRTypePosDef const *ask, NRTypePosDef const *bid);

static void nr_type_read_private_list (void);
#ifdef WIN32
void nr_type_read_w32_list (void);
#endif

static NRTypeDict *typedict = NULL;
static NRTypeDict *familydict = NULL;

static NRFamilyDef *families = NULL;

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

NRTypeFace *
nr_type_directory_lookup_fuzzy(gchar const *family, NRTypePosDef apos)
{
	NRFamilyDef *fdef, *bestfdef;
	NRTypeFaceDef *tdef, *besttdef;

	if (!typedict) nr_type_directory_build ();

	unsigned fbest = ~0u;
	bestfdef = NULL;

	for (fdef = families; fdef; fdef = fdef->next) {
		unsigned const dist = nr_type_distance_family (family, fdef->name);
		if (dist < fbest) {
			fbest = dist;
			bestfdef = fdef;
			if (dist == 0) {
				break;
			}
		}
	}

	if (!bestfdef) return NULL;

	double best = NR_HUGE;
	besttdef = NULL;

	/* fixme: In reality the latter method reqires full qualified name */

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

	if (!besttdef->typeface) {
		besttdef->typeface = nr_typeface_new (besttdef);
	} else {
		nr_typeface_ref (besttdef->typeface);
	}

	return besttdef->typeface;
}

void
nr_type_directory_forget_face (NRTypeFace *tf)
{
	tf->def->typeface = NULL;
}

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
	flist->names = fnames;
	flist->destructor = NULL;

	return flist;
}

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

static void
nr_type_directory_style_list_destructor (NRNameList *list)
{
	if (list->names) nr_free (list->names);
}

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
		styles->names = nr_new (gchar *, styles->length);
		pos = 0;
		for (tdef = fdef->faces; tdef; tdef = tdef->next) {
			styles->names[pos] = tdef->name;
			pos += 1;
		}
	} else {
		styles->length = 0;
		styles->names = NULL;
	}

	return styles;
}

static int
nr_type_family_def_compare (const void *a, const void *b)
{
#ifndef WIN32
	return strcasecmp ((*((NRFamilyDef **) a))->name, (*((NRFamilyDef **) b))->name);
#else
	return stricmp ((*((NRFamilyDef **) a))->name, (*((NRFamilyDef **) b))->name);
#endif
}

static void
nr_type_directory_build (void)
{
	NRFamilyDef *fdef, **ffdef;
	NRTypePosDef *pdefs;
	int fnum, tnum, pos, i;

	typedict = nr_type_dict_new ();
	familydict = nr_type_dict_new ();

	nr_type_read_private_list ();

#ifdef WIN32
	nr_type_read_w32_list ();
#endif

#ifdef WITH_XFT
	nr_type_read_xft_list ();
#endif

#ifdef WITH_GNOME_PRINT
	nr_type_read_gnome_list ();
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
			*tdef->pdef = NRTypePosDef(tdef->name);
			++pos;
		}
	}
}

static unsigned
nr_type_distance_family (const gchar *ask, const gchar *bid)
{
	if (!g_ascii_strcasecmp (ask, bid)) {
		return 0;
	}

	size_t const alen = strlen (ask);
	size_t const blen = strlen (bid);
	if ( ( blen < alen )
	     && !g_ascii_strncasecmp(ask, bid, blen) ) {
		return 1;
	} else if (!g_ascii_strcasecmp(bid, "bitstream cyberbit")) {
		return 10;
	} else if (!g_ascii_strcasecmp (bid, "arial")) {
		return 100;
	} else if (!g_ascii_strcasecmp (bid, "helvetica")) {
		return 1000;
	} else {
		return 10000;
	}
}

#define NR_TYPE_ITALIC_SCALE 10000.0F
#define NR_TYPE_OBLIQUE_SCALE 1000.0F
#define NR_TYPE_WEIGHT_SCALE 100.0F
#define NR_TYPE_STRETCH_SCALE 2000.0F

static double
nr_type_distance_position (NRTypePosDef const *ask, NRTypePosDef const *bid)
{
	double ditalic, doblique, dweight, dstretch;
	double dist;

	ditalic = NR_TYPE_ITALIC_SCALE * ((int) ask->italic - (int) bid->italic);
	doblique = NR_TYPE_OBLIQUE_SCALE * ((int) ask->oblique - (int) bid->oblique);
	dweight = NR_TYPE_WEIGHT_SCALE * ((int) ask->weight - (int) bid->weight);
	dstretch = NR_TYPE_STRETCH_SCALE * ((int) ask->stretch - (int) bid->stretch);

	dist = sqrt (ditalic * ditalic  +
		     doblique * doblique  +
		     dweight * dweight  +
		     dstretch * dstretch);

	return dist;
}

static gchar privatename[] = "/.inkscape/private-fonts";

/*#if 0*/
#if defined (_WIN32) || defined(__WIN32__)
static unsigned int
nr_type_next_token (const gchar *img, unsigned int len, unsigned int p, int *tokenp)
{
	/* Skip whitespace */
	while (((img[p] == ' ') || (img[p] == '\t')) && (p < len)) p++;
	if (p >= len) return p;
	if (!isalnum (img[p]) && (img[p] != '/')) return p;
	*tokenp = p;
	while (!iscntrl (img[p]) && (img[p] != ',') && (p < len)) p++;
	return p;
}
#endif

#include <unistd.h>
#if !defined (_WIN32) && !defined(__WIN32__)
#include <sys/mman.h>
#else
#endif

static void
nr_type_read_private_list (void)
{
	gchar *homedir, *filename;
	int len;
	struct stat st;

	homedir = getenv ("HOME");
	if (!homedir) return;
	len = strlen (homedir);
	filename = nr_new (gchar, len + sizeof (privatename) + 1);
	strcpy (filename, homedir);
	strcpy (filename + len, privatename);

#ifndef S_ISREG
#define S_ISREG(st) 1
#endif

#ifndef O_BINARY
#define O_BINARY 0
#endif

/*#if 1*/
#if !defined (_WIN32) && !defined(__WIN32__)
	if (!stat (filename, &st) && S_ISREG (st.st_mode) && (st.st_size > 8)) {
		gchar *cdata;
		ArikkeiToken ft, lt;
		int fd;
		fd = open (filename, O_RDONLY | O_BINARY);
		if (!fd) return;
		cdata = (gchar*)mmap (NULL, st.st_size, PROT_READ, MAP_SHARED, fd, 0);
		close (fd);
		if ((cdata == NULL) || (cdata == (gchar *) -1)) return;
		arikkei_token_set_from_data (&ft, cdata, 0, st.st_size);
		arikkei_token_get_first_line (&ft, &lt);
		while (lt.start < lt.end) {
			if (!arikkei_token_is_empty (&lt) && (lt.cdata[lt.start] != '#')) {
				ArikkeiToken tokens[4];
				int ntokens;
				ntokens = arikkei_token_tokenize_ws (&lt, tokens, 4, ",", FALSE);
				if (ntokens >= 3) {
					ArikkeiToken fnt[2];
					ArikkeiToken filet, namet, familyt;
					int nfnt, face;
					nfnt = arikkei_token_tokenize_ws (&tokens[0], fnt, 2, ":", FALSE);
					arikkei_token_strip (&fnt[0], &filet);
					arikkei_token_strip (&tokens[1], &namet);
					arikkei_token_strip (&tokens[2], &familyt);
					face = 0;
					if (nfnt > 0) {
						gchar b[32];
						arikkei_token_strncpy (&fnt[1], b, 32);
						face = atoi (b);
					}
					if (!arikkei_token_is_empty (&filet) &&
					    !arikkei_token_is_empty (&namet) &&
					    !arikkei_token_is_empty (&familyt)) {
						NRTypeFaceDefFT2 *dft2;
						gchar f[1024], n[1024], m[1024];
						dft2 = nr_new (NRTypeFaceDefFT2, 1);
						dft2->def.next = NULL;
						dft2->def.pdef = NULL;
						arikkei_token_strncpy (&filet, f, 1024);
						arikkei_token_strncpy (&namet, n, 1024);
						arikkei_token_strncpy (&familyt, m, 1024);
						nr_type_ft2_build_def (dft2, f, n, m, face);
						nr_type_register ((NRTypeFaceDef *) dft2);
						printf ("Regstered %s : %d, %s, %s\n", f, face, n, m);
					}
				}
			}
			arikkei_token_next_line (&ft, &lt, &lt);
		}
		munmap (cdata, st.st_size);
	}
#else
	if (!stat (filename, &st) && S_ISREG (st.st_mode) && (st.st_size > 8)) {
		gchar *img;
		int fh, rbytes, nentries, p;
		img = nr_new (gchar, st.st_size + 1);
		if (!img) return;
		fh = open (filename, O_RDONLY);
		if (fh < 1) return;
		rbytes = read (fh, img, st.st_size);
		close (fh);
		if (rbytes < st.st_size) return;
		*(img + st.st_size) = 0;

		/* format: file, name, family */
		nentries = 0;
		p = 0;
		while (p < st.st_size) {
			int filep, namep, familyp;
			int e0, e1, e2;
			filep = -1;
			namep = -1;
			familyp = -1;
			/* File */
			p = nr_type_next_token (img, st.st_size, p, &filep);
			if (p >= st.st_size) break;
			if (!iscntrl (img[p])) {
				e0 = p;
				p += 1;
				/* Name */
				p = nr_type_next_token (img, st.st_size, p, &namep);
				if (p >= st.st_size) break;
				if (!iscntrl (img[p])) {
					e1 = p;
					p += 1;
					/* Family */
					p = nr_type_next_token (img, st.st_size, p, &familyp);
					e2 = p;
					p += 1;
					if ((filep >= 0) && (namep >= 0) && (familyp >= 0)) {
						struct stat st;
						if (!stat (filename, &st) && S_ISREG (st.st_mode)) {
							NRTypeFaceDefFT2 *dft2;
							int face;
							char *cp;
							img[e0] = 0;
							img[e1] = 0;
							img[e2] = 0;
							cp = strchr (img + filep, ':');
							if (cp) {
								*cp = 0;
								face = atoi (cp + 1);
							} else {
								face = 0;
							}
							/* printf ("Found %s | %d | %s | %s\n", img + filep, face, img + namep, img + familyp); */
							dft2 = nr_new (NRTypeFaceDefFT2, 1);
							dft2->def.next = NULL;
							dft2->def.pdef = NULL;
							nr_type_ft2_build_def (dft2, img + namep, img + familyp, img + filep, face);
							nr_type_register ((NRTypeFaceDef *) dft2);
							nentries += 1;
						}
					}
				}
			}
			while (iscntrl (img[p]) && (p < st.st_size)) p++;
		}

		if (nentries > 0) {
		}
	}
#endif

	nr_free (filename);
}

NRTypeFace *
nr_type_build (const gchar *name, const gchar *family,
	       const gchar *data, unsigned int size, unsigned int face)
{
	NRTypeFaceDefFT2 *dft2;

	if (!typedict) nr_type_directory_build ();

	dft2 = nr_new (NRTypeFaceDefFT2, 1);
	dft2->def.next = NULL;
	dft2->def.pdef = NULL;
	nr_type_ft2_build_def_data (dft2, name, family, data, size, face);
	nr_type_register ((NRTypeFaceDef *) dft2);

	return nr_type_directory_lookup (name);
}

