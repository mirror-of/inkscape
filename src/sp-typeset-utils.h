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

#include "libnrtype/nr-font.h"

#include <pango/pango.h>
#include <glib.h>

class text_with_info {
public:
  char*        utf8_text;
  int          last_st,last_len;
  
  int          utf8_length;
  int          unicode_length;
  
  gunichar*    uni_text;
  int*         char_offset; // size unicode_length+1 (for safety)
  
  double*      kern_x;
  double*      kern_y;
  
  char*        stripped; // in case there is a markup
  int          markupType;
  void*        markup;
  
  text_with_info(char* inText);
  ~text_with_info(void);

  char*        UTF8Text(void);
  gunichar*    UnicodeText(void);
  int          UTF8Length(void);
  int          UnicodeLength(void);
  
  void         Kill(void);
  
  void         SetStdText(char* inText);
  void         AppendStdText(char* inText);
  
  void         SetPangoText(char* inText);
  void         AppendPangoText(char* inText);
  
  void         KernXForLastAddition(double *i_kern_x,int i_len);
  void         KernYForLastAddition(double *i_kern_y,int i_len);
};



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
  
	void                  AppendShape(Shape* iShape,Shape* iExcl=NULL);
	void                  ComputeLine(float y,float a,float d,int shNo);
  
  virtual box_solution   VeryFirst(void);
  virtual box_solution   NextLine(box_solution& after,double asc,double desc,double lead);
  virtual box_solution   NextBox(box_solution& after,double asc,double desc,double lead,bool &stillSameLine);
  virtual double         RemainingOnLine(box_solution& after);
};

class dest_path_chunker: public dest_chunker {
public:
  typedef struct one_path_elem {
    Path*    theP;
    double   length;
  } one_path_elem;
  
	int                   nbPath,maxPath;
	one_path_elem         *paths;
  
  
  dest_path_chunker(void);
  virtual ~dest_path_chunker(void);
  
	void                  AppendPath(Path* iPath);
  
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
    double       x_adv;
    int          code_point;
  } glyph_box;

  // source data
  text_with_info* theText;
  bool            own_text;
  char*           theFace;
  double          theSize;

  // tempStuff
  int             textLength;
  double          baselineY;
  PangoAttrList*  theAttrs;
    
  
  elem_box*       words;
  int             nbWord,maxWord;
  
  glyph_box*      charas;
  
  pango_text_chunker(text_with_info* inText,char* font_family,double font_size,int flags);
  virtual ~pango_text_chunker(void);
  
  virtual void                 SetText(char* inText,int flags);
  virtual int                  MaxIndex(void);
  
  virtual void                 InitialMetricsAt(int startPos,double &ascent,double &descent);
  virtual text_chunk_solution* StuffThatBox(int start_ind,double minLength,double nominalLength,double maxLength,bool strict);
  
  virtual void                 GlyphsAndPositions(int start_ind,int end_ind,to_SVG_context *hungry);
  virtual void                 GlyphsInfo(int start_ind,int end_ind,int &nbG,double &totLength);

  void                         AddBox(int st,int en,bool whit,bool retu,PangoGlyphString* from,int offset,PangoFont* theFont,NRFont* inkFont,NR::Point &cumul);
  void                         SetTextWithAttrs(text_with_info* inText,int flags);
  void                         AddDullGlyphs(to_SVG_context *hungry,double &cumul,int c_st,int c_en);
  void                         AddAttributedGlyphs(to_SVG_context *hungry,double &cumul,int c_st,int c_en,PangoAttrIterator *theIt);
};


class box_to_SVG_context : public to_SVG_context{
public:
  double           box_y,box_start,box_end;
  
  double           cur_x,cur_y,cur_next,cur_spacing;
  double           cur_start,cur_by;
  
  double           *dxs;
  double           *dys;
  double           *xs;
  double           *ys;
  double           letter_spacing;
  char             *text;
  int              st,en,orig_st;
  int              st_g,en_g,orig_st_g,n_g;
  
  SPRepr           *text_repr;
  
  SPCSSAttr        *style_repr;
  SPRepr           *span_repr;
  
  box_to_SVG_context(SPRepr* in_repr,double y,double x_start,double x_end);
  virtual ~box_to_SVG_context(void);
  
  virtual void            Finalize(void);
  
  virtual void            ResetStyle(void);
  virtual void            AddFontFamily(char* n_fam);
  virtual void            AddFontSize(double n_siz);
  virtual void            AddFontVariant(int n_var);
  virtual void            AddFontStretch(int n_str);
  virtual void            AddFontWeight(int n_wei);
  virtual void            AddLetterSpacing(double n_spc);
  
  virtual void            SetText(char*  n_txt,int n_len);
  virtual void            SetLetterSpacing(double n_spc);
  
  void            Flush(void);
  virtual void            AddGlyph(int a_g,int f_c,int l_c,const NR::Point &at,double advance);
  void            SetY(int a_g,int f_c,int l_c,double to);
};

class path_to_SVG_context : public to_SVG_context{
public:
  Path*            thePath;
  double           path_length;
  double           path_delta;
  
  double           cur_x;
  
  double           letter_spacing;
  char             *text;
  int              st,en,orig_st;
  int              st_g,en_g,orig_st_g,n_g;
  
  SPRepr           *text_repr;
  
  SPCSSAttr        *style_repr;
  SPRepr           *span_repr;
  
  path_to_SVG_context(SPRepr* in_repr,Path *iPath,double iLength,double iDelta);
  virtual ~path_to_SVG_context(void);
  
  virtual void            Finalize(void);
  
  virtual void            ResetStyle(void);
  virtual void            AddFontFamily(char* n_fam);
  virtual void            AddFontSize(double n_siz);
  virtual void            AddFontVariant(int n_var);
  virtual void            AddFontStretch(int n_str);
  virtual void            AddFontWeight(int n_wei);
  virtual void            AddLetterSpacing(double n_spc);
  
  virtual void            SetText(char*  n_txt,int n_len);
  virtual void            SetLetterSpacing(double n_spc);
  
  virtual void            AddGlyph(int a_g,int f_c,int l_c,const NR::Point &at,double advance);
};

#endif

