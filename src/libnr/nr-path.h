#ifndef __NR_PATH_H__
#define __NR_PATH_H__

/*
 * Pixel buffer rendering library
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * This code is in public domain
 */

#include <libnr/nr-forward.h>
#include <libnr/nr-coord.h>

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
  c-file-offsets:((innamespace . 0)(inline-open . 0))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=c++:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :
