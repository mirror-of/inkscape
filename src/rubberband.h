#ifndef __RUBBERBAND_H__
#define __RUBBERBAND_H__

/*
 * Rubberbanding selector
 *
 * Author:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * Copyright (C) 1999-2002 Lauris Kaplinski
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */


#include "forward.h"
#include <libnr/nr-forward.h>
#include <libnr/nr-point.h>

/* fixme: do multidocument safe */

void sp_rubberband_start(SPDesktop *desktop, NR::Point const &p);

inline void sp_rubberband_start (SPDesktop *desktop, double x, double y)
{
  sp_rubberband_start(desktop, NR::Point(x, y));
}

void sp_rubberband_move(NR::Point const &p);
void sp_rubberband_move (double x, double y);
void sp_rubberband_stop (void);

gboolean sp_rubberband_rect (NRRect *rect);

#endif

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=c++:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :
