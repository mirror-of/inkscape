#ifndef INKSCAPE_LPE_DASH_STROKE_H
#define INKSCAPE_LPE_DASH_STROKE_H

/*
 * Inkscape::LPEDashStroke
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "live_effects/effect.h"
#include "live_effects/parameter/enum.h"
#include "live_effects/parameter/text.h"
#include "live_effects/parameter/message.h"

namespace Inkscape {
namespace LivePathEffect {

enum DashAdjustArraymethod {
    DD_NONE,
    DD_STRETCH,
    DD_STRETCH_DASHES,
    DD_STRETCH_GAPS,
    DD_COMPRESS,
    DD_COMPRESS_DASHES,
    DD_COMPRESS_GAPS,
    DD_END
};

class LPEDashStroke : public Effect {
public:
    LPEDashStroke(LivePathEffectObject *lpeobject);
    ~LPEDashStroke() override;
    void doBeforeEffect (SPLPEItem const* lpeitem) override;
    Geom::PathVector doEffect_path (Geom::PathVector const & path_in) override;
    double timeAtLength(double const A, Geom::Path const &segment);
    double timeAtLength(double const A, Geom::Piecewise<Geom::D2<Geom::SBasis> > pwd2);
private:
    EnumParam< DashAdjustArraymethod> dash_adjust;
    TextParam stroke_dasharray;
    TextParam stroke_dashcorner;
    TextParam stroke_dashofset;
    MessageParam message;
};

} //namespace LivePathEffect
} //namespace Inkscape
#endif
