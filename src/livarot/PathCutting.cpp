/*
 *  PathCutting.cpp
 *  nlivarot
 *
 *  Created by fred on someday in 2004.
 *  public domain
 *
 */

#include "Path.h"
#include "../libnr/nr-point-fns.h"
#include "../libnr/nr-point-ops.h"
#include "../libnr/nr-matrix-ops.h"

/** Returns ceil( (double) numerator / denominator ). */
static inline unsigned roundup_div(unsigned const numerator, unsigned const denominator)
{
	g_assert( denominator != 0 );
	unsigned const ret = ( numerator == 0
                         ? 0
                         : 1 + ( numerator - 1 ) / denominator );
	g_assert( numerator == 0 && ret == 0
            || ( ret != 0
                 && ( ret - 1 ) * denominator < numerator
                 && numerator / ret <= denominator ) );
	return ret;
}

/** Returns the number of NR::Point's worth of space required for the path_descr_blah type
corresponding to \a typ.
*/
static inline unsigned SizeForData(int const typ)
{
	/* impl: inline because always called with a constant, so the compiler should be able to
	 * evaluate the function at compile time. */
	size_t res;
  
	switch (typ) {
    case descr_moveto:        res = sizeof(Path::path_descr_moveto); break;
    case descr_lineto:        res = sizeof(Path::path_descr_lineto); break;
    case descr_cubicto:       res = sizeof(Path::path_descr_cubicto); break;
    case descr_arcto:         res = sizeof(Path::path_descr_arcto); break;
    case descr_bezierto:      res = sizeof(Path::path_descr_bezierto); break;
    case descr_interm_bezier: res = sizeof(Path::path_descr_intermbezierto); break;
      
    case descr_close:
    case descr_forced:
    default:
      res = 0;
      break;
	}
  
	return roundup_div(res, sizeof(NR::Point));
}

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
double      Path::Surface(void)
{
  double   surf=0;
  if ( nbPt <= 0 ) return 0;
  NR::Point  lastM;
  NR::Point  lastP;
  if ( back ) {
    lastM=((path_lineto_b *) pts)[0].p;
  } else {
    lastM=((path_lineto *) pts)[0].p;
  }
  lastP=lastM;
  for (int i=0;i<nbPt;i++) {
    path_lineto  cur;
    if ( back ) {
      cur.p=((path_lineto_b *) pts)[i].p;
      cur.isMoveTo=((path_lineto_b *) pts)[i].isMoveTo;
    } else {
      cur.p=((path_lineto *) pts)[i].p;
      cur.isMoveTo=((path_lineto *) pts)[i].isMoveTo;
    }
    if ( cur.isMoveTo == polyline_moveto ) {
      surf+=NR::cross(lastM-lastP,lastM);
      lastP=lastM=cur.p;
    } else {
      surf+=NR::cross(cur.p-lastP,cur.p);
      lastP=cur.p;
    }
  }
  
  return surf;
}
Path**      Path::SubPaths(int &outNb,bool killNoSurf)
{
  int      nbRes=0;
  Path**   res=NULL;
  Path*    curAdd=NULL;
  
  for (int i=0;i<descr_nb;i++) {
    int typ=descr_cmd[i].flags&descr_type_mask;
    switch ( typ ) {
      case descr_moveto:
        if ( curAdd ) {
          if ( curAdd->descr_nb > 1 ) {
            curAdd->Convert(1.0);
            double addSurf=curAdd->Surface();
            if ( fabs(addSurf) > 0.0001 || killNoSurf == false ) {
              res=(Path**)realloc(res,(nbRes+1)*sizeof(Path*));
              res[nbRes++]=curAdd;
            } else { 
              delete curAdd;
            }
          } else {
            delete curAdd;
          }
          curAdd=NULL;
        }
        curAdd=new Path;
        curAdd->SetBackData(false);
        {
          path_descr_moveto *nData = reinterpret_cast<path_descr_moveto *>( descr_data + descr_cmd[i].dStart );
          curAdd->MoveTo(nData->p);
        }
          break;
      case descr_close:
      {
        curAdd->Close();
      }
        break;        
      case descr_lineto:
      {
        path_descr_lineto *nData = reinterpret_cast<path_descr_lineto *>( descr_data + descr_cmd[i].dStart );
        curAdd->LineTo(nData->p);
      }
        break;
      case descr_cubicto:
      {
        path_descr_cubicto *nData = reinterpret_cast<path_descr_cubicto *>( descr_data + descr_cmd[i].dStart );
        curAdd->CubicTo(nData->p,nData->stD,nData->enD);
      }
        break;
      case descr_arcto:
      {
        path_descr_arcto *nData = reinterpret_cast<path_descr_arcto *>( descr_data + descr_cmd[i].dStart );
        curAdd->ArcTo(nData->p,nData->rx,nData->ry,nData->angle,nData->large,nData->clockwise);
      }
        break;
      case descr_bezierto:
      {
        path_descr_bezierto *nData = reinterpret_cast<path_descr_bezierto *>( descr_data + descr_cmd[i].dStart );
        curAdd->BezierTo(nData->p);
      }
        break;
      case descr_interm_bezier:
      {
        path_descr_intermbezierto *nData = reinterpret_cast<path_descr_intermbezierto *>( descr_data + descr_cmd[i].dStart );
        curAdd->IntermBezierTo(nData->p);
      }
        break;
      default:
        break;
    }
  }
  if ( curAdd ) {
    if ( curAdd->descr_nb > 1 ) {
      curAdd->Convert(1.0);
      double addSurf=curAdd->Surface();
      if ( fabs(addSurf) > 0.0001 || killNoSurf == false  ) {
        res=(Path**)realloc(res,(nbRes+1)*sizeof(Path*));
        res[nbRes++]=curAdd;
      } else {
        delete curAdd;
      }
    } else {
      delete curAdd;
    }
  }
  curAdd=NULL;
  
  outNb=nbRes;
  return res;
}
Path**      Path::SubPathsWithNesting(int &outNb,bool killNoSurf,int nbNest,int* nesting,int* conts)
{
  int      nbRes=0;
  Path**   res=NULL;
  Path*    curAdd=NULL;
  bool     increment=false;
  
  for (int i=0;i<descr_nb;i++) {
    int typ=descr_cmd[i].flags&descr_type_mask;
    switch ( typ ) {
      case descr_moveto:
      {
        if ( curAdd && increment == false ) {
          if ( curAdd->descr_nb > 1 ) {
            // sauvegarder descr_cmd[0].associated
            int savA=curAdd->descr_cmd[0].associated;
            curAdd->Convert(1.0);
            curAdd->descr_cmd[0].associated=savA; // associated n'est pas utilise apres
            double addSurf=curAdd->Surface();
            if ( fabs(addSurf) > 0.0001 || killNoSurf == false ) {
              res=(Path**)realloc(res,(nbRes+1)*sizeof(Path*));
              res[nbRes++]=curAdd;
            } else { 
              delete curAdd;
            }
          } else {
            delete curAdd;
          }
          curAdd=NULL;
        }
        Path*  hasDad=NULL;
        for (int j=0;j<nbNest;j++) {
          if ( conts[j] == i && nesting[j] >= 0 ) {
            int  dadMvt=conts[nesting[j]];
            for (int k=0;k<nbRes;k++) {
              if ( res[k] && res[k]->descr_nb > 0 && res[k]->descr_cmd[0].associated == dadMvt ) {
                hasDad=res[k];
                break;
              }
            }
          }
          if ( conts[j] > i  ) break;
        }
        if ( hasDad ) {
          curAdd=hasDad;
          increment=true;
        } else {
          curAdd=new Path;
          curAdd->SetBackData(false);
          increment=false;
        }
        path_descr_moveto *nData = reinterpret_cast<path_descr_moveto *>( descr_data + descr_cmd[i].dStart );
        int mNo=curAdd->MoveTo(nData->p);
        curAdd->descr_cmd[mNo].associated=i;
        }
        break;
      case descr_close:
      {
        curAdd->Close();
      }
        break;        
      case descr_lineto:
      {
        path_descr_lineto *nData = reinterpret_cast<path_descr_lineto *>( descr_data + descr_cmd[i].dStart );
        curAdd->LineTo(nData->p);
      }
        break;
      case descr_cubicto:
      {
        path_descr_cubicto *nData = reinterpret_cast<path_descr_cubicto *>( descr_data + descr_cmd[i].dStart );
        curAdd->CubicTo(nData->p,nData->stD,nData->enD);
      }
        break;
      case descr_arcto:
      {
        path_descr_arcto *nData = reinterpret_cast<path_descr_arcto *>( descr_data + descr_cmd[i].dStart );
        curAdd->ArcTo(nData->p,nData->rx,nData->ry,nData->angle,nData->large,nData->clockwise);
      }
        break;
      case descr_bezierto:
      {
        path_descr_bezierto *nData = reinterpret_cast<path_descr_bezierto *>( descr_data + descr_cmd[i].dStart );
        curAdd->BezierTo(nData->p);
      }
        break;
      case descr_interm_bezier:
      {
        path_descr_intermbezierto *nData = reinterpret_cast<path_descr_intermbezierto *>( descr_data + descr_cmd[i].dStart );
        curAdd->IntermBezierTo(nData->p);
      }
        break;
      default:
        break;
    }
  }
  if ( curAdd && increment == false ) {
    if ( curAdd->descr_nb > 1 ) {
      curAdd->Convert(1.0);
      double addSurf=curAdd->Surface();
      if ( fabs(addSurf) > 0.0001 || killNoSurf == false  ) {
        res=(Path**)realloc(res,(nbRes+1)*sizeof(Path*));
        res[nbRes++]=curAdd;
      } else {
        delete curAdd;
      }
    } else {
      delete curAdd;
    }
  }
  curAdd=NULL;
  
  outNb=nbRes;
  return res;
}
void        Path::ConvertForcedToMoveTo(void)
{  
  NR::Point  lastSeen;
  NR::Point  lastMove;
  lastSeen[0]=lastSeen[1]=0;
  lastMove=lastSeen;
  bool       hasMoved=false;
    
  {
    int  lastPos=ddata_nb;
    for (int i=descr_nb-1;i>=0;i--) {
      int typ=descr_cmd[i].flags&descr_type_mask;
      switch ( typ ) {
        case descr_forced:
        case descr_close:
          descr_cmd[i].dStart=lastPos;
          break;
        case descr_moveto:
        case descr_lineto:
        case descr_arcto:
        case descr_cubicto:
        case descr_bezierto:
        case descr_interm_bezier:
          lastPos=descr_cmd[i].dStart;
          break;
        default:
          break;
      }
    }
  }
  for (int i=0;i<descr_nb;i++) {
    int typ=descr_cmd[i].flags&descr_type_mask;
    switch ( typ ) {
      case descr_forced:
      if ( i < descr_nb-1 && hasMoved ) { // sinon il termine le chemin
        // decale la suite d'un moveto
        int dataPos=descr_cmd[i+1].dStart;
        int  add=SizeForData(descr_moveto);
        AlloueDData(add);
        if ( dataPos < ddata_nb ) memmove(descr_data+(dataPos+add),descr_data+dataPos,(ddata_nb-dataPos)*sizeof(NR::Point));
        ddata_nb+=add;
        for (int j=i+1;j<descr_nb;j++) descr_cmd[j].dStart+=add;
        descr_cmd[i].dStart=dataPos;
        descr_cmd[i].flags&=~descr_type_mask;
        descr_cmd[i].flags|=descr_moveto;
        path_descr_moveto *nData = reinterpret_cast<path_descr_moveto *>( descr_data + descr_cmd[i].dStart );
        nData->p=lastSeen;
        
        lastMove=nData->p;
        hasMoved=true;
     }
        break;
      case descr_moveto:
        {
          path_descr_moveto *nData = reinterpret_cast<path_descr_moveto *>( descr_data + descr_cmd[i].dStart );
          lastMove=lastSeen=nData->p;
          hasMoved=true;
        }
          break;
      case descr_close:
      {
        lastSeen=lastMove;
      }
        break;        
      case descr_lineto:
      {
        path_descr_lineto *nData = reinterpret_cast<path_descr_lineto *>( descr_data + descr_cmd[i].dStart );
        lastSeen=nData->p;
      }
        break;
      case descr_cubicto:
      {
        path_descr_cubicto *nData = reinterpret_cast<path_descr_cubicto *>( descr_data + descr_cmd[i].dStart );
        lastSeen=nData->p;
     }
        break;
      case descr_arcto:
      {
        path_descr_arcto *nData = reinterpret_cast<path_descr_arcto *>( descr_data + descr_cmd[i].dStart );
        lastSeen=nData->p;
      }
        break;
      case descr_bezierto:
      {
        path_descr_bezierto *nData = reinterpret_cast<path_descr_bezierto *>( descr_data + descr_cmd[i].dStart );
        lastSeen=nData->p;
     }
        break;
      case descr_interm_bezier:
      {
        path_descr_intermbezierto *nData = reinterpret_cast<path_descr_intermbezierto *>( descr_data + descr_cmd[i].dStart );
        lastSeen=nData->p;
      }
        break;
      default:
        break;
    }
  }
}
static int       CmpPosition(const void * p1, const void * p2) {
  Path::cut_position *cp1=(Path::cut_position*)p1;
  Path::cut_position *cp2=(Path::cut_position*)p2;
  if ( cp1->piece < cp2->piece ) return -1;
  if ( cp1->piece > cp2->piece ) return 1;
  if ( cp1->t < cp2->t ) return -1;
  if ( cp1->t > cp2->t ) return 1;
  return 0;
}

void        Path::ConvertPositionsToMoveTo(int nbPos,cut_position* poss)
{
  if ( nbPos <= 0 )  return;
  {
    int  lastPos=ddata_nb;
    for (int i=descr_nb-1;i>=0;i--) {
      int typ=descr_cmd[i].flags&descr_type_mask;
      switch ( typ ) {
        case descr_forced:
          descr_cmd[i].dStart=lastPos;
          break;
        case descr_close:
        {
          int  add=SizeForData(descr_lineto);
          ShiftDData(lastPos,add);
          
          descr_cmd[i].dStart=lastPos; // dStart a ete changé par shift
          descr_cmd[i].flags&=~descr_type_mask;
          descr_cmd[i].flags|=descr_lineto;
          path_descr_lineto *nData = reinterpret_cast<path_descr_lineto *>( descr_data + descr_cmd[i].dStart );
          int fp=i-1;
          while ( fp >= 0 && (descr_cmd[fp].flags&descr_type_mask) != descr_moveto ) fp--;
          if ( fp >= 0 ) {
            path_descr_lineto *oData = reinterpret_cast<path_descr_lineto *>( descr_data + descr_cmd[fp].dStart );
            nData->p=oData->p;
          }
        }
          break;
        case descr_bezierto:
        {
          path_descr_bezierto *nData = reinterpret_cast<path_descr_bezierto *>( descr_data + descr_cmd[i].dStart );
          NR::Point  theP=nData->p;
          if ( nData->nb == 0 ) {
            int  add=SizeForData(descr_lineto)-SizeForData(descr_bezierto);
            ShiftDData(lastPos,add);
                        
            path_descr_lineto *nDatal = reinterpret_cast<path_descr_lineto *>( descr_data + descr_cmd[i].dStart );
            nDatal->p=theP;
            lastPos=descr_cmd[i].dStart;
          }
        }
          break;
        case descr_moveto:
        case descr_lineto:
        case descr_arcto:
        case descr_cubicto:
        case descr_interm_bezier:
          lastPos=descr_cmd[i].dStart;
          break;
        default:
          break;
      }
    }
  }
  

  qsort(poss, nbPos, sizeof(cut_position), CmpPosition);
  
  for (int curP=0;curP<nbPos;curP++) {
    int   cp=poss[curP].piece;
    if ( cp < 0 || cp >= descr_nb ) break;
    float ct=poss[curP].t;
    if ( ct < 0 ) continue;
    if ( ct > 1 ) continue;
    
    int typ=descr_cmd[cp].flags&descr_type_mask;
    if ( typ == descr_moveto || typ == descr_forced || typ == descr_close ) {
      // ponctuel= rien a faire
    } else if ( typ == descr_lineto || typ == descr_arcto || typ == descr_cubicto ) {
      // facile: creation d'un morceau et d'un moveto -> 2 commandes
      NR::Point        theP;
      NR::Point        theT;
      NR::Point        startP;
      startP=PrevPoint(cp-1);
      if ( typ == descr_cubicto ) {
        double           len,rad;
        NR::Point        stD,enD,endP;
        {
          path_descr_cubicto *oData = reinterpret_cast<path_descr_cubicto *>( descr_data + descr_cmd[cp].dStart );
          stD=oData->stD;
          enD=oData->enD;
          endP=oData->p;
          TangentOnCubAt (ct, startP, *oData,true, theP,theT,len,rad);
        }
        
        theT*=len;
        
        InsertCubicTo(endP,(1-ct)*theT,(1-ct)*enD,cp+1);
        InsertMoveTo(theP,cp+1);
        {
          path_descr_cubicto *nData = reinterpret_cast<path_descr_cubicto *>( descr_data + descr_cmd[cp].dStart );
          nData->stD=ct*stD;
          nData->enD=ct*theT;
          nData->p=theP;
        }
        for (int j=curP+1;j<nbPos;j++) {
          if ( poss[j].piece == cp ) {
            poss[j].piece+=2;
            poss[j].t=(poss[j].t-ct)/(1-ct);
          } else {
            poss[j].piece+=2;
          }
        }
      } else if ( typ == descr_lineto ) {
        NR::Point        endP;
        {
          path_descr_lineto *oData = reinterpret_cast<path_descr_lineto *>( descr_data + descr_cmd[cp].dStart );
          endP=oData->p;
        }
        
        theP=ct*endP+(1-ct)*startP;
        
        InsertLineTo(endP,cp+1);
        InsertMoveTo(theP,cp+1);
        {
          path_descr_lineto *nData = reinterpret_cast<path_descr_lineto *>( descr_data + descr_cmd[cp].dStart );
          nData->p=theP;
        }
        for (int j=curP+1;j<nbPos;j++) {
          if ( poss[j].piece == cp ) {
            poss[j].piece+=2;
            poss[j].t=(poss[j].t-ct)/(1-ct);
          } else {
            poss[j].piece+=2;
          }
        }
        
      } else if ( typ == descr_arcto ) {
        NR::Point        endP;
        double           rx,ry,angle;
        bool             clockw,large;
        double   delta=0;
        {
          path_descr_arcto *oData = reinterpret_cast<path_descr_arcto *>( descr_data + descr_cmd[cp].dStart );
          endP=oData->p;
          rx=oData->rx;
          ry=oData->ry;
          angle=oData->angle;
          clockw=oData->clockwise;
          large=oData->large;
        }
        {
          double      sang,eang;
          ArcAngles(startP,endP,rx,ry,angle,large,clockw,sang,eang);
          
          if (clockw) {
            if ( sang < eang ) sang += 2*M_PI;
            delta=eang-sang;
          } else {
            if ( sang > eang ) sang -= 2*M_PI;
            delta=eang-sang;
          }
          if ( delta < 0 ) delta=-delta;
        }
        
        PointAt (cp,ct, theP);
        
        if ( delta*(1-ct) > M_PI ) {
          InsertArcTo(endP,rx,ry,angle,true,clockw,cp+1);
        } else {
          InsertArcTo(endP,rx,ry,angle,false,clockw,cp+1);
        }
        InsertMoveTo(theP,cp+1);
        {
          path_descr_arcto *nData = reinterpret_cast<path_descr_arcto *>( descr_data + descr_cmd[cp].dStart );
          nData->p=theP;
          if ( delta*ct > M_PI ) {
            nData->clockwise=true;
          } else {
            nData->clockwise=false;
          }
        }
        for (int j=curP+1;j<nbPos;j++) {
          if ( poss[j].piece == cp ) {
            poss[j].piece+=2;
            poss[j].t=(poss[j].t-ct)/(1-ct);
          } else {
            poss[j].piece+=2;
          }
        }
        
      }
      
      
    } else if ( typ == descr_bezierto || typ == descr_interm_bezier ) {
      // dur
    }
  }
}



