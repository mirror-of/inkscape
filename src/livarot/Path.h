/*
 *  Path.h
 *  nlivarot
 *
 *  Created by fred on Tue Jun 17 2003.
 *
 */

#ifndef my_path
#define my_path

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
//#include <iostream.h>

#include "LivarotDefs.h"
//#include "MyMath.h"

#include "../libnr/nr-types.h"

enum
{
  descr_moveto = 0,
  descr_lineto = 1,
  descr_cubicto = 2,
  descr_bezierto = 3,
  descr_arcto = 4,
  descr_close = 5,
  descr_interm_bezier = 6,
  descr_forced = 7,

  descr_type_mask = 15,

  descr_weighted = 16
};

enum
{
  polyline_lineto = 0,
  polyline_moveto = 1,
  polyline_forced = 2
};

class Shape;

typedef struct dashTo_info
{
  float     nDashAbs;
  NR::Point prevP;
  NR::Point curP;
  NR::Point prevD;
  float     prevW;
  float     curW;
} dashTo_info;

// path creation: 2 phases: first the path is given as a succession of commands (MoveTo, LineTo, CurveTo...); then it
// is converted in a polyline
// a polylone can be stroked or filled to make a polygon
class Path
{
  friend class Shape;
public:
  // command list structures

  // lineto: a point, maybe with a weight
  typedef struct path_descr_moveto
  {
    NR::Point  p;
    int pathLength;		// number of description for this subpath
  }
  path_descr_moveto;

  typedef struct path_descr_moveto_w:public path_descr_moveto
  {
    float w;
  }
  path_descr_moveto_w;

  // lineto: a point, maybe with a weight
  // MoveTos fit in this category
  typedef struct path_descr_lineto
  {
    NR::Point  p;
  }
  path_descr_lineto;
  typedef struct path_descr_lineto_w:public path_descr_lineto
  {
    float w;
  }
  path_descr_lineto_w;

  // quadratic bezier curves: a set of control points, and an endpoint
  typedef struct path_descr_bezierto
  {
    int nb;
    NR::Point    p;			// the endpoint's coordinates
  }
  path_descr_bezierto;
  typedef struct path_descr_bezierto_w:public path_descr_bezierto
  {
    float w;
  }
  path_descr_bezierto_w;
  typedef struct path_descr_intermbezierto
  {
    NR::Point    p;			// control point coordinates
  }
  path_descr_intermbezierto;
  typedef struct path_descr_intermbezierto_w:public path_descr_intermbezierto
  {
    float w;
  }
  path_descr_intermbezierto_w;

  // cubic spline curve: 2 tangents and one endpoint
  typedef struct path_descr_cubicto
  {
    NR::Point    p;
    NR::Point    stD;
    NR::Point    enD;
  }
  path_descr_cubicto;
  typedef struct path_descr_cubicto_w:public path_descr_cubicto
  {
    float w;			// weight for the endpoint (if any)
  }
  path_descr_cubicto_w;

  // arc: endpoint, 2 radii and one angle, plus 2 booleans to choose the arc (svg style)
  typedef struct path_descr_arcto
  {
    NR::Point    p;
    float        rx,ry;
    float        angle;
    bool         large, clockwise;
  }
  path_descr_arcto;
  typedef struct path_descr_arcto_w:public path_descr_arcto
  {
    float w;			// weight for the endpoint (if any)
  }
  path_descr_arcto_w;

  typedef struct path_descr
  {
    int flags;
    int associated;		// le no du moveto/lineto dans la polyligne. ou alors no de la piece dans l'original
    float tSt, tEn;
    struct
    {
      path_descr_moveto_w m;
      path_descr_lineto_w l;
      path_descr_cubicto_w c;
      path_descr_arcto_w a;
      path_descr_bezierto_w b;
      path_descr_intermbezierto_w i;
    }
    d;
  }
  path_descr;

  enum
  {
    descr_ready = 0,
    descr_adding_bezier = 1,
    descr_doing_subpath = 2,
    descr_delayed_bezier = 4,
    descr_dirty = 16
  };
public:
  int descr_flags;
  int descr_max, descr_nb;
  path_descr *descr_data;
  int pending_bezier;
  int pending_moveto;

  // polyline storage: a serie of coordinates (and maybe weights)
  typedef struct path_lineto
  {
    int isMoveTo;
    NR::Point  p;
  }
  path_lineto;
  typedef struct path_lineto_w:public path_lineto
  {
    float w;
  }
  path_lineto_w;
  typedef struct path_lineto_b:public path_lineto
  {
    int piece;
    float t;
  }
  path_lineto_b;
  typedef struct path_lineto_wb:public path_lineto_w
  {
    int piece;
    float t;
  }
  path_lineto_wb;

public:
  bool weighted;
  bool back;
  int nbPt, maxPt, sizePt;
  char *pts;

    Path (void);
   ~Path (void);

  // creation of the path description
  void Reset (void);		// reset to the empty description
  void Copy (Path * who);

  // dumps the path description on the standard output
  void Affiche (void);

  // the commands...
  int ForcePoint (void);
  int Close (void);
  int MoveTo (NR::Point &ip);
  int MoveTo (NR::Point &ip, float iw);
  int LineTo (NR::Point &ip);
  int LineTo (NR::Point &ip, float iw);
  int CubicTo (NR::Point &ip, NR::Point &iStD, NR::Point &iEnD);
  int CubicTo (NR::Point &ip, NR::Point &iStD, NR::Point &iEnD, float iw);
  int ArcTo (NR::Point &ip, float iRx, float iRy, float angle,
	     bool iLargeArc, bool iClockwise);
  int ArcTo (NR::Point &ip, float iRx, float iRy, float angle,
	     bool iLargeArc, bool iClockwise, float iw);
  int IntermBezierTo (NR::Point &ip);	// add a quadratic bezier spline control point
  int IntermBezierTo (NR::Point &ip, float iw);
  int BezierTo (NR::Point &ip);	// quadratic bezier spline to this point (control points can be added after this)
  int BezierTo (NR::Point &ip, float iw);
  int TempBezierTo (void);	// start a quadratic bezier spline (control points can be added after this)
  int TempBezierToW (void);
  int EndBezierTo (void);
  int EndBezierTo (NR::Point &ip);	// ends a quadratic bezier spline (for curves started with TempBezierTo)
  int EndBezierTo (NR::Point &ip, float iw);

  // transforms a description in a polyline (for stroking and filling)
  // treshhold is the max length^2 (sort of)
  void Convert (float treshhold);
  void ConvertEvenLines (float treshhold);	// decomposes line segments too, for later recomposition
  // same function for use when you want to later recompose the curves from the polyline
  void ConvertWithBackData (float treshhold);
  // same function for use when you want to later recompose the curves from the polyline
  void ConvertForOffset (float treshhold, Path * orig, float off_dec);

  // creation of the polyline (you can tinker with these function if you want)
  void SetWeighted (bool nVal);	// is weighted?
  void SetBackData (bool nVal);	// has back data?
  void ResetPoints (int expected = 0);	// resets to the empty polyline
  int AddPoint (NR::Point &iPt, bool mvto = false);	// add point
  int AddPoint (NR::Point &iPt, float iw, bool mvto = false);
  int AddPoint (NR::Point &iPt, int ip, float it, bool mvto = false);
  int AddPoint (NR::Point &iPt, float iw, int ip, float it, bool mvto =
		false);
  int AddForcedPoint (NR::Point &iPt);	// add point
  int AddForcedPoint (NR::Point &iPt, float iw);
  int AddForcedPoint (NR::Point &iPt, int ip, float it);
  int AddForcedPoint (NR::Point &iPt, float iw, int ip, float it);

  // transform in a polygon (in a graph, in fact; a subsequent call to ConvertToShape is needed)
  //  - fills the polyline; justAdd=true doesn't reset the Shape dest, but simply adds the polyline into it
  // closeIfNeeded=false prevent the function from closing the path (resulting in a non-eulerian graph
  // pathID is a identification number for the path, and is used for recomposing curves from polylines
  // give each different Path a different ID, and feed the appropriate orig[] to the ConvertToForme() function
  void Fill (Shape * dest, int pathID = -1, bool justAdd =
	     false, bool closeIfNeeded = true, bool invert = false);
  // - stroke the path; usual parameters: type of cap=butt, type of join=join and miter (see LivarotDefs.h)
  // doClose treat the path as closed (ie a loop)
  void Stroke (Shape * dest, bool doClose, float width, JoinType join,
	       ButtType butt, float miter, bool justAdd = false);
  // stroke with dashes
  void Stroke (Shape * dest, bool doClose, float width, JoinType join,
	       ButtType butt, float miter, int nbDash, one_dash * dashs,
	       bool justAdd = false);
  // build a Path that is the outline of the Path instance's description (the result is stored in dest)
  // it doesn't compute the exact offset (it's way too complicated, but an approximation made of cubic bezier patches
  //  and segments. the algorithm was found in a plugin for Impress (by Chris Cox), but i can't find it back...
  void Outline (Path * dest, float width, JoinType join, ButtType butt,
		float miter);
  // half outline with edges having the same direction as the original
  void OutsideOutline (Path * dest, float width, JoinType join, ButtType butt,
		       float miter);
  // half outline with edges having the opposite direction as the original
  void InsideOutline (Path * dest, float width, JoinType join, ButtType butt,
		      float miter);

  // polyline to cubic bezier
  void Simplify (float treshhold);
  // description simplification
  void Coalesce (float tresh);

  // utilities
  void PointAt (int piece, float at, NR::Point & pos);
  void PointAndTangentAt (int piece, float at, NR::Point & pos, NR::Point & tgt);

  void PrevPoint (int i, NR::Point &oPt);
private:
  void Alloue (int addSize);
  void CancelBezier (void);
  void CloseSubpath (int add);
  // winding of the path (treated as a loop)
  int Winding (void);

  // fonctions utilisees par la conversion
  void DoArc (NR::Point &iS, NR::Point &iE, float rx, float ry,
	      float angle, bool large, bool wise, float tresh);
  void DoArc (NR::Point &iS, float sw, NR::Point &iE, float ew,
	      float rx, float ry, float angle, bool large, bool wise,
	      float tresh);
  void RecCubicTo (NR::Point &iS, NR::Point &iSd, NR::Point &iE, NR::Point &iEd, float tresh, int lev,
		   float maxL = -1.0);
  void RecCubicTo (NR::Point &iS, float sw, NR::Point &iSd,
		   NR::Point &iE, float ew, NR::Point &iEd,
		   float tresh, int lev, float maxL = -1.0);
  void RecBezierTo (NR::Point &iPt, NR::Point &iS, NR::Point &iE, float treshhold, int lev, float maxL = -1.0);
  void RecBezierTo (NR::Point &iPt, float pw, NR::Point &iS,
		    float sw, NR::Point &iE, float ew, float treshhold,
		    int lev, float maxL = -1.0);

  void DoArc (NR::Point &iS, NR::Point &iE, float rx, float ry,
	      float angle, bool large, bool wise, float tresh, int piece);
  void DoArc (NR::Point &iS, float sw, NR::Point &iE, float ew,
	      float rx, float ry, float angle, bool large, bool wise,
	      float tresh, int piece);
  void RecCubicTo (NR::Point &iS, NR::Point &iSd, NR::Point &iE, NR::Point &iEd, float tresh, int lev,
		   float st, float et, int piece);
  void RecCubicTo (NR::Point &iS, float sw, NR::Point &iSd,
		   NR::Point &iE, float ew, NR::Point &iEd,
		   float tresh, int lev, float st, float et, int piece);
  void RecBezierTo (NR::Point &iPt, NR::Point &iS, NR::Point &iE, float treshhold, int lev, float st, float et,
		    int piece);
  void RecBezierTo (NR::Point &iPt, float pw, NR::Point &iS,
		    float sw, NR::Point &iE, float ew, float treshhold,
		    int lev, float st, float et, int piece);

  typedef struct offset_orig
  {
    Path *orig;
    int piece;
    float tSt, tEn;
    float off_dec;
  }
  offset_orig;
  void DoArc (NR::Point &iS, NR::Point &iE, float rx, float ry,
	      float angle, bool large, bool wise, float tresh, int piece,
	      offset_orig & orig);
  void RecCubicTo (NR::Point &iS, NR::Point &iSd, NR::Point &iE, NR::Point &iEd, float tresh, int lev,
		   float st, float et, int piece, offset_orig & orig);
  void RecBezierTo (NR::Point &iPt, NR::Point &iS, NR::Point &iE, float treshhold, int lev, float st, float et,
		    int piece, offset_orig & orig);

  static void ArcAngles (NR::Point &iS, NR::Point &iE, float rx,
                         float ry, float angle, bool large, bool wise,
                         float &sang, float &eang);
  static void ArcAnglesAndCenter (NR::Point &iS, NR::Point &iE, float rx,
                         float ry, float angle, bool large, bool wise,
                                  float &sang, float &eang,NR::Point &dr);
  static void QuadraticPoint (float t, NR::Point &oPt,  NR::Point &iS,  NR::Point &iM,  NR::Point &iE);
  static void CubicTangent (float t, NR::Point &oPt, NR::Point &iS,
			    NR::Point &iSd, NR::Point &iE,
			    NR::Point &iEd);

  typedef struct outline_callback_data
  {
    Path *orig;
    int piece;
    float tSt, tEn;
    Path *dest;
    float x1, y1, x2, y2;
    union
    {
      struct
      {
	float dx1, dy1, dx2, dy2;
      }
      c;
      struct
      {
	float mx, my;
      }
      b;
      struct
      {
	float rx, ry, angle;
	bool clock, large;
	float stA, enA;
      }
      a;
    }
    d;
  }
  outline_callback_data;

  typedef void (outlineCallback) (outline_callback_data * data, float tol,
				  float width);
  typedef struct outline_callbacks
  {
    outlineCallback *cubicto;
    outlineCallback *bezierto;
    outlineCallback *arcto;
  }
  outline_callbacks;

  void SubContractOutline (Path * dest, outline_callbacks & calls,
			   float tolerance, float width, JoinType join,
			   ButtType butt, float miter, bool closeIfNeeded,
			   bool skipMoveto, NR::Point & lastP, NR::Point & lastT);
  void DoOutsideOutline (Path * dest, float width, JoinType join,
			 ButtType butt, float miter, int &stNo, int &enNo);
  void DoInsideOutline (Path * dest, float width, JoinType join,
			ButtType butt, float miter, int &stNo, int &enNo);
  void DoStroke (Shape * dest, bool doClose, float width, JoinType join,
		 ButtType butt, float miter, bool justAdd = false);
  void DoStroke (Shape * dest, bool doClose, float width, JoinType join,
		 ButtType butt, float miter, int nbDash, one_dash * dashs,
		 bool justAdd = false);

  static void TangentOnSegAt (float at, NR::Point &iS,
			      path_descr_lineto & fin, NR::Point & pos, NR::Point & tgt,
			      float &len);
  static void TangentOnArcAt (float at, NR::Point &iS,
			      path_descr_arcto & fin, NR::Point & pos, NR::Point & tgt,
			      float &len, float &rad);
  static void TangentOnCubAt (float at, NR::Point &iS,
			      path_descr_cubicto & fin, bool before,
			      NR::Point & pos, NR::Point & tgt, float &len, float &rad);
  static void TangentOnBezAt (float at, NR::Point &iS,
			      path_descr_intermbezierto & mid,
			      path_descr_bezierto & fin, bool before,
			      NR::Point & pos, NR::Point & tgt, float &len, float &rad);
  static void OutlineJoin (Path * dest, NR::Point pos, NR::Point stNor, NR::Point enNor,
			   float width, JoinType join, float miter);

  static bool IsNulCurve (path_descr * curD, NR::Point &curX);

  static void RecStdCubicTo (outline_callback_data * data, float tol,
			     float width, int lev);
  static void StdCubicTo (outline_callback_data * data, float tol,
			  float width);
  static void StdBezierTo (outline_callback_data * data, float tol,
			   float width);
  static void RecStdArcTo (outline_callback_data * data, float tol,
			   float width, int lev);
  static void StdArcTo (outline_callback_data * data, float tol, float width);


  // fonctions annexes pour le stroke
  static void DoButt (Shape * dest, float width, ButtType butt, NR::Point pos,
		      NR::Point dir, int &leftNo, int &rightNo);
  static void DoJoin (Shape * dest, float width, JoinType join, NR::Point pos,
		      NR::Point prev, NR::Point next, float miter, float prevL,
		      float nextL, int &leftStNo, int &leftEnNo,
		      int &rightStNo, int &rightEnNo);
  static void DoLeftJoin (Shape * dest, float width, JoinType join, NR::Point pos,
			  NR::Point prev, NR::Point next, float miter, float prevL,
			  float nextL, int &leftStNo, int &leftEnNo,int pathID=-1,int pieceID=0,float tID=0.0);
  static void DoRightJoin (Shape * dest, float width, JoinType join, NR::Point pos,
			   NR::Point prev, NR::Point next, float miter, float prevL,
			   float nextL, int &rightStNo, int &rightEnNo,int pathID=-1,int pieceID=0,float tID=0.0);
  static void RecRound (Shape * dest, int sNo, int eNo, NR::Point &iPt,
			NR::Point &iS, NR::Point &iE, float tresh,
			int lev);
  static void DashTo (Shape * dest, dashTo_info * dTo, float &dashAbs,
		      int &dashNo, float &dashPos, bool & inGap,
		      int &lastLeft, int &lastRight, int nbDash,
		      one_dash * dashs);

  void DoCoalesce (Path * dest, float tresh);

  void DoSimplify (float treshhold);
  bool AttemptSimplify (float treshhold, path_descr_cubicto & res);
  float RaffineTk (NR::Point pt, NR::Point p0, NR::Point p1, NR::Point p2, NR::Point p3, float it);
};
#endif
