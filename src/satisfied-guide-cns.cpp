#include "satisfied-guide-cns.h"
#include <desktop.h>
#include <libnr/nr-point-fns.h>
#include <sp-guide.h>
#include <sp-guide-attachment.h>
#include <sp-guide-constraint.h>
#include <sp-namedview.h>
#include <approx-equal.h>

void satisfied_guide_cns(SPDesktop const &desktop,
                         int const n_snappoints, NR::Point const snappoints[],
                         std::vector<SPGuideConstraint> &cns)
{
    g_assert(SP_IS_DESKTOP(&desktop));
    SPNamedView const &nv = *desktop.namedview;
    for (GSList const *l = nv.guides; l != NULL; l = l->next) {
        SPGuide &g = *SP_GUIDE(l->data);
        for (int i = 0; i < n_snappoints; ++i) {
            if (approx_equal(dot(g.normal, snappoints[i]), g.position)) {
                cns.push_back(SPGuideConstraint(&g, i));
            }
        }
    }
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
