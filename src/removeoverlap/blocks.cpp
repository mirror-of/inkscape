/**
 * \brief A block structure defined over the variables
 *
 * A block structure defined over the variables such that each block contains
 * 1 or more variables, with the invariant that all constraints inside a block
 * are satisfied by keeping the variables fixed relative to one another
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
#include "assert.h"
#include <vector>
#include <list>
#include "stdio.h"
//#define LOGGING
using namespace std;

Blocks::Blocks(Variable *vs[], const int n) : vs(vs),nvs(n) {
	for(int i=0;i<nvs;i++) {
		insert(new Block(vs[i]));
	}
}
Blocks::~Blocks(void)
{
	for(set<Block*>::iterator i=begin();i!=end();i++) {
		delete *i;
	}
	clear();
}

/**
 * returns a list of variables with total ordering determined by the constraint 
 * DAG
 */
list<Variable*> *Blocks::totalOrder() {
	list<Variable*> *order = new list<Variable*>;
	for(int i=0;i<nvs;i++) {
		vs[i]->visited=false;
	}
	for(int i=0;i<nvs;i++) {
		if(vs[i]->in.size()==0) {
			dfsVisit(vs[i],order);
		}
	}
	return order;
}
// Recursive depth first search giving total order by pushing nodes in the DAG
// onto the front of the list when we finish searching them
void Blocks::dfsVisit(Variable *v, list<Variable*> *order) {
	v->visited=true;
	vector<Constraint*>::iterator it=v->out.begin();
	for(;it!=v->out.end();it++) {
		Constraint *c=*it;
		if(!c->right->visited) {
			dfsVisit(c->right, order);
		}
	}
	order->push_front(v);
}
/**
 * Processes incoming constraints, most violated to least, merging with the
 * neighbouring (left) block until no more violated constraints are found
 */
void Blocks::mergeLeft(Block *r) {	
#ifdef LOGGING
	FILE *logfile=fopen("cplacement.log","a");
	char *str=r->toString();
	fprintf(logfile,"merge_left called on %s\n",str);
	delete str;
	fclose(logfile);
#endif
	r->setUpInConstraints();
	Constraint *c=r->findMinInConstraint();
	while (c != NULL && c->slack()<0) {
		
#ifdef LOGGING
		logfile=fopen("cplacement.log","a");
		char *str=c->toString();
		fprintf(logfile,"  merge_left on constraint: %s\n",str);
		assert(strncmp(str,"(v4=-1.432)+0.100000<=(v2=-1.422)(-0.089346)",44)!=0);
		delete str;
		fclose(logfile);
#endif
		r->deleteMinInConstraint();
		Block *l = c->left->block;		
		//assert(r->nextRight!=l);
		double dist = c->left->offset + c->gap - c->right->offset;
		if (r->vars->size() > l->vars->size()) {
			r->merge(l, c, -dist);
		} else {
			l->merge(r, c, dist);
			Block *tmp = r;
			r = l;
			l = tmp;
		}
		erase(l);
		delete l;
		c=r->findMinInConstraint();
	}		
#ifdef LOGGING
	logfile=fopen("cplacement.log","a");
	fprintf(logfile,"  merged %s\n",r->toString());
	fclose(logfile);
#endif
}	
/**
 * Symmetrical to mergeLeft
 */
void Blocks::mergeRight(Block *l) {	
#ifdef LOGGING
	FILE *logfile=fopen("cplacement.log","a");
	fprintf(logfile,"merge_right called on %s\n",l->toString());
	fclose(logfile);
#endif	
	l->setUpOutConstraints();
	Constraint *c = l->findMinOutConstraint();
	while (c != NULL && c->slack()<0) {		
#ifdef LOGGING
		logfile=fopen("cplacement.log","a");
		fprintf(logfile,"  merge_right on constraint: %s\n",c->toString());
		fclose(logfile);
#endif
		l->deleteMinOutConstraint();
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
		erase(r);
		delete r;
		c=l->findMinOutConstraint();
	}	
#ifdef LOGGING
	logfile=fopen("cplacement.log","a");
	fprintf(logfile,"  merged %s\n",l->toString());
	fclose(logfile);
#endif
}	

/**
 * Splits block b across constraint c into two new blocks, l and r (c's left
 * and right sides respectively)
 */
void Blocks::split(Block *b, Block *&l, Block *&r, Constraint *c) {
	b->split(l,r,c);
#ifdef LOGGING
	FILE *logfile=fopen("cplacement.log","a");
	fprintf(logfile,"Left: %s, Right: %s\n",l->toString(),r->toString());
	fclose(logfile);
#endif
	r->posn = b->posn;
	r->wposn = r->posn * r->weight;
	mergeLeft(l);
	// r may have been merged!
	r = c->right->block;
	r->wposn = r->desiredWeightedPosition();
	r->posn = r->wposn / r->weight;
	mergeRight(r);
	
	erase(b);
	delete b;
	insert(l);
	insert(r);
}
/**
 * returns the cost total squared distance of variables from their desired
 * positions
 */
double Blocks::cost() {
	double c = 0;
	for(set<Block*>::iterator i=begin();i!=end();i++) {
		c += (*i)->cost();
	}
	return c;
}
