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


// data stored for each shape:
typedef struct one_shape_elem {
	Shape*   theS;     // the shape itself
	double   l,t,r,b;  // the bounding box
	int      curPt;
	float    curY;
} one_shape_elem;

class dest_shape_chunker: public dest_chunker {
public:
	int                   nbShape,maxShape;
	one_shape_elem        *shapes;
  
	typedef struct cached_line {
		int          date;
		FloatLigne*  theLine;
		int          shNo;
		double       y,a,d;
	} cached_line;
  
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
  
	void                  AppendShape(Shape* iShape) {
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
  };
	void                  ComputeLine(float y,float a,float d,int shNo) {
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
    shapes[shNo].theS->Scan(shapes[shNo].curY,shapes[shNo].curPt,top,1.0);
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
  };

  virtual box_solution   VeryFirst(void) {
    box_solution res;
    res.finished=(nbShape<=0)?true:false;
    res.y=res.ascent=res.descent=res.x_start=res.x_end=0;
    res.frame_no=-1;
    return res;
  };
  virtual box_solution   NextLine(box_solution& after,double asc,double desc,double lead) {
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
  };
  
  virtual box_solution   NextBox(box_solution& after,double asc,double desc,double lead,bool &stillSameLine) {
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
  };
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

class pango_text_chunker : public text_chunker {
public:
  char*          theText; // not owned by the class
  
  char*          theFace;
  double         theSize;

  int            textLength;
  PangoContext*  pContext;
  PangoLayout*   pLayout;
  PangoLogAttr  *pAttrs;
  int            pNAttr;
  
  int            baselineY;
  
  pango_text_chunker(char* inText,char* font_family,double font_size):text_chunker(inText){
    theText=inText;
    theFace=strdup(font_family);
    theSize=font_size;
    
    pContext=gdk_pango_context_get();
    pLayout=pango_layout_new(pContext);
    pango_layout_set_wrap(pLayout,PANGO_WRAP_WORD);
    PangoFontDescription  *pfd;
    
    pfd = pango_font_description_from_string (theFace);
    pango_font_description_set_size (pfd,(int)(PANGO_SCALE*theSize));
    pango_layout_set_font_description(pLayout, pfd);
    pango_layout_set_spacing  (pLayout,0);
    pNAttr=0;
    pAttrs=NULL;
    textLength=0;
    if ( theText ) {
      pango_layout_set_text(pLayout, theText, -1);
      pango_layout_get_log_attrs(pLayout,&pAttrs,&pNAttr);
//      textLength=strlen(theText);
      textLength=pNAttr; // is it correct?
      
      PangoLayoutIter* pIter=pango_layout_get_iter(pLayout);
      baselineY=pango_layout_iter_get_baseline(pIter);
      pango_layout_iter_free (pIter);
    }
  };
  virtual ~pango_text_chunker(void) {
    if ( theFace ) free(theFace);
    g_object_unref(pContext);
    g_object_unref(pLayout);
    pContext=NULL;
  };
  
  virtual void                 SetText(char* inText) {
    theText=inText;
    if ( pAttrs ) g_free(pAttrs);
    pNAttr=0;
    pAttrs=NULL;
    textLength=0;
    if ( theText ) {       
      textLength=pNAttr; // is it correct?
      pango_layout_set_text(pLayout, theText, -1);
      pango_layout_get_log_attrs(pLayout,&pAttrs,&pNAttr);
    }
  };
  virtual void                 ChangeText(int /*startPos*/,int /*endPos*/,char* inText) {
    SetText(inText);
  };
  virtual int                  MaxIndex(void) {return textLength;};
  
  virtual void                 InitialMetricsAt(int startPos,double &ascent,double &descent) {
    if ( startPos < 0 ) startPos=0;
    if ( startPos >= textLength ) {
      ascent=0;
      descent=0;
      return;
    }
    PangoRectangle  meas;
    pango_layout_index_to_pos(pLayout,startPos,&meas);
    ascent=0.001*((double)(baselineY-meas.y));
    descent=0.001*((double)(meas.y+meas.height-baselineY));
  };
  virtual text_chunk_solution* StuffThatBox(int start_ind,double minLength,double /*nominalLength*/,double maxLength,bool strict) {
    if ( theText == NULL || pLayout == NULL || pAttrs == NULL ) return NULL;
    if ( start_ind < 0 ) start_ind=0;
    if ( start_ind >= textLength ) return NULL;
    
    int                   solSize=1;
    text_chunk_solution*  sol=(text_chunk_solution*)malloc(solSize*sizeof(text_chunk_solution));
    sol[0].end_of_array=true;
    
    PangoRectangle  meas;
    pango_layout_index_to_pos(pLayout,start_ind,&meas);
    int             startX=meas.x;
    
    text_chunk_solution lastBefore;
    text_chunk_solution lastWord;
    text_chunk_solution curSol;
    curSol.end_of_array=false;
    curSol.start_ind=start_ind;
    curSol.end_ind=-1;
    curSol.start_ind=0;
    curSol.length=0;
    curSol.ascent=0;
    curSol.descent=0;
    lastBefore=lastWord=curSol;

    bool            inLeadingWhite=true;
        
    for (int cur_ind=start_ind;cur_ind<textLength;cur_ind++) {
      pango_layout_index_to_pos(pLayout,cur_ind,&meas);
      int    endX=meas.x+meas.width;
      double nAscent=0.001*((double)(baselineY-meas.y));
      double nDescent=0.001*((double)(meas.y+meas.height-baselineY));
      
      curSol.end_ind=cur_ind;
      if ( inLeadingWhite ) {
        if ( pAttrs[cur_ind].is_white ) {
          startX=endX;
          curSol.start_ind=cur_ind+1;
        } else {
          curSol.start_ind=cur_ind;
          inLeadingWhite=false;
        }
      } else {
      }
      
      curSol.length=0.001*((double)(endX-startX));
      if ( nAscent > curSol.ascent ) curSol.ascent=nAscent;
      if ( nDescent > curSol.descent ) curSol.descent=nDescent;
      bool breaksAfter=false;
      if ( cur_ind < textLength-1 ) {
        if ( pAttrs[cur_ind+1].is_white || pAttrs[cur_ind].is_word_end ) {
          breaksAfter=true;
        }
      } else {
        // last character of the text
        breaksAfter=true;
      }
      
      if ( pAttrs[cur_ind].is_white ) {
      } else {
        lastWord=curSol;
      }
      if ( curSol.length < minLength ) {
        if ( breaksAfter ) {
          if ( lastWord.end_ind >= lastWord.start_ind ) {
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
  };
  virtual void                 GlyphsAndPositions(int start_ind,int end_ind,int &nbS,glyphs_for_span* &spans) {
    nbS=1;
    spans=(glyphs_for_span*)malloc(sizeof(glyphs_for_span));
    spans[0].nbG=-1;
    spans[0].g_pos=NULL;
    spans[0].g_start=NULL;
    spans[0].g_end=NULL;
    spans[0].style[0]=0;
    if ( end_ind < start_ind ) return;
    sprintf(spans[0].style,"font-family:%s;font-size:%f;",theFace,theSize);
    spans[0].nbG=end_ind-start_ind+1;
    spans[0].g_pos=(NR::Point*)malloc((spans[0].nbG+1)*sizeof(NR::Point));
    spans[0].g_start=(int*)malloc((spans[0].nbG+1)*sizeof(int));
    spans[0].g_end=(int*)malloc((spans[0].nbG+1)*sizeof(int));
    PangoRectangle meas;
    pango_layout_index_to_pos(pLayout,start_ind,&meas);
    int startX=meas.x;
    int endX=meas.x+meas.width;
    for (int i=start_ind;i<=end_ind;i++) {
      pango_layout_index_to_pos(pLayout,i,&meas);
      spans[0].g_pos[i-start_ind]=NR::Point(0.001*((double)(meas.x-startX)),0);
      spans[0].g_start[i-start_ind]=i; // assumes correspondance char <-> grapheme
      spans[0].g_end[i-start_ind]=i;
      endX=meas.x+meas.width;
    }
    spans[0].g_pos[spans[0].nbG]=NR::Point(0.001*((double)(endX-startX)),0);
    spans[0].g_start[spans[0].nbG]=end_ind+1;
    spans[0].g_end[spans[0].nbG]=end_ind+1;
  };
};



/*
 *
 *
 *
 */

typedef struct typeset_step {
  box_solution      box;
  int               start_ind,end_ind;
  int               nbGlyph;
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
      typeset->theSrc = new pango_text_chunker(typeset->srcText, (gchar *) val_family, fsize);
    } else {
      typeset->theSrc = new pango_text_chunker(typeset->srcText, "Luxi Sans", fsize);
    }

  } else if ( typeset->srcType == has_pango_txt ) {
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
      steps[prev_line_step].box=typeset->theDst->VeryFirst();
      last_step=prev_line_step;
            
      do {
        int           cur_pos=steps[last_step].end_ind+1;
        bool          sameLine=false;
        box_solution  cur_box=typeset->theDst->NextBox(steps[last_step].box,nAscent,nDescent,2.0,sameLine);
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
        for (int i=1;sol[i].end_of_array==false;i++) {
          if ( sol[i].length < nLen ) {
            best=i;
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
          if ( last_step > prev_line_step ) {
            nb_step=prev_line_step+1;
            last_step=prev_line_step;
          }
          if ( steps[last_step].end_ind+1 >= maxIndex ) break;
          continue;
        }
        
        steps=(typeset_step*)realloc(steps,(nb_step+1)*sizeof(typeset_step));
        steps[nb_step].box=cur_box;
        steps[nb_step].start_ind=sol[best].start_ind;
        steps[nb_step].end_ind=sol[best].end_ind;
        steps[nb_step].nbGlyph=0;
        nb_step++;
        
        last_step=nb_step-1;
        if ( sameLine ) {
        } else {
          prev_line_step=last_step;
        }
        typeset->theSrc->InitialMetricsAt(steps[last_step].end_ind+1,nAscent,nDescent);
        
        free(sol);
      } while ( steps[last_step].end_ind+1 < maxIndex );
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
//      printf("line %i : %i -> %i = %f; %lf -> %lf (%lf %lf %lf)\n",i,steps[i].start_ind,steps[i].end_ind,,steps[i].length
//             ,steps[i].box.x_start,steps[i].box.x_end
//             ,steps[i].box.y,steps[i].box.ascent,steps[i].box.descent);
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
          if ( steps[i].end_ind < maxIndex-1 || spacing < 0 ) {
            spacing/=(nbSrcChar-1);
          } else {
            spacing=0;
          }
        } else {
          spacing=0;
        }
        
        for (int k=0;k<nbS;k++) {
          SPRepr* span_repr = sp_repr_new ("tspan");
          
          if ( span_info[k].style[0] != 0 ) sp_repr_set_attr (span_repr, "style", span_info[k].style);
          
          if ( span_info[k].g_pos && span_info[k].g_start && span_info[k].g_end && span_info[k].nbG > 0 ) {
            NR::Point   textPos(steps[i].box.x_start,steps[i].box.y);
            textPos+=span_info[k].g_pos[0];
            sp_repr_set_double (span_repr, "x", textPos[0]);
            sp_repr_set_double (span_repr, "y", textPos[1]);
            
            {
              SPCSSAttr *ocss;
              ocss = sp_repr_css_attr (span_repr, "style");              
              sp_repr_set_double ((SPRepr*)ocss, "letter-spacing", spacing);
              sp_repr_css_change (span_repr, ocss, "style");
              sp_repr_css_attr_unref (ocss);
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
              memcpy(temp_content+content_length,typeset->srcText+t_st,(t_en-t_st+1)*sizeof(char));
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
