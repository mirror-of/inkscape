/*
 *  BitLigne.cpp
 *  nlivarot
 *
 *  Created by fred on Wed Jul 23 2003.
 *  public domain
 *
 */

#include "BitLigne.h"

#include <math.h>

BitLigne::BitLigne(int ist,int ien,float iScale)
{
  scale=iScale;
  invScale=1/iScale;
	st=ist;
	en=ien;
	if ( en <= st ) en=st+1;
	stBit=(int)floorf(((float)st)*invScale);
	enBit=(int)ceilf(((float)en)*invScale);
	int  nbBit=enBit-stBit;
	if ( nbBit&0x0000001F ) {
		nbInt=nbBit/32+1;
	} else {
		nbInt=nbBit/32;
	}
	fullB=(uint32_t*)malloc(nbInt*sizeof(uint32_t));
	partB=(uint32_t*)malloc(nbInt*sizeof(uint32_t));

	curMin=en;
	curMax=st;
}
BitLigne::~BitLigne(void)
{
	free(fullB);
	free(partB);
}

void             BitLigne::Reset(void)
{
	curMin=en;
	curMax=st;
	memset(fullB,0,nbInt*sizeof(uint32_t));
	memset(partB,0,nbInt*sizeof(uint32_t));
}
int              BitLigne::AddBord(float spos,float epos,bool full)
{
	if ( spos >= epos ) return 0;
	
	int   ffBit,lfBit; // debut et fin de la zone pleine
	ffBit=(int)(ceilf(invScale*spos));
	lfBit=(int)(floorf(invScale*epos));
	int   fpBit,lpBit; // debut  et fin de la zone partielle
	fpBit=(int)(floorf(invScale*spos));
	lpBit=(int)(ceilf(invScale*epos));
  
	if ( spos < curMin ) curMin=(int)spos;
	if ( ceilf(epos) > curMax ) curMax=(int)ceilf(epos);

	if ( ffBit < stBit ) ffBit=stBit;
	if ( ffBit > enBit ) ffBit=enBit;
	if ( lfBit < stBit ) lfBit=stBit;
	if ( lfBit > enBit ) lfBit=enBit;
	if ( fpBit < stBit ) fpBit=stBit;
	if ( fpBit > enBit ) fpBit=enBit;
	if ( lpBit < stBit ) lpBit=stBit;
	if ( lpBit > enBit ) lpBit=enBit;
  
	ffBit-=stBit;
	lfBit-=stBit;
	fpBit-=stBit;
	lpBit-=stBit;

	int   ffRem=ffBit&0x0000001F;
	int   lfRem=lfBit&0x0000001F;
	int   ffPos=ffBit>>5;
	int   lfPos=lfBit>>5;
	int   fpRem=fpBit&0x0000001F;
	int   lpRem=lpBit&0x0000001F;
	int   fpPos=fpBit>>5;
	int   lpPos=lpBit>>5;
	if ( fpPos == lpPos ) {
		uint32_t  add=0xFFFFFFFF;
		add>>=32-lpRem;
		add<<=32-lpRem;
		add<<=fpRem;
		add>>=fpRem;
    fullB[fpPos]&=~(add);
    partB[fpPos]|=add;
    if ( full ) { // full est tjs contenu dans partiel
      add=0xFFFFFFFF;
      add>>=32-lfRem;
      add<<=32-lfRem;
      add<<=ffRem;
      add>>=ffRem;
			fullB[ffPos]|=add;
			partB[ffPos]&=~(add);
    }
	} else {
		uint32_t  add=0xFFFFFFFF;
		add<<=fpRem;
		add>>=fpRem;
    fullB[fpPos]&=~(add);
    partB[fpPos]|=add;
		
		add=0xFFFFFFFF;
		add>>=32-lpRem;
		add<<=32-lpRem;
    fullB[lpPos]&=~(add);
    partB[lpPos]|=add;

    memset(fullB+(fpPos+1),0x00,(lpPos-fpPos-1)*sizeof(uint32_t));
    memset(partB+(fpPos+1),0xFF,(lpPos-fpPos-1)*sizeof(uint32_t));

		if ( full ) {
      add=0xFFFFFFFF;
      add<<=ffRem;
      add>>=ffRem;
      fullB[ffPos]|=add;
      partB[ffPos]&=~add;
      
      add=0xFFFFFFFF;
      add>>=32-lfRem;
      add<<=32-lfRem;
      fullB[lfPos]|=add;
      partB[lfPos]&=~add;

			memset(fullB+(ffPos+1),0xFF,(lfPos-ffPos-1)*sizeof(uint32_t));
			memset(partB+(ffPos+1),0x00,(lfPos-ffPos-1)*sizeof(uint32_t));
		}
	}
	return 0;
}


void             BitLigne::Affiche(void)
{
	for (int i=0;i<nbInt;i++) printf(" %.8x",fullB[i]);
	printf("\n");
	for (int i=0;i<nbInt;i++) printf(" %.8x",partB[i]);
	printf("\n\n");
}

