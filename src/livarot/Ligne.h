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

/*
 * pixel coverage classes. the goal is to salvage exact coverage info in the sweepline performed by Scan() or QuickScan(), then
 * clean up a bit, convert floating point bounds to integer bounds, because pixel have integer bounds, and then raster runs of the type:
 *   position on the (pixel) line:                st         en
 *                                                |          |
 *   coverage value (0=empty, 1=full)            vst   ->   ven
 */

class IntLigne;
class BitLigne;

// a coverage portion with floating point boundaries
typedef struct float_ligne_run {
	float       st,en;
	float       vst,ven;
	float       pente;   // (ven-vst)/(en-st)
} float_ligne_run;

// temporary data for the FloatLigne()
// each float_ligne_bord is a boundary of some coverage
// the Flatten() function will extract non-overlapping runs and produce an array of float_ligne_run
// the float_ligne_bord are stored in an array, but linked like a boubly-linked list
// the idea behind that is that a given edge produces one float_ligne_bord at the beginning of Scan() and possibly another
// in AvanceEdge() and DestroyEdge(); but that second float_ligne_bord will not be far away in the list from the first,
// so it's faster to salvage the index of the first float_ligne_bord and try to insert the second from that salvaged position
typedef struct float_ligne_bord {
	float       pos;   // position of the boundary
	bool        start; // is the beginning of the coverage portion?
	float       val;   // amount of coverage (ie vst if start==true, and ven if start==false)
	float       pente; // (ven-vst)/(en-st)
//	float       delta;
	int         other; // index, in the array of float_ligne_bord, of the other boundary associated to this one
	int         prev,next; // not used
	int         s_prev,s_next; // indices of the previous and next bord in the doubly-linked list
  int         pend_ind;  // bords[i].pend_ind is the index of the float_ligne_bord that is the start of the
                         // coverage portion being scanned (in the Flatten() )  
  int         pend_inv;  // inverse of pend_ind, for faster handling of insertion/removal in the "pending" array
} float_ligne_bord;

class FloatLigne {
public:
  // array of coverage boundaries
	int           nbBord,maxBord;
	float_ligne_bord*   bords;

  // array of runs
	int           nbRun,maxRun;
	float_ligne_run*    runs;

  // unused
	int           firstAc,lastAc;
  // first and last boundaries in the doubly-linked list
	int           s_first,s_last;

	FloatLigne(void);
	~FloatLigne(void);

  // reset the line to  empty (boundaries and runs)
	void             Reset(void);
  // add a coverage portion
  // guess is the position from where we should try to insert the first boundary, or -1 if we don't have a clue
	int              AddBord(float spos,float sval,float epos,float eval,int guess=-1);
	int              AddBord(float spos,float sval,float epos,float eval,float pente,int guess=-1);
  // minor difference: guess is the position from where we should try to insert the last boundary
	int              AddBordR(float spos,float sval,float epos,float eval,float pente,int guess=-1);
  // simply append boundaries at the end of the list, 'cause we know they are on the right
	int              AppendBord(float spos,float sval,float epos,float eval,float pente);
  // insertion primitive, for private use
	void             InsertBord(int no,float p,int guess);

  // extract a set of non-overlapping runs from the boundaries
  // it does so by scanning the boundaries left to right, maintaining a set of coverage portions currently being scanned
  // for each such portion, the function will add the index of its first boundary in an array; but instead of allocating
  // another array, it uses a field in float_ligne_bord: pend_ind
	void             Flatten(void);
//	void             FlattenB(void);

  // debug dump of the instance
	void             Affiche(void);

  // internal use only
	int              AddRun(float st,float en,float vst,float ven);
	int              AddRun(float st,float en,float vst,float ven,float pente);

  // operations on FloatLigne instances:
  // computes a mod b and stores the result in the present FloatLigne
	void             Booleen(FloatLigne* a,FloatLigne* b,BooleanOp mod);
  // clips the coverage runs to tresh
  // if addIt == false, it only leaves the parts that are not entirely under tresh
  // if addIt == true, it's the coverage clamped to tresh
	void             Max(FloatLigne* a,float tresh,bool addIt);
  // cuts the parts having less than tresh coverage
	void             Min(FloatLigne* a,float tresh,bool addIt);
  // cuts the coverage "a" in 2 parts: "over" will receive the parts where coverage > tresh, while 
  // the present FloatLigne will receive the parts where coverage <= tresh
	void             Split(FloatLigne* a,float tresh,FloatLigne* over);
  // extract the parts where coverage > tresh
	void             Over(FloatLigne* a,float tresh);
	
  // copy the runs from another coverage structure into this one
	void             Copy(IntLigne* a);
	void             Copy(FloatLigne* a);

  // private use
	void             Enqueue(int no);
	void             Dequeue(int no);
  // computes the sum of the coverages of the runs currently being scanned, of which there are "pending"
	float            RemainingValAt(float at,int pending);
  
  // sorting of the float_ligne_bord, for when we use a quicksort (not the case, right now)
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

  // miscanellous
	inline float     ValAt(float at,float ps,float pe,float vs,float ve) {
		return ((at-ps)*ve+(pe-at)*vs)/(pe-ps);
	};

};



/*
 * coverage with integer boundaries: what we want for actual rasterization
 * it contains the same stuff as FloatLigne, but technically only the Copy() functions are used
 */
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

  // copy another IntLigne
	void             Copy(IntLigne* a);
  // copy a FloatLigne's runs, ie compute non-overlapping runs with integer boundaries from a set
  // of runs with floating-point boundaries
	void             Copy(FloatLigne* a);
  // transform a line of bits into pixel coverage values.
  // this is where you go from supersampled data to alpha values
	void             Copy(BitLigne* a);
	void             Copy(int nbSub,BitLigne* *a); // nbSub is the number of BitLigne in the array "a"

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

  // rasterization.
  // the parameters have the same meaning as in the AlphaLigne class
	void             Raster(raster_info &dest,void* color,RasterInRunFunc worker);
};

#endif
