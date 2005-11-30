/**
 *  \file guide-snapper.cpp
 *  \brief Snapping things to guides.
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   Frank Felfe <innerspace@iname.com>
 *   Carl Hetherington <inkscape@carlh.net>
 *
 * Copyright (C) 1999-2002 Authors 
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "libnr/nr-values.h"
#include "libnr/nr-point-fns.h"
#include "sp-namedview.h"
#include "sp-guide.h"
#include "guide-snapper.h"

GuideSnapper::GuideSnapper(SPNamedView const *nv, NR::Coord const d) : Snapper(nv, d)
{

}

Snapper::LineList GuideSnapper::get_snap_lines(NR::Point const &p) const
{
    LineList s;
    
    for (GSList const *l = _named_view->guides; l != NULL; l = l->next) {
        SPGuide const &g = *SP_GUIDE(l->data);
        s.push_back(std::make_pair(g.normal, g.position));
    }

    return s;
}

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :
