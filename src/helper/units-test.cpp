#include <math.h>

#include <helper/sp-intl.h>
#include <helper/units.h>
#include <utest/utest.h>


/* N.B. Wrongly returns false if both near 0.  (Not a problem for current users.) */
static bool
approx_equal(double const x, double const y)
{
    return fabs(x / y - 1) < 1e-15;
}


int
main(int argc, char *argv[])
{
    utest_start("sp_units_get_points, sp_points_get_units");

    struct Case { double x; char const *abbr; double pts; } const tests[] = {
        { 1.0, "pt", 1.0 },
        { 5.0, "pt", 5.0 },
        { 1.0, "in", 72.0 },
        { 2.0, "in", 144.0 },
        { 254., "mm", 720.0 },
        { 254., "cm", 7200. },
        { 254., "m", 720000. },
        { 1.5, "mm", (15 * 72. / 254) }
    };
    for (unsigned i = 0; i < G_N_ELEMENTS(tests); ++i) {
        char name[80];
        Case const &c = tests[i];
        SPUnit const &unit = *sp_unit_get_by_abbreviation(N_(c.abbr));

        double const calc_pts = sp_units_get_points(c.x, &unit);
        snprintf(name, sizeof(name), "%.1f %s -> %.1f pt", c.x, c.abbr, c.pts);
        UTEST_TEST(name) {
            UTEST_ASSERT(approx_equal(calc_pts, c.pts));
        }

        double const calc_x = sp_points_get_units(c.pts, &unit);
        snprintf(name, sizeof(name), "%.1f pt -> %.1f %s", c.pts, c.x, c.abbr);
        UTEST_TEST(name) {
            UTEST_ASSERT(approx_equal(calc_x, c.x));
        }

        double tmp = c.x;
        bool const converted_to_pts = sp_convert_distance(&tmp, &unit, SP_PS_UNIT);
        snprintf(name, sizeof(name), "convert %.1f %s -> %.1f pt", c.x, c.abbr, c.pts);
        UTEST_TEST(name) {
            UTEST_ASSERT(converted_to_pts);
            UTEST_ASSERT(approx_equal(tmp, c.pts));
        }

        tmp = c.pts;
        bool const converted_from_pts = sp_convert_distance(&tmp, SP_PS_UNIT, &unit);
        snprintf(name, sizeof(name), "convert %.1f pt -> %.1f %s", c.pts, c.x, c.abbr);
        UTEST_TEST(name) {
            UTEST_ASSERT(converted_from_pts);
            UTEST_ASSERT(approx_equal(tmp, c.x));
        }
    }

    return ( utest_end()
             ? EXIT_SUCCESS
             : EXIT_FAILURE );
}


/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :

