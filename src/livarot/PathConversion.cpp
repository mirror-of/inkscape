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

void            Path::ConvertWithBackData(double treshhold)
{
	if ( descr_flags&descr_adding_bezier ) CancelBezier();
	if ( descr_flags&descr_doing_subpath ) CloseSubpath(0);
	
	SetBackData(true);
	ResetPoints(descr_nb);
	if ( descr_nb <= 0 ) return;
	NR::Point curX;
	double     curW;
	int       curP=1;
	int       lastMoveTo=0;
	
	// le moveto
	curX=(descr_data)->d.m.p;
	if ( (descr_data)->flags&descr_weighted ) {
		curW=(descr_data)->d.m.w;
	} else {
		curW=1;
	}
	if ( weighted ) lastMoveTo=AddPoint(curX,curW,0,0.0,true); else lastMoveTo=AddPoint(curX,0,0.0,true);
	
	// et le reste, 1 par 1
	while ( curP < descr_nb ) {
		path_descr*  curD=descr_data+curP;
		int          nType=curD->flags&descr_type_mask;
		bool         nWeight=curD->flags&descr_weighted;
		NR::Point    nextX;
		double        nextW;
		if ( nType == descr_forced ) {
			if ( weighted ) AddForcedPoint(curX,curW,curP,1.0); else AddForcedPoint(curX,curP,1.0);
			curP++;
		} else if ( nType == descr_moveto ) {
			nextX=curD->d.m.p;
			if ( nWeight ) nextW=curD->d.m.w; else nextW=1;
			if ( weighted ) lastMoveTo=AddPoint(nextX,nextW,curP,0.0,true); else lastMoveTo=AddPoint(nextX,curP,0.0,true);
			// et on avance
			curP++;
		} else if ( nType == descr_close ) {
			if ( weighted ) {
				nextX=((path_lineto_wb*)pts)[lastMoveTo].p;
				nextW=((path_lineto_wb*)pts)[lastMoveTo].w;
				AddPoint(nextX,nextW,curP,1.0,false);
			} else {
				nextX=((path_lineto_b*)pts)[lastMoveTo].p;
				AddPoint(nextX,curP,1.0,false);
			}
			curP++;
		} else if ( nType == descr_lineto ) {
			nextX=curD->d.l.p;
			if ( nWeight ) nextW=curD->d.l.w; else nextW=1;
			if ( weighted ) AddPoint(nextX,nextW,curP,1.0,false); else AddPoint(nextX,curP,1.0,false);
			// et on avance
			curP++;
		} else if ( nType == descr_cubicto ) {
			nextX=curD->d.c.p;
			if ( nWeight ) nextW=curD->d.c.w; else nextW=1;
			if ( weighted ) {
				RecCubicTo(curX,curW,curD->d.c.stD,nextX,nextW,curD->d.c.enD,treshhold,8,0.0,1.0,curP);
				AddPoint(nextX,nextW,curP,1.0,false);
			} else {
				RecCubicTo(curX,curD->d.c.stD,nextX,curD->d.c.enD,treshhold,8,0.0,1.0,curP);
				AddPoint(nextX,curP,1.0,false);
			}
			// et on avance
			curP++;
		} else if ( nType == descr_arcto ) {
			nextX=curD->d.a.p;
			if ( nWeight ) nextW=curD->d.a.w; else nextW=1;
			if ( weighted ) {
				DoArc(curX,curW,nextX,nextW,curD->d.a.rx,curD->d.a.ry,curD->d.a.angle,curD->d.a.large,curD->d.a.clockwise,treshhold,curP);
				AddPoint(nextX,nextW,curP,1.0,false);
			} else {
				DoArc(curX,nextX,curD->d.a.rx,curD->d.a.ry,curD->d.a.angle,curD->d.a.large,curD->d.a.clockwise,treshhold,curP);
				AddPoint(nextX,curP,1.0,false);
			}
			// et on avance
			curP++;
		} else if ( nType == descr_bezierto ) {
			int   nbInterm=curD->d.b.nb;
			nextX=curD->d.b.p;
			if ( nWeight ) nextW=curD->d.b.w; else nextW=1;
			
			curD=descr_data+(curP+1);
			path_descr* intermPoints=curD;
			
			if ( nbInterm <= 0 ) {
			} else if ( nbInterm >= 1 ) {
				NR::Point   bx=curX;
				NR::Point cx=curX;
				NR::Point dx=curX;
				double       bw=curW,cw=curW,dw=curW;
					
				dx=intermPoints->d.i.p;
				if ( nWeight ) {
					dw=intermPoints->d.i.w;
				} else {
					dw=1;
				}
				intermPoints++;
					
				cx=2*bx-dx;
				cw=2*bw-dw;
					
				for (int k=0;k<nbInterm-1;k++) {
					bx=cx;bw=cw;
					cx=dx;cw=dw;
						
					dx=intermPoints->d.i.p;
					if ( nWeight ) {
						dw=intermPoints->d.i.w;
					} else {
						dw=1;
					}
					intermPoints++;
						
					NR::Point  stx;
					stx=(bx+cx)/2;
					double  stw=(bw+cw)/2;
					if ( k > 0 ) {
						if ( weighted ) AddPoint(stx,stw,curP-1+k,1.0,false); else AddPoint(stx,curP-1+k,1.0,false);
					}
						
					if ( weighted ) {
						NR::Point mx;
						mx=(cx+dx)/2;
						RecBezierTo(cx,cw,stx,stw,mx,(cw+dw)/2,treshhold,8,0.0,1.0,curP+k);
					} else {
						NR::Point mx;
						mx=(cx+dx)/2;
						RecBezierTo(cx,stx,mx,treshhold,8,0.0,1.0,curP+k);
					}
				}
				{
					bx=cx;bw=cw;
					cx=dx;cw=dw;
						
					dx=nextX;
					if ( nWeight ) {
						dw=nextW;
					} else {
						dw=1;
					}
					dx=2*dx-cx;
					dw=2*dw-cw;
						
					NR::Point  stx;
					stx=(bx+cx)/2;
					double  stw=(bw+cw)/2;
						
					if ( nbInterm > 1 ) {
						if ( weighted ) AddPoint(stx,stw,curP+nbInterm-2,1.0,false); else AddPoint(stx,curP+nbInterm-2,1.0,false);
					}
						
					if ( weighted ) {
						NR::Point mx;
						mx=(cx+dx)/2;
						RecBezierTo(cx,cw,stx,stw,mx,(cw+dw)/2,treshhold,8,0.0,1.0,curP+nbInterm-1);
					} else {
						NR::Point mx;
						mx=(cx+dx)/2;
						RecBezierTo(cx,stx,mx,treshhold,8,0.0,1.0,curP+nbInterm-1);
					}
				}
					
			}
			
			
			if ( weighted ) AddPoint(nextX,nextW,curP-1+nbInterm,1.0,false); else AddPoint(nextX,curP-1+nbInterm,1.0,false);
			
			// et on avance
			curP+=1+nbInterm;
		}
		curX=nextX;
		curW=nextW;
	}
}
void            Path::ConvertForOffset(double treshhold,Path* orig,double off_dec)
{
	if ( descr_flags&descr_adding_bezier ) CancelBezier();
	if ( descr_flags&descr_doing_subpath ) CloseSubpath(0);
	
	SetBackData(true);
	ResetPoints(descr_nb);
	if ( descr_nb <= 0 ) return;
	NR::Point curX;
	double     curW;
	int       curP=1;
	int       lastMoveTo=0;
	
	// le moveto
	curX=(descr_data)->d.m.p;
	if ( (descr_data)->flags&descr_weighted ) {
		curW=(descr_data)->d.m.w;
	} else {
		curW=1;
	}
	lastMoveTo=AddPoint(curX,0,0.0,true);
	
	offset_orig     off_data;
	off_data.orig=orig;
	off_data.off_dec=off_dec;
	
	// et le reste, 1 par 1
	while ( curP < descr_nb ) {
		path_descr*  curD=descr_data+curP;
		int          nType=curD->flags&descr_type_mask;
		bool         nWeight=curD->flags&descr_weighted;
		NR::Point    nextX;
		double        nextW;
		if ( nType == descr_forced ) {
			AddForcedPoint(curX,curP,1.0);
			curP++;
		} else if ( nType == descr_moveto ) {
			nextX=curD->d.m.p;
			if ( nWeight ) nextW=curD->d.m.w; else nextW=1;
			lastMoveTo=AddPoint(nextX,curP,0.0,true);
			// et on avance
			curP++;
		} else if ( nType == descr_close ) {
			nextX=((path_lineto_b*)pts)[lastMoveTo].p;
			AddPoint(nextX,curP,1.0,false);
			curP++;
		} else if ( nType == descr_lineto ) {
			nextX=curD->d.l.p;
			if ( nWeight ) nextW=curD->d.l.w; else nextW=1;
			AddPoint(nextX,curP,1.0,false);
			// et on avance
			curP++;
		} else if ( nType == descr_cubicto ) {
			nextX=curD->d.c.p;
			if ( nWeight ) nextW=curD->d.c.w; else nextW=1;
			off_data.piece=curD->associated;
			off_data.tSt=curD->tSt;
			off_data.tEn=curD->tEn;
			if ( curD->associated >= 0 ) {
				RecCubicTo(curX,curD->d.c.stD,nextX,curD->d.c.enD,treshhold,8,0.0,1.0,curP,off_data);
			} else {
				RecCubicTo(curX,curD->d.c.stD,nextX,curD->d.c.enD,treshhold,8,0.0,1.0,curP);
			}
			AddPoint(nextX,curP,1.0,false);
			// et on avance
			curP++;
		} else if ( nType == descr_arcto ) {
			nextX=curD->d.a.p;
			if ( nWeight ) nextW=curD->d.a.w; else nextW=1;
			off_data.piece=curD->associated;
			off_data.tSt=curD->tSt;
			off_data.tEn=curD->tEn;
			if ( curD->associated >= 0 ) {
				DoArc(curX,nextX,curD->d.a.rx,curD->d.a.ry,curD->d.a.angle,curD->d.a.large,curD->d.a.clockwise,treshhold,curP,off_data);
			} else {
				DoArc(curX,nextX,curD->d.a.rx,curD->d.a.ry,curD->d.a.angle,curD->d.a.large,curD->d.a.clockwise,treshhold,curP);
			}
			AddPoint(nextX,curP,1.0,false);
			// et on avance
			curP++;
		} else if ( nType == descr_bezierto ) {
			// on ne devrait jamais avoir de bezier quadratiques dans les offsets
			// mais bon, par precaution...
			int   nbInterm=curD->d.b.nb;
			nextX=curD->d.b.p;
			if ( nWeight ) nextW=curD->d.b.w; else nextW=1;
			
			curD=descr_data+(curP+1);
			path_descr* intermPoints=curD;
			
			if ( nbInterm <= 0 ) {
			} else if ( nbInterm >= 1 ) {
				NR::Point   bx=curX;
				NR::Point cx=curX;
				NR::Point dx=curX;
				double       bw=curW,cw=curW,dw=curW;
					
				dx=intermPoints->d.i.p;
				if ( nWeight ) {
					dw=intermPoints->d.i.w;
				} else {
					dw=1;
				}
				intermPoints++;
					
				cx=2*bx-dx;
				cw=2*bw-dw;
					
				for (int k=0;k<nbInterm-1;k++) {
					bx=cx;bw=cw;
					cx=dx;cw=dw;
						
					dx=intermPoints->d.i.p;
					if ( nWeight ) {
						dw=intermPoints->d.i.w;
					} else {
						dw=1;
					}
					intermPoints++;
						
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
					bx=cx;bw=cw;
					cx=dx;cw=dw;
						
					dx=nextX;
					if ( nWeight ) {
						dw=nextW;
					} else {
						dw=1;
					}
					dx=2*dx-cx;
					dw=2*dw-cw;
						
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
		curW=nextW;
	}
}
void            Path::Convert(double treshhold)
{
	if ( descr_flags&descr_adding_bezier ) CancelBezier();
	if ( descr_flags&descr_doing_subpath ) CloseSubpath(0);
	
	SetBackData(false);
	ResetPoints(descr_nb);
	if ( descr_nb <= 0 ) return;
	NR::Point curX;
	double     curW;
	int       curP=1;
	int       lastMoveTo=0;
	
	// le moveto
	curX=(descr_data)->d.m.p;
	if ( (descr_data)->flags&descr_weighted ) {
		curW=(descr_data)->d.m.w;
	} else {
		curW=1;
	}
	if ( weighted ) lastMoveTo=AddPoint(curX,curW,true); else lastMoveTo=AddPoint(curX,true);
	(descr_data)->associated=lastMoveTo;
	
	// et le reste, 1 par 1
	while ( curP < descr_nb ) {
		path_descr*  curD=descr_data+curP;
		int          nType=curD->flags&descr_type_mask;
		bool         nWeight=curD->flags&descr_weighted;
		NR::Point    nextX;
		double        nextW;
		if ( nType == descr_forced ) {
			if ( weighted ) (curD)->associated=AddForcedPoint(curX,curW); else (curD)->associated=AddForcedPoint(curX);
			curP++;
		} else if ( nType == descr_moveto ) {
			nextX=curD->d.m.p;
			if ( nWeight ) nextW=curD->d.m.w; else nextW=1;
			if ( weighted ) lastMoveTo=AddPoint(nextX,nextW,true); else lastMoveTo=AddPoint(nextX,true);
			curD->associated=lastMoveTo;
			
			// et on avance
			curP++;
		} else if ( nType == descr_close ) {
			if ( weighted ) {
				nextX=((path_lineto_w*)pts)[lastMoveTo].p;
				nextW=((path_lineto_w*)pts)[lastMoveTo].w;
				curD->associated=AddPoint(nextX,nextW,false);
				if ( curD->associated < 0 ) {
					if ( curP == 0 ) {
						curD->associated=0;
					} else {
						curD->associated=(curD-1)->associated;
					}
				}
			} else {
				nextX=((path_lineto*)pts)[lastMoveTo].p;
				curD->associated=AddPoint(nextX,false);
				if ( curD->associated < 0 ) {
					if ( curP == 0 ) {
						curD->associated=0;
					} else {
						curD->associated=(curD-1)->associated;
					}
				}
			}
			curP++;
		} else if ( nType == descr_lineto ) {
			nextX=curD->d.l.p;
			if ( nWeight ) nextW=curD->d.l.w; else nextW=1;
			if ( weighted ) curD->associated=AddPoint(nextX,nextW,false); else curD->associated=AddPoint(nextX,false);
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
			nextX=curD->d.c.p;
			if ( nWeight ) nextW=curD->d.c.w; else nextW=1;
			if ( weighted ) {
				RecCubicTo(curX,curW,curD->d.c.stD,nextX,nextW,curD->d.c.enD,treshhold,8);
				curD->associated=AddPoint(nextX,nextW,false);
				if ( curD->associated < 0 ) {
					if ( curP == 0 ) {
						curD->associated=0;
					} else {
						curD->associated=(curD-1)->associated;
					}
				}
			} else {
				RecCubicTo(curX,curD->d.c.stD,nextX,curD->d.c.enD,treshhold,8);
				curD->associated=AddPoint(nextX,false);
				if ( curD->associated < 0 ) {
					if ( curP == 0 ) {
						curD->associated=0;
					} else {
						curD->associated=(curD-1)->associated;
					}
				}
			}
			// et on avance
			curP++;
		} else if ( nType == descr_arcto ) {
			nextX=curD->d.a.p;
			if ( nWeight ) nextW=curD->d.a.w; else nextW=1;
			if ( weighted ) {
				DoArc(curX,curW,nextX,nextW,curD->d.a.rx,curD->d.a.ry,curD->d.a.angle,curD->d.a.large,curD->d.a.clockwise,treshhold);
				curD->associated=AddPoint(nextX,nextW,false);
				if ( curD->associated < 0 ) {
					if ( curP == 0 ) {
						curD->associated=0;
					} else {
						curD->associated=(curD-1)->associated;
					}
				}
			} else {
				DoArc(curX,nextX,curD->d.a.rx,curD->d.a.ry,curD->d.a.angle,curD->d.a.large,curD->d.a.clockwise,treshhold);
				curD->associated=AddPoint(nextX,false);
				if ( curD->associated < 0 ) {
					if ( curP == 0 ) {
						curD->associated=0;
					} else {
						curD->associated=(curD-1)->associated;
					}
				}
			}
			// et on avance
			curP++;
		} else if ( nType == descr_bezierto ) {
			int   nbInterm=curD->d.b.nb;
			nextX=curD->d.b.p;
			if ( nWeight ) nextW=curD->d.b.w; else nextW=1;
			path_descr* curBD=curD;
			
			curP++;
			curD=descr_data+curP;
			path_descr* intermPoints=curD;
			
			if ( nbInterm <= 0 ) {
			} else if ( nbInterm == 1 ) {
				NR::Point  midX;
				double midW;
				midX=intermPoints->d.i.p;
				if ( nWeight ) {
					midW=intermPoints->d.i.w;
				} else {
					midW=1;
				}
				if ( weighted ) {
					RecBezierTo(midX,midW,curX,curW,nextX,nextW,treshhold,8);
				} else {
					RecBezierTo(midX,curX,nextX,treshhold,8);
				}
			} else if ( nbInterm > 1 ) {
				NR::Point   bx=curX;
				NR::Point cx=curX;
				NR::Point dx=curX;
				double       bw=curW,cw=curW,dw=curW;
								
				dx=intermPoints->d.i.p;
				if ( nWeight ) {
					dw=intermPoints->d.i.w;
				} else {
					dw=1;
				}
				intermPoints++;
				
				cx=2*bx-dx;
				cw=2*bw-dw;
				
				for (int k=0;k<nbInterm-1;k++) {
					bx=cx;bw=cw;
					cx=dx;cw=dw;
					
					dx=intermPoints->d.i.p;
					if ( nWeight ) {
						dw=intermPoints->d.i.w;
					} else {
						dw=1;
					}
					intermPoints++;
					
					NR::Point  stx;
					stx=(bx+cx)/2;
					double  stw=(bw+cw)/2;
					if ( k > 0 ) {
						if ( weighted ) (intermPoints-2)->associated=AddPoint(stx,stw,false); else (intermPoints-2)->associated=AddPoint(stx,false);
						if ( (intermPoints-2)->associated < 0 ) {
							if ( curP == 0 ) {
								(intermPoints-2)->associated=0;
							} else {
								(intermPoints-2)->associated=(intermPoints-3)->associated;
							}
						}
					}
					
					if ( weighted ) {
						NR::Point mx;
						mx=(cx+dx)/2;
						RecBezierTo(cx,cw,stx,stw,mx,(cw+dw)/2,treshhold,8);
					} else {
						NR::Point mx;
						mx=(cx+dx)/2;
						RecBezierTo(cx,stx,mx,treshhold,8);
					}
				}
				{
					bx=cx;bw=cw;
					cx=dx;cw=dw;
					
					dx=nextX;
					if ( nWeight ) {
						dw=nextW;
					} else {
						dw=1;
					}
					dx=2*dx-cx;
					dw=2*dw-cw;
					
					NR::Point  stx;
					stx=(bx+cx)/2;
					double  stw=(bw+cw)/2;
					
					if ( weighted ) (intermPoints-1)->associated=AddPoint(stx,stw,false); else (intermPoints-1)->associated=AddPoint(stx,false);
					if ( (intermPoints-1)->associated < 0 ) {
						if ( curP == 0 ) {
							(intermPoints-1)->associated=0;
						} else {
							(intermPoints-1)->associated=(intermPoints-2)->associated;
						}
					}
					
					if ( weighted ) {
						NR::Point mx;
						mx=(cx+dx)/2;
						RecBezierTo(cx,cw,stx,stw,mx,(cw+dw)/2,treshhold,8);
					} else {
						NR::Point mx;
						mx=(cx+dx)/2;
						RecBezierTo(cx,stx,mx,treshhold,8);
					}
				}
			}
			if ( weighted ) curBD->associated=AddPoint(nextX,nextW,false); else curBD->associated=AddPoint(nextX,false);
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
		curW=nextW;
	}
}
void            Path::ConvertEvenLines(double treshhold)
{
	if ( descr_flags&descr_adding_bezier ) CancelBezier();
	if ( descr_flags&descr_doing_subpath ) CloseSubpath(0);
	
	SetBackData(false);
	ResetPoints(descr_nb);
	if ( descr_nb <= 0 ) return;
	NR::Point curX;
	double    curW;
	int      curP=1;
	int      lastMoveTo=0;
	
	// le moveto
	curX=(descr_data)->d.m.p;
	if ( (descr_data)->flags&descr_weighted ) {
		curW=(descr_data)->d.m.w;
	} else {
		curW=1;
	}
	if ( weighted ) lastMoveTo=AddPoint(curX,curW,true); else lastMoveTo=AddPoint(curX,true);
	(descr_data)->associated=lastMoveTo;
	
	// et le reste, 1 par 1
	while ( curP < descr_nb ) {
		path_descr*  curD=descr_data+curP;
		int          nType=curD->flags&descr_type_mask;
		bool         nWeight=curD->flags&descr_weighted;
		NR::Point    nextX;
		double        nextW;
		if ( nType == descr_forced ) {
			if ( weighted ) (curD)->associated=AddForcedPoint(curX,curW); else (curD)->associated=AddForcedPoint(curX);
			curP++;
		} else if ( nType == descr_moveto ) {
			nextX=curD->d.m.p;
			if ( nWeight ) nextW=curD->d.m.w; else nextW=1;
			if ( weighted ) lastMoveTo=AddPoint(nextX,nextW,true); else lastMoveTo=AddPoint(nextX,true);
			(curD)->associated=lastMoveTo;
			// et on avance
			curP++;
		} else if ( nType == descr_close ) {
			if ( weighted ) {
				nextX=((path_lineto_w*)pts)[lastMoveTo].p;
				nextW=((path_lineto_w*)pts)[lastMoveTo].w;
				{
					NR::Point nexcur=nextX-curX;
					const double segL=NR::L2(nexcur);
					if ( segL > treshhold ) {
						for (double i=treshhold;i<segL;i+=treshhold) {
							NR::Point  nX=((segL-i)*curX+i*nextX)/segL;
							AddPoint(nX,((segL-i)*curW+i*nextW)/segL);
						}
					}
				}
				curD->associated=AddPoint(nextX,nextW,false);
				if ( curD->associated < 0 ) {
					if ( curP == 0 ) {
						curD->associated=0;
					} else {
						curD->associated=(curD-1)->associated;
					}
				}
			} else {
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
			}
			curP++;
		} else if ( nType == descr_lineto ) {
			nextX=curD->d.l.p;
			if ( nWeight ) nextW=curD->d.l.w; else nextW=1;
			if ( weighted ) {
				NR::Point nexcur=nextX-curX;
				const double segL=NR::L2(nexcur);
				if ( segL > treshhold ) {
					for (double i=treshhold;i<segL;i+=treshhold) {
						NR::Point  nX = ((segL-i)*curX+i*nextX)/segL;
						AddPoint(nX,((segL-i)*curW+i*nextW)/segL);
					}
				}
				curD->associated=AddPoint(nextX,nextW,false);
				if ( curD->associated < 0 ) {
					if ( curP == 0 ) {
						curD->associated=0;
					} else {
						curD->associated=(curD-1)->associated;
					}
				}
			} else {
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
			}
			// et on avance
			curP++;
		} else if ( nType == descr_cubicto ) {
			nextX=curD->d.c.p;
			if ( nWeight ) nextW=curD->d.c.w; else nextW=1;
			if ( weighted ) {
				RecCubicTo(curX,curW,curD->d.c.stD,nextX,nextW,curD->d.c.enD,treshhold,8,4*treshhold);
				curD->associated=AddPoint(nextX,nextW,false);
				if ( curD->associated < 0 ) {
					if ( curP == 0 ) {
						curD->associated=0;
					} else {
						curD->associated=(curD-1)->associated;
					}
				}
			} else {
				RecCubicTo(curX,curD->d.c.stD,nextX,curD->d.c.enD,treshhold,8,4*treshhold);
				curD->associated=AddPoint(nextX,false);
				if ( curD->associated < 0 ) {
					if ( curP == 0 ) {
						curD->associated=0;
					} else {
						curD->associated=(curD-1)->associated;
					}
				}
			}
			// et on avance
			curP++;
		} else if ( nType == descr_arcto ) {
			nextX=curD->d.a.p;
			if ( nWeight ) nextW=curD->d.a.w; else nextW=1;
			if ( weighted ) {
				DoArc(curX,curW,nextX,nextW,curD->d.a.rx,curD->d.a.ry,curD->d.a.angle,curD->d.a.large,curD->d.a.clockwise,treshhold);
				curD->associated=AddPoint(nextX,nextW,false);
				if ( curD->associated < 0 ) {
					if ( curP == 0 ) {
						curD->associated=0;
					} else {
						curD->associated=(curD-1)->associated;
					}
				}
			} else {
				DoArc(curX,nextX,curD->d.a.rx,curD->d.a.ry,curD->d.a.angle,curD->d.a.large,curD->d.a.clockwise,treshhold);
				curD->associated=AddPoint(nextX,false);
				if ( curD->associated < 0 ) {
					if ( curP == 0 ) {
						curD->associated=0;
					} else {
						curD->associated=(curD-1)->associated;
					}
				}
			}
			// et on avance
			curP++;
		} else if ( nType == descr_bezierto ) {
			int   nbInterm=curD->d.b.nb;
			nextX=curD->d.b.p;
			if ( nWeight ) nextW=curD->d.b.w; else nextW=1;
			path_descr*  curBD=curD;
			
			curP++;
			curD=descr_data+curP;
			path_descr* intermPoints=curD;
			
			if ( nbInterm <= 0 ) {
			} else if ( nbInterm == 1 ) {
				double midW;
				NR::Point  midX=intermPoints->d.i.p;
				if ( nWeight ) {
					midW=intermPoints->d.i.w;
				} else {
					midW=1;
				}
				if ( weighted ) {
					RecBezierTo(midX,midW,curX,curW,nextX,nextW,treshhold,8,4*treshhold);
				} else {
					RecBezierTo(midX,curX,nextX,treshhold,8,4*treshhold);
				}
			} else if ( nbInterm > 1 ) {
				NR::Point   bx=curX,cx=curX,dx=curX;
				double       bw=curW,cw=curW,dw=curW;
								
				dx=intermPoints->d.i.p;
				if ( nWeight ) {
					dw=intermPoints->d.i.w;
				} else {
					dw=1;
				}
				intermPoints++;
				
				cx=2*bx-dx;
				cw=2*bw-dw;
				
				for (int k=0;k<nbInterm-1;k++) {
					bx=cx;bw=cw;
					cx=dx;cw=dw;
					
					dx=intermPoints->d.i.p;
					if ( nWeight ) {
						dw=intermPoints->d.i.w;
					} else {
						dw=1;
					}
					intermPoints++;
					
					NR::Point  stx = (bx+cx)/2;
					double  stw=(bw+cw)/2;
					if ( k > 0 ) {
						if ( weighted ) (intermPoints-2)->associated=AddPoint(stx,stw,false); else (intermPoints-2)->associated=AddPoint(stx,false);
						if ( (intermPoints-2)->associated < 0 ) {
							if ( curP == 0 ) {
								(intermPoints-2)->associated=0;
							} else {
								(intermPoints-2)->associated=(intermPoints-3)->associated;
							}
						}
					}
					
					if ( weighted ) {
						const NR::Point mx=(cx+dx)/2;
						RecBezierTo(cx,cw,stx,stw,mx,(cw+dw)/2,treshhold,8,4*treshhold);
					} else {
						const NR::Point mx=(cx+dx)/2;
						RecBezierTo(cx,stx,mx,treshhold,8,4*treshhold);
					}
				}
				{
					bx=cx;bw=cw;
					cx=dx;cw=dw;
					
					dx=nextX;
					if ( nWeight ) {
						dw=nextW;
					} else {
						dw=1;
					}
					dx=2*dx-cx;
					dw=2*dw-cw;
					
					const NR::Point  stx = (bx+cx)/2;
					double  stw=(bw+cw)/2;
					
					if ( weighted ) (intermPoints-1)->associated=AddPoint(stx,stw,false); else (intermPoints-1)->associated=AddPoint(stx,false);
					if ( (intermPoints-1)->associated < 0 ) {
						if ( curP == 0 ) {
							(intermPoints-1)->associated=0;
						} else {
							(intermPoints-1)->associated=(intermPoints-2)->associated;
						}
					}
					
					if ( weighted ) {
						const NR::Point mx = (cx+dx)/2;
						RecBezierTo(cx,cw,stx,stw,mx,(cw+dw)/2,treshhold,8,4*treshhold);
					} else {
						const NR::Point mx = (cx+dx)/2;
						RecBezierTo(cx,stx,mx,treshhold,8,4*treshhold);
					}
				}
			}
			if ( weighted ) curBD->associated=AddPoint(nextX,nextW,false); else curBD->associated=AddPoint(nextX,false);
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
		if ( fabsf(curX.pt[0]-nextX.pt[0]) > 0.00001 || fabsf(curX.pt[1]-nextX.pt[1]) > 0.00001 ) {
			curX=nextX;
		}
		curW=nextW;
	}
}
void						Path::PrevPoint(int i,NR::Point &oPt)
{
	if ( i < 0 ) return;
	int t=descr_data[i].flags&descr_type_mask;
	if ( t == descr_forced ) {
		PrevPoint(i-1,oPt);
	} else if ( t == descr_moveto ) {
		oPt=descr_data[i].d.m.p;
	} else if ( t == descr_lineto ) {
		oPt=descr_data[i].d.l.p;
	} else if ( t == descr_arcto ) {
		oPt=descr_data[i].d.a.p;
	} else if ( t == descr_cubicto ) {
		oPt=descr_data[i].d.c.p;
	} else if ( t == descr_bezierto ) {
		oPt=descr_data[i].d.b.p;
	} else if ( t == descr_interm_bezier ) {
		PrevPoint(i-1,oPt);
	} else if ( t == descr_close ) {
		PrevPoint(i-1,oPt);
	}
}
void            Path::QuadraticPoint(double t, NR::Point &oPt, 
				     const NR::Point &iS, const NR::Point &iM, const NR::Point &iE)
{
	NR::Point ax = iE-2*iM+iS;
	NR::Point bx = 2*iM-2*iS;
	NR::Point cx = iS;
	
	oPt = t*t*ax+t*bx+cx;
}
void            Path::CubicTangent(double t, NR::Point &oPt, const NR::Point &iS, const NR::Point &isD, const NR::Point &iE, const NR::Point &ieD)
{
	NR::Point ax = ieD-2*iE+2*iS+isD;
	NR::Point bx = 3*iE-ieD-2*isD-3*iS;
	NR::Point cx = isD;
	NR::Point dx = iS;
	
	oPt = 3*t*t*ax+2*t*bx+cx;
}
void            Path::ArcAngles( const NR::Point &iS, const NR::Point &iE,double rx,double ry,double angle,bool large,bool wise,double &sang,double &eang)
{
	NR::Point  dr;
	ArcAnglesAndCenter(iS,iE,rx,ry,angle,large,wise,sang,eang,dr);
}
void Path::ArcAnglesAndCenter(const NR::Point &iS, const NR::Point &iE,
			      double rx, double ry, double angle,
			      bool large, bool wise,
			      double &sang, double &eang, NR::Point &dr)
{
	NR::Point se = iE-iS;
	NR::Point ca(cos(angle),sin(angle));
	NR::Point cse(dot(se,ca),cross(se,ca));
	cse.pt[0] /= rx;
	cse.pt[1] /= ry;
	double   l = dot(cse,cse);
	const double d = sqrt(std::max(1-l/4, 0.0));
	NR::Point   csd = cse.ccw();
	l=sqrt(l);
	csd/=l;
	csd*=d;
  
	NR::Point  ra;
	ra=-(csd+0.5*cse);
	if ( ra.pt[0] <= -1 ) {
		sang=M_PI;
	} else if ( ra.pt[0] >= 1 ) {
		sang=0;
	} else {
		sang=acos(ra.pt[0]);
		if ( ra.pt[1] < 0 ) sang=2*M_PI-sang;
	}
	ra=-csd+0.5*cse;
	if ( ra.pt[0] <= -1 ) {
		eang=M_PI;
	} else if ( ra.pt[0] >= 1 ) {
		eang=0;
	} else {
		eang=acos(ra.pt[0]);
		if ( ra.pt[1] < 0 ) eang=2*M_PI-eang;
	}
	
	csd.pt[0]*=rx;csd.pt[1]*=ry;
	ca.pt[1]=-ca.pt[1]; // because it's the inverse rotation
  
	dr.pt[0]=dot(csd,ca);
	dr.pt[1]=cross(csd,ca);
  
	ca.pt[1]=-ca.pt[1];
	
	if ( wise ) {
		if ( large == true ) {
			dr=-dr;
			double  swap=eang;eang=sang;sang=swap;
			eang+=M_PI;sang+=M_PI;
			if ( eang >= 2*M_PI ) eang-=2*M_PI;
			if ( sang >= 2*M_PI ) sang-=2*M_PI;
		}
	} else {
		if ( large == false ) {
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
		 double rx, double ry, double angle,
		 bool large, bool wise, double tresh)
{
	if ( rx <= 0.0001 || ry <= 0.0001 )
		return; // on ajoute toujours un lineto apres, donc c bon
	
	double       sang,eang;
	NR::Point   dr;
	ArcAnglesAndCenter(iS,iE,rx,ry,angle,large,wise,sang,eang,dr);
  
	NR::Point  ca(cos(angle),sin(angle));
	if ( wise ) {
		if ( sang < eang ) sang+=2*M_PI;
		for (double b=sang-0.1;b>eang;b-=0.1) {
			NR::Point  cb(cos(b),sin(b));
			NR::Point  ar(rx,ry);
			NR::Point  u=ca^cb;
			u*=ar;
			AddPoint(u+dr);
		}
	} else {
		if ( sang > eang ) sang-=2*M_PI;
		for (double b=sang+0.1;b<eang;b+=0.1) {
			NR::Point  cb(cos(b),sin(b));
			NR::Point  ar(rx,ry);
			NR::Point  u=ca^cb;
			u*=ar;
			AddPoint(u + dr);
		}
	}
}
void Path::DoArc(NR::Point const &iS, double sw, 
		 NR::Point const &iE, double ew,
		 double rx, double ry, double angle,
		 bool large, bool wise, double tresh)
{
	if ( rx <= 0.0001 || ry <= 0.0001 ) return; // on ajoute toujours un lineto apres, donc c bon

	double       sang,eang;
	NR::Point   dr;
	ArcAnglesAndCenter(iS,iE,rx,ry,angle,large,wise,sang,eang,dr);
  
	NR::Point  ca(cos(angle),sin(angle));
	if ( wise ) {
		if ( sang < eang ) sang+=2*M_PI;
		for (double b=sang-0.1;b>eang;b-=0.1) {
			NR::Point  cb(cos(b),sin(b));
			NR::Point  ar(rx,ry);
			NR::Point  u=ca^cb;
			u*=ar;
			u+=dr;
			double  nw=(sw*(b-eang)+ew*(sang-b))/(sang-eang);
			AddPoint(u,nw);
		}
	} else {
		if ( sang > eang ) sang-=2*M_PI;
		for (double b=sang+0.1;b<eang;b+=0.1) {
			NR::Point  cb(cos(b),sin(b));
			NR::Point  ar(rx,ry);
			NR::Point  u=ca^cb;
			u*=ar;
			u+=dr;
			double  nw=(sw*(eang-b)+ew*(b-sang))/(eang-sang);
			AddPoint(u,nw);
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
void            Path::RecCubicTo( const NR::Point &iS,double sw, const NR::Point &isD, const NR::Point &iE,double ew, const NR::Point &ieD,double tresh,int lev,double maxL)
{
	NR::Point  se=iE-iS;
	double dC=NR::L2(se);
	if ( dC < 0.01 ) {
		double sC=dot(isD,isD);
		double eC=dot(ieD,ieD);
		if ( sC < tresh && eC < tresh ) return;
	} else {
		const double sC = fabs(cross(se,isD)) / dC;
		const double eC = fabs(cross(se,ieD)) / dC;
		if ( sC < tresh && eC < tresh ) {
			// presque tt droit -> attention si on nous demande de bien subdiviser les petits segments
			if ( maxL > 0 && dC > maxL ) {
				if ( lev <= 0 ) return;
				NR::Point  m=0.5*(iS+iE)+0.125*(isD-ieD);
				NR::Point  md=0.75*(iE-iS)-0.125*(isD+ieD);
				double   mw=(sw+ew)/2;
				
				NR::Point  hisD=0.5*isD;
				NR::Point  hieD=0.5*ieD;
				
				RecCubicTo(iS,sw,hisD,m,mw,md,tresh,lev-1,maxL);
				AddPoint(m,mw);
				RecCubicTo(m,mw,md,iE,ew,hieD,tresh,lev-1,maxL);
			}
			return;
		}
	}
		
	if ( lev <= 0 ) return;
	{
		NR::Point  m=0.5*(iS+iE)+0.125*(isD-ieD);
		NR::Point  md=0.75*(iE-iS)-0.125*(isD+ieD);
		double   mw=(sw+ew)/2;
				
		NR::Point  hisD=0.5*isD;
		NR::Point  hieD=0.5*ieD;

		RecCubicTo(iS,sw,hisD,m,mw,md,tresh,lev-1,maxL);
		AddPoint(m,mw);
		RecCubicTo(m,mw,md,iE,ew,hieD,tresh,lev-1,maxL);
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

void Path::RecBezierTo(const NR::Point &iP,
		       double pw,
		       const NR::Point &iS,
		       double sw,
		       const NR::Point &iE,
		       double ew,double tresh,int lev,double maxL)
{
	if ( lev <= 0 ) return;
	NR::Point ps=iS-iP;
	NR::Point pe=iE-iP;
	NR::Point se=iE-iS;
	const double s = fabs(cross(pe,ps));
	if ( s < tresh ) {
		double l=NR::L2(se);
		if ( maxL > 0 && l > maxL ) {
			double  mw=0.25*(sw+ew+2*pw);
			NR::Point  m = 0.25*(iS+iE+2*iP);
			NR::Point  md = 0.5*(iS+iP);
			double  mdw=(sw+pw)/2;
			RecBezierTo(md,mdw,iS,sw,m,mw,tresh,lev-1,maxL);
			AddPoint(m,mw);
			md=0.5*(iP+iE);
			mdw=(pw+ew)/2;
			RecBezierTo(md,mdw,m,mw,iE,ew,tresh,lev-1,maxL);	
		}
		return;
	}
	
	{
		double      mw=0.25*(sw+ew+2*pw);
		NR::Point  m=0.25*(iS+iE+2*iP);
		NR::Point  md=0.5*(iS+iP);
		double      mdw=(sw+pw)/2;
		RecBezierTo(md,mdw,iS,sw,m,mw,tresh,lev-1,maxL);
		AddPoint(m,mw);
		md=0.5*(iP+iE);
		mdw=(pw+ew)/2;
		RecBezierTo(md,mdw,m,mw,iE,ew,tresh,lev-1,maxL);	
	}
}

void Path::DoArc(NR::Point const &iS, NR::Point const &iE,
		 double rx,double ry,double angle,
		 bool large,bool wise,double tresh,int piece)
{
	if ( rx <= 0.0001 || ry <= 0.0001 ) return; // on ajoute toujours un lineto apres, donc c bon
	
	double       sang,eang;
	NR::Point   dr;
	ArcAnglesAndCenter(iS,iE,rx,ry,angle,large,wise,sang,eang,dr);
  
	NR::Point  ca(cos(angle),sin(angle));
	if ( wise ) {
		if ( sang < eang ) sang+=2*M_PI;
		for (double b=sang-0.1;b>eang;b-=0.1) {
			NR::Point  cb(cos(b),sin(b));
			NR::Point  ar(rx,ry);
			NR::Point  u=ca^cb;
			u*=ar;
			u+=dr;
			AddPoint(u,piece,(sang-b)/(sang-eang));
		}
	} else {
		if ( sang > eang ) sang-=2*M_PI;
		for (double b=sang+0.1;b<eang;b+=0.1) {
			NR::Point  cb(cos(b),sin(b));
			NR::Point  ar(rx,ry);
			NR::Point  u=ca^cb;
			u*=ar;
			u+=dr;
			AddPoint(u,piece,(b-sang)/(eang-sang));
		}
	}
}
void Path::DoArc(NR::Point const &iS,double sw, NR::Point const &iE,double ew,
		 double rx,double ry,double angle,
		 bool large,bool wise,double tresh,int piece)
{
	if ( rx <= 0.0001 || ry <= 0.0001 ) return; // on ajoute toujours un lineto apres, donc c bon

	double       sang,eang;
	NR::Point   dr;
	ArcAnglesAndCenter(iS,iE,rx,ry,angle,large,wise,sang,eang,dr);
  
	NR::Point  ca(cos(angle),sin(angle));
	if ( wise ) {
		if ( sang < eang ) sang+=2*M_PI;
		for (double b=sang-0.1;b>eang;b-=0.1) {
			NR::Point  cb(cos(b),sin(b));
			NR::Point  ar(rx,ry);
			NR::Point  u=ca^cb;
			u*=ar;
			u+=dr;
			double  nw=(sw*(b-eang)+ew*(sang-b))/(sang-eang);
			AddPoint(u,nw,piece,(sang-b)/(sang-eang));
		}
	} else {
		if ( sang > eang ) sang-=2*M_PI;
		for (double b=sang+0.1;b<eang;b+=0.1) {
			NR::Point  cb(cos(b),sin(b));
			NR::Point  ar(rx,ry);
			NR::Point  u=ca^cb;
			u*=ar;
			u+=dr;
			double  nw=(sw*(eang-b)+ew*(b-sang))/(eang-sang);
			AddPoint(u,nw,piece,(b-sang)/(eang-sang));
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
void Path::RecCubicTo(NR::Point const &iS,double sw,
		      NR::Point const &isD, NR::Point const &iE, double ew, 
		      NR::Point const &ieD, double tresh,int lev,double st,double et,int piece)
{
	const NR::Point se = iE - iS;
	const double dC = NR::L2(se);
	if ( dC < 0.01 ) {
		const double sC = dot(isD,isD);
		const double eC = dot(ieD,ieD);
		if ( sC < tresh && eC < tresh ) return;
	} else {
		const double sC = fabs(cross(se,isD)) / dC;
		const double eC = fabs(cross(se,ieD)) / dC;
		if ( sC < tresh && eC < tresh ) return;
	}
		
	if ( lev <= 0 ) return;
	const NR::Point m = 0.5*(iS+iE)+0.125*(isD-ieD);
	const NR::Point md = 0.75*(iE-iS)-0.125*(isD+ieD);
	const double mt = (st+et)/2;
	const double mw = (ew+sw)/2;
	
	const NR::Point hisD=0.5*isD;
	const NR::Point hieD=0.5*ieD;
	
	RecCubicTo(iS,sw,hisD,m,mw,md,tresh,lev-1,st,mt,piece);
	AddPoint(m,mw,piece,mt);
	RecCubicTo(m,mw,md,iE,ew,hieD,tresh,lev-1,mt,et,piece);
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
void Path::RecBezierTo(NR::Point const &iP, double pw, 
		       NR::Point const &iS, double sw,
		       NR::Point const &iE, double ew,
		       double tresh, int lev, double st, double et, int piece)
{
	if ( lev <= 0 ) return;
	const NR::Point ps=iS-iP;
	const NR::Point pe=iE-iP;
	const NR::Point se=iE-iS;
	const double s = fabs(cross(pe,ps));
	if ( s < tresh ) return;
	
	{
		const double mw = 0.25*(sw+ew+2*pw),mt=(st+et)/2;
		const NR::Point m = 0.25*(iS+iE+2*iP);
		RecBezierTo(0.5*(iS+iP), (sw+pw)/2,
			    iS,sw,m,mw,tresh,lev-1,st,mt,piece);
		AddPoint(m,piece,mt);
		RecBezierTo(0.5*(iP+iE),(pw+ew)/2,
			    m,mw,iE,ew,tresh,lev-1,mt,et,piece);	
	}
}

void Path::DoArc(NR::Point const &iS,NR::Point const &iE,
		 double rx, double ry, double angle,
		 bool large, bool wise, double tresh,
		 int piece, offset_orig& orig)
{
	// on n'arrivera jamais ici, puisque les offsets sont fait de cubiques
	if ( rx <= 0.0001 || ry <= 0.0001 ) return; // on ajoute toujours un lineto apres, donc c bon
	
	double       sang,eang;
	NR::Point   dr;
	ArcAnglesAndCenter(iS,iE,rx,ry,angle,large,wise,sang,eang,dr);
  
	NR::Point  ca(cos(angle),sin(angle));
	if ( wise ) {
		if ( sang < eang ) sang+=2*M_PI;
		for (double b=sang-0.1;b>eang;b-=0.1) {
			NR::Point  cb(cos(b),sin(b));
			NR::Point  ar(rx,ry);
			NR::Point  u=ca^cb;
			u*=ar;
			AddPoint(u + dr,piece,(sang-b)/(sang-eang));
		}
	} else {
		if ( sang > eang ) sang-=2*M_PI;
		for (double b=sang+0.1;b<eang;b+=0.1) {
			NR::Point  cb(cos(b),sin(b));
			NR::Point  ar(rx,ry);
			NR::Point  u=ca^cb;
			u*=ar;
			AddPoint(u + dr,piece,(b-sang)/(eang-sang));
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
 * conversions
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
	} else {
		if ( back ) {
			if ( weighted ) {
				// !invert && back && weighted
				for (int i=0;i<nbPt;i++) dest->AddPoint(((path_lineto_wb*)pts)[i].p);
				int               lastM=0;
				int								curP=1;
				int               pathEnd=0;
				bool              closed=false;
				int               lEdge=-1;
				while ( curP < nbPt ) {
					path_lineto_wb*    sbp=((path_lineto_wb*)pts)+curP;
					path_lineto_wb*    lm=((path_lineto_wb*)pts)+lastM;
					path_lineto_wb*    prp=((path_lineto_wb*)pts)+pathEnd;
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
						if ( fabs(sbp->p.pt[0]-prp->p.pt[0]) < 0.00001 && fabs(sbp->p.pt[1]-prp->p.pt[1]) < 0.00001 ) {
						} else {
							lEdge=dest->AddEdge(first+pathEnd,first+curP);
							dest->ebData[lEdge].pathID=pathID;
							dest->ebData[lEdge].pieceID=sbp->piece;
							if ( sbp->piece == prp->piece ) {
								dest->ebData[lEdge].tSt=prp->t;
								dest->ebData[lEdge].tEn=sbp->t;
							} else {
								dest->ebData[lEdge].tSt=0.0;
								dest->ebData[lEdge].tEn=1.0;
							}
							pathEnd=curP;
							if ( fabs(sbp->p.pt[0]-lm->p.pt[0]) < 0.00001 && fabs(sbp->p.pt[1]-lm->p.pt[1]) < 0.00001 ) {
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
						path_lineto_wb*    lm=((path_lineto_wb*)pts)+lastM;
						lEdge=dest->AddEdge(first+pathEnd,first+lastM);
						dest->ebData[lEdge].pathID=pathID;
						dest->ebData[lEdge].pieceID=lm->piece;
						dest->ebData[lEdge].tSt=0.0;
						dest->ebData[lEdge].tEn=1.0;
					}
				}
			} else {
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
						if ( fabs(sbp->p.pt[0]-prp->p.pt[0]) < 0.00001 && fabs(sbp->p.pt[1]-prp->p.pt[1]) < 0.00001 ) {
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
							if ( fabs(sbp->p.pt[0]-lm->p.pt[0]) < 0.00001 && fabs(sbp->p.pt[1]-lm->p.pt[1]) < 0.00001 ) {
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
			if ( weighted ) {
				// !invert && !back && weighted
				for (int i=0;i<nbPt;i++) dest->AddPoint(((path_lineto_w*)pts)[i].p);
				int               lastM=0;
				int								curP=1;
				int               pathEnd=0;
				bool              closed=false;
				int               lEdge=-1;
				while ( curP < nbPt ) {
					path_lineto_w*    sbp=((path_lineto_w*)pts)+curP;
					path_lineto_w*    lm=((path_lineto_w*)pts)+lastM;
					path_lineto_w*    prp=((path_lineto_w*)pts)+pathEnd;
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
						if ( fabs(sbp->p.pt[0]-prp->p.pt[0]) < 0.00001 && fabs(sbp->p.pt[1]-prp->p.pt[1]) < 0.00001 ) {
						} else {
							lEdge=dest->AddEdge(first+pathEnd,first+curP);
							pathEnd=curP;
							if ( fabs(sbp->p.pt[0]-lm->p.pt[0]) < 0.00001 && fabs(sbp->p.pt[1]-lm->p.pt[1]) < 0.00001 ) {
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
			} else {
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
						if ( fabs(sbp->p.pt[0]-prp->p.pt[0]) < 0.00001 && fabs(sbp->p.pt[1]-prp->p.pt[1]) < 0.00001 ) {
						} else {
							lEdge=dest->AddEdge(first+pathEnd,first+curP);
							pathEnd=curP;
							if ( fabs(sbp->p.pt[0]-lm->p.pt[0]) < 0.00001 && fabs(sbp->p.pt[1]-lm->p.pt[1]) < 0.00001 ) {
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
