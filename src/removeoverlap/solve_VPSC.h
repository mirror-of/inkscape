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

#ifndef SEEN_REMOVEOVERLAP_SOLVE_VPSC_H
#define SEEN_REMOVEOVERLAP_SOLVE_VPSC_H

class Variable;
class Constraint;
class Blocks;

/**
 * Variable Placement with Separation Constraints problem instance
 */
class VPSC {
public:
	double satisfy();
	double solve();

	bool move_and_split();
	VPSC(Variable *vs[], const int n, Constraint *cs[], const int m);
	~VPSC();
protected:
	Blocks *bs;
private:
	void printBlocks();
	bool constraintGraphIsCyclic(Variable *vs[], const int n);
	bool blockGraphIsCyclic();
};

#endif // SEEN_REMOVEOVERLAP_SOLVE_VPSC_H
