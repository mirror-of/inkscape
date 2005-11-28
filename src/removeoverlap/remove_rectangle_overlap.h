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

#ifndef SEEN_REMOVEOVERLAP_H
#define SEEN_REMOVEOVERLAP_H
void removeRectangleOverlap(Rectangle *rs[], const int n, const double xBorder, const double yBorder);
#endif // SEEN_REMOVEOVERLAP_H