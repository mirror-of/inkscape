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

class Variable;
class Constraint;

double satisfy_VPSC(Variable *vs[], const int n, Constraint *cs[], const int m);

double solve_VPSC(Variable *vs[], const int n, Constraint *cs[], const int m);

void cleanup();

