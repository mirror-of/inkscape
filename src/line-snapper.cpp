#include "libnr/nr-values.h"
#include "libnr/nr-point-fns.h"
#include "geom.h"
#include "line-snapper.h"
#include "snap.h"

Inkscape::LineSnapper::LineSnapper(SPNamedView const *nv, NR::Coord const d) : Snapper(nv, d)
{

}

NR::Coord Inkscape::LineSnapper::do_free_snap(NR::Point &req, std::list<SPItem const *> const &it) const
{
    NR::Point result = req;
	
    NR::Coord const dh = do_vector_snap(result, component_vectors[NR::X], it);
    result[NR::Y] = req[NR::Y];
    NR::Coord const dv = do_vector_snap(result, component_vectors[NR::Y], it);
    req = result;
	
    if (dh < NR_HUGE && dv < NR_HUGE) {
        return hypot(dh, dv);
    }

    if (dh < NR_HUGE) {
        return dh;
    }

    if (dv < NR_HUGE) {
	return dv;
    }
    
    return NR_HUGE;
}

NR::Coord Inkscape::LineSnapper::do_vector_snap(NR::Point &req, NR::Point const &d,
                                                std::list<SPItem const *> const &it) const
{
    NR::Point const v = NR::unit_vector(d);

    /* Set to the snapped point, if we snap */
    NR::Point snapped = req;
    /* Distance to best snap point */
    NR::Coord best = NR_HUGE;
    /* Current upper limit for an acceptable snap */
    NR::Coord upper = getDistance();

    /* Get the lines that we will try to snap to */
    const LineList s = get_snap_lines(req);

    for (LineList::const_iterator i = s.begin(); i != s.end(); i++) {

        NR::Point trial(req);

        /* Normal to the line we're trying to snap along */
        NR::Point const n2(NR::rot90(v));

        /* Hence constant term of the line we're trying to snap along */
        NR::Coord const d2 = dot(n2, req);

        /* Try to intersect this line with the target line */
        if (intersector_line_intersection(n2, d2, i->first, i->second, trial) == INTERSECTS) {
            const NR::Coord dist = L2(trial - req);
            if (dist < upper) {
                upper = best = dist;
                snapped = trial;
            }
        }
    }

    req = snapped;
    return best;
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
