#define __sp_typeset_layout_utils_C__

/*
 * layout routines for the typeset element
 *
 * public domain
 *
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

#include "libnrtype/FontFactory.h"
#include "libnrtype/font-instance.h"
#include "libnrtype/TextWrapper.h"
#include "libnrtype/nr-type-pos-def.h"

#include "xml/repr.h"

#include <pango/pango.h>
#include <pango/pango-engine.h>
#include <pango/pango-context.h>

#define pango_to_ink  0.0009765625
#define ink_to_pango  1024

  
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


/*  typedef struct one_path_elem {
    Path*    theP;
    double   length;
  } one_path_elem;
  
	int                   nbPath,maxPath;
	one_path_elem         *paths;*/
  
dest_path_chunker::dest_path_chunker(void)
{
  nbPath=maxPath=0;
  paths=NULL;
}
dest_path_chunker::~dest_path_chunker(void)
{
  for (int i=0;i<nbPath;i++) delete paths[i].theP;
  if ( paths ) free(paths);
  nbPath=maxPath=0;
  paths=NULL;
}

void                  dest_path_chunker::AppendPath(Path* oPath)
{
  int         nb_sub=0;
  Path**      subps=oPath->SubPaths(nb_sub,false);

  for (int i=0;i<nb_sub;i++) {
    Path* iPath=subps[i];
    double nl=iPath->Length();  // compute the bounding box
    if ( nl < 0.001 ) {
      delete iPath;
      continue;
    }
    
    if ( nbPath >= maxPath ) {
      maxPath=2*nbPath+1;
      paths=(one_path_elem*)realloc(paths,maxPath*sizeof(one_path_elem));
    }
    int np=nbPath++;
    paths[np].theP=new Path;
    paths[np].theP->Copy(iPath);
    paths[np].length=nl;
  }
  if ( subps ) free(subps);
}

box_solution   dest_path_chunker::VeryFirst(void)
{
  box_solution res;
  res.finished=(nbPath<=0)?true:false;
  res.y=res.ascent=res.descent=res.x_start=res.x_end=0;
  res.frame_no=-1;
  return res;
}
box_solution   dest_path_chunker::NextLine(box_solution& after,double asc,double desc,double /*lead*/)
{
  if ( nbPath <= 0 || after.frame_no+1 >= nbPath ) {
    box_solution res;
    res.finished=true;
    res.y=res.ascent=res.descent=res.x_start=res.x_end=0;
    res.frame_no=-1;
    return res;
  }
  if ( after.frame_no < 0 ) {
    box_solution res;
    res.finished=false;
    res.y=0;
    res.ascent=asc;
    res.descent=desc;
    res.x_start=0;
    res.x_end=paths[0].length;
    res.frame_no=0;
    return res;
  }
  {
    box_solution res;
    res.finished=false;
    res.frame_no=after.frame_no+1;
    res.y=0;
    res.ascent=asc;
    res.descent=desc;
    res.x_start=0;
    res.x_end=paths[res.frame_no].length;
    return res;
  }
}
box_solution   dest_path_chunker::NextBox(box_solution& after,double asc,double desc,double lead,bool &stillSameLine)
{
  stillSameLine=false;
  return NextLine(after,asc,desc,lead);
}
double         dest_path_chunker::RemainingOnLine(box_solution& /*after*/)
{
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

void                  dest_shape_chunker::AppendShape(Shape* iShape,Shape* iExcl) {
  Shape* mCopy=NULL;
  if ( iExcl && iExcl->nbAr > 1 ) {
    mCopy=new Shape;
    mCopy->Booleen(iShape,iExcl,bool_op_diff);
    mCopy->CalcBBox(true);  // compute the bounding box
    if ( mCopy->rightX-mCopy->leftX < 0.001 || mCopy->bottomY-mCopy->topY < 0.001 ) {
      delete mCopy;
      return;
    }
  } else {
    iShape->CalcBBox(true);  // compute the bounding box
    if ( iShape->rightX-iShape->leftX < 0.001 || iShape->bottomY-iShape->topY < 0.001 ) {
      return;
    }
    mCopy=new Shape;
    mCopy->Copy(iShape);
  }
  
  if ( nbShape >= maxShape ) {
    maxShape=2*nbShape+1;
    shapes=(one_shape_elem*)realloc(shapes,maxShape*sizeof(one_shape_elem));
  }
  int nShape=nbShape++;
  shapes[nShape].theS=mCopy;
  shapes[nShape].theS->BeginRaster(shapes[nShape].curY,shapes[nShape].curPt,1.0);  // a bit of preparation
  shapes[nShape].theS->CalcBBox(true);
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

/*
 *
 */

pango_text_chunker::pango_text_chunker(text_wrapper* inText,char* font_family,double font_size,int flags):text_chunker(inText->utf8_text)
{  
  theText=inText;
  own_text=false;
  theFace=strdup(font_family);
  theSize=font_size;
  theAttrs=NULL;
  
  textLength=0;
  words=NULL;
  nbWord=maxWord=0;
  
  SetTextWithAttrs(inText,flags);
}
pango_text_chunker::~pango_text_chunker(void)
{
  if ( theAttrs ) pango_attr_list_unref(theAttrs);
  if ( theFace ) free(theFace);
  if ( words ) free(words);
  if ( theText && own_text ) delete theText;
}

void                 pango_text_chunker::SetText(char* /*inText*/,int /*flags*/) 
{
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
      curSol.endOfParagraph=true;
    }
    
    if ( words[cur_ind].is_white ) {
    } else {
      lastWord=curSol;
    }
    if ( words[cur_ind].is_return || cur_ind >= nbWord-1 ) {
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

void                 pango_text_chunker::GlyphsInfo(int start_ind,int end_ind,int &nbG,double &totLength)
{
  nbG=0;
  totLength=0;
  while ( start_ind <= end_ind && words[end_ind].is_white ) end_ind--;
  while ( start_ind <= end_ind && words[start_ind].is_white ) start_ind++;
  if ( end_ind < start_ind ) return;
  
  int      char_st=words[start_ind].t_first,char_en=words[end_ind].t_last;
  nbG=theText->NbLetter(char_st,char_en+1);
  totLength=theSize*(theText->glyph_text[char_en+1].x-theText->glyph_text[char_st].x);
//  printf("ginfo %i -> %i  = %f + %i\n",start_ind,end_ind,totLength,nbG);
}
void                 pango_text_chunker::GlyphsAndPositions(int start_ind,int end_ind,to_SVG_context *hungry)
{
  while ( start_ind <= end_ind && words[start_ind].is_white ) start_ind++;
  while ( start_ind <= end_ind && words[end_ind].is_white ) end_ind--;
  if ( end_ind < start_ind ) return;
  
//  printf("gpos %i -> %i \n",start_ind,end_ind);

  int      real_st=words[start_ind].t_first;
  int      real_en=words[end_ind].t_last;
  hungry->SetText(theText,-1);
  
  int      attr_st=real_st,attr_en=real_st;
	attr_st=real_en+1;
	attr_en=real_en+2;
  double     cumul=words[start_ind].x_pos;
  int        cur_ind=start_ind;
  int        char_st=words[cur_ind].t_first,char_en=words[cur_ind].t_last;
	hungry->ResetStyle();
  while ( char_st <= real_en) {
    if ( attr_st > char_st ) {
      AddDullGlyphs(hungry,cumul,char_st,(char_en < attr_st-1)?char_en:attr_st-1);        
      char_st=(char_en+1 < attr_st)?char_en+1:attr_st;
    } else {
    }
    if ( char_st > char_en ) {
//      do {
        cur_ind++;
//      } while ( cur_ind <= end_ind && words[cur_ind].is_white );
      if ( cur_ind > end_ind ) break;
      char_st=words[cur_ind].t_first;
      char_en=words[cur_ind].t_last;
    }
  }
}

void                         pango_text_chunker::SetTextWithAttrs(text_wrapper* inText,int /*flags*/) 
{
  theText=inText;
  textLength=0;
  nbWord=0;
  if ( theText == NULL || theText->uni32_length <= 0 ) return;
	
  {
		font_instance* def_font=(font_factory::Default())->FaceFromDescr(theFace);
		if ( def_font == NULL ) return;
		theText->SetDefaultFont(def_font);
		def_font->Unref();
	}
	theText->DoLayout();
	theText->AddDxDy();
	theText->MeasureBoxes();
	
	if ( theText->nbBox >= maxWord ) {
		maxWord=theText->nbBox+1;
		words=(elem_box*)realloc(words,maxWord*sizeof(elem_box));
	}
	nbWord=0;
	for (int i=0;i<theText->nbBox;i++) {
		words[i].t_first=theText->boxes[i].g_st;
		words[i].t_last=theText->boxes[i].g_en-1;
    words[i].x_pos=theSize*theText->glyph_text[theText->boxes[i].g_st].x;
		words[i].y_ascent=theSize*theText->boxes[i].ascent;
		words[i].y_descent=theSize*theText->boxes[i].descent;
		words[i].x_length=theSize*theText->boxes[i].width;
		int s_pos=theText->glyph_text[words[i].t_first].uni_st;
		words[i].is_return=theText->glyph_text[words[i].t_last+1].para_start;
    words[i].is_white=words[i].is_return | (g_unichar_isspace(theText->uni32_text[s_pos]));
		words[i].end_of_word=theText->boxes[i].word_end;
//		printf("word %i: %i -> %i  l=%f  w=%i r=%i e=%i\n",i,words[i].t_first,words[i].t_last,words[i].x_length,(words[i].is_white)?1:0
//					 ,(words[i].is_return)?1:0,(words[i].end_of_word)?1:0);
	}
	nbWord=theText->nbBox;
}
void                         pango_text_chunker::AddDullGlyphs(to_SVG_context *hungry,double &cumul,int c_st,int c_en)
{
//  hungry->ResetStyle();
  hungry->AddFontFamily(theFace);
  hungry->AddFontSize(theSize);
  double startX=cumul;
	int    nbLetter=0;
  for (int i=c_st;i<=c_en;i++) {
		if ( i > c_st && theText->glyph_text[i].char_start ) nbLetter++;
    NR::Point at(theSize*theText->glyph_text[i].x-startX,theSize*theText->glyph_text[i].y);
    hungry->AddGlyph(nbLetter,theText->glyph_text[i].uni_st,theText->glyph_text[i].uni_en,at,theSize*(theText->glyph_text[i+1].x-theText->glyph_text[i].x));
	}
}

/*
 *
 */
box_to_SVG_context::box_to_SVG_context(SPRepr* in_repr,double y,double x_start,double x_end):to_SVG_context(in_repr)
{
  dxs=dys=xs=ys=NULL;
  letter_spacing=0;
  text=NULL;
  orig_st=st=-1;
  en=-2;
  
  text_repr=in_repr;
  if ( text_repr ) sp_repr_ref(text_repr);
  style_repr=NULL;
  span_repr=NULL;
  
  box_y=y;
  box_start=x_start;
  box_end=x_end;
  
  cur_x=cur_start=box_start;
  cur_y=cur_by=box_y;
  cur_next=cur_x;
  cur_spacing=0;
}
box_to_SVG_context::~box_to_SVG_context(void)
{
  Finalize();
  
  if ( dxs ) free(dxs);
  if ( dys ) free(dys);
  if ( xs ) free(xs);
  if ( ys ) free(ys);
  if ( text_repr ) sp_repr_unref(text_repr);
  if ( span_repr ) sp_repr_unref(span_repr);
  if ( style_repr ) sp_repr_css_attr_unref(style_repr);
}

void            box_to_SVG_context::Finalize(void)
{
  Flush();
}

void            box_to_SVG_context::ResetStyle(void)
{
  Flush();
  if ( style_repr ) {
    sp_repr_css_attr_unref(style_repr);
    style_repr=NULL;
  }
}
void            box_to_SVG_context::AddFontFamily(char* n_fam)
{
  if ( style_repr == NULL ) {
    style_repr=sp_repr_css_attr_new();
  }
  sp_repr_set_attr ((SPRepr*)style_repr,"font-family", n_fam);
}
void            box_to_SVG_context::AddFontSize(double n_siz)
{
  if ( style_repr == NULL ) {
    style_repr=sp_repr_css_attr_new();
  }
  sp_repr_set_double ((SPRepr*)style_repr,"font-size", n_siz);
}
void            box_to_SVG_context::AddFontVariant(int n_var)
{
  if ( style_repr == NULL ) {
    style_repr=sp_repr_css_attr_new();
  }
  if ( n_var == PANGO_STYLE_OBLIQUE ) {
    sp_repr_set_attr ((SPRepr*)style_repr,"font-style", "oblique");
  } else if  ( n_var == PANGO_STYLE_ITALIC ) {
    sp_repr_set_attr ((SPRepr*)style_repr,"font-style", "italic");
  }
}
void            box_to_SVG_context::AddFontStretch(int n_str)
{
  if ( style_repr == NULL ) {
    style_repr=sp_repr_css_attr_new();
  }
  if ( n_str == PANGO_STRETCH_ULTRA_CONDENSED ) {
    sp_repr_set_attr ((SPRepr*)style_repr,"font-stretch", "ultra-condensed");
  } else if ( n_str == PANGO_STRETCH_EXTRA_CONDENSED ) {
    sp_repr_set_attr ((SPRepr*)style_repr,"font-stretch", "extra-condensed");
  } else if ( n_str == PANGO_STRETCH_CONDENSED ) {
    sp_repr_set_attr ((SPRepr*)style_repr,"font-stretch", "condensed");
  } else if ( n_str == PANGO_STRETCH_SEMI_CONDENSED ) {
    sp_repr_set_attr ((SPRepr*)style_repr,"font-stretch", "semi-condensed");
  } else if ( n_str == PANGO_STRETCH_NORMAL ) {
    sp_repr_set_attr ((SPRepr*)style_repr,"font-stretch", "normal");
  } else if ( n_str == PANGO_STRETCH_SEMI_EXPANDED ) {
    sp_repr_set_attr ((SPRepr*)style_repr,"font-stretch", "semi-expanded");
  } else if ( n_str == PANGO_STRETCH_EXPANDED ) {
    sp_repr_set_attr ((SPRepr*)style_repr,"font-stretch", "expanded");
  } else if ( n_str == PANGO_STRETCH_EXTRA_EXPANDED ) {
    sp_repr_set_attr ((SPRepr*)style_repr,"font-stretch", "extra-expanded");
  } else if ( n_str == PANGO_STRETCH_ULTRA_EXPANDED ) {
    sp_repr_set_attr ((SPRepr*)style_repr,"font-stretch", "ultra-expanded");
  }
}
void            box_to_SVG_context::AddFontWeight(int n_wei)
{
  if ( style_repr == NULL ) {
    style_repr=sp_repr_css_attr_new();
  }
  sp_repr_set_int ((SPRepr*)style_repr,"font-weight", n_wei);
}
void            box_to_SVG_context::AddLetterSpacing(double n_spc)
{
  if ( style_repr == NULL ) {
    style_repr=sp_repr_css_attr_new();
  }
  sp_repr_set_double ((SPRepr*)style_repr,"letter-spacing", n_spc);
}

void            box_to_SVG_context::SetText(text_wrapper*  n_txt,int /*n_len*/)
{
  text=n_txt;
  orig_st=st=-1;
  en=-2;
}
void            box_to_SVG_context::SetLetterSpacing(double n_spc)
{
  letter_spacing=n_spc;
  cur_spacing=0;
}

void            box_to_SVG_context::SetY(int a_g,int /*f_c*/,int /*l_c*/,double to)
{
	cur_y=cur_by=to;
}
void            box_to_SVG_context::Flush(void)
{
  if ( st < en && text_repr ) {
    SPRepr* parent=sp_repr_parent(text_repr);
    span_repr = sp_repr_new ("tspan");

		AddLetterSpacing(letter_spacing);
//    if ( dxs == NULL && fabs(cur_spacing) > 0 ) {
//      AddLetterSpacing(cur_spacing);
//    }
    if ( style_repr ) {
      sp_repr_css_set (span_repr,style_repr, "style");
      sp_repr_css_attr_unref(style_repr);
      style_repr=NULL;
    }
    
    sp_repr_set_double (span_repr, "x", cur_start);
    sp_repr_set_double (span_repr, "y", cur_by);
      
    if ( text->kern_x ) {
      gchar c[64];
      gchar *s = NULL;
      for (int j = st; j <= en ; j ++) {
        c[0]=0;
        g_ascii_formatd (c, sizeof (c), "%.8g", text->kern_x[j]);
        if (s == NULL) {
          s = g_strdup (c);
        }  else {
          s = g_strjoin (" ", s, c, NULL);
        }
      }
      sp_repr_set_attr (span_repr, "dx", s);
      g_free(s);
      free(dxs);
      dxs=NULL;
    }
    if ( text->kern_y ) {
      gchar c[64];
      gchar *s = NULL;
      for (int j = st; j <= en ; j ++) {
        c[0]=0;
        g_ascii_formatd (c, sizeof (c), "%.8g", text->kern_y[j]);
        if (s == NULL) {
          s = g_strdup (c);
        }  else {
          s = g_strjoin (" ", s, c, NULL);
        }
      }
      sp_repr_set_attr (span_repr, "dy", s);
      g_free(s);
      free(dys);
      dys=NULL;
    }
    if ( st < 0 ) st=0;
		if ( st > text->uni32_length ) st=text->uni32_length;
    if ( en < 0 ) en=0;
		if ( en > text->uni32_length ) en=text->uni32_length;
		int  f_u=text->utf8_codepoint[st],l_u=text->utf8_codepoint[en];
    char  savC=text->utf8_text[l_u];
    text->utf8_text[l_u]=0;
    SPRepr* rstr = sp_xml_document_createTextNode (sp_repr_document (parent), text->utf8_text+f_u);
    sp_repr_append_child (span_repr, rstr);
    sp_repr_unref (rstr);
    text->utf8_text[l_u]=savC;
    
    sp_repr_append_child (text_repr, span_repr);
    sp_repr_unref (span_repr);
    span_repr=NULL; // unref'ing already done
  }
  
  if ( dxs ) free(dxs);
  if ( dys ) free(dys);
  if ( xs ) free(xs);
  if ( ys ) free(ys);
  dxs=dys=xs=ys=NULL;
  st=-1;
  en=-2;
  
  if ( style_repr ) sp_repr_css_attr_unref(style_repr);
  style_repr=NULL;
  if ( span_repr ) sp_repr_unref(span_repr);
  span_repr=NULL;
}
void            box_to_SVG_context::AddGlyph(int a_g,int f_c,int l_c,const NR::Point &oat,double advance)
{
  if ( st < 0 || f_c < st ) st=f_c;
  if ( orig_st < 0 || f_c < orig_st ) orig_st=f_c;
  if ( en < 0 || l_c > en ) en=l_c;
}

  
path_to_SVG_context::path_to_SVG_context(SPRepr* in_repr,Path *iPath,double iLength,double iDelta):to_SVG_context(in_repr)
{
  thePath=iPath;
  if ( thePath ) thePath->ConvertWithBackData(1.0);
  path_length=iLength;
  path_delta=iDelta;
  
  cur_x=0;
  
  letter_spacing=0;
  text=NULL;
  st=-1;
  en=-2;
  
  text_repr=in_repr;
  if ( text_repr ) sp_repr_ref(text_repr);
  style_repr=NULL;
  span_repr=NULL;
}
path_to_SVG_context::~path_to_SVG_context(void)
{
  Finalize();
  
  if ( text_repr ) sp_repr_unref(text_repr);
  if ( span_repr ) sp_repr_unref(span_repr);
  if ( style_repr ) sp_repr_css_attr_unref(style_repr);
}

void            path_to_SVG_context::Finalize(void)
{
}

void            path_to_SVG_context::ResetStyle(void)
{
  if ( style_repr ) {
    sp_repr_css_attr_unref(style_repr);
    style_repr=NULL;
  }
}
void            path_to_SVG_context::AddFontFamily(char* n_fam)
{
  if ( style_repr == NULL ) {
    style_repr=sp_repr_css_attr_new();
  }
  sp_repr_set_attr ((SPRepr*)style_repr,"font-family", n_fam);
}
void            path_to_SVG_context::AddFontSize(double n_siz)
{
  if ( style_repr == NULL ) {
    style_repr=sp_repr_css_attr_new();
  }
  sp_repr_set_double ((SPRepr*)style_repr,"font-size", n_siz);
}
void            path_to_SVG_context::AddFontVariant(int n_var)
{
  if ( style_repr == NULL ) {
    style_repr=sp_repr_css_attr_new();
  }
  if ( n_var == PANGO_STYLE_OBLIQUE ) {
    sp_repr_set_attr ((SPRepr*)style_repr,"font-style", "oblique");
  } else if  ( n_var == PANGO_STYLE_ITALIC ) {
    sp_repr_set_attr ((SPRepr*)style_repr,"font-style", "italic");
  }
}
void            path_to_SVG_context::AddFontStretch(int n_str)
{
  if ( style_repr == NULL ) {
    style_repr=sp_repr_css_attr_new();
  }
  if ( n_str == PANGO_STRETCH_ULTRA_CONDENSED ) {
    sp_repr_set_attr ((SPRepr*)style_repr,"font-stretch", "ultra-condensed");
  } else if ( n_str == PANGO_STRETCH_EXTRA_CONDENSED ) {
    sp_repr_set_attr ((SPRepr*)style_repr,"font-stretch", "extra-condensed");
  } else if ( n_str == PANGO_STRETCH_CONDENSED ) {
    sp_repr_set_attr ((SPRepr*)style_repr,"font-stretch", "condensed");
  } else if ( n_str == PANGO_STRETCH_SEMI_CONDENSED ) {
    sp_repr_set_attr ((SPRepr*)style_repr,"font-stretch", "semi-condensed");
  } else if ( n_str == PANGO_STRETCH_NORMAL ) {
    sp_repr_set_attr ((SPRepr*)style_repr,"font-stretch", "normal");
  } else if ( n_str == PANGO_STRETCH_SEMI_EXPANDED ) {
    sp_repr_set_attr ((SPRepr*)style_repr,"font-stretch", "semi-expanded");
  } else if ( n_str == PANGO_STRETCH_EXPANDED ) {
    sp_repr_set_attr ((SPRepr*)style_repr,"font-stretch", "expanded");
  } else if ( n_str == PANGO_STRETCH_EXTRA_EXPANDED ) {
    sp_repr_set_attr ((SPRepr*)style_repr,"font-stretch", "extra-expanded");
  } else if ( n_str == PANGO_STRETCH_ULTRA_EXPANDED ) {
    sp_repr_set_attr ((SPRepr*)style_repr,"font-stretch", "ultra-expanded");
  }
}
void            path_to_SVG_context::AddFontWeight(int n_wei)
{
  if ( style_repr == NULL ) {
    style_repr=sp_repr_css_attr_new();
  }
  sp_repr_set_int ((SPRepr*)style_repr,"font-weight", n_wei);
}
void            path_to_SVG_context::AddLetterSpacing(double n_spc)
{
  if ( style_repr == NULL ) {
    style_repr=sp_repr_css_attr_new();
  }
  sp_repr_set_double ((SPRepr*)style_repr,"letter-spacing", n_spc);
}

void            path_to_SVG_context::SetText(text_wrapper*  n_txt,int /*n_len*/)
{
  text=n_txt;
  st=-1;
  en=-2;
}
void            path_to_SVG_context::SetLetterSpacing(double n_spc)
{
  letter_spacing=n_spc;
}

void            path_to_SVG_context::AddGlyph(int a_g,int f_c,int l_c,const NR::Point &oat,double advance)
{
  if ( l_c <= f_c ) return;
  
  NR::Point at=oat;
	at[0]+=((double)a_g)*letter_spacing;
	
  int              nbp=0;
  double           mid_glyph=at[0]+0.5*advance;
  mid_glyph+=path_delta;
  Path::cut_position*    cup=thePath->CurvilignToPosition(1,&mid_glyph,nbp);
  
  if ( cup ) {
    NR::Point        ts_pos,ts_tgt,ts_nor;
    thePath->PointAndTangentAt(cup[0].piece,cup[0].t,ts_pos,ts_tgt);

    ts_nor=ts_tgt.cw();
    ts_pos-=0.5*advance*ts_tgt;
    ts_pos+=at[1]*ts_nor;
    double ang=0;
    if ( ts_tgt[0] >= 1 ) {
      ang=0;
    } else if ( ts_tgt[0] <= -1 ) {
      ang=M_PI;
    } else {
      ang=acos(ts_tgt[0]);
      if ( ts_tgt[1] < 0 ) ang=-ang;
    }
    free(cup);

    SPRepr* parent=sp_repr_parent(text_repr);
    span_repr = sp_repr_new ("tspan");
  
    if ( style_repr ) {
      sp_repr_css_set (span_repr,style_repr, "style");
    }
  
    sp_repr_set_double (span_repr, "rotate", 180*ang/M_PI);
    sp_repr_set_double (span_repr, "x", ts_pos[0]);
    sp_repr_set_double (span_repr, "y", ts_pos[1]);
    
    if ( f_c < 0 ) st=0;
		if ( f_c > text->uni32_length ) f_c=text->uni32_length;
    if ( l_c < 0 ) l_c=0;
		if ( l_c > text->uni32_length ) l_c=text->uni32_length;
		
		int f_u=text->utf8_codepoint[f_c],l_u=text->utf8_codepoint[l_c];
    char  savC=text->utf8_text[l_u];
    text->utf8_text[l_u]=0;
    SPRepr* rstr = sp_xml_document_createTextNode (sp_repr_document (parent), text->utf8_text+f_u);
    sp_repr_append_child (span_repr, rstr);
    sp_repr_unref (rstr);
    text->utf8_text[l_u]=savC;
    
    sp_repr_append_child (text_repr, span_repr);
    sp_repr_unref (span_repr);
    span_repr=NULL; // unref'ing already done
  }  
  
  cur_x=at[0];
  if ( st < 0 || f_c < st ) st=f_c;
  if ( orig_st < 0 || f_c < orig_st ) orig_st=f_c;
  if ( en < 0 || l_c > en ) en=l_c;
  
}



