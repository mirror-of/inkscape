#ifndef __SP_ITEM_FLOWTEXT_H__
#define __SP_ITEM_FLOWTEXT_H__

/*
 */

#include "sp-item.h"

#include "display/nr-arena-forward.h"

#include "libnrtype/FlowSrc.h"

#define SP_TYPE_FLOWTEXT            (sp_flowtext_get_type ())
#define SP_FLOWTEXT(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), SP_TYPE_FLOWTEXT, SPFlowtext))
#define SP_FLOWTEXT_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), SP_TYPE_FLOWTEXT, SPFlowtextClass))
#define SP_IS_FLOWTEXT(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), SP_TYPE_FLOWTEXT))
#define SP_IS_FLOWTEXT_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), SP_TYPE_FLOWTEXT))

class flow_dest;
class flow_src;
class flow_res;
class font_instance;
class text_style;

struct SPFlowtext : public SPItem {
	flow_dest*        f_dst;
	flow_dest*        f_excl;
	flow_src*					f_src;
	flow_res*         f_res;
	
	div_flow_src      contents;
	
	// layout options
	bool              justify;
	double            par_indent;
	int               algo;
	double            min_scale,max_scale;
	
	void              UpdateFlowSource(void);
	void              UpdateFlowDest(void);
	void              ComputeFlowRes(void);
	void              ClearFlow(NRArenaGroup* in_arena);
	void              BuildFlow(NRArenaGroup* in_arena, 	NRRect *paintbox);
};

struct SPFlowtextClass {
	SPItemClass parent_class;
};

GType sp_flowtext_get_type (void);

void sp_item_flowtext_to_text (SPFlowtext *flowt);

void convert_to_text(void);

#endif
