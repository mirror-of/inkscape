#define __SP_LIVAROT_C__
/*
 *  splivarot.cpp
 *  Inkscape
 *
 *  Created by fred on Fri Dec 05 2003.
 *  public domain
 *
 */

/*
 * contains lots of stitched pieces of path-chemistry.c
 */

#include <string.h>
#include <libart_lgpl/art_misc.h>
#include "xml/repr.h"
#include "svg/svg.h"
#include "sp-path.h"
#include "sp-text.h"
#include "style.h"
#include "inkscape.h"
#include "document.h"
#include "selection.h"
#include "desktop-handles.h"
#include "path-chemistry.h"
#include "desktop.h"
#include "splivarot.h"
#include "helper/canvas-bpath.h"
#include "helper/sp-intl.h"
#include "view.h"
#include "prefs-utils.h"

#include "libnr/nr-matrix.h"
#include "libnr/nr-point.h"
#include "xml/repr.h"
#include "xml/repr-private.h"

#include "livarot/Path.h"
#include "livarot/Shape.h"
#include "livarot/LivarotDefs.h"

Path   *Path_for_item (SPItem * item,bool doTransformation);
gchar  *liv_svg_dump_path (Path * path);
SPRepr *LCA (SPRepr * a, SPRepr * b);
bool   Ancetre (SPRepr * a, SPRepr * who);
SPRepr *AncetreFils (SPRepr * a, SPRepr * d);

void sp_selected_path_boolop (bool_op bop);
void sp_selected_path_do_offset (bool expand);
void sp_selected_path_create_offset_object (int expand,bool updating);

void
sp_selected_path_union ()
{
  sp_selected_path_boolop (bool_op_union);
}

void
sp_selected_path_intersect ()
{
  sp_selected_path_boolop (bool_op_inters);
}

void
sp_selected_path_diff ()
{
  sp_selected_path_boolop (bool_op_diff);
}

void
sp_selected_path_symdiff ()
{
  sp_selected_path_boolop (bool_op_symdiff);
}

void
sp_selected_path_boolop (bool_op bop)
{
  SPDesktop *desktop;
  SPSelection *selection;
  GSList *il;
  GSList *l;
  SPRepr *repr;
  SPItem *item;
  gchar *d, *style;
  
  desktop = SP_ACTIVE_DESKTOP;
  if (!SP_IS_DESKTOP (desktop))
    return;
  selection = SP_DT_SELECTION (desktop);
  
  il = (GSList *) sp_selection_item_list (selection);
  
  if (g_slist_length (il) < 2) {
    sp_view_set_statusf_error(SP_VIEW(desktop), _("Select at least 2 paths to perform a boolean operation."));
    return;
  }
  
  if (g_slist_length (il) > 2)
  {
    if (bop == bool_op_diff || bop == bool_op_symdiff)
    {
      sp_view_set_statusf_error(SP_VIEW(desktop), _("Select exactly 2 paths to perform difference or XOR."));
      return;
    }
  }
  
  
  bool reverseOrderForOp = false;
  // mettre les elements de la liste dans l'ordre pour ces operations
  if (bop == bool_op_diff || bop == bool_op_symdiff)
  {
    SPRepr *a = SP_OBJECT_REPR (il->data);
    SPRepr *b = SP_OBJECT_REPR (il->next->data);
    if (a == NULL || b == NULL)
    {
      sp_view_set_statusf_error(SP_VIEW(desktop), _("Unable to determine the z-order of the objects selected for difference."));
      return;
    }
    if (Ancetre (a, b))
    {
    }
    else if (Ancetre (b, a))
    {
      // mauvais sens
      reverseOrderForOp = true;
    }
    else
    {
      SPRepr *dad = LCA (a, b);
      if (dad == NULL)
	    {
        sp_view_set_statusf_error(SP_VIEW(desktop), _("Unable to determine the z-order of the objects selected for difference."));
	      return;
	    }
      SPRepr *as = AncetreFils (a, dad);
      SPRepr *bs = AncetreFils (b, dad);
      for (SPRepr * child = dad->children; child; child = child->next)
	    {
	      if (child == as)
        {
          // a en premier->mauvais sens
          reverseOrderForOp = true;
          break;
        }
	      if (child == bs)
          break;
	    }
    }
  }
  
  il = g_slist_copy (il);
  
  for (l = il; l != NULL; l = l->next)
  {
    item = (SPItem *) l->data;
    if (!SP_IS_SHAPE (item) && !SP_IS_TEXT (item))
    {
      sp_view_set_statusf_error(SP_VIEW(desktop), _("One of the objects is not a path, cannot perform boolean operation."));
      g_slist_free (il);
      return;
    }
  }
  
  // choper les originaux pour faire l'operation demande
  int nbOriginaux = g_slist_length (il);
  Path *originaux[nbOriginaux];
  int curOrig;
  {
    curOrig = 0;
    for (l = il; l != NULL; l = l->next)
    {
      originaux[curOrig] = Path_for_item ((SPItem *) l->data,true);
      if (originaux[curOrig] == NULL || originaux[curOrig]->descr_nb <= 1)
      {
        for (int i = curOrig; i >= 0; i--)
          delete originaux[i];
        g_slist_free (il);
        return;
      }
      curOrig++;
    }
  }
  
  Shape *theShapeA = new Shape;
  Shape *theShapeB = new Shape;
  Shape *theShape = new Shape;
  Path *res = new Path;
  res->SetBackData (false);
  
  {
    SPCSSAttr *css;
    const gchar *val;
    
    originaux[0]->ConvertWithBackData (1.0);
     
    originaux[0]->Fill (theShape, 0);
 /*   { // fait dans PathForItem
      NRMatrix i2root;
      sp_item_i2root_affine (SP_ITEM (il->data), &i2root);
      for (int i = 0; i < theShape->nbPt; i++)
      {
        NR::Point x;
        x[0]=
        i2root.c[0] * theShape->pts[i].x[0] +
        i2root.c[2] * theShape->pts[i].x[1] + i2root.c[4];
        x[1] =
          i2root.c[1] * theShape->pts[i].x[0] +
          i2root.c[3] * theShape->pts[i].x[1] + i2root.c[5];
        theShape->pts[i].x[0] = x[0];
        theShape->pts[i].x[1] = x[1];
      }
    }*/
    css = sp_repr_css_attr (SP_OBJECT_REPR (il->data), "style");
    val = sp_repr_css_property (css, "fill-rule", NULL);
    if (val && strcmp (val, "nonzero") == 0)
    {
      theShapeA->ConvertToShape (theShape, fill_nonZero);
    }
    else if (val && strcmp (val, "evenodd") == 0)
    {
      theShapeA->ConvertToShape (theShape, fill_oddEven);
    }
    else
    {
      theShapeA->ConvertToShape (theShape, fill_nonZero);
    }
  }
  
  curOrig = 1;
  for (l = il->next; l != NULL; l = l->next)
  {
    SPCSSAttr *css;
    const gchar *val;
    
    originaux[curOrig]->ConvertWithBackData (1.0);
    
    originaux[curOrig]->Fill (theShape, curOrig);
    
/*    {
      NRMatrix i2root;
      sp_item_i2root_affine (SP_ITEM (l->data), &i2root);
      for (int i = 0; i < theShape->nbPt; i++)
      {
        NR::Point x;
        x[0]=
	      i2root.c[0] * theShape->pts[i].x[0] +
	      i2root.c[2] * theShape->pts[i].x[1] + i2root.c[4];
        x[1] =
          i2root.c[1] * theShape->pts[i].x[0] +
          i2root.c[3] * theShape->pts[i].x[1] + i2root.c[5];
        theShape->pts[i].x[0] = x[0];
        theShape->pts[i].x[1] = x[1];
      }
    }*/
    css = sp_repr_css_attr (SP_OBJECT_REPR (l->data), "style");
    val = sp_repr_css_property (css, "fill-rule", NULL);
    if (val && strcmp (val, "nonzero") == 0)
    {
      theShapeB->ConvertToShape (theShape, fill_nonZero);
    }
    else if (val && strcmp (val, "evenodd") == 0)
    {
      theShapeB->ConvertToShape (theShape, fill_oddEven);
    }
    else
    {
      theShapeB->ConvertToShape (theShape, fill_nonZero);
    }
    
    // les elements arrivent en ordre inverse dans la liste
    if (reverseOrderForOp)
    {
      theShape->Booleen (theShapeA, theShapeB, bop);
    }
    else
    {
      theShape->Booleen (theShapeB, theShapeA, bop);
    }
    
    {
      Shape *swap = theShape;
      theShape = theShapeA;
      theShapeA = swap;
    }
    curOrig++;
  }
  // pour compenser le swap juste avant
  {
    Shape *swap = theShape;
    theShape = theShapeA;
    theShapeA = swap;
  }
  
  theShape->ConvertToForme (res, nbOriginaux, originaux);
  
  delete theShape;
  delete theShapeA;
  delete theShapeB;
  for (int i = 0; i < nbOriginaux; i++)
    delete originaux[i];
  
  if (res->descr_nb <= 1)
  {
    // pas vraiment de points sur le resultat               
    // donc il ne reste rien
    for (l = il; l != NULL; l = l->next)
    {
      sp_repr_unparent (SP_OBJECT_REPR (l->data));
    }
    sp_document_done (SP_DT_DOCUMENT (desktop));
    sp_selection_empty (selection);
    
    delete res;
    g_slist_free (il);
    return;
  }
  d = liv_svg_dump_path (res);
  delete res;
  
  if (reverseOrderForOp)
  {
    style = g_strdup (sp_repr_attr ((SP_OBJECT (il->data))->repr, "style"));
  }
  else
  {
    style =
    g_strdup (sp_repr_attr ((SP_OBJECT (il->next->data))->repr, "style"));
  }
  
  for (l = il; l != NULL; l = l->next)
  {
    sp_repr_unparent (SP_OBJECT_REPR (l->data));
  }
  
  g_slist_free (il);
  
  repr = sp_repr_new ("path");
  sp_repr_set_attr (repr, "style", style);
  g_free (style);
  sp_repr_set_attr (repr, "d", d);
  g_free (d);
  item = (SPItem *) sp_document_add_repr (SP_DT_DOCUMENT (desktop), repr);
  sp_document_done (SP_DT_DOCUMENT (desktop));
  sp_repr_unref (repr);
  
  sp_selection_empty (selection);
  sp_selection_set_item (selection, item);
}

void
sp_selected_path_outline ()
{
  SPSelection *selection;
  SPRepr *repr;
  SPItem *item;
  SPCurve *curve;
  gchar *style, *str;
  SPDesktop *desktop;
  float o_width, o_miter;
  JoinType o_join;
  ButtType o_butt;
  NRMatrix i2root;
  
  curve = NULL;
  
  desktop = SP_ACTIVE_DESKTOP;
  if (!SP_IS_DESKTOP (desktop))
    return;
  
  selection = SP_DT_SELECTION (desktop);
  
  item = sp_selection_item (selection);
  
  if (item == NULL || ( !SP_IS_SHAPE (item) && !SP_IS_TEXT (item) ) ) {
    sp_view_set_statusf_error(SP_VIEW(desktop), _("Selected object is not a path, cannot outline."));
    return;
  }
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
  
  {
    SPCSSAttr *css;
    const gchar *val;
    
    css = sp_repr_css_attr (SP_OBJECT_REPR (item), "style");
    val = sp_repr_css_property (css, "stroke", NULL);
    
    if (val == NULL || strcmp (val, "none") == 0)
    {
      // pas de stroke pas de chocolat
      sp_curve_unref (curve);
      sp_view_set_statusf_error(SP_VIEW(desktop), _("Selected object is not stroked, cannot outline."));
      return;
    }
  }
  
  sp_item_i2root_affine (item, &i2root);
  style = g_strdup (sp_repr_attr (SP_OBJECT (item)->repr, "style"));
  
  {
    SPStyle *i_style = SP_OBJECT (item)->style;
    int jointype, captype;
    
    o_width = 1.0;
    o_miter = 4 * o_width;
    o_join = join_straight;
    o_butt = butt_straight;
    
    jointype = i_style->stroke_linejoin.value;
    captype = i_style->stroke_linecap.value;
    o_width = i_style->stroke_width.computed;
    if (jointype == SP_STROKE_LINEJOIN_MITER)
    {
      o_join = join_pointy;
    }
    else if (jointype == SP_STROKE_LINEJOIN_ROUND)
    {
      o_join = join_round;
    }
    else
    {
      o_join = join_straight;
    }
    if (captype == SP_STROKE_LINECAP_SQUARE)
    {
      o_butt = butt_square;
    }
    else if (captype == SP_STROKE_LINECAP_ROUND)
    {
      o_butt = butt_round;
    }
    else
    {
      o_butt = butt_straight;
    }
    if (o_width < 0.1)
      o_width = 0.1;
    o_miter = 4 * o_width;
  }
  
  Path *orig = Path_for_item (item,false);
  if (orig == NULL)
  {
    g_free (style);
    sp_curve_unref (curve);
    sp_view_set_statusf_error(SP_VIEW(desktop), _("Selected object has no curve, cannot outline."));
    return;
  }
  
  Path *res = new Path;
  res->SetBackData (false);
  
  sp_curve_unref (curve);
  
  {
    
    orig->Outline (res, 0.5 * o_width, o_join, o_butt, o_miter);
    
    //              orig->ConvertEvenLines(0.1*o_width);
    //              orig->Simplify(0.05*o_width);
    orig->Coalesce (0.5 * o_width);
    
    
    Shape *theShape = new Shape;
    Shape *theRes = new Shape;
    
    res->ConvertWithBackData (1.0);
    res->Fill (theShape, 0);
    theRes->ConvertToShape (theShape, fill_positive);
    
    Path *originaux[1];
    originaux[0] = res;
    theRes->ConvertToForme (orig, 1, originaux);
    
    delete theShape;
    delete theRes;
    
  }
  
  /*	{
		Shape*  theIn=new Shape;
		Shape*  theOut=new Shape;
		Shape*  theShape=new Shape;
		
  
		orig->Convert(1.0);
		
		orig->Fill(theShape,0);
		theIn->ConvertToShape(theShape,fill_oddEven);
		theShape->MakeOffset(theIn,-0.5*o_width,o_join,o_miter);
		theIn->ConvertToShape(theShape,fill_positive);
		
		orig->Fill(theShape,0);
		theOut->ConvertToShape(theShape,fill_oddEven);
		theShape->MakeOffset(theOut,0.5*o_width,o_join,o_miter);
		theOut->ConvertToShape(theShape,fill_positive);
		
		theShape->Booleen(theOut,theIn,bool_op_diff);
		
		theShape->ConvertToForme(orig);
		orig->ConvertEvenLines(0.5*o_width);
		orig->Simplify(0.25*o_width);
		
		delete theShape;
		delete theIn;
		delete theOut;
		
	}*/
  
  if (orig->descr_nb <= 1)
  {
    // ca a merd, ou bien le resultat est vide
    delete res;
    delete orig;
    g_free (style);
    
    sp_view_set_statusf_error(SP_VIEW(desktop), _("Outline of object could not be computed."));
    return;
  }
  
  sp_repr_unparent (SP_OBJECT_REPR (item));
  
  if (orig->descr_nb <= 1)
  {
    // pas vraiment de points sur le resultat               
    // donc il ne reste rien
    sp_document_done (SP_DT_DOCUMENT (desktop));
    sp_selection_empty (selection);
    
    delete res;
    delete orig;
    g_free (style);
    return;
  }
  
  {
    gchar tstr[80];
    
    tstr[79] = '\0';
    
    repr = sp_repr_new ("path");
    if (sp_svg_transform_write (tstr, 80, &i2root))
    {
      sp_repr_set_attr (repr, "transform", tstr);
    }
    else
    {
      sp_repr_set_attr (repr, "transform", NULL);
    }
    
    sp_repr_set_attr (repr, "style", style);
    
    str = liv_svg_dump_path (orig);
    sp_repr_set_attr (repr, "d", str);
    g_free (str);
    item = (SPItem *) sp_document_add_repr (SP_DT_DOCUMENT (desktop), repr);
    sp_repr_unref (repr);
    sp_selection_empty (selection);
    sp_selection_add_item (selection, item);
    
    {
      SPCSSAttr *ocss;
      SPCSSAttr *css;
      const gchar *val;
      const gchar *opac;
      
      ocss = sp_repr_css_attr (SP_OBJECT_REPR (item), "style");
      val = sp_repr_css_property (ocss, "stroke", NULL);
      opac = sp_repr_css_property (ocss, "stroke-opacity", NULL);
      
      css = sp_repr_css_attr_new ();
      
      sp_repr_css_set_property (css, "stroke", "none");
      sp_repr_css_set_property (css, "stroke-opacity", "1.0");
      sp_repr_css_set_property (css, "fill", val);
      if ( opac ) {
        sp_repr_css_set_property (css, "fill-opacity", opac);
      } else {
        sp_repr_css_set_property (css, "fill-opacity", "1.0");
      }
      sp_repr_css_change (SP_OBJECT_REPR (item), css, "style");
      
      sp_repr_css_attr_unref (css);
    }
  }
  
  sp_document_done (SP_DT_DOCUMENT (desktop));
  
  delete res;
  delete orig;
  
  g_free (style);
}

void
sp_selected_path_offset ()
{
  sp_selected_path_do_offset (true);
}
void
sp_selected_path_inset ()
{
  sp_selected_path_do_offset (false);
}

void sp_selected_path_create_offset_object_zero ()
{
  sp_selected_path_create_offset_object (0, false);
}

void sp_selected_path_create_offset ()
{
  sp_selected_path_create_offset_object (1, false);
}
void sp_selected_path_create_inset ()
{
  sp_selected_path_create_offset_object (-1, false);
}

void sp_selected_path_create_updating_offset_object_zero ()
{
  sp_selected_path_create_offset_object (0, true);
}

void sp_selected_path_create_updating_offset ()
{
  sp_selected_path_create_offset_object (1,true);
}
void sp_selected_path_create_updating_inset ()
{
  sp_selected_path_create_offset_object (-1,true);
}

void
sp_selected_path_create_offset_object (int expand,bool updating)
{
  SPSelection *selection;
  SPRepr *repr;
  SPItem *item;
  SPCurve *curve;
  gchar *style, *str;
  SPDesktop *desktop;
  float o_width, o_miter;
  JoinType o_join;
  ButtType o_butt;
  NRMatrix i2root;
  
  curve = NULL;
  
  desktop = SP_ACTIVE_DESKTOP;
  if (!SP_IS_DESKTOP (desktop))
    return;
  
  selection = SP_DT_SELECTION (desktop);
  
  item = sp_selection_item (selection);
  
  if (item == NULL || ( !SP_IS_SHAPE (item) && !SP_IS_TEXT (item) ) ) {
    sp_view_set_statusf_error(SP_VIEW(desktop), _("Selected object is not a path, cannot inset/outset."));
    return;
  }
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
  
  sp_item_i2root_affine (item, &i2root);
  style = g_strdup (sp_repr_attr (SP_OBJECT (item)->repr, "style"));
  
  {
    SPStyle *i_style = SP_OBJECT (item)->style;
    int jointype, captype;
    
    o_width = 1.0;
    o_miter = 4 * o_width;
    o_join = join_straight;
    o_butt = butt_straight;
    
    jointype = i_style->stroke_linejoin.value;
    captype = i_style->stroke_linecap.value;
    o_width = i_style->stroke_width.computed;
    if (jointype == SP_STROKE_LINEJOIN_MITER)
    {
      o_join = join_pointy;
    }
    else if (jointype == SP_STROKE_LINEJOIN_ROUND)
    {
      o_join = join_round;
    }
    else
    {
      o_join = join_straight;
    }
    if (captype == SP_STROKE_LINECAP_SQUARE)
    {
      o_butt = butt_square;
    }
    else if (captype == SP_STROKE_LINECAP_ROUND)
    {
      o_butt = butt_round;
    }
    else
    {
      o_butt = butt_straight;
    }
    
    {
      double    prefOffset=1.0;
      prefOffset=prefs_get_double_attribute("options.defaultoffsetwidth","value",prefOffset);
      o_width=prefOffset;
    }
    
    if (o_width < 0.25)
      o_width = 0.25;               
    o_miter = 4 * o_width;
  }
  
  Path *orig = Path_for_item (item,true);
  if (orig == NULL)
  {
    g_free (style);
    sp_curve_unref (curve);
    return;
  }
  
  Path *res = new Path;
  res->SetBackData (false);
  
  {
    SPCSSAttr *css;
    const gchar *val;
    
    Shape *theShape = new Shape;
    Shape *theRes = new Shape;
    
    orig->ConvertWithBackData (1.0);
    orig->Fill (theShape, 0);
    
    css = sp_repr_css_attr (SP_OBJECT_REPR (item), "style");
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
    theRes->ConvertToForme (res, 1, originaux);
    
    delete theShape;
    delete theRes;
  }
  
  sp_curve_unref (curve);
  if ( updating ) {
    // on conserve l'original
  } else {
    sp_repr_unparent (SP_OBJECT_REPR (item));
  }
  
  if (res->descr_nb <= 1)
  {
    // pas vraiment de points sur le resultat
    // donc il ne reste rien
    sp_document_done (SP_DT_DOCUMENT (desktop));
    sp_selection_empty (selection);
    
    delete res;
    delete orig;
    g_free (style);
    return;
  }
  
  {
    //              SPCSSAttr *css;
    //              const gchar *val;
    gchar tstr[80];
    
    tstr[79] = '\0';
    
    repr = sp_repr_new ("path");
    sp_repr_set_attr (repr, "sodipodi:type", "inkscape:offset");
    if (expand > 0)
      {
	sp_repr_set_double (repr, "inkscape:radius", o_width);
      }
    else if (expand < 0)
      {
	sp_repr_set_double (repr, "inkscape:radius", -o_width);
      }
    else 
      {
	sp_repr_set_double (repr, "inkscape:radius",  0);
      }

    str = liv_svg_dump_path (res);
    sp_repr_set_attr (repr, "inkscape:original", str);
    g_free (str);
    
    if ( updating ) {
      sp_repr_set_attr (repr, "inkscape:href", sp_repr_attr(SP_OBJECT(item)->repr,"id"));
    } else {
      sp_repr_set_attr (repr, "inkscape:href", NULL);
    }
    
    // pas de transformation, sinon offset pas correct
/*    if (sp_svg_transform_write (tstr, 80, &i2root))
    {
      sp_repr_set_attr (repr, "transform", tstr);
    }
    else
    {
      sp_repr_set_attr (repr, "transform", NULL);
    }*/
    
    sp_repr_set_attr (repr, "style", style);
    SPItem* nitem = (SPItem *) sp_document_add_repr (SP_DT_DOCUMENT (desktop), repr);
    sp_repr_unref (repr);
    sp_selection_empty (selection);
    sp_selection_add_item (selection, nitem);
    
  }
  
  sp_document_done (SP_DT_DOCUMENT (desktop));
  
  delete res;
  delete orig;
  
  g_free (style);
}

void
sp_selected_path_do_offset (bool expand)
{
  SPSelection *selection;
  SPRepr *repr;
  SPItem *item;
  SPCurve *curve;
  gchar *style, *str;
  SPDesktop *desktop;
  float o_width, o_miter;
  JoinType o_join;
  ButtType o_butt;
  NRMatrix i2root;
  
  curve = NULL;
  
  desktop = SP_ACTIVE_DESKTOP;
  if (!SP_IS_DESKTOP (desktop))
    return;
  
  selection = SP_DT_SELECTION (desktop);
  
  item = sp_selection_item (selection);
  
  if (item == NULL || (!SP_IS_SHAPE (item) && !SP_IS_TEXT (item))) {
    sp_view_set_statusf_error(SP_VIEW(desktop), _("Selected object is not a path, cannot inset/outset."));
    return;
  }
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
  
  /*  {
    SPCSSAttr *css;
  const gchar *val;
  
  css = sp_repr_css_attr (SP_OBJECT_REPR (item), "style");
  val = sp_repr_css_property (css, "stroke", NULL);
  
  if (val == NULL || strcmp (val, "none") == 0)
  {
    // pas de stroke pas de chocolat
	  sp_view_set_statusf_error(SP_VIEW(desktop),"the offset/inset operation uses the stroke width as offset value: give a stroke to the object");
    sp_curve_unref (curve);
    return;
  }
  }*/
  
  sp_item_i2root_affine (item, &i2root);
  style = g_strdup (sp_repr_attr (SP_OBJECT (item)->repr, "style"));
  
  {
    SPStyle *i_style = SP_OBJECT (item)->style;
    int jointype, captype;
    
    o_width = 1.0;
    o_miter = 4 * o_width;
    o_join = join_straight;
    o_butt = butt_straight;
    
    jointype = i_style->stroke_linejoin.value;
    captype = i_style->stroke_linecap.value;
    o_width = i_style->stroke_width.computed;
    if (jointype == SP_STROKE_LINEJOIN_MITER)
    {
      o_join = join_pointy;
    }
    else if (jointype == SP_STROKE_LINEJOIN_ROUND)
    {
      o_join = join_round;
    }
    else
    {
      o_join = join_straight;
    }
    if (captype == SP_STROKE_LINECAP_SQUARE)
    {
      o_butt = butt_square;
    }
    else if (captype == SP_STROKE_LINECAP_ROUND)
    {
      o_butt = butt_round;
    }
    else
    {
      o_butt = butt_straight;
    }
    
    // recuperer l'offset dans les preferences
    {
      double    prefOffset=1.0;
      prefOffset=prefs_get_double_attribute("options.defaultoffsetwidth","value",prefOffset);
      o_width=prefOffset;
    }
    
    if (o_width < 0.1)
      o_width = 0.1;
    o_miter = 4 * o_width;
  }
  
  Path *orig = Path_for_item (item,true);
  if (orig == NULL)
  {
    g_free (style);
    sp_curve_unref (curve);
    return;
  }
  
  Path *res = new Path;
  res->SetBackData (false);
  
  {
    SPCSSAttr *css;
    const gchar *val;
    
    Shape *theShape = new Shape;
    Shape *theRes = new Shape;
    
    orig->ConvertWithBackData (1.0);
    orig->Fill (theShape, 0);
    
    css = sp_repr_css_attr (SP_OBJECT_REPR (item), "style");
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
    theRes->ConvertToForme (res, 1, originaux);
    
    // et maintenant: offset
    // methode inexacte
 /*      if (expand)
    {
      res->OutsideOutline (orig, 0.5 * o_width, o_join, o_butt, o_miter);
    }
    else
    {
      res->OutsideOutline (orig, -0.5 * o_width, o_join, o_butt, o_miter);
    }
    
    orig->ConvertWithBackData (1.0);
    orig->Fill (theShape, 0);
    theRes->ConvertToShape (theShape, fill_positive);
    originaux[0] = orig;
    theRes->ConvertToForme (res, 1, originaux);
    
    if (o_width >= 0.5)
    {
 //     res->Coalesce (1.0);
      res->ConvertEvenLines (1.0);
      res->Simplify (1.0);
    }
    else
    {
//      res->Coalesce (o_width);
      res->ConvertEvenLines (1.0*o_width);
      res->Simplify (1.0 * o_width);
    }*/
    // methode par makeoffset
    if (expand)
    {
      theShape->MakeOffset(theRes, o_width, o_join, o_miter);
    }
    else
    {
      theShape->MakeOffset(theRes, -o_width, o_join, o_miter);
    }
    theRes->ConvertToShape(theShape,fill_positive);
    
    res->Reset();
    theRes->ConvertToForme (res);
        
    if (o_width >= 1.0)
    {
      res->ConvertEvenLines (1.0);
      res->Simplify (1.0);
    }
    else
    {
      res->ConvertEvenLines (1.0*o_width);
      res->Simplify (1.0 * o_width);
    }
    
    delete theShape;
    delete theRes;
  }
  
  sp_curve_unref (curve);
  sp_repr_unparent (SP_OBJECT_REPR (item));
  
  if (res->descr_nb <= 1)
  {
    // pas vraiment de points sur le resultat               
    // donc il ne reste rien
    sp_document_done (SP_DT_DOCUMENT (desktop));
    sp_selection_empty (selection);
    
    delete res;
    delete orig;
    g_free (style);
    return;
  }
  
  {
    //              SPCSSAttr *css;
    //              const gchar *val;
    gchar tstr[80];
    
    tstr[79] = '\0';
    
    repr = sp_repr_new ("path");
/*    if (sp_svg_transform_write (tstr, 80, &i2root))
    {
      sp_repr_set_attr (repr, "transform", tstr);
    }
    else
    {
      sp_repr_set_attr (repr, "transform", NULL);
    }*/
    
    sp_repr_set_attr (repr, "style", style);
    str = liv_svg_dump_path (res);
    sp_repr_set_attr (repr, "d", str);
    g_free (str);
    item = (SPItem *) sp_document_add_repr (SP_DT_DOCUMENT (desktop), repr);
    sp_repr_unref (repr);
    sp_selection_empty (selection);
    sp_selection_add_item (selection, item);
    
  }
  
  sp_document_done (SP_DT_DOCUMENT (desktop));
  
  delete res;
  delete orig;
  
  g_free (style);
}

void
sp_selected_path_simplify ()
{
  SPSelection *selection;
  SPRepr *repr;
  SPItem *item;
  SPCurve *curve;
  gchar *style, *str;
  SPDesktop *desktop;
  NRMatrix i2root;
  
  curve = NULL;
  
  desktop = SP_ACTIVE_DESKTOP;
  if (!SP_IS_DESKTOP (desktop))
    return;
  
  selection = SP_DT_SELECTION (desktop);
  
  item = sp_selection_item (selection);
  
  if (item == NULL)
    return;
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
  
  sp_item_i2root_affine (item, &i2root);
  style = g_strdup (sp_repr_attr (SP_OBJECT (item)->repr, "style"));
  
  Path *orig = Path_for_item (item,false);
  if (orig == NULL)
  {
    g_free (style);
    sp_curve_unref (curve);
    return;
  }
  
  sp_curve_unref (curve);
  sp_repr_unparent (SP_OBJECT_REPR (item));
  
  {
    orig->ConvertEvenLines (1.0);
    orig->Simplify (0.5);
  }
  
  {
    gchar tstr[80];
    
    tstr[79] = '\0';
    
    repr = sp_repr_new ("path");
    if (sp_svg_transform_write (tstr, 80, &i2root))
    {
      sp_repr_set_attr (repr, "transform", tstr);
    }
    else
    {
      sp_repr_set_attr (repr, "transform", NULL);
    }
    
    sp_repr_set_attr (repr, "style", style);
    
    str = liv_svg_dump_path (orig);
    sp_repr_set_attr (repr, "d", str);
    g_free (str);
    item = (SPItem *) sp_document_add_repr (SP_DT_DOCUMENT (desktop), repr);
    sp_repr_unref (repr);
    sp_selection_empty (selection);
    sp_selection_add_item (selection, item);
    
  }
  
  sp_document_done (SP_DT_DOCUMENT (desktop));
  
  delete orig;
  
  g_free (style);
  
}


// fonctions utilitaires
SPRepr *
AncetreFils (SPRepr * a, SPRepr * d)
{
  if (a == NULL || d == NULL)
    return NULL;
  if (sp_repr_parent (a) == d)
    return a;
  return AncetreFils (sp_repr_parent (a), d);
}

bool
Ancetre (SPRepr * a, SPRepr * who)
{
  if (who == NULL || a == NULL)
    return false;
  if (who == a)
    return true;
  return Ancetre (sp_repr_parent (a), who);
}

SPRepr *
LCA (SPRepr * a, SPRepr * b)
{
  if (a == NULL || b == NULL)
    return NULL;
  if (Ancetre (a, b))
    return b;
  if (Ancetre (b, a))
    return a;
  SPRepr *t = sp_repr_parent (a);
  while (t && !Ancetre (b, t))
    t = sp_repr_parent (t);
  return t;
}
Path *
Path_for_item (SPItem * item,bool doTransformation)
{
  SPCurve *curve;
  
  if (!item)
    return NULL;
  
  if (SP_IS_SHAPE (item))
  {
    curve = sp_shape_get_curve (SP_SHAPE (item));
  }
  else if (SP_IS_TEXT (item))
  {
    curve = sp_text_normalized_bpath (SP_TEXT (item));
  }
  else
  {
    curve = NULL;
  }
  
  if (!curve)
    return NULL;
  ArtBpath *bpath = curve->bpath;
  if (bpath == NULL)
    return NULL;
  
  if ( doTransformation ) {
    NRMatrix i2root;
    sp_item_i2root_affine (item, &i2root);
 		bpath = art_bpath_affine_transform (curve->bpath, NR_MATRIX_D_TO_DOUBLE (&i2root));
    sp_curve_unref (curve);
    curve=NULL;
  } else {
    bpath=curve->bpath;
  }

  Path *dest = new Path;
  dest->SetBackData (false);
  {
    int   i;
    bool  closed = false;
    float lastX  = 0.0;
    float lastY  = 0.0;
    
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
          NR::Point  tmp,tms,tme;
          tmp[0]=bpath[i].x3;
          tmp[1]=bpath[i].y3;
          tms[0]=3 * (bpath[i].x1 - lastX);
          tms[1]=3 * (bpath[i].y1 - lastY);
          tme[0]=3 * (bpath[i].x3 - bpath[i].x2);
          tme[1]=3 * (bpath[i].y3 - bpath[i].y2);
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
  
  if ( doTransformation ) {
    if ( bpath ) art_free (bpath);
  } else {
    sp_curve_unref(curve);
  }
  return dest;
}

gchar *
liv_svg_dump_path (Path * path)
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
                         (nData->large) ? 1 : 0,(nData->clockwise) ? 0 : 1, nData->p[NR::X],nData->p[NR::Y]);
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
