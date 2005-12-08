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

#include "variable.h"
char *Variable::toString() {
	char *str=new char[_TOSTRINGBUFFSIZE];
	snprintf(str, _TOSTRINGBUFFSIZE, "(%3d=%.3f)", id, position());
	return str;
}

