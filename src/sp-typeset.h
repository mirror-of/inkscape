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
 
 modalities:
 one 'root' typeset element, with clidren typeset elements containing a paragraph of the text each.
 the place where the text is flowed is defined by the root typeset. the root typeset can also contain 
 one paragraph
 
 attributes of a typeset:
 source text (only one kind of text at a time)=
   inkscape:srcNoMarkup   = straight text
   inkscape:srcPango      = text with pango markup (see pango's docs)
 
 destinations (only one kind of destinaition)=
   inkscape:dstColumn     = one column with a given width 
   inkscape:dstBox        = a set of rects given as a succession of "[left top right bottom]"
   inkscape:dstPath       = a set of path given as uris; text is put on path successively
   inkscape:dstShape      = a set of shapes given as uris
 
 modifiers:
   inkscape:excludeShape  = a set of shapes given as uris that are to be substracted from the destination
           only works with dstShape
   inkscape:style         = style transfered to the text element created by the typeset; use it to define
           the default font
   inkscape:layoutOptions = a css attributes with the following fields:
        alignment   = left / center / right
        layoutAlgo  = simple / better : toggles between dull algo and knuth-plass
        justify     = true / false
 */


#include "sp-item-group.h"
#include "livarot/livarot-forward.h"
#include "xml/repr.h"

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

class text_wrapper;

// structs used by the typeset element to hold the info about destination
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

// structs used by the layout algos to communicate with source and destination
typedef struct text_chunk_solution {
  bool          end_of_array;
  int           start_ind,end_ind;
  double        length;
  double        ascent,descent; // may differ from the initial one
  bool          endOfParagraph;
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


// abstract class that generates SVG from glyphs
// the text_chunker must use the AddFont*() functions to set the style, then call AddGlyph() for
// each glyph
// a to_SVG_context is created by the layout algo for each line of the layout
class to_SVG_context {
public:  
  to_SVG_context(SPRepr* /*in_repr*/) {};
  virtual ~to_SVG_context(void) {};
  
  virtual void            Finalize(void) {};
  
  virtual void            ResetStyle(void) {};
  virtual void            AddFontFamily(char* /*n_fam*/) {};
  virtual void            AddFontSize(double /*n_siz*/) {};
  virtual void            AddFontVariant(int /*n_var*/) {};
  virtual void            AddFontStretch(int /*n_str*/) {};
  virtual void            AddFontWeight(int /*n_wei*/) {};
  virtual void            AddLetterSpacing(double /*n_spc*/) {};
  
  virtual void            SetText(text_wrapper*  /*n_txt*/,int /*n_len*/) {};
  virtual void            SetLetterSpacing(double /*n_spc*/) {};
  
  virtual void            AddGlyph(int /*a_g*/,int /*f_c*/,int /*l_c*/,const NR::Point &/*at*/,double /*advance*/) {};
};

// abstract class that handles the source text
class text_chunker {
public:
  
  text_chunker(char* /*inText*/) {};
  virtual ~text_chunker(void) {};

  virtual void                 SetText(char* /*inText*/,int /*flags*/) {};
  virtual void                 ChangeText(int /*startPos*/,int /*endPos*/,char* /*inText*/,int /*flags*/) {};
  // return the number of boxes(=words if no hyphenation) in the text 
  virtual int                  MaxIndex(void) {return 0;}; // index in visual text != char index in source text
  
  // ascent and descent for a given position
  // consider the line starts at index(=word) startPos
  virtual void                 InitialMetricsAt(int /*startPos*/,double &/*ascent*/,double &/*descent*/) {};
  // tries to stuff text in a box of length nominalLength
  // the text is allowed to stretch from minLength to maxLength, and if strict == false, the solution just before minLength
  // and the one just after maxLength are also accepted
  // text is must start at index start_ind
  // the result is an array of text_chunk_solution, with the last element of the array having field end_of_array=true
  virtual text_chunk_solution* StuffThatBox(int /*start_ind*/,double /*minLength*/,double /*nominalLength*/,double /*maxLength*/,bool /*strict*/) {return NULL;};
  // feeds the SVG constructor with glyphs for the range [start_ind .. end_ind]
  // first glyph is supposed to have x=0
  virtual void                 GlyphsAndPositions(int /*start_ind*/,int /*end_ind*/,to_SVG_context */*hungry*/) {};
  // returns info on the space needed to put the portion [start_ind .. end_ind] of the text on a line
  // nbG is the number of glyphs needed
  virtual void                 GlyphsInfo(int /*start_ind*/,int /*end_ind*/,int &nbG,double &totLength) {nbG=0;totLength=0;};
};

// abstract class that handles a destinations
// return boxes to fill with text
class dest_chunker {
public:
  
  dest_chunker(void) {};
  virtual ~dest_chunker(void) {};
  
  // returns the very first box, before any part of the region to fill with text
  virtual box_solution   VeryFirst(void) {box_solution res;res.finished=true;res.y=res.ascent=res.descent=res.x_start=res.x_end=0;res.frame_no=0;return res;};
  // return the line after the box 'after'; the new line should have ascent 'asc' and descnet 'desc'. 'lead' is the leading between lines
  virtual box_solution   NextLine(box_solution& /*after*/,double /*asc*/,double /*desc*/,double /*lead*/) {box_solution res;
    res.finished=true;res.y=res.ascent=res.descent=res.x_start=res.x_end=0;res.frame_no=0;return res;};
  // return the box after 'after', possibly on the same line. if the returned box in on another line, stillSameLine must be set to true
  virtual box_solution   NextBox(box_solution& after,double asc,double desc,double lead,bool &stillSameLine) {stillSameLine=false;return NextLine(after,asc,desc,lead);};
  // return the length remaining on a line after the x_end of 'after'
  virtual double         RemainingOnLine(box_solution& /*after*/) {return 0;};
};


// the typeset element
struct SPTypeset {
	SPGroup     group;
  
  bool        layoutDirty;
  bool        destDirty;
  bool        exclDirty;
  bool        stdLayoutAlgo;
  
  bool        justify;
  int         centering; // 0=left 1=center 2=right
  
  int         srcType;
  char*       srcText;
  text_chunker* theSrc;
  
  int         dstType;
  GSList*     dstElems;
  dest_chunker* theDst;

  Shape*      excluded;
  GSList*     exclElems;
};

struct SPTypesetClass {
	SPGroupClass group_class;
};

GType sp_typeset_get_type (void);

void        sp_typeset_set_text(SPObject* object,char* in_text,int text_type);
void        sp_typeset_chain_shape(SPObject* object,char* shapeID);

void        sp_typeset_build_one(void);

#endif
