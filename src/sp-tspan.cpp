#define __SP_TSPAN_C__

/*
 * SVG <text> and <tspan> implementation
 *
 * Author:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * Copyright (C) 1999-2002 Lauris Kaplinski
 * Copyright (C) 2000-2001 Ximian, Inc.
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

/*
 * fixme:
 *
 * These subcomponents should not be items, or alternately
 * we have to invent set of flags to mark, whether standard
 * attributes are applicable to given item (I even like this
 * idea somewhat - Lauris)
 *
 */

#include "config.h"

#include <string.h>

#include <libnr/nr-rect.h>
#include <libnr/nr-matrix.h>
#include <libnr/nr-matrix-ops.h>
#include <libnr/nr-matrix-fns.h>
#include <libnr/nr-rotate.h>
//#include <libnrtype/nr-typeface.h>
//#include <libnrtype/FontFactory.h>
//#include <libnrtype/font-instance.h>
//#include <libnrtype/font-style-to-pos.h>

#include <libnrtype/FlowDefs.h>
//#include <libnrtype/TextWrapper.h>

#include <livarot/LivarotDefs.h>
#include <livarot/Shape.h>
#include <livarot/Path.h>

#include <glib.h>
//#include <gtk/gtk.h>

#include "helper/sp-intl.h"
#include "xml/repr-private.h"
#include "svg/svg.h"
#include "svg/stringstream.h"
#include "display/nr-arena-group.h"
#include "display/nr-arena-glyphs.h"
#include "attributes.h"
#include "document.h"
#include "desktop.h"
#include "style.h"
//#include "version.h"
//#include "inkscape.h"
//#include "view.h"
//#include "print.h"

#include "sp-use-reference.h"


#include "sp-shape.h"
#include "sp-tspan.h"
#include "sp-text.h"

#include "prefs-utils.h"

/*
 *
 */
void read_length_array(int &nb,SPSVGLength* &array,const char* value)
{
	if ( array ) free(array);
	array=NULL;
	nb=0;
	if ( value == NULL ) {
	} else {
		GList* list=sp_svg_length_list_read (value);
		nb=g_list_length(list);
		array=(SPSVGLength*)malloc(nb*sizeof(SPSVGLength));
		for (int i=0;i<nb;i++) sp_svg_length_unset (array+i, SP_SVG_UNIT_NONE, 0.0, 0.0);
		int    cur=0;
		for (GList* l=list;l;l=l->next) {
			SPSVGLength* nl=(SPSVGLength*)l->data;
			if ( cur < nb ) array[cur++]=*nl; // overcautious
			g_free(l->data);
		}
		g_list_free(list);
	}
}
char* write_length_array(int nb,SPSVGLength* array)
{
	if ( nb <= 0 ) return NULL;
	gchar c[32];
	gchar *s = NULL;
	
	for (int i=0;i<nb;i++) {
		g_ascii_formatd (c, sizeof (c), "%.8g", array[i].computed);
		if (i == 0) {
			s = g_strdup (c);
		}  else {
			s = g_strjoin (" ", s, c, NULL);
		}
	}
	return s;
}

/*#####################################################
#  SPTSPAN
#####################################################*/

static void sp_tspan_class_init (SPTSpanClass *classname);
static void sp_tspan_init (SPTSpan *tspan);

static void sp_tspan_build (SPObject * object, SPDocument * document, SPRepr * repr);
static void sp_tspan_release (SPObject *object);
static void sp_tspan_set (SPObject *object, unsigned int key, const gchar *value);
static void sp_tspan_update (SPObject *object, SPCtx *ctx, guint flags);
static void sp_tspan_modified (SPObject *object, unsigned int flags);
static SPRepr *sp_tspan_write (SPObject *object, SPRepr *repr, guint flags);

static SPItemClass *tspan_parent_class;

/**
*
 */
GType
sp_tspan_get_type ()
{
	static GType type = 0;
	if (!type) {
		GTypeInfo info = {
			sizeof (SPTSpanClass),
			NULL,    /* base_init */
			NULL,    /* base_finalize */
			(GClassInitFunc) sp_tspan_class_init,
			NULL,    /* class_finalize */
			NULL,    /* class_data */
			sizeof (SPTSpan),
			16,    /* n_preallocs */
			(GInstanceInitFunc) sp_tspan_init,
			NULL,    /* value_table */
		};
		type = g_type_register_static (SP_TYPE_ITEM, "SPTSpan", &info, (GTypeFlags)0);
	}
	return type;
}

static void
sp_tspan_class_init (SPTSpanClass *classname)
{
	SPObjectClass * sp_object_class;
	SPItemClass * item_class;
	
	sp_object_class = (SPObjectClass *) classname;
	item_class = (SPItemClass *) classname;
	
	tspan_parent_class = (SPItemClass*)g_type_class_ref (SP_TYPE_ITEM);
	
	sp_object_class->build = sp_tspan_build;
	sp_object_class->release = sp_tspan_release;
	sp_object_class->set = sp_tspan_set;
	sp_object_class->update = sp_tspan_update;
	sp_object_class->modified = sp_tspan_modified;
	sp_object_class->write = sp_tspan_write;
}
static void
sp_tspan_init (SPTSpan *tspan)
{
	tspan->role=SP_TSPAN_ROLE_UNSPECIFIED;
	tspan->last_tspan=false;
	new (&tspan->contents) div_flow_src(SP_OBJECT(tspan),txt_tline);
}
static void
sp_tspan_release (SPObject *object)
{
	SPTSpan *tspan = SP_TSPAN (object);

	tspan->contents.~div_flow_src();
	
	if (((SPObjectClass *) tspan_parent_class)->release)
		((SPObjectClass *) tspan_parent_class)->release (object);
}
static void
sp_tspan_build (SPObject *object, SPDocument *doc, SPRepr *repr)
{
	//SPTSpan *tspan = SP_TSPAN (object);
	
	sp_object_read_attr (object, "x");
	sp_object_read_attr (object, "y");
	sp_object_read_attr (object, "dx");
	sp_object_read_attr (object, "dy");
	sp_object_read_attr (object, "rotate");
	sp_object_read_attr (object, "sodipodi:role");
	
	bool  no_content=true;
	for (SPRepr* rch = repr->children; rch != NULL; rch = rch->next) {
		if ( rch->type == SP_XML_TEXT_NODE ) {no_content=false;break;}
	}
	
	if ( no_content ) {
		SPRepr* rch = sp_xml_document_createTextNode (sp_repr_document (repr), "");
		sp_repr_add_child (repr, rch, NULL);
	}
	
	if (((SPObjectClass *) tspan_parent_class)->build)
		((SPObjectClass *) tspan_parent_class)->build (object, doc, repr);
}

static void
sp_tspan_set (SPObject *object, unsigned int key, const gchar *value)
{
	SPTSpan *tspan = SP_TSPAN (object);
	
	/* fixme: Vectors */
	switch (key) {
    case SP_ATTR_X:
			tspan->contents.SetX(value);
			if ( tspan->contents.nb_x > 0 ) {
				tspan->x=tspan->contents.x_s[0];
			} else {
				tspan->x.set=0;
			}
				if ( tspan->role != SP_TSPAN_ROLE_LINE ) object->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
				break;
    case SP_ATTR_Y:
			tspan->contents.SetY(value);
			if ( tspan->contents.nb_y > 0 ) {
				tspan->y=tspan->contents.y_s[0];
			} else {
				tspan->y.set=0;
			}
				if ( tspan->role != SP_TSPAN_ROLE_LINE ) object->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
				break;
    case SP_ATTR_DX:
			tspan->contents.SetDX(value);
				if ( tspan->role != SP_TSPAN_ROLE_LINE ) object->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
				break;
    case SP_ATTR_DY:
			tspan->contents.SetDY(value);
				if ( tspan->role != SP_TSPAN_ROLE_LINE ) object->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
				break;
    case SP_ATTR_ROTATE:
			tspan->contents.SetRot(value);
				if ( tspan->role != SP_TSPAN_ROLE_LINE ) object->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
				break;
    case SP_ATTR_SODIPODI_ROLE:
			if (value && (!strcmp (value, "line") || !strcmp (value, "paragraph"))) {
				tspan->role = SP_TSPAN_ROLE_LINE;
			} else {
				tspan->role = SP_TSPAN_ROLE_UNSPECIFIED;
			}
			break;
    default:
			if (((SPObjectClass *) tspan_parent_class)->set)
				(((SPObjectClass *) tspan_parent_class)->set) (object, key, value);
			break;
	}
}

static void
sp_tspan_update (SPObject *object, SPCtx *ctx, guint flags)
{
//	SPTSpan *tspan = SP_TSPAN (object);
//	SPStyle *style = SP_OBJECT_STYLE (object);
//	SPItemCtx *ictx = (SPItemCtx *) ctx;
//	GList *i;
	
	if (((SPObjectClass *) tspan_parent_class)->update)
		((SPObjectClass *) tspan_parent_class)->update (object, ctx, flags);
	
	if (flags & SP_OBJECT_MODIFIED_FLAG) flags |= SP_OBJECT_PARENT_MODIFIED_FLAG;
	flags &= SP_OBJECT_MODIFIED_CASCADE;
	
	SPObject *ochild;
	for ( ochild = sp_object_first_child(object) ; ochild ; ochild = SP_OBJECT_NEXT(ochild) ) {
		if ( flags || ( ochild->uflags & SP_OBJECT_MODIFIED_FLAG )) {
	    ochild->updateDisplay(ctx, flags);
		}
	}
}

static void
sp_tspan_modified (SPObject *object, unsigned int flags)
{
	if (((SPObjectClass *) tspan_parent_class)->modified)
		((SPObjectClass *) tspan_parent_class)->modified (object, flags);
	
	if (flags & SP_OBJECT_MODIFIED_FLAG)
		flags |= SP_OBJECT_PARENT_MODIFIED_FLAG;
	flags &= SP_OBJECT_MODIFIED_CASCADE;
	
	SPObject *ochild;
	for ( ochild = sp_object_first_child(object) ; ochild ; ochild = SP_OBJECT_NEXT(ochild) ) {
		if (flags || (ochild->mflags & SP_OBJECT_MODIFIED_FLAG)) {
			ochild->emitModified(flags);
		}
	}
}

static SPRepr *
sp_tspan_write (SPObject *object, SPRepr *repr, guint flags)
{
	SPTSpan *tspan = SP_TSPAN (object);
	
	if ((flags & SP_OBJECT_WRITE_BUILD) && !repr) {
		repr = sp_repr_new ("tspan");
	}
	
	bool  became_empty=false;
	if ( tspan->role != SP_TSPAN_ROLE_UNSPECIFIED ) {
		if ( tspan->contents.type == txt_tline && tspan->last_tspan == false && tspan->contents.utf8_st >= tspan->contents.utf8_en ) {
			became_empty=true;
		}
	}
		
	char* nlist=NULL;
	if ( became_empty == false ) {
		if ( (nlist=tspan->contents.GetX()) ) {
			sp_repr_set_attr(repr,"x",nlist);
			g_free(nlist);
		} else {
			if ( tspan->x.set ) sp_repr_set_double (repr, "x", tspan->x.computed); else sp_repr_set_attr (repr, "x", NULL);
		}
		if ( (nlist=tspan->contents.GetY()) ) {
			sp_repr_set_attr(repr,"y",nlist);
			g_free(nlist);
		} else {
			if ( tspan->y.set ) sp_repr_set_double (repr, "y", tspan->y.computed); sp_repr_set_attr (repr, "y", NULL);
		}
	} else {
		sp_repr_set_attr (repr, "x", NULL);
		sp_repr_set_attr (repr, "y", NULL);
	}
	if ( (nlist=tspan->contents.GetDX()) ) {
		sp_repr_set_attr(repr,"dx",nlist);
		g_free(nlist);
	} else {
		sp_repr_set_attr (repr, "dx", NULL);
	}
	if ( (nlist=tspan->contents.GetDY()) ) {
		sp_repr_set_attr(repr,"dy",nlist);
		g_free(nlist);
	} else {
		sp_repr_set_attr (repr, "dy", NULL);
	}
	if ( (nlist=tspan->contents.GetRot()) ) {
		sp_repr_set_attr(repr,"rotate",nlist);
		g_free(nlist);
	} else {
		sp_repr_set_attr (repr, "rotate", NULL);
	}
	if (flags & SP_OBJECT_WRITE_EXT) {
		if ( tspan->role != SP_TSPAN_ROLE_UNSPECIFIED ) {
			if ( became_empty == false ) {
				sp_repr_set_attr (repr, "sodipodi:role", "line");
			} else {
				sp_repr_set_attr (repr, "sodipodi:role", NULL);
			}
		} else {
			sp_repr_set_attr (repr, "sodipodi:role", NULL);
		}
	}
	
	if ( flags&SP_OBJECT_WRITE_BUILD ) {
		GSList *l = NULL;
		for (SPObject* child = sp_object_first_child(object) ; child != NULL ; child = SP_OBJECT_NEXT(child) ) {
			SPRepr* c_repr=NULL;
			if ( SP_IS_TSPAN (child) ) {
				c_repr = child->updateRepr(NULL, flags);
			} else if ( SP_IS_TEXTPATH(child) ) {
				//c_repr = child->updateRepr(NULL, flags); // shouldn't happen
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
			if ( SP_IS_TSPAN (child) ) {
				child->updateRepr(flags);
			} else if ( SP_IS_TEXTPATH(child) ) {
				//c_repr = child->updateRepr(NULL, flags); // shouldn't happen
			} else if ( SP_IS_STRING(child) ) {
				sp_repr_set_content (SP_OBJECT_REPR (child), (SP_STRING_TEXT (child))?SP_STRING_TEXT (child):"");
			}
		}
	}
	
	if (((SPObjectClass *) tspan_parent_class)->write)
		((SPObjectClass *) tspan_parent_class)->write (object, repr, flags);
	
	return repr;
}

/*#####################################################
#  SPTEXTPATH
#####################################################*/

static void sp_textpath_class_init (SPTextPathClass *classname);
static void sp_textpath_init (SPTextPath *textpath);
static void sp_textpath_finalize(GObject *obj);

static void sp_textpath_build (SPObject * object, SPDocument * document, SPRepr * repr);
static void sp_textpath_release (SPObject *object);
static void sp_textpath_set (SPObject *object, unsigned int key, const gchar *value);
static void sp_textpath_update (SPObject *object, SPCtx *ctx, guint flags);
static void sp_textpath_modified (SPObject *object, unsigned int flags);
static SPRepr *sp_textpath_write (SPObject *object, SPRepr *repr, guint flags);

static SPItemClass *textpath_parent_class;

void   refresh_textpath_source(SPTextPath* offset);


/**
*
 */
GType
sp_textpath_get_type ()
{
	static GType type = 0;
	if (!type) {
		GTypeInfo info = {
			sizeof (SPTextPathClass),
			NULL,    /* base_init */
			NULL,    /* base_finalize */
			(GClassInitFunc) sp_textpath_class_init,
			NULL,    /* class_finalize */
			NULL,    /* class_data */
			sizeof (SPTextPath),
			16,    /* n_preallocs */
			(GInstanceInitFunc) sp_textpath_init,
			NULL,    /* value_table */
		};
		type = g_type_register_static (SP_TYPE_ITEM, "SPTextPath", &info, (GTypeFlags)0);
	}
	return type;
}
static void
sp_textpath_class_init (SPTextPathClass *classname)
{
	GObjectClass  *gobject_class = (GObjectClass *) classname;
	SPObjectClass * sp_object_class;
	SPItemClass * item_class;
	
	sp_object_class = (SPObjectClass *) classname;
	item_class = (SPItemClass *) classname;
	
	textpath_parent_class = (SPItemClass*)g_type_class_ref (SP_TYPE_ITEM);
	
		gobject_class->finalize = sp_textpath_finalize;
		
    sp_object_class->build = sp_textpath_build;
    sp_object_class->release = sp_textpath_release;
    sp_object_class->set = sp_textpath_set;
    sp_object_class->update = sp_textpath_update;
    sp_object_class->modified = sp_textpath_modified;
    sp_object_class->write = sp_textpath_write;
}
static void
sp_textpath_init (SPTextPath *textpath)
{
	new (&textpath->contents) div_flow_src(SP_OBJECT(textpath),txt_textpath);
	
	textpath->originalPath = NULL;
	textpath->isUpdating=false;
		// set up the uri reference
	textpath->sourcePath = new SPUsePath(SP_OBJECT(textpath));
	textpath->sourcePath->user_unlink = sp_textpath_to_text;
}
static void
sp_textpath_finalize(GObject *obj)
{
	SPTextPath *textpath = (SPTextPath *) obj;
	
	delete textpath->sourcePath;
}
static void
sp_textpath_release (SPObject *object)
{
	SPTextPath *textpath = SP_TEXTPATH (object);
	
	textpath->contents.~div_flow_src();
	
	if (textpath->originalPath) delete textpath->originalPath;
	textpath->originalPath = NULL;
		
	if (((SPObjectClass *) textpath_parent_class)->release)
		((SPObjectClass *) textpath_parent_class)->release (object);
}
static void
sp_textpath_build (SPObject *object, SPDocument *doc, SPRepr *repr)
{
	//SPTextPath *textpath = SP_TEXTPATH (object);
	
	sp_object_read_attr (object, "x");
	sp_object_read_attr (object, "y");
	sp_object_read_attr (object, "dx");
	sp_object_read_attr (object, "dy");
	sp_object_read_attr (object, "rotate");
	sp_object_read_attr (object, "xlink:href");
	
	bool  no_content=true;
	for (SPRepr* rch = repr->children; rch != NULL; rch = rch->next) {
		if ( rch->type == SP_XML_TEXT_NODE ) {no_content=false;break;}
	}
	
	if ( no_content ) {
		SPRepr* rch = sp_xml_document_createTextNode (sp_repr_document (repr), "");
		sp_repr_add_child (repr, rch, NULL);
	}
	
	if (((SPObjectClass *) textpath_parent_class)->build)
		((SPObjectClass *) textpath_parent_class)->build (object, doc, repr);
}
static void
sp_textpath_set (SPObject *object, unsigned int key, const gchar *value)
{
	SPTextPath *textpath = SP_TEXTPATH (object);
	
	/* fixme: Vectors */
	switch (key) {
    case SP_ATTR_X:
			textpath->contents.SetX(value);
			if ( textpath->contents.nb_x > 0 ) {
				textpath->x=textpath->contents.x_s[0];
			} else {
				textpath->x.set=0;
			}
				object->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
				break;
    case SP_ATTR_Y:
			textpath->contents.SetY(value);
			if ( textpath->contents.nb_y > 0 ) {
				textpath->y=textpath->contents.y_s[0];
			} else {
				textpath->y.set=0;
			}
				object->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
				break;
    case SP_ATTR_DX:
			textpath->contents.SetDX(value);
				object->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
				break;
    case SP_ATTR_DY:
			textpath->contents.SetDY(value);
				object->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
				break;
    case SP_ATTR_ROTATE:
			textpath->contents.SetRot(value);
				object->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
				break;
    case SP_ATTR_XLINK_HREF:
			textpath->sourcePath->link((char*)value);
      break;
    default:
			if (((SPObjectClass *) textpath_parent_class)->set)
				(((SPObjectClass *) textpath_parent_class)->set) (object, key, value);
			break;
	}
}

static void
sp_textpath_update (SPObject *object, SPCtx *ctx, guint flags)
{
	SPTextPath *textpath = SP_TEXTPATH (object);
	//SPStyle *style = SP_OBJECT_STYLE (object);
	//SPItemCtx *ictx = (SPItemCtx *) ctx;
	//GList *i;
	
	textpath->isUpdating=true;
	if ( textpath->sourcePath->sourceDirty ) refresh_textpath_source(textpath);
	textpath->isUpdating=false;
		
	if (((SPObjectClass *) textpath_parent_class)->update)
		((SPObjectClass *) textpath_parent_class)->update (object, ctx, flags);
		
	if (flags & SP_OBJECT_MODIFIED_FLAG) flags |= SP_OBJECT_PARENT_MODIFIED_FLAG;
	flags &= SP_OBJECT_MODIFIED_CASCADE;
			
	SPObject *ochild;
	for ( ochild = sp_object_first_child(object) ; ochild ; ochild = SP_OBJECT_NEXT(ochild) ) {
		if ( flags || ( ochild->uflags & SP_OBJECT_MODIFIED_FLAG )) {
			ochild->updateDisplay(ctx, flags);
		}
	}
}


void   refresh_textpath_source(SPTextPath* tp)
{
  if ( tp == NULL ) return;
	tp->sourcePath->refresh_source();
  tp->sourcePath->sourceDirty=false;
	
  // finalisons
  if ( tp->sourcePath->originalPath ) { 
 		if (tp->originalPath) {
			delete tp->originalPath;
		}
		tp->originalPath = NULL;
		
		tp->originalPath = new Path;
		tp->originalPath->Copy(tp->sourcePath->originalPath);
		tp->originalPath->ConvertWithBackData(0.5);
		
  }
}
static void
sp_textpath_modified (SPObject *object, unsigned int flags)
{
	if (((SPObjectClass *) textpath_parent_class)->modified)
		((SPObjectClass *) textpath_parent_class)->modified (object, flags);
	
	if (flags & SP_OBJECT_MODIFIED_FLAG)
		flags |= SP_OBJECT_PARENT_MODIFIED_FLAG;
	flags &= SP_OBJECT_MODIFIED_CASCADE;
	
	SPObject *ochild;
	for ( ochild = sp_object_first_child(object) ; ochild ; ochild = SP_OBJECT_NEXT(ochild) ) {
		if (flags || (ochild->mflags & SP_OBJECT_MODIFIED_FLAG)) {
			ochild->emitModified(flags);
		}
	}
}
static SPRepr *
sp_textpath_write (SPObject *object, SPRepr *repr, guint flags)
{
	SPTextPath *textpath = SP_TEXTPATH (object);
	
	if ((flags & SP_OBJECT_WRITE_BUILD) && !repr) {
		repr = sp_repr_new ("textpath");
	}
	
	char* nlist=NULL;
	if ( (nlist=textpath->contents.GetX()) ) {
		sp_repr_set_attr(repr,"x",nlist);
		g_free(nlist);
	} else {
		if ( textpath->x.set ) sp_repr_set_double (repr, "x", textpath->x.computed); else sp_repr_set_attr (repr, "x", NULL);
	}
	if ( (nlist=textpath->contents.GetY()) ) {
		sp_repr_set_attr(repr,"y",nlist);
		g_free(nlist);
	} else {
		if ( textpath->y.set ) sp_repr_set_double (repr, "y", textpath->y.computed); else sp_repr_set_attr (repr, "y", NULL);
	}
	if ( (nlist=textpath->contents.GetDX()) ) {
		sp_repr_set_attr(repr,"dx",nlist);
		g_free(nlist);
	} else {
		sp_repr_set_attr (repr, "dx", NULL);
	}
	if ( (nlist=textpath->contents.GetDY())) {
		sp_repr_set_attr(repr,"dy",nlist);
		g_free(nlist);
	} else {
		sp_repr_set_attr (repr, "dy", NULL);
	}
	if ( (nlist=textpath->contents.GetRot()) ) {
		sp_repr_set_attr(repr,"rotate",nlist);
		g_free(nlist);
	} else {
		sp_repr_set_attr (repr, "rotate", NULL);
	}
	if ( textpath->sourcePath->sourceHref ) sp_repr_set_attr (repr, "xlink:href", textpath->sourcePath->sourceHref);
	
	if ( flags&SP_OBJECT_WRITE_BUILD ) {
		GSList *l = NULL;
		for (SPObject* child = sp_object_first_child(object) ; child != NULL ; child = SP_OBJECT_NEXT(child) ) {
			SPRepr* c_repr=NULL;
			if ( SP_IS_TSPAN (child) ) {
				c_repr = child->updateRepr(NULL, flags);
			} else if ( SP_IS_TEXTPATH(child) ) {
				//c_repr = child->updateRepr(NULL, flags); // shouldn't happen
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
			if ( SP_IS_TSPAN (child) ) {
				child->updateRepr(flags);
			} else if ( SP_IS_TEXTPATH(child) ) {
				//c_repr = child->updateRepr(NULL, flags); // shouldn't happen
			} else if ( SP_IS_STRING(child) ) {
				sp_repr_set_content (SP_OBJECT_REPR (child), (SP_STRING_TEXT (child))?SP_STRING_TEXT (child):"");
			}
		}
	}
	
	if (((SPObjectClass *) textpath_parent_class)->write)
		((SPObjectClass *) textpath_parent_class)->write (object, repr, flags);
	
	return repr;
}


SPItem *
sp_textpath_get_path_item (SPTextPath *tp)
{
	if (tp && tp->sourcePath) {
		SPItem *refobj = tp->sourcePath->getObject();
		if (SP_IS_ITEM (refobj))
			return (SPItem *) refobj;
	}
	return NULL;
}

void
sp_textpath_to_text (SPObject *tp)
{
	SPObject *text = SP_OBJECT_PARENT (tp);

        // make a list of textpath children
        GSList *tp_reprs = NULL;
        for (SPObject *o = SP_OBJECT(tp)->children; o != NULL; o = o->next) {
            tp_reprs = g_slist_prepend (tp_reprs, SP_OBJECT_REPR (o));
        }

        for ( GSList *i = tp_reprs ; i ; i = i->next ) {
            // make a copy of each textpath child
            SPRepr *copy = sp_repr_duplicate((SPRepr *) i->data);
            // remove the old repr from under textpath
            sp_repr_remove_child(SP_OBJECT_REPR(tp), (SPRepr *) i->data); 
            // put its copy into under textPath
            sp_repr_add_child (SP_OBJECT_REPR(text), copy, NULL); // fixme: copy id
        }

        //remove textpath
        tp->deleteObject();
        g_slist_free(tp_reprs);
}
