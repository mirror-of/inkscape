#include "attributes.h"
#include "inkscape-private.h"
#include "sp-gradient.h"
#include "sp-object.h"
#include "libnr/nr-matrix.h"
#include "libnr/nr-matrix-fns.h"
#include "libnr/nr-matrix-ops.h"
#include "libnr/nr-rect.h"
#include "libnr/nr-rotate-fns.h"
#include "svg/svg.h"
#include "utest/utest.h"
#include "xml/repr.h"

static NRRect copy_of(NR::Rect const &src)
{
    NRRect dest;
    using NR::X; using NR::Y;
    dest.x0 = src.min()[X];
    dest.x1 = src.max()[X];
    dest.y0 = src.min()[Y];
    dest.y1 = src.max()[Y];
    return dest;
}

static bool
test_gradient()
{
    utest_start("gradient");
    UTEST_TEST("init") {
        SPGradient *gr = static_cast<SPGradient *>(g_object_new(SP_TYPE_GRADIENT, NULL));
        UTEST_ASSERT(gr->gradientTransform.test_identity());
        UTEST_ASSERT(gr->gradientTransform == NR::identity());
        g_object_unref(gr);
    }

    /* Create the global inkscape object. */
    static_cast<void>(g_object_new(inkscape_get_type(), NULL));

    SPDocument *doc = static_cast<SPDocument *>(g_object_new(SP_TYPE_DOCUMENT, NULL));
    inkscape_add_document(doc);

    UTEST_TEST("sp_object_set(\"gradientTransform\")") {
        SPGradient *gr = static_cast<SPGradient *>(g_object_new(SP_TYPE_GRADIENT, NULL));
        SP_OBJECT(gr)->document = doc;
        sp_object_set(SP_OBJECT(gr), SP_ATTR_GRADIENTTRANSFORM, "translate(5, 8)");
        UTEST_ASSERT(gr->gradientTransform == NR::Matrix(NR::translate(5, 8)));
        sp_object_set(SP_OBJECT(gr), SP_ATTR_GRADIENTTRANSFORM, "");
        UTEST_ASSERT(gr->gradientTransform == NR::identity());
        sp_object_set(SP_OBJECT(gr), SP_ATTR_GRADIENTTRANSFORM, "rotate(90)");
        UTEST_ASSERT(gr->gradientTransform == NR::Matrix(rotate_degrees(90)));
        g_object_unref(gr);
    }

    UTEST_TEST("write") {
        SPGradient *gr = static_cast<SPGradient *>(g_object_new(SP_TYPE_GRADIENT, NULL));
        SP_OBJECT(gr)->document = doc;
        sp_object_set(SP_OBJECT(gr), SP_ATTR_GRADIENTTRANSFORM, "matrix(0, 1, -1, 0, 0, 0)");
        SPRepr *repr = sp_repr_new("radialGradient");
        SP_OBJECT(gr)->updateRepr(repr, SP_OBJECT_WRITE_ALL);
        {
            gchar const *tr = sp_repr_attr(repr, "gradientTransform");
            NR::Matrix svd;
            bool const valid = sp_svg_transform_read(tr, &svd);
            UTEST_ASSERT(valid);
            UTEST_ASSERT(svd == NR::Matrix(rotate_degrees(90)));
        }
        g_object_unref(gr);
    }

    UTEST_TEST("get_g2d") {
        SPGradient *gr = static_cast<SPGradient *>(g_object_new(SP_TYPE_GRADIENT, NULL));
        SP_OBJECT(gr)->document = doc;
        NR::Rect const unit_rect(NR::Point(0, 0), NR::Point(1, 1));
        NRRect const unit_nrrect(copy_of(unit_rect));
        {
            NRMatrix nr_g2d;
            sp_gradient_get_g2d_matrix_f(gr, &NR_MATRIX_IDENTITY, &unit_nrrect, &nr_g2d);
            NR::Matrix const g2d(sp_gradient_get_g2d_matrix(gr, NR::identity(), unit_rect));
            UTEST_ASSERT(g2d == NR::Matrix(nr_g2d));
            UTEST_ASSERT(nr_matrix_test_identity(&nr_g2d, 1e-14));
            UTEST_ASSERT(g2d == NR::identity());
        }

        NR::Matrix const funny(2, 3,
                               4, 5,
                               6, 7);
        NRMatrix const nr_funny = funny;
        UTEST_ASSERT(nr_funny[3] == 5);
        {
            NRMatrix nr_g2d;
            sp_gradient_get_g2d_matrix_f(gr, &nr_funny, &unit_nrrect, &nr_g2d);
            NR::Matrix const g2d(sp_gradient_get_g2d_matrix(gr, funny, unit_rect));
            UTEST_ASSERT(g2d == NR::Matrix(nr_g2d));
            UTEST_ASSERT(g2d == funny);
            UTEST_ASSERT(NR_MATRIX_DF_TEST_CLOSE(&nr_g2d, &nr_funny, 1e-14));
        }

        NR::Rect const larger_rect(NR::Point(5, 6), NR::Point(8, 10));
        NRRect const larger_nrrect(copy_of(larger_rect));
        {
            NRMatrix nr_g2d;
            sp_gradient_get_g2d_matrix_f(gr, &nr_funny, &larger_nrrect, &nr_g2d);
            NR::Matrix const g2d(sp_gradient_get_g2d_matrix(gr, funny, larger_rect));
            UTEST_ASSERT(g2d == NR::Matrix(nr_g2d));
            UTEST_ASSERT(g2d == NR::Matrix(3, 0,
                                           0, 4,
                                           5, 6) * funny);
            sp_object_set(SP_OBJECT(gr), SP_ATTR_GRADIENTUNITS, "userSpaceOnUse");
            sp_gradient_get_g2d_matrix_f(gr, &nr_funny, &larger_nrrect, &nr_g2d);
            NR::Matrix const user_g2d(sp_gradient_get_g2d_matrix(gr, funny, larger_rect));
            UTEST_ASSERT(user_g2d == NR::Matrix(nr_g2d));
            UTEST_ASSERT(user_g2d == funny);
        }
        g_object_unref(gr);
    }
    /* TODO: get/set gs2d matrix. */

    return utest_end();
}

int main()
{
    g_type_init();
    return ( test_gradient()
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
