#include <libnr/nr-point-fns.h>
#include <cmath>
#include <algorithm>

using NR::Point;

/** Compute the L1 norm, or manhattan distance, of \a p. */
NR::Coord NR::L1(Point const &p) {
	NR::Coord d = 0;
	for ( int i = 0 ; i < 2 ; i++ ) {
		d += fabs(p[i]);
	}
	return d;
}

/** Compute the L2, or euclidean, norm of \a p. */
NR::Coord NR::L2(Point const &p) {
	return hypot(p[0], p[1]);
}

/** Compute the L infinity, or maximum, norm of \a p. */
NR::Coord NR::LInfty(Point const &p) {
	NR::Coord d=0;
	for ( int i = 0 ; i < 2 ; i++ ) {
		d = std::max(d, fabs(p[i]));
	}
	return d;
}

NR::Coord NR::atan2(Point const p) {
	return std::atan2(p[NR::Y], p[NR::X]);
}

