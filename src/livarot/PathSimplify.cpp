/*
 *  PathSimplify.cpp
 *  nlivarot
 *
 *  Created by fred on Fri Dec 12 2003.
 *
 */

#include "Path.h"
//#include "MyMath.h"
#include <math.h>
#include "../libnr/nr-matrix.h"

// algo d'origine: http://www.cs.mtu.edu/~shene/COURSES/cs3621/NOTES/INT-APP/CURVE-APP-global.html

// need the b-spline basis for cubic splines
// pas oublier que c'est une b-spline clampee
// et que ca correspond a une courbe de bezier normale
#define N03(t) ((1.0-t)*(1.0-t)*(1.0-t))
#define N13(t) (3*t*(1.0-t)*(1.0-t))
#define N23(t) (3*t*t*(1.0-t))
#define N33(t) (t*t*t)
// quadratic b-splines (jsut in case)
#define N02(t) ((1.0-t)*(1.0-t))
#define N12(t) (2*t*(1.0-t))
#define N22(t) (t*t)
// linear interpolation b-splines
#define N01(t) ((1.0-t))
#define N11(t) (t)


void
Path::Simplify (double treshhold)
{
  if (nbPt <= 1)
    return;
  Reset ();
  
  char *savPts = pts;
  int savNbPt = nbPt;
  
  int lastM = 0;
  while (lastM < savNbPt)
  {
    int lastP = lastM + 1;
    if (back)
    {
    {
      path_lineto_b *tp = (path_lineto_b *) savPts;
      while (lastP < savNbPt
             && ((tp + lastP)->isMoveTo == polyline_lineto
                 || (tp + lastP)->isMoveTo == polyline_forced))
        lastP++;
      pts = (char *) (tp + lastM);
      nbPt = lastP - lastM;
    }
    }
    else
    {
    {
      path_lineto *tp = (path_lineto *) savPts;
      while (lastP < savNbPt
             && ((tp + lastP)->isMoveTo == polyline_lineto
                 || (tp + lastP)->isMoveTo == polyline_forced))
        lastP++;
      pts = (char *) (tp + lastM);
      nbPt = lastP - lastM;
    }
    }
    
    DoSimplify (treshhold);
    
    lastM = lastP;
  }
  
  pts = savPts;
  nbPt = savNbPt;
}

void
Path::DoSimplify (double treshhold)
{
  if (nbPt <= 1)
    return;
  int curP = 0;
  
  char *savPts = pts;
  int savNbPt = nbPt;
  NR::Point moveToPt, endToPt;
  if (back)
  {
	{
	  path_lineto_b *tp = (path_lineto_b *) savPts;
	  moveToPt = tp[0].p;
	}
  }
  else
  {
	{
	  path_lineto *tp = (path_lineto *) savPts;
	  moveToPt = tp[0].p;
	}
  }
  MoveTo (moveToPt);
  endToPt = moveToPt;
  
  while (curP < savNbPt - 1)
  {
    int lastP = curP + 1;
    nbPt = 2;
    if (back)
    {
    {
      path_lineto_b *tp = (path_lineto_b *) savPts;
      pts = (char *) (tp + curP);
    }
    }
    else
    {
    {
      path_lineto *tp = (path_lineto *) savPts;
      pts = (char *) (tp + curP);
    }
    }
    
    path_descr_cubicto res;
    bool contains_forced = false;
    int  forced_pt=-1;
    do
    {
      if (back)
	    {
      {
        path_lineto_b *tp = (path_lineto_b *) savPts;
        if ((tp + lastP)->isMoveTo == polyline_forced)
          contains_forced = true;
        forced_pt=lastP;
      }
	    }
      else
	    {
      {
        path_lineto *tp = (path_lineto *) savPts;
        if ((tp + lastP)->isMoveTo == polyline_forced)
          contains_forced = true;
        forced_pt=lastP;
      }
	    }
      lastP++;
      nbPt++;
      //        if (kissGoodbye)
      //          break;
    }
    while (lastP < savNbPt
           && AttemptSimplify ((contains_forced) ? 0.1 * treshhold : treshhold,
                               res));
    
    if (lastP >= savNbPt)
    {
      //                      printf("o");
      lastP--;
      nbPt--;
    }
    else
    {
      // le dernier a echou√©
      lastP--;
      nbPt--;
      if ( contains_forced ) {
        lastP=forced_pt;
        nbPt=lastP-curP+1;
        AttemptSimplify (treshhold,res);       // ca passe forcement
      }
    }
    if (back)
    {
    {
      path_lineto_b *tp = (path_lineto_b *) savPts;
      endToPt = tp[lastP].p;
    }
    }
    else
    {
    {
      path_lineto *tp = (path_lineto *) savPts;
      endToPt = tp[lastP].p;
    }
    }
    if (nbPt <= 2)
    {
      LineTo (endToPt);
    }
    else
    {
      CubicTo (endToPt, res.stD, res.enD);
    }
    
    curP = lastP;
  }
  
  if (fabs (endToPt.pt[0] - moveToPt.pt[0]) < 0.00001
      && fabs (endToPt.pt[1] - moveToPt.pt[1]) < 0.00001)
    Close ();
  
  pts = savPts;
  nbPt = savNbPt;
}

bool Path::AttemptSimplify (double treshhold, path_descr_cubicto & res)
{
  NR::Point start,end;
  // pour une coordonnee
  double * Xk;				// la coordonnee traitee (x puis y)
  double * Yk;				// la coordonnee traitee (x puis y)
  double * tk;				// les tk
  double *  Qk;				// les Qk
  NR::Matrix M;				// la matrice tNN
  NR::Point P;
  NR::Point Q;
  
  NR::Point cp1, cp2;
  
  if (nbPt == 2)
    return true;
  
  if (back)
  {
 	{
	  path_lineto_b *
    tp = (path_lineto_b *)
    pts;
	  start = tp[0].p;
	  cp1 = tp[1].p;
	  end = tp[nbPt - 1].p;
	}
  }
  else
  {
	{
	  path_lineto *
    tp = (path_lineto *)
    pts;
	  start = tp[0].p;
	  cp1 = tp[1].p;
	  end = tp[nbPt - 1].p;
	}
  }
  
  if (nbPt == 3)
  {
    // start -> cp1 -> end
    res.p = end;
    res.stD = cp1 - start;
    res.enD = end - cp1;
    return true;
  }
  
  // Totally inefficient, allocates & deallocates all the time.
  tk = (double *) malloc (nbPt * sizeof (double));
  Qk = (double *) malloc (nbPt * sizeof (double));
  Xk = (double *) malloc (nbPt * sizeof (double));
  Yk = (double *) malloc (nbPt * sizeof (double));
  
  // chord length method
  tk[0] = 0.0;
  {
    NR::Point prevP =start;
    for (int i = 1; i < nbPt; i++)
    {
      if (back)
      {
      {
        path_lineto_b *
        tp = (path_lineto_b *)
        pts;
        Xk[i] = tp[i].p.pt[0];
        Yk[i] = tp[i].p.pt[1];
      }
      }
      else
      {
      {
        path_lineto *
        tp = (path_lineto *)
        pts;
        Xk[i] = tp[i].p.pt[0];
        Yk[i] = tp[i].p.pt[1];
      }
      }
      NR::Point diff;
      diff.pt[0] = Xk[i] - prevP.pt[0];
      diff.pt[1] = Yk[i] - prevP.pt[1];
      prevP.pt[0] = Xk[i];
      prevP.pt[1] = Yk[i];
      const double l = NR::L2(diff);
      tk[i] = tk[i - 1] + l;
    }
  }
  if (tk[nbPt - 1] < 0.00001)
  {
    // longueur nulle 
    free (tk);
    free (Qk);
    free (Xk);
    free (Yk);
    return false;
  }
  for (int i = 1; i < nbPt - 1; i++)
    tk[i] /= tk[nbPt - 1];
  
  // la matrice tNN
  M.c[0] = M.c[2] = M.c[1] = M.c[3] = M.c[4] = M.c[5] = 0;
  for (int i = 1; i < nbPt - 1; i++)
  {
    M.c[0] += N13 (tk[i]) * N13 (tk[i]);
    M.c[1] += N23 (tk[i]) * N13 (tk[i]);
    M.c[2] += N13 (tk[i]) * N23 (tk[i]);
    M.c[3] += N23 (tk[i]) * N23 (tk[i]);
  }
  
  double
    det=M.det();
  if (fabs (det) < 0.000001)
  {
    // aie, non-inversible
    free (tk);
    free (Qk);
    free (Xk);
    free (Yk);
    return false;
  }
  {
    NR::Matrix  iM=M.inverse();
    M=iM;
  }
  
  // phase 1: abcisses
  // calcul des Qk
  Xk[0] = start.pt[0];
  Yk[0] = start.pt[1];
  Xk[nbPt - 1] = end.pt[0];
  Yk[nbPt - 1] = end.pt[1];
  
  for (int i = 1; i < nbPt - 1; i++)
    Qk[i] = Xk[i] - N03 (tk[i]) * Xk[0] - N33 (tk[i]) * Xk[nbPt - 1];
  
  // le vecteur Q
  Q.pt[0] = Q.pt[1] = 0;
  for (int i = 1; i < nbPt - 1; i++)
  {
    Q.pt[0] += N13 (tk[i]) * Qk[i];
    Q.pt[1] += N23 (tk[i]) * Qk[i];
  }
  
  P=M*Q;
  //  L_MAT_MulV (M, Q, P);
  cp1.pt[0] = P.pt[0];
  cp2.pt[0] = P.pt[1];
  
  // phase 2: les ordonnees
  for (int i = 1; i < nbPt - 1; i++)
    Qk[i] = Yk[i] - N03 (tk[i]) * Yk[0] - N33 (tk[i]) * Yk[nbPt - 1];
  
  // le vecteur Q
  Q.pt[0] = Q.pt[1] = 0;
  for (int i = 1; i < nbPt - 1; i++)
  {
    Q.pt[0] += N13 (tk[i]) * Qk[i];
    Q.pt[1] += N23 (tk[i]) * Qk[i];
  }
  
  P=M*Q;
  //  L_MAT_MulV (M, Q, P);
  cp1.pt[1] = P.pt[0];
  cp2.pt[1] = P.pt[1];
  
  double
    delta =
    0;
  for (int i = 1; i < nbPt - 1; i++)
  {
    NR::Point
    appP;
    appP.pt[0] = N13 (tk[i]) * cp1.pt[0] + N23 (tk[i]) * cp2.pt[0];
    appP.pt[1] = N13 (tk[i]) * cp1.pt[1] + N23 (tk[i]) * cp2.pt[1];
    appP.pt[0] -= Xk[i] - N03 (tk[i]) * Xk[0] - N33 (tk[i]) * Xk[nbPt - 1];
    appP.pt[1] -= Yk[i] - N03 (tk[i]) * Yk[0] - N33 (tk[i]) * Yk[nbPt - 1];
    delta += dot(appP,appP);
  }
  
  
  if (delta < treshhold * treshhold)
  {
    // premier jet
    res.stD = 3.0 * (cp1 - start);
    res.enD = -3.0 * (cp2 - end);
    res.p = end;
    
    // Refine a little.
    for (int i = 1; i < nbPt - 1; i++)
    {
      NR::Point
	    pt;
      pt.pt[0] = Xk[i];
      pt.pt[1] = Yk[i];
      tk[i] = RaffineTk (pt, start, cp1, cp2, end, tk[i]);
      if (tk[i] < tk[i - 1])
	    {
	      // Force tk to be monotonic non-decreasing.
	      tk[i] = tk[i - 1];
	    }
    }
    
    // la matrice tNN
    M.c[0] = M.c[2] = M.c[1] = M.c[3] = M.c[4] = M.c[5] = 0;
    for (int i = 1; i < nbPt - 1; i++)
    {
      M.c[0] += N13 (tk[i]) * N13 (tk[i]);
      M.c[1] += N23 (tk[i]) * N13 (tk[i]);
      M.c[2] += N13 (tk[i]) * N23 (tk[i]);
      M.c[3] += N23 (tk[i]) * N23 (tk[i]);
    }
    
    det=M.det();
    if (fabs (det) < 0.000001)
    {
      // aie, non-invertible
      
      free (tk);
      free (Qk);
      free (Xk);
      free (Yk);
      return true;
    }
    {
      NR::Matrix  iM=M.inverse();
      M=iM;
    }
    
    
    // phase 1: abcisses
    // calcul des Qk
    Xk[0] = start.pt[0];
    Yk[0] = start.pt[1];
    Xk[nbPt - 1] = end.pt[0];
    Yk[nbPt - 1] = end.pt[1];
    
    for (int i = 1; i < nbPt - 1; i++)
      Qk[i] = Xk[i] - N03 (tk[i]) * Xk[0] - N33 (tk[i]) * Xk[nbPt - 1];
    
    // le vecteur Q
    Q.pt[0] = Q.pt[1] = 0;
    for (int i = 1; i < nbPt - 1; i++)
    {
      Q.pt[0] += N13 (tk[i]) * Qk[i];
      Q.pt[1] += N23 (tk[i]) * Qk[i];
    }
    
    P=M*Q;
    //      L_MAT_MulV (M, Q, P);
    cp1.pt[0] = P.pt[0];
    cp2.pt[0] = P.pt[1];
    
    // phase 2: les ordonnees
    for (int i = 1; i < nbPt - 1; i++)
      Qk[i] = Yk[i] - N03 (tk[i]) * Yk[0] - N33 (tk[i]) * Yk[nbPt - 1];
    
    // le vecteur Q
    Q.pt[0] = Q.pt[1] = 0;
    for (int i = 1; i < nbPt - 1; i++)
    {
      Q.pt[0] += N13 (tk[i]) * Qk[i];
      Q.pt[1] += N23 (tk[i]) * Qk[i];
    }
    
    P=M*Q;
    //      L_MAT_MulV (M, Q, P);
    cp1.pt[1] = P.pt[0];
    cp2.pt[1] = P.pt[1];
    
    double
      ndelta =
      0;
    for (int i = 1; i < nbPt - 1; i++)
    {
      NR::Point
	    appP;
      appP.pt[0] = N13 (tk[i]) * cp1.pt[0] + N23 (tk[i]) * cp2.pt[0];
      appP.pt[1] = N13 (tk[i]) * cp1.pt[1] + N23 (tk[i]) * cp2.pt[1];
      appP.pt[0] -= Xk[i] - N03 (tk[i]) * Xk[0] - N33 (tk[i]) * Xk[nbPt - 1];
      appP.pt[1] -= Yk[i] - N03 (tk[i]) * Yk[0] - N33 (tk[i]) * Yk[nbPt - 1];
      ndelta += dot(appP,appP);
    }
    
    free (tk);
    free (Qk);
    free (Xk);
    free (Yk);
    
    if (ndelta < delta + 0.00001)
    {
      res.stD = 3.0 * (cp1 - start);
      res.enD = -3.0 * (cp2 - end);
      res.p = end;
      return true;
    }
    
    
    return false;
  }
  
  free (tk);
  free (Qk);
  free (Xk);
  free (Yk);
  return false;
}

double
Path::RaffineTk (NR::Point pt, NR::Point p0, NR::Point p1, NR::Point p2, NR::Point p3, double it)
{
  // Refinement of the tk values. 
  // Just one iteration of Newtow Raphson, given that we're approaching the curve anyway.
  // [fr: vu que de toute facon la courbe est approch√©e]
  double Ax, Bx, Cx;
  double Ay, By, Cy;
  Ax =
    pt.pt[0] - p0.pt[0] * N03 (it) - p1.pt[0] * N13 (it) - p2.pt[0] * N23 (it) -
    p3.pt[0] * N33 (it);
  Bx =
    (p1.pt[0] - p0.pt[0]) * N02 (it) + (p2.pt[0] - p1.pt[0]) * N12 (it) + (p3.pt[0] -
                                                                           p2.pt[0]) * N22 (it);
  Cx =
    (p0.pt[0] - 2 * p1.pt[0] + p2.pt[0]) * N01 (it) + (p3.pt[0] - 2 * p2.pt[0] + p1.pt[0]) * N11 (it);
  Ay =
    pt.pt[1] - p0.pt[1] * N03 (it) - p1.pt[1] * N13 (it) - p2.pt[1] * N23 (it) -
    p3.pt[1] * N33 (it);
  By =
    (p1.pt[1] - p0.pt[1]) * N02 (it) + (p2.pt[1] - p1.pt[1]) * N12 (it) + (p3.pt[1] -
                                                                           p2.pt[1]) * N22 (it);
  Cy =
    (p0.pt[1] - 2 * p1.pt[1] + p2.pt[1]) * N01 (it) + (p3.pt[1] - 2 * p2.pt[1] + p1.pt[1]) * N11 (it);
  double dF, ddF;
  dF = -6 * (Ax * Bx + Ay * By);
  ddF = 18 * (Bx * Bx + By * By) - 12 * (Ax * Cx + Ay * Cy);
  if (fabs (ddF) > 0.0000001)
  {
    return it - dF / ddF;
  }
  return it;
}

void
Path::Coalesce (double tresh)
{
  if (descr_flags & descr_adding_bezier) CancelBezier ();
  if (descr_flags & descr_doing_subpath) CloseSubpath (0);
  if (descr_nb <= 2) return;
  
  SetBackData (false);
  Path*  tempDest=new Path();
  tempDest->SetBackData(false);
  
  ConvertEvenLines (tresh);
  
  
  int                 lastP = 0;
  int                 lastAP=-1;
  // comme les elements sont stockes dans un tableau a part, plus la peine d'optimiser la réécriture dans la meme tableau
//  int writeP = 0; 
  int lastA = descr_cmd[0].associated;
  int prevA = lastA;
  NR::Point           firstP;
  path_descr          lastAddition;
  path_descr_cubicto  pending_cubic;
  
  lastAddition.flags = descr_moveto;
  for (int curP = 0; curP < descr_nb; curP++) {
    int typ = descr_cmd[curP].flags & descr_type_mask;
    int nextA = lastA;
    if (typ == descr_moveto) {
      if (lastAddition.flags != descr_moveto) {
 //       descr_cmd[writeP++] = lastAddition;
        FlushPendingAddition(tempDest,lastAddition,pending_cubic,lastAP);
      }
      lastAddition = descr_cmd[curP];
      lastAP=curP;
//      descr_cmd[writeP++] = lastAddition;
      FlushPendingAddition(tempDest,lastAddition,pending_cubic,lastAP);
      // Added automatically (too bad about multiple moveto's).
      // [fr: (tant pis pour les moveto multiples)]
      
      path_descr_moveto *nData=(path_descr_moveto*)(descr_data+descr_cmd[curP].dStart);
      firstP = nData->p;
      lastA = descr_cmd[curP].associated;
      prevA = lastA;
      lastP = curP;
    } else if (typ == descr_close) {
      nextA = descr_cmd[curP].associated;
      if (lastAddition.flags != descr_moveto)
	    {
	      path_lineto *sav_pts = (path_lineto *) pts;
	      int sav_nbPt = nbPt;
        
	      pts = (char *) (sav_pts + lastA);
	      nbPt = nextA - lastA + 1;
        
	      path_descr_cubicto res;
	      if (AttemptSimplify (tresh, res))
        {
          lastAddition.flags = descr_cubicto;
          pending_cubic=res;
          lastAP=-1;
        } else {
        }
        
//	      descr_cmd[writeP++] = lastAddition;
        FlushPendingAddition(tempDest,lastAddition,pending_cubic,lastAP);
//	      descr_cmd[writeP++] = descr_cmd[curP];
        FlushPendingAddition(tempDest,descr_cmd[curP],pending_cubic,curP);
        
	      pts = (char *) sav_pts;
	      nbPt = sav_nbPt;
	    } else {
//	      descr_cmd[writeP++] = descr_cmd[curP];
        FlushPendingAddition(tempDest,descr_cmd[curP],pending_cubic,curP);
	    }
      lastAddition.flags = descr_moveto;
      prevA = lastA = nextA;
      lastP = curP;
      lastAP=curP;
    } else if (typ == descr_forced) {
      nextA = descr_cmd[curP].associated;
      if (lastAddition.flags != descr_moveto)
	    {
	      path_lineto *sav_pts = (path_lineto *) pts;
	      int sav_nbPt = nbPt;
        
	      pts = (char *) (sav_pts + lastA);
	      nbPt = nextA - lastA + 1;
        
	      path_descr_cubicto res;
	      if (AttemptSimplify (0.1 * tresh, res))
        {		// plus sensible parce que point force
            // ca passe
        } else  {
          // on force l'addition
//          descr_cmd[writeP++] = lastAddition;
          FlushPendingAddition(tempDest,lastAddition,pending_cubic,lastAP);
          lastAddition.flags = descr_moveto;
          prevA = lastA = nextA;
          lastP = curP;
          lastAP=curP;
        }
        
	      pts = (char *) sav_pts;
	      nbPt = sav_nbPt;
	    }  else
	    {
	    }
    } else if (typ == descr_lineto || typ == descr_cubicto
             || typ == descr_arcto) {
      nextA = descr_cmd[curP].associated;
      if (lastAddition.flags != descr_moveto) {
	      path_lineto *sav_pts = (path_lineto *) pts;
	      int sav_nbPt = nbPt;
        
	      pts = (char *) (sav_pts + lastA);
	      nbPt = nextA - lastA + 1;
        
	      path_descr_cubicto res;
	      if (AttemptSimplify (tresh, res))
        {
          lastAddition.flags = descr_cubicto;
          pending_cubic=res;
          lastAddition.associated = lastA;
          lastP = curP;
          lastAP=-1;
        }  else {
          lastA = descr_cmd[lastP].associated;	// pourrait etre surecrit par la ligne suivante
//          descr_cmd[writeP++] = lastAddition;
          FlushPendingAddition(tempDest,lastAddition,pending_cubic,lastAP);
          lastAddition = descr_cmd[curP];
          if ( typ == descr_cubicto ) {
            pending_cubic=*((path_descr_cubicto*)(descr_data+descr_cmd[curP].dStart));
          }
          lastAP=curP;
        }
        
	      pts = (char *) sav_pts;
	      nbPt = sav_nbPt;
	    } else {
	      lastA = prevA /*descr_cmd[curP-1].associated */ ;
	      lastAddition = descr_cmd[curP];
        if ( typ == descr_cubicto ) {
          pending_cubic=*((path_descr_cubicto*)(descr_data+descr_cmd[curP].dStart));
        }
        lastAP=curP;
	    }
      prevA = nextA;
    } else if (typ == descr_bezierto) {
      if (lastAddition.flags != descr_moveto) {
//	      descr_cmd[writeP++] = lastAddition;
        FlushPendingAddition(tempDest,lastAddition,pending_cubic,lastAP);
	      lastAddition.flags = descr_moveto;
	    } else {
	    }
      lastAP=-1;
      lastA = descr_cmd[curP].associated;
      lastP = curP;
      path_descr_bezierto *nBData=(path_descr_bezierto*)(descr_data+descr_cmd[curP].dStart);
      for (int i = 1; i <= nBData->nb; i++) {
//        descr_cmd[writeP++] = descr_cmd[curP + i];
        FlushPendingAddition(tempDest,descr_cmd[curP + i],pending_cubic,curP+i);
      }
      curP += nBData->nb;
      prevA = nextA;
    } else if (typ == descr_interm_bezier) {
      continue;
      //              } else if ( typ == descr_forced ) {
      //                      continue;
      } else {
      continue;
    }
  }
  if (lastAddition.flags != descr_moveto) {
//    descr_cmd[writeP++] = lastAddition;
    FlushPendingAddition(tempDest,lastAddition,pending_cubic,lastAP);
  }
//  descr_nb = writeP;
  
  Copy(tempDest);
  delete tempDest;
}

void
Path::DoCoalesce (Path * dest, double tresh)
{
  
  
}
void   Path::FlushPendingAddition(Path* dest,path_descr &lastAddition,path_descr_cubicto &lastCubic,int lastAP)
{
  int   typ=lastAddition.flags&descr_type_mask;
  if ( typ == descr_moveto ) {
    if ( lastAP >= 0 ) {
      path_descr_moveto* nData=(path_descr_moveto*)(descr_data+descr_cmd[lastAP].dStart);
      dest->MoveTo(nData->p);
    }
  } else if ( typ == descr_close ) {
    dest->Close();
  } else if ( typ == descr_forced ) {
  } else if ( typ == descr_cubicto ) {
    dest->CubicTo(lastCubic.p,lastCubic.stD,lastCubic.enD);
  } else if ( typ == descr_lineto ) {
    if ( lastAP >= 0 ) {
      path_descr_lineto* nData=(path_descr_lineto*)(descr_data+descr_cmd[lastAP].dStart);
      dest->LineTo(nData->p);
    }
  } else if ( typ == descr_arcto ) {
    if ( lastAP >= 0 ) {
      path_descr_arcto* nData=(path_descr_arcto*)(descr_data+descr_cmd[lastAP].dStart);
      dest->ArcTo(nData->p,nData->rx,nData->ry,nData->angle,nData->large,nData->clockwise);
    }
  } else if ( typ == descr_bezierto ) {
    if ( lastAP >= 0 ) {
      path_descr_bezierto* nData=(path_descr_bezierto*)(descr_data+descr_cmd[lastAP].dStart);
      dest->BezierTo(nData->p);
    }
  } else if ( typ == descr_interm_bezier ) {
    if ( lastAP >= 0 ) {
      path_descr_intermbezierto* nData=(path_descr_intermbezierto*)(descr_data+descr_cmd[lastAP].dStart);
      dest->IntermBezierTo(nData->p);
    }
  }
}

