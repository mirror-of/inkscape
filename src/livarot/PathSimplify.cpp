/*
 *  PathSimplify.cpp
 *  nlivarot
 *
 *  Created by fred on Fri Dec 12 2003.
 *
 */

#include "evil-malloc.h"
#include "Path.h"
//#include "MyMath.h"
#include <math.h>
#include <libnr/nr-matrix.h>
#include <libnr/nr-matrix-ops.h>
#include <libnr/nr-point-fns.h>

/*
 * Reassembling polyline segments into cubic bezier patches
 * thes functions do not need the back data. but they are slower than recomposing
 * path descriptions when you have said back data (it's always easier with a model)
 * there's a bezier fitter in bezier-utils.cpp too. the main difference is the way bezier patch are split
 * here: walk on the polyline, trying to extend the portion you can fit by respecting the treshhold, split when 
 * treshhold is exceeded. when encountering a "forced" point, lower the treshhold to favor splitting at that point
 * in bezier-utils: fit the whole polyline, get the position with the higher deviation to the fitted curve, split
 * there and recurse
 */


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

//#define pseudo_douglas_pecker 1


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
    
#ifdef pseudo_douglas_pecker
    {
      NR::Point moveToPt,endToPt;
      if (back) {
        path_lineto_b *tp = (path_lineto_b *) pts;
        moveToPt = tp[0].p;
      } else {
        path_lineto *tp = (path_lineto *) pts;
        moveToPt = tp[0].p;
      }
      if (back) {
        path_lineto_b *tp = (path_lineto_b *) pts;
        endToPt = tp[nbPt-1].p;
      } else {
        path_lineto *tp = (path_lineto *) pts;
        endToPt = tp[nbPt-1].p;
      }
      MoveTo (moveToPt);
      
      if ( nbPt <= 1 ) {
      } else if ( nbPt == 2 ) {
        LineTo(endToPt);
      } else {
        DoSimplify (treshhold,16);
        if (NR::LInfty (endToPt - moveToPt) < 0.0001) Close ();
      }
    }
#else
    DoSimplify (treshhold);
#endif

    lastM = lastP;
  }
  
  pts = savPts;
  nbPt = savNbPt;
}
// dichomtomic method to get distance to curve approximation
// a real polynomial solver would get the minimum more efficiently, but since the polynom
// would likely be of degree >= 5, that would imply using some generic solver, liek using the sturm metod
double RecDistanceToCubic(NR::Point const &iS, NR::Point const &isD, 
                        NR::Point const &iE, NR::Point const &ieD,
                        NR::Point &pt,double current,int lev,double st,double et)
{	
	if ( lev <= 0 ) return current;
	
	NR::Point  m = 0.5*(iS+iE) + 0.125*(isD-ieD);
	NR::Point  md = 0.75*(iE-iS) - 0.125*(isD+ieD);
	double     mt=(st+et)/2;
	
	NR::Point  hisD=0.5*isD;
	NR::Point  hieD=0.5*ieD;
	
  NR::Point  mp=pt-m;
  double     nle=NR::dot(mp,mp);
  if ( nle < current ) {
    current=nle;
    nle=RecDistanceToCubic(iS,hisD,m,md,pt,current,lev-1,st,mt);
    if ( nle < current ) current=nle;
    nle=RecDistanceToCubic(m,md,iE,hieD,pt,current,lev-1,mt,et);
    if ( nle < current ) current=nle;
  } else if ( nle < 2*current ) {
    nle=RecDistanceToCubic(iS,hisD,m,md,pt,current,lev-1,st,mt);
    if ( nle < current ) current=nle;
    nle=RecDistanceToCubic(m,md,iE,hieD,pt,current,lev-1,mt,et);
    if ( nle < current ) current=nle;
  }
  return current;
}
double    DistanceToCubic(NR::Point &start,Path::path_descr_cubicto res,NR::Point &pt)
{
  NR::Point  sp=pt-start;
  NR::Point  ep=pt-res.p;
  double     nle=NR::dot(sp,sp);
  double     nnle=NR::dot(ep,ep);
  if ( nnle < nle ) nle=nnle;
  NR::Point seg=res.p-start;
  nnle=NR::cross(seg,sp);
  nnle*=nnle;
  nnle/=NR::dot(seg,seg);
  if ( nnle < nle ) {
    if ( NR::dot(sp,seg) >= 0 ) {
      seg=start-res.p;
      if ( NR::dot(ep,seg) >= 0 ) {
        nle=nnle;
      }
    }
  }
  return nle;
  
//  return RecDistanceToCubic(start,res.stD,res.p,res.enD,pt,nle,8,0.0,1.0);
}
// simplification on a subpath
#ifdef pseudo_douglas_pecker
void
Path::DoSimplify (double treshhold,int recLevel)
#else 
void
Path::DoSimplify (double treshhold,int /*recLevel*/)
#endif
{
#ifdef pseudo_douglas_pecker
  if ( nbPt <= 2 ) {
    if (back) {
      path_lineto_b *tp = (path_lineto_b *) pts;
      LineTo(tp[nbPt-1].p);
    } else {
      path_lineto *tp = (path_lineto *) pts;
      LineTo(tp[nbPt-1].p);
    }
    return;
  }
  // dichotomic method: split at worse approximation point, and more precisely, at the closest forced point, if one exists
  path_descr_cubicto  res;
  NR::Point moveToPt;
  if (back) {
	  path_lineto_b *tp = (path_lineto_b *) pts;
	  moveToPt = tp[0].p;
	} else {
	  path_lineto *tp = (path_lineto *) pts;
	  moveToPt = tp[0].p;
  }

  int    worstP=-1;
  if ( AttemptSimplify (treshhold,res,worstP) ) {
    CubicTo(res.p,res.stD,res.enD);
  } else {
    if ( recLevel <= 0 ) {
      CubicTo(res.p,res.stD,res.enD);
      return;
    }
    if ( worstP > 0 && worstP < nbPt-1) { // pas les 2 extremites
      char *savPts = pts;
      int savNbPt = nbPt;
      
      nbPt=worstP;
      DoSimplify (treshhold,recLevel-1);      
      
      if (back) {
        path_lineto_b *tp = (path_lineto_b *) savPts;
        pts = (char *) (tp + worstP);
      } else {
        path_lineto *tp = (path_lineto *) savPts;
        pts = (char *) (tp + worstP);
      }
      nbPt=savNbPt-worstP;
      DoSimplify (treshhold,recLevel-1);      
      
      pts = savPts;
      nbPt = savNbPt;
    } else {
      // pas trouve de point pour casser: pas bon
      CubicTo(res.p,res.stD,res.enD);
    }
  }

#else
  // non-dichotomic method: grow an interval of points approximated by a curve, until you reach the treshhold, and repeat
  if (nbPt <= 1)
    return;
  int curP = 0;
  
  char *savPts = pts;
  int savNbPt = nbPt;
  fitting_tables  data;
  data.Xk=data.Yk=data.Qk=NULL;
  data.tk=data.lk=NULL;
  data.fk=NULL;
  data.totLen=0;
  data.nbPt=data.maxPt=data.inPt=0;
  
  NR::Point moveToPt, endToPt;
  if (back) {
	  path_lineto_b *tp = (path_lineto_b *) savPts;
	  moveToPt = tp[0].p;
  } else {
	  path_lineto *tp = (path_lineto *) savPts;
	  moveToPt = tp[0].p;
  }
  MoveTo (moveToPt);
  endToPt = moveToPt;
  
  while (curP < savNbPt - 1)
  {
    int lastP = curP + 1;
    nbPt = 2;
    if (back) {
      path_lineto_b *tp = (path_lineto_b *) savPts;
      pts = (char *) (tp + curP);
    } else {
      path_lineto *tp = (path_lineto *) savPts;
      pts = (char *) (tp + curP);
    }
    
    // remettre a zero
    data.inPt=data.nbPt=0;

    path_descr_cubicto res;
    bool contains_forced = false;
    int step=64;
    while ( step > 0 ) {   
      int  forced_pt=-1;
      int  worstP=-1;
      do {
        if (back) {
          path_lineto_b *tp = (path_lineto_b *) savPts;
          if ((tp + lastP)->isMoveTo == polyline_forced) contains_forced = true;
          forced_pt=lastP;
        } else {
          path_lineto *tp = (path_lineto *) savPts;
          if ((tp + lastP)->isMoveTo == polyline_forced) contains_forced = true;
          forced_pt=lastP;
        }
        lastP+=step;
        nbPt+=step;
        //        if (kissGoodbye)
        //          break;
      }
      while (lastP < savNbPt && ExtendFit(data,(contains_forced) ? 0.05 * treshhold : treshhold,res,worstP) );
      //       && AttemptSimplify ((contains_forced) ? 0.1 * treshhold : treshhold, res,worstP));
      
      if (lastP >= savNbPt)
      {
        lastP-=step;
        nbPt-=step;
      }
      else
      {
        // le dernier a echoue
        lastP-=step;
        nbPt-=step;
        if ( contains_forced ) {
          lastP=forced_pt;
          nbPt=lastP-curP+1;
        }
        AttemptSimplify (treshhold,res,worstP);       // ca passe forcement
      }
      step/=2;
    }
    
    if (back) {
      path_lineto_b *tp = (path_lineto_b *) savPts;
      endToPt = tp[lastP].p;
    } else {
      path_lineto *tp = (path_lineto *) savPts;
      endToPt = tp[lastP].p;
    }
    if (nbPt <= 2) {
      LineTo (endToPt);
    } else {
      CubicTo (endToPt, res.stD, res.enD);
    }
        
    curP = lastP;
  }
  
  if (NR::LInfty (endToPt - moveToPt) < 0.00001) Close ();
  
  if ( data.Xk ) free(data.Xk);
  if ( data.Yk ) free(data.Yk);
  if ( data.Qk ) free(data.Qk);
  if ( data.tk ) free(data.tk);
  if ( data.lk ) free(data.lk);
  if ( data.fk ) free(data.fk);
  pts = savPts;
  nbPt = savNbPt;
#endif
}

// warning: slow
// idea behing this feature: splotches appear when trying to fit a small number of points: you can
// get a cubic bezier that fits the points very well but doesn't fit the polyline itself
// so we add a bit of the error at the middle of each segment of the polyline
// also we restrict this to <=20 points, to avoid unnecessary computations
#define with_splotch_killer

// primitive= calc the cubic bezier patche that fits Xk and Yk best
// Qk est deja alloue
// retourne false si probleme (matrice non-inversible)
bool Path::FitCubic(NR::Point &start,path_descr_cubicto & res,double* Xk,double* Yk,double* Qk,double* tk,int nbPt)
{
  NR::Point end=res.p;
  NR::Point cp1,cp2;
  NR::Point P,Q;
  // la matrice tNN
  NR::Matrix M(0, 0, 0, 0, 0, 0);
  for (int i = 1; i < nbPt - 1; i++)
  {
    M[0] += N13 (tk[i]) * N13 (tk[i]);
    M[1] += N23 (tk[i]) * N13 (tk[i]);
    M[2] += N13 (tk[i]) * N23 (tk[i]);
    M[3] += N23 (tk[i]) * N23 (tk[i]);
  }
  
  double det=M.det();
  if (fabs (det) < 0.000001) {
    res.stD[0]=res.stD[1]=0.0;
    res.enD[0]=res.enD[1]=0.0;
    return false;
  }
  {
    NR::Matrix  iM=M.inverse();
    M=iM;
  }
  
  // phase 1: abcisses
  // calcul des Qk
  Xk[0] = start[0];
  Yk[0] = start[1];
  Xk[nbPt - 1] = end[0];
  Yk[nbPt - 1] = end[1];
  
  for (int i = 1; i < nbPt - 1; i++) Qk[i] = Xk[i] - N03 (tk[i]) * Xk[0] - N33 (tk[i]) * Xk[nbPt - 1];
  
  // le vecteur Q
  Q[0] = Q[1] = 0;
  for (int i = 1; i < nbPt - 1; i++) {
    Q[0] += N13 (tk[i]) * Qk[i];
    Q[1] += N23 (tk[i]) * Qk[i];
  }
  
  P = Q * M;
  //  L_MAT_MulV (M, Q, P);
  cp1[0] = P[0];
  cp2[0] = P[1];
  
  // phase 2: les ordonnees
  for (int i = 1; i < nbPt - 1; i++) Qk[i] = Yk[i] - N03 (tk[i]) * Yk[0] - N33 (tk[i]) * Yk[nbPt - 1];
  
  // le vecteur Q
  Q[0] = Q[1] = 0;
  for (int i = 1; i < nbPt - 1; i++) {
    Q[0] += N13 (tk[i]) * Qk[i];
    Q[1] += N23 (tk[i]) * Qk[i];
  }
  
  P = Q * M;
  //  L_MAT_MulV (M, Q, P);
  cp1[1] = P[0];
  cp2[1] = P[1];
  
  res.stD = 3.0 * (cp1 - start);
  res.enD = 3.0 * (end - cp2 );

  return true;
}
/*typedef struct fitting_tables {
  int      nbPt,maxPt,inPt;
  double   *Xk;
  double   *Yk;
  double   *Qk;
  double   *tk;
  double   *lk;
  char     *fk;
} fitting_tables;*/
bool   Path::ExtendFit(fitting_tables &data,double treshhold, path_descr_cubicto & res,int &worstP)
{
  if ( nbPt >= data.maxPt ) {
    data.maxPt=2*nbPt+1;
    data.Xk=(double*)realloc(data.Xk,data.maxPt*sizeof(double));
    data.Yk=(double*)realloc(data.Yk,data.maxPt*sizeof(double));
    data.Qk=(double*)realloc(data.Qk,data.maxPt*sizeof(double));
    data.tk=(double*)realloc(data.tk,data.maxPt*sizeof(double));
    data.lk=(double*)realloc(data.lk,data.maxPt*sizeof(double));
    data.fk=(char*)realloc(data.fk,data.maxPt*sizeof(char));
  }
  if ( nbPt > data.inPt ) {
    if ( back ) {
      path_lineto_b*  tp=(path_lineto_b*)pts;
      for (int i=data.inPt;i<nbPt;i++) {
        data.Xk[i] = tp[i].p[0];
        data.Yk[i] = tp[i].p[1];
        data.fk[i]=( tp[i].isMoveTo == polyline_forced )? 0x01:0x00;        
      }
    } else {
      path_lineto*  tp=(path_lineto*)pts;
      for (int i=data.inPt;i<nbPt;i++) {
        data.Xk[i] = tp[i].p[0];
        data.Yk[i] = tp[i].p[1];
        data.fk[i]=( tp[i].isMoveTo == polyline_forced )? 0x01:0x00;        
      }
    }
    data.lk[0]=0;
    data.tk[0]=0;
    double  prevLen=0;
    for (int i=0;i<data.inPt;i++) prevLen+=data.lk[i];
    data.totLen=prevLen;
    for (int i=((data.inPt>0)?data.inPt:1);i<nbPt;i++) {
      NR::Point diff;
      diff[0] = data.Xk[i] - data.Xk[i-1];
      diff[1] = data.Yk[i] - data.Yk[i-1];
      data.lk[i] = NR::L2(diff);
      data.totLen+=data.lk[i];
      data.tk[i] = data.totLen;
    }
    for (int i=0;i<data.inPt;i++) {
      data.tk[i]*=prevLen;
      data.tk[i]/=data.totLen;
    }
    for (int i=data.inPt;i<nbPt;i++) data.tk[i]/=data.totLen;
     data.inPt=nbPt;
  }
  if ( nbPt < data.nbPt ) {
    // on est allŽ trop loin
    // faut recalculer les tk
    data.totLen=0;
    data.tk[0]=0;
    data.lk[0]=0;
    for (int i=1;i<nbPt;i++) {
      data.totLen+=data.lk[i];
      data.tk[i]=data.totLen;
    }
    for (int i=1;i<nbPt;i++) data.tk[i]/=data.totLen;
  }
  
  data.nbPt=nbPt;
  
  if ( data.nbPt <= 0 ) return false;
  
  res.p[0]=data.Xk[data.nbPt-1];
  res.p[1]=data.Yk[data.nbPt-1];
  res.stD[0]=res.stD[1]=0;
  res.enD[0]=res.enD[1]=0;
  worstP=1;
  if ( nbPt <= 2 ) return true;
  
  if ( data.totLen < 0.0001 ) {
    double    worstD=0;
    NR::Point start;
    worstP=-1;
    start[0]=data.Xk[0];
    start[1]=data.Yk[0];
    for (int i=1;i<nbPt;i++) {
      NR::Point nPt;
      bool      isForced=data.fk[i];
      nPt[0]=data.Xk[i];
      nPt[1]=data.Yk[i];
      
      double nle=DistanceToCubic(start,res,nPt);
      if ( isForced ) {
        // forced points are favored for splitting the recursion; we do this by increasing their distance
        if ( worstP < 0 || 2*nle > worstD ) {
          worstP=i;
          worstD=2*nle;
        }
      } else {
        if ( worstP < 0 || nle > worstD ) {
          worstP=i;
          worstD=nle;
        }
      }
    }
    return true;
  }
  
  return AttemptSimplify(data,treshhold,res,worstP);
}
// fit a polyline to a bezier patch, return true is treshhold not exceeded (ie: you can continue)
// version that uses tables from the previous iteration, to minimize amount of work done
bool Path::AttemptSimplify (fitting_tables &data,double treshhold, path_descr_cubicto & res,int &worstP)
{
  NR::Point start,end;
  // pour une coordonnee
  NR::Point cp1, cp2;
  
  worstP=1;
  if (nbPt == 2) {
    return true;
  }
  
  start[0]=data.Xk[0];
  start[1]=data.Yk[0];
  cp1[0]=data.Xk[1];
  cp1[1]=data.Yk[1];
  end[0]=data.Xk[data.nbPt-1];
  end[1]=data.Yk[data.nbPt-1];
  cp2=cp1;
  
  if (nbPt == 3)
  {
    // start -> cp1 -> end
    res.stD = cp1 - start;
    res.enD = end - cp1;
    worstP=1;
    return true;
  }
  
  if ( FitCubic(start,res,data.Xk,data.Yk,data.Qk,data.tk,data.nbPt) ) {
    cp1=start+res.stD/3;
    cp2=end-res.enD/3;
  } else {
    // aie, non-inversible
    double worstD=0;
    worstP=-1;
    for (int i=1;i<data.nbPt;i++) {
      NR::Point nPt;
      nPt[0]=data.Xk[i];
      nPt[1]=data.Yk[i];
      double nle=DistanceToCubic(start,res,nPt);
      if ( data.fk[i] ) {
        // forced points are favored for splitting the recursion; we do this by increasing their distance
        if ( worstP < 0 || 2*nle > worstD ) {
          worstP=i;
          worstD=2*nle;
        }
      } else {
        if ( worstP < 0 || nle > worstD ) {
          worstP=i;
          worstD=nle;
        }
      }
    }
    return false;
  }
   
  // calcul du delta= pondere par les longueurs des segments
  double delta = 0;
  {
    double worstD=0;
    worstP=-1;
    NR::Point   prevAppP;
    NR::Point   prevP;
    double      prevDist;
    prevP[0]=data.Xk[0];
    prevP[1]=data.Yk[0];
    prevAppP=prevP; // le premier seulement
    prevDist=0;
#ifdef with_splotch_killer
    if ( data.nbPt <= 20 ) {
      for (int i = 1; i < data.nbPt - 1; i++)
      {
        NR::Point curAppP;
        NR::Point curP;
        double    curDist;
        NR::Point midAppP;
        NR::Point midP;
        double    midDist;
        
        curAppP[0] = N13 (data.tk[i]) * cp1[0] + N23 (data.tk[i]) * cp2[0] + N03 (data.tk[i]) * data.Xk[0] + N33 (data.tk[i]) * data.Xk[data.nbPt - 1];
        curAppP[1] = N13 (data.tk[i]) * cp1[1] + N23 (data.tk[i]) * cp2[1] + N03 (data.tk[i]) * data.Yk[0] + N33 (data.tk[i]) * data.Yk[data.nbPt - 1];
        curP[0]=data.Xk[i];
        curP[1]=data.Yk[i];
        double mtk=0.5*(data.tk[i]+data.tk[i-1]);
        midAppP[0] = N13 (mtk) * cp1[0] + N23 (mtk) * cp2[0] + N03 (mtk) * data.Xk[0] + N33 (mtk) * data.Xk[data.nbPt - 1];
        midAppP[1] = N13 (mtk) * cp1[1] + N23 (mtk) * cp2[1] + N03 (mtk) * data.Yk[0] + N33 (mtk) * data.Yk[data.nbPt - 1];
        midP=0.5*(curP+prevP);
        
        NR::Point diff;
        diff=curAppP-curP;
        curDist=dot(diff,diff);
        diff=midAppP-midP;
        midDist=dot(diff,diff);
        
        delta+=0.3333*(curDist+prevDist+midDist) *data.lk[i];
        if ( curDist > worstD ) {
          worstD=curDist;
          worstP=i;
        } else if ( data.fk[i] && 2*curDist > worstD ) {
          worstD=2*curDist;
          worstP=i;
        }
        prevP=curP;
        prevAppP=curAppP;
        prevDist=curDist;
      }
      delta/=data.totLen;
    } else {
#endif
      for (int i = 1; i < data.nbPt - 1; i++)
      {
        NR::Point curAppP;
        NR::Point curP;
        double    curDist;
        
        curAppP[0] = N13 (data.tk[i]) * cp1[0] + N23 (data.tk[i]) * cp2[0] + N03 (data.tk[i]) * data.Xk[0] + N33 (data.tk[i]) * data.Xk[data.nbPt - 1];
        curAppP[1] = N13 (data.tk[i]) * cp1[1] + N23 (data.tk[i]) * cp2[1] + N03 (data.tk[i]) * data.Yk[0] + N33 (data.tk[i]) * data.Yk[data.nbPt - 1];
        curP[0]=data.Xk[i];
        curP[1]=data.Yk[i];
      
        NR::Point diff;
        diff=curAppP-curP;
        curDist=dot(diff,diff);
        delta+=curDist;
        
        if ( curDist > worstD ) {
          worstD=curDist;
          worstP=i;
        } else if ( data.fk[i] && 2*curDist > worstD ) {
          worstD=2*curDist;
          worstP=i;
        }
        prevP=curP;
        prevAppP=curAppP;
        prevDist=curDist;
      }
#ifdef with_splotch_killer
    }
#endif
  }
  
  if (delta < treshhold * treshhold)
  {
    // premier jet
    
    // Refine a little.
    for (int i = 1; i < data.nbPt - 1; i++)
    {
      NR::Point
	    pt;
      pt[0] = data.Xk[i];
      pt[1] = data.Yk[i];
      data.tk[i] = RaffineTk (pt, start, cp1, cp2, end, data.tk[i]);
      if (data.tk[i] < data.tk[i - 1])
	    {
	      // Force tk to be monotonic non-decreasing.
	      data.tk[i] = data.tk[i - 1];
	    }
    }
    
    if ( FitCubic(start,res,data.Xk,data.Yk,data.Qk,data.tk,data.nbPt) ) {
    } else {
      // ca devrait jamais arriver, mais bon
      res.stD = 3.0 * (cp1 - start);
      res.enD = 3.0 * (end - cp2 );
      return true;
    }
    double ndelta = 0;
    {
      double worstD=0;
      worstP=-1;
      NR::Point   prevAppP;
      NR::Point   prevP;
      double      prevDist;
      prevP[0]=data.Xk[0];
      prevP[1]=data.Yk[0];
      prevAppP=prevP; // le premier seulement
      prevDist=0;
#ifdef with_splotch_killer
      if ( data.nbPt <= 20 ) {
        for (int i = 1; i < data.nbPt - 1; i++)
        {
          NR::Point curAppP;
          NR::Point curP;
          double    curDist;
          NR::Point midAppP;
          NR::Point midP;
          double    midDist;
          
          curAppP[0] = N13 (data.tk[i]) * cp1[0] + N23 (data.tk[i]) * cp2[0] + N03 (data.tk[i]) * data.Xk[0] + N33 (data.tk[i]) * data.Xk[data.nbPt - 1];
          curAppP[1] = N13 (data.tk[i]) * cp1[1] + N23 (data.tk[i]) * cp2[1] + N03 (data.tk[i]) * data.Yk[0] + N33 (data.tk[i]) * data.Yk[data.nbPt - 1];
          curP[0]=data.Xk[i];
          curP[1]=data.Yk[i];
          double mtk=0.5*(data.tk[i]+data.tk[i-1]);
          midAppP[0] = N13 (mtk) * cp1[0] + N23 (mtk) * cp2[0] + N03 (mtk) * data.Xk[0] + N33 (mtk) * data.Xk[data.nbPt - 1];
          midAppP[1] = N13 (mtk) * cp1[1] + N23 (mtk) * cp2[1] + N03 (mtk) * data.Yk[0] + N33 (mtk) * data.Yk[data.nbPt - 1];
          midP=0.5*(curP+prevP);
          
          NR::Point diff;
          diff=curAppP-curP;
          curDist=dot(diff,diff);
          
          diff=midAppP-midP;
          midDist=dot(diff,diff);
          
          ndelta+=0.3333*(curDist+prevDist+midDist) *data.lk[i];
          
          if ( curDist > worstD ) {
            worstD=curDist;
            worstP=i;
          } else if ( data.fk[i] && 2*curDist > worstD ) {
            worstD=2*curDist;
            worstP=i;
          }
          prevP=curP;
          prevAppP=curAppP;
          prevDist=curDist;
        }
        ndelta/=data.totLen;
      } else {
#endif
        for (int i = 1; i < data.nbPt - 1; i++)
        {
          NR::Point curAppP;
          NR::Point curP;
          double    curDist;
          
          curAppP[0] = N13 (data.tk[i]) * cp1[0] + N23 (data.tk[i]) * cp2[0] + N03 (data.tk[i]) * data.Xk[0] + N33 (data.tk[i]) * data.Xk[data.nbPt - 1];
          curAppP[1] = N13 (data.tk[i]) * cp1[1] + N23 (data.tk[i]) * cp2[1] + N03 (data.tk[i]) * data.Yk[0] + N33 (data.tk[i]) * data.Yk[data.nbPt - 1];
          curP[0]=data.Xk[i];
          curP[1]=data.Yk[i];
        
          NR::Point diff;
          diff=curAppP-curP;
          curDist=dot(diff,diff);

          ndelta+=curDist;

          if ( curDist > worstD ) {
            worstD=curDist;
            worstP=i;
          } else if ( data.fk[i] && 2*curDist > worstD ) {
            worstD=2*curDist;
            worstP=i;
          }
          prevP=curP;
          prevAppP=curAppP;
          prevDist=curDist;
        }
#ifdef with_splotch_killer
      }
#endif
    }
    
    if (ndelta < delta + 0.00001)
    {
      return true;
    } else {
      // nothing better to do
      res.stD = 3.0 * (cp1 - start);
      res.enD = 3.0 * (end - cp2 );
    }
    return true;
  } else {    
    // nothing better to do
  }
  
  return false;
}
bool Path::AttemptSimplify (double treshhold, path_descr_cubicto & res,int &worstP)
{
  NR::Point start,end;
  // pour une coordonnee
  double * Xk;				// la coordonnee traitee (x puis y)
  double * Yk;				// la coordonnee traitee (x puis y)
  double * lk;				// les longueurs de chaque segment
  double * tk;				// les tk
  double *  Qk;				// les Qk
  char *    fk;       // si point force
  
  NR::Point cp1, cp2;
  
  if (nbPt == 2) {
    worstP=1;
    return true;
  }
  
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
  
  res.p = end;
  res.stD[0]=res.stD[1]=0;
  res.enD[0]=res.enD[1]=0;
  if (nbPt == 3)
  {
    // start -> cp1 -> end
    res.stD = cp1 - start;
    res.enD = end - cp1;
    worstP=1;
    return true;
  }
  
  // Totally inefficient, allocates & deallocates all the time.
  tk = (double *) evil_malloc (nbPt * sizeof (double));
  Qk = (double *) evil_malloc (nbPt * sizeof (double));
  Xk = (double *) evil_malloc (nbPt * sizeof (double));
  Yk = (double *) evil_malloc (nbPt * sizeof (double));
  lk = (double *) evil_malloc (nbPt * sizeof (double));
  fk = (char *) evil_malloc (nbPt * sizeof (char));
  
  // chord length method
  tk[0] = 0.0;
  lk[0] = 0.0;
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
        Xk[i] = tp[i].p[0];
        Yk[i] = tp[i].p[1];
        if ( tp[i].isMoveTo == polyline_forced ) fk[i]=0x01; else fk[i]=0;
      }
      }
      else
      {
      {
        path_lineto *
        tp = (path_lineto *)
        pts;
        Xk[i] = tp[i].p[0];
        Yk[i] = tp[i].p[1];
        if ( tp[i].isMoveTo == polyline_forced ) fk[i]=0x01; else fk[i]=0;
      }
      }
      NR::Point diff;
      diff[0] = Xk[i] - prevP[0];
      diff[1] = Yk[i] - prevP[1];
      prevP[0] = Xk[i];
      prevP[1] = Yk[i];
      lk[i] = NR::L2(diff);
      tk[i] = tk[i - 1] + lk[i];
    }
  }
  if (tk[nbPt - 1] < 0.00001)
  {
    // longueur nulle 
    res.stD[0]=res.stD[1]=0;
    res.enD[0]=res.enD[1]=0;
    double worstD=0;
    worstP=-1;
    for (int i=1;i<nbPt;i++) {
      NR::Point nPt;
      bool      isForced=fk[i];
      nPt[0]=Xk[i];
      nPt[1]=Yk[i];
 
      double nle=DistanceToCubic(start,res,nPt);
      if ( isForced ) {
        // forced points are favored for splitting the recursion; we do this by increasing their distance
        if ( worstP < 0 || 2*nle > worstD ) {
          worstP=i;
          worstD=2*nle;
        }
      } else {
        if ( worstP < 0 || nle > worstD ) {
          worstP=i;
          worstD=nle;
        }
      }
    }
    free (tk);
    free (Qk);
    free (Xk);
    free (Yk);
    free (fk);
    free (lk);
    return false;
  }
  double   totLen=tk[nbPt - 1];
  for (int i = 1; i < nbPt - 1; i++) tk[i] /= totLen;
  
  res.p=end;
  if ( FitCubic(start,res,Xk,Yk,Qk,tk,nbPt) ) {
    cp1=start+res.stD/3;
    cp2=end+res.enD/3;
  } else {
    // aie, non-inversible
    res.stD[0]=res.stD[1]=0;
    res.enD[0]=res.enD[1]=0;
    double worstD=0;
    worstP=-1;
    for (int i=1;i<nbPt;i++) {
      NR::Point nPt;
      bool      isForced=fk[i];
      nPt[0]=Xk[i];
      nPt[1]=Yk[i];
      double nle=DistanceToCubic(start,res,nPt);
      if ( isForced ) {
        // forced points are favored for splitting the recursion; we do this by increasing their distance
        if ( worstP < 0 || 2*nle > worstD ) {
          worstP=i;
          worstD=2*nle;
        }
      } else {
        if ( worstP < 0 || nle > worstD ) {
          worstP=i;
          worstD=nle;
        }
      }
    }
    free (tk);
    free (Qk);
    free (Xk);
    free (Yk);
    free (fk);
    free (lk);
    return false;
  }
   
  // calcul du delta= pondere par les longueurs des segments
  double delta = 0;
  {
    double worstD=0;
    worstP=-1;
    NR::Point   prevAppP;
    NR::Point   prevP;
    double      prevDist;
    prevP[0]=Xk[0];
    prevP[1]=Yk[0];
    prevAppP=prevP; // le premier seulement
    prevDist=0;
#ifdef with_splotch_killer
    if ( nbPt <= 20 ) {
      for (int i = 1; i < nbPt - 1; i++)
      {
        NR::Point curAppP;
        NR::Point curP;
        double    curDist;
        NR::Point midAppP;
        NR::Point midP;
        double    midDist;
        
        curAppP[0] = N13 (tk[i]) * cp1[0] + N23 (tk[i]) * cp2[0] + N03 (tk[i]) * Xk[0] + N33 (tk[i]) * Xk[nbPt - 1];
        curAppP[1] = N13 (tk[i]) * cp1[1] + N23 (tk[i]) * cp2[1] + N03 (tk[i]) * Yk[0] + N33 (tk[i]) * Yk[nbPt - 1];
        curP[0]=Xk[i];
        curP[1]=Yk[i];
        midAppP[0] = N13 (0.5*(tk[i]+tk[i-1])) * cp1[0] + N23 (0.5*(tk[i]+tk[i-1])) * cp2[0] + N03 (0.5*(tk[i]+tk[i-1])) * Xk[0] + N33 (0.5*(tk[i]+tk[i-1])) * Xk[nbPt - 1];
        midAppP[1] = N13 (0.5*(tk[i]+tk[i-1])) * cp1[1] + N23 (0.5*(tk[i]+tk[i-1])) * cp2[1] + N03 (0.5*(tk[i]+tk[i-1])) * Yk[0] + N33 (0.5*(tk[i]+tk[i-1])) * Yk[nbPt - 1];
        midP=0.5*(curP+prevP);
        
        NR::Point diff;
        diff=curAppP-curP;
        curDist=dot(diff,diff);

        diff=midAppP-midP;
        midDist=dot(diff,diff);
        
        delta+=0.3333*(curDist+prevDist+midDist)/**lk[i]*/;

        if ( curDist > worstD ) {
          worstD=curDist;
          worstP=i;
        } else if ( fk[i] && 2*curDist > worstD ) {
          worstD=2*curDist;
          worstP=i;
        }
        prevP=curP;
        prevAppP=curAppP;
        prevDist=curDist;
      }
      delta/=totLen;
    } else {
#endif
      for (int i = 1; i < nbPt - 1; i++)
      {
        NR::Point curAppP;
        NR::Point curP;
        double    curDist;
        
        curAppP[0] = N13 (tk[i]) * cp1[0] + N23 (tk[i]) * cp2[0] + N03 (tk[i]) * Xk[0] + N33 (tk[i]) * Xk[nbPt - 1];
        curAppP[1] = N13 (tk[i]) * cp1[1] + N23 (tk[i]) * cp2[1] + N03 (tk[i]) * Yk[0] + N33 (tk[i]) * Yk[nbPt - 1];
        curP[0]=Xk[i];
        curP[1]=Yk[i];
        
        NR::Point diff;
        diff=curAppP-curP;
        curDist=dot(diff,diff);
        delta+=curDist;
        if ( curDist > worstD ) {
          worstD=curDist;
          worstP=i;
        } else if ( fk[i] && 2*curDist > worstD ) {
          worstD=2*curDist;
          worstP=i;
        }
        prevP=curP;
        prevAppP=curAppP;
        prevDist=curDist;
      }
#ifdef with_splotch_killer
    }
#endif
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
      pt[0] = Xk[i];
      pt[1] = Yk[i];
      tk[i] = RaffineTk (pt, start, cp1, cp2, end, tk[i]);
      if (tk[i] < tk[i - 1])
	    {
	      // Force tk to be monotonic non-decreasing.
	      tk[i] = tk[i - 1];
	    }
    }
    
    if ( FitCubic(start,res,Xk,Yk,Qk,tk,nbPt) ) {
    } else {
      // ca devrait jamais arriver, mais bon
      res.stD = 3.0 * (cp1 - start);
      res.enD = -3.0 * (cp2 - end);
      free (tk);
      free (Qk);
      free (Xk);
      free (Yk);
      free(fk);
      free (lk);
      return true;
    }
    double ndelta = 0;
    {
      double worstD=0;
      worstP=-1;
      NR::Point   prevAppP;
      NR::Point   prevP;
      double      prevDist;
      prevP[0]=Xk[0];
      prevP[1]=Yk[0];
      prevAppP=prevP; // le premier seulement
      prevDist=0;
#ifdef with_splotch_killer
      if ( nbPt <= 20 ) {
        for (int i = 1; i < nbPt - 1; i++)
        {
          NR::Point curAppP;
          NR::Point curP;
          double    curDist;
          NR::Point midAppP;
          NR::Point midP;
          double    midDist;
          
          curAppP[0] = N13 (tk[i]) * cp1[0] + N23 (tk[i]) * cp2[0] + N03 (tk[i]) * Xk[0] + N33 (tk[i]) * Xk[nbPt - 1];
          curAppP[1] = N13 (tk[i]) * cp1[1] + N23 (tk[i]) * cp2[1] + N03 (tk[i]) * Yk[0] + N33 (tk[i]) * Yk[nbPt - 1];
          curP[0]=Xk[i];
          curP[1]=Yk[i];
          midAppP[0] = N13 (0.5*(tk[i]+tk[i-1])) * cp1[0] + N23 (0.5*(tk[i]+tk[i-1])) * cp2[0] + N03 (0.5*(tk[i]+tk[i-1])) * Xk[0] + N33 (0.5*(tk[i]+tk[i-1])) * Xk[nbPt - 1];
          midAppP[1] = N13 (0.5*(tk[i]+tk[i-1])) * cp1[1] + N23 (0.5*(tk[i]+tk[i-1])) * cp2[1] + N03 (0.5*(tk[i]+tk[i-1])) * Yk[0] + N33 (0.5*(tk[i]+tk[i-1])) * Yk[nbPt - 1];
          midP=0.5*(curP+prevP);
          
          NR::Point diff;
          diff=curAppP-curP;
          curDist=dot(diff,diff);
          diff=midAppP-midP;
          midDist=dot(diff,diff);
          
          ndelta+=0.3333*(curDist+prevDist+midDist)/**lk[i]*/;

          if ( curDist > worstD ) {
            worstD=curDist;
            worstP=i;
          } else if ( fk[i] && 2*curDist > worstD ) {
            worstD=2*curDist;
            worstP=i;
          }
          prevP=curP;
          prevAppP=curAppP;
          prevDist=curDist;
        }
        ndelta/=totLen;
      } else {
#endif
        for (int i = 1; i < nbPt - 1; i++)
        {
          NR::Point curAppP;
          NR::Point curP;
          double    curDist;
          
          curAppP[0] = N13 (tk[i]) * cp1[0] + N23 (tk[i]) * cp2[0] + N03 (tk[i]) * Xk[0] + N33 (tk[i]) * Xk[nbPt - 1];
          curAppP[1] = N13 (tk[i]) * cp1[1] + N23 (tk[i]) * cp2[1] + N03 (tk[i]) * Yk[0] + N33 (tk[i]) * Yk[nbPt - 1];
          curP[0]=Xk[i];
          curP[1]=Yk[i];
          
          NR::Point diff;
          diff=curAppP-curP;
          curDist=dot(diff,diff);
          ndelta+=curDist;

          if ( curDist > worstD ) {
            worstD=curDist;
            worstP=i;
          } else if ( fk[i] && 2*curDist > worstD ) {
            worstD=2*curDist;
            worstP=i;
          }
          prevP=curP;
          prevAppP=curAppP;
          prevDist=curDist;
        }
#ifdef with_splotch_killer
      }
#endif
    }
    
    free (tk);
    free (Qk);
    free (Xk);
    free (Yk);
    free(fk);
    free (lk);
    
    if (ndelta < delta + 0.00001)
    {
      return true;
    } else {
      // nothing better to do
      res.stD = 3.0 * (cp1 - start);
      res.enD = -3.0 * (cp2 - end);
    }
    return true;
  } else {    
    // nothing better to do
  }
  
  free (tk);
  free (Qk);
  free (Xk);
  free (Yk);
  free(fk);
  free (lk);
  return false;
}

double
Path::RaffineTk (NR::Point pt, NR::Point p0, NR::Point p1, NR::Point p2, NR::Point p3, double it)
{
  // Refinement of the tk values. 
  // Just one iteration of Newtow Raphson, given that we're approaching the curve anyway.
  // [fr: vu que de toute facon la courbe est approchC)e]
  double Ax, Bx, Cx;
  double Ay, By, Cy;
  Ax =
    pt[0] - p0[0] * N03 (it) - p1[0] * N13 (it) - p2[0] * N23 (it) -
    p3[0] * N33 (it);
  Bx =
    (p1[0] - p0[0]) * N02 (it) + (p2[0] - p1[0]) * N12 (it) + (p3[0] -
                                                                           p2[0]) * N22 (it);
  Cx =
    (p0[0] - 2 * p1[0] + p2[0]) * N01 (it) + (p3[0] - 2 * p2[0] + p1[0]) * N11 (it);
  Ay =
    pt[1] - p0[1] * N03 (it) - p1[1] * N13 (it) - p2[1] * N23 (it) -
    p3[1] * N33 (it);
  By =
    (p1[1] - p0[1]) * N02 (it) + (p2[1] - p1[1]) * N12 (it) + (p3[1] -
                                                                           p2[1]) * N22 (it);
  Cy =
    (p0[1] - 2 * p1[1] + p2[1]) * N01 (it) + (p3[1] - 2 * p2[1] + p1[1]) * N11 (it);
  double dF, ddF;
  dF = -6 * (Ax * Bx + Ay * By);
  ddF = 18 * (Bx * Bx + By * By) - 12 * (Ax * Cx + Ay * Cy);
  if (fabs (ddF) > 0.0000001)
  {
    return it - dF / ddF;
  }
  return it;
}

// variation on the fitting theme: try to merge path commands into cubic bezier patches
// the goal was to reduce the number of path commands, especially when ooperations on path produce lots of small path elements
// ideally you could get rid of very small segments at reduced visual cost
void
Path::Coalesce (double tresh)
{
  if ( descr_flags & descr_adding_bezier ) {
    CancelBezier();
  }
  if ( descr_flags & descr_doing_subpath ) {
    CloseSubpath();
  }
  if (descr_nb <= 2) return;
  
  SetBackData (false);
  Path*  tempDest=new Path();
  tempDest->SetBackData(false);
  
  ConvertEvenLines (0.25*tresh);
  
  
  int                 lastP = 0;
  int                 lastAP=-1;
  // As the elements are stored in a separate tableau, it's no longer worth optimizing the rewriting in the same tableau.
  // [[comme les elements sont stockes dans un tableau a part, plus la peine d'optimiser la rŽŽcriture dans la meme tableau]]
//  int writeP = 0; 
  int lastA = descr_cmd[0].associated;
  int prevA = lastA;
  NR::Point           firstP;
  path_descr          lastAddition;
  bool                containsForced=false;
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
      containsForced=false;
      
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
        int worstP=-1;
	      if (AttemptSimplify ((containsForced)?0.05*tresh:tresh, res,worstP))
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
      containsForced=false;
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
        int worstP=-1;
	      if (AttemptSimplify (0.05 * tresh, res,worstP))
        {		// plus sensible parce que point force
            // ca passe
          containsForced=true;
        } else  {
          // on force l'addition
//          descr_cmd[writeP++] = lastAddition;
          FlushPendingAddition(tempDest,lastAddition,pending_cubic,lastAP);
          lastAddition.flags = descr_moveto;
          prevA = lastA = nextA;
          lastP = curP;
          lastAP=curP;
          containsForced=false;
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
        int worstP=-1;
	      if (AttemptSimplify (tresh, res,worstP))
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
          containsForced=false;
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
        containsForced=false;
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
Path::DoCoalesce (Path * /*dest*/, double /*tresh*/)
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

