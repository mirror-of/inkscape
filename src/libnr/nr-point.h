#ifndef SEEN_NR_POINT_H
#define SEEN_NR_POINT_H

#include <math.h>
#include <stdexcept>
#include <iostream>
#include <iomanip>

#include <libnr/nr-coord.h>
#include <libnr/nr-dim2.h>

#include "round.h"

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

    Point(Point const &p) {
        for (unsigned i = 0; i < 2; ++i) {
            _pt[i] = p._pt[i];
        }
    }

    Point &operator=(Point const &p) {
        for (unsigned i = 0; i < 2; ++i) {
            _pt[i] = p._pt[i];
        }
        return *this;
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

    /**
        \brief A function to lower the precision of the point
        \param  places  The number of decimal places that should be in
                        the final number.

        This function accomplishes its goal by doing a 10^places and
        multipling the X and Y coordinates by that.  It then casts that
        to an integer (to get rid of all decimal points) and then divides
        by 10^places back again.
    */
    inline void round (int places = 0) {
        _pt[X] = (Coord)(Inkscape::round((double)_pt[X] * pow(10, places)) / pow(10, places));
        _pt[Y] = (Coord)(Inkscape::round((double)_pt[Y] * pow(10, places)) / pow(10, places));
        return;
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

    inline int operator == (const Point &in_pnt) {
        return ((_pt[0] == in_pnt[0]) && (_pt[1] == in_pnt[1]));
    }

    friend inline std::ostream &operator<< (std::ostream &out_file, const NR::Point &in_pnt);

private:
    Coord _pt[2];
};

/** A function to print out the Point.  It just prints out the coords
    on the given output stream */
inline std::ostream &operator<< (std::ostream &out_file, const NR::Point &in_pnt) {
    out_file << "X: " << in_pnt[0] << "  Y: " << in_pnt[1];
    return out_file;
}

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
*/
// vim: filetype=c++:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :
