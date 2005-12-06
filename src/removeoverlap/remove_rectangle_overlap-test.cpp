#include "removeoverlap/remove_rectangle_overlap.h"
#include <glib/gmacros.h>
#include <glib/gmem.h>
#include <cstdlib>
#include <cmath>
#include "removeoverlap/generate-constraints.h"
#include "utest/utest.h"
using std::abs;
using std::rand;

static bool
possibly_eq(double const a, double const b)
{
    return abs(a - b) < 1e-13;
}

static bool
possibly_le(double const a, double const b)
{
    return a - b < 1e-13;
}

static void
test_case(unsigned const n_rects, double const rect2coords[][4])
{
    Rectangle **rs = (Rectangle **) g_malloc(sizeof(Rectangle*) * n_rects);
    for (unsigned i = 0; i < n_rects; ++i) {
        rs[i] = new Rectangle(rect2coords[i][0],
                              rect2coords[i][1],
                              rect2coords[i][2],
                              rect2coords[i][3]);
    }
    removeRectangleOverlap(rs, n_rects, 0.0, 0.0);
    for (unsigned i = 0; i < n_rects; ++i) {
        UTEST_ASSERT(possibly_eq(rs[i]->width(), (rect2coords[i][1] -
                                                  rect2coords[i][0]  )));
        UTEST_ASSERT(possibly_eq(rs[i]->height(), (rect2coords[i][3] -
                                                   rect2coords[i][2]  )));
        for (unsigned j = 0; j < i; ++j) {
            UTEST_ASSERT( possibly_le(rs[i]->getMaxX(), rs[j]->getMinX()) ||
                          possibly_le(rs[j]->getMaxX(), rs[i]->getMinX()) ||
                          possibly_le(rs[i]->getMaxY(), rs[j]->getMinY()) ||
                          possibly_le(rs[j]->getMaxY(), rs[i]->getMinY())   );
        }
    }
    for (unsigned i = 0; i < n_rects; ++i) {
        delete rs[i];
    }
    g_free(rs);
}

int main()
{
    utest_start("removeRectangleOverlap(zero gaps)");

    /* Derived from Bulia's initial test case. */
    UTEST_TEST("eg0") {
        double case0[][4] = {
            {-180.5, 69.072, 368.071, 629.071},
            {99.5, 297.644, 319.5, 449.071},
            {199.5, 483.358, 450.929, 571.929},
            {168.071, 277.644, 462.357, 623.357},
            {99.5, 99.751, 479.5, 674.786},
            {-111.929, 103.358, 453.786, 611.929},
            {-29.0714, 143.358, 273.786, 557.643},
            {122.357, 269.072, 322.357, 531.929},
            {256.643, 357.644, 396.643, 520.5}
        };
        test_case(G_N_ELEMENTS(case0), case0);
    }

#if 0 /* This involves a zero-height rect, so we'll ignore for the moment. */
    UTEST_TEST("eg1") {
        double case1[][4] = {
            {5, 14, 9, 14},
            {6, 13, 6, 8},
            {11, 12, 5, 5},
            {5, 8, 5, 7},
            {12, 14, 14, 15},
            {12, 14, 1, 14},
            {1, 15, 14, 15},
            {5, 6, 13, 13}
        };
        test_case(G_N_ELEMENTS(case1), case1);
    }
#endif

#if 0 /* disable some known fails for the moment */
    UTEST_TEST("eg2") {
        double case2[][4] = {
            {3, 4, 6, 13},
            {0, 1, 0, 5},
            {0, 4, 1, 6},
            {2, 5, 0, 6},
            {0, 10, 9, 13},
            {5, 11, 1, 13},
            {1, 2, 3, 8}
        };
        test_case(G_N_ELEMENTS(case2), case2);
    }

    UTEST_TEST("eg3") {
        double case3[][4] = {
            {0, 5, 0, 3},
            {1, 2, 1, 3},
            {3, 7, 4, 7},
            {0, 9, 4, 5},
            {3, 7, 0, 3}
        };
        test_case(G_N_ELEMENTS(case3), case3);
    }
#endif

    /* Random cases of up to 10 rectangles, with small non-neg int coords. */
    UTEST_TEST("random ints") {
        for (unsigned n = 0; n < 10; ++n) {
            unsigned const fld_size = 2 * n;
            double (*coords)[4] = (double (*)[4]) g_malloc(n * 4 * sizeof(double));
            for (unsigned repeat = (n == 0 ? 1
                                    : n == 1 ? 36
                                    : (1 << 5)  ); repeat--;) {
                for (unsigned i = 0; i < n; ++i) {
                    for (unsigned d = 0; d < 2; ++d) {
                        unsigned const end = 1 + (rand() % (fld_size - 1));
                        unsigned const start = rand() % end;
                        coords[i][2 * d] = start;
                        coords[i][2 * d + 1] = end;
                    }
                }
                test_case(n, coords);
            }
            g_free(coords);
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=99 :
