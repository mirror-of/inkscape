#ifndef SEEN_LIBNRTYPE_RASTER_GLYPH_H
#define SEEN_LIBNRTYPE_RASTER_GLYPH_H

#include <libnr/nr-forward.h>
#include <libnrtype/nrtype-forward.h>
#include <livarot/livarot-forward.h>


class raster_glyph {
public:
    raster_font*      daddy;
    int               glyph_id;

    Path*             outline;  // transformed by the matrix in style (may be factorized, but is small)
    Shape*            polygon;

    int               nb_sub_pixel;
    raster_position*  sub_pixel;

    raster_glyph(void);
    ~raster_glyph(void);

    void      SetSubPixelPositionning(int nb_pos);
    void      LoadSubPixelPosition(int no);

    void      Blit(NR::Point const &at, NRPixBlock &over); // alpha only
};


#endif /* !SEEN_LIBNRTYPE_RASTER_GLYPH_H */

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
