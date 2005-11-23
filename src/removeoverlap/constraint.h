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

#pragma once
#include "variable.h"

class Constraint
{
public:
	Variable *left;
	Variable *right;
	double gap;
	bool active;
	bool visited;
	double lm;
	char *toString();
	Constraint(Variable *left, Variable *right, double gap);
	~Constraint(void){};
	inline double Constraint::slack() const { return right->position() - gap - left->position(); }
	//inline bool operator<(Constraint const &o) const { return slack() < o.slack(); }
};

#include <limits>
static inline bool compareConstraints(Constraint *&l, Constraint *&r) {
	double sl = l->slack();
	double sr = r->slack();
	//if(l->left->block==l->right->block) sl=std::numeric_limits<double>::min();
	//if(r->left->block==r->right->block) sr=std::numeric_limits<double>::min();
	if(l->left->block==l->right->block) sl=-999999;
	if(r->left->block==r->right->block) sr=-999999;
	if(sl==sr) {
		int c = strcmp(l->left->name,r->left->name);
		if(c==0) {
			c = strcmp(l->right->name,r->right->name);
			if(c<0) return true;
			return false;
		}
		if(c<0) return true;
		return false;
	}
	return sl < sr;
}

