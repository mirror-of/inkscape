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

typedef struct _NRVPath NRVPath;
typedef struct _NRBPath NRBPath;

enum {
	NR_PATH_LINETO,
	NR_PATH_CURVETO
};

enum {
	NR_WIND_RULE_NONZERO,
	NR_WIND_RULE_EVENODD
};

#include <libart_lgpl/art_vpath.h>
#include <libart_lgpl/art_bpath.h>

typedef struct _NRPathCode NRPathCode;
typedef union _NRPathElement NRPathElement;

struct _NRPathCode {
	unsigned int length : 24;
	unsigned int closed : 1;
	unsigned int code : 1;
};

union _NRPathElement {
	NRPathCode code;
	NR::Coord value;
};

/*
 * VPath structure:
 *   Number of elements
 *   Number of segments
 *   Length + closed
 *   x, y, x, y...
 */

struct _NRVPath {
	NRPathElement *elements;
};

/* fixme: (Lauris) */

NRVPath *nr_vpath_setup_from_art_vpath (NRVPath *d, const ArtVpath *avpath);

void nr_vpath_release (NRVPath *vpath);

struct _NRBPath {
	ArtBpath *path;
};

NRBPath *nr_path_duplicate_transform(NRBPath *d, NRBPath *s, NRMatrix const *transform);

NRBPath *nr_path_duplicate_transform(NRBPath *d, NRBPath *s, NR::Matrix const transform);

void nr_path_matrix_point_bbox_wind_distance (NRBPath *bpath, NR::Matrix const &m, NR::Point &pt,
					      NRRect *bbox, int *wind, NR::Coord *dist,
					      NR::Coord tolerance);

void nr_path_matrix_bbox_union (NRBPath *bpath, NRMatrix const *m, NRRect *bbox, NR::Coord tolerance);

#endif
