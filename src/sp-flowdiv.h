#ifndef __SP_ITEM_FLOWDIV_H__
#define __SP_ITEM_FLOWDIV_H__

/*
 */

#include "sp-item.h"

#define SP_TYPE_FLOWDIV            (sp_flowdiv_get_type ())
#define SP_FLOWDIV(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), SP_TYPE_FLOWDIV, SPFlowdiv))
#define SP_FLOWDIV_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), SP_TYPE_FLOWDIV, SPFlowdivClass))
#define SP_IS_FLOWDIV(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), SP_TYPE_FLOWDIV))
#define SP_IS_FLOWDIV_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), SP_TYPE_FLOWDIV))

#define SP_TYPE_FLOWTSPAN            (sp_flowtspan_get_type ())
#define SP_FLOWTSPAN(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), SP_TYPE_FLOWTSPAN, SPFlowtspan))
#define SP_FLOWTSPAN_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), SP_TYPE_FLOWTSPAN, SPFlowtspanClass))
#define SP_IS_FLOWTSPAN(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), SP_TYPE_FLOWTSPAN))
#define SP_IS_FLOWTSPAN_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), SP_TYPE_FLOWTSPAN))

#define SP_TYPE_FLOWPARA            (sp_flowpara_get_type ())
#define SP_FLOWPARA(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), SP_TYPE_FLOWPARA, SPFlowpara))
#define SP_FLOWPARA_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), SP_TYPE_FLOWPARA, SPFlowparaClass))
#define SP_IS_FLOWPARA(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), SP_TYPE_FLOWPARA))
#define SP_IS_FLOWPARA_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), SP_TYPE_FLOWPARA))

class flow_src;

struct SPFlowdiv : public SPItem {
	void                CollectFlow(flow_src* collector);
};

struct SPFlowtspan : public SPItem {
	void                CollectFlow(flow_src* collector);
};

struct SPFlowpara : public SPItem {
	void                CollectFlow(flow_src* collector);
};

struct SPFlowdivClass {
	SPItemClass parent_class;
};

struct SPFlowtspanClass {
	SPItemClass parent_class;
};

struct SPFlowparaClass {
	SPItemClass parent_class;
};

GType sp_flowdiv_get_type (void);
GType sp_flowtspan_get_type (void);
GType sp_flowpara_get_type (void);

#endif
