/*
 *  PathSimplify.cpp
 *  nlivarot
 *
 *  Created by fred on Fri Dec 12 2003.
 *
 */

#include "Path.h"
#include "MyMath.h"
#include <math.h>

// algo d'origine: http://www.cs.mtu.edu/~shene/COURSES/cs3621/NOTES/INT-APP/CURVE-APP-global.html

// need the b-spline basis for cubic splines
// pas oublier que c'est une b-spline clampee
// et que ca correspond a une courbe de bezier normale
#define N03(t) ((1.0-t)*(1.0-t)*(1.0-t))
#define N13(t) (3*t*(1.0-t)*(1.0-t))
#define N23(t) (3*t*t*(1.0-t))
#define N33(t) (t*t*t)
// quadratic b-splines (jsut in case)
#define N02(t) ((1.0-t)*(1.0-t))
#define N12(t) (2*t*(1.0-t))
#define N22(t) (t*t)
// linear interpolation b-splines
#define N01(t) ((1.0-t))
#define N11(t) (t)


void            Path::Simplify(float treshhold)
{
	if ( nbPt <= 1 ) return;
	Reset();
	
	char*  savPts=pts;
	int    savNbPt=nbPt;
	
	int    lastM=0;
	while ( lastM < savNbPt ) {
		int  lastP=lastM+1;
		if ( back ) {
			if ( weighted ) {
				path_lineto_wb* tp=(path_lineto_wb*)savPts;
				while ( lastP < savNbPt && ( (tp+lastP)->isMoveTo == polyline_lineto || (tp+lastP)->isMoveTo == polyline_forced ) ) lastP++;
				pts=(char*)(tp+lastM);
				nbPt=lastP-lastM;
			} else {
				path_lineto_b* tp=(path_lineto_b*)savPts;
				while ( lastP < savNbPt && ( (tp+lastP)->isMoveTo == polyline_lineto || (tp+lastP)->isMoveTo == polyline_forced ) ) lastP++;
				pts=(char*)(tp+lastM);
				nbPt=lastP-lastM;
			}
		} else {
			if ( weighted ) {
				path_lineto_w* tp=(path_lineto_w*)savPts;
				while ( lastP < savNbPt && ( (tp+lastP)->isMoveTo == polyline_lineto || (tp+lastP)->isMoveTo == polyline_forced ) ) lastP++;
				pts=(char*)(tp+lastM);
				nbPt=lastP-lastM;
			} else {
				path_lineto* tp=(path_lineto*)savPts;
				while ( lastP < savNbPt && ( (tp+lastP)->isMoveTo == polyline_lineto || (tp+lastP)->isMoveTo == polyline_forced ) ) lastP++;
				pts=(char*)(tp+lastM);
				nbPt=lastP-lastM;
			}
		}

		DoSimplify(treshhold);
		
		lastM=lastP;
	}
	
	pts=savPts;
	nbPt=savNbPt;
}
void                   Path::DoSimplify(float treshhold)
{
	if ( nbPt <= 1 ) return;
	int    curP=0;
	
	char*  savPts=pts;
	int    savNbPt=nbPt;
	vec2   moveToPt,endToPt;
	if ( back ) {
		if ( weighted ) {
			path_lineto_wb* tp=(path_lineto_wb*)savPts;
			moveToPt.x=tp[0].x;
			moveToPt.y=tp[0].y;
		} else {
			path_lineto_b* tp=(path_lineto_b*)savPts;
			moveToPt.x=tp[0].x;
			moveToPt.y=tp[0].y;
		}
	} else {
		if ( weighted ) {
			path_lineto_w* tp=(path_lineto_w*)savPts;
			moveToPt.x=tp[0].x;
			moveToPt.y=tp[0].y;
		} else {
			path_lineto* tp=(path_lineto*)savPts;
			moveToPt.x=tp[0].x;
			moveToPt.y=tp[0].y;
		}
	}
	MoveTo(moveToPt.x,moveToPt.y);
	endToPt=moveToPt;
		
	while ( curP < savNbPt-1 ) {
		int lastP=curP+1;
		nbPt=2;
		if ( back ) {
			if ( weighted ) {
				path_lineto_wb* tp=(path_lineto_wb*)savPts;
				pts=(char*)(tp+curP);
			} else {
				path_lineto_b* tp=(path_lineto_b*)savPts;
				pts=(char*)(tp+curP);
			}
		} else {
			if ( weighted ) {
				path_lineto_w* tp=(path_lineto_w*)savPts;
				pts=(char*)(tp+curP);
			} else {
				path_lineto* tp=(path_lineto*)savPts;
				pts=(char*)(tp+curP);
			}
		}
		
		path_descr_cubicto   res;
		do {
			bool kissGoodbye=false;
			if ( back ) {
				if ( weighted ) {
					path_lineto_wb* tp=(path_lineto_wb*)savPts;
					if ( (tp+lastP)->isMoveTo == polyline_forced ) kissGoodbye=true;
				} else {
					path_lineto_b* tp=(path_lineto_b*)savPts;
					if ( (tp+lastP)->isMoveTo == polyline_forced ) kissGoodbye=true;
				}
			} else {
				if ( weighted ) {
					path_lineto_w* tp=(path_lineto_w*)savPts;
					if ( (tp+lastP)->isMoveTo == polyline_forced ) kissGoodbye=true;
				} else {
					path_lineto* tp=(path_lineto*)savPts;
					if ( (tp+lastP)->isMoveTo == polyline_forced ) kissGoodbye=true;
				}
			}
			lastP++;
			nbPt++;
			if ( kissGoodbye ) break;
		} while ( lastP < savNbPt && AttemptSimplify(treshhold,res) );
		
		if ( lastP >= savNbPt ) {
//			printf("o");
			lastP--;
			nbPt--;
		} else {
			// le dernier a echoué
			lastP--;
			nbPt--;
		}
		if ( back ) {
			if ( weighted ) {
				path_lineto_wb* tp=(path_lineto_wb*)savPts;
				endToPt.x=tp[lastP].x;
				endToPt.y=tp[lastP].y;
			} else {
				path_lineto_b* tp=(path_lineto_b*)savPts;
				endToPt.x=tp[lastP].x;
				endToPt.y=tp[lastP].y;
			}
		} else {
			if ( weighted ) {
				path_lineto_w* tp=(path_lineto_w*)savPts;
				endToPt.x=tp[lastP].x;
				endToPt.y=tp[lastP].y;
			} else {
				path_lineto* tp=(path_lineto*)savPts;
				endToPt.x=tp[lastP].x;
				endToPt.y=tp[lastP].y;
			}
		}
		if ( nbPt <= 2 ) {
			LineTo(endToPt.x,endToPt.y);
		} else {
			CubicTo(endToPt.x,endToPt.y,res.stDx,res.stDy,res.enDx,res.enDy);
		}
		
		curP=lastP;
	}
	
	if ( fabs(endToPt.x-moveToPt.x) < 0.00001 && fabs(endToPt.y-moveToPt.y) < 0.00001 ) Close();
	
	pts=savPts;
	nbPt=savNbPt;
}
bool                   Path::AttemptSimplify(float treshhold,path_descr_cubicto &res)
{
	vec2      start,end;
	// pour une coordonnee
	double    *Xk;     // la coordonnee traitee (x puis y)
	double    *Yk;     // la coordonnee traitee (x puis y)
	double    *tk;     // les tk
	double    *Qk;     // les Qk
	mat2d     M; // la matrice tNN
	vec2d     P;
	vec2d	  	Q;
	
	vec2      cp1,cp2;
	
	if ( nbPt == 2 ) return true;
	
	if ( back ) {
		if ( weighted ) {
			path_lineto_wb* tp=(path_lineto_wb*)pts;
			start.x=tp[0].x;
			start.y=tp[0].y;
			cp1.x=tp[1].x;
			cp1.y=tp[1].y;
			end.x=tp[nbPt-1].x;
			end.y=tp[nbPt-1].y;
		} else {
			path_lineto_b* tp=(path_lineto_b*)pts;
			start.x=tp[0].x;
			start.y=tp[0].y;
			cp1.x=tp[1].x;
			cp1.y=tp[1].y;
			end.x=tp[nbPt-1].x;
			end.y=tp[nbPt-1].y;
		}
	} else {
		if ( weighted ) {
			path_lineto_w* tp=(path_lineto_w*)pts;
			start.x=tp[0].x;
			start.y=tp[0].y;
			cp1.x=tp[1].x;
			cp1.y=tp[1].y;
			end.x=tp[nbPt-1].x;
			end.y=tp[nbPt-1].y;
		} else {
			path_lineto* tp=(path_lineto*)pts;
			start.x=tp[0].x;
			start.y=tp[0].y;
			cp1.x=tp[1].x;
			cp1.y=tp[1].y;
			end.x=tp[nbPt-1].x;
			end.y=tp[nbPt-1].y;
		}
	}
	
	if ( nbPt == 3 ) {
		// start -> cp1 -> end
		res.x=end.x;
		res.y=end.y;
		res.stDx=cp1.x-start.x;
		res.stDy=cp1.y-start.y;
		res.enDx=end.x-cp1.x;
		res.enDy=end.y-cp1.y;
		return true;
	}

	// totalement inefficace, alloue et desalloue tout le temps
	tk=(double*)malloc(nbPt*sizeof(double));
	Qk=(double*)malloc(nbPt*sizeof(double));
	Xk=(double*)malloc(nbPt*sizeof(double));
	Yk=(double*)malloc(nbPt*sizeof(double));
	
	// chord length method
	tk[0]=0.0;
	{
		vec2  prevP=start;
		for (int i=1;i<nbPt;i++) {
			if ( back ) {
				if ( weighted ) {
					path_lineto_wb* tp=(path_lineto_wb*)pts;
					Xk[i]=tp[i].x;
					Yk[i]=tp[i].y;
				} else {
					path_lineto_b* tp=(path_lineto_b*)pts;
					Xk[i]=tp[i].x;
					Yk[i]=tp[i].y;
				}
			} else {
				if ( weighted ) {
					path_lineto_w* tp=(path_lineto_w*)pts;
					Xk[i]=tp[i].x;
					Yk[i]=tp[i].y;
				} else {
					path_lineto* tp=(path_lineto*)pts;
					Xk[i]=tp[i].x;
					Yk[i]=tp[i].y;
				}
			}
			vec2 diff;
			diff.x=Xk[i]-prevP.x;
			diff.y=Yk[i]-prevP.y;
			prevP.x=Xk[i];
			prevP.y=Yk[i];
			float l=sqrt(diff.x*diff.x+diff.y*diff.y);
			tk[i]=tk[i-1]+l;
		}
	}
	if ( tk[nbPt-1] < 0.00001 ) {
		// longueur nulle 
		free(tk);
		free(Qk);
		free(Xk);
		free(Yk);
		return false;
	}
	for (int i=1;i<nbPt-1;i++) tk[i]/=tk[nbPt-1];
	
	// la matrice tNN
	M.xx=M.xy=M.yx=M.yy=0;
	for (int i=1;i<nbPt-1;i++) {
		M.xx+=N13(tk[i])*N13(tk[i]);
		M.xy+=N23(tk[i])*N13(tk[i]);
		M.yx+=N13(tk[i])*N23(tk[i]);
		M.yy+=N23(tk[i])*N23(tk[i]);
	}
	
	double det;
	L_MAT_Det(M,det);
	if ( fabs(det) < 0.000001 ) {
		// aie, non-inversible
		
		free(tk);
		free(Qk);
		free(Xk);
		free(Yk);
		return false;
	}
	L_MAT_Inv(M);

	
	// phase 1: abcisses
	// calcul des Qk
	Xk[0]=start.x;
	Yk[0]=start.y;
	Xk[nbPt-1]=end.x;
	Yk[nbPt-1]=end.y;
	
	for (int i=1;i<nbPt-1;i++) Qk[i]=Xk[i]-N03(tk[i])*Xk[0]-N33(tk[i])*Xk[nbPt-1];
	
	// le vecteur Q
	Q.x=Q.y=0;
	for (int i=1;i<nbPt-1;i++) {
		Q.x+=N13(tk[i])*Qk[i];
		Q.y+=N23(tk[i])*Qk[i];
	}
	
	L_MAT_MulV(M,Q,P);
	cp1.x=P.x;
	cp2.x=P.y;
	
	// phase 2: les ordonnees
	for (int i=1;i<nbPt-1;i++) Qk[i]=Yk[i]-N03(tk[i])*Yk[0]-N33(tk[i])*Yk[nbPt-1];
	
	// le vecteur Q
	Q.x=Q.y=0;
	for (int i=1;i<nbPt-1;i++) {
		Q.x+=N13(tk[i])*Qk[i];
		Q.y+=N23(tk[i])*Qk[i];
	}
	
	L_MAT_MulV(M,Q,P);
	cp1.y=P.x;
	cp2.y=P.y;
	
	float  delta=0;
	for (int i=1;i<nbPt-1;i++) {
		vec2 appP;
		appP.x=N13(tk[i])*cp1.x+N23(tk[i])*cp2.x;
		appP.y=N13(tk[i])*cp1.y+N23(tk[i])*cp2.y;
		appP.x-=Xk[i]-N03(tk[i])*Xk[0]-N33(tk[i])*Xk[nbPt-1];
		appP.y-=Yk[i]-N03(tk[i])*Yk[0]-N33(tk[i])*Yk[nbPt-1];
		delta+=appP.x*appP.x+appP.y*appP.y;
	}
		
	
	if ( delta < treshhold*treshhold ) {
		// premier jet
		res.stDx=3.0*(cp1.x-start.x);
		res.stDy=3.0*(cp1.y-start.y);
		res.enDx=-3.0*(cp2.x-end.x);
		res.enDy=-3.0*(cp2.y-end.y);
		res.x=end.x;
		res.y=end.y;

		// on raffine un peu
		for (int i=1;i<nbPt-1;i++) {
			vec2 pt;
			pt.x=Xk[i];pt.y=Yk[i];
			tk[i]=RaffineTk(pt,start,cp1,cp2,end,tk[i]);
			if ( tk[i] < tk[i-1] ) {
				// forcer l'ordre croissant
				tk[i]=tk[i-1];
			}
		}
		
		// la matrice tNN
		M.xx=M.xy=M.yx=M.yy=0;
		for (int i=1;i<nbPt-1;i++) {
			M.xx+=N13(tk[i])*N13(tk[i]);
			M.xy+=N23(tk[i])*N13(tk[i]);
			M.yx+=N13(tk[i])*N23(tk[i]);
			M.yy+=N23(tk[i])*N23(tk[i]);
		}
		
		L_MAT_Det(M,det);
		if ( fabs(det) < 0.000001 ) {
			// aie, non-inversible
			
			free(tk);
			free(Qk);
			free(Xk);
			free(Yk);
			return true;
		}
		L_MAT_Inv(M);
		
		
		// phase 1: abcisses
	// calcul des Qk
		Xk[0]=start.x;
		Yk[0]=start.y;
		Xk[nbPt-1]=end.x;
		Yk[nbPt-1]=end.y;
		
		for (int i=1;i<nbPt-1;i++) Qk[i]=Xk[i]-N03(tk[i])*Xk[0]-N33(tk[i])*Xk[nbPt-1];
		
		// le vecteur Q
		Q.x=Q.y=0;
		for (int i=1;i<nbPt-1;i++) {
			Q.x+=N13(tk[i])*Qk[i];
			Q.y+=N23(tk[i])*Qk[i];
		}
		
		L_MAT_MulV(M,Q,P);
		cp1.x=P.x;
		cp2.x=P.y;
		
		// phase 2: les ordonnees
		for (int i=1;i<nbPt-1;i++) Qk[i]=Yk[i]-N03(tk[i])*Yk[0]-N33(tk[i])*Yk[nbPt-1];
		
		// le vecteur Q
		Q.x=Q.y=0;
		for (int i=1;i<nbPt-1;i++) {
			Q.x+=N13(tk[i])*Qk[i];
			Q.y+=N23(tk[i])*Qk[i];
		}
		
		L_MAT_MulV(M,Q,P);
		cp1.y=P.x;
		cp2.y=P.y;
		
		float ndelta=0;
		for (int i=1;i<nbPt-1;i++) {
			vec2 appP;
			appP.x=N13(tk[i])*cp1.x+N23(tk[i])*cp2.x;
			appP.y=N13(tk[i])*cp1.y+N23(tk[i])*cp2.y;
			appP.x-=Xk[i]-N03(tk[i])*Xk[0]-N33(tk[i])*Xk[nbPt-1];
			appP.y-=Yk[i]-N03(tk[i])*Yk[0]-N33(tk[i])*Yk[nbPt-1];
			ndelta+=appP.x*appP.x+appP.y*appP.y;
		}
		
		free(tk);
		free(Qk);
		free(Xk);
		free(Yk);

		if ( ndelta < delta+0.00001 ) {
			res.stDx=3.0*(cp1.x-start.x);
			res.stDy=3.0*(cp1.y-start.y);
			res.enDx=-3.0*(cp2.x-end.x);
			res.enDy=-3.0*(cp2.y-end.y);
			res.x=end.x;
			res.y=end.y;
			return true;
		}
		

		return false;
	}
	
	free(tk);
	free(Qk);
	free(Xk);
	free(Yk);
	return false;
}

float                  Path::RaffineTk(vec2 pt,vec2 p0,vec2 p1,vec2 p2,vec2 p3,float it)
{
	// raffinement des tk
	// une seule iteration de newtow rhapston, vu que de toute facon la courbe est approchée
	double   Ax,Bx,Cx;
	double   Ay,By,Cy;
	Ax=pt.x-p0.x*N03(it)-p1.x*N13(it)-p2.x*N23(it)-p3.x*N33(it);
	Bx=(p1.x-p0.x)*N02(it)+(p2.x-p1.x)*N12(it)+(p3.x-p2.x)*N22(it);
	Cx=(p0.x-2*p1.x+p2.x)*N01(it)+(p3.x-2*p2.x+p1.x)*N11(it);
	Ay=pt.y-p0.y*N03(it)-p1.y*N13(it)-p2.y*N23(it)-p3.y*N33(it);
	By=(p1.y-p0.y)*N02(it)+(p2.y-p1.y)*N12(it)+(p3.y-p2.y)*N22(it);
	Cy=(p0.y-2*p1.y+p2.y)*N01(it)+(p3.y-2*p2.y+p1.y)*N11(it);
	double   dF,ddF;
	dF=-6*(Ax*Bx+Ay*By);
	ddF=18*(Bx*Bx+By*By)-12*(Ax*Cx+Ay*Cy);
	if ( fabs(ddF) > 0.0000001 ) {
		return it-dF/ddF;
	}
	return it;
}

void            Path::Coalesce(float tresh)
{
	if ( descr_flags&descr_adding_bezier ) CancelBezier();
	if ( descr_nb <= 2 ) return;
	
	SetWeighted(false);
	SetBackData(false);
	
	ConvertEvenLines(tresh);

	int         lastP=0;
	int         writeP=0;
	int					lastA=descr_data[0].associated;
	vec2        firstP;
	path_descr  lastAddition;
	lastAddition.flags=descr_moveto;
	for (int curP=0;curP<descr_nb;curP++) {
		int typ=descr_data[curP].flags&descr_type_mask;
		int nextA=lastA;
		if ( typ == descr_moveto ) {
			if ( lastAddition.flags != descr_moveto ) {
				descr_data[writeP++]=lastAddition;
			}
			lastAddition=descr_data[curP];
			descr_data[writeP++]=lastAddition; // ajouté automatiquement (tant pis pour les moveto multiples)
			
			firstP.x=descr_data[curP].d.m.x;
			firstP.y=descr_data[curP].d.m.y;
			lastA=descr_data[curP].associated;
		} else if ( typ == descr_close ) {
			nextA=descr_data[curP].associated;
			if ( lastAddition.flags != descr_moveto ) {
				path_lineto* sav_pts=(path_lineto*)pts;
				int          sav_nbPt=nbPt;
				
				pts=(char*)(sav_pts+lastA);
				nbPt=nextA-lastA+1;
				
				path_descr_cubicto res;
				if ( AttemptSimplify(tresh,res) ) {
					lastAddition.flags=descr_cubicto;
					lastAddition.d.c.x=res.x;
					lastAddition.d.c.y=res.y;
					lastAddition.d.c.stDx=res.stDx;
					lastAddition.d.c.stDy=res.stDy;
					lastAddition.d.c.enDx=res.enDx;
					lastAddition.d.c.enDy=res.enDy;
				} else {
				}
				
				descr_data[writeP++]=lastAddition;
				descr_data[writeP++]=descr_data[curP];
				
				pts=(char*)sav_pts;
				nbPt=sav_nbPt;
			} else {
				descr_data[writeP++]=descr_data[curP];
			}
			lastAddition.flags=descr_moveto;
			lastA=nextA;
		} else if ( typ == descr_lineto || typ == descr_cubicto || typ == descr_arcto) {
			nextA=descr_data[curP].associated;
			if ( lastAddition.flags != descr_moveto ) {
				path_lineto* sav_pts=(path_lineto*)pts;
				int          sav_nbPt=nbPt;
				
				pts=(char*)(sav_pts+lastA);
				nbPt=nextA-lastA+1;
				
				path_descr_cubicto res;
				if ( AttemptSimplify(tresh,res) ) {
					lastAddition.flags=descr_cubicto;
					lastAddition.d.c.x=res.x;
					lastAddition.d.c.y=res.y;
					lastAddition.d.c.stDx=res.stDx;
					lastAddition.d.c.stDy=res.stDy;
					lastAddition.d.c.enDx=res.enDx;
					lastAddition.d.c.enDy=res.enDy;
				} else {
					lastA=descr_data[curP-1].associated; // pourrait etre surecrit par la ligne suivante
					descr_data[writeP++]=lastAddition;
					lastAddition=descr_data[curP];
				}
								
				pts=(char*)sav_pts;
				nbPt=sav_nbPt;
			} else {
				lastA=descr_data[curP-1].associated;
				lastAddition=descr_data[curP];
			}
		} else if ( typ == descr_bezierto ) {
			if ( lastAddition.flags != descr_moveto ) {
				descr_data[writeP++]=lastAddition;
				lastAddition.flags=descr_moveto;
			} else {
			}
			lastA=descr_data[curP].associated;
			for (int i=1;i<=descr_data[curP].d.b.nb;i++) descr_data[writeP++]=descr_data[curP+i];
			curP+=descr_data[curP].d.b.nb;
		} else if ( typ == descr_interm_bezier ) {
			continue;
		} else if ( typ == descr_forced ) {
			continue;
		} else {
			continue;		
		}
	}
	if ( lastAddition.flags != descr_moveto ) {
		descr_data[writeP++]=lastAddition;
	}
	descr_nb=writeP;
}
void            Path::DoCoalesce(Path* dest,float tresh)
{
	
	
}
