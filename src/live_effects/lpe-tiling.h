// SPDX-License-Identifier: GPL-2.0-or-later
#ifndef INKSCAPE_LPE_TILING_H
#define INKSCAPE_LPE_TILING_H

/** \file
 * LPE <tiling> implementation, see lpe-tiling.cpp.
 * Released under GNU GPL v2+, read the file 'COPYING' for more information.
 */

#include "live_effects/effect.h"
#include "live_effects/lpegroupbbox.h"
#include "live_effects/parameter/enum.h"
#include "live_effects/parameter/parameter.h"
#include "live_effects/parameter/point.h"
#include "live_effects/parameter/satellitearray.h"
#include "live_effects/parameter/text.h"
#include "live_effects/parameter/unit.h"
#include "live_effects/parameter/random.h"
// this is only to fillrule
#include "livarot/Shape.h"

namespace Inkscape {
namespace LivePathEffect {

namespace CoS {
// we need a separate namespace to avoid clashes with other LPEs
class KnotHolderEntityCopyGapX;
class KnotHolderEntityCopyGapY;
}

typedef FillRule FillRuleBool;

class LPETiling : public Effect, GroupBBoxEffect {
public:
    LPETiling(LivePathEffectObject *lpeobject);
    ~LPETiling() override;
    void doOnApply (SPLPEItem const* lpeitem) override;
    Geom::PathVector doEffect_path (Geom::PathVector const & path_in) override;
    void doBeforeEffect (SPLPEItem const* lpeitem) override;
    void doAfterEffect (SPLPEItem const* lpeitem, SPCurve *curve) override;
    void split(Geom::PathVector &path_in, Geom::Path const &divider);
    void resetDefaults(SPItem const* item) override;
    void doOnRemove (SPLPEItem const* /*lpeitem*/) override;
    bool doOnOpen(SPLPEItem const * /*lpeitem*/) override;
    void doOnVisibilityToggled(SPLPEItem const* /*lpeitem*/) override;
    Gtk::Widget * newWidget() override;
    void cloneStyle(SPObject *orig, SPObject *dest);
    Geom::PathVector doEffect_path_post (Geom::PathVector const & path_in, FillRuleBool fillrule);
    SPItem * toItem(size_t i, bool reset, bool &write);
    void cloneD(SPObject *orig, SPObject *dest);
    Inkscape::XML::Node * createPathBase(SPObject *elemref);
    friend class CoS::KnotHolderEntityCopyGapX;
    friend class CoS::KnotHolderEntityCopyGapY;
    void addKnotHolderEntities(KnotHolder * knotholder, SPItem * item) override;
protected:
    void addCanvasIndicators(SPLPEItem const *lpeitem, std::vector<Geom::PathVector> &hp_vec) override;
    KnotHolder *_knotholder;
    double gapx_unit = 0;
    double gapy_unit = 0;
    double offset_unit = 0;
private:
    void setOffsetCols();
    void setOffsetRows();
    void setScaleInterpolate(bool x, bool y);
    void setRotateInterpolate(bool x, bool y);
    void setScaleRandom();
    void setRotateRandom();
    void setGapXMode(bool random);
    void setGapYMode(bool random);
    bool getActiveMirror(gint index);
    double end_scale(double scale_fix, bool tomax) const;
    bool _updating = false;
    void setMirroring(gint index);
    Glib::ustring getMirrorMap(gint index);
    void generate_buttons(Gtk::Box *container, Gtk::RadioButton::Group &group, gint pos);
    UnitParam unit;
    SatelliteArrayParam lpesatellites;
    ScalarParam gapx;
    ScalarParam gapy;
    ScalarParam num_rows;
    ScalarParam num_cols;
    ScalarParam rotate;
    ScalarParam scale;
    ScalarParam offset;
    BoolParam offset_type;
    BoolParam random_scale;
    BoolParam random_rotate;
    BoolParam random_gap_x;
    BoolParam random_gap_y;
    RandomParam seed;
    BoolParam interpolate_scalex;
    BoolParam interpolate_rotatex;
    BoolParam interpolate_scaley;
    BoolParam interpolate_rotatey;
    BoolParam mirrorrowsx;
    BoolParam mirrorrowsy;
    BoolParam mirrorcolsx;
    BoolParam mirrorcolsy;
    BoolParam mirrortrans;
    BoolParam split_items;
    BoolParam link_styles;
    BoolParam shrink_interp;
    double original_width = 0;
    double original_height = 0;
    Geom::OptRect gap_bbox;
    Geom::OptRect originalbbox;
    double prev_num_cols;
    double prev_num_rows;
    bool reset;
    gdouble scaleok = 1.0;
    Glib::ustring prev_unit = "px";
    std::vector<double> random_x;
    std::vector<double> random_y;
    std::vector<double> random_s;
    std::vector<double> random_r;
    Geom::Affine affinebase = Geom::identity();
    bool prev_split = false;
    SPObject *container;
    LPETiling(const LPETiling&) = delete;
    LPETiling& operator=(const LPETiling&) = delete;
};

} //namespace LivePathEffect
} //namespace Inkscape

#endif

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-gaps:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :
