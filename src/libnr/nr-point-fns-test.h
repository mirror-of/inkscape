// nr-point-fns-test.h
#include <cassert>
#include <cmath>
#include <glib/gmacros.h>
#include <stdlib.h>

#include "utest/utest.h"
#include "libnr/nr-point-fns.h"
#include "isnan.h"

using NR::Point;

class NrPointFnsTest : public CxxTest::TestSuite
{
public:
    void testL1(void)
    {
        Point const p3n4(3.0, -4.0);
        Point const p0(0.0, 0.0);
        double const small = pow(2.0, -1070);
        double const inf = 1e400;
        double const nan = inf - inf;

        Point const small_left(-small, 0.0);
        Point const small_n3_4(-3.0 * small, 4.0 * small);
        Point const part_nan(3., nan);
        Point const inf_left(-inf, 5.0);

        assert(isNaN(nan));
        assert(!isNaN(small));

        TS_ASSERT_EQUALS( NR::L1(p0), 0.0  );
        TS_ASSERT_EQUALS( NR::L1(p3n4), 7.0  );
        TS_ASSERT_EQUALS( NR::L1(small_left), small  );
        TS_ASSERT_EQUALS( NR::L1(inf_left), inf  );
        TS_ASSERT_EQUALS( NR::L1(small_n3_4), 7.0 * small  );
        TS_ASSERT(isNaN(NR::L1(part_nan)));
    }

};

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=99 :
