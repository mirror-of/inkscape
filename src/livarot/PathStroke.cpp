/*
 *  PathStroke.cpp
 *  nlivarot
 *
 *  Created by fred on Tue Jun 17 2003.
 *
 */

#include "Path.h"
#include "Shape.h"

void            Path::Stroke(Shape* dest,bool doClose,float width,JoinType join,ButtType butt,float miter,bool justAdd)
{
	if ( dest == NULL ) return;
	if ( justAdd == false ) {
		dest->Reset(3*nbPt,3*nbPt);
	}
	if ( nbPt <= 1 ) return;
	dest->MakeBackData(false);

	char*  savPts=pts;
	int    savNbPt=nbPt;
	
	int    lastM=0;
	while ( lastM < savNbPt ) {
		int  lastP=lastM+1;
		if ( back ) {
			if ( weighted ) {
				path_lineto_wb* tp=(path_lineto_wb*)savPts;
				while ( lastP < savNbPt && ( (tp+lastP)->isMoveTo == polyline_lineto  || (tp+lastP)->isMoveTo == polyline_forced )) lastP++;
				pts=(char*)(tp+lastM);
				nbPt=lastP-lastM;
			} else {
				path_lineto_b* tp=(path_lineto_b*)savPts;
				while ( lastP < savNbPt && ( (tp+lastP)->isMoveTo == polyline_lineto  || (tp+lastP)->isMoveTo == polyline_forced ) ) lastP++;
				pts=(char*)(tp+lastM);
				nbPt=lastP-lastM;
			}
		} else {
			if ( weighted ) {
				path_lineto_w* tp=(path_lineto_w*)savPts;
				while ( lastP < savNbPt && ( (tp+lastP)->isMoveTo == polyline_lineto  || (tp+lastP)->isMoveTo == polyline_forced ) ) lastP++;
				pts=(char*)(tp+lastM);
				nbPt=lastP-lastM;
			} else {
				path_lineto* tp=(path_lineto*)savPts;
				while ( lastP < savNbPt && ( (tp+lastP)->isMoveTo == polyline_lineto  || (tp+lastP)->isMoveTo == polyline_forced ) ) lastP++;
				pts=(char*)(tp+lastM);
				nbPt=lastP-lastM;
			}
		}
		DoStroke(dest,doClose,width,join,butt,miter,true);
		lastM=lastP;
	}
	
	pts=savPts;
	nbPt=savNbPt;
}
void            Path::DoStroke(Shape* dest,bool doClose,float width,JoinType join,ButtType butt,float miter,bool justAdd)
{
	if ( nbPt <= 1 ) return;

	vec2      curP,prevP,nextP;
	float     curW,prevW,nextW;
	int       curI,prevI,nextI;
	int       upTo;

	curI=0;
	curP.x=((path_lineto*)pts)[0].x;
	curP.y=((path_lineto*)pts)[0].y;
	if ( weighted ) curW=((path_lineto_w*)pts)[0].w; else curW=1;

	if ( doClose ) {
		path_lineto*  curPt=(path_lineto*)pts;
		prevI=nbPt-1;
		if ( back ) {
			if ( weighted ) curPt=(path_lineto*)(((char*)curPt)+prevI*sizeof(path_lineto_wb)); else curPt=(path_lineto*)(((char*)curPt)+prevI*sizeof(path_lineto_b));
		} else {
			if ( weighted ) curPt=(path_lineto*)(((char*)curPt)+prevI*sizeof(path_lineto_w)); else curPt=(path_lineto*)(((char*)curPt)+prevI*sizeof(path_lineto));
		}
		while ( prevI > 0 ) {
			prevP.x=curPt->x;
			prevP.y=curPt->y;
			if ( weighted ) prevW=((path_lineto_w*)curPt)->w; else prevW=1;
			float   dist=(curP.x-prevP.x)*(curP.x-prevP.x)+(curP.y-prevP.y)*(curP.y-prevP.y);
			if ( dist > 0.001 ) {
				break;
			}
			prevI--;
			if ( back ) {
				if ( weighted ) curPt=(path_lineto*)(((char*)curPt)-sizeof(path_lineto_wb)); else curPt=(path_lineto*)(((char*)curPt)-sizeof(path_lineto_b));
			} else {
				if ( weighted ) curPt=(path_lineto*)(((char*)curPt)-sizeof(path_lineto_w)); else curPt=(path_lineto*)(((char*)curPt)-sizeof(path_lineto));
			}
		}
		if ( prevI <= 0 ) return;
		upTo=prevI;
	} else {
		prevP=curP;
		prevW=curW;
		prevI=curI;
		upTo=nbPt-1;
	}
	{
		path_lineto*  curPt=(path_lineto*)pts;
		nextI=1;
		if ( back ) {
			if ( weighted ) curPt=(path_lineto*)(((char*)curPt)+nextI*sizeof(path_lineto_wb)); else curPt=(path_lineto*)(((char*)curPt)+nextI*sizeof(path_lineto_b));
		} else {
			if ( weighted ) curPt=(path_lineto*)(((char*)curPt)+nextI*sizeof(path_lineto_w)); else curPt=(path_lineto*)(((char*)curPt)+nextI*sizeof(path_lineto));
		}
		while ( nextI <= upTo ) {
			nextP.x=curPt->x;
			nextP.y=curPt->y;
			if ( weighted ) nextW=((path_lineto_w*)curPt)->w; else nextW=1;
			float   dist=(curP.x-nextP.x)*(curP.x-nextP.x)+(curP.y-nextP.y)*(curP.y-nextP.y);
			if ( dist > 0.001 ) {
				break;
			}
			nextI++;
			if ( back ) {
				if ( weighted ) curPt=(path_lineto*)(((char*)curPt)+sizeof(path_lineto_wb)); else curPt=(path_lineto*)(((char*)curPt)+sizeof(path_lineto_b));
			} else {
				if ( weighted ) curPt=(path_lineto*)(((char*)curPt)+sizeof(path_lineto_w)); else curPt=(path_lineto*)(((char*)curPt)+sizeof(path_lineto));
			}
		}
		if ( nextI > upTo ) return;
	}

	int      startLeft=-1,startRight=-1;
	int      lastLeft=-1,lastRight=-1;
	vec2     prevD,nextD;
	float    prevLe,nextLe;
	prevD.x=curP.x-prevP.x;
	prevD.y=curP.y-prevP.y;
	nextD.x=nextP.x-curP.x;
	nextD.y=nextP.y-curP.y;
	prevLe=sqrt(prevD.x*prevD.x+prevD.y*prevD.y);
	nextLe=sqrt(nextD.x*nextD.x+nextD.y*nextD.y);
	Normalize(prevD);
	Normalize(nextD);
	if ( doClose ) {
		DoJoin(dest,curW*width,join,curP,prevD,nextD,miter,prevLe,nextLe,startLeft,lastLeft,startRight,lastRight);
	} else {
		nextD.x=-nextD.x;
		nextD.y=-nextD.y;
		DoButt(dest,curW*width,butt,curP,nextD,lastRight,lastLeft);
		nextD.x=-nextD.x;
		nextD.y=-nextD.y;
	}
	do {
		prevP=curP;
		prevI=curI;
		prevW=curW;
		curP=nextP;
		curI=nextI;
		curW=nextW;
		prevD=nextD;
		prevLe=nextLe;
		nextI++;
		path_lineto*  curPt=(path_lineto*)pts;
		if ( back ) {
			if ( weighted ) curPt=(path_lineto*)(((char*)curPt)+nextI*sizeof(path_lineto_wb)); else curPt=(path_lineto*)(((char*)curPt)+nextI*sizeof(path_lineto_b));
		} else {
			if ( weighted ) curPt=(path_lineto*)(((char*)curPt)+nextI*sizeof(path_lineto_w)); else curPt=(path_lineto*)(((char*)curPt)+nextI*sizeof(path_lineto));
		}
		while ( nextI <= upTo ) {
			nextP.x=curPt->x;
			nextP.y=curPt->y;
			if ( weighted ) nextW=((path_lineto_w*)curPt)->w; else nextW=1;
			float   dist=(curP.x-nextP.x)*(curP.x-nextP.x)+(curP.y-nextP.y)*(curP.y-nextP.y);
			if ( dist > 0.001 ) {
				break;
			}
			nextI++;
			if ( back ) {
				if ( weighted ) curPt=(path_lineto*)(((char*)curPt)+sizeof(path_lineto_wb)); else curPt=(path_lineto*)(((char*)curPt)+sizeof(path_lineto_b));
			} else {
				if ( weighted ) curPt=(path_lineto*)(((char*)curPt)+sizeof(path_lineto_w)); else curPt=(path_lineto*)(((char*)curPt)+sizeof(path_lineto));
			}
		}
		if ( nextI > upTo ) break;

		nextD.x=nextP.x-curP.x;
		nextD.y=nextP.y-curP.y;
		nextLe=sqrt(nextD.x*nextD.x+nextD.y*nextD.y);
		Normalize(nextD);
		int   nStL=-1,nStR=-1,nEnL=-1,nEnR=-1;
		DoJoin(dest,curW*width,join,curP,prevD,nextD,miter,prevLe,nextLe,nStL,nEnL,nStR,nEnR);
		dest->AddEdge(nStL,lastLeft);
		lastLeft=nEnL;
		dest->AddEdge(lastRight,nStR);
		lastRight=nEnR;
	} while ( nextI <= upTo );
	if ( doClose ) {
/*		prevP=curP;
		prevI=curI;
		prevW=curW;
		curP=nextP;
		curI=nextI;
		curW=nextW;
		prevD=nextD;*/
		path_lineto*  curPt=(path_lineto*)pts;
		nextP.x=curPt->x;
		nextP.y=curPt->y;
		if ( weighted ) nextW=((path_lineto_w*)curPt)->w; else nextW=1;
		
		nextD.x=nextP.x-curP.x;
		nextD.y=nextP.y-curP.y;
		nextLe=sqrt(nextD.x*nextD.x+nextD.y*nextD.y);
		Normalize(nextD);
		int   nStL=-1,nStR=-1,nEnL=-1,nEnR=-1;
		DoJoin(dest,curW*width,join,curP,prevD,nextD,miter,prevLe,nextLe,nStL,nEnL,nStR,nEnR);
		dest->AddEdge(nStL,lastLeft);
		lastLeft=nEnL;
		dest->AddEdge(lastRight,nStR);
		lastRight=nEnR;

		dest->AddEdge(startLeft,lastLeft);
		dest->AddEdge(lastRight,startRight);
	} else {
		int   endRight,endLeft;
		DoButt(dest,curW*width,butt,curP,prevD,endLeft,endRight);
		dest->AddEdge(endLeft,lastLeft);
		dest->AddEdge(lastRight,endRight);
	}
}
void            Path::DoButt(Shape* dest,float width,ButtType butt,vec2 pos,vec2 dir,int &leftNo,int &rightNo)
{
	vec2 nor;
	RotCCWTo(dir,nor);

	if ( butt == butt_square ) {
		float   x,y;
		x=pos.x+width*dir.x+width*nor.x;
		y=pos.y+width*dir.y+width*nor.y;
		int bleftNo=dest->AddPoint(x,y);
		x=pos.x+width*dir.x-width*nor.x;
		y=pos.y+width*dir.y-width*nor.y;
		int brightNo=dest->AddPoint(x,y);
		x=pos.x+width*nor.x;
		y=pos.y+width*nor.y;
		leftNo=dest->AddPoint(x,y);
		x=pos.x-width*nor.x;
		y=pos.y-width*nor.y;
		rightNo=dest->AddPoint(x,y);
		dest->AddEdge(rightNo,brightNo);
		dest->AddEdge(brightNo,bleftNo);
		dest->AddEdge(bleftNo,leftNo);
	} else if ( butt == butt_pointy ) {
		float   x,y;
		x=pos.x+width*nor.x;
		y=pos.y+width*nor.y;
		leftNo=dest->AddPoint(x,y);
		x=pos.x-width*nor.x;
		y=pos.y-width*nor.y;
		rightNo=dest->AddPoint(x,y);
		x=pos.x+width*dir.x;
		y=pos.y+width*dir.y;
		int mid=dest->AddPoint(x,y);
		dest->AddEdge(rightNo,mid);
		dest->AddEdge(mid,leftNo);
	} else if ( butt == butt_round ) {
		float   sx,sy,ex,ey,mx,my;
		sx=pos.x+width*nor.x;
		sy=pos.y+width*nor.y;
		ex=pos.x-width*nor.x;
		ey=pos.y-width*nor.y;
		mx=pos.x+width*dir.x;
		my=pos.y+width*dir.y;
		leftNo=dest->AddPoint(sx,sy);
		rightNo=dest->AddPoint(ex,ey);
		int   midNo=dest->AddPoint(mx,my);
		
		float   dx,dy;
		dx=pos.x-width*nor.x+width*dir.x;
		dy=pos.y-width*nor.y+width*dir.y;
		RecRound(dest,rightNo,midNo,dx,dy,ex,ey,mx,my,5.0,8);
		dx=pos.x+width*nor.x+width*dir.x;
		dy=pos.y+width*nor.y+width*dir.y;
		RecRound(dest,midNo,leftNo,dx,dy,mx,my,sx,sy,5.0,8);
	} else {
		float   x,y;
		x=pos.x+width*nor.x;
		y=pos.y+width*nor.y;
		leftNo=dest->AddPoint(x,y);
		x=pos.x-width*nor.x;
		y=pos.y-width*nor.y;
		rightNo=dest->AddPoint(x,y);
		dest->AddEdge(rightNo,leftNo);
	}
}
void            Path::DoJoin(Shape* dest,float width,JoinType join,vec2 pos,vec2 prev,vec2 next,float miter,float prevL,float nextL,int &leftStNo,int &leftEnNo,int &rightStNo,int &rightEnNo)
{
//	DoLeftJoin(dest,width,join,pos,prev,next,miter,prevL,nextL,leftStNo,leftEnNo);
//	DoRightJoin(dest,width,join,pos,prev,next,miter,prevL,nextL,rightStNo,rightEnNo);
//	return;
		
	vec2   pnor,nnor;
	RotCCWTo(prev,pnor);
	RotCCWTo(next,nnor);
	float   angSi=Dot(prev,next);
	if ( angSi > -0.0001 && angSi < 0.0001 ) {
		float   angCo=Cross(prev,next);
		if ( angCo > 0.9999 ) {
			// tout droit
			float   x,y;
			x=pos.x+width*pnor.x;
			y=pos.y+width*pnor.y;
			leftStNo=leftEnNo=dest->AddPoint(x,y);
			x=pos.x-width*pnor.x;
			y=pos.y-width*pnor.y;
			rightStNo=rightEnNo=dest->AddPoint(x,y);
		} else {
			// demi-tour
			float   x,y;
			x=pos.x+width*pnor.x;
			y=pos.y+width*pnor.y;
			leftStNo=rightEnNo=dest->AddPoint(x,y);
			x=pos.x-width*pnor.x;
			y=pos.y-width*pnor.y;
			rightStNo=leftEnNo=dest->AddPoint(x,y);
			dest->AddEdge(leftEnNo,leftStNo);
			dest->AddEdge(rightStNo,rightEnNo);
		}
		return;
	}
	if ( angSi < 0 ) {
		float   x,y;
		{
			vec2     biss;
			biss.x=next.x-prev.x;
			biss.y=next.y-prev.y;
			float   c2=Dot(biss,next);
			float   l=width/c2;
			float		projn=l*(Cross(biss,next));
			float		projp=-l*(Cross(biss,prev));
			if ( projp <= 0.5*prevL && projn <= 0.5*nextL ) {
				x=pos.x+l*biss.x;
				y=pos.y+l*biss.y;
				leftEnNo=leftStNo=dest->AddPoint(x,y);
			} else {
				x=pos.x+width*pnor.x;
				y=pos.y+width*pnor.y;
				leftStNo=dest->AddPoint(x,y);
				x=pos.x+width*nnor.x;
				y=pos.y+width*nnor.y;
				leftEnNo=dest->AddPoint(x,y);
				dest->AddEdge(leftEnNo,leftStNo);
			}
		}
		if ( join == join_pointy ) {
			x=pos.x-width*pnor.x;
			y=pos.y-width*pnor.y;
			rightStNo=dest->AddPoint(x,y);
			x=pos.x-width*nnor.x;
			y=pos.y-width*nnor.y;
			rightEnNo=dest->AddPoint(x,y);
			
//			dest->AddEdge(rightStNo,rightEnNo);
			vec2     biss;
			biss.x=pnor.x+nnor.x;
			biss.y=pnor.y+nnor.y;
			Normalize(biss);
			float   c2=Cross(biss,nnor);
			float   l=width/c2;
			float   emiter=width*c2;
			if ( emiter < miter ) emiter=miter;
			int nrightStNo,nrightEnNo;
			if ( l <= emiter ) {
				x=pos.x-l*biss.x;
				y=pos.y-l*biss.y;
				nrightStNo=nrightEnNo=dest->AddPoint(x,y);
			} else {
				float   s2=Dot(biss,nnor);
				float   dec=(l-emiter)*c2/s2;
				vec2    tbiss;
				RotCCWTo(biss,tbiss);

				x=pos.x-emiter*biss.x-dec*tbiss.x;
				y=pos.y-emiter*biss.y-dec*tbiss.y;
				nrightStNo=dest->AddPoint(x,y);
				x=pos.x-emiter*biss.x+dec*tbiss.x;
				y=pos.y-emiter*biss.y+dec*tbiss.y;
				nrightEnNo=dest->AddPoint(x,y);
			}
			dest->AddEdge(rightStNo,nrightStNo);
			dest->AddEdge(nrightStNo,nrightEnNo);
			dest->AddEdge(nrightEnNo,rightEnNo);
		} else if ( join == join_round ) {
			float sx=pos.x-width*pnor.x;
			float sy=pos.y-width*pnor.y;
			rightStNo=dest->AddPoint(sx,sy);
			float ex=pos.x-width*nnor.x;
			float ey=pos.y-width*nnor.y;
			rightEnNo=dest->AddPoint(ex,ey);

			vec2     biss;
			biss.x=pnor.x+nnor.x;
			biss.y=pnor.y+nnor.y;
			Normalize(biss);
			float   c2=Cross(biss,nnor);
			float   l=width/c2;
			float    typ=Cross(pnor,nnor);
			if ( typ >= 0 ) {
				x=pos.x-l*biss.x;
				y=pos.y-l*biss.y;
				RecRound(dest,rightStNo,rightEnNo,x,y,sx,sy,ex,ey,5.0,8);
			} else {
				float   s2=Dot(biss,nnor);
				float   dec=(l-width)*c2/s2;
				vec2    tbiss;
				RotCCWTo(biss,tbiss);
				float nsx=pos.x-width*biss.x-dec*tbiss.x;
				float nsy=pos.y-width*biss.y-dec*tbiss.y;
				float nex=pos.x-width*biss.x+dec*tbiss.x;
				float ney=pos.y-width*biss.y+dec*tbiss.y;
				float mx=pos.x-width*biss.x;
				float my=pos.y-width*biss.y;
				int midNo=dest->AddPoint(mx,my);
				RecRound(dest,rightStNo,midNo,nsx,nsy,sx,sy,mx,my,5.0,8);
				RecRound(dest,midNo,rightEnNo,nex,ney,mx,my,ex,ey,5.0,8);
			}
		} else {
			x=pos.x-width*pnor.x;
			y=pos.y-width*pnor.y;
			rightStNo=dest->AddPoint(x,y);
			x=pos.x-width*nnor.x;
			y=pos.y-width*nnor.y;
			rightEnNo=dest->AddPoint(x,y);
			dest->AddEdge(rightStNo,rightEnNo);
		}
	} else {
		float   x,y;
		{
			vec2     biss;
			biss.x=next.x-prev.x;
			biss.y=next.y-prev.y;
			float   c2=Dot(next,biss);
			float   l=width/c2;
			float		projn=l*(Cross(biss,next));
			float		projp=-l*(Cross(biss,prev));
			if ( projp <= 0.5*prevL && projn <= 0.5*nextL ) {
				x=pos.x+l*biss.x;
				y=pos.y+l*biss.y;
				rightEnNo=rightStNo=dest->AddPoint(x,y);
			} else {
				x=pos.x-width*pnor.x;
				y=pos.y-width*pnor.y;
				rightStNo=dest->AddPoint(x,y);
				x=pos.x-width*nnor.x;
				y=pos.y-width*nnor.y;
				rightEnNo=dest->AddPoint(x,y);
				dest->AddEdge(rightStNo,rightEnNo);
			}
		}
		if ( join == join_pointy ) {
			x=pos.x+width*pnor.x;
			y=pos.y+width*pnor.y;
			leftStNo=dest->AddPoint(x,y);
			x=pos.x+width*nnor.x;
			y=pos.y+width*nnor.y;
			leftEnNo=dest->AddPoint(x,y);
	//		dest->AddEdge(leftEnNo,leftStNo);

			vec2     biss;
			biss.x=pnor.x+nnor.x;
			biss.y=pnor.y+nnor.y;
			Normalize(biss);
			float   c2=Cross(biss,nnor);
			float   l=width/c2;
			float   emiter=width*c2;
			if ( emiter < miter ) emiter=miter;
			int nleftStNo,nleftEnNo;
			if ( l <= emiter ) {
				x=pos.x+l*biss.x;
				y=pos.y+l*biss.y;
				nleftStNo=nleftEnNo=dest->AddPoint(x,y);
			} else {
				float   s2=Dot(biss,nnor);
				float   dec=(l-emiter)*c2/s2;
				vec2    tbiss;
				RotCCWTo(biss,tbiss);

				x=pos.x+emiter*biss.x+dec*tbiss.x;
				y=pos.y+emiter*biss.y+dec*tbiss.y;
				nleftStNo=dest->AddPoint(x,y);
				x=pos.x+emiter*biss.x-dec*tbiss.x;
				y=pos.y+emiter*biss.y-dec*tbiss.y;
				nleftEnNo=dest->AddPoint(x,y);
			}
			dest->AddEdge(leftEnNo,nleftEnNo);
			dest->AddEdge(nleftEnNo,nleftStNo);
			dest->AddEdge(nleftStNo,leftStNo);
		} else if ( join == join_round ) {
			float sx=pos.x+width*pnor.x;
			float sy=pos.y+width*pnor.y;
			leftStNo=dest->AddPoint(sx,sy);
			float ex=pos.x+width*nnor.x;
			float ey=pos.y+width*nnor.y;
			leftEnNo=dest->AddPoint(ex,ey);

			vec2     biss;
			biss.x=pnor.x+nnor.x;
			biss.y=pnor.y+nnor.y;
			Normalize(biss);
			float   c2=Cross(biss,nnor);
			float   l=width/c2;
			float    typ=Cross(pnor,nnor);
			if ( typ >= 0 ) {
				x=pos.x+l*biss.x;
				y=pos.y+l*biss.y;
				RecRound(dest,leftEnNo,leftStNo,x,y,ex,ey,sx,sy,5.0,8);
			} else {
				float   s2=Dot(biss,nnor);
				float   dec=(l-width)*c2/s2;
				vec2    tbiss;
				RotCCWTo(biss,tbiss);

				float mx=pos.x+width*biss.x;
				float my=pos.y+width*biss.y;
				int midNo=dest->AddPoint(mx,my);

				float nsx=pos.x+width*biss.x+dec*tbiss.x;
				float nsy=pos.y+width*biss.y+dec*tbiss.y;
				float nex=pos.x+width*biss.x-dec*tbiss.x;
				float ney=pos.y+width*biss.y-dec*tbiss.y;
				RecRound(dest,leftEnNo,midNo,nex,ney,ex,ey,mx,my,5.0,8);
				RecRound(dest,midNo,leftStNo,nsx,nsy,mx,my,sx,sy,5.0,8);
			}
		} else {
			x=pos.x+width*pnor.x;
			y=pos.y+width*pnor.y;
			leftStNo=dest->AddPoint(x,y);
			x=pos.x+width*nnor.x;
			y=pos.y+width*nnor.y;
			leftEnNo=dest->AddPoint(x,y);
			dest->AddEdge(leftEnNo,leftStNo);
		}
	}
}

void            Path::DoLeftJoin(Shape* dest,float width,JoinType join,vec2 pos,vec2 prev,vec2 next,float miter,float prevL,float nextL,int &leftStNo,int &leftEnNo)
{
	vec2   pnor,nnor;
	RotCCWTo(prev,pnor);
	RotCCWTo(next,nnor);
	float   angSi=Dot(prev,next);
	if ( angSi > -0.0001 && angSi < 0.0001 ) {
		float   angCo=Cross(prev,next);
		if ( angCo > 0.9999 ) {
			// tout droit
			float   x,y;
			x=pos.x+width*pnor.x;
			y=pos.y+width*pnor.y;
			leftEnNo=leftStNo=dest->AddPoint(x,y);
		} else {
			// demi-tour
			float   x,y;
			x=pos.x+width*pnor.x;
			y=pos.y+width*pnor.y;
			leftStNo=dest->AddPoint(x,y);
			x=pos.x-width*pnor.x;
			y=pos.y-width*pnor.y;
			leftEnNo=dest->AddPoint(x,y);
			dest->AddEdge(leftEnNo,leftStNo);
		}
		return;
	}
	if ( angSi < 0 ) {
/*		vec2     biss;
		biss.x=next.x-prev.x;
		biss.y=next.y-prev.y;
		float   c2=Dot(biss,next);
		float   l=width/c2;
		float		projn=l*(Cross(biss,next));
		float		projp=-l*(Cross(biss,prev));
		if ( projp <= 0.5*prevL && projn <= 0.5*nextL ) {
			float   x,y;
			x=pos.x+l*biss.x;
			y=pos.y+l*biss.y;
			leftEnNo=leftStNo=dest->AddPoint(x,y);
		} else {*/
			float   x,y;
			x=pos.x+width*pnor.x;
			y=pos.y+width*pnor.y;
			leftStNo=dest->AddPoint(x,y);
			x=pos.x+width*nnor.x;
			y=pos.y+width*nnor.y;
			leftEnNo=dest->AddPoint(x,y);
			x=pos.x;
			y=pos.y;
			int midNo=dest->AddPoint(x,y);
			dest->AddEdge(leftEnNo,midNo);
			dest->AddEdge(midNo,leftStNo);
//		}
	} else {
		float   x,y;
		if ( join == join_pointy ) {
			float sx=pos.x+width*pnor.x;
			float sy=pos.y+width*pnor.y;
			leftStNo=dest->AddPoint(sx,sy);
			float ex=pos.x+width*nnor.x;
			float ey=pos.y+width*nnor.y;
			leftEnNo=dest->AddPoint(ex,ey);

			vec2     biss;
			biss.x=pnor.x+nnor.x;
			biss.y=pnor.y+nnor.y;
			Normalize(biss);
			float   c2=Cross(biss,nnor);
			float   l=width/c2;
			float   emiter=width*c2;
			if ( emiter < miter ) emiter=miter;
			if ( l <= emiter ) {
				x=pos.x+l*biss.x;
				y=pos.y+l*biss.y;
				int nleftStNo=dest->AddPoint(x,y);
				dest->AddEdge(leftEnNo,nleftStNo);
				dest->AddEdge(nleftStNo,leftStNo);
			} else {
				float   s2=Dot(biss,nnor);
				float   dec=(l-emiter)*c2/s2;
				vec2    tbiss;
				RotCCWTo(biss,tbiss);

				x=pos.x+emiter*biss.x+dec*tbiss.x;
				y=pos.y+emiter*biss.y+dec*tbiss.y;
				int nleftStNo=dest->AddPoint(x,y);
				x=pos.x+emiter*biss.x-dec*tbiss.x;
				y=pos.y+emiter*biss.y-dec*tbiss.y;
				int nleftEnNo=dest->AddPoint(x,y);
				dest->AddEdge(nleftEnNo,nleftStNo);
				dest->AddEdge(leftEnNo,nleftEnNo);
				dest->AddEdge(nleftStNo,leftStNo);
			}
		} else if ( join == join_round ) {
			float sx=pos.x+width*pnor.x;
			float sy=pos.y+width*pnor.y;
			leftStNo=dest->AddPoint(sx,sy);
			float ex=pos.x+width*nnor.x;
			float ey=pos.y+width*nnor.y;
			leftEnNo=dest->AddPoint(ex,ey);

			vec2     biss;
			biss.x=pnor.x+nnor.x;
			biss.y=pnor.y+nnor.y;
			Normalize(biss);
			float   c2=Cross(biss,nnor);
			float   l=width/c2;
			float    typ=Cross(pnor,nnor);
			if ( typ >= 0 ) {
				x=pos.x+l*biss.x;
				y=pos.y+l*biss.y;
				RecRound(dest,leftEnNo,leftStNo,x,y,ex,ey,sx,sy,5.0,8);
			} else {
				float   s2=Dot(biss,nnor);
				float   dec=(l-width)*c2/s2;
				vec2    tbiss;
				RotCCWTo(biss,tbiss);

				float mx=pos.x+width*biss.x;
				float my=pos.y+width*biss.y;
				int midNo=dest->AddPoint(mx,my);

				float nsx=pos.x+width*biss.x+dec*tbiss.x;
				float nsy=pos.y+width*biss.y+dec*tbiss.y;
				float nex=pos.x+width*biss.x-dec*tbiss.x;
				float ney=pos.y+width*biss.y-dec*tbiss.y;
				RecRound(dest,leftEnNo,midNo,nex,ney,ex,ey,mx,my,5.0,8);
				RecRound(dest,midNo,leftStNo,nsx,nsy,mx,my,sx,sy,5.0,8);
			}
		} else {
			x=pos.x+width*pnor.x;
			y=pos.y+width*pnor.y;
			leftStNo=dest->AddPoint(x,y);
			x=pos.x+width*nnor.x;
			y=pos.y+width*nnor.y;
			leftEnNo=dest->AddPoint(x,y);
			dest->AddEdge(leftEnNo,leftStNo);
		}
	}
}
void            Path::DoRightJoin(Shape* dest,float width,JoinType join,vec2 pos,vec2 prev,vec2 next,float miter,float prevL,float nextL,int &rightStNo,int &rightEnNo)
{
	vec2   pnor,nnor;
	RotCCWTo(prev,pnor);
	RotCCWTo(next,nnor);
	float   angSi=Dot(prev,next);
	if ( angSi > -0.0001 && angSi < 0.0001 ) {
		float   angCo=Cross(prev,next);
		if ( angCo > 0.9999 ) {
			// tout droit
			float   x,y;
			x=pos.x-width*pnor.x;
			y=pos.y-width*pnor.y;
			rightEnNo=rightStNo=dest->AddPoint(x,y);
		} else {
			// demi-tour
			float   x,y;
			x=pos.x+width*pnor.x;
			y=pos.y+width*pnor.y;
			rightEnNo=dest->AddPoint(x,y);
			x=pos.x-width*pnor.x;
			y=pos.y-width*pnor.y;
			rightStNo=dest->AddPoint(x,y);
			dest->AddEdge(rightStNo,rightEnNo);
		}
		return;
	}
	if ( angSi < 0 ) {
		float   x,y;

		if ( join == join_pointy ) {
			float sx=pos.x-width*pnor.x;
			float sy=pos.y-width*pnor.y;
			rightStNo=dest->AddPoint(sx,sy);
			float ex=pos.x-width*nnor.x;
			float ey=pos.y-width*nnor.y;
			rightEnNo=dest->AddPoint(ex,ey);

			vec2     biss;
			biss.x=pnor.x+nnor.x;
			biss.y=pnor.y+nnor.y;
			Normalize(biss);
			float   c2=Cross(biss,nnor);
			float   l=width/c2;
			float   emiter=width*c2;
			if ( emiter < miter ) emiter=miter;
			if ( l <= emiter ) {
				x=pos.x-l*biss.x;
				y=pos.y-l*biss.y;
				int nrightStNo=dest->AddPoint(x,y);
				dest->AddEdge(rightStNo,nrightStNo);
				dest->AddEdge(nrightStNo,rightEnNo);
			} else {
				float   s2=Dot(biss,nnor);
				float   dec=(l-emiter)*c2/s2;
				vec2    tbiss;
				RotCCWTo(biss,tbiss);

				x=pos.x-emiter*biss.x-dec*tbiss.x;
				y=pos.y-emiter*biss.y-dec*tbiss.y;
				int nrightStNo=dest->AddPoint(x,y);
				x=pos.x-emiter*biss.x+dec*tbiss.x;
				y=pos.y-emiter*biss.y+dec*tbiss.y;
				int nrightEnNo=dest->AddPoint(x,y);
				dest->AddEdge(rightStNo,nrightStNo);
				dest->AddEdge(nrightStNo,nrightEnNo);
				dest->AddEdge(nrightEnNo,rightEnNo);
			}
		} else if ( join == join_round ) {
			float sx=pos.x-width*pnor.x;
			float sy=pos.y-width*pnor.y;
			rightStNo=dest->AddPoint(sx,sy);
			float ex=pos.x-width*nnor.x;
			float ey=pos.y-width*nnor.y;
			rightEnNo=dest->AddPoint(ex,ey);

			vec2     biss;
			biss.x=pnor.x+nnor.x;
			biss.y=pnor.y+nnor.y;
			Normalize(biss);
			float   c2=Cross(biss,nnor);
			float   l=width/c2;
			float    typ=Cross(pnor,nnor);
			if ( typ >= 0 ) {
				x=pos.x-l*biss.x;
				y=pos.y-l*biss.y;
				RecRound(dest,rightStNo,rightEnNo,x,y,sx,sy,ex,ey,5.0,8);
			} else {
				float   s2=Dot(biss,nnor);
				float   dec=(l-width)*c2/s2;
				vec2    tbiss;
				RotCCWTo(biss,tbiss);
				float nsx=pos.x-width*biss.x-dec*tbiss.x;
				float nsy=pos.y-width*biss.y-dec*tbiss.y;
				float nex=pos.x-width*biss.x+dec*tbiss.x;
				float ney=pos.y-width*biss.y+dec*tbiss.y;
				float mx=pos.x-width*biss.x;
				float my=pos.y-width*biss.y;
				int midNo=dest->AddPoint(mx,my);
				RecRound(dest,rightStNo,midNo,nsx,nsy,sx,sy,mx,my,5.0,8);
				RecRound(dest,midNo,rightEnNo,nex,ney,mx,my,ex,ey,5.0,8);
			}
		} else {
			x=pos.x-width*pnor.x;
			y=pos.y-width*pnor.y;
			rightStNo=dest->AddPoint(x,y);
			x=pos.x-width*nnor.x;
			y=pos.y-width*nnor.y;
			rightEnNo=dest->AddPoint(x,y);
			dest->AddEdge(rightStNo,rightEnNo);
		}
	} else {
/*		vec2     biss;
		biss.x=next.x-prev.x;
		biss.y=next.y-prev.y;
		float   c2=Dot(next,biss);
		float   l=width/c2;
		float		projn=l*(Cross(biss,next));
		float		projp=-l*(Cross(biss,prev));
		if ( projp <= 0.5*prevL && projn <= 0.5*nextL ) {
			float   x,y;
			x=pos.x+l*biss.x;
			y=pos.y+l*biss.y;
			rightEnNo=rightStNo=dest->AddPoint(x,y);
		} else {*/
			float   x,y;
			x=pos.x-width*pnor.x;
			y=pos.y-width*pnor.y;
			rightStNo=dest->AddPoint(x,y);
			x=pos.x-width*nnor.x;
			y=pos.y-width*nnor.y;
			rightEnNo=dest->AddPoint(x,y);
			x=pos.x;
			y=pos.y;
			int midNo=dest->AddPoint(x,y);
			dest->AddEdge(rightStNo,midNo);
			dest->AddEdge(midNo,rightEnNo);
//		}
	}
}


void            Path::RecRound(Shape* dest,int sNo,int eNo,float px,float py,float sx,float sy,float ex,float ey,float tresh,int lev)
{
	if ( lev <= 0 ) {
		dest->AddEdge(sNo,eNo);
		return;
	}
	float s=(sx-px)*(ey-py)-(sy-py)*(ex-px);
	if ( s < 0 ) s=-s;
	if ( s < tresh ) {
		dest->AddEdge(sNo,eNo);
		return;
	}
	
	float   mx,my,mdx,mdy;
	mx=(sx+ex+2*px)/4;
	my=(sy+ey+2*py)/4;
	int mNo=dest->AddPoint(mx,my);

	mdx=(sx+px)/2;
	mdy=(sy+py)/2;
	RecRound(dest,sNo,mNo,mdx,mdy,sx,sy,mx,my,tresh,lev-1);
	mdx=(ex+px)/2;
	mdy=(ey+py)/2;
	RecRound(dest,mNo,eNo,mdx,mdy,mx,my,ex,ey,tresh,lev-1);
}


/*
 *
 * dashed version
 *
 */

void            Path::Stroke(Shape* dest,bool doClose,float width,JoinType join,ButtType butt,float miter,int nbDash,one_dash* dashs,bool justAdd)
{
	if ( nbDash <= 0 ) {
		Stroke(dest,doClose,width,join,butt,miter,justAdd);
		return;
	}

	if ( dest == NULL ) return;
	if ( justAdd == false ) {
		dest->Reset(3*nbPt,3*nbPt);
	}
	if ( nbPt <= 1 ) return;
	dest->MakeBackData(false);
	
	char*  savPts=pts;
	int    savNbPt=nbPt;
	
	int    lastM=0;
	while ( lastM < savNbPt ) {
		int  lastP=lastM+1;
		if ( back ) {
			if ( weighted ) {
				path_lineto_wb* tp=(path_lineto_wb*)savPts;
				while ( lastP < savNbPt && ( (tp+lastP)->isMoveTo == polyline_lineto  || (tp+lastP)->isMoveTo == polyline_forced ) ) lastP++;
				pts=(char*)(tp+lastM);
				nbPt=lastP-lastM;
			} else {
				path_lineto_b* tp=(path_lineto_b*)savPts;
				while ( lastP < savNbPt && ( (tp+lastP)->isMoveTo == polyline_lineto  || (tp+lastP)->isMoveTo == polyline_forced ) ) lastP++;
				pts=(char*)(tp+lastM);
				nbPt=lastP-lastM;
			}
		} else {
			if ( weighted ) {
				path_lineto_w* tp=(path_lineto_w*)savPts;
				while ( lastP < savNbPt && ( (tp+lastP)->isMoveTo == polyline_lineto  || (tp+lastP)->isMoveTo == polyline_forced ) ) lastP++;
				pts=(char*)(tp+lastM);
				nbPt=lastP-lastM;
			} else {
				path_lineto* tp=(path_lineto*)savPts;
				while ( lastP < savNbPt && ( (tp+lastP)->isMoveTo == polyline_lineto  || (tp+lastP)->isMoveTo == polyline_forced ) ) lastP++;
				pts=(char*)(tp+lastM);
				nbPt=lastP-lastM;
			}
		}
		DoStroke(dest,doClose,width,join,butt,miter,nbDash,dashs,true);
		lastM=lastP;
	}
	
	pts=savPts;
	nbPt=savNbPt;
}
void            Path::DoStroke(Shape* dest,bool doClose,float width,JoinType join,ButtType butt,float miter,int nbDash,one_dash* dashs,bool justAdd)
{

	if ( dest == NULL ) return;
	if ( nbPt <= 1 ) return;

	vec2      curP,prevP,nextP;
	float     curW,prevW,nextW;
	int       curI,prevI,nextI;
	int       upTo;
	float     curA,prevA,nextA;
	float     dashPos=0,dashAbs=0;
	int       dashNo=0;

	curI=0;
	curP.x=((path_lineto*)pts)[0].x;
	curP.y=((path_lineto*)pts)[0].y;
	if ( weighted ) curW=((path_lineto_w*)pts)[0].w; else curW=1;

	if ( doClose ) {
		path_lineto*  curPt=(path_lineto*)pts;
		prevI=nbPt-1;
		if ( back ) {
			if ( weighted ) curPt=(path_lineto*)(((char*)curPt)+prevI*sizeof(path_lineto_wb)); else curPt=(path_lineto*)(((char*)curPt)+prevI*sizeof(path_lineto_b));
		} else {
			if ( weighted ) curPt=(path_lineto*)(((char*)curPt)+prevI*sizeof(path_lineto_w)); else curPt=(path_lineto*)(((char*)curPt)+prevI*sizeof(path_lineto));
		}
		while ( prevI > 0 ) {
			prevP.x=curPt->x;
			prevP.y=curPt->y;
			if ( weighted ) prevW=((path_lineto_w*)curPt)->w; else prevW=1;
			float   dist=(curP.x-prevP.x)*(curP.x-prevP.x)+(curP.y-prevP.y)*(curP.y-prevP.y);
			if ( dist > 0.001 ) {
				break;
			}
			prevI--;
			if ( back ) {
				if ( weighted ) curPt=(path_lineto*)(((char*)curPt)-sizeof(path_lineto_wb)); else curPt=(path_lineto*)(((char*)curPt)-sizeof(path_lineto_b));
			} else {
				if ( weighted ) curPt=(path_lineto*)(((char*)curPt)-sizeof(path_lineto_w)); else curPt=(path_lineto*)(((char*)curPt)-sizeof(path_lineto));
			}
		}
		if ( prevI <= 0 ) return;
		upTo=prevI;
	} else {
		prevP=curP;
		prevW=curW;
		prevI=curI;
		upTo=nbPt-1;
	}
	{
		path_lineto*  curPt=(path_lineto*)pts;
		nextI=1;
		if ( back ) {
			if ( weighted ) curPt=(path_lineto*)(((char*)curPt)+nextI*sizeof(path_lineto_wb)); else curPt=(path_lineto*)(((char*)curPt)+nextI*sizeof(path_lineto_b));
		} else {
			if ( weighted ) curPt=(path_lineto*)(((char*)curPt)+nextI*sizeof(path_lineto_w)); else curPt=(path_lineto*)(((char*)curPt)+nextI*sizeof(path_lineto));
		}
		while ( nextI <= upTo ) {
			nextP.x=curPt->x;
			nextP.y=curPt->y;
			if ( weighted ) nextW=((path_lineto_w*)curPt)->w; else nextW=1;
			float   dist=(curP.x-nextP.x)*(curP.x-nextP.x)+(curP.y-nextP.y)*(curP.y-nextP.y);
			if ( dist > 0.001 ) {
				break;
			}
			nextI++;
			if ( back ) {
				if ( weighted ) curPt=(path_lineto*)(((char*)curPt)+sizeof(path_lineto_wb)); else curPt=(path_lineto*)(((char*)curPt)+sizeof(path_lineto_b));
			} else {
				if ( weighted ) curPt=(path_lineto*)(((char*)curPt)+sizeof(path_lineto_w)); else curPt=(path_lineto*)(((char*)curPt)+sizeof(path_lineto));
			}
		}
		if ( nextI > upTo ) return;
	}

	int      startLeft=-1,startRight=-1;
	int      lastLeft=-1,lastRight=-1;
	bool     noStartJoin=false,inGap=true;
	vec2     prevD,nextD;
	float    prevLe,nextLe;
	prevD.x=curP.x-prevP.x;
	prevD.y=curP.y-prevP.y;
	nextD.x=nextP.x-curP.x;
	nextD.y=nextP.y-curP.y;
	curA=0;
	prevA=-sqrt(prevD.x*prevD.x+prevD.y*prevD.y);
	nextA=sqrt(nextD.x*nextD.x+nextD.y*nextD.y);
	prevLe=sqrt(prevD.x*prevD.x+prevD.y*prevD.y);
	nextLe=sqrt(nextD.x*nextD.x+nextD.y*nextD.y);
	Normalize(prevD);
	Normalize(nextD);
	dashTo_info dTo;
		
	{
		int    cDashNo=dashNo;
		float  nDashAbs=0;
		do {
			if ( dashAbs+(dashs[dashNo].length-dashPos) <= nDashAbs ) {
				dashNo++;
				if ( dashNo >= nbDash ) dashNo-=nbDash;
				dashPos=0;
			} else {
				break;
			}
		}	while ( dashNo != cDashNo );
	}
	if ( doClose ) {		
		if ( dashs[dashNo].gap ) {
			noStartJoin=true;
			inGap=true;
		} else {
			noStartJoin=false;
			DoJoin(dest,curW*width,join,curP,prevD,nextD,miter,prevLe,nextLe,startLeft,lastLeft,startRight,lastRight);
			inGap=false;
		}
	} else {
		if ( dashs[dashNo].gap ) {
			inGap=true;
		} else {
			nextD.x=-nextD.x;
			nextD.y=-nextD.y;
			DoButt(dest,curW*width,butt,curP,nextD,lastRight,lastLeft);
			nextD.x=-nextD.x;
			nextD.y=-nextD.y;
			inGap=false;
		}
	}
	do {
		prevP=curP;
		prevI=curI;
		prevW=curW;
		prevA=curA;
		curP=nextP;
		curI=nextI;
		curW=nextW;
		curA=nextA;
		prevLe=nextLe;
		prevD=nextD;
		nextI++;
		path_lineto*  curPt=(path_lineto*)pts;
		if ( back ) {
			if ( weighted ) curPt=(path_lineto*)(((char*)curPt)+nextI*sizeof(path_lineto_wb)); else curPt=(path_lineto*)(((char*)curPt)+nextI*sizeof(path_lineto_b));
		} else {
			if ( weighted ) curPt=(path_lineto*)(((char*)curPt)+nextI*sizeof(path_lineto_w)); else curPt=(path_lineto*)(((char*)curPt)+nextI*sizeof(path_lineto));
		}
		while ( nextI <= upTo ) {
			nextP.x=curPt->x;
			nextP.y=curPt->y;
			if ( weighted ) nextW=((path_lineto_w*)curPt)->w; else nextW=1;
			float   dist=(curP.x-nextP.x)*(curP.x-nextP.x)+(curP.y-nextP.y)*(curP.y-nextP.y);
			if ( dist > 0.001 ) {
				break;
			}
			nextI++;
			if ( back ) {
				if ( weighted ) curPt=(path_lineto*)(((char*)curPt)+sizeof(path_lineto_wb)); else curPt=(path_lineto*)(((char*)curPt)+sizeof(path_lineto_b));
			} else {
				if ( weighted ) curPt=(path_lineto*)(((char*)curPt)+sizeof(path_lineto_w)); else curPt=(path_lineto*)(((char*)curPt)+sizeof(path_lineto));
			}
		}
		if ( nextI > upTo ) break;

		nextD.x=nextP.x-curP.x;
		nextD.y=nextP.y-curP.y;
		nextA=curA+sqrt(nextD.x*nextD.x+nextD.y*nextD.y);
		nextLe=sqrt(nextD.x*nextD.x+nextD.y*nextD.y);
		Normalize(nextD);

		dTo.nDashAbs=curA;
		dTo.prevP=prevP;
		dTo.curP=curP;
		dTo.prevD=prevD;
		dTo.prevW=prevW*width;
		dTo.curW=curW*width;

		DashTo(dest,&dTo,dashAbs,dashNo,dashPos,inGap,lastLeft,lastRight,nbDash,dashs);

		if ( inGap == false ) {
			int   nStL=-1,nStR=-1,nEnL=-1,nEnR=-1;
			DoJoin(dest,curW*width,join,curP,prevD,nextD,miter,prevLe,nextLe,nStL,nEnL,nStR,nEnR);
			dest->AddEdge(nStL,lastLeft);
			lastLeft=nEnL;
			dest->AddEdge(lastRight,nStR);
			lastRight=nEnR;
		}
	} while ( nextI <= upTo );
	if ( doClose ) {
/*		prevP=curP;
		prevI=curI;
		prevW=curW;
		prevA=curA;
		curP=nextP;
		curI=nextI;
		curW=nextW;
		curA=nextA;
		prevD=nextD;*/
		path_lineto*  curPt=(path_lineto*)pts;
		nextP.x=curPt->x;
		nextP.y=curPt->y;
		if ( weighted ) nextW=((path_lineto_w*)curPt)->w; else nextW=1;

		nextD.x=nextP.x-curP.x;
		nextD.y=nextP.y-curP.y;
		nextA=curA+sqrt(nextD.x*nextD.x+nextD.y*nextD.y);
		nextLe=sqrt(nextD.x*nextD.x+nextD.y*nextD.y);
		Normalize(nextD);

		dTo.nDashAbs=curA;
		dTo.prevP=prevP;
		dTo.curP=curP;
		dTo.prevD=prevD;
		dTo.prevW=prevW*width;
		dTo.curW=curW*width;

		DashTo(dest,&dTo,dashAbs,dashNo,dashPos,inGap,lastLeft,lastRight,nbDash,dashs);
		if ( inGap == false ) {
			int   nStL=-1,nStR=-1,nEnL=-1,nEnR=-1;
			DoJoin(dest,curW*width,join,curP,prevD,nextD,miter,prevLe,nextLe,nStL,nEnL,nStR,nEnR);
			dest->AddEdge(nStL,lastLeft);
			lastLeft=nEnL;
			dest->AddEdge(lastRight,nStR);
			lastRight=nEnR;
		}
		dTo.nDashAbs=nextA;
		dTo.prevP=curP;
		dTo.curP=nextP;
		dTo.prevD=nextD;
		dTo.prevW=curW*width;
		dTo.curW=nextW*width;

		DashTo(dest,&dTo,dashAbs,dashNo,dashPos,inGap,lastLeft,lastRight,nbDash,dashs);

		if ( inGap == false ) {
			if ( noStartJoin == false ) {
				dest->AddEdge(startLeft,lastLeft);
				dest->AddEdge(lastRight,startRight);
			} else {
				dest->AddEdge(lastRight,lastLeft);
			}
		} else {
			if ( noStartJoin == false ) {
				dest->AddEdge(startLeft,startRight);
			}
		}
	} else {
		dTo.nDashAbs=curA;
		dTo.prevP=prevP;
		dTo.curP=curP;
		dTo.prevD=prevD;
		dTo.prevW=prevW*width;
		dTo.curW=curW*width;

		DashTo(dest,&dTo,dashAbs,dashNo,dashPos,inGap,lastLeft,lastRight,nbDash,dashs);
		if ( inGap == false ) {
			int   endRight,endLeft;
			DoButt(dest,curW*width,butt,curP,prevD,endLeft,endRight);
			dest->AddEdge(endLeft,lastLeft);
			dest->AddEdge(lastRight,endRight);
		}
	}
	
}

void            Path::DashTo(Shape* dest,dashTo_info *dTo,float &dashAbs,int& dashNo,float& dashPos,bool& inGap,int& lastLeft,int& lastRight,int nbDash,one_dash* dashs)
{
//	printf("%f %i %f %i -> %f\n",dashAbs,dashNo,dashPos,(inGap)?1:0,dTo->nDashAbs);
	vec2   pnor;
	RotCCWTo(dTo->prevD,pnor);
	float oDashAbs=dashAbs;
	while ( dashAbs < dTo->nDashAbs ) {
		int    cDashNo=dashNo;
		do {
			float   delta=dashs[dashNo].length-dashPos;
			if ( delta <= dTo->nDashAbs-dashAbs ) {
				dashNo++;
				dashPos=0;
				if ( dashNo >= nbDash ) dashNo-=nbDash;
				while ( dashNo != cDashNo && dashs[dashNo].length <= 0 ) {
					dashNo++;
					dashPos=0;
					if ( dashNo >= nbDash ) dashNo-=nbDash;
				}
				if ( dashs[dashNo].length > 0 ) {
					vec2  pos;
					float nw;
					dashAbs+=delta;
					pos.x=(dTo->prevP.x*(dTo->nDashAbs-dashAbs)+dTo->curP.x*(dashAbs-oDashAbs))/(dTo->nDashAbs-oDashAbs);
					pos.y=(dTo->prevP.y*(dTo->nDashAbs-dashAbs)+dTo->curP.y*(dashAbs-oDashAbs))/(dTo->nDashAbs-oDashAbs);
					nw=(dTo->prevW*(dTo->nDashAbs-dashAbs)+dTo->curW*(dashAbs-oDashAbs))/(dTo->nDashAbs-oDashAbs);
					
					if ( inGap && dashs[dashNo].gap == false ) {
						float   x,y;
						x=pos.x+nw*pnor.x;
						y=pos.y+nw*pnor.y;
						int nleftNo=dest->AddPoint(x,y);
						x=pos.x-nw*pnor.x;
						y=pos.y-nw*pnor.y;
						int nrightNo=dest->AddPoint(x,y);
						dest->AddEdge(nleftNo,nrightNo);
						lastLeft=nleftNo;
						lastRight=nrightNo;

						inGap=false;
					} else if ( inGap == false && dashs[dashNo].gap ) {
						float   x,y;
						x=pos.x+nw*pnor.x;
						y=pos.y+nw*pnor.y;
						int nleftNo=dest->AddPoint(x,y);
						x=pos.x-nw*pnor.x;
						y=pos.y-nw*pnor.y;
						int nrightNo=dest->AddPoint(x,y);
						dest->AddEdge(nrightNo,nleftNo);
						dest->AddEdge(lastRight,nrightNo);
						dest->AddEdge(nleftNo,lastLeft);
						lastLeft=-1;
						lastRight=-1;

						inGap=true;
					} else {
						
					}
				}
			} else {
				dashPos+=dTo->nDashAbs-dashAbs;
				dashAbs=dTo->nDashAbs;
				break;
			}
		}	while ( dashNo != cDashNo );
	}
	inGap=dashs[dashNo].gap;
}
