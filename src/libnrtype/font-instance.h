#ifndef SEEN_LIBNRTYPE_FONT_INSTANCE_H
#define SEEN_LIBNRTYPE_FONT_INSTANCE_H

#include <ext/hash_map>
#include <pango/pango-types.h>
#include <pango/pango-font.h>
#include <require-config.h>
#ifdef WITH_XFT
//# include <freetype/freetype.h>
# include <ft2build.h>
# include FT_FREETYPE_H
#endif
#ifdef WIN32
# include <windows.h>
# include <windowsx.h>
#endif

#include <libnr/nr-forward.h>
#include <libnrtype/nrtype-forward.h>
#include <libnrtype/font-style.h>
#include <livarot/livarot-forward.h>

struct font_style_hash : public std::unary_function<font_style, size_t> {
    size_t operator()(font_style const &x) const;
};

struct font_style_equal : public std::binary_function<font_style, font_style, bool> {
    bool operator()(font_style const &a, font_style const &b);
};

class font_instance {
public:
    __gnu_cxx::hash_map<font_style, raster_font*, font_style_hash, font_style_equal>     loadedStyles;

    PangoFont*            pFont;
#if defined(WITH_XFT)
    FT_Face               theFace;
#elif defined(WIN32)
    LOGFONT*              theLogFont;
    HFONT                 wFont;
    OUTLINETEXTMETRIC     otm;
#endif
    PangoFontDescription* descr;
    int                   refCount;
    font_factory*         daddy;

    // common glyph definitions for all the rasterfonts
    __gnu_cxx::hash_map<int, int>     id_to_no;
    int                   nbGlyph, maxGlyph;
    font_glyph*           glyphs;

    font_instance(void);
    ~font_instance(void);

    void                 Ref(void);
    void                 Unref(void);

    bool                 IsOutlineFont(void);
    void                 InstallFace(PangoFont* iFace);

    int                  MapUnicodeChar(gunichar c);
    void                 LoadGlyph(int glyph_id);
    Path*                Outline(int glyph_id, Path *copyInto=NULL);
    void*                ArtBPath(int glyph_id);
    double               Advance(int glyph_id, bool vertical);
    bool                 FontMetrics(double &ascent, double &descent, double &leading);
    NR::Rect             BBox(int glyph_id);

    raster_font*         RasterFont(NR::Matrix const &trs, double stroke_width,
                                    bool vertical = false, JoinType stroke_join = join_straight,
                                    ButtType stroke_cap = butt_straight);
    raster_font*         RasterFont(font_style const &iStyle);
    void                 RemoveRasterFont(raster_font *who);

    unsigned             Name(gchar *str, unsigned size);
    unsigned             Family(gchar *str, unsigned size);
    unsigned             Attribute(gchar const *key, gchar *str, unsigned size);
};


#endif /* !SEEN_LIBNRTYPE_FONT_INSTANCE_H */

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
