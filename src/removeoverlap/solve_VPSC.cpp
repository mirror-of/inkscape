/**
 * \brief Remove overlaps function
 *
 * Authors:
 *   Tim Dwyer <tgdwyer@gmail.com>
 *
 * Copyright (C) 2005 Authors
 *
 * Released under GNU GPL.  Read the file 'COPYING' for more information.
 */

#include <vector>

#include "constraint.h"
#include "block.h"
#include "blocks.h"
#include "variable.h"
#include "solve_VPSC.h"
#include <stdio.h>
#include <assert.h>

using namespace std;

VPSC::VPSC(Variable *vs[], const int n, Constraint *cs[], const int m) {
	bs=new Blocks(vs,n);
#ifdef LOGGING
	printBlocks();
#endif
}
VPSC::~VPSC() {
	delete bs;
}

void VPSC::printBlocks() {
	Block *b = bs->head;
	int i=0;
	while (b != NULL) {
		FILE *logfile=fopen("cplacement.log","a");
		fprintf(logfile,"%d) %s\n",i,b->toString());
		fclose(logfile);
		b = b->nextRight;
		i++;
	}
}

double VPSC::satisfy() {
	Block *b = bs->head;
	while (b != NULL) {
		Block *r=b->nextRight;
		bs->merge_left(b);
		b=r;
	}
	//for(int i=0;i<m;i++) {
	//	assert(cs[i]->slack()>-0.0000001);
	//}
	return bs->cost();
}
double VPSC::solve() {
	satisfy();
	bool solved=false;
	while(!solved) {
		solved=true;
		Block *b=bs->head;
		while(b!=NULL) {
			Constraint *c=b->find_min_lm();
			if(c!=NULL && c->lm<0) {
#ifdef LOGGING
				FILE *logfile=fopen("cplacement.log","a");
				fprintf(logfile,"Split on constraint: %s\n",c->toString());
				fclose(logfile);
#endif
				solved=false;
				// Split on c
				Block *l=NULL, *r=NULL;
				bs->split(b,l,r,c);
				delete b;
				b=bs->head;
			}
			b=b->nextRight;
		}
	}
	//for(int i=0;i<m;i++) {
	//	assert(cs[i]->slack()>-0.0000001);
	//}
	return bs->cost();
}

bool VPSC::split_once() {
	bool optimal=true;
	Block *b=bs->head;
	while(b!=NULL) {
		Constraint *c=b->find_min_lm();
		if(c!=NULL && c->lm<0) {
#ifdef LOGGING
			FILE *logfile=fopen("cplacement.log","a");
			fprintf(logfile,"Split on constraint: %s\n",c->toString());
			fclose(logfile);
#endif
			optimal=false;
			// Split on c
			Block *l=NULL, *r=NULL;
			bs->split(b,l,r,c);
			delete b;
			b=bs->head;
		}
		b=b->nextRight;
	}
	return optimal;
}
