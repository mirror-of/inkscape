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
static void sp_flowdiv_modified (SPObject *object, guint flags);

static void sp_flowtspan_class_init (SPFlowtspanClass *klass);
static void sp_flowtspan_init (SPFlowtspan *group);
static SPRepr *sp_flowtspan_write (SPObject *object, SPRepr *repr, guint flags);
static void sp_flowtspan_modified (SPObject *object, guint flags);

static void sp_flowpara_class_init (SPFlowparaClass *klass);
static void sp_flowpara_init (SPFlowpara *group);
static SPRepr *sp_flowpara_write (SPObject *object, SPRepr *repr, guint flags);
static void sp_flowpara_modified (SPObject *object, guint flags);

static void sp_flowline_class_init (SPFlowlineClass *klass);
static void sp_flowline_init (SPFlowline *group);
static SPRepr *sp_flowline_write (SPObject *object, SPRepr *repr, guint flags);
static void sp_flowline_modified (SPObject *object, guint flags);

static void sp_flowregionbreak_class_init (SPFlowregionbreakClass *klass);
static void sp_flowregionbreak_init (SPFlowregionbreak *group);
static SPRepr *sp_flowregionbreak_write (SPObject *object, SPRepr *repr, guint flags);
static void sp_flowregionbreak_modified (SPObject *object, guint flags);

static SPItemClass * flowdiv_parent_class;
static SPItemClass * flowtspan_parent_class;
static SPItemClass * flowpara_parent_class;
static SPObjectClass * flowline_parent_class;
static SPObjectClass * flowregionbreak_parent_class;

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
	
	object_class = (GObjectClass *) klass;
	sp_object_class = (SPObjectClass *) klass;
	
	flowdiv_parent_class = (SPItemClass *)g_type_class_ref (SP_TYPE_ITEM);
	
	sp_object_class->write = sp_flowdiv_write;
	sp_object_class->modified = sp_flowdiv_modified;
}

static void
sp_flowdiv_init (SPFlowdiv */*group*/)
{
}

static void
sp_flowdiv_modified (SPObject *object, guint flags)
{
	SPFlowdiv *group;
	SPObject *child;
	GSList *l;
	
	group = SP_FLOWDIV (object);	
	
	if (((SPObjectClass *) (flowdiv_parent_class))->modified)
		((SPObjectClass *) (flowdiv_parent_class))->modified (object, flags);
	
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
	
	object_class = (GObjectClass *) klass;
	sp_object_class = (SPObjectClass *) klass;

	flowtspan_parent_class = (SPItemClass *)g_type_class_ref (SP_TYPE_ITEM);
	
	sp_object_class->write = sp_flowtspan_write;
	sp_object_class->modified = sp_flowtspan_modified;
}

static void
sp_flowtspan_init (SPFlowtspan */*group*/)
{
}

static void
sp_flowtspan_modified (SPObject *object, guint flags)
{
	SPFlowtspan *group;
	SPObject *child;
	GSList *l;
	
	group = SP_FLOWTSPAN (object);	
	
	if (((SPObjectClass *) (flowtspan_parent_class))->modified)
		((SPObjectClass *) (flowtspan_parent_class))->modified (object, flags);
	
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
	
	object_class = (GObjectClass *) klass;
	sp_object_class = (SPObjectClass *) klass;
	
	flowpara_parent_class = (SPItemClass *)g_type_class_ref (SP_TYPE_ITEM);
	
	sp_object_class->write = sp_flowpara_write;
	sp_object_class->modified = sp_flowpara_modified;
}

static void
sp_flowpara_init (SPFlowpara */*group*/)
{
}

static void
sp_flowpara_modified (SPObject *object, guint flags)
{
	SPFlowpara *group;
	SPObject *child;
	GSList *l;
	
	group = SP_FLOWPARA (object);	
	
	if (((SPObjectClass *) (flowpara_parent_class))->modified)
		((SPObjectClass *) (flowpara_parent_class))->modified (object, flags);

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

/*
 *
 */

GType
sp_flowline_get_type (void)
{
	static GType group_type = 0;
	if (!group_type) {
		GTypeInfo group_info = {
			sizeof (SPFlowlineClass),
			NULL,	/* base_init */
			NULL,	/* base_finalize */
			(GClassInitFunc) sp_flowline_class_init,
			NULL,	/* class_finalize */
			NULL,	/* class_data */
			sizeof (SPFlowline),
			16,	/* n_preallocs */
			(GInstanceInitFunc) sp_flowline_init,
			NULL,	/* value_table */
		};
		group_type = g_type_register_static (SP_TYPE_OBJECT, "SPFlowline", &group_info, (GTypeFlags)0);
	}
	return group_type;
}

static void
sp_flowline_class_init (SPFlowlineClass *klass)
{
	GObjectClass * object_class;
	SPObjectClass * sp_object_class;
	
	object_class = (GObjectClass *) klass;
	sp_object_class = (SPObjectClass *) klass;
	
	flowline_parent_class = (SPObjectClass *)g_type_class_ref (SP_TYPE_OBJECT);
	
	sp_object_class->write = sp_flowline_write;
	sp_object_class->modified = sp_flowline_modified;
}

static void
sp_flowline_init (SPFlowline */*group*/)
{
}

static void
sp_flowline_modified (SPObject *object, guint flags)
{
	SPFlowline *group;
	
	group = SP_FLOWLINE (object);	
	
	if (((SPObjectClass *) (flowline_parent_class))->modified)
		((SPObjectClass *) (flowline_parent_class))->modified (object, flags);
	
	if (flags & SP_OBJECT_MODIFIED_FLAG) flags |= SP_OBJECT_PARENT_MODIFIED_FLAG;
	flags &= SP_OBJECT_MODIFIED_CASCADE;
}
static SPRepr *
sp_flowline_write (SPObject *object, SPRepr *repr, guint flags)
{
	//	SPFlowline *group = SP_FLOWLINE (object);
	
	if ( flags&SP_OBJECT_WRITE_BUILD ) {
		if ( repr == NULL ) repr = sp_repr_new ("flowLine");
	} else {
	}
		
	if (((SPObjectClass *) (flowline_parent_class))->write)
		((SPObjectClass *) (flowline_parent_class))->write (object, repr, flags);
	
	return repr;
}

/*
 *
 */

GType
sp_flowregionbreak_get_type (void)
{
	static GType group_type = 0;
	if (!group_type) {
		GTypeInfo group_info = {
			sizeof (SPFlowregionbreakClass),
			NULL,	/* base_init */
			NULL,	/* base_finalize */
			(GClassInitFunc) sp_flowregionbreak_class_init,
			NULL,	/* class_finalize */
			NULL,	/* class_data */
			sizeof (SPFlowregionbreak),
			16,	/* n_preallocs */
			(GInstanceInitFunc) sp_flowregionbreak_init,
			NULL,	/* value_table */
		};
		group_type = g_type_register_static (SP_TYPE_OBJECT, "SPFlowregionbreak", &group_info, (GTypeFlags)0);
	}
	return group_type;
}

static void
sp_flowregionbreak_class_init (SPFlowregionbreakClass *klass)
{
	GObjectClass * object_class;
	SPObjectClass * sp_object_class;
	
	object_class = (GObjectClass *) klass;
	sp_object_class = (SPObjectClass *) klass;
	
	flowregionbreak_parent_class = (SPObjectClass *)g_type_class_ref (SP_TYPE_OBJECT);
	
	sp_object_class->write = sp_flowregionbreak_write;
	sp_object_class->modified = sp_flowregionbreak_modified;
}

static void
sp_flowregionbreak_init (SPFlowregionbreak */*group*/)
{
}

static void
sp_flowregionbreak_modified (SPObject *object, guint flags)
{
	SPFlowregionbreak *group;
	
	group = SP_FLOWREGIONBREAK (object);	
	
	if (((SPObjectClass *) (flowregionbreak_parent_class))->modified)
		((SPObjectClass *) (flowregionbreak_parent_class))->modified (object, flags);
	
	if (flags & SP_OBJECT_MODIFIED_FLAG) flags |= SP_OBJECT_PARENT_MODIFIED_FLAG;
	flags &= SP_OBJECT_MODIFIED_CASCADE;
}
static SPRepr *
sp_flowregionbreak_write (SPObject *object, SPRepr *repr, guint flags)
{
	//	SPFlowregionbreak *group = SP_FLOWREGIONBREAK (object);
	
	if ( flags&SP_OBJECT_WRITE_BUILD ) {
		if ( repr == NULL ) repr = sp_repr_new ("flowLine");
	} else {
	}
		
	if (((SPObjectClass *) (flowregionbreak_parent_class))->write)
		((SPObjectClass *) (flowregionbreak_parent_class))->write (object, repr, flags);
	
	return repr;
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
void                SPFlowline::CollectFlow(flow_src* collector)
{
	collector->AddControl(flw_line_brk);
}
void                SPFlowregionbreak::CollectFlow(flow_src* collector)
{
	collector->AddControl(flw_rgn_brk);
}

static void DoCollectFlow(SPObject* object,flow_src* collector,bool add_line_brk,bool add_rgn_brk)
{
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
		} else if ( SP_IS_FLOWLINE (child) ) {
			SPFlowline*       c_child=SP_FLOWLINE(child);
			c_child->CollectFlow(collector);
		} else if ( SP_IS_FLOWREGIONBREAK (child) ) {
			SPFlowregionbreak*       c_child=SP_FLOWREGIONBREAK(child);
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

