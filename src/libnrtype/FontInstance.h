/*
 *  FontInstance.h
 *  testICU
 *
 */

#ifndef my_font_instance
#define my_font_instance

#include <functional>
#include <hash_map.h>

#include <livarot/LivarotDefs.h>

#include <pango/pango.h>

#include <libnr/nr-point.h>
#include <libnr/nr-matrix.h>
#include <libnr/nr-rect.h>

#ifdef WITH_XFT
//#include <freetype/freetype.h>
#include <ft2build.h>
#include FT_FREETYPE_H
#endif
#ifdef WIN32
#include <windows.h>
#include <windowsx.h>
#endif

class Path;
class Shape;
class font_factory;
class raster_font;
class font_instance;

// different raster styles
typedef struct font_style {
  NR::Matrix		transform;
	bool          vertical;
  double				stroke_width;
  JoinType			stroke_join;
  ButtType			stroke_cap;
  int						nbDash;
  double				dash_offset;
  double*				dashes;
	
	void          Apply(Path* src,Shape* dst);
} font_style;

struct font_style_hash : public unary_function<font_style,size_t> {
  size_t  operator()(const font_style &x) const;
};
struct font_style_equal : public binary_function<font_style,font_style,bool> {
  bool  operator()(const font_style &a,const font_style &b);
};

typedef struct font_glyph {
	double         h_advance,h_width;
	double         v_advance,v_width;
	double         bbox[4];
  Path*          outline;
	void*          artbpath;
} font_glyph;

class font_instance {
public:
	hash_map<font_style,raster_font*,font_style_hash,font_style_equal>     loadedStyles;

	PangoFont*            pFont;
#ifdef WITH_XFT
	FT_Face 							theFace;
#endif
#ifdef WIN32
	LOGFONT*              theLogFont;
	HFONT                 wFont;
	OUTLINETEXTMETRIC     otm;
#endif
	PangoFontDescription* descr;
	int                   refCount;
	font_factory*         daddy;
	
	// common glyph definitions for all the rasterfonts
  hash_map<int,int>     id_to_no;
  int                   nbGlyph,maxGlyph;
  font_glyph*           glyphs;

	font_instance(void);
	~font_instance(void);

  void                 Ref(void);
	void								 Unref(void);
	
	bool                 IsOutlineFont(void);
	void                 InstallFace(PangoFont* iFace);
	
	int                  MapUnicodeChar(gunichar c);
  void                 LoadGlyph(int glyph_id);
	Path*                Outline(int glyph_id,Path* copyInto=NULL);
	void*                ArtBPath(int glyph_id);
	double               Advance(int glyph_id,bool vertical);
	NR::Rect             BBox(int glyph_id);
	
	raster_font*         RasterFont(const NR::Matrix &trs,double stroke_width,bool vertical=false,JoinType stroke_join=join_straight,ButtType stroke_cap=butt_straight);
	raster_font*         RasterFont(const font_style &iStyle);
	void                 RemoveRasterFont(raster_font* who);
	
	unsigned int         Name(gchar *str, unsigned int size);
	unsigned int         Family(gchar *str, unsigned int size);
	unsigned int         Attribute(const gchar *key, gchar *str, unsigned int size);
};


#endif


