/*
 * FlowDest.cpp
 */

#include <config.h>
#include <string.h>

#include "FlowDest.h"

#include "libnr/nr-point.h"
#include "libnr/nr-matrix.h"
#include "libnr/nr-point-ops.h"
#include "libnr/nr-matrix-ops.h"
#include "libnr/nr-path.h"

#include "livarot/LivarotDefs.h"
#include "livarot/Path.h"
#include "livarot/Shape.h"
#include "livarot/Ligne.h"

flow_dest::flow_dest(void)
{
	rgn_dest=new Shape;
	rgn_flow=new Shape;
	next_in_flow=NULL;

  tempLine=new FloatLigne();
  tempLine2=new FloatLigne();
  lastDate=0;
  maxCache=64;
  nbCache=0;
  caches=(cached_line*)malloc(maxCache*sizeof(cached_line));
}
flow_dest::~flow_dest(void)
{
	delete rgn_dest;
	delete rgn_flow;
	
  if ( maxCache > 0 ) {
    for (int i=0;i<nbCache;i++) delete caches[i].theLine;
    free(caches);
  }
  maxCache=nbCache=0;
  caches=NULL;
  delete tempLine;
  delete tempLine2;
}
	
box_sol   flow_dest::NextBox(box_sol& after,double asc,double desc,double lead,bool skip,bool &stillSameLine,double min_length)
{
	box_sol  n_res;
	n_res.ascent=n_res.descent=n_res.leading=0;
	n_res.y=b;
	n_res.x_start=n_res.x_end=0;
	n_res.frame=NULL;
	n_res.before_rgn=false;
	n_res.after_rgn=true;
	
  if ( after.frame != this ) {
		if ( next_in_flow ) return next_in_flow->NextBox(after,asc,desc,lead,skip,stillSameLine,min_length);
		return n_res;
	}
	
  stillSameLine=false;
  
  if ( skip == false && after.ascent >= asc && after.descent >= desc && after.ascent+after.leading >= asc+lead ) {
    // we want a span that has the same height as the previous one stored in dest -> we can look for a
    // span on the same line
    ComputeLine(after.y,after.ascent,after.descent); // compute the line (should be cached for speed)
    int  elem=0;                // start on the left
    while ( elem < tempLine->nbRun && ( tempLine->runs[elem].en < after.x_start+0.1 || (tempLine->runs[elem].en-tempLine->runs[elem].st) < min_length ) ) {
      // while we're before the end of the last span, go right
      elem++;
    }
    if ( elem >= 0 && elem < tempLine->nbRun ) {
      // we found a span after the current one->return it
      n_res.before_rgn=false;
      n_res.after_rgn=false;
      n_res.y=after.y;
      n_res.ascent=(after.ascent>asc)?after.ascent:asc;
      n_res.descent=(after.descent>desc)?after.descent:desc;
      n_res.leading=(after.ascent+after.leading>asc+lead)?(after.ascent+after.leading-n_res.ascent):(asc+lead-n_res.ascent);
      n_res.frame=this;
      n_res.x_start=(after.x_start<tempLine->runs[elem].st)?tempLine->runs[elem].st:after.x_start;
      n_res.x_end=tempLine->runs[elem].en;
      stillSameLine=true;
      return n_res;
    }
  }
  
	n_res.before_rgn=false;
	n_res.after_rgn=false;
  n_res.ascent=asc;   // change the ascent and descent
  n_res.descent=desc;
	n_res.leading=lead;
  n_res.x_start=n_res.x_end=0;
	if ( after.before_rgn ) {
		n_res.y=t; // get the skyline
	} else {
		n_res.y=after.y+after.descent+lead; // get the skyline
	}
  while ( n_res.y < b ) {
    n_res.y+=n_res.ascent;
    ComputeLine(n_res.y,asc,desc);
    int  elem=0;
		while ( elem < tempLine->nbRun && (tempLine->runs[elem].en-tempLine->runs[elem].st) < min_length) {
			elem++;
		}
    if ( elem < tempLine->nbRun ) {
      // found something
      n_res.before_rgn=false;
      n_res.after_rgn=false;
      n_res.frame=this;
      n_res.x_start=tempLine->runs[elem].st;
      n_res.x_end=tempLine->runs[elem].en;
      return n_res;
    } else {
      //
    }
    n_res.y+=n_res.descent;
    n_res.y+=lead;
  }
  // nothing in this frame -> go to the next one
	if ( next_in_flow == NULL ) {
		n_res.ascent=n_res.descent=n_res.leading=0;
		n_res.y=b;
		n_res.x_start=n_res.x_end=0;
		n_res.frame=NULL;
		n_res.before_rgn=false;
		n_res.after_rgn=true;
		return n_res;
	}
  n_res.frame=next_in_flow;
	n_res.before_rgn=true;
	n_res.after_rgn=false;
  n_res.y=next_in_flow->t;
  n_res.x_start=0;
  n_res.x_end=0;  // precaution
  n_res.ascent=n_res.descent=n_res.leading=0;
  return next_in_flow->NextBox(n_res,asc,desc,lead,skip,stillSameLine,min_length);
}
box_sol   flow_dest::TxenBox(box_sol& after,double asc,double desc,double lead,bool skip,bool &stillSameLine,double min_length)
{
	box_sol  n_res;
	n_res.ascent=n_res.descent=n_res.leading=0;
	n_res.y=b;
	n_res.x_start=n_res.x_end=0;
	n_res.frame=NULL;
	n_res.before_rgn=false;
	n_res.after_rgn=true;
	
  if ( after.frame != this ) {
		if ( next_in_flow ) return next_in_flow->TxenBox(after,asc,desc,lead,skip,stillSameLine,min_length);
		return n_res;
	}
	
  stillSameLine=false;
  
  if ( skip == false && after.ascent >= asc && after.descent >= desc && after.ascent+after.leading >= asc+lead ) {
    // we want a span that has the same height as the previous one stored in dest -> we can look for a
    // span on the same line
    ComputeLine(after.y,after.ascent,after.descent); // compute the line (should be cached for speed)
    int  elem=tempLine->nbRun-1;                // start on the left
    while ( elem >= 0 && ( tempLine->runs[elem].st > after.x_start-0.1 || (tempLine->runs[elem].en-tempLine->runs[elem].st) < min_length ) ) {
      // while we're before the end of the last span, go right
      elem--;
    }
    if ( elem >= 0 && elem < tempLine->nbRun ) {
      // we found a span after the current one->return it
      n_res.before_rgn=false;
      n_res.after_rgn=false;
      n_res.y=after.y;
      n_res.ascent=(after.ascent>asc)?after.ascent:asc;
      n_res.descent=(after.descent>desc)?after.descent:desc;
      n_res.leading=(after.ascent+after.leading>asc+lead)?(after.ascent+after.leading-n_res.ascent):(asc+lead-n_res.ascent);
      n_res.frame=this;
      n_res.x_end=(after.x_start>tempLine->runs[elem].en)?tempLine->runs[elem].en:after.x_start;
      n_res.x_start=tempLine->runs[elem].st;
      stillSameLine=true;
      return n_res;
    }
  }
  
	n_res.before_rgn=false;
	n_res.after_rgn=false;
  n_res.ascent=asc;   // change the ascent and descent
  n_res.descent=desc;
	n_res.leading=lead;
  n_res.x_start=n_res.x_end=0;
	if ( after.before_rgn ) {
		n_res.y=t; // get the skyline
	} else {
		n_res.y=after.y+after.descent+lead; // get the skyline
	}
  while ( n_res.y < b ) {
    n_res.y+=n_res.ascent;
    ComputeLine(n_res.y,asc,desc);
    int  elem=tempLine->nbRun-1;
		while ( elem >= 0 && (tempLine->runs[elem].en-tempLine->runs[elem].st) < min_length) {
			elem--;
		}
    if ( elem >= 0 ) {
      // found something
      n_res.before_rgn=false;
      n_res.after_rgn=false;
      n_res.frame=this;
      n_res.x_start=tempLine->runs[elem].st;
      n_res.x_end=tempLine->runs[elem].en;
      return n_res;
    } else {
      //
    }
    n_res.y+=n_res.descent;
    n_res.y+=lead;
  }
  // nothing in this frame -> go to the next one
	if ( next_in_flow == NULL ) {
		n_res.ascent=n_res.descent=n_res.leading=0;
		n_res.y=b;
		n_res.x_start=n_res.x_end=0;
		n_res.frame=NULL;
		n_res.before_rgn=false;
		n_res.after_rgn=true;
		return n_res;
	}
  n_res.frame=next_in_flow;
	n_res.before_rgn=true;
	n_res.after_rgn=false;
  n_res.y=next_in_flow->t;
  n_res.x_start=0;
  n_res.x_end=0;  // precaution
  n_res.ascent=n_res.descent=n_res.leading=0;
  return next_in_flow->TxenBox(n_res,asc,desc,lead,skip,stillSameLine,min_length);
}
double         flow_dest::RemainingOnLine(box_sol& after)
{
  if ( after.frame != this ) {
		if ( next_in_flow ) return next_in_flow->RemainingOnLine(after);
		return 0;
	}
  
  ComputeLine(after.y,after.ascent,after.descent);  // get the line
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
double         flow_dest::RemainingOnEnil(box_sol& after)
{
  if ( after.frame != this ) {
		if ( next_in_flow ) return next_in_flow->RemainingOnEnil(after);
		return 0;
	}
  
  ComputeLine(after.y,after.ascent,after.descent);  // get the line
  int  elem=tempLine->nbRun-1;
  while ( elem >= 0 && tempLine->runs[elem].st > after.x_start ) elem--;
  float  left=0;
  // and add the spaces
  while ( elem >= 0 ) {
    if ( after.x_start > tempLine->runs[elem].en ) {
      left+=tempLine->runs[elem].en-tempLine->runs[elem].st;
    } else {
      left+=after.x_start-tempLine->runs[elem].st;
    }
    elem--;
  }
  return left;
}
void           flow_dest::Reset(void)
{
	delete rgn_dest;
	rgn_dest=new Shape;
	delete rgn_flow;
	rgn_flow=new Shape;
}
void           flow_dest::AddShape(Shape* i_shape)
{
	if ( rgn_dest->hasEdges() == false ) {
		rgn_dest->Copy(i_shape);
	} else if ( i_shape->hasEdges() == false ) {
	} else {
		Shape* temp=new Shape;
		temp->Booleen(i_shape,rgn_dest,bool_op_union);
		delete rgn_dest;
		rgn_dest=temp;
	}
}
void           flow_dest::Prepare(void)
{
	rgn_flow->CalcBBox(true);
	l=rgn_flow->leftX;
	t=rgn_flow->topY;
	r=rgn_flow->rightX;
	b=rgn_flow->bottomY;
	curY=t-1.0;
	curPt=0;
  rgn_flow->BeginRaster(curY,curPt,1.0);
	// empty line cache
	for (int i=0;i<nbCache;i++) delete caches[i].theLine;
	nbCache=0;
  lastDate=0;
}
void           flow_dest::UnPrepare(void)
{
  rgn_flow->EndRaster();  
}


void                  flow_dest::ComputeLine(float y,float a,float d)
{
  double   top=y-a,bottom=y+d;
  
  lastDate++;
  
  int oldest=-1;
  int age=lastDate;
  for (int i=0;i<nbCache;i++) {
    if ( caches[i].date < age ) {
      age=caches[i].date;
      oldest=i;
    }
		if ( fabs(caches[i].y-y) <= 0.0001 ) {
			if ( fabs(caches[i].a-a) <= 0.0001 ) {
				if ( fabs(caches[i].d-d) <= 0.0001 ) {
					tempLine->Copy(caches[i].theLine);
					return;
				}
			}
		}
  }
  
  if ( fabs(bottom-top) < 0.001 ) {
    tempLine->Reset();
    return;
  }
  
  tempLine2->Reset();  // par securite
  rgn_flow->Scan(curY,curPt,top,bottom-top);
  rgn_flow->Scan(curY,curPt,bottom,tempLine2,true,bottom-top);
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
  } else {
    oldest=nbCache++;
    caches[oldest].theLine=new FloatLigne();
    caches[oldest].theLine->Copy(tempLine);
    caches[oldest].date=lastDate;
    caches[oldest].y=y;
    caches[oldest].a=a;
    caches[oldest].d=d;
  }
}
