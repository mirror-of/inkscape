/*
 *  PathStroke.cpp
 *  nlivarot
 *
 *  Created by fred on Tue Jun 17 2003.
 *
 */

#include "Path.h"
#include "Shape.h"

// until i find something better
#define StrokeNormalize(v) {\
  double _l=sqrt(dot(v,v)); \
    if ( _l < 0.0000001 ) { \
      v.pt[0]=v.pt[1]=0; \
    } else { \
      v/=_l; \
    }\
}

void
Path::Stroke (Shape * dest, bool doClose, double width, JoinType join,
              ButtType butt, double miter, bool justAdd)
{
  if (dest == NULL)
    return;
  if (justAdd == false)
  {
    dest->Reset (3 * nbPt, 3 * nbPt);
  }
  if (nbPt <= 1)
    return;
  dest->MakeBackData (false);
  
  char *savPts = pts;
  int savNbPt = nbPt;
  
  int lastM = 0;
  while (lastM < savNbPt)
  {
    int lastP = lastM + 1;
    if (back)
    {
      if (weighted)
	    {
	      path_lineto_wb *tp = (path_lineto_wb *) savPts;
	      while (lastP < savNbPt
               && ((tp + lastP)->isMoveTo == polyline_lineto
                   || (tp + lastP)->isMoveTo == polyline_forced))
          lastP++;
	      pts = (char *) (tp + lastM);
	      nbPt = lastP - lastM;
	    }
      else
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
      if (weighted)
	    {
	      path_lineto_w *tp = (path_lineto_w *) savPts;
	      while (lastP < savNbPt
               && ((tp + lastP)->isMoveTo == polyline_lineto
                   || (tp + lastP)->isMoveTo == polyline_forced))
          lastP++;
	      pts = (char *) (tp + lastM);
	      nbPt = lastP - lastM;
	    }
      else
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
    DoStroke (dest, doClose, width, join, butt, miter, true);
    lastM = lastP;
  }
  
  pts = savPts;
  nbPt = savNbPt;
}

void
Path::DoStroke (Shape * dest, bool doClose, double width, JoinType join,
                ButtType butt, double miter, bool justAdd)
{
  if (nbPt <= 1)
    return;
  
  NR::Point curP, prevP, nextP;
  double curW, prevW, nextW;
  int curI, prevI, nextI;
  int upTo;
  
  curI = 0;
  curP = ((path_lineto *) pts)[0].p;
  if (weighted)
    curW = ((path_lineto_w *) pts)[0].w;
  else
    curW = 1;
  
  if (doClose)
  {
    path_lineto *curPt = (path_lineto *) pts;
    prevI = nbPt - 1;
    if (back)
    {
      if (weighted)
        curPt =
          (path_lineto *) (((char *) curPt) +
                           prevI * sizeof (path_lineto_wb));
      else
        curPt =
          (path_lineto *) (((char *) curPt) +
                           prevI * sizeof (path_lineto_b));
    }
    else
    {
      if (weighted)
        curPt =
          (path_lineto *) (((char *) curPt) +
                           prevI * sizeof (path_lineto_w));
      else
        curPt =
          (path_lineto *) (((char *) curPt) +
                           prevI * sizeof (path_lineto));
    }
    while (prevI > 0)
    {
      prevP = curPt->p;
      if (weighted)
        prevW = ((path_lineto_w *) curPt)->w;
      else
        prevW = 1;
      NR::Point diff=curP-prevP;
      double dist =dot(diff,diff);
      if (dist > 0.001)
	    {
	      break;
	    }
      prevI--;
      if (back)
	    {
	      if (weighted)
          curPt =
            (path_lineto *) (((char *) curPt) -
                             sizeof (path_lineto_wb));
	      else
          curPt =
            (path_lineto *) (((char *) curPt) - sizeof (path_lineto_b));
	    }
      else
	    {
	      if (weighted)
          curPt =
            (path_lineto *) (((char *) curPt) - sizeof (path_lineto_w));
	      else
          curPt =
            (path_lineto *) (((char *) curPt) - sizeof (path_lineto));
	    }
    }
    if (prevI <= 0)
      return;
    upTo = prevI;
  }
  else
  {
    prevP = curP;
    prevW = curW;
    prevI = curI;
    upTo = nbPt - 1;
  }
  {
    path_lineto *curPt = (path_lineto *) pts;
    nextI = 1;
    if (back)
    {
      if (weighted)
        curPt =
          (path_lineto *) (((char *) curPt) +
                           nextI * sizeof (path_lineto_wb));
      else
        curPt =
          (path_lineto *) (((char *) curPt) +
                           nextI * sizeof (path_lineto_b));
    }
    else
    {
      if (weighted)
        curPt =
          (path_lineto *) (((char *) curPt) +
                           nextI * sizeof (path_lineto_w));
      else
        curPt =
          (path_lineto *) (((char *) curPt) + nextI * sizeof (path_lineto));
    }
    while (nextI <= upTo)
    {
      nextP = curPt->p;
      if (weighted)
        nextW = ((path_lineto_w *) curPt)->w;
      else
        nextW = 1;
      NR::Point diff=curP-nextP;
      double dist =dot(diff,diff);
      if (dist > 0.001)
      {
        break;
      }
      nextI++;
      if (back)
      {
        if (weighted)
          curPt =
            (path_lineto *) (((char *) curPt) + sizeof (path_lineto_wb));
        else
          curPt =
            (path_lineto *) (((char *) curPt) + sizeof (path_lineto_b));
      }
      else
      {
        if (weighted)
          curPt =
            (path_lineto *) (((char *) curPt) + sizeof (path_lineto_w));
        else
          curPt =
            (path_lineto *) (((char *) curPt) + sizeof (path_lineto));
      }
    }
    if (nextI > upTo)
      return;
  }
  
  int startLeft = -1, startRight = -1;
  int lastLeft = -1, lastRight = -1;
  NR::Point prevD, nextD;
  double prevLe, nextLe;
  prevD = curP - prevP;
  nextD = nextP - curP;
  prevLe = sqrt (dot(prevD,prevD));
  nextLe = sqrt (dot(nextD,nextD));
  StrokeNormalize (prevD);
  StrokeNormalize (nextD);
  if (doClose)
  {
    DoJoin (dest, curW * width, join, curP, prevD, nextD, miter, prevLe,
            nextLe, startLeft, lastLeft, startRight, lastRight);
  }
  else
  {
    nextD = -nextD;
    DoButt (dest, curW * width, butt, curP, nextD, lastRight, lastLeft);
    nextD = -nextD;
  }
  do
  {
    prevP = curP;
    prevI = curI;
    prevW = curW;
    curP = nextP;
    curI = nextI;
    curW = nextW;
    prevD = nextD;
    prevLe = nextLe;
    nextI++;
    path_lineto *curPt = (path_lineto *) pts;
    if (back)
    {
      if (weighted)
        curPt =
          (path_lineto *) (((char *) curPt) +
                           nextI * sizeof (path_lineto_wb));
      else
        curPt =
          (path_lineto *) (((char *) curPt) +
                           nextI * sizeof (path_lineto_b));
    }
    else
    {
      if (weighted)
        curPt =
          (path_lineto *) (((char *) curPt) +
                           nextI * sizeof (path_lineto_w));
      else
        curPt =
          (path_lineto *) (((char *) curPt) +
                           nextI * sizeof (path_lineto));
    }
    while (nextI <= upTo)
    {
      nextP = curPt->p;
      if (weighted)
        nextW = ((path_lineto_w *) curPt)->w;
      else
        nextW = 1;
      NR::Point diff=curP-nextP;
      double dist =dot(diff,diff);
      if (dist > 0.001)
	    {
	      break;
	    }
      nextI++;
      if (back)
	    {
	      if (weighted)
          curPt =
            (path_lineto *) (((char *) curPt) +
                             sizeof (path_lineto_wb));
	      else
          curPt =
            (path_lineto *) (((char *) curPt) + sizeof (path_lineto_b));
	    }
      else
	    {
	      if (weighted)
          curPt =
            (path_lineto *) (((char *) curPt) + sizeof (path_lineto_w));
	      else
          curPt =
            (path_lineto *) (((char *) curPt) + sizeof (path_lineto));
	    }
    }
    if (nextI > upTo)
      break;
    
    nextD = nextP - curP;
    nextLe = sqrt (dot(nextD,nextD));
    StrokeNormalize (nextD);
    int nStL = -1, nStR = -1, nEnL = -1, nEnR = -1;
    DoJoin (dest, curW * width, join, curP, prevD, nextD, miter, prevLe,
            nextLe, nStL, nEnL, nStR, nEnR);
    dest->AddEdge (nStL, lastLeft);
    lastLeft = nEnL;
    dest->AddEdge (lastRight, nStR);
    lastRight = nEnR;
  }
  while (nextI <= upTo);
  if (doClose)
  {
    /*		prevP=curP;
		prevI=curI;
		prevW=curW;
		curP=nextP;
		curI=nextI;
		curW=nextW;
		prevD=nextD;*/
    path_lineto *curPt = (path_lineto *) pts;
    nextP = curPt->p;
    if (weighted)
      nextW = ((path_lineto_w *) curPt)->w;
    else
      nextW = 1;
    
    nextD = nextP - curP;
    nextLe = sqrt (dot(nextD,nextD));
    StrokeNormalize (nextD);
    int nStL = -1, nStR = -1, nEnL = -1, nEnR = -1;
    DoJoin (dest, curW * width, join, curP, prevD, nextD, miter, prevLe,
            nextLe, nStL, nEnL, nStR, nEnR);
    dest->AddEdge (nStL, lastLeft);
    lastLeft = nEnL;
    dest->AddEdge (lastRight, nStR);
    lastRight = nEnR;
    
    dest->AddEdge (startLeft, lastLeft);
    dest->AddEdge (lastRight, startRight);
  }
  else
  {
    int endRight, endLeft;
    DoButt (dest, curW * width, butt, curP, prevD, endLeft, endRight);
    dest->AddEdge (endLeft, lastLeft);
    dest->AddEdge (lastRight, endRight);
  }
}
void
Path::DoButt (Shape * dest, double width, ButtType butt, NR::Point pos, NR::Point dir,
              int &leftNo, int &rightNo)
{
  NR::Point nor;
  nor=dir.ccw();
  
  if (butt == butt_square)
  {
    NR::Point x;
    x = pos + width * dir + width * nor;
    int bleftNo = dest->AddPoint (x);
    x = pos + width * dir - width * nor;
    int brightNo = dest->AddPoint (x);
    x = pos + width * nor;
    leftNo = dest->AddPoint (x);
    x = pos - width * nor;
    rightNo = dest->AddPoint (x);
    dest->AddEdge (rightNo, brightNo);
    dest->AddEdge (brightNo, bleftNo);
    dest->AddEdge (bleftNo, leftNo);
  }
  else if (butt == butt_pointy)
  {
    NR::Point x;
    x = pos + width * nor;
    leftNo = dest->AddPoint (x);
    x = pos - width * nor;
    rightNo = dest->AddPoint (x);
    x = pos + width * dir;
    int mid = dest->AddPoint (x);
    dest->AddEdge (rightNo, mid);
    dest->AddEdge (mid, leftNo);
  }
  else if (butt == butt_round)
  {
    NR::Point sx, ex, mx;
    sx = pos + width * nor;
    ex = pos - width * nor;
    mx = pos + width * dir;
    leftNo = dest->AddPoint (sx);
    rightNo = dest->AddPoint (ex);
    int midNo = dest->AddPoint (mx);
    
    NR::Point dx;
    dx = pos - width * nor + width * dir;
    RecRound (dest, rightNo, midNo, dx, ex, mx, 5.0, 8);
    dx = pos + width * nor + width * dir;
    RecRound (dest, midNo, leftNo, dx, mx, sx, 5.0, 8);
  }
  else
  {
    NR::Point x;
    x = pos + width * nor;
    leftNo = dest->AddPoint (x);
    x = pos - width * nor;
    rightNo = dest->AddPoint (x);
    dest->AddEdge (rightNo, leftNo);
  }
}
void
Path::DoJoin (Shape * dest, double width, JoinType join, NR::Point pos, NR::Point prev,
              NR::Point next, double miter, double prevL, double nextL, int &leftStNo,
              int &leftEnNo, int &rightStNo, int &rightEnNo)
{
  //      DoLeftJoin(dest,width,join,pos,prev,next,miter,prevL,nextL,leftStNo,leftEnNo);
  //      DoRightJoin(dest,width,join,pos,prev,next,miter,prevL,nextL,rightStNo,rightEnNo);
  //      return;
  
  NR::Point pnor, nnor;
  pnor=prev.ccw();
  nnor=next.ccw();
  double angSi = cross (next, prev);
  if (angSi > -0.0001 && angSi < 0.0001)
  {
    double angCo = dot (prev, next);
    if (angCo > 0.9999)
    {
      // tout droit
      NR::Point x;
      x = pos + width * pnor;
      leftStNo = leftEnNo = dest->AddPoint (x);
      x = pos - width * pnor;
      rightStNo = rightEnNo = dest->AddPoint (x);
    }
    else
    {
      // demi-tour
      NR::Point x;
      x = pos + width * pnor;
      leftStNo = rightEnNo = dest->AddPoint (x);
      x = pos - width * pnor;
      rightStNo = leftEnNo = dest->AddPoint (x);
      dest->AddEdge (leftEnNo, leftStNo);
      dest->AddEdge (rightStNo, rightEnNo);
    }
    return;
  }
  if (angSi < 0)
  {
    NR::Point x;
    {
      NR::Point biss;
      biss = next - prev;
      double c2 = cross (biss, next);
      double l = width / c2;
      double projn = l * (dot (biss, next));
      double projp = -l * (dot (biss, prev));
      if (projp <= 0.5 * prevL && projn <= 0.5 * nextL)
      {
        x = pos + l * biss;
        leftEnNo = leftStNo = dest->AddPoint (x);
      }
      else
      {
        x = pos + width * pnor;
        leftStNo = dest->AddPoint (x);
        x = pos + width * nnor;
        leftEnNo = dest->AddPoint (x);
        dest->AddEdge (leftEnNo, leftStNo);
      }
    }
    if (join == join_pointy)
    {
      x = pos - width * pnor;
      rightStNo = dest->AddPoint (x);
      x = pos - width * nnor;
      rightEnNo = dest->AddPoint (x);
      
      //                      dest->AddEdge(rightStNo,rightEnNo);
      NR::Point biss;
      biss = pnor + nnor;
      StrokeNormalize (biss);
      double c2 = dot (biss, nnor);
      double l = width / c2;
      double emiter = width * c2;
      if (emiter < miter)
        emiter = miter;
      int nrightStNo, nrightEnNo;
      if (l <= emiter)
	    {
	      x = pos - l * biss;
	      nrightStNo = nrightEnNo = dest->AddPoint (x);
	    }
      else
	    {
	      double s2 = cross (biss, nnor);
	      double dec = (l - emiter) * c2 / s2;
	      NR::Point tbiss=biss.ccw();
        
	      x = pos - emiter * biss - dec * tbiss;
	      nrightStNo = dest->AddPoint (x);
	      x = pos - emiter * biss + dec * tbiss;
	      nrightEnNo = dest->AddPoint (x);
	    }
      dest->AddEdge (rightStNo, nrightStNo);
      dest->AddEdge (nrightStNo, nrightEnNo);
      dest->AddEdge (nrightEnNo, rightEnNo);
    }
    else if (join == join_round)
    {
      NR::Point sx = pos - width * pnor;
      rightStNo = dest->AddPoint (sx);
      NR::Point ex = pos - width * nnor;
      rightEnNo = dest->AddPoint (ex);
      
      NR::Point biss;
      biss = pnor + nnor;
      StrokeNormalize (biss);
      double c2 = dot (biss, nnor);
      double l = width / c2;
      double typ = dot (pnor, nnor);
      if (typ >= 0)
	    {
	      x = pos - l * biss;
	      RecRound (dest, rightStNo, rightEnNo, x, sx, ex, 5.0,
                  8);
	    }
      else
	    {
	      double s2 = cross (biss, nnor);
	      double dec = (l - width) * c2 / s2;
	      NR::Point tbiss=biss.ccw();
        
        NR::Point nsx = pos - width * biss - dec * tbiss;
        NR::Point nex = pos - width * biss + dec * tbiss;
        NR::Point mx = pos - width * biss;
	      int midNo = dest->AddPoint (mx);
	      RecRound (dest, rightStNo, midNo, nsx, sx, mx, 5.0,
                  8);
	      RecRound (dest, midNo, rightEnNo, nex, mx, ex, 5.0,
                  8);
	    }
    }
    else
    {
      x = pos - width * pnor;
      rightStNo = dest->AddPoint (x);
      x = pos - width * nnor;
      rightEnNo = dest->AddPoint (x);
      dest->AddEdge (rightStNo, rightEnNo);
    }
  }
  else
  {
    NR::Point x;
    {
      NR::Point biss;
      biss = next - prev;
      double c2 = cross (next, biss);
      double l = width / c2;
      double projn = l * (dot (biss, next));
      double projp = -l * (dot (biss, prev));
      if (projp <= 0.5 * prevL && projn <= 0.5 * nextL)
      {
        x = pos + l * biss;
        rightEnNo = rightStNo = dest->AddPoint (x);
      }
      else
      {
        x = pos - width * pnor;
        rightStNo = dest->AddPoint (x);
        x = pos - width * nnor;
        rightEnNo = dest->AddPoint (x);
        dest->AddEdge (rightStNo, rightEnNo);
      }
    }
    if (join == join_pointy)
    {
      x = pos + width * pnor;
      leftStNo = dest->AddPoint (x);
      x = pos + width * nnor;
      leftEnNo = dest->AddPoint (x);
      //              dest->AddEdge(leftEnNo,leftStNo);
      
      NR::Point biss;
      biss = pnor + nnor;
      StrokeNormalize (biss);
      double c2 = dot (biss, nnor);
      double l = width / c2;
      double emiter = width * c2;
      if (emiter < miter)
        emiter = miter;
      int nleftStNo, nleftEnNo;
      if (l <= emiter)
	    {
	      x = pos + l * biss;
	      nleftStNo = nleftEnNo = dest->AddPoint (x);
	    }
      else
	    {
	      double s2 = cross (biss, nnor);
	      double dec = (l - emiter) * c2 / s2;
	      NR::Point tbiss=biss.ccw();
        
	      x = pos + emiter * biss + dec * tbiss;
	      nleftStNo = dest->AddPoint (x);
	      x = pos + emiter * biss - dec * tbiss;
	      nleftEnNo = dest->AddPoint (x);
	    }
      dest->AddEdge (leftEnNo, nleftEnNo);
      dest->AddEdge (nleftEnNo, nleftStNo);
      dest->AddEdge (nleftStNo, leftStNo);
    }
    else if (join == join_round)
    {
      NR::Point sx = pos + width * pnor;
      leftStNo = dest->AddPoint (sx);
      NR::Point ex = pos + width * nnor;
      leftEnNo = dest->AddPoint (ex);
      
      NR::Point biss;
      biss = pnor + nnor;
      StrokeNormalize (biss);
      double c2 = dot (biss, nnor);
      double l = width / c2;
      double typ = dot (pnor, nnor);
      if (typ >= 0)
	    {
	      x = pos + l * biss;
	      RecRound (dest, leftEnNo, leftStNo, x, ex, sx, 5.0,
                  8);
	    }
      else
	    {
	      double s2 = cross (biss, nnor);
	      double dec = (l - width) * c2 / s2;
	      NR::Point tbiss=biss.ccw();
        
        NR::Point mx = pos + width * biss;
	      int midNo = dest->AddPoint (mx);
        
        NR::Point nsx = pos + width * biss + dec * tbiss;
        NR::Point nex = pos + width * biss - dec * tbiss;
	      RecRound (dest, leftEnNo, midNo, nex, ex, mx, 5.0,
                  8);
	      RecRound (dest, midNo, leftStNo, nsx, mx, sx, 5.0,
                  8);
	    }
    }
    else
    {
      x = pos + width * pnor;
      leftStNo = dest->AddPoint (x);
      x = pos + width * nnor;
      leftEnNo = dest->AddPoint (x);
      dest->AddEdge (leftEnNo, leftStNo);
    }
  }
}

void
Path::DoLeftJoin (Shape * dest, double width, JoinType join, NR::Point pos,
                  NR::Point prev, NR::Point next, double miter, double prevL, double nextL,
                  int &leftStNo, int &leftEnNo,int pathID,int pieceID,double tID)
{
  NR::Point pnor, nnor;
  pnor=prev.ccw();
  nnor=next.ccw();
  double angSi = cross (next, prev);
  if (angSi > -0.0001 && angSi < 0.0001)
  {
    double angCo = dot (prev, next);
    if (angCo > 0.9999)
    {
      // tout droit
      NR::Point x;
      x = pos + width * pnor;
      leftEnNo = leftStNo = dest->AddPoint (x);
    }
    else
    {
      // demi-tour
      NR::Point x;
      x = pos + width * pnor;
      leftStNo = dest->AddPoint (x);
      x = pos - width * pnor;
      leftEnNo = dest->AddPoint (x);
      int nEdge=dest->AddEdge (leftEnNo, leftStNo);
      if ( dest->HasBackData() ) {
        dest->ebData[nEdge].pathID=pathID;
        dest->ebData[nEdge].pieceID=pieceID;
        dest->ebData[nEdge].tSt=dest->ebData[nEdge].tEn=tID;
      }
    }
    return;
  }
  if (angSi < 0)
  {
    /*		NR::Point     biss;
		biss.x=next.x-prev.x;
		biss.y=next.y-prev.y;
		double   c2=cross(biss,next);
		double   l=width/c2;
		double		projn=l*(dot(biss,next));
		double		projp=-l*(dot(biss,prev));
		if ( projp <= 0.5*prevL && projn <= 0.5*nextL ) {
			double   x,y;
			x=pos.x+l*biss.x;
			y=pos.y+l*biss.y;
			leftEnNo=leftStNo=dest->AddPoint(x,y);
		} else {*/
    NR::Point x;
    x = pos + width * pnor;
    leftStNo = dest->AddPoint (x);
    x = pos + width * nnor;
    leftEnNo = dest->AddPoint (x);
    x = pos;
    int midNo = dest->AddPoint (x);
    int nEdge=dest->AddEdge (leftEnNo, midNo);
    if ( dest->HasBackData() ) {
      dest->ebData[nEdge].pathID=pathID;
      dest->ebData[nEdge].pieceID=pieceID;
      dest->ebData[nEdge].tSt=dest->ebData[nEdge].tEn=tID;
    }
    nEdge=dest->AddEdge (midNo, leftStNo);
    if ( dest->HasBackData() ) {
      dest->ebData[nEdge].pathID=pathID;
      dest->ebData[nEdge].pieceID=pieceID;
      dest->ebData[nEdge].tSt=dest->ebData[nEdge].tEn=tID;
    }
    //              }
  }
  else
  {
    NR::Point x;
    if (join == join_pointy)
    {
      NR::Point sx = pos + width * pnor;
      leftStNo = dest->AddPoint (sx);
      NR::Point ex = pos + width * nnor;
      leftEnNo = dest->AddPoint (ex);
      
      NR::Point biss;
      biss = pnor + nnor;
      StrokeNormalize (biss);
      double c2 = dot (biss, nnor);
      double l = width / c2;
      double emiter = width * c2;
      if (emiter < miter)
        emiter = miter;
      if (l <= emiter)
	    {
	      x = pos + l * biss;
	      int nleftStNo = dest->AddPoint (x);
	      int nEdge=dest->AddEdge (leftEnNo, nleftStNo);
        if ( dest->HasBackData() ) {
          dest->ebData[nEdge].pathID=pathID;
          dest->ebData[nEdge].pieceID=pieceID;
          dest->ebData[nEdge].tSt=dest->ebData[nEdge].tEn=tID;
        }
	      nEdge=dest->AddEdge (nleftStNo, leftStNo);
        if ( dest->HasBackData() ) {
          dest->ebData[nEdge].pathID=pathID;
          dest->ebData[nEdge].pieceID=pieceID;
          dest->ebData[nEdge].tSt=dest->ebData[nEdge].tEn=tID;
        }
	    }
      else
	    {
	      double s2 = cross (biss, nnor);
	      double dec = (l - emiter) * c2 / s2;
	      NR::Point tbiss=biss.ccw();
        
	      x = pos + emiter * biss + dec * tbiss;
	      int nleftStNo = dest->AddPoint (x);
	      x = pos + emiter * biss - dec * tbiss;
	      int nleftEnNo = dest->AddPoint (x);
	      int nEdge=dest->AddEdge (nleftEnNo, nleftStNo);
        if ( dest->HasBackData() ) {
          dest->ebData[nEdge].pathID=pathID;
          dest->ebData[nEdge].pieceID=pieceID;
          dest->ebData[nEdge].tSt=dest->ebData[nEdge].tEn=tID;
        }
	      nEdge=dest->AddEdge (leftEnNo, nleftEnNo);
        if ( dest->HasBackData() ) {
          dest->ebData[nEdge].pathID=pathID;
          dest->ebData[nEdge].pieceID=pieceID;
          dest->ebData[nEdge].tSt=dest->ebData[nEdge].tEn=tID;
        }
	      nEdge=dest->AddEdge (nleftStNo, leftStNo);
        if ( dest->HasBackData() ) {
          dest->ebData[nEdge].pathID=pathID;
          dest->ebData[nEdge].pieceID=pieceID;
          dest->ebData[nEdge].tSt=dest->ebData[nEdge].tEn=tID;
        }
	    }
    }
    else if (join == join_round)
    {
      NR::Point sx = pos + width * pnor;
      leftStNo = dest->AddPoint (sx);
      NR::Point ex = pos + width * nnor;
      leftEnNo = dest->AddPoint (ex);
      
      NR::Point biss;
      biss = pnor + nnor;
      StrokeNormalize (biss);
      double c2 = dot (biss, nnor);
      double l = width / c2;
      double typ = dot (pnor, nnor);
      if (typ >= 0)
	    {
	      x = pos + l * biss;
	      RecRound (dest, leftEnNo, leftStNo, x, ex, sx, 5.0,
                  8);
	    }
      else
	    {
	      double s2 = cross (biss, nnor);
	      double dec = (l - width) * c2 / s2;
	      NR::Point tbiss=biss.ccw();
        
        NR::Point mx = pos + width * biss;
	      int midNo = dest->AddPoint (mx);
        
        NR::Point nsx = pos + width * biss + dec * tbiss;
        NR::Point nex = pos + width * biss - dec * tbiss;
	      RecRound (dest, leftEnNo, midNo, nex, ex, mx, 5.0,
                  8);
	      RecRound (dest, midNo, leftStNo, nsx, mx, sx, 5.0,
                  8);
	    }
    }
    else
    {
      x = pos + width * pnor;
      leftStNo = dest->AddPoint (x);
      x = pos + width * nnor;
      leftEnNo = dest->AddPoint (x);
      int nEdge=dest->AddEdge (leftEnNo, leftStNo);
      if ( dest->HasBackData() ) {
        dest->ebData[nEdge].pathID=pathID;
        dest->ebData[nEdge].pieceID=pieceID;
        dest->ebData[nEdge].tSt=dest->ebData[nEdge].tEn=tID;
      }
    }
  }
}
void
Path::DoRightJoin (Shape * dest, double width, JoinType join, NR::Point pos,
                   NR::Point prev, NR::Point next, double miter, double prevL,
                   double nextL, int &rightStNo, int &rightEnNo,int pathID,int pieceID,double tID)
{
  NR::Point pnor, nnor;
  pnor=prev.ccw();
  nnor=next.ccw();
  double angSi = cross (next,prev);
  if (angSi > -0.0001 && angSi < 0.0001)
  {
    double angCo = dot (prev, next);
    if (angCo > 0.9999)
    {
      // tout droit
      NR::Point x;
      x = pos - width * pnor;
      rightEnNo = rightStNo = dest->AddPoint (x);
    }
    else
    {
      // demi-tour
      NR::Point x;
      x = pos + width * pnor;
      rightEnNo = dest->AddPoint (x);
      x = pos - width * pnor;
      rightStNo = dest->AddPoint (x);
      int nEdge=dest->AddEdge (rightStNo, rightEnNo);
      if ( dest->HasBackData() ) {
        dest->ebData[nEdge].pathID=pathID;
        dest->ebData[nEdge].pieceID=pieceID;
        dest->ebData[nEdge].tSt=dest->ebData[nEdge].tEn=tID;
      }
    }
    return;
  }
  if (angSi < 0)
  {
    NR::Point x;
    
    if (join == join_pointy)
    {
      NR::Point sx = pos - width * pnor;
      rightStNo = dest->AddPoint (sx);
      NR::Point ex = pos - width * nnor;
      rightEnNo = dest->AddPoint (ex);
      
      NR::Point biss;
      biss = pnor + nnor;
      StrokeNormalize (biss);
      double c2 = dot (biss, nnor);
      double l = width / c2;
      double emiter = width * c2;
      if (emiter < miter)
        emiter = miter;
      if (l <= emiter)
	    {
	      x = pos - l * biss;
	      int nrightStNo = dest->AddPoint (x);
	      int nEdge=dest->AddEdge (rightStNo, nrightStNo);
        if ( dest->HasBackData() ) {
          dest->ebData[nEdge].pathID=pathID;
          dest->ebData[nEdge].pieceID=pieceID;
          dest->ebData[nEdge].tSt=dest->ebData[nEdge].tEn=tID;
        }
	      nEdge=dest->AddEdge (nrightStNo, rightEnNo);
        if ( dest->HasBackData() ) {
          dest->ebData[nEdge].pathID=pathID;
          dest->ebData[nEdge].pieceID=pieceID;
          dest->ebData[nEdge].tSt=dest->ebData[nEdge].tEn=tID;
        }
	    }
      else
	    {
	      double s2 = cross (biss, nnor);
	      double dec = (l - emiter) * c2 / s2;
	      NR::Point tbiss=biss.ccw();
        
	      x = pos - emiter * biss - dec * tbiss;
	      int nrightStNo = dest->AddPoint (x);
	      x = pos - emiter * biss + dec * tbiss;
	      int nrightEnNo = dest->AddPoint (x);
	      int nEdge=dest->AddEdge (rightStNo, nrightStNo);
        if ( dest->HasBackData() ) {
          dest->ebData[nEdge].pathID=pathID;
          dest->ebData[nEdge].pieceID=pieceID;
          dest->ebData[nEdge].tSt=dest->ebData[nEdge].tEn=tID;
        }
	      nEdge=dest->AddEdge (nrightStNo, nrightEnNo);
        if ( dest->HasBackData() ) {
          dest->ebData[nEdge].pathID=pathID;
          dest->ebData[nEdge].pieceID=pieceID;
          dest->ebData[nEdge].tSt=dest->ebData[nEdge].tEn=tID;
        }
	      nEdge=dest->AddEdge (nrightEnNo, rightEnNo);
        if ( dest->HasBackData() ) {
          dest->ebData[nEdge].pathID=pathID;
          dest->ebData[nEdge].pieceID=pieceID;
          dest->ebData[nEdge].tSt=dest->ebData[nEdge].tEn=tID;
        }
	    }
    }
    else if (join == join_round)
    {
      NR::Point sx = pos - width * pnor;
      rightStNo = dest->AddPoint (sx);
      NR::Point ex = pos - width * nnor;
      rightEnNo = dest->AddPoint (ex);
      
      NR::Point biss;
      biss = pnor + nnor;
      StrokeNormalize (biss);
      double c2 = dot (biss, nnor);
      double l = width / c2;
      double typ = dot (pnor, nnor);
      if (typ >= 0)
	    {
	      x = pos - l * biss;
	      RecRound (dest, rightStNo, rightEnNo, x, sx, ex, 5.0,
                  8);
	    }
      else
	    {
	      double s2 = cross (biss, nnor);
	      double dec = (l - width) * c2 / s2;
	      NR::Point tbiss=biss.ccw();
        
        NR::Point nsx = pos - width * biss - dec * tbiss;
	      NR::Point nex = pos - width * biss + dec * tbiss;
	      NR::Point mx = pos - width * biss;
	      int midNo = dest->AddPoint (mx);
	      RecRound (dest, rightStNo, midNo, nsx, sx, mx, 5.0,
                  8);
	      RecRound (dest, midNo, rightEnNo, nex, mx, ex, 5.0,
                  8);
	    }
    }
    else
    {
      x = pos - width * pnor;
      rightStNo = dest->AddPoint (x);
      x = pos - width * nnor;
      rightEnNo = dest->AddPoint (x);
      int nEdge=dest->AddEdge (rightStNo, rightEnNo);
      if ( dest->HasBackData() ) {
        dest->ebData[nEdge].pathID=pathID;
        dest->ebData[nEdge].pieceID=pieceID;
        dest->ebData[nEdge].tSt=dest->ebData[nEdge].tEn=tID;
      }
    }
  }
  else
  {
    /*		NR::Point     biss;
		biss=next.x-prev.x;
		biss.y=next.y-prev.y;
		double   c2=cross(next,biss);
		double   l=width/c2;
		double		projn=l*(dot(biss,next));
		double		projp=-l*(dot(biss,prev));
		if ( projp <= 0.5*prevL && projn <= 0.5*nextL ) {
			double   x,y;
			x=pos.x+l*biss.x;
			y=pos.y+l*biss.y;
			rightEnNo=rightStNo=dest->AddPoint(x,y);
		} else {*/
    NR::Point x;
    x = pos - width * pnor;
    rightStNo = dest->AddPoint (x);
    x = pos - width * nnor;
    rightEnNo = dest->AddPoint (x);
    x = pos;
    int midNo = dest->AddPoint (x);
    int nEdge=dest->AddEdge (rightStNo, midNo);
    if ( dest->HasBackData() ) {
      dest->ebData[nEdge].pathID=pathID;
      dest->ebData[nEdge].pieceID=pieceID;
      dest->ebData[nEdge].tSt=dest->ebData[nEdge].tEn=tID;
    }
    nEdge=dest->AddEdge (midNo, rightEnNo);
    if ( dest->HasBackData() ) {
      dest->ebData[nEdge].pathID=pathID;
      dest->ebData[nEdge].pieceID=pieceID;
      dest->ebData[nEdge].tSt=dest->ebData[nEdge].tEn=tID;
    }
    //              }
  }
}


void
Path::RecRound (Shape * dest, int sNo, int eNo, NR::Point &iP, NR::Point &iS,NR::Point &iE, double tresh, int lev)
{
  if (lev <= 0)
  {
    dest->AddEdge (sNo, eNo);
    return;
  }
  NR::Point ps=iS-iP,pe=iE-iP;
  double s = cross(pe,ps);
  if (s < 0)
    s = -s;
  if (s < tresh)
  {
    dest->AddEdge (sNo, eNo);
    return;
  }
  
  NR::Point m, md;
  m = (iS + iE + 2 * iP) / 4;
  int mNo = dest->AddPoint (m);
  
  md = (iS + iP) / 2;
   RecRound (dest, sNo, mNo, md,  iS,m, tresh, lev - 1);
   md = (iE + iP) / 2;
  RecRound (dest, mNo, eNo, md, m,iE, tresh, lev - 1);
}

/*
 *
 * dashed version
 *
 */

void
Path::Stroke (Shape * dest, bool doClose, double width, JoinType join,
              ButtType butt, double miter, int nbDash, one_dash * dashs,
              bool justAdd)
{
  if (nbDash <= 0)
  {
    Stroke (dest, doClose, width, join, butt, miter, justAdd);
    return;
  }
  
  if (dest == NULL)
    return;
  if (justAdd == false)
  {
    dest->Reset (3 * nbPt, 3 * nbPt);
  }
  if (nbPt <= 1)
    return;
  dest->MakeBackData (false);
  
  char *savPts = pts;
  int savNbPt = nbPt;
  
  int lastM = 0;
  while (lastM < savNbPt)
  {
    int lastP = lastM + 1;
    if (back)
    {
      if (weighted)
	    {
	      path_lineto_wb *tp = (path_lineto_wb *) savPts;
	      while (lastP < savNbPt
               && ((tp + lastP)->isMoveTo == polyline_lineto
                   || (tp + lastP)->isMoveTo == polyline_forced))
          lastP++;
	      pts = (char *) (tp + lastM);
	      nbPt = lastP - lastM;
	    }
      else
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
      if (weighted)
	    {
	      path_lineto_w *tp = (path_lineto_w *) savPts;
	      while (lastP < savNbPt
               && ((tp + lastP)->isMoveTo == polyline_lineto
                   || (tp + lastP)->isMoveTo == polyline_forced))
          lastP++;
	      pts = (char *) (tp + lastM);
	      nbPt = lastP - lastM;
	    }
      else
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
    DoStroke (dest, doClose, width, join, butt, miter, nbDash, dashs, true);
    lastM = lastP;
  }
  
  pts = savPts;
  nbPt = savNbPt;
}

void
Path::DoStroke (Shape * dest, bool doClose, double width, JoinType join,
                ButtType butt, double miter, int nbDash, one_dash * dashs,
                bool justAdd)
{
  
  if (dest == NULL)
    return;
  if (nbPt <= 1)
    return;
  
  NR::Point curP, prevP, nextP;
  double curW, prevW, nextW;
  int curI, prevI, nextI;
  int upTo;
  double curA, prevA, nextA;
  double dashPos = 0, dashAbs = 0;
  int dashNo = 0;
  
  curI = 0;
  curP = ((path_lineto *) pts)[0].p;
  if (weighted)
    curW = ((path_lineto_w *) pts)[0].w;
  else
    curW = 1;
  
  if (doClose)
  {
    path_lineto *curPt = (path_lineto *) pts;
    prevI = nbPt - 1;
    if (back)
    {
      if (weighted)
        curPt =
          (path_lineto *) (((char *) curPt) +
                           prevI * sizeof (path_lineto_wb));
      else
        curPt =
          (path_lineto *) (((char *) curPt) +
                           prevI * sizeof (path_lineto_b));
    }
    else
    {
      if (weighted)
        curPt =
          (path_lineto *) (((char *) curPt) +
                           prevI * sizeof (path_lineto_w));
      else
        curPt =
          (path_lineto *) (((char *) curPt) +
                           prevI * sizeof (path_lineto));
    }
    while (prevI > 0)
    {
      prevP = curPt->p;
      if (weighted)
        prevW = ((path_lineto_w *) curPt)->w;
      else
        prevW = 1;
      NR::Point diff=curP-prevP;
      double dist =dot(diff,diff);
      if (dist > 0.001)
	    {
	      break;
	    }
      prevI--;
      if (back)
	    {
	      if (weighted)
          curPt =
            (path_lineto *) (((char *) curPt) -
                             sizeof (path_lineto_wb));
	      else
          curPt =
            (path_lineto *) (((char *) curPt) - sizeof (path_lineto_b));
	    }
      else
	    {
	      if (weighted)
          curPt =
            (path_lineto *) (((char *) curPt) - sizeof (path_lineto_w));
	      else
          curPt =
            (path_lineto *) (((char *) curPt) - sizeof (path_lineto));
	    }
    }
    if (prevI <= 0)
      return;
    upTo = prevI;
  }
  else
  {
    prevP = curP;
    prevW = curW;
    prevI = curI;
    upTo = nbPt - 1;
  }
  {
    path_lineto *curPt = (path_lineto *) pts;
    nextI = 1;
    if (back)
    {
      if (weighted)
        curPt =
          (path_lineto *) (((char *) curPt) +
                           nextI * sizeof (path_lineto_wb));
      else
        curPt =
          (path_lineto *) (((char *) curPt) +
                           nextI * sizeof (path_lineto_b));
    }
    else
    {
      if (weighted)
        curPt =
          (path_lineto *) (((char *) curPt) +
                           nextI * sizeof (path_lineto_w));
      else
        curPt =
          (path_lineto *) (((char *) curPt) + nextI * sizeof (path_lineto));
    }
    while (nextI <= upTo)
    {
      nextP = curPt->p;
      if (weighted)
        nextW = ((path_lineto_w *) curPt)->w;
      else
        nextW = 1;
      NR::Point diff=curP-nextP;
      double dist =dot(diff,diff);
      if (dist > 0.001)
      {
        break;
      }
      nextI++;
      if (back)
      {
        if (weighted)
          curPt =
            (path_lineto *) (((char *) curPt) + sizeof (path_lineto_wb));
        else
          curPt =
            (path_lineto *) (((char *) curPt) + sizeof (path_lineto_b));
      }
      else
      {
        if (weighted)
          curPt =
            (path_lineto *) (((char *) curPt) + sizeof (path_lineto_w));
        else
          curPt =
            (path_lineto *) (((char *) curPt) + sizeof (path_lineto));
      }
    }
    if (nextI > upTo)
      return;
  }
  
  int startLeft = -1, startRight = -1;
  int lastLeft = -1, lastRight = -1;
  bool noStartJoin = false, inGap = true;
  NR::Point prevD, nextD;
  double prevLe, nextLe;
  prevD = curP - prevP;
  nextD = nextP - curP;
  curA = 0;
  prevA = -sqrt (dot(prevD,prevD));
  nextA = sqrt (dot(nextD,nextD));
  prevLe = sqrt (dot(prevD,prevD));
  nextLe = sqrt (dot(nextD,nextD));
  StrokeNormalize (prevD);
  StrokeNormalize (nextD);
  dashTo_info dTo;
  
  {
    int cDashNo = dashNo;
    double nDashAbs = 0;
    do
    {
      if (dashAbs + (dashs[dashNo].length - dashPos) <= nDashAbs)
      {
        dashNo++;
        if (dashNo >= nbDash)
          dashNo -= nbDash;
        dashPos = 0;
      }
      else
      {
        break;
      }
    }
    while (dashNo != cDashNo);
  }
  if (doClose)
  {
    if (dashs[dashNo].gap)
    {
      noStartJoin = true;
      inGap = true;
    }
    else
    {
      noStartJoin = false;
      DoJoin (dest, curW * width, join, curP, prevD, nextD, miter, prevLe,
              nextLe, startLeft, lastLeft, startRight, lastRight);
      inGap = false;
    }
  }
  else
  {
    if (dashs[dashNo].gap)
    {
      inGap = true;
    }
    else
    {
      nextD = -nextD;
      DoButt (dest, curW * width, butt, curP, nextD, lastRight, lastLeft);
      nextD = -nextD;
      inGap = false;
    }
  }
  do
  {
    prevP = curP;
    prevI = curI;
    prevW = curW;
    prevA = curA;
    curP = nextP;
    curI = nextI;
    curW = nextW;
    curA = nextA;
    prevLe = nextLe;
    prevD = nextD;
    nextI++;
    path_lineto *curPt = (path_lineto *) pts;
    if (back)
    {
      if (weighted)
        curPt =
          (path_lineto *) (((char *) curPt) +
                           nextI * sizeof (path_lineto_wb));
      else
        curPt =
          (path_lineto *) (((char *) curPt) +
                           nextI * sizeof (path_lineto_b));
    }
    else
    {
      if (weighted)
        curPt =
          (path_lineto *) (((char *) curPt) +
                           nextI * sizeof (path_lineto_w));
      else
        curPt =
          (path_lineto *) (((char *) curPt) +
                           nextI * sizeof (path_lineto));
    }
    while (nextI <= upTo)
    {
      nextP = curPt->p;
      if (weighted)
        nextW = ((path_lineto_w *) curPt)->w;
      else
        nextW = 1;
      NR::Point   diff=curP-nextP;
      double dist =dot(diff,diff);
      if (dist > 0.001)
	    {
	      break;
	    }
      nextI++;
      if (back)
	    {
	      if (weighted)
          curPt =
            (path_lineto *) (((char *) curPt) +
                             sizeof (path_lineto_wb));
	      else
          curPt =
            (path_lineto *) (((char *) curPt) + sizeof (path_lineto_b));
	    }
      else
	    {
	      if (weighted)
          curPt =
            (path_lineto *) (((char *) curPt) + sizeof (path_lineto_w));
	      else
          curPt =
            (path_lineto *) (((char *) curPt) + sizeof (path_lineto));
	    }
    }
    if (nextI > upTo)
      break;
    
    nextD= nextP - curP;
    nextA = curA + sqrt (dot(nextD,nextD));
    nextLe = sqrt (dot(nextD,nextD));
    StrokeNormalize (nextD);
    
    dTo.nDashAbs = curA;
    dTo.prevP = prevP;
    dTo.curP = curP;
    dTo.prevD = prevD;
    dTo.prevW = prevW * width;
    dTo.curW = curW * width;
    
    DashTo (dest, &dTo, dashAbs, dashNo, dashPos, inGap, lastLeft,
            lastRight, nbDash, dashs);
    
    if (inGap == false)
    {
      int nStL = -1, nStR = -1, nEnL = -1, nEnR = -1;
      DoJoin (dest, curW * width, join, curP, prevD, nextD, miter, prevLe,
              nextLe, nStL, nEnL, nStR, nEnR);
      dest->AddEdge (nStL, lastLeft);
      lastLeft = nEnL;
      dest->AddEdge (lastRight, nStR);
      lastRight = nEnR;
    }
  }
  while (nextI <= upTo);
  if (doClose)
  {
    /*		prevP=curP;
		prevI=curI;
		prevW=curW;
		prevA=curA;
		curP=nextP;
		curI=nextI;
		curW=nextW;
		curA=nextA;
		prevD=nextD;*/
    path_lineto *curPt = (path_lineto *) pts;
    nextP = curPt->p;
    if (weighted)
      nextW = ((path_lineto_w *) curPt)->w;
    else
      nextW = 1;
    
    nextD = nextP - curP;
    nextA = curA + sqrt (dot(nextD,nextD));
    nextLe = sqrt (dot(nextD,nextD));
    StrokeNormalize (nextD);
    
    dTo.nDashAbs = curA;
    dTo.prevP = prevP;
    dTo.curP = curP;
    dTo.prevD = prevD;
    dTo.prevW = prevW * width;
    dTo.curW = curW * width;
    
    DashTo (dest, &dTo, dashAbs, dashNo, dashPos, inGap, lastLeft,
            lastRight, nbDash, dashs);
    if (inGap == false)
    {
      int nStL = -1, nStR = -1, nEnL = -1, nEnR = -1;
      DoJoin (dest, curW * width, join, curP, prevD, nextD, miter, prevLe,
              nextLe, nStL, nEnL, nStR, nEnR);
      dest->AddEdge (nStL, lastLeft);
      lastLeft = nEnL;
      dest->AddEdge (lastRight, nStR);
      lastRight = nEnR;
    }
    dTo.nDashAbs = nextA;
    dTo.prevP = curP;
    dTo.curP = nextP;
    dTo.prevD = nextD;
    dTo.prevW = curW * width;
    dTo.curW = nextW * width;
    
    DashTo (dest, &dTo, dashAbs, dashNo, dashPos, inGap, lastLeft,
            lastRight, nbDash, dashs);
    
    if (inGap == false)
    {
      if (noStartJoin == false)
	    {
	      dest->AddEdge (startLeft, lastLeft);
	      dest->AddEdge (lastRight, startRight);
	    }
      else
	    {
	      dest->AddEdge (lastRight, lastLeft);
	    }
    }
    else
    {
      if (noStartJoin == false)
	    {
	      dest->AddEdge (startLeft, startRight);
	    }
    }
  }
  else
  {
    dTo.nDashAbs = curA;
    dTo.prevP = prevP;
    dTo.curP = curP;
    dTo.prevD = prevD;
    dTo.prevW = prevW * width;
    dTo.curW = curW * width;
    
    DashTo (dest, &dTo, dashAbs, dashNo, dashPos, inGap, lastLeft,
            lastRight, nbDash, dashs);
    if (inGap == false)
    {
      int endRight, endLeft;
      DoButt (dest, curW * width, butt, curP, prevD, endLeft, endRight);
      dest->AddEdge (endLeft, lastLeft);
      dest->AddEdge (lastRight, endRight);
    }
  }
  
}

void
Path::DashTo (Shape * dest, dashTo_info * dTo, double &dashAbs, int &dashNo,
              double &dashPos, bool & inGap, int &lastLeft, int &lastRight,
              int nbDash, one_dash * dashs)
{
  //      printf("%f %i %f %i -> %f\n",dashAbs,dashNo,dashPos,(inGap)?1:0,dTo->nDashAbs);
  NR::Point pnor=dTo->prevD.ccw();
  
  double oDashAbs = dashAbs;
  while (dashAbs < dTo->nDashAbs)
  {
    int cDashNo = dashNo;
    do
    {
      double delta = dashs[dashNo].length - dashPos;
      if (delta <= dTo->nDashAbs - dashAbs)
	    {
	      dashNo++;
	      dashPos = 0;
	      if (dashNo >= nbDash)
          dashNo -= nbDash;
	      while (dashNo != cDashNo && dashs[dashNo].length <= 0)
        {
          dashNo++;
          dashPos = 0;
          if (dashNo >= nbDash)
            dashNo -= nbDash;
        }
	      if (dashs[dashNo].length > 0)
        {
          NR::Point pos;
          double nw;
          dashAbs += delta;
          pos =
            ((dTo->nDashAbs - dashAbs) * dTo->prevP+
             (dashAbs - oDashAbs)*dTo->curP) / (dTo->nDashAbs -
                                                oDashAbs);
          nw =
            (dTo->prevW * (dTo->nDashAbs - dashAbs) +
             dTo->curW * (dashAbs - oDashAbs)) / (dTo->nDashAbs -
                                                  oDashAbs);
          
          if (inGap && dashs[dashNo].gap == false)
          {
            NR::Point x;
            x = pos + nw * pnor;
            int nleftNo = dest->AddPoint (x);
            x = pos - nw * pnor;
            int nrightNo = dest->AddPoint (x);
            dest->AddEdge (nleftNo, nrightNo);
            lastLeft = nleftNo;
            lastRight = nrightNo;
            
            inGap = false;
          }
          else if (inGap == false && dashs[dashNo].gap)
          {
            NR::Point x;
            x = pos + nw * pnor;
            int nleftNo = dest->AddPoint (x);
            x = pos - nw * pnor;
            int nrightNo = dest->AddPoint (x);
            dest->AddEdge (nrightNo, nleftNo);
            dest->AddEdge (lastRight, nrightNo);
            dest->AddEdge (nleftNo, lastLeft);
            lastLeft = -1;
            lastRight = -1;
            
            inGap = true;
          }
          else
          {
            
          }
        }
	    }
      else
	    {
	      dashPos += dTo->nDashAbs - dashAbs;
	      dashAbs = dTo->nDashAbs;
	      break;
	    }
    }
    while (dashNo != cDashNo);
  }
  inGap = dashs[dashNo].gap;
}


#undef StrokeNormalize
