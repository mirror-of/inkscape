/*
 *  PathConversion.cpp
 *  nlivarot
 *
 *  Created by fred on Mon Nov 03 2003.
 *
 */

#include "Path.h"
#include "Shape.h"
//#include "MyMath.h"

#include <libnr/nr-point-fns.h>
#include <libnr/nr-matrix.h>
#include <libnr/nr-rotate-ops.h>
#include <libnr/nr-scale-ops.h>

/*
 * path description -> polyline
 * and Path -> Shape (the Fill() function at the bottom)
 * nathing fancy here: take each command and append an approximation of it to the polyline
 */

void            Path::ConvertWithBackData(double treshhold)
{
	if ( descr_flags & descr_adding_bezier ) {
		CancelBezier();
	}
	if ( descr_flags & descr_doing_subpath ) {
		CloseSubpath();
	}
	SetBackData(true);
	ResetPoints(descr_nb);
	if ( descr_nb <= 0 ) return;
	NR::Point curX;
	int       curP=1;
	int       lastMoveTo=-1;
	
	// le moveto
  {
    int firstTyp=descr_cmd[0].flags&descr_type_mask;
    if ( firstTyp == descr_moveto ) {
      curX=((path_descr_moveto*)(descr_data))->p;
    } else {
      curP=0;
      curX[0]=curX[1]=0;
    }
    lastMoveTo=AddPoint(curX,0,0.0,true);
  }
	// et le reste, 1 par 1
	while ( curP < descr_nb ) {
		path_descr*  curD=descr_cmd+curP;
		int          nType=curD->flags&descr_type_mask;
		NR::Point    nextX;
		if ( nType == descr_forced ) {
			AddForcedPoint(curX,curP,1.0);
			curP++;
		} else if ( nType == descr_moveto ) {
      path_descr_moveto*  nData=(path_descr_moveto*)(descr_data+curD->dStart);
			nextX=nData->p;
			lastMoveTo=AddPoint(nextX,curP,0.0,true);
			// et on avance
			curP++;
		} else if ( nType == descr_close ) {
      nextX=((path_lineto_b*)pts)[lastMoveTo].p;
      AddPoint(nextX,curP,1.0,false);
			curP++;
		} else if ( nType == descr_lineto ) {
      path_descr_lineto*  nData=(path_descr_lineto*)(descr_data+curD->dStart);
			nextX=nData->p;
			AddPoint(nextX,curP,1.0,false);
			// et on avance
			curP++;
		} else if ( nType == descr_cubicto ) {
      path_descr_cubicto*  nData=(path_descr_cubicto*)(descr_data+curD->dStart);
			nextX=nData->p;
      RecCubicTo(curX,nData->stD,nextX,nData->enD,treshhold,8,0.0,1.0,curP);
      AddPoint(nextX,curP,1.0,false);
			// et on avance
			curP++;
		} else if ( nType == descr_arcto ) {
      path_descr_arcto*  nData=(path_descr_arcto*)(descr_data+curD->dStart);
			nextX=nData->p;
      DoArc(curX,nextX,nData->rx,nData->ry,nData->angle,nData->large,nData->clockwise,treshhold,curP);
      AddPoint(nextX,curP,1.0,false);
			// et on avance
			curP++;
		} else if ( nType == descr_bezierto ) {
      path_descr_bezierto*  nBData=(path_descr_bezierto*)(descr_data+curD->dStart);
			int   nbInterm=nBData->nb;
			nextX=nBData->p;
			
			curD=descr_cmd+(curP+1);
			path_descr* intermPoints=curD;
      path_descr_intermbezierto*  nData=(path_descr_intermbezierto*)(descr_data+intermPoints->dStart);
			
			if ( nbInterm <= 0 ) {
			} else if ( nbInterm >= 1 ) {
				NR::Point   bx=curX;
				NR::Point   cx=curX;
				NR::Point   dx=curX;
        
				dx=nData->p;
				intermPoints++;
        nData=(path_descr_intermbezierto*)(descr_data+intermPoints->dStart);
        
				cx=2*bx-dx;
        
				for (int k=0;k<nbInterm-1;k++) {
					bx=cx;
					cx=dx;
          
					dx=nData->p;
					intermPoints++;
					nData=(path_descr_intermbezierto*)(descr_data+intermPoints->dStart);	
          
					NR::Point  stx;
					stx=(bx+cx)/2;
					if ( k > 0 ) {
						AddPoint(stx,curP-1+k,1.0,false);
					}
          
          {
						NR::Point mx;
						mx=(cx+dx)/2;
						RecBezierTo(cx,stx,mx,treshhold,8,0.0,1.0,curP+k);
					}
				}
				{
					bx=cx;
					cx=dx;
          
					dx=nextX;
					dx=2*dx-cx;
          
					NR::Point  stx;
					stx=(bx+cx)/2;
          
					if ( nbInterm > 1 ) {
						AddPoint(stx,curP+nbInterm-2,1.0,false);
					}
          
					{
						NR::Point mx;
						mx=(cx+dx)/2;
						RecBezierTo(cx,stx,mx,treshhold,8,0.0,1.0,curP+nbInterm-1);
					}
				}
        
			}
			
			
			AddPoint(nextX,curP-1+nbInterm,1.0,false);
			
			// et on avance
			curP+=1+nbInterm;
		}
		curX=nextX;
	}
}
void            Path::ConvertForOffset(double treshhold,Path* orig,double off_dec)
{
	if ( descr_flags & descr_adding_bezier ) {
		CancelBezier();
	}
	if ( descr_flags & descr_doing_subpath ) {
		CloseSubpath();
	}
	SetBackData(true);
	ResetPoints(descr_nb);
	if ( descr_nb <= 0 ) return;
	NR::Point curX;
	int       curP=1;
	int       lastMoveTo=-1;
	
	// le moveto
  {
    int firstTyp=descr_cmd[0].flags&descr_type_mask;
    if ( firstTyp == descr_moveto ) {
      curX=((path_descr_moveto*)(descr_data))->p;
    } else {
      curP=0;
      curX[0]=curX[1]=0;
    }
    lastMoveTo=AddPoint(curX,0,0.0,true);
  }
	
	offset_orig     off_data;
	off_data.orig=orig;
	off_data.off_dec=off_dec;
	
	// et le reste, 1 par 1
	while ( curP < descr_nb ) {
		path_descr*  curD=descr_cmd+curP;
		int          nType=curD->flags&descr_type_mask;
		NR::Point    nextX;
		if ( nType == descr_forced ) {
			AddForcedPoint(curX,curP,1.0);
			curP++;
		} else if ( nType == descr_moveto ) {
      path_descr_moveto*  nData=(path_descr_moveto*)(descr_data+curD->dStart);
			nextX=nData->p;
			lastMoveTo=AddPoint(nextX,curP,0.0,true);
			// et on avance
			curP++;
		} else if ( nType == descr_close ) {
			nextX=((path_lineto_b*)pts)[lastMoveTo].p;
			AddPoint(nextX,curP,1.0,false);
			curP++;
		} else if ( nType == descr_lineto ) {
      path_descr_lineto*  nData=(path_descr_lineto*)(descr_data+curD->dStart);
			nextX=nData->p;
			AddPoint(nextX,curP,1.0,false);
			// et on avance
			curP++;
		} else if ( nType == descr_cubicto ) {
      path_descr_cubicto*  nData=(path_descr_cubicto*)(descr_data+curD->dStart);
			nextX=nData->p;
			off_data.piece=curD->associated;
			off_data.tSt=curD->tSt;
			off_data.tEn=curD->tEn;
			if ( curD->associated >= 0 ) {
				RecCubicTo(curX,nData->stD,nextX,nData->enD,treshhold,8,0.0,1.0,curP,off_data);
			} else {
				RecCubicTo(curX,nData->stD,nextX,nData->enD,treshhold,8,0.0,1.0,curP);
			}
			AddPoint(nextX,curP,1.0,false);
			// et on avance
			curP++;
		} else if ( nType == descr_arcto ) {
      path_descr_arcto*  nData=(path_descr_arcto*)(descr_data+curD->dStart);
			nextX=nData->p;
      off_data.piece=curD->associated;
			off_data.tSt=curD->tSt;
			off_data.tEn=curD->tEn;
			if ( curD->associated >= 0 ) {
				DoArc(curX,nextX,nData->rx,nData->ry,nData->angle,nData->large,nData->clockwise,treshhold,curP,off_data);
			} else {
				DoArc(curX,nextX,nData->rx,nData->ry,nData->angle,nData->large,nData->clockwise,treshhold,curP);
			}
			AddPoint(nextX,curP,1.0,false);
			// et on avance
			curP++;
		} else if ( nType == descr_bezierto ) {
			// on ne devrait jamais avoir de bezier quadratiques dans les offsets
			// mais bon, par precaution...
      path_descr_bezierto*  nBData=(path_descr_bezierto*)(descr_data+curD->dStart);
			int   nbInterm=nBData->nb;
			nextX=nBData->p;
			
			curD=descr_cmd+(curP+1);
			path_descr* intermPoints=curD;
      path_descr_intermbezierto*  nData=(path_descr_intermbezierto*)(descr_data+intermPoints->dStart);
			
			if ( nbInterm <= 0 ) {
			} else if ( nbInterm >= 1 ) {
				NR::Point   bx=curX;
				NR::Point   cx=curX;
				NR::Point   dx=curX;
        
				dx=nData->p;
				intermPoints++;
        nData=(path_descr_intermbezierto*)(descr_data+intermPoints->dStart);
        
				cx=2*bx-dx;
        
				for (int k=0;k<nbInterm-1;k++) {
					bx=cx;
					cx=dx;
          
					dx=nData->p;
					intermPoints++;
          nData=(path_descr_intermbezierto*)(descr_data+intermPoints->dStart);
          
					NR::Point  stx;
					stx=(bx+cx)/2;
          //						double  stw=(bw+cw)/2;
					if ( k > 0 ) {
						AddPoint(stx,curP-1+k,1.0,false);
					}
          
					off_data.piece=intermPoints->associated;
					off_data.tSt=intermPoints->tSt;
					off_data.tEn=intermPoints->tEn;
					if ( intermPoints->associated >= 0 ) {
						NR::Point mx;
						mx=(cx+dx)/2;
						RecBezierTo(cx,stx,mx,treshhold,8,0.0,1.0,curP+k,off_data);
					} else {
						NR::Point mx;
						mx=(cx+dx)/2;
						RecBezierTo(cx,stx,mx,treshhold,8,0.0,1.0,curP+k);
					}
				}
				{
					bx=cx;
					cx=dx;
          
					dx=nextX;
					dx=2*dx-cx;
          
					NR::Point  stx;
					stx=(bx+cx)/2;
          //						double  stw=(bw+cw)/2;
          
					if ( nbInterm > 1 ) {
						AddPoint(stx,curP+nbInterm-2,1.0,false);
					}
          
					off_data.piece=curD->associated;
					off_data.tSt=curD->tSt;
					off_data.tEn=curD->tEn;
					if ( curD->associated >= 0 ) {
						NR::Point mx;
						mx=(cx+dx)/2;
						RecBezierTo(cx,stx,mx,treshhold,8,0.0,1.0,curP+nbInterm-1,off_data);
					} else {
						NR::Point mx;
						mx=(cx+dx)/2;
						RecBezierTo(cx,stx,mx,treshhold,8,0.0,1.0,curP+nbInterm-1);
					}
				}
        
			}
			
			
			AddPoint(nextX,curP-1+nbInterm,1.0,false);
			
			// et on avance
			curP+=1+nbInterm;
		}
		curX=nextX;
	}
}
void            Path::Convert(double treshhold)
{
	if ( descr_flags & descr_adding_bezier ) {
		CancelBezier();
	}
	if ( descr_flags & descr_doing_subpath ) {
		CloseSubpath();
	}
	SetBackData(false);
	ResetPoints(descr_nb);
	if ( descr_nb <= 0 ) return;
	NR::Point curX;
	int       curP=1;
	int       lastMoveTo=0;
	
	// le moveto
  {
    int firstTyp=descr_cmd[0].flags&descr_type_mask;
    if ( firstTyp == descr_moveto ) {
      curX=((path_descr_moveto*)(descr_data))->p;
    } else {
      curP=0;
      curX[0]=curX[1]=0;
    }
    lastMoveTo=AddPoint(curX,true);
  }
	(descr_cmd)->associated=lastMoveTo;
	
	// et le reste, 1 par 1
	while ( curP < descr_nb ) {
		path_descr*  curD=descr_cmd+curP;
		int          nType=curD->flags&descr_type_mask;
		NR::Point    nextX;
		if ( nType == descr_forced ) {
			(curD)->associated=AddForcedPoint(curX);
			curP++;
		} else if ( nType == descr_moveto ) {
      path_descr_moveto*  nData=(path_descr_moveto*)(descr_data+curD->dStart);
			nextX=nData->p;
			lastMoveTo=AddPoint(nextX,true);
			curD->associated=lastMoveTo;
			
			// et on avance
			curP++;
		} else if ( nType == descr_close ) {
      nextX=((path_lineto*)pts)[lastMoveTo].p;
      curD->associated=AddPoint(nextX,false);
      if ( curD->associated < 0 ) {
        if ( curP == 0 ) {
          curD->associated=0;
        } else {
          curD->associated=(curD-1)->associated;
        }
      }
			curP++;
		} else if ( nType == descr_lineto ) {
      path_descr_lineto*  nData=(path_descr_lineto*)(descr_data+curD->dStart);
			nextX=nData->p;
			curD->associated=AddPoint(nextX,false);
			if ( curD->associated < 0 ) {
				if ( curP == 0 ) {
					curD->associated=0;
				} else {
					curD->associated=(curD-1)->associated;
				}
			}
			// et on avance
			curP++;
		} else if ( nType == descr_cubicto ) {
      path_descr_cubicto*  nData=(path_descr_cubicto*)(descr_data+curD->dStart);
			nextX=nData->p;
      RecCubicTo(curX,nData->stD,nextX,nData->enD,treshhold,8);
      curD->associated=AddPoint(nextX,false);
      if ( curD->associated < 0 ) {
        if ( curP == 0 ) {
          curD->associated=0;
        } else {
          curD->associated=(curD-1)->associated;
        }
      }
			// et on avance
			curP++;
		} else if ( nType == descr_arcto ) {
      path_descr_arcto*  nData=(path_descr_arcto*)(descr_data+curD->dStart);
			nextX=nData->p;
      DoArc(curX,nextX,nData->rx,nData->ry,nData->angle,nData->large,nData->clockwise,treshhold);
      curD->associated=AddPoint(nextX,false);
      if ( curD->associated < 0 ) {
        if ( curP == 0 ) {
          curD->associated=0;
        } else {
          curD->associated=(curD-1)->associated;
        }
      }
			// et on avance
			curP++;
		} else if ( nType == descr_bezierto ) {
      path_descr_bezierto*  nBData=(path_descr_bezierto*)(descr_data+curD->dStart);
			int   nbInterm=nBData->nb;
			nextX=nBData->p;
			path_descr* curBD=curD;
			
			curP++;
			curD=descr_cmd+curP;
			path_descr* intermPoints=curD;
      path_descr_intermbezierto*  nData=(path_descr_intermbezierto*)(descr_data+intermPoints->dStart);
			
			if ( nbInterm <= 0 ) {
			} else if ( nbInterm == 1 ) {
				NR::Point  midX;
				midX=nData->p;
        RecBezierTo(midX,curX,nextX,treshhold,8);
			} else if ( nbInterm > 1 ) {
				NR::Point   bx=curX;
				NR::Point   cx=curX;
				NR::Point   dx=curX;
								
				dx=nData->p;
				intermPoints++;
        nData=(path_descr_intermbezierto*)(descr_data+intermPoints->dStart);
				
				cx=2*bx-dx;
				
				for (int k=0;k<nbInterm-1;k++) {
					bx=cx;
					cx=dx;
					
					dx=nData->p;
					intermPoints++;
          nData=(path_descr_intermbezierto*)(descr_data+intermPoints->dStart);
					
					NR::Point  stx;
					stx=(bx+cx)/2;
					if ( k > 0 ) {
						(intermPoints-2)->associated=AddPoint(stx,false);
						if ( (intermPoints-2)->associated < 0 ) {
							if ( curP == 0 ) {
								(intermPoints-2)->associated=0;
							} else {
								(intermPoints-2)->associated=(intermPoints-3)->associated;
							}
						}
					}
					
          {
						NR::Point mx;
						mx=(cx+dx)/2;
						RecBezierTo(cx,stx,mx,treshhold,8);
					}
				}
				{
					bx=cx;
					cx=dx;
					
					dx=nextX;
					dx=2*dx-cx;
					
					NR::Point  stx;
					stx=(bx+cx)/2;
					
					(intermPoints-1)->associated=AddPoint(stx,false);
					if ( (intermPoints-1)->associated < 0 ) {
						if ( curP == 0 ) {
							(intermPoints-1)->associated=0;
						} else {
							(intermPoints-1)->associated=(intermPoints-2)->associated;
						}
					}
					
					{
						NR::Point mx;
						mx=(cx+dx)/2;
						RecBezierTo(cx,stx,mx,treshhold,8);
					}
				}
			}
			curBD->associated=AddPoint(nextX,false);
			if ( (curBD)->associated < 0 ) {
				if ( curP == 0 ) {
					(curBD)->associated=0;
				} else {
					(curBD)->associated=(curBD-1)->associated;
				}
			}
			
			// et on avance
			curP+=nbInterm;
		}
		curX=nextX;
	}
}
void            Path::ConvertEvenLines(double treshhold)
{
	if ( descr_flags & descr_adding_bezier ) {
		CancelBezier();
	}
	if ( descr_flags & descr_doing_subpath ) {
		CloseSubpath();
	}
	SetBackData(false);
	ResetPoints(descr_nb);
	if ( descr_nb <= 0 ) return;
	NR::Point curX;
	int      curP=1;
	int      lastMoveTo=0;
	
	// le moveto
  {
    int firstTyp=descr_cmd[0].flags&descr_type_mask;
    if ( firstTyp == descr_moveto ) {
      curX=((path_descr_moveto*)(descr_data))->p;
    } else {
      curP=0;
      curX[0]=curX[1]=0;
    }
    lastMoveTo=AddPoint(curX,true);
  }
	(descr_cmd)->associated=lastMoveTo;
	
	// et le reste, 1 par 1
	while ( curP < descr_nb ) {
		path_descr*  curD=descr_cmd+curP;
		int          nType=curD->flags&descr_type_mask;
		NR::Point    nextX;
		if ( nType == descr_forced ) {
			(curD)->associated=AddForcedPoint(curX);
			curP++;
		} else if ( nType == descr_moveto ) {
      path_descr_moveto*  nData=(path_descr_moveto*)(descr_data+curD->dStart);
			nextX=nData->p;
			lastMoveTo=AddPoint(nextX,true);
			(curD)->associated=lastMoveTo;
			// et on avance
			curP++;
		} else if ( nType == descr_close ) {
      nextX=((path_lineto*)pts)[lastMoveTo].p;
      {
        NR::Point nexcur;
        nexcur=nextX-curX;
        const double segL=NR::L2(nexcur);
        if ( segL > treshhold ) {
          for (double i=treshhold;i<segL;i+=treshhold) {
            NR::Point  nX;
            nX=(segL-i)*curX+i*nextX;
            nX/=segL;
            AddPoint(nX);
          }
        }
      }
      curD->associated=AddPoint(nextX,false);
      if ( curD->associated < 0 ) {
        if ( curP == 0 ) {
          curD->associated=0;
        } else {
          curD->associated=(curD-1)->associated;
        }
      }
			curP++;
		} else if ( nType == descr_lineto ) {
      path_descr_lineto*  nData=(path_descr_lineto*)(descr_data+curD->dStart);
			nextX=nData->p;
      NR::Point nexcur = nextX-curX;
      const double segL = L2(nexcur);
      if ( segL > treshhold ) {
        for (double i=treshhold;i<segL;i+=treshhold) {
          NR::Point  nX=((segL-i)*curX+i*nextX)/segL;
          AddPoint(nX);
        }
      }
      curD->associated=AddPoint(nextX,false);
      if ( curD->associated < 0 ) {
        if ( curP == 0 ) {
          curD->associated=0;
        } else {
          curD->associated=(curD-1)->associated;
        }
      }
			// et on avance
			curP++;
		} else if ( nType == descr_cubicto ) {
      path_descr_cubicto*  nData=(path_descr_cubicto*)(descr_data+curD->dStart);
			nextX=nData->p;
      RecCubicTo(curX,nData->stD,nextX,nData->enD,treshhold,8,4*treshhold);
      curD->associated=AddPoint(nextX,false);
      if ( curD->associated < 0 ) {
        if ( curP == 0 ) {
          curD->associated=0;
        } else {
          curD->associated=(curD-1)->associated;
        }
      }
			// et on avance
			curP++;
		} else if ( nType == descr_arcto ) {
      path_descr_arcto*  nData=(path_descr_arcto*)(descr_data+curD->dStart);
			nextX=nData->p;
      DoArc(curX,nextX,nData->rx,nData->ry,nData->angle,nData->large,nData->clockwise,treshhold);
      curD->associated=AddPoint(nextX,false);
      if ( curD->associated < 0 ) {
        if ( curP == 0 ) {
          curD->associated=0;
        } else {
          curD->associated=(curD-1)->associated;
        }
      }
			// et on avance
			curP++;
		} else if ( nType == descr_bezierto ) {
      path_descr_bezierto*  nBData=(path_descr_bezierto*)(descr_data+curD->dStart);
			int   nbInterm=nBData->nb;
			nextX=nBData->p;
			path_descr*  curBD=curD;
			
			curP++;
			curD=descr_cmd+curP;
			path_descr* intermPoints=curD;
      path_descr_intermbezierto*  nData=(path_descr_intermbezierto*)(descr_data+intermPoints->dStart);
			
			if ( nbInterm <= 0 ) {
			} else if ( nbInterm == 1 ) {
				NR::Point  midX=nData->p;
        RecBezierTo(midX,curX,nextX,treshhold,8,4*treshhold);
			} else if ( nbInterm > 1 ) {
				NR::Point   bx=curX,cx=curX,dx=curX;
								
				dx=nData->p;
				intermPoints++;
        nData=(path_descr_intermbezierto*)(descr_data+intermPoints->dStart);
				
				cx=2*bx-dx;
				
				for (int k=0;k<nbInterm-1;k++) {
					bx=cx;
					cx=dx;
					
					dx=nData->p;
					intermPoints++;
          nData=(path_descr_intermbezierto*)(descr_data+intermPoints->dStart);
					
					NR::Point  stx = (bx+cx)/2;
					if ( k > 0 ) {
						(intermPoints-2)->associated=AddPoint(stx,false);
						if ( (intermPoints-2)->associated < 0 ) {
							if ( curP == 0 ) {
								(intermPoints-2)->associated=0;
							} else {
								(intermPoints-2)->associated=(intermPoints-3)->associated;
							}
						}
					}
					
					{
						const NR::Point mx=(cx+dx)/2;
						RecBezierTo(cx,stx,mx,treshhold,8,4*treshhold);
					}
				}
				{
					bx=cx;
					cx=dx;
					
					dx=nextX;
					dx=2*dx-cx;
					
					const NR::Point  stx = (bx+cx)/2;
					
					(intermPoints-1)->associated=AddPoint(stx,false);
					if ( (intermPoints-1)->associated < 0 ) {
						if ( curP == 0 ) {
							(intermPoints-1)->associated=0;
						} else {
							(intermPoints-1)->associated=(intermPoints-2)->associated;
						}
					}
					
					{
						const NR::Point mx = (cx+dx)/2;
						RecBezierTo(cx,stx,mx,treshhold,8,4*treshhold);
					}
				}
			}
			curBD->associated=AddPoint(nextX,false);
			if ( (curBD)->associated < 0 ) {
				if ( curP == 0 ) {
					(curBD)->associated=0;
				} else {
					(curBD)->associated=(curBD-1)->associated;
				}
			}
						
			// et on avance
			curP+=nbInterm;
		}
		if ( NR::LInfty(curX-nextX) > 0.00001 ) {
			curX=nextX;
		}
	}
}

const NR::Point Path::PrevPoint(int i) const
{
	/* TODO: I suspect this should assert `(unsigned) i < descr_nb'.  We can probably change
	   the argument to unsigned.  descr_nb should probably be changed to unsigned too. */
	g_assert( i >= 0 );
	switch ( descr_cmd[i].flags & descr_type_mask ) {
		case descr_moveto: {
			path_descr_moveto *nData=(path_descr_moveto*)(descr_data+descr_cmd[i].dStart);
			return nData->p;
		}
		case descr_lineto: {
			path_descr_lineto *nData=(path_descr_lineto*)(descr_data+descr_cmd[i].dStart);
			return nData->p;
		}
		case descr_arcto: {
			path_descr_arcto *nData=(path_descr_arcto*)(descr_data+descr_cmd[i].dStart);
			return nData->p;
		}
		case descr_cubicto: {
			path_descr_cubicto *nData=(path_descr_cubicto*)(descr_data+descr_cmd[i].dStart);
			return nData->p;
		}
		case descr_bezierto: {
			path_descr_bezierto *nData=(path_descr_bezierto*)(descr_data+descr_cmd[i].dStart);
			return nData->p;
		}
		case descr_interm_bezier:
		case descr_close:
		case descr_forced:
			return PrevPoint(i-1);
		default:
			g_assert_not_reached();
			return descr_data[descr_cmd[i].dStart];
	}
}

// utilitaries: given a quadratic bezier curve (start point, control point, end point, ie that's a clamped curve),
// and an abcissis on it, get the point with that abcissis.
// warning: it's NOT a curvilign abcissis (or whatever you call that in english), so "t" is NOT the length of "start point"->"result point"
void Path::QuadraticPoint(double t, NR::Point &oPt, 
                          const NR::Point &iS, const NR::Point &iM, const NR::Point &iE)
{
	NR::Point ax = iE-2*iM+iS;
	NR::Point bx = 2*iM-2*iS;
	NR::Point cx = iS;
	
	oPt = t*t*ax+t*bx+cx;
}
// idem for cubic bezier patch
void Path::CubicTangent(double t, NR::Point &oPt, const NR::Point &iS, const NR::Point &isD, const NR::Point &iE, const NR::Point &ieD)
{
	NR::Point ax = ieD-2*iE+2*iS+isD;
	NR::Point bx = 3*iE-ieD-2*isD-3*iS;
	NR::Point cx = isD;
	NR::Point dx = iS;
	
	oPt = 3*t*t*ax+2*t*bx+cx;
}

// extract interesting info of a SVG arc description
static void ArcAnglesAndCenter(NR::Point const &iS, NR::Point const &iE,
			       double rx, double ry, double angle,
			       bool large, bool wise,
			       double &sang, double &eang, NR::Point &dr);

void Path::ArcAngles( const NR::Point &iS, const NR::Point &iE,double rx,double ry,double angle,bool large,bool wise,double &sang,double &eang)
{
	NR::Point  dr;
	ArcAnglesAndCenter(iS,iE,rx,ry,angle,large,wise,sang,eang,dr);
}

/* N.B. If iS == iE then sang,eang,dr each become NaN.  Probably a bug. */
static void ArcAnglesAndCenter(NR::Point const &iS, NR::Point const &iE,
                              double rx, double ry, double angle,
                              bool large, bool wise,
                              double &sang, double &eang, NR::Point &dr)
{
	NR::Point se = iE-iS;
	NR::Point ca(cos(angle),sin(angle));
	NR::Point cse(dot(se,ca),cross(se,ca));
	cse[0] /= rx;
	cse[1] /= ry;
	double const lensq = dot(cse,cse);
	NR::Point csd = ( ( lensq < 4
			    ? sqrt( 1/lensq - .25 )
			    : 0.0 )
			  * cse.ccw() );

	NR::Point ra;
	ra = -csd - 0.5 * cse;
	if ( ra[0] <= -1 ) {
		sang=M_PI;
	} else if ( ra[0] >= 1 ) {
		sang=0;
	} else {
		sang=acos(ra[0]);
		if ( ra[1] < 0 ) sang=2*M_PI-sang;
	}

	ra = -csd + 0.5 * cse;
	if ( ra[0] <= -1 ) {
		eang=M_PI;
	} else if ( ra[0] >= 1 ) {
		eang=0;
	} else {
		eang=acos(ra[0]);
		if ( ra[1] < 0 ) eang=2*M_PI-eang;
	}
	
	csd[0]*=rx;csd[1]*=ry;
	ca[1]=-ca[1]; // because it's the inverse rotation

	dr[0]=dot(csd,ca);
	dr[1]=cross(csd,ca);

	ca[1]=-ca[1];
	
	if ( wise ) {
		if (large) {
			dr=-dr;
			double  swap=eang;eang=sang;sang=swap;
			eang+=M_PI;sang+=M_PI;
			if ( eang >= 2*M_PI ) eang-=2*M_PI;
			if ( sang >= 2*M_PI ) sang-=2*M_PI;
		}
	} else {
		if (!large) {
			dr=-dr;
			double  swap=eang;eang=sang;sang=swap;
			eang+=M_PI;sang+=M_PI;
			if ( eang >= 2*M_PI ) eang-=2*M_PI;
			if ( sang >= 2*M_PI ) sang-=2*M_PI;
		}
	}
	dr+=0.5*(iS+iE);
}

void Path::DoArc(NR::Point const &iS, NR::Point const &iE,
                 double const rx, double const ry, double const angle,
                 bool const large, bool const wise, double const /*tresh*/)
{
	/* TODO: Check that our behaviour is standards-conformant if iS and iE are (much) further
	   apart than the diameter.  Also check that we do the right thing for negative radius.
	   (Same for the other DoArc functions in this file.) */
	if ( rx <= 0.0001 || ry <= 0.0001 ) {
		return;
		// We always add a lineto afterwards, so this is fine.
		// [on ajoute toujours un lineto apres, donc c bon]
	}

	double sang, eang;
	NR::Point dr;
	ArcAnglesAndCenter(iS,iE,rx,ry,angle,large,wise,sang,eang,dr);
	/* TODO: This isn't as good numerically as treating iS and iE as primary.  E.g. consider
	   the case of low curvature (i.e. very large radius). */

	NR::scale const ar(rx, ry);
	NR::rotate cb( angle + sang );
	if (wise) {
		double const incr = -0.1;
		if ( sang < eang ) sang += 2*M_PI;
		NR::rotate const omega(incr);
		for (double b = sang + incr ; b > eang ; b += incr) {
			cb = omega * cb;
			AddPoint( cb.vec * ar + dr );
		}
	} else {
		double const incr = 0.1;
		if ( sang > eang ) sang -= 2*M_PI;
		NR::rotate const omega(incr);
		for (double b = sang + incr ; b < eang ; b += incr) {
			cb = omega * cb;
			AddPoint( cb.vec * ar + dr);
		}
	}
}

void Path::RecCubicTo( NR::Point const &iS, NR::Point const &isD, 
                       NR::Point const &iE, NR::Point const &ieD,
                       double tresh,int lev,double maxL)
{
	NR::Point  se;
	se=iE-iS;
	const double dC=NR::L2(se);
	if ( dC < 0.01 ) {
		const double sC = dot(isD,isD);
		const double eC = dot(ieD,ieD);
		if ( sC < tresh && eC < tresh ) return;
	} else {
		const double sC = fabs(cross(se,isD)) / dC;
		const double eC = fabs(cross(se,ieD)) / dC;
		if ( sC < tresh && eC < tresh ) {
			// presque tt droit -> attention si on nous demande de bien subdiviser les petits segments
			if ( maxL > 0 && dC > maxL ) {
				if ( lev <= 0 ) return;
				NR::Point  m = 0.5*(iS+iE)+0.125*(isD-ieD);
				NR::Point  md = 0.75*(iE-iS)-0.125*(isD+ieD);
				
				NR::Point  hisD=0.5*isD;
				NR::Point  hieD=0.5*ieD;
				
				RecCubicTo(iS,hisD,m,md,tresh,lev-1,maxL);
				AddPoint(m);
				RecCubicTo(m,md,iE,hieD,tresh,lev-1,maxL);
			}
			return;
		}
	}
	
	if ( lev <= 0 ) return;
	{
		NR::Point  m=0.5*(iS+iE)+0.125*(isD-ieD);
		NR::Point  md=0.75*(iE-iS)-0.125*(isD+ieD);
				
		NR::Point  hisD=0.5*isD;
		NR::Point  hieD=0.5*ieD;
    
		RecCubicTo(iS,hisD,m,md,tresh,lev-1,maxL);
		AddPoint(m);
		RecCubicTo(m,md,iE,hieD,tresh,lev-1,maxL);
	}
}
void Path::RecBezierTo(const NR::Point &iP,
                       const NR::Point &iS,
                       const NR::Point &iE,
                       double tresh,int lev,double maxL)
{
	if ( lev <= 0 ) return;
	NR::Point ps=iS-iP;
	NR::Point pe=iE-iP;
	NR::Point se=iE-iS;
	double s = fabs(cross(pe,ps));
	if ( s < tresh ) {
		const double l=L2(se);
		if ( maxL > 0 && l > maxL ) {
			const NR::Point  m=0.25*(iS+iE+2*iP);
			NR::Point  md = 0.5*(iS+iP);
			RecBezierTo(md,iS,m,tresh,lev-1,maxL);
			AddPoint(m);
			md=0.5*(iP+iE);
			RecBezierTo(md,m,iE,tresh,lev-1,maxL);	
		}
		return;
	}
	{
		const NR::Point  m=0.25*(iS+iE+2*iP);
		NR::Point  md=0.5*(iS+iP);
		RecBezierTo(md,iS,m,tresh,lev-1,maxL);
		AddPoint(m);
		md=0.5*(iP+iE);
		RecBezierTo(md,m,iE,tresh,lev-1,maxL);	
	}
}

void Path::DoArc(NR::Point const &iS, NR::Point const &iE,
                 double const rx, double const ry, double const angle,
                 bool const large, bool const wise, double const /*tresh*/, int const piece)
{
	/* TODO: Check that our behaviour is standards-conformant if iS and iE are (much) further
	   apart than the diameter.  Also check that we do the right thing for negative radius.
	   (Same for the other DoArc functions in this file.) */
	if ( rx <= 0.0001 || ry <= 0.0001 ) {
		return;
		// We always add a lineto afterwards, so this is fine.
		// [on ajoute toujours un lineto apres, donc c bon]
	}

	double sang, eang;
	NR::Point dr;
	ArcAnglesAndCenter(iS,iE,rx,ry,angle,large,wise,sang,eang,dr);
	/* TODO: This isn't as good numerically as treating iS and iE as primary.  E.g. consider
	   the case of low curvature (i.e. very large radius). */

	NR::scale const ar(rx, ry);
	NR::rotate cb( angle + sang );
	if (wise) {
		double const incr = -0.1;
		if ( sang < eang ) sang += 2*M_PI;
		NR::rotate const omega(incr);
		for (double b = sang + incr ; b > eang ; b += incr) {
			cb = omega * cb;
			AddPoint(cb.vec * ar + dr, piece, (sang-b)/(sang-eang));
		}
	} else {
		double const incr = 0.1;
		if ( sang > eang ) sang -= 2*M_PI;
		NR::rotate const omega(incr);
		for (double b = sang + incr ; b < eang ; b += incr) {
			cb = omega * cb;
			AddPoint(cb.vec * ar + dr, piece, (b-sang)/(eang-sang));
		}
	}
}

void Path::RecCubicTo(NR::Point const &iS, NR::Point const &isD, 
                      NR::Point const &iE, NR::Point const &ieD,
                      double tresh,int lev,double st,double et,int piece)
{
	const NR::Point  se=iE-iS;
	const double dC=NR::L2(se);
	if ( dC < 0.01 ) {
		const double sC=dot(isD,isD);
		const double eC=dot(ieD,ieD);
		if ( sC < tresh && eC < tresh ) return;
	} else {
		const double sC = fabs(cross(se,isD)) / dC;
		const double eC = fabs(cross(se,ieD)) / dC;
		if ( sC < tresh && eC < tresh ) return;
	}
	
	if ( lev <= 0 ) return;
	
	NR::Point  m = 0.5*(iS+iE) + 0.125*(isD-ieD);
	NR::Point  md = 0.75*(iE-iS) - 0.125*(isD+ieD);
	double   mt=(st+et)/2;
	
	NR::Point  hisD=0.5*isD;
	NR::Point  hieD=0.5*ieD;
	
	RecCubicTo(iS,hisD,m,md,tresh,lev-1,st,mt,piece);
	AddPoint(m,piece,mt);
	RecCubicTo(m,md,iE,hieD,tresh,lev-1,mt,et,piece);
	
}
void Path::RecBezierTo(NR::Point const &iP,
                       NR::Point const &iS,
                       NR::Point const &iE,
                       double tresh,int lev,double st,double et,int piece)
{
	if ( lev <= 0 ) return;
	NR::Point ps=iS-iP;
	NR::Point pe=iE-iP;
	NR::Point se=iE-iS;
	const double s = fabs(cross(pe,ps));
	if ( s < tresh ) return ;
	
	{
		const double mt=(st+et)/2;
		const NR::Point  m=0.25*(iS+iE+2*iP);
		RecBezierTo(0.5*(iS+iP),iS,m,tresh,lev-1,st,mt,piece);
		AddPoint(m,piece,mt);
		RecBezierTo(0.5*(iP+iE), 
                m,iE,tresh,lev-1,mt,et,piece);	
	}
}

void Path::DoArc(NR::Point const &iS, NR::Point const &iE,
                 double const rx, double const ry, double const angle,
                 bool const large, bool const wise, double const /*tresh*/,
                 int const piece, offset_orig &/*orig*/)
{
	// Will never arrive here, as offsets are made of cubics.
	// [on n'arrivera jamais ici, puisque les offsets sont fait de cubiques]
	/* TODO: Check that our behaviour is standards-conformant if iS and iE are (much) further
	   apart than the diameter.  Also check that we do the right thing for negative radius.
	   (Same for the other DoArc functions in this file.) */
	if ( rx <= 0.0001 || ry <= 0.0001 ) {
		return;
		// We always add a lineto afterwards, so this is fine.
		// [on ajoute toujours un lineto apres, donc c bon]
	}

	double sang, eang;
	NR::Point dr;
	ArcAnglesAndCenter(iS,iE,rx,ry,angle,large,wise,sang,eang,dr);
	/* TODO: This isn't as good numerically as treating iS and iE as primary.  E.g. consider
	   the case of low curvature (i.e. very large radius). */

	NR::scale const ar(rx, ry);
	NR::rotate cb( angle + sang );
	if (wise) {
		double const incr = -0.1;
		if ( sang < eang ) sang += 2*M_PI;
		NR::rotate const omega(incr);
		for (double b = sang + incr ; b > eang ; b += incr) {
			cb = omega * cb;
			AddPoint(cb.vec * ar + dr, piece, (sang-b)/(sang-eang));
		}
	} else {
		double const incr = 0.1;
		if ( sang > eang ) sang -= 2*M_PI;
		NR::rotate const omega(incr);
		for (double b = sang + incr ; b < eang ; b += incr) {
			cb = omega * cb;
			AddPoint(cb.vec * ar + dr, piece, (b-sang)/(eang-sang));
		}
	}
}

void Path::RecCubicTo(NR::Point const &iS, NR::Point const &isD, 
                      NR::Point const &iE, NR::Point const &ieD,
                      double tresh, int lev, double st, double et,
                      int piece, offset_orig& orig)
{
	const NR::Point  se = iE-iS;
	const double dC = NR::L2(se);
	bool  doneSub=false;
	if ( dC < 0.01 ) {
		const double sC=dot(isD,isD);
		const double eC=dot(ieD,ieD);
		if ( sC < tresh && eC < tresh ) return;
	} else {
		const double sC = fabs(cross(se,isD)) / dC;
		const double eC = fabs(cross(se,ieD)) / dC;
		if ( sC < tresh && eC < tresh ) doneSub=true;
	}
	
	if ( lev <= 0 ) doneSub=true;
	
	// test des inversions
	bool stInv=false,enInv=false;
	{
		NR::Point  os_pos,os_tgt,oe_pos,oe_tgt;
		orig.orig->PointAndTangentAt(orig.piece,orig.tSt*(1-st)+orig.tEn*st,os_pos,os_tgt);
		orig.orig->PointAndTangentAt(orig.piece,orig.tSt*(1-et)+orig.tEn*et,oe_pos,oe_tgt);
		
    
		NR::Point   n_tgt=isD;
		double si=dot(n_tgt,os_tgt);
		if ( si < 0 ) stInv=true;
		n_tgt=ieD;
		si=dot(n_tgt,oe_tgt);
		if ( si < 0 ) enInv=true;
		if ( stInv && enInv ) {
			AddPoint(os_pos,-1,0.0);
			AddPoint(iE,piece,et);
			AddPoint(iS,piece,st);
			AddPoint(oe_pos,-1,0.0);
			return;
		} else if ( ( stInv && !enInv ) || ( !stInv && enInv ) ) {
			return;
		}
	}
	if ( ( !stInv && !enInv && doneSub ) || lev <= 0 ) return;
	
	{
		const NR::Point  m=0.5*(iS+iE)+0.125*(isD-ieD);
		const NR::Point  md=0.75*(iE-iS)-0.125*(isD+ieD);
		const double   mt=(st+et)/2;
		const NR::Point  hisD=0.5*isD;
		const NR::Point  hieD=0.5*ieD;
    
		RecCubicTo(iS,hisD,m,md,tresh,lev-1,st,mt,piece,orig);
		AddPoint(m,piece,mt);
		RecCubicTo(m,md,iE,hieD,tresh,lev-1,mt,et,piece,orig);
	}
}
void Path::RecBezierTo(NR::Point const &iP, NR::Point const &iS,NR::Point const &iE,
                       double tresh,int lev,double st,double et,
                       int piece,offset_orig& orig)
{
	bool doneSub=false;
	if ( lev <= 0 ) return;
	const NR::Point ps=iS-iP;
	const NR::Point pe=iE-iP;
	const NR::Point se=iE-iS;
	const double s = fabs(cross(pe,ps));
	if ( s < tresh ) doneSub=true ;
  
	// test des inversions
	bool stInv=false,enInv=false;
	{
		NR::Point  os_pos,os_tgt,oe_pos,oe_tgt,n_tgt,n_pos;
		double n_len,n_rad;
		path_descr_intermbezierto mid;
		mid.p=iP;
		path_descr_bezierto fin;
		fin.nb=1;
		fin.p=iE;
		
		TangentOnBezAt(0.0,iS,mid,fin,false,n_pos,n_tgt,n_len,n_rad);
		orig.orig->PointAndTangentAt(orig.piece,orig.tSt*(1-st)+orig.tEn*st,os_pos,os_tgt);
		double si=dot(n_tgt,os_tgt);
		if ( si < 0 ) stInv=true;
		
		TangentOnBezAt(1.0,iS,mid,fin,false,n_pos,n_tgt,n_len,n_rad);
		orig.orig->PointAndTangentAt(orig.piece,orig.tSt*(1-et)+orig.tEn*et,oe_pos,oe_tgt);
		si=dot(n_tgt,oe_tgt);
		if ( si < 0 ) enInv=true;
		
		if ( stInv && enInv ) {
			AddPoint(os_pos,-1,0.0);
			AddPoint(iE,piece,et);
			AddPoint(iS,piece,st);
			AddPoint(oe_pos,-1,0.0);
			return;
			//		} else if ( ( stInv && !enInv ) || ( !stInv && enInv ) ) {
			//			return;
      }
	}
	if ( !stInv && !enInv && doneSub ) return;
  
	{
		double      mt=(st+et)/2;
		NR::Point  m=0.25*(iS+iE+2*iP);
		NR::Point  md=0.5*(iS+iP);
		RecBezierTo(md,iS,m,tresh,lev-1,st,mt,piece,orig);
		AddPoint(m,piece,mt);
		md=0.5*(iP+iE);
		RecBezierTo(md,m,iE,tresh,lev-1,mt,et,piece,orig);	
	}
}


/*
 * put a polyline in a Shape instance, for further fun
 * pathID is the ID you want this Path instance to be associated with, for when you're going to recompose the polyline
 * in a path description ( you need to have prepared the back data for that, of course)
 */

void Path::Fill(Shape* dest,int pathID,bool justAdd,bool closeIfNeeded,bool invert)
{
	if ( dest == NULL ) return;
	if ( justAdd == false ) {
		dest->Reset(nbPt,nbPt);
	}
	if ( nbPt <= 1 ) return;
	int   first=dest->nbPt;
  //	bool  startIsEnd=false;
	
	if ( back ) dest->MakeBackData(true);
	
	if ( invert ) {
		if ( back ) {
    {
      // invert && back && !weighted
      for (int i=0;i<nbPt;i++) dest->AddPoint(((path_lineto_b*)pts)[i].p);
      int               lastM=0;
      int								curP=1;
      int               pathEnd=0;
      bool              closed=false;
      int               lEdge=-1;
      while ( curP < nbPt ) {
        path_lineto_b*    sbp=((path_lineto_b*)pts)+curP;
        path_lineto_b*    lm=((path_lineto_b*)pts)+lastM;
        path_lineto_b*    prp=((path_lineto_b*)pts)+pathEnd;
        if ( sbp->isMoveTo == polyline_moveto ) {
          if ( closeIfNeeded ) {
            if ( closed && lEdge >= 0 ) {
              dest->DisconnectStart(lEdge);
              dest->ConnectStart(first+lastM,lEdge);
            } else {
              dest->AddEdge(first+lastM,first+pathEnd);
              dest->ebData[lEdge].pathID=pathID;
              dest->ebData[lEdge].pieceID=lm->piece;
              dest->ebData[lEdge].tSt=1.0;
              dest->ebData[lEdge].tEn=0.0;
            }
          }
          lastM=curP;
          pathEnd=curP;
          closed=false;
          lEdge=-1;
        } else {
          if ( NR::LInfty(sbp->p-prp->p) < 0.00001 ) {
          } else {
            lEdge=dest->AddEdge(first+curP,first+pathEnd);
            dest->ebData[lEdge].pathID=pathID;
            dest->ebData[lEdge].pieceID=sbp->piece;
            if ( sbp->piece == prp->piece ) {
              dest->ebData[lEdge].tSt=sbp->t;
              dest->ebData[lEdge].tEn=prp->t;
            } else {
              dest->ebData[lEdge].tSt=sbp->t;
              dest->ebData[lEdge].tEn=0.0;
            }
            pathEnd=curP;
            if ( NR::LInfty(sbp->p-lm->p) < 0.00001 ) {
              closed=true;
            } else {
              closed=false;
            }
          }
        }
        curP++;
      }
      if ( closeIfNeeded ) {
        if ( closed && lEdge >= 0 ) {
          dest->DisconnectStart(lEdge);
          dest->ConnectStart(first+lastM,lEdge);
        } else {
          path_lineto_b*    lm=((path_lineto_b*)pts)+lastM;
          lEdge=dest->AddEdge(first+lastM,first+pathEnd);
          dest->ebData[lEdge].pathID=pathID;
          dest->ebData[lEdge].pieceID=lm->piece;
          dest->ebData[lEdge].tSt=1.0;
          dest->ebData[lEdge].tEn=0.0;
        }
      }
    }
		} else {
    {
      // invert && !back && !weighted
      for (int i=0;i<nbPt;i++) dest->AddPoint(((path_lineto*)pts)[i].p);
      int               lastM=0;
      int								curP=1;
      int               pathEnd=0;
      bool              closed=false;
      int               lEdge=-1;
      while ( curP < nbPt ) {
        path_lineto*    sbp=((path_lineto*)pts)+curP;
        path_lineto*    lm=((path_lineto*)pts)+lastM;
        path_lineto*    prp=((path_lineto*)pts)+pathEnd;
        if ( sbp->isMoveTo == polyline_moveto ) {
          if ( closeIfNeeded ) {
            if ( closed && lEdge >= 0 ) {
              dest->DisconnectStart(lEdge);
              dest->ConnectStart(first+lastM,lEdge);
            } else {
              dest->AddEdge(first+lastM,first+pathEnd);
            }
          }
          lastM=curP;
          pathEnd=curP;
          closed=false;
          lEdge=-1;
        } else {
          if ( NR::LInfty(sbp->p-prp->p) < 0.00001 ) {
          } else {
            lEdge=dest->AddEdge(first+curP,first+pathEnd);
            pathEnd=curP;
            if ( NR::LInfty(sbp->p-lm->p) < 0.00001 ) {
              closed=true;
            } else {
              closed=false;
            }
          }
        }
        curP++;
      }
      
      if ( closeIfNeeded ) {
        if ( closed && lEdge >= 0 ) {
          dest->DisconnectStart(lEdge);
          dest->ConnectStart(first+lastM,lEdge);
        } else {
          dest->AddEdge(first+lastM,first+pathEnd);
        }
      }
      
    }
		}
	} else {
		if ( back ) {
    {
      // !invert && back && !weighted
      for (int i=0;i<nbPt;i++) dest->AddPoint(((path_lineto_b*)pts)[i].p);
      int               lastM=0;
      int								curP=1;
      int               pathEnd=0;
      bool              closed=false;
      int               lEdge=-1;
      while ( curP < nbPt ) {
        path_lineto_b*    sbp=((path_lineto_b*)pts)+curP;
        path_lineto_b*    lm=((path_lineto_b*)pts)+lastM;
        path_lineto_b*    prp=((path_lineto_b*)pts)+pathEnd;
        if ( sbp->isMoveTo == polyline_moveto ) {
          if ( closeIfNeeded ) {
            if ( closed && lEdge >= 0 ) {
              dest->DisconnectEnd(lEdge);
              dest->ConnectEnd(first+lastM,lEdge);
            } else {
              dest->AddEdge(first+pathEnd,first+lastM);
              dest->ebData[lEdge].pathID=pathID;
              dest->ebData[lEdge].pieceID=lm->piece;
              dest->ebData[lEdge].tSt=0.0;
              dest->ebData[lEdge].tEn=1.0;
            }
          }
          lastM=curP;
          pathEnd=curP;
          closed=false;
          lEdge=-1;
        } else {
          if ( NR::LInfty(sbp->p-prp->p) < 0.00001 ) {
          } else {
            lEdge=dest->AddEdge(first+pathEnd,first+curP);
            dest->ebData[lEdge].pathID=pathID;
            dest->ebData[lEdge].pieceID=sbp->piece;
            if ( sbp->piece == prp->piece ) {
              dest->ebData[lEdge].tSt=prp->t;
              dest->ebData[lEdge].tEn=sbp->t;
            } else {
              dest->ebData[lEdge].tSt=0.0;
              dest->ebData[lEdge].tEn=sbp->t;
            }
            pathEnd=curP;
            if ( NR::LInfty(sbp->p-lm->p) < 0.00001 ) {
              closed=true;
            } else {
              closed=false;
            }
          }
        }
        curP++;
      }
      if ( closeIfNeeded ) {
        if ( closed && lEdge >= 0 ) {
          dest->DisconnectEnd(lEdge);
          dest->ConnectEnd(first+lastM,lEdge);
        } else {
          path_lineto_b*    lm=((path_lineto_b*)pts)+lastM;
          lEdge=dest->AddEdge(first+pathEnd,first+lastM);
          dest->ebData[lEdge].pathID=pathID;
          dest->ebData[lEdge].pieceID=lm->piece;
          dest->ebData[lEdge].tSt=0.0;
          dest->ebData[lEdge].tEn=1.0;
        }
      }
    }
		} else {
    {
      // !invert && !back && !weighted
      for (int i=0;i<nbPt;i++) dest->AddPoint(((path_lineto*)pts)[i].p);
      int               lastM=0;
      int								curP=1;
      int               pathEnd=0;
      bool              closed=false;
      int               lEdge=-1;
      while ( curP < nbPt ) {
        path_lineto*    sbp=((path_lineto*)pts)+curP;
        path_lineto*    lm=((path_lineto*)pts)+lastM;
        path_lineto*    prp=((path_lineto*)pts)+pathEnd;
        if ( sbp->isMoveTo == polyline_moveto ) {
          if ( closeIfNeeded ) {
            if ( closed && lEdge >= 0 ) {
              dest->DisconnectEnd(lEdge);
              dest->ConnectEnd(first+lastM,lEdge);
            } else {
              dest->AddEdge(first+pathEnd,first+lastM);
            }
          }
          lastM=curP;
          pathEnd=curP;
          closed=false;
          lEdge=-1;
        } else {
          if ( NR::LInfty(sbp->p-prp->p) < 0.00001 ) {
          } else {
            lEdge=dest->AddEdge(first+pathEnd,first+curP);
            pathEnd=curP;
            if ( NR::LInfty(sbp->p-lm->p) < 0.00001 ) {
              closed=true;
            } else {
              closed=false;
            }
          }
        }
        curP++;
      }
      
      if ( closeIfNeeded ) {
        if ( closed && lEdge >= 0 ) {
          dest->DisconnectEnd(lEdge);
          dest->ConnectEnd(first+lastM,lEdge);
        } else {
          dest->AddEdge(first+pathEnd,first+lastM);
        }
      }
      
    }
		}
	}
}
