/*
 *  FlowDest.h
 */

#ifndef my_flow_dest
#define my_flow_dest

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "FlowDefs.h"

class Path;
class Shape;
class flow_dest;
class FloatLigne;

class flow_dest {
public:
	Shape*       rgn_dest; // prior to diff by regionExclude
	Shape*       rgn_flow; // after ...
	flow_dest*   next_in_flow; // as the name says; set by the flowtext owning the flowregion
	
	double       l,t,r,b;
	int          curPt;
	float        curY;

	typedef struct cached_line {
		int          date;
		FloatLigne*  theLine;
		double       y,a,d;
	} cached_line;
	int                   lastDate; // date for the cache
	int                   maxCache,nbCache;
	cached_line*          caches;  
	FloatLigne*           tempLine;
	FloatLigne*           tempLine2;

	flow_dest(void);
	~flow_dest(void);
	
	void           Reset(void);
	void           AddShape(Shape* i_shape);
	void           Prepare(void);
	void           UnPrepare(void);
	
	void					 ComputeLine(float y,float a,float d);
  box_sol        NextBox(box_sol& after,double asc,double desc,double lead,bool skip,bool &stillSameLine,double min_length);
  box_sol        TxenBox(box_sol& after,double asc,double desc,double lead,bool skip,bool &stillSameLine,double min_length);
  double         RemainingOnLine(box_sol& after);
  double         RemainingOnEnil(box_sol& after);
};

#endif
