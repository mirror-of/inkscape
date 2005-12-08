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

#ifndef SEEN_REMOVEOVERLAP_VARIABLE_H
#define SEEN_REMOVEOVERLAP_VARIABLE_H

#include <vector>
class Block;
class Constraint;

class Variable
{
public:
	static const unsigned int _TOSTRINGBUFFSIZE=20;
	int id; // useful in log files
	double desiredPosition;
	double weight;
	double offset;
	Block *block;
	bool visited;
	std::vector<Constraint*> in;
	std::vector<Constraint*> out;
	char *toString();
	inline Variable(const int id, const double desiredPos, const double weight)
		: id(id)
		, desiredPosition(desiredPos)
		, weight(weight)
	{
	}
	inline double position();
	~Variable(void){
		in.clear();
		out.clear();
	}
};
#include "block.h"

inline double Variable::position() {
	return block->posn+offset;
}

#endif // SEEN_REMOVEOVERLAP_VARIABLE_H
