#ifndef __SP_ITEM_FLOWREGION_H__
#define __SP_ITEM_FLOWREGION_H__

/*
 */

#include "sp-item.h"

#define SP_TYPE_FLOWREGION            (sp_flowregion_get_type ())
#define SP_FLOWREGION(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), SP_TYPE_FLOWREGION, SPFlowregion))
#define SP_FLOWREGION_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), SP_TYPE_FLOWREGION, SPFlowregionClass))
#define SP_IS_FLOWREGION(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), SP_TYPE_FLOWREGION))
#define SP_IS_FLOWREGION_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), SP_TYPE_FLOWREGION))

#define SP_TYPE_FLOWREGIONEXCLUDE            (sp_flowregionexclude_get_type ())
#define SP_FLOWREGIONEXCLUDE(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), SP_TYPE_FLOWREGIONEXCLUDE, SPFlowregionExclude))
#define SP_FLOWREGIONEXCLUDE_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), SP_TYPE_FLOWREGIONEXCLUDE, SPFlowregionExcludeClass))
#define SP_IS_FLOWREGIONEXCLUDE(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), SP_TYPE_FLOWREGIONEXCLUDE))
#define SP_IS_FLOWREGIONEXCLUDE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), SP_TYPE_FLOWREGIONEXCLUDE))

#include "libnrtype/FlowDefs.h"

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

struct SPFlowregion : public SPItem {
	int              nbComp,maxComp;
	flow_dest*       *computed;
	
	void             UpdateComputed(void);
};

struct SPFlowregionClass {
	SPItemClass parent_class;
};

GType sp_flowregion_get_type (void);

struct SPFlowregionExclude : public SPItem {
	flow_dest*       computed;
	
	void             UpdateComputed(void);
};

struct SPFlowregionExcludeClass {
	SPItemClass parent_class;
};

GType sp_flowregionexclude_get_type (void);

#endif
