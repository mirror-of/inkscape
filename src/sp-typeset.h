#ifndef __SP_TYPESET_H__
#define __SP_TYPESET_H__

/*
 * typeset object
 * it's a subclass of SPGroup
 */

/*
example:
<g sodipodi:type="typeset" srcNoMarkup="whatever text" dstShape="[path524]"/> </>
creates a child text object with the text properly 'typeset'
 */


#include "sp-item-group.h"

#define SP_TYPE_TYPESET          (sp_typeset_get_type ())
#define SP_TYPESET(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), SP_TYPE_TYPESET, SPTypeset))
#define SP_TYPESET_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), SP_TYPE_TYPESET, SPTypesetClass))
#define SP_IS_TYPESET(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), SP_TYPE_TYPESET))
#define SP_IS_TYPESET_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), SP_TYPE_TYPESET))


enum {
  has_no_src    = 0,
  has_std_txt   = 1,
  has_pango_txt = 2
};

enum {
  has_no_dest   = 0,
  has_col_dest  = 1,
  has_box_dest  = 2,
  has_path_dest = 4,
  has_shape_dest= 8
};

class Path;
class Shape;

typedef struct column_dest {
  double        width;
} column_dest;
typedef struct box_dest {
  NR::Rect      box;
} box_dest;
typedef struct path_dest {
  char*         originalID;
  SPRepr*       originalObj;
  Path*         thePath;
  double        length;
} path_dest;
typedef struct shape_dest {
  char*         originalID;
  SPRepr*       originalObj;
  int           windingRule;
  Shape*        theShape;
  NR::Rect      bbox;
} shape_dest;

typedef struct text_chunk_solution {
  bool          end_of_array;
  int           start_ind,end_ind;
  double        length;
  double        ascent,descent; // may differ from the initial one
} text_chunk_solution;

typedef struct box_solution {
  int          frame_no;
  double       y,ascent,descent;
  double       x_start,x_end;
  bool         finished;
} box_solution;

typedef struct glyphs_for_span {
  char            style[256];
  int             nbG; // arrays are nbG+1 to hold the 'terminating' glyph
  int             *g_start;
  int             *g_end;
  NR::Point       *g_pos; // wrp the start of the span
  char            *g_text;
} glyphs_for_span;

class text_chunker {
public:
  
  text_chunker(char* /*inText*/) {};
  virtual ~text_chunker(void) {};

  virtual void                 SetText(char* /*inText*/,int /*flags*/) {};
  virtual void                 ChangeText(int /*startPos*/,int /*endPos*/,char* /*inText*/,int /*flags*/) {};
  virtual int                  MaxIndex(void) {return 0;}; // index in visual text != char index in source text
  
  virtual void                 InitialMetricsAt(int /*startPos*/,double &/*ascent*/,double &/*descent*/) {};
  virtual text_chunk_solution* StuffThatBox(int /*start_ind*/,double /*minLength*/,double /*nominalLength*/,double /*maxLength*/,bool /*strict*/) {return NULL;};
  virtual void                 GlyphsAndPositions(int /*start_ind*/,int /*end_ind*/,int &nbS,glyphs_for_span* &spans) {nbS=0;spans=NULL;};
};

class dest_chunker {
public:
  
  dest_chunker(void) {};
  virtual ~dest_chunker(void) {};
  
  virtual box_solution   VeryFirst(void) {box_solution res;res.finished=true;res.y=res.ascent=res.descent=res.x_start=res.x_end=0;res.frame_no=0;return res;};
  virtual box_solution   NextLine(box_solution& /*after*/,double /*asc*/,double /*desc*/,double /*lead*/) {box_solution res;
    res.finished=true;res.y=res.ascent=res.descent=res.x_start=res.x_end=0;res.frame_no=0;return res;};
  virtual box_solution   NextBox(box_solution& after,double asc,double desc,double lead,bool &stillSameLine) {stillSameLine=false;return NextLine(after,asc,desc,lead);};
  virtual double         RemainingOnLine(box_solution& /*after*/) {return 0;};
};


struct SPTypeset {
	SPGroup     group;
  
  bool        layoutDirty;
  bool        destDirty;
  
  int         srcType;
  char*       srcText;
  text_chunker* theSrc;
  
  int         dstType;
  GSList*     dstElems;
  dest_chunker* theDst;
};

struct SPTypesetClass {
	SPGroupClass group_class;
};

GType sp_typeset_get_type (void);


#endif
