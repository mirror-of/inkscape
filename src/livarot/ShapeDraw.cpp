/*
 *  ShapeDraw.cpp
 *  nlivarot
 *
 *  Created by fred on Mon Jun 16 2003.
 *
 */

#include "Shape.h"
//#include <ApplicationServices/ApplicationServices.h>

// debug routine for vizualizing the polygons
void              Shape::Plot(float ix,float iy,float ir,float mx,float my,bool doPoint,bool edgesNo,bool pointsNo,bool doDir)
{
/*	Rect   r;
	::SetRect(&r,0,0,800,800);
	::EraseRect(&r);

	int i;
	if ( doPoint ) {
		RGBColor  color;
		color.red=65535;
		color.green=0;
		color.blue=0;
		::RGBForeColor(&color);
		for (i=0;i<nbPt;i++) {
			float   ph=(pts[i].x-ix)*ir+mx;
			float   pv=(pts[i].y-iy)*ir+my;
			int      iph=(int)ph,ipv=(int)pv;
			Rect r;
			::SetRect(&r,iph-1,ipv-1,iph+1,ipv+1);
			::PaintRect(&r);
		}
		color.red=0;
		color.green=0;
		color.blue=0;
		::RGBForeColor(&color);
	}
	{
		for (i=0;i<nbAr;i++) {
			int     stP=aretes[i].st;
			int     enP=aretes[i].en;
			if ( stP < 0 || enP < 0 ) continue;
			float   sh=(pts[stP].x-ix)*ir+mx;
			float   sv=(pts[stP].y-iy)*ir+my;
			float   eh=(pts[enP].x-ix)*ir+mx;
			float   ev=(pts[enP].y-iy)*ir+my;
			int      ish=(int)sh,isv=(int)sv,ieh=(int)eh,iev=(int)ev;
			::MoveTo(ish,isv);
			if ( doDir ) {
				float   endh=(9*eh+1*sh)/10;
				float   endv=(9*ev+1*sv)/10;
				int     iendh=(int)endh,iendv=(int)endv;
				::LineTo(iendh,iendv);
			} else {
				::LineTo(ieh,iev);
			}
			if ( edgesNo ) {
				::MoveTo((ish+ieh)/2+2,(isv+iev)/2);
				char  tempSt[256];
				if ( HasSweepDestData() ) {
					sprintf(tempSt+1,"%d %d %d %d",i,swdData[i].leW,swdData[i].riW,swdData[i].ind);
				} else {
					sprintf(tempSt+1,"%d",i);
				}
				tempSt[0]=strlen(tempSt+1);
				::DrawString((unsigned char*)tempSt);
			}
		}
	}
	if ( pointsNo ) {
		for (i=0;i<nbPt;i++) {
			float   ph=(pts[i].x-ix)*ir+mx;
			float   pv=(pts[i].y-iy)*ir+my;
			int      iph=(int)ph,ipv=(int)pv;
			::MoveTo(iph+2,ipv);
			 char  tempSt[256];
			 if ( HasPointsData() ) {
				 sprintf(tempSt+1,"%i %i",i,pData[i].askForWindingB);
			 } else {
				 sprintf(tempSt+1,"%i ",i);
			 }
			tempSt[0]=strlen(tempSt+1);
			::DrawString((unsigned char*)tempSt);
		}
	}*/

}
