#define __SP_FLOWREGION_C__

/*
 */

#include <config.h>
#include <string.h>

#include <xml/repr.h>

#include "sp-object.h"
#include "sp-item.h"
#include "sp-shape.h"
#include "sp-text.h"
#include "sp-use.h"

#include "sp-flowregion.h"

#include "display/curve.h"

#include "libnr/nr-point.h"
#include "libnr/nr-matrix.h"
#include "libnr/nr-point-ops.h"
#include "libnr/nr-matrix-ops.h"
#include "libnr/nr-path.h"

#include "livarot/LivarotDefs.h"
#include "livarot/Path.h"
#include "livarot/Shape.h"
#include "livarot/Ligne.h"

static void sp_flowregion_class_init (SPFlowregionClass *klass);
static void sp_flowregion_init (SPFlowregion *group);
static void sp_flowregion_dispose (GObject *object);

static void sp_flowregion_child_added (SPObject * object, SPRepr * child, SPRepr * ref);
static void sp_flowregion_remove_child (SPObject * object, SPRepr * child);
static void sp_flowregion_update (SPObject *object, SPCtx *ctx, guint flags);
static void sp_flowregion_modified (SPObject *object, guint flags);
static SPRepr *sp_flowregion_write (SPObject *object, SPRepr *repr, guint flags);

static gchar * sp_flowregion_description (SPItem * item);

static SPItemClass * flowregion_parent_class;

static void sp_flowregionexclude_class_init (SPFlowregionExcludeClass *klass);
static void sp_flowregionexclude_init (SPFlowregionExclude *group);
static void sp_flowregionexclude_dispose (GObject *object);

static void sp_flowregionexclude_child_added (SPObject * object, SPRepr * child, SPRepr * ref);
static void sp_flowregionexclude_remove_child (SPObject * object, SPRepr * child);
static void sp_flowregionexclude_update (SPObject *object, SPCtx *ctx, guint flags);
static void sp_flowregionexclude_modified (SPObject *object, guint flags);
static SPRepr *sp_flowregionexclude_write (SPObject *object, SPRepr *repr, guint flags);

static gchar * sp_flowregionexclude_description (SPItem * item);

static SPItemClass * flowregionexclude_parent_class;

GType
sp_flowregion_get_type (void)
{
	static GType group_type = 0;
	if (!group_type) {
		GTypeInfo group_info = {
			sizeof (SPFlowregionClass),
			NULL,	/* base_init */
			NULL,	/* base_finalize */
			(GClassInitFunc) sp_flowregion_class_init,
			NULL,	/* class_finalize */
			NULL,	/* class_data */
			sizeof (SPFlowregion),
			16,	/* n_preallocs */
			(GInstanceInitFunc) sp_flowregion_init,
			NULL,	/* value_table */
		};
		group_type = g_type_register_static (SP_TYPE_ITEM, "SPFlowregion", &group_info, (GTypeFlags)0);
	}
	return group_type;
}

static void
sp_flowregion_class_init (SPFlowregionClass *klass)
{
	GObjectClass * object_class;
	SPObjectClass * sp_object_class;
	SPItemClass * item_class;
	
	object_class = (GObjectClass *) klass;
	sp_object_class = (SPObjectClass *) klass;
	item_class = (SPItemClass *) klass;
	
	flowregion_parent_class = (SPItemClass *)g_type_class_ref (SP_TYPE_ITEM);
	
	object_class->dispose = sp_flowregion_dispose;
	
	sp_object_class->child_added = sp_flowregion_child_added;
	sp_object_class->remove_child = sp_flowregion_remove_child;
	sp_object_class->update = sp_flowregion_update;
	sp_object_class->modified = sp_flowregion_modified;
	sp_object_class->write = sp_flowregion_write;
	
	item_class->description = sp_flowregion_description;
}

static void
sp_flowregion_init (SPFlowregion *group)
{
	group->computed=new flow_dest;
}

static void
sp_flowregion_dispose(GObject *object)
{
	SPFlowregion *group=(SPFlowregion *)object;
	delete group->computed;
}

static void
sp_flowregion_child_added (SPObject *object, SPRepr *child, SPRepr *ref)
{
	SPItem *item;
	
	item = SP_ITEM (object);
	
	if (((SPObjectClass *) (flowregion_parent_class))->child_added)
		(* ((SPObjectClass *) (flowregion_parent_class))->child_added) (object, child, ref);
	
	object->requestModified(SP_OBJECT_MODIFIED_FLAG);
}

/* fixme: hide (Lauris) */

static void
sp_flowregion_remove_child (SPObject * object, SPRepr * child)
{
	if (((SPObjectClass *) (flowregion_parent_class))->remove_child)
		(* ((SPObjectClass *) (flowregion_parent_class))->remove_child) (object, child);
	
	object->requestModified(SP_OBJECT_MODIFIED_FLAG);
}


static void
sp_flowregion_update (SPObject *object, SPCtx *ctx, unsigned int flags)
{
	SPFlowregion *group;
	SPObject *child;
	SPItemCtx *ictx, cctx;
	GSList *l;
	
	group = SP_FLOWREGION (object);
	ictx = (SPItemCtx *) ctx;
	cctx = *ictx;
	
	if (((SPObjectClass *) (flowregion_parent_class))->update)
		((SPObjectClass *) (flowregion_parent_class))->update (object, ctx, flags);
	
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

	group->UpdateComputed();
}
static void         CollectDest(SPObject* object,flow_dest* computed);

void             SPFlowregion::UpdateComputed(void)
{
	CollectDest(SP_OBJECT(this),computed);
}

static void
sp_flowregion_modified (SPObject *object, guint flags)
{
	SPFlowregion *group;
	SPObject *child;
	GSList *l;
	
	group = SP_FLOWREGION (object);	
	
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
sp_flowregion_write (SPObject *object, SPRepr *repr, guint flags)
{
	SPFlowregion *group;
	
	group = SP_FLOWREGION (object);
	
	if ( flags&SP_OBJECT_WRITE_BUILD ) {
		if ( repr == NULL ) repr = sp_repr_new ("flowRegion");
	}

	if (((SPObjectClass *) (flowregion_parent_class))->write)
		((SPObjectClass *) (flowregion_parent_class))->write (object, repr, flags);
	
	return repr;
}


static gchar * sp_flowregion_description (SPItem * item)
{
	SPFlowregion * group;
	
	group = SP_FLOWREGION (item);
	
	//	return g_strdup_printf(_("Flow region"));
	return g_strdup_printf("Flow region");
}

/*
 *
 */

GType
sp_flowregionexclude_get_type (void)
{
	static GType group_type = 0;
	if (!group_type) {
		GTypeInfo group_info = {
			sizeof (SPFlowregionExcludeClass),
			NULL,	/* base_init */
			NULL,	/* base_finalize */
			(GClassInitFunc) sp_flowregionexclude_class_init,
			NULL,	/* class_finalize */
			NULL,	/* class_data */
			sizeof (SPFlowregionExclude),
			16,	/* n_preallocs */
			(GInstanceInitFunc) sp_flowregionexclude_init,
			NULL,	/* value_table */
		};
		group_type = g_type_register_static (SP_TYPE_ITEM, "SPFlowregionExclude", &group_info, (GTypeFlags)0);
	}
	return group_type;
}

static void
sp_flowregionexclude_class_init (SPFlowregionExcludeClass *klass)
{
	GObjectClass * object_class;
	SPObjectClass * sp_object_class;
	SPItemClass * item_class;
	
	object_class = (GObjectClass *) klass;
	sp_object_class = (SPObjectClass *) klass;
	item_class = (SPItemClass *) klass;
	
	flowregionexclude_parent_class = (SPItemClass *)g_type_class_ref (SP_TYPE_ITEM);
	
	object_class->dispose = sp_flowregionexclude_dispose;
	
	sp_object_class->child_added = sp_flowregionexclude_child_added;
	sp_object_class->remove_child = sp_flowregionexclude_remove_child;
	sp_object_class->update = sp_flowregionexclude_update;
	sp_object_class->modified = sp_flowregionexclude_modified;
	sp_object_class->write = sp_flowregionexclude_write;
	
	item_class->description = sp_flowregionexclude_description;
}

static void
sp_flowregionexclude_init (SPFlowregionExclude *group)
{
	group->computed=new flow_dest;
}

static void
sp_flowregionexclude_dispose(GObject *object)
{
	SPFlowregionExclude *group=(SPFlowregionExclude *)object;
	delete group->computed;
}

static void
sp_flowregionexclude_child_added (SPObject *object, SPRepr *child, SPRepr *ref)
{
	SPItem *item;
	
	item = SP_ITEM (object);
	
	if (((SPObjectClass *) (flowregionexclude_parent_class))->child_added)
		(* ((SPObjectClass *) (flowregionexclude_parent_class))->child_added) (object, child, ref);
	
	object->requestModified(SP_OBJECT_MODIFIED_FLAG);
}

/* fixme: hide (Lauris) */

static void
sp_flowregionexclude_remove_child (SPObject * object, SPRepr * child)
{
	if (((SPObjectClass *) (flowregionexclude_parent_class))->remove_child)
		(* ((SPObjectClass *) (flowregionexclude_parent_class))->remove_child) (object, child);
	
	object->requestModified(SP_OBJECT_MODIFIED_FLAG);
}


static void
sp_flowregionexclude_update (SPObject *object, SPCtx *ctx, unsigned int flags)
{
	SPFlowregionExclude *group;
	SPObject *child;
	SPItemCtx *ictx, cctx;
	GSList *l;
	
	group = SP_FLOWREGIONEXCLUDE (object);
	ictx = (SPItemCtx *) ctx;
	cctx = *ictx;
		
	if (((SPObjectClass *) (flowregionexclude_parent_class))->update)
		((SPObjectClass *) (flowregionexclude_parent_class))->update (object, ctx, flags);
	
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

	group->UpdateComputed();
}
void             SPFlowregionExclude::UpdateComputed(void)
{
	CollectDest(SP_OBJECT(this),computed);
}

static void
sp_flowregionexclude_modified (SPObject *object, guint flags)
{
	SPFlowregionExclude *group;
	SPObject *child;
	GSList *l;
	
	group = SP_FLOWREGIONEXCLUDE (object);
	
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
sp_flowregionexclude_write (SPObject *object, SPRepr *repr, guint flags)
{
	SPFlowregionExclude *group;
	
	group = SP_FLOWREGIONEXCLUDE (object);
	
	if ( flags&SP_OBJECT_WRITE_BUILD ) {
		if ( repr == NULL ) repr = sp_repr_new ("flowRegionExclude");
	}
	
	if (((SPObjectClass *) (flowregionexclude_parent_class))->write)
		((SPObjectClass *) (flowregionexclude_parent_class))->write (object, repr, flags);
	
	return repr;
}


static gchar * sp_flowregionexclude_description (SPItem * item)
{
	SPFlowregionExclude * group;
	
	group = SP_FLOWREGIONEXCLUDE (item);
	
	//	return g_strdup_printf(_("Flow region"));
	return g_strdup_printf("Flow excluded region");
}

/*
 *
 */

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
	if ( rgn_dest->nbAr <= 0 ) {
		rgn_dest->Copy(i_shape);
	} else if ( i_shape->nbAr <= 0 ) {
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


static void         CollectDest(SPObject* object,flow_dest* computed)
{	
	computed->Reset();
	NR::Matrix itr_mat=sp_item_i2root_affine (SP_ITEM(object));
	itr_mat=itr_mat.inverse();
	
	for (SPObject* child = sp_object_first_child(object) ; child != NULL ; child = SP_OBJECT_NEXT(child) ) {
    SPCurve *curve=NULL;
		
		SPObject* u_child=child;
		if ( SP_IS_USE(u_child) ) {
			u_child=SP_USE(u_child)->child;
		}
		if ( SP_IS_SHAPE (u_child) ) {
			curve = sp_shape_get_curve (SP_SHAPE (u_child));
    } else if ( SP_IS_TEXT (u_child) ) {
			curve = sp_text_normalized_bpath (SP_TEXT (u_child));
    }
		
    if ( curve ) {
		  Path*   temp=new Path;
			NR::Matrix tr_mat=sp_item_i2root_affine (SP_ITEM(u_child));
			tr_mat=itr_mat*tr_mat;
			temp->LoadArtBPath(curve->bpath,tr_mat,true);
			Shape*  n_shp=new Shape;
			temp->Convert(0.25);
			temp->Fill(n_shp,0);
			Shape*  uncross=new Shape;
			uncross->ConvertToShape(n_shp,fill_nonZero);
			computed->AddShape(uncross);
			delete uncross;
			delete n_shp;
			delete temp;
			sp_curve_unref(curve);
		} else {
			printf("no curve\n");
		}
	}
}

