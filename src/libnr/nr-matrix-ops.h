/* operator functions for NR::Matrix. */
#ifndef SEEN_NR_MATRIX_OPS_H
#define SEEN_NR_MATRIX_OPS_H

#include <libnr/nr-matrix.h>
#include <libnr/nr-matrix-fns.h>
#include <libnr/nr-translate.h>
#include <libnr/nr-scale.h>


namespace NR {

inline bool operator==(Matrix const &a, Matrix const &b)
{
    for(unsigned i = 0; i < 6; ++i) {
        if ( a[i] != b[i] ) {
            return false;
        }
    }
    return true;
}

inline bool operator!=(Matrix const &a, Matrix const &b)
{
    return !( a == b );
}

inline Point operator*(Point const &v, Matrix const &m)
{
#if 1  /* Which code makes it easier to see what's happening? */
    NR::Point const xform_col0(m[0],
                               m[2]);
    NR::Point const xform_col1(m[1],
                               m[3]);
    NR::Point const xlate(m[4], m[5]);
    return ( Point(dot(v, xform_col0),
                   dot(v, xform_col1))
             + xlate );
#else
    return Point(v[X] * m[0]  +  v[Y] * m[2]  +  m[4],
                 v[X] * m[1]  +  v[Y] * m[3]  +  m[5]);
#endif
}

inline Point &Point::operator*=(Matrix const &m)
{
    *this = *this * m;
    return *this;
}

Matrix operator*(Matrix const &a, Matrix const &b);

inline Matrix operator*(Matrix const &m, rotate const &r)
{
    return m * Matrix(r);
}

inline Matrix operator*(Matrix const &m, translate const &t)
{
    Matrix ret(m);
    ret[4] += t[X];
    ret[5] += t[Y];
    assert_close( ret, m * Matrix(t) );
    return ret;
}

inline Matrix operator*(translate const &t, Matrix const &m)
{
    Matrix ret(m);
    ret[4] += m[0] * t[X] + m[2] * t[Y];
    ret[5] += m[1] * t[X] + m[3] * t[Y];
    assert_close( ret, Matrix(t) * m );
    return ret;
}

inline Matrix operator*(translate const &t, scale const &s)
{
    Matrix ret(s);
    ret[4] = t[X] * s[X];
    ret[5] = t[Y] * s[Y];
    assert_close( ret, Matrix(t) * Matrix(s) );
    return ret;
}

inline Matrix operator*(scale const &s, translate const &t)
{
    return Matrix(s) * t;
}

inline Matrix operator*(Matrix const &m, scale const &s)
{
    Matrix ret(m);
    ret[0] *= s[X]; ret[1] *= s[Y];
    ret[2] *= s[X]; ret[3] *= s[Y];
    ret[4] *= s[X]; ret[5] *= s[Y];
    assert_close( ret, m * Matrix(s) );
    return ret;
}

inline Matrix operator*(scale const &s, Matrix const &m)
{
    Matrix ret(m);
    ret[0] *= s[X];
    ret[1] *= s[X];
    ret[2] *= s[Y];
    ret[3] *= s[Y];
    assert_close( ret, Matrix(s) * m );
    return ret;
}

inline Matrix operator*(Matrix const &a, NRMatrix const &b)
{
    return a * NR::Matrix(b);
}

} /* namespace NR */


#endif /* !SEEN_NR_MATRIX_OPS_H */

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
