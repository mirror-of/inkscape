#define __sp_typeset_layout_C__

/*
 * layout routines for the typeset element
 */

#include <config.h>
#include <string.h>

#include "display/nr-arena-group.h"
#include "xml/repr-private.h"
#include "xml/repr.h"
#include <libnr/nr-matrix.h>
#include <libnr/nr-matrix-ops.h>
#include "sp-object-repr.h"
#include "svg/svg.h"
#include "inkscape.h"
#include "desktop-handles.h"
#include "document.h"
#include "style.h"
#include "attributes.h"

#include "sp-root.h"
#include "sp-use.h"
#include "sp-typeset.h"
#include "helper/sp-intl.h"

#include "sp-text.h"
#include "sp-shape.h"

#include "display/curve.h"
#include "livarot/Path.h"
#include "livarot/Ligne.h"
#include "livarot/Shape.h"
#include "livarot/LivarotDefs.h"

#include <pango/pango.h>
//#include <pango/pangoxft.h>
#include <gdk/gdk.h>

void   sp_typeset_relayout(SPTypeset *typeset);

class dest_col_chunker : public dest_chunker  {
public:
  double     colWidth;
  
  dest_col_chunker(double iWidth) {colWidth=(iWidth>0)?iWidth:0;};
  virtual ~dest_col_chunker(void) {};
  
  virtual box_solution   VeryFirst(void) {box_solution res;res.finished=false;res.y=res.ascent=res.descent=res.x_start=res.x_end=0;res.frame_no=-1;return res;};
  virtual box_solution   NextLine(box_solution& after,double asc,double desc,double lead) {
    box_solution res;
    if ( after.frame_no < 0 ) {
      res.y=asc;
      res.ascent=asc;
      res.descent=desc;
      res.x_start=0;
      res.x_end=colWidth;
      res.frame_no=0;
    } else {
      res.y=after.y+after.descent+lead+asc;
      res.ascent=asc;
      res.descent=desc;
      res.x_start=0;
      res.x_end=colWidth;
      res.frame_no=0;
    }
    res.finished=false;
    return res;
  };
  virtual box_solution   NextBox(box_solution& after,double asc,double desc,double lead,bool &stillSameLine) {
    stillSameLine=false;
    return NextLine(after,asc,desc,lead);
  };
  virtual double         RemainingOnLine(box_solution& /*after*/) {return 0;};
};

class dest_box_chunker : public dest_chunker  {
public:
  int          nbBoxes;
  NR::Rect     *boxes;
  
  dest_box_chunker(void) {boxes=NULL;nbBoxes=0;};
  virtual ~dest_box_chunker(void) {if ( boxes ) free(boxes);};
  
  void           AppendBox(const NR::Rect &iBox) {
    if ( iBox.isEmpty() ) {
    } else {
      boxes=(NR::Rect*)realloc(boxes,(nbBoxes+1)*sizeof(NR::Rect));
      boxes[nbBoxes]=iBox;
      nbBoxes++;
    }
  };
  
  virtual box_solution   VeryFirst(void) {
    box_solution res;
    res.finished=(nbBoxes<=0)?true:false;
    res.y=res.ascent=res.descent=res.x_start=res.x_end=0;
    res.frame_no=-1;
    return res;
  };
  virtual box_solution   NextLine(box_solution& after,double asc,double desc,double lead) {
    box_solution res;
    if ( after.frame_no >= nbBoxes ) {
      res.finished=true;
      res.y=res.ascent=res.descent=0;
      res.x_start=res.x_end=0;
      res.frame_no=nbBoxes;
    } else if ( after.frame_no < 0 ) {
      if ( nbBoxes <= 0 ) {
        res.finished=true;
        res.y=res.ascent=res.descent=0;
        res.x_start=res.x_end=0;
        res.frame_no=nbBoxes;
      } else {
        // force the line even if the descent makes it exceed the height of the box (yes, it's bad)
        res.finished=false;
        NR::Point mib=boxes[0].min(),mab=boxes[0].max();
        res.y=mib[1]+asc;
        res.ascent=asc;
        res.descent=desc;
        res.x_start=mib[0];
        res.x_end=mab[0];
        res.frame_no=0;
      }
    } else {
      NR::Point mib=boxes[after.frame_no].min(),mab=boxes[after.frame_no].max();
      res.y=after.y+after.descent+lead+asc;
      if ( res.y+desc > mab[1] ) {
        if ( after.frame_no+1 >= nbBoxes ) {
          res.finished=true;
          res.y=res.ascent=res.descent=0;
          res.x_start=res.x_end=0;
          res.frame_no=nbBoxes;
        } else {
          res.finished=false;
          NR::Point mib=boxes[after.frame_no+1].min(),mab=boxes[after.frame_no+1].max();
          res.y=mib[1]+asc;
          res.ascent=asc;
          res.descent=desc;
          res.x_start=mib[0];
          res.x_end=mab[0];
          res.frame_no=after.frame_no+1;
        }
      } else {
        res.finished=false;
        res.ascent=asc;
        res.descent=desc;
        res.x_start=mib[0];
        res.x_end=mab[0];
        res.frame_no=after.frame_no;
      }
    }
    return res;
  };
  virtual box_solution   NextBox(box_solution& after,double asc,double desc,double lead,bool &stillSameLine) {
    stillSameLine=false;
    return NextLine(after,asc,desc,lead);
  };
  virtual double         RemainingOnLine(box_solution& /*after*/) {return 0;};
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
  
  dest_shape_chunker(void) {
    nbShape=maxShape=0;
    shapes=NULL;
    tempLine=new FloatLigne();
    tempLine2=new FloatLigne();
    lastDate=0;
    maxCache=64;
    nbCache=0;
    caches=(cached_line*)malloc(maxCache*sizeof(cached_line));
  };
  virtual ~dest_shape_chunker(void) {
  	if ( maxCache > 0 ) {
      for (int i=0;i<nbCache;i++) delete caches[i].theLine;
      free(caches);
    }
    maxCache=nbCache=0;
    caches=NULL;
    if ( maxShape > 0 ) {
      for (int i=0;i<nbShape;i++) delete shapes[i].theS;
      free(shapes);
    }
    nbShape=maxShape=0;
    shapes=NULL;
    delete tempLine;
    delete tempLine2;
  };
  
	void                  AppendShape(Shape* iShape);
	void                  ComputeLine(float y,float a,float d,int shNo);
  
  virtual box_solution   VeryFirst(void) {
    box_solution res;
    res.finished=(nbShape<=0)?true:false;
    res.y=res.ascent=res.descent=res.x_start=res.x_end=0;
    res.frame_no=-1;
    return res;
  };
  virtual box_solution   NextLine(box_solution& after,double asc,double desc,double lead);
  virtual box_solution   NextBox(box_solution& after,double asc,double desc,double lead,bool &stillSameLine);
  virtual double         RemainingOnLine(box_solution& after) {
    if ( nbShape <= 0 ) return 0;
    if ( after.frame_no < 0 || after.frame_no >= nbShape ) return 0; // fin
    
    ComputeLine(after.y,after.ascent,after.descent,after.frame_no);  // get the line
    int  elem=0;
    while ( elem < tempLine->nbRun && tempLine->runs[elem].en < after.x_end ) elem++;
    float  left=0;
    // and add the spaces
    while ( elem < tempLine->nbRun ) {
      if ( after.x_end < tempLine->runs[elem].st ) {
        left+=tempLine->runs[elem].en-tempLine->runs[elem].st;
      } else {
        left+=tempLine->runs[elem].en-after.x_end;
      }
      elem++;
    }
    return left;
  };
};
void                  dest_shape_chunker::AppendShape(Shape* iShape) {
  iShape->CalcBBox();  // compute the bounding box
  if ( iShape->rightX-iShape->leftX < 0.001 || iShape->bottomY-iShape->topY < 0.001 ) {
    return;
  }
  
  if ( nbShape >= maxShape ) {
    maxShape=2*nbShape+1;
    shapes=(one_shape_elem*)realloc(shapes,maxShape*sizeof(one_shape_elem));
  }
  int nShape=nbShape++;
		shapes[nShape].theS=new Shape();
		shapes[nShape].theS->Copy(iShape);
    shapes[nShape].theS->BeginRaster(shapes[nShape].curY,shapes[nShape].curPt,1.0);  // a bit of preparation
    shapes[nShape].theS->CalcBBox();
    shapes[nShape].l=shapes[nShape].theS->leftX;
    shapes[nShape].t=shapes[nShape].theS->topY;
    shapes[nShape].r=shapes[nShape].theS->rightX;
    shapes[nShape].b=shapes[nShape].theS->bottomY;
}
void                  dest_shape_chunker::ComputeLine(float y,float a,float d,int shNo) {
  double   top=y-a,bottom=y+d;
  if ( shNo < 0 || shNo >= nbShape ) {
    tempLine->Reset();
    return;
  }
  
  lastDate++;
  
  int oldest=-1;
  int age=lastDate;
  for (int i=0;i<nbCache;i++) {
    if ( caches[i].date < age ) {
      age=caches[i].date;
      oldest=i;
    }
    if ( caches[i].shNo == shNo ) {
      if ( fabs(caches[i].y-y) <= 0.0001 ) {
        if ( fabs(caches[i].a-a) <= 0.0001 ) {
          if ( fabs(caches[i].d-d) <= 0.0001 ) {
            tempLine->Copy(caches[i].theLine);
            return;
          }
        }
      }
    }
  }
  
  tempLine2->Reset();  // par securite
  shapes[shNo].theS->Scan(shapes[shNo].curY,shapes[shNo].curPt,top,bottom-top);
  shapes[shNo].theS->Scan(shapes[shNo].curY,shapes[shNo].curPt,bottom,tempLine2,true,bottom-top);
  tempLine2->Flatten();
  tempLine->Over(tempLine2,0.9*(bottom-top));
  
  if ( nbCache >= maxCache ) {
    if ( oldest < 0 || oldest >= maxCache ) oldest=0;
    caches[oldest].theLine->Reset();
    caches[oldest].theLine->Copy(tempLine);
    caches[oldest].date=lastDate;
    caches[oldest].y=y;
    caches[oldest].a=a;
    caches[oldest].d=d;
    caches[oldest].shNo=caches[oldest].shNo;
  } else {
    oldest=nbCache++;
    caches[oldest].theLine=new FloatLigne();
    caches[oldest].theLine->Copy(tempLine);
    caches[oldest].date=lastDate;
    caches[oldest].y=y;
    caches[oldest].a=a;
    caches[oldest].d=d;
    caches[oldest].shNo=caches[oldest].shNo;
  }
}

box_solution   dest_shape_chunker::NextBox(box_solution& after,double asc,double desc,double lead,bool &stillSameLine) {
  stillSameLine=false;
  if ( nbShape <= 0 || after.frame_no >= nbShape ) {
    box_solution nres;
    nres.finished=true;
    nres.y=nres.ascent=nres.descent=nres.x_start=nres.x_end=0;
    nres.frame_no=(after.frame_no>=nbShape)?nbShape:-1;
    return nres;
  }
  
  if ( after.frame_no >= 0 && after.ascent >= asc && after.descent >= desc ) {
    // we want a span that has the same height as the previous one stored in dest -> we can look for a
    // span on the same line
    ComputeLine(after.y,asc,desc,after.frame_no); // compute the line (should be cached for speed)
    int  elem=0;                // start on the left
    while ( elem < tempLine->nbRun && tempLine->runs[elem].st < after.x_end ) {
      // while we're before the end of the last span, go right
      elem++;
    }
    if ( elem >= 0 && elem < tempLine->nbRun ) {
      // we found a span after the current one->return it
      box_solution nres;
      nres.finished=false;
      nres.y=after.y;
      nres.ascent=(after.ascent>asc)?after.ascent:asc;
      nres.descent=(after.descent>desc)?after.descent:desc;
      nres.frame_no=after.frame_no;
      nres.x_start=tempLine->runs[elem].st;
      nres.x_end=tempLine->runs[elem].en;
      stillSameLine=true;
      return nres;
    }
  }
  
  box_solution dest;
  dest.finished=true;
  dest.ascent=asc;   // change the ascent and descent
  dest.descent=desc;
  dest.x_start=dest.x_end=0;
  if ( after.frame_no < 0 ) {
    dest.frame_no=0;
    dest.y=shapes[0].t; // get the skyline
  } else {
    dest.frame_no=after.frame_no;
    if ( after.y < shapes[after.frame_no].t ) {
      dest.y=shapes[after.frame_no].t; // get the skyline
    } else {
      dest.y=after.y+after.descent+lead; // get the skyline
    }
  }
  while ( dest.y < shapes[dest.frame_no].b ) {
    dest.y+=dest.ascent;
    ComputeLine(dest.y,asc,desc,dest.frame_no);
    int  elem=0;
    if ( elem < tempLine->nbRun ) {
      // found something
      dest.finished=false;
      dest.x_start=tempLine->runs[elem].st;
      dest.x_end=tempLine->runs[elem].en;
      return dest;
    } else {
      //
    }
    dest.y+=dest.descent;
    dest.y+=lead;
  }
  // nothing in this frame -> go to the next one
  dest.frame_no++;
  if ( dest.frame_no < 0 || dest.frame_no >= nbShape ) {
    dest.finished=true;
    dest.frame_no=nbShape;
    return dest;
  }
  dest.y=shapes[dest.frame_no].t-1.0;
  dest.x_start=shapes[dest.frame_no].l-1.0;
  dest.x_end=shapes[dest.frame_no].l-1.0;  // precaution
  dest.ascent=dest.descent=0;
  return NextBox(dest,asc,desc,lead,stillSameLine);
}
box_solution   dest_shape_chunker::NextLine(box_solution& after,double asc,double desc,double lead) {
  if ( nbShape <= 0 || after.frame_no >= nbShape ) {
    box_solution nres;
    nres.finished=true;
    nres.y=nres.ascent=nres.descent=nres.x_start=nres.x_end=0;
    nres.frame_no=(after.frame_no>=nbShape)?nbShape:-1;
    return nres;
  }
  
  box_solution dest;
  dest.finished=true;
  dest.ascent=asc;   // change the ascent and descent
  dest.descent=desc;
  dest.x_start=dest.x_end=0;
  if ( after.frame_no < 0 ) {
    dest.frame_no=0;
  } else {
    dest.frame_no=after.frame_no;
    if ( after.y < shapes[after.frame_no].t ) {
      dest.y=shapes[after.frame_no].t; // get the skyline
    } else {
      dest.y=after.y+after.descent+lead; // get the skyline
    }
  }
  while ( dest.y < shapes[dest.frame_no].b ) {
    dest.y+=dest.ascent;
    ComputeLine(dest.y,asc,desc,dest.frame_no);
    int  elem=0;
    if ( elem < tempLine->nbRun ) {
      // found something
      dest.finished=false;
      dest.x_start=tempLine->runs[elem].st;
      dest.x_end=tempLine->runs[elem].en;
      return dest;
    } else {
      //
    }
    dest.y+=dest.descent;
    dest.y+=lead;
  }
  // nothing in this frame -> go to the next one
  dest.frame_no++;
  if ( dest.frame_no < 0 || dest.frame_no >= nbShape ) {
    dest.finished=true;
    dest.frame_no=nbShape;
    return dest;
  }
  dest.y=shapes[dest.frame_no].t-1.0;
  dest.x_start=shapes[dest.frame_no].l-1.0;
  dest.x_end=shapes[dest.frame_no].l-1.0;  // precaution
  return NextLine(dest,asc,desc,lead);
}

enum {
  p_t_c_none        = 0,
  p_t_c_mergeWhite  = 1
};

class pango_text_chunker : public text_chunker {
public:
  typedef struct elem_box {
    double       x_pos,y_ascent,y_descent,x_length;
    int          t_first,t_last;
    bool         is_white,is_return;
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
  
  pango_text_chunker(char* inText,char* font_family,double font_size,int flags,bool isMarked):text_chunker(inText){
    theText=inText;
    own_text=false;
    theFace=strdup(font_family);
    theSize=font_size;
    theAttrs=NULL;
    
    textLength=0;
    words=NULL;
    nbWord=maxWord=0;
    charas=NULL;
    
    if ( isMarked ) {
      SetTextWithMarkup(inText,flags);
    } else {
      SetText(inText,flags);
    }
  };
  virtual ~pango_text_chunker(void) {
    if ( theAttrs ) pango_attr_list_unref(theAttrs);
    if ( theFace ) free(theFace);
    if ( words ) free(words);
    if ( charas ) free(charas);
    if ( theText && own_text ) free(theText);
  };
  
  virtual void                 SetText(char* inText,int flags) {
    SetTextWithAttrs(inText,NULL,flags);
  };
  virtual void                 ChangeText(int /*startPos*/,int /*endPos*/,char* inText,int flags) {
    SetText(inText,flags);
  };
  virtual int                  MaxIndex(void) {return nbWord;};
  
  virtual void                 InitialMetricsAt(int startPos,double &ascent,double &descent) {
    if ( startPos < 0 ) startPos=0;
    if ( startPos >= nbWord ) {
      ascent=0;
      descent=0;
      return;
    }
    ascent=words[startPos].y_ascent;
    descent=words[startPos].y_descent;
  };
  virtual text_chunk_solution* StuffThatBox(int start_ind,double minLength,double /*nominalLength*/,double maxLength,bool strict);
  
  virtual void                 GlyphsAndPositions(int start_ind,int end_ind,int &nbS,glyphs_for_span* &spans);
  void                         AddBox(int st,int en,bool whit,bool retu,PangoGlyphString* from,int offset,PangoFont* theFont,NR::Point &cumul);
  void                         SetTextWithMarkup(char* inText,int flags);
  void                         SetTextWithAttrs(char* inText,PangoAttrList* resAttr,int flags);
  void                         AddDullGlyphs(glyphs_for_span &dest,double &cumul,int c_st,int c_en);
  void                         AddAttributedGlyphs(glyphs_for_span &dest,double &cumul,int c_st,int c_en,PangoAttrIterator *theIt);
};

text_chunk_solution* pango_text_chunker::StuffThatBox(int start_ind,double minLength,double /*nominalLength*/,double maxLength,bool strict) {
  if ( theText == NULL ) return NULL;
  if ( start_ind < 0 ) start_ind=0;
  if ( start_ind >= nbWord ) return NULL;
  
  int                   solSize=1;
  text_chunk_solution*  sol=(text_chunk_solution*)malloc(solSize*sizeof(text_chunk_solution));
  sol[0].end_of_array=true;
  
  double              startX=words[start_ind].x_pos;
  
  text_chunk_solution lastBefore;
  text_chunk_solution lastWord;
  text_chunk_solution curSol;
  curSol.end_of_array=false;
  curSol.endOfParagraph=false;
  curSol.start_ind=start_ind;
  curSol.end_ind=-1;
  curSol.start_ind=0;
  curSol.length=0;
  curSol.ascent=0;
  curSol.descent=0;
  lastBefore=lastWord=curSol;
  
  bool            inLeadingWhite=true;
  
  for (int cur_ind=start_ind;cur_ind<nbWord;cur_ind++) {
    double endX=words[cur_ind].x_pos+words[cur_ind].x_length;
    double nAscent=words[cur_ind].y_ascent;
    double nDescent=words[cur_ind].y_descent;
    
    curSol.end_ind=cur_ind;
    if ( inLeadingWhite ) {
      if ( words[cur_ind].is_white ) {
        startX=endX;
        curSol.start_ind=cur_ind+1;
      } else {
        curSol.start_ind=cur_ind;
        inLeadingWhite=false;
      }
    } else {
    }
    
    curSol.length=endX-startX;
    if ( nAscent > curSol.ascent ) curSol.ascent=nAscent;
    if ( nDescent > curSol.descent ) curSol.descent=nDescent;
    bool breaksAfter=false;
    curSol.endOfParagraph=false;
    if ( cur_ind < nbWord-1 ) {
      if ( 1 /*pAttrs[cur_ind+1].is_white || pAttrs[cur_ind].is_word_end*/ ) { // already chunked in words
        breaksAfter=true;
      }
    } else {
      // last character of the text
      breaksAfter=true;
    }
    
    if ( words[cur_ind].is_white ) {
    } else {
      lastWord=curSol;
    }
    if ( cur_ind >= nbWord-1 || words[cur_ind+1].is_return ) {
      lastWord.endOfParagraph=true;
    }
    if ( curSol.length < minLength ) {
      if ( breaksAfter ) {
        if ( lastWord.end_ind >= lastWord.start_ind ) {
          if ( lastBefore.end_ind >= lastBefore.start_ind ) {
            if ( lastBefore.endOfParagraph ) {
              sol=(text_chunk_solution*)realloc(sol,(solSize+1)*sizeof(text_chunk_solution));
              sol[solSize]=sol[solSize-1];
              sol[solSize-1]=lastBefore;
              lastBefore.end_ind=lastBefore.start_ind-1;
              solSize++;
            }
          }
          lastBefore=lastWord;
          lastWord.end_ind=lastWord.start_ind-1;
        }
      }
    } else if ( curSol.length > maxLength ) {
      if ( breaksAfter ) {
        if ( strict == false || solSize <= 1 ) {
          if ( lastBefore.end_ind >= lastBefore.start_ind ) {
            sol=(text_chunk_solution*)realloc(sol,(solSize+1)*sizeof(text_chunk_solution));
            sol[solSize]=sol[solSize-1];
            sol[solSize-1]=lastBefore;
            lastBefore.end_ind=lastBefore.start_ind-1;
            solSize++;
          }
          if ( lastWord.end_ind >= lastWord.start_ind ) {
            sol=(text_chunk_solution*)realloc(sol,(solSize+1)*sizeof(text_chunk_solution));
            sol[solSize]=sol[solSize-1];
            sol[solSize-1]=lastWord;
            lastWord.end_ind=lastWord.start_ind-1;
            solSize++;
          }
        }
        break;
      }
    } else {
      if ( breaksAfter ) {
        if ( strict == false ) {
          if ( lastBefore.end_ind >= lastBefore.start_ind ) {
            sol=(text_chunk_solution*)realloc(sol,(solSize+1)*sizeof(text_chunk_solution));
            sol[solSize]=sol[solSize-1];
            sol[solSize-1]=lastBefore;
            lastBefore.end_ind=lastBefore.start_ind-1;
            solSize++;
          }
        }
        if ( lastWord.end_ind >= lastWord.start_ind ) {
          sol=(text_chunk_solution*)realloc(sol,(solSize+1)*sizeof(text_chunk_solution));
          sol[solSize]=sol[solSize-1];
          sol[solSize-1]=lastWord;
          lastWord.end_ind=lastWord.start_ind-1;
          solSize++;
        }
      }
    }
  }
  if ( strict == false || solSize <= 1 ) {
    if ( lastBefore.end_ind >= lastBefore.start_ind ) {
      sol=(text_chunk_solution*)realloc(sol,(solSize+1)*sizeof(text_chunk_solution));
      sol[solSize]=sol[solSize-1];
      sol[solSize-1]=lastBefore;
      lastBefore.end_ind=lastBefore.start_ind-1;
      solSize++;
    }
  }
  return sol;
}

void                 pango_text_chunker::GlyphsAndPositions(int start_ind,int end_ind,int &nbS,glyphs_for_span* &spans) {
  nbS=0;
  int nbSpan=0,maxSpan=0;
  spans=NULL;
  if ( end_ind < start_ind ) return;
  
  PangoAttrIterator* theIt=(theAttrs)?pango_attr_list_get_iterator (theAttrs):NULL;
  int      char_st=words[start_ind].t_first,char_en=words[end_ind].t_last;
  int      attr_st=char_st,attr_en=char_st;
  if ( theIt ) {
    do {
      pango_attr_iterator_range(theIt,&attr_st,&attr_en);
      if ( attr_en > char_st ) break;
    } while ( pango_attr_iterator_next(theIt) );
  } else {
    attr_st=char_en+1;
    attr_en=char_en+2;
  }
  double     cumul=0;
  while ( char_st <= char_en) {
    if ( attr_st > char_st ) {
      if ( nbSpan >= maxSpan ) {
        maxSpan=2*nbSpan+1;
        spans=(glyphs_for_span*)realloc(spans,maxSpan*sizeof(glyphs_for_span));
      }
      AddDullGlyphs(spans[nbSpan],cumul,char_st,(char_en < attr_st-1)?char_en:attr_st-1);        
      nbSpan++;
      char_st=(char_en+1 < attr_st)?char_en+1:attr_st;
    } else {
      if ( attr_en-1 <= char_en ) {
        if ( nbSpan >= maxSpan ) {
          maxSpan=2*nbSpan+1;
          spans=(glyphs_for_span*)realloc(spans,maxSpan*sizeof(glyphs_for_span));
        }
        AddAttributedGlyphs(spans[nbSpan],cumul,char_st,attr_en-1,theIt);
        nbSpan++;
        char_st=attr_en;
        if ( theIt ) {
          if ( pango_attr_iterator_next(theIt) == false ) break;
          pango_attr_iterator_range(theIt,&attr_st,&attr_en);
        } else {
        }
      } else {
        if ( nbSpan >= maxSpan ) {
          maxSpan=2*nbSpan+1;
          spans=(glyphs_for_span*)realloc(spans,maxSpan*sizeof(glyphs_for_span));
        }
        AddAttributedGlyphs(spans[nbSpan],cumul,char_st,char_en,theIt);
        nbSpan++;
        char_st=char_en+1;
        break;
      }
    }
  }
  nbS=nbSpan;
  if ( theIt ) pango_attr_iterator_destroy(theIt);
}
void                         pango_text_chunker::AddBox(int st,int en,bool whit,bool retu,PangoGlyphString* from,int offset,PangoFont* theFont,NR::Point &cumul) {
  if ( en < st ) return;
  PangoRectangle ink_rect,logical_rect;
  pango_glyph_string_extents_range(from,st-offset,en+1-offset,theFont,&ink_rect,&logical_rect);
  
  if ( retu ) whit=false; // so that returns don't get aggregated
  
  if ( nbWord >= maxWord ) {
    maxWord=2*nbWord+1;
    words=(elem_box*)realloc(words,maxWord*sizeof(elem_box));
  }
  words[nbWord].t_first=st;
  words[nbWord].t_last=en;
  words[nbWord].is_white=whit;
  words[nbWord].is_return=retu;
  words[nbWord].x_pos=cumul[0]+(1/((double)PANGO_SCALE))*((double)(logical_rect.x));
  words[nbWord].x_length=(1/((double)PANGO_SCALE))*((double)(logical_rect.width));

  PangoFontMetrics *metrics = pango_font_get_metrics (theFont, NULL);
  
  words[nbWord].y_ascent = (1/((double)PANGO_SCALE))*((double)(pango_font_metrics_get_ascent (metrics)));  
  words[nbWord].y_descent = (1/((double)PANGO_SCALE))*((double)(pango_font_metrics_get_descent (metrics)));  
  words[nbWord].y_ascent+=cumul[1];
  words[nbWord].y_descent-=cumul[1];
  
//  words[nbWord].y_ascent=(1/((double)PANGO_SCALE))*((double)(-logical_rect.y));
//  words[nbWord].y_descent=(1/((double)PANGO_SCALE))*((double)(logical_rect.height+logical_rect.y));
  double   cur_pos=cumul[0];
  int      cur_g=0;
  for (int i=st;i<=en;i++) {
    while ( cur_g < from->num_glyphs-1 && from->log_clusters[cur_g+1] <= i-offset ) cur_g++;
    pango_glyph_string_extents_range(from,i-offset,i+1-offset,theFont,&ink_rect,&logical_rect);
    charas[i].x_pos=cur_pos+(1/((double)PANGO_SCALE))*((double)(logical_rect.x));
    charas[i].y_pos=cumul[1];
    charas[i].x_dpos=(1/((double)PANGO_SCALE))*((double)(from->glyphs[cur_g].geometry.x_offset));
    charas[i].y_dpos=(1/((double)PANGO_SCALE))*((double)(from->glyphs[cur_g].geometry.y_offset));
    cur_pos=charas[i].x_pos+charas[i].x_dpos+(1/((double)PANGO_SCALE))*((double)(logical_rect.width));
  }
  cumul[0]+=words[nbWord].x_length;
  charas[en+1].x_pos=cumul[0];
  charas[en+1].y_pos=cumul[1];
  charas[en+1].x_dpos=0;
  charas[en+1].y_dpos=0;
  nbWord++;
}
void                         pango_text_chunker::SetTextWithMarkup(char* inText,int flags) {
  char*            resText=NULL;
  if ( inText == NULL || strlen(inText) <= 0 ) return;
  if ( pango_parse_markup(inText,strlen(inText),0,&theAttrs,&resText,NULL,NULL) ) {
    SetTextWithAttrs(resText,theAttrs,flags);
    own_text=true;
  } else {
    SetText(NULL,flags);
  }
}
void                         pango_text_chunker::SetTextWithAttrs(char* inText,PangoAttrList* resAttr,int flags) {
  theText=inText;
  textLength=0;
  nbWord=0;
  if ( inText == NULL || strlen(inText) <= 0 ) return;
  
  // pango structures
  PangoContext*         pContext;
  PangoFontDescription* pfd;
  PangoLogAttr*         pAttrs;
  
  pContext=gdk_pango_context_get();
  pfd = pango_font_description_from_string (theFace);
  pango_font_description_set_size (pfd,(int)(PANGO_SCALE*theSize));
  pango_context_set_font_description(pContext,pfd);
  
  int     srcLen=strlen(inText);
  charas=(glyph_box*)malloc((srcLen+2)*sizeof(glyph_box));
  pAttrs=(PangoLogAttr*)malloc((srcLen+2)*sizeof(PangoLogAttr));
  PangoAttrList*   tAttr=(resAttr)?resAttr:pango_attr_list_new();
  GList*  pItems=pango_itemize(pContext,inText,0,srcLen,tAttr,NULL);
  GList*  pGlyphs=NULL;
  for (GList* l=pItems;l;l=l->next) {
    PangoItem*  theItem=(PangoItem*)l->data;
    pango_break(inText+theItem->offset,theItem->length,&theItem->analysis,pAttrs+theItem->offset,theItem->length+1);
    PangoGlyphString*  nGl=pango_glyph_string_new();
    pango_shape(inText+theItem->offset,theItem->length,&theItem->analysis,nGl);
    pGlyphs=g_list_append(pGlyphs,nGl);
  }
  
  int       t_pos=0,l_pos=0;
  NR::Point cumul(0,0);
  bool      inWhite=false;
  
  int       next_par_end=0;
  int       next_par_start=0;
  pango_find_paragraph_boundary(inText,-1,&next_par_end,&next_par_start);
  
  PangoAttrIterator* theIt=(resAttr)?pango_attr_list_get_iterator (resAttr):NULL;

  for (GList* l=pItems,*g=pGlyphs;l&&g;l=l->next,g=g->next) {
    PangoItem*         theItem=(PangoItem*)l->data;
    PangoGlyphString*  theGlyphs=(PangoGlyphString*)g->data;
    
    int      char_st=t_pos,char_en=char_st+theItem->length-1;
    int      attr_st=char_st,attr_en=char_st;
    if ( theIt ) {
      do {
        pango_attr_iterator_range(theIt,&attr_st,&attr_en);
        if ( attr_en > char_st ) break;
      } while ( pango_attr_iterator_next(theIt) );
    } else {
      attr_st=char_en+1;
      attr_en=char_en+2;
    }
    cumul[1]=0;
    if ( attr_st <= char_st ) {
      PangoAttribute* one_attr=NULL;
      one_attr=pango_attr_iterator_get(theIt,PANGO_ATTR_RISE);
      if ( one_attr ) {
        cumul[1]=(1/((double)PANGO_SCALE))*((double)((PangoAttrInt*)one_attr)->value);
      }
    }
    
    bool is_retu=(t_pos >= next_par_end);
    for (int g_pos=0;g_pos<theItem->length;g_pos++) {
      if ( pAttrs[t_pos].is_white ) {
        if ( inWhite ) {
          if ( flags&p_t_c_mergeWhite ) {
          } else {
            if ( l_pos < t_pos ) {
              AddBox(l_pos,t_pos-1,true,is_retu,theGlyphs,theItem->offset,theItem->analysis.font,cumul);
              l_pos=t_pos;
            }
          }
        } else {
          if ( l_pos < t_pos ) {
            AddBox(l_pos,t_pos-1,false,is_retu,theGlyphs,theItem->offset,theItem->analysis.font,cumul);
            l_pos=t_pos;
          }
        }
        inWhite=true;
      } else {
        if ( inWhite ) {
          if ( l_pos < t_pos ) {
            AddBox(l_pos,t_pos-1,true,is_retu,theGlyphs,theItem->offset,theItem->analysis.font,cumul);
            l_pos=t_pos;
          }
        } else if ( pAttrs[t_pos].is_word_start ) {
          if ( l_pos < t_pos ) {
            AddBox(l_pos,t_pos-1,false,is_retu,theGlyphs,theItem->offset,theItem->analysis.font,cumul);
            l_pos=t_pos;
          }
        }
        inWhite=false;
      }
      is_retu=(t_pos >= next_par_end);
      if ( t_pos >= next_par_start ) {
        int old_dec=next_par_start;
        pango_find_paragraph_boundary(inText+old_dec,-1,&next_par_end,&next_par_start);
        next_par_end+=old_dec;
        next_par_start+=old_dec;
        is_retu=(t_pos >= next_par_end);
      }
      
      t_pos++;
   }
    if ( l_pos < t_pos ) {
      AddBox(l_pos,t_pos-1,inWhite,false,theGlyphs,theItem->offset,theItem->analysis.font,cumul);
      l_pos=t_pos;
    }
  }
  
//  printf("%i mots\n",nbWord);
//  for (int i=0;i<nbWord;i++) {
//    printf("%i->%i  x=%f l=%f a=%f d=%f w=%i r=%i\n",words[i].t_first,words[i].t_last,words[i].x_pos,words[i].x_length,words[i].y_ascent,words[i].y_descent
//           ,words[i].is_white,words[i].is_return);
//    printf("chars: ");
//    for (int j=words[i].t_first;j<=words[i].t_last;j++) {
//      printf("(%f %f d= %f) ",charas[j].x_pos,charas[j].y_pos,charas[j].y_dpos);
//    }
//    printf("\n");
//  }
  
  if ( theIt ) pango_attr_iterator_destroy(theIt);

  while ( pGlyphs ) {
    PangoGlyphString*  nGl=(PangoGlyphString*)pGlyphs->data;
    pGlyphs=g_list_remove(pGlyphs,nGl);
    pango_glyph_string_free(nGl);
  }
  while ( pItems ) {
    PangoItem*  nIt=(PangoItem*)pItems->data;
    pItems=g_list_remove(pItems,nIt);
    pango_item_free(nIt);
  }
  free(pAttrs);
  g_object_unref(pContext);
  pango_font_description_free(pfd);
  if ( resAttr == NULL ) pango_attr_list_unref(tAttr);
}
void                         pango_text_chunker::AddDullGlyphs(glyphs_for_span &dest,double &cumul,int c_st,int c_en) {
  dest.style[0]=0;
  dest.g_text=theText;
  sprintf(dest.style,"font-family:%s;font-size=%f;",theFace,theSize);
  dest.nbG=c_en-c_st+1;
  dest.g_pos=(NR::Point*)malloc((dest.nbG+1)*sizeof(NR::Point));
  dest.g_start=(int*)malloc((dest.nbG+1)*sizeof(int));
  dest.g_end=(int*)malloc((dest.nbG+1)*sizeof(int));
  double startX=charas[c_st].x_pos-cumul;
  for (int i=c_st;i<=c_en;i++) {
    dest.g_pos[i-c_st]=NR::Point(charas[i].x_pos-startX,charas[i].y_dpos);
    dest.g_start[i-c_st]=i;
    dest.g_end[i-c_st]=i;
  }
  dest.g_pos[dest.nbG]=NR::Point(charas[c_en+1].x_pos-startX,charas[c_en+1].y_dpos);
  dest.g_start[dest.nbG]=c_en+1;
  dest.g_end[dest.nbG]=c_en+1;
  cumul+=dest.g_pos[dest.nbG][0]-dest.g_pos[0][0];
}
void                         pango_text_chunker::AddAttributedGlyphs(glyphs_for_span &dest,double &cumul,int c_st,int c_en,PangoAttrIterator *theIt) {
  dest.style[0]=0;
  dest.g_text=theText;
  PangoAttribute* one_attr=NULL;
  one_attr=pango_attr_iterator_get(theIt,PANGO_ATTR_FAMILY);
  if ( one_attr ) {
    sprintf(dest.style,"font-family:%s;",((PangoAttrString*)one_attr)->value);
  }
  one_attr=pango_attr_iterator_get(theIt,PANGO_ATTR_SIZE);
  if ( one_attr ) {
    double nSize=(1/((double)PANGO_SCALE))*((double)((PangoAttrInt*)one_attr)->value);
    if ( one_attr ) {
      nSize*=((double)((PangoAttrFloat*)one_attr)->value);
    }
    sprintf(dest.style+strlen(dest.style),"font-size:%lf;",(1/((double)PANGO_SCALE))*((double)((PangoAttrInt*)one_attr)->value));
  } else {
    one_attr=pango_attr_iterator_get(theIt,PANGO_ATTR_SCALE);
    if ( one_attr ) {
      sprintf(dest.style+strlen(dest.style),"font-size:%lf;",theSize*((double)((PangoAttrFloat*)one_attr)->value));
    }
  }
  double y_shift=0;
  one_attr=pango_attr_iterator_get(theIt,PANGO_ATTR_RISE);
  if ( one_attr ) {
    y_shift=(1/((double)PANGO_SCALE))*((double)((PangoAttrInt*)one_attr)->value);
  }
  dest.nbG=c_en-c_st+1;
  dest.g_pos=(NR::Point*)malloc((dest.nbG+1)*sizeof(NR::Point));
  dest.g_start=(int*)malloc((dest.nbG+1)*sizeof(int));
  dest.g_end=(int*)malloc((dest.nbG+1)*sizeof(int));
  double startX=charas[c_st].x_pos-cumul;
  for (int i=c_st;i<=c_en;i++) {
    dest.g_pos[i-c_st]=NR::Point(charas[i].x_pos-startX,charas[i].y_dpos-y_shift);
    dest.g_start[i-c_st]=i;
    dest.g_end[i-c_st]=i;
  }
  dest.g_pos[dest.nbG]=NR::Point(charas[c_en+1].x_pos-startX,charas[c_en+1].y_dpos-y_shift);
  dest.g_start[dest.nbG]=c_en+1;
  dest.g_end[dest.nbG]=c_en+1;
  cumul+=dest.g_pos[dest.nbG][0]-dest.g_pos[0][0];
}




/*
 *
 *
 *
 */

typedef struct typeset_step {
  box_solution      box;
  int               start_ind,end_ind;
  int               nbGlyph;
  bool              no_justification;
} typeset_step;

void   sp_typeset_relayout(SPTypeset *typeset)
{
  if ( typeset == NULL || typeset->layoutDirty == false ) return;
  typeset->layoutDirty=false;
  
  if ( typeset->theDst ) delete typeset->theDst;
  typeset->theDst=NULL;
  if ( typeset->dstType == has_shape_dest ) {
    GSList* l=typeset->dstElems;
    dest_shape_chunker* nd=new dest_shape_chunker();
    typeset->theDst=nd;
    while ( l && l->data ) {
      shape_dest* theData=(shape_dest*)l->data;
      if ( theData->theShape ) nd->AppendShape(theData->theShape);
      l=l->next;
    }
  } else if ( typeset->dstType == has_path_dest ) {
  } else if ( typeset->dstType == has_box_dest ) {
    GSList* l=typeset->dstElems;
    dest_box_chunker* nd=new dest_box_chunker();
    typeset->theDst=nd;
    while ( l && l->data ) {
      box_dest* theData=(box_dest*)l->data;
      nd->AppendBox(theData->box);
      l=l->next;
    }
  } else if ( typeset->dstType == has_col_dest ) {
    if ( typeset->dstElems && typeset->dstElems->data ) {
      column_dest* theData=(column_dest*)typeset->dstElems->data;
      typeset->theDst=new dest_col_chunker(theData->width);
    }
  }
  if ( typeset->theSrc ) delete typeset->theSrc;
  typeset->theSrc=NULL;
  if ( typeset->srcType == has_std_txt ) {
    SPCSSAttr *css;
    css = sp_repr_css_attr (SP_OBJECT_REPR (SP_OBJECT(typeset)), "style");
    const gchar *val_size = sp_repr_css_property (css, "font-size", NULL);
    double  fsize=12.0;
    if ( val_size ) fsize = sp_repr_css_double_property (css, "font-size", 12.0);
    const gchar *val_family = sp_repr_css_property (css, "font-family", NULL);
    if ( val_family ) {
      typeset->theSrc = new pango_text_chunker(typeset->srcText, (gchar *) val_family, fsize, p_t_c_none,false);
    } else {
      typeset->theSrc = new pango_text_chunker(typeset->srcText, "Luxi Sans", fsize, p_t_c_none,false);
    }
  } else if ( typeset->srcType == has_pango_txt ) {
    SPCSSAttr *css;
    css = sp_repr_css_attr (SP_OBJECT_REPR (SP_OBJECT(typeset)), "style");
    const gchar *val_size = sp_repr_css_property (css, "font-size", NULL);
    double  fsize=12.0;
    if ( val_size ) fsize = sp_repr_css_double_property (css, "font-size", 12.0);
    const gchar *val_family = sp_repr_css_property (css, "font-family", NULL);
    if ( val_family ) {
      typeset->theSrc = new pango_text_chunker(typeset->srcText, (gchar *) val_family, fsize, p_t_c_none,true);
    } else {
      typeset->theSrc = new pango_text_chunker(typeset->srcText, "Luxi Sans", fsize, p_t_c_none,true);
    }
  }
  
  
  // kill children
  {
    GSList *l=NULL;
    for (	SPObject * child = sp_object_first_child(SP_OBJECT(typeset)) ; child != NULL ; child = SP_OBJECT_NEXT(child) ) {
      l=g_slist_prepend(l,child);
    }
    while ( l ) {
      SPObject *child=(SPObject*)l->data;
      child->deleteObject();
//      sp_object_unref(child, SP_OBJECT(typeset));
      l=g_slist_remove(l,child);
    }
  }
  // do layout
  typeset_step  *steps=(typeset_step*)malloc(sizeof(typeset_step));
  int           nb_step=1;
  steps[0].box.x_start=steps[0].box.x_end=0;
  steps[0].box.y=steps[0].box.ascent=steps[0].box.descent=0;
  steps[0].start_ind=-1;
  steps[0].end_ind=-1;
  steps[0].nbGlyph=0;

  if ( typeset->theSrc && typeset->theDst ) {
    // dumb layout: stuff 'til it's too big    
    double nAscent=0.0,nDescent=0.0;
    typeset->theSrc->InitialMetricsAt(0,nAscent,nDescent);
    int           maxIndex=typeset->theSrc->MaxIndex();
    if ( nAscent < 0.0001 && nDescent < 0.0001 ) {
      // nothing to stuff?
    } else {
      int           last_step=0;
      int           prev_line_step=0;
      bool          jump_to_next_line=false;
      bool          start_need_reach=false;
      int           need_to_reach_ind=-1;
      typeset_step* sav_steps=NULL;
      int           nb_sav_step=0;
      
      steps[prev_line_step].box=typeset->theDst->VeryFirst();
      last_step=prev_line_step;
            
      do {
        int           cur_pos=steps[last_step].end_ind+1;
        bool          sameLine=false;
        box_solution  cur_box;
        if ( jump_to_next_line ) {
//          printf("it's just a jump to the left\n");
          cur_box=typeset->theDst->NextLine(steps[last_step].box,nAscent,nDescent,0.0);
          jump_to_next_line=false;
        } else {
          cur_box=typeset->theDst->NextBox(steps[last_step].box,nAscent,nDescent,0.0,sameLine);
        }
        if ( cur_box.finished ) break;
        double nLen=cur_box.x_end-cur_box.x_start;
        text_chunk_solution* sol=typeset->theSrc->StuffThatBox(cur_pos,0.8*nLen,nLen,1.2*nLen,false);
        if ( sol == NULL ) {
          break;
        }
        if ( sol[0].end_of_array ) {
          free(sol);
          break;
        }
        
        int     best=0;
        if ( sol[best].endOfParagraph ) {
        } else {
          for (int i=1;sol[i].end_of_array==false;i++) {
            if ( sol[i].length < nLen ) {
              best=i;
              if ( sol[best].endOfParagraph ) {
                break;
              }
            }
          }
        }
        if ( sameLine == false ) {
          if ( start_need_reach ) {
            start_need_reach=false;
          } else {
            if ( need_to_reach_ind >= 0 ) {
              if ( sol[best].end_ind <= need_to_reach_ind ) {
                // didn't improve, revert to saved steps
                last_step=prev_line_step+nb_sav_step;
                memcpy(steps+(prev_line_step+1),sav_steps,nb_sav_step*sizeof(typeset_step));
                need_to_reach_ind=-1;
                if ( sav_steps ) free(sav_steps);
                sav_steps=NULL;
                nb_sav_step=0;
                prev_line_step=last_step;
                jump_to_next_line=true;
                continue;
              } else {
                need_to_reach_ind=-1;
                if ( sav_steps ) free(sav_steps);
                sav_steps=NULL;
                nb_sav_step=0;
              }
            }
          }
        }

        if ( sol[best].length > nLen ) {
          // ouchie: doesn't fit in line
          steps=(typeset_step*)realloc(steps,(nb_step+1)*sizeof(typeset_step));
          steps[nb_step].box=cur_box;
          steps[nb_step].start_ind=sol[best].start_ind;
          steps[nb_step].end_ind=sol[best].start_ind-1;
          steps[nb_step].nbGlyph=0;
          nb_step++;
          
          last_step=nb_step-1;
          if ( sameLine ) {
          } else {
            prev_line_step=last_step;
          }
          typeset->theSrc->InitialMetricsAt(steps[last_step].end_ind+1,nAscent,nDescent);
          
          free(sol);
          if ( steps[last_step].end_ind+1 >= maxIndex ) break;
          continue;
        }
        if ( sol[best].ascent > nAscent || sol[best].descent > nDescent ) {
          nAscent=sol[best].ascent;
          nDescent=sol[best].descent;
          free(sol);
          start_need_reach=true;
          need_to_reach_ind=steps[last_step].end_ind;
          nb_sav_step=last_step-prev_line_step;
          sav_steps=(typeset_step*)malloc(nb_sav_step*sizeof(typeset_step));
          memcpy(sav_steps,steps+(prev_line_step+1),nb_sav_step*sizeof(typeset_step));
          if ( last_step > prev_line_step ) {
            nb_step=prev_line_step+1;
            last_step=prev_line_step;
          }
          if ( steps[last_step].end_ind+1 >= maxIndex ) {
            free(sav_steps);
            sav_steps=NULL;
            nb_sav_step=0;
            break;
          }
          continue;
        }
        
        steps=(typeset_step*)realloc(steps,(nb_step+1)*sizeof(typeset_step));
        steps[nb_step].box=cur_box;
        steps[nb_step].start_ind=sol[best].start_ind;
        steps[nb_step].end_ind=sol[best].end_ind;
        steps[nb_step].nbGlyph=0;
        steps[nb_step].no_justification=false;
        jump_to_next_line=false;
        if ( sol[best].endOfParagraph ) {
          steps[nb_step].no_justification=true;
          jump_to_next_line=true;
        }
        nb_step++;
        
        last_step=nb_step-1;
        if ( sameLine ) {
        } else {
          prev_line_step=last_step;
        }
        typeset->theSrc->InitialMetricsAt(steps[last_step].end_ind+1,nAscent,nDescent);
        
        free(sol);
      } while ( steps[last_step].end_ind+1 < maxIndex );
      if ( sav_steps ) free(sav_steps);
    }
  }
  // create offspring
  {
    int           maxIndex=(typeset->theSrc)?typeset->theSrc->MaxIndex():0;

    SPRepr *parent = SP_OBJECT_REPR(SP_OBJECT(typeset));
    char const *style = sp_repr_attr (parent, "style");
    if (!style) {
	    style = "font-family:Sans;font-size:12;";
    }
    
    SPRepr* text_repr = sp_repr_new ("text");
    sp_repr_set_attr (text_repr, "style", style);
    sp_repr_append_child (parent, text_repr);
    sp_repr_unref (text_repr);

    for (int i=1;i<nb_step;i++) {
      if ( steps[i].end_ind >= steps[i].start_ind ) {
        int              nbS=0;
        glyphs_for_span  *span_info=NULL;
        typeset->theSrc->GlyphsAndPositions(steps[i].start_ind,steps[i].end_ind,nbS,span_info);
        double spacing=steps[i].box.x_end-steps[i].box.x_start;
        int    nbSrcChar=0;
        for (int k=0;k<nbS;k++) {
          if ( span_info[k].nbG > 0 ) {
            spacing-=span_info[k].g_pos[span_info[k].nbG][0]-span_info[k].g_pos[0][0];
            for (int j=0;j<span_info[k].nbG;j++) nbSrcChar+=span_info[k].g_end[j]-span_info[k].g_start[j]+1;
          }
        }
        if ( nbSrcChar > 1 ) {
          if ( ( steps[i].end_ind < maxIndex-1 && steps[i].no_justification == false ) || spacing < 0 ) {
            spacing/=(nbSrcChar-1);
          } else {
            spacing=0;
          }
        } else {
          spacing=0;
        }
        
        for (int k=0;k<nbS;k++) {
          SPRepr* span_repr = sp_repr_new ("tspan");
          
          if ( span_info[k].style[0] != 0 ) {
            sp_repr_set_attr (span_repr, "style", span_info[k].style);
          }
          
          if ( span_info[k].g_pos && span_info[k].g_start && span_info[k].g_end && span_info[k].nbG > 0 ) {
            NR::Point   textPos(steps[i].box.x_start,steps[i].box.y);
            textPos+=span_info[k].g_pos[0];
            int nbPrevChar=span_info[k].g_start[0]-span_info[0].g_start[0];
            sp_repr_set_double (span_repr, "x", textPos[0]+spacing*((double)(nbPrevChar)));
            sp_repr_set_double (span_repr, "y", textPos[1]);
            
            {
              SPCSSAttr *ocss;
              ocss = sp_repr_css_attr (span_repr, "style");              
              sp_repr_set_double ((SPRepr*)ocss, "letter-spacing", spacing);
              sp_repr_css_change (span_repr, ocss, "style");
              sp_repr_css_attr_unref (ocss);
            }
            bool  do_dy=false;
            for (int j = 0; j < span_info[k].nbG ; j ++) {
              if ( fabs(span_info[k].g_pos[j][1]) > 0.001 ) {
                do_dy=true;
                break;
              }
            }
            if ( do_dy ) {
              gchar c[32];
              gchar *s = NULL;
              double    lastY=span_info[k].g_pos[0][1];
              for (int j = 0; j < span_info[k].nbG ; j ++) {
                int     t_st=span_info[k].g_start[j];
                int     t_en=span_info[k].g_end[j];
                for (int g=t_st;g<=t_en;g++) {
                  g_ascii_formatd (c, sizeof (c), "%.8g", span_info[k].g_pos[j][1]-lastY);
                  lastY=span_info[k].g_pos[j][1];
                  if (s == NULL) {
                    s = g_strdup (c);
                  }  else {
                    s = g_strjoin (" ", s, c, NULL);
                  }
                }
              }
              sp_repr_set_attr (span_repr, "dy", s);
              g_free(s);
            }
          } else {
            sp_repr_set_double (span_repr, "x", steps[i].box.x_start);
            sp_repr_set_double (span_repr, "y", steps[i].box.y);
          }
          
          int   content_length=0;
          char* temp_content=NULL;
          for (int j = 0; j < span_info[k].nbG ; j ++) {
            int     t_st=span_info[k].g_start[j];
            int     t_en=span_info[k].g_end[j];
            if ( t_en >= t_st ) {
              temp_content=(char*)realloc(temp_content,(content_length+t_en-t_st+1)*sizeof(char));
              memcpy(temp_content+content_length,span_info[k].g_text+t_st,(t_en-t_st+1)*sizeof(char));
              content_length+=t_en-t_st+1;
            }
          }
          temp_content=(char*)realloc(temp_content,(content_length+1)*sizeof(char));
          temp_content[content_length]=0;
          SPRepr* rstr = sp_xml_document_createTextNode (sp_repr_document (parent), temp_content);
          sp_repr_append_child (span_repr, rstr);
          sp_repr_unref (rstr);
          free(temp_content);
          
          sp_repr_append_child (text_repr, span_repr);
          sp_repr_unref (span_repr);
        }
        for (int k=0;k<nbS;k++) {
          if ( span_info[k].g_start ) free(span_info[k].g_start);
          if ( span_info[k].g_end ) free(span_info[k].g_end);
          if ( span_info[k].g_pos ) free(span_info[k].g_pos);
        }
        if ( span_info ) free(span_info);
      }
    }
    free(steps);
    
    SPDesktop *desktop = SP_ACTIVE_DESKTOP;
    sp_document_done (SP_DT_DOCUMENT (desktop));
  }
}
