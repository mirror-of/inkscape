#define __SP_OFFSET_C__

/*
 * <sodipodi:offset> implementation
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

#define noOFFSET_VERBOSE

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
static int sp_offset_snappoints (SPItem * item, NRPoint * p, int size);
static void sp_offset_set_shape (SPShape * shape);

Path *bpath_to_liv_path (ArtBpath * bpath);

void   refresh_offset_source(SPOffset* offset);

// pour recevoir les changements de l'objet source
static void sp_offset_source_attr_changed (SPRepr * repr, const gchar * key,
				    const gchar * oldval,
				    const gchar * newval, void *data);
static void sp_offset_source_destroy (SPRepr * repr, void *data);
static void sp_offset_source_child_added (SPRepr *repr, SPRepr *child, SPRepr *ref, void * data);
static void sp_offset_source_child_removed (SPRepr *repr, SPRepr *child, SPRepr *ref, void * data);
static void sp_offset_source_content_changed (SPRepr *repr, const gchar *oldcontent, const gchar *newcontent, void * data);

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
  GObjectClass *gobject_class;
  SPObjectClass *sp_object_class;
  SPItemClass *item_class;
  SPShapeClass *shape_class;

  gobject_class = (GObjectClass *) klass;
  sp_object_class = (SPObjectClass *) klass;
  item_class = (SPItemClass *) klass;
  shape_class = (SPShapeClass *) klass;

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

  sp_object_read_attr (object, "sodipodi:radius");
  sp_object_read_attr (object, "sodipodi:original");
  sp_object_read_attr (object, "xlink:href");
}

static SPRepr *
sp_offset_write (SPObject * object, SPRepr * repr, guint flags)
{
  SPOffset *offset;
  char *d;

  offset = SP_OFFSET (object);

  if ((flags & SP_OBJECT_WRITE_BUILD) && !repr)
    {
      repr = sp_repr_new ("path");
    }

  if (flags & SP_OBJECT_WRITE_EXT)
    {
      /* Fixme: we may replace these attributes by
       * sodipodi:offset="cx cy exp revo rad arg t0"
       */
      sp_repr_set_attr (repr, "sodipodi:type", "offset");
      sp_repr_set_double_attribute (repr, "sodipodi:radius", offset->rad);
      sp_repr_set_attr (repr, "sodipodi:original", offset->original);
      sp_repr_set_attr (repr, "xlink:href", offset->sourceObject);
    }

  d = sp_svg_write_path (((SPShape *) offset)->curve->bpath);
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
  SPItem *item;
  SPOffset *offset;

  item = (SPItem *) object;
  offset = (SPOffset *) object;

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

static void
sp_offset_set (SPObject * object, unsigned int key, const gchar * value)
{
  SPOffset *offset;
  SPShape *shape;
  gulong unit;

  offset = SP_OFFSET (object);
  shape = SP_SHAPE (object);

  if ( offset->sourceDirty ) refresh_offset_source(offset);

  /* fixme: we should really collect updates */
  switch (key)
    {
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
    case SP_ATTR_SODIPODI_RADIUS:
      if (!sp_svg_length_read_lff (value, &unit, NULL, &offset->rad) ||
	  (unit != SP_SVG_UNIT_EM) ||
	  (unit != SP_SVG_UNIT_EX) || (unit != SP_SVG_UNIT_PERCENT))
	{
	  if (fabs (offset->rad) < 0.25)
	    offset->rad = (offset->rad < 0) ? -0.25 : 0.25;
	  offset->knotSet = false;
	}
      sp_object_request_update (object, SP_OBJECT_MODIFIED_FLAG);
      break;
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

static void
sp_offset_update (SPObject * object, SPCtx * ctx, guint flags)
{
  SPOffset* offset=SP_OFFSET(object);
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

// duplique de splivarot
Path *
bpath_to_liv_path (ArtBpath * bpath)
{
  if (bpath == NULL)
    return NULL;

  Path *dest = new Path;
  dest->SetWeighted (false);
  dest->SetBackData (false);
  {
    int i;
    bool closed = false;
    float lastX, lastY;

    for (i = 0; bpath[i].code != ART_END; i++)
      {
	switch (bpath[i].code)
	  {
	  case ART_LINETO:
	    lastX = bpath[i].x3;
	    lastY = bpath[i].y3;
	    dest->LineTo (lastX, lastY);
	    break;

	  case ART_CURVETO:
	    dest->CubicTo (bpath[i].x3, bpath[i].y3,
			   3 * (bpath[i].x1 - lastX),
			   3 * (bpath[i].y1 - lastY),
			   3 * (bpath[i].x3 - bpath[i].x2),
			   3 * (bpath[i].y3 - bpath[i].y2));
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
	    dest->MoveTo (lastX, lastY);
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

gchar *
liv_svg_dump_path2 (Path * path)
{
  GString *result;
  result = g_string_sized_new (40);

  for (int i = 0; i < path->descr_nb; i++)
    {
      Path::path_descr theD = path->descr_data[i];
      int typ = theD.flags & descr_type_mask;
      if (typ == descr_moveto)
	{
	  g_string_sprintfa (result, "M %lf %lf ", theD.d.m.x, theD.d.m.y);
	}
      else if (typ == descr_lineto)
	{
	  g_string_sprintfa (result, "L %lf %lf ", theD.d.l.x, theD.d.l.y);
	}
      else if (typ == descr_cubicto)
	{
	  float lastX, lastY;
	  path->PrevPoint (i - 1, lastX, lastY);
	  g_string_sprintfa (result, "C %lf %lf %lf %lf %lf %lf ",
			     lastX + theD.d.c.stDx / 3,
			     lastY + theD.d.c.stDy / 3,
			     theD.d.c.x - theD.d.c.enDx / 3,
			     theD.d.c.y - theD.d.c.enDy / 3, theD.d.c.x,
			     theD.d.c.y);
	}
      else if (typ == descr_arcto)
	{
//                      g_string_sprintfa (result, "L %lf %lf ",theD.d.a.x,theD.d.a.y);
	  g_string_sprintfa (result, "A %g %g %g %i %i %g %g ", theD.d.a.rx,
			     theD.d.a.ry, theD.d.a.angle,
			     (theD.d.a.large) ? 1 : 0,
			     (theD.d.a.clockwise) ? 0 : 1, theD.d.a.x,
			     theD.d.a.y);
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
  SPOffset *offset;
  SPCurve *c;

  offset = SP_OFFSET (shape);

  sp_object_request_modified (SP_OBJECT (offset), SP_OBJECT_MODIFIED_FLAG);


#ifdef OFFSET_VERBOSE
  g_print ("rad=%g\n", offset->rad);
#endif
  // au boulot

  if (fabs (offset->rad) < 0.25)
    offset->rad = (offset->rad < 0) ? -0.25 : 0.25;

  Path *orig = new Path;
  orig->Copy ((Path *) offset->originalPath);

/*  {
  // version par outline
    Shape *theShape = new Shape;
    Shape *theRes = new Shape;
    Path *originaux[1];
    Path *res = new Path;
    res->SetWeighted (false);
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

    if (o_width >= 1.0)
      {
//        orig->Coalesce (1.0);
	orig->ConvertEvenLines (1.0);
	orig->Simplify (0.5);
      }
    else
      {
//      orig->Coalesce (o_width);
	orig->ConvertEvenLines (o_width);
	orig->Simplify (0.5 * o_width);
      }

    delete theShape;
    delete theRes;
    delete res;
  } */   
  {
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

    if (o_width >= 1.0)
      {
	orig->ConvertWithBackData (0.1);
      }
    else
      {
	orig->ConvertWithBackData (0.1*o_width);
      }
    orig->Fill (theShape, 0);
    theRes->ConvertToShape (theShape, fill_positive);
    theShape->MakeOffset(theRes,offset->rad,join_round,20.0);
    theRes->ConvertToShape (theShape, fill_positive);
    theRes->ConvertToForme (orig);

    if (o_width >= 1.0)
      {
	orig->ConvertEvenLines (0.1);
	orig->Simplify (0.5);
      }
    else
      {
	orig->ConvertEvenLines (0.1*o_width);
	orig->Simplify (0.5 * o_width);
      }

    delete theShape;
    delete theRes;
  }     
  
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
  c = sp_curve_new_from_bpath (bpath);
  sp_shape_set_curve_insync ((SPShape *) offset, c, TRUE);
  sp_curve_unref (c);

  free (res_d);
}


static int
sp_offset_snappoints (SPItem * item, NRPoint * p, int size)
{
  if (((SPItemClass *) parent_class)->snappoints)
    return ((SPItemClass *) parent_class)->snappoints (item, p, size);

  return 0;
}


// utilitaires pour les poignees

bool
vectors_are_clockwise (double ax, double ay, double bx, double by, double cx,
		       double cy)
{
  double ab_s = ay * bx - ax * by;
  double ab_c = ax * bx + ay * by;
  double bc_s = by * cx - bx * cy;
  double bc_c = bx * cx + by * cy;
  double ca_s = cy * ax - cx * ay;
  double ca_c = cx * ax + cy * ay;

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

double
sp_offset_distance_to_original (SPOffset * offset, double px, double py)
{
  if (offset == NULL || offset->originalPath == NULL
      || ((Path *) offset->originalPath)->descr_nb <= 1)
    return 1.0;
  double dist = 1.0;
  Shape *theShape = new Shape;
  Shape *theRes = new Shape;

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
      for (int i = 0; i < theRes->nbPt; i++)
	{
	  if (theRes->pts[i].dI + theRes->pts[i].dO > 0)
	    {
	      double nx = theRes->pts[i].x;
	      double ny = theRes->pts[i].y;
	      double ndist =
		sqrt ((px - nx) * (px - nx) + (py - ny) * (py - ny));
	      if (ptSet == false || fabs (ndist) < fabs (ptDist))
		{

		  nx = px - theRes->pts[i].x;
		  ny = py - theRes->pts[i].y;
		  double nlen = sqrt (nx * nx + ny * ny);
		  nx /= nlen;
		  ny /= nlen;
		  int pb, cb, fb;
		  fb = theRes->pts[i].lastA;
		  pb = theRes->pts[i].lastA;
		  cb = theRes->pts[i].firstA;
		  do
		    {
		      double prx, pry, nex, ney;
		      prx = theRes->aretes[pb].dx;
		      pry = theRes->aretes[pb].dy;
		      nlen = sqrt (prx * prx + pry * pry);
		      prx /= nlen;
		      pry /= nlen;
		      nex = theRes->aretes[cb].dx;
		      ney = theRes->aretes[cb].dy;
		      nlen = sqrt (nex * nex + ney * ney);
		      nex /= nlen;
		      ney /= nlen;
		      if (theRes->aretes[pb].en == i)
			{
			  prx = -prx;
			  pry = -pry;
			}
		      if (theRes->aretes[cb].en == i)
			{
			  nex = -nex;
			  ney = -ney;
			}

		      if (vectors_are_clockwise (nex, ney, nx, ny, prx, pry))
			{
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
      for (int i = 0; i < theRes->nbAr; i++)
	{
	  double sx = theRes->pts[theRes->aretes[i].st].x;
	  double sy = theRes->pts[theRes->aretes[i].st].y;
	  double ex = theRes->pts[theRes->aretes[i].en].x;
	  double ey = theRes->pts[theRes->aretes[i].en].y;
	  double nx = ex - sx;
	  double ny = ey - sy;
	  double len = sqrt (nx * nx + ny * ny);
	  if (len > 0.0001)
	    {
	      double ab = nx * (px - sx) + ny * (py - sy);
	      if (ab > 0 && ab < len * len)
		{
		  double ndist = (nx * (py - sy) - ny * (px - sx)) / len;
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

void
sp_offset_top_point (SPOffset * offset, double *px, double *py)
{
  *px = *py = 0;
  if (offset == NULL)
    return;

  if (offset->knotSet)
    {
      *px = offset->knotx;
      *py = offset->knoty;
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
      *py = theShape->pts[0].y;
    }

  delete theShape;
  delete finalPath;
  sp_curve_unref (curve);
}

static void
sp_offset_source_attr_changed (SPRepr * repr, const gchar * key,
			       const gchar * oldval, const gchar * newval,
			       void *data)
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
  if (strcmp ((const char *) key, "sodipodi:original") == 0)
    return;

  Path *orig = NULL;
  // le bon cas: il y a un attribut d
  if (strcmp ((const char *) key, "d") == 0)
    {
      if (!newval)
	return;
//    sp_repr_set_attr (SP_OBJECT(offset)->repr, "sodipodi:original", newval);


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

    sp_repr_set_attr (SP_OBJECT (offset)->repr, "sodipodi:original", res_d);

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
static void sp_offset_source_child_added (SPRepr *repr, SPRepr *child, SPRepr *ref, void * data)
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
static void sp_offset_source_child_removed (SPRepr *repr, SPRepr *child, SPRepr *ref, void * data)
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
static void sp_offset_source_content_changed (SPRepr *repr, const gchar *oldcontent, const gchar *newcontent, void * data)
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

     sp_repr_set_attr (SP_OBJECT (offset)->repr, "sodipodi:original", res_d);

     free (res_d);
   }
 }

 

