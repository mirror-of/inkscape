/*
 *  FontFactory.h
 *  testICU
 *
 */

#ifndef my_font_factory
#define my_font_factory

#include <functional>
#include <hash_map.h>

#include <pango/pango.h>
#include "nr-type-primitives.h"
#include "nr-type-pos-def.h"

#ifdef WITH_XFT
#include <pango/pangoft2.h>
#include <freetype/freetype.h>
#endif
#ifdef WIN32
#include <windows.h>
#include <windowsx.h>
#include <pango/pangowin32.h>
#endif

class font_instance;

// for the hashmap
struct font_descr_hash : public unary_function<PangoFontDescription*,size_t> {
	size_t  operator()( PangoFontDescription* const&x) const {
		return pango_font_description_hash (x);
	}
};
struct font_descr_equal : public binary_function<PangoFontDescription*,PangoFontDescription*,bool> {
  bool  operator()( PangoFontDescription* const&a, PangoFontDescription* const&b) {
		if ( pango_font_description_equal(a,b) ) return true;
		return false;
	}
};

class font_factory {
public:
	static font_factory*  lUsine;
  
	PangoFontMap*					fontServer;
	PangoContext*         fontContext;
	
#ifdef WIN32
	HDC                   wDevice;
	PangoWin32FontCache*  wCache;
#endif
	
  hash_map<PangoFontDescription*,font_instance*,font_descr_hash,font_descr_equal>     loadedFaces;
	
	font_factory(void);
	~font_factory(void);
	
	static font_factory*  Default(void);
	
	font_instance*        FaceFromDescr(const char* descr);
	font_instance*        Face(PangoFontDescription* descr,bool canFail=true);
	font_instance*        Face(const char* family,int variant=PANGO_VARIANT_NORMAL,int style=PANGO_STYLE_NORMAL,int weight=PANGO_WEIGHT_NORMAL,int stretch=PANGO_STRETCH_NORMAL, int size=10, int spacing=0);
	font_instance*        Face(const char* family, NRTypePosDef apos);

	void                  UnrefFace(font_instance* who);
	
	NRNameList*           Families(NRNameList *flist);
	NRStyleList*           Styles(const gchar *family, NRStyleList *slist);
};


#endif



