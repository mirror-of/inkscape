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
void
Shape::Plot (double ix, double iy, double ir, double mx, double my, bool doPoint,
	     bool edgesNo, bool pointsNo, bool doDir,char* fileName)
{
  FILE*  outFile=fopen(fileName,"w+");
//  fprintf(outFile,"\n\n\n");
  fprintf(outFile,"<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\"?>\n");
  fprintf(outFile,"<!DOCTYPE svg PUBLIC \"-//W3C//DTD SVG 1.0//EN\"\n");
  fprintf(outFile,"\"http://www.w3.org/TR/2001/REC-SVG-20010904/DTD/svg10.dtd\">\n");
  fprintf(outFile,"<svg\n");
  fprintf(outFile,"   id=\"svg1\"\n");
  fprintf(outFile,"   sodipodi:version=\"0.32\"\n");
  fprintf(outFile,"   inkscape:version=\"0.38cvs\"\n");
  fprintf(outFile,"   xmlns=\"http://www.w3.org/2000/svg\"\n");
  fprintf(outFile,"   xmlns:sodipodi=\"http://sodipodi.sourceforge.net/DTD/sodipodi-0.dtd\"\n");
  fprintf(outFile,"   xmlns:inkscape=\"http://www.inkscape.org/namespaces/inkscape\"\n");
  fprintf(outFile,"   xmlns:xlink=\"http://www.w3.org/1999/xlink\"\n");
  fprintf(outFile,"   width=\"210mm\"\n");
  fprintf(outFile,"   height=\"297mm\"\n");
  fprintf(outFile,"   sodipodi:docbase=\"/Volumes/Sancho/inkscapecvs\"\n");
  fprintf(outFile,"   sodipodi:docname=\"/Volumes/Sancho/inkscapecvs/modele.svg\">\n");
  fprintf(outFile,"  <defs\n");
  fprintf(outFile,"     id=\"defs3\" />\n");
  fprintf(outFile,"  <sodipodi:namedview\n");
  fprintf(outFile,"     id=\"base\"\n");
  fprintf(outFile,"     pagecolor=\"#ffffff\"\n");
  fprintf(outFile,"     bordercolor=\"#666666\"\n");
  fprintf(outFile,"     borderopacity=\"1.0\"\n");
  fprintf(outFile,"     inkscape:pageopacity=\"0.0\"\n");
  fprintf(outFile,"     inkscape:pageshadow=\"2\"\n");
  fprintf(outFile,"     inkscape:zoom=\"0.43415836\"\n");
  fprintf(outFile,"     inkscape:cx=\"305.25952637\"\n");
  fprintf(outFile,"     inkscape:cy=\"417.84947271\"\n");
  fprintf(outFile,"     inkscape:window-width=\"640\"\n");
  fprintf(outFile,"     inkscape:window-height=\"496\"\n");
  fprintf(outFile,"     inkscape:window-x=\"20\"\n");
  fprintf(outFile,"     inkscape:window-y=\"42\" />\n");
  
	if ( doPoint ) {
		for (int i=0;i<nbPt;i++) {
			double   ph=(pts[i].x[0]-ix)*ir+mx;
			double   pv=(pts[i].x[1]-iy)*ir+my;
      fprintf(outFile,"     <circle cx=\"%f\" cy=\"%f\" r=\"5\" fill=\"none\" stroke=\"red\" stroke-width=\"0.25\" />\n",ph,pv);
    }
	}
  if ( pointsNo ) {
		for (int i=0;i<nbPt;i++) {
			double   ph=(pts[i].x[0]-ix)*ir+mx;
			double   pv=(pts[i].x[1]-iy)*ir+my;
      fprintf(outFile,"     <text x=\"%f\" y=\"%f\" font-family=\"Monaco\" font-size=\"5\" fill=\"blue\" >\n",ph-2,pv+1);
      fprintf(outFile,"%i\n",i);
      fprintf(outFile,"     </text>\n");
    }
  }
	{
		for (int i=0;i<nbAr;i++) {
			int     stP=aretes[i].st;
			int     enP=aretes[i].en;
			if ( stP < 0 || enP < 0 ) continue;
			double   sh=(pts[stP].x[0]-ix)*ir+mx;
			double   sv=(pts[stP].x[1]-iy)*ir+my;
			double   eh=(pts[enP].x[0]-ix)*ir+mx;
			double   ev=(pts[enP].x[1]-iy)*ir+my;
			if ( doDir ) {
				double   endh=(9*eh+1*sh)/10;
				double   endv=(9*ev+1*sv)/10;
        fprintf(outFile,"     <line x1=\"%f\" y1=\"%f\" x2=\"%f\" y2=\"%f\" stroke=\"black\" stroke-width=\"0.5\" />\n",sh,sv,endh,endv);
			} else {
        fprintf(outFile,"     <line x1=\"%f\" y1=\"%f\" x2=\"%f\" y2=\"%f\" stroke=\"black\" stroke-width=\"0.5\" />\n",sh,sv,eh,ev);
			}
		}
	}
  if ( edgesNo ) {
		for (int i=0;i<nbAr;i++) {
			int     stP=aretes[i].st;
			int     enP=aretes[i].en;
			if ( stP < 0 || enP < 0 ) continue;
			double   sh=(pts[stP].x[0]-ix)*ir+mx;
			double   sv=(pts[stP].x[1]-iy)*ir+my;
			double   eh=(pts[enP].x[0]-ix)*ir+mx;
			double   ev=(pts[enP].x[1]-iy)*ir+my;
      fprintf(outFile,"     <text x=\"%f\" y=\"%f\" font-family=\"Monaco\" font-size=\"5\" fill=\"blue\" >\n",(sh+eh)/2+2,(sv+ev)/2);
      fprintf(outFile,"%i\n",i);
      fprintf(outFile,"     </text>\n");
		}
  }
  
  fprintf(outFile,"</svg>\n");
  fclose(outFile);

}
