#define __sp_typeset_C__

/*
 * SVG <g> implementation
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * Copyright (C) 1999-2002 authors
 * Copyright (C) 2000-2001 Ximian, Inc.
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <config.h>
#include <string.h>

#include "display/nr-arena-group.h"
#include "xml/repr-private.h"
#include <libnr/nr-matrix.h>
#include <libnr/nr-matrix-ops.h>
#include "sp-object-repr.h"
#include "svg/svg.h"
#include "document.h"
#include "style.h"
#include "attributes.h"

#include "sp-root.h"
#include "sp-use.h"
#include "sp-typeset.h"
#include "helper/sp-intl.h"

#include "sp-text.h"
#include "sp-shape.h"

#include "display/curve.h"
#include "livarot/Path.h"
#include "livarot/Shape.h"
#include "livarot/LivarotDefs.h"

static void sp_typeset_class_init (SPTypesetClass *klass);
static void sp_typeset_init (SPTypeset *group);

static void sp_typeset_build (SPObject * object, SPDocument * document, SPRepr * repr);
static void sp_typeset_release (SPObject * object);
static void sp_typeset_child_added (SPObject * object, SPRepr * child, SPRepr * ref);
static void sp_typeset_remove_child (SPObject * object, SPRepr * child);
static void sp_typeset_order_changed (SPObject * object, SPRepr * child, SPRepr * old_ref, SPRepr * new_ref);
static void sp_typeset_update (SPObject *object, SPCtx *ctx, guint flags);
static void sp_typeset_set (SPObject *object, unsigned int key, const gchar *value);
static void sp_typeset_modified (SPObject *object, guint flags);
static SPRepr *sp_typeset_write (SPObject *object, SPRepr *repr, guint flags);

static void sp_typeset_bbox (SPItem *item, NRRect *bbox, const NRMatrix *transform, unsigned int flags);
static void sp_typeset_print (SPItem * item, SPPrintContext *ctx);
static gchar * sp_typeset_description (SPItem * item);
static NRArenaItem *sp_typeset_show (SPItem *item, NRArena *arena, unsigned int key, unsigned int flags);
static void sp_typeset_hide (SPItem * item, unsigned int key);


void   sp_typeset_relayout(SPTypeset *typeset);
void   sp_typeset_ditch_dest(SPTypeset *typeset);
void   refresh_typeset_source(SPTypeset *typeset,shape_dest *nDst);
void   refresh_typeset_source(SPTypeset *typeset,path_dest *nDst);

static void sp_typeset_source_attr_changed (SPRepr * repr, const gchar * key,
                                           const gchar * oldval,
                                           const gchar * newval,
                                           bool is_interactive, void * data);
static void sp_typeset_source_destroy (SPRepr * repr, void *data);
static void sp_typeset_source_child_added (SPRepr *repr, SPRepr *child, SPRepr *ref, void * data);
static void sp_typeset_source_child_removed (SPRepr *repr, SPRepr *child, SPRepr *ref, void * data);
static void sp_typeset_source_content_changed (SPRepr *repr, const gchar *oldcontent, const gchar *newcontent, void * data);

// the regular listener vector
SPReprEventVector typeset_source_event_vector = {
  sp_typeset_source_destroy,
  NULL,				/* Add child */
  sp_typeset_source_child_added,
  NULL,
  sp_typeset_source_child_removed,				/* Child removed */
  NULL,
  sp_typeset_source_attr_changed,
  NULL,				/* Change content */
  sp_typeset_source_content_changed,
  NULL,				/* change_order */
  NULL
};


static SPGroupClass * parent_class;

GType
sp_typeset_get_type (void)
{
	static GType typeset_type = 0;
	if (!typeset_type) {
		GTypeInfo typeset_info = {
			sizeof (SPTypesetClass),
			NULL,	/* base_init */
			NULL,	/* base_finalize */
			(GClassInitFunc) sp_typeset_class_init,
			NULL,	/* class_finalize */
			NULL,	/* class_data */
			sizeof (SPTypeset),
			16,	/* n_preallocs */
			(GInstanceInitFunc) sp_typeset_init,
			NULL,	/* value_table */
		};
		typeset_type = g_type_register_static (SP_TYPE_GROUP, "SPTypeset", &typeset_info, (GTypeFlags)0);
	}
	return typeset_type;
}

static void
sp_typeset_class_init (SPTypesetClass *klass)
{
	GObjectClass * object_class;
	SPObjectClass * sp_object_class;
	SPItemClass * item_class;

	object_class = (GObjectClass *) klass;
	sp_object_class = (SPObjectClass *) klass;
	item_class = (SPItemClass *) klass;

  parent_class = (SPGroupClass *) g_type_class_ref (SP_TYPE_GROUP);

	sp_object_class->build = sp_typeset_build;
	sp_object_class->release = sp_typeset_release;
	sp_object_class->child_added = sp_typeset_child_added;
	sp_object_class->remove_child = sp_typeset_remove_child;
	sp_object_class->order_changed = sp_typeset_order_changed;
	sp_object_class->set = sp_typeset_set;
	sp_object_class->update = sp_typeset_update;
	sp_object_class->modified = sp_typeset_modified;
	sp_object_class->write = sp_typeset_write;

	item_class->bbox = sp_typeset_bbox;
	item_class->print = sp_typeset_print;
	item_class->description = sp_typeset_description;
	item_class->show = sp_typeset_show;
	item_class->hide = sp_typeset_hide;
}

static void
sp_typeset_init (SPTypeset *object)
{
	SPTypeset *typeset;
  
	typeset = SP_TYPESET (object);
  
  typeset->srcType=has_no_src;
  typeset->srcText=NULL;
  typeset->dstType=has_no_dest;
  typeset->dstElems=NULL;
  
  typeset->layoutDirty=false;
  typeset->theSrc=NULL;
  typeset->theDst=NULL;
}
static void
sp_typeset_release (SPObject *object)
{
	SPTypeset *typeset;
  
	typeset = SP_TYPESET (object);
  
  typeset->srcType=has_no_src;
  if (typeset->srcText) free(typeset->srcText);
  sp_typeset_ditch_dest(typeset);
  if ( typeset->theSrc ) delete typeset->theSrc;
  if ( typeset->theDst ) delete typeset->theDst;
  
  if (((SPObjectClass *) parent_class)->release) {
    ((SPObjectClass *) parent_class)->release (object);
  }
}

static void sp_typeset_build (SPObject *object, SPDocument * document, SPRepr * repr)
{
	SPTypeset *typeset;
  
	typeset = SP_TYPESET (object);
  
	sp_object_read_attr (object, "inkscape:srcNoMarkup");
	sp_object_read_attr (object, "inkscape:srcPango");
	sp_object_read_attr (object, "inkscape:dstShape");
	sp_object_read_attr (object, "inkscape:dstPath");
	sp_object_read_attr (object, "inkscape:dstBox");
	sp_object_read_attr (object, "inkscape:dstColumn");

	if (((SPObjectClass *) (parent_class))->build)
		(* ((SPObjectClass *) (parent_class))->build) (object, document, repr);
}

static void
sp_typeset_child_added (SPObject *object, SPRepr *child, SPRepr *ref)
{
	if (((SPObjectClass *) (parent_class))->child_added)
		(* ((SPObjectClass *) (parent_class))->child_added) (object, child, ref);
}

/* fixme: hide (Lauris) */

static void
sp_typeset_remove_child (SPObject * object, SPRepr * child)
{
	if (((SPObjectClass *) (parent_class))->remove_child)
		(* ((SPObjectClass *) (parent_class))->remove_child) (object, child);
}

static void
sp_typeset_order_changed (SPObject *object, SPRepr *child, SPRepr *old_ref, SPRepr *new_ref)
{
	if (((SPObjectClass *) (parent_class))->order_changed)
		(* ((SPObjectClass *) (parent_class))->order_changed) (object, child, old_ref, new_ref);
}

static void
sp_typeset_update (SPObject *object, SPCtx *ctx, unsigned int flags)
{
	SPTypeset *typeset;

	typeset = SP_TYPESET (object);

  if ( typeset->layoutDirty ) {
    if ( typeset->dstType == has_shape_dest ) {
      GSList* l=typeset->dstElems;
      while ( l ) {
        shape_dest* theData=(shape_dest*)l->data;
        if ( theData->originalObj == NULL ) {
          // need to resolve it
          refresh_typeset_source(typeset,theData);
          if ( theData->originalObj ) sp_repr_add_listener (theData->originalObj, &typeset_source_event_vector, typeset);
        }
        l=l->next;
      }
    } else if ( typeset->dstType == has_path_dest ) {
      GSList* l=typeset->dstElems;
      while ( l ) {
        path_dest* theData=(path_dest*)l->data;
        if ( theData->originalObj == NULL ) {
          // need to resolve it
          refresh_typeset_source(typeset,theData);
          if ( theData->originalObj ) sp_repr_add_listener (theData->originalObj, &typeset_source_event_vector, typeset);
        }
        l=l->next;
      }
    }
    
    sp_typeset_relayout(typeset);
    typeset->layoutDirty=false;
  }
  
	if (((SPObjectClass *) (parent_class))->update)
		((SPObjectClass *) (parent_class))->update (object, ctx, flags);
}

static void
sp_typeset_modified (SPObject *object, guint flags)
{
	SPTypeset *group;

	group = SP_TYPESET (object);

	if (flags & SP_OBJECT_MODIFIED_FLAG) flags |= SP_OBJECT_PARENT_MODIFIED_FLAG;
	flags &= SP_OBJECT_MODIFIED_CASCADE;
}

static SPRepr *
sp_typeset_write (SPObject *object, SPRepr *repr, guint flags)
{
	SPTypeset *group;

	group = SP_TYPESET (object);

	if (((SPObjectClass *) (parent_class))->write)
		((SPObjectClass *) (parent_class))->write (object, repr, flags);

	return repr;
}

static void
sp_typeset_bbox (SPItem *item, NRRect *bbox, const NRMatrix *transform, unsigned int flags)
{
	SPTypeset * group;

	group = SP_TYPESET (item);

	if (((SPItemClass *) (parent_class))->bbox)
		((SPItemClass *) (parent_class))->bbox (item, bbox,transform, flags);
}

static void
sp_typeset_print (SPItem * item, SPPrintContext *ctx)
{
	SPTypeset * group;

	group = SP_TYPESET (item);

	if (((SPItemClass *) (parent_class))->print)
		((SPItemClass *) (parent_class))->print (item, ctx);
}

static gchar * sp_typeset_description (SPItem * item)
{
	SPTypeset * group;

	group = SP_TYPESET (item);

	if (((SPItemClass *) (parent_class))->description)
		return ((SPItemClass *) (parent_class))->description (item);

	return g_strdup_printf(_("typeset object"));
}

static NRArenaItem *
sp_typeset_show (SPItem *item, NRArena *arena, unsigned int key, unsigned int flags)
{
	SPTypeset *group;

	group = (SPTypeset *) item;

	if (((SPItemClass *) (parent_class))->show)
		return ((SPItemClass *) (parent_class))->show (item,arena,key,flags);

	return NULL;
}

static void
sp_typeset_hide (SPItem *item, unsigned int key)
{
	SPTypeset * group;

	group = (SPTypeset *) item;

	if (((SPItemClass *) parent_class)->hide)
		((SPItemClass *) parent_class)->hide (item, key);
}


void
sp_typeset_set (SPObject *object, unsigned int key, const gchar *value)
{
	SPTypeset *typeset = SP_TYPESET (object);

	switch (key) {
    case SP_ATTR_TEXT_NOMARKUP:
      if ( value ) {
        if ( typeset->srcText ) {
          free(typeset->srcText);
        } else {
        }
        typeset->srcText=strdup(value);
        printf("typeset %x: n src= %s\n",object,value);
        typeset->layoutDirty=true;
        typeset->srcType=has_std_txt;
      } else {
        if ( typeset->srcType == has_std_txt ) {
          if ( typeset->srcText ) {
            free(typeset->srcText);
          } else {
          }
          typeset->srcText=NULL;
          typeset->srcType=has_no_src;
        } else {
        }
      }
      break;
    case SP_ATTR_TEXT_PANGOMARKUP:
      if ( value ) {
        if ( typeset->srcText ) {
          free(typeset->srcText);
        } else {
        }
        typeset->srcText=strdup(value);
        printf("typeset %x: n src= %s\n",object,value);
        typeset->layoutDirty=true;
       typeset->srcType=has_pango_txt;
      } else {
        if ( typeset->srcType == has_pango_txt ) {
          if ( typeset->srcText ) {
            free(typeset->srcText);
          } else {
          }
          typeset->srcText=NULL;
          typeset->srcType=has_no_src;
        } else {
        }
      }
      break;
    case SP_ATTR_TEXT_INSHAPE:
      if ( value ) {
        sp_typeset_ditch_dest(typeset);
        printf("typeset %x: n shape dst= %s\n",object,value);
        char*  dup_value=strdup(value);
        int pos=0,max=strlen(dup_value);
        while ( pos < max ) {
          while ( pos < max && dup_value[pos] != '[' ) pos++;
          int spos=pos+1,epos=pos+1;
          while ( epos < max && dup_value[epos] != ']' ) epos++;
          if ( epos < max ) {
            dup_value[epos]=0;
//            SPObject *refobj = sp_document_lookup_id (SP_OBJECT (typeset)->document, dup_value+spos);
//            if ( refobj && refobj->repr )  {
              shape_dest  *nSrc=(shape_dest*)malloc(sizeof(shape_dest));
 //             nSrc->originalObj = refobj->repr;
              nSrc->originalObj = NULL;
              nSrc->originalID = strdup(dup_value+spos);
              nSrc->windingRule=fill_nonZero;
              nSrc->theShape=NULL;
              nSrc->bbox=NR::Rect(NR::Point(0,0),NR::Point(0,0));
 //             sp_repr_add_listener (nSrc->originalObj, &typeset_source_event_vector, typeset);
              typeset->dstElems=g_slist_append(typeset->dstElems,nSrc);
              typeset->dstType=has_shape_dest;
              typeset->layoutDirty=true;
              
 //             refresh_typeset_source(typeset,nSrc);
 //           } else {
 //           }
            dup_value[epos]=']';
          }
          while ( pos < max && dup_value[pos] != ']' ) pos++;
        }
        free(dup_value);
      } else {
        if ( typeset->dstType == has_shape_dest ) {
          sp_typeset_ditch_dest(typeset);
        }
      }
      break;
    case SP_ATTR_TEXT_ONPATH:
      if ( value ) {
        sp_typeset_ditch_dest(typeset);
        printf("typeset %x: n path dst= %s\n",object,value);
        char *dup_value=strdup(value);
        int pos=0,max=strlen(dup_value);
        while ( pos < max ) {
          while ( pos < max && dup_value[pos] != '[' ) pos++;
          int spos=pos+1,epos=pos+1;
          while ( epos < max && dup_value[epos] != ']' ) epos++;
          if ( epos < max ) {
            dup_value[epos]=0;
//            SPObject *refobj = sp_document_lookup_id (SP_OBJECT (typeset)->document, dup_value+spos);
//            if ( refobj && refobj->repr )  {
              path_dest  *nSrc=(path_dest*)malloc(sizeof(path_dest));
//              nSrc->originalObj = refobj->repr;
              nSrc->originalObj = NULL;
              nSrc->originalID = strdup(dup_value+spos);
              nSrc->thePath=NULL;
              nSrc->length=0;
 //             sp_repr_add_listener (nSrc->originalObj, &typeset_source_event_vector, typeset);
              typeset->dstElems=g_slist_append(typeset->dstElems,nSrc);
              typeset->dstType=has_path_dest;
              
//              refresh_typeset_source(typeset,nSrc);
              typeset->layoutDirty=true;
//            } else {
//            }
            dup_value[epos]=']';
          }
          while ( pos < max && dup_value[pos] != ']' ) pos++;
        }
        free(dup_value);
      } else {
        if ( typeset->dstType == has_path_dest ) {
          sp_typeset_ditch_dest(typeset);
        }
      }
      break;
    case SP_ATTR_TEXT_INBOX:
      if ( value ) {
        sp_typeset_ditch_dest(typeset);
        printf("typeset %x: n box dst= %s\n",object,value);
        int pos=0,max=strlen(value);
        while ( pos < max ) {
          while ( pos < max && value[pos] != '[' ) pos++;
          double    l,r,t,b;
          int match=sscanf(value+(pos+1),"%lf %lf %lf %lf",&l,&t,&r,&b);
          if ( match == 4 ) {
            box_dest* nDst=(box_dest*)malloc(sizeof(box_dest));
            nDst->box=NR::Rect(NR::Point(l,t),NR::Point(r,b));
            typeset->layoutDirty=true;
            typeset->dstElems=g_slist_append(typeset->dstElems,nDst);
            typeset->dstType=has_box_dest;
          } else {
            break;
          }
          while ( pos < max && value[pos] != ']' ) pos++;
        }
      } else {
        if ( typeset->dstType == has_box_dest ) {
          sp_typeset_ditch_dest(typeset);
        }
      }
      break;
    case SP_ATTR_TEXT_INCOLUMN:
      if ( value ) {
        sp_typeset_ditch_dest(typeset);
        printf("typeset %x: n col dst= %s\n",object,value);
        column_dest* nDst=(column_dest*)malloc(sizeof(column_dest));
        nDst->width=0;
        sscanf(value,"%lf",&nDst->width);
        typeset->layoutDirty=true;
        typeset->dstElems=g_slist_append(typeset->dstElems,nDst);
        typeset->dstType=has_col_dest;
      } else {
        if ( typeset->dstType == has_col_dest ) {
          sp_typeset_ditch_dest(typeset);
        }
      }
      break;
    default:
      if (((SPObjectClass *) parent_class)->set) {
        ((SPObjectClass *) parent_class)->set (object, key, value);
      }
      break;
	}
  if ( typeset->layoutDirty ) {
    sp_object_request_update (SP_OBJECT(typeset), SP_OBJECT_MODIFIED_FLAG);
  }
}
void sp_typeset_ditch_dest(SPTypeset *typeset)
{
  printf("ditch\n");
  if ( typeset->dstType == has_shape_dest ) {
    GSList* l=typeset->dstElems;
    while ( l ) {
      shape_dest* cur=(shape_dest*)l->data;
      if ( cur->theShape ) delete cur->theShape;
      if ( cur->originalID ) free(cur->originalID);
      if ( cur->originalObj ) sp_repr_remove_listener_by_data (cur->originalObj, typeset);
      l=l->next;
    }
    while ( typeset->dstElems ) {
      void* cur=(void*)typeset->dstElems->data;
      typeset->dstElems=g_slist_remove(typeset->dstElems,cur);
      free(cur);
    }
  } else if ( typeset->dstType == has_path_dest ) {
    GSList* l=typeset->dstElems;
    while ( l ) {
      path_dest* cur=(path_dest*)l->data;
      if ( cur->thePath ) delete cur->thePath;
      if ( cur->originalID ) free(cur->originalID);
      if ( cur->originalObj ) sp_repr_remove_listener_by_data (cur->originalObj, typeset);
      l=l->next;
    }
    while ( typeset->dstElems ) {
      void* cur=(void*)typeset->dstElems->data;
      typeset->dstElems=g_slist_remove(typeset->dstElems,cur);
      free(cur);
    }
  } else if ( typeset->dstType != has_no_dest ) {
    while ( typeset->dstElems ) {
      void* cur=(void*)typeset->dstElems->data;
      typeset->dstElems=g_slist_remove(typeset->dstElems,cur);
      free(cur);
    }
  }
  typeset->dstType=has_no_dest;
  typeset->layoutDirty=true;
}

// the listening functions
static void sp_typeset_source_attr_changed (SPRepr * repr, const gchar * key,
                               const gchar * /*oldval*/, const gchar * newval,
                               bool is_interactive, void * data)
{
  SPTypeset *typeset = (SPTypeset *) data;
  printf("attrchg %x of %x to %s\n",repr,typeset,newval);
  if ( typeset == NULL || repr == NULL ) return;
  
  if ( typeset->dstType == has_shape_dest ) {
    GSList *l=typeset->dstElems;
    while ( l ) {
      shape_dest *cur=(shape_dest*)(l->data);
      if ( cur->originalObj == repr ) break;
      l=l->next;
    }
    if ( l ) {
      shape_dest *cur=(shape_dest*)(l->data);
      refresh_typeset_source(typeset,cur);
    } else {
      // repr not used in this typeset
      printf("not fount\n");
    }
  } else if ( typeset->dstType == has_path_dest ) {
    GSList *l=typeset->dstElems;
    while ( l ) {
      path_dest *cur=(path_dest*)l->data;
      if ( cur->originalObj == repr ) break;
      l=l->next;
    }
    if ( l ) {
      path_dest *cur=(path_dest*)l->data;
      refresh_typeset_source(typeset,cur);
    } else {
      // repr not used in this typeset
      printf("not fount\n");
    }
  }
}

static void sp_typeset_source_destroy (SPRepr * repr, void *data)
{
  SPTypeset *typeset = (SPTypeset *) data;
  printf("destroy %x of %x\n",repr,typeset);
//  printf("destroy %x tps=%x\n",repr,data);
  if ( typeset == NULL ) return;
  if ( repr == NULL ) return;
  if ( typeset->dstType == has_shape_dest ) {
    GSList *l=typeset->dstElems;
    while ( l ) {
      shape_dest *cur=(shape_dest*)l->data;
      if ( cur->originalObj == repr ) break;
      l=l->next;
    }
    if ( l ) {
      shape_dest *cur=(shape_dest*)l->data;
      if ( cur->theShape ) delete cur->theShape;
      if ( cur->originalID ) free(cur->originalID);
      if ( cur->originalObj ) sp_repr_remove_listener_by_data (cur->originalObj, typeset);
      typeset->dstElems=g_slist_remove(typeset->dstElems,cur);
      free(cur);
      typeset->layoutDirty=true;
      sp_object_request_update (SP_OBJECT(typeset), SP_OBJECT_MODIFIED_FLAG);
    } else {
      // repr not used in this typeset
      printf("not fount\n");
    }
  } else if ( typeset->dstType == has_path_dest ) {
    GSList *l=typeset->dstElems;
    while ( l ) {
      path_dest *cur=(path_dest*)l->data;
      if ( cur->originalObj == repr ) break;
      l=l->next;
    }
    if ( l ) {
      path_dest *cur=(path_dest*)l->data;
      if ( cur->thePath) delete cur->thePath;
      if ( cur->originalID ) free(cur->originalID);
      if ( cur->originalObj ) sp_repr_remove_listener_by_data (cur->originalObj, typeset);
      typeset->dstElems=g_slist_remove(typeset->dstElems,cur);
      free(cur);
      typeset->layoutDirty=true;
      sp_object_request_update (SP_OBJECT(typeset), SP_OBJECT_MODIFIED_FLAG);
    } else {
      // repr not used in this typeset
      printf("not fount\n");
    }
  }
}
static void sp_typeset_source_child_added (SPRepr *repr, SPRepr *child, SPRepr */*ref*/, void * data)
{
//  printf("addchild %x tps=%x\n",repr,data);
  SPTypeset *typeset = (SPTypeset *) data;
  if (typeset == NULL) return;
  if ( child == NULL ) return;
  if ( repr == NULL ) return; // juste le premier niveau.
}
static void sp_typeset_source_child_removed (SPRepr *repr, SPRepr *child, SPRepr */*ref*/, void * data)
{
//  printf("remchild %x tps=%x\n",repr,data);
  SPTypeset *typeset = (SPTypeset *) data;
  if (typeset == NULL) return;
  if ( child == NULL ) return;
  if ( repr == NULL ) return; // juste le premier niveau.
}
static void sp_typeset_source_content_changed (SPRepr *repr, const gchar */*oldcontent*/, const gchar */*newcontent*/, void * data)
{
//  printf("chgcont %x tps=%x\n",repr,data);
  SPTypeset *typeset = (SPTypeset *) data;
  if (typeset == NULL) return;
}

void   refresh_typeset_source(SPTypeset *typeset,shape_dest *nDst)
{
  if ( nDst == NULL ) return;
  
  SPObject *refobj = sp_document_lookup_id (SP_OBJECT (typeset)->document, nDst->originalID);
  nDst->originalObj=NULL;
  if ( refobj == NULL ) return;
  nDst->originalObj=refobj->repr;
  
  SPItem *item = SP_ITEM (refobj);
  
  SPCurve *curve=NULL;
  if (!SP_IS_SHAPE (item) && !SP_IS_TEXT (item)) return;
  if (SP_IS_SHAPE (item)) {
    curve = sp_shape_get_curve (SP_SHAPE (item));
    if (curve == NULL)  return;
  }
  if (SP_IS_TEXT (item)) {
 	  curve = sp_text_normalized_bpath (SP_TEXT (item));
 	  if (curve == NULL) return;
  }
  
  Path *orig = new Path;
  {
    NR::Matrix dummy;
    orig->LoadArtBPath (curve->bpath,dummy,false);
  }
  sp_curve_unref (curve);
  
  if ( nDst->theShape == NULL ) nDst->theShape=new Shape;
  // finalisons
  {
    SPCSSAttr *css;
    const gchar *val;
    Shape *theTemp = new Shape;
    
    orig->Convert (1.0);
    orig->Fill (theTemp, 0);
    
    css = sp_repr_css_attr (nDst->originalObj , "style");
    val = sp_repr_css_property (css, "fill-rule", NULL);
    if (val && strcmp (val, "nonzero") == 0) {
      nDst->windingRule=fill_nonZero;
      nDst->theShape->ConvertToShape (theTemp, fill_nonZero);
    } else if (val && strcmp (val, "evenodd") == 0) {
      nDst->windingRule=fill_oddEven;
      nDst->theShape->ConvertToShape (theTemp, fill_oddEven);
    } else {
      nDst->windingRule=fill_nonZero;
      nDst->theShape->ConvertToShape (theTemp, fill_nonZero);
    }
    
    delete theTemp;
  }
  delete orig;
  nDst->theShape->CalcBBox();
  nDst->bbox=NR::Rect(NR::Point(nDst->theShape->leftX,nDst->theShape->topY),NR::Point(nDst->theShape->rightX,nDst->theShape->bottomY));
  typeset->layoutDirty=true;
  
  sp_object_request_update (SP_OBJECT(typeset), SP_OBJECT_MODIFIED_FLAG);
}

void   refresh_typeset_source(SPTypeset *typeset,path_dest *nDst)
{
  if ( nDst == NULL ) return;
  
  SPObject *refobj = sp_document_lookup_id (SP_OBJECT (typeset)->document, nDst->originalID);
  nDst->originalObj=NULL;
  if ( refobj == NULL ) return;
  nDst->originalObj=refobj->repr;

  SPItem *item = SP_ITEM (refobj);
  
  SPCurve *curve=NULL;
  if (!SP_IS_SHAPE (item) && !SP_IS_TEXT (item)) return;
  if (SP_IS_SHAPE (item)) {
    curve = sp_shape_get_curve (SP_SHAPE (item));
    if (curve == NULL)  return;
  }
  if (SP_IS_TEXT (item)) {
 	  curve = sp_text_normalized_bpath (SP_TEXT (item));
 	  if (curve == NULL) return;
  }
  
  if ( nDst->thePath ) nDst->thePath->Reset(); else nDst->thePath=new Path;
  {
    NR::Matrix dummy;
    nDst->thePath->LoadArtBPath (curve->bpath,dummy,false);
  }
  sp_curve_unref (curve);
  nDst->length=nDst->thePath->Length();
  typeset->layoutDirty=true;

  sp_object_request_update (SP_OBJECT(typeset), SP_OBJECT_MODIFIED_FLAG);
}
