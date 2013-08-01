#ifndef INKSCAPE_LPE_PTS_TO_ELLIPSE_H
#define INKSCAPE_LPE_PTS_TO_ELLIPSE_H

/** \file
 * LPE "Points to Ellipse" implementation
 */

/*
 * Authors:
 *   Markus Schwienbacher
 *
 * Copyright (C) Markus Schwienbacher 2013 <mschwienbacher@gmail.com>
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "live_effects/effect.h"
#include "live_effects/parameter/bool.h"
// #include "live_effects/parameter/parameter.h"
// #include "live_effects/parameter/point.h"

namespace Inkscape {
  namespace LivePathEffect {

    class LPEPts2Ellipse : public Effect {
    public:
      LPEPts2Ellipse(LivePathEffectObject *lpeobject);
      virtual ~LPEPts2Ellipse();

      virtual std::vector<Geom::Path> doEffect_path (std::vector<Geom::Path> const & path_in);

    // protected:
    //   virtual void addCanvasIndicators(SPLPEItem const *lpeitem, std::vector< Geom::PathVector > &hp_vec);

    private:
      LPEPts2Ellipse(const LPEPts2Ellipse&);
      LPEPts2Ellipse& operator=(const LPEPts2Ellipse&);
      

      int genIsometricEllipse (std::vector<Geom::Point> const & points_in,
                               std::vector<Geom::Path> & path_out);

      int  genFitEllipse (std::vector<Geom::Point> const & points_in,
                          std::vector<Geom::Path> & path_out);

      BoolParam only_circle;
      BoolParam gen_isometric;
      BoolParam gen_isometric_frame;
      BoolParam gen_arc;
      BoolParam other_arc;
      BoolParam slice_arc;
      BoolParam draw_axes;
      ScalarParam rot_axes;
      BoolParam draw_ori_path;

      // Geom::Path helper_path;
      std::vector<Geom::Point> points;
    };

  } //namespace LivePathEffect
} //namespace Inkscape

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
