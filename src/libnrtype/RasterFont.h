/*
 *  RasterFont.h
 *  testICU
 *
 */

#ifndef my_raster_font
#define my_raster_font

#include <hash_map.h>

#include <livarot/Ligne.h>
#include <libnr/nr-pixblock.h>
#include <libnr/nr-point.h>

#include "FontInstance.h"

class Path;
class Shape;
class font_instance;
class raster_glyph;

class raster_font {
public:
  font_instance*                daddy;
	int                           refCount;
	
	font_style                    style;  
	
  hash_map<int,int>             glyph_id_to_raster_glyph_no;
  int                           nbBase,maxBase;
  raster_glyph**                bases;
	
  raster_font(void);
  ~raster_font(void);
   
	void					 Unref(void);
	void					 Ref(void);
	
	NR::Point      Advance(int glyph_id);
	void           BBox(int glyph_id,NRRect *area);         

  raster_glyph*  GetGlyph(int glyph_id);
  
  void           LoadRasterGlyph(int glyph_id); // refreshes outline/polygon if needed
  void           RemoveRasterGlyph(raster_glyph* who);
  
};

class raster_position {
public:
  int               top,bottom; // baseline is y=0
  int*              run_on_line; // array of size (bottom-top+1): run_on_line[i] gives the number of runs on line top+i
  int               nbRun;
  float_ligne_run*  runs;
  
  raster_position(void);
  ~raster_position(void);
  
  void      AppendRuns(int add,float_ligne_run* src,int y);
  
  void      Blit(float ph,int pv,NRPixBlock &over);
};

class raster_glyph {
public:
  raster_font*      daddy;
  int               glyph_id;
  
  Path*             outline;  // transformed by the matrix in style (may be factorized, but is small)
  Shape*            polygon;
  
  int               nb_sub_pixel;
  raster_position*  sub_pixel;
  
  raster_glyph(void);
  ~raster_glyph(void);
  
  void      SetSubPixelPositionning(int nb_pos);
  void      LoadSubPixelPosition(int no);
	
  void      Blit(const NR::Point &at,NRPixBlock &over); // alpha only
};

#endif


