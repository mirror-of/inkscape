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

#include <time.h>
#include <string.h>

#include <libart_lgpl/art_misc.h>
#include "xml/repr.h"
#include "svg/svg.h"
#include "stringstream.h"
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
#include "display/canvas-bpath.h"
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
void sp_selected_path_do_offset (bool expand, double prefOffset);
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
sp_selected_path_cut ()
{
  sp_selected_path_boolop (bool_op_cut);
}
void
sp_selected_path_slice ()
{
  sp_selected_path_boolop (bool_op_slice);
}

// boolean operations
// take the source paths from the file, do the operation, delete the originals and add the results
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
    if (bop == bool_op_diff || bop == bool_op_symdiff || bop == bool_op_cut || bop == bool_op_slice )
    {
      sp_view_set_statusf_error(SP_VIEW(desktop), _("Select exactly 2 paths to perform difference or XOR."));
      return;
    }
  }
  
  // reverseOrderForOp marks whether the order of the list is the top->down order 
  // it's only used when there are 2 objects, and for operations who need to know the
  // topmost object (differences, cuts)
  bool reverseOrderForOp = false;
  // mettre les elements de la liste dans l'ordre pour ces operations
  if (bop == bool_op_diff || bop == bool_op_symdiff|| bop == bool_op_cut || bop == bool_op_slice)
  {
    // ugly check in the tree to find which element of the selection list is topmost (only 2 elements)
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
  
  // first check if all the input objects have shapes
  // otherwise bail out
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
  
  // extract the livarot Paths from the source objects
  // also get the winding rule specified in the style
  int nbOriginaux = g_slist_length (il);
  Path *originaux[nbOriginaux];
  FillRule  origWind[nbOriginaux];
  int curOrig;
  {
    curOrig = 0;
    for (l = il; l != NULL; l = l->next)
    {
      SPCSSAttr *css;
      const gchar *val;
      css = sp_repr_css_attr (SP_OBJECT_REPR (il->data), "style");
      val = sp_repr_css_property (css, "fill-rule", NULL);
      if (val && strcmp (val, "nonzero") == 0) {
        origWind[curOrig]= fill_nonZero;
      } else if (val && strcmp (val, "evenodd") == 0) {
        origWind[curOrig]= fill_oddEven;
      } else {
        origWind[curOrig]= fill_nonZero;
      }

      originaux[curOrig] = Path_for_item ((SPItem *) l->data,true);
      if (originaux[curOrig] == NULL || originaux[curOrig]->descr_nb <= 1)
      {
        for (int i = curOrig; i >= 0; i--) delete originaux[i];
        g_slist_free (il);
        return;
      }
      curOrig++;
    }
  }
  // reverse if needed
  // note that the selection list keeps its order
  if ( reverseOrderForOp ) {
    Path* swap=originaux[0];originaux[0]=originaux[1];originaux[1]=swap;
    FillRule swai=origWind[0]; origWind[0]=origWind[1]; origWind[1]=swai;
  }
  
  // and work
  // some temporary instances, first
  Shape *theShapeA = new Shape;
  Shape *theShapeB = new Shape;
  Shape *theShape = new Shape;
  Path *res = new Path;
  res->SetBackData (false);
  Path::cut_position  *toCut=NULL;
  int                  nbToCut=0;
  
  if ( bop == bool_op_inters || bop == bool_op_union || bop == bool_op_diff || bop == bool_op_symdiff ) {
    // true boolean op
    // get the polygons of each path, with the winding rule specified, and apply the operation iteratively
    originaux[0]->ConvertWithBackData (1.0);
     
    originaux[0]->Fill (theShape, 0);

    theShapeA->ConvertToShape (theShape, origWind[0]);
    
    curOrig = 1;
    for (l = il->next; l != NULL; l = l->next) {    
      originaux[curOrig]->ConvertWithBackData (1.0);
      
      originaux[curOrig]->Fill (theShape, curOrig);
      
      theShapeB->ConvertToShape (theShape, origWind[curOrig]);
      
      // les elements arrivent en ordre inverse dans la liste
      theShape->Booleen (theShapeB, theShapeA, bop);
      
      {
        Shape *swap = theShape;
        theShape = theShapeA;
        theShapeA = swap;
      }
      curOrig++;
    }
    {
      Shape *swap = theShape;
      theShape = theShapeA;
      theShapeA = swap;
    }
  } else if ( bop == bool_op_cut ) {
    // cuts= sort of a bastard boolean operation, thus not the axact same modus operandi
    // technically, the cut path is not necessarily a polygon (thus has no winding rule)
    // it is just uncrossed, and cleaned from duplicate edges and points
    // then it's fed to Booleen() which will uncross it against the other path
    // then comes the trick: each edge of the cut path is duplicated (one in each direction),
    // thus making a polygon. the weight of the edges of the cut are all 0, but 
    // the Booleen need to invert the ones inside the source polygon (for the subsequent 
    // ConvertToForme)
    
    // the cut path needs to have the highest pathID in the back data
    // that's how the Booleen() function knows it's an edge of the cut
  {
    Path* swap=originaux[0];originaux[0]=originaux[1];originaux[1]=swap;
    int   swai=origWind[0];origWind[0]=origWind[1];origWind[1]=(fill_typ)swai;
  }
    originaux[0]->ConvertWithBackData (1.0);
    
    originaux[0]->Fill (theShape, 0);
    
    theShapeA->ConvertToShape (theShape, origWind[0]);

    originaux[1]->ConvertWithBackData (1.0);
    
    originaux[1]->Fill (theShape, 1,false,false,false); //do not closeIfNeeded
    
    theShapeB->ConvertToShape (theShape, fill_justDont); // fill_justDont doesn't computes winding numbers
    
    // les elements arrivent en ordre inverse dans la liste
    theShape->Booleen (theShapeB, theShapeA, bool_op_cut,1);
    
  } else if ( bop == bool_op_slice ) {
    // slice is not really a boolean operation
    // you just put the 2 shapes in a single polygon, uncross it
    // the points where the degree is > 2 are intersections
    // just check it's an intersection on the path you want to cut, and keep it
    // the intersections you have found are then fed to ConvertPositionsToMoveTo() which will
    // make new subpath at each one of these positions
    // inversion pour l'opŽration
  {
    Path* swap=originaux[0];originaux[0]=originaux[1];originaux[1]=swap;
    int   swai=origWind[0];origWind[0]=origWind[1];origWind[1]=(fill_typ)swai;
  }
    originaux[0]->ConvertWithBackData (1.0);
    
    originaux[0]->Fill (theShapeA, 0,false,false,false); // don't closeIfNeeded
        
    originaux[1]->ConvertWithBackData (1.0);
    
    originaux[1]->Fill (theShapeA, 1,true,false,false);// don't closeIfNeeded and just dump in the shape, don't reset it
    
    theShape->ConvertToShape (theShapeA, fill_justDont);
  
    if ( theShape->HasBackData() ) {
      // should always be the case, but ya never know
      {
        for (int i=0;i<theShape->nbPt;i++) {
          if ( theShape->pts[i].dI+theShape->pts[i].dO > 2 ) { 
            // possibly an intersection
            // we need to check that at least one edge from the source path is incident to it
            // before we declare it's an intersection
            int   cb=theShape->pts[i].firstA;
            int   nbOrig=0;
            int   nbOther=0;
            int   piece=-1;
            float t=0.0;
            while ( cb >= 0 && cb < theShape->nbAr ) {
              if ( theShape->ebData[cb].pathID == 0 ) {
                // the source has an edge incident to the point, get its position on the path
                piece=theShape->ebData[cb].pieceID;
                if ( theShape->aretes[cb].st == i ) {
                  t=theShape->ebData[cb].tSt;
                } else {
                  t=theShape->ebData[cb].tEn;
                }
                nbOrig++;
              }
              if ( theShape->ebData[cb].pathID == 1 ) nbOther++; // the cut is incident to this point
              cb=theShape->NextAt(i,cb);
            }
            if ( nbOrig > 0 && nbOther > 0 ) {
              // point incident to both path and cut: an intersection
              // note that you only keep one position on the source; you could have degenerate
              // cases where the source crosses itself at this point, and you wouyld miss an intersection
              toCut=(Path::cut_position*)realloc(toCut,(nbToCut+1)*sizeof(Path::cut_position));
              toCut[nbToCut].piece=piece;
              toCut[nbToCut].t=t;
              nbToCut++;
            }
          }
        }
      }
      {
        // i think it's useless now
        int i=theShape->nbAr-1;
        for (;i>=0;i--) {
          if ( theShape->ebData[i].pathID == 1 ) {
            theShape->SubEdge(i);
          }
        }
      }
      
    }
  }
  
  int*    nesting=NULL;
  int*    conts=NULL;
  int     nbNest=0;
  // pour compenser le swap juste avant
  if ( bop == bool_op_slice ) {
//    theShape->ConvertToForme (res, nbOriginaux, originaux,true);
//    res->ConvertForcedToMoveTo();
    res->Copy(originaux[0]);
    res->ConvertPositionsToMoveTo(nbToCut,toCut); // cut where you found intersections
    free(toCut);
  } else if ( bop == bool_op_cut ) {
    // il faut appeler pour desallouer PointData (pas vital, mais bon)
    // the Booleen() function did not deallocated the point_data array in theShape, because this 
    // function needs it.
    // this function uses the point_data to get the winding number of each path (ie: is a hole or not)
    // for later reconstruction in objects, you also need to extract which path is parent of holes (nesting info)
    theShape->ConvertToFormeNested (res,nbOriginaux, originaux, 1,nbNest,nesting,conts);
  } else {
    theShape->ConvertToForme (res, nbOriginaux, originaux);
  }
  
  delete theShape;
  delete theShapeA;
  delete theShapeB;
  for (int i = 0; i < nbOriginaux; i++)  delete originaux[i];
  
  if (res->descr_nb <= 1)
  {
    // only one command, presumably a moveto: it isn't a path
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
  
  // now that we have the result, add it on the canvas
  if ( bop == bool_op_diff || bop == bool_op_symdiff || bop == bool_op_cut || bop == bool_op_slice ) {
    if (reverseOrderForOp)
    {
      style = g_strdup (sp_repr_attr ((SP_OBJECT (il->data))->repr, "style"));
    }
    else
    {
      style = g_strdup (sp_repr_attr ((SP_OBJECT (il->next->data))->repr, "style"));
    }
  } else {
    // on prend le style du premier
    style = g_strdup (sp_repr_attr ((SP_OBJECT (il->data))->repr, "style"));
  }
  
  for (l = il; l != NULL; l = l->next)
  {
    sp_repr_unparent (SP_OBJECT_REPR (l->data));
  }
  
  g_slist_free (il);
  
  sp_selection_empty (selection);

  if ( bop == bool_op_cut || bop == bool_op_slice ) {
    int    nbRP=0;
    Path** resPath;
    if ( bop == bool_op_slice ) {
      // there are moveto's at each intersection, but it's still one unique path
      // so break it down and add each subpath independently
      // we could call break_apart to do this, but while we have the description...
      resPath=res->SubPaths(nbRP,false);
    } else {
      // cut operation is a bit wicked: you need to keep holes
      // that's why you needed the nesting
      // ConvertToFormeNested() dumped all the subpath in a single Path "res", so we need
      // to get the path for each part of the polygon. that's why you need the nesting info:
      // to know in wich subpath to add a subpath
      resPath=res->SubPathsWithNesting(nbRP,true,nbNest,nesting,conts);
      
      // cleaning
      if ( conts ) free(conts);
      if ( nesting ) free(nesting);
    }
    for (int i=0;i<nbRP;i++) {
      d = liv_svg_dump_path (resPath[i]);
      
      repr = sp_repr_new ("path");
      sp_repr_set_attr (repr, "style", style);
      sp_repr_set_attr (repr, "d", d);
      g_free (d);
      item = (SPItem *) sp_document_add_repr (SP_DT_DOCUMENT (desktop), repr);
      sp_repr_unref (repr);
      
      sp_selection_add_item (selection, item);
      
      if (bop == bool_op_slice) {
        SPCSSAttr *css;        
        
        css = sp_repr_css_attr_new ();
        sp_repr_css_set_property (css, "fill", "none");
 
        sp_repr_css_change (SP_OBJECT_REPR (item), css, "style");
        
        sp_repr_css_attr_unref (css);
      }
      delete resPath[i];
    }
    sp_document_done (SP_DT_DOCUMENT (desktop));
    if ( resPath ) free(resPath);
    g_free (style);
  } else {
    d = liv_svg_dump_path (res);

    repr = sp_repr_new ("path");
    sp_repr_set_attr (repr, "style", style);
    g_free (style);
    sp_repr_set_attr (repr, "d", d);
    g_free (d);
    item = (SPItem *) sp_document_add_repr (SP_DT_DOCUMENT (desktop), repr);
    sp_document_done (SP_DT_DOCUMENT (desktop));
    sp_repr_unref (repr);
  
    sp_selection_set_item (selection, item);
  }
  delete res;
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
    // ca a merdÂŽ, ou bien le resultat est vide
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
	double prefOffset = prefs_get_double_attribute ("options.defaultoffsetwidth", "value", 1.0);

	sp_selected_path_do_offset (true, prefOffset);
}
void
sp_selected_path_inset ()
{
	double prefOffset = prefs_get_double_attribute ("options.defaultoffsetwidth", "value", 1.0);

	sp_selected_path_do_offset (false, prefOffset);
}

void
sp_selected_path_offset_screen (double pixels)
{
	sp_selected_path_do_offset (true,  pixels / SP_DESKTOP_ZOOM (SP_ACTIVE_DESKTOP));
}

void
sp_selected_path_inset_screen (double pixels)
{
	sp_selected_path_do_offset (false,  pixels / SP_DESKTOP_ZOOM (SP_ACTIVE_DESKTOP));
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
    
    if (o_width < 0.01)
      o_width = 0.01;               
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
sp_selected_path_do_offset (bool expand, double prefOffset)
{
	SPDesktop *desktop = SP_ACTIVE_DESKTOP;
	if (!SP_IS_DESKTOP (desktop))
		return;

	SPSelection *selection = SP_DT_SELECTION (desktop);

	if (sp_selection_is_empty (selection)) {
		sp_view_set_statusf_flash (SP_VIEW(desktop), _("Select some paths to inset/outset."));
		return;
	}

	bool did = false;

	for (GSList *items = g_slist_copy((GSList *) sp_selection_item_list(SP_DT_SELECTION(desktop)));
			 items != NULL;
			 items = items->next) {

		SPItem *item = (SPItem *) items->data;

		if (!SP_IS_SHAPE (item) && !SP_IS_TEXT (item))
			continue;

		SPCurve *curve = NULL;
		if (SP_IS_SHAPE (item)) {
			curve = sp_shape_get_curve (SP_SHAPE (item));
			if (curve == NULL)
				continue;
		}
		if (SP_IS_TEXT (item)) {
			curve = sp_text_normalized_bpath (SP_TEXT (item));
			if (curve == NULL)
				continue;
		}

		NRMatrix i2root;
		sp_item_i2root_affine (item, &i2root);
		gchar *style = g_strdup (sp_repr_attr (SP_OBJECT_REPR (item), "style"));

		float o_width, o_miter;
		JoinType o_join;
		ButtType o_butt;
     
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

			switch (jointype) {
			case SP_STROKE_LINEJOIN_MITER:
				o_join = join_pointy;
				break;
			case SP_STROKE_LINEJOIN_ROUND:
				o_join = join_round;
				break;
			default:
				o_join = join_straight;
				break;
			}

			switch (captype) {
			case SP_STROKE_LINECAP_SQUARE:
				o_butt = butt_square;
				break;
			case SP_STROKE_LINECAP_ROUND:
				o_butt = butt_round;
				break;
			default:
				o_butt = butt_straight;
				break;
			}

			o_width = prefOffset;

			if (o_width < 0.1)
				o_width = 0.1;
			o_miter = 4 * o_width;
		}

		Path *orig = Path_for_item (item, true);
		if (orig == NULL) {
			g_free (style);
			sp_curve_unref (curve);
			continue;
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

			// et maintenant: offset
			// methode inexacte
/*			Path *originaux[1];
			originaux[0] = orig;
			theRes->ConvertToForme (res, 1, originaux);

			if (expand) {
				res->OutsideOutline (orig, 0.5 * o_width, o_join, o_butt, o_miter);
			} else {
				res->OutsideOutline (orig, -0.5 * o_width, o_join, o_butt, o_miter);
			}
    
			orig->ConvertWithBackData (1.0);
			orig->Fill (theShape, 0);
			theRes->ConvertToShape (theShape, fill_positive);
			originaux[0] = orig;
			theRes->ConvertToForme (res, 1, originaux);
    
			if (o_width >= 0.5) {
				//     res->Coalesce (1.0);
				res->ConvertEvenLines (1.0);
				res->Simplify (1.0);
			} else {
				//      res->Coalesce (o_width);
				res->ConvertEvenLines (1.0*o_width);
				res->Simplify (1.0 * o_width);
			}    */
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

		did = true;

		sp_curve_unref (curve);
		// remember the position of the item
		gint pos = sp_repr_position (SP_OBJECT_REPR (item));
		// remember parent
		SPRepr *parent = SP_OBJECT_REPR (item)->parent;

		sp_selection_remove_item (selection, item);
		sp_repr_unparent (SP_OBJECT_REPR (item));

		if (res->descr_nb > 1) { // if there's 0 or 1 node left, drop this path altogether

			gchar tstr[80];

			tstr[79] = '\0';

			SPRepr *repr = sp_repr_new ("path");

			/*    if (sp_svg_transform_write (tstr, 80, &i2root))
						{
						sp_repr_set_attr (repr, "transform", tstr);
						}
						else
						{
						sp_repr_set_attr (repr, "transform", NULL);
						}*/

			sp_repr_set_attr (repr, "style", style);

			gchar *str = liv_svg_dump_path (res);
			sp_repr_set_attr (repr, "d", str);
			g_free (str);

			// add the new repr to the parent
			sp_repr_append_child (parent, repr);

			// move to the saved position 
			sp_repr_set_position_absolute (repr, pos > 0 ? pos : 0);

			sp_selection_add_repr (selection, repr);

			sp_repr_unref (repr);
		}
	}
  
 	if (did) {
		sp_document_done (SP_DT_DOCUMENT (desktop));
	} else {
		sp_view_set_statusf_flash (SP_VIEW(desktop), _("No paths to inset/outset in the selection."));
		return;
	} 
}



// globals for keeping track of accelerated simplify
static double prev_time = 0;
static gdouble simplify_multiply = 1;

void
sp_selected_path_simplify (void)
{
	gdouble simplify_threshold = prefs_get_double_attribute ("options.simplifythreshold","value", 0.003);
	bool simplify_justcoalesce = (bool) prefs_get_int_attribute ("options.simplifyjustcoalesce","value", 0);

	GTimeVal cu;
	g_get_current_time (&cu);
	double current = cu.tv_sec * 1000000 + cu.tv_usec; // current time

	if (prev_time != 0 && current - prev_time < 500000) { // last call of the command was within 0.5 sec
		simplify_multiply += 0.5; // add to the threshold 1/2 of its original value
		simplify_threshold *= simplify_multiply;
	} else {
		simplify_multiply = 1; // reset to the default
	}
	prev_time = current; // remember time

	//g_print ("%g\n", simplify_threshold);

	sp_selected_path_simplify_withparams (simplify_threshold, simplify_justcoalesce, 0.0, false);
}

void
sp_selected_path_simplify_withparams (float threshold, bool justCoalesce, float angleLimit, bool breakableAngles)
{
	SPDesktop *desktop = SP_ACTIVE_DESKTOP;
	if (!SP_IS_DESKTOP (desktop))
		return;

	SPSelection *selection = SP_DT_SELECTION (desktop);

	if (sp_selection_is_empty (selection)) {
		sp_view_set_statusf_flash (SP_VIEW(desktop), _("Select some paths to simplify."));
		return;
	}

	// remember selection size
	NR::Rect bbox = sp_selection_bbox (selection);
	gdouble size = L2(bbox.dimensions());

	bool did = false;

	for (GSList *items = g_slist_copy((GSList *) sp_selection_item_list(SP_DT_SELECTION(desktop)));
	      items != NULL;
	      items = items->next) {

		SPItem *item = (SPItem *) items->data;

		if (!SP_IS_SHAPE (item) && !SP_IS_TEXT (item))
			continue;

		SPCurve *curve = NULL;
		if (SP_IS_SHAPE (item)) {
			curve = sp_shape_get_curve (SP_SHAPE (item));
			if (curve == NULL)
				continue;
		}
		if (SP_IS_TEXT (item)) {
			curve = sp_text_normalized_bpath (SP_TEXT (item));
			if (curve == NULL)
				continue;
		}

		NRMatrix i2root;
		sp_item_i2root_affine (item, &i2root);
		gchar *style = g_strdup (sp_repr_attr (SP_OBJECT_REPR (item), "style"));

		Path *orig = Path_for_item (item, false);
		if (orig == NULL) {
			g_free (style);
			sp_curve_unref (curve);
			continue;
		}

		did = true;
		sp_curve_unref (curve);
		// remember the position of the item
		gint pos = sp_repr_position (SP_OBJECT_REPR (item));
		// remember parent
		SPRepr *parent = SP_OBJECT_REPR (item)->parent;

		sp_selection_remove_item (selection, item);
		sp_repr_unparent (SP_OBJECT_REPR (item));

		{
			if ( justCoalesce ) {
				orig->Coalesce (threshold * size);
			} else {
				orig->ConvertEvenLines (threshold * size);
				orig->Simplify (threshold * size);
			}
		}

		{
			gchar tstr[80];

			tstr[79] = '\0';

			SPRepr *repr = sp_repr_new ("path");

			if (sp_svg_transform_write (tstr, 80, &i2root))	{
				sp_repr_set_attr (repr, "transform", tstr);
			}
			else	{
				sp_repr_set_attr (repr, "transform", NULL);
			}

			sp_repr_set_attr (repr, "style", style);

			gchar *str = liv_svg_dump_path (orig);
			sp_repr_set_attr (repr, "d", str);
			g_free (str);

			// add the new repr to the parent
			sp_repr_append_child (parent, repr);

			// move to the saved position 
			sp_repr_set_position_absolute (repr, pos > 0 ? pos : 0);

			sp_selection_add_repr (selection, repr);

			sp_repr_unref (repr);
		}
	}

	if (did) {
		sp_document_done (SP_DT_DOCUMENT (desktop));
	} else {
		sp_view_set_statusf_flash (SP_VIEW(desktop), _("No paths to simplify in the selection."));
		return;
	}
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
  Inkscape::SVGOStringStream os;
  
  for (int i = 0; i < path->descr_nb; i++)
  {
    Path::path_descr theD = path->descr_cmd[i];
    int typ = theD.flags & descr_type_mask;
    if (typ == descr_moveto)
    {
      Path::path_descr_moveto*  nData=(Path::path_descr_moveto*)(path->descr_data+theD.dStart);
	  os << "M " << nData->p[0] << " " << nData->p[1] << " ";
    }
    else if (typ == descr_lineto)
    {
      Path::path_descr_lineto*  nData=(Path::path_descr_lineto*)(path->descr_data+theD.dStart);
	  os << "L " << nData->p[0] << " " << nData->p[1] << " ";
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
		os << "C " << lastX + nData->stD[0] / 3 << " "
			<< lastY + nData->stD[1] / 3 << " "
            << nData->p[0] - nData->enD[0] / 3 << " "
            << nData->p[1] - nData->enD[1] / 3 << " "
			<< nData->p[0] << " "
			<< nData->p[1] << " ";
	}
    else if (typ == descr_arcto)
    {
      Path::path_descr_arcto*  nData=(Path::path_descr_arcto*)(path->descr_data+theD.dStart);
      os << "A " << nData->rx << " "
			<< nData->ry << " "
			<< nData->angle << " "
			<< ((nData->large) ? "1" : "0") << " "
			<< ((nData->clockwise) ? "0" : "1") << " "
			<< nData->p[NR::X] << " "
			<< nData->p[NR::Y] << " ";
    }
    else if (typ == descr_close)
    {
	  os << "z ";
    }
    else
    {
    }
  }
  
  return g_strdup (os.str().c_str());

}
