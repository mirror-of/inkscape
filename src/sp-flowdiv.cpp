#define __SP_FLOWDIV_C__

/*
 */

#include <config.h>
#include <string.h>

#include "xml/repr.h"
#include "svg/svg.h"

#include "sp-object.h"
#include "sp-item.h"
#include "style.h"

#include "libnrtype/font-instance.h"
#include "libnrtype/FontFactory.h"
#include "libnrtype/font-style-to-pos.h"

#include "libnrtype/FlowSrc.h"
#include "libnrtype/text_style.h"

#include "sp-flowregion.h"
#include "sp-flowdiv.h"
#include "sp-text.h" // for the SPString stuff

static void sp_flowdiv_class_init (SPFlowdivClass *klass);
static void sp_flowdiv_init (SPFlowdiv *group);
static SPRepr *sp_flowdiv_write (SPObject *object, SPRepr *repr, guint flags);
static gchar * sp_flowdiv_description (SPItem * item);
static void sp_flowdiv_update (SPObject *object, SPCtx *ctx, guint flags);
static void sp_flowdiv_modified (SPObject *object, guint flags);

static void sp_flowtspan_class_init (SPFlowtspanClass *klass);
static void sp_flowtspan_init (SPFlowtspan *group);
static SPRepr *sp_flowtspan_write (SPObject *object, SPRepr *repr, guint flags);
static gchar * sp_flowtspan_description (SPItem * item);
static void sp_flowtspan_update (SPObject *object, SPCtx *ctx, guint flags);
static void sp_flowtspan_modified (SPObject *object, guint flags);

static void sp_flowpara_class_init (SPFlowparaClass *klass);
static void sp_flowpara_init (SPFlowpara *group);
static SPRepr *sp_flowpara_write (SPObject *object, SPRepr *repr, guint flags);
static gchar * sp_flowpara_description (SPItem * item);
static void sp_flowpara_update (SPObject *object, SPCtx *ctx, guint flags);
static void sp_flowpara_modified (SPObject *object, guint flags);

static SPItemClass * flowdiv_parent_class;
static SPItemClass * flowtspan_parent_class;
static SPItemClass * flowpara_parent_class;

GType
sp_flowdiv_get_type (void)
{
	static GType group_type = 0;
	if (!group_type) {
		GTypeInfo group_info = {
			sizeof (SPFlowdivClass),
			NULL,	/* base_init */
			NULL,	/* base_finalize */
			(GClassInitFunc) sp_flowdiv_class_init,
			NULL,	/* class_finalize */
			NULL,	/* class_data */
			sizeof (SPFlowdiv),
			16,	/* n_preallocs */
			(GInstanceInitFunc) sp_flowdiv_init,
			NULL,	/* value_table */
		};
		group_type = g_type_register_static (SP_TYPE_ITEM, "SPFlowdiv", &group_info, (GTypeFlags)0);
	}
	return group_type;
}

static void
sp_flowdiv_class_init (SPFlowdivClass *klass)
{
	GObjectClass * object_class;
	SPObjectClass * sp_object_class;
	SPItemClass * item_class;
	
	object_class = (GObjectClass *) klass;
	sp_object_class = (SPObjectClass *) klass;
	item_class = (SPItemClass *) klass;
	
	flowdiv_parent_class = (SPItemClass *)g_type_class_ref (SP_TYPE_ITEM);
	
	sp_object_class->write = sp_flowdiv_write;
	sp_object_class->update = sp_flowdiv_update;
	sp_object_class->modified = sp_flowdiv_modified;
	
	item_class->description = sp_flowdiv_description;
}

static void
sp_flowdiv_init (SPFlowdiv */*group*/)
{
}

static void
sp_flowdiv_update (SPObject *object, SPCtx *ctx, unsigned int flags)
{
	SPFlowdiv *group;
	SPObject *child;
	SPItemCtx *ictx, cctx;
	GSList *l;
	
	group = SP_FLOWDIV (object);
	ictx = (SPItemCtx *) ctx;
	cctx = *ictx;
	
	if (((SPObjectClass *) (flowdiv_parent_class))->update)
		((SPObjectClass *) (flowdiv_parent_class))->update (object, ctx, flags);
	
	if (flags & SP_OBJECT_MODIFIED_FLAG) flags |= SP_OBJECT_PARENT_MODIFIED_FLAG;
	flags &= SP_OBJECT_MODIFIED_CASCADE;
	
	l = NULL;
	for (child = sp_object_first_child(object) ; child != NULL ; child = SP_OBJECT_NEXT(child) ) {
		g_object_ref (G_OBJECT (child));
		l = g_slist_prepend (l, child);
	}
	l = g_slist_reverse (l);
	while (l) {
		child = SP_OBJECT (l->data);
		l = g_slist_remove (l, child);
		if (flags || (child->uflags & (SP_OBJECT_MODIFIED_FLAG | SP_OBJECT_CHILD_MODIFIED_FLAG))) {
			if (SP_IS_ITEM (child)) {
				SPItem *chi;
				chi = SP_ITEM (child);
				nr_matrix_multiply (&cctx.i2doc, &chi->transform, &ictx->i2doc);
				nr_matrix_multiply (&cctx.i2vp, &chi->transform, &ictx->i2vp);
				child->updateDisplay((SPCtx *)&cctx, flags);
			} else {
				child->updateDisplay(ctx, flags);
			}
		}
		g_object_unref (G_OBJECT (child));
	}
}
static void
sp_flowdiv_modified (SPObject *object, guint flags)
{
	SPFlowdiv *group;
	SPObject *child;
	GSList *l;
	
	group = SP_FLOWDIV (object);	
	
	if (flags & SP_OBJECT_MODIFIED_FLAG) flags |= SP_OBJECT_PARENT_MODIFIED_FLAG;
	flags &= SP_OBJECT_MODIFIED_CASCADE;
	
	l = NULL;
	for (child = sp_object_first_child(object) ; child != NULL ; child = SP_OBJECT_NEXT(child) ) {
		g_object_ref (G_OBJECT (child));
		l = g_slist_prepend (l, child);
	}
	l = g_slist_reverse (l);
	while (l) {
		child = SP_OBJECT (l->data);
		l = g_slist_remove (l, child);
		if (flags || (child->mflags & (SP_OBJECT_MODIFIED_FLAG | SP_OBJECT_CHILD_MODIFIED_FLAG))) {
			child->emitModified(flags);
		}
		g_object_unref (G_OBJECT (child));
	}
}
static SPRepr *
sp_flowdiv_write (SPObject *object, SPRepr *repr, guint flags)
{
//	SPFlowdiv *group = SP_FLOWDIV (object);
	
	if ( flags&SP_OBJECT_WRITE_BUILD ) {
		if ( repr == NULL ) repr = sp_repr_new ("flowDiv");
		GSList *l = NULL;
		for (SPObject* child = sp_object_first_child(object) ; child != NULL ; child = SP_OBJECT_NEXT(child) ) {
			SPRepr* c_repr=NULL;
			if ( SP_IS_FLOWTSPAN (child) ) {
				c_repr = child->updateRepr(NULL, flags);
			} else if ( SP_IS_FLOWPARA(child) ) {
				c_repr = child->updateRepr(NULL, flags);
			} else if ( SP_IS_STRING(child) ) {
				c_repr = sp_xml_document_createTextNode (sp_repr_document (repr), SP_STRING_TEXT (child));
			}
			if ( c_repr ) l = g_slist_prepend (l, c_repr);
		}
		while ( l ) {
			sp_repr_add_child (repr, (SPRepr *) l->data, NULL);
			sp_repr_unref ((SPRepr *) l->data);
			l = g_slist_remove (l, l->data);
		}
	} else {
		for (SPObject* child = sp_object_first_child(object) ; child != NULL ; child = SP_OBJECT_NEXT(child) ) {
			if ( SP_IS_FLOWTSPAN (child) ) {
				child->updateRepr(flags);
			} else if ( SP_IS_FLOWPARA(child) ) {
				child->updateRepr(flags);
			} else if ( SP_IS_STRING(child) ) {
				sp_repr_set_content (SP_OBJECT_REPR (child), SP_STRING_TEXT (child));
			}
		}
	}
		
	if (((SPObjectClass *) (flowdiv_parent_class))->write)
		((SPObjectClass *) (flowdiv_parent_class))->write (object, repr, flags);
	
	return repr;
}



static gchar * sp_flowdiv_description (SPItem * item)
{
	SPFlowdiv * group;
	
	group = SP_FLOWDIV (item);
	
	//	return g_strdup_printf(_("Flow text"));
	return g_strdup_printf("Flow division");
}

/*
 *
 */

GType
sp_flowtspan_get_type (void)
{
	static GType group_type = 0;
	if (!group_type) {
		GTypeInfo group_info = {
			sizeof (SPFlowtspanClass),
			NULL,	/* base_init */
			NULL,	/* base_finalize */
			(GClassInitFunc) sp_flowtspan_class_init,
			NULL,	/* class_finalize */
			NULL,	/* class_data */
			sizeof (SPFlowtspan),
			16,	/* n_preallocs */
			(GInstanceInitFunc) sp_flowtspan_init,
			NULL,	/* value_table */
		};
		group_type = g_type_register_static (SP_TYPE_ITEM, "SPFlowtspan", &group_info, (GTypeFlags)0);
	}
	return group_type;
}

static void
sp_flowtspan_class_init (SPFlowtspanClass *klass)
{
	GObjectClass * object_class;
	SPObjectClass * sp_object_class;
	SPItemClass * item_class;
	
	object_class = (GObjectClass *) klass;
	sp_object_class = (SPObjectClass *) klass;
	item_class = (SPItemClass *) klass;
	
	flowtspan_parent_class = (SPItemClass *)g_type_class_ref (SP_TYPE_ITEM);
	
	sp_object_class->write = sp_flowtspan_write;
	sp_object_class->update = sp_flowtspan_update;
	sp_object_class->modified = sp_flowtspan_modified;
	
	item_class->description = sp_flowtspan_description;
}

static void
sp_flowtspan_init (SPFlowtspan */*group*/)
{
}

static void
sp_flowtspan_update (SPObject *object, SPCtx *ctx, unsigned int flags)
{
	SPFlowtspan *group;
	SPObject *child;
	SPItemCtx *ictx, cctx;
	GSList *l;
	
	group = SP_FLOWTSPAN (object);
	ictx = (SPItemCtx *) ctx;
	cctx = *ictx;
	
	if (((SPObjectClass *) (flowtspan_parent_class))->update)
		((SPObjectClass *) (flowtspan_parent_class))->update (object, ctx, flags);
	
	if (flags & SP_OBJECT_MODIFIED_FLAG) flags |= SP_OBJECT_PARENT_MODIFIED_FLAG;
	flags &= SP_OBJECT_MODIFIED_CASCADE;
	
	l = NULL;
	for (child = sp_object_first_child(object) ; child != NULL ; child = SP_OBJECT_NEXT(child) ) {
		g_object_ref (G_OBJECT (child));
		l = g_slist_prepend (l, child);
	}
	l = g_slist_reverse (l);
	while (l) {
		child = SP_OBJECT (l->data);
		l = g_slist_remove (l, child);
		if (flags || (child->uflags & (SP_OBJECT_MODIFIED_FLAG | SP_OBJECT_CHILD_MODIFIED_FLAG))) {
			if (SP_IS_ITEM (child)) {
				SPItem *chi;
				chi = SP_ITEM (child);
				nr_matrix_multiply (&cctx.i2doc, &chi->transform, &ictx->i2doc);
				nr_matrix_multiply (&cctx.i2vp, &chi->transform, &ictx->i2vp);
				child->updateDisplay((SPCtx *)&cctx, flags);
			} else {
				child->updateDisplay(ctx, flags);
			}
		}
		g_object_unref (G_OBJECT (child));
	}
}
static void
sp_flowtspan_modified (SPObject *object, guint flags)
{
	SPFlowtspan *group;
	SPObject *child;
	GSList *l;
	
	group = SP_FLOWTSPAN (object);	
	
	if (flags & SP_OBJECT_MODIFIED_FLAG) flags |= SP_OBJECT_PARENT_MODIFIED_FLAG;
	flags &= SP_OBJECT_MODIFIED_CASCADE;
	
	l = NULL;
	for (child = sp_object_first_child(object) ; child != NULL ; child = SP_OBJECT_NEXT(child) ) {
		g_object_ref (G_OBJECT (child));
		l = g_slist_prepend (l, child);
	}
	l = g_slist_reverse (l);
	while (l) {
		child = SP_OBJECT (l->data);
		l = g_slist_remove (l, child);
		if (flags || (child->mflags & (SP_OBJECT_MODIFIED_FLAG | SP_OBJECT_CHILD_MODIFIED_FLAG))) {
			child->emitModified(flags);
		}
		g_object_unref (G_OBJECT (child));
	}
}
static SPRepr *
sp_flowtspan_write (SPObject *object, SPRepr *repr, guint flags)
{
//	SPFlowtspan *group = SP_FLOWTSPAN (object);
	
	if ( flags&SP_OBJECT_WRITE_BUILD ) {
		if ( repr == NULL ) repr = sp_repr_new ("flowSpan");
		GSList *l = NULL;
		for (SPObject* child = sp_object_first_child(object) ; child != NULL ; child = SP_OBJECT_NEXT(child) ) {
			SPRepr* c_repr=NULL;
			if ( SP_IS_FLOWTSPAN (child) ) {
				c_repr = child->updateRepr(NULL, flags);
			} else if ( SP_IS_FLOWPARA (child) ) {
					c_repr = child->updateRepr(NULL, flags);
			} else if ( SP_IS_STRING(child) ) {
				c_repr = sp_xml_document_createTextNode (sp_repr_document (repr), SP_STRING_TEXT (child));
			}
			if ( c_repr ) l = g_slist_prepend (l, c_repr);
		}
		while ( l ) {
			sp_repr_add_child (repr, (SPRepr *) l->data, NULL);
			sp_repr_unref ((SPRepr *) l->data);
			l = g_slist_remove (l, l->data);
		}
	} else {
		for (SPObject* child = sp_object_first_child(object) ; child != NULL ; child = SP_OBJECT_NEXT(child) ) {
			if ( SP_IS_FLOWTSPAN (child) ) {
				child->updateRepr(flags);
			} else if ( SP_IS_FLOWPARA (child) ) {
					child->updateRepr(flags);
			} else if ( SP_IS_STRING(child) ) {
				sp_repr_set_content (SP_OBJECT_REPR (child), SP_STRING_TEXT (child));
			}
		}
	}
		
	if (((SPObjectClass *) (flowtspan_parent_class))->write)
		((SPObjectClass *) (flowtspan_parent_class))->write (object, repr, flags);
	
	return repr;
}



static gchar * sp_flowtspan_description (SPItem * item)
{
	SPFlowtspan * group;
	
	group = SP_FLOWTSPAN (item);
	
	//	return g_strdup_printf(_("Flow text"));
	return g_strdup_printf("Flow tspan");
}

/*
 *
 */

GType
sp_flowpara_get_type (void)
{
	static GType group_type = 0;
	if (!group_type) {
		GTypeInfo group_info = {
			sizeof (SPFlowparaClass),
			NULL,	/* base_init */
			NULL,	/* base_finalize */
			(GClassInitFunc) sp_flowpara_class_init,
			NULL,	/* class_finalize */
			NULL,	/* class_data */
			sizeof (SPFlowpara),
			16,	/* n_preallocs */
			(GInstanceInitFunc) sp_flowpara_init,
			NULL,	/* value_table */
		};
		group_type = g_type_register_static (SP_TYPE_ITEM, "SPFlowpara", &group_info, (GTypeFlags)0);
	}
	return group_type;
}

static void
sp_flowpara_class_init (SPFlowparaClass *klass)
{
	GObjectClass * object_class;
	SPObjectClass * sp_object_class;
	SPItemClass * item_class;
	
	object_class = (GObjectClass *) klass;
	sp_object_class = (SPObjectClass *) klass;
	item_class = (SPItemClass *) klass;
	
	flowpara_parent_class = (SPItemClass *)g_type_class_ref (SP_TYPE_ITEM);
	
	sp_object_class->write = sp_flowpara_write;
	sp_object_class->update = sp_flowpara_update;
	sp_object_class->modified = sp_flowpara_modified;
	
	item_class->description = sp_flowpara_description;
}

static void
sp_flowpara_init (SPFlowpara */*group*/)
{
}

static void
sp_flowpara_update (SPObject *object, SPCtx *ctx, unsigned int flags)
{
	SPFlowpara *group;
	SPObject *child;
	SPItemCtx *ictx, cctx;
	GSList *l;
	
	group = SP_FLOWPARA (object);
	ictx = (SPItemCtx *) ctx;
	cctx = *ictx;
	
	if (((SPObjectClass *) (flowpara_parent_class))->update)
		((SPObjectClass *) (flowpara_parent_class))->update (object, ctx, flags);
	
	if (flags & SP_OBJECT_MODIFIED_FLAG) flags |= SP_OBJECT_PARENT_MODIFIED_FLAG;
	flags &= SP_OBJECT_MODIFIED_CASCADE;
	
	l = NULL;
	for (child = sp_object_first_child(object) ; child != NULL ; child = SP_OBJECT_NEXT(child) ) {
		g_object_ref (G_OBJECT (child));
		l = g_slist_prepend (l, child);
	}
	l = g_slist_reverse (l);
	while (l) {
		child = SP_OBJECT (l->data);
		l = g_slist_remove (l, child);
		if (flags || (child->uflags & (SP_OBJECT_MODIFIED_FLAG | SP_OBJECT_CHILD_MODIFIED_FLAG))) {
			if (SP_IS_ITEM (child)) {
				SPItem *chi;
				chi = SP_ITEM (child);
				nr_matrix_multiply (&cctx.i2doc, &chi->transform, &ictx->i2doc);
				nr_matrix_multiply (&cctx.i2vp, &chi->transform, &ictx->i2vp);
				child->updateDisplay((SPCtx *)&cctx, flags);
			} else {
				child->updateDisplay(ctx, flags);
			}
		}
		g_object_unref (G_OBJECT (child));
	}
}
static void
sp_flowpara_modified (SPObject *object, guint flags)
{
	SPFlowpara *group;
	SPObject *child;
	GSList *l;
	
	group = SP_FLOWPARA (object);	
	
	if (flags & SP_OBJECT_MODIFIED_FLAG) flags |= SP_OBJECT_PARENT_MODIFIED_FLAG;
	flags &= SP_OBJECT_MODIFIED_CASCADE;
	
	l = NULL;
	for (child = sp_object_first_child(object) ; child != NULL ; child = SP_OBJECT_NEXT(child) ) {
		g_object_ref (G_OBJECT (child));
		l = g_slist_prepend (l, child);
	}
	l = g_slist_reverse (l);
	while (l) {
		child = SP_OBJECT (l->data);
		l = g_slist_remove (l, child);
		if (flags || (child->mflags & (SP_OBJECT_MODIFIED_FLAG | SP_OBJECT_CHILD_MODIFIED_FLAG))) {
			child->emitModified(flags);
		}
		g_object_unref (G_OBJECT (child));
	}
}
static SPRepr *
sp_flowpara_write (SPObject *object, SPRepr *repr, guint flags)
{
//	SPFlowpara *group = SP_FLOWPARA (object);
	
	if ( flags&SP_OBJECT_WRITE_BUILD ) {
		if ( repr == NULL ) repr = sp_repr_new ("flowPara");
		GSList *l = NULL;
		for (SPObject* child = sp_object_first_child(object) ; child != NULL ; child = SP_OBJECT_NEXT(child) ) {
			SPRepr* c_repr=NULL;
			if ( SP_IS_FLOWTSPAN (child) ) {
				c_repr = child->updateRepr(NULL, flags);
			} else if ( SP_IS_FLOWPARA (child) ) {
					c_repr = child->updateRepr(NULL, flags);
			} else if ( SP_IS_STRING(child) ) {
				c_repr = sp_xml_document_createTextNode (sp_repr_document (repr), SP_STRING_TEXT (child));
			}
			if ( c_repr ) l = g_slist_prepend (l, c_repr);
		}
		while ( l ) {
			sp_repr_add_child (repr, (SPRepr *) l->data, NULL);
			sp_repr_unref ((SPRepr *) l->data);
			l = g_slist_remove (l, l->data);
		}
	} else {
		for (SPObject* child = sp_object_first_child(object) ; child != NULL ; child = SP_OBJECT_NEXT(child) ) {
			if ( SP_IS_FLOWTSPAN (child) ) {
				child->updateRepr(flags);
			} else if ( SP_IS_FLOWPARA (child) ) {
					child->updateRepr(flags);
			} else if ( SP_IS_STRING(child) ) {
				sp_repr_set_content (SP_OBJECT_REPR (child), SP_STRING_TEXT (child));
			}
		}
	}
		
	if (((SPObjectClass *) (flowpara_parent_class))->write)
		((SPObjectClass *) (flowpara_parent_class))->write (object, repr, flags);
	
	return repr;
}



static gchar * sp_flowpara_description (SPItem * item)
{
	SPFlowpara * group;
	
	group = SP_FLOWPARA (item);
	
	//	return g_strdup_printf(_("Flow text"));
	return g_strdup_printf("Flow paragraph");
}

/*
 *
 * flow preparation
 *
 */

static void DoCollectFlow(SPObject* object,flow_src* collector,bool add_line_brk,bool add_rgn_brk);

void                SPFlowdiv::CollectFlow(flow_src* collector)
{
	DoCollectFlow(SP_OBJECT(this),collector,true,false);
}
void                SPFlowtspan::CollectFlow(flow_src* collector)
{
	DoCollectFlow(SP_OBJECT(this),collector,false,false);
}
void                SPFlowpara::CollectFlow(flow_src* collector)
{
	DoCollectFlow(SP_OBJECT(this),collector,true,false);
}

static void DoCollectFlow(SPObject* object,flow_src* collector,bool add_line_brk,bool add_rgn_brk)
{
	SPItem*   item=SP_ITEM(object);
	
	text_style*      n_style=new text_style;
	n_style->SetStyle(SP_OBJECT_STYLE (object));
	bool             do_preserve=(object->xml_space.value == SP_XML_SPACE_PRESERVE);
	SPRepr*          o_repr=SP_OBJECT_REPR(object);
	const char*      kern_val=NULL;
	double*          kern_x=NULL;
	double*          kern_y=NULL;
	int              nb_kern_x=0;
	int              nb_kern_y=0;
	
	collector->Push(n_style);
	kern_val=sp_repr_attr(o_repr,"dx");
	if ( kern_val ) {
		GList* kern_list=sp_svg_length_list_read (kern_val);
		if ( kern_list ) {
			int      kern_length=0;
			for (GList* l=kern_list;l;l=l->next) kern_length++;
			kern_x=(double*)malloc(kern_length*sizeof(double));
			kern_length=0;
			for (GList* l=kern_list;l;l=l->next) {
				SPSVGLength* nl=(SPSVGLength*)l->data;
				kern_x[kern_length]=nl->computed;
				kern_length++;
			}
			nb_kern_x=kern_length;
			for (GList* l=kern_list;l;l=l->next) if ( l->data ) g_free(l->data);
			g_list_free(kern_list);
			collector->SetKern(kern_x,nb_kern_x,true);
		}
	}
	kern_val=sp_repr_attr(o_repr,"dy");
	if ( kern_val ) {
		GList*   kern_list=sp_svg_length_list_read (kern_val);
		if ( kern_list ) {
			int      kern_length=0;
			for (GList* l=kern_list;l;l=l->next) kern_length++;
			kern_y=(double*)malloc(kern_length*sizeof(double));
			kern_length=0;
			for (GList* l=kern_list;l;l=l->next) {
				SPSVGLength* nl=(SPSVGLength*)l->data;
				kern_y[kern_length]=nl->computed;
				kern_length++;
			}
			nb_kern_y=kern_length;
			for (GList* l=kern_list;l;l=l->next) if ( l->data ) g_free(l->data);
			g_list_free(kern_list);
			collector->SetKern(kern_y,nb_kern_y,false);
		}
	}
	
	for (SPObject* child = sp_object_first_child(object) ; child != NULL ; child = SP_OBJECT_NEXT(child) ) {
		if ( SP_IS_FLOWTSPAN (child) ) {
			SPFlowtspan*      c_child=SP_FLOWTSPAN(child);
			c_child->CollectFlow(collector);
		} else if ( SP_IS_FLOWPARA (child) ) {
			SPFlowpara*       c_child=SP_FLOWPARA(child);
			c_child->CollectFlow(collector);
		} else if ( SP_IS_STRING(child) ) {
			SPString*         c_child=SP_STRING(child);
			collector->AddUTF8(SP_STRING_TEXT(c_child),-1,do_preserve);
		}
	}
	if ( add_line_brk ) collector->AddControl(flw_line_brk);
	if ( add_rgn_brk ) collector->AddControl(flw_rgn_brk);
	
	collector->Pop();
	if ( kern_x ) free(kern_x);
	if ( kern_y ) free(kern_y);
}

