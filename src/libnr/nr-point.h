#ifndef SEEN_NR_POINT_H
#define SEEN_NR_POINT_H

#include <math.h>
#include <stdexcept>

#include <libnr/nr-coord.h>
#include <libnr/nr-dim2.h>

struct NRPoint {
    NR::Coord x, y;
};

namespace NR {

class Matrix;

class Point {
public:
    Point()
    { }

    Point(Coord x, Coord y) {
        _pt[X] = x;
        _pt[Y] = y;
    }

    Point(NRPoint const &p) {
        _pt[X] = p.x;
        _pt[Y] = p.y;
    }

    Coord operator[](unsigned i) const throw(std::out_of_range) {
        if ( i > Y ) {
            throw std::out_of_range("index out of range");
        }
        return _pt[i];
    }

    Coord &operator[](unsigned i) throw(std::out_of_range) {
        if ( i > Y ) {
            throw std::out_of_range("index out of range");
        }
        return _pt[i];
    }

    Coord operator[](Dim2 d) const throw() { return _pt[d]; }
    Coord &operator[](Dim2 d) throw() { return _pt[d]; }

    /** Return a point like this point but rotated -90 degrees.
        (If the y axis grows downwards and the x axis grows to the
        right, then this is 90 degrees counter-clockwise.)
    **/
    Point ccw() const {
        return Point(_pt[Y], -_pt[X]);
    }

    /** Return a point like this point but rotated +90 degrees.
        (If the y axis grows downwards and the x axis grows to the
        right, then this is 90 degrees clockwise.)
    **/
    Point cw() const {
        return Point(-_pt[Y], _pt[X]);
    }

    void normalize();

    __attribute__((__deprecated__)) operator NRPoint() const {
        NRPoint nrp;
        nrp.x = _pt[X];
        nrp.y = _pt[Y];
        return nrp;
    }

    Point &operator+=(Point const &o) {
        for ( unsigned i = 0 ; i < 2 ; ++i ) {
            _pt[i] += o._pt[i];
        }
        return *this;
    }
  
    Point &operator-=(Point const &o) {
        for ( unsigned i = 0 ; i < 2 ; ++i ) {
            _pt[i] -= o._pt[i];
        }
        return *this;
    }
  
    Point &operator/=(double const s) {
        for ( unsigned i = 0 ; i < 2 ; ++i ) {
            _pt[i] /= s;
        }
        return *this;
    }

    Point &operator*=(double const s) {
        for ( unsigned i = 0 ; i < 2 ; ++i ) {
            _pt[i] *= s;
        }
        return *this;
    }
  
    __attribute__((__deprecated__))
    Point &operator*=(Point const &s) {
        for ( unsigned i = 0 ; i < 2 ; ++i ) {
            _pt[i] *= s[i];
        }
        return *this;
    }

    Point &operator*=(Matrix const &m);

private:
    Coord _pt[2];
};

} /* namespace NR */


#endif /* !SEEN_NR_POINT_H */

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0))
  indent-tabs-mode:nil
  fill-column:99
  End:
  vim: filetype=c++:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :
*/
