#ifndef SEEN_LIBNRTYPE_RASTER_POSITION_H
#define SEEN_LIBNRTYPE_RASTER_POSITION_H

#include <libnr/nr-forward.h>
#include <libnrtype/nrtype-forward.h>
#include <livarot/livarot-forward.h>


class raster_position {
public:
    int               top, bottom; // baseline is y=0
    int*              run_on_line; // array of size (bottom-top+1): run_on_line[i] gives the number of runs on line top+i
    int               nbRun;
    float_ligne_run*  runs;

public:
    raster_position();
    ~raster_position();

    void AppendRuns(int add, float_ligne_run *src, int y);

    void Blit(float ph, int pv, NRPixBlock &over);
};


#endif /* !SEEN_LIBNRTYPE_RASTER_POSITION_H */

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
