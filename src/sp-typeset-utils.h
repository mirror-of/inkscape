#ifndef __sp_typeset_layout_utils_H__
#define __sp_typeset_layout_utils_H__

/*
 * layout utilities for the typeset element
 */

#include <config.h>

#include "style.h"
#include "attributes.h"

#include "sp-typeset.h"

#include "display/curve.h"
#include "livarot/Path.h"
#include "livarot/Ligne.h"
#include "livarot/Shape.h"
#include "livarot/LivarotDefs.h"

#include <pango/pango.h>
#include <gdk/gdk.h>

class dest_col_chunker : public dest_chunker  {
public:
  double     colWidth;
  
  dest_col_chunker(double iWidth);
  virtual ~dest_col_chunker(void);
  
  virtual box_solution   VeryFirst(void);
  virtual box_solution   NextLine(box_solution& after,double asc,double desc,double lead);
  virtual box_solution   NextBox(box_solution& after,double asc,double desc,double lead,bool &stillSameLine);
  virtual double         RemainingOnLine(box_solution& after);
};

class dest_box_chunker : public dest_chunker  {
public:
  int          nbBoxes;
  NR::Rect     *boxes;
  
  dest_box_chunker(void);
  virtual ~dest_box_chunker(void);
  
  void           AppendBox(const NR::Rect &iBox);
  
  virtual box_solution   VeryFirst(void);
  virtual box_solution   NextLine(box_solution& after,double asc,double desc,double lead);
  virtual box_solution   NextBox(box_solution& after,double asc,double desc,double lead,bool &stillSameLine);
  virtual double         RemainingOnLine(box_solution& after);
};


class dest_shape_chunker: public dest_chunker {
public:
	typedef struct cached_line {
		int          date;
		FloatLigne*  theLine;
		int          shNo;
		double       y,a,d;
	} cached_line;
  // data stored for each shape:
  typedef struct one_shape_elem {
    Shape*   theS;     // the shape itself
    double   l,t,r,b;  // the bounding box
    int      curPt;
    float    curY;
  } one_shape_elem;
  
	int                   nbShape,maxShape;
	one_shape_elem        *shapes;
    
	int                   lastDate; // date for the cache
	int                   maxCache,nbCache;
	cached_line*          caches;  
	FloatLigne*           tempLine;
	FloatLigne*           tempLine2;
  
  dest_shape_chunker(void);
  virtual ~dest_shape_chunker(void);
  
	void                  AppendShape(Shape* iShape);
	void                  ComputeLine(float y,float a,float d,int shNo);
  
  virtual box_solution   VeryFirst(void);
  virtual box_solution   NextLine(box_solution& after,double asc,double desc,double lead);
  virtual box_solution   NextBox(box_solution& after,double asc,double desc,double lead,bool &stillSameLine);
  virtual double         RemainingOnLine(box_solution& after);
};


/*
 *
 */
enum {
  p_t_c_none        = 0,
  p_t_c_mergeWhite  = 1
};

class pango_text_chunker : public text_chunker {
public:
  typedef struct elem_box {
    double       x_pos,y_ascent,y_descent,x_length;
    int          t_first,t_last;
    bool         is_white,is_return,end_of_word;
  } elem_box;
  typedef struct glyph_box {
    double       x_pos,y_pos;
    double       x_dpos,y_dpos;
  } glyph_box;

  // source data
  char*          theText; // not owned by the class
  bool           own_text;
  char*          theFace;
  double         theSize;

  // tempStuff
  int            textLength;
  double         baselineY;
  PangoAttrList* theAttrs;
    
  
  elem_box*      words;
  int            nbWord,maxWord;
  
  glyph_box*     charas;
  
  pango_text_chunker(char* inText,char* font_family,double font_size,int flags,bool isMarked);
  virtual ~pango_text_chunker(void);
  
  virtual void                 SetText(char* inText,int flags);
  virtual void                 ChangeText(int startPos,int endPos,char* inText,int flags);
  virtual int                  MaxIndex(void);
  
  virtual void                 InitialMetricsAt(int startPos,double &ascent,double &descent);
  virtual text_chunk_solution* StuffThatBox(int start_ind,double minLength,double nominalLength,double maxLength,bool strict);
  
  virtual void                 GlyphsAndPositions(int start_ind,int end_ind,int &nbS,glyphs_for_span* &spans);
  void                         AddBox(int st,int en,bool whit,bool retu,PangoGlyphString* from,int offset,PangoFont* theFont,NR::Point &cumul);
  void                         SetTextWithMarkup(char* inText,int flags);
  void                         SetTextWithAttrs(char* inText,PangoAttrList* resAttr,int flags);
  void                         AddDullGlyphs(glyphs_for_span &dest,double &cumul,int c_st,int c_en);
  void                         AddAttributedGlyphs(glyphs_for_span &dest,double &cumul,int c_st,int c_en,PangoAttrIterator *theIt);
};

#endif

