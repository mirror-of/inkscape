// nr-types-test.h
#include <cxxtest/TestSuite.h>

#include "../utest/utest.h"
#include "libnr/nr-types.h"
#include "libnr/nr-point-fns.h"
#include <cmath>
using NR::Point;
using NR::X;
using NR::Y;

class NrTypesTest : public CxxTest::TestSuite
{
public:
    NrTypesTest() :
        a( 1.5, 2.0 ),
        b(-2.0, 3.0),
        ab(-0.5, 5.0),
        small(pow(2.0, -1070)),
        small_left(-small, 0.0),
        smallish_3_neg4(3.0 * small, -4.0 * small)
    {}
    virtual ~NrTypesTest() {}

// createSuite and destroySuite get us per-suite setup and teardown
// without us having to worry about static initialization order, etc.
    static NrTypesTest *createSuite() { return new NrTypesTest(); }
    static void destroySuite( NrTypesTest *suite ) { delete suite; }

    NR::Point const a;
    NR::Point const b;
    NR::Point const ab;
    double const small;
    Point const small_left;
    Point const smallish_3_neg4;


    void testXYValues( void )
    {
        TS_ASSERT_EQUALS( X, 0 );
        TS_ASSERT_EQUALS( Y, 1 );
    }

    void testXYCtorAndArrayConst(void)
    {
        TS_ASSERT_EQUALS( a[X], 1.5 );
        TS_ASSERT_EQUALS( a[Y], 2.0 );
    }

    void testCopyCtor(void)
    {
        NR::Point a_copy(a);

        TS_ASSERT_EQUALS( a, a_copy );
        TS_ASSERT( !(a != a_copy) );
    }

    void testNonConstArrayOperator(void)
    {
        NR::Point a_copy(a);
        a_copy[X] = -2.0;
        TS_ASSERT_DIFFERS( a_copy, a );
        TS_ASSERT_DIFFERS( a_copy, b );
        a_copy[Y] = 3.0;
        TS_ASSERT_EQUALS( a_copy, b );
    }

    void testBinaryPlusMinus(void)
    {
        TS_ASSERT_DIFFERS( a, b );
        TS_ASSERT_EQUALS( a + b, ab );
        TS_ASSERT_EQUALS( ab - a, b );
        TS_ASSERT_EQUALS( ab - b, a );
        TS_ASSERT_DIFFERS( ab + a, b );
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
