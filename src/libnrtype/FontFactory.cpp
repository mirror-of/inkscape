/*
 *  FontFactory.cpp
 *  testICU
 *
 *
 */

#include "FontFactory.h"
#include "FontInstance.h"

#include <pango/pango.h>

#ifdef WITH_XFT
#include <pango/pangoft2.h>
#endif
#ifdef WIN32
#include <pango/pangowin32.h>
#endif

/////////////////// helper functions

/**
 * A wrapper for strcasestr that also provides an implementation for Win32.
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
	const char *a = (char *) aa;
	const char *b = (char *) bb;

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

static int
style_record_compare (const void *aa, const void *bb)
{
	NRStyleRecord *a = (NRStyleRecord *) aa;
	NRStyleRecord *b = (NRStyleRecord *) bb;

	return (style_name_compare (a->name, b->name));
}

static void font_factory_name_list_destructor (NRNameList *list) 
{
       for (unsigned int i=0; i<list->length; i++) 
		 free(list->names[i]);
       if ( list->names ) nr_free (list->names);
}

static void font_factory_style_list_destructor (NRStyleList *list) 
{
       for (unsigned int i=0; i<list->length; i++) {
           free((list->records)[i].name);
           free((list->records)[i].descr);
       }
       if ( list->records ) nr_free (list->records);
}

/**
 * On Win32 performs a stricmp(a,b), otherwise does a strcasecmp(a,b)
 */
static int
family_name_compare (const void *a, const void *b)
{
#ifndef WIN32
        return strcasecmp ((*((const char **) a)), (*((const char **) b)));
#else
        return stricmp ((*((const char **) a)), (*((const char **) b)));
#endif
}

void noop (...) {}
//#define PANGO_DEBUG g_print
#define PANGO_DEBUG noop



///////////////////// FontFactory

font_factory*  font_factory::lUsine=NULL;

font_factory*  font_factory::Default(void)
{
	if ( lUsine == NULL ) lUsine=new font_factory;
	return lUsine;
}

font_factory::font_factory(void)
{
	fontSize=512;
#ifdef WITH_XFT
	fontServer=pango_ft2_font_map_new();
	pango_ft2_font_map_set_resolution((PangoFT2FontMap*)fontServer, 72, 72);
	fontContext=pango_ft2_font_map_create_context((PangoFT2FontMap*)fontServer);
#endif
#ifdef WIN32
	fontContext=pango_win32_get_context();
	fontServer=pango_win32_font_map_for_display();
	wCache=pango_win32_font_map_get_font_cache(fontServer);
	wDevice = CreateDC ("DISPLAY", NULL, NULL, NULL);
//	wDevice=pango_win32_get_dc();
	fontSize*= 67.55223 ;
	fontSize/= ((double)GetDeviceCaps (wDevice, LOGPIXELSY));
#endif
}

font_factory::~font_factory(void)
{
#ifdef WITH_XFT
	g_object_unref(fontServer);
//	pango_ft2_shutdown_display();
#endif
#ifdef WIN32
	pango_win32_shutdown_display();
#endif
//	g_object_unref(fontContext);
}

font_instance* font_factory::FaceFromDescr(const char* descr)
{
	PangoFontDescription* temp_descr=pango_font_description_from_string(descr);
	font_instance *res=Face(temp_descr);
//	pango_font_description_free(temp_descr);
	return res;
}

font_instance* font_factory::Face(PangoFontDescription* descr, bool canFail)
{
	pango_font_description_set_size (descr, (int) (fontSize*PANGO_SCALE)); // mandatory huge size (hinting workaround)

//	char* tc=pango_font_description_to_string(descr);
//	printf("asked: %s (family=%s)\n",tc,pango_font_description_get_family(descr));
//	free(tc);
	// besides we only do outline fonts
  font_instance*  res=NULL;
  if ( loadedFaces.find(descr) == loadedFaces.end() ) {
    // not yet loaded
		PangoFont* nFace=pango_font_map_load_font(fontServer,fontContext,descr);
		if ( nFace ) {
			// duplicate FcPattern, the hard way
			res=new font_instance();
			res->descr=pango_font_description_copy(descr);
			res->daddy=this;
			res->InstallFace(nFace);
			if ( res->pFont == NULL ) {
				// failed to install face -> bitmap font
				delete res;
				res=NULL;
				if ( canFail ) {
					PANGO_DEBUG ("falling back to Sans\n");
					pango_font_description_set_family(descr,"Sans");
					res=Face(descr,false);
				}
			} else {
				loadedFaces[res->descr]=res;
				res->Ref();
			}
		} else {
			// no match
			if ( canFail ) {
				PANGO_DEBUG ("falling back to Sans\n");
				pango_font_description_set_family(descr,"Sans");
				res=Face(descr,false);
			}
		}
	} else {
    // already here
    res=loadedFaces[descr];
    res->Ref();
  }
  return res;
}

font_instance* font_factory::Face(const char* family, int variant, int style, int weight, int stretch, int /*size*/, int /*spacing*/)
{
	PangoFontDescription*  temp_descr=pango_font_description_new();
	pango_font_description_set_family(temp_descr,family);
	pango_font_description_set_weight(temp_descr,(PangoWeight)weight);
	pango_font_description_set_stretch(temp_descr,(PangoStretch)stretch);
	pango_font_description_set_style(temp_descr,(PangoStyle)style);
	pango_font_description_set_variant(temp_descr,(PangoVariant)variant);
	font_instance *res=Face(temp_descr);
	pango_font_description_free(temp_descr);
	return res;
}

font_instance* font_factory::Face(const char* family, NRTypePosDef apos)
{
	PangoFontDescription*  temp_descr=pango_font_description_new();

	pango_font_description_set_family (temp_descr, family);

	if ( apos.variant == NR_POS_VARIANT_SMALLCAPS ) {
		pango_font_description_set_variant (temp_descr, PANGO_VARIANT_SMALL_CAPS);
	} else {
		pango_font_description_set_variant (temp_descr, PANGO_VARIANT_NORMAL);
	}

	if ( apos.italic ) {
		pango_font_description_set_style (temp_descr, PANGO_STYLE_ITALIC);
	} else if ( apos.oblique ) {
		pango_font_description_set_style (temp_descr, PANGO_STYLE_OBLIQUE);
	} else {
		pango_font_description_set_style (temp_descr, PANGO_STYLE_NORMAL);
	}

	if ( apos.weight <= NR_POS_WEIGHT_ULTRA_LIGHT ) {
		pango_font_description_set_weight (temp_descr, PANGO_WEIGHT_ULTRALIGHT);
	} else if ( apos.weight <= NR_POS_WEIGHT_LIGHT ) {
		pango_font_description_set_weight (temp_descr, PANGO_WEIGHT_LIGHT);
	} else if ( apos.weight <= NR_POS_WEIGHT_NORMAL ) {
		pango_font_description_set_weight (temp_descr, PANGO_WEIGHT_NORMAL);
	} else if ( apos.weight <= NR_POS_WEIGHT_BOLD ) {
		pango_font_description_set_weight (temp_descr, PANGO_WEIGHT_BOLD);
	} else if ( apos.weight <= NR_POS_WEIGHT_ULTRA_BOLD ) {
		pango_font_description_set_weight (temp_descr, PANGO_WEIGHT_ULTRABOLD);
	} else {
		pango_font_description_set_weight (temp_descr, PANGO_WEIGHT_HEAVY);
	}

	if ( apos.stretch <= NR_POS_STRETCH_ULTRA_CONDENSED ) {
		pango_font_description_set_stretch (temp_descr, PANGO_STRETCH_EXTRA_CONDENSED);
	} else if ( apos.stretch <= NR_POS_STRETCH_CONDENSED ) {
		pango_font_description_set_stretch (temp_descr, PANGO_STRETCH_CONDENSED);
	} else if ( apos.stretch <= NR_POS_STRETCH_SEMI_CONDENSED ) {
		pango_font_description_set_stretch (temp_descr, PANGO_STRETCH_SEMI_CONDENSED);
	} else if ( apos.stretch <= NR_POS_WEIGHT_NORMAL ) {
		pango_font_description_set_stretch (temp_descr, PANGO_STRETCH_NORMAL);
	} else if ( apos.stretch <= NR_POS_STRETCH_SEMI_EXPANDED ) {
		pango_font_description_set_stretch (temp_descr, PANGO_STRETCH_SEMI_EXPANDED);
	} else if ( apos.stretch <= NR_POS_STRETCH_EXPANDED ) {
		pango_font_description_set_stretch (temp_descr, PANGO_STRETCH_EXPANDED);
	} else {
		pango_font_description_set_stretch (temp_descr, PANGO_STRETCH_EXTRA_EXPANDED);
	}

	font_instance *res = Face (temp_descr);
	pango_font_description_free (temp_descr);
	return res;
}

void font_factory::UnrefFace(font_instance* who)
{
    if ( who == NULL ) return;
    if ( loadedFaces.find(who->descr) == loadedFaces.end() ) {
        // not found
    } else {
        loadedFaces.erase(loadedFaces.find(who->descr));
    }
}

NRNameList* font_factory::Families(NRNameList *flist)
{
	PangoFontFamily**  fams=NULL;
	int                nbFam=0;
	pango_font_map_list_families(fontServer, &fams, &nbFam);

	PANGO_DEBUG ("got %d families\n", nbFam);
	
	flist->length = nbFam;
	flist->names = (guchar **)malloc(nbFam*sizeof(guchar*));
	flist->destructor = font_factory_name_list_destructor;

	for (int i=0;i<nbFam;i++) {
		bool skip_add=true;
		font_instance* test_scal=Face(pango_font_family_get_name(fams[i]));
		if ( test_scal ) {
			if ( test_scal->IsOutlineFont() ) {
				skip_add=false;
			} else {
				skip_add=true;
			}
			test_scal->Unref();
		} 
		if ( skip_add == false ) {
			flist->names[i]=(guchar*)strdup(pango_font_family_get_name(fams[i]));
		}
	}

	qsort (flist->names, nbFam, sizeof (guchar *), family_name_compare);
	
	g_free(fams);
	
	return flist;
}

NRStyleList* font_factory::Styles(const gchar *family, NRStyleList *slist)
{
	PangoFontFamily* theFam=NULL;

	// search available families
	{
		PangoFontFamily**  fams=NULL;
		int                nbFam=0;
		pango_font_map_list_families(fontServer, &fams, &nbFam);
		
		for (int i=0;i<nbFam;i++) {
			const char* fname=pango_font_family_get_name(fams[i]);
			if ( fname && strcmp(family,fname) == 0 ) {
				theFam=fams[i];
				break;
			}
		}
		
		g_free(fams);
	}

	// nothing found
	if ( theFam == NULL ) {
		slist->length = 0;
		slist->records = NULL;
		slist->destructor = NULL;
		return slist;
	}

	// search faces in the found family
	PangoFontFace**  faces=NULL;
	int nFaces=0;
	pango_font_family_list_faces (theFam, &faces, &nFaces);

	slist->records = (NRStyleRecord *) malloc (nFaces * sizeof (NRStyleRecord));
	slist->destructor = font_factory_style_list_destructor;

	int nr = 0;
	for (int i=0; i<nFaces; i++) {

                // no unnamed faces
                if (pango_font_face_get_face_name (faces[i]) == NULL)
                    continue;
                if (pango_font_face_describe(faces[i]) == NULL)
                    continue;
                if (pango_font_description_to_string (pango_font_face_describe(faces[i])) == NULL)
                    continue;

                guchar *name = (guchar *) g_strdup (pango_font_face_get_face_name (faces[i]));
                guchar *descr = (guchar *) g_strdup (pango_font_description_to_string (pango_font_face_describe(faces[i])));

                // no duplicates
                for (int j = 0; j < nr; j ++) {
 			 if (!strcmp ((const char *) ((slist->records)[j].name), (const char *) name)) {
                                continue;
 			 }
                }

                slist->records[nr].name = name;
                slist->records[nr].descr = descr;
                nr ++;
	}

	slist->length = nr;

	qsort (slist->records, slist->length, sizeof (NRStyleRecord), style_record_compare);
	
	g_free(faces);
	
	return slist;
}

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=c++:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :
