/*
 *  Path.h
 *  nlivarot
 *
 *  Created by fred on Tue Jun 17 2003.
 *
 */

#ifndef my_path
#define my_path

#include "LivarotDefs.h"
#include "livarot/livarot-forward.h"
#include "libnr/nr-point.h"

/*
 * the Path class: a structure to hold path description and their polyline approximation (not kept in sync)
 * the path description is built with regular commands like MoveTo() LineTo(), etc
 * the polyline approximation is built by a call to Convert() or its variants
 * another possibility would be to call directly the AddPoint() functions, but that is not encouraged
 * the conversion to polyline can salvage data as to where on the path each polyline's point lies; use
 * ConvertWithBackData() for this. after this call, it's easy to rewind the polyline: sequences of points
 * of the same path command can be reassembled in a command
 */

// path description commands
enum
{
  descr_moveto = 0,         // a moveto
  descr_lineto = 1,         // a (guess what) lineto
  descr_cubicto = 2,
  descr_bezierto = 3,       // "beginning" of a quadratic bezier spline, will contain its endpoint (i know, it's bad...)
  descr_arcto = 4,
  descr_close = 5,
  descr_interm_bezier = 6,  // control point of the bezier spline
  descr_forced = 7,

  descr_type_mask = 15      // the command no will be stored in a "flags" field, potentially with other info, so we need 
                            // a mask to AND the field and extract the command
};

// polyline description commands
enum
{
  polyline_lineto = 0,  // a lineto 
  polyline_moveto = 1,  // a moveto
  polyline_forced = 2   // a forced point, ie a point that was an angle or an intersection in a previous life
                        // or more realistically a control point in the path description that created the polyline
                        // forced points are used as "breakable" points for the polyline -> cubic bezier patch operations
                        // each time the bezier fitter encounters such a point in the polyline, it decreases its treshhold,
                        // so that it is more likely to cut the polyline at that position and produce a bezier patch
};

class Shape;

// a little structure you shouldnt pay attention to
// created because the function invocation was starting to be 2 lines long
struct dashTo_info
{
  double     nDashAbs;
  NR::Point prevP;
  NR::Point curP;
  NR::Point prevD;
  double     prevW;
  double     curW;
};

// path creation: 2 phases: first the path is given as a succession of commands (MoveTo, LineTo, CurveTo...); then it
// is converted in a polyline
// a polylone can be stroked or filled to make a polygon
class Path
{
  friend class Shape;
public:

  // command list structures

  // lineto: a point
  struct path_descr_moveto
  {
    NR::Point  p;
  };

  // lineto: a point
  // MoveTos fit in this category
  struct path_descr_lineto
  {
    NR::Point  p;
  };

  // quadratic bezier curves: a set of control points, and an endpoint
  struct path_descr_bezierto
  {
    NR::Point    p;			// the endpoint's coordinates
    int nb;             // number of control points, stored in the next path description commands
  };
  
  struct path_descr_intermbezierto
  {
    NR::Point    p;			// control point coordinates
  };

  // cubic spline curve: 2 tangents and one endpoint
  struct path_descr_cubicto
  {
    NR::Point    p;
    NR::Point    stD;
    NR::Point    enD;
  };

  // arc: endpoint, 2 radii and one angle, plus 2 booleans to choose the arc (svg style)
  struct path_descr_arcto
  {
    NR::Point    p;
    double       rx,ry;
    double       angle;
    bool         large, clockwise;
  };

  struct path_descr
  {
    int    flags;         // most notably contains the path command no
    int    associated;		// index in the polyline of the point that ends the path portion of this command
    double tSt, tEn;
    int    dStart;        // commands' data is stored in a separate array; dStart is the index of the 
                          // start of the storage for this command
  };

  // flags for the path construction
  enum
  {
    descr_ready = 0,        
    descr_adding_bezier = 1, // we're making a bezier spline, so you can expect  pending_bezier_* to have a value
    descr_doing_subpath = 2, // we're doing a path, so there is a moveto somewhere
    descr_delayed_bezier = 4,// the bezier spline we're doing was initiated by a TempBezierTo(), so we'll need an endpoint
    descr_dirty = 16         // the path description was modified
  };
  
  // some data for the construction: what's pending, and some flags
  int         descr_flags;
  int         pending_bezier_cmd;
  int         pending_bezier_data;
  int         pending_moveto_cmd;
  int         pending_moveto_data;
  // the path description:
  // first the commands: an array of size descr_max containing descr_nb commands
  int         descr_max, descr_nb;
  path_descr  *descr_cmd;
  // and an array of NR::Points, of size ddata_max, of which ddata_nb are used
  // the choice of NR::Point is arbitrary, it could be anything, in fact
  int         ddata_max,ddata_nb;
  NR::Point   *descr_data;

  // polyline storage: a series of coordinates (and maybe weights)
  struct path_lineto
  {
    int isMoveTo;
    NR::Point  p;
  };
  
  // back data: info on where this polyline's segment comes from, ie wich command in the path description: "piece"
  // and what abcissis on the chunk of path for this command: "t"
  // t=0 means it's at the start of the command's chunk, t=1 it's at the end
  struct path_lineto_b : public path_lineto
  {
    int piece;
    double t;
  };
  
  int     nbPt, maxPt, sizePt;
  char    *pts;

  bool back;

  Path();
  ~Path();

  // creation of the path description
  void Reset();		// reset to the empty description
  void Copy (Path * who);

  // the commands...
  int ForcePoint();
  int Close();
  int MoveTo ( NR::Point const &ip);
  int LineTo ( NR::Point const &ip);
  int CubicTo ( NR::Point const &ip,  NR::Point const &iStD,  NR::Point const &iEnD);
  int ArcTo ( NR::Point const &ip, double iRx, double iRy, double angle, bool iLargeArc, bool iClockwise);
  int IntermBezierTo ( NR::Point const &ip);	// add a quadratic bezier spline control point
  int BezierTo ( NR::Point const &ip);	// quadratic bezier spline to this point (control points can be added after this)
  int TempBezierTo();	// start a quadratic bezier spline (control points can be added after this)
  int EndBezierTo();
  int EndBezierTo ( NR::Point const &ip);	// ends a quadratic bezier spline (for curves started with TempBezierTo)

  // transforms a description in a polyline (for stroking and filling)
  // treshhold is the max length^2 (sort of)
  void Convert (double treshhold);
  void ConvertEvenLines (double treshhold);	// decomposes line segments too, for later recomposition
  // same function for use when you want to later recompose the curves from the polyline
  void ConvertWithBackData (double treshhold);
  // bad naughty evil useless function
  void ConvertForOffset (double treshhold, Path * orig, double off_dec);

  // creation of the polyline (you can tinker with these function if you want)
  void SetBackData (bool nVal);	// has back data?
  void ResetPoints (int expected = 0);	// resets to the empty polyline
  int AddPoint ( NR::Point const &iPt, bool mvto = false);	// add point
  int AddPoint ( NR::Point const &iPt, int ip, double it, bool mvto = false);
  int AddForcedPoint ( NR::Point const &iPt);	// add point
  int AddForcedPoint ( NR::Point const &iPt, int ip, double it);

  // transform in a polygon (in a graph, in fact; a subsequent call to ConvertToShape is needed)
  //  - fills the polyline; justAdd=true doesn't reset the Shape dest, but simply adds the polyline into it
  // closeIfNeeded=false prevent the function from closing the path (resulting in a non-eulerian graph
  // pathID is a identification number for the path, and is used for recomposing curves from polylines
  // give each different Path a different ID, and feed the appropriate orig[] to the ConvertToForme() function
  void Fill (Shape * dest, int pathID = -1, bool justAdd =
	     false, bool closeIfNeeded = true, bool invert = false);
  // - stroke the path; usual parameters: type of cap=butt, type of join=join and miter (see LivarotDefs.h)
  // doClose treat the path as closed (ie a loop)
  void Stroke (Shape * dest, bool doClose, double width, JoinType join,
	       ButtType butt, double miter, bool justAdd = false);
  // stroke with dashes
  void Stroke (Shape * dest, bool doClose, double width, JoinType join,
	       ButtType butt, double miter, int nbDash, one_dash * dashs,
	       bool justAdd = false);
  // build a Path that is the outline of the Path instance's description (the result is stored in dest)
  // it doesn't compute the exact offset (it's way too complicated, but an approximation made of cubic bezier patches
  //  and segments. the algorithm was found in a plugin for Impress (by Chris Cox), but i can't find it back...
  void Outline (Path * dest, double width, JoinType join, ButtType butt,
		double miter);
  // half outline with edges having the same direction as the original
  void OutsideOutline (Path * dest, double width, JoinType join, ButtType butt,
		       double miter);
  // half outline with edges having the opposite direction as the original
  void InsideOutline (Path * dest, double width, JoinType join, ButtType butt,
		      double miter);

  // polyline to cubic bezier patches
  void Simplify (double treshhold);
  // description simplification
  void Coalesce (double tresh);

  // utilities
  // piece is a command no in the command list
  // "at" is an abcissis on the path portion associated with this command
  // 0=beginning of portion, 1=end of portion.
  void PointAt (int piece, double at, NR::Point & pos);
  void PointAndTangentAt (int piece, double at, NR::Point & pos, NR::Point & tgt);

  // last control point before the command i (i included)
  // used when dealing with quadratic bezier spline, cause these can contain arbitrarily many commands
  const NR::Point PrevPoint (const int i) const;
  
  // dash the polyline
  // the result is stored in the polyline, so you lose the original. make a copy before if needed
  void  DashPolyline(float head,float tail,float body,int nbD,float *dashs,bool stPlain,float stOffset);
  
  //utilitaire pour inkscape
  void  LoadArtBPath(void *iP,NR::Matrix const &tr,bool doTransformation);
	void* MakeArtBPath();
	
	void  Transform(const NR::Matrix &trans);
  
  // decompose le chemin en ses sous-chemin
  // killNoSurf=true -> oublie les chemins de surface nulle
  Path**      SubPaths(int &outNb,bool killNoSurf);
  // pour recuperer les trous
  // nbNest= nombre de contours
  // conts= debut de chaque contour
  // nesting= parent de chaque contour
  Path**      SubPathsWithNesting(int &outNb,bool killNoSurf,int nbNest,int* nesting,int* conts);
  // surface du chemin (considere comme ferme)
  double      Surface();
  void        PolylineBoundingBox(double &l,double &t,double &r,double &b);
  void        FastBBox(double &l,double &t,double &r,double &b);
  // longueur (totale des sous-chemins)
  double      Length();
  
  void             ConvertForcedToMoveTo();
  void             ConvertForcedToVoid();
  struct cut_position {
    int          piece;
    float        t;
  };
  cut_position*    CurvilignToPosition(int nbCv,double* cvAbs,int &nbCut);
  
  // caution: not tested on quadratic b-splines, most certainly buggy
  void             ConvertPositionsToMoveTo(int nbPos,cut_position* poss);
  void             ConvertPositionsToForced(int nbPos,cut_position* poss);

  void  Affiche();
  char *svg_dump_path ();

private:
    // path storage primitives
  void AlloueDCmd (int addNb);
  void AlloueDData (int addNb);
  void ShiftDCmd(int at,int dec);
  void ShiftDData(int at,int dec);
  // utilitary functions for the path contruction
  void CancelBezier ();
  void CloseSubpath();
  void InsertMoveTo (NR::Point const &iPt,int at);
  void InsertForcePoint (int at);
  void InsertLineTo (NR::Point const &iPt,int at);
  void InsertArcTo (NR::Point const &ip, double iRx, double iRy, double angle, bool iLargeArc, bool iClockwise,int at);
  void InsertCubicTo (NR::Point const &ip,  NR::Point const &iStD,  NR::Point const &iEnD,int at);
  void InsertBezierTo (NR::Point const &iPt,int iNb,int at);
  void InsertIntermBezierTo (NR::Point const &iPt,int at);
  
  int  DataPosForAfter(int cmd);
 
  // creation of dashes: take the polyline given by spP (length spL) and dash it according to head, body, etc. put the result in
  // the polyline of this instance
  void DashSubPath(int spL,char* spP,float head,float tail,float body,int nbD,float *dashs,bool stPlain,float stOffset);

  // Functions used by the conversion.
  // they append points to the polyline
  void DoArc ( NR::Point const &iS,  NR::Point const &iE, double rx, double ry,
	      double angle, bool large, bool wise, double tresh);
  void RecCubicTo ( NR::Point const &iS,  NR::Point const &iSd,  NR::Point const &iE,  NR::Point const &iEd, double tresh, int lev,
		   double maxL = -1.0);
  void RecBezierTo ( NR::Point const &iPt,  NR::Point const &iS,  NR::Point const &iE, double treshhold, int lev, double maxL = -1.0);

  void DoArc ( NR::Point const &iS,  NR::Point const &iE, double rx, double ry,
	      double angle, bool large, bool wise, double tresh, int piece);
  void RecCubicTo ( NR::Point const &iS,  NR::Point const &iSd,  NR::Point const &iE,  NR::Point const &iEd, double tresh, int lev,
		   double st, double et, int piece);
  void RecBezierTo ( NR::Point const &iPt,  NR::Point const &iS, const  NR::Point &iE, double treshhold, int lev, double st, double et,
		    int piece);

  // don't pay attention
  struct offset_orig
  {
    Path *orig;
    int piece;
    double tSt, tEn;
    double off_dec;
  };
  void DoArc ( NR::Point const &iS,  NR::Point const &iE, double rx, double ry,
	      double angle, bool large, bool wise, double tresh, int piece,
	      offset_orig & orig);
  void RecCubicTo ( NR::Point const &iS,  NR::Point const &iSd,  NR::Point const &iE,  NR::Point const &iEd, double tresh, int lev,
		   double st, double et, int piece, offset_orig & orig);
  void RecBezierTo ( NR::Point const &iPt,  NR::Point const &iS,  NR::Point const &iE, double treshhold, int lev, double st, double et,
		    int piece, offset_orig & orig);

  static void ArcAngles ( NR::Point const &iS,  NR::Point const &iE, double rx,
                         double ry, double angle, bool large, bool wise,
                         double &sang, double &eang);
  static void QuadraticPoint (double t,  NR::Point &oPt,   NR::Point const &iS,   NR::Point const &iM,   NR::Point const &iE);
  static void CubicTangent (double t,  NR::Point &oPt,  NR::Point const &iS,
			     NR::Point const &iSd,  NR::Point const &iE,
			     NR::Point const &iEd);

  struct outline_callback_data
  {
    Path *orig;
    int piece;
    double tSt, tEn;
    Path *dest;
    double x1, y1, x2, y2;
    union
    {
      struct
      {
	double dx1, dy1, dx2, dy2;
      }
      c;
      struct
      {
	double mx, my;
      }
      b;
      struct
      {
	double rx, ry, angle;
	bool clock, large;
	double stA, enA;
      }
      a;
    }
    d;
  };

  typedef void (outlineCallback) (outline_callback_data * data, double tol,  double width);
  struct outline_callbacks
  {
    outlineCallback *cubicto;
    outlineCallback *bezierto;
    outlineCallback *arcto;
  };

  void SubContractOutline (Path * dest, outline_callbacks & calls,
			   double tolerance, double width, JoinType join,
			   ButtType butt, double miter, bool closeIfNeeded,
			   bool skipMoveto, NR::Point & lastP, NR::Point & lastT);
  void DoOutsideOutline (Path * dest, double width, JoinType join,
			 ButtType butt, double miter, int &stNo, int &enNo);
  void DoInsideOutline (Path * dest, double width, JoinType join,
			ButtType butt, double miter, int &stNo, int &enNo);
  void DoStroke (Shape * dest, bool doClose, double width, JoinType join,
		 ButtType butt, double miter, bool justAdd = false);
  void DoStroke (Shape * dest, bool doClose, double width, JoinType join,
		 ButtType butt, double miter, int nbDash, one_dash * dashs,
		 bool justAdd = false);

  static void TangentOnSegAt (double at, NR::Point const &iS,
			      path_descr_lineto & fin, NR::Point & pos, NR::Point & tgt,
			      double &len);
  static void TangentOnArcAt (double at, NR::Point const &iS,
			      path_descr_arcto & fin, NR::Point & pos, NR::Point & tgt,
			      double &len, double &rad);
  static void TangentOnCubAt (double at, NR::Point const &iS,
			      path_descr_cubicto & fin, bool before,
			      NR::Point & pos, NR::Point & tgt, double &len, double &rad);
  static void TangentOnBezAt (double at, NR::Point const &iS,
			      path_descr_intermbezierto & mid,
			      path_descr_bezierto & fin, bool before,
			      NR::Point & pos, NR::Point & tgt, double &len, double &rad);
  static void OutlineJoin (Path * dest, NR::Point pos, NR::Point stNor, NR::Point enNor,
			   double width, JoinType join, double miter);

  static bool IsNulCurve (path_descr const * curD, NR::Point const &curX,NR::Point* ddata);

  static void RecStdCubicTo (outline_callback_data * data, double tol,
			     double width, int lev);
  static void StdCubicTo (outline_callback_data * data, double tol,
			  double width);
  static void StdBezierTo (outline_callback_data * data, double tol,
			   double width);
  static void RecStdArcTo (outline_callback_data * data, double tol,
			   double width, int lev);
  static void StdArcTo (outline_callback_data * data, double tol, double width);


  // fonctions annexes pour le stroke
  static void DoButt (Shape * dest, double width, ButtType butt, NR::Point pos,
		      NR::Point dir, int &leftNo, int &rightNo);
  static void DoJoin (Shape * dest, double width, JoinType join, NR::Point pos,
		      NR::Point prev, NR::Point next, double miter, double prevL,
		      double nextL, int &leftStNo, int &leftEnNo,
		      int &rightStNo, int &rightEnNo);
  static void DoLeftJoin (Shape * dest, double width, JoinType join, NR::Point pos,
			  NR::Point prev, NR::Point next, double miter, double prevL,
			  double nextL, int &leftStNo, int &leftEnNo,int pathID=-1,int pieceID=0,double tID=0.0);
  static void DoRightJoin (Shape * dest, double width, JoinType join, NR::Point pos,
			   NR::Point prev, NR::Point next, double miter, double prevL,
			   double nextL, int &rightStNo, int &rightEnNo,int pathID=-1,int pieceID=0,double tID=0.0);
  static void RecRound (Shape * dest, int sNo, int eNo, NR::Point const &iPt,
			NR::Point const &iS, NR::Point const &iE, double tresh,
			int lev,NR::Point &origine,float width);
  static void DashTo (Shape * dest, dashTo_info * dTo, double &dashAbs,
		      int &dashNo, double &dashPos, bool & inGap,
		      int &lastLeft, int &lastRight, int nbDash,
		      one_dash * dashs);

  void DoCoalesce (Path * dest, double tresh);

  void DoSimplify (double treshhold,int recLevel=0);
  bool AttemptSimplify (double treshhold, path_descr_cubicto & res,int &worstP);
  static bool FitCubic(NR::Point &start,path_descr_cubicto & res,double* Xk,double* Yk,double* Qk,double* tk,int nbPt);
  struct fitting_tables {
    int      nbPt,maxPt,inPt;
    double   *Xk;
    double   *Yk;
    double   *Qk;
    double   *tk;
    double   *lk;
    char     *fk;
    double   totLen;
  };
  bool   AttemptSimplify (fitting_tables &data,double treshhold, path_descr_cubicto & res,int &worstP);
  bool   ExtendFit(fitting_tables &data,double treshhold, path_descr_cubicto & res,int &worstP);
  double RaffineTk (NR::Point pt, NR::Point p0, NR::Point p1, NR::Point p2, NR::Point p3, double it);
  void   FlushPendingAddition(Path* dest,path_descr &lastAddition,path_descr_cubicto &lastCubic,int lastAD);
};
#endif
