#include <libnr/nr-point-fns.h>
#include <libnr/nr-dim2.h>
#include <cmath>
#include <algorithm>

using NR::Point;

/** Compute the L infinity, or maximum, norm of \a p. */
NR::Coord NR::LInfty(Point const &p) {
	NR::Coord const a(fabs(p[NR::X]));
	NR::Coord const b(fabs(p[NR::Y]));
	return ( a < b || isnan(b)
		 ? b
		 : a );
}

NR::Coord NR::atan2(Point const p) {
	return std::atan2(p[NR::Y], p[NR::X]);
}

Point NR::unit_vector(Point const &a)
{
	Point ret(a);
	ret.normalize();
	return ret;
}

NR::Point abs(NR::Point const &b)
{
	NR::Point ret;
	for ( int i = 0 ; i < 2 ; i++ ) {
		ret[i] = fabs(b[i]);
	}
	return ret;
}
