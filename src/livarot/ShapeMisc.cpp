/*
 *  ShapeMisc.cpp
 *  nlivarot
 *
 *  Created by fred on Sun Jul 20 2003.
 *
 */

#include "Shape.h"
#include <libnr/nr-point.h>
#include <libnr/nr-point-fns.h>
//#include "MyMath.h"
#include "Path.h"
#include <glib.h>

/*
 * polygon offset and polyline to path reassembling (when using back data)
 */

// until i find something better
#define MiscNormalize(v) {\
  double _l=sqrt(dot(v,v)); \
    if ( _l < 0.0000001 ) { \
      v[0]=v[1]=0; \
    } else { \
      v/=_l; \
    }\
}

// extracting the contour of an uncrossed polygon: a mere depth first search
// more precisely that's extracting an eulerian path from a graph, but here we want to split
// the polygon into contours and avoid holes. so we take a "next counter-clockwise edge first" approach
// (make a checkboard and extract its contours to see the difference)
void
Shape::ConvertToForme (Path * dest)
{
  if (nbPt <= 1 || nbAr <= 1)
    return;
  if (Eulerian (true) == false)
    return;
  
  // prepare
  dest->Reset ();
  
  MakePointData (true);
  MakeEdgeData (true);
  MakeSweepDestData (true);
  
  for (int i = 0; i < nbPt; i++)
  {
    pData[i].rx[0] = Round (pts[i].x[0]);
    pData[i].rx[1] = Round (pts[i].x[1]);
  }
  for (int i = 0; i < nbAr; i++)
  {
    eData[i].rdx = pData[aretes[i].en].rx - pData[aretes[i].st].rx;
  }
  
  // sort edge clockwise, with the closest after midnight being first in the doubly-linked list
  // that's vital to the algorithm...
  SortEdges ();
  
  // depth-first search implies: we make a stack of edges traversed.
  // precParc: previous in the stack
  // suivParc: next in the stack
  for (int i = 0; i < nbAr; i++)
  {
    swdData[i].misc = 0;
    swdData[i].precParc = swdData[i].suivParc = -1;
  }
  
  int searchInd = 0;
  
  int lastPtUsed = 0;
  do
  {
    // first get a starting point, and a starting edge
    // -> take the upper left point, and take its first edge
    // points traversed have swdData[].misc != 0, so it's easy
    int startBord = -1;
    {
      int fi = 0;
      for (fi = lastPtUsed; fi < nbPt; fi++)
      {
        if (pts[fi].firstA >= 0 && swdData[pts[fi].firstA].misc == 0)
          break;
      }
      lastPtUsed = fi + 1;
      if (fi < nbPt)
      {
        int bestB = pts[fi].firstA;
        while (bestB >= 0 && aretes[bestB].st != fi)
          bestB = NextAt (fi, bestB);
        if (bestB >= 0)
	      {
          startBord = bestB;
          dest->MoveTo (pts[aretes[startBord].en].x);
	      }
      }
    }
    // and walk the graph, doing contours when needed
    if (startBord >= 0)
    {
      // parcours en profondeur pour mettre les leF et riF a leurs valeurs
      swdData[startBord].misc = (void *) 1;
      //                      printf("part de %d\n",startBord);
      int curBord = startBord;
      bool back = false;
      swdData[curBord].precParc = -1;
      swdData[curBord].suivParc = -1;
      do
	    {
	      int cPt = aretes[curBord].en;
	      int nb = curBord;
        //                              printf("de curBord= %d au point %i  -> ",curBord,cPt);
        // get next edge
	      do
        {
          int nnb = CycleNextAt (cPt, nb);
          if (nnb == nb)
          {
            // cul-de-sac
            nb = -1;
            break;
          }
          nb = nnb;
          if (nb < 0 || nb == curBord)
            break;
        }
	      while (swdData[nb].misc != 0 || aretes[nb].st != cPt);
        
	      if (nb < 0 || nb == curBord)
        {
          // no next edge: end of this contour, we get back
          if (back == false)
            dest->Close ();
          back = true;
          // retour en arriere
          curBord = swdData[curBord].precParc;
          //                                      printf("retour vers %d\n",curBord);
          if (curBord < 0)
            break;
        }
	      else
        {
          // new edge, maybe for a new contour
          if (back)
          {
            // we were backtracking, so if we have a new edge, that means we're creating a new contour
            dest->MoveTo (pts[cPt].x);
            back = false;
          }
          swdData[nb].misc = (void *) 1;
          swdData[nb].ind = searchInd++;
          swdData[nb].precParc = curBord;
          swdData[curBord].suivParc = nb;
          curBord = nb;
          //                                      printf("suite %d\n",curBord);
          {
            // add that edge
            dest->LineTo (pts[aretes[nb].en].x);
          }
        }
	    }
      while (1 /*swdData[curBord].precParc >= 0 */ );
      // fin du cas non-oriente
    }
  }
  while (lastPtUsed < nbPt);
  
  MakePointData (false);
  MakeEdgeData (false);
  MakeSweepDestData (false);
}

// same as before, but each time we have a contour, try to reassemble the segments on it to make chunks of
// the original(s) path(s)
// originals are in the orig array, whose size is nbP
void
Shape::ConvertToForme (Path * dest, int nbP, Path * *orig, bool splitWhenForced)
{
  if (nbPt <= 1 || nbAr <= 1)
    return;
//  if (Eulerian (true) == false)
//    return;
  
  if (HasBackData () == false)
  {
    ConvertToForme (dest);
    return;
  }
  
  dest->Reset ();
  
  MakePointData (true);
  MakeEdgeData (true);
  MakeSweepDestData (true);
  
  for (int i = 0; i < nbPt; i++)
  {
    pData[i].rx[0] = Round (pts[i].x[0]);
    pData[i].rx[1] = Round (pts[i].x[1]);
  }
  for (int i = 0; i < nbAr; i++)
  {
    eData[i].rdx = pData[aretes[i].en].rx - pData[aretes[i].st].rx;
  }
  
  SortEdges ();
  
  for (int i = 0; i < nbAr; i++)
  {
    swdData[i].misc = 0;
    swdData[i].precParc = swdData[i].suivParc = -1;
  }
  
  int searchInd = 0;
  
  int lastPtUsed = 0;
  do
  {
    int startBord = -1;
    {
      int fi = 0;
      for (fi = lastPtUsed; fi < nbPt; fi++)
      {
        if (pts[fi].firstA >= 0 && swdData[pts[fi].firstA].misc == 0)
          break;
      }
      lastPtUsed = fi + 1;
      if (fi < nbPt)
      {
        int bestB = pts[fi].firstA;
        while (bestB >= 0 && aretes[bestB].st != fi)
          bestB = NextAt (fi, bestB);
        if (bestB >= 0)
	      {
          startBord = bestB;
	      }
      }
    }
    if (startBord >= 0)
    {
      // parcours en profondeur pour mettre les leF et riF a leurs valeurs
      swdData[startBord].misc = (void *) 1;
      //printf("part de %d\n",startBord);
      int curBord = startBord;
      bool back = false;
      swdData[curBord].precParc = -1;
      swdData[curBord].suivParc = -1;
      int curStartPt=aretes[curBord].st;
      do
	    {
	      int cPt = aretes[curBord].en;
	      int nb = curBord;
        //printf("de curBord= %d au point %i  -> ",curBord,cPt);
	      do
        {
          int nnb = CycleNextAt (cPt, nb);
          if (nnb == nb)
          {
            // cul-de-sac
            nb = -1;
            break;
          }
          nb = nnb;
          if (nb < 0 || nb == curBord)
            break;
        }
	      while (swdData[nb].misc != 0 || aretes[nb].st != cPt);
        
	      if (nb < 0 || nb == curBord)
        {
          if (back == false)
          {
            if (curBord == startBord || curBord < 0)
            {
              // probleme -> on vire le moveto
              //                                                      dest->descr_nb--;
            }
            else
            {
              swdData[curBord].suivParc = -1;
              AddContour (dest, nbP, orig, startBord, curBord,splitWhenForced);
            }
            //                                              dest->Close();
          }
          back = true;
          // retour en arriere
          curBord = swdData[curBord].precParc;
          //printf("retour vers %d\n",curBord);
          if (curBord < 0)
            break;
        }
	      else
        {
          if (back)
          {
            back = false;
            startBord = nb;
            curStartPt=aretes[nb].st;
          } else {
            if ( aretes[curBord].en == curStartPt ) {
              //printf("contour %i ",curStartPt);
              swdData[curBord].suivParc = -1;
              AddContour (dest, nbP, orig, startBord, curBord,splitWhenForced);
              startBord=nb;
            }
          }
          swdData[nb].misc = (void *) 1;
          swdData[nb].ind = searchInd++;
          swdData[nb].precParc = curBord;
          swdData[curBord].suivParc = nb;
          curBord = nb;
          //printf("suite %d\n",curBord);
        }
	    }
      while (1 /*swdData[curBord].precParc >= 0 */ );
      // fin du cas non-oriente
    }
  }
  while (lastPtUsed < nbPt);
  
  MakePointData (false);
  MakeEdgeData (false);
  MakeSweepDestData (false);
}
void 
Shape::ConvertToFormeNested (Path * dest, int nbP, Path * *orig, int wildPath,int &nbNest,int *&nesting,int *&contStart,bool splitWhenForced)
{
  nesting=NULL;
  contStart=NULL;
  nbNest=0;

  if (nbPt <= 1 || nbAr <= 1)
    return;
  //  if (Eulerian (true) == false)
  //    return;
  
  if (HasBackData () == false)
  {
    ConvertToForme (dest);
    return;
  }
  
  dest->Reset ();
  
//  MakePointData (true);
  MakeEdgeData (true);
  MakeSweepDestData (true);
  
  for (int i = 0; i < nbPt; i++)
  {
    pData[i].rx[0] = Round (pts[i].x[0]);
    pData[i].rx[1] = Round (pts[i].x[1]);
  }
  for (int i = 0; i < nbAr; i++)
  {
    eData[i].rdx = pData[aretes[i].en].rx - pData[aretes[i].st].rx;
  }
  
  SortEdges ();
  
  for (int i = 0; i < nbAr; i++)
  {
    swdData[i].misc = 0;
    swdData[i].precParc = swdData[i].suivParc = -1;
  }
  
  int searchInd = 0;
  
  int lastPtUsed = 0;
  do
  {
    int dadContour=-1;
    int startBord = -1;
    {
      int fi = 0;
      for (fi = lastPtUsed; fi < nbPt; fi++)
      {
        if (pts[fi].firstA >= 0 && swdData[pts[fi].firstA].misc == 0)
          break;
      }
      {
        int askTo = pData[fi].askForWindingB;
        if (askTo < 0 || askTo >= nbAr ) {
          dadContour=-1;
        } else {
          dadContour = (int) swdData[askTo].misc;
          dadContour-=1; // pour compenser le decalage
        }
      }
      lastPtUsed = fi + 1;
      if (fi < nbPt)
      {
        int bestB = pts[fi].firstA;
        while (bestB >= 0 && aretes[bestB].st != fi)
          bestB = NextAt (fi, bestB);
        if (bestB >= 0)
	      {
          startBord = bestB;
	      }
      }
    }
    if (startBord >= 0)
    {
      // parcours en profondeur pour mettre les leF et riF a leurs valeurs
      swdData[startBord].misc = (void *) (1+nbNest);
      //printf("part de %d\n",startBord);
      int curBord = startBord;
      bool back = false;
      swdData[curBord].precParc = -1;
      swdData[curBord].suivParc = -1;
      int curStartPt=aretes[curBord].st;
      do
	    {
	      int cPt = aretes[curBord].en;
	      int nb = curBord;
        //printf("de curBord= %d au point %i  -> ",curBord,cPt);
	      do
        {
          int nnb = CycleNextAt (cPt, nb);
          if (nnb == nb)
          {
            // cul-de-sac
            nb = -1;
            break;
          }
          nb = nnb;
          if (nb < 0 || nb == curBord)
            break;
        }
	      while (swdData[nb].misc != 0 || aretes[nb].st != cPt);
        
	      if (nb < 0 || nb == curBord)
        {
          if (back == false)
          {
            if (curBord == startBord || curBord < 0)
            {
              // probleme -> on vire le moveto
              //                                                      dest->descr_nb--;
            }
            else
            {
              bool escapePath=false;
              int tb=curBord;
              while ( tb >= 0 && tb < nbAr ) {
                if ( ebData[tb].pathID == wildPath ) {
                  escapePath=true;
                  break;
                }
                tb=swdData[tb].precParc;
              }
              nesting=(int*)g_realloc(nesting,(nbNest+1)*sizeof(int));
              contStart=(int*)g_realloc(contStart,(nbNest+1)*sizeof(int));
              contStart[nbNest]=dest->descr_nb;
              if ( escapePath ) {
                nesting[nbNest++]=-1; // contient des bouts de coupure -> a part
              } else {
                nesting[nbNest++]=dadContour;
              }
              swdData[curBord].suivParc = -1;
              AddContour (dest, nbP, orig, startBord, curBord,splitWhenForced);
            }
            //                                              dest->Close();
          }
          back = true;
          // retour en arriere
          curBord = swdData[curBord].precParc;
          //printf("retour vers %d\n",curBord);
          if (curBord < 0)
            break;
        }
	      else
        {
          if (back)
          {
            back = false;
            startBord = nb;
            curStartPt=aretes[nb].st;
          } else {
            if ( aretes[curBord].en == curStartPt ) {
              //printf("contour %i ",curStartPt);
              
              bool escapePath=false;
              int tb=curBord;
              while ( tb >= 0 && tb < nbAr ) {
                if ( ebData[tb].pathID == wildPath ) {
                  escapePath=true;
                  break;
                }
                tb=swdData[tb].precParc;
              }
              nesting=(int*)g_realloc(nesting,(nbNest+1)*sizeof(int));
              contStart=(int*)g_realloc(contStart,(nbNest+1)*sizeof(int));
              contStart[nbNest]=dest->descr_nb;
              if ( escapePath ) {
                nesting[nbNest++]=-1; // contient des bouts de coupure -> a part
              } else {
                nesting[nbNest++]=dadContour;
              }

              swdData[curBord].suivParc = -1;
              AddContour (dest, nbP, orig, startBord, curBord,splitWhenForced);
              startBord=nb;
            }
          }
          swdData[nb].misc = (void *) (1+nbNest);
          swdData[nb].ind = searchInd++;
          swdData[nb].precParc = curBord;
          swdData[curBord].suivParc = nb;
          curBord = nb;
          //printf("suite %d\n",curBord);
        }
	    }
      while (1 /*swdData[curBord].precParc >= 0 */ );
      // fin du cas non-oriente
    }
  }
  while (lastPtUsed < nbPt);
  
  MakePointData (false);
  MakeEdgeData (false);
  MakeSweepDestData (false);
}

// offsets
// take each edge, offset it, and make joins with previous at edge start and next at edge end (previous and
// next being with respect to the clockwise order)
// you gotta be very careful with the join, has anything but the right one will fuck everything up
// see PathStroke.cpp for the "right" joins
int
Shape::MakeOffset (Shape * a, double dec, JoinType join, double miter)
{
  Reset (0, 0);
  if ( a->HasBackData() ) MakeBackData(true); else MakeBackData (false);
  if (dec == 0)
  {
    nbPt = a->nbPt;
    if (nbPt > maxPt)
    {
      maxPt = nbPt;
      pts = (dg_point *) g_realloc(pts, maxPt * sizeof (dg_point));
      if (HasPointsData ())
        pData =
          (point_data *) g_realloc(pData, maxPt * sizeof (point_data));
    }
    memcpy (pts, a->pts, nbPt * sizeof (dg_point));
    
    nbAr = a->nbAr;
    if (nbAr > maxAr)
    {
      maxAr = nbAr;
      aretes.reserve(maxAr);
      if (HasEdgesData ())
        eData = (edge_data *) g_realloc(eData, maxAr * sizeof (edge_data));
      if (HasSweepSrcData ())
        swsData =
          (sweep_src_data *) g_realloc(swsData,
                                      maxAr * sizeof (sweep_src_data));
      if (HasSweepDestData ())
        swdData =
          (sweep_dest_data *) g_realloc(swdData,
                                       maxAr * sizeof (sweep_dest_data));
      if (HasRasterData ())
        swrData =
          (raster_data *) g_realloc(swrData, maxAr * sizeof (raster_data));
      if (HasBackData ())
        ebData =
          (back_data *) g_realloc(ebData, maxAr * sizeof (back_data));
    }
    aretes = a->aretes;
    return 0;
  }
  if (a->nbPt <= 1 || a->nbAr <= 1 || a->type != shape_polygon)
    return shape_input_err;
  
  a->SortEdges ();
  
  a->MakeSweepDestData (true);
  a->MakeSweepSrcData (true);
  
  for (int i = 0; i < a->nbAr; i++)
  {
    //              int    stP=a->swsData[i].stPt/*,enP=a->swsData[i].enPt*/;
    int stB = -1, enB = -1;
    if (dec > 0)
    {
      stB = a->CycleNextAt (a->aretes[i].st, i);
      enB = a->CyclePrevAt (a->aretes[i].en, i);
    }
    else
    {
      stB = a->CyclePrevAt (a->aretes[i].st, i);
      enB = a->CycleNextAt (a->aretes[i].en, i);
    }
    
    NR::Point stD, seD, enD;
    double stL, seL, enL;
    stD = a->aretes[stB].dx;
    seD = a->aretes[i].dx;
    enD = a->aretes[enB].dx;
    
    stL = sqrt (dot(stD,stD));
    seL = sqrt (dot(seD,seD));
    enL = sqrt (dot(enD,enD));
    MiscNormalize (stD);
    MiscNormalize (enD);
    MiscNormalize (seD);
    
    NR::Point ptP;
    int stNo, enNo;
    ptP = a->pts[a->aretes[i].st].x;
    int   usePathID=-1;
    int   usePieceID=0;
    double useT=0.0;
    if ( a->HasBackData() ) {
      if ( a->ebData[i].pathID >= 0 && a->ebData[stB].pathID == a->ebData[i].pathID && a->ebData[stB].pieceID == a->ebData[i].pieceID
           && a->ebData[stB].tEn == a->ebData[i].tSt ) {
        usePathID=a->ebData[i].pathID;
        usePieceID=a->ebData[i].pieceID;
        useT=a->ebData[i].tSt;
      } else {
        usePathID=a->ebData[i].pathID;
        usePieceID=0;
        useT=0;
      }
    }
    if (dec > 0)
    {
      Path::DoRightJoin (this, dec, join, ptP, stD, seD, miter, stL, seL,
                         stNo, enNo,usePathID,usePieceID,useT);
      a->swsData[i].stPt = enNo;
      a->swsData[stB].enPt = stNo;
    }
    else
    {
      Path::DoLeftJoin (this, -dec, join, ptP, stD, seD, miter, stL, seL,
                        stNo, enNo,usePathID,usePieceID,useT);
      a->swsData[i].stPt = enNo;
      a->swsData[stB].enPt = stNo;
    }
  }
  if (dec < 0)
  {
    for (int i = 0; i < nbAr; i++)
      Inverse (i);
  }
  if ( HasBackData() ) {
    for (int i = 0; i < a->nbAr; i++)
    {
      int nEd=AddEdge (a->swsData[i].stPt, a->swsData[i].enPt);
      ebData[nEd]=a->ebData[i];
    }
  } else {
    for (int i = 0; i < a->nbAr; i++)
    {
      AddEdge (a->swsData[i].stPt, a->swsData[i].enPt);
    }
  }
  a->MakeSweepSrcData (false);
  a->MakeSweepDestData (false);
  
  return 0;
}

// we found a contour, now reassemble the edges on it, instead of dumping them in the Path "dest" as a
// polyline. since it was a DFS, the precParc and suivParc make a nice doubly-linked list of the edges in
// the contour. the first and last edges of the contour are startBord and curBord
void
Shape::AddContour (Path * dest, int nbP, Path * *orig, int startBord, int curBord, bool splitWhenForced)
{
  int bord = startBord;
  
  {
    dest->MoveTo (pts[aretes[bord].st].x);
  }
  
  while (bord >= 0)
  {
    int nPiece = ebData[bord].pieceID;
    int nPath = ebData[bord].pathID;
    
    if (nPath < 0 || nPath >= nbP || orig[nPath] == NULL)
    {
      // segment batard
      dest->LineTo (pts[aretes[bord].en].x);
      bord = swdData[bord].suivParc;
    }
    else
    {
      Path *from = orig[nPath];
      if (nPiece < 0 || nPiece >= from->descr_nb)
	    {
	      // segment batard
	      dest->LineTo (pts[aretes[bord].en].x);
	      bord = swdData[bord].suivParc;
	    }
      else
	    {
	      int nType = from->descr_cmd[nPiece].flags & descr_type_mask;
	      if (nType == descr_close || nType == descr_moveto
            || nType == descr_forced)
        {
          // devrait pas arriver
          dest->LineTo (pts[aretes[bord].en].x);
          bord = swdData[bord].suivParc;
        }
	      else if (nType == descr_lineto)
        {
          bord = ReFormeLineTo (bord, curBord, dest, from);
        }
	      else if (nType == descr_arcto)
        {
          bord = ReFormeArcTo (bord, curBord, dest, from);
        }
	      else if (nType == descr_cubicto)
        {
          bord = ReFormeCubicTo (bord, curBord, dest, from);
        }
	      else if (nType == descr_bezierto)
        {
          Path::Path::path_descr_bezierto* nBData=(Path::Path::path_descr_bezierto*)(from->descr_data+from->descr_cmd[nPiece].dStart);
          if (nBData->nb == 0)
          {
            bord = ReFormeLineTo (bord, curBord, dest, from);
          }
          else
          {
            bord = ReFormeBezierTo (bord, curBord, dest, from);
          }
        }
	      else if (nType == descr_interm_bezier)
        {
          bord = ReFormeBezierTo (bord, curBord, dest, from);
        }
	      else
        {
          // devrait pas arriver non plus
          dest->LineTo (pts[aretes[bord].en].x);
          bord = swdData[bord].suivParc;
        }
	      if (bord >= 0 && pts[aretes[bord].st].dI + pts[aretes[bord].st].dO > 2 ) {
          dest->ForcePoint ();
        } else if ( bord >= 0 && pts[aretes[bord].st].oldDegree > 2 && pts[aretes[bord].st].dI + pts[aretes[bord].st].dO == 2)  {
          if ( splitWhenForced ) {
            // pour les coupures
            dest->ForcePoint ();
         } else {
            if ( HasBackData() ) {
              int   prevEdge=pts[aretes[bord].st].firstA;
              int   nextEdge=pts[aretes[bord].st].lastA;
              if ( aretes[prevEdge].en != aretes[bord].st ) {
                int  swai=prevEdge;prevEdge=nextEdge;nextEdge=swai;
              }
              if ( ebData[prevEdge].pieceID == ebData[nextEdge].pieceID  && ebData[prevEdge].pathID == ebData[nextEdge].pathID ) {
                if ( fabs(ebData[prevEdge].tEn-ebData[nextEdge].tSt) < 0.05 ) {
                } else {
                  dest->ForcePoint ();
                }
              } else {
                dest->ForcePoint ();
              }
            } else {
              dest->ForcePoint ();
            }    
          }
        }
      }
    }
  }
  dest->Close ();
}

int
Shape::ReFormeLineTo (int bord, int curBord, Path * dest, Path * orig)
{
  int nPiece = ebData[bord].pieceID;
  int nPath = ebData[bord].pathID;
  double /*ts=ebData[bord].tSt, */ te = ebData[bord].tEn;
  NR::Point nx = pts[aretes[bord].en].x;
  bord = swdData[bord].suivParc;
  while (bord >= 0)
  {
    if (pts[aretes[bord].st].dI + pts[aretes[bord].st].dO > 2
        || pts[aretes[bord].st].oldDegree > 2)
    {
      break;
    }
    if (ebData[bord].pieceID == nPiece && ebData[bord].pathID == nPath)
    {
      if (fabs (te - ebData[bord].tSt) > 0.0001)
        break;
      nx = pts[aretes[bord].en].x;
      te = ebData[bord].tEn;
    }
    else
    {
      break;
    }
    bord = swdData[bord].suivParc;
  }
  {
    dest->LineTo (nx);
  }
  return bord;
}

int
Shape::ReFormeArcTo (int bord, int curBord, Path * dest, Path * from)
{
  int nPiece = ebData[bord].pieceID;
  int nPath = ebData[bord].pathID;
  double ts = ebData[bord].tSt, te = ebData[bord].tEn;
  //      double  px=pts[aretes[bord].st].x,py=pts[aretes[bord].st].y;
  NR::Point nx = pts[aretes[bord].en].x;
  bord = swdData[bord].suivParc;
  while (bord >= 0)
  {
    if (pts[aretes[bord].st].dI + pts[aretes[bord].st].dO > 2
        || pts[aretes[bord].st].oldDegree > 2)
    {
      break;
    }
    if (ebData[bord].pieceID == nPiece && ebData[bord].pathID == nPath)
    {
      if (fabs (te - ebData[bord].tSt) > 0.0001)
	    {
	      break;
	    }
      nx = pts[aretes[bord].en].x;
      te = ebData[bord].tEn;
    }
    else
    {
      break;
    }
    bord = swdData[bord].suivParc;
  }
  double sang, eang;
  Path::path_descr_arcto* nData=(Path::path_descr_arcto*)(from->descr_data+from->descr_cmd[nPiece].dStart);
  bool nLarge = nData->large;
  bool nClockwise = nData->clockwise;
  Path::ArcAngles (from->PrevPoint (nPiece - 1), nData->p,nData->rx,nData->ry,nData->angle, nLarge, nClockwise,  sang, eang);
  if (nClockwise)
  {
    if (sang < eang)
      sang += 2 * M_PI;
  }
  else
  {
    if (sang > eang)
      sang -= 2 * M_PI;
  }
  double delta = eang - sang;
  double ndelta = delta * (te - ts);
  if (ts > te)
    nClockwise = !nClockwise;
  if (ndelta < 0)
    ndelta = -ndelta;
  if (ndelta > M_PI)
    nLarge = true;
  else
    nLarge = false;
  /*	if ( delta < 0 ) delta=-delta;
	if ( ndelta < 0 ) ndelta=-ndelta;
	if ( ( delta < M_PI && ndelta < M_PI ) || ( delta >= M_PI && ndelta >= M_PI ) ) {
		if ( ts < te ) {
		} else {
			nClockwise=!(nClockwise);
		}
	} else {
    //		nLarge=!(nLarge);
		nLarge=false; // c'est un sous-segment -> l'arc ne peut que etre plus petit
		if ( ts < te ) {
		} else {
			nClockwise=!(nClockwise);
		}
	}*/
  {
    Path::path_descr_arcto* nData=(Path::path_descr_arcto*)(from->descr_data+from->descr_cmd[nPiece].dStart);
    dest->ArcTo (nx, nData->rx,nData->ry,nData->angle, nLarge, nClockwise);
  }
  return bord;
}

int
Shape::ReFormeCubicTo (int bord, int curBord, Path * dest, Path * from)
{
  int nPiece = ebData[bord].pieceID;
  int nPath = ebData[bord].pathID;
  double ts = ebData[bord].tSt, te = ebData[bord].tEn;
  NR::Point nx = pts[aretes[bord].en].x;
  bord = swdData[bord].suivParc;
  while (bord >= 0)
  {
    if (pts[aretes[bord].st].dI + pts[aretes[bord].st].dO > 2
        || pts[aretes[bord].st].oldDegree > 2)
    {
      break;
    }
    if (ebData[bord].pieceID == nPiece && ebData[bord].pathID == nPath)
    {
      if (fabs (te - ebData[bord].tSt) > 0.0001)
	    {
	      break;
	    }
      nx = pts[aretes[bord].en].x;
      te = ebData[bord].tEn;
    }
    else
    {
      break;
    }
    bord = swdData[bord].suivParc;
  }
  NR::Point prevx = from->PrevPoint (nPiece - 1);
  
  NR::Point sDx, eDx;
  {
    Path::path_descr_cubicto* nData=(Path::path_descr_cubicto*)(from->descr_data+from->descr_cmd[nPiece].dStart);
    Path::CubicTangent (ts, sDx, prevx,nData->stD,nData->p,nData->enD);
    Path::CubicTangent (te, eDx, prevx,nData->stD,nData->p,nData->enD);
  }
  sDx *= (te - ts);
  eDx *= (te - ts);
  {
    dest->CubicTo (nx,sDx,eDx);
  }
  return bord;
}

int
Shape::ReFormeBezierTo (int bord, int curBord, Path * dest, Path * from)
{
  int nPiece = ebData[bord].pieceID;
  int nPath = ebData[bord].pathID;
  double ts = ebData[bord].tSt, te = ebData[bord].tEn;
  int ps = nPiece, pe = nPiece;
  NR::Point px = pts[aretes[bord].st].x;
  NR::Point nx = pts[aretes[bord].en].x;
  int inBezier = -1, nbInterm = -1;
  int typ;
  typ = from->descr_cmd[nPiece].flags & descr_type_mask;
  Path::path_descr_bezierto *nBData = NULL;
  if (typ == descr_bezierto)
  {
    nBData=(Path::path_descr_bezierto*)(from->descr_data+from->descr_cmd[nPiece].dStart);
    inBezier = nPiece;
    nbInterm = nBData->nb;
  }
  else
  {
    int n = nPiece - 1;
    while (n > 0)
    {
      typ = from->descr_cmd[n].flags & descr_type_mask;
      if (typ == descr_bezierto)
      {
        inBezier = n;
        nBData=(Path::path_descr_bezierto*)(from->descr_data+from->descr_cmd[n].dStart);
        nbInterm = nBData->nb;
        break;
      }
      n--;
    }
    if (inBezier < 0)
    {
      bord = swdData[bord].suivParc;
      dest->LineTo (nx);
      return bord;
    }
  }
  bord = swdData[bord].suivParc;
  while (bord >= 0)
  {
    if (pts[aretes[bord].st].dI + pts[aretes[bord].st].dO > 2
        || pts[aretes[bord].st].oldDegree > 2)
    {
      break;
    }
    if (ebData[bord].pathID == nPath)
    {
      if (ebData[bord].pieceID < inBezier
          || ebData[bord].pieceID >= inBezier + nbInterm)
        break;
      if (ebData[bord].pieceID == pe
          && fabs (te - ebData[bord].tSt) > 0.0001)
        break;
      if (ebData[bord].pieceID != pe
          && (ebData[bord].tSt > 0.0001 && ebData[bord].tSt < 0.9999))
        break;
      if (ebData[bord].pieceID != pe && (te > 0.0001 && te < 0.9999))
        break;
      nx = pts[aretes[bord].en].x;
      te = ebData[bord].tEn;
      pe = ebData[bord].pieceID;
    }
    else
    {
      break;
    }
    bord = swdData[bord].suivParc;
  }

  g_return_val_if_fail(nBData != NULL, 0);
  
  if (pe == ps)
  {
    ReFormeBezierChunk (px, nx, dest, inBezier, nbInterm, from, ps,
                        ts, te);
  }
  else if (ps < pe)
  {
    if (ts < 0.0001)
    {
      if (te > 0.9999)
      {
        dest->BezierTo (nx);
        for (int i = ps; i <= pe; i++)
        {
          Path::path_descr_intermbezierto* nData=(Path::path_descr_intermbezierto*)(from->descr_data+from->descr_cmd[i+1].dStart);
          dest->IntermBezierTo (nData->p);
        }
        dest->EndBezierTo ();
      }
      else
      {
        NR::Point tx;
        {
          Path::path_descr_intermbezierto* psData=(Path::path_descr_intermbezierto*)(from->descr_data+from->descr_cmd[pe].dStart);
          Path::path_descr_intermbezierto* pnData=(Path::path_descr_intermbezierto*)(from->descr_data+from->descr_cmd[pe+1].dStart);
          tx = (pnData->p + psData->p) / 2;
        }
        dest->BezierTo (tx);
        for (int i = ps; i < pe; i++)
        {
          Path::path_descr_intermbezierto* nData=(Path::path_descr_intermbezierto*)(from->descr_data+from->descr_cmd[i+1].dStart);
          dest->IntermBezierTo (nData->p);
        }
        dest->EndBezierTo ();
        ReFormeBezierChunk (tx, nx, dest, inBezier, nbInterm,
                            from, pe, 0.0, te);
      }
    }
    else
    {
      if (te > 0.9999)
      {
        NR::Point tx;
        {
          Path::path_descr_intermbezierto* psData=(Path::path_descr_intermbezierto*)(from->descr_data+from->descr_cmd[ps+1].dStart);
          Path::path_descr_intermbezierto* pnData=(Path::path_descr_intermbezierto*)(from->descr_data+from->descr_cmd[ps+2].dStart);
          tx = (psData->p +  pnData->p) / 2;
        }
        ReFormeBezierChunk (px, tx, dest, inBezier, nbInterm,
                            from, ps, ts, 1.0);
        dest->BezierTo (nx);
        for (int i = ps + 1; i <= pe; i++)
        {
          Path::path_descr_intermbezierto* nData=(Path::path_descr_intermbezierto*)(from->descr_data+from->descr_cmd[i+1].dStart);
          dest->IntermBezierTo (nData->p);
        }
        dest->EndBezierTo ();
      }
      else
      {
        NR::Point tx;
        {
          Path::path_descr_intermbezierto* psData=(Path::path_descr_intermbezierto*)(from->descr_data+from->descr_cmd[ps+1].dStart);
          Path::path_descr_intermbezierto* pnData=(Path::path_descr_intermbezierto*)(from->descr_data+from->descr_cmd[ps+2].dStart);
          tx = (pnData->p + psData->p) / 2;
        }
        ReFormeBezierChunk (px, tx, dest, inBezier, nbInterm,
                            from, ps, ts, 1.0);
        {
          Path::path_descr_intermbezierto* psData=(Path::path_descr_intermbezierto*)(from->descr_data+from->descr_cmd[pe].dStart);
          Path::path_descr_intermbezierto* pnData=(Path::path_descr_intermbezierto*)(from->descr_data+from->descr_cmd[pe+1].dStart);
          tx = (pnData->p + psData->p) / 2;
        }
         dest->BezierTo (tx);
        for (int i = ps + 1; i <= pe; i++)
        {
          Path::path_descr_intermbezierto* nData=(Path::path_descr_intermbezierto*)(from->descr_data+from->descr_cmd[i+1].dStart);
          dest->IntermBezierTo (nData->p);
        }
        dest->EndBezierTo ();
        ReFormeBezierChunk (tx, nx, dest, inBezier, nbInterm,
                            from, pe, 0.0, te);
      }
    }
  }
  else
  {
    if (ts > 0.9999)
    {
      if (te < 0.0001)
      {
        dest->BezierTo (nx);
        for (int i = ps; i >= pe; i--)
        {
          Path::path_descr_intermbezierto* nData=(Path::path_descr_intermbezierto*)(from->descr_data+from->descr_cmd[i+1].dStart);
          dest->IntermBezierTo (nData->p);
        }
        dest->EndBezierTo ();
      }
      else
      {
        NR::Point tx;
        {
          Path::path_descr_intermbezierto* psData=(Path::path_descr_intermbezierto*)(from->descr_data+from->descr_cmd[pe+1].dStart);
          Path::path_descr_intermbezierto* pnData=(Path::path_descr_intermbezierto*)(from->descr_data+from->descr_cmd[pe+2].dStart);
          tx = (pnData->p + psData->p) / 2;
        }
        dest->BezierTo (tx);
        for (int i = ps; i > pe; i--)
        {
          Path::path_descr_intermbezierto* nData=(Path::path_descr_intermbezierto*)(from->descr_data+from->descr_cmd[i+1].dStart);
          dest->IntermBezierTo (nData->p);
        }
        dest->EndBezierTo ();
        ReFormeBezierChunk (tx, nx, dest, inBezier, nbInterm,
                            from, pe, 1.0, te);
      }
    }
    else
    {
      if (te < 0.0001)
      {
        NR::Point tx;
        {
          Path::path_descr_intermbezierto* psData=(Path::path_descr_intermbezierto*)(from->descr_data+from->descr_cmd[ps].dStart);
          Path::path_descr_intermbezierto* pnData=(Path::path_descr_intermbezierto*)(from->descr_data+from->descr_cmd[ps+1].dStart);
          tx = (pnData->p + psData->p) / 2;
        }
         ReFormeBezierChunk (px, tx, dest, inBezier, nbInterm,
                            from, ps, ts, 0.0);
        dest->BezierTo (nx);
        for (int i = ps + 1; i >= pe; i--)
        {
          Path::path_descr_intermbezierto* nData=(Path::path_descr_intermbezierto*)(from->descr_data+from->descr_cmd[i].dStart);
          dest->IntermBezierTo (nData->p);
        }
        dest->EndBezierTo ();
      }
      else
      {
        NR::Point tx;
        {
          Path::path_descr_intermbezierto* psData=(Path::path_descr_intermbezierto*)(from->descr_data+from->descr_cmd[ps].dStart);
          Path::path_descr_intermbezierto* pnData=(Path::path_descr_intermbezierto*)(from->descr_data+from->descr_cmd[ps+1].dStart);
          tx = (pnData->p + psData->p) / 2;
        }
        ReFormeBezierChunk (px, tx, dest, inBezier, nbInterm,
                            from, ps, ts, 0.0);
        {
          Path::path_descr_intermbezierto* psData=(Path::path_descr_intermbezierto*)(from->descr_data+from->descr_cmd[pe+1].dStart);
          Path::path_descr_intermbezierto* pnData=(Path::path_descr_intermbezierto*)(from->descr_data+from->descr_cmd[pe+2].dStart);
          tx = (pnData->p + psData->p) / 2;
        }
        dest->BezierTo (tx);
        for (int i = ps + 1; i > pe; i--)
        {
          Path::path_descr_intermbezierto* nData=(Path::path_descr_intermbezierto*)(from->descr_data+from->descr_cmd[i].dStart);
          dest->IntermBezierTo (nData->p);
        }
        dest->EndBezierTo ();
        ReFormeBezierChunk (tx, nx, dest, inBezier, nbInterm,
                            from, pe, 1.0, te);
      }
    }
  }
  return bord;
}

void
Shape::ReFormeBezierChunk (NR::Point px, NR::Point nx,
                           Path * dest, int inBezier, int nbInterm,
                           Path * from, int p, double ts, double te)
{
  Path::path_descr_bezierto* nBData=(Path::path_descr_bezierto*)(from->descr_data+from->descr_cmd[inBezier].dStart);
  NR::Point bstx = from->PrevPoint (inBezier - 1);
  NR::Point benx = nBData->p;
  
  NR::Point mx;
  if (p == inBezier)
  {
    // premier bout
    if (nbInterm <= 1)
    {
      // seul bout de la spline
      Path::path_descr_intermbezierto* nData=(Path::path_descr_intermbezierto*)(from->descr_data+from->descr_cmd[inBezier+1].dStart);
      mx = nData->p;
    }
    else
    {
      // premier bout d'une spline qui en contient plusieurs
      Path::path_descr_intermbezierto* nData=(Path::path_descr_intermbezierto*)(from->descr_data+from->descr_cmd[inBezier+1].dStart);
      mx = nData->p;
      nData=(Path::path_descr_intermbezierto*)(from->descr_data+from->descr_cmd[inBezier+2].dStart);
      benx = (nData->p + mx) / 2;
    }
  }
  else if (p == inBezier + nbInterm - 1)
  {
    // dernier bout
    // si nbInterm == 1, le cas a deja ete traite
    // donc dernier bout d'une spline qui en contient plusieurs
    Path::path_descr_intermbezierto* nData=(Path::path_descr_intermbezierto*)(from->descr_data+from->descr_cmd[inBezier+nbInterm].dStart);
    mx = nData->p;
   nData=(Path::path_descr_intermbezierto*)(from->descr_data+from->descr_cmd[inBezier+nbInterm-1].dStart);
    bstx = (nData->p + mx) / 2;
  }
  else
  {
    // la spline contient forcément plusieurs bouts, et ce n'est ni le premier ni le dernier
    Path::path_descr_intermbezierto* nData=(Path::path_descr_intermbezierto*)(from->descr_data+from->descr_cmd[p+1].dStart);
    mx = nData->p;
   nData=(Path::path_descr_intermbezierto*)(from->descr_data+from->descr_cmd[p].dStart);
    bstx = (nData->p + mx) / 2;
    nData=(Path::path_descr_intermbezierto*)(from->descr_data+from->descr_cmd[p+2].dStart);
    benx = (nData->p + mx) / 2;
  }
  NR::Point cx;
  {
    Path::QuadraticPoint ((ts + te) / 2, cx, bstx, mx, benx);
  }
  cx = 2 * cx - (px + nx) / 2;
  {
    dest->BezierTo (nx);
    dest->IntermBezierTo (cx);
    dest->EndBezierTo ();
  }
}

#undef MiscNormalize
