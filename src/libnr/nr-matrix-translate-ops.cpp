#include "libnr/nr-matrix-translate-ops.h"
#include "libnr/nr-matrix-ops.h"

namespace NR {

Matrix
operator*(Matrix const &m, translate const &t)
{
    Matrix ret(m);
    ret[4] += t[X];
    ret[5] += t[Y];
    assert_close( ret, m * Matrix(t) );
    return ret;
}

};
