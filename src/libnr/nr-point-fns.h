#ifndef __NR_POINT_OPS_H__
#define __NR_POINT_OPS_H__

#include <libnr/nr-types.h>

namespace NR {

extern double L1(Point const &p);
extern double L2(Point const &p);
extern double LInfty(Point const &p);
extern double atan2(Point const p);

/** Return NR::rotate(pi/2)*p.  This is much more efficient. */
inline Point rot90(Point const &p) {
	return Point(-p[Y], p[X]);
}

/** Given two points and a parameter t \in [0, 1], return a point
 * proportionally from a to b by t. */
inline Point Lerp(double t, Point const a, Point const b) {
	return (1-t)*a + t*b;
}

} /* namespace NR */

#endif /* !__NR_POINT_OPS_H__ */
