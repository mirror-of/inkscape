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

void noop (...) {}
//#define PANGO_DEBUG g_print
#define PANGO_DEBUG noop

font_factory*  font_factory::lUsine=NULL;

font_factory*  font_factory::Default(void)
{
	if ( lUsine == NULL ) lUsine=new font_factory;
	return lUsine;
}

font_factory::font_factory(void)
{
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

font_instance*        font_factory::FaceFromDescr(const char* descr)
{
	PangoFontDescription* temp_descr=pango_font_description_from_string(descr);
	font_instance *res=Face(temp_descr);
//	pango_font_description_free(temp_descr);
	return res;
}

font_instance*        font_factory::Face(PangoFontDescription* descr, bool canFail)
{
#ifdef WITH_XFT
	pango_font_description_set_size (descr, 512*PANGO_SCALE); // mandatory huge size (hinting workaround)
#endif
#ifdef WIN32
	// Pango on Windows used to give some weird font sizes, bigger than necessary and depending on resolution.
	// Here we compensate for that so that Linux and Windows display the same font sizes.
	// The nature of the magic number 67.55223 (obtained experimentally) is a subject of debate; 
	// if you can offer any insight, it will be much appreciated.
	HDC screen = GetDC(0);
	double resolution = GetDeviceCaps (screen, LOGPIXELSY);
	pango_font_description_set_size (descr, (int) (512 * PANGO_SCALE * 67.55223 / resolution));
#endif

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

font_instance*        font_factory::Face(const char* family, int variant, int style, int weight, int stretch, int /*size*/, int /*spacing*/)
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

font_instance*        font_factory::Face(const char* family, NRTypePosDef apos)
{
	PangoFontDescription*  temp_descr=pango_font_description_new();
	pango_font_description_set_family(temp_descr,family);
	if ( apos.variant == NR_POS_VARIANT_SMALLCAPS ) {
		pango_font_description_set_variant(temp_descr,PANGO_VARIANT_SMALL_CAPS);
	} else {
		pango_font_description_set_variant(temp_descr,PANGO_VARIANT_NORMAL);
	}
	if ( apos.italic ) {
		pango_font_description_set_style(temp_descr,PANGO_STYLE_ITALIC);
	} else if ( apos.oblique ) {
		pango_font_description_set_style(temp_descr,PANGO_STYLE_OBLIQUE);
	} else {
		pango_font_description_set_style(temp_descr,PANGO_STYLE_NORMAL);
	}
	if ( apos.weight <= NR_POS_WEIGHT_ULTRA_LIGHT ) {
		pango_font_description_set_weight(temp_descr,PANGO_WEIGHT_ULTRALIGHT);
	} else if ( apos.weight <= NR_POS_WEIGHT_LIGHT ) {
		pango_font_description_set_weight(temp_descr,PANGO_WEIGHT_LIGHT);
	} else if ( apos.weight <= NR_POS_WEIGHT_NORMAL ) {
		pango_font_description_set_weight(temp_descr,PANGO_WEIGHT_NORMAL);
	} else if ( apos.weight <= NR_POS_WEIGHT_SEMIBOLD ) {
		pango_font_description_set_weight(temp_descr,PANGO_WEIGHT_BOLD);
	} else if ( apos.weight <= NR_POS_WEIGHT_BOLD ) {
		pango_font_description_set_weight(temp_descr,PANGO_WEIGHT_ULTRABOLD);
	} else {
		pango_font_description_set_weight(temp_descr,PANGO_WEIGHT_HEAVY);
	}
	if ( apos.stretch <= NR_POS_STRETCH_ULTRA_CONDENSED ) {
		pango_font_description_set_stretch(temp_descr,PANGO_STRETCH_EXTRA_CONDENSED);
	} else if ( apos.stretch <= NR_POS_STRETCH_CONDENSED ) {
		pango_font_description_set_stretch(temp_descr,PANGO_STRETCH_CONDENSED);
	} else if ( apos.stretch <= NR_POS_STRETCH_SEMI_CONDENSED ) {
		pango_font_description_set_stretch(temp_descr,PANGO_STRETCH_SEMI_CONDENSED);
	} else if ( apos.stretch <= NR_POS_WEIGHT_NORMAL ) {
		pango_font_description_set_stretch(temp_descr,PANGO_STRETCH_NORMAL);
	} else if ( apos.stretch <= NR_POS_STRETCH_SEMI_EXPANDED ) {
		pango_font_description_set_stretch(temp_descr,PANGO_STRETCH_SEMI_EXPANDED);
	} else if ( apos.stretch <= NR_POS_STRETCH_EXPANDED ) {
		pango_font_description_set_stretch(temp_descr,PANGO_STRETCH_EXPANDED);
	} else {
		pango_font_description_set_stretch(temp_descr,PANGO_STRETCH_EXTRA_EXPANDED);
	}
	font_instance *res=Face(temp_descr);
	pango_font_description_free(temp_descr);
	return res;
}

void                  font_factory::UnrefFace(font_instance* who)
{
  if ( who == NULL ) return;
  if ( loadedFaces.find(who->descr) == loadedFaces.end() ) {
    // not found
  } else {
    loadedFaces.erase(loadedFaces.find(who->descr));
  }
}

static void font_factory_name_list_destructor (NRNameList *list) {
       for (unsigned int i=0;i<list->length;i++) free(list->names[i]);
       if ( list->names ) nr_free(list->names);
}

static void font_factory_style_list_destructor (NRStyleList *list) {
       for (unsigned int i=0;i<list->length;i++) free(list->names[i]);
       if ( list->names ) nr_free(list->names);
       if ( list->pango_descrs ) g_free(list->pango_descrs);
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

NRNameList*           font_factory::Families(NRNameList *flist)
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
NRStyleList*           font_factory::Styles(const gchar *family, NRStyleList *slist)
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
				fams[i]=NULL;
				break;
			}
		}
		
		g_free(fams);
	}

	// nothing found
	if ( theFam == NULL ) {
		slist->length = 0;
		slist->names = NULL;
		slist->pango_descrs = NULL;
		slist->destructor = NULL;
		return slist;
	}

	// search faces in the found family
	PangoFontFace**  facs=NULL;
	int							 nbFac=0;
	pango_font_family_list_faces(theFam, &facs, &nbFac);

	slist->length = nbFac;
	slist->names = (guchar **)malloc(nbFac*sizeof(guchar*));
	slist->pango_descrs = (guchar **)malloc(nbFac*sizeof(guchar*));
	slist->destructor = font_factory_style_list_destructor;

	for (int i=0;i<nbFac;i++) {

               // no unnamed faces
               if (pango_font_face_get_face_name (facs[i]) == NULL)
                       continue;
               if (pango_font_description_to_string (pango_font_face_describe(facs[i])) == NULL)
                       continue;

               guchar *name = (guchar *) g_strdup (pango_font_face_get_face_name (facs[i]));
               guchar *descr = (guchar *) g_strdup (pango_font_description_to_string (pango_font_face_describe(facs[i])));

               // no duplicates
               for (int j = 0; j < i; j ++) {
                       if (!strcmp ((const char *) slist->names[j], (const char *) name))
                               continue;
               }

               slist->names[i] = name;
               slist->pango_descrs[i] = descr;
	}
	
	g_free(facs);
	
	return slist;
}

