/*
 *  BitLigne.h
 *  nlivarot
 *
 *  Created by fred on Wed Jul 23 2003.
 *  public domain
 *
 */

#ifndef my_bit_ligne
#define my_bit_ligne

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "LivarotDefs.h"


class BitLigne {
public:
	int           st,en;
	int           stBit,enBit;
	int           nbInt;
	uint32_t*     fullB;
	uint32_t*     partB;

	int           curMin,curMax;
  float         scale,invScale;

	BitLigne(int ist,int ien,float iScale=0.25);
	~BitLigne(void);

	void             Reset(void);
	
	int              AddBord(float spos,float epos,bool full);

	void             Affiche(void);

};

#endif


