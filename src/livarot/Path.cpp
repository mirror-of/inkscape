/*
 *  Path.cpp
 *  nlivarot
 *
 *  Created by fred on Tue Jun 17 2003.
 *  public domain
 *
 */

#include "Path.h"
//#include "Shape.h"


Path::Path(void)
{
	descr_flags=0;
	descr_max=descr_nb=0;
	descr_data=NULL;
	pending_bezier=-1;
	pending_moveto=-1;

	weighted=false;
	back=false;
	nbPt=maxPt=sizePt=0;
	pts=NULL;
}
Path::~Path(void)
{
	if ( descr_data ) {
		free(descr_data);
		descr_data=NULL;
	}
	if ( pts ) {
		free(pts);
		pts=NULL;
	}
	descr_max=descr_nb=0;
	descr_data=NULL;
	nbPt=maxPt=sizePt=0;
	pts=NULL;
}

/*
 * description
 */
void					  Path::Affiche(void)
{
	printf("path descr %i elems\n",descr_nb);
	for (int i=0;i<descr_nb;i++) {
		printf("  ");
		if ( (descr_data+i)->flags&descr_weighted ) printf(" w ");
		int ty=(descr_data+i)->flags&descr_type_mask;
		if ( ty == descr_moveto ) {
			printf("M %f %f %i\n",(descr_data+i)->d.m.x,(descr_data+i)->d.m.y,(descr_data+i)->d.m.pathLength);
		} else if ( ty == descr_lineto ) {
			printf("L %f %f\n",(descr_data+i)->d.l.x,(descr_data+i)->d.l.y);
		} else if ( ty == descr_arcto ) {
			printf("A %f %f %f %f %i %i\n",(descr_data+i)->d.a.x,(descr_data+i)->d.a.y,(descr_data+i)->d.a.rx,(descr_data+i)->d.a.ry,((descr_data+i)->d.a.large)?1:0,((descr_data+i)->d.a.clockwise)?1:0);
		} else if ( ty == descr_cubicto ) {
			printf("C %f %f %f %f %f %f\n",(descr_data+i)->d.c.x,(descr_data+i)->d.c.y,(descr_data+i)->d.c.stDx,(descr_data+i)->d.c.stDy,(descr_data+i)->d.c.enDx,(descr_data+i)->d.c.enDy);
		} else if ( ty == descr_bezierto ) {
			printf("B %f %f %i\n",(descr_data+i)->d.b.x,(descr_data+i)->d.b.y,(descr_data+i)->d.b.nb);
		} else if ( ty == descr_interm_bezier ) {
			printf("I %f %f\n",(descr_data+i)->d.i.x,(descr_data+i)->d.i.y);
		} else if ( ty == descr_close ) {
			printf("Z\n");
		}
	}
}
void            Path::Reset(void)
{
	descr_nb=0;
	pending_bezier=-1;
	pending_moveto=-1;
	descr_flags=0;
}
void            Path::Alloue(int addSize)
{
	if ( descr_nb+addSize > descr_max ) {
		descr_max=2*descr_nb+addSize;
		descr_data=(path_descr*)realloc(descr_data,descr_max*sizeof(path_descr));
	}
}
void            Path::CloseSubpath(int add)
{
	for (int i=descr_nb-1;i>=0;i--) {
		int ty=(descr_data+i)->flags&descr_type_mask;
		if ( ty == descr_moveto ) {
			(descr_data+i)->d.m.pathLength=descr_nb-i+add; // il faut compter le close qui n'est pas encore ajouté
			break;
		}
	}
	descr_flags&=~(descr_doing_subpath);
	pending_moveto=-1;
}

void            Path::Close(void)
{
	if ( descr_flags&descr_adding_bezier ) CancelBezier();
	if ( descr_flags&descr_doing_subpath ) {
		CloseSubpath(1);
	} else {
		// rien a fermer => byebye
		return;
	}
	Alloue(1);
	path_descr *nElem=descr_data+descr_nb;
	descr_nb++;
	nElem->flags=descr_close;
	descr_flags&=~(descr_doing_subpath);
	pending_moveto=-1;
}
void            Path::MoveTo(float ix,float iy)
{
	if ( descr_flags&descr_adding_bezier ) EndBezierTo(ix,iy);
	if ( descr_flags&descr_doing_subpath ) CloseSubpath(0);
	
	pending_moveto=descr_nb;
	Alloue(1);
	path_descr *nElem=descr_data+descr_nb;
	descr_nb++;
	nElem->flags=descr_moveto;
	nElem->d.m.x=ix;
	nElem->d.m.y=iy;
	nElem->d.m.pathLength=0;
	descr_flags|=descr_doing_subpath;
}
void            Path::MoveTo(float ix,float iy,float iw)
{
	if ( descr_flags&descr_adding_bezier ) EndBezierTo(ix,iy,iw);
	if ( descr_flags&descr_doing_subpath ) CloseSubpath(0);
	
	pending_moveto=descr_nb;
	Alloue(1);
	path_descr *nElem=descr_data+descr_nb;
	descr_nb++;
	nElem->flags=descr_moveto|descr_weighted;
	nElem->d.m.x=ix;
	nElem->d.m.y=iy;
	nElem->d.m.w=iw;
	nElem->d.m.pathLength=0;
	descr_flags|=descr_doing_subpath;
}
void            Path::LineTo(float ix,float iy)
{
	if ( descr_flags&descr_adding_bezier ) EndBezierTo(ix,iy);
	if ( descr_flags&descr_doing_subpath ) {
	} else {
		MoveTo(ix,iy);
		return;
	}
	Alloue(1);
	path_descr *nElem=descr_data+descr_nb;
	descr_nb++;
	nElem->flags=descr_lineto;
	nElem->d.l.x=ix;
	nElem->d.l.y=iy;
}
void            Path::LineTo(float ix,float iy,float iw)
{
	if ( descr_flags&descr_adding_bezier ) EndBezierTo(ix,iy,iw);
	if ( descr_flags&descr_doing_subpath ) {
	} else {
		MoveTo(ix,iy,iw);
		return;
	}
	Alloue(1);
	path_descr *nElem=descr_data+descr_nb;
	descr_nb++;
	nElem->flags=descr_lineto|descr_weighted;
	nElem->d.l.x=ix;
	nElem->d.l.y=iy;
	nElem->d.l.w=iw;
}
void            Path::CubicTo(float ix,float iy,float isDx,float isDy,float ieDx,float ieDy)
{
	if ( descr_flags&descr_adding_bezier ) EndBezierTo(ix,iy);
	if ( descr_flags&descr_doing_subpath ) {
	} else {
		MoveTo(ix,iy);
		return;
	}
	Alloue(1);
	path_descr *nElem=descr_data+descr_nb;
	descr_nb++;
	nElem->flags=descr_cubicto;
	nElem->d.c.x=ix;
	nElem->d.c.y=iy;
	nElem->d.c.stDx=isDx;
	nElem->d.c.stDy=isDy;
	nElem->d.c.enDx=ieDx;
	nElem->d.c.enDy=ieDy;
}
void            Path::CubicTo(float ix,float iy,float isDx,float isDy,float ieDx,float ieDy,float iw)
{
	if ( descr_flags&descr_adding_bezier ) EndBezierTo(ix,iy,iw);
	if ( descr_flags&descr_doing_subpath ) {
	} else {
		MoveTo(ix,iy,iw);
		return;
	}
	Alloue(1);
	path_descr *nElem=descr_data+descr_nb;
	descr_nb++;
	nElem->flags=descr_cubicto|descr_weighted;
	nElem->d.c.x=ix;
	nElem->d.c.y=iy;
	nElem->d.c.w=iw;
	nElem->d.c.stDx=isDx;
	nElem->d.c.stDy=isDy;
	nElem->d.c.enDx=ieDx;
	nElem->d.c.enDy=ieDy;
}
void            Path::ArcTo(float ix,float iy,float iRx,float iRy,float angle,bool iLargeArc,bool iClockwise)
{
	if ( descr_flags&descr_adding_bezier ) EndBezierTo(ix,iy);
	if ( descr_flags&descr_doing_subpath ) {
	} else {
		MoveTo(ix,iy);
		return;
	}
	Alloue(1);
	path_descr *nElem=descr_data+descr_nb;
	descr_nb++;
	nElem->flags=descr_arcto;
	nElem->d.a.x=ix;
	nElem->d.a.y=iy;
	nElem->d.a.rx=iRx;
	nElem->d.a.ry=iRy;
	nElem->d.a.angle=angle;
	nElem->d.a.large=iLargeArc;
	nElem->d.a.clockwise=iClockwise;
}
void            Path::ArcTo(float ix,float iy,float iRx,float iRy,float angle,bool iLargeArc,bool iClockwise,float iw)
{
	if ( descr_flags&descr_adding_bezier ) EndBezierTo(ix,iy,iw);
	if ( descr_flags&descr_doing_subpath ) {
	} else {
		MoveTo(ix,iy,iw);
		return;
	}
	Alloue(1);
	path_descr *nElem=descr_data+descr_nb;
	descr_nb++;
	nElem->flags=descr_arcto|descr_weighted;
	nElem->d.a.x=ix;
	nElem->d.a.y=iy;
	nElem->d.a.w=iw;
	nElem->d.a.rx=iRx;
	nElem->d.a.ry=iRy;
	nElem->d.a.angle=angle;
	nElem->d.a.large=iLargeArc;
	nElem->d.a.clockwise=iClockwise;
}
void            Path::TempBezierTo(void)
{
	if ( descr_flags&descr_adding_bezier ) CancelBezier();
	if ( descr_flags&descr_doing_subpath ) {
	} else {
		// pas de pt de départ-> pas bon
		return;
	}
	pending_bezier=descr_nb;
	Alloue(1);
	path_descr *nElem=descr_data+descr_nb;
	descr_nb++;
	nElem->flags=descr_bezierto;
	nElem->d.b.nb=0;
	descr_flags|=descr_adding_bezier;
	descr_flags|=descr_delayed_bezier;
}
void            Path::TempBezierToW(void)
{
	if ( descr_flags&descr_adding_bezier ) CancelBezier();
	if ( descr_flags&descr_doing_subpath ) {
	} else {
		// pas de pt de départ-> pas bon
		return;
	}
	pending_bezier=descr_nb;
	Alloue(1);
	path_descr *nElem=descr_data+descr_nb;
	descr_nb++;
	nElem->flags=descr_bezierto|descr_weighted;
	nElem->d.b.nb=0;
	descr_flags|=descr_adding_bezier;
	descr_flags|=descr_delayed_bezier;
}
void            Path::CancelBezier(void)
{
	descr_flags&=~(descr_adding_bezier);
	descr_flags&=~(descr_delayed_bezier);
	if ( pending_bezier < 0 ) return;
	descr_nb=pending_bezier;
	pending_bezier=-1;
}
void            Path::EndBezierTo(void)
{
	if ( descr_flags&descr_delayed_bezier ) {
		CancelBezier();
	} else {
		pending_bezier=-1;
		descr_flags&=~(descr_adding_bezier);
		descr_flags&=~(descr_delayed_bezier);
	}
}
void            Path::EndBezierTo(float ix,float iy)
{
	if ( descr_flags&descr_adding_bezier ) {
	} else {
		LineTo(ix,iy);
		return;
	}
	if ( descr_flags&descr_doing_subpath ) {
	} else {
		MoveTo(ix,iy);
		return;
	}
	if ( descr_flags&descr_delayed_bezier ) {
	} else {
		EndBezierTo();
	}
	(descr_data+pending_bezier)->d.b.x=ix;
	(descr_data+pending_bezier)->d.b.y=iy;
	if ( (descr_data+pending_bezier)->flags&descr_weighted ) (descr_data+pending_bezier)->d.b.w=1;
	pending_bezier=-1;
	descr_flags&=~(descr_adding_bezier);
	descr_flags&=~(descr_delayed_bezier);
}
void            Path::EndBezierTo(float ix,float iy,float iw)
{
	if ( descr_flags&descr_adding_bezier ) {
	} else {
		LineTo(ix,iy,iw);
		return;
	}
	if ( descr_flags&descr_doing_subpath ) {
	} else {
		MoveTo(ix,iy,iw);
		return;
	}
	if ( descr_flags&descr_delayed_bezier ) {
	} else {
		EndBezierTo();
	}
	(descr_data+pending_bezier)->d.b.x=ix;
	(descr_data+pending_bezier)->d.b.y=iy;
	(descr_data+pending_bezier)->d.b.w=iw;
	pending_bezier=-1;
	descr_flags&=~(descr_adding_bezier);
	descr_flags&=~(descr_delayed_bezier);
}
void            Path::IntermBezierTo(float ix,float iy)
{
	if ( descr_flags&descr_adding_bezier ) {
	} else {
		LineTo(ix,iy);
		return;
	}
	if ( descr_flags&descr_doing_subpath ) {
	} else {
		MoveTo(ix,iy);
		return;
	}

	if ( (descr_data+pending_bezier)->flags&descr_weighted ) {
		IntermBezierTo(ix,iy,1);
	} else {
		Alloue(1);
		path_descr *nElem=descr_data+descr_nb;
		descr_nb++;
		nElem->flags=descr_interm_bezier;
		nElem->d.i.x=ix;
		nElem->d.i.y=iy;
		(descr_data+pending_bezier)->d.b.nb++;
	}
}
void            Path::IntermBezierTo(float ix,float iy,float iw)
{
	if ( descr_flags&descr_adding_bezier ) {
	} else {
		LineTo(ix,iy,iw);
		return;
	}
	if ( descr_flags&descr_doing_subpath ) {
	} else {
		MoveTo(ix,iy,iw);
		return;
	}
	
	if ( (descr_data+pending_bezier)->flags&descr_weighted ) {
		Alloue(1);
		path_descr *nElem=descr_data+descr_nb;
		descr_nb++;
		nElem->flags=descr_interm_bezier|descr_weighted;
		nElem->d.i.x=ix;
		nElem->d.i.y=iy;
		nElem->d.i.w=iw;
		(descr_data+pending_bezier)->d.b.nb++;
	} else {
		IntermBezierTo(ix,iy);
	}
}
void            Path::BezierTo(float ix,float iy)
{
	if ( descr_flags&descr_adding_bezier ) EndBezierTo(ix,iy);
	if ( descr_flags&descr_doing_subpath ) {
	} else {
		MoveTo(ix,iy);
		return;
	}
	pending_bezier=descr_nb;
	Alloue(1);
	path_descr *nElem=descr_data+descr_nb;
	descr_nb++;
	nElem->flags=descr_bezierto;
	nElem->d.b.nb=0;
	nElem->d.b.x=ix;
	nElem->d.b.y=iy;
	descr_flags|=descr_adding_bezier;
	descr_flags&=~(descr_delayed_bezier);
}
void            Path::BezierTo(float ix,float iy,float iw)
{
	if ( descr_flags&descr_adding_bezier ) EndBezierTo(ix,iy,iw);
	if ( descr_flags&descr_doing_subpath ) {
	} else {
		MoveTo(ix,iy,iw);
		return;
	}
	pending_bezier=descr_nb;
	Alloue(1);
	path_descr *nElem=descr_data+descr_nb;
	descr_nb++;
	nElem->flags=descr_bezierto|descr_weighted;
	nElem->d.b.nb=0;
	nElem->d.b.x=ix;
	nElem->d.b.y=iy;
	nElem->d.b.w=iw;
	descr_flags|=descr_adding_bezier;
	descr_flags&=~(descr_delayed_bezier);
}


/*
 * points de la polyligne
 */
void            Path::SetWeighted(bool nVal)
{
	if ( back == false ) {
		if ( nVal == true && weighted == false ) {
			weighted=true;
			ResetPoints(nbPt);
		} else if ( nVal == false && weighted == true ) {
			weighted=false;
			ResetPoints(nbPt);
		}
	} else {
		if ( nVal == true && weighted == false ) {
			weighted=true;
			ResetPoints(nbPt);
		} else if ( nVal == false && weighted == true ) {
			weighted=false;
			ResetPoints(nbPt);
		}
	}
}
void            Path::SetBackData(bool nVal)
{
	if ( back == false ) {
		if ( nVal == true && back == false ) {
			back=true;
			ResetPoints(nbPt);
		} else if ( nVal == false && back == true ) {
			back=false;
			ResetPoints(nbPt);
		}
	} else {
		if ( nVal == true && back == false ) {
			back=true;
			ResetPoints(nbPt);
		} else if ( nVal == false && back == true ) {
			back=false;
			ResetPoints(nbPt);
		}
	}
}
void            Path::ResetPoints(int expected)
{
	nbPt=0;
	if ( back ) {
		if ( weighted ) {
			sizePt=expected*sizeof(path_lineto_wb);
		} else {
			sizePt=expected*sizeof(path_lineto_b);
		}
	} else {
		if ( weighted ) {
			sizePt=expected*sizeof(path_lineto_w);
		} else {
			sizePt=expected*sizeof(path_lineto);
		}
	}
	if ( sizePt > maxPt ) {
		maxPt=sizePt;
		pts=(char*)realloc(pts,maxPt);
	}
}
int             Path::AddPoint(float ix,float iy,bool mvto)
{
	if ( back ) {
		return AddPoint(ix,iy,-1,0.0,mvto);
	} else {
		if ( weighted ) {
			return AddPoint(ix,iy,1.0,mvto);
		}
	}
	int  nextSize=sizePt+sizeof(path_lineto);
	if ( nextSize > maxPt ) {
		maxPt=2*sizePt+sizeof(path_lineto);
		pts=(char*)realloc(pts,maxPt);
	}
	if ( mvto == false && nbPt > 0 && ((path_lineto*)pts)[nbPt-1].x == ix && ((path_lineto*)pts)[nbPt-1].y == iy ) return -1;
	int   n=nbPt++;
	sizePt=nextSize;
	if ( mvto ) ((path_lineto*)pts)[n].isMoveTo=0; else ((path_lineto*)pts)[n].isMoveTo=-1;
	((path_lineto*)pts)[n].x=ix;
	((path_lineto*)pts)[n].y=iy;
	return n;
}
int             Path::AddPoint(float ix,float iy,float iw,bool mvto)
{
	if ( back ) {
		return AddPoint(ix,iy,iw,-1,0.0,mvto);
	} else {
		if ( weighted ) {
		} else {
			return AddPoint(ix,iy,mvto);
		}
	}
	int  nextSize=sizePt+sizeof(path_lineto_w);
	if ( nextSize > maxPt ) {
		maxPt=2*sizePt+sizeof(path_lineto_w);
		pts=(char*)realloc(pts,maxPt);
	}
	if ( mvto == false && nbPt > 0 && ((path_lineto_w*)pts)[nbPt-1].x == ix && ((path_lineto_w*)pts)[nbPt-1].y == iy ) return -1;
	int   n=nbPt++;
	sizePt=nextSize;
	if ( mvto ) ((path_lineto_w*)pts)[n].isMoveTo=0; else ((path_lineto_w*)pts)[n].isMoveTo=-1;
	((path_lineto_w*)pts)[n].x=ix;
	((path_lineto_w*)pts)[n].y=iy;
	((path_lineto_w*)pts)[n].w=iw;
	return n;
}
int             Path::AddPoint(float ix,float iy,int ip,float it,bool mvto)
{
	if ( back ) {
		if ( weighted ) {
			return AddPoint(ix,iy,1.0,ip,it,mvto);
		}
	} else {
		return AddPoint(ix,iy,mvto);
	}
	int  nextSize=sizePt+sizeof(path_lineto_b);
	if ( nextSize > maxPt ) {
		maxPt=2*sizePt+sizeof(path_lineto_b);
		pts=(char*)realloc(pts,maxPt);
	}
	if ( mvto == false && nbPt > 0 && ((path_lineto_b*)pts)[nbPt-1].x == ix && ((path_lineto_b*)pts)[nbPt-1].y == iy ) return -1;
	int   n=nbPt++;
	sizePt=nextSize;
	if ( mvto ) ((path_lineto_b*)pts)[n].isMoveTo=0; else ((path_lineto_b*)pts)[n].isMoveTo=-1;
	((path_lineto_b*)pts)[n].x=ix;
	((path_lineto_b*)pts)[n].y=iy;
	((path_lineto_b*)pts)[n].piece=ip;
	((path_lineto_b*)pts)[n].t=it;
	return n;
}
int             Path::AddPoint(float ix,float iy,float iw,int ip,float it,bool mvto)
{
	if ( back ) {
		if ( weighted ) {
		} else {
			return AddPoint(ix,iy,ip,it,mvto);
		}
	} else {
		return AddPoint(ix,iy,iw,mvto);
	}
	int  nextSize=sizePt+sizeof(path_lineto_wb);
	if ( nextSize > maxPt ) {
		maxPt=2*sizePt+sizeof(path_lineto_wb);
		pts=(char*)realloc(pts,maxPt);
	}
	if ( mvto == false && nbPt > 0 && ((path_lineto_wb*)pts)[nbPt-1].x == ix && ((path_lineto_wb*)pts)[nbPt-1].y == iy ) return -1;
	int   n=nbPt++;
	sizePt=nextSize;
	if ( mvto ) ((path_lineto_wb*)pts)[n].isMoveTo=0; else ((path_lineto_wb*)pts)[n].isMoveTo=-1;
	((path_lineto_wb*)pts)[n].x=ix;
	((path_lineto_wb*)pts)[n].y=iy;
	((path_lineto_wb*)pts)[n].w=iw;
	((path_lineto_wb*)pts)[n].piece=ip;
	((path_lineto_wb*)pts)[n].t=it;
	return n;
}

int              Path::Winding(void)
{
	if ( nbPt <= 1 ) return 0;
	float   sum=0;
	if ( weighted ) {
		for (int i=0;i<nbPt-1;i++) {
			sum+=(((path_lineto_w*)pts)[i].x+((path_lineto_w*)pts)[i+1].x)*(((path_lineto_w*)pts)[i+1].y-((path_lineto_w*)pts)[i].y);
		}
		sum+=(((path_lineto_w*)pts)[nbPt-1].x+((path_lineto_w*)pts)[0].x)*(((path_lineto_w*)pts)[0].y-((path_lineto_w*)pts)[nbPt-1].y);
	} else {
		for (int i=0;i<nbPt-1;i++) {
			sum+=(((path_lineto*)pts)[i].x+((path_lineto*)pts)[i+1].x)*(((path_lineto*)pts)[i+1].y-((path_lineto*)pts)[i].y);
		}
		sum+=(((path_lineto*)pts)[nbPt-1].x+((path_lineto*)pts)[0].x)*(((path_lineto*)pts)[0].y-((path_lineto*)pts)[nbPt-1].y);
	}
	return (sum>0)?1:-1;
}
