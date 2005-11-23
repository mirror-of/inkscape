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
#include "variable.h"

Constraint::Constraint(Variable *left, Variable *right, double gap)
{
	this->left=left; 
	left->out.push_back(this);
	this->right=right;
	right->in.push_back(this);
	this->gap=gap;
	active=false;
	visited=false;
}
char *Constraint::toString() {
	char *str=new char[50];
	sprintf(str,"%s+%f<=%s(%f)\n",left->toString(),gap,right->toString(),slack());
	return str;
}
