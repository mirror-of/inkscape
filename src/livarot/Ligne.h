/*
 *  Ligne.h
 *  nlivarot
 *
 *  Created by fred on Fri Jul 18 2003.
 *  public domain
 *
 */

#ifndef my_ligne
#define my_ligne

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "LivarotDefs.h"

class IntLigne;
class BitLigne;

typedef struct float_ligne_run {
	float       st,en;
	float       vst,ven;
	float       pente;
} float_ligne_run;

typedef struct float_ligne_bord {
	float       pos;
	bool        start;
	float       val;
	float       pente;
//	float       delta;
	int         other;
	int         prev,next;
	int         s_prev,s_next;
  int         pend_ind;
  int         pend_inv;
} float_ligne_bord;

class FloatLigne {
public:
	int           nbBord,maxBord;
	float_ligne_bord*   bords;

	int           nbRun,maxRun;
	float_ligne_run*    runs;

	int           firstAc,lastAc;
	int           s_first,s_last;

	FloatLigne(void);
	~FloatLigne(void);

	void             Reset(void);
	int              AddBord(float spos,float sval,float epos,float eval,int guess=-1);
	int              AddBord(float spos,float sval,float epos,float eval,float pente,int guess=-1);
	int              AddBordR(float spos,float sval,float epos,float eval,float pente,int guess=-1);
	int              AppendBord(float spos,float sval,float epos,float eval,float pente);
	void             InsertBord(int no,float p,int guess);

	void             Flatten(void);
//	void             FlattenB(void);

	void             Affiche(void);

	int              AddRun(float st,float en,float vst,float ven);
	int              AddRun(float st,float en,float vst,float ven,float pente);

	void             Booleen(FloatLigne* a,FloatLigne* b,BooleanOp mod);
	void             Max(FloatLigne* a,float tresh,bool addIt);
	void             Min(FloatLigne* a,float tresh,bool addIt);
	void             Split(FloatLigne* a,float tresh,FloatLigne* over);
	void             Over(FloatLigne* a,float tresh);
	
	void             Copy(IntLigne* a);
	void             Copy(FloatLigne* a);

	void             Enqueue(int no);
	void             Dequeue(int no);
	float            RemainingValAt(float at,int pending);
  
  void             SwapBords(int a,int b);
  void             SwapBords(int a,int b,int c);
  void             SortBords(int s,int e);

	static int       CmpBord(const void * p1, const void * p2) {
		float_ligne_bord* d1=(float_ligne_bord*)p1;
		float_ligne_bord* d2=(float_ligne_bord*)p2;
		if ( d1->pos == d2->pos ) {
			if ( d1->start && !(d2->start) ) return 1;
			if ( !(d1->start) && d2->start ) return -1;
			return 0;
		}
		return (( d1->pos < d2->pos )?-1:1);
	};

	inline float     ValAt(float at,float ps,float pe,float vs,float ve) {
		return ((at-ps)*ve+(pe-at)*vs)/(pe-ps);
	};

};



typedef struct int_ligne_run {
	int         st,en;
	float       vst,ven;
} int_ligne_run;

typedef struct int_ligne_bord {
	int         pos;
	bool        start;
	float       val;
	int         other;
	int         prev,next;
} int_ligne_bord;

class IntLigne {
public:
	int           nbBord,maxBord;
	int_ligne_bord*   bords;

	int           nbRun,maxRun;
	int_ligne_run*    runs;

	int           firstAc,lastAc;

	IntLigne(void);
	~IntLigne(void);

	void             Reset(void);
	int              AddBord(int spos,float sval,int epos,float eval);

	void             Flatten(void);

	void             Affiche(void);

	int              AddRun(int st,int en,float vst,float ven);

	void             Booleen(IntLigne* a,IntLigne* b,BooleanOp mod);

	void             Copy(IntLigne* a);
	void             Copy(FloatLigne* a);
	void             Copy(BitLigne* a);
	void             Copy(int nbSub,BitLigne* *a);

	void             Enqueue(int no);
	void             Dequeue(int no);
	float            RemainingValAt(int at);

	static int       CmpBord(const void * p1, const void * p2) {
		int_ligne_bord* d1=(int_ligne_bord*)p1;
		int_ligne_bord* d2=(int_ligne_bord*)p2;
		if ( d1->pos == d2->pos ) {
			if ( d1->start && !(d2->start) ) return 1;
			if ( !(d1->start) && d2->start ) return -1;
			return 0;
		}
		return (( d1->pos < d2->pos )?-1:1);
	};

	inline float     ValAt(int at,int ps,int pe,float vs,float ve) {
		return ((at-ps)*ve+(pe-at)*vs)/(pe-ps);
	};

	void             Raster(raster_info &dest,void* color,RasterInRunFunc worker);
};

#endif
