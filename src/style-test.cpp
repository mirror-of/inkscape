#include <cassert>
#include <cstring>
#include <cmath>
#include "utest/utest.h"
#include "streq.h"
#include "strneq.h"
#include "style.h"

/* Extracted mechanically from http://www.w3.org/TR/SVG11/types.html#ColorKeywords:
 *
 *   tidy -wrap 999 < tmp/types.html 2> /dev/null |
 *     egrep '(prop|color-keyword)-value' |
 *     sed 's,<td><span class="prop-value">,    {",;s/<td><span class="color-keyword-value">rgb(/", {/;s%).*%}},@%;s%</span></td>%%' |
 *     tr -d \\n |
 *     tr @ \\n
 */
static struct {
    char const *color_keyword;
    struct {unsigned r; unsigned g; unsigned b; } rgb;
} const color_keywords[] = {
    {"aliceblue", {240, 248, 255}},
    {"antiquewhite", {250, 235, 215}},
    {"aqua", { 0, 255, 255}},
    {"aquamarine", {127, 255, 212}},
    {"azure", {240, 255, 255}},
    {"beige", {245, 245, 220}},
    {"bisque", {255, 228, 196}},
    {"black", { 0, 0, 0}},
    {"blanchedalmond", {255, 235, 205}},
    {"blue", { 0, 0, 255}},
    {"blueviolet", {138, 43, 226}},
    {"brown", {165, 42, 42}},
    {"burlywood", {222, 184, 135}},
    {"cadetblue", { 95, 158, 160}},
    {"chartreuse", {127, 255, 0}},
    {"chocolate", {210, 105, 30}},
    {"coral", {255, 127, 80}},
    {"cornflowerblue", {100, 149, 237}},
    {"cornsilk", {255, 248, 220}},
    {"crimson", {220, 20, 60}},
    {"cyan", { 0, 255, 255}},
    {"darkblue", { 0, 0, 139}},
    {"darkcyan", { 0, 139, 139}},
    {"darkgoldenrod", {184, 134, 11}},
    {"darkgray", {169, 169, 169}},
    {"darkgreen", { 0, 100, 0}},
    {"darkgrey", {169, 169, 169}},
    {"darkkhaki", {189, 183, 107}},
    {"darkmagenta", {139, 0, 139}},
    {"darkolivegreen", { 85, 107, 47}},
    {"darkorange", {255, 140, 0}},
    {"darkorchid", {153, 50, 204}},
    {"darkred", {139, 0, 0}},
    {"darksalmon", {233, 150, 122}},
    {"darkseagreen", {143, 188, 143}},
    {"darkslateblue", { 72, 61, 139}},
    {"darkslategray", { 47, 79, 79}},
    {"darkslategrey", { 47, 79, 79}},
    {"darkturquoise", { 0, 206, 209}},
    {"darkviolet", {148, 0, 211}},
    {"deeppink", {255, 20, 147}},
    {"deepskyblue", { 0, 191, 255}},
    {"dimgray", {105, 105, 105}},
    {"dimgrey", {105, 105, 105}},
    {"dodgerblue", { 30, 144, 255}},
    {"firebrick", {178, 34, 34}},
    {"floralwhite", {255, 250, 240}},
    {"forestgreen", { 34, 139, 34}},
    {"fuchsia", {255, 0, 255}},
    {"gainsboro", {220, 220, 220}},
    {"ghostwhite", {248, 248, 255}},
    {"gold", {255, 215, 0}},
    {"goldenrod", {218, 165, 32}},
    {"gray", {128, 128, 128}},
    {"grey", {128, 128, 128}},
    {"green", { 0, 128, 0}},
    {"greenyellow", {173, 255, 47}},
    {"honeydew", {240, 255, 240}},
    {"hotpink", {255, 105, 180}},
    {"indianred", {205, 92, 92}},
    {"indigo", { 75, 0, 130}},
    {"ivory", {255, 255, 240}},
    {"khaki", {240, 230, 140}},
    {"lavender", {230, 230, 250}},
    {"lavenderblush", {255, 240, 245}},
    {"lawngreen", {124, 252, 0}},
    {"lemonchiffon", {255, 250, 205}},
    {"lightblue", {173, 216, 230}},
    {"lightcoral", {240, 128, 128}},
    {"lightcyan", {224, 255, 255}},
    {"lightgoldenrodyellow", {250, 250, 210}},
    {"lightgray", {211, 211, 211}},
    {"lightgreen", {144, 238, 144}},
    {"lightgrey", {211, 211, 211}},
    {"lightpink", {255, 182, 193}},
    {"lightsalmon", {255, 160, 122}},
    {"lightseagreen", { 32, 178, 170}},
    {"lightskyblue", {135, 206, 250}},
    {"lightslategray", {119, 136, 153}},
    {"lightslategrey", {119, 136, 153}},
    {"lightsteelblue", {176, 196, 222}},
    {"lightyellow", {255, 255, 224}},
    {"lime", { 0, 255, 0}},
    {"limegreen", { 50, 205, 50}},
    {"linen", {250, 240, 230}},
    {"magenta", {255, 0, 255}},
    {"maroon", {128, 0, 0}},
    {"mediumaquamarine", {102, 205, 170}},
    {"mediumblue", { 0, 0, 205}},
    {"mediumorchid", {186, 85, 211}},
    {"mediumpurple", {147, 112, 219}},
    {"mediumseagreen", { 60, 179, 113}},
    {"mediumslateblue", {123, 104, 238}},
    {"mediumspringgreen", { 0, 250, 154}},
    {"mediumturquoise", { 72, 209, 204}},
    {"mediumvioletred", {199, 21, 133}},
    {"midnightblue", { 25, 25, 112}},
    {"mintcream", {245, 255, 250}},
    {"mistyrose", {255, 228, 225}},
    {"moccasin", {255, 228, 181}},
    {"navajowhite", {255, 222, 173}},
    {"navy", { 0, 0, 128}},
    {"oldlace", {253, 245, 230}},
    {"olive", {128, 128, 0}},
    {"olivedrab", {107, 142, 35}},
    {"orange", {255, 165, 0}},
    {"orangered", {255, 69, 0}},
    {"orchid", {218, 112, 214}},
    {"palegoldenrod", {238, 232, 170}},
    {"palegreen", {152, 251, 152}},
    {"paleturquoise", {175, 238, 238}},
    {"palevioletred", {219, 112, 147}},
    {"papayawhip", {255, 239, 213}},
    {"peachpuff", {255, 218, 185}},
    {"peru", {205, 133, 63}},
    {"pink", {255, 192, 203}},
    {"plum", {221, 160, 221}},
    {"powderblue", {176, 224, 230}},
    {"purple", {128, 0, 128}},
    {"red", {255, 0, 0}},
    {"rosybrown", {188, 143, 143}},
    {"royalblue", { 65, 105, 225}},
    {"saddlebrown", {139, 69, 19}},
    {"salmon", {250, 128, 114}},
    {"sandybrown", {244, 164, 96}},
    {"seagreen", { 46, 139, 87}},
    {"seashell", {255, 245, 238}},
    {"sienna", {160, 82, 45}},
    {"silver", {192, 192, 192}},
    {"skyblue", {135, 206, 235}},
    {"slateblue", {106, 90, 205}},
    {"slategray", {112, 128, 144}},
    {"slategrey", {112, 128, 144}},
    {"snow", {255, 250, 250}},
    {"springgreen", { 0, 255, 127}},
    {"steelblue", { 70, 130, 180}},
    {"tan", {210, 180, 140}},
    {"teal", { 0, 128, 128}},
    {"thistle", {216, 191, 216}},
    {"tomato", {255, 99, 71}},
    {"turquoise", { 64, 224, 208}},
    {"violet", {238, 130, 238}},
    {"wheat", {245, 222, 179}},
    {"white", {255, 255, 255}},
    {"whitesmoke", {245, 245, 245}},
    {"yellow", {255, 255, 0}},
    {"yellowgreen", {154, 205, 50}},
    {NULL, {0, 0, 0}}
};

static char const *const display_vals[] = {
    "inline", "block", "list-item", "run-in", "compact", "marker", "table", "inline-table",
    "table-row-group", "table-header-group", "table-footer-group", "table-row",
    "table-column-group", "table-column", "table-cell", "table-caption", "none", NULL
};

static char const *const font_stretch_vals[] = {
    "normal",
    "wider",
    "narrower",
    "ultra-condensed",
    "extra-condensed",
    "condensed",
    "semi-condensed",
    "semi-expanded",
    "expanded",
    "extra-expanded",
    "ultra-expanded",
    NULL
};

static char const *const font_style_vals[] = {"normal", "italic", "oblique", NULL};

static char const *const font_variant_vals[] = {"normal", "small-caps", NULL};

static char const *const font_weight_vals[] = {"normal", "bold", "bolder", "lighter",
                                               "100", "200", "300", "400", "500",
                                               "600", "700", "800", "900", NULL};

static char const *const normal_val[] = {"normal", NULL};

static char const *const none_val[] = {"none", NULL};

static char const *const linecap_vals[] = {"butt", "round", "square", NULL};

static char const *const linejoin_vals[] = {"miter", "round", "bevel", NULL};

static char const *const text_anchor_vals[] = {"start", "middle", "end", NULL};

static char const *const visibility_vals[] = {"visible", "hidden", "collapse", NULL};

static char const *const writing_mode_vals[] = {"lr-tb", "rl-tb", "tb-rl", /*"lr", "rl", "tb",*/ NULL};
/* TODO: Inkscape correctly accepts lr,rl,tb, but reports them as lr-tb etc.
   Either change inkscape or write custom test. */

static char const *const fill_rule_vals[] = {"nonzero", "evenodd", NULL};

static char const *const paint_enum_vals[] = {"none", "currentColor", NULL};

static gchar *
merge_then_write_string(gchar const *const str, guint const flags)
{
    SPStyle *const style = sp_style_new();
    sp_style_merge_from_style_string(style, str);
    gchar *const ret = sp_style_write_string(style, flags);
    sp_style_unref(style);
    return ret;
}

static void
enum_val(char const prop[], char const *const vals[])
{
    assert(vals);
    assert(vals[0]);

    for (unsigned i = 0; vals[i]; ++i) {
        char *prop_eq_val = g_strdup_printf("%s:%s", prop, vals[i]);
        gchar *ifset_str = merge_then_write_string(prop_eq_val, SP_STYLE_FLAG_IFSET);
        UTEST_ASSERT(streq(ifset_str, prop_eq_val));
        g_free(ifset_str);
        g_free(prop_eq_val);
    }
}

static void
color_val(char const prop[], char const *const dummy_vals[])
{
    assert(dummy_vals == NULL);
    char const *const extra_vals[] = {"#0000ff", NULL};
    enum_val(prop, extra_vals);
#if 0
    char const *color_vals[G_N_ELEMENTS(color_keywords) + G_N_ELEMENTS(extra_vals)];
    for (unsigned i = 0; i < G_N_ELEMENTS(extra_vals); ++i) {
        color_vals[i] = extra_vals[i];
    }
    for (unsigned i = 0; i < G_N_ELEMENTS(color_keywords); ++i) {
        color_vals[G_N_ELEMENTS(extra_vals) + i] = color_keywords[i].color_keyword;
    }
    enum_val(prop, color_vals);
    /* todo: other color stuff (rgb(), #123) */
#endif
}

static void
paint_val(char const prop[], char const *const dummy_vals[])
{
    /* Ref: http://www.w3.org/TR/SVG11/painting.html#SpecifyingPaint */
    assert(dummy_vals == NULL);
    color_val(prop, NULL);
    enum_val(prop, paint_enum_vals);
    /* todo: uri, `<uri> none' etc. */
}

static void
number_val(char const prop[], double const min, double const max, double const eps)
{
    assert(min <= max);
    /* TODO */
    double const propns[] = {0., 1e-7, 1e-5, 0.13, 1/7., 2/3., 10/11., 1.0};
    size_t const prop_len = std::strlen(prop);
    for (unsigned i = 0; i < G_N_ELEMENTS(propns); ++i) {
        double const propn = propns[i];
        double const val = ( propn == 0
                             ? min
                             : ( propn == 1
                                 ? max
                                 : min + (propns[i] * (max - min)) ) );
        char *prop_eq_val = g_strdup_printf("%s:%.17g", prop, val);
        gchar *ifset_str = merge_then_write_string(prop_eq_val, SP_STYLE_FLAG_IFSET);
        UTEST_ASSERT(strneq(ifset_str, prop_eq_val, prop_len + 1));
        char *endptr;
        double const found_val = g_strtod(ifset_str + prop_len + 1, &endptr);
        UTEST_ASSERT(*endptr == '\0');
        if (fabs(val) < 1.) {
            UTEST_ASSERT(std::fabs(found_val - val) < eps);
        } else {
            UTEST_ASSERT(std::fabs(found_val / val - 1.) < eps);
        }
        g_free(ifset_str);
        g_free(prop_eq_val);
    }
}

static void
miterlimit_val(char const prop[], char const *const dummy_vals[])
{
    // Ref: http://www.w3.org/TR/SVG11/painting.html#StrokeMiterlimitProperty
    assert(dummy_vals == NULL);
    number_val(prop, 1., 20., 2e-7);
#if 0
    static char const *const miterlimit_vals[] = {
        "1", "1.0", "1.5", "2", "4", "8", "1e1", NULL
    };
    // bad values: <1, percentages, strings (none etc.).
#endif
}

static void
font_family_val(char const prop[], char const *const dummy_vals[])
{
    assert(dummy_vals == NULL);
    static char const *const generic_font_family_vals[] = {"serif", "sans-serif", "cursive", "fantasy", "monospace", NULL};
    enum_val(prop, generic_font_family_vals);
    /* todo: unrecognized fonts, comma-separated lists. */
}

static void
length_val(char const prop[], char const *const dummy_vals[])
{
    assert(dummy_vals == NULL);
    number_val(prop, 0., 6., 1e-7);
    /* todo: units */
    /* todo: exponential notation */
}

static void
length_or_enum_val(char const prop[], char const *const vals[])
{
    length_val(prop, NULL);
    enum_val(prop, vals);
}

static void
opacity_val(char const prop[], char const *const dummy_vals[])
{
    assert(dummy_vals == NULL);
    number_val(prop, 0., 1., 1e-7);
     /* todo: exponential notation */
}

static void
uri_or_enum_val(char const prop[], char const *const vals[])
{
    enum_val(prop, vals);
    /* todo: uri's */
}

static bool
test_style()
{
    struct {
        char const *property;
        char const *ink_dfl;
        char const *spec_dfl;
        void (*tst_fn)(char const[], char const *const[]);
        char const *const *tst_fn_arg;
        bool can_explicitly_inherit;
    } const props[] = {
        {"color", "#000000", "#000000", color_val, NULL, true},
        // initial value "depends on user agent"
        {"display", "inline", "inline", enum_val, display_vals, true},
        {"fill", "#000000", "black", paint_val, NULL, true},
        {"fill-opacity", "1.0000000", "1", opacity_val, NULL, true},
        {"fill-rule", "nonzero", "nonzero", enum_val, fill_rule_vals, true},
        {"font-family", "Bitstream Vera Sans", "Bitstream Vera Sans", font_family_val, NULL, true},
        // initial value depends on user agent
        {"font-size", "medium", "medium", length_val, NULL, true},
        // TODO: abs, rel, pcnt.
        {"font-stretch", "normal", "normal", enum_val, font_stretch_vals, true},
        {"font-style", "normal", "normal", enum_val, font_style_vals, true},
        {"font-variant", "normal", "normal", enum_val, font_variant_vals, true},
        {"font-weight", "normal", "normal", enum_val, font_weight_vals, true},
        {"letter-spacing", "normal", "normal", length_or_enum_val, normal_val, true},
        {"marker", "none", "none", uri_or_enum_val, none_val, true},
        {"marker-end", "none", "none", uri_or_enum_val, none_val, true},
        {"marker-mid", "none", "none", uri_or_enum_val, none_val, true},
        {"marker-start", "none", "none", uri_or_enum_val, none_val, true},
        {"opacity", "1.0000000", "1", opacity_val, NULL, true},
        {"stroke", "none", "none", paint_val, NULL, true},
        {"stroke-dasharray", "none", "none", enum_val, none_val, true},
        // TODO: http://www.w3.org/TR/SVG11/painting.html#StrokeDasharrayProperty
        //{"stroke-dashoffset", "0", "0", length_val, NULL, true},
        // fixme: dashoffset currently fails a number of tests, but the relevant code
        // is being worked on for something else at the time of writing, so I'm
        // delaying fixing it.  It should be changed to SPILength.
        {"stroke-linecap", "butt", "butt", enum_val, linecap_vals, true},
        {"stroke-linejoin", "miter", "miter", enum_val, linejoin_vals, true},
        {"stroke-miterlimit", "4.0000000", "4", miterlimit_val, NULL, true},
        {"stroke-opacity", "1.0000000", "1", opacity_val, NULL, true},
        {"stroke-width", "1.0000000", "1", length_val, NULL, true},
        {"text-anchor", "start", "start", enum_val, text_anchor_vals, true},
        {"visibility", "visible", "visible", enum_val, visibility_vals, true},
        {"writing-mode", "lr-tb", "lr-tb", enum_val, writing_mode_vals, true}
    };

    char const str0_all_exp[] = "font-size:medium;font-style:normal;font-variant:normal;font-weight:normal;font-stretch:normal;opacity:1.0000000;color:#000000;fill:#000000;fill-opacity:1.0000000;fill-rule:nonzero;stroke:none;stroke-width:1.0000000;stroke-linecap:butt;stroke-linejoin:miter;marker:none;marker-start:none;marker-mid:none;marker-end:none;stroke-miterlimit:4.0000000;stroke-dasharray:none;stroke-dashoffset:0;stroke-opacity:1.0000000;visibility:visible;display:inline;font-family:Bitstream Vera Sans;letter-spacing:normal;text-anchor:start;writing-mode:lr-tb";

    utest_start("style");
    UTEST_TEST("sp_style_new, sp_style_write_string") {
        SPStyle *style = sp_style_new();
        gchar *str0_all = sp_style_write_string(style, SP_STYLE_FLAG_ALWAYS);
        gchar *str0_set = sp_style_write_string(style, SP_STYLE_FLAG_IFSET);
        UTEST_ASSERT(*str0_set == '\0');
        UTEST_ASSERT(streq(str0_all, str0_all_exp));
        g_free(str0_all);
        g_free(str0_set);
        sp_style_unref(style);
    }

    UTEST_TEST("sp_style_merge_from_style_string(whitespace): ifset") {
        gchar *str0_set = merge_then_write_string("   \t            \t\t", SP_STYLE_FLAG_IFSET);
        UTEST_ASSERT(*str0_set == '\0');
        g_free(str0_set);
    }

    UTEST_TEST("sp_style_merge_from_style_string(whitespace): always") {
        gchar *str0_all = merge_then_write_string("   \t            \t\t", SP_STYLE_FLAG_ALWAYS);
        UTEST_ASSERT(streq(str0_all, str0_all_exp));
        g_free(str0_all);
    }

#if 0 /* fails because of dashoffset:0 vs dashoffset:0.00000000 */
    UTEST_TEST("sp_style_merge_from_style_string(default): ifset") {
        gchar *ifset_str = merge_then_write_string(str0_all_exp, SP_STYLE_FLAG_IFSET);
        UTEST_ASSERT(streq(ifset_str, str0_all_exp));
        g_free(ifset_str);
    }

    UTEST_TEST("sp_style_merge_from_style_string(default): always") {
        gchar *ifset_str = merge_then_write_string(str0_all_exp, SP_STYLE_FLAG_ALWAYS);
        UTEST_ASSERT(streq(ifset_str, str0_all_exp));
        g_free(ifset_str);
    }
#endif

    UTEST_TEST("sp_style_merge_from_style_string") {
        /* Try setting default values, check that the all string is unaffected
           but that the ifset string is affected. */
        for (unsigned i = 0; i < G_N_ELEMENTS(props); ++i) {
            char *prop_eq_val = g_strdup_printf("%s:%s", props[i].property, props[i].spec_dfl);
            char *exp_set_str = g_strdup_printf("%s:%s", props[i].property, props[i].ink_dfl);
            gchar *str0_all = merge_then_write_string(prop_eq_val, SP_STYLE_FLAG_ALWAYS);
            gchar *str0_set = merge_then_write_string(prop_eq_val, SP_STYLE_FLAG_IFSET);
            UTEST_ASSERT(streq(str0_all, str0_all_exp));
            UTEST_ASSERT(streq(str0_set, exp_set_str));
            g_free(str0_set);
            g_free(str0_all);
            g_free(exp_set_str);
            g_free(prop_eq_val);
        }

        /* Check that explicit `inherit' is correctly preserved by write(ifset). */
        for (unsigned i = 0; i < G_N_ELEMENTS(props); ++i) {
            if (!props[i].can_explicitly_inherit) {
                continue;
            }
            char *prop_eq_val = g_strdup_printf("%s:inherit", props[i].property);
            gchar *ifset_str = merge_then_write_string(prop_eq_val, SP_STYLE_FLAG_IFSET);
            UTEST_ASSERT(streq(ifset_str, prop_eq_val));
            g_free(ifset_str);
            g_free(prop_eq_val);
        }

        for (unsigned i = 0; i < G_N_ELEMENTS(props); ++i) {
            props[i].tst_fn(props[i].property, props[i].tst_fn_arg);
        }
    }

    return utest_end();
}

int main()
{
    return ( test_style()
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
