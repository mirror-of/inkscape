/*
 *  Ligne.cpp
 *  nlivarot
 *
 *  Created by fred on Fri Jul 18 2003.
 *  public domain
 *
 */

#include "Ligne.h"
//#include "Buffer.h"
#include "BitLigne.h"

#include <math.h>



//int   showCopy=0;
//#define faster_flatten 1

FloatLigne::FloatLigne(void)
{
	nbBord=maxBord=0;
	bords=NULL;

	nbRun=maxRun=0;
	runs=NULL;

	firstAc=lastAc=-1;
	s_first=s_last=-1;
}
FloatLigne::~FloatLigne(void)
{
	if ( maxBord > 0 ) {
		free(bords);
		nbBord=maxBord=0;
		bords=NULL;
	}
	if ( maxRun > 0 ) {
		free(runs);
		nbRun=maxRun=0;
		runs=NULL;
	}
}

void             FloatLigne::Reset(void)
{
	nbBord=0;
	nbRun=0;
	firstAc=lastAc=-1;
	s_first=s_last=-1;
}
int              FloatLigne::AddBord(float spos,float sval,float epos,float eval,int guess)
{
//  if ( showCopy ) printf("b= %f %f -> %f %f \n",spos,sval,epos,eval);
	if ( spos >= epos ) return -1;
  // allocate the boundaries in the array
	if ( nbBord+1 >= maxBord ) {
		maxBord=2*nbBord+2;
		bords=(float_ligne_bord*)realloc(bords,maxBord*sizeof(float_ligne_bord));
	}
  float pente=(eval-sval)/(epos-spos);
#ifdef faster_flatten
  if ( fabsf(epos-spos) < 0.001 || fabsf(pente) > 1000 ) {
    return;
    epos=spos;
    pente=0;
  }
#endif
  
  if ( guess >= nbBord ) guess=-1;
  
  // add the left boundary
	int n=nbBord++;
	bords[n].pos=spos;
	bords[n].val=sval;
	bords[n].start=true;
	bords[n].other=n+1;
	bords[n].pente=pente;
	bords[n].prev=bords[n].next=-1;
	bords[n].s_prev=bords[n].s_next=-1;
//	bords[n].delta=sval-eval;

  // insert it in the doubly-linked list
	InsertBord(n,spos,guess);

  // add the right boundary
	n=nbBord++;
	bords[n].pos=epos;
	bords[n].val=eval;
	bords[n].start=false;
	bords[n].other=n-1;
	bords[n].pente=bords[n-1].pente;
	bords[n].prev=bords[n].next=-1;
	bords[n].s_prev=bords[n].s_next=-1;
//	bords[n].delta=eval-sval;

  // insert it in the doubly-linked list, knowing that boundary at index n-1 is not too far before me
	InsertBord(n,epos,n-1);
  	
	return n;
}
int              FloatLigne::AddBord(float spos,float sval,float epos,float eval,float pente,int guess)
{
//  if ( showCopy ) printf("b= %f %f -> %f %f \n",spos,sval,epos,eval);
	if ( spos >= epos ) return -1;
	if ( nbBord+1 >= maxBord ) {
		maxBord=2*nbBord+2;
		bords=(float_ligne_bord*)realloc(bords,maxBord*sizeof(float_ligne_bord));
	}
#ifdef faster_flatten
  if ( fabsf(epos-spos) < 0.001 || fabsf(pente) > 1000 ) {
    return;
    epos=spos;
    pente=0;
  }
#endif
  if ( guess >= nbBord ) guess=-1;
  
  int n=nbBord++;
	bords[n].pos=spos;
	bords[n].val=sval;
	bords[n].start=true;
	bords[n].other=n+1;
	bords[n].pente=pente;
	bords[n].prev=bords[n].next=-1;
	bords[n].s_prev=bords[n].s_next=-1;
	//	bords[n].delta=sval-eval;


	n=nbBord++;
	bords[n].pos=epos;
	bords[n].val=eval;
	bords[n].start=false;
	bords[n].other=n-1;
	bords[n].pente=bords[n-1].pente;
	bords[n].prev=bords[n].next=-1;
	bords[n].s_prev=bords[n].s_next=-1;
	//	bords[n].delta=eval-sval;

	InsertBord(n-1,spos,guess);
	InsertBord(n,epos,n-1);
/*	if ( bords[n-1].s_next < 0 ) {
		bords[n].s_next=-1;
		s_last=n;

		bords[n].s_prev=n-1;
		bords[n-1].s_next=n;
	} else if ( bords[bords[n-1].s_next].pos >= epos ) {
		bords[n].s_next=bords[n-1].s_next;
		bords[bords[n].s_next].s_prev=n;
		
		bords[n].s_prev=n-1;
		bords[n-1].s_next=n;
	} else {
		int c=bords[bords[n-1].s_next].s_next;
		while ( c >= 0 && bords[c].pos < epos ) c=bords[c].s_next;
		if ( c < 0 ) {
			bords[n].s_prev=s_last;
			bords[s_last].s_next=n;
			s_last=n;
		} else {
			bords[n].s_prev=bords[c].s_prev;
			bords[bords[n].s_prev].s_next=n;

			bords[n].s_next=c;
			bords[c].s_prev=n;
		}

	}*/
	return n;
}
int              FloatLigne::AddBordR(float spos,float sval,float epos,float eval,float pente,int guess)
{
//  if ( showCopy ) printf("br= %f %f -> %f %f \n",spos,sval,epos,eval);
//	return AddBord(spos,sval,epos,eval,pente,guess);
	if ( spos >= epos ) return -1;
	if ( nbBord+1 >= maxBord ) {
		maxBord=2*nbBord+2;
		bords=(float_ligne_bord*)realloc(bords,maxBord*sizeof(float_ligne_bord));
	}

#ifdef faster_flatten
  if ( fabsf(epos-spos) < 0.001 || fabsf(pente) > 1000 ) {
    return;
    epos=spos;
    pente=0;
  }
#endif

  if ( guess >= nbBord ) guess=-1;
  
  int n=nbBord++;
	bords[n].pos=spos;
	bords[n].val=sval;
	bords[n].start=true;
	bords[n].other=n+1;
	bords[n].pente=pente;
	bords[n].prev=bords[n].next=-1;
	bords[n].s_prev=bords[n].s_next=-1;


	n=nbBord++;
	bords[n].pos=epos;
	bords[n].val=eval;
	bords[n].start=false;
	bords[n].other=n-1;
	bords[n].pente=bords[n-1].pente;
	bords[n].prev=bords[n].next=-1;
	bords[n].s_prev=bords[n].s_next=-1;

	InsertBord(n,epos,guess);
	InsertBord(n-1,spos,n);
/*	if ( bords[n].s_prev < 0 ) {
		bords[n-1].s_prev=-1;
		s_first=n-1;

		bords[n-1].s_next=n;
		bords[n].s_prev=n-1;
	} else if ( bords[bords[n].s_prev].pos <= spos ) {
		bords[n-1].s_prev=bords[n].s_prev;
		bords[bords[n-1].s_prev].s_next=n-1;

		bords[n-1].s_next=n;
		bords[n].s_prev=n-1;
	} else {
		int c=bords[bords[n].s_prev].s_prev;
		while ( c >= 0 && bords[c].pos > spos ) c=bords[c].s_prev;
		if ( c < 0 ) {
			bords[n-1].s_next=s_first;
			bords[s_first].s_prev=n-1;
			s_first=n-1;
		} else {
			bords[n-1].s_next=bords[c].s_next;
			bords[bords[n-1].s_next].s_prev=n-1;

			bords[n-1].s_prev=c;
			bords[c].s_next=n-1;
		}
		
	}*/
	return n-1;
}
// variant where insertion is known to be trivial: just append to the list
int            FloatLigne::AppendBord(float spos,float sval,float epos,float eval,float pente)
{
//  if ( showCopy ) printf("b= %f %f -> %f %f \n",spos,sval,epos,eval);
//	return AddBord(spos,sval,epos,eval,pente,s_last);
	if ( spos >= epos ) return -1;
	if ( nbBord+1 >= maxBord ) {
		maxBord=2*nbBord+2;
		bords=(float_ligne_bord*)realloc(bords,maxBord*sizeof(float_ligne_bord));
	}

#ifdef faster_flatten
  if ( fabsf(epos-spos) < 0.001 || fabsf(pente) > 1000 ) {
    return;
    epos=spos;
    pente=0;
  }
#endif
	int n=nbBord++;
	bords[n].pos=spos;
	bords[n].val=sval;
	bords[n].start=true;
	bords[n].other=n+1;
	bords[n].pente=pente;
	bords[n].prev=bords[n].next=-1;
	bords[n].s_prev=s_last;
	bords[n].s_next=n+1;

	if ( s_last >= 0 ) bords[s_last].s_next=n;
	if ( s_first < 0 ) s_first=n;

	n=nbBord++;
	bords[n].pos=epos;
	bords[n].val=eval;
	bords[n].start=false;
	bords[n].other=n-1;
	bords[n].pente=bords[n-1].pente;
	bords[n].prev=bords[n].next=-1;
	bords[n].s_prev=n-1;
	bords[n].s_next=-1;

	s_last=n;

	return n;
}
// insertion in a boubly-linked list. nothing interesting here
void             FloatLigne::InsertBord(int no,float p,int guess)
{
  if ( no < 0 || no >= nbBord ) return;
  if ( s_first < 0 ) {
    s_first=s_last=no;
    bords[no].s_prev=-1;
    bords[no].s_next=-1;
    return;
  }
  if ( guess < 0 || guess >= nbBord ) {
    int c=s_first;
    while ( c >= 0 && c < nbBord && CmpBord(bords+c,bords+no) < 0 ) c=bords[c].s_next;
    if ( c < 0 || c >= nbBord ) {
      bords[no].s_prev=s_last;
      bords[s_last].s_next=no;
      s_last=no;
    } else {
      bords[no].s_prev=bords[c].s_prev;
      if ( bords[no].s_prev >= 0 ) {
        bords[bords[no].s_prev].s_next=no;
      } else {
        s_first=no;
      }
      bords[no].s_next=c;
      bords[c].s_prev=no;
    }
	} else {
		int c=guess;
    int stTst=CmpBord(bords+c,bords+no);
		if ( stTst == 0 ) {
			bords[no].s_prev=bords[c].s_prev;
			if ( bords[no].s_prev >= 0 ) {
				bords[bords[no].s_prev].s_next=no;
			} else {
				s_first=no;
			}
			bords[no].s_next=c;
			bords[c].s_prev=no;
		} else if ( stTst > 0 ) {
			while ( c >= 0 && c < nbBord && CmpBord(bords+c,bords+no) > 0 ) c=bords[c].s_prev;
			if ( c < 0 || c >= nbBord ) {
				bords[no].s_next=s_first;
				bords[s_first].s_prev=no; // s_first != -1
				s_first=no;
			} else {
				bords[no].s_next=bords[c].s_next;
				if ( bords[no].s_next >= 0 ) {
					bords[bords[no].s_next].s_prev=no;
				} else {
					s_last=no;
				}
				bords[no].s_prev=c;
				bords[c].s_next=no;
			}
		} else {
			while ( c >= 0 && c < nbBord && CmpBord(bords+c,bords+no) < 0 ) c=bords[c].s_next;
			if ( c < 0 || c >= nbBord ) {
				bords[no].s_prev=s_last;
				bords[s_last].s_next=no;
				s_last=no;
			} else {
				bords[no].s_prev=bords[c].s_prev;
				if ( bords[no].s_prev >= 0 ) {
					bords[bords[no].s_prev].s_next=no;
				} else {
					s_first=no;
				}
				bords[no].s_next=c;
				bords[c].s_prev=no;
			}
		}
	}
}
float            FloatLigne::RemainingValAt(float at,int pending)
{
	float   sum=0;
/*	int     no=firstAc;
	while ( no >= 0 && no < nbBord ) {
		int   nn=bords[no].other;
		sum+=bords[nn].val+(at-bords[nn].pos)*bords[nn].pente;
//				sum+=((at-bords[nn].pos)*bords[no].val+(bords[no].pos-at)*bords[nn].val)/(bords[no].pos-bords[nn].pos);
//		sum+=ValAt(at,bords[nn].pos,bords[no].pos,bords[nn].val,bords[no].val);
		no=bords[no].next;
	}*/
  // for each portion being scanned, compute coverage at position "at" and sum.
  // we could simply compute the sum of portion coverages as a "f(x)=ux+y" and evaluate it at "x=at",
  // but there are numerical problems with this approach, and it produces ugly lines of incorrectly 
  // computed alpha values, so i reverted to this "safe but slow" version
  for (int i=0;i<pending;i++) {
		int   nn=bords[i].pend_ind;
		sum+=bords[nn].val+(at-bords[nn].pos)*bords[nn].pente;
  }
	return sum;
}
// sorting
void             FloatLigne::SwapBords(int a,int b)
{
  int   oa=bords[a].other;
  int   ob=bords[b].other;
  
  float_ligne_bord swap=bords[a];
  bords[a]=bords[b];
  bords[b]=swap;
  
  if ( oa == b ) bords[b].other=a; else bords[oa].other=b;
  if ( ob == a ) bords[a].other=b; else bords[ob].other=a;
}
void             FloatLigne::SwapBords(int a,int b,int c)
{
  if (a == b || b == c || a == c) return;
  SwapBords (a, b);
  SwapBords (b, c);
}
void             FloatLigne::SortBords(int s,int e)
{
  if (s >= e) return;
  if (e == s + 1)  {
    if ( CmpBord(bords+s,bords+e) > 0 ) SwapBords (s, e);
    return;
  }
  
  int ppos = (s + e) / 2;
  int plast = ppos;
  float_ligne_bord pval = bords[ppos];
  
  int le = s, ri = e;
  while (le < ppos || ri > plast) {
    if (le < ppos) {
      do {
	      int test = CmpBord(bords+le,&pval);
	      if (test == 0) {
          // on colle les valeurs egales au pivot ensemble
          if (le < ppos - 1) {
            SwapBords (le, ppos - 1, ppos);
            ppos--;
            continue;	// sans changer le
          } else if (le == ppos - 1) {
            ppos--;
            break;
          } else {
            // oupsie
            break;
          }
        }
	      if (test > 0) {
          break;
        }
	      le++;
	    } while (le < ppos);
    }
    if (ri > plast) {
      do {
	      int test = CmpBord(bords+ri,&pval);
	      if (test == 0) {
          // on colle les valeurs egales au pivot ensemble
          if (ri > plast + 1) {
            SwapBords (ri, plast + 1, plast);
            plast++;
            continue;	// sans changer ri
          } else if (ri == plast + 1) {
            plast++;
            break;
          } else {
            // oupsie
            break;
          }
        }
	      if (test < 0) {
          break;
        }
	      ri--;
	    } while (ri > plast);
    }
    if (le < ppos) {
      if (ri > plast)  {
	      SwapBords (le, ri);
	      le++;
	      ri--;
	    } else {
	      if (le < ppos - 1) {
          SwapBords (ppos - 1, plast, le);
          ppos--;
          plast--;
        } else if (le == ppos - 1) {
          SwapBords (plast, le);
          ppos--;
          plast--;
        }
	    }
    } else {
      if (ri > plast + 1) {
	      SwapBords (plast + 1, ppos, ri);
	      ppos++;
	      plast++;
	    }  else if (ri == plast + 1) {
	      SwapBords (ppos, ri);
	      ppos++;
	      plast++;
	    } else {
	      break;
	    }
    }
  }
  SortBords (s, ppos - 1);
  SortBords (plast + 1, e);
}
// computation of non-overlapping runs of coverage
void             FloatLigne::Flatten(void)
{
	if ( nbBord <= 1 ) {
		Reset();
		return;
	}
	nbRun=0;
	firstAc=lastAc=-1;

//	qsort(bords,nbBord,sizeof(float_ligne_bord),FloatLigne::CmpBord);
//	SortBords(0,nbBord-1);
  
	float     totPente=0,totStart=0,totX=bords[0].pos;
	bool      startExists=false;
	float     lastStart=0,lastVal=0;
	int       pending=0;
//	for (int i=0;i<nbBord;) {
  // read the list from left to right, adding a run for each boundary crossed, minus runs with alpha=0
	for (int i=/*0*/s_first;i>=0 && i < nbBord ;) {
		float  cur=bords[i].pos;  // position of the current boundary (there may be several boundaries at this position)
		float  leftV=0,rightV=0;  // deltas in coverage value at this position
		float  leftP=0,rightP=0;  // deltas in coverage increase per unit length at this position
    // more precisely, leftV is the sum of decreases of coverage value, while rightV is the sum of increases, so that
    // leftV+rightV is the delta. idem for leftP and rightP
    
    // start by scanning all boundaries that end a portion at this position
		while ( i >= 0 && i < nbBord && bords[i].pos == cur && bords[i].start == false ) {
			leftV+=bords[i].val;
			leftP+=bords[i].pente;
#ifndef faster_flatten
      // we need to remove the boundary that started this coverage portion for the pending list
      if ( bords[i].other >= 0 && bords[i].other < nbBord ) {
        // so we use the pend_inv "array"
        int  k=bords[bords[i].other].pend_inv;
        if ( k >= 0 && k < pending ) {
          // and update the pend_ind array and its inverse pend_inv
          bords[k].pend_ind=bords[pending-1].pend_ind;
          bords[bords[k].pend_ind].pend_inv=k;
        }
      }
#endif
      // one less portion pending
			pending--;
      // and we move to the next boundary in the doubly linked list
      i=bords[i].s_next;
			//i++;
		}
    // then scan all boundaries that start a portion at this position
		while ( i >= 0 && i < nbBord && bords[i].pos == cur && bords[i].start == true ) {
			rightV+=bords[i].val;
			rightP+=bords[i].pente;
#ifndef faster_flatten
      bords[pending].pend_ind=i;
      bords[i].pend_inv=pending;
#endif
			pending++;
      i=bords[i].s_next;
			//i++;
		}

    // coverage value at end of the run will be "start coverage"+"delta per unit length"*"length"
		totStart=totStart+totPente*(cur-totX);
    
		if ( startExists ) {
      // add that run
			AddRun(lastStart,cur,lastVal,totStart,totPente);
		}
    // update "delta coverage per unit length"
		totPente+=rightP-leftP;
    // not really needed here
		totStart+=rightV-leftV;
    // update position
		totX=cur;
		if ( pending > 0 ) {
			startExists=true;
#ifndef faster_flatten
      // to avoid accumulation of numerical errors, we compute an accurate coverage for this position "cur"
      totStart=RemainingValAt(cur,pending);
#endif
			lastVal=totStart;
			lastStart=cur;
		} else {
			startExists=false;
			totStart=0;
      totPente=0;
		}
	}

}
void             FloatLigne::Affiche(void)
{
	printf("%i : \n",nbBord);
	for (int i=0;i<nbBord;i++) printf("(%f %f %f %i) ",bords[i].pos,bords[i].val,bords[i].pente,(bords[i].start?1:0));
	printf("\n");
	printf("%i : \n",nbRun);
	for (int i=0;i<nbRun;i++) printf("(%f %f -> %f %f / %f) ",runs[i].st,runs[i].vst,runs[i].en,runs[i].ven,runs[i].pente);
	printf("\n");
}

int              FloatLigne::AddRun(float st,float en,float vst,float ven)
{
	if ( st >= en ) return -1;

	if ( nbRun >= maxRun ) {
		maxRun=2*nbRun+1;
		runs=(float_ligne_run*)realloc(runs,maxRun*sizeof(float_ligne_run));
	}
/*  if ( nbRun > 0 && st < runs[nbRun-1].en-0.1 ) {
    printf("o");
  }*/
	int n=nbRun++;
	runs[n].st=st;
	runs[n].en=en;
	runs[n].vst=vst;
	runs[n].ven=ven;
	runs[n].pente=(ven-vst)/(en-st);
	return n;
}
int              FloatLigne::AddRun(float st,float en,float vst,float ven,float pente)
{
	if ( st >= en ) return -1;

	if ( nbRun >= maxRun ) {
		maxRun=2*nbRun+1;
		runs=(float_ligne_run*)realloc(runs,maxRun*sizeof(float_ligne_run));
	}
/*  if ( nbRun > 0 && st < runs[nbRun-1].en-0.1 ) {
    printf("o");
  }*/
	int n=nbRun++;
	runs[n].st=st;
	runs[n].en=en;
	runs[n].vst=vst;
	runs[n].ven=ven;
	runs[n].pente=pente;
	return n;
}

void             FloatLigne::Copy(FloatLigne* a)
{
	if ( a->nbRun <= 0 ) {
		Reset();
		return;
	}
	nbBord=0;
	nbRun=a->nbRun;
	if ( nbRun > maxRun ) {
		maxRun=nbRun;
		runs=(float_ligne_run*)realloc(runs,maxRun*sizeof(float_ligne_run));
	}
	memcpy(runs,a->runs,nbRun*sizeof(float_ligne_run));
}
void             FloatLigne::Copy(IntLigne* a)
{
	if ( a->nbRun <= 0 ) {
		Reset();
		return;
	}
	nbBord=0;
	nbRun=a->nbRun;
	if ( nbRun > maxRun ) {
		maxRun=nbRun*2;
		runs=(float_ligne_run*)realloc(runs,maxRun*sizeof(float_ligne_run));
	}
	for (int i=0;i<nbRun;i++) {
		runs[i].st=a->runs[i].st;
		runs[i].en=a->runs[i].en;
		runs[i].vst=a->runs[i].vst;
		runs[i].ven=a->runs[i].ven;
	}
}
// all operation on the FloatLigne instances are done by scanning the runs left to right in the source(s) instances, and adding the 
// necessary runs to the solution. thus it reduces to computing the operation between float_ligne_run elements
// but details are not pretty (et c'est une litote)
void             FloatLigne::Booleen(FloatLigne* a,FloatLigne* b,BooleanOp mod)
{
	Reset();
	if ( a->nbRun <= 0 && b->nbRun <= 0 ) return;
	if ( a->nbRun <= 0 ) {
		if ( mod == bool_op_union || mod == bool_op_symdiff ) Copy(b);
		return;
	}
	if ( b->nbRun <= 0 ) {
		if ( mod == bool_op_union || mod == bool_op_diff || mod == bool_op_symdiff ) Copy(a);
		return;
	}

	int     curA=0,curB=0;
	float   curPos=(a->runs[0].st<b->runs[0].st) ? a->runs[0].st : b->runs[0].st;
	float   nextPos=curPos;
	bool    inA=false,inB=false;
	float   valA=0,valB=0;
	if ( curPos == a->runs[0].st ) valA=a->runs[0].vst;
	if ( curPos == b->runs[0].st ) valB=b->runs[0].vst;
	
	while ( curA < a->nbRun && curB < b->nbRun ) {
		float_ligne_run  runA=a->runs[curA];
		float_ligne_run  runB=b->runs[curB];
		inA = ( curPos >= runA.st && curPos < runA.en );
		inB = ( curPos >= runB.st && curPos < runB.en );

		bool  startA=false,startB=false,endA=false,endB=false;
		if ( curPos < runA.st ) {
			if ( curPos < runB.st ) {
				startA = runA.st <= runB.st;
				startB = runA.st >= runB.st;
				nextPos = startA ? runA.st : runB.st;
			} else if ( curPos >= runB.st ) {
				startA = runA.st <= runB.en;
				endB = runA.st >= runB.en;
				nextPos = startA ? runA.st : runB.en;
			}
		} else if ( curPos == runA.st ) {
			if ( curPos < runB.st ) {
				endA = runA.en <= runB.st;
				startB = runA.en >= runB.st;
				nextPos = endA ? runA.en : runB.st;
			} else if ( curPos == runB.st ) {
				endA = runA.en <= runB.en;
				endB = runA.en >= runB.en;
				nextPos = endA? runA.en : runB.en;
			} else {
				endA = runA.en <= runB.en;
				endB = runA.en >= runB.en;
				nextPos = endA ? runA.en : runB.en;
			}
		} else {
			if ( curPos < runB.st ) {
				endA = runA.en <= runB.st;
				startB = runA.en >= runB.st;
				nextPos = startB ? runB.st : runA.en;
			} else if ( curPos == runB.st ) {
				endA = runA.en <= runB.en;
				endB = runA.en >= runB.en;
				nextPos = endA ? runA.en : runB.en;
			} else {
				endA = runA.en <= runB.en;
				endB = runA.en >= runB.en;
				nextPos = endA ? runA.en : runB.en;
			}
		}

		float  oValA=valA,oValB=valB;
		valA = inA ? ValAt(nextPos,runA.st,runA.en,runA.vst,runA.ven) : 0;
		valB = inB ? ValAt(nextPos,runB.st,runB.en,runB.vst,runB.ven) : 0;
		
		if ( mod == bool_op_union ) {
			if ( inA || inB )
					AddRun(curPos,nextPos,oValA+oValB,valA+valB);
		} else if ( mod == bool_op_inters ) {
			if ( inA && inB )
					AddRun(curPos,nextPos,oValA*oValB,valA*valB);
		} else if ( mod == bool_op_diff ) {
			if ( inA )
					AddRun(curPos,nextPos,oValA-oValB,valA-valB);
		} else if ( mod == bool_op_symdiff ) {
			if ( inA && !(inB) )
					AddRun(curPos,nextPos,oValA-oValB,valA-valB);
			if ( !(inA) && inB )
					AddRun(curPos,nextPos,oValB-oValA,valB-valA);
		}

		curPos=nextPos;
		if ( startA ) {
			inA=true;
			valA=runA.vst;
		}
		if ( startB ) {
			inB=true;
			valB=runB.vst;
		}
		if ( endA ) {
			inA=false;
			valA=0;
			curA++;
			if ( curA < a->nbRun && a->runs[curA].st == curPos )
					valA=a->runs[curA].vst;
		}
		if ( endB ) {
			inB=false;
			valB=0;
			curB++;
			if ( curB < b->nbRun && b->runs[curB].st == curPos )
					valB=b->runs[curB].vst;
		}
	}
	while ( curA < a->nbRun ) {
		float_ligne_run  runA=a->runs[curA];
		inA = (curPos >= runA.st && curPos < runA.en );
		inB=false;

		bool  startA=false,endA=false;
		if ( curPos < runA.st ) {
			nextPos=runA.st;
			startA=true;
		} else if ( curPos >= runA.st ) {
			nextPos=runA.en;
			endA=true;
		}

		float  oValA=valA,oValB=valB;
		valA = inA ? ValAt(nextPos,runA.st,runA.en,runA.vst,runA.ven) : 0;
		valB = 0;

		if ( mod == bool_op_union ) {
			if ( inA || inB )
					AddRun(curPos,nextPos,oValA+oValB,valA+valB);
		} else if ( mod == bool_op_inters ) {
			if ( inA && inB )
					AddRun(curPos,nextPos,oValA*oValB,valA*valB);
		} else if ( mod == bool_op_diff ) {
			if ( inA )
					AddRun(curPos,nextPos,oValA-oValB,valA-valB);
		} else if ( mod == bool_op_symdiff ) {
			if ( inA && !(inB) )
					AddRun(curPos,nextPos,oValA-oValB,valA-valB);
			if ( !(inA) && inB )
					AddRun(curPos,nextPos,oValB-oValA,valB-valA);
		}

		curPos = nextPos;
		if ( startA ) {
			inA = true;
			valA = runA.vst;
		}
		if ( endA ) {
			inA = false;
			valA = 0;
			curA++;
			if ( curA < a->nbRun && a->runs[curA].st == curPos )
					valA=a->runs[curA].vst;
		}
	}
	while ( curB < b->nbRun ) {
		float_ligne_run  runB=b->runs[curB];
		inB = ( curPos >= runB.st && curPos < runB.en );
		inA=false;

		bool  startB=false,endB=false;
		if ( curPos < runB.st ) {
			nextPos=runB.st;
			startB=true;
		} else if ( curPos >= runB.st ) { // trivially true?
			nextPos=runB.en;
			endB=true;
		}

		float  oValA=valA,oValB=valB;
		valB = inB ? ValAt(nextPos,runB.st,runB.en,runB.vst,runB.ven) : 0;
		valA = 0;

		if ( mod == bool_op_union ) {
			if ( inA || inB ) AddRun(curPos,nextPos,oValA+oValB,valA+valB);
		} else if ( mod == bool_op_inters ) {
			if ( inA && inB ) AddRun(curPos,nextPos,oValA*oValB,valA*valB);
		} else if ( mod == bool_op_diff ) {
			if ( inA ) AddRun(curPos,nextPos,oValA-oValB,valA-valB);
		} else if ( mod == bool_op_symdiff ) {
			if ( inA && !(inB) ) AddRun(curPos,nextPos,oValA-oValB,valA-valB);
			if ( !(inA) && inB ) AddRun(curPos,nextPos,oValB-oValA,valB-valA);
		}

		curPos=nextPos;
		if ( startB ) {
			inB=true;
			valB=runB.vst;
		}
		if ( endB ) {
			inB=false;
			valB=0;
			curB++;
			if ( curB < b->nbRun && b->runs[curB].st == curPos ) valB=b->runs[curB].vst;
		}
	}
}
void             FloatLigne::Min(FloatLigne* a,float tresh,bool addIt)
{
	Reset();
	if ( a->nbRun <= 0 ) return;

	bool  startExists=false;
	float lastStart=0,lastEnd=0;
	for (int i=0;i<a->nbRun;i++) {
		float_ligne_run runA=a->runs[i];
		if ( runA.vst <= tresh ) {
			if ( runA.ven <= tresh ) {
				if ( startExists ) {
					if ( lastEnd >= runA.st-0.00001 ) {
						lastEnd=runA.en;
					} else {
						if ( addIt ) AddRun(lastStart,lastEnd,tresh,tresh);
						lastStart=runA.st;lastEnd=runA.en;
					}
				} else {
					lastStart=runA.st;lastEnd=runA.en;
				}
				startExists=true;
			} else {
				float  cutPos=(runA.st*(tresh-runA.ven)+runA.en*(runA.vst-tresh))/(runA.vst-runA.ven);
				if ( startExists ) {
					if ( lastEnd >= runA.st-0.00001 ) {
						if ( addIt ) AddRun(lastStart,cutPos,tresh,tresh);
						AddRun(cutPos,runA.en,tresh,runA.ven);
					} else {
						if ( addIt ) AddRun(lastStart,lastEnd,tresh,tresh);
						if ( addIt ) AddRun(runA.st,cutPos,tresh,tresh);
						AddRun(cutPos,runA.en,tresh,runA.ven);
					}
				} else {
					if ( addIt ) AddRun(runA.st,cutPos,tresh,tresh);
					AddRun(cutPos,runA.en,tresh,runA.ven);
				}
				startExists=false;
			}
		} else {
			if ( runA.ven <= tresh ) {
				float  cutPos=(runA.st*(runA.ven-tresh)+runA.en*(tresh-runA.vst))/(runA.ven-runA.vst);
				if ( startExists ) {
					if ( addIt ) AddRun(lastStart,lastEnd,tresh,tresh);
				}
				AddRun(runA.st,cutPos,runA.vst,tresh);
				startExists=true;
				lastStart=cutPos;lastEnd=runA.en;
			} else {
				if ( startExists ) {
					if ( addIt ) AddRun(lastStart,lastEnd,tresh,tresh);
				}
				startExists=false;
				AddRun(runA.st,runA.en,runA.vst,runA.ven);
			}
		}
	}
	if ( startExists ) {
		if ( addIt ) AddRun(lastStart,lastEnd,tresh,tresh);
	}
}
void             FloatLigne::Split(FloatLigne* a,float tresh,FloatLigne* over)
{
	Reset();
	if ( a->nbRun <= 0 ) return;

	for (int i=0;i<a->nbRun;i++) {
		float_ligne_run runA=a->runs[i];
		if ( runA.vst >= tresh ) {
			if ( runA.ven >= tresh ) {
				if ( over ) over->AddRun(runA.st,runA.en,runA.vst,runA.ven);
			} else {
				float  cutPos=(runA.st*(tresh-runA.ven)+runA.en*(runA.vst-tresh))/(runA.vst-runA.ven);
				if ( over ) over->AddRun(runA.st,cutPos,runA.vst,tresh);
				AddRun(cutPos,runA.en,tresh,runA.ven);
			}
		} else {
			if ( runA.ven >= tresh ) {
				float  cutPos=(runA.st*(runA.ven-tresh)+runA.en*(tresh-runA.vst))/(runA.ven-runA.vst);
				AddRun(runA.st,cutPos,runA.vst,tresh);
				if ( over ) over->AddRun(cutPos,runA.en,tresh,runA.ven);
			} else {
				AddRun(runA.st,runA.en,runA.vst,runA.ven);
			}
		}
	}
}
void             FloatLigne::Max(FloatLigne* a,float tresh,bool addIt)
{
	Reset();
	if ( a->nbRun <= 0 ) return;

	bool  startExists=false;
	float lastStart=0,lastEnd=0;
	for (int i=0;i<a->nbRun;i++) {
		float_ligne_run runA=a->runs[i];
		if ( runA.vst >= tresh ) {
			if ( runA.ven >= tresh ) {
				if ( startExists ) {
					if ( lastEnd >= runA.st-0.00001 ) {
						lastEnd=runA.en;
					} else {
						if ( addIt ) AddRun(lastStart,lastEnd,tresh,tresh);
						lastStart=runA.st;lastEnd=runA.en;
					}
				} else {
					lastStart=runA.st;lastEnd=runA.en;
				}
				startExists=true;
			} else {
				float  cutPos=(runA.st*(tresh-runA.ven)+runA.en*(runA.vst-tresh))/(runA.vst-runA.ven);
				if ( startExists ) {
					if ( lastEnd >= runA.st-0.00001 ) {
						if ( addIt ) AddRun(lastStart,cutPos,tresh,tresh);
						AddRun(cutPos,runA.en,tresh,runA.ven);
					} else {
						if ( addIt ) AddRun(lastStart,lastEnd,tresh,tresh);
						if ( addIt ) AddRun(runA.st,cutPos,tresh,tresh);
						AddRun(cutPos,runA.en,tresh,runA.ven);
					}
				} else {
					if ( addIt ) AddRun(runA.st,cutPos,tresh,tresh);
					AddRun(cutPos,runA.en,tresh,runA.ven);
				}
				startExists=false;
			}
		} else {
			if ( runA.ven >= tresh ) {
				float  cutPos=(runA.st*(runA.ven-tresh)+runA.en*(tresh-runA.vst))/(runA.ven-runA.vst);
				if ( startExists ) {
					if ( addIt ) AddRun(lastStart,lastEnd,tresh,tresh);
				}
				AddRun(runA.st,cutPos,runA.vst,tresh);
				startExists=true;
				lastStart=cutPos;lastEnd=runA.en;
			} else {
				if ( startExists ) {
					if ( addIt ) AddRun(lastStart,lastEnd,tresh,tresh);
				}
				startExists=false;
				AddRun(runA.st,runA.en,runA.vst,runA.ven);
			}
		}
	}
	if ( startExists ) {
		if ( addIt ) AddRun(lastStart,lastEnd,tresh,tresh);
	}
}
void             FloatLigne::Over(FloatLigne* a,float tresh)
{
	Reset();
	if ( a->nbRun <= 0 ) return;

	bool  startExists=false;
	float lastStart=0,lastEnd=0;
	for (int i=0;i<a->nbRun;i++) {
		float_ligne_run runA=a->runs[i];
		if ( runA.vst >= tresh ) {
			if ( runA.ven >= tresh ) {
				if ( startExists ) {
					if ( lastEnd >= runA.st-0.00001 ) {
						lastEnd=runA.en;
					} else {
						AddRun(lastStart,lastEnd,tresh,tresh);
						lastStart=runA.st;lastEnd=runA.en;
					}
				} else {
					lastStart=runA.st;lastEnd=runA.en;
				}
				startExists=true;
			} else {
				float  cutPos=(runA.st*(tresh-runA.ven)+runA.en*(runA.vst-tresh))/(runA.vst-runA.ven);
				if ( startExists ) {
					if ( lastEnd >= runA.st-0.00001 ) {
						AddRun(lastStart,cutPos,tresh,tresh);
					} else {
						AddRun(lastStart,lastEnd,tresh,tresh);
						AddRun(runA.st,cutPos,tresh,tresh);
					}
				} else {
					AddRun(runA.st,cutPos,tresh,tresh);
				}
				startExists=false;
			}
		} else {
			if ( runA.ven >= tresh ) {
				float  cutPos=(runA.st*(runA.ven-tresh)+runA.en*(tresh-runA.vst))/(runA.ven-runA.vst);
				if ( startExists ) {
					AddRun(lastStart,lastEnd,tresh,tresh);
				}
				startExists=true;
				lastStart=cutPos;lastEnd=runA.en;
			} else {
				if ( startExists ) {
					AddRun(lastStart,lastEnd,tresh,tresh);
				}
				startExists=false;
			}
		}
	}
	if ( startExists ) {
		AddRun(lastStart,lastEnd,tresh,tresh);
	}
}

void             FloatLigne::Enqueue(int no)
{
	if ( firstAc < 0 ) {
		firstAc=lastAc=no;
		bords[no].prev=bords[no].next=-1;
	} else {
		bords[no].next=-1;
		bords[no].prev=lastAc;
		bords[lastAc].next=no;
		lastAc=no;
	}
}
void             FloatLigne::Dequeue(int no)
{
	if ( no == firstAc ) {
		if ( no == lastAc ) {
			firstAc=lastAc=-1;
		} else {
			firstAc=bords[no].next;
		}
	} else if ( no == lastAc ) {
		lastAc=bords[no].prev;
	} else {
	}
	if ( bords[no].prev >= 0 ) bords[bords[no].prev].next=bords[no].next;
	if ( bords[no].next >= 0 ) bords[bords[no].next].prev=bords[no].prev;
	bords[no].prev=bords[no].next=-1;
}


/*
 *
 *
 *
 */


IntLigne::IntLigne(void)
{
	nbBord=maxBord=0;
	bords=NULL;

	nbRun=maxRun=0;
	runs=NULL;

	firstAc=lastAc=-1;
}
IntLigne::~IntLigne(void)
{
	if ( maxBord > 0 ) {
		free(bords);
		nbBord=maxBord=0;
		bords=NULL;
	}
	if ( maxRun > 0 ) {
		free(runs);
		nbRun=maxRun=0;
		runs=NULL;
	}
}

void             IntLigne::Reset(void)
{
	nbBord=0;
	nbRun=0;
	firstAc=lastAc=-1;
}
int              IntLigne::AddBord(int spos,float sval,int epos,float eval)
{
	if ( nbBord+1 >= maxBord ) {
		maxBord=2*nbBord+2;
		bords=(int_ligne_bord*)realloc(bords,maxBord*sizeof(int_ligne_bord));
	}
	int n=nbBord++;
	bords[n].pos=spos;
	bords[n].val=sval;
	bords[n].start=true;
	bords[n].other=n+1;
	bords[n].prev=bords[n].next=-1;

	n=nbBord++;
	bords[n].pos=epos;
	bords[n].val=eval;
	bords[n].start=false;
	bords[n].other=n-1;
	bords[n].prev=bords[n].next=-1;

	return n-1;
}
float            IntLigne::RemainingValAt(int at)
{
	int     no=firstAc;
	float   sum=0;
	while ( no >= 0 ) {
		int   nn=bords[no].other;
		sum+=ValAt(at,bords[nn].pos,bords[no].pos,bords[nn].val,bords[no].val);
		no=bords[no].next;
	}
	return sum;
}

void             IntLigne::Flatten(void)
{
	if ( nbBord <= 1 ) {
		Reset();
		return;
	}
	nbRun=0;
	firstAc=lastAc=-1;
	
	for (int i=0;i<nbBord;i++) bords[i].prev=i;
	qsort(bords,nbBord,sizeof(int_ligne_bord),IntLigne::CmpBord);
	for (int i=0;i<nbBord;i++) bords[bords[i].prev].next=i;
	for (int i=0;i<nbBord;i++) bords[i].other=bords[bords[i].other].next;
	
	int     lastStart=0;
	float   lastVal=0;
	bool    startExists=false;
	for (int i=0;i<nbBord;) {
		int    cur=bords[i].pos;
		float  leftV=0,rightV=0,midV=0;
		while ( i < nbBord && bords[i].pos == cur && bords[i].start == false ) {
			Dequeue(i);
			leftV += bords[i].val;
			i++;
		}
		midV=RemainingValAt(cur);
		while ( i < nbBord && bords[i].pos == cur && bords[i].start == true ) {
			rightV += bords[i].val;
			Enqueue(bords[i].other);
			i++;
		}

		if ( startExists ) {
			AddRun(lastStart,cur,lastVal,leftV+midV);
		}
		if ( firstAc >= 0 ) {
			startExists=true;
			lastVal=midV+rightV;
			lastStart=cur;
		} else {
			startExists=false;
		}
	}
}
void             IntLigne::Affiche(void)
{
	printf("%i : \n",nbRun);
	for (int i=0;i<nbRun;i++)
		printf("(%i %f -> %i %f) ",runs[i].st,runs[i].vst,runs[i].en,runs[i].ven);
	printf("\n");
}

int              IntLigne::AddRun(int st,int en,float vst,float ven)
{
	if ( st >= en ) return -1;

	if ( nbRun >= maxRun ) {
		maxRun=2*nbRun+1;
		runs=(int_ligne_run*)realloc(runs,maxRun*sizeof(int_ligne_run));
	}
	int n=nbRun++;
	runs[n].st=st;
	runs[n].en=en;
	runs[n].vst=vst;
	runs[n].ven=ven;
	return n;
}

void             IntLigne::Booleen(IntLigne* a,IntLigne* b,BooleanOp mod)
{
	Reset();
	if ( a->nbRun <= 0 && b->nbRun <= 0 ) return;
	if ( a->nbRun <= 0 ) {
		if ( mod == bool_op_union || mod == bool_op_symdiff ) Copy(b);
		return;
	}
	if ( b->nbRun <= 0 ) {
		if ( mod == bool_op_union || mod == bool_op_diff || mod == bool_op_symdiff ) Copy(a);
		return;
	}

	int     curA=0,curB=0;
	int     curPos=(a->runs[0].st<b->runs[0].st)?a->runs[0].st:b->runs[0].st;
	int     nextPos=curPos;
	float   valA=0,valB=0;
	if ( curPos == a->runs[0].st ) valA=a->runs[0].vst;
	if ( curPos == b->runs[0].st ) valB=b->runs[0].vst;
	
	while ( curA < a->nbRun && curB < b->nbRun ) {
		int_ligne_run  runA=a->runs[curA];
		int_ligne_run  runB=b->runs[curB];
		const bool inA = ( curPos >= runA.st && curPos < runA.en );
		const bool inB = ( curPos >= runB.st && curPos < runB.en );

		bool  startA=false,startB=false,endA=false,endB=false;
		if ( curPos < runA.st ) {
			if ( curPos < runB.st ) {
				startA = runA.st <= runB.st;
				startB = runA.st >= runB.st;
				nextPos = startA ? runA.st : runB.st;
			} else if ( curPos >= runB.st ) {
				startA = runA.st <= runB.en;
				endB = runA.st >= runB.en;
				nextPos = startA ? runA.st : runB.en;
			}
		} else if ( curPos == runA.st ) {
			if ( curPos < runB.st ) {
				endA = runA.en <= runB.st;
				startB = runA.en >= runB.st;
				nextPos = startB ? runB.en : runA.st;
			} else if ( curPos == runB.st ) {
				endA = runA.en <= runB.en;
				endB = runA.en >= runB.en;
				nextPos = endA? runA.en : runB.en;
			} else {
				endA = runA.en <= runB.en;
				endB = runA.en >= runB.en;
				nextPos = endA ? runA.en : runB.en;
			}
		} else {
			if ( curPos < runB.st ) {
				endA = runA.en <= runB.st;
				startB = runA.en >= runB.st;
				nextPos = startB ? runB.st : runA.en;
			} else if ( curPos == runB.st ) {
				endA = runA.en <= runB.en;
				endB = runA.en >= runB.en;
				nextPos = endA ? runA.en : runB.en;
			} else {
				endA = runA.en <= runB.en;
				endB = runA.en >= runB.en;
				nextPos = endA ? runA.en : runB.en;
			}
		}

		float  oValA=valA,oValB=valB;
		valA = inA ? ValAt(nextPos,runA.st,runA.en,runA.vst,runA.ven) : 0;
		valB = inB ? ValAt(nextPos,runB.st,runB.en,runB.vst,runB.ven) : 0;
		
		if ( mod == bool_op_union ) {
			if ( inA || inB ) AddRun(curPos,nextPos,oValA+oValB,valA+valB);
		} else if ( mod == bool_op_inters ) {
			if ( inA && inB ) AddRun(curPos,nextPos,oValA*oValB,valA*valB);
		} else if ( mod == bool_op_diff ) {
			if ( inA ) AddRun(curPos,nextPos,oValA-oValB,valA-valB);
		} else if ( mod == bool_op_symdiff ) {
			if ( inA && !(inB) ) AddRun(curPos,nextPos,oValA-oValB,valA-valB);
			if ( !(inA) && inB ) AddRun(curPos,nextPos,oValB-oValA,valB-valA);
		}

		curPos=nextPos;
		if ( startA ) {
				// inA=true; these are never used
			valA=runA.vst;
		}
		if ( startB ) {
				//inB=true;
			valB=runB.vst;
		}
		if ( endA ) {
				//inA=false;
			valA=0;
			curA++;
			if ( curA < a->nbRun && a->runs[curA].st == curPos )
					valA=a->runs[curA].vst;
		}
		if ( endB ) {
				//inB=false;
			valB=0;
			curB++;
			if ( curB < b->nbRun && b->runs[curB].st == curPos )
					valB=b->runs[curB].vst;
		}
	}
	while ( curA < a->nbRun ) {
		int_ligne_run  runA=a->runs[curA];
		const bool inA = ( curPos >= runA.st && curPos < runA.en );
		const bool inB = false;

		bool  startA=false,endA=false;
		if ( curPos < runA.st ) {
			nextPos=runA.st;
			startA=true;
		} else if ( curPos >= runA.st ) {
			nextPos=runA.en;
			endA=true;
		}

		float  oValA=valA,oValB=valB;
		valA = inA ? ValAt(nextPos,runA.st,runA.en,runA.vst,runA.ven) : 0;
		valB = 0;

		if ( mod == bool_op_union ) {
			if ( inA || inB ) AddRun(curPos,nextPos,oValA+oValB,valA+valB);
		} else if ( mod == bool_op_inters ) {
			if ( inA && inB ) AddRun(curPos,nextPos,oValA*oValB,valA*valB);
		} else if ( mod == bool_op_diff ) {
			if ( inA ) AddRun(curPos,nextPos,oValA-oValB,valA-valB);
		} else if ( mod == bool_op_symdiff ) {
			if ( inA && !(inB) ) AddRun(curPos,nextPos,oValA-oValB,valA-valB);
			if ( !(inA) && inB ) AddRun(curPos,nextPos,oValB-oValA,valB-valA);
		}

		curPos=nextPos;
		if ( startA ) {
				//inA=true;
			valA=runA.vst;
		}
		if ( endA ) {
				//inA=false;
			valA=0;
			curA++;
			if ( curA < a->nbRun && a->runs[curA].st == curPos ) valA=a->runs[curA].vst;
		}
	}
	while ( curB < b->nbRun ) {
		int_ligne_run  runB=b->runs[curB];
		const bool inB = ( curPos >= runB.st && curPos < runB.en );
		const bool inA = false;

		bool  startB=false,endB=false;
		if ( curPos < runB.st ) {
			nextPos=runB.st;
			startB=true;
		} else if ( curPos >= runB.st ) {
			nextPos=runB.en;
			endB=true;
		}

		float  oValA=valA,oValB=valB;
		valB = inB ? ValAt(nextPos,runB.st,runB.en,runB.vst,runB.ven) : 0;
		valA = 0;

		if ( mod == bool_op_union ) {
			if ( inA || inB ) AddRun(curPos,nextPos,oValA+oValB,valA+valB);
		} else if ( mod == bool_op_inters ) {
			if ( inA && inB ) AddRun(curPos,nextPos,oValA*oValB,valA*valB);
		} else if ( mod == bool_op_diff ) {
			if ( inA ) AddRun(curPos,nextPos,oValA-oValB,valA-valB);
		} else if ( mod == bool_op_symdiff ) {
			if ( inA && !(inB) ) AddRun(curPos,nextPos,oValA-oValB,valA-valB);
			if ( !(inA) && inB ) AddRun(curPos,nextPos,oValB-oValA,valB-valA);
		}

		curPos=nextPos;
		if ( startB ) {
				//inB=true;
			valB=runB.vst;
		}
		if ( endB ) {
				//inB=false;
			valB=0;
			curB++;
			if ( curB < b->nbRun && b->runs[curB].st == curPos ) valB=b->runs[curB].vst;
		}
	}
}

// supersampled to alpha value. see the other Copy(int nbSub,BitLigne* *a)
void             IntLigne::Copy(BitLigne* a)
{
	if ( a->curMax <= a->curMin ) {
		Reset();
		return;
	}
	if ( a->curMin < a->st ) a->curMin=a->st;
	if ( a->curMax < a->st ) {
		Reset();
		return;
	}
	if ( a->curMin > a->en ) {
		Reset();
		return;
	}
	if ( a->curMax > a->en ) a->curMax=a->en;
	nbBord=0;
	nbRun=0;

	int     lastVal=0;
	int     lastStart=0;
	bool    startExists=false;

	int     masks[16]={0,1,1,2,1,2,2,3,1,2,2,3,2,3,3,4};

	uint32_t  c_full=a->fullB[(a->curMin-a->st)>>3];
	uint32_t  c_part=a->partB[(a->curMin-a->st)>>3];
	c_full<<=4*((a->curMin-a->st)&0x00000007);
	c_part<<=4*((a->curMin-a->st)&0x00000007);
	for (int i=a->curMin;i<=a->curMax;i++) {
		int   nbBit=masks[c_full>>28]+masks[c_part>>28];

		if ( nbBit > 0 ) {
			if ( startExists ) {
				if ( lastVal == nbBit ) {
					// on continue le run
				} else {
					AddRun(lastStart,i,((float)lastVal)/4,((float)lastVal)/4);
					lastStart=i;
					lastVal=nbBit;
				}
			} else {
				lastStart=i;
				lastVal=nbBit;
				startExists=true;
			}
		} else {
			if ( startExists ) {
				AddRun(lastStart,i,((float)lastVal)/4,((float)lastVal)/4);
			}
			startExists=false;
		}
		int chg=(i+1-a->st)&0x00000007;
		if ( chg == 0 ) {
			c_full=a->fullB[(i+1-a->st)>>3];
			c_part=a->partB[(i+1-a->st)>>3];
		} else {
			c_full<<=4;
			c_part<<=4;
		}
	}
	if ( startExists ) {
		AddRun(lastStart,a->curMax+1,((float)lastVal)/4,((float)lastVal)/4);
	}
}
// alpha values are computed from supersampled data, so we have to scan the BitLigne left to right, 
// summing the bits in each pixel. the alpha value is then "number of bits"/(nbSub*nbSub)"
// full bits and partial bits are treated as equals because the method produces ugly results otherwise
void             IntLigne::Copy(int nbSub,BitLigne* *as)
{
	if ( nbSub <= 0 ) {Reset();return;}
	if ( nbSub == 1 ) {Copy(as[0]);return;}
  // compute the min-max of the pixels to be rasterized from the min-max of the  inpur bitlignes
	int  curMin=as[0]->curMin,curMax=as[0]->curMax;
	for (int i=1;i<nbSub;i++) {
		if ( as[i]->curMin < curMin ) curMin=as[i]->curMin;
		if ( as[i]->curMax > curMax ) curMax=as[i]->curMax;
	}
	if ( curMin < as[0]->st ) curMin=as[0]->st;
	if ( curMax > as[0]->en ) curMax=as[0]->en;
	if ( curMax <= curMin ) {
		Reset();
		return;
	}
	nbBord=0;
	nbRun=0;

	int     lastVal=0;
	int     lastStart=0;
	bool    startExists=false;
	float   spA;
	int     masks[16]={0,1,1,2,1,2,2,3,1,2,2,3,2,3,3,4};

	int    theSt=as[0]->st;
	if ( nbSub == 4 ) {
    // special case for 4*4 supersampling, to avoid a few loops
		uint32_t  c_full[4];
		c_full[0]=as[0]->fullB[(curMin-theSt)>>3] | as[0]->partB[(curMin-theSt)>>3];
		c_full[0]<<=4*((curMin-theSt)&7);
		c_full[1]=as[1]->fullB[(curMin-theSt)>>3] | as[1]->partB[(curMin-theSt)>>3];
		c_full[1]<<=4*((curMin-theSt)&7);
		c_full[2]=as[2]->fullB[(curMin-theSt)>>3] | as[2]->partB[(curMin-theSt)>>3];
		c_full[2]<<=4*((curMin-theSt)&7);
		c_full[3]=as[3]->fullB[(curMin-theSt)>>3] | as[3]->partB[(curMin-theSt)>>3];
		c_full[3]<<=4*((curMin-theSt)&7);

		spA=1.0/(4*4);
		for (int i=curMin;i<=curMax;i++) {
			int   nbBit=0;
			if ( c_full[0] == 0 && c_full[1] == 0 && c_full[2] == 0 && c_full[3] == 0 ) {
				if ( startExists ) {
					AddRun(lastStart,i,((float)lastVal)*spA,((float)lastVal)*spA);
				}
				startExists=false;
				i=theSt+(((i-theSt)&(~7))+7);
			} else if ( c_full[0] == 0xFFFFFFFF && c_full[1] == 0xFFFFFFFF &&
							 c_full[2] == 0xFFFFFFFF && c_full[3] == 0xFFFFFFFF ) {
				if ( startExists ) {
					if ( lastVal == 4*4) {
					} else {
						AddRun(lastStart,i,((float)lastVal)*spA,((float)lastVal)*spA);
						lastStart=i;
					}
				} else {
					lastStart=i;
				}
				lastVal=4*4;
				startExists=true;
				i=theSt+(((i-theSt)&(~7))+7);
			} else {
				nbBit+=masks[c_full[0]>>28];
				nbBit+=masks[c_full[1]>>28];
				nbBit+=masks[c_full[2]>>28];
				nbBit+=masks[c_full[3]>>28];

				if ( nbBit > 0 ) {
					if ( startExists ) {
						if ( lastVal == nbBit ) {
							// on continue le run
						} else {
							AddRun(lastStart,i,((float)lastVal)*spA,((float)lastVal)*spA);
							lastStart=i;
							lastVal=nbBit;
						}
					} else {
						lastStart=i;
						lastVal=nbBit;
						startExists=true;
					}
				} else {
					if ( startExists ) {
						AddRun(lastStart,i,((float)lastVal)*spA,((float)lastVal)*spA);
					}
					startExists=false;
				}
			}
			int chg=(i+1-theSt)&7;
			if ( chg == 0 ) {
        if ( i < curMax ) {
          c_full[0]=as[0]->fullB[(i+1-theSt)>>3] | as[0]->partB[(i+1-theSt)>>3];
          c_full[1]=as[1]->fullB[(i+1-theSt)>>3] | as[1]->partB[(i+1-theSt)>>3];
          c_full[2]=as[2]->fullB[(i+1-theSt)>>3] | as[2]->partB[(i+1-theSt)>>3];
          c_full[3]=as[3]->fullB[(i+1-theSt)>>3] | as[3]->partB[(i+1-theSt)>>3];
        } else {
        // end of line. byebye
        }
			} else {
				c_full[0]<<=4;
				c_full[1]<<=4;
				c_full[2]<<=4;
				c_full[3]<<=4;
			}
		}
	} else {
		uint32_t  c_full[16]; // we take nbSub < 16, since 16*16 supersampling makes a 1/256 precision in alpha values
                          // and that's the max of what 32bit argb can represent
                          // in fact, we'll treat it as 4*nbSub supersampling, so that's a half truth and a full lazyness from me
		//	uint32_t  c_part[16];
    // start by putting the bits of the nbSub BitLignes in as[] in their respective c_full
		for (int i=0;i<nbSub;i++) {
			c_full[i]=as[i]->fullB[(curMin-theSt)>>3] | as[i]->partB[(curMin-theSt)>>3]; // fullB and partB treated equally
			c_full[i]<<=4*((curMin-theSt)&7);
			/*		c_part[i]=as[i]->partB[(curMin-theSt)>>3];
			c_part[i]<<=4*((curMin-theSt)&7);*/
		}

		spA=1.0/(4*nbSub); // contribution to the alpha value of a single bit of the supersampled data
		for (int i=curMin;i<=curMax;i++) {
			int   nbBit=0;
      //			int nbPartBit=0;
      // a little acceleration: if the lines only contain full or empty bits, we can flush what's remaining in the c_full
      // at best we flush an entire c_full, ie 32 bits, or 32/4=8 pixels
			bool  allEmpty=true;
			bool  allFull=true;
			for (int j=0;j<nbSub;j++) if ( c_full[j] != 0 /*|| c_part[j] != 0*/ ) {allEmpty=false;break;}
        if ( allEmpty ) {
          // the remaining bits in c_full[] are empty: flush
          if ( startExists ) {
					AddRun(lastStart,i,((float)lastVal)*spA,((float)lastVal)*spA);
				}
				startExists=false;
				i=theSt+(((i-theSt)&(~7))+7);
			} else {
				for (int j=0;j<nbSub;j++) if ( c_full[j] != 0xFFFFFFFF ) {allFull=false;break;}
				if ( allFull ) {
          // the remaining bits in c_full[] are empty: flush
					if ( startExists ) {
						if ( lastVal == 4*nbSub) {
						} else {
							AddRun(lastStart,i,((float)lastVal)*spA,((float)lastVal)*spA);
							lastStart=i;
						}
					} else {
						lastStart=i;
					}
					lastVal=4*nbSub;
					startExists=true;
					i=theSt+(((i-theSt)&(~7))+7);
				} else {
          // alpha values will be between 0 and 1, so we have more work to do
          // compute how many bit this pixel holds
					for (int j=0;j<nbSub;j++) {
						nbBit+=masks[c_full[j]>>28];
//						nbPartBit+=masks[c_part[j]>>28];
					}
          // and add a single-pixel run if needed, or extend the current run if the alpha value hasn't changed
					if ( nbBit > 0 ) {
						if ( startExists ) {
							if ( lastVal == nbBit ) {
								// alpha value hasn't changed: we continue
							} else {
                // alpha value did change: put the run that was being done,...
								AddRun(lastStart,i,((float)lastVal)*spA,((float)lastVal)*spA);
                // ... and start a new one
								lastStart=i;
								lastVal=nbBit;
							}
						} else {
              // alpha value was 0, so we "create" a new run with alpha nbBit
							lastStart=i;
							lastVal=nbBit;
							startExists=true;
						}
					} else {
						if ( startExists ) {
							AddRun(lastStart,i,((float)lastVal)*spA,((float)lastVal)*spA);
						}
						startExists=false;
					}
				}
			}
        // move to the right: shift bits in the c_full[], and if we shifted everything, load the next c_full[]
			int chg=(i+1-theSt)&7;
			if ( chg == 0 ) {
        if ( i < curMax ) {
          for (int j=0;j<nbSub;j++) {
            c_full[j]=as[j]->fullB[(i+1-theSt)>>3] | as[j]->partB[(i+1-theSt)>>3];
            //			c_part[j]=as[j]->partB[(i+1-theSt)>>3];
          }
        } else {
          // end of line. byebye
        }        
			} else {
				for (int j=0;j<nbSub;j++) {
					c_full[j]<<=4;
					//			c_part[j]<<=4;
				}
			}
		}
	}
      if ( startExists ) {
        AddRun(lastStart,curMax+1,((float)lastVal)*spA,((float)lastVal)*spA);
      }
}
void             IntLigne::Copy(IntLigne* a)
{
	if ( a->nbRun <= 0 ) {
		Reset();
		return;
	}
	nbBord=0;
	nbRun=a->nbRun;
	if ( nbRun > maxRun ) {
		maxRun=nbRun;
		runs=(int_ligne_run*)realloc(runs,maxRun*sizeof(int_ligne_run));
	}
	memcpy(runs,a->runs,nbRun*sizeof(int_ligne_run));
}
// go from runs with floating-point boundaries to integer boundaries:
// that involves replacing floating-point boundaries that are not integer by single-pixel runs
// so this function contains plenty of rounding and float->integer conversion (read: time-consuming)
void             IntLigne::Copy(FloatLigne* a)
{
	if ( a->nbRun <= 0 ) {
		Reset();
		return;
	}
/*  if ( showCopy ) {
    printf("\nfloatligne:\n");
    a->Affiche();
  }*/
	nbBord=0;
	nbRun=0;
	firstAc=lastAc=-1;
	bool  pixExists=false;
	int   curPos=(int)floorf(a->runs[0].st)-1;
	float lastSurf=0;
	
  // we take each run of the FloatLigne in sequence and make single-pixel runs of its boundaries as needed
  // since the float_ligne_runs are non-overlapping, when a single-pixel run intersects with another runs, 
  // it must intersect with the single-pixel run created for the end of that run. so instead of creating a new
  // int_ligne_run, we just add the coverage to that run.
	for (int i=0;i<a->nbRun;i++) {
		float_ligne_run   runA=a->runs[i];
		float curStF=floorf(runA.st);
		float curEnF=floorf(runA.en);
		int  curSt=(int)curStF;
		int  curEn=(int)curEnF;

    // stEx: start boundary is not integer -> create single-pixel run for it
    // enEx: end boundary is not integer -> create single-pixel run for it
    // miEx: the runs minus the eventual single-pixel runs is not empty
		bool  stEx=true,miEx=true,enEx=true;
		if ( runA.st-curStF < 0.00001 ) stEx=false;
		if ( runA.en-curEnF < 0.00001 ) enEx=false;
		int   miSt=curSt;
		float miStF=curStF;
		if ( stEx ) {
			miSt=curSt+1;
			miStF=curStF+1.0;
		} else {
			miSt=curSt;
      miStF=curStF;
		}
		if ( miSt >= curEn ) miEx=false;

    // msv and mev are the start and end value of the middle section of the run, that is the run minus the
    // single-pixel runs creaed for its boundaries
		float   msv;
		if ( stEx == false /*miSt == runA.st*/ ) {
			msv=runA.vst;
		} else if ( enEx == false && miSt == curEn ) {
			msv=runA.ven;
		} else {
//			msv=a->ValAt(miSt,runA.st,runA.en,runA.vst,runA.ven);
			msv=runA.vst+(miStF-runA.st)*runA.pente;
		}
		float   mev;
		if ( stEx == false && miEx == false /*curEn == runA.st*/ ) {
			mev=runA.vst;
		} else if ( enEx == false /*curEn == runA.en*/ ) {
			mev=runA.ven;
		} else {
//			mev=a->ValAt(curEn,runA.st,runA.en,runA.vst,runA.ven);
			mev=runA.vst+(curEnF-runA.st)*runA.pente;
		}
		
    // check the different cases
		if ( stEx ) {
			if ( enEx ) {
				// stEx && enEx
				if ( curEn > curSt ) {
					if ( pixExists ) {
						if ( curPos < curSt ) {
							AddRun(curPos,curPos+1,lastSurf,lastSurf);
							lastSurf=0.5*(msv+a->runs[i].vst)*(miStF-a->runs[i].st);
							AddRun(curSt,curSt+1,lastSurf,lastSurf);
						} else {
							lastSurf+=0.5*(msv+a->runs[i].vst)*(miStF-a->runs[i].st);
							AddRun(curSt,curSt+1,lastSurf,lastSurf);
						}
						pixExists=false;
					} else {
						lastSurf=0.5*(msv+a->runs[i].vst)*(miStF-a->runs[i].st);
						AddRun(curSt,curSt+1,lastSurf,lastSurf);						
					}
				} else {
					if ( pixExists ) {
						if ( curPos < curSt ) {
							AddRun(curPos,curPos+1,lastSurf,lastSurf);
							lastSurf=0.5*(a->runs[i].ven+a->runs[i].vst)*(a->runs[i].en-a->runs[i].st);
							curPos=curSt;
						} else {
							lastSurf+=0.5*(a->runs[i].ven+a->runs[i].vst)*(a->runs[i].en-a->runs[i].st);
						}
					} else {
						lastSurf=0.5*(a->runs[i].ven+a->runs[i].vst)*(a->runs[i].en-a->runs[i].st);
						curPos=curSt;
						pixExists=true;
					}
				}
			} else {
				// stEx && !enEx
				if ( pixExists ) {
					if ( curPos < curSt ) {
						AddRun(curPos,curPos+1,lastSurf,lastSurf);
						lastSurf=0.5*(msv+a->runs[i].vst)*(miStF-a->runs[i].st);
						AddRun(curSt,curSt+1,lastSurf,lastSurf);
					} else {
						lastSurf+=0.5*(msv+a->runs[i].vst)*(miStF-a->runs[i].st);
						AddRun(curSt,curSt+1,lastSurf,lastSurf);
					}
					pixExists=false;
				} else {
					lastSurf=0.5*(msv+a->runs[i].vst)*(miStF-a->runs[i].st);
					AddRun(curSt,curSt+1,lastSurf,lastSurf);
				}				
			}
		}
		if ( miEx ) {
			if ( pixExists ) {
				if ( curPos < miSt ) {
					AddRun(curPos,curPos+1,lastSurf,lastSurf);
				}
			}
			pixExists=false;
			AddRun(miSt,curEn,msv,mev);
		}
		if ( enEx ) {
			if ( curEn > curSt ) {
				lastSurf=0.5*(mev+a->runs[i].ven)*(a->runs[i].en-curEnF);
				pixExists=true;
				curPos=curEn;
			} else {
				if ( stEx ) {
				} else {
					if ( pixExists ) {
						AddRun(curPos,curPos+1,lastSurf,lastSurf);
					}
					lastSurf=0.5*(mev+a->runs[i].ven)*(a->runs[i].en-curEnF);
					pixExists=true;
					curPos=curEn;					
				}
			}
		}
	}
	if ( pixExists ) {
		AddRun(curPos,curPos+1,lastSurf,lastSurf);
	}
/*  if ( showCopy ) {
    printf("-> intligne:\n");
    Affiche();
  }*/
}


void             IntLigne::Enqueue(int no)
{
	if ( firstAc < 0 ) {
		firstAc=lastAc=no;
		bords[no].prev=bords[no].next=-1;
	} else {
		bords[no].next=-1;
		bords[no].prev=lastAc;
		bords[lastAc].next=no;
		lastAc=no;
	}
}
void             IntLigne::Dequeue(int no)
{
	if ( no == firstAc ) {
		if ( no == lastAc ) {
			firstAc=lastAc=-1;
		} else {
			firstAc=bords[no].next;
		}
	} else if ( no == lastAc ) {
		lastAc=bords[no].prev;
	} else {
	}
	if ( bords[no].prev >= 0 ) bords[bords[no].prev].next=bords[no].next;
	if ( bords[no].next >= 0 ) bords[bords[no].next].prev=bords[no].prev;
	bords[no].prev=bords[no].next=-1;
}
void        IntLigne::Raster(raster_info &dest,void* color,RasterInRunFunc worker)
{
  if ( nbRun <= 0 ) return;
	int min=runs[0].st;
	int max=runs[nbRun-1].en;
	if ( dest.endPix <= min || dest.startPix >= max ) return;

	int  curRun=-1;
	for (curRun=0;curRun<nbRun;curRun++) {
		if ( runs[curRun].en > dest.startPix ) break;
	}
  
	if ( curRun >= nbRun ) return;
  
  if ( runs[curRun].st < dest.startPix ) {
    int   nst=runs[curRun].st,nen=runs[curRun].en;
    float vst=runs[curRun].vst,ven=runs[curRun].ven;
    float nvst=(vst*(nen-dest.startPix)+ven*(dest.startPix-nst))/((float)(nen-nst));
    if ( runs[curRun].en <= dest.endPix ) {
      (worker)(dest,color,dest.startPix,nvst,runs[curRun].en,runs[curRun].ven);
    } else {
      float nven=(vst*(nen-dest.endPix)+ven*(dest.endPix-nst))/((float)(nen-nst));
      (worker)(dest,color,dest.startPix,nvst,dest.endPix,nven);
      return;
    }
    curRun++;
  }

	for (;(curRun < nbRun && runs[curRun].en <= dest.endPix);curRun++) {
    (worker)(dest,color,runs[curRun].st,runs[curRun].vst,runs[curRun].en,runs[curRun].ven);
//Buffer::RasterRun(*dest,color,runs[curRun].st,runs[curRun].vst,runs[curRun].en,runs[curRun].ven);
	}
	if ( curRun >= nbRun ) return;
  if ( runs[curRun].st < dest.endPix && runs[curRun].en > dest.endPix ) {
    int   nst=runs[curRun].st,nen=runs[curRun].en;
    float vst=runs[curRun].vst,ven=runs[curRun].ven;
    float nven=(vst*(nen-dest.endPix)+ven*(dest.endPix-nst))/((float)(nen-nst));
   
    (worker)(dest,color,runs[curRun].st,runs[curRun].vst,dest.endPix,nven);
    curRun++;
  }
}



