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

void  Path::DashPolyline(float head,float tail,float body,int nbD,float *dashs,bool stPlain,float stOffset)
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
        DashSubPath(i-lastMI,lastMP,head,tail,body,nbD,dashs,stPlain,stOffset);
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
    DashSubPath(origNb-lastMI,lastMP,head,tail,body,nbD,dashs,stPlain,stOffset);
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



void Path::DashSubPath(int spL,char* spP,float head,float tail,float body,int nbD,float *dashs,bool stPlain,float stOffset)
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
        dashPos=stOffset;
        bool nPlain=stPlain;
        while ( dashs[dashInd] < stOffset ) {
          dashInd++;
          nPlain=!(nPlain);
          if ( dashInd >= nbD ) {
            dashPos=0;
            dashInd=0;
            break;
          }
        }
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
      // faire les tirets
      if ( curLength >= head /*&& curLength+nl <= totLength-tail*/ ) {
        while ( curLength <= totLength-tail && nl > 0 ) {
          if ( enLength <= totLength-tail ) nl=enLength-curLength; else nl=totLength-tail-curLength;
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
              //nPlain=stPlain;
              nPlain=dashPlain;
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
        nl=enLength-curLength;
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
      // continuer
      curLength=enLength;
      lastP=n;
      lastPiece=nPiece;
      lastT=nT;
    }
  }
}
#include "../helper/canvas-bpath.h"

void  Path::LoadArtBPath(void *iV,NR::Matrix const &trans,bool doTransformation)
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
double      Path::Length(void)
{
  double   len=0;
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
      lastP=lastM=cur.p;
    } else {
      len+=NR::L2(cur.p-lastP);
      lastP=cur.p;
    }
  }
  return len;
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
void        Path::ConvertForcedToVoid(void)
{  
  for (int i=0;i<descr_nb;i++) {
    int typ=descr_cmd[i].flags&descr_type_mask;
    switch ( typ ) {
      case descr_forced:
        ShiftDCmd(i,-1);
        break;
      case descr_moveto:
        break;
      case descr_close:
        break;        
      case descr_lineto:
        break;
      case descr_cubicto:
        break;
      case descr_arcto:
        break;
      case descr_bezierto:
        break;
      case descr_interm_bezier:
        break;
      default:
        break;
    }
  }
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
static int       CmpCurv(const void * p1, const void * p2) {
  double *cp1=(double*)p1;
  double *cp2=(double*)p2;
  if ( *cp1 < *cp2 ) return -1;
  if ( *cp1 > *cp2 ) return 1;
  return 0;
}
Path::cut_position*  Path::CurvilignToPosition(int nbCv,double* cvAbs,int &nbCut)
{
  if ( nbCv <= 0 ) return NULL;
  if ( nbPt <= 0 ) return NULL;
  if ( back == false ) return NULL;
  
  qsort(cvAbs, nbCv, sizeof(double), CmpCurv);
  
  cut_position *res=NULL;
  nbCut=0;
  int      curCv=0;
  
  double   len=0;
  NR::Point  lastM;
  NR::Point  lastP;
  double     lastT=0;
  lastM=((path_lineto_b *) pts)[0].p;
  
  lastP=lastM;
  for (int i=0;i<nbPt;i++) {
    path_lineto_b  cur;
    cur=((path_lineto_b *) pts)[i];
    if ( cur.isMoveTo == polyline_moveto ) {
      lastP=lastM=cur.p;
      lastT=cur.t;
    } else {
      double add=NR::L2(cur.p-lastP);
      double curPos=len,curAdd=add;
      while ( curAdd > 0.0001 && curCv < nbCv && curPos+curAdd >= cvAbs[curCv] ) {
        double theta=(cvAbs[curCv]-len)/add;
        res=(cut_position*)realloc(res,(nbCut+1)*sizeof(cut_position));
        res[nbCut].piece=cur.piece;
        res[nbCut].t=theta*cur.t+(1-theta)*lastT;
        nbCut++;
        curAdd-=cvAbs[curCv]-curPos;
        curPos=cvAbs[curCv];
        curCv++;
      }
      len+=add;
      lastP=cur.p;
      lastT=cur.t;
    }
  }
  return res;
}
int         Path::DataPosForAfter(int cmd)
{
  if ( cmd < 0 ) return 0;
  if ( cmd >= descr_nb-1 ) return ddata_nb;
  do {
    int ntyp=descr_cmd[cmd].flags&descr_type_mask;
    if ( ntyp == descr_moveto || ntyp == descr_lineto || ntyp == descr_cubicto || ntyp == descr_arcto 
         || ntyp == descr_bezierto || ntyp == descr_interm_bezier ) {
      return descr_cmd[cmd].dStart;
    }
    cmd++;
  } while ( cmd < descr_nb );
  return ddata_nb;
}
void        Path::ConvertPositionsToForced(int nbPos,cut_position* poss)
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
          
          descr_cmd[i].dStart=lastPos; // dStart a ete changBŽ par shift
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
      // facile: creation d'un morceau et d'un forced -> 2 commandes
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
        InsertForcePoint(cp+1);
        {
          path_descr_cubicto *nData = reinterpret_cast<path_descr_cubicto *>( descr_data + descr_cmd[cp].dStart );
          nData->stD=ct*stD;
          nData->enD=ct*theT;
          nData->p=theP;
        }
        // decalages dans le tableau des positions de coupe
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
        InsertForcePoint(cp+1);
        {
          path_descr_lineto *nData = reinterpret_cast<path_descr_lineto *>( descr_data + descr_cmd[cp].dStart );
          nData->p=theP;
        }
        // decalages dans le tableau des positions de coupe
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
        InsertForcePoint(cp+1);
        {
          path_descr_arcto *nData = reinterpret_cast<path_descr_arcto *>( descr_data + descr_cmd[cp].dStart );
          nData->p=theP;
          if ( delta*ct > M_PI ) {
            nData->clockwise=true;
          } else {
            nData->clockwise=false;
          }
        }
        // decalages dans le tableau des positions de coupe
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
      int theBDI=cp;
      while ( theBDI >= 0 && (descr_cmd[theBDI].flags&descr_type_mask) != descr_bezierto ) theBDI--;
      if ( (descr_cmd[theBDI].flags&descr_type_mask) == descr_bezierto ) {
        path_descr_bezierto theBD=*(reinterpret_cast<path_descr_bezierto *>( descr_data + descr_cmd[theBDI].dStart ));
        if ( cp >= theBDI && cp < theBDI+theBD.nb ) {
          if ( theBD.nb == 1 ) {
            NR::Point        endP=theBD.p;
            NR::Point        midP;
            NR::Point        startP;
            startP=PrevPoint(theBDI-1);
            {
              path_descr_intermbezierto *nData = reinterpret_cast<path_descr_intermbezierto *>( descr_data + descr_cmd[theBDI+1].dStart );
              midP=nData->p;
            }
            NR::Point       aP=ct*midP+(1-ct)*startP;
            NR::Point       bP=ct*endP+(1-ct)*midP;
            NR::Point       knotP=ct*bP+(1-ct)*aP;
                        
            InsertIntermBezierTo(bP,theBDI+2);
            InsertBezierTo(knotP,1,theBDI+2);
            InsertForcePoint(theBDI+2);
            {
              path_descr_intermbezierto *nData = reinterpret_cast<path_descr_intermbezierto *>( descr_data + descr_cmd[theBDI+1].dStart );
              nData->p=aP;
            }
            {
              path_descr_bezierto *nData = reinterpret_cast<path_descr_bezierto *>( descr_data + descr_cmd[theBDI].dStart );
              nData->p=knotP;
            }
            // decalages dans le tableau des positions de coupe
            for (int j=curP+1;j<nbPos;j++) {
              if ( poss[j].piece == cp ) {
                poss[j].piece+=3;
                poss[j].t=(poss[j].t-ct)/(1-ct);
              } else {
                poss[j].piece+=3;
              }
            }
            
          } else {
            // decouper puis repasser
            if ( cp > theBDI ) {
              NR::Point   pcP,ncP;
              {
                path_descr_intermbezierto *nData = reinterpret_cast<path_descr_intermbezierto *>( descr_data + descr_cmd[cp].dStart );
                pcP=nData->p;
              }
              {
                path_descr_intermbezierto *nData = reinterpret_cast<path_descr_intermbezierto *>( descr_data + descr_cmd[cp+1].dStart );
                ncP=nData->p;
              }
              NR::Point knotP=0.5*(pcP+ncP);
              
              InsertBezierTo(knotP,theBD.nb-(cp-theBDI),cp+1);
              {
                path_descr_bezierto *nData = reinterpret_cast<path_descr_bezierto *>( descr_data + descr_cmd[theBDI].dStart );
                nData->nb=cp-theBDI;
              }
              
              // decalages dans le tableau des positions de coupe
              for (int j=curP;j<nbPos;j++) {
                if ( poss[j].piece == cp ) {
                  poss[j].piece+=1;
                } else {
                  poss[j].piece+=1;
                }
              }
              curP--;
            } else {
              NR::Point   pcP,ncP;
              {
                path_descr_intermbezierto *nData = reinterpret_cast<path_descr_intermbezierto *>( descr_data + descr_cmd[cp+1].dStart );
                pcP=nData->p;
              }
              {
                path_descr_intermbezierto *nData = reinterpret_cast<path_descr_intermbezierto *>( descr_data + descr_cmd[cp+2].dStart );
                ncP=nData->p;
              }
              NR::Point knotP=0.5*(pcP+ncP);
              
              InsertBezierTo(knotP,theBD.nb-1,cp+2);
              {
                path_descr_bezierto *nData = reinterpret_cast<path_descr_bezierto *>( descr_data + descr_cmd[theBDI].dStart );
                nData->nb=1;
              }
              
              // decalages dans le tableau des positions de coupe
              for (int j=curP;j<nbPos;j++) {
                if ( poss[j].piece == cp ) {
//                  poss[j].piece+=1;
                } else {
                  poss[j].piece+=1;
                }
              }
              curP--;
            }
          }
        } else {
          // on laisse aussi tomber
        }
      } else {
        // on laisse tomber
      }
    }
  }
}

void        Path::ConvertPositionsToMoveTo(int nbPos,cut_position* poss)
{
  ConvertPositionsToForced(nbPos,poss);
//  ConvertForcedToMoveTo();
  // on fait une version customizee a la place
  
  Path*  res=new Path;
  
  NR::Point    lastP(0,0);
  for (int i=0;i<descr_nb;i++) {
    int typ=descr_cmd[i].flags&descr_type_mask;
    if ( typ == descr_moveto ) {
      NR::Point  np;
      {
        path_descr_moveto *nData = reinterpret_cast<path_descr_moveto *>( descr_data + descr_cmd[i].dStart );
        np=nData->p;
      }
      NR::Point  endP;
      bool       hasClose=false;
      int        hasForced=-1;
      bool       doesClose=false;
      int        j=i+1;
      for (;j<descr_nb;j++) {
        int ntyp=descr_cmd[j].flags&descr_type_mask;
        if ( ntyp == descr_moveto ) {
          j--;
          break;
        } else if ( ntyp == descr_forced ) {
          if ( hasForced < 0 ) hasForced=j;
        } else if ( ntyp == descr_close ) {
          hasClose=true;
          break;
        } else if ( ntyp == descr_lineto ) {
          path_descr_lineto *nData = reinterpret_cast<path_descr_lineto *>( descr_data + descr_cmd[j].dStart );
          endP=nData->p;
        } else if ( ntyp == descr_arcto ) {
          path_descr_arcto *nData = reinterpret_cast<path_descr_arcto *>( descr_data + descr_cmd[j].dStart );
          endP=nData->p;
        } else if ( ntyp == descr_cubicto ) {
          path_descr_cubicto *nData = reinterpret_cast<path_descr_cubicto *>( descr_data + descr_cmd[j].dStart );
          endP=nData->p;
        } else if ( ntyp == descr_bezierto ) {
          path_descr_bezierto *nData = reinterpret_cast<path_descr_bezierto *>( descr_data + descr_cmd[j].dStart );
          endP=nData->p;
        } else {
        }
      }
      if ( NR::LInfty(endP-np) < 0.00001 ) {
        doesClose=true;
      }
      if ( ( doesClose || hasClose ) && hasForced >= 0 ) {
 //       printf("nasty i=%i j=%i frc=%i\n",i,j,hasForced);
        // aghhh.
        NR::Point   nMvtP=PrevPoint(hasForced);
        res->MoveTo(nMvtP);
        NR::Point   nLastP=nMvtP;
        for (int k=hasForced+1;k<=j;k++) {
          int ntyp=descr_cmd[k].flags&descr_type_mask;
          if ( ntyp == descr_moveto ) {
            // ne doit pas arriver
          } else if ( ntyp == descr_forced ) {
            res->MoveTo(nLastP);
          } else if ( ntyp == descr_close ) {
            // rien a faire ici; de plus il ne peut y en avoir qu'un
          } else if ( ntyp == descr_lineto ) {
            path_descr_lineto *nData = reinterpret_cast<path_descr_lineto *>( descr_data + descr_cmd[k].dStart );
            res->LineTo(nData->p);
            nLastP=nData->p;
          } else if ( ntyp == descr_arcto ) {
            path_descr_arcto *nData = reinterpret_cast<path_descr_arcto *>( descr_data + descr_cmd[k].dStart );
            res->ArcTo(nData->p,nData->rx,nData->ry,nData->angle,nData->large,nData->clockwise);
            nLastP=nData->p;
          } else if ( ntyp == descr_cubicto ) {
            path_descr_cubicto *nData = reinterpret_cast<path_descr_cubicto *>( descr_data + descr_cmd[k].dStart );
            res->CubicTo(nData->p,nData->stD,nData->enD);
            nLastP=nData->p;
          } else if ( ntyp == descr_bezierto ) {
            path_descr_bezierto *nData = reinterpret_cast<path_descr_bezierto *>( descr_data + descr_cmd[k].dStart );
            res->BezierTo(nData->p);
            nLastP=nData->p;
          } else if ( ntyp == descr_interm_bezier ) {
            path_descr_intermbezierto *nData = reinterpret_cast<path_descr_intermbezierto *>( descr_data + descr_cmd[k].dStart );
            res->IntermBezierTo(nData->p);
          } else {
          }
        }
        if ( doesClose == false ) res->LineTo(np);
        nLastP=np;
        for (int k=i+1;k<hasForced;k++) {
          int ntyp=descr_cmd[k].flags&descr_type_mask;
          if ( ntyp == descr_moveto ) {
            // ne doit pas arriver
          } else if ( ntyp == descr_forced ) {
            res->MoveTo(nLastP);
          } else if ( ntyp == descr_close ) {
            // rien a faire ici; de plus il ne peut y en avoir qu'un
          } else if ( ntyp == descr_lineto ) {
            path_descr_lineto *nData = reinterpret_cast<path_descr_lineto *>( descr_data + descr_cmd[k].dStart );
            res->LineTo(nData->p);
            nLastP=nData->p;
          } else if ( ntyp == descr_arcto ) {
            path_descr_arcto *nData = reinterpret_cast<path_descr_arcto *>( descr_data + descr_cmd[k].dStart );
            res->ArcTo(nData->p,nData->rx,nData->ry,nData->angle,nData->large,nData->clockwise);
            nLastP=nData->p;
          } else if ( ntyp == descr_cubicto ) {
            path_descr_cubicto *nData = reinterpret_cast<path_descr_cubicto *>( descr_data + descr_cmd[k].dStart );
            res->CubicTo(nData->p,nData->stD,nData->enD);
            nLastP=nData->p;
          } else if ( ntyp == descr_bezierto ) {
            path_descr_bezierto *nData = reinterpret_cast<path_descr_bezierto *>( descr_data + descr_cmd[k].dStart );
            res->BezierTo(nData->p);
            nLastP=nData->p;
          } else if ( ntyp == descr_interm_bezier ) {
            path_descr_intermbezierto *nData = reinterpret_cast<path_descr_intermbezierto *>( descr_data + descr_cmd[k].dStart );
            res->IntermBezierTo(nData->p);
          } else {
          }
        }
        lastP=nMvtP;
        i=j;
      } else {
        // regular, just move on
        res->MoveTo(np);
        lastP=np;
      }
    } else if ( typ == descr_close ) {
      res->Close();
    } else if ( typ == descr_forced ) {
      res->MoveTo(lastP);
    } else if ( typ == descr_lineto ) {
      path_descr_lineto *nData = reinterpret_cast<path_descr_lineto *>( descr_data + descr_cmd[i].dStart );
      res->LineTo(nData->p);
      lastP=nData->p;
    } else if ( typ == descr_arcto ) {
      path_descr_arcto *nData = reinterpret_cast<path_descr_arcto *>( descr_data + descr_cmd[i].dStart );
      res->ArcTo(nData->p,nData->rx,nData->ry,nData->angle,nData->large,nData->clockwise);
      lastP=nData->p;
    } else if ( typ == descr_cubicto ) {
      path_descr_cubicto *nData = reinterpret_cast<path_descr_cubicto *>( descr_data + descr_cmd[i].dStart );
      res->CubicTo(nData->p,nData->stD,nData->enD);
      lastP=nData->p;
    } else if ( typ == descr_bezierto ) {
      path_descr_bezierto *nData = reinterpret_cast<path_descr_bezierto *>( descr_data + descr_cmd[i].dStart );
      res->BezierTo(nData->p);
      lastP=nData->p;
    } else if ( typ == descr_interm_bezier ) {
      path_descr_intermbezierto *nData = reinterpret_cast<path_descr_intermbezierto *>( descr_data + descr_cmd[i].dStart );
      res->IntermBezierTo(nData->p);
    } else {
    }
  }
  
  Copy(res);
  delete res;
  return;
}



