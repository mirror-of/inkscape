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
private:
	void reset_active_lm(Variable *v, Variable *u);
	double compute_dfdv(Variable *v, Variable *u, Constraint *&min_lm);
	bool canFollowLeft(Constraint *c, Variable *last);
	bool canFollowRight(Constraint *c, Variable *last);
	void populateSplitBlock(Block *b, Variable *v, Variable *u);
	void addVariable(Variable *v);
	void setUpConstraintHeap(PairingHeap<Constraint*>* &h,bool in);
public:
	PairingHeap<Constraint*> *in;
	PairingHeap<Constraint*> *out;
	//StupidPriorityQueue *ins;
	//StupidPriorityQueue *outs;
	std::vector<Variable*> *vars;
	double posn;
	double weight;
	double wposn;
	Block *nextLeft, *nextRight;
	Block();
	Block(Variable *v);
	~Block(void);
	Constraint *find_min_lm();
	Constraint *find_min_in_constraint();
	Constraint *find_min_out_constraint();
	double desiredWeightedPosition();
	void merge(Block *b, Constraint *c, double dist);
	void split(Block *&l, Block *&r, Constraint *c);
	void setUpInConstraints();
	void setUpOutConstraints();
	double cost();
	char *toString();
};

#include "variable.h"
using namespace std;

inline void Block::addVariable(Variable *v) {
	v->block=this;
	vars->push_back(v);
	weight+=v->weight;
	wposn += v->weight * (v->desiredPosition - v->offset);
	posn=wposn/weight;
}
inline Block::Block(Variable *v) {
	vars=new vector<Variable*>;
	weight=wposn=0;
	v->offset=0;
	addVariable(v);
	in=NULL;
	out=NULL;
	//ins=NULL;
	//outs=NULL;
}
inline Block::Block() {
	vars=new vector<Variable*>;
	weight=0;
	posn=0;
	wposn=0;
	in=NULL;
	out=NULL;
	//ins=NULL;
	//outs=NULL;
}

inline double Block::desiredWeightedPosition() {
	double wp = 0;
	for (vector<Variable*>::iterator v=vars->begin();v!=vars->end();v++) {
		wp += ((*v)->desiredPosition - (*v)->offset) * (*v)->weight;
	}
	return wp;
}

#endif // SEEN_REMOVEOVERLAP_BLOCK_H
