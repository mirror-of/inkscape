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

// comment out the extern "C" if compiling with g++
extern "C" {
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
}

#include "livarot/Path.h"
#include "livarot/Shape.h"
#include "livarot/LivarotDefs.h"

Path*				Path_for_item(SPItem* item);
gchar*      liv_svg_dump_path (Path* path);
void        sp_selected_path_boolop (bool_op bop);
void        sp_selected_path_do_offset(bool expand);

void sp_selected_path_union(void)
{
	sp_selected_path_boolop(bool_op_union);
}
void sp_selected_path_intersect(void)
{
	sp_selected_path_boolop(bool_op_inters);
}
void sp_selected_path_diff(void)
{
	sp_selected_path_boolop(bool_op_diff);
}
void sp_selected_path_symdiff(void)
{
	sp_selected_path_boolop(bool_op_symdiff);
}

void
sp_selected_path_boolop (bool_op bop)
{
	SPDesktop * desktop;
	SPSelection * selection;
	GSList * il;
	GSList * l;
	SPRepr * repr;
	SPItem * item;
	gchar * d, * style;

	desktop = SP_ACTIVE_DESKTOP;
	if (!SP_IS_DESKTOP(desktop)) return;
	selection = SP_DT_SELECTION (desktop);

	il = (GSList *) sp_selection_item_list (selection);

	if (g_slist_length (il) < 2) return;
	
	if (g_slist_length (il) > 2) {
		if ( bop == bool_op_diff || bop == bool_op_symdiff ) {
			g_warning ("Difference and symetric difference need exactly 2 path");
			return;
		}
	}
	
	for (l = il; l != NULL; l = l->next) {
		item = (SPItem *) l->data;
		if (!SP_IS_SHAPE (item) && !SP_IS_TEXT (item)) return;
	}

	il = g_slist_copy (il);

	// choper les originaux pour faire l'operation demandŽe
	int     nbOriginaux=g_slist_length (il);
	Path*   originaux[nbOriginaux];
	int     curOrig;
	{
		curOrig=0;
		for (l = il; l != NULL; l = l->next) {
			originaux[curOrig]=Path_for_item((SPItem *) l->data);
			if ( originaux[curOrig] == NULL || originaux[curOrig]->descr_nb <= 1 ) {
				for (int i=curOrig;i>=0;i--) delete originaux[i];
				g_slist_free (il);
				return;
			}
			curOrig++;
		}
	}
	
	Shape*  theShapeA=new Shape;
	Shape*  theShapeB=new Shape;
	Shape*  theShape=new Shape;
	Path*   res=new Path;
	res->SetWeighted(false);
	res->SetBackData(false);
	
	{
		SPCSSAttr *css;
		const gchar *val;

		originaux[0]->ConvertWithBackData(1.0);

		originaux[0]->Fill(theShape,0);
		{
			NRMatrixF i2root;
			sp_item_i2root_affine (SP_ITEM (il->data), &i2root);
			for (int i=0;i<theShape->nbPt;i++) {
				float x=i2root.c[0]*theShape->pts[i].x+i2root.c[2]*theShape->pts[i].y+i2root.c[4];
				float y=i2root.c[1]*theShape->pts[i].x+i2root.c[3]*theShape->pts[i].y+i2root.c[5];
				theShape->pts[i].x=x;
				theShape->pts[i].y=y;
			}
		}
		css = sp_repr_css_attr (SP_OBJECT_REPR (il->data), "style");
		val = sp_repr_css_property (css, "fill-rule", NULL);
		if ( val && strcmp (val,"nonzero") == 0 ) {
			theShapeA->ConvertToShape(theShape,fill_nonZero);
		} else if ( val && strcmp (val,"evenodd") == 0 ) {
			theShapeA->ConvertToShape(theShape,fill_oddEven);
		} else {
			theShapeA->ConvertToShape(theShape,fill_positive);
		}
	}
	
	curOrig=1;
	for (l = il->next; l != NULL; l = l->next) {
		SPCSSAttr *css;
		const gchar *val;

		originaux[curOrig]->ConvertWithBackData(1.0);
		
		originaux[curOrig]->Fill(theShape,curOrig);
		
		{
			NRMatrixF i2root;
			sp_item_i2root_affine (SP_ITEM (l->data), &i2root);
			for (int i=0;i<theShape->nbPt;i++) {
				float x=i2root.c[0]*theShape->pts[i].x+i2root.c[2]*theShape->pts[i].y+i2root.c[4];
				float y=i2root.c[1]*theShape->pts[i].x+i2root.c[3]*theShape->pts[i].y+i2root.c[5];
				theShape->pts[i].x=x;
				theShape->pts[i].y=y;
			}
		}
		css = sp_repr_css_attr (SP_OBJECT_REPR (l->data), "style");
		val = sp_repr_css_property (css, "fill-rule", NULL);
		if ( val && strcmp (val,"nonzero") == 0 ) {
			theShapeB->ConvertToShape(theShape,fill_nonZero);
		} else if ( val && strcmp (val,"evenodd") == 0 ) {
			theShapeB->ConvertToShape(theShape,fill_oddEven);
		} else {
			theShapeB->ConvertToShape(theShape,fill_positive);
		}
		
		theShape->Booleen(theShapeA,theShapeB,bop);
		
		{Shape* swap=theShape;theShape=theShapeA;theShapeA=swap;}
		curOrig++;
	}
	// pour compenser le swap juste avant
	{Shape* swap=theShape;theShape=theShapeA;theShapeA=swap;}
	
	theShape->ConvertToForme(res,nbOriginaux,originaux);

	delete theShape;
	delete theShapeA;
	delete theShapeB;
	for (int i=0;i<nbOriginaux;i++) delete originaux[i];
		
	d = liv_svg_dump_path(res);
	delete res;
	
	style = g_strdup (sp_repr_attr ((SP_OBJECT (il->data))->repr, "style"));

	for (l = il; l != NULL; l = l->next) {
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

	sp_selection_set_item (selection, item);
}


Path* Path_for_item(SPItem* item)
{
	SPCurve *curve;
	
	if (!item)
		return NULL;
		
	if (SP_IS_SHAPE (item)) {
		curve = sp_shape_get_curve (SP_SHAPE (item));
	} else if (SP_IS_TEXT (item)) {
		curve = sp_text_normalized_bpath (SP_TEXT (item));
	} else {
		curve = NULL;
	}
	
	if (!curve) return NULL;
	ArtBpath * bpath=curve->bpath;
	if ( bpath == NULL ) return NULL;
	
	Path* dest=new Path;
	dest->SetWeighted(false);
	dest->SetBackData(false);
	{		
		int   i;
		bool   closed = false;
		float lastX,lastY;
			
		for (i = 0; bpath[i].code != ART_END; i++){
			switch (bpath [i].code){
				case ART_LINETO:
					lastX=bpath[i].x3;
					lastY=bpath[i].y3;
					dest->LineTo(lastX,lastY);
					break;
					
				case ART_CURVETO:
					dest->CubicTo(bpath[i].x3,bpath[i].y3,3*(bpath[i].x1-lastX),3*(bpath[i].y1-lastY)
									 ,3*(bpath[i].x3-bpath[i].x2),3*(bpath[i].y3-bpath[i].y2));
					lastX=bpath[i].x3;
					lastY=bpath[i].y3;
						break;
						
					case ART_MOVETO_OPEN:
					case ART_MOVETO:
						if ( closed ) dest->Close();
						closed=(bpath[i].code == ART_MOVETO);
						lastX=bpath[i].x3;
						lastY=bpath[i].y3;
						dest->MoveTo(lastX,lastY);
						break;
					default:
						break;
			}
		}
		if ( closed ) dest->Close();
	}
	
	sp_curve_unref (curve);
	
	return dest;
}
gchar*  liv_svg_dump_path (Path* path)
{
	GString *result;
	result = g_string_sized_new (40);

	for (int i=0;i<path->descr_nb;i++) {
Path::path_descr  theD=path->descr_data[i];
		int typ=theD.flags&descr_type_mask;
		if ( typ == descr_moveto ) {
			g_string_sprintfa (result, "M %lf %lf ",theD.d.m.x,theD.d.m.y);
		} else if ( typ == descr_lineto ) {
			g_string_sprintfa (result, "L %lf %lf ",theD.d.l.x,theD.d.l.y);
		} else if ( typ == descr_cubicto ) {
			float  lastX,lastY;
			path->PrevPoint(i-1,lastX,lastY);
			g_string_sprintfa (result, "C %lf %lf %lf %lf %lf %lf ",lastX+theD.d.c.stDx/3,lastY+theD.d.c.stDy/3,
											theD.d.c.x-theD.d.c.enDx/3,theD.d.c.y-theD.d.c.enDy/3,
											theD.d.c.x,theD.d.c.y);
		} else if ( typ == descr_arcto ) {
//			g_string_sprintfa (result, "L %lf %lf ",theD.d.a.x,theD.d.a.y);
			g_string_sprintfa (result, "A %g %g %g %i %i %g %g ",theD.d.a.rx,theD.d.a.ry,theD.d.a.angle,
											(theD.d.a.large)?1:0,(theD.d.a.clockwise)?0:1,
											theD.d.a.x,theD.d.a.y);
		} else if ( typ == descr_close ) {
			g_string_sprintfa (result, "z ");
		} else {
		}
	}
	
	char *res;
	res = result->str;
	g_string_free (result, FALSE);
	
	return res;
}

void sp_selected_path_outline(void)
{
	SPSelection * selection;
	SPRepr * repr;
	SPItem * item;
	SPPath * path;
	SPCurve * curve;
	gchar * style, * str;
	SPDesktop    *desktop;
	float        o_width,o_miter;
	JoinType		 o_join;
	ButtType     o_butt;
	NRMatrixF    i2root;
	
	desktop = SP_ACTIVE_DESKTOP;
	if (!SP_IS_DESKTOP(desktop)) return;
	
	selection = SP_DT_SELECTION (desktop);
	
	item = sp_selection_item (selection);
	
	if (item == NULL) return;
	if ( !SP_IS_PATH (item) && !SP_IS_TEXT (item) ) return;
	if ( SP_IS_PATH(item) ) {
		path = SP_PATH (item);
		curve = sp_shape_get_curve (SP_SHAPE (path));
		if (curve == NULL) return;
	}
	if ( SP_IS_TEXT(item) ) {
		curve = sp_text_normalized_bpath (SP_TEXT (item));
		if (curve == NULL) return;
	}
	
Shape::round_power=3;
	
	sp_item_i2root_affine (item, &i2root);
	style = g_strdup (sp_repr_attr (SP_OBJECT (item)->repr, "style"));
	
	{
		SPStyle     *i_style=SP_OBJECT(item)->style;
		int         jointype,captype;
		
		o_width=1.0;
		o_miter=4*o_width;
		o_join=join_straight;
		o_butt=butt_straight;

		jointype=i_style->stroke_linejoin.value;
		captype=i_style->stroke_linecap.value;
		o_width=i_style->stroke_width.computed;
		if ( jointype == SP_STROKE_LINEJOIN_MITER ) {
			o_join=join_pointy;
		} else if ( jointype == SP_STROKE_LINEJOIN_ROUND ) {
			o_join=join_round;
		} else {
			o_join=join_straight;
		}
		if ( captype == SP_STROKE_LINECAP_SQUARE ) {
			o_butt=butt_square;
		} else if ( captype == SP_STROKE_LINECAP_ROUND ) {
			o_butt=butt_round;
		} else {
			o_butt=butt_straight;
		}
		if ( o_width < 0.1 ) o_width=0.1;
		o_miter=4*o_width;
	}
	
	Path* orig=Path_for_item(item);
	if ( orig == NULL ) {
		g_free (style);
		sp_curve_unref (curve);
		return;
	}
		
	Path*  res=new Path;
	res->SetWeighted(false);
	res->SetBackData(false);
	
	sp_curve_unref (curve);
	
	{

		orig->Outline(res,0.5*o_width,o_join,o_butt,o_miter);
	
		res->ConvertEvenLines(0.25*o_width);
		res->Simplify(0.05*o_width);

		Shape*  theShape=new Shape;
		res->ConvertWithBackData(1.0);
		res->Fill(theShape,0);
		Shape*  theRes=new Shape;
		theRes->ConvertToShape(theShape,fill_positive);
	
		Path*  originaux[1];
		originaux[0]=res;
		orig->Reset();
		theRes->ConvertToForme(orig,1,originaux);
		
		delete theShape;
		delete theRes;
		
	}
	
	if ( orig->descr_nb <= 1 ) {
		// ca a merdŽ, ou bien le resultat est vide
		delete res;
		delete orig;
		g_free (style);

		return;
	}
	
	sp_repr_unparent (SP_OBJECT_REPR (item));

	{
		gchar tstr[80];
		
		tstr[79] = '\0';
		
		repr = sp_repr_new ("path");
		if (sp_svg_transform_write (tstr, 80, &i2root)) {
			sp_repr_set_attr (repr, "transform", tstr);
		} else {
			sp_repr_set_attr (repr, "transform", NULL);
		}
		
		sp_repr_set_attr (repr, "style", style);

		str = liv_svg_dump_path (orig);
		sp_repr_set_attr (repr, "d", str);
		g_free (str);
		item = (SPItem *) sp_document_add_repr (SP_DT_DOCUMENT (desktop), repr);
		sp_repr_unref (repr);
		sp_selection_add_item (selection, item);

		{
			SPCSSAttr *ocss;
			SPCSSAttr *css;
			const gchar *val;

			ocss = sp_repr_css_attr (SP_OBJECT_REPR (item), "style");
			val = sp_repr_css_property (ocss, "stroke", NULL);

			css = sp_repr_css_attr_new ();

			sp_repr_css_set_property (css, "stroke", "none");
			sp_repr_css_set_property (css, "fill", val);
			sp_repr_css_change (SP_OBJECT_REPR (item), css, "style");
			
			sp_repr_css_attr_unref (css);
		}
	}
	
	sp_document_done (SP_DT_DOCUMENT (desktop));
	
	delete res;
	delete orig;
	
	g_free (style);
}
void sp_selected_path_offset(void)
{
	sp_selected_path_do_offset(true);
}
void sp_selected_path_inset(void)
{
	sp_selected_path_do_offset(false);
}
void        sp_selected_path_do_offset(bool expand)
{
	SPSelection * selection;
	SPRepr * repr;
	SPItem * item;
	SPPath * path;
	SPCurve * curve;
	gchar * style, * str;
	SPDesktop    *desktop;
	float        o_width,o_miter;
	JoinType		 o_join;
	ButtType     o_butt;
	NRMatrixF    i2root;
	
	desktop = SP_ACTIVE_DESKTOP;
	if (!SP_IS_DESKTOP(desktop)) return;
	
	selection = SP_DT_SELECTION (desktop);
	
	item = sp_selection_item (selection);
	
	if (item == NULL) return;
	if (!SP_IS_PATH (item)) return;
	path = SP_PATH (item);
	curve = sp_shape_get_curve (SP_SHAPE (path));
	if (curve == NULL) return;
	
	sp_item_i2root_affine (item, &i2root);
	style = g_strdup (sp_repr_attr (SP_OBJECT (item)->repr, "style"));
	
	{
		SPStyle     *i_style=SP_OBJECT(item)->style;
		int         jointype,captype;
		
		o_width=1.0;
		o_miter=4*o_width;
		o_join=join_straight;
		o_butt=butt_straight;
		
		jointype=i_style->stroke_linejoin.value;
		captype=i_style->stroke_linecap.value;
		o_width=i_style->stroke_width.computed;
		if ( jointype == SP_STROKE_LINEJOIN_MITER ) {
			o_join=join_pointy;
		} else if ( jointype == SP_STROKE_LINEJOIN_ROUND ) {
			o_join=join_round;
		} else {
			o_join=join_straight;
		}
		if ( captype == SP_STROKE_LINECAP_SQUARE ) {
			o_butt=butt_square;
		} else if ( captype == SP_STROKE_LINECAP_ROUND ) {
			o_butt=butt_round;
		} else {
			o_butt=butt_straight;
		}
		if ( o_width < 0.1 ) o_width=0.1;
		o_miter=4*o_width;
	}
	
	Path* orig=Path_for_item(item);
	if ( orig == NULL ) {
		g_free (style);
		sp_curve_unref (curve);
		return;
	}
	
	Path*  res=new Path;
	res->SetWeighted(false);
	res->SetBackData(false);
	
	{
		SPCSSAttr *css;
		const gchar *val;

		Shape*  theShape=new Shape;
		Shape*  theRes=new Shape;
		orig->ConvertWithBackData(1.0);
		orig->Fill(theShape,0);
		
		css = sp_repr_css_attr (SP_OBJECT_REPR (path), "style");
		val = sp_repr_css_property (css, "fill-rule", NULL);
		if ( val && strcmp (val,"nonzero") == 0 ) {
			theRes->ConvertToShape(theShape,fill_nonZero);
		} else if ( val && strcmp (val,"evenodd") == 0 ) {
			theRes->ConvertToShape(theShape,fill_oddEven);
		} else {
			theRes->ConvertToShape(theShape,fill_positive);
		}
		Path*  originaux[1];
		originaux[0]=orig;
		theRes->ConvertToForme(res,1,originaux);
		
		// et maintenant: offset
		if ( expand ) {
			res->OutsideOutline(orig,0.5*o_width,o_join,o_butt,o_miter);
		} else {
			res->OutsideOutline(orig,-0.5*o_width,o_join,o_butt,o_miter);
		}
		orig->ConvertWithBackData(1.0);
		orig->Fill(theShape,0);
		theRes->ConvertToShape(theShape,fill_positive);
		
		originaux[0]=orig;
		theRes->ConvertToForme(res,1,originaux);
		
		delete theShape;
		delete theRes;

		res->ConvertEvenLines(1.0);
		res->Simplify(0.5);
	}
	
	sp_curve_unref (curve);
	sp_repr_unparent (SP_OBJECT_REPR (item));
	

	{
		SPCSSAttr *css;
		const gchar *val;
		gchar tstr[80];
		
		tstr[79] = '\0';
		
		repr = sp_repr_new ("path");
		if (sp_svg_transform_write (tstr, 80, &i2root)) {
			sp_repr_set_attr (repr, "transform", tstr);
		} else {
			sp_repr_set_attr (repr, "transform", NULL);
		}
		
		sp_repr_set_attr (repr, "style", style);
		str = liv_svg_dump_path (res);
		sp_repr_set_attr (repr, "d", str);
		g_free (str);
		item = (SPItem *) sp_document_add_repr (SP_DT_DOCUMENT (desktop), repr);
		sp_repr_unref (repr);
		sp_selection_add_item (selection, item);

		{
			SPCSSAttr *ocss;
			SPCSSAttr *css;
			const gchar *val;
			
			ocss = sp_repr_css_attr (SP_OBJECT_REPR (item), "style");
			val = sp_repr_css_property (ocss, "stroke", NULL);
			
			css = sp_repr_css_attr_new ();
			
			sp_repr_css_set_property (css, "stroke", "none");
			sp_repr_css_set_property (css, "fill", val);
			sp_repr_css_change (SP_OBJECT_REPR (item), css, "style");
			
			sp_repr_css_attr_unref (css);
		}
	}
	
	sp_document_done (SP_DT_DOCUMENT (desktop));
	
	delete res;
	delete orig;
	
	g_free (style);
}

void sp_selected_path_simplify(void)
{
		SPSelection * selection;
	SPRepr * repr;
	SPItem * item;
	SPPath * path;
	SPCurve * curve;
	gchar * style, * str;
	SPDesktop    *desktop;
	NRMatrixF    i2root;
	
	desktop = SP_ACTIVE_DESKTOP;
	if (!SP_IS_DESKTOP(desktop)) return;
	
	selection = SP_DT_SELECTION (desktop);
	
	item = sp_selection_item (selection);
	
	if (item == NULL) return;
	if (!SP_IS_PATH (item)) return;
	path = SP_PATH (item);
	curve = sp_shape_get_curve (SP_SHAPE (path));
	if (curve == NULL) return;
		
	sp_item_i2root_affine (item, &i2root);
	style = g_strdup (sp_repr_attr (SP_OBJECT (item)->repr, "style"));
		
	Path* orig=Path_for_item(item);
	if ( orig == NULL ) {
		g_free (style);
		sp_curve_unref (curve);
		return;
	}
			
	sp_curve_unref (curve);
	sp_repr_unparent (SP_OBJECT_REPR (item));
	
	{
		orig->ConvertEvenLines(1.0);
		orig->Simplify(0.5);
	}
	
	{
		gchar tstr[80];
		
		tstr[79] = '\0';
		
		repr = sp_repr_new ("path");
		if (sp_svg_transform_write (tstr, 80, &i2root)) {
			sp_repr_set_attr (repr, "transform", tstr);
		} else {
			sp_repr_set_attr (repr, "transform", NULL);
		}
		
		sp_repr_set_attr (repr, "style", style);
		
		str = liv_svg_dump_path (orig);
		sp_repr_set_attr (repr, "d", str);
		g_free (str);
		item = (SPItem *) sp_document_add_repr (SP_DT_DOCUMENT (desktop), repr);
		sp_repr_unref (repr);
		sp_selection_add_item (selection, item);
		
	}
	
	sp_document_done (SP_DT_DOCUMENT (desktop));
	
	delete orig;
	
	g_free (style);
	
}

