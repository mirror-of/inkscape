#define __sp_typeset_layout_C__

/*
 * SVG <g> implementation
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * Copyright (C) 1999-2002 authors
 * Copyright (C) 2000-2001 Ximian, Inc.
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <config.h>
#include <string.h>

#include "display/nr-arena-group.h"
#include "xml/repr-private.h"
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
  virtual double         RemainingOnLine(box_solution& after) {return 0;};
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
  virtual double         RemainingOnLine(box_solution& after) {return 0;};
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
  int            textLength;
  PangoContext*  pContext;
  PangoLayout*   pLayout;
  PangoLogAttr  *pAttrs;
  int            pNAttr;
  
  double         theAscent,theDescent;
  
  pango_text_chunker(char* inText):text_chunker(inText){
    theText=inText;
    pContext=gdk_pango_context_get();
    pLayout=pango_layout_new(pContext);
    pango_layout_set_wrap(pLayout,PANGO_WRAP_WORD_CHAR);
    PangoFontDescription  *pfd;
    
    pfd = pango_font_description_from_string ("Sans 14");
    pango_layout_set_font_description(pLayout, pfd);
    theAscent=10;
    theDescent=4;
    pNAttr=0;
    pAttrs=NULL;
    textLength=0;
    if ( theText ) {
      textLength=strlen(theText);
      pango_layout_set_text(pLayout, theText, -1);
      pango_layout_get_log_attrs(pLayout,&pAttrs,&pNAttr);
    }
  };
  virtual ~pango_text_chunker(void) {
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
      textLength=strlen(theText);
      pango_layout_set_text(pLayout, theText, -1);
      pango_layout_get_log_attrs(pLayout,&pAttrs,&pNAttr);
    }
  };
  virtual void                 ChangeText(int startPos,int endPos,char* inText) {
    SetText(inText);
  };
  
  virtual void                 InitialMetricsAt(int startPos,double &ascent,double &descent) {
    ascent=theAscent;
    descent=theDescent;
  };
  virtual text_chunk_solution* StuffThatBox(int start_ind,double minLength,double nominalLength,double maxLength,bool strict) {
    if ( theText == NULL || pLayout == NULL || pAttrs == NULL ) return NULL;
    if ( start_ind < 0 ) start_ind=0;
    if ( start_ind >= textLength ) return NULL;
    
    int                   solSize=1;
    text_chunk_solution*  sol=(text_chunk_solution*)malloc(solSize*sizeof(text_chunk_solution));
    sol[0].end_of_array=true;
    
    PangoRectangle  meas;
    pango_layout_index_to_pos(pLayout,start_ind,&meas);
    int             startX=meas.x;
    
    int             lastBeforeInd=-1;
    double          lastBeforeLen=0.0;
    int             firstAfterInd=-1;
    double          firstAfterLen=0.0;
    
    for (int cur_ind=start_ind;cur_ind<textLength;cur_ind++) {
      pango_layout_index_to_pos(pLayout,cur_ind,&meas);
      int    endX=meas.x+meas.width;
      double lineLength=endX;
      lineLength-=startX;
      lineLength/=1000;
      bool breaksAfter=false;
      if ( cur_ind < textLength-1 ) {
        if ( pAttrs[cur_ind+1].is_char_break || pAttrs[cur_ind+1].is_white || pAttrs[cur_ind+1].is_mandatory_break ) {
          breaksAfter=true;
        }
      } else {
        // last character of the text
        breaksAfter=true;
      }
      if ( lineLength < minLength ) {
        if ( breaksAfter ) {
          lastBeforeInd=cur_ind;
          lastBeforeLen=lineLength;
        }
      } else if ( lineLength > maxLength ) {
        if ( breaksAfter ) {
          firstAfterInd=cur_ind;
          firstAfterLen=lineLength;
          if ( strict == false || solSize <= 1 ) {
            sol=(text_chunk_solution*)realloc(sol,(solSize+1)*sizeof(text_chunk_solution));
            sol[solSize]=sol[solSize-1];
            sol[solSize-1].end_of_array=false;
            sol[solSize-1].start_ind=start_ind;
            sol[solSize-1].end_ind=firstAfterInd;
            sol[solSize-1].length=firstAfterLen;
            sol[solSize-1].ascent=theAscent;
            sol[solSize-1].descent=theDescent;
            solSize++;
            lastBeforeInd=-1;
          }
          break;
        }
      } else {
        if ( breaksAfter ) {
          if ( strict ) {
          } else {
            if ( lastBeforeInd > 0 ) {
              sol=(text_chunk_solution*)realloc(sol,(solSize+1)*sizeof(text_chunk_solution));
              sol[solSize]=sol[solSize-1];
              sol[solSize-1].end_of_array=false;
              sol[solSize-1].start_ind=start_ind;
              sol[solSize-1].end_ind=lastBeforeInd;
              sol[solSize-1].length=lastBeforeLen;
              sol[solSize-1].ascent=theAscent;
              sol[solSize-1].descent=theDescent;
              lastBeforeInd=-1;
              solSize++;
            }
          }
          sol=(text_chunk_solution*)realloc(sol,(solSize+1)*sizeof(text_chunk_solution));
          sol[solSize]=sol[solSize-1];
          sol[solSize-1].end_of_array=false;
          sol[solSize-1].start_ind=start_ind;
          sol[solSize-1].end_ind=cur_ind;
          sol[solSize-1].length=lineLength;
          sol[solSize-1].ascent=theAscent;
          sol[solSize-1].descent=theDescent;
          solSize++;
        }
      }
    }
    if ( strict == false || solSize <= 1 ) {
      if ( lastBeforeInd > 0 ) {
        sol=(text_chunk_solution*)realloc(sol,(solSize+1)*sizeof(text_chunk_solution));
        sol[solSize]=sol[solSize-1];
        sol[solSize-1].end_of_array=false;
        sol[solSize-1].start_ind=start_ind;
        sol[solSize-1].end_ind=lastBeforeInd;
        sol[solSize-1].length=lastBeforeLen;
        sol[solSize-1].ascent=theAscent;
        sol[solSize-1].descent=theDescent;
        lastBeforeInd=-1;
        solSize++;
      }
    }
    return sol;
  };
  virtual void                 GlyphsAndPositions(int start_ind,int end_ind,int &nbG,int* &starts,NR::Point* &glyphs) {
    nbG=0;
    glyphs=NULL;
    starts=NULL;
    if ( end_ind < start_ind ) return;
    nbG=end_ind-start_ind+1;
    glyphs=(NR::Point*)malloc(nbG*sizeof(NR::Point));
    starts=(int*)malloc(nbG*sizeof(int));
    PangoRectangle meas;
    pango_layout_index_to_pos(pLayout,start_ind,&meas);
    int startX=meas.x;
    for (int i=start_ind;i<=end_ind;i++) {
      pango_layout_index_to_pos(pLayout,i,&meas);
      glyphs[i-start_ind]=NR::Point(((double)(meas.x-startX))/1000,0);
      starts[i-start_ind]=i;
    }
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
  NR::Point         *glyphs;
  int               *starts;
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
    typeset->theSrc=new pango_text_chunker(typeset->srcText);
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
   //   sp_object_unref(child, SP_OBJECT(typeset));
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
  steps[0].glyphs=NULL;
  steps[0].starts=NULL;

  if ( typeset->theSrc && typeset->theDst ) {
    // dumb layout: stuff 'til it's too big    
    double nAscent=0.0,nDescent=0.0;
    typeset->theSrc->InitialMetricsAt(0,nAscent,nDescent);
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
        if ( sol == NULL ) break;
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
        if ( sol[best].ascent > nAscent || sol[best].descent > nDescent ) {
          nAscent=sol[best].ascent;
          nDescent=sol[best].descent;
          free(sol);
          if ( last_step > prev_line_step ) {
            for (int i=prev_line_step+1;i<=last_step;i++) {
              if ( steps[i].glyphs ) free(steps[i].glyphs);
              if ( steps[i].starts ) free(steps[i].starts);
            }
            nb_step=prev_line_step+1;
            last_step=prev_line_step;
          }
          continue;
        }
        
        steps=(typeset_step*)realloc(steps,(nb_step+1)*sizeof(typeset_step));
        steps[nb_step].box=cur_box;
        steps[nb_step].start_ind=sol[best].start_ind;
        steps[nb_step].end_ind=sol[best].end_ind;
        steps[nb_step].nbGlyph=0;
        steps[nb_step].glyphs=NULL;
        steps[nb_step].starts=NULL;
        nb_step++;
        
        last_step=nb_step-1;
        if ( sameLine ) {
        } else {
          prev_line_step=last_step;
        }
        typeset->theSrc->InitialMetricsAt(steps[last_step].end_ind+1,nAscent,nDescent);
        
        free(sol);
      } while ( steps[last_step].end_ind+1 < ((int)strlen(typeset->srcText)) );
    }
  }
  // create offspring
  {
    SPRepr *parent = SP_OBJECT_REPR(SP_OBJECT(typeset));
    const char *style = sp_repr_attr (parent, "style");
    
    SPRepr* text_repr = sp_repr_new ("text");
    sp_repr_set_attr (text_repr, "style", style);
    sp_repr_append_child (parent, text_repr);
    sp_repr_unref (text_repr);

    for (int i=1;i<nb_step;i++) {
      SPRepr* span_repr = sp_repr_new ("tspan");
      sp_repr_set_double (span_repr, "x", steps[i].box.x_start);
      sp_repr_set_double (span_repr, "y", steps[i].box.y);
      
      int   nbG=0;
      NR::Point* glyphs=NULL;
      int*       starts=NULL;
      typeset->theSrc->GlyphsAndPositions(steps[i].start_ind,steps[i].end_ind,nbG,starts,glyphs);
      if ( glyphs && starts && nbG > 0 ) {
 /*       gchar c[32];
        gchar *s = NULL;
        
        for (int i = 0; i < nbG ; i ++) {
          g_ascii_formatd (c, sizeof (c), "%.8g", steps[i].box.x_start+glyphs[i][0]);
          if (i == 0) {
            s = g_strdup (c);
          }  else {
            s = g_strjoin (" ", s, c, NULL);
          }
        }
        sp_repr_set_attr (span_repr, "x", s);
        g_free(s);
        
        s=NULL;
        for (int i = 0; i < nbG ; i ++) {
          g_ascii_formatd (c, sizeof (c), "%.8g", steps[i].box.y+glyphs[i][1]);
          if (i == 0) {
            s = g_strdup (c);
          }  else {
            s = g_strjoin (" ", s, c, NULL);
          }
        }
        sp_repr_set_attr (span_repr, "y", s);
        g_free(s);*/
      } else {
      }
      if ( starts ) free(starts);
      if ( glyphs ) free(glyphs);
      
      int   cur_pos=steps[i].start_ind,end_pos=steps[i].end_ind+1;
      char* temp_content=(char*)malloc((end_pos+1-cur_pos)*sizeof(char));
      memcpy(temp_content,typeset->srcText+cur_pos,(end_pos-cur_pos)*sizeof(char));
      temp_content[end_pos-cur_pos]=0;
      
      SPRepr* rstr = sp_xml_document_createTextNode (sp_repr_document (parent), temp_content);
      sp_repr_append_child (span_repr, rstr);
      sp_repr_unref (rstr);
      
      free(temp_content);
      sp_repr_append_child (text_repr, span_repr);
      sp_repr_unref (span_repr);
      
      if ( steps[i].glyphs ) free(steps[i].glyphs);
      if ( steps[i].starts ) free(steps[i].starts);
    }
    free(steps);
    
    SPDesktop *desktop = SP_ACTIVE_DESKTOP;
    sp_document_done (SP_DT_DOCUMENT (desktop));
  }
}
