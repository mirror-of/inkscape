#define __sp_typeset_C__

/*
 * based on SVG <g> implementation
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

#include "document.h"
#include "selection.h"
#include "inkscape.h"
#include "desktop.h"
#include "desktop-handles.h"

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

static void sp_typeset_bbox(SPItem *item, NRRect *bbox, NR::Matrix const &transform, unsigned const flags);
static void sp_typeset_print (SPItem * item, SPPrintContext *ctx);
static gchar * sp_typeset_description (SPItem * item);
static NRArenaItem *sp_typeset_show (SPItem *item, NRArena *arena, unsigned int key, unsigned int flags);
static void sp_typeset_hide (SPItem * item, unsigned int key);


void   sp_typeset_relayout(SPTypeset *typeset);
void   sp_typeset_rekplayout(SPTypeset *typeset);

void   sp_typeset_ditch_dest(SPTypeset *typeset);
void   sp_typeset_ditch_excl(SPTypeset *typeset);
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
  
  typeset->excluded=NULL;
  typeset->exclElems=NULL;
  
  typeset->layoutDirty=false;
  typeset->destDirty=false;
  typeset->exclDirty=false;
  typeset->stdLayoutAlgo=true;
  
  typeset->theSrc=NULL;
  typeset->theDst=NULL;
  
  typeset->justify=true;
  typeset->centering=0;
}
static void
sp_typeset_release (SPObject *object)
{
	SPTypeset *typeset;
  
	typeset = SP_TYPESET (object);
  
  typeset->srcType=has_no_src;
  if (typeset->srcText) free(typeset->srcText);
  sp_typeset_ditch_dest(typeset);
  sp_typeset_ditch_excl(typeset);
  if ( typeset->excluded ) delete typeset->excluded;
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
	sp_object_read_attr (object, "inkscape:excludeShape");
	sp_object_read_attr (object, "inkscape:dstPath");
	sp_object_read_attr (object, "inkscape:dstBox");
	sp_object_read_attr (object, "inkscape:dstColumn");
	sp_object_read_attr (object, "inkscape:layoutOptions");

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

  // refresh excluded
  {
    GSList* l=typeset->exclElems;
    while ( l ) {
      shape_dest* theData=(shape_dest*)l->data;
      if ( theData->originalObj == NULL || typeset->exclDirty ) {
        // need to resolve it
        bool add_listener=(theData->originalObj == NULL);
        refresh_typeset_source(typeset,theData);
        if ( add_listener && theData->originalObj ) {
          sp_repr_add_listener (theData->originalObj, &typeset_source_event_vector, typeset);
        }
      }
      l=l->next;
    }
    typeset->exclDirty=false;
    if ( typeset->exclElems == NULL ) {
      if ( typeset->excluded ) delete typeset->excluded;
      typeset->excluded=NULL;
    } else {
      if ( typeset->excluded == NULL ) typeset->excluded=new Shape;
      typeset->excluded->Reset();
      l=typeset->exclElems;
      Shape* temp=new Shape;
      while ( l ) {
        shape_dest* theData=(shape_dest*)l->data;
        if ( theData->theShape ) {
          if ( typeset->excluded->nbPt <= 1 || typeset->excluded->nbAr <= 1 ) {
            typeset->excluded->Copy(theData->theShape);
          } else {
            temp->Booleen(typeset->excluded,theData->theShape,bool_op_union);
            Shape *swap=temp;temp=typeset->excluded;typeset->excluded=swap;
          }
        }
        l=l->next;
      }
      delete temp;
    }
  }
  {
    if ( typeset->dstType == has_shape_dest ) {
      GSList* l=typeset->dstElems;
      while ( l ) {
        shape_dest* theData=(shape_dest*)l->data;
        if ( theData->originalObj == NULL || typeset->destDirty ) {
          // need to resolve it
          bool add_listener=(theData->originalObj == NULL);
          refresh_typeset_source(typeset,theData);
          if ( add_listener && theData->originalObj ) {
            sp_repr_add_listener (theData->originalObj, &typeset_source_event_vector, typeset);
          }
        }
        l=l->next;
      }
    } else if ( typeset->dstType == has_path_dest ) {
      GSList* l=typeset->dstElems;
      while ( l ) {
        path_dest* theData=(path_dest*)l->data;
        if ( theData->originalObj == NULL || typeset->destDirty ) {
          // need to resolve it
          bool add_listener=(theData->originalObj == NULL);
          refresh_typeset_source(typeset,theData);
          if ( add_listener && theData->originalObj ) sp_repr_add_listener (theData->originalObj, &typeset_source_event_vector, typeset);
        }
        l=l->next;
      }
    }
    typeset->destDirty=false;
  }
  if ( typeset->layoutDirty ) {
    if ( SP_IS_TYPESET(SP_OBJECT_PARENT(typeset)) ) {
      sp_object_request_update (SP_OBJECT(SP_OBJECT_PARENT(typeset)), SP_OBJECT_MODIFIED_FLAG);
    } else {
      if ( typeset->stdLayoutAlgo ) {
        sp_typeset_relayout(typeset);
      } else {
        sp_typeset_rekplayout(typeset);
      }
    }
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
sp_typeset_bbox(SPItem *item, NRRect *bbox, NR::Matrix const &transform, unsigned const flags)
{
	if (((SPItemClass *) (parent_class))->bbox)
		((SPItemClass *) (parent_class))->bbox(item, bbox, transform, flags);
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
//        printf("typeset %x: n src= %s\n",(int)object,value);
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
//        printf("typeset %x: n src= %s\n",(int)object,value);
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
//        printf("typeset %x: n shape dst= %s\n",(int)object,value);
        char*  dup_value=strdup(value);
        int pos=0,max=strlen(dup_value);
        while ( pos < max ) {
          while ( pos < max && dup_value[pos] != '#' ) {
            pos++;
          }
          int spos=pos+1,epos=pos+1;
          while ( epos < max ) {
            char nc=dup_value[epos];
            if ( ( nc >= '0' && nc <= '9' ) || ( nc >= 'a' && nc <= 'z' ) || ( nc >= 'A' && nc <= 'Z' ) || nc == '_' ) {
            } else {
              break;
            }
            epos++;
          }
          if ( epos <= max ) {
            char savC=dup_value[epos];
            dup_value[epos]=0;
            shape_dest  *nSrc=(shape_dest*)malloc(sizeof(shape_dest));
            nSrc->originalObj = NULL;
            nSrc->originalID = strdup(dup_value+spos);
            nSrc->windingRule=fill_nonZero;
            nSrc->theShape=NULL;
            nSrc->bbox=NR::Rect(NR::Point(0,0),NR::Point(0,0));
            typeset->dstElems=g_slist_append(typeset->dstElems,nSrc);
            typeset->dstType=has_shape_dest;
            typeset->layoutDirty=true;
            dup_value[epos]=savC;
          }
          pos=epos;
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
//        printf("typeset %x: n path dst= %s\n",(int)object,value);
        char *dup_value=strdup(value);
        int  pos=0,max=strlen(dup_value);
        while ( pos < max ) {
          while ( pos < max && dup_value[pos] != '#' ) {
            pos++;
          }
          int spos=pos+1,epos=pos+1;
          while ( epos <= max ) {
            char nc=dup_value[epos];
            if ( ( nc >= '0' && nc <= '9' ) || ( nc >= 'a' && nc <= 'z' ) || ( nc >= 'A' && nc <= 'Z' ) || nc == '_' ) {
            } else {
              break;
            }
            epos++;
          }
          if ( epos <= max ) {
            char savC=dup_value[epos];
            dup_value[epos]=0;
            path_dest  *nSrc=(path_dest*)malloc(sizeof(path_dest));
            nSrc->originalObj = NULL;
            nSrc->originalID = strdup(dup_value+spos);
            nSrc->thePath=NULL;
            nSrc->length=0;
            typeset->dstElems=g_slist_append(typeset->dstElems,nSrc);
            typeset->dstType=has_path_dest;
            
            typeset->layoutDirty=true;
            dup_value[epos]=savC;
          }
          pos=epos;
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
//        printf("typeset %x: n box dst= %s\n",(int)object,value);
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
//        printf("typeset %x: n col dst= %s\n",(int)object,value);
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
    case SP_ATTR_TEXT_EXCLUDE:
      if ( value ) {
        sp_typeset_ditch_excl(typeset);
//        printf("typeset %x: n excl dst= %s\n",(int)object,value);
        char*  dup_value=strdup(value);
        int pos=0,max=strlen(dup_value);
        while ( pos < max ) {
          while ( pos < max && dup_value[pos] != '#' ) {
            pos++;
          }
          int spos=pos+1,epos=pos+1;
          while ( epos < max ) {
            char nc=dup_value[epos];
            if ( ( nc >= '0' && nc <= '9' ) || ( nc >= 'a' && nc <= 'z' ) || ( nc >= 'A' && nc <= 'Z' ) || nc == '_' ) {
            } else {
              break;
            }
            epos++;
          }
          if ( epos <= max ) {
            char savC=dup_value[epos];
            dup_value[epos]=0;
            shape_dest  *nSrc=(shape_dest*)malloc(sizeof(shape_dest));
            nSrc->originalObj = NULL;
            nSrc->originalID = strdup(dup_value+spos);
            nSrc->windingRule=fill_nonZero;
            nSrc->theShape=NULL;
            nSrc->bbox=NR::Rect(NR::Point(0,0),NR::Point(0,0));
            typeset->exclElems=g_slist_append(typeset->exclElems,nSrc);
            typeset->layoutDirty=true;
            typeset->exclDirty=true;
            dup_value[epos]=savC;
          }
          pos=epos;
        }
        free(dup_value);
      } else {
        sp_typeset_ditch_excl(typeset);
        if ( typeset->excluded ) delete typeset->excluded;
        typeset->excluded=false;
      }
      break;
    case SP_ATTR_STYLE:
      typeset->layoutDirty=true;
      break;
    case SP_ATTR_LAYOUT_OPTIONS:
    {
      SPCSSAttr * opts=sp_repr_css_attr ((SP_OBJECT(typeset))->repr, "inkscape:layoutOptions");
      {
        const gchar * val=sp_repr_css_property (opts,"justification", NULL);
        if ( val == NULL ) {
          typeset->justify=true;
        } else {
          if ( strcmp(val,"0") == 0 || strcmp(val,"false") == 0 ) {
            typeset->justify=false;
          } else {
            typeset->justify=true;
          }
        }
      }
      {
        const gchar * val=sp_repr_css_property (opts,"alignment", NULL);
        if ( val == NULL ) {
          typeset->centering=0;
        } else {
          if ( strcmp(val,"left") == 0 ) {
            typeset->centering=0;
          } else if ( strcmp(val,"center") == 0 ) {
            typeset->centering=1;
          } else if ( strcmp(val,"right") == 0 ) {
            typeset->centering=2;
          }
        }
      }
      {
        const gchar * val=sp_repr_css_property (opts,"layoutAlgo", NULL);
        if ( val == NULL ) {
          typeset->stdLayoutAlgo=false;
        } else {
          if ( strcmp(val,"better") == 0 ) {
            typeset->stdLayoutAlgo=false;
          } else if ( strcmp(val,"simple") == 0 ) {
            typeset->stdLayoutAlgo=true;
          }
        }
      }
      typeset->layoutDirty=true;
      sp_repr_css_attr_unref (opts);
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
void sp_typeset_ditch_excl(SPTypeset *typeset)
{
  GSList* l=typeset->exclElems;
  while ( l ) {
    shape_dest* cur=(shape_dest*)l->data;
    if ( cur->theShape ) delete cur->theShape;
    if ( cur->originalID ) free(cur->originalID);
    if ( cur->originalObj ) sp_repr_remove_listener_by_data (cur->originalObj, typeset);
    l=l->next;
  }
  while ( typeset->exclElems ) {
    void* cur=(void*)typeset->exclElems->data;
    typeset->exclElems=g_slist_remove(typeset->exclElems,cur);
    free(cur);
  }
  typeset->layoutDirty=true;
}
void sp_typeset_ditch_dest(SPTypeset *typeset)
{
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
                               const gchar * /*oldval*/, const gchar * /*newval*/,
                               bool /*is_interactive*/, void * data)
{
  SPTypeset *typeset = (SPTypeset *) data;
//  printf("attrchg %x of %x to %s\n",(int)repr,(int)typeset,newval);
  if ( typeset == NULL || repr == NULL ) return;
  
  if ( strcmp(key,"point") != 0 && strcmp(key,"d") != 0 && strcmp(key,"transform") != 0 ) return;
  
  {
    GSList *l=typeset->exclElems;
    while ( l ) {
      shape_dest *cur=(shape_dest*)(l->data);
      if ( cur->originalObj == repr ) break;
      l=l->next;
    }
    if ( l ) {
      //      shape_dest *cur=(shape_dest*)(l->data);
      typeset->exclDirty=true;
      sp_object_request_update (SP_OBJECT(typeset), SP_OBJECT_MODIFIED_FLAG);
    } else {
      // repr not used in this typeset
      //      printf("not fount\n");
    }
  }
  
  if ( typeset->dstType == has_shape_dest ) {
    GSList *l=typeset->dstElems;
    while ( l ) {
      shape_dest *cur=(shape_dest*)(l->data);
      if ( cur->originalObj == repr ) break;
      l=l->next;
    }
    if ( l ) {
//      shape_dest *cur=(shape_dest*)(l->data);
      typeset->destDirty=true;
      sp_object_request_update (SP_OBJECT(typeset), SP_OBJECT_MODIFIED_FLAG);
    } else {
      // repr not used in this typeset
//      printf("not fount\n");
    }
  } else if ( typeset->dstType == has_path_dest ) {
    GSList *l=typeset->dstElems;
    while ( l ) {
      path_dest *cur=(path_dest*)l->data;
      if ( cur->originalObj == repr ) break;
      l=l->next;
    }
    if ( l ) {
 //     path_dest *cur=(path_dest*)l->data;
      typeset->destDirty=true;
      sp_object_request_update (SP_OBJECT(typeset), SP_OBJECT_MODIFIED_FLAG);
    } else {
      // repr not used in this typeset
//      printf("not fount\n");
    }
  }
}

static void sp_typeset_source_destroy (SPRepr * repr, void *data)
{
  SPTypeset *typeset = (SPTypeset *) data;
//  printf("destroy %x of %x\n",(int)repr,(int)typeset);
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
//      printf("not fount\n");
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
//      printf("not fount\n");
    }
  }
}
static void sp_typeset_source_child_added (SPRepr *repr, SPRepr *child, SPRepr */*ref*/, void * data)
{
//  printf("addchild %x tps=%x\n",(int)repr,data);
  SPTypeset *typeset = (SPTypeset *) data;
  if (typeset == NULL) return;
  if ( child == NULL ) return;
  if ( repr == NULL ) return; // juste le premier niveau.
}
static void sp_typeset_source_child_removed (SPRepr *repr, SPRepr *child, SPRepr */*ref*/, void * data)
{
//  printf("remchild %x tps=%x\n",(int)repr,data);
  SPTypeset *typeset = (SPTypeset *) data;
  if (typeset == NULL) return;
  if ( child == NULL ) return;
  if ( repr == NULL ) return; // juste le premier niveau.
}
static void sp_typeset_source_content_changed (SPRepr */*repr*/, const gchar */*oldcontent*/, const gchar */*newcontent*/, void * data)
{
//  printf("chgcont %x tps=%x\n",(int)repr,data);
  SPTypeset *typeset = (SPTypeset *) data;
  if (typeset == NULL) return;
}

void   refresh_typeset_source(SPTypeset *typeset,shape_dest *nDst)
{
  if ( nDst == NULL ) return;
  
  SPObject *refobj = sp_document_lookup_id (SP_OBJECT (typeset)->document, nDst->originalID);

  if ( nDst->originalObj == NULL ) {
    if ( refobj == NULL ) return;
    nDst->originalObj=refobj->repr;
    if ( nDst->originalObj == NULL ) return;
  }
  
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
    NR::Matrix dummy=sp_item_i2root_affine (item);
    orig->LoadArtBPath (curve->bpath,dummy,true);
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
    NR::Matrix dummy=sp_item_i2root_affine (item);
    nDst->thePath->LoadArtBPath (curve->bpath,dummy,true);
  }
  sp_curve_unref (curve);
  nDst->length=nDst->thePath->Length();
  typeset->layoutDirty=true;

  sp_object_request_update (SP_OBJECT(typeset), SP_OBJECT_MODIFIED_FLAG);
}


/*
 * creation/manipulation
 */

void        sp_typeset_set_text(SPObject* object,char* in_text,int text_type)
{
	SPTypeset *typeset = SP_TYPESET (object);
  if ( typeset == NULL ) return;
  if ( SP_IS_TYPESET(SP_OBJECT_PARENT(object)) ) {
    sp_typeset_set_text(SP_OBJECT_PARENT(object),in_text,text_type);
    return;
  }
  
  SPDesktop *desktop = SP_ACTIVE_DESKTOP;
  if (!SP_IS_DESKTOP (desktop)) return;

  if ( in_text == NULL || in_text[0] == 0 ) {
    sp_repr_set_attr(SP_OBJECT_REPR(object), "inkscape:srcNoMarkup", NULL);
    sp_repr_set_attr(SP_OBJECT_REPR(object), "inkscape:srcPango", NULL);
  } else {
    if ( text_type == 0 || text_type == 1 ) {
      GSList *l=NULL;
      for (	SPObject * child = sp_object_first_child(object) ; child != NULL ; child = SP_OBJECT_NEXT(child) ) {
        if ( SP_IS_TYPESET(child) ) {
          l=g_slist_prepend(l,child);
        } else {
        }
      }
      while ( l ) {
        SPObject *child=(SPObject*)l->data;
        //      sp_object_unref(child, SP_OBJECT(typeset));
        child->deleteObject();
        l=g_slist_remove(l,child);
      }
    }
    if ( text_type == 0 ) {
      int   p_st=0,p_en=0,t_len=strlen(in_text);
      while ( p_st < t_len ) {
        p_en=p_st;
        while ( p_en < t_len && in_text[p_en] != '\n' && in_text[p_en] != '\r' ) p_en++;
        while ( p_en < t_len && ( in_text[p_en] == '\n' || in_text[p_en] == '\r' ) ) p_en++;
        char  sav_c=in_text[p_en];
        in_text[p_en]=0;
        
        SPRepr *repr = sp_repr_new ("g");
        sp_repr_set_attr (repr, "sodipodi:type", "typeset");
        sp_repr_set_attr (repr, "inkscape:srcNoMarkup", in_text+p_st);
        
        sp_repr_append_child (SP_OBJECT_REPR(object), repr);        
        sp_repr_unref (repr);
        
        in_text[p_en]=sav_c;
      }
      sp_document_done (SP_DT_DOCUMENT (desktop));
    } else if ( text_type == 1 ) {
      int   p_st=0,p_en=0,t_len=strlen(in_text);
      while ( p_st < t_len ) {
        p_en=p_st;
        while ( p_en < t_len && in_text[p_en] != '\n' && in_text[p_en] != '\r' ) p_en++;
        while ( p_en < t_len && ( in_text[p_en] == '\n' || in_text[p_en] == '\r' ) ) p_en++;
        char  sav_c=in_text[p_en];
        in_text[p_en]=0;
        
        SPRepr *repr = sp_repr_new ("g");
        sp_repr_set_attr (repr, "sodipodi:type", "typeset");
        sp_repr_set_attr (repr, "inkscape:srcPango", in_text+p_st);
        
        sp_repr_append_child (SP_OBJECT_REPR(object), repr);        
        sp_repr_unref (repr);
        
        in_text[p_en]=sav_c;
      }
      sp_document_done (SP_DT_DOCUMENT (desktop));
    } else {
    }
  }
}
void        sp_typeset_chain_shape(SPObject* object,char* shapeID)
{
	SPTypeset *typeset = SP_TYPESET (object);
  if ( typeset == NULL ) return;
  if ( SP_IS_TYPESET(SP_OBJECT_PARENT(typeset)) ) {
    sp_typeset_chain_shape(SP_OBJECT_PARENT(typeset),shapeID);
    return;
  }
  if ( shapeID == NULL || shapeID[0] == 0 ) return;
  
  SPDesktop *desktop = SP_ACTIVE_DESKTOP;
  if (!SP_IS_DESKTOP (desktop)) return;

  sp_repr_set_attr(SP_OBJECT_REPR(object), "inkscape:dstPath", NULL);
  sp_repr_set_attr(SP_OBJECT_REPR(object), "inkscape:dstBox", NULL);
  sp_repr_set_attr(SP_OBJECT_REPR(object), "inkscape:dstColumn", NULL);
  char* n_dst_shape=strdup(sp_repr_attr(SP_OBJECT_REPR(object),"inkscape:dstShape"));
  if ( n_dst_shape == NULL ) {
    n_dst_shape=(char*)malloc((2+strlen(shapeID))*sizeof(char));
    n_dst_shape[0]='#';
    memcpy(n_dst_shape+1,shapeID,strlen(shapeID)*sizeof(char));
    n_dst_shape[1+strlen(shapeID)]=0;
  } else {
    int len=strlen(n_dst_shape);
    n_dst_shape=(char*)realloc(n_dst_shape,(3+len+strlen(shapeID))*sizeof(char));
    n_dst_shape[len]=' ';
    n_dst_shape[len+1]='#';
    memcpy(n_dst_shape+(len+2),shapeID,strlen(shapeID)*sizeof(char));
    n_dst_shape[2+len+strlen(shapeID)]=0;
  }
  sp_repr_set_attr(SP_OBJECT_REPR(object), "inkscape:dstShape", n_dst_shape);
  free(n_dst_shape);
}


