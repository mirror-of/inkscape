/*
 *  PathOutline.cpp
 *  nlivarot
 *
 *  Created by fred on Fri Nov 28 2003.
 *
 */

#include "Path.h"
//#include "MyMath.h"
#include <math.h>

void
Path::Outline (Path * dest, float width, JoinType join, ButtType butt,
               float miter)
{
  if (descr_flags & descr_adding_bezier)
    CancelBezier ();
  if (descr_flags & descr_doing_subpath)
    CloseSubpath (0);
  if (descr_nb <= 2)
    return;
  if (dest == NULL)
    return;
  dest->Reset ();
  dest->SetWeighted (false);
  dest->SetBackData (false);
  
  outline_callbacks calls;
  NR::Point endButt, endPos;
  calls.cubicto = StdCubicTo;
  calls.bezierto = StdBezierTo;
  calls.arcto = StdArcTo;
  
  path_descr *sav_descr = descr_data;
  int sav_descr_nb = descr_nb;
  
  Path *rev = new Path;
  
  rev->SetWeighted (false);
  int curP = 0;
  do
  {
    int lastM = curP;
    do
    {
      curP++;
      if (curP >= sav_descr_nb)
        break;
      int typ = sav_descr[curP].flags & descr_type_mask;
      if (typ == descr_moveto)
        break;
    }
    while (curP < sav_descr_nb);
    if (curP >= sav_descr_nb)
      curP = sav_descr_nb;
    if (curP > lastM + 1)
    {
      // sinon il n'y a qu'un point
      int curD = curP - 1;
      NR::Point curX;
      NR::Point nextX;
      bool needClose = false;
      int firstTyp = sav_descr[curD].flags & descr_type_mask;
      if (firstTyp == descr_close)
        needClose = true;
      while (curD > lastM
             && (sav_descr[curD].flags & descr_type_mask) == descr_close)
        curD--;
      int realP = curD + 1;
      if (curD > lastM)
	    {
	      descr_data = sav_descr;
	      descr_nb = sav_descr_nb;
	      PrevPoint (curD, curX);
	      rev->Reset ();
	      rev->MoveTo (curX);
	      while (curD > lastM)
        {
          int typ = sav_descr[curD].flags & descr_type_mask;
          if (typ == descr_moveto)
          {
            //                                              rev->Close();
            curD--;
          }
          else if (typ == descr_forced)
          {
            //                                              rev->Close();
            curD--;
          }
          else if (typ == descr_lineto)
          {
            PrevPoint (curD - 1, nextX);
            rev->LineTo (nextX);
            curX = nextX;
            curD--;
          }
          else if (typ == descr_cubicto)
          {
            PrevPoint (curD - 1, nextX);
            NR::Point  isD=-sav_descr[curD].d.c.stD;
            NR::Point  ieD=-sav_descr[curD].d.c.enD;
            rev->CubicTo (nextX, ieD,isD);
            curX = nextX;
            curD--;
          }
          else if (typ == descr_arcto)
          {
            PrevPoint (curD - 1, nextX);
            rev->ArcTo (nextX, sav_descr[curD].d.a.rx,
                        sav_descr[curD].d.a.ry,
                        sav_descr[curD].d.a.angle,
                        sav_descr[curD].d.a.large,
                        !sav_descr[curD].d.a.clockwise);
            curX = nextX;
            curD--;
          }
          else if (typ == descr_bezierto)
          {
            PrevPoint (curD - 1, nextX);
            rev->LineTo (nextX);
            curX = nextX;
            curD--;
          }
          else if (typ == descr_interm_bezier)
          {
            int nD = curD - 1;
            while (nD > lastM
                   && (sav_descr[nD].flags & descr_type_mask) !=
                   descr_bezierto)
              nD--;
            if ((sav_descr[nD].flags & descr_type_mask) !=
                descr_bezierto)
            {
              // pas trouve le debut!?
              PrevPoint (nD, nextX);
              rev->LineTo (nextX);
              curX = nextX;
            }
            else
            {
              PrevPoint (nD - 1, nextX);
              rev->BezierTo (nextX);
              for (int i = curD; i > nD; i--)
                rev->IntermBezierTo (sav_descr[i].d.i.p);
              rev->EndBezierTo ();
              curX = nextX;
            }
            curD = nD - 1;
          }
          else
          {
            curD--;
          }
        }
	      if (needClose)
        {
          rev->Close ();
          rev->SubContractOutline (dest, calls,
                                   0.0025 * width * width, width,
                                   join, butt, miter, true, false,
                                   endPos, endButt);
          descr_data = sav_descr + lastM;
          descr_nb = realP + 1 - lastM;
          SubContractOutline (dest, calls, 0.0025 * width * width,
                              width, join, butt, miter, true, false,
                              endPos, endButt);
        }
	      else
        {
          rev->SubContractOutline (dest, calls,
                                   0.0025 * width * width, width,
                                   join, butt, miter, false, false,
                                   endPos, endButt);
          NR::Point endNor=endButt.ccw();
          if (butt == butt_round)
          {
            NR::Point tmp=endPos+width*endNor;
            dest->ArcTo (tmp,
                         1.0001 * width, 1.0001 * width, 0.0, true,
                         true);
          }
          else if (butt == butt_square)
          {
            NR::Point tmp=endPos-width*endNor+width*endButt;
            dest->LineTo (tmp);
            tmp=endPos+width*endNor+width*endButt;
            dest->LineTo (tmp);
            tmp=endPos+width*endNor;
            dest->LineTo (tmp);
          }
          else if (butt == butt_pointy)
          {
            NR::Point tmp=endPos+width*endButt;
            dest->LineTo (tmp);
            tmp=endPos+width*endNor;
            dest->LineTo (tmp);
          }
          else
          {
            NR::Point tmp=endPos+width*endNor;
            dest->LineTo (tmp);
          }
          descr_data = sav_descr + lastM;
          descr_nb = realP - lastM;
          SubContractOutline (dest, calls, 0.0025 * width * width,
                              width, join, butt, miter, false, true,
                              endPos, endButt);
          endNor=endButt.ccw();
          if (butt == butt_round)
          {
            NR::Point tmp=endPos+width*endNor;
            dest->ArcTo (tmp,
                         1.0001 * width, 1.0001 * width, 0.0, true,
                         true);
          }
          else if (butt == butt_square)
          {
            NR::Point tmp=endPos-width*endNor+width*endButt;
            dest->LineTo (tmp);
            tmp=endPos+width*endNor+width*endButt;
            dest->LineTo (tmp);
            tmp=endPos+width*endNor;
            dest->LineTo (tmp);
          }
          else if (butt == butt_pointy)
          {
            NR::Point tmp=endPos+width*endButt;
            dest->LineTo (tmp);
            tmp=endPos+width*endNor;
            dest->LineTo (tmp);
          }
          else
          {
            NR::Point tmp=endPos+width*endNor;
            dest->LineTo (tmp);
          }
          dest->Close ();
        }
	    }
    }
  }
  while (curP < sav_descr_nb);
  
  delete rev;
  descr_data = sav_descr;
  descr_nb = sav_descr_nb;
  
}

void
Path::OutsideOutline (Path * dest, float width, JoinType join, ButtType butt,
                      float miter)
{
  if (descr_flags & descr_adding_bezier)
    CancelBezier ();
  if (descr_flags & descr_doing_subpath)
    CloseSubpath (0);
  if (descr_nb <= 2)
    return;
  if (dest == NULL)
    return;
  dest->Reset ();
  dest->SetWeighted (false);
  dest->SetBackData (false);
  
  outline_callbacks calls;
  NR::Point endButt, endPos;
  calls.cubicto = StdCubicTo;
  calls.bezierto = StdBezierTo;
  calls.arcto = StdArcTo;
  SubContractOutline (dest, calls, 0.0025 * width * width, width, join, butt,
                      miter, true, false, endPos, endButt);
}

void
Path::InsideOutline (Path * dest, float width, JoinType join, ButtType butt,
                     float miter)
{
  if (descr_flags & descr_adding_bezier)
    CancelBezier ();
  if (descr_flags & descr_doing_subpath)
    CloseSubpath (0);
  if (descr_nb <= 2)
    return;
  if (dest == NULL)
    return;
  dest->Reset ();
  dest->SetWeighted (false);
  dest->SetBackData (false);
  
  outline_callbacks calls;
  NR::Point endButt, endPos;
  calls.cubicto = StdCubicTo;
  calls.bezierto = StdBezierTo;
  calls.arcto = StdArcTo;
  
  path_descr *sav_descr = descr_data;
  int sav_descr_nb = descr_nb;
  
  Path *rev = new Path;
  
  rev->SetWeighted (false);
  int curP = 0;
  do
  {
    int lastM = curP;
    do
    {
      curP++;
      if (curP >= sav_descr_nb)
        break;
      int typ = sav_descr[curP].flags & descr_type_mask;
      if (typ == descr_moveto)
        break;
    }
    while (curP < sav_descr_nb);
    if (curP >= sav_descr_nb)
      curP = sav_descr_nb;
    if (curP > lastM + 1)
    {
      // sinon il n'y a qu'un point
      int curD = curP - 1;
      NR::Point curX;
      NR::Point nextX;
      while (curD > lastM
             && (sav_descr[curD].flags & descr_type_mask) == descr_close)
        curD--;
      if (curD > lastM)
	    {
	      descr_data = sav_descr;
	      descr_nb = sav_descr_nb;
	      PrevPoint (curD, curX);
	      rev->Reset ();
	      rev->MoveTo (curX);
	      while (curD > lastM)
        {
          int typ = sav_descr[curD].flags & descr_type_mask;
          if (typ == descr_moveto)
          {
            rev->Close ();
            curD--;
          }
          else if (typ == descr_forced)
          {
            curD--;
          }
          else if (typ == descr_lineto)
          {
            PrevPoint (curD - 1, nextX);
            rev->LineTo (nextX);
            curX = nextX;
            curD--;
          }
          else if (typ == descr_cubicto)
          {
            PrevPoint (curD - 1, nextX);
            NR::Point  isD=-sav_descr[curD].d.c.stD;
            NR::Point  ieD=-sav_descr[curD].d.c.enD;
            rev->CubicTo (nextX, ieD,isD);
            curX = nextX;
            curD--;
          }
          else if (typ == descr_arcto)
          {
            PrevPoint (curD - 1, nextX);
            rev->ArcTo (nextX, sav_descr[curD].d.a.rx,
                        sav_descr[curD].d.a.ry,
                        sav_descr[curD].d.a.angle,
                        sav_descr[curD].d.a.large,
                        !sav_descr[curD].d.a.clockwise);
            curX = nextX;
            curD--;
          }
          else if (typ == descr_bezierto)
          {
            PrevPoint (curD - 1, nextX);
            rev->LineTo (nextX);
            curX = nextX;
            curD--;
          }
          else if (typ == descr_interm_bezier)
          {
            int nD = curD - 1;
            while (nD > lastM
                   && (sav_descr[nD].flags & descr_type_mask) !=
                   descr_bezierto)
              nD--;
            if (sav_descr[nD].flags & descr_type_mask !=
                descr_bezierto)
            {
              // pas trouve le debut!?
              PrevPoint (nD, nextX);
              rev->LineTo (nextX);
              curX = nextX;
            }
            else
            {
              PrevPoint (nD - 1, nextX);
              rev->BezierTo (nextX);
              for (int i = curD; i > nD; i--)
                rev->IntermBezierTo (sav_descr[i].d.i.p);
              rev->EndBezierTo ();
              curX = nextX;
            }
            curD = nD - 1;
          }
          else
          {
            curD--;
          }
        }
	      rev->Close ();
	      rev->SubContractOutline (dest, calls, 0.0025 * width * width,
                                 width, join, butt, miter, true, false,
                                 endPos, endButt);
	    }
    }
  }
  while (curP < sav_descr_nb);
  
  delete rev;
  descr_data = sav_descr;
  descr_nb = sav_descr_nb;
}

void
Path::DoOutsideOutline (Path * dest, float width, JoinType join,
                        ButtType butt, float miter, int &stNo, int &enNo)
{
}
void
Path::DoInsideOutline (Path * dest, float width, JoinType join, ButtType butt,
                       float miter, int &stNo, int &enNo)
{
}
void
Path::SubContractOutline (Path * dest, outline_callbacks & calls,
                          float tolerance, float width, JoinType join,
                          ButtType butt, float miter, bool closeIfNeeded,
                          bool skipMoveto, NR::Point & lastP, NR::Point & lastT)
{
  NR::Point curX;
  float curW;
  int curP = 1;
  NR::Point curT;
  NR::Point firstP, firstT;
  bool doFirst;
  outline_callback_data callsData;
  
  callsData.orig = this;
  callsData.dest = dest;
  
  // le moveto
  curX = (descr_data)->d.m.p;
  if ((descr_data)->flags & descr_weighted)
  {
    curW = (descr_data)->d.m.w;
  }
  else
  {
    curW = 1;
  }
  curT.pt[0] = curT.pt[1] = 0;
  
  doFirst = true;
  firstP.pt[0] = firstP.pt[1] = 0;
  firstT.pt[0] = firstT.pt[1] = 0;
  
  // et le reste, 1 par 1
  while (curP < descr_nb)
  {
    path_descr *curD = descr_data + curP;
    int nType = curD->flags & descr_type_mask;
    bool nWeight = curD->flags & descr_weighted;
    NR::Point nextX;
    float nextW;
    NR::Point stPos, enPos, stTgt, enTgt, stNor, enNor;
    float stRad, enRad, stTle, enTle;
    if (nType == descr_forced)
    {
      curP++;
    }
    else if (nType == descr_moveto)
    {
      nextX = curD->d.m.p;
      if (nWeight)
        nextW = curD->d.m.w;
      else
        nextW = 1;
      // et on avance
      if (doFirst)
	    {
	    }
      else
	    {
	      if (closeIfNeeded)
        {
          if (fabsf (curX.pt[0] - firstP.pt[0]) < 0.0001
              && fabsf (curX.pt[1] - firstP.pt[1]) < 0.0001)
          {
            OutlineJoin (dest, firstP, curT, firstT, width, join,
                         miter);
            dest->Close ();
          }
          else
          {
            path_descr_lineto temp;
            temp.p = firstP;
            
            TangentOnSegAt (0.0, curX, temp, stPos, stTgt,
                            stTle);
            TangentOnSegAt (1.0, curX, temp, enPos, enTgt,
                            enTle);
            stNor=stTgt.cw();
            enNor=enTgt.cw();
            
            // jointure
            {
              NR::Point pos;
              pos = curX;
              OutlineJoin (dest, pos, curT, stNor, width, join,
                           miter);
            }
            NR::Point  tmp=enPos+width*enNor;
            dest->LineTo (tmp);
            
            // jointure
            {
              NR::Point pos;
              pos = firstP;
              OutlineJoin (dest, enPos, enNor, firstT, width, join,
                           miter);
              dest->Close ();
            }
          }
        }
	    }
      firstP = nextX;
      curP++;
    }
    else if (nType == descr_close)
    {
      if (doFirst == false)
	    {
	      if (fabsf (curX.pt[0] - firstP.pt[0]) < 0.0001
            && fabsf (curX.pt[1] - firstP.pt[1]) < 0.0001)
        {
          OutlineJoin (dest, firstP, curT, firstT, width, join,
                       miter);
          dest->Close ();
        }
	      else
        {
          path_descr_lineto temp;
          temp.p = firstP;
          nextX = firstP;
          
          TangentOnSegAt (0.0, curX, temp, stPos, stTgt, stTle);
          TangentOnSegAt (1.0, curX, temp, enPos, enTgt, enTle);
          stNor=stTgt.cw();
          enNor=enTgt.cw();
          
          // jointure
          {
            OutlineJoin (dest, stPos, curT, stNor, width, join,
                         miter);
          }
          
          NR::Point tmp=enPos+width*enNor;
          dest->LineTo (tmp);
          
          // jointure
          {
            OutlineJoin (dest, enPos, enNor, firstT, width, join,
                         miter);
            dest->Close ();
          }
        }
	    }
      doFirst = true;
      curP++;
    }
    else if (nType == descr_lineto)
    {
      nextX = curD->d.l.p;
      if (nWeight)
        nextW = curD->d.l.w;
      else
        nextW = 1;
      // test de nullité du segment
      if (IsNulCurve (curD, curX))
	    {
	      curP++;
	      continue;
	    }
      // et on avance
      TangentOnSegAt (0.0, curX, curD->d.l, stPos, stTgt, stTle);
      TangentOnSegAt (1.0, curX, curD->d.l, enPos, enTgt, enTle);
      stNor=stTgt.cw();
      enNor=enTgt.cw();
      
      lastP = enPos;
      lastT = enTgt;
      
      if (doFirst)
	    {
	      doFirst = false;
	      firstP = stPos;
	      firstT = stNor;
        NR::Point  tmp=curX+width*stNor;
	      if (skipMoveto)
        {
          skipMoveto = false;
        }
	      else
          dest->MoveTo (tmp);
	    }
      else
	    {
	      // jointure
	      NR::Point pos;
	      pos = curX;
	      OutlineJoin (dest, pos, curT, stNor, width, join, miter);
	    }
      
      NR::Point  tmp=nextX+width*enNor;
      int n_d =
        dest->LineTo (tmp);
      if (n_d >= 0)
	    {
	      dest->descr_data[n_d].associated = curP;
	      dest->descr_data[n_d].tSt = 0.0;
	      dest->descr_data[n_d].tEn = 1.0;
	    }
      curP++;
    }
    else if (nType == descr_cubicto)
    {
      nextX = curD->d.c.p;
      if (nWeight)
        nextW = curD->d.c.w;
      else
        nextW = 1;
      // test de nullité du segment
      if (IsNulCurve (curD, curX))
	    {
	      curP++;
	      continue;
	    }
      // et on avance
      TangentOnCubAt (0.0, curX, curD->d.c, false, stPos, stTgt,
                      stTle, stRad);
      TangentOnCubAt (1.0, curX, curD->d.c, true, enPos, enTgt,
                      enTle, enRad);
      stNor=stTgt.cw();
      enNor=enTgt.cw();
      
      lastP = enPos;
      lastT = enTgt;
      
      if (doFirst)
	    {
	      doFirst = false;
	      firstP = stPos;
	      firstT = stNor;
        NR::Point  tmp=curX+width*stNor;
	      if (skipMoveto)
        {
          skipMoveto = false;
        }
	      else
          dest->MoveTo (tmp);
	    }
      else
	    {
	      // jointure
	      NR::Point pos;
	      pos = curX;
	      OutlineJoin (dest, pos, curT, stNor, width, join, miter);
	    }
      
      callsData.piece = curP;
      callsData.tSt = 0.0;
      callsData.tEn = 1.0;
      callsData.x1 = curX.pt[0];
      callsData.y1 = curX.pt[1];
      callsData.x2 = nextX.pt[0];
      callsData.y2 = nextX.pt[1];
      callsData.d.c.dx1 = curD->d.c.stD.pt[0];
      callsData.d.c.dy1 = curD->d.c.stD.pt[1];
      callsData.d.c.dx2 = curD->d.c.enD.pt[0];
      callsData.d.c.dy2 = curD->d.c.enD.pt[1];
      (calls.cubicto) (&callsData, tolerance, width);
      
      curP++;
    }
    else if (nType == descr_arcto)
    {
      nextX = curD->d.a.p;
      if (nWeight)
        nextW = curD->d.a.w;
      else
        nextW = 1;
      // test de nullité du segment
      if (IsNulCurve (curD, curX))
	    {
	      curP++;
	      continue;
	    }
      // et on avance
      TangentOnArcAt (0.0, curX, curD->d.a, stPos, stTgt, stTle,
                      stRad);
      TangentOnArcAt (1.0, curX, curD->d.a, enPos, enTgt, enTle,
                      enRad);
      stNor=stTgt.cw();
      enNor=enTgt.cw();
      
      lastP = enPos;
      lastT = enTgt;	// tjs definie
      
      if (doFirst)
	    {
	      doFirst = false;
	      firstP = stPos;
	      firstT = stNor;
        NR::Point  tmp=curX+width*stNor;
	      if (skipMoveto)
        {
          skipMoveto = false;
        }
	      else
          dest->MoveTo (tmp);
	    }
      else
	    {
	      // jointure
	      NR::Point pos;
	      pos = curX;
	      OutlineJoin (dest, pos, curT, stNor, width, join, miter);
	    }
      
      callsData.piece = curP;
      callsData.tSt = 0.0;
      callsData.tEn = 1.0;
      callsData.x1 = curX.pt[0];
      callsData.y1 = curX.pt[1];
      callsData.x2 = nextX.pt[0];
      callsData.y2 = nextX.pt[1];
      callsData.d.a.rx = curD->d.a.rx;
      callsData.d.a.ry = curD->d.a.ry;
      callsData.d.a.angle = curD->d.a.angle;
      callsData.d.a.clock = curD->d.a.clockwise;
      callsData.d.a.large = curD->d.a.large;
      (calls.arcto) (&callsData, tolerance, width);
      
      curP++;
    }
    else if (nType == descr_bezierto)
    {
      int nbInterm = curD->d.b.nb;
      nextX = curD->d.b.p;
      if (nWeight)
        nextW = curD->d.b.w;
      else
        nextW = 1;
      
      if (IsNulCurve (curD, curX))
	    {
	      curP += nbInterm + 1;
	      continue;
	    }
      
      path_descr *bezStart = curD;
      curP++;
      curD = descr_data + curP;
      path_descr *intermPoints = curD;
      
      if (nbInterm <= 0)
	    {
	      // et on avance
	      path_descr_lineto temp;
	      temp.p = nextX;
	      TangentOnSegAt (0.0, curX, temp, stPos, stTgt, stTle);
	      TangentOnSegAt (1.0, curX, temp, enPos, enTgt, enTle);
        stNor=stTgt.cw();
        enNor=enTgt.cw();
        
	      lastP = enPos;
	      lastT = enTgt;
        
	      if (doFirst)
        {
          doFirst = false;
          firstP = stPos;
          firstT = stNor;
          NR::Point  tmp=curX+width*stNor;
          if (skipMoveto)
          {
            skipMoveto = false;
          }
          else
            dest->MoveTo (tmp);
        }
	      else
        {
          // jointure
          NR::Point pos;
          pos = curX;
          if (stTle > 0)
            OutlineJoin (dest, pos, curT, stNor, width, join, miter);
        }
        NR::Point  tmp=nextX+width*enNor;
	      int n_d =
          dest->LineTo (tmp);
	      if (n_d >= 0)
        {
          dest->descr_data[n_d].associated = curP - 1;
          dest->descr_data[n_d].tSt = 0.0;
          dest->descr_data[n_d].tEn = 1.0;
        }
	    }
      else if (nbInterm == 1)
	    {
        NR::Point  midX;
        float  midW;
	      midX = intermPoints->d.i.p;
	      if (nWeight)
        {
          midW = intermPoints->d.i.w;
        }
	      else
        {
          midW = 1;
        }
	      // et on avance
	      TangentOnBezAt (0.0, curX, intermPoints->d.i,
                        bezStart->d.b, false, stPos, stTgt, stTle,
                        stRad);
	      TangentOnBezAt (1.0, curX, intermPoints->d.i,
                        bezStart->d.b, true, enPos, enTgt, enTle,
                        enRad);
        stNor=stTgt.cw();
        enNor=enTgt.cw();
        
	      lastP = enPos;
	      lastT = enTgt;
        
	      if (doFirst)
        {
          doFirst = false;
          firstP = stPos;
          firstT = stNor;
          NR::Point  tmp=curX+width*stNor;
          if (skipMoveto)
          {
            skipMoveto = false;
          }
          else
            dest->MoveTo (tmp);
        }
	      else
        {
          // jointure
          NR::Point pos;
          pos = curX;
          OutlineJoin (dest, pos, curT, stNor, width, join, miter);
        }
        
	      callsData.piece = curP;
	      callsData.tSt = 0.0;
	      callsData.tEn = 1.0;
	      callsData.x1 = curX.pt[0];
	      callsData.y1 = curX.pt[1];
	      callsData.x2 = nextX.pt[0];
	      callsData.y2 = nextX.pt[1];
	      callsData.d.b.mx = midX.pt[0];
	      callsData.d.b.my = midX.pt[1];
	      (calls.bezierto) (&callsData, tolerance, width);
        
	    }
      else if (nbInterm > 1)
	    {
        NR::Point  bx=curX;
        NR::Point cx=curX;
        NR::Point dx=curX;
	      float bw = curW, cw = curW, dw = curW;
        
	      dx = intermPoints->d.i.p;
	      if (nWeight)
        {
          dw = intermPoints->d.i.w;
        }
	      else
        {
          dw = 1;
        }
	      TangentOnBezAt (0.0, curX, intermPoints->d.i,
                        bezStart->d.b, false, stPos, stTgt, stTle,
                        stRad);
        stNor=stTgt.cw();
        
	      intermPoints++;
        
	      // et on avance
	      if (stTle > 0)
        {
          if (doFirst)
          {
            doFirst = false;
            firstP = stPos;
            firstT = stNor;
            NR::Point  tmp=curX+width*stNor;
            if (skipMoveto)
            {
              skipMoveto = false;
            }
            else
              dest->MoveTo (tmp);
          }
          else
          {
            // jointure
            NR::Point pos=curX;
            OutlineJoin (dest, pos, stTgt, stNor, width, join,
                         miter);
            //                                              dest->LineTo(curX+width*stNor.x,curY+width*stNor.y);
          }
        }
        
	      cx = 2 * bx - dx;
	      cw = 2 * bw - dw;
        
	      for (int k = 0; k < nbInterm - 1; k++)
        {
          bx = cx;
          bw = cw;
          cx = dx;
          cw = dw;
          
          dx = intermPoints->d.i.p;
          if (nWeight)
          {
            dw = intermPoints->d.i.w;
          }
          else
          {
            dw = 1;
          }
          intermPoints++;
          
          NR::Point stx = (bx + cx) / 2;
          //                                      float  stw=(bw+cw)/2;
          
          path_descr_bezierto tempb;
          path_descr_intermbezierto tempi;
          tempb.nb = 1;
          tempb.p = (cx + dx) / 2;
          tempi.p = cx;
          TangentOnBezAt (1.0, stx, tempi, tempb, true, enPos,
                          enTgt, enTle, enRad);
          enNor=enTgt.cw();
          
          lastP = enPos;
          lastT = enTgt;
          
          callsData.piece = curP + k;
          callsData.tSt = 0.0;
          callsData.tEn = 1.0;
          callsData.x1 = stx.pt[0];
          callsData.y1 = stx.pt[1];
          callsData.x2 = (cx.pt[0] + dx.pt[0]) / 2;
          callsData.y2 = (cx.pt[1] + dx.pt[1]) / 2;
          callsData.d.b.mx = cx.pt[0];
          callsData.d.b.my = cx.pt[1];
          (calls.bezierto) (&callsData, tolerance, width);
        }
	      {
          bx = cx;
          bw = cw;
          cx = dx;
          cw = dw;
          
          dx = nextX;
          if (nWeight)
          {
            dw = nextW;
          }
          else
          {
            dw = 1;
          }
          dx = 2 * dx - cx;
          dw = 2 * dw - cw;
          
          NR::Point stx = (bx + cx) / 2;
          //                                      float  stw=(bw+cw)/2;
          
          path_descr_bezierto tempb;
          path_descr_intermbezierto tempi;
          tempb.nb = 1;
          tempb.p = (cx + dx) / 2;
          tempi.p = cx;
          TangentOnBezAt (1.0, stx, tempi, tempb, true, enPos,
                          enTgt, enTle, enRad);
          enNor=enTgt.cw();
          
          lastP = enPos;
          lastT = enTgt;
          
          callsData.piece = curP + nbInterm - 1;
          callsData.tSt = 0.0;
          callsData.tEn = 1.0;
          callsData.x1 = stx.pt[0];
          callsData.y1 = stx.pt[1];
          callsData.x2 = (cx.pt[0] + dx.pt[0]) / 2;
          callsData.y2 = (cx.pt[1] + dx.pt[1]) / 2;
          callsData.d.b.mx = cx.pt[0];
          callsData.d.b.my = cx.pt[1];
          (calls.bezierto) (&callsData, tolerance, width);
          
	      }
	    }
      
      // et on avance
      curP += nbInterm;
    }
    curX = nextX;
    curW = nextW;
    curT = enNor;		// sera tjs bien definie
  }
  if (closeIfNeeded)
  {
    if (doFirst == false)
    {
    }
  }
}

/*
 *
 * utilitaires pour l'outline
 *
 */

bool
Path::IsNulCurve (path_descr * curD, NR::Point &curX)
{
  int typ = curD->flags & descr_type_mask;
  if (typ == descr_lineto)
  {
    if (fabsf (curD->d.l.p.pt[0] - curX.pt[0]) < 0.00001
        && fabsf (curD->d.l.p.pt[1] - curX.pt[1]) < 0.00001)
    {
      return true;
    }
    return false;
  }
  else if (typ == descr_cubicto)
  {
    float ax, bx, cx, dx;
    float ay, by, cy, dy;
    ax = curD->d.c.stD.pt[0] + curD->d.c.enD.pt[0] + 2 * curX.pt[0] - 2 * curD->d.c.p.pt[0];
    bx = 3 * curD->d.c.p.pt[0] - 3 * curX.pt[0] - 2 * curD->d.c.stD.pt[0] - curD->d.c.enD.pt[0];
    cx = curD->d.c.stD.pt[0];
    dx = curX.pt[0];
    ay = curD->d.c.stD.pt[1] + curD->d.c.enD.pt[1] + 2 * curX.pt[1] - 2 * curD->d.c.p.pt[1];
    by = 3 * curD->d.c.p.pt[1] - 3 * curX.pt[1] - 2 * curD->d.c.stD.pt[1] - curD->d.c.enD.pt[1];
    cy = curD->d.c.stD.pt[1];
    dy = curX.pt[1];
    if (fabsf (ax) < 0.0001 && fabsf (bx) < 0.0001 && fabsf (cx) < 0.0001 &&
        fabsf (ay) < 0.0001 && fabsf (by) < 0.0001 && fabsf (cy) < 0.0001)
    {
      return true;
    }
    return false;
  }
  else if (typ == descr_arcto)
  {
    if (fabsf (curD->d.a.p.pt[0] - curX.pt[0]) < 0.00001
        && fabsf (curD->d.a.p.pt[1] - curX.pt[1]) < 0.00001)
    {
      if (curD->d.a.large == false)
	    {
	      return true;
	    }
      if (fabsf (curD->d.a.rx) < 0.00001
          || fabsf (curD->d.a.ry) < 0.00001)
	    {
	      return true;
	    }
    }
    return false;
  }
  else if (typ == descr_bezierto)
  {
    if (curD->d.b.nb <= 0)
    {
      if (fabsf (curD->d.b.p.pt[0] - curX.pt[0]) < 0.00001
          && fabsf (curD->d.b.p.pt[1] - curX.pt[1]) < 0.00001)
	    {
	      return true;
	    }
      return false;
    }
    else if (curD->d.b.nb == 1)
    {
      if (fabsf (curD->d.b.p.pt[0] - curX.pt[0]) < 0.00001
          && fabsf (curD->d.b.p.pt[1] - curX.pt[1]) < 0.00001)
	    {
	      path_descr *interm = curD + 1;
	      if (fabsf (interm->d.i.p.pt[0] - curX.pt[0]) < 0.00001
            && fabsf (interm->d.i.p.pt[1] - curX.pt[1]) < 0.00001)
        {
          return true;
        }
	    }
      return false;
    }
    else
    {
      if (fabsf (curD->d.b.p.pt[0] - curX.pt[0]) < 0.00001
          && fabsf (curD->d.b.p.pt[1] - curX.pt[1]) < 0.00001)
	    {
	      bool diff = false;
	      for (int i = 1; i <= curD->d.b.nb; i++)
        {
          path_descr *interm = curD + i;
          if (fabsf (interm->d.i.p.pt[0] - curX.pt[0]) > 0.00001
              || fabsf (interm->d.i.p.pt[1] - curX.pt[1]) > 0.00001)
          {
            diff = true;
            break;
          }
        }
	      if (diff == false)
          return true;
	    }
      return false;
    }
  }
  return true;
}

void
Path::TangentOnSegAt (float at, NR::Point &iS, path_descr_lineto & fin,
                      NR::Point & pos, NR::Point & tgt, float &len)
{
  NR::Point iE;
  iE = fin.p;
  NR::Point seg;
  seg = iE-iS;
  float l = sqrtf (dot(seg,seg));
  if (l <= 0.000001)
  {
    pos = iS;
    tgt.pt[0] = 0;
    tgt.pt[1] = 0;
    len = 0;
  }
  else
  {
    tgt = seg / l;
    pos = (1 - at) * iS + at * iE;
    len = l;
  }
}
void
Path::TangentOnArcAt (float at, NR::Point &iS, path_descr_arcto & fin,
                      NR::Point & pos, NR::Point & tgt, float &len, float &rad)
{
  NR::Point iE;
  iE = fin.p;
  float rx, ry, angle;
  rx = fin.rx;
  ry = fin.ry;
  angle = fin.angle;
  bool large, wise;
  large = fin.large;
  wise = fin.clockwise;
  
  pos = iS;
  tgt.pt[0] = tgt.pt[1] = 0;
  if (rx <= 0.0001 || ry <= 0.0001)
    return;
  
  float sex = iE.pt[0] - iS.pt[0], sey = iE.pt[1] - iS.pt[1];
  float ca = cos (angle), sa = sin (angle);
  float csex = ca * sex + sa * sey, csey = -sa * sex + ca * sey;
  csex /= rx;
  csey /= ry;
  float l = csex * csex + csey * csey;
  if (l >= 4)
    return;
  float d = 1 - l / 4;
  if (d < 0)
    d = 0;
  d = sqrt (d);
  float csdx = csey, csdy = -csex;
  l = sqrt (l);
  csdx /= l;
  csdy /= l;
  csdx *= d;
  csdy *= d;
  
  float sang, eang;
  float rax = -csdx - csex / 2, ray = -csdy - csey / 2;
  if (rax < -1)
  {
    sang = M_PI;
  }
  else if (rax > 1)
  {
    sang = 0;
  }
  else
  {
    sang = acos (rax);
    if (ray < 0)
      sang = 2 * M_PI - sang;
  }
  rax = -csdx + csex / 2;
  ray = -csdy + csey / 2;
  if (rax < -1)
  {
    eang = M_PI;
  }
  else if (rax > 1)
  {
    eang = 0;
  }
  else
  {
    eang = acos (rax);
    if (ray < 0)
      eang = 2 * M_PI - eang;
  }
  
  csdx *= rx;
  csdy *= ry;
  float drx = ca * csdx - sa * csdy, dry = sa * csdx + ca * csdy;
  
  if (wise)
  {
    if (large == true)
    {
      drx = -drx;
      dry = -dry;
      float swap = eang;
      eang = sang;
      sang = swap;
      eang += M_PI;
      sang += M_PI;
      if (eang >= 2 * M_PI)
        eang -= 2 * M_PI;
      if (sang >= 2 * M_PI)
        sang -= 2 * M_PI;
    }
  }
  else
  {
    if (large == false)
    {
      drx = -drx;
      dry = -dry;
      float swap = eang;
      eang = sang;
      sang = swap;
      eang += M_PI;
      sang += M_PI;
      if (eang >= 2 * M_PI)
        eang -= 2 * M_PI;
      if (sang >= 2 * M_PI)
        sang -= 2 * M_PI;
    }
  }
  drx += (iS.pt[0] + iE.pt[0]) / 2;
  dry += (iS.pt[1] + iE.pt[1]) / 2;
  
  if (wise)
  {
    if (sang < eang)
      sang += 2 * M_PI;
    float b = sang * (1 - at) + eang * at;
    float cb = cos (b), sb = sin (b);
    pos.pt[0] = drx + ca * rx * cb - sa * ry * sb;
    pos.pt[1] = dry + sa * rx * cb + ca * ry * sb;
    tgt.pt[0] = ca * rx * sb + sa * ry * cb;
    tgt.pt[1] = sa * rx * sb - ca * ry * cb;
    NR::Point dtgt;
    dtgt.pt[0] = -ca * rx * cb + sa * ry * sb;
    dtgt.pt[1] = -sa * rx * cb - ca * ry * sb;
    len = sqrtf (tgt.pt[0] * tgt.pt[0] + tgt.pt[1] * tgt.pt[1]);
    rad =
      len * (tgt.pt[0] * tgt.pt[0] + tgt.pt[1] * tgt.pt[1]) / (tgt.pt[0] * dtgt.pt[1] -
                                                               tgt.pt[1] * dtgt.pt[0]);
    tgt.pt[0] /= len;
    tgt.pt[1] /= len;
  }
  else
  {
    if (sang > eang)
      sang -= 2 * M_PI;
    float b = sang * (1 - at) + eang * at;
    float cb = cos (b), sb = sin (b);
    pos.pt[0] = drx + ca * rx * cb - sa * ry * sb;
    pos.pt[1] = dry + sa * rx * cb + ca * ry * sb;
    tgt.pt[0] = ca * rx * sb + sa * ry * cb;
    tgt.pt[1] = sa * rx * sb - ca * ry * cb;
    NR::Point dtgt;
    dtgt.pt[0] = -ca * rx * cb + sa * ry * sb;
    dtgt.pt[1] = -sa * rx * cb - ca * ry * sb;
    len = sqrtf (tgt.pt[0] * tgt.pt[0] + tgt.pt[1] * tgt.pt[1]);
    rad =
      len * (tgt.pt[0] * tgt.pt[0] + tgt.pt[1] * tgt.pt[1]) / (tgt.pt[0] * dtgt.pt[1] -
                                                               tgt.pt[1] * dtgt.pt[0]);
    tgt.pt[0] /= len;
    tgt.pt[1] /= len;
    
  }
}
void
Path::TangentOnCubAt (float at, NR::Point &iS, path_descr_cubicto & fin,
                      bool before, NR::Point & pos, NR::Point & tgt, float &len,
                      float &rad)
{
  float ex, ey;
  ex = fin.p.pt[0];
  ey = fin.p.pt[1];
  float sdx, sdy, edx, edy;
  edx = fin.enD.pt[0];
  edy = fin.enD.pt[1];
  sdx = fin.stD.pt[0];
  sdy = fin.stD.pt[1];
  
  pos = iS;
  tgt.pt[0] = 0;
  tgt.pt[1] = 0;
  len = rad = 0;
  
  float ax, bx, cx, dx;
  float ay, by, cy, dy;
  ax = sdx + edx - 2 * ex + 2 * iS.pt[0];
  bx = 0.5 * (edx - sdx);
  cx = 0.25 * (6 * ex - 6 * iS.pt[0] - sdx - edx);
  dx = 0.125 * (4 * iS.pt[0] + 4 * ex - edx + sdx);
  ay = sdy + edy - 2 * ey + 2 * iS.pt[1];
  by = 0.5 * (edy - sdy);
  cy = 0.25 * (6 * ey - 6 * iS.pt[1] - sdy - edy);
  dy = 0.125 * (4 * iS.pt[1] + 4 * ey - edy + sdy);
  float atb = at - 0.5;
  pos.pt[0] = ax * atb * atb * atb + bx * atb * atb + cx * atb + dx;
  pos.pt[1] = ay * atb * atb * atb + by * atb * atb + cy * atb + dy;
  NR::Point der, dder, ddder;
  der.pt[0] = 3 * ax * atb * atb + 2 * bx * atb + cx;
  der.pt[1] = 3 * ay * atb * atb + 2 * by * atb + cy;
  dder.pt[0] = 6 * ax * atb + 2 * bx;
  dder.pt[1] = 6 * ay * atb + 2 * by;
  ddder.pt[0] = 6 * ax;
  ddder.pt[1] = 6 * ay;
  /*	ax=sdx+edx+2*sx-2*ex;
	bx=3*ex-3*sx-2*sdx-edx;
	cx=sdx;
	dx=sx;
	ay=sdy+edy+2*sy-2*ey;
	by=3*ey-3*sy-2*sdy-edy;
	cy=sdy;
	dy=sy;
	
	pos.x=ax*at*at*at+bx*at*at+cx*at+dx;
	pos.y=ay*at*at*at+by*at*at+cy*at+dy;
	NR::Point  der,dder,ddder;
	der.x=3*ax*at*at+2*bx*at+cx;
	der.y=3*ay*at*at+2*by*at+cy;
	dder.x=6*ax*at+2*bx;
	dder.y=6*ay*at+2*by;
	ddder.x=6*ax;
	ddder.y=6*ay;*/
  float l = sqrtf (dot(der,der));
  if (l <= 0.0001)
  {
    len = 0;
    l = sqrtf (dot(dder,dder));
    if (l <= 0.0001)
    {
      l = sqrtf (dot(ddder,ddder));
      if (l <= 0.0001)
	    {
	      // pas de segment....
	      return;
	    }
      rad = 100000000;
      tgt = ddder / l;
      if (before)
	    {
	      tgt = -tgt;
	    }
      return;
    }
    rad =
      -l * (dot(dder,dder)) / (cross(ddder,dder));
    tgt = dder / l;
    if (before)
    {
      tgt = -tgt;
    }
    return;
  }
  len = l;
  
  rad =
    -l * (dot(der,der)) / (cross(dder,der));
  
  tgt = der / l;
}

void
Path::TangentOnBezAt (float at, NR::Point &iS,
                      path_descr_intermbezierto & mid,
                      path_descr_bezierto & fin, bool before, NR::Point & pos,
                      NR::Point & tgt, float &len, float &rad)
{
  float ex, ey;
  ex = fin.p.pt[0];
  ey = fin.p.pt[1];
  float mx, my;
  mx = mid.p.pt[0];
  my = mid.p.pt[1];
  
  pos = iS;
  tgt.pt[0] = tgt.pt[1] = 0;
  len = rad = 0;
  float ax, bx, cx;
  float ay, by, cy;
  ax = ex + iS.pt[0] - 2 * mx;
  bx = 2 * mx - 2 * iS.pt[0];
  cx = iS.pt[0];
  ay = ey + iS.pt[1] - 2 * my;
  by = 2 * my - 2 * iS.pt[1];
  cy = iS.pt[1];
  
  pos.pt[0] = ax * at * at + bx * at + cx;
  pos.pt[1] = ay * at * at + by * at + cy;
  NR::Point der, dder;
  der.pt[0] = 2 * ax * at + bx;
  der.pt[1] = 2 * ay * at + by;
  dder.pt[0] = 2 * ax;
  dder.pt[1] = 2 * ay;
  float l = sqrtf (dot(der,der));
  if (l <= 0.0001)
  {
    len = 0;
    l = sqrtf (dot(dder,dder));
    if (l <= 0.0001)
    {
      // pas de segment....
      return;
    }
    rad = 100000000;
    tgt = dder / l;
    if (before)
    {
      tgt = -tgt;
    }
    return;
  }
  len = l;
  rad =
    -l * (dot(der,der)) / (cross(dder,der));
  
  tgt = der / l;
}

void
Path::OutlineJoin (Path * dest, NR::Point pos, NR::Point stNor, NR::Point enNor, float width,
                   JoinType join, float miter)
{
  float angSi = cross (enNor,stNor);
  float angCo = dot (stNor, enNor);
  // 1/1000 est tres grossier, mais sinon ca merde tout azimut
  if ((width >= 0 && angSi > -0.001) || (width < 0 && angSi < 0.001))
  {
    if (angCo > 0.999)
    {
      // tout droit
    }
    else if (angCo < -0.999)
    {
      // demit-tour
      NR::Point tmp=pos+width*enNor;
      dest->LineTo (tmp);
    }
    else
    {
      dest->LineTo (pos);
      NR::Point tmp=pos+width*enNor;
      dest->LineTo (tmp);
    }
  }
  else
  {
    if (join == join_round)
    {
      // utiliser des bouts de cubique: approximation de l'arc (au point ou on en est...), et supporte mieux 
      // l'arrondi des coordonnees des extremites
      /*			float   angle=acos(angCo);
			if ( angCo >= 0 ) {
				NR::Point   stTgt,enTgt;
				RotCCWTo(stNor,stTgt);
				RotCCWTo(enNor,enTgt);
				dest->CubicTo(pos.x+width*enNor.x,pos.y+width*enNor.y,angle*width*stTgt.x,angle*width*stTgt.y,angle*width*enTgt.x,angle*width*enTgt.y);
			} else {
				NR::Point   biNor;
				NR::Point   stTgt,enTgt,biTgt;
				biNor.x=stNor.x+enNor.x;
				biNor.y=stNor.y+enNor.y;
				float  biL=sqrt(biNor.x*biNor.x+biNor.y*biNor.y);
				biNor.x/=biL;
				biNor.y/=biL;
				RotCCWTo(stNor,stTgt);
				RotCCWTo(enNor,enTgt);
				RotCCWTo(biNor,biTgt);
				dest->CubicTo(pos.x+width*biNor.x,pos.y+width*biNor.y,angle*width*stTgt.x,angle*width*stTgt.y,angle*width*biTgt.x,angle*width*biTgt.y);
				dest->CubicTo(pos.x+width*enNor.x,pos.y+width*enNor.y,angle*width*biTgt.x,angle*width*biTgt.y,angle*width*enTgt.x,angle*width*enTgt.y);
			}*/
      if (width > 0)
	    {
        NR::Point tmp=pos+width*enNor;
	      dest->ArcTo (tmp,
                     1.0001 * width, 1.0001 * width, 0.0, false, true);
	    }
      else
	    {
        NR::Point tmp=pos+width*enNor;
	      dest->ArcTo (tmp,
                     -1.0001 * width, -1.0001 * width, 0.0, false,
                     false);
	    }
    }
    else if (join == join_pointy)
    {
      NR::Point biss;
      biss = stNor + enNor;
      float lb = sqrt (dot(biss,biss));
      biss /= lb;
      float angCo = dot (biss, enNor);
      float angSi = cross (biss, stNor);
      float l = width / angCo;
      if (miter < 0.5 * lb)
        miter = 0.5 * lb;
      if (l > miter)
	    {
	      float r = (l - miter) * angCo / angSi;
        NR::Point tmp;
        tmp.pt[0]=miter*biss.pt[0]-r*biss.pt[1];
        tmp.pt[1]=miter*biss.pt[1]+r*biss.pt[0];
        tmp+=pos;
	      dest->LineTo (tmp);
        tmp.pt[0]=miter*biss.pt[0]+r*biss.pt[1];
        tmp.pt[1]=miter*biss.pt[1]-r*biss.pt[0];
        tmp+=pos;
	      dest->LineTo (tmp);
        tmp=pos+width*enNor;
	      dest->LineTo (tmp);
	    }
      else
	    {
        NR::Point tmp=pos+l*biss;
	      dest->LineTo (tmp);
        tmp=pos+width*enNor;
	      dest->LineTo (tmp);
	    }
    }
    else
    {
      NR::Point tmp=pos+width*enNor;
      dest->LineTo (tmp);
    }
  }
}

// les callbacks

void
Path::RecStdCubicTo (outline_callback_data * data, float tol, float width,
                     int lev)
{
  NR::Point stPos, miPos, enPos;
  NR::Point stTgt, enTgt, miTgt, stNor, enNor, miNor;
  float stRad, miRad, enRad;
  float stTle, miTle, enTle;
  // un cubic
  {
    path_descr_cubicto temp;
    temp.p.pt[0] = data->x2;
    temp.p.pt[1] = data->y2;
    temp.stD.pt[0] = data->d.c.dx1;
    temp.stD.pt[1] = data->d.c.dy1;
    temp.enD.pt[0] = data->d.c.dx2;
    temp.enD.pt[1] = data->d.c.dy2;
    NR::Point tmp;
    tmp.pt[0]=data->x1;
    tmp.pt[1]=data->y1;
    TangentOnCubAt (0.0, tmp, temp, false, stPos, stTgt, stTle,
                    stRad);
    TangentOnCubAt (0.5, tmp, temp, false, miPos, miTgt, miTle,
                    miRad);
    TangentOnCubAt (1.0, tmp, temp, true, enPos, enTgt, enTle,
                    enRad);
    stNor=stTgt.cw();
    miNor=miTgt.cw();
    enNor=enTgt.cw();
  }
  
  float stGue = 1, miGue = 1, enGue = 1;
  if (fabsf (stRad) > 0.01)
    stGue += width / stRad;
  if (fabsf (miRad) > 0.01)
    miGue += width / miRad;
  if (fabsf (enRad) > 0.01)
    enGue += width / enRad;
  stGue *= stTle;
  miGue *= miTle;
  enGue *= enTle;
  
  
  if (lev <= 0)
  {
    NR::Point  tmp=enPos+width*enNor;
    NR::Point tms=stGue*stTgt;
    NR::Point tme=enGue*enTgt;
    int n_d =
      data->dest->CubicTo (tmp,tms,tme);
    if (n_d >= 0)
    {
      data->dest->descr_data[n_d].associated = data->piece;
      data->dest->descr_data[n_d].tSt = data->tSt;
      data->dest->descr_data[n_d].tEn = data->tEn;
    }
    return;
  }
  
  NR::Point req, chk;
  req = miPos + width * miNor;
  {
    path_descr_cubicto temp;
    float chTle, chRad;
    NR::Point chTgt;
    temp.p = enPos + width * enNor;
    temp.stD = stGue * stTgt;
    temp.enD = enGue * enTgt;
    NR::Point tmp=stPos+width*stNor;
    TangentOnCubAt (0.5, tmp,
                    temp, false, chk, chTgt, chTle, chRad);
  }
  NR::Point diff;
  diff = req - chk;
  float err = (dot(diff,diff));
  if (err <= tol * tol)
  {
    NR::Point  tmp=enPos+width*enNor;
    NR::Point tms=stGue*stTgt;
    NR::Point tme=enGue*enTgt;
    int n_d =
      data->dest->CubicTo (tmp,tms,tme);
    if (n_d >= 0)
    {
      data->dest->descr_data[n_d].associated = data->piece;
      data->dest->descr_data[n_d].tSt = data->tSt;
      data->dest->descr_data[n_d].tEn = data->tEn;
    }
  }
  else
  {
    outline_callback_data desc = *data;
    
    desc.tSt = data->tSt;
    desc.tEn = (data->tSt + data->tEn) / 2;
    desc.x1 = data->x1;
    desc.y1 = data->y1;
    desc.x2 = miPos.pt[0];
    desc.y2 = miPos.pt[1];
    desc.d.c.dx1 = 0.5 * stTle * stTgt.pt[0];
    desc.d.c.dy1 = 0.5 * stTle * stTgt.pt[1];
    desc.d.c.dx2 = 0.5 * miTle * miTgt.pt[0];
    desc.d.c.dy2 = 0.5 * miTle * miTgt.pt[1];
    RecStdCubicTo (&desc, tol, width, lev - 1);
    
    desc.tSt = (data->tSt + data->tEn) / 2;
    desc.tEn = data->tEn;
    desc.x1 = miPos.pt[0];
    desc.y1 = miPos.pt[1];
    desc.x2 = data->x2;
    desc.y2 = data->y2;
    desc.d.c.dx1 = 0.5 * miTle * miTgt.pt[0];
    desc.d.c.dy1 = 0.5 * miTle * miTgt.pt[1];
    desc.d.c.dx2 = 0.5 * enTle * enTgt.pt[0];
    desc.d.c.dy2 = 0.5 * enTle * enTgt.pt[1];
    RecStdCubicTo (&desc, tol, width, lev - 1);
  }
}

void
Path::StdCubicTo (Path::outline_callback_data * data, float tol, float width)
{
  fflush (stdout);
  RecStdCubicTo (data, tol, width, 8);
}

void
Path::StdBezierTo (Path::outline_callback_data * data, float tol, float width)
{
  path_descr_bezierto tempb;
  path_descr_intermbezierto tempi;
  tempb.nb = 1;
  tempb.p.pt[0] = data->x2;
  tempb.p.pt[1] = data->y2;
  tempi.p.pt[0] = data->d.b.mx;
  tempi.p.pt[1] = data->d.b.my;
  NR::Point stPos, enPos, stTgt, enTgt;
  float stRad, enRad, stTle, enTle;
  NR::Point  tmp(data->x1,data->y1);
  TangentOnBezAt (0.0, tmp, tempi, tempb, false, stPos, stTgt,
                  stTle, stRad);
  TangentOnBezAt (1.0, tmp, tempi, tempb, true, enPos, enTgt,
                  enTle, enRad);
  data->d.c.dx1 = stTle * stTgt.pt[0];
  data->d.c.dy1 = stTle * stTgt.pt[1];
  data->d.c.dx2 = enTle * enTgt.pt[0];
  data->d.c.dy2 = enTle * enTgt.pt[1];
  RecStdCubicTo (data, tol, width, 8);
}

void
Path::RecStdArcTo (outline_callback_data * data, float tol, float width,
                   int lev)
{
  NR::Point stPos, miPos, enPos;
  NR::Point stTgt, enTgt, miTgt, stNor, enNor, miNor;
  float stRad, miRad, enRad;
  float stTle, miTle, enTle;
  // un cubic
  {
    path_descr_arcto temp;
    temp.p.pt[0] = data->x2;
    temp.p.pt[1] = data->y2;
    temp.rx = data->d.a.rx;
    temp.ry = data->d.a.ry;
    temp.angle = data->d.a.angle;
    temp.clockwise = data->d.a.clock;
    temp.large = data->d.a.large;
    NR::Point tmp(data->x1,data->y1);
    TangentOnArcAt (data->d.a.stA, tmp, temp, stPos, stTgt,
                    stTle, stRad);
    TangentOnArcAt ((data->d.a.stA + data->d.a.enA) / 2, tmp,
                    temp, miPos, miTgt, miTle, miRad);
    TangentOnArcAt (data->d.a.enA, tmp, temp, enPos, enTgt,
                    enTle, enRad);
    stNor=stTgt.cw();
    miNor=miTgt.cw();
    enNor=enTgt.cw();
  }
  
  float stGue = 1, miGue = 1, enGue = 1;
  if (fabsf (stRad) > 0.01)
    stGue += width / stRad;
  if (fabsf (miRad) > 0.01)
    miGue += width / miRad;
  if (fabsf (enRad) > 0.01)
    enGue += width / enRad;
  stGue *= stTle;
  miGue *= miTle;
  enGue *= enTle;
  float sang, eang;
  {
    NR::Point  tms(data->x1,data->y1),tme(data->x2,data->y2);
    ArcAngles (tms,tme, data->d.a.rx,
               data->d.a.ry, data->d.a.angle, data->d.a.large, !data->d.a.clock,
               sang, eang);
  }
  float scal = eang - sang;
  if (scal < 0)
    scal += 2 * M_PI;
  if (scal > 2 * M_PI)
    scal -= 2 * M_PI;
  scal *= data->d.a.enA - data->d.a.stA;
  
  if (lev <= 0)
  {
    NR::Point  tmp=enPos+width*enNor;
    NR::Point tms=stGue*scal*stTgt;
    NR::Point tme=enGue*scal*enTgt;
    int n_d =
      data->dest->CubicTo (tmp,tms,tme);
    if (n_d >= 0)
    {
      data->dest->descr_data[n_d].associated = data->piece;
      data->dest->descr_data[n_d].tSt = data->d.a.stA;
      data->dest->descr_data[n_d].tEn = data->d.a.enA;
    }
    return;
  }
  
  NR::Point req, chk;
  req = miPos + width * miNor;
  {
    path_descr_cubicto temp;
    float chTle, chRad;
    NR::Point chTgt;
    temp.p = enPos + width * enNor;
    temp.stD = stGue * scal * stTgt;
    temp.enD = enGue * scal * enTgt;
    NR::Point tmp=stPos+width*stNor;
    TangentOnCubAt (0.5, tmp,
                    temp, false, chk, chTgt, chTle, chRad);
  }
  NR::Point diff;
  diff = req - chk;
  float err = (dot(diff,diff));
  if (err <= tol * tol)
  {
    NR::Point  tmp=enPos+width*enNor;
    NR::Point tms=stGue*scal*stTgt;
    NR::Point tme=enGue*scal*enTgt;
    int n_d =
      data->dest->CubicTo (tmp,tms,tme);
    if (n_d >= 0)
    {
      data->dest->descr_data[n_d].associated = data->piece;
      data->dest->descr_data[n_d].tSt = data->d.a.stA;
      data->dest->descr_data[n_d].tEn = data->d.a.enA;
    }
  }
  else
  {
    outline_callback_data desc = *data;
    
    desc.d.a.stA = data->d.a.stA;
    desc.d.a.enA = (data->d.a.stA + data->d.a.enA) / 2;
    RecStdArcTo (&desc, tol, width, lev - 1);
    
    desc.d.a.stA = (data->d.a.stA + data->d.a.enA) / 2;
    desc.d.a.enA = data->d.a.enA;
    RecStdArcTo (&desc, tol, width, lev - 1);
  }
}
void
Path::StdArcTo (Path::outline_callback_data * data, float tol, float width)
{
  data->d.a.stA = 0.0;
  data->d.a.enA = 1.0;
  RecStdArcTo (data, tol, width, 8);
}
