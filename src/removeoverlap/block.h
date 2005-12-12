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

#ifndef SEEN_REMOVEOVERLAP_BLOCK_H
#define SEEN_REMOVEOVERLAP_BLOCK_H

#include <vector>
class Variable;
class Constraint;
template <class T> class PairingHeap;
class StupidPriorityQueue;

class Block
{
public:
	std::vector<Variable*> *vars;
	double posn;
	double weight;
	double wposn;
	Block(Variable *v=NULL);
	~Block(void);
	Constraint *findMinLM();
	Constraint *findMinInConstraint();
	Constraint *findMinOutConstraint();
	void deleteMinInConstraint();
	void deleteMinOutConstraint();
	double desiredWeightedPosition();
	void merge(Block *b, Constraint *c, double dist);
	void split(Block *&l, Block *&r, Constraint *c);
	void setUpInConstraints();
	void setUpOutConstraints();
	double cost();
	char *toString();
	bool deleted;
private:
	PairingHeap<Constraint*> *in;
	PairingHeap<Constraint*> *out;
	void reset_active_lm(Variable *v, Variable *u);
	double compute_dfdv(Variable *v, Variable *u, Constraint *&min_lm);
	bool canFollowLeft(Constraint *c, Variable *last);
	bool canFollowRight(Constraint *c, Variable *last);
	void populateSplitBlock(Block *b, Variable *v, Variable *u);
	void addVariable(Variable *v);
	void setUpConstraintHeap(PairingHeap<Constraint*>* &h,bool in);
};

#include "variable.h"

inline void Block::addVariable(Variable *v) {
	v->block=this;
	vars->push_back(v);
	weight+=v->weight;
	wposn += v->weight * (v->desiredPosition - v->offset);
	posn=wposn/weight;
}
inline Block::Block(Variable *v) {
	posn=weight=wposn=0;
	in=NULL;
	out=NULL;
	deleted=false;
	vars=new std::vector<Variable*>;
	if(v!=NULL) {
	v->offset=0;
	addVariable(v);
	}
}

inline double Block::desiredWeightedPosition() {
	double wp = 0;
	for (std::vector<Variable*>::iterator v=vars->begin();v!=vars->end();v++) {
		wp += ((*v)->desiredPosition - (*v)->offset) * (*v)->weight;
	}
	return wp;
}

#endif // SEEN_REMOVEOVERLAP_BLOCK_H
