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

#include "blocks.h"
#include "block.h"
#include "variable.h"
#include "constraint.h"
#include "pairingheap/PairingHeap.h"
#include "assert.h"
#include <vector>
#include "stdio.h"

using namespace std;
Blocks *Blocks::instance;

// creates list of blocks with a block for each variable, 
// with a total ordering dependant on the variable's
// position in the constraint DAG
Blocks::Blocks(Variable* vs[], const int n) {
	Blocks::instance=this;
	head=NULL;
	for(int i=0;i<n;i++) {
		vs[i]->visited=false;
	}
	for(int i=0;i<n;i++) {
		if(vs[i]->in.size()==0) {
			dfsVisit(vs[i]);
		}
	}
}
Blocks::~Blocks(void)
{
	Block *b=head;
	while (b != NULL) {
		Block *doomed=b;
		b = b->nextRight;
		delete doomed;
	}
}

void Blocks::add(Block *b) {
	b->nextLeft=NULL;
	b->nextRight=NULL;
	if(head!=NULL) {
		b->nextRight=head;
		head->nextLeft=b;
	}
	head=b;
}
void Blocks::remove(Block *b) {
	if (b == head) {
		head = b->nextRight;
		head->nextLeft = NULL;
	} else {
		b->nextLeft->nextRight = b->nextRight;
	}
	if (b->nextRight != NULL) {
		b->nextRight->nextLeft = b->nextLeft;
	}
	delete b;
}
void Blocks::dfsVisit(Variable *v) {
	v->visited=true;
	vector<Constraint*>::iterator it=v->out.begin();
	for(;it!=v->out.end();it++) {
		Constraint *c=*it;
		if(!c->right->visited) {
			dfsVisit(c->right);
		}
	}
	add(new Block(v));
}
void Blocks::merge_left(Block *r) {	
#ifdef LOGGING
	FILE *logfile=fopen("cplacement.log","a");
	fprintf(logfile,"merge_left called on %s\n",r->toString());
	fclose(logfile);
#endif
	r->setUpInConstraints();
	Constraint *c=r->find_min_in_constraint();
	while (c != NULL && c->slack()<0) {
		
#ifdef LOGGING
		logfile=fopen("cplacement.log","a");
		fprintf(logfile,"  merge_left on constraint: %s\n",c->toString());
		fclose(logfile);
#endif
		r->in->deleteMin();
		//r->ins->deleteMin();
		Block *l = c->left->block;		
		double dist = c->left->offset + c->gap - c->right->offset;
		if (r->vars->size() > l->vars->size()) {
			r->merge(l, c, -dist);
		} else {
			l->merge(r, c, dist);
			Block *tmp = r;
			r = l;
			l = tmp;
		}
		remove(l);
		c=r->find_min_in_constraint();
	}		
#ifdef LOGGING
	logfile=fopen("cplacement.log","a");
	fprintf(logfile,"  merged %s\n",r->toString());
	fclose(logfile);
#endif
}	
void Blocks::merge_right(Block *l) {	
#ifdef LOGGING
	FILE *logfile=fopen("cplacement.log","a");
	fprintf(logfile,"merge_right called on %s\n",l->toString());
	fclose(logfile);
#endif	
	l->setUpOutConstraints();
	Constraint *c = l->find_min_out_constraint();
	while (c != NULL && c->slack()<0) {		
#ifdef LOGGING
		logfile=fopen("cplacement.log","a");
		fprintf(logfile,"  merge_right on constraint: %s\n",c->toString());
		fclose(logfile);
#endif
		l->out->deleteMin();
		//l->outs->deleteMin();
		Block *r = c->right->block;
		double dist = c->left->offset + c->gap - c->right->offset;
		if (l->vars->size() > r->vars->size()) {
			l->merge(r, c, dist);
		} else {
			r->merge(l, c, -dist);
			Block *tmp = l;
			l = r;
			r = tmp;
		}
		remove(r);
		c=l->find_min_out_constraint();
	}	
#ifdef LOGGING
	logfile=fopen("cplacement.log","a");
	fprintf(logfile,"  merged %s\n",l->toString());
	fclose(logfile);
#endif
}	

void Blocks::split(Block *b, Block *&l, Block *&r, Constraint *c) {
	b->split(l,r,c);
#ifdef LOGGING
	FILE *logfile=fopen("cplacement.log","a");
	fprintf(logfile,"Left: %s, Right: %s\n",l->toString(),r->toString());
	fclose(logfile);
#endif
	r->posn = b->posn;
	r->wposn = r->posn * r->weight;
	l->nextLeft = b->nextLeft;
	if (l->nextLeft != NULL) {
		l->nextLeft->nextRight = l;
	} else {
		head = l;
	}
	l->nextRight = r;
	r->nextLeft = l;
	r->nextRight = b->nextRight;
	if (r->nextRight != NULL) {
		r->nextRight->nextLeft = r;
	}
	merge_left(l);
	// r may have been merged!
	r = c->right->block;
	r->wposn = r->desiredWeightedPosition();
	r->posn = r->wposn / r->weight;
	merge_right(r);
}
double Blocks::cost() {
	double c = 0;
	Block *b = head;
	while (b != NULL) {
		c += b->cost();
		b = b->nextRight;
	}
	return c;
}
