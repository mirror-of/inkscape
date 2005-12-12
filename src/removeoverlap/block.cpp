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


#include "constraint.h"
#include "block.h"
#include "blocks.h"
#include "variable.h"
#include "pairingheap/PairingHeap.h"
#include <cassert>

Block::~Block(void)
{
	delete vars;
	delete in;
	delete out;
}
void Block::setUpInConstraints() {
	setUpConstraintHeap(in,true);
}
void Block::setUpOutConstraints() {
	setUpConstraintHeap(out,false);
}
void Block::setUpConstraintHeap(PairingHeap<Constraint*>* &h,bool in) {
	delete h;
	h = new PairingHeap<Constraint*>(&compareConstraints);
	for (vector<Variable*>::iterator i=vars->begin();i!=vars->end();i++) {
		Variable *v=*i;
		vector<Constraint*> *cs=in?&(v->in):&(v->out);
		for (vector<Constraint*>::iterator j=cs->begin();j!=cs->end();j++) {
			Constraint *c=*j;
			if (c->left->block != this && in || c->right->block != this && !in) {
				h->insert(c);
			}
		}
	}
}
// Merge b into this block
void Block::merge(Block *b, Constraint *c, double dist) {
	c->active=true;
	wposn+=b->wposn-dist*b->weight;
	weight+=b->weight;
	posn=wposn/weight;
	for(vector<Variable*>::iterator i=b->vars->begin();i!=b->vars->end();i++) {
		Variable *v=*i;
		v->block=this;
		v->offset+=dist;
		vars->push_back(v);
	}
	if (in == NULL)
		setUpInConstraints();
	if (b->in == NULL)
		b->setUpInConstraints();
	in->merge(b->in);
	if (out == NULL)
		setUpOutConstraints();
	if (b->out == NULL)
		b->setUpOutConstraints();
	out->merge(b->out);
}
Constraint *Block::findMinInConstraint() {
	//assert(ins->size()==in->size());
	if(in->isEmpty()) return NULL;
	Constraint *v = in->findMin();
	while (v->left->block == v->right->block) {
		in->deleteMin();
		//ins->deleteMin();
		if(in->isEmpty()) return NULL;
		v = in->findMin();
	}
	//assert(v==ins->findMin());
	return v;
}
Constraint *Block::findMinOutConstraint() {
	//assert(outs->size()==out->size());
	if(out->isEmpty()) return NULL;
	Constraint *v = out->findMin();
	while (v->left->block == v->right->block) {
		out->deleteMin();
		//outs->deleteMin();
		if(out->isEmpty()) return NULL;
		v = out->findMin();
	}
	//assert(v==outs->findMin());
	return v;
}
void Block::deleteMinInConstraint() {
	in->deleteMin();
}
void Block::deleteMinOutConstraint() {
	out->deleteMin();
}
inline bool Block::canFollowLeft(Constraint *c, Variable *last) {
	return c->left->block==this && c->active && last!=c->left;
}
inline bool Block::canFollowRight(Constraint *c, Variable *last) {
	return c->right->block==this && c->active && last!=c->right;
}
double Block::compute_dfdv(Variable *v, Variable *u, Constraint *&min_lm) {
	double dfdv=v->weight*(v->position() - v->desiredPosition);
	for(vector<Constraint*>::iterator it=v->out.begin();it!=v->out.end();it++) {
		Constraint *c=*it;
		if(canFollowRight(c,u)) {
			dfdv+=c->lm=compute_dfdv(c->right,v,min_lm);
			if(min_lm==NULL||c->lm<min_lm->lm) min_lm=c;
		}
	}
	for(vector<Constraint*>::iterator it=v->in.begin();it!=v->in.end();it++) {
		Constraint *c=*it;
		if(canFollowLeft(c,u)) {
			dfdv-=c->lm=-compute_dfdv(c->left,v,min_lm);
			if(min_lm==NULL||c->lm<min_lm->lm) min_lm=c;
		}
	}
	return dfdv;
}
void Block::reset_active_lm(Variable *v, Variable *u) {
	for(vector<Constraint*>::iterator it=v->out.begin();it!=v->out.end();it++) {
		Constraint *c=*it;
		if(canFollowRight(c,u)) {
			c->lm=0;
			reset_active_lm(c->right,v);
		}
	}
	for(vector<Constraint*>::iterator it=v->in.begin();it!=v->in.end();it++) {
		Constraint *c=*it;
		if(canFollowLeft(c,u)) {
			c->lm=0;
			reset_active_lm(c->left,v);
		}
	}
}
void Block::populateSplitBlock(Block *b, Variable *v, Variable *u) {
	b->addVariable(v);
	for (vector<Constraint*>::iterator c=v->in.begin();c!=v->in.end();c++) {
		if (canFollowLeft(*c,u))
			populateSplitBlock(b, (*c)->left, v);
	}
	for (vector<Constraint*>::iterator c=v->out.begin();c!=v->out.end();c++) {
		if (canFollowRight(*c,u)) 
			populateSplitBlock(b, (*c)->right, v);
	}
}
Constraint *Block::findMinLM() {
	Constraint *min_lm=NULL;
	reset_active_lm(vars->front(),NULL);
	compute_dfdv(vars->front(),NULL,min_lm);
	return min_lm;
}

/**
 * Creates two new blocks, l and r, and splits this block across constraint c,
 * placing the left subtree of constraints (and associated variables) into l
 * and the right into r 
 */
void Block::split(Block *&l, Block *&r, Constraint *c) {
	c->active=false;
	l=new Block();
	populateSplitBlock(l,c->left,c->right);
	r=new Block();
	populateSplitBlock(r,c->right,c->left);
}

/**
 * Computes the cost (squared euclidean distance from desired positions) of the
 * current positions for variables in this block
 */
double Block::cost() {
	double c = 0;
	for (vector<Variable*>::iterator v=vars->begin();v!=vars->end();v++) {
		double diff = (*v)->position() - (*v)->desiredPosition;
		c += (*v)->weight * diff * diff;
	}
	return c;
}

#ifdef WIN32
#define snprintf _snprintf
#endif
char *Block::toString() {
	int buffsize=vars->size() * Variable::_TOSTRINGBUFFSIZE;
	char *str=new char[buffsize];
	sprintf(str,"Block:");
	for(vector<Variable*>::iterator v=vars->begin();v!=vars->end();v++) {
		snprintf(str,buffsize,"%s %s",str,(*v)->toString());
	}
	return str;
}
