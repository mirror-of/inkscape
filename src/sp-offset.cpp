#define __SP_OFFSET_C__

/*
 * <inkscape:offset> implementation
 *
 * Authors (of the sp-spiral.c upon which this file was constructed):
 *   Mitsuru Oka <oka326@parkcity.ne.jp>
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * Copyright (C) 1999-2002 Lauris Kaplinski
 * Copyright (C) 2000-2001 Ximian, Inc.
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <config.h>

#include <math.h>
#include <string.h>
#include <stdlib.h>

#include "svg/svg.h"
#include "attributes.h"
#include "document.h"
#include "helper/bezier-utils.h"
#include "dialogs/object-attributes.h"
#include "helper/sp-intl.h"
#include "xml/repr-private.h"

#include "livarot/Path.h"
#include "livarot/Shape.h"

#include "sp-text.h"
#include "sp-shape.h"

#include "sp-offset.h"

#include "libnr/nr-point.h"
#include <libnr/nr-point-fns.h>
#include <libnr/nr-point-ops.h>
#include <libnr/nr-matrix.h>
#include <libnr/nr-matrix-fns.h>

#define noOFFSET_VERBOSE

/*
 * sp-offset is a derivative of SPShape, much like the sp-spiral or sp-rect
 * the goal is to have a source shape (= originalPath), an offset (= radius) and
 * cmpute the offset of the source by the radius. to get it to work, one needs to know
 * what the source is and what the radius is, and how it's stored in the xml
 * the object itself is a "path" element, to get lots of shape functionnality for free
 * the source is the easy part: it's stored in a "inkscape:original" attribute in the 
 * path. in case of "linked" offset, as they've been dubbed, there is an additional
 * "inkscape:href" that contains the id of an element of the svg. when built, the object will
 * attach a listener vector to that object and rebuild the "inkscape:original" whenever the href'd 
 * object changes. this is of course grossly inefficient, and also does not react to changes 
 * to the href'd during context stuff (like changing the shape of a star by dragging control points)
 * unless the path of that object is change during the context (seems to be the case for
 * sp-ellipse). the computation of the offset is done in sp_offset_set_shape(), a function that is 
 * called whenever a change occurs to the offset (change of source or change of radius).
 * just like the sp-star and other, this path derivative can make control points, or
 * more precisely one control point, that's enough to define the radius. look in object-edit
 */

static void sp_offset_class_init (SPOffsetClass * klass);
static void sp_offset_init (SPOffset * offset);

static void sp_offset_build (SPObject * object, SPDocument * document,
                             SPRepr * repr);
static SPRepr *sp_offset_write (SPObject * object, SPRepr * repr,
                                guint flags);
static void sp_offset_set (SPObject * object, unsigned int key,
                           const gchar * value);
static void sp_offset_update (SPObject * object, SPCtx * ctx, guint flags);
static void sp_offset_release (SPObject * object);

static gchar *sp_offset_description (SPItem * item);
static int sp_offset_snappoints(SPItem *item, NR::Point p[], int size);
static void sp_offset_set_shape (SPShape * shape);

Path *bpath_to_liv_path (ArtBpath * bpath);

void   refresh_offset_source(SPOffset* offset);

// pour recevoir les changements de l'objet source
static void sp_offset_source_attr_changed (SPRepr * repr, const gchar * key,
                                           const gchar * oldval,
                                           const gchar * newval,
					   bool is_interactive, void * data);
static void sp_offset_source_destroy (SPRepr * repr, void *data);
static void sp_offset_source_child_added (SPRepr *repr, SPRepr *child, SPRepr *ref, void * data);
static void sp_offset_source_child_removed (SPRepr *repr, SPRepr *child, SPRepr *ref, void * data);
static void sp_offset_source_content_changed (SPRepr *repr, const gchar *oldcontent, const gchar *newcontent, void * data);

// the regular listener vector
SPReprEventVector offset_source_event_vector = {
  sp_offset_source_destroy,
  NULL,				/* Add child */
  sp_offset_source_child_added,
  NULL,
  sp_offset_source_child_removed,				/* Child removed */
  NULL,
  sp_offset_source_attr_changed,
  NULL,				/* Change content */
  sp_offset_source_content_changed,
  NULL,				/* change_order */
  NULL
};

// listener vector for sons of source object
// it is intended to deal with text elements
// indeed, text is mostly stored in tspans (or text itself), so we need to listen to these
// the text object has listener attached that will track adding and removal of children,
// and add this listener vector to them
// theoritically, children should get the same listener vector as their parents, but that would require us to
// track in the sp-offset what is the source and what is the childrens. having only one object with the
// offset_source_event_vector makes that easy
SPReprEventVector offset_source_child_event_vector = {
  sp_offset_source_destroy,
  NULL,				/* Add child */
  NULL,
  NULL,
  NULL,				/* Child removed */
  NULL,
  NULL,
  NULL,				/* Change content */
  sp_offset_source_content_changed,
  NULL,				/* change_order */
  NULL
};

// slow= source path->polygon->offset of polygon->polygon->path
// fast= source path->offset of source path->polygon->path
// fast is not mathematically correct, because computing the offset of a single
// cubic bezier patch is not trivial; in particular, there are problems with holes 
// reappearing in offset when the radius becomes too large
static bool   use_slow_but_correct_offset_method=false;


// nothing special here, same for every class in sodipodi/inkscape
static SPShapeClass *parent_class;

GType
sp_offset_get_type (void)
{
  static GType offset_type = 0;
  
  if (!offset_type)
  {
    GTypeInfo offset_info = {
      sizeof (SPOffsetClass),
      NULL,			/* base_init */
      NULL,			/* base_finalize */
      (GClassInitFunc) sp_offset_class_init,
      NULL,			/* class_finalize */
      NULL,			/* class_data */
      sizeof (SPOffset),
      16,			/* n_preallocs */
      (GInstanceInitFunc) sp_offset_init,
      NULL,			/* value_table */
    };
    offset_type =
      g_type_register_static (SP_TYPE_SHAPE, "SPOffset", &offset_info,
                              (GTypeFlags) 0);
  }
  return offset_type;
}

static void
sp_offset_class_init (SPOffsetClass * klass)
{
  SPObjectClass *sp_object_class = (SPObjectClass *) klass;
  SPItemClass *item_class = (SPItemClass *) klass;
  SPShapeClass *shape_class = (SPShapeClass *) klass;
  
  parent_class = (SPShapeClass *) g_type_class_ref (SP_TYPE_SHAPE);
  
  sp_object_class->build = sp_offset_build;
  sp_object_class->write = sp_offset_write;
  sp_object_class->set = sp_offset_set;
  sp_object_class->update = sp_offset_update;
  sp_object_class->release = sp_offset_release;
  
  item_class->description = sp_offset_description;
  item_class->snappoints = sp_offset_snappoints;
  
  shape_class->set_shape = sp_offset_set_shape;
}

static void
sp_offset_init (SPOffset * offset)
{
  offset->rad = 1.0;
  offset->original = NULL;
  offset->originalPath = NULL;
  offset->knotSet = false;
  offset->sourceObject = NULL;
  offset->sourceRepr = NULL;
  offset->sourceDirty=false;
}

static void
sp_offset_build (SPObject * object, SPDocument * document, SPRepr * repr)
{
  if (((SPObjectClass *) parent_class)->build)
    ((SPObjectClass *) parent_class)->build (object, document, repr);
  
  if ( sp_repr_attr (object->repr, "inkscape:radius") ) {
    sp_object_read_attr (object, "inkscape:radius");
  } else {
    const gchar* oldA=sp_repr_attr(object->repr,"sodipodi:radius");
    sp_repr_set_attr(object->repr,"inkscape:radius",oldA);
    sp_repr_set_attr(object->repr,"sodipodi:radius",NULL);
    
    sp_object_read_attr (object, "inkscape:radius");
  }
  if ( sp_repr_attr (object->repr, "inkscape:original") ) {
    sp_object_read_attr (object, "inkscape:original");
  } else {    
    const gchar* oldA=sp_repr_attr(object->repr,"sodipodi:original");
    sp_repr_set_attr(object->repr,"inkscape:original",oldA);
    sp_repr_set_attr(object->repr,"sodipodi:original",NULL);
    
    sp_object_read_attr (object, "inkscape:original");
  }
  if ( sp_repr_attr (object->repr, "inkscape:href") ) {
    sp_object_read_attr (object, "inkscape:href");
  } else {
    const gchar* oldA=sp_repr_attr(object->repr,"xlink:href");
    sp_repr_set_attr(object->repr,"inkscape:href",oldA);
    sp_repr_set_attr(object->repr,"xlink:href",NULL);
    
    sp_object_read_attr (object, "inkscape:href");
  }
}

static SPRepr *
sp_offset_write (SPObject * object, SPRepr * repr, guint flags)
{
  SPOffset *offset = SP_OFFSET (object);
  
  if ((flags & SP_OBJECT_WRITE_BUILD) && !repr)
  {
    repr = sp_repr_new ("path");
  }
  
  if (flags & SP_OBJECT_WRITE_EXT)
  {
    /* Fixme: we may replace these attributes by
    * inkscape:offset="cx cy exp revo rad arg t0"
    */
    sp_repr_set_attr (repr, "sodipodi:type", "inkscape:offset");
    sp_repr_set_double (repr, "inkscape:radius", offset->rad);
    sp_repr_set_attr (repr, "inkscape:original", offset->original);
    sp_repr_set_attr (repr, "inkscape:href", offset->sourceObject);
  }
  
  char *d = sp_svg_write_path (((SPShape *) offset)->curve->bpath);
  sp_repr_set_attr (repr, "d", d);
  g_free (d);
  
  if (((SPObjectClass *) (parent_class))->write)
    ((SPObjectClass *) (parent_class))->write (object, repr,
                                               flags | SP_SHAPE_WRITE_PATH);
  
  return repr;
}

static void
sp_offset_release (SPObject * object)
{
  SPOffset *offset = (SPOffset *) object;
  
  if (offset->original)
    free (offset->original);
  if (offset->originalPath)
    delete ((Path *) offset->originalPath);
  
  offset->original = NULL;
  offset->originalPath = NULL;
  
  if (offset->sourceObject)
    free (offset->sourceObject);
  offset->sourceObject = NULL;
  
  if (offset->sourceRepr)
  {
    sp_repr_remove_listener_by_data (offset->sourceRepr, offset);
    for (SPRepr *child=offset->sourceRepr->children;child;child=child->next) sp_repr_remove_listener_by_data (child, offset);
  }
  offset->sourceRepr = NULL;
  
  if (((SPObjectClass *) parent_class)->release)
  {
    ((SPObjectClass *) parent_class)->release (object);
  }
  
}

// the function that is called whenever a change is made to the description of the object
static void
sp_offset_set (SPObject * object, unsigned int key, const gchar * value)
{
  SPOffset *offset = SP_OFFSET (object);
  
  if ( offset->sourceDirty ) refresh_offset_source(offset);
  
  /* fixme: we should really collect updates */
  switch (key)
  {
    case SP_ATTR_INKSCAPE_ORIGINAL:
    case SP_ATTR_SODIPODI_ORIGINAL:
      if (value == NULL)
      {
      }
      else
      {
        if (offset->original)
        {
          free (offset->original);
          delete ((Path *) offset->originalPath);
          offset->original = NULL;
          offset->originalPath = NULL;
        }
        ArtBpath *bpath;
        SPCurve *curve;
        
        offset->original = strdup (value);
        
        bpath = sp_svg_read_path (offset->original);
        curve = sp_curve_new_from_bpath (bpath);	// curve se chargera de detruire bpath
        offset->originalPath = bpath_to_liv_path (curve->bpath);
        sp_curve_unref (curve);
        
        offset->knotSet = false;
        sp_object_request_update (object, SP_OBJECT_MODIFIED_FLAG);
      }
      break;
    case SP_ATTR_INKSCAPE_RADIUS:
    case SP_ATTR_SODIPODI_RADIUS:
	if (!sp_svg_length_read_computed_absolute (value, &offset->rad)) {
        if (fabs (offset->rad) < 0.01)
          offset->rad = (offset->rad < 0) ? -0.01 : 0.01;
        offset->knotSet = false; // knotset=false because it's not set from the context
      }
      sp_object_request_update (object, SP_OBJECT_MODIFIED_FLAG);
      break;
    case SP_ATTR_INKSCAPE_HREF:
    case SP_ATTR_XLINK_HREF:
      if (value)
      {
        if (offset->sourceObject)
        {
          if (strcmp (value, offset->sourceObject) == 0)
            return;
          free (offset->sourceObject);
          offset->sourceObject = strdup (value);
        }
        else
        {
          offset->sourceObject = strdup (value);
        }
        if (offset->sourceRepr)
        {
          sp_repr_remove_listener_by_data (offset->sourceRepr, offset);
        }
        offset->sourceRepr = NULL;
        SPObject *refobj;
        refobj =
          sp_document_lookup_id (SP_OBJECT (offset)->document,
                                 offset->sourceObject);
        if (refobj)
          offset->sourceRepr = refobj->repr;
        if (offset->sourceRepr)
        {
          sp_repr_add_listener (offset->sourceRepr,
                                &offset_source_event_vector, offset);
          // et les enfants s'il y en a (pour le texte)
          SPRepr   *cur_child=offset->sourceRepr->children;
          while ( cur_child ) {
            sp_repr_add_listener (cur_child,
                                  &offset_source_child_event_vector, offset);
            cur_child=cur_child->next;
          }
        }
        else
        {
          free (offset->sourceObject);
          offset->sourceObject = NULL;
        }
        if (offset->sourceRepr)
        {
          refresh_offset_source(offset);
        }
      }
      else
      {
        if (offset->sourceObject)
        {
          free (offset->sourceObject);
          offset->sourceObject = NULL;
          if (offset->sourceRepr)
          {
            sp_repr_remove_listener_by_data (offset->sourceRepr,
                                             offset);
          }
          offset->sourceRepr = NULL;
        }
      }
      break;
    default:
      if (((SPObjectClass *) parent_class)->set)
        ((SPObjectClass *) parent_class)->set (object, key, value);
      break;
  }
}

// the object has changed, recompute its shape
static void
sp_offset_update (SPObject * object, SPCtx * ctx, guint flags)
{
  SPOffset* offset = SP_OFFSET(object);
  if ( offset->sourceDirty ) refresh_offset_source(offset);
  if (flags &
      (SP_OBJECT_MODIFIED_FLAG | SP_OBJECT_STYLE_MODIFIED_FLAG |
       SP_OBJECT_VIEWPORT_MODIFIED_FLAG))
  {
    sp_shape_set_shape ((SPShape *) object);
  }
  
  if (((SPObjectClass *) parent_class)->update)
    ((SPObjectClass *) parent_class)->update (object, ctx, flags);
}

static gchar *
sp_offset_description (SPItem * item)
{
  SPOffset *offset = SP_OFFSET (item);
  char tempSt[256];
  if (offset->rad > 0)
  {
    sprintf (tempSt, "Path outset by %f pt", offset->rad);
    return g_strdup (tempSt);
  }
  if (offset->rad < 0)
  {
    sprintf (tempSt, "Path inset by %f pt", -offset->rad);
    return g_strdup (tempSt);
  }
  return g_strdup ("Offset by 0pt = original path");
}

// duplicate of splivarot
// loads a ArtBpath (like the one stored in a SPCurve) into a livarot Path 
Path *
bpath_to_liv_path (ArtBpath * bpath)
{
  if (bpath == NULL)
    return NULL;
  
  Path *dest = new Path;
  dest->SetBackData (false);
  {
    int i;
    bool closed = false;
    float lastX = 0.0;
    float lastY = 0.0;
    
    for (i = 0; bpath[i].code != ART_END; i++)
    {
      switch (bpath[i].code)
      {
        case ART_LINETO:
          lastX = bpath[i].x3;
          lastY = bpath[i].y3;
          {
            NR::Point  tmp(lastX,lastY);
            dest->LineTo (tmp);
          }
            break;
          
        case ART_CURVETO:
        {
          NR::Point  tmp(bpath[i].x3, bpath[i].y3);
          NR::Point  tms;
          tms[0]=3 * (bpath[i].x1 - lastX);
          tms[1]=3 * (bpath[i].y1 - lastY);
          NR::Point  tme;
          tme[0]=3 * (bpath[i].x3 - bpath[i].x2);
          tme[1]= 3 * (bpath[i].y3 - bpath[i].y2);
          dest->CubicTo (tmp,tms,tme);
        }
          lastX = bpath[i].x3;
          lastY = bpath[i].y3;
          break;
          
        case ART_MOVETO_OPEN:
        case ART_MOVETO:
          if (closed)
            dest->Close ();
          closed = (bpath[i].code == ART_MOVETO);
          lastX = bpath[i].x3;
          lastY = bpath[i].y3;
          {
            NR::Point  tmp(lastX,lastY);
            dest->MoveTo (tmp);
          }
            break;
        default:
          break;
      }
    }
    if (closed)
      dest->Close ();
  }
  
  return dest;
}

// write the livarot Path into a "d" element to be placed in the xml
gchar *
liv_svg_dump_path2 (Path * path)
{
  GString *result;
  result = g_string_sized_new (40);
  
  for (int i = 0; i < path->descr_nb; i++)
  {
    Path::path_descr theD = path->descr_cmd[i];
    int typ = theD.flags & descr_type_mask;
    if (typ == descr_moveto)
    {
      Path::path_descr_moveto*  nData=(Path::path_descr_moveto*)(path->descr_data+theD.dStart);
      g_string_sprintfa (result, "M %lf %lf ", nData->p[0], nData->p[1]);
    }
    else if (typ == descr_lineto)
    {
      Path::path_descr_lineto*  nData=(Path::path_descr_lineto*)(path->descr_data+theD.dStart);
      g_string_sprintfa (result, "L %lf %lf ", nData->p[0], nData->p[1]);
    }
    else if (typ == descr_cubicto)
    {
      Path::path_descr_cubicto*  nData=(Path::path_descr_cubicto*)(path->descr_data+theD.dStart);
      float lastX, lastY;
      {
        NR::Point tmp = path->PrevPoint (i - 1);
        lastX=tmp[0];
        lastY=tmp[1];
      }
      g_string_sprintfa (result, "C %lf %lf %lf %lf %lf %lf ",
                         lastX + nData->stD[0] / 3,
                         lastY + nData->stD[1] / 3,
                         nData->p[0] - nData->enD[0] / 3,
                         nData->p[1] - nData->enD[1] / 3, nData->p[0],nData->p[1]);
    }
    else if (typ == descr_arcto)
    {
      Path::path_descr_arcto*  nData=(Path::path_descr_arcto*)(path->descr_data+theD.dStart);
     //                      g_string_sprintfa (result, "L %lf %lf ",theD.d.a.x,theD.d.a.y);
      g_string_sprintfa (result, "A %g %g %g %i %i %g %g ", nData->rx,nData->ry, nData->angle,
                         (nData->large) ? 1 : 0,(nData->clockwise) ? 0 : 1, nData->p[0],nData->p[1]);
    }
    else if (typ == descr_close)
    {
      g_string_sprintfa (result, "z ");
    }
    else
    {
    }
  }
  
  char *res;
  res = result->str;
  g_string_free (result, FALSE);
  
  return res;
}

static void
sp_offset_set_shape (SPShape * shape)
{
  SPOffset *offset = SP_OFFSET (shape);
  
  sp_object_request_modified (SP_OBJECT (offset), SP_OBJECT_MODIFIED_FLAG);
  
  if ( offset->originalPath == NULL ) {
    // oops : no path?! (the offset object should do harakiri)
    return;
  }
#ifdef OFFSET_VERBOSE
  g_print ("rad=%g\n", offset->rad);
#endif
  // au boulot
  
  if ( fabs(offset->rad) < 0.01 ) {
    // grosso modo: 0
    // just put the source shape as the offseted one, no one will notice
    // it's also useless to compute the offset with a 0 radius
    
    const char *res_d = sp_repr_attr(SP_OBJECT(shape)->repr,"inkscape:original");
    if ( res_d ) {
      ArtBpath *bpath = sp_svg_read_path (res_d);
      SPCurve *c = sp_curve_new_from_bpath (bpath);
      sp_shape_set_curve_insync ((SPShape *) offset, c, TRUE);
      sp_curve_unref (c);
    }
    return;
  }
  
  // extra paraniac careful check. the preceding if () should take care of this case
  if (fabs (offset->rad) < 0.01)
    offset->rad = (offset->rad < 0) ? -0.01 : 0.01;
  
  Path *orig = new Path;
  orig->Copy ((Path *) offset->originalPath);
  
  if ( use_slow_but_correct_offset_method == false ) {
    // version par outline
    Shape *theShape = new Shape;
  Shape *theRes = new Shape;
  Path *originaux[1];
  Path *res = new Path;
  res->SetBackData (false);
  
  // et maintenant: offset
  float o_width;
  if (offset->rad >= 0)
  {
    o_width = offset->rad;
    orig->OutsideOutline (res, o_width, join_round, butt_straight, 20.0);
  }
  else
  {
    o_width = -offset->rad;
    orig->OutsideOutline (res, -o_width, join_round, butt_straight, 20.0);
  }
  
  if (o_width >= 1.0)
  {
    //      res->ConvertForOffset (1.0, orig, offset->rad);
    res->ConvertWithBackData (1.0);
  }
  else
  {
    //      res->ConvertForOffset (o_width, orig, offset->rad);
    res->ConvertWithBackData (o_width);
  }
  res->Fill (theShape, 0);
  theRes->ConvertToShape (theShape, fill_positive);
  originaux[0] = res;
  
  theRes->ConvertToForme (orig, 1, originaux);

		SPItem *item = &(shape->item);
		NR::Rect bbox = sp_item_bbox_desktop (item);
		if (bbox.topleft()[NR::X] < bbox.bottomright()[NR::X]) { // otherwise the bbox is invalid
			gdouble size = L2(bbox.dimensions());
			gdouble exp = NR::expansion(NR::Matrix(item->transform));
			if (exp != 0) 
				size /= exp;
			orig->Coalesce (size * 0.001);
			//g_print ("coa %g    exp %g    item %p\n", size * 0.001, exp, item);
		}

  
			//  if (o_width >= 1.0)
			//  {
			//    orig->Coalesce (0.1);  // small treshhold, since we only want to get rid of small segments
                           // the curve should already be computed by the Outline() function
 //   orig->ConvertEvenLines (1.0);
 //   orig->Simplify (0.5);
			//  }
			//  else
		//  {
		//          orig->Coalesce (0.1*o_width);
 //   orig->ConvertEvenLines (o_width);
 //   orig->Simplify (0.5 * o_width);
		//  }
  
  delete theShape;
  delete theRes;
  delete res;
  } else {
    // version par makeoffset
    Shape *theShape = new Shape;
    Shape *theRes = new Shape;
    
    
    // et maintenant: offset
    float o_width;
    if (offset->rad >= 0)
    {
      o_width = offset->rad;
    }
    else
    {
      o_width = -offset->rad;
    }
    
    // il faudrait avoir une mesure des details
    if (o_width >= 1.0)
    {
      orig->ConvertWithBackData (0.5);
    }
    else
    {
      orig->ConvertWithBackData (0.5*o_width);
    }
    orig->Fill (theShape, 0);
    theRes->ConvertToShape (theShape, fill_positive);
    Path *originaux[1];
    originaux[0]=orig;
    Path *res = new Path;
    theRes->ConvertToForme (res, 1, originaux);
    int    nbPart=0;
    Path** parts=res->SubPaths(nbPart,true);
    char   *holes=(char*)malloc(nbPart*sizeof(char));
    // we offsets contours separately, because we can.
    // this way, we avoid doing a unique big ConvertToShape when dealing with big shapes with lots of holes
    {
      Shape* onePart=new Shape;
      Shape* oneCleanPart=new Shape;
      theShape->Reset();
      for (int i=0;i<nbPart;i++) {
        double partSurf=parts[i]->Surface();
        parts[i]->Convert(1.0);
        {
          // raffiner si besoin
          double  bL,bT,bR,bB;
          parts[i]->PolylineBoundingBox(bL,bT,bR,bB);
          double  mesure=((bR-bL)+(bB-bT))*0.5;
          if ( mesure < 10.0 ) {
            parts[i]->Convert(0.02*mesure);
          }
        }
        if ( partSurf < 0 ) { // inverse par rapport a la realite
          // plein
          holes[i]=0;
          parts[i]->Fill(oneCleanPart,0);
          onePart->ConvertToShape(oneCleanPart,fill_positive); // there aren't intersections in that one, but maybe duplicate points and null edges
          oneCleanPart->MakeOffset(onePart,offset->rad,join_round,20.0);
          onePart->ConvertToShape(oneCleanPart,fill_positive);
          
          onePart->CalcBBox();
          double  typicalSize=0.5*((onePart->rightX-onePart->leftX)+(onePart->bottomY-onePart->topY));
          if ( typicalSize < 0.05 ) typicalSize=0.05;
          typicalSize*=0.01;
          if ( typicalSize > 1.0 ) typicalSize=1.0;
          onePart->ConvertToForme (parts[i]);
          parts[i]->ConvertEvenLines (typicalSize);
          parts[i]->Simplify (typicalSize);
          double nPartSurf=parts[i]->Surface();
          if ( nPartSurf >= 0 ) {
            // inversion de la surface -> disparait
            delete parts[i];
            parts[i]=NULL;
          } else {
          }
/*          int  firstP=theShape->nbPt;
          for (int j=0;j<onePart->nbPt;j++) theShape->AddPoint(onePart->pts[j].x);
          for (int j=0;j<onePart->nbAr;j++) theShape->AddEdge(firstP+onePart->aretes[j].st,firstP+onePart->aretes[j].en);*/
        } else {
          // trou
          holes[i]=1;
          parts[i]->Fill(oneCleanPart,0,false,true,true);
          onePart->ConvertToShape(oneCleanPart,fill_positive);
          oneCleanPart->MakeOffset(onePart,-offset->rad,join_round,20.0);
          onePart->ConvertToShape(oneCleanPart,fill_positive);
//          for (int j=0;j<onePart->nbAr;j++) onePart->Inverse(j); // pas oublier de reinverser
          
          onePart->CalcBBox();
          double  typicalSize=0.5*((onePart->rightX-onePart->leftX)+(onePart->bottomY-onePart->topY));
          if ( typicalSize < 0.05 ) typicalSize=0.05;
          typicalSize*=0.01;
          if ( typicalSize > 1.0 ) typicalSize=1.0;
          onePart->ConvertToForme (parts[i]);
          parts[i]->ConvertEvenLines (typicalSize);
          parts[i]->Simplify (typicalSize);
          double nPartSurf=parts[i]->Surface();
          if ( nPartSurf >= 0 ) {
            // inversion de la surface -> disparait
            delete parts[i];
            parts[i]=NULL;
          } else {
          }
          
 /*         int  firstP=theShape->nbPt;
          for (int j=0;j<onePart->nbPt;j++) theShape->AddPoint(onePart->pts[j].x);
          for (int j=0;j<onePart->nbAr;j++) theShape->AddEdge(firstP+onePart->aretes[j].en,firstP+onePart->aretes[j].st);*/
        }
//        delete parts[i];
      }
//      theShape->MakeOffset(theRes,offset->rad,join_round,20.0);
      delete onePart;
      delete oneCleanPart;
    }
    if ( nbPart > 1 ) {
      theShape->Reset();
      for (int i=0;i<nbPart;i++) {
        if ( parts[i] ) {
          parts[i]->ConvertWithBackData(1.0);
          if ( holes[i] ) {
            parts[i]->Fill(theShape,i,true,true,true);        
          } else {
            parts[i]->Fill(theShape,i,true,true,false);        
          }
        }
      }
      theRes->ConvertToShape (theShape, fill_positive);
      theRes->ConvertToForme (orig,nbPart,parts);
      for (int i=0;i<nbPart;i++) if ( parts[i] ) delete parts[i];
    } else if ( nbPart == 1 ) {
      orig->Copy(parts[0]);
      for (int i=0;i<nbPart;i++) if ( parts[i] ) delete parts[i];
    } else {
      orig->Reset();
    }
//    theRes->ConvertToShape (theShape, fill_positive);
//    theRes->ConvertToForme (orig);
    
/*    if (o_width >= 1.0) {
      orig->ConvertEvenLines (1.0);
      orig->Simplify (1.0);
    } else {
      orig->ConvertEvenLines (1.0*o_width);
      orig->Simplify (1.0 * o_width);
    }*/
    
    if ( parts ) free(parts);
    if ( holes ) free(holes);
    delete res;
    delete theShape;
    delete theRes;
  } 
  {
    char *res_d = NULL;
    if (orig->descr_nb <= 1)
    {
      // aie.... plus rien
      res_d = strdup ("M 0 0 L 0 0 z");
      printf("%s\n",res_d);
    }
    else
    {
      
      res_d = liv_svg_dump_path2 (orig);
    } 
    delete orig;
    
    ArtBpath *bpath = sp_svg_read_path (res_d);
    SPCurve *c = sp_curve_new_from_bpath (bpath);
    sp_shape_set_curve_insync ((SPShape *) offset, c, TRUE);
    sp_curve_unref (c);
    
    free (res_d);
  }
}


static int sp_offset_snappoints(SPItem *item, NR::Point p[], int size)
{
  if (((SPItemClass *) parent_class)->snappoints)
    return ((SPItemClass *) parent_class)->snappoints (item, p, size);
  
  return 0;
}


// utilitaires pour les poignees
// used to get the distance to the shape: distance to polygon give the fabs(radius), we still need
// the sign. for edges, it's easy to determine which side the point is on, for points of the polygon
// it's trickier: we need to identify which angle the point is in; to that effect, we take each 
// successive clockwise angle (A,C) and check if the vector B given by the point is in the angle or
// outside.
// another method would be to use the Winding() function to test whether the point is inside or outside 
// the polygon (it would be wiser to do so, in fact, but i like being stupid)
bool
vectors_are_clockwise (NR::Point A, NR::Point B, NR::Point C)
/* FIXME: This can be done using linear operations, more stably and
 *  faster.  method: transform A and C into B's space, A should be
 *  negative and B should be positive in the orthogonal component.  I
 *  think this is equivalent to 
 *  dot(A, rot90(B))*dot(C, rot90(B)) == -1.  
 *    -- njh */
{
  double ab_s = dot(A, rot90(B));
  double ab_c = dot(A, B);
  double bc_s = dot(B, rot90(C));
  double bc_c = dot(B, C);
  double ca_s = dot(C, rot90(A));
  double ca_c = dot(C, A);
  
  double ab_a = acos (ab_c);
  if (ab_c <= -1.0)
    ab_a = M_PI;
  if (ab_c >= 1.0)
    ab_a = 0;
  if (ab_s < 0)
    ab_a = 2 * M_PI - ab_a;
  double bc_a = acos (bc_c);
  if (bc_c <= -1.0)
    bc_a = M_PI;
  if (bc_c >= 1.0)
    bc_a = 0;
  if (bc_s < 0)
    bc_a = 2 * M_PI - bc_a;
  double ca_a = acos (ca_c);
  if (ca_c <= -1.0)
    ca_a = M_PI;
  if (ca_c >= 1.0)
    ca_a = 0;
  if (ca_s < 0)
    ca_a = 2 * M_PI - ca_a;
  
  double lim = 2 * M_PI - ca_a;
  
  if (ab_a < lim)
    return true;
  return false;
}
// distance to the original path; that funciton is called from object-edit to set the radius
// when the control knot moves.
// the sign of the result is the radius we're going to offset the shape with, so result > 0 ==outset
// and result < 0 ==inset. thus result<0 means 'px inside source'
double
sp_offset_distance_to_original (SPOffset * offset, NR::Point px)
{
  if (offset == NULL || offset->originalPath == NULL
      || ((Path *) offset->originalPath)->descr_nb <= 1)
    return 1.0;
  double dist = 1.0;
  Shape *theShape = new Shape;
  Shape *theRes = new Shape;
  
  // awfully damn stupid method: uncross the source path EACH TIME you need to compute the distance
  // the good way to do this would be to store the uncrossed source path somewhere, and delete it when the 
  // context is finished.
  // hopefully this part is much faster than actually computing the offset (which happen just after), so the
  // time spent in this function should end up being negligible with respect to the delay of one context
  // move
  ((Path *) offset->originalPath)->Convert (1.0);
  ((Path *) offset->originalPath)->Fill (theShape, 0);
  theRes->ConvertToShape (theShape, fill_oddEven);
  
  if (theRes->nbAr <= 1)
  {
    
  }
  else
  {
    double ptDist = -1.0;
    bool ptSet = false;
    double arDist = -1.0;
    bool arSet = false;
    // first get the minimum distance to the points
    for (int i = 0; i < theRes->nbPt; i++)
    {
      if (theRes->pts[i].dI + theRes->pts[i].dO > 0)
	    {
        NR::Point nx = theRes->pts[i].x;
        NR::Point nxpx = px-nx;
	      double ndist = sqrt (dot(nxpx,nxpx));
	      if (ptSet == false || fabs (ndist) < fabs (ptDist))
        {
          // we have a new minimum distance
          // now we need to wheck if px is inside or outside (for the sign)
          nx = px - theRes->pts[i].x;
          double nlen = sqrt (dot(nx , nx));
          nx /= nlen;
          int pb, cb, fb;
          fb = theRes->pts[i].lastA;
          pb = theRes->pts[i].lastA;
          cb = theRes->pts[i].firstA;
          do
          {
            // one angle
            NR::Point prx, nex;
            prx = theRes->aretes[pb].dx;
            nlen = sqrt (dot(prx, prx));
            prx /= nlen;
            nex = theRes->aretes[cb].dx;
            nlen = sqrt (dot(nex , nex));
            nex /= nlen;
            if (theRes->aretes[pb].en == i)
            {
              prx = -prx;
            }
            if (theRes->aretes[cb].en == i)
            {
              nex = -nex;
            }
            
            if (vectors_are_clockwise (nex, nx, prx))
            {
              // we're in that angle. set the sign, and exit that loop
              if (theRes->aretes[cb].st == i)
              {
                ptDist = -ndist;
                ptSet = true;
              }
              else
              {
                ptDist = ndist;
                ptSet = true;
              }
              break;
            }
            pb = cb;
            cb = theRes->NextAt (i, cb);
          }
          while (cb >= 0 && pb >= 0 && pb != fb);
        }
	    }
    }
    // loop over the edges to try to improve the distance
    for (int i = 0; i < theRes->nbAr; i++)
    {
      NR::Point sx = theRes->pts[theRes->aretes[i].st].x;
      NR::Point ex = theRes->pts[theRes->aretes[i].en].x;
      NR::Point nx = ex - sx;
      double len = sqrt (dot(nx,nx));
      if (len > 0.0001)
	    {
        NR::Point   pxsx=px-sx;
	      double ab = dot(nx,pxsx);
	      if (ab > 0 && ab < len * len)
        {
          // we're in the zone of influence of the segment
          double ndist = (cross(pxsx,nx)) / len;
          if (arSet == false || fabs (ndist) < fabs (arDist))
          {
            arDist = ndist;
            arSet = true;
          }
        }
	    }
    }
    if (arSet || ptSet)
    {
      if (arSet == false)
        arDist = ptDist;
      if (ptSet == false)
        ptDist = arDist;
      if (fabs (ptDist) < fabs (arDist))
        dist = ptDist;
      else
        dist = arDist;
    }
  }
  
  delete theShape;
  delete theRes;
  
  return dist;
}

// computes a point on the offset
// used to set a "seed" position for the control knot
// return the topmost point on the offset
void
sp_offset_top_point (SPOffset * offset, NR::Point *px)
{
  (*px) = NR::Point(0, 0);
  if (offset == NULL)
    return;
  
  if (offset->knotSet)
  {
    (*px) = offset->knot;
    return;
  }
  
  SPCurve *curve = sp_shape_get_curve (SP_SHAPE (offset));
  if (curve == NULL)
  {
    sp_offset_set_shape (SP_SHAPE (offset));
    curve = sp_shape_get_curve (SP_SHAPE (offset));
    if (curve == NULL)
      return;
  }
  
  Path *finalPath = bpath_to_liv_path (curve->bpath);
  if (finalPath == NULL)
  {
    sp_curve_unref (curve);
    return;
  }
  
  Shape *theShape = new Shape;
  
  finalPath->Convert (1.0);
  finalPath->Fill (theShape, 0);
  
  if (theShape->nbPt > 0)
  {
    theShape->SortPoints ();
    *px = theShape->pts[0].x;
  }
  
  delete theShape;
  delete finalPath;
  sp_curve_unref (curve);
}

// the listening functions
static void
sp_offset_source_attr_changed (SPRepr * repr, const gchar * key,
                               const gchar * /*oldval*/, const gchar * newval,
                               bool is_interactive, void * data)
{
  SPOffset *offset = (SPOffset *) data;
  if ( offset == NULL ) return;
  if (repr == NULL ||  repr != offset->sourceRepr ) {
    return; 
  }
  
  // deux dans lesquels on ne fera rien
  if (strcmp ((const char *) key, "id") == 0)
    return;
  if (strcmp ((const char *) key, "transform") == 0)
    return;
  if (strcmp ((const char *) key, "inkscape:original") == 0)
    return;
  
  Path *orig = NULL;
  // le bon cas: il y a un attribut d
  if (strcmp ((const char *) key, "d") == 0)
  {
    if (!newval)
      return;
    //    sp_repr_set_attr (SP_OBJECT(offset)->repr, "inkscape:original", newval);
    
    
    ArtBpath *bpath;
    SPCurve *curve;
    
    bpath = sp_svg_read_path (newval);
    curve = sp_curve_new_from_bpath (bpath);	// curve se chargera de detruire bpath
    orig = bpath_to_liv_path (curve->bpath);
    sp_curve_unref (curve);
  }
  else
  {
    // le mauvais cas: pas d'attribut d => il faut verifier que c'est une SPShape puis prendre le contour
    SPObject *refobj;
    refobj =
      sp_document_lookup_id (SP_OBJECT (offset)->document,
                             offset->sourceObject);
    SPItem *item = SP_ITEM (refobj);
    
    SPCurve *curve=NULL;
    if (!SP_IS_SHAPE (item) && !SP_IS_TEXT (item))
      return;
    if (SP_IS_SHAPE (item))
    {
      curve = sp_shape_get_curve (SP_SHAPE (item));
      if (curve == NULL)
        return;
    }
    if (SP_IS_TEXT (item))
    {
      curve = sp_text_normalized_bpath (SP_TEXT (item));
      if (curve == NULL)
        return;
    }
    orig = bpath_to_liv_path (curve->bpath);
    sp_curve_unref (curve);
  }
  
  // finalisons
  {
    SPCSSAttr *css;
    const gchar *val;
    Shape *theShape = new Shape;
    Shape *theRes = new Shape;
    
    orig->ConvertWithBackData (1.0);
    orig->Fill (theShape, 0);
    
    css = sp_repr_css_attr (repr /*SP_OBJECT_REPR (repr) */ , "style");
    val = sp_repr_css_property (css, "fill-rule", NULL);
    if (val && strcmp (val, "nonzero") == 0)
    {
      theRes->ConvertToShape (theShape, fill_nonZero);
    }
    else if (val && strcmp (val, "evenodd") == 0)
    {
      theRes->ConvertToShape (theShape, fill_oddEven);
    }
    else
    {
      theRes->ConvertToShape (theShape, fill_nonZero);
    }
    
    Path *originaux[1];
    originaux[0] = orig;
    Path *res = new Path;
    theRes->ConvertToForme (res, 1, originaux);
    
    delete theShape;
    delete theRes;
    
    char *res_d = liv_svg_dump_path2 (res);
    delete res;
    delete orig;
    
    sp_repr_set_attr (SP_OBJECT (offset)->repr, "inkscape:original", res_d);
    
    free (res_d);
  }
  
}

static void
sp_offset_source_destroy (SPRepr * repr, void *data)
{
  SPOffset *offset = (SPOffset *) data;
  if (offset == NULL)
    return;
  if ( repr == NULL || repr != offset->sourceRepr ) return;
  if (offset->sourceObject)
  {
    free (offset->sourceObject);
    offset->sourceObject = NULL;
  }
  if (offset->sourceRepr)
  {
    offset->sourceRepr = NULL;
    // pas besoin d'enlever le listener
  }
}
static void sp_offset_source_child_added (SPRepr *repr, SPRepr *child, SPRepr */*ref*/, void * data)
{
  SPOffset *offset = (SPOffset *) data;
  if (offset == NULL)
    return;
  if ( child == NULL ) return;
  if ( repr == NULL || offset->sourceRepr != repr ) return; // juste le premier niveau.
  
  sp_repr_add_listener (child,&offset_source_child_event_vector, offset);
  
  offset->sourceDirty=true;
  sp_object_request_update (SP_OBJECT(offset), SP_OBJECT_MODIFIED_FLAG);
}
static void sp_offset_source_child_removed (SPRepr *repr, SPRepr *child, SPRepr */*ref*/, void * data)
{
  SPOffset *offset = (SPOffset *) data;
  if (offset == NULL)
    return;
  if ( child == NULL ) return;
  if ( repr == NULL || offset->sourceRepr != repr ) return; // juste le premier niveau.
  
  sp_repr_remove_listener_by_data (child, offset);
  
  offset->sourceDirty=true;
  sp_object_request_update (SP_OBJECT(offset), SP_OBJECT_MODIFIED_FLAG);
}
static void sp_offset_source_content_changed (SPRepr */*repr*/, const gchar */*oldcontent*/, const gchar */*newcontent*/, void * data)
{
  SPOffset *offset = (SPOffset *) data;
  if (offset == NULL)
    return;
  
  offset->sourceDirty=true;
  sp_object_request_update (SP_OBJECT(offset), SP_OBJECT_MODIFIED_FLAG);
}

void   refresh_offset_source(SPOffset* offset)
{
  if ( offset == NULL ) return;
  offset->sourceDirty=false;
  Path *orig = NULL;
  
  // le mauvais cas: pas d'attribut d => il faut verifier que c'est une SPShape puis prendre le contour
  SPObject *refobj;
  refobj = sp_document_lookup_id (SP_OBJECT (offset)->document,
                                  offset->sourceObject);
  if ( refobj == NULL ) return;
  
  SPItem *item = SP_ITEM (refobj);
  
  SPCurve *curve=NULL;
  if (!SP_IS_SHAPE (item) && !SP_IS_TEXT (item))
  	return;
  if (SP_IS_SHAPE (item))
  {
    curve = sp_shape_get_curve (SP_SHAPE (item));
    if (curve == NULL)
      return;
  }
  if (SP_IS_TEXT (item))
 	{
 	  curve = sp_text_normalized_bpath (SP_TEXT (item));
 	  if (curve == NULL)
	    return;
  }
  orig = bpath_to_liv_path (curve->bpath);
  sp_curve_unref (curve);
  
  
  // finalisons
  {
    SPCSSAttr *css;
    const gchar *val;
    Shape *theShape = new Shape;
    Shape *theRes = new Shape;
    
    orig->ConvertWithBackData (1.0);
    orig->Fill (theShape, 0);
    
    css = sp_repr_css_attr (offset->sourceRepr , "style");
    val = sp_repr_css_property (css, "fill-rule", NULL);
    if (val && strcmp (val, "nonzero") == 0)
    {
      theRes->ConvertToShape (theShape, fill_nonZero);
    }
    else if (val && strcmp (val, "evenodd") == 0)
    {
      theRes->ConvertToShape (theShape, fill_oddEven);
    }
    else
    {
      theRes->ConvertToShape (theShape, fill_nonZero);
    }
    
    Path *originaux[1];
    originaux[0] = orig;
    Path *res = new Path;
    theRes->ConvertToForme (res, 1, originaux);
    
    delete theShape;
    delete theRes;
    
    char *res_d = liv_svg_dump_path2 (res);
    delete res;
    delete orig;
    
    sp_repr_set_attr (SP_OBJECT (offset)->repr, "inkscape:original", res_d);
    
    free (res_d);
  }
}



