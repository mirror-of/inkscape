#define __sp_typeset_layout_utils_C__

/*
 * layout routines for the typeset element
 */

#include <config.h>
#include <string.h>

#include "sp-typeset.h"
#include "sp-typeset-utils.h"

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

  
dest_col_chunker::dest_col_chunker(double iWidth)
{
  colWidth=(iWidth>0)?iWidth:0;
}
dest_col_chunker::~dest_col_chunker(void)
{
}

box_solution   dest_col_chunker::VeryFirst(void)
{
  box_solution res;
  res.finished=false;
  res.y=res.ascent=res.descent=res.x_start=res.x_end=0;
  res.frame_no=-1;
  return res;
}
box_solution   dest_col_chunker::NextLine(box_solution& after,double asc,double desc,double lead)
{
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
}
box_solution   dest_col_chunker::NextBox(box_solution& after,double asc,double desc,double lead,bool &stillSameLine)
{
  stillSameLine=false;
  return NextLine(after,asc,desc,lead);
}
double         dest_col_chunker::RemainingOnLine(box_solution& /*after*/)
{
  return 0;
}


dest_box_chunker::dest_box_chunker(void) 
{
  boxes=NULL;
  nbBoxes=0;
}
dest_box_chunker::~dest_box_chunker(void) 
{
  if ( boxes ) free(boxes);
}

void           dest_box_chunker::AppendBox(const NR::Rect &iBox)
{
  if ( iBox.isEmpty() ) {
  } else {
    boxes=(NR::Rect*)realloc(boxes,(nbBoxes+1)*sizeof(NR::Rect));
    boxes[nbBoxes]=iBox;
    nbBoxes++;
  }
}

box_solution   dest_box_chunker::VeryFirst(void)
{
  box_solution res;
  res.finished=(nbBoxes<=0)?true:false;
  res.y=res.ascent=res.descent=res.x_start=res.x_end=0;
  res.frame_no=-1;
  return res;
}
box_solution   dest_box_chunker::NextLine(box_solution& after,double asc,double desc,double lead)
{
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
}
box_solution   dest_box_chunker::NextBox(box_solution& after,double asc,double desc,double lead,bool &stillSameLine)
{
  stillSameLine=false;
  return NextLine(after,asc,desc,lead);
}
double         dest_box_chunker::RemainingOnLine(box_solution& /*after*/) {
  return 0;
}


dest_shape_chunker::dest_shape_chunker(void) 
{
  nbShape=maxShape=0;
  shapes=NULL;
  tempLine=new FloatLigne();
  tempLine2=new FloatLigne();
  lastDate=0;
  maxCache=64;
  nbCache=0;
  caches=(cached_line*)malloc(maxCache*sizeof(cached_line));
}
dest_shape_chunker::~dest_shape_chunker(void)
{
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
}

box_solution   dest_shape_chunker::VeryFirst(void) 
{
  box_solution res;
  res.finished=(nbShape<=0)?true:false;
  res.y=res.ascent=res.descent=res.x_start=res.x_end=0;
  res.frame_no=-1;
  return res;
}
double         dest_shape_chunker::RemainingOnLine(box_solution& after) 
{
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
}

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
  
  if ( fabs(bottom-top) < 0.001 ) {
    tempLine->Reset();
    return;
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


pango_text_chunker::pango_text_chunker(char* inText,char* font_family,double font_size,int flags,bool isMarked):text_chunker(inText)
{
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
}
pango_text_chunker::~pango_text_chunker(void)
{
  if ( theAttrs ) pango_attr_list_unref(theAttrs);
  if ( theFace ) free(theFace);
  if ( words ) free(words);
  if ( charas ) free(charas);
  if ( theText && own_text ) free(theText);
}

void                 pango_text_chunker::SetText(char* inText,int flags) 
{
  SetTextWithAttrs(inText,NULL,flags);
}
void                 pango_text_chunker::ChangeText(int /*startPos*/,int /*endPos*/,char* inText,int flags)
{
  SetText(inText,flags);
}
int                  pango_text_chunker::MaxIndex(void) 
{
  return nbWord;
}

void                 pango_text_chunker::InitialMetricsAt(int startPos,double &ascent,double &descent) 
{
  if ( startPos < 0 ) startPos=0;
  if ( startPos >= nbWord ) {
    ascent=0;
    descent=0;
    return;
  }
  ascent=words[startPos].y_ascent;
  descent=words[startPos].y_descent;
}


text_chunk_solution* pango_text_chunker::StuffThatBox(int start_ind,double minLength,double /*nominalLength*/,double maxLength,bool strict) 
{
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
      if ( words[cur_ind].end_of_word ) { // already chunked in words
        if ( words[cur_ind].is_return == false && words[cur_ind+1].is_return ) {
          // return goes with the previous word
        } else {
          breaksAfter=true;
        }
      }
    } else {
      // last character of the text
      breaksAfter=true;
    }
    
    if ( words[cur_ind].is_white ) {
    } else {
      lastWord=curSol;
    }
    if ( words[cur_ind].is_return ) {
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
              if ( sol[solSize-2].endOfParagraph ) return sol;
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
            if ( sol[solSize-2].endOfParagraph ) return sol;
          }
          if ( lastWord.end_ind >= lastWord.start_ind ) {
            sol=(text_chunk_solution*)realloc(sol,(solSize+1)*sizeof(text_chunk_solution));
            sol[solSize]=sol[solSize-1];
            sol[solSize-1]=lastWord;
            lastWord.end_ind=lastWord.start_ind-1;
            solSize++;
            if ( sol[solSize-2].endOfParagraph ) return sol;
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
            if ( sol[solSize-2].endOfParagraph ) return sol;
          }
        }
        if ( lastWord.end_ind >= lastWord.start_ind ) {
          sol=(text_chunk_solution*)realloc(sol,(solSize+1)*sizeof(text_chunk_solution));
          sol[solSize]=sol[solSize-1];
          sol[solSize-1]=lastWord;
          lastWord.end_ind=lastWord.start_ind-1;
          solSize++;
          if ( sol[solSize-2].endOfParagraph ) return sol;
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
      if ( sol[solSize-2].endOfParagraph ) return sol;
    }
  }
  return sol;
}

void                 pango_text_chunker::GlyphsAndPositions(int start_ind,int end_ind,int &nbS,glyphs_for_span* &spans)
{
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
void                         pango_text_chunker::AddBox(int st,int en,bool whit,bool retu,PangoGlyphString* from,int offset,PangoFont* theFont,NR::Point &cumul)
{
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
  words[nbWord].end_of_word=true;
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
void                         pango_text_chunker::SetTextWithMarkup(char* inText,int flags)
{
  char*            resText=NULL;
  if ( inText == NULL || strlen(inText) <= 0 ) return;
  if ( pango_parse_markup(inText,strlen(inText),0,&theAttrs,&resText,NULL,NULL) ) {
    SetTextWithAttrs(resText,theAttrs,flags);
    own_text=true;
  } else {
    SetText(NULL,flags);
  }
}
void                         pango_text_chunker::SetTextWithAttrs(char* inText,PangoAttrList* resAttr,int flags) 
{
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
      if ( pAttrs[t_pos-1].is_word_end ) {
      } else {
        words[nbWord-1].end_of_word=false;
      }
      l_pos=t_pos;
    }
  }
  
  //  printf("%i mots\n",nbWord);
  //  for (int i=0;i<nbWord;i++) {
  //    printf("%i->%i  x=%f l=%f a=%f d=%f w=%i r=%i ew=%i\n",words[i].t_first,words[i].t_last,words[i].x_pos,words[i].x_length,words[i].y_ascent,words[i].y_descent
  //           ,words[i].is_white,words[i].is_return,words[i].end_of_word);
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
void                         pango_text_chunker::AddDullGlyphs(glyphs_for_span &dest,double &cumul,int c_st,int c_en)
{
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

