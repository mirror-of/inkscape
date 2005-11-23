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
#include <stdio.h>
#include <assert.h>

using namespace std;

void printBlocks() {
	Block *b = Blocks::instance->head;
	int i=0;
	while (b != NULL) {
		FILE *logfile=fopen("cplacement.log","a");
		fprintf(logfile,"%d) %s\n",i,b->toString());
		fclose(logfile);
		b = b->nextRight;
		i++;
	}
}
double satisfy_VPSC(Variable *vs[], const int n, Constraint *cs[], const int m) {
	Blocks::instance=new Blocks(vs,n);		
	Block *b = Blocks::instance->head;
#ifdef LOGGING
	printBlocks();
#endif
	while (b != NULL) {
		Block *r=b->nextRight;
		Blocks::instance->merge_left(b);
		b=r;
	}
	//for(int i=0;i<m;i++) {
	//	assert(cs[i]->slack()>-0.0000001);
	//}
	return Blocks::instance->cost();
}
double solve_VPSC(Variable *vs[], const int n, Constraint *cs[], const int m) {
	satisfy_VPSC(vs,n,cs,m);
	bool solved=false;
	while(!solved) {
		solved=true;
		Block *b=Blocks::instance->head;
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
				Blocks::instance->split(b,l,r,c);
				delete b;
				b=Blocks::instance->head;
			}
			b=b->nextRight;
		}
	}
	//for(int i=0;i<m;i++) {
	//	assert(cs[i]->slack()>-0.0000001);
	//}
	return Blocks::instance->cost();
}

void cleanup() {
	delete Blocks::instance;
}
