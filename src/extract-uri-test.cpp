#include <utest/utest.h>
#include "extract-uri.h"
#include <string.h>
#include <glib.h>

int main(int argc, char *argv[])
{
    utest_start("extract_uri");

    struct Case { char const *input; char const *exp; } const cases[] = {
        { "url(#foo)", "#foo" },
        { "url  foo  ", "foo" },
        { "url", NULL },
        { "url ", NULL },
        { "url()", NULL },
        { "url ( ) ", NULL },
        { "url foo bar ", "foo bar" }
    };

    for(unsigned i = 0; i < G_N_ELEMENTS(cases); ++i) {
        Case const &c = cases[i];
        char *p = NULL;
        UTEST_TEST(c.input) {
            p = extract_uri(c.input);
            UTEST_ASSERT( ( p == NULL ) == ( c.exp == NULL ) );
            if (p) {
                UTEST_ASSERT( strcmp(p, c.exp) == 0 );
            }
        }
        g_free(p);
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
