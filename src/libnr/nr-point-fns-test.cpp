#include <stdlib.h>
#include "../utest/utest.h"
#include <libnr/nr-point-fns.h>
#include <cmath>

using NR::Point;

int main(int argc, char *argv[])
{
    utest_start("nr-point-fns");

    Point const p3n4(3.0, -4.0);
    Point const p0(0.0, 0.0);
    double const small = pow(2.0, -1070);

    Point const small_left(-small, 0.0);
    Point const small_n3_4(-3.0 * small, 4.0 * small);

    UTEST_TEST("L1") {
        UTEST_ASSERT( NR::L1(p0) == 0.0 );
        UTEST_ASSERT( NR::L1(p3n4) == 7.0 );
        UTEST_ASSERT( NR::L1(small_left) == small );
        UTEST_ASSERT( NR::L1(small_n3_4) == 7.0 * small );
    }

    UTEST_TEST("L2") {
        UTEST_ASSERT( NR::L2(p0) == 0.0 );
        UTEST_ASSERT( NR::L2(p3n4) == 5.0 );
        UTEST_ASSERT( NR::L2(small_left) == small );
        UTEST_ASSERT( NR::L2(small_n3_4) == 5.0 * small );
    }

    UTEST_TEST("LInfty") {
        UTEST_ASSERT( NR::LInfty(p0) == 0.0 );
        UTEST_ASSERT( NR::LInfty(p3n4) == 4.0 );
        UTEST_ASSERT( NR::LInfty(small_left) == small );
        UTEST_ASSERT( NR::LInfty(small_n3_4) == 4.0 * small );
    }

    UTEST_TEST("atan2") {
        UTEST_ASSERT( NR::atan2(p3n4) == atan2(-4.0, 3.0) );
        UTEST_ASSERT( NR::atan2(small_left) == atan2(0.0, -1.0) );
        UTEST_ASSERT( NR::atan2(small_n3_4) == atan2(4.0, -3.0) );
    }

    UTEST_TEST("unit_vector") {
        UTEST_ASSERT( NR::unit_vector(p3n4) == Point(.6, -0.8) );
        UTEST_ASSERT( NR::unit_vector(small_left) == Point(-1.0, 0.0) );
        UTEST_ASSERT( NR::unit_vector(small_n3_4) == Point(-.6, 0.8) );
    }

    return ( utest_end()
             ? EXIT_SUCCESS
             : EXIT_FAILURE );
}

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
