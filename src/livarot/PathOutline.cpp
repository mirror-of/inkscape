/*
 *  PathOutline.cpp
 *  nlivarot
 *
 *  Created by fred on Fri Nov 28 2003.
 *  Copyright (c) 2003 __MyCompanyName__. All rights reserved.
 *
 */

#include "Path.h"
#include "MyMath.h"
#include <math.h>

void						Path::Outline(Path* dest,float width,JoinType join,ButtType butt,float miter)
{
	if ( descr_flags&descr_adding_bezier ) CancelBezier();
	if ( descr_nb <= 2 ) return;
	if ( dest == NULL ) return;
	dest->Reset();
	dest->SetWeighted(false);
	dest->SetBackData(false);
		
	outline_callbacks  calls;
	vec2    endButt,endPos;
	calls.cubicto=StdCubicTo;
	calls.bezierto=StdBezierTo;
	calls.arcto=StdArcTo;
	
	path_descr* sav_descr=descr_data;
	int         sav_descr_nb=descr_nb;
	
	Path*  rev=new Path;
	
	rev->SetWeighted(false);
	int   curP=0;
	do {
		int   lastM=curP;
		do {
			curP++;
			if ( curP >= sav_descr_nb ) break;
			int typ=sav_descr[curP].flags&descr_type_mask;
			if ( typ == descr_moveto ) break;
		} while ( curP < sav_descr_nb );
		if ( curP >= sav_descr_nb ) curP=sav_descr_nb;
		if ( curP > lastM+1 ) {
			// sinon il n'y a qu'un point
			int   curD=curP-1;
			float curX,curY;
			float nextX,nextY;
			bool  needClose=false;
			int   firstTyp=sav_descr[curD].flags&descr_type_mask;
			if ( firstTyp == descr_close ) needClose=true;
			while ( curD > lastM && (sav_descr[curD].flags&descr_type_mask) == descr_close ) curD--;
			int   realP=curD+1;
			if ( curD > lastM ) {
				descr_data=sav_descr;
				descr_nb=sav_descr_nb;
				PrevPoint(curD,curX,curY);
				rev->Reset();
				rev->MoveTo(curX,curY);
				while ( curD > lastM ) {
					int typ=sav_descr[curD].flags&descr_type_mask;
					if ( typ == descr_moveto ) {
						//						rev->Close();
						curD--;
					} else if ( typ == descr_lineto ) {
						PrevPoint(curD-1,nextX,nextY);
						rev->LineTo(nextX,nextY);
						curX=nextX;
						curY=nextY;
						curD--;
					} else if ( typ == descr_cubicto ) {
						PrevPoint(curD-1,nextX,nextY);
						rev->CubicTo(nextX,nextY,-sav_descr[curD].d.c.enDx,-sav_descr[curD].d.c.enDy
									 ,-sav_descr[curD].d.c.stDx,-sav_descr[curD].d.c.stDy);
						curX=nextX;
						curY=nextY;
						curD--;
					} else if ( typ == descr_arcto ) {
						PrevPoint(curD-1,nextX,nextY);
						rev->ArcTo(nextX,nextY,sav_descr[curD].d.a.rx,sav_descr[curD].d.a.ry,sav_descr[curD].d.a.angle,
								 sav_descr[curD].d.a.large,!sav_descr[curD].d.a.clockwise);
						curX=nextX;
						curY=nextY;
						curD--;
					} else if ( typ == descr_bezierto ) {
						PrevPoint(curD-1,nextX,nextY);
						rev->LineTo(nextX,nextY);
						curX=nextX;
						curY=nextY;
						curD--;
					} else if ( typ == descr_interm_bezier ) {
						int  nD=curD-1;
						while ( nD > lastM && (sav_descr[nD].flags&descr_type_mask) != descr_bezierto ) nD--;
						if ( (sav_descr[nD].flags&descr_type_mask) != descr_bezierto ) {
							// pas trouve le debut!?
							PrevPoint(nD,nextX,nextY);
							rev->LineTo(nextX,nextY);
							curX=nextX;
							curY=nextY;
						} else {
							PrevPoint(nD-1,nextX,nextY);
							rev->BezierTo(nextX,nextY);
							for (int i=curD;i>nD;i--) rev->IntermBezierTo(sav_descr[i].d.i.x,sav_descr[i].d.i.y);
							rev->EndBezierTo();
							curX=nextX;
							curY=nextY;
						}
						curD=nD-1;
					} else {
						curD--;
					}
				}
				if ( needClose ) {
					rev->Close();
					rev->SubContractOutline(dest,calls,0.0025*width*width,width,join,butt,miter,true,false,endPos,endButt);
					descr_data=sav_descr+lastM;
					descr_nb=realP+1-lastM;
					SubContractOutline(dest,calls,0.0025*width*width,width,join,butt,miter,true,false,endPos,endButt);
				} else {
					rev->SubContractOutline(dest,calls,0.0025*width*width,width,join,butt,miter,false,false,endPos,endButt);
					vec2 endNor;
					RotCCWTo(endButt,endNor);
					if ( butt == butt_round ) {
						dest->ArcTo(endPos.x+width*endNor.x,endPos.y+width*endNor.y,1.0001*width,1.0001*width,0.0,true,true);
					} else if ( butt == butt_square ) {
						dest->LineTo(endPos.x-width*endNor.x+width*endButt.x,endPos.y-width*endNor.y+width*endButt.y);
						dest->LineTo(endPos.x+width*endNor.x+width*endButt.x,endPos.y+width*endNor.y+width*endButt.y);
						dest->LineTo(endPos.x+width*endNor.x,endPos.y+width*endNor.y);
					} else if ( butt == butt_pointy ) {
						dest->LineTo(endPos.x+width*endButt.x,endPos.y+width*endButt.y);
						dest->LineTo(endPos.x+width*endNor.x,endPos.y+width*endNor.y);
					} else {
						dest->LineTo(endPos.x+width*endNor.x,endPos.y+width*endNor.y);
					}
					descr_data=sav_descr+lastM;
					descr_nb=realP-lastM;
					SubContractOutline(dest,calls,0.0025*width*width,width,join,butt,miter,false,true,endPos,endButt);
					RotCCWTo(endButt,endNor);
					if ( butt == butt_round ) {
						dest->ArcTo(endPos.x+width*endNor.x,endPos.y+width*endNor.y,1.0001*width,1.0001*width,0.0,true,true);
					} else if ( butt == butt_square ) {
						dest->LineTo(endPos.x-width*endNor.x+width*endButt.x,endPos.y-width*endNor.y+width*endButt.y);
						dest->LineTo(endPos.x+width*endNor.x+width*endButt.x,endPos.y+width*endNor.y+width*endButt.y);
						dest->LineTo(endPos.x+width*endNor.x,endPos.y+width*endNor.y);
					} else if ( butt == butt_pointy ) {
						dest->LineTo(endPos.x+width*endButt.x,endPos.y+width*endButt.y);
						dest->LineTo(endPos.x+width*endNor.x,endPos.y+width*endNor.y);
					} else {
						dest->LineTo(endPos.x+width*endNor.x,endPos.y+width*endNor.y);
					}
					dest->Close();
				}
			}
		}
	} while ( curP < sav_descr_nb );
	
	delete rev;
	descr_data=sav_descr;
	descr_nb=sav_descr_nb;
	
}
void            Path::OutsideOutline(Path* dest,float width,JoinType join,ButtType butt,float miter)
{
	if ( descr_flags&descr_adding_bezier ) CancelBezier();
	if ( descr_nb <= 2 ) return;
	if ( dest == NULL ) return;
	dest->Reset();
	dest->SetWeighted(false);
	dest->SetBackData(false);
		
	outline_callbacks  calls;
	vec2    endButt,endPos;
	calls.cubicto=StdCubicTo;
	calls.bezierto=StdBezierTo;
	calls.arcto=StdArcTo;
	SubContractOutline(dest,calls,0.0025*width*width,width,join,butt,miter,true,false,endPos,endButt);
}
void            Path::InsideOutline(Path* dest,float width,JoinType join,ButtType butt,float miter)
{
	if ( descr_flags&descr_adding_bezier ) CancelBezier();
	if ( descr_nb <= 2 ) return;
	if ( dest == NULL ) return;
	dest->Reset();
	dest->SetWeighted(false);
	dest->SetBackData(false);
	
	outline_callbacks  calls;
	vec2    endButt,endPos;
	calls.cubicto=StdCubicTo;
	calls.bezierto=StdBezierTo;
	calls.arcto=StdArcTo;

	path_descr* sav_descr=descr_data;
	int         sav_descr_nb=descr_nb;

	Path*  rev=new Path;
	
	rev->SetWeighted(false);
	int   curP=0;
	do {
		int   lastM=curP;
		do {
			curP++;
			if ( curP >= sav_descr_nb ) break;
			int typ=sav_descr[curP].flags&descr_type_mask;
			if ( typ == descr_moveto ) break;
		} while ( curP < sav_descr_nb );
		if ( curP >= sav_descr_nb ) curP=sav_descr_nb;
		if ( curP > lastM+1 ) {
			// sinon il n'y a qu'un point
			int   curD=curP-1;
			float curX,curY;
			float nextX,nextY;
			while ( curD > lastM && (sav_descr[curD].flags&descr_type_mask) == descr_close ) curD--;
			if ( curD > lastM ) {
				descr_data=sav_descr;
				descr_nb=sav_descr_nb;
				PrevPoint(curD,curX,curY);
				rev->Reset();
				rev->MoveTo(curX,curY);
				while ( curD > lastM ) {
					int typ=sav_descr[curD].flags&descr_type_mask;
					if ( typ == descr_moveto ) {
						rev->Close();
						curD--;
					} else if ( typ == descr_lineto ) {
						PrevPoint(curD-1,nextX,nextY);
						rev->LineTo(nextX,nextY);
						curX=nextX;
						curY=nextY;
						curD--;
					} else if ( typ == descr_cubicto ) {
						PrevPoint(curD-1,nextX,nextY);
						rev->CubicTo(nextX,nextY,-sav_descr[curD].d.c.enDx,-sav_descr[curD].d.c.enDy
									 ,-sav_descr[curD].d.c.stDx,-sav_descr[curD].d.c.stDy);
						curX=nextX;
						curY=nextY;
						curD--;
					} else if ( typ == descr_arcto ) {
						PrevPoint(curD-1,nextX,nextY);
						rev->ArcTo(nextX,nextY,sav_descr[curD].d.a.rx,sav_descr[curD].d.a.ry,sav_descr[curD].d.a.angle,
								 sav_descr[curD].d.a.large,!sav_descr[curD].d.a.clockwise);
						curX=nextX;
						curY=nextY;
						curD--;
					} else if ( typ == descr_bezierto ) {
						PrevPoint(curD-1,nextX,nextY);
						rev->LineTo(nextX,nextY);
						curX=nextX;
						curY=nextY;
						curD--;
					} else if ( typ == descr_interm_bezier ) {
						int  nD=curD-1;
						while ( nD > lastM && (sav_descr[nD].flags&descr_type_mask) != descr_bezierto ) nD--;
						if ( sav_descr[nD].flags&descr_type_mask != descr_bezierto ) {
							// pas trouve le debut!?
							PrevPoint(nD,nextX,nextY);
							rev->LineTo(nextX,nextY);
							curX=nextX;
							curY=nextY;
						} else {
							PrevPoint(nD-1,nextX,nextY);
							rev->BezierTo(nextX,nextY);
							for (int i=curD;i>nD;i--) rev->IntermBezierTo(sav_descr[i].d.i.x,sav_descr[i].d.i.y);
							rev->EndBezierTo();
							curX=nextX;
							curY=nextY;
						}
						curD=nD-1;
					} else {
						curD--;
					}
				}
				rev->Close();
				rev->SubContractOutline(dest,calls,0.0025*width*width,width,join,butt,miter,true,false,endPos,endButt);
			}
		}
	} while ( curP < sav_descr_nb );
	
	delete rev;
	descr_data=sav_descr;
	descr_nb=sav_descr_nb;
}
void            Path::DoOutsideOutline(Path* dest,float width,JoinType join,ButtType butt,float miter,int &stNo,int &enNo)
{
}
void            Path::DoInsideOutline(Path* dest,float width,JoinType join,ButtType butt,float miter,int &stNo,int &enNo)
{
}
void						Path::SubContractOutline(Path* dest,outline_callbacks &calls,float tolerance,float width,JoinType join,ButtType butt,float miter,bool closeIfNeeded,bool skipMoveto,vec2 &lastP,vec2 &lastT)
{
	float    curX,curY,curW;
	int      curP=1;
	vec2     curT;
	vec2     firstP,firstT;
	bool     doFirst;
	outline_callback_data  callsData;
	
	callsData.orig=this;
	callsData.dest=dest;
	
	// le moveto
	curX=(descr_data)->d.m.x;
	curY=(descr_data)->d.m.y;
	if ( (descr_data)->flags&descr_weighted ) {
		curW=(descr_data)->d.m.w;
	} else {
		curW=1;
	}
	curT.x=curT.y=0;
	
	doFirst=true;
	firstP.x=firstP.y=0;
	firstT.x=firstT.y=0;
	
		// et le reste, 1 par 1
	while ( curP < descr_nb ) {
		path_descr*  curD=descr_data+curP;
		int          nType=curD->flags&descr_type_mask;
		bool         nWeight=curD->flags&descr_weighted;
		float        nextX,nextY,nextW;
		vec2         stPos,enPos,stTgt,enTgt,stNor,enNor;
		float        stRad,enRad,stTle,enTle;
		if ( nType == descr_moveto ) {
			nextX=curD->d.m.x;
			nextY=curD->d.m.y;
			if ( nWeight ) nextW=curD->d.m.w; else nextW=1;
			// et on avance
			if ( doFirst ) {
			} else {
				if ( closeIfNeeded ) {
					if ( fabsf(curX-firstP.x) < 0.0001 && fabsf(curY-firstP.y) < 0.0001 ) {
						OutlineJoin(dest,firstP,curT,firstT,width,join,miter);
						dest->Close();
					} else {
						path_descr_lineto  temp;
						temp.x=firstP.x;
						temp.y=firstP.y;
						
						TangentOnSegAt(0.0,curX,curY,temp,stPos,stTgt,stTle);
						TangentOnSegAt(1.0,curX,curY,temp,enPos,enTgt,enTle);
						RotCWTo(stTgt,stNor);
						RotCWTo(enTgt,enNor);
						
						// jointure
						{
							vec2  pos;
							pos.x=curX;
							pos.y=curY;
							OutlineJoin(dest,pos,curT,stNor,width,join,miter);
						}
						
						dest->LineTo(enPos.x+width*enNor.x,enPos.y+width*enNor.y);
						
						// jointure
						{
							vec2  pos;
							pos=firstP;
							OutlineJoin(dest,enPos,enNor,firstT,width,join,miter);
							dest->Close();
						}
					}
				}
			}
			firstP.x=nextX;
			firstP.y=nextY;
			curP++;
		} else if ( nType == descr_close ) {
			if ( doFirst == false ) {
				if ( fabsf(curX-firstP.x) < 0.0001 && fabsf(curY-firstP.y) < 0.0001 ) {
					OutlineJoin(dest,firstP,curT,firstT,width,join,miter);
					dest->Close();
				} else {
					path_descr_lineto  temp;
					temp.x=firstP.x;
					temp.y=firstP.y;
					nextX=firstP.x;
					nextY=firstP.y;
					
					TangentOnSegAt(0.0,curX,curY,temp,stPos,stTgt,stTle);
					TangentOnSegAt(1.0,curX,curY,temp,enPos,enTgt,enTle);
					RotCWTo(stTgt,stNor);
					RotCWTo(enTgt,enNor);
					
					// jointure
					{
						OutlineJoin(dest,stPos,curT,stNor,width,join,miter);
					}
					
					dest->LineTo(enPos.x+width*enNor.x,enPos.y+width*enNor.y);
					
					// jointure
					{
						OutlineJoin(dest,enPos,enNor,firstT,width,join,miter);
						dest->Close();
					}
				}
			}
			doFirst=true;
			curP++;
		} else if ( nType == descr_lineto ) {
			nextX=curD->d.l.x;
			nextY=curD->d.l.y;
			if ( nWeight ) nextW=curD->d.l.w; else nextW=1;
			// test de nullité du segment
			if ( IsNulCurve(curD,curX,curY) ) {
				curP++;
				continue;
			}
			// et on avance
			TangentOnSegAt(0.0,curX,curY,curD->d.l,stPos,stTgt,stTle);
			TangentOnSegAt(1.0,curX,curY,curD->d.l,enPos,enTgt,enTle);
			RotCWTo(stTgt,stNor);
			RotCWTo(enTgt,enNor);
			
			lastP=enPos;
			lastT=enTgt;

			if ( doFirst ) {
				doFirst=false;
				firstP=stPos;
				firstT=stNor;
				if ( skipMoveto ) {
					skipMoveto=false;
				} else dest->MoveTo(curX+width*stNor.x,curY+width*stNor.y);
			} else {
				// jointure
				vec2  pos;
				pos.x=curX;pos.y=curY;
				OutlineJoin(dest,pos,curT,stNor,width,join,miter);
			}
			
			dest->LineTo(nextX+width*enNor.x,nextY+width*enNor.y);
			
			curP++;
		} else if ( nType == descr_cubicto ) {
			nextX=curD->d.c.x;
			nextY=curD->d.c.y;
			if ( nWeight ) nextW=curD->d.c.w; else nextW=1;
			// test de nullité du segment
			if ( IsNulCurve(curD,curX,curY) ) {
				curP++;
				continue;
			}
			// et on avance
			TangentOnCubAt(0.0,curX,curY,curD->d.c,false,stPos,stTgt,stTle,stRad);
			TangentOnCubAt(1.0,curX,curY,curD->d.c,true,enPos,enTgt,enTle,enRad);
			RotCWTo(stTgt,stNor);
			RotCWTo(enTgt,enNor);
			
			lastP=enPos;
			lastT=enTgt;

			if ( doFirst ) {
				doFirst=false;
				firstP=stPos;
				firstT=stNor;
				if ( skipMoveto ) {
					skipMoveto=false;
				} else dest->MoveTo(curX+width*stNor.x,curY+width*stNor.y);
			} else {
				// jointure
				vec2  pos;
				pos.x=curX;pos.y=curY;
				OutlineJoin(dest,pos,curT,stNor,width,join,miter);
			}
			
			callsData.x1=curX;
			callsData.y1=curY;
			callsData.x2=nextX;
			callsData.y2=nextY;
			callsData.d.c.dx1=curD->d.c.stDx;
			callsData.d.c.dy1=curD->d.c.stDy;
			callsData.d.c.dx2=curD->d.c.enDx;
			callsData.d.c.dy2=curD->d.c.enDy;
			(calls.cubicto)(&callsData,tolerance,width);
			
			curP++;
		} else if ( nType == descr_arcto ) {
			nextX=curD->d.a.x;
			nextY=curD->d.a.y;
			if ( nWeight ) nextW=curD->d.a.w; else nextW=1;
			// test de nullité du segment
			if ( IsNulCurve(curD,curX,curY) ) {
				curP++;
				continue;
			}
			// et on avance
			TangentOnArcAt(0.0,curX,curY,curD->d.a,stPos,stTgt,stTle,stRad);
			TangentOnArcAt(1.0,curX,curY,curD->d.a,enPos,enTgt,enTle,enRad);
			RotCWTo(stTgt,stNor);
			RotCWTo(enTgt,enNor);
			
			lastP=enPos;
			lastT=enTgt; // tjs definie
			
			if ( doFirst ) {
				doFirst=false;
				firstP=stPos;
				firstT=stNor;
				if ( skipMoveto ) {
					skipMoveto=false;
				} else dest->MoveTo(curX+width*stNor.x,curY+width*stNor.y);
			} else {
				// jointure
				vec2  pos;
				pos.x=curX;pos.y=curY;
				OutlineJoin(dest,pos,curT,stNor,width,join,miter);
			}
			
			callsData.x1=curX;
			callsData.y1=curY;
			callsData.x2=nextX;
			callsData.y2=nextY;
			callsData.d.a.rx=curD->d.a.rx;
			callsData.d.a.ry=curD->d.a.ry;
			callsData.d.a.angle=curD->d.a.angle;
			callsData.d.a.clock=curD->d.a.clockwise;
			callsData.d.a.large=curD->d.a.large;
			(calls.arcto)(&callsData,tolerance,width);
			
			curP++;
		} else if ( nType == descr_bezierto ) {
			int   nbInterm=curD->d.b.nb;
			nextX=curD->d.b.x;
			nextY=curD->d.b.y;
			if ( nWeight ) nextW=curD->d.b.w; else nextW=1;
			
			if ( IsNulCurve(curD,curX,curY) ) {
				curP+=nbInterm+1;
				continue;
			}

			path_descr*  bezStart=curD;
			curP++;
			curD=descr_data+curP;
			path_descr* intermPoints=curD;
			
			if ( nbInterm <= 0 ) {
				// et on avance
				path_descr_lineto  temp;
				temp.x=nextX;
				temp.y=nextY;
				TangentOnSegAt(0.0,curX,curY,temp,stPos,stTgt,stTle);
				TangentOnSegAt(1.0,curX,curY,temp,enPos,enTgt,enTle);
				RotCWTo(stTgt,stNor);
				RotCWTo(enTgt,enNor);
				
				lastP=enPos;
				lastT=enTgt;
				
				if ( doFirst ) {
					doFirst=false;
					firstP=stPos;
					firstT=stNor;
					if ( skipMoveto ) {
						skipMoveto=false;
					} else dest->MoveTo(curX+width*stNor.x,curY+width*stNor.y);
				} else {
					// jointure
					vec2  pos;
					pos.x=curX;pos.y=curY;
					if ( stTle > 0 ) OutlineJoin(dest,pos,curT,stNor,width,join,miter);
				}
				dest->LineTo(nextX+width*enNor.x,nextY+width*enNor.y);
			} else if ( nbInterm == 1 ) {
				float midX,midY,midW;
				midX=intermPoints->d.i.x;
				midY=intermPoints->d.i.y;
				if ( nWeight ) {
					midW=intermPoints->d.i.w;
				} else {
					midW=1;
				}
				// et on avance
				TangentOnBezAt(0.0,curX,curY,intermPoints->d.i,bezStart->d.b,false,stPos,stTgt,stTle,stRad);
				TangentOnBezAt(1.0,curX,curY,intermPoints->d.i,bezStart->d.b,true,enPos,enTgt,enTle,enRad);
				RotCWTo(stTgt,stNor);
				RotCWTo(enTgt,enNor);
				
				lastP=enPos;
				lastT=enTgt;
				
				if ( doFirst ) {
					doFirst=false;
					firstP=stPos;
					firstT=stNor;
					if ( skipMoveto ) {
						skipMoveto=false;
					} else dest->MoveTo(curX+width*stNor.x,curY+width*stNor.y);
				} else {
					// jointure
					vec2  pos;
					pos.x=curX;pos.y=curY;
					OutlineJoin(dest,pos,curT,stNor,width,join,miter);
				}
				
				callsData.x1=curX;
				callsData.y1=curY;
				callsData.x2=nextX;
				callsData.y2=nextY;
				callsData.d.b.mx=midX;
				callsData.d.b.my=midY;
				(calls.bezierto)(&callsData,tolerance,width);

			} else if ( nbInterm > 1 ) {
				float   bx=curX,by=curY,bw=curW;
				float   cx=curX,cy=curY,cw=curW;
				float   dx=curX,dy=curY,dw=curW;
								
				dx=intermPoints->d.i.x;
				dy=intermPoints->d.i.y;
				if ( nWeight ) {
					dw=intermPoints->d.i.w;
				} else {
					dw=1;
				}
				TangentOnBezAt(0.0,curX,curY,intermPoints->d.i,bezStart->d.b,false,stPos,stTgt,stTle,stRad);
				RotCWTo(stTgt,stNor);
				
				intermPoints++;
				
				// et on avance
				if ( stTle > 0 ) {
					if ( doFirst ) {
						doFirst=false;
						firstP=stPos;
						firstT=stNor;
						if ( skipMoveto ) {
							skipMoveto=false;
						} else dest->MoveTo(curX+width*stNor.x,curY+width*stNor.y);
					} else {
						// jointure
						dest->LineTo(curX+width*stNor.x,curY+width*stNor.y);
					}
				}
				
				cx=2*bx-dx;
				cy=2*by-dy;
				cw=2*bw-dw;
				
				for (int k=0;k<nbInterm-1;k++) {
					bx=cx;by=cy;bw=cw;
					cx=dx;cy=dy;cw=dw;
					
					dx=intermPoints->d.i.x;
					dy=intermPoints->d.i.y;
					if ( nWeight ) {
						dw=intermPoints->d.i.w;
					} else {
						dw=1;
					}
					intermPoints++;
					
					float  stx=(bx+cx)/2;
					float  sty=(by+cy)/2;
//					float  stw=(bw+cw)/2;
					
					path_descr_bezierto        tempb;
					path_descr_intermbezierto  tempi;
					tempb.nb=1;
					tempb.x=(cx+dx)/2;
					tempb.y=(cy+dy)/2;
					tempi.x=cx;
					tempi.y=cy;
					TangentOnBezAt(1.0,stx,sty,tempi,tempb,true,enPos,enTgt,enTle,enRad);
					RotCWTo(enTgt,enNor);

					lastP=enPos;
					lastT=enTgt;

					callsData.x1=stx;
					callsData.y1=sty;
					callsData.x2=(cx+dx)/2;
					callsData.y2=(cy+dy)/2;
					callsData.d.b.mx=cx;
					callsData.d.b.my=cy;
					(calls.bezierto)(&callsData,tolerance,width);
				}
				{
					bx=cx;by=cy;bw=cw;
					cx=dx;cy=dy;cw=dw;
					
					dx=nextX;
					dy=nextY;
					if ( nWeight ) {
						dw=nextW;
					} else {
						dw=1;
					}
					dx=2*dx-cx;
					dy=2*dy-cy;
					dw=2*dw-cw;
					
					float  stx=(bx+cx)/2;
					float  sty=(by+cy)/2;
//					float  stw=(bw+cw)/2;
					
					path_descr_bezierto        tempb;
					path_descr_intermbezierto  tempi;
					tempb.nb=1;
					tempb.x=(cx+dx)/2;
					tempb.y=(cy+dy)/2;
					tempi.x=cx;
					tempi.y=cy;
					TangentOnBezAt(1.0,stx,sty,tempi,tempb,true,enPos,enTgt,enTle,enRad);
					RotCWTo(enTgt,enNor);
					
					lastP=enPos;
					lastT=enTgt;
					
					callsData.x1=stx;
					callsData.y1=sty;
					callsData.x2=(cx+dx)/2;
					callsData.y2=(cy+dy)/2;
					callsData.d.b.mx=cx;
					callsData.d.b.my=cy;
					(calls.bezierto)(&callsData,tolerance,width);
					
				}
			}
			
			// et on avance
			curP+=nbInterm;
		}
		curX=nextX;
		curY=nextY;
		curW=nextW;
		curT=enNor; // sera tjs bien definie
	}
	if ( closeIfNeeded ) {
		if ( doFirst == false ) {
		}
	}
}
/*
 *
 * utilitaires pour l'outline
 *
 */

bool		Path::IsNulCurve(path_descr* curD,float curX,float curY)
{
	int typ=curD->flags&descr_type_mask;
	if ( typ == descr_lineto ) {
		if ( fabsf(curD->d.l.x-curX) < 0.00001 && fabsf(curD->d.l.y-curY) < 0.00001 ) {
			return true;
		}		
		return false;
	} else if ( typ == descr_cubicto ) {
		float   ax,bx,cx,dx;
		float   ay,by,cy,dy;
		ax=curD->d.c.stDx+curD->d.c.enDx+2*curX-2*curD->d.c.x;
		bx=3*curD->d.c.x-3*curX-2*curD->d.c.stDx-curD->d.c.enDx;
		cx=curD->d.c.stDx;
		dx=curX;
		ay=curD->d.c.stDy+curD->d.c.enDy+2*curY-2*curD->d.c.y;
		by=3*curD->d.c.y-3*curY-2*curD->d.c.stDy-curD->d.c.enDy;
		cy=curD->d.c.stDy;
		dy=curY;
		if ( fabsf(ax) < 0.0001 && fabsf(bx) < 0.0001 && fabsf(cx) < 0.0001 &&
			 fabsf(ay) < 0.0001 && fabsf(by) < 0.0001 && fabsf(cy) < 0.0001 ) {
			return true;
		}
		return false;
	} else if ( typ == descr_arcto ) {
		if ( fabsf(curD->d.a.x-curX) < 0.00001 && fabsf(curD->d.a.y-curY) < 0.00001 ) {
			if ( curD->d.a.large == false ) {
				return true;
			}
			if ( fabsf(curD->d.a.rx) < 0.00001 || fabsf(curD->d.a.ry) < 0.00001 ) {
				return true;
			}
		}		
		return false;
	} else if ( typ == descr_bezierto ) {
		if ( curD->d.b.nb <= 0 ) {
			if ( fabsf(curD->d.b.x-curX) < 0.00001 && fabsf(curD->d.b.y-curY) < 0.00001 ) {
				return true;
			}		
			return false;
		} else if ( curD->d.b.nb == 1 ) {
			if ( fabsf(curD->d.b.x-curX) < 0.00001 && fabsf(curD->d.b.y-curY) < 0.00001 ) {
				path_descr  *interm=curD+1;
				if ( fabsf(interm->d.i.x-curX) < 0.00001 && fabsf(interm->d.i.y-curY) < 0.00001 ) {
					return true;
				}		
			}
			return false;
		} else {
			if ( fabsf(curD->d.b.x-curX) < 0.00001 && fabsf(curD->d.b.y-curY) < 0.00001 ) {
				bool  diff=false;
				for (int i=1;i<=curD->d.b.nb;i++) {
					path_descr *interm=curD+i;
					if ( fabsf(interm->d.i.x-curX) > 0.00001 || fabsf(interm->d.i.y-curY) > 0.00001 ) {
						diff=true;
						break;
					}
				}
				if ( diff == false ) return true;
			}
			return false;
		}
	}
	return true;
}
void     Path::TangentOnSegAt(float at,float sx,float sy,path_descr_lineto& fin,vec2& pos,vec2& tgt,float &len)
{
	float ex,ey;
	ex=fin.x;
	ey=fin.y;
	vec2   seg;
	seg.x=ex-sx;
	seg.y=ey-sy;
	float   l=sqrtf(seg.x*seg.x+seg.y*seg.y);
	if ( l <= 0.000001 ) {
		pos.x=sx;
		pos.y=sy;
		tgt.x=0;
		tgt.y=0;
		len=0;
	} else {
		tgt.x=seg.x/l;
		tgt.y=seg.y/l;
		pos.x=(1-at)*sx+at*ex;
		pos.y=(1-at)*sy+at*ey;
		len=l;
	}
}
void     Path::TangentOnArcAt(float at,float sx,float sy,path_descr_arcto& fin,vec2& pos,vec2& tgt,float &len,float &rad)
{
	float ex,ey;
	ex=fin.x;
	ey=fin.y;
	float rx,ry,angle;
	rx=fin.rx;
	ry=fin.ry;
	angle=fin.angle;
	bool  large,wise;
	large=fin.large;
	wise=fin.clockwise;

	pos.x=sx;
	pos.y=sy;
	tgt.x=0;
	tgt.y=0;
	if ( rx <= 0.0001 || ry <= 0.0001 ) return;

	float   sex=ex-sx,sey=ey-sy;
	float   ca=cos(angle),sa=sin(angle);
	float   csex=ca*sex+sa*sey,csey=-sa*sex+ca*sey;
	csex/=rx;csey/=ry;
	float   l=csex*csex+csey*csey;
	if ( l >= 4 ) return;
	float   d=1-l/4;
	if ( d < 0 ) d=0;
	d=sqrt(d);
	float   csdx=csey,csdy=-csex;
	l=sqrt(l);
	csdx/=l;csdy/=l;
	csdx*=d;csdy*=d;
	
	float   sang,eang;
	float   rax=-csdx-csex/2,ray=-csdy-csey/2;
	if ( rax < -1 ) {
		sang=M_PI;
	} else if ( rax > 1 ) {
		sang=0;
	} else {
		sang=acos(rax);
		if ( ray < 0 ) sang=2*M_PI-sang;
	}
	rax=-csdx+csex/2;ray=-csdy+csey/2;
	if ( rax < -1 ) {
		eang=M_PI;
	} else if ( rax > 1 ) {
		eang=0;
	} else {
		eang=acos(rax);
		if ( ray < 0 ) eang=2*M_PI-eang;
	}
	
	csdx*=rx;csdy*=ry;
	float   drx=ca*csdx-sa*csdy,dry=sa*csdx+ca*csdy;
	
	if ( wise ) {
		if ( large == true ) {
			drx=-drx;dry=-dry;
			float  swap=eang;eang=sang;sang=swap;
			eang+=M_PI;sang+=M_PI;
			if ( eang >= 2*M_PI ) eang-=2*M_PI;
			if ( sang >= 2*M_PI ) sang-=2*M_PI;
		}
	} else {
		if ( large == false ) {
			drx=-drx;dry=-dry;
			float  swap=eang;eang=sang;sang=swap;
			eang+=M_PI;sang+=M_PI;
			if ( eang >= 2*M_PI ) eang-=2*M_PI;
			if ( sang >= 2*M_PI ) sang-=2*M_PI;
		}
	}
	drx+=(sx+ex)/2;dry+=(sy+ey)/2;
	
	if ( wise ) {
		if ( sang < eang ) sang+=2*M_PI;
		float b=sang*(1-at)+eang*at;
		float  cb=cos(b),sb=sin(b);
		pos.x=drx+ca*rx*cb-sa*ry*sb;
		pos.y=dry+sa*rx*cb+ca*ry*sb;
		tgt.x=ca*rx*sb+sa*ry*cb;
		tgt.y=sa*rx*sb-ca*ry*cb;
		vec2 dtgt;
		dtgt.x=-ca*rx*cb+sa*ry*sb;
		dtgt.y=-sa*rx*cb-ca*ry*sb;
		len=sqrtf(tgt.x*tgt.x+tgt.y*tgt.y);
		rad=len*(tgt.x*tgt.x+tgt.y*tgt.y)/(tgt.x*dtgt.y-tgt.y*dtgt.x);
		tgt.x/=len;
		tgt.y/=len;
	} else {
		if ( sang > eang ) sang-=2*M_PI;
		float b=sang*(1-at)+eang*at;
		float  cb=cos(b),sb=sin(b);
		pos.x=drx+ca*rx*cb-sa*ry*sb;
		pos.y=dry+sa*rx*cb+ca*ry*sb;
		tgt.x=ca*rx*sb+sa*ry*cb;
		tgt.y=sa*rx*sb-ca*ry*cb;
		vec2 dtgt;
		dtgt.x=-ca*rx*cb+sa*ry*sb;
		dtgt.y=-sa*rx*cb-ca*ry*sb;
		len=sqrtf(tgt.x*tgt.x+tgt.y*tgt.y);
		rad=len*(tgt.x*tgt.x+tgt.y*tgt.y)/(tgt.x*dtgt.y-tgt.y*dtgt.x);
		tgt.x/=len;
		tgt.y/=len;

	}
}
void     Path::TangentOnCubAt(float at,float sx,float sy,path_descr_cubicto& fin,bool before,vec2& pos,vec2& tgt,float &len,float &rad)
{
	float ex,ey;
	ex=fin.x;
	ey=fin.y;
	float sdx,sdy,edx,edy;
	edx=fin.enDx;
	edy=fin.enDy;
	sdx=fin.stDx;
	sdy=fin.stDy;
	
	pos.x=sx;
	pos.y=sy;
	tgt.x=0;
	tgt.y=0;
	len=rad=0;

	float   ax,bx,cx,dx;
	float   ay,by,cy,dy;
	ax=sdx+edx-2*ex+2*sx;
	bx=0.5*(edx-sdx);
	cx=0.25*(6*ex-6*sx-sdx-edx);
	dx=0.125*(4*sx+4*ex-edx+sdx);
	ay=sdy+edy-2*ey+2*sy;
	by=0.5*(edy-sdy);
	cy=0.25*(6*ey-6*sy-sdy-edy);
	dy=0.125*(4*sy+4*ey-edy+sdy);
	float   atb=at-0.5;
	pos.x=ax*atb*atb*atb+bx*atb*atb+cx*atb+dx;
	pos.y=ay*atb*atb*atb+by*atb*atb+cy*atb+dy;
	vec2  der,dder,ddder;
	der.x=3*ax*atb*atb+2*bx*atb+cx;
	der.y=3*ay*atb*atb+2*by*atb+cy;
	dder.x=6*ax*atb+2*bx;
	dder.y=6*ay*atb+2*by;
	ddder.x=6*ax;
	ddder.y=6*ay;
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
	vec2  der,dder,ddder;
	der.x=3*ax*at*at+2*bx*at+cx;
	der.y=3*ay*at*at+2*by*at+cy;
	dder.x=6*ax*at+2*bx;
	dder.y=6*ay*at+2*by;
	ddder.x=6*ax;
	ddder.y=6*ay;*/
	float   l=sqrtf(der.x*der.x+der.y*der.y);
	if ( l <= 0.0001 ) {
		len=0;
		l=sqrtf(dder.x*dder.x+dder.y*dder.y);
		if ( l <= 0.0001 ) {
			l=sqrtf(ddder.x*ddder.x+ddder.y*ddder.y);
			if ( l <= 0.0001 ) {
				// pas de segment....
				return;
			}
			rad=100000000;
			tgt.x=ddder.x/l;
			tgt.y=ddder.y/l;
			if ( before ) {tgt.x=-tgt.x;tgt.y=-tgt.y;}
			return;
		}
		rad=-l*(dder.x*dder.x+dder.y*dder.y)/(dder.x*ddder.y-dder.y*ddder.x);
		tgt.x=dder.x/l;
		tgt.y=dder.y/l;
		if ( before ) {tgt.x=-tgt.x;tgt.y=-tgt.y;}
		return;
	}
	len=l;
	
	rad=-l*(der.x*der.x+der.y*der.y)/(der.x*dder.y-der.y*dder.x);
	
	tgt.x=der.x/l;
	tgt.y=der.y/l;
}
void     Path::TangentOnBezAt(float at,float sx,float sy,path_descr_intermbezierto& mid,path_descr_bezierto& fin,bool before,vec2& pos,vec2& tgt,float &len,float &rad)
{
	float ex,ey;
	ex=fin.x;
	ey=fin.y;
	float mx,my;
	mx=mid.x;
	my=mid.y;
	
	pos.x=sx;
	pos.y=sy;
	tgt.x=0;
	tgt.y=0;
	len=rad=0;
	float   ax,bx,cx;
	float   ay,by,cy;
	ax=ex+sx-2*mx;
	bx=2*mx-2*sx;
	cx=sx;
	ay=ey+sy-2*my;
	by=2*my-2*sy;
	cy=sy;
	
	pos.x=ax*at*at+bx*at+cx;
	pos.y=ay*at*at+by*at+cy;
	vec2  der,dder;
	der.x=2*ax*at+bx;
	der.y=2*ay*at+by;
	dder.x=2*ax;
	dder.y=2*ay;
	float   l=sqrtf(der.x*der.x+der.y*der.y);
	if ( l <= 0.0001 ) {
		len=0;
		l=sqrtf(dder.x*dder.x+dder.y*dder.y);
		if ( l <= 0.0001 ) {		
			// pas de segment....
			return;
		}
		rad=100000000;
		tgt.x=dder.x/l;
		tgt.y=dder.y/l;
		if ( before ) {tgt.x=-tgt.x;tgt.y=-tgt.y;}
		return;
	}
	len=l;
	rad=-l*(der.x*der.x+der.y*der.y)/(der.x*dder.y-der.y*dder.x);
	
	tgt.x=der.x/l;
	tgt.y=der.y/l;
}
void     Path::OutlineJoin(Path* dest,vec2 pos,vec2 stNor,vec2 enNor,float width,JoinType join,float miter)
{
	float    angSi=Dot(stNor,enNor);
	float    angCo=Cross(stNor,enNor);
	// 1/1000 est tres grossier, mais sinon ca merde tout azimut
	if ( ( width >= 0 && angSi > -0.001 ) || ( width < 0 && angSi < 0.001 ) ) {
		if ( angCo > 0.999 ) {
			// tout droit
		} else if ( angCo < -0.999 ) {
			// demit-tour
			dest->LineTo(pos.x+width*enNor.x,pos.y+width*enNor.y);
		} else {
			dest->LineTo(pos.x,pos.y);
			dest->LineTo(pos.x+width*enNor.x,pos.y+width*enNor.y);
		}
	} else {
		if ( join == join_round ) {
			// utiliser des bouts de cubique: approximation de l'arc (au point ou on en est...), et supporte mieux 
			// l'arrondi des coordonnees des extremites
/*			float   angle=acos(angCo);
			if ( angCo >= 0 ) {
				vec2   stTgt,enTgt;
				RotCCWTo(stNor,stTgt);
				RotCCWTo(enNor,enTgt);
				dest->CubicTo(pos.x+width*enNor.x,pos.y+width*enNor.y,angle*width*stTgt.x,angle*width*stTgt.y,angle*width*enTgt.x,angle*width*enTgt.y);
			} else {
				vec2   biNor;
				vec2   stTgt,enTgt,biTgt;
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
			if ( width > 0 ) {
				dest->ArcTo(pos.x+width*enNor.x,pos.y+width*enNor.y,1.0001*width,1.0001*width,0.0,false,true);
			} else {
				dest->ArcTo(pos.x+width*enNor.x,pos.y+width*enNor.y,-1.0001*width,-1.0001*width,0.0,false,false);
			}
		} else if ( join == join_pointy ) {
			vec2     biss;
			biss.x=stNor.x+enNor.x;
			biss.y=stNor.y+enNor.y;
			float    lb=sqrt(biss.x*biss.x+biss.y*biss.y);
			biss.x/=lb;biss.y/=lb;
			float    angCo=Cross(biss,enNor);
			float    angSi=Dot(biss,stNor);
			float    l=width/angCo;
			if ( miter < 0.5*lb ) miter=0.5*lb;
			if ( l > miter ) {
				float r=(l-miter)*angCo/angSi;
				dest->LineTo(pos.x+miter*biss.x-r*biss.y,pos.y+miter*biss.y+r*biss.x);
				dest->LineTo(pos.x+miter*biss.x+r*biss.y,pos.y+miter*biss.y-r*biss.x);
				dest->LineTo(pos.x+width*enNor.x,pos.y+width*enNor.y);
			} else {
				dest->LineTo(pos.x+l*biss.x,pos.y+l*biss.y);
				dest->LineTo(pos.x+width*enNor.x,pos.y+width*enNor.y);
			}
		} else {
			dest->LineTo(pos.x+width*enNor.x,pos.y+width*enNor.y);
		}
	}
}

// les callbacks

void Path::RecStdCubicTo(outline_callback_data *data,float tol,float width,int lev)
{
	vec2 stPos,miPos,enPos;
	vec2 stTgt,enTgt,miTgt,stNor,enNor,miNor;
	float stRad,miRad,enRad;
	float stTle,miTle,enTle;
	// un cubic
  {
		path_descr_cubicto temp;
		temp.x=data->x2;
		temp.y=data->y2;
		temp.stDx=data->d.c.dx1;
		temp.stDy=data->d.c.dy1;
		temp.enDx=data->d.c.dx2;
		temp.enDy=data->d.c.dy2;
		TangentOnCubAt(0.0,data->x1,data->y1,temp,false,stPos,stTgt,stTle,stRad);
		TangentOnCubAt(0.5,data->x1,data->y1,temp,false,miPos,miTgt,miTle,miRad);
		TangentOnCubAt(1.0,data->x1,data->y1,temp,true,enPos,enTgt,enTle,enRad);
		RotCWTo(stTgt,stNor);
		RotCWTo(miTgt,miNor);
		RotCWTo(enTgt,enNor);
	}
	
	float  stGue=1,miGue=1,enGue=1;
	if ( fabsf(stRad) > 0.01 ) stGue+=width/stRad;
	if ( fabsf(miRad) > 0.01 ) miGue+=width/miRad;
	if ( fabsf(enRad) > 0.01 ) enGue+=width/enRad;
	stGue*=stTle;
	miGue*=miTle;
	enGue*=enTle;
	
	
	if ( lev <= 0 ) {
		data->dest->CubicTo(enPos.x+width*enNor.x,enPos.y+width*enNor.y,
											stGue*stTgt.x,stGue*stTgt.y,enGue*enTgt.x,enGue*enTgt.y);
		return;
	}
	
	vec2  req,chk;
	req.x=miPos.x+width*miNor.x;
	req.y=miPos.y+width*miNor.y;
	{
		path_descr_cubicto temp;
		float  chTle,chRad;
		vec2   chTgt;
		temp.x=enPos.x+width*enNor.x;
		temp.y=enPos.y+width*enNor.y;
		temp.stDx=stGue*stTgt.x;
		temp.stDy=stGue*stTgt.y;
		temp.enDx=enGue*enTgt.x;
		temp.enDy=enGue*enTgt.y;
		TangentOnCubAt(0.5,stPos.x+width*stNor.x,stPos.y+width*stNor.y,temp,false,chk,chTgt,chTle,chRad);		
	}
	vec2  diff;
	diff.x=req.x-chk.x;
	diff.y=req.y-chk.y;
	float   err=(diff.x*diff.x+diff.y*diff.y);
	if ( err <= tol*tol ) {
		data->dest->CubicTo(enPos.x+width*enNor.x,enPos.y+width*enNor.y,
											stGue*stTgt.x,stGue*stTgt.y,enGue*enTgt.x,enGue*enTgt.y);
	} else {
		outline_callback_data  desc=*data;
		
		desc.x1=data->x1;
		desc.y1=data->y1;
		desc.x2=miPos.x;
		desc.y2=miPos.y;
		desc.d.c.dx1=0.5*stTle*stTgt.x;
		desc.d.c.dy1=0.5*stTle*stTgt.y;
		desc.d.c.dx2=0.5*miTle*miTgt.x;
		desc.d.c.dy2=0.5*miTle*miTgt.y;
		RecStdCubicTo(&desc,tol,width,lev-1);
		
		desc.x1=miPos.x;
		desc.y1=miPos.y;
		desc.x2=data->x2;
		desc.y2=data->y2;
		desc.d.c.dx1=0.5*miTle*miTgt.x;
		desc.d.c.dy1=0.5*miTle*miTgt.y;
		desc.d.c.dx2=0.5*enTle*enTgt.x;
		desc.d.c.dy2=0.5*enTle*enTgt.y;
		RecStdCubicTo(&desc,tol,width,lev-1);
	}
}

void Path::StdCubicTo(Path::outline_callback_data *data,float tol,float width)
{
	fflush(stdout);
	RecStdCubicTo(data,tol,width,8);
}

void Path::StdBezierTo(Path::outline_callback_data *data,float tol,float width)
{
	path_descr_bezierto       tempb;
	path_descr_intermbezierto tempi;
	tempb.nb=1;
	tempb.x=data->x2;
	tempb.y=data->y2;
	tempi.x=data->d.b.mx;
	tempi.y=data->d.b.my;
	vec2  stPos,enPos,stTgt,enTgt;
	float stRad,enRad,stTle,enTle;
	TangentOnBezAt(0.0,data->x1,data->y1,tempi,tempb,false,stPos,stTgt,stTle,stRad);
	TangentOnBezAt(1.0,data->x1,data->y1,tempi,tempb,true,enPos,enTgt,enTle,enRad);
	data->d.c.dx1=stTle*stTgt.x;
	data->d.c.dy1=stTle*stTgt.y;
	data->d.c.dx2=enTle*enTgt.x;
	data->d.c.dy2=enTle*enTgt.y;
	RecStdCubicTo(data,tol,width,8);
}

void Path::RecStdArcTo(outline_callback_data *data,float tol,float width,int lev)
{
	vec2 stPos,miPos,enPos;
	vec2 stTgt,enTgt,miTgt,stNor,enNor,miNor;
	float stRad,miRad,enRad;
	float stTle,miTle,enTle;
	// un cubic
  {
		path_descr_arcto temp;
		temp.x=data->x2;
		temp.y=data->y2;
		temp.rx=data->d.a.rx;
		temp.ry=data->d.a.ry;
		temp.angle=data->d.a.angle;
		temp.clockwise=data->d.a.clock;
		temp.large=data->d.a.large;
		TangentOnArcAt(data->d.a.stA,data->x1,data->y1,temp,stPos,stTgt,stTle,stRad);
		TangentOnArcAt((data->d.a.stA+data->d.a.enA)/2,data->x1,data->y1,temp,miPos,miTgt,miTle,miRad);
		TangentOnArcAt(data->d.a.enA,data->x1,data->y1,temp,enPos,enTgt,enTle,enRad);
		RotCWTo(stTgt,stNor);
		RotCWTo(miTgt,miNor);
		RotCWTo(enTgt,enNor);
	}
	
	float  stGue=1,miGue=1,enGue=1;
	if ( fabsf(stRad) > 0.01 ) stGue+=width/stRad;
	if ( fabsf(miRad) > 0.01 ) miGue+=width/miRad;
	if ( fabsf(enRad) > 0.01 ) enGue+=width/enRad;
	stGue*=stTle;
	miGue*=miTle;
	enGue*=enTle;
	float  sang,eang;
	ArcAngles(data->x1,data->y1,data->x2,data->y2,data->d.a.rx,data->d.a.ry,data->d.a.angle,
													 data->d.a.large,!data->d.a.clock,sang,eang);
	float  scal=eang-sang;
	if ( scal < 0 ) scal+=2*M_PI;
	if ( scal > 2*M_PI ) scal-=2*M_PI;
	scal*=data->d.a.enA-data->d.a.stA;
	
	if ( lev <= 0 ) {
		data->dest->CubicTo(enPos.x+width*enNor.x,enPos.y+width*enNor.y,
											stGue*stTgt.x*scal,stGue*stTgt.y*scal,enGue*enTgt.x*scal,enGue*enTgt.y*scal);
		return;
	}
	
	vec2  req,chk;
	req.x=miPos.x+width*miNor.x;
	req.y=miPos.y+width*miNor.y;
	{
		path_descr_cubicto temp;
		float  chTle,chRad;
		vec2   chTgt;
		temp.x=enPos.x+width*enNor.x;
		temp.y=enPos.y+width*enNor.y;
		temp.stDx=stGue*stTgt.x*scal;
		temp.stDy=stGue*stTgt.y*scal;
		temp.enDx=enGue*enTgt.x*scal;
		temp.enDy=enGue*enTgt.y*scal;
		TangentOnCubAt(0.5,stPos.x+width*stNor.x,stPos.y+width*stNor.y,temp,false,chk,chTgt,chTle,chRad);		
	}
	vec2  diff;
	diff.x=req.x-chk.x;
	diff.y=req.y-chk.y;
	float   err=(diff.x*diff.x+diff.y*diff.y);
	if ( err <= tol*tol ) {
		data->dest->CubicTo(enPos.x+width*enNor.x,enPos.y+width*enNor.y,
											stGue*stTgt.x*scal,stGue*stTgt.y*scal,enGue*enTgt.x*scal,enGue*enTgt.y*scal);
	} else {
		outline_callback_data  desc=*data;
		
		desc.d.a.stA=data->d.a.stA;
		desc.d.a.enA=(data->d.a.stA+data->d.a.enA)/2;
		RecStdArcTo(&desc,tol,width,lev-1);
		
		desc.d.a.stA=(data->d.a.stA+data->d.a.enA)/2;
		desc.d.a.enA=data->d.a.enA;
		RecStdArcTo(&desc,tol,width,lev-1);
	}
}
void Path::StdArcTo(Path::outline_callback_data *data,float tol,float width)
{
	data->d.a.stA=0.0;
	data->d.a.enA=1.0;
	RecStdArcTo(data,tol,width,8);
}


