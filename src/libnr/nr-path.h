#ifndef __NR_PATH_H__
#define __NR_PATH_H__

/*
 * NArtBpath: Curve component.  Adapted from libart.
 */

/*
 * libart_lgpl/art_bpath.h copyright information:
 *
 * Copyright (C) 1998 Raph Levien
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */


#include <libnr/nr-forward.h>
#include <libnr/nr-coord.h>
#include <libnr/nr-point.h>

typedef enum {
  NR_MOVETO,
  NR_MOVETO_OPEN,
  NR_CURVETO,
  NR_LINETO,
  NR_END
} NRPathcode;

class NArtBpath {
 public:
  /*< public >*/
  NRPathcode code;
  double x1, y1;
  double x2, y2;
  double x3, y3;

    NR::Point c(unsigned const i) const {
        switch (i) {
            case 1: return NR::Point(x1, y1);
            case 2: return NR::Point(x2, y2);
            case 3: return NR::Point(x3, y3);
            default: abort();
        }
    }

    void setC(unsigned const i, NR::Point const &p) {
        using NR::X; using NR::Y;
        switch (i) {
            case 1: x1 = p[X]; y1 = p[Y]; break;
            case 2: x2 = p[X]; y2 = p[Y]; break;
            case 3: x3 = p[X]; y3 = p[Y]; break;
            default: abort();
        }
    }
};

NArtBpath* nr_artpath_affine(NArtBpath *s, NR::Matrix const &transform);

//ArtBpath* nr_artpath_to_art_bpath(NArtBpath *s); // this lives in src/extension/internal/gnome.cpp to avoid requiring libart everywhere

struct NRBPath {
	NArtBpath *path;
};

NRBPath *nr_path_duplicate_transform(NRBPath *d, NRBPath *s, NRMatrix const *transform);

NRBPath *nr_path_duplicate_transform(NRBPath *d, NRBPath *s, NR::Matrix const transform);

void nr_path_matrix_point_bbox_wind_distance (NRBPath *bpath, NR::Matrix const &m, NR::Point &pt,
					      NRRect *bbox, int *wind, NR::Coord *dist,
					      NR::Coord tolerance);

void nr_path_matrix_bbox_union(NRBPath const *bpath, NR::Matrix const &m, NRRect *bbox);

#endif

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
