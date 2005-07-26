#include <cxxtest/TestSuite.h>
#include "streq.h"
#include "svg/stringstream.h"

#if 0
inline void
TS_ASSERT_STREQ(char const *const a, char const *const b)
{
    if (!streq(a, b)) {
        std::ostringstream s;
        s << "Expected `" << a << "' to equal `" << b << "'";
        TS_FAIL(s.str().cstr());
    }
}
#endif

#if 0
struct cstr_equal : public std::binary_function<char const *, char const *, bool>
{
    bool operator()(char const *const a, char const *const b) const
    {
        return streq(a, b);
    }
};
#endif

template<typename T>
static void
test_datum(T const x, std::string const &exp_str)
{
    Inkscape::SVGOStringStream s;
    s << x;
    TS_ASSERT_EQUALS(s.str(), exp_str);
}

static void
test_float(float const x, std::string const &exp_str)
{
    test_datum(x, exp_str);
    test_datum((double) x, exp_str);
}

class StringStreamTest : public CxxTest::TestSuite
{
public:
    void testFloats()
    {
        test_float(4.5, "4.5");
        test_float(4.0, "4");
        test_float(0.0, "0");
        test_float(-3.75, "-3.75");
        test_float(-2.0625, "-2.0625");
        test_float(-0.0625, "-0.0625");
        test_float(30.0, "30");
        test_float(12345678.0, "12345678");
        test_float(3e9, "3e+09");
        test_float(-3.5e9, "-3.5e+09");
        test_float(32768e9, "3.2768e+13");
        test_float(-10.5, "-10.5");
    }

    void testOtherTypes()
    {
        test_datum('3', "3");
        test_datum('x', "x");
        test_datum((unsigned char) '$', "$");
        test_datum((signed char) 'Z', "Z");
        test_datum("  my string  ", "  my string  ");
        test_datum((signed char const *) "023", "023");
        test_datum((unsigned char const *) "023", "023");
    }

    void testConcat()
    {
        Inkscape::SVGOStringStream s;
        s << "hello, ";
        s << -53.5;
        TS_ASSERT_EQUALS(s.str(), std::string("hello, -53.5"));
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
