/*
 *  RasterFont.h
 *  testICU
 *
 */

#ifndef my_raster_font
#define my_raster_font

#include <ext/hash_map>

#include <libnr/nr-forward.h>
#include <libnrtype/nrtype-forward.h>
#include <libnrtype/font-style.h>


class raster_font {
public:
    font_instance*                daddy;
    int                           refCount;

    font_style                    style;  

    __gnu_cxx::hash_map<int,int>             glyph_id_to_raster_glyph_no;
    int                           nbBase,maxBase;
    raster_glyph**                bases;

    raster_font(void);
    ~raster_font(void);
   
    void                          Unref(void);
    void                          Ref(void);

    NR::Point      Advance(int glyph_id);
    void           BBox(int glyph_id,NRRect *area);         

    raster_glyph*  GetGlyph(int glyph_id);
  
    void           LoadRasterGlyph(int glyph_id); // refreshes outline/polygon if needed
    void           RemoveRasterGlyph(raster_glyph* who);
  
};

#endif


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
