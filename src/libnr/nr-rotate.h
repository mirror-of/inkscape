#ifndef SEEN_NR_ROTATE_H
#define SEEN_NR_ROTATE_H

#include <libnr/nr-point.h>
#include <libnr/nr-point-fns.h>
#include <libnr/nr-point-ops.h>

namespace NR {

class rotate {
public:
    Point vec;

private:
    rotate();

public:
    explicit rotate(Coord theta);
    explicit rotate(Point const &p) : vec(p) {}
    explicit rotate(Coord const x, Coord const y) : vec(x, y) {}

    bool operator==(rotate const &o) const {
        return vec == o.vec;
    }

    bool operator!=(rotate const &o) const {
        return vec != o.vec;
    }

    inline rotate &operator*=(rotate const &b);
    /* Defined in nr-rotate-ops.h. */

    rotate inverse() const {
        /* TODO: In the usual case that vec is a unit vector (within rounding error),
           dividing by len_sq is either a noop or numerically harmful.
           Make a unit_rotate class (or the like) that knows its length is 1. */
        double const len_sq = dot(vec, vec);
        return rotate( Point(vec[X], -vec[Y])
                       / len_sq );
    }
};

} /* namespace NR */


#endif /* !SEEN_NR_ROTATE_H */

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
