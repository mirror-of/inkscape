/*
 *  AlphaLigne.cpp
 *  nlivarot
 *
 *  Created by fred on Fri Jul 25 2003.
 *  public domain
 *
 */

#include "AlphaLigne.h"
//#include "Buffer.h"

#include <math.h>

/*int       min,max;
int       length;
uin16_t   *alpha;*/

AlphaLigne::AlphaLigne(int iMin,int iMax)
{
	min=iMin;
	max=iMax;
	if ( max < min+1 ) max=min+1;
	steps=NULL;
	nbStep=maxStep=0;
	before.x=min-1;
	before.delta=0;
	after.x=max+1;
	after.delta=0;
}
AlphaLigne::~AlphaLigne(void)
{
	if ( steps ) free(steps);
	steps=NULL;
	nbStep=maxStep=0;
}
void						 AlphaLigne::Affiche(void)
{
	printf("%i steps\n",nbStep);
	for (int i=0;i<nbStep;i++) {
		printf("(%i %f) ",steps[i].x,steps[i].delta);
	}
	printf("\n");
}


void             AlphaLigne::Reset(void)
{
	curMin=max;
	curMax=min;
	nbStep=0;
	before.x=min-1;
	before.delta=0;
	after.x=max+1;
	after.delta=0;
}
int              AlphaLigne::AddBord(float spos,float sval,float epos,float eval,float tPente)
{
//	printf("%f %f -> %f %f / %f\n",spos,sval,epos,eval,tPente);
	if ( sval == eval ) return 0;
	float  curStF=floorf(spos);
	float  curEnF=floorf(epos);
	int   curSt=(int)curStF;
	int   curEn=(int)curEnF;
	
	if ( curSt > max ) {
    if ( eval < sval ) curMax=max;
    return 0; // en dehors des limites (attention a ne pas faire ca avec curEn)
  }
	if ( curSt < curMin ) curMin=curSt;
	if ( ceilf(epos) > curMax ) curMax=(int)ceilf(epos);
	
	if ( curMax > max ) curMax=max;
	if ( curMin < min ) curMin=min;
	
	float  needed=eval-sval;
	float    needC=/*(int)ldexpf(*/needed/*,24)*/;
	
	if ( curEn < min ) {
    before.delta+=needC;
    return 0; // en dehors des limites (attention a ne pas faire ca avec curEn)
	}
	if ( curSt == curEn ) {
		if ( curSt+1 < min ) {
			before.delta+=needC;
		} else {
			if ( nbStep+2 >= maxStep ) {
				maxStep=2*nbStep+2;
				steps=(alpha_step*)realloc(steps,maxStep*sizeof(alpha_step));
			}
			float  stC=/*(int)ldexpf(*/(eval-sval)*(0.5*(epos-spos)+curStF+1-epos)/*,24)*/;
			steps[nbStep].x=curSt;
			steps[nbStep].delta=stC;
			nbStep++;
			steps[nbStep].x=curSt+1;
			steps[nbStep].delta=needC-stC; //  au final, on a toujours le bon delta, meme avec une arete completement verticale
			nbStep++;
		}
	} else if ( curEn == curSt+1 ) {
		if ( curSt+2 < min ) {
			before.delta+=needC;
		} else {
			if ( nbStep+3 >= maxStep ) {
				maxStep=2*nbStep+3;
				steps=(alpha_step*)realloc(steps,maxStep*sizeof(alpha_step));
			}
			float  stC=/*(int)ldexpf(*/0.5*tPente*(curEnF-spos)*(curEnF-spos)/*,24)*/;
			float  enC=/*(int)ldexpf(*/tPente-0.5*tPente*((spos-curStF)*(spos-curStF)+(curEnF+1.0-epos)*(curEnF+1.0-epos))/*,24)*/;
			steps[nbStep].x=curSt;
			steps[nbStep].delta=stC;
			nbStep++;
			steps[nbStep].x=curEn;
			steps[nbStep].delta=enC;
			nbStep++;
			steps[nbStep].x=curEn+1;
			steps[nbStep].delta=needC-stC-enC;
			nbStep++;
		}
	} else {
		float  stC=/*(int)ldexpf(*/0.5*tPente*(curStF+1-spos)*(curStF+1-spos)/*,24)*/;
		float  stFC=/*(int)ldexpf(*/tPente-0.5*tPente*(spos-curStF)*(spos-curStF)/*,24)*/;
		float  enC=/*(int)ldexpf(*/tPente-0.5*tPente*(curEnF+1.0-epos)*(curEnF+1.0-epos)/*,24)*/;
		float  miC=/*(int)ldexpf(*/tPente/*,24)*/;
		if ( curSt < min ) {
			if ( curEn > max ) {
				if ( nbStep+(max-min) >= maxStep ) {
					maxStep=2*nbStep+(max-min);
					steps=(alpha_step*)realloc(steps,maxStep*sizeof(alpha_step));
				}
				float  bfd=min-curSt-1;
				bfd*=miC;
				before.delta+=stC+bfd;
				for (int i=min;i<max;i++) {
					steps[nbStep].x=i;
					steps[nbStep].delta=miC;
					nbStep++;
				}
			} else {
				if ( nbStep+(curEn-min)+2 >= maxStep ) {
					maxStep=2*nbStep+(curEn-min)+2;
					steps=(alpha_step*)realloc(steps,maxStep*sizeof(alpha_step));
				}
				float  bfd=min-curSt-1;
				bfd*=miC;
				before.delta+=stC+bfd;
				for (int i=min;i<curEn;i++) {
					steps[nbStep].x=i;
					steps[nbStep].delta=miC;
					nbStep++;
				}
				steps[nbStep].x=curEn;
				steps[nbStep].delta=enC;
				nbStep++;
				steps[nbStep].x=curEn+1;
				steps[nbStep].delta=needC-stC-stFC-enC-(curEn-curSt-2)*miC;
				nbStep++;
			}
		} else {
			if ( curEn > max ) {
				if ( nbStep+3+(max-curSt) >= maxStep ) {
					maxStep=2*nbStep+3+(curEn-curSt);
					steps=(alpha_step*)realloc(steps,maxStep*sizeof(alpha_step));
				}
				steps[nbStep].x=curSt;
				steps[nbStep].delta=stC;
				nbStep++;
				steps[nbStep].x=curSt+1;
				steps[nbStep].delta=stFC;
				nbStep++;
				for (int i=curSt+2;i<max;i++) {
					steps[nbStep].x=i;
					steps[nbStep].delta=miC;
					nbStep++;
				}
			} else {
				if ( nbStep+3+(curEn-curSt) >= maxStep ) {
					maxStep=2*nbStep+3+(curEn-curSt);
					steps=(alpha_step*)realloc(steps,maxStep*sizeof(alpha_step));
				}
				steps[nbStep].x=curSt;
				steps[nbStep].delta=stC;
				nbStep++;
				steps[nbStep].x=curSt+1;
				steps[nbStep].delta=stFC;
				nbStep++;
				for (int i=curSt+2;i<curEn;i++) {
					steps[nbStep].x=i;
					steps[nbStep].delta=miC;
					nbStep++;
				}
				steps[nbStep].x=curEn;
				steps[nbStep].delta=enC;
				nbStep++;
				steps[nbStep].x=curEn+1;
				steps[nbStep].delta=needC-stC-stFC-enC-(curEn-curSt-2)*miC;
				nbStep++;
			}
		}
	}

	return 0;
}
int              AlphaLigne::AddBord(float spos,float sval,float epos,float eval)
{
	// pas de pente dans ce cas; on ajoute le delta au premier pixel
	float   tPente=(eval-sval);

	float curStF=floorf(spos);
	float curEnF=floorf(epos);
	int   curSt=(int)curStF;
	int   curEn=(int)curEnF;
	
	if ( curSt > max ) {
    if ( eval < sval ) curMax=max;
    return 0; // en dehors des limites (attention a ne pas faire ca avec curEn)
  }
	if ( curEn < min ) {
    before.delta+=eval-sval;
    return 0; // en dehors des limites (attention a ne pas faire ca avec curEn)
	}
	
	if ( curSt < curMin ) curMin=curSt;
//	int   curEn=(int)curEnF;
	if ( ceilf(epos) > curMax-1 ) curMax=1+(int)ceilf(epos);
	if ( curSt < min ) {
		before.delta+=eval-sval;
	} else {
		AddRun(curSt,/*(int)ldexpf(*/(((float)(curSt+1))-spos)*tPente/*,24)*/);
		AddRun(curSt+1,/*(int)ldexpf(*/(spos-((float)(curSt)))*tPente/*,24)*/);
	}
	return 0;
}

void             AlphaLigne::Flatten(void)
{
	if ( nbStep > 0 ) qsort(steps,nbStep,sizeof(alpha_step),CmpStep);
}
void             AlphaLigne::AddRun(int st,float pente)
{
	if ( nbStep >= maxStep ) {
		maxStep=2*nbStep+1;
		steps=(alpha_step*)realloc(steps,maxStep*sizeof(alpha_step));
	}
	int nStep=nbStep++;
	steps[nStep].x=st;
	steps[nStep].delta=pente;
}

void             AlphaLigne::Raster(raster_info &dest,void* color,RasterInRunFunc worker)
{
	if ( curMax <= curMin ) return;
	if ( dest.endPix <= curMin || dest.startPix >= curMax ) return;

	int    nMin=curMin,nMax=curMax;
	float  alpSum=before.delta;
	int    curStep=0;
  
  while ( curStep < nbStep && steps[curStep].x < nMin ) {
    alpSum+=steps[curStep].delta;
    curStep++;
  }
	if ( nMin < dest.startPix ) {
		for (;( curStep < nbStep && steps[curStep].x < dest.startPix) ;curStep++) alpSum+=steps[curStep].delta;
		nMin=dest.startPix;
	}
	if ( nMax > dest.endPix ) nMax=dest.endPix;

	int       curPos=dest.startPix;
	for (;curStep<nbStep;curStep++) {
		if ( alpSum > 0 && steps[curStep].x > curPos ) {
			int  nst=curPos,nen=steps[curStep].x;
//Buffer::RasterRun(dest,color,nst,alpSum,nen,alpSum);
      (worker)(dest,color,nst,alpSum,nen,alpSum);
		}
		alpSum+=steps[curStep].delta;
		curPos=steps[curStep].x;
		if ( curPos >= nMax ) break;
	}
	if ( alpSum > 0 && curPos < nMax ) {
		int  nst=curPos,nen=max;
    (worker)(dest,color,nst,alpSum,nen,alpSum);
//Buffer::RasterRun(dest,color,nst,alpSum,nen,alpSum);
	}
}
