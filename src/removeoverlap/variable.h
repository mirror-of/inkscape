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

#include <vector>
class Block;
class Constraint;

class Variable
{
public:
	char* name;
	double desiredPosition;
	double weight;
	double offset;
	Block *block;
	bool visited;
	std::vector<Constraint*> in;
	std::vector<Constraint*> out;
	char *toString();
	inline Variable(const char *name, const double desiredPos, const double weight)
		: name(new char[strlen(name)+1])
		, desiredPosition(desiredPos)
		, weight(weight)
	{
		strcpy(this->name,name);
	}
	inline double position();
	~Variable(void){
		delete name;
		in.clear();
		out.clear();
	}
};
#include "block.h"

inline double Variable::position() {
	return block->posn+offset;
}
