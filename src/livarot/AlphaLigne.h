/*
 *  AlphaLigne.h
 *  nlivarot
 *
 *  Created by fred on Fri Jul 25 2003.
 *  public domain
 *
 */

#ifndef my_alpha_ligne
#define my_alpha_ligne

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
//#include <iostream.h>

#include "LivarotDefs.h"

typedef struct alpha_step {
	int           x;
	float         delta;
} alpha_step;

class AlphaLigne {
public:
	int          min,max;
	int          length;

	alpha_step   before,after;
	alpha_step*  steps;
	int          nbStep,maxStep;
	
	int          curMin,curMax;

	AlphaLigne(int iMin,int iMax);
	~AlphaLigne(void);

	void             Reset(void);
	int              AddBord(float spos,float sval,float epos,float eval,float iPente);
	int              AddBord(float spos,float sval,float epos,float eval);

	void             Flatten(void);
	
	void						 Affiche(void);

	void             AddRun(int st,float pente);

	void             Raster(raster_info &dest,void* color,RasterInRunFunc worker);
	
	static int       CmpStep(const void * p1, const void * p2) {
		alpha_step* d1=(alpha_step*)p1;
		alpha_step* d2=(alpha_step*)p2;
		return  d1->x - d2->x ;
	};
};


#endif

