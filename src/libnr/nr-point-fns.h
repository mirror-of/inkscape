#ifndef __NR_POINT_OPS_H__
#define __NR_POINT_OPS_H__

#include <libnr/nr-point-ops.h>
#include <libnr/nr-dim2.h>

namespace NR {

extern double L1(Point const &p);
extern double L2(Point const &p);
extern double LInfty(Point const &p);
extern double atan2(Point const p);

/** Return NR::rotate(pi/2)*p.  This is much more efficient. */
inline Point rot90(Point const &p)
{
    return Point(-p[Y], p[X]);
}

/** Given two points and a parameter t \in [0, 1], return a point
 * proportionally from a to b by t. */
inline Point Lerp(double const t, Point const a, Point const b)
{
    return ( ( 1 - t ) * a
             + t * b );
}

Point unit_vector(Point const &a);

inline Coord dot(Point const &a, Point const &b)
{
    Coord ret = 0;
    for ( int i = 0 ; i < 2 ; i++ ) {
        ret += a[i] * b[i];
    }
    return ret;
}

/** Defined as dot(a, b.cw()). */
inline Coord cross(Point const &a, Point const &b)
{
    Coord ret = 0;
    ret -= a[0] * b[1];
    ret += a[1] * b[0];
    return ret;
}

Point abs(Point const &b);

} /* namespace NR */

#endif /* !__NR_POINT_OPS_H__ */

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
