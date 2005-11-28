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

#ifndef SEEN_REMOVEOVERLAP_BLOCKS_H
#define SEEN_REMOVEOVERLAP_BLOCKS_H

class Block;
class Variable;
class Constraint;
class Blocks
{
public:
	Block *head;
	Blocks(Variable* vs[], const int n);
	~Blocks(void);
	void add(Block *b);
	void remove(Block *b);
	void merge_left(Block *r);
	void merge_right(Block *l);
	void split(Block *b, Block *&l, Block *&r, Constraint *c);
	double cost();
private:
	void dfsVisit(Variable *v);
};
#endif // SEEN_REMOVEOVERLAP_BLOCKS_H
