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

#include "libnrtype/nr-type-directory.h"
#include "libnrtype/nr-typeface.h"
#include "libnrtype/nr-font.h"
#include "libnrtype/nr-type-pos-def.h"

#include "xml/repr.h"

#include <pango/pango.h>
#include <pango/pango-engine.h>
#include <pango/pango-context.h>
//#include <pango/pangoxft.h>
#ifdef WITH_XFT
#include <pango/pangoft2.h>
#endif
#include <gdk/gdk.h>

#define pango_to_ink  0.0009765625
#define ink_to_pango  1024


  
text_with_info::text_with_info(char* inText)
{
  utf8_text=NULL;
  last_st=0;
  last_len=0;
  utf8_length=0;
  unicode_length=0;
  uni_text=NULL;
  char_offset=NULL;   
  kern_x=NULL;
  kern_y=NULL;
  stripped=NULL;
  markupType=0;
  markup=NULL;
  SetStdText(inText);
}
text_with_info::~text_with_info(void)
{
  Kill();
}
char*        text_with_info::UTF8Text(void)
{
  if ( markupType == 1 ) return stripped;
  return utf8_text;
}
gunichar*    text_with_info::UnicodeText(void)
{
  return uni_text;
}
int          text_with_info::UTF8Length(void)
{
  if ( markupType == 1 ) return ((stripped)?strlen(stripped):0);
  return utf8_length;
}
int          text_with_info::UnicodeLength(void)
{
  return unicode_length;
}


void         text_with_info::Kill(void)
{
  if ( utf8_text ) free(utf8_text);
  if ( uni_text ) free(uni_text);
  if ( char_offset ) free(char_offset);   
  if ( kern_x ) free(kern_x);
  if ( kern_y ) free(kern_y);
  if ( stripped ) free(stripped);
  switch ( markupType ) {
    case 1:
      if ( markup ) pango_attr_list_unref((PangoAttrList*)markup);
      break;
    default:
      break;
  }
  utf8_text=NULL;
  last_st=0;
  last_len=0;
  utf8_length=0;
  unicode_length=0;
  uni_text=NULL;
  char_offset=NULL;   
  kern_x=NULL;
  kern_y=NULL;
  stripped=NULL;
  markupType=0;
  markup=NULL;
}
void         text_with_info::SetStdText(char* inText)
{
  if ( inText == NULL ) {
    Kill();
    return;
  }
  {
    const gchar* end_val=NULL;
    if ( g_utf8_validate(inText,-1,&end_val) ) {
    } else {
      return; // malformed utf8
    }
  }
  AppendStdText(inText);
}
void         text_with_info::AppendStdText(char* inText)
{
  if ( inText == NULL ) return;
  if ( markupType != 0 ) Kill();
  markupType=0;
  
  last_st=unicode_length;
  last_len=0;
  {
    const gchar* end_val=NULL;
    if ( g_utf8_validate(inText,-1,&end_val) ) {
    } else {
      return; // malformed utf8
    }
  }
  {
    int   len=strlen(inText);
    if ( len <= 0 ) return;
    int at=utf8_length;
    utf8_text=(char*)realloc(utf8_text,(utf8_length+len+1)*sizeof(char));
    utf8_length+=len;
    
    memcpy(utf8_text+at,inText,len*sizeof(char));
    utf8_text[utf8_length]=0; // NULL-terminated
  }
  {
    int   len=0;
    char* pos=inText;
    while ( pos && *pos != 0 ) {
      len++;
      pos=g_utf8_next_char(pos);
    }
    if ( len <= 0 ) return;
    int  at=unicode_length;
    
    uni_text=(gunichar*)realloc(uni_text,(unicode_length+len+1)*sizeof(gunichar));
    char_offset=(int*)realloc(char_offset,(unicode_length+len+1)*sizeof(int));
    if ( kern_x ) kern_x=(double*)realloc(kern_x,(unicode_length+len)*sizeof(double));
    if ( kern_y ) kern_y=(double*)realloc(kern_y,(unicode_length+len)*sizeof(double));
    unicode_length+=len;
    uni_text[unicode_length]=0;
    
    pos=inText;
    int add=0;
    int  n_of=( at>0 )? n_of=char_offset[at] : 0;
    while ( add < len ) {
      uni_text[at+add]=g_utf8_get_char(pos);
      char_offset[at+add]=n_of+((int)pos)-((int)inText);
      
      add++;
      pos=g_utf8_next_char(pos);
    }
    char_offset[unicode_length]=utf8_length;
    if ( kern_x ) {
      for (int i=0;i<len;i++) kern_x[at+i]=0;
    }
    if ( kern_y ) {
      for (int i=0;i<len;i++) kern_y[at+i]=0;
    }
    last_st=at;
    last_len=len;
  }
}

void         text_with_info::SetPangoText(char* inText)
{
  if ( inText == NULL ) {
    Kill();
    return;
  }
  {
    const gchar* end_val=NULL;
    if ( g_utf8_validate(inText,-1,&end_val) ) {
    } else {
      return; // malformed utf8
    }
  }
  AppendPangoText(inText);
}
void         text_with_info::AppendPangoText(char* inText)
{
  if ( inText == NULL ) return;
  if ( markupType != 1 ) Kill();
  markupType=1;
  
  {
    const gchar* end_val=NULL;
    if ( g_utf8_validate(inText,-1,&end_val) ) {
    } else {
      return; // malformed utf8
    }
  }
  {
    int   len=strlen(inText);
    if ( len <= 0 ) return;
    int at=utf8_length;
    utf8_text=(char*)realloc(utf8_text,(utf8_length+len+1)*sizeof(char));
    utf8_length+=len;
    
    memcpy(utf8_text+at,inText,len*sizeof(char));
    utf8_text[utf8_length]=0; // NULL-terminated
  }
  {
    PangoAttrList* inAttrs=NULL;
    char*          resStripped=NULL;
    if ( pango_parse_markup(inText,strlen(inText),0,&inAttrs,&resStripped,NULL,NULL) ) {
      int   slen=strlen(resStripped);
      if ( slen <= 0 ) {
        if ( resStripped ) free(resStripped);
        return;
      }
      int   olen=((stripped)?strlen(stripped):0);
      stripped=(char*)realloc(stripped,(olen+slen+1)*sizeof(char));
      memcpy(stripped+olen,resStripped,slen*sizeof(char));
      stripped[olen+slen]=0; // NULL-terminated

      {
        int   len=0;
        char* pos=resStripped;
        while ( pos && *pos != 0 ) {
          len++;
          pos=g_utf8_next_char(pos);
        }
        if ( len <= 0 ) {
          if ( resStripped ) free(resStripped);
          return;
        }
        int  at=unicode_length;
        
        uni_text=(gunichar*)realloc(uni_text,(unicode_length+len+1)*sizeof(gunichar));
        char_offset=(int*)realloc(char_offset,(unicode_length+len+1)*sizeof(int));
        if ( kern_x ) kern_x=(double*)realloc(kern_x,(unicode_length+len)*sizeof(double));
        if ( kern_y ) kern_y=(double*)realloc(kern_y,(unicode_length+len)*sizeof(double));
        unicode_length+=len;
        uni_text[unicode_length]=0;
       
        pos=resStripped;
        int add=0;
        int  n_of=( at > 0 )? n_of=char_offset[at] : 0;
        while ( add < len ) {
          uni_text[at+add]=g_utf8_get_char(pos);
          char_offset[at+add]=n_of+((int)pos)-((int)resStripped);
          
          add++;
          pos=g_utf8_next_char(pos);
        }
        char_offset[unicode_length]=olen+slen;
        if ( kern_x ) {
          for (int i=0;i<len;i++) kern_x[at+i]=0;
        }
        if ( kern_y ) {
          for (int i=0;i<len;i++) kern_y[at+i]=0;
        }
        last_st=at;
        last_len=len;
      }
      if ( resStripped ) free(resStripped);
      if ( markup == NULL ) markup=pango_attr_list_new();
      pango_attr_list_splice((PangoAttrList *)markup,inAttrs,last_st,0);
      pango_attr_list_unref(inAttrs);
    } else {
      return; // bad markup
    }
  }
}

void         text_with_info::KernXForLastAddition(double *i_kern_x,int i_len)
{
  if ( last_st < 0 || last_st >= unicode_length || last_len <= 0 ) return;
  if ( kern_x == NULL ) {
    kern_x=(double*)malloc(unicode_length*sizeof(double));
    for (int i=0;i<unicode_length;i++) kern_x[i]=0;
  }
  if ( last_len > unicode_length-last_st ) last_len=unicode_length-last_st;
  if ( i_len > last_len ) i_len=last_len;
  for (int i=0;i<i_len;i++) kern_x[last_st+i]=i_kern_x[i];
}
void         text_with_info::KernYForLastAddition(double *i_kern_y,int i_len)
{
  if ( last_st < 0 || last_st >= unicode_length || last_len <= 0 ) return;
  if ( kern_y == NULL ) {
    kern_y=(double*)malloc(unicode_length*sizeof(double));
    for (int i=0;i<unicode_length;i++) kern_y[i]=0;
  }
  if ( last_len > unicode_length-last_st ) last_len=unicode_length-last_st;
  if ( i_len > last_len ) i_len=last_len;
  for (int i=0;i<i_len;i++) kern_y[last_st+i]=i_kern_y[i];
}







  
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

#ifdef WITH_XFT
PangoFontMap  *theFontMap=NULL;
#endif

pango_text_chunker::pango_text_chunker(text_with_info* inText,char* font_family,double font_size,int flags):text_chunker(inText->UTF8Text())
{
#ifdef WITH_XFT
  if ( theFontMap == NULL ) {
    theFontMap=pango_ft2_font_map_new();
  }
#endif
  
  theText=inText;
  own_text=false;
  theFace=strdup(font_family);
  theSize=font_size;
  theAttrs=NULL;
  
  textLength=0;
  words=NULL;
  nbWord=maxWord=0;
  charas=NULL;
  
//  printf("dest chunker gets: %s\n");
  
  SetTextWithAttrs(inText,flags);
}
pango_text_chunker::~pango_text_chunker(void)
{
  if ( theAttrs ) pango_attr_list_unref(theAttrs);
  if ( theFace ) free(theFace);
  if ( words ) free(words);
  if ( charas ) free(charas);
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
  nbG=charas[char_en].code_point-charas[char_st].code_point+1;
  totLength=charas[char_en].x_pos+charas[char_en].x_adv-charas[char_st].x_pos;
//  printf("%i -> %i  = %f\n",start_ind,end_ind,totLength);
}
void                 pango_text_chunker::GlyphsAndPositions(int start_ind,int end_ind,to_SVG_context *hungry)
{
  while ( start_ind <= end_ind && words[start_ind].is_white ) start_ind++;
  while ( start_ind <= end_ind && words[end_ind].is_white ) end_ind--;
  if ( end_ind < start_ind ) return;
  
  int      char_st=words[start_ind].t_first,char_en=words[end_ind].t_last;
  hungry->SetText(theText->UTF8Text(),theText->UTF8Length());
  
  PangoAttrIterator* theIt=NULL;
  if ( theText->markupType == 1 ) theIt=pango_attr_list_get_iterator ((PangoAttrList*)theText->markup);
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
  double     cumul=words[start_ind].x_pos;
  while ( char_st <= char_en) {
    if ( attr_st > char_st ) {
      AddDullGlyphs(hungry,cumul,char_st,(char_en < attr_st-1)?char_en:attr_st-1);        
      char_st=(char_en+1 < attr_st)?char_en+1:attr_st;
    } else {
      if ( attr_en-1 <= char_en ) {
        AddAttributedGlyphs(hungry,cumul,char_st,attr_en-1,theIt);
        char_st=attr_en;
        if ( theIt ) {
          if ( pango_attr_iterator_next(theIt) == false ) break;
          pango_attr_iterator_range(theIt,&attr_st,&attr_en);
        } else {
        }
      } else {
        AddAttributedGlyphs(hungry,cumul,char_st,char_en,theIt);
        char_st=char_en+1;
        break;
      }
    }
  }
  if ( theIt ) pango_attr_iterator_destroy(theIt);
}
void                         pango_text_chunker::AddBox(int st,int en,bool whit,bool retu,PangoGlyphString* from,int offset,PangoFont* theFont,NRFont* inkFont,NR::Point &cumul)
{
  if ( en < st ) return;
//  PangoRectangle ink_rect,logical_rect;
//  int   cd_offset=charas[offset].code_point;
//  pango_glyph_string_extents_range(from,charas[st].code_point-cd_offset,charas[en].code_point-cd_offset+1,theFont,&ink_rect,&logical_rect);
  
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
  words[nbWord].x_pos=cumul[0];
  
  PangoFontMetrics *metrics = pango_font_get_metrics (theFont, NULL);
  
  words[nbWord].y_ascent = pango_to_ink*((double)(pango_font_metrics_get_ascent (metrics)));  
  words[nbWord].y_descent = pango_to_ink*((double)(pango_font_metrics_get_descent (metrics)));  

  double   cur_pos=words[nbWord].x_pos;
  int      cur_g=0;
  double   min_y=cumul[1],max_y=cumul[1];
  for (int i=st;i<=en;) {
    int cd_p=charas[i].code_point;
    int n_en=i;
    while ( n_en <= en && charas[n_en].code_point == cd_p ) n_en++;
    while ( cur_g < from->num_glyphs-1 && from->log_clusters[cur_g+1] <= i-offset ) cur_g++;
//    pango_glyph_string_extents_range(from,charas[i].code_point-cd_offset,charas[i].code_point-cd_offset+1,theFont,&ink_rect,&logical_rect);
    if ( theText->kern_x ) {
      cur_pos+=theText->kern_x[cd_p];
    }
    if ( theText->kern_y ) {
      cumul[1]+=theText->kern_y[cd_p];
    }
    for (int j=i;j<n_en;j++) {
      charas[j].x_pos=cur_pos;
      charas[j].x_adv=0/*pango_to_ink*((double)(logical_rect.width))*/;
      charas[j].y_pos=cumul[1];
      if ( cumul[1] < min_y ) min_y=cumul[1];
      if ( cumul[1] > max_y ) max_y=cumul[1];
      charas[j].x_dpos=pango_to_ink*((double)(from->glyphs[cur_g].geometry.x_offset));
      charas[j].y_dpos=pango_to_ink*((double)(from->glyphs[cur_g].geometry.y_offset));
    }
    NR::Point t_adv=nr_font_glyph_advance_get (inkFont, from->glyphs[cur_g].glyph);
    charas[i].x_adv=t_adv[0];
//    charas[i].x_adv=pango_to_ink*((double)(from->glyphs[cur_g].geometry.width));
    cur_pos+=charas[i].x_adv;
    i=n_en;
  }
  
  words[nbWord].y_ascent-=min_y;
  words[nbWord].y_descent+=max_y;
  words[nbWord].x_length=cur_pos-words[nbWord].x_pos;
  
  cumul[0]=cur_pos;
  charas[en+1].x_pos=cumul[0];
  charas[en+1].y_pos=cumul[1];
  charas[en+1].x_adv=0;
  charas[en+1].x_dpos=0;
  charas[en+1].y_dpos=0;
  nbWord++;
}
void                         pango_text_chunker::SetTextWithAttrs(text_with_info* inText,int /*flags*/) 
{
  theText=inText;
  textLength=0;
  nbWord=0;
  if ( theText == NULL || theText->UnicodeLength() <= 0 ) return;
  
  // pango structures
  PangoContext*         pContext;
  PangoFontDescription* pfd;
  PangoLogAttr*         pAttrs;
  
#ifdef WITH_XFT
  if ( theFontMap ) {
    pContext=pango_ft2_font_map_create_context((PangoFT2FontMap*)theFontMap);
  } else {
    pContext=gdk_pango_context_get();
  }
#else
  pContext=gdk_pango_context_get();
#endif
  pfd = pango_font_description_from_string (theFace);
  pango_font_description_set_size (pfd,(int)(ink_to_pango*theSize));
  pango_context_set_font_description(pContext,pfd);
  
  int     srcLen=theText->UTF8Length();
  charas=(glyph_box*)malloc((srcLen+2)*sizeof(glyph_box));
  pAttrs=(PangoLogAttr*)malloc((srcLen+2)*sizeof(PangoLogAttr));
  PangoAttrList*   tAttr=NULL;
  if ( theText->markupType == 1 ) {
    tAttr=(PangoAttrList*)theText->markup;
  } else {
    tAttr=pango_attr_list_new();
  }
  
  int       next_par_end=0;
  int       next_par_start=0;
  {
    for (int i=0;i<=srcLen;i++) charas[i].code_point=0;
    for (int i=0;i<theText->UnicodeLength();i++) {
      for (int j=theText->char_offset[i];j<theText->char_offset[i+1];j++) charas[j].code_point=i;
    }
    charas[theText->UTF8Length()].code_point=theText->UnicodeLength();
  }
  GList*  pItems=pango_itemize(pContext,theText->UTF8Text(),0,theText->UTF8Length(),tAttr,NULL);
  GList*  pGlyphs=NULL;
  for (GList* l=pItems;l;l=l->next) {
    PangoItem*  theItem=(PangoItem*)l->data;
    pango_get_log_attrs((theText->UTF8Text())+theItem->offset,theItem->length,-1,theItem->analysis.language,pAttrs+theItem->offset,theItem->length+1);
    PangoGlyphString*  nGl=pango_glyph_string_new();
    pango_shape((theText->UTF8Text())+theItem->offset,theItem->length,&theItem->analysis,nGl);
    pGlyphs=g_list_append(pGlyphs,nGl);
    
    int start_c=charas[theItem->offset].code_point;
    for (int i=theItem->offset+theItem->length;i>=theItem->offset;i--) {
      int cur_c=charas[i].code_point;
      if ( cur_c < 0 ) cur_c=0;
      if ( cur_c > srcLen ) cur_c=srcLen;
      pAttrs[i]=pAttrs[cur_c-start_c+theItem->offset];
      if ( pAttrs[i].is_word_start ) {
        if ( i > 0 && charas[i-1].code_point == charas[i].code_point ) {
          pAttrs[i].is_word_start=0;
        }
      }
      if ( pAttrs[i].is_word_end ) {
        if ( i < srcLen && charas[i+1].code_point == charas[i].code_point ) {
          pAttrs[i].is_word_end=0;
        }
      }
    }
  }
  
  int       t_pos=0/*,l_pos=0*/;
  NR::Point cumul(0,0);
  double    pango_rise=0;
//  bool      inWhite=false;
  
  next_par_end=0;
  next_par_start=0;
  pango_find_paragraph_boundary(theText->UTF8Text(),-1,&next_par_end,&next_par_start);
  
  PangoAttrIterator* theIt=(tAttr)?pango_attr_list_get_iterator (tAttr):NULL;
  
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
    double old_rise=pango_rise;
    if ( attr_st <= char_st ) {
      PangoAttribute* one_attr=NULL;
      one_attr=pango_attr_iterator_get(theIt,PANGO_ATTR_RISE);
      if ( one_attr ) {
        pango_rise=-pango_to_ink*((double)((PangoAttrInt*)one_attr)->value);
      } else {
        pango_rise=0;
      }
    } else {
      pango_rise=0;
    }
    cumul[1]+=pango_rise-old_rise;
    
    for (int g_pos=t_pos;g_pos<t_pos+theItem->length;) {
      int h_pos=g_pos+1;
      while ( h_pos < t_pos+theItem->length && pAttrs[h_pos].is_white == false && pAttrs[h_pos].is_word_end == false && pAttrs[h_pos].is_word_start == false ) h_pos++;
      if ( pAttrs[h_pos].is_white || pAttrs[h_pos].is_word_start ) h_pos--;
      bool  is_retu=(h_pos>=next_par_end);
      NRFont*  nrt_font=NULL;
      NRTypeFace *nrt_face =NULL;
      
      if ( theItem->analysis.font ) {
        PangoFontDescription*  cur_descr=pango_font_describe (theItem->analysis.font);
        char*                  pf_descr=pango_font_description_to_string(cur_descr);
        double                 theS=pango_to_ink*((double)pango_font_description_get_size(cur_descr));
        nrt_face= nr_type_directory_lookup_fuzzy(pango_font_description_get_family(cur_descr), NRTypePosDef(pf_descr));
        nrt_font = nr_font_new_default (nrt_face, NR_TYPEFACE_METRICS_HORIZONTAL, theS);
        if ( pf_descr ) g_free(pf_descr);
        if ( cur_descr ) pango_font_description_free(cur_descr);
      }
      AddBox(g_pos,h_pos,pAttrs[g_pos].is_white,is_retu,theGlyphs,theItem->offset,theItem->analysis.font,nrt_font,cumul);
      if ( nrt_font ) nr_font_unref (nrt_font);
      if ( nrt_face ) nr_typeface_unref (nrt_face);
      if ( h_pos >= next_par_end ) {
        int old_dec=next_par_start;
        pango_find_paragraph_boundary((theText->UTF8Text())+old_dec,-1,&next_par_end,&next_par_start);
        next_par_end+=old_dec;
        next_par_start+=old_dec;
      }      
      g_pos=h_pos+1;
    }
    t_pos+=theItem->length;
    if ( nbWord > 0 && pAttrs[t_pos].is_word_end == false && pAttrs[t_pos].is_word_start == false ) {
      words[nbWord-1].end_of_word=false;
    }
  }
  
//  printf("%i mots\n",nbWord);
//  for (int i=0;i<nbWord;i++) {
//    printf("%i->%i  x=%f l=%f a=%f d=%f w=%i r=%i ew=%i\n",words[i].t_first,words[i].t_last,words[i].x_pos,words[i].x_length,words[i].y_ascent,words[i].y_descent
//           ,words[i].is_white,words[i].is_return,words[i].end_of_word);
//    printf("chars: ");
//    for (int j=words[i].t_first;j<=words[i].t_last;j++) {
//      printf("(%f %f a=%f) ",charas[j].x_pos,charas[j].y_pos,charas[j].x_adv);
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
  if ( theText->markupType == 1 ) {
  } else {
    pango_attr_list_unref(tAttr);
  }
}
void                         pango_text_chunker::AddDullGlyphs(to_SVG_context *hungry,double &cumul,int c_st,int c_en)
{
  hungry->ResetStyle();
  hungry->AddFontFamily(theFace);
  hungry->AddFontSize(theSize);
  double startX=cumul;
  for (int i=c_st;i<=c_en;) {
    int  ne=i;
    int  cd_p=charas[i].code_point;
    while ( ne <= c_en && charas[ne].code_point == cd_p ) ne++;
    NR::Point at(charas[i].x_pos-startX,charas[i].y_pos);
    hungry->AddGlyph(cd_p,i,ne-1,at,charas[i].x_adv);
    i=ne;
  }
}
void                         pango_text_chunker::AddAttributedGlyphs(to_SVG_context *hungry,double &cumul,int c_st,int c_en,PangoAttrIterator *theIt) 
{
  hungry->ResetStyle();
  PangoAttribute* one_attr=NULL;
  one_attr=pango_attr_iterator_get(theIt,PANGO_ATTR_FAMILY);
  if ( one_attr ) {
    hungry->AddFontFamily(((PangoAttrString*)one_attr)->value);
  }
  one_attr=pango_attr_iterator_get(theIt,PANGO_ATTR_SIZE);
  if ( one_attr ) {
    double nSize=/*pango_to_ink**/((double)((PangoAttrInt*)one_attr)->value);
    one_attr=pango_attr_iterator_get(theIt,PANGO_ATTR_SCALE);
    if ( one_attr ) {
      nSize*=((double)((PangoAttrFloat*)one_attr)->value);
    }
    hungry->AddFontSize(nSize);
  } else {
    one_attr=pango_attr_iterator_get(theIt,PANGO_ATTR_SCALE);
    double nSize=theSize;
    if ( one_attr ) {
      nSize*=((double)((PangoAttrFloat*)one_attr)->value);
    }
    hungry->AddFontSize(nSize);
  }
  one_attr=pango_attr_iterator_get(theIt,PANGO_ATTR_STYLE);
  if ( one_attr ) {
    hungry->AddFontVariant(((PangoAttrInt*)one_attr)->value);
  }
  one_attr=pango_attr_iterator_get(theIt,PANGO_ATTR_WEIGHT);
  if ( one_attr ) {
    hungry->AddFontWeight(((PangoAttrInt*)one_attr)->value);
  }
  one_attr=pango_attr_iterator_get(theIt,PANGO_ATTR_STRETCH);
  if ( one_attr ) {
    hungry->AddFontStretch(((PangoAttrInt*)one_attr)->value);
  }
  
  double startX=cumul;
  for (int i=c_st;i<=c_en;) {
    int  ne=i;
    int  cd_p=charas[i].code_point;
    while ( ne <= c_en && charas[ne].code_point == cd_p ) ne++;
    NR::Point at(charas[i].x_pos-startX,charas[i].y_pos);
    hungry->AddGlyph(cd_p,i,ne-1,at,charas[i].x_adv);
    i=ne;
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
  n_g=0;
  orig_st_g=st_g=-1;
  en_g=-2;
  
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

void            box_to_SVG_context::SetText(char*  n_txt,int /*n_len*/)
{
  text=n_txt;
  orig_st=st=-1;
  en=-2;
  n_g=0;
  orig_st_g=st_g=-1;
  en_g=-2;
}
void            box_to_SVG_context::SetLetterSpacing(double n_spc)
{
  letter_spacing=n_spc;
  cur_spacing=0;
}

void            box_to_SVG_context::SetY(int a_g,int /*f_c*/,int /*l_c*/,double to)
{
  if ( fabs(to-cur_y) < 0.01 ) {
    if ( dys ) {
      dys=(double*)realloc(dys,(a_g-st_g+1)*sizeof(double));
      dys[a_g-st_g]=0;
    }
  } else {
    if ( n_g > 0 ) {
      if ( dys == NULL ) {
        cur_by=box_y;
        dys=(double*)malloc((en_g-st_g+1)*sizeof(double));
        dys[0]=cur_y-cur_by;
        for (int i=st_g+1;i<=en_g;i++) dys[i-st_g]=0;
      }
      dys=(double*)realloc(dys,(a_g-st_g+1)*sizeof(double));
      dys[a_g-st_g]=to-cur_y;
      cur_y=to;
    } else {
      cur_y=cur_by=to;
    }
  }
}
void            box_to_SVG_context::Flush(void)
{
  if ( n_g > 0 && text_repr ) {
    SPRepr* parent=sp_repr_parent(text_repr);
    span_repr = sp_repr_new ("tspan");

    if ( dxs == NULL && fabs(cur_spacing) > 0 ) {
      AddLetterSpacing(cur_spacing);
    }
    if ( style_repr ) {
      sp_repr_css_set (span_repr,style_repr, "style");
      sp_repr_css_attr_unref(style_repr);
      style_repr=NULL;
    }
    
    sp_repr_set_double (span_repr, "x", cur_start);
    sp_repr_set_double (span_repr, "y", cur_by);
      
    if ( dxs ) {
      gchar c[64];
      gchar *s = NULL;
      for (int j = st_g; j <= en_g ; j ++) {
        c[0]=0;
        g_ascii_formatd (c, sizeof (c), "%.8g", dxs[j-st_g]);
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
    if ( dys ) {
      gchar c[64];
      gchar *s = NULL;
      for (int j = st_g; j <= en_g ; j ++) {
        c[0]=0;
        g_ascii_formatd (c, sizeof (c), "%.8g", dys[j-st_g]);
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
    
    char  savC=text[en+1];
    text[en+1]=0;
    SPRepr* rstr = sp_xml_document_createTextNode (sp_repr_document (parent), text+st);
    sp_repr_append_child (span_repr, rstr);
    sp_repr_unref (rstr);
    text[en+1]=savC;
    
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
  n_g=0;
  st_g=-1;
  en_g=-2;
  
  if ( style_repr ) sp_repr_css_attr_unref(style_repr);
  style_repr=NULL;
  if ( span_repr ) sp_repr_unref(span_repr);
  span_repr=NULL;
}
void            box_to_SVG_context::AddGlyph(int a_g,int f_c,int l_c,const NR::Point &oat,double advance)
{
  if ( l_c < f_c ) return;
  NR::Point at=oat;
  at[0]+=box_start;
  at[1]+=box_y;
  if ( orig_st_g >= 0 ) at[0]+=((double)(a_g-orig_st_g))*letter_spacing;
//  printf("add g  %i -> %i   at %f %f -> pos %f %f\n",f_c,l_c,oat[0],oat[1],at[0],at[1]);

  if ( n_g > 0 && fabs(at[0]-cur_next-cur_spacing) < 0.01 ) {
    if ( dxs ) {
      dxs=(double*)realloc(dxs,(a_g-st_g+1)*sizeof(double));
      for (int i=en_g+1;i<a_g;i++) dxs[i-st_g]=0;
      dxs[a_g-st_g]=cur_spacing;
    }
    cur_x=at[0];
    cur_next=cur_x+advance;
    SetY(a_g,f_c,l_c,at[1]);
  } else {
    if ( n_g > 0  ) {
      if ( n_g == 1 ) {
        cur_x=at[0];
        cur_spacing=cur_x-cur_next;
        cur_next=cur_x+advance;
        SetY(a_g,f_c,l_c,at[1]);
      } else {
        if ( dxs == NULL ) {
          dxs=(double*)malloc((en_g-st_g+1)*sizeof(double));
          dxs[0]=0;
          for (int i=st_g+1;i<=en_g;i++) dxs[i-st_g]=letter_spacing; // warning= multi-character glyphs invalidate this approcah
        }
        dxs=(double*)realloc(dxs,(a_g-st_g+1)*sizeof(double));
        for (int i=en_g+1;i<a_g;i++) dxs[i-st_g]=0;
        dxs[a_g-st_g]=at[0]-cur_next-cur_spacing;
        cur_x=at[0];
        cur_spacing=cur_x-cur_next;
        cur_next=cur_x+advance;
        SetY(a_g,f_c,l_c,at[1]);
      }
    } else {
      cur_x=at[0];
      cur_start=at[0];
      cur_next=cur_x+advance;
      cur_spacing=letter_spacing;
      SetY(a_g,f_c,l_c,at[1]);
    }
  }
  if ( st < 0 || f_c < st ) st=f_c;
  if ( orig_st < 0 || f_c < orig_st ) orig_st=f_c;
  if ( en < 0 || l_c > en ) en=l_c;
  if ( st_g < 0 || a_g < st_g ) st_g=a_g;
  if ( orig_st_g < 0 || a_g < orig_st ) orig_st_g=a_g;
  if ( en_g < 0 || a_g > en_g ) en_g=a_g;
  n_g++;
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
  st_g=-1;
  en_g=-2;
  orig_st_g=-1;
  n_g=0;
  
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

void            path_to_SVG_context::SetText(char*  n_txt,int /*n_len*/)
{
  text=n_txt;
  st=-1;
  en=-2;
  n_g=0;
  st_g=-1;
  en_g=-2;
  orig_st_g=-1;
}
void            path_to_SVG_context::SetLetterSpacing(double n_spc)
{
  letter_spacing=n_spc;
}

void            path_to_SVG_context::AddGlyph(int a_g,int f_c,int l_c,const NR::Point &oat,double advance)
{
  if ( l_c < f_c ) return;
  
  NR::Point at=oat;
  if ( orig_st_g >= 0 ) at[0]+=((double)(a_g-orig_st_g))*letter_spacing;
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
    
    char  savC=text[l_c+1];
    text[l_c+1]=0;
    SPRepr* rstr = sp_xml_document_createTextNode (sp_repr_document (parent), text+f_c);
    sp_repr_append_child (span_repr, rstr);
    sp_repr_unref (rstr);
    text[l_c+1]=savC;
    
    sp_repr_append_child (text_repr, span_repr);
    sp_repr_unref (span_repr);
    span_repr=NULL; // unref'ing already done
  }  
  
  cur_x=at[0];
  if ( st < 0 || f_c < st ) st=f_c;
  if ( orig_st < 0 || f_c < orig_st ) orig_st=f_c;
  if ( en < 0 || l_c > en ) en=l_c;
  if ( st_g < 0 || a_g < st_g ) st_g=a_g;
  if ( orig_st_g < 0 || a_g < orig_st ) orig_st_g=a_g;
  if ( en_g < 0 || a_g > en_g ) en_g=a_g;
  n_g++;
  
}



