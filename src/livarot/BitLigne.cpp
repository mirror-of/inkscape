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
	stBit=(int)floorf(((float)st)*invScale); // round to pixel boundaries in the canvas
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
	
  // separation of full and not entirely full bits is a bit useless
  // the goal is to obtain a set of bits that are "on the edges" of the polygon, so that their coverage
  // will be 1/2 on the average. in practice it's useless for anything but the even-odd fill rule
	int   ffBit,lfBit; // first and last bit of the portion of the line that is entirely covered
	ffBit=(int)(ceilf(invScale*spos));
	lfBit=(int)(floorf(invScale*epos));
	int   fpBit,lpBit; // first and last bit of the portion of the line that is not entirely but partially covered
	fpBit=(int)(floorf(invScale*spos));
	lpBit=(int)(ceilf(invScale*epos));
  
  // update curMin and curMax to reflect the start and end pixel that need to be updated on the canvas
	if ( spos < curMin ) curMin=(int)spos;
	if ( ceilf(epos) > curMax ) curMax=(int)ceilf(epos);

  // clamp to the line
	if ( ffBit < stBit ) ffBit=stBit;
	if ( ffBit > enBit ) ffBit=enBit;
	if ( lfBit < stBit ) lfBit=stBit;
	if ( lfBit > enBit ) lfBit=enBit;
	if ( fpBit < stBit ) fpBit=stBit;
	if ( fpBit > enBit ) fpBit=enBit;
	if ( lpBit < stBit ) lpBit=stBit;
	if ( lpBit > enBit ) lpBit=enBit;
  
  // offset to get actual bit position in the array
	ffBit-=stBit;
	lfBit-=stBit;
	fpBit-=stBit;
	lpBit-=stBit;

  // get the end and start indices of the elements of fullB and partB that will receives coverage
	int   ffPos=ffBit>>5;
	int   lfPos=lfBit>>5;
	int   fpPos=fpBit>>5;
	int   lpPos=lpBit>>5;
  // get bit numbers in the last and first changed elements of the fullB and partB arrays
	int   ffRem=ffBit&0x0000001F;
	int   lfRem=lfBit&0x0000001F;
	int   fpRem=fpBit&0x0000001F;
	int   lpRem=lpBit&0x0000001F;
  // add the coverage
  // note that the "full" bits are always a subset of the "not empty" bits, ie of the partial bits
  // the function is a bit lame: since there is at most one bit that is partial but not full, or no full bit,
  // it does 2 times the optimal amount of work when the coverage is full. but i'm too lazy to change that...
	if ( fpPos == lpPos ) { // only one element of the arrays is modified
    // compute the vector of changed bits in the element
		uint32_t  add=0xFFFFFFFF;
		add>>=32-lpRem;
		add<<=32-lpRem;
		add<<=fpRem;
		add>>=fpRem;
    // and put it in the line
    fullB[fpPos]&=~(add); // partial is exclusive from full, so partial bits are removed from fullB
    partB[fpPos]|=add;    // and added to partB
    if ( full ) { // if the coverage is full, add the vector of full bits
      add=0xFFFFFFFF;
      add>>=32-lfRem;
      add<<=32-lfRem;
      add<<=ffRem;
      add>>=ffRem;
			fullB[ffPos]|=add;
			partB[ffPos]&=~(add);
    }
	} else {
    // first and last elements are differents, so add what appropriate to each
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
    
    // and fill what's in between with partial bits
    memset(fullB+(fpPos+1),0x00,(lpPos-fpPos-1)*sizeof(uint32_t));
    memset(partB+(fpPos+1),0xFF,(lpPos-fpPos-1)*sizeof(uint32_t));

		if ( full ) { // is the coverage is full, do your magic
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

