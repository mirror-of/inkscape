#include "Path.h"
#include "../libnr/nr-point-fns.h"
#include "../libnr/nr-point-ops.h"
#include "../libnr/nr-matrix-ops.h"

void  Path::DashPolyline(float head,float tail,float body,int nbD,float *dashs,bool stPlain)
{
  if ( nbD <= 0 || body <= 0.0001 ) return; // pas de tirets, en fait
  
  int    origNb=nbPt;
  char*  origPts=pts;
  nbPt=maxPt=0;
  pts=NULL;
  
  int       lastMI=-1;
  char*     curP=origPts;
  char*     lastMP=NULL;
  
  for (int i=0;i<origNb;i++) {
    int   typ;
    if ( back ) {
      typ=((path_lineto_b*)curP)->isMoveTo;
    } else {
      typ=((path_lineto*)curP)->isMoveTo;
    }
    if ( typ == polyline_moveto ) {
      if ( lastMI >= 0 && lastMI < i-1 ) { // au moins 2 points
        DashSubPath(i-lastMI,lastMP,head,tail,body,nbD,dashs,stPlain);
      }
      lastMI=i;
      lastMP=curP;
    } else if ( typ == polyline_forced ) {
    } else {
    }
    if ( back ) {
      curP+=sizeof(path_lineto_b);
    } else {
      curP+=sizeof(path_lineto);
    }
  }
  if ( lastMI >= 0 && lastMI < origNb-1 ) {
    DashSubPath(origNb-lastMI,lastMP,head,tail,body,nbD,dashs,stPlain);
  }
  
  if ( back ) {
    curP+=sizeof(path_lineto_b);
  } else {
    curP+=sizeof(path_lineto);
  }
  free(origPts);
  
/*  for (int i=0;i<nbPt;i++) {
    NR::Point  np;
    int        nm;
    if ( back ) np=((path_lineto_b*)pts)[i].p; else np=((path_lineto*)pts)[i].p;
    if ( back ) nm=((path_lineto_b*)pts)[i].isMoveTo; else nm=((path_lineto*)pts)[i].isMoveTo;
    printf("(%f %f  %i) ",np[0],np[1],nm);
  }
  printf("\n");*/
}



void Path::DashSubPath(int spL,char* spP,float head,float tail,float body,int nbD,float *dashs,bool stPlain)
{
/*  printf("%f [%f : %i ",head,body,nbD);
  for (int i=0;i<nbD;i++) printf("%f ",dashs[i]);
  printf("]* %f\n",tail);
  for (int i=0;i<spL;i++) {
    NR::Point  np;
    int        nm;
    if ( back ) np=((path_lineto_b*)spP)[i].p; else np=((path_lineto*)spP)[i].p;
    if ( back ) nm=((path_lineto_b*)spP)[i].isMoveTo; else nm=((path_lineto*)spP)[i].isMoveTo;
    printf("(%f %f  %i) ",np[0],np[1],nm);
  }
  printf("\n");*/

  if ( spL <= 0 || spP == NULL ) return;
  
  double      totLength=0;
  NR::Point   lastP;
  if ( back ) {
    lastP=((path_lineto_b*)spP)[0].p;
  } else {
    lastP=((path_lineto*)spP)[0].p;
  }
  for (int i=1;i<spL;i++) {
    NR::Point   n;
    if ( back ) {
      n=((path_lineto_b*)spP)[i].p;
    } else {
      n=((path_lineto*)spP)[i].p;
    }
    NR::Point d=n-lastP;
    double    nl=NR::L2(d);
    if ( nl > 0.0001 ) {
      totLength+=nl;
      lastP=n;
    }
  }
  
  if ( totLength <= head+tail ) return; // tout mange par la tete et la queue
  
  double    curLength=0;
  double    dashPos=0;
  int       dashInd=0;
  bool      dashPlain=false;
  double    lastT=0;
  int       lastPiece=-1;
  if ( back ) {
    lastP=((path_lineto_b*)spP)[0].p;
  } else {
    lastP=((path_lineto*)spP)[0].p;
  }
  for (int i=1;i<spL;i++) {
    NR::Point   n;
    int         nPiece=-1;
    double      nT=0;
    if ( back ) {
      n=((path_lineto_b*)spP)[i].p;
      nPiece=((path_lineto_b*)spP)[i].piece;
      nT=((path_lineto_b*)spP)[i].t;
    } else {
      n=((path_lineto*)spP)[i].p;
    }
    NR::Point d=n-lastP;
    double    nl=NR::L2(d);
    if ( nl > 0.0001 ) {
      double   stLength=curLength;
      double   enLength=curLength+nl;
      // couper les bouts en trop
      if ( curLength <= head && curLength+nl > head ) {
        nl-=head-curLength;
        curLength=head;
        dashInd=0;
        dashPos=0;
        bool nPlain=stPlain;
        if ( nPlain == true && dashPlain == false ) {
          NR::Point  p=(enLength-curLength)*lastP+(curLength-stLength)*n;
          p/=(enLength-stLength);
          if ( back ) {
            double pT=0;
            if ( nPiece == lastPiece ) {
              pT=(lastT*(enLength-curLength)+nT*(curLength-stLength))/(enLength-stLength);
            } else {
              pT=(nPiece*(curLength-stLength))/(enLength-stLength);
            }
            AddPoint(p,nPiece,pT,true);
          } else {
            AddPoint(p,true);
          }
        } else if ( nPlain == false && dashPlain == true ) {
        }
        dashPlain=nPlain;
      }
      if ( curLength <= totLength-tail && curLength+nl > totLength-tail ) {
        nl=totLength-tail-curLength;
        dashInd=0;
        dashPos=0;
        bool nPlain=false;
        if ( nPlain == true && dashPlain == false ) {
        } else if ( nPlain == false && dashPlain == true ) {
          NR::Point  p=(enLength-curLength)*lastP+(curLength-stLength)*n;
          p/=(enLength-stLength);
          if ( back ) {
            double pT=0;
            if ( nPiece == lastPiece ) {
              pT=(lastT*(enLength-curLength)+nT*(curLength-stLength))/(enLength-stLength);
            } else {
              pT=(nPiece*(curLength-stLength))/(enLength-stLength);
            }
            AddPoint(p,nPiece,pT,false);
          } else {
            AddPoint(p,false);
          }
        }
        dashPlain=nPlain;
      }
      // faire les tirets
      if ( curLength >= head && curLength+nl <= totLength-tail ) {
        while ( nl > 0 ) {
          double  leftInDash=body-dashPos;
          if ( dashInd < nbD ) {
            leftInDash=dashs[dashInd]-dashPos;
          }
          if ( leftInDash <= nl ) {
            bool nPlain=false;
            if ( dashInd < nbD ) {
              dashPos=dashs[dashInd];
              dashInd++;
              if ( dashPlain ) nPlain=false; else nPlain=true;
            } else {
              dashInd=0;
              dashPos=0;
              nPlain=stPlain;
            }
            if ( nPlain == true && dashPlain == false ) {
              NR::Point  p=(enLength-curLength-leftInDash)*lastP+(curLength+leftInDash-stLength)*n;
              p/=(enLength-stLength);
              if ( back ) {
                double pT=0;
                if ( nPiece == lastPiece ) {
                  pT=(lastT*(enLength-curLength-leftInDash)+nT*(curLength+leftInDash-stLength))/(enLength-stLength);
                } else {
                  pT=(nPiece*(curLength+leftInDash-stLength))/(enLength-stLength);
                }
                AddPoint(p,nPiece,pT,true);
              } else {
                AddPoint(p,true);
              }
            } else if ( nPlain == false && dashPlain == true ) {
              NR::Point  p=(enLength-curLength-leftInDash)*lastP+(curLength+leftInDash-stLength)*n;
              p/=(enLength-stLength);
              if ( back ) {
                double pT=0;
                if ( nPiece == lastPiece ) {
                  pT=(lastT*(enLength-curLength-leftInDash)+nT*(curLength+leftInDash-stLength))/(enLength-stLength);
                } else {
                  pT=(nPiece*(curLength+leftInDash-stLength))/(enLength-stLength);
                }
                AddPoint(p,nPiece,pT,false);
              } else {
                AddPoint(p,false);
              }
            }
            dashPlain=nPlain;
            
            curLength+=leftInDash;
            nl-=leftInDash;
          } else {
            dashPos+=nl;
            curLength+=nl;
            nl=0;
          }
        }
        if ( dashPlain ) {
          if ( back ) {
            AddPoint(n,nPiece,nT,false);
          } else {
            AddPoint(n,false);
          }
        }
      }
      // continuer
      curLength=enLength;
      lastP=n;
      lastPiece=nPiece;
      lastT=nT;
    }
  }
}
#include "../helper/canvas-bpath.h"

void  Path::LoadArtBPath(void *iV,NR::Matrix &trans,bool doTransformation)
{
  if ( iV == NULL ) return;
  ArtBpath *bpath = (ArtBpath*)iV;
  
  SetBackData (false);
  Reset();
  {
    int   i;
    bool  closed = false;
    NR::Point lastX(0,0);
    
    for (i = 0; bpath[i].code != ART_END; i++)
    {
      switch (bpath[i].code)
      {
        case ART_LINETO:
          lastX[0] = bpath[i].x3;
          lastX[1] = bpath[i].y3;
          if ( doTransformation ) {
            lastX*=trans;
          }
            LineTo (lastX);
            break;
          
        case ART_CURVETO:
        {
          NR::Point  tmp,tms(0,0),tme(0,0),tm1,tm2;
          tmp[0]=bpath[i].x3;
          tmp[1]=bpath[i].y3;
          tm1[0]=bpath[i].x1;
          tm1[1]=bpath[i].y1;
          tm2[0]=bpath[i].x2;
          tm2[1]=bpath[i].y2;
          if ( doTransformation ) {
            tmp*=trans;
            tm1*=trans;
            tm2*=trans;
          }
          tms=3 * (tm1 - lastX);
          tme=3 * (tmp - tm2);
          CubicTo (tmp,tms,tme);
        }
          lastX[0] = bpath[i].x3;
          lastX[1] = bpath[i].y3;
          if ( doTransformation ) {
            lastX*=trans;
          }
          break;
          
        case ART_MOVETO_OPEN:
        case ART_MOVETO:
          if (closed) Close ();
          closed = (bpath[i].code == ART_MOVETO);
          lastX[0] = bpath[i].x3;
          lastX[1] = bpath[i].y3;
          if ( doTransformation ) {
            lastX*=trans;
          }
            MoveTo (lastX);
            break;
        default:
          break;
      }
    }
    if (closed) Close ();
  }
}


