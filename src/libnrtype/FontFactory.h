/*
 *  FontFactory.h
 *  testICU
 *
 */

#ifndef my_font_factory
#define my_font_factory

#include <functional>
#include <algorithm>
#include <ext/hash_map>

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <pango/pango.h>
#include "nr-type-primitives.h"
#include "nr-type-pos-def.h"
#include <libnrtype/nrtype-forward.h>

#ifdef WITH_XFT
#include <pango/pangoft2.h>
#include <freetype/freetype.h>
#endif
#ifdef WIN32
#include <windows.h>
#include <windowsx.h>
#include <pango/pangowin32.h>
#endif

// for the hashmap
struct font_descr_hash : public std::unary_function<PangoFontDescription*,size_t> {
	size_t  operator()( PangoFontDescription* const&x) const;
};
struct font_descr_equal : public std::binary_function<PangoFontDescription*,PangoFontDescription*,bool> {
  bool  operator()( PangoFontDescription* const&a, PangoFontDescription* const&b);
};

class font_factory {
public:
	static font_factory*  lUsine;
	
	typedef struct font_entry {
		font_instance*    f;
		double            age;
	} font_entry;
	int                   nbEnt,maxEnt;
  font_entry*           ents;
	
	PangoFontMap*					fontServer;
	PangoContext*         fontContext;
	double                fontSize;
#ifdef WIN32
	HDC                   wDevice;
	PangoWin32FontCache*  wCache;
#endif
	
	__gnu_cxx::hash_map<PangoFontDescription*,font_instance*,font_descr_hash,font_descr_equal>     loadedFaces;
	
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
	
	void                  AddInCache(font_instance* who);
};


#endif



