#ifndef __NR_POINT_OPS_H__
#define __NR_POINT_OPS_H__

#include <libnr/nr-types.h>

namespace NR {
extern double L1(Point const &p);
extern double L2(Point const &p);
extern double LInfty(Point const &p);

inline Point rot90(Point const &p) {
	return Point(-p.pt[Y], p.pt[X]);
}

} /* namespace NR */

#endif /* !__NR_POINT_OPS_H__ */
