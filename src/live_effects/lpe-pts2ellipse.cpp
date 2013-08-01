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

#include "live_effects/lpe-pts2ellipse.h"

#include "sp-shape.h"
#include "sp-item.h"
#include "sp-path.h"
#include "sp-item-group.h"
#include "svg/svg.h"
// #include "ui/widget/scalar.h"
#include "display/curve.h"

#include <2geom/path.h>
#include <2geom/circle.h>
#include <2geom/ellipse.h>
#include <2geom/pathvector.h>
#include <2geom/svg-elliptical-arc.h>
// #include <2geom/path-intersection.h> // for winding()

#include <glib/gi18n.h>

using namespace Geom;

namespace Inkscape {
namespace LivePathEffect {

LPEPts2Ellipse::LPEPts2Ellipse(LivePathEffectObject *lpeobject) :
    Effect(lpeobject),
    only_circle(_("_Only circle"), _("Fits points to circle instead of ellipse"), "only_circle", &wr, this, false),
    gen_isometric(_("_Isometric from parallelogram"), _("Takes the first three points, and fits it into the parallelogram.\nIdeal for Isometric views"), "gen_isometric", &wr, this, false),
    gen_isometric_frame(_("_Parallelogram frame (isometric)"), _("Draw Parallelogram around the ellipse"), "gen_isometric_frame", &wr, this, false),
    gen_arc(_("_Arc"), _("Generate open arc (open ellipse)"), "gen_arc", &wr, this, false),
    other_arc(_("_Other Arc side"), _("switch sides of the arc"), "arc_other", &wr, this, false),
    slice_arc(_("_Slice Arc"), _("slice the arc"), "slice_arc", &wr, this, false),
    draw_axes(_("A_xes"), _("Draw both semi-major and semi-minor axes"), "draw_axes", &wr, this, false),
    rot_axes(_("Axes Rotation"), _("Axes rotation angle"), "rot_axes", &wr, this, 0),
    draw_ori_path(_("Source _Path"), _("Show the original source path"), "draw_ori_path", &wr, this, false)
{
    registerParameter( dynamic_cast<Parameter *>(&only_circle) );
    registerParameter( dynamic_cast<Parameter *>(&gen_isometric) );
    registerParameter( dynamic_cast<Parameter *>(&gen_isometric_frame) );
    registerParameter( dynamic_cast<Parameter *>(&gen_arc) );
    registerParameter( dynamic_cast<Parameter *>(&other_arc) );
    registerParameter( dynamic_cast<Parameter *>(&slice_arc) );
    registerParameter( dynamic_cast<Parameter *>(&draw_axes) );
    registerParameter( dynamic_cast<Parameter *>(&rot_axes) );
    registerParameter( dynamic_cast<Parameter *>(&draw_ori_path) );

    rot_axes.param_set_range(-360,360);
    rot_axes.param_set_increments(1,10);
}

LPEPts2Ellipse::~LPEPts2Ellipse()
{
}

// helper function, transforms a given value into range [0, 2pi]
inline double
range2pi(double a)
{
    a = fmod(a, 2*M_PI);
    if(a<0) a+=2*M_PI;
    return a;
}

inline double
deg2rad(double a)
{
    return a*M_PI/180.0;
}

inline double
rad2deg(double a)
{
    return a*180.0/M_PI;
}

// helper function, calculates the angle between a0 and a1 in ccw sense
// examples: 0..1->1, -1..1->2, pi/4..-pi/4->1.5pi
// full rotations: 0..2pi->2pi, -pi..pi->2pi, pi..-pi->0, 2pi..0->0
inline double
calc_delta_angle(const double a0, const double a1)
{
    double da=range2pi(a1-a0);
    if((fabs(da)<1e-9) && (a0<a1))
        da=2*M_PI;
    return da;
}

int
unit_arc_path(Geom::Path &path, Geom::Affine &affine,
              double start=0.0, double end=2*M_PI, // angles
              bool slice=false)
{
    //
    double Da = calc_delta_angle(start,end);
    if (fabs(Da) < 1e-9) {
        g_warning("angle was 0");
        return -1;
    }

    // the delta angle
    double da=M_PI_2;
    // number of segments with da length
    int nda=(int)ceil(Da/M_PI_2);
    // recalculate da
    da=Da/(double)nda;

    bool closed=false;
    if (fabs(Da - 2*M_PI) < 1e-8) {
        closed = true;
        da=M_PI_2;
        nda=4;
    }

    start = range2pi(start);
    end=start+Da;

    // adopted from: sp-ellipse.cpp
    SPCurve * curve=new SPCurve();
    // start point
    curve->moveto(cos(start), sin(start));
    double s = start;
    for (int i=0; i < nda; s = (++i)*da+start) {
        double e = s + da;
        if (e > end)
            e = end;
        const double len = 4*tan((e - s)/4)/3;
        const double x0 = cos(s);
        const double y0 = sin(s);
        const double x1 = x0 + len * cos(s + M_PI_2);
        const double y1 = y0 + len * sin(s + M_PI_2);
        const double x3 = cos(e);
        const double y3 = sin(e);
        const double x2 = x3 + len * cos(e - M_PI_2);
        const double y2 = y3 + len * sin(e - M_PI_2);
        // #ifdef ELLIPSE_VERBOSE
        //               g_print("step %d s %f e %f coords %f %f %f %f %f %f\n",
        //                       i, s, e, x1, y1, x2, y2, x3, y3);
        // #endif
        curve->curveto(x1,y1, x2,y2, x3,y3);
    }

    if (slice && !closed) {
        curve->lineto(0., 0.);
    }
    curve->transform(affine);

    path.append(*curve->first_path());
    if ((slice && !closed) || closed) {
        path.close(true);
    }
    // give to GC
    curve->unref();
    return 0;
}

void
gen_axes_paths(Geom::PathVector &path_out, const Geom::Affine &affine)//, const double &rot_axes)
{
    // Coord sa,ca;
    // sincos(deg2rad(rot_axes),sa,ca);
    // LineSegment clx(Point(-ca,-sa),Point(ca,sa));
    // LineSegment cly(Point(sa,-ca),Point(-sa,ca));

    LineSegment clx(Point(-1,0),Point(1,0));
    LineSegment cly(Point(0,-1),Point(0,1));

    Path plx, ply;
    plx.append(clx);
    ply.append(cly);
    plx*=affine;
    ply*=affine;

    path_out.push_back(plx);
    path_out.push_back(ply);
}

bool
is_ccw(const std::vector<Geom::Point> & pts)
{
    // method: sum up the angles between edges
    size_t n=pts.size();
    // edges about vertex 0
    Point e0=pts.front()-pts.back();
    Point e1=pts.front()-pts[1];
    Coord sum=cross(e0,e1);
    // the rest
    for(size_t i=1;i<n;i++) {
        e0=e1;
        e1=pts[i]-pts[i-1];
        sum+=cross(e0,e1);
    }
    // edges about last vertex (closing)
    e0=e1;
    e1=pts.front()-pts.back();
    sum+=cross(e0,e1);

    // close the
    if(sum<0)
        return true;
    else
        return false;
}

void swapCoords(Geom::Coord &a0, Geom::Coord &a1)
{
    Geom::Coord tmp=a0;
    a0=a1;
    a1=tmp;
}


/**
 * Generates an ellipse (or circle) from the vertices of a given path.  Thereby, using fitting
 * algorithms from 2geom. Depending on the settings made by the user regarding things like arc,
 * slice, circle etc. the final result will be different
 */
std::vector<Geom::Path>
LPEPts2Ellipse::doEffect_path (std::vector<Geom::Path> const & path_in)
{
    PathVector path_out;

    if(draw_ori_path.get_value()){
        path_out.insert(path_out.end(),path_in.begin(),path_in.end());
    }


    // from: extension/internal/odf.cpp
    // get all points
    std::vector<Point> pts;
    for(PathVector::const_iterator pit = path_in.begin(); pit!= path_in.end(); ++pit) {
        // extract first point of this path
        pts.push_back(pit->initialPoint());
        // iterate over all curves
        for (Path::const_iterator cit = pit->begin(); cit != pit->end(); ++cit) {
            pts.push_back(cit->finalPoint());
        }
    }

    // avoid identical start-point and end-point
    if(pts.front() == pts.back()) {
        pts.pop_back();
    }

    // special mode: Use first two edges, interpret them as two sides of a parallelogram and
    // generate an ellipse residing inside the parallelogram. This effect is quite useful when
    // generating isometric views. Hence, the name.
    if(gen_isometric.get_value())
    {
        if(0!=genIsometricEllipse (pts, path_out))
            return path_in;
    }
    else
    {
        if(0!=genFitEllipse(pts, path_out))
            return path_in;
    }
    return path_out;
}


/**
 * Generates an ellipse (or circle) from the vertices of a given path. Thereby, using fitting
 * algorithms from 2geom. Depending on the settings made by the user regarding things like arc,
 * slice, circle etc. the final result will be different. We need at least 5 points to fit an
 * ellipse. With 5 points each point is on the ellipse. For less points we get a circle.
 */
int
LPEPts2Ellipse::genFitEllipse (std::vector<Geom::Point> const & pts,
                               std::vector<Geom::Path> & path_out)
{
    // rotation angle based on user provided rot_axes to position the vertices
    const double rot_angle=deg2rad(rot_axes);
    if(pts.size()<2) {
        return -1;
    } else if(pts.size()==2) {
        // simple line: circle in the middle of the line to the vertices
        Point line=pts.front()-pts.back();
        double radius=line.length()*0.5;
        if(radius<1e-9)
            return -1;
        Point center=middle_point(pts.front(),pts.back());
        Circle circle(center[0],center[1],radius);
        Affine affine;
        affine*=Rotate(rot_angle);
        affine*=Scale(circle.ray());
        affine*=Translate(circle.center());
        Path path;
        unit_arc_path(path,affine);
        path_out.push_back(path);
    } else if(pts.size()>=5 && !only_circle.get_value()) {
        // do ellipse
        try {
            Ellipse ellipse(pts);
            Affine affine;
            affine*=Rotate(rot_angle);
            affine*=Scale(ellipse.ray(X),ellipse.ray(Y));
            affine*=Rotate(ellipse.rot_angle());
            affine*=Translate(ellipse.center());

            Coord a0=0;
            Coord a1=2*M_PI;

            if(gen_arc.get_value()) {
                Affine inv_affine=affine.inverse();

                Point p0=pts.front()*inv_affine;
                Point p1=pts.back()*inv_affine;
                if(!p0.isZero() && !p1.isZero()) {
                    a0=atan2(p0);
                    a1=atan2(p1);

                    const bool ccw_wind=is_ccw(pts);

                    if(!ccw_wind) {
                        swapCoords(a0,a1);
                    }
                    if(other_arc.get_value()) {
                        swapCoords(a0,a1);
                    }
                }
            }

            Path path;
            unit_arc_path(path,affine,a0,a1,slice_arc.get_value());
            path_out.push_back(path);

            if(draw_axes.get_value()) {
                gen_axes_paths(path_out,affine);
            }
        } catch(...) {
            return -1;
        }
    } else {
        // do a circle (3,4 points, or only_circle set)
        try {
            Circle circle(pts);
            Affine affine;
            // affine*=Rotate(rot_angle);
            affine*=Scale(circle.ray());
            affine*=Translate(circle.center());

            Coord a0=0.0;
            Coord a1=2*M_PI;

            if(gen_arc.get_value())
            {
                Point p0=pts.front()-circle.center();
                Point p1=pts.back()-circle.center();
                if(!p0.isZero() && !p1.isZero()) {
                    a0=atan2(p0);
                    a1=atan2(p1);

                    const bool ccw_wind=is_ccw(pts);
                    // printf("p0=[%e, %e], p1=[%e, %e]\n",p0[0],p0[1],p1[0],p1[1]);
                    // printf("a0=%f, a1=%f, have ccw winding: %d\n",a0,a1,ccw_wind);

                    if(!ccw_wind) {
                        swapCoords(a0,a1);
                    }
                    if(other_arc.get_value()) {
                        swapCoords(a0,a1);
                    }
                }
            }
            Path path;
            unit_arc_path(path,affine,a0,a1,slice_arc.get_value());
            path_out.push_back(path);
        } catch(...) {
            return -1;
        }
    }
    return 0;
}

int
LPEPts2Ellipse::genIsometricEllipse (std::vector<Geom::Point> const & pts,
                                     std::vector<Geom::Path> & path_out)

{
    // take the first 3 vertices for the edges
    if(pts.size() < 3) return -1;
    // calc edges
    Point e0=pts[0]-pts[1];
    Point e1=pts[2]-pts[1];

    Coord ce=cross(e0,e1);
    // parallel or one is zero?
    if(fabs(ce)<1e-9) return -1;
    // unit vectors along edges
    Point u0=unit_vector(e0);
    Point u1=unit_vector(e1);
    // calc angles
    Coord a0=atan2(e0);
    // Coord a1=M_PI_2-atan2(e1)-a0;
    Coord a1=acos(dot(u0,u1))-M_PI_2;
    // if(fabs(a1)<1e-9) return -1;
    if(ce<0) a1=-a1;
    // lengths: l0= length of edge 0; l1= height of parallelogram
    Coord l0=e0.length()*0.5;
    Point e0n=e1-dot(u0,e1)*u0;
    Coord l1=e0n.length()*0.5;
    // Coord l1=e1.length()*0.5;
    // printf("angles [%f %f] pi, diameters [%f, %f]\n",a0/M_PI,a1/M_PI,l0,l1);

    // center of the ellipse
    Point pos=pts[1]+0.5*(e0+e1);

    // rotation angle based on user provided rot_axes to position the vertices
    const double rot_angle=deg2rad(rot_axes);

    // build up the affine transformation
    Affine affine;
    affine*=Rotate(rot_angle);
    affine*=Scale(l0,l1);
    affine*=HShear(tan(a1));
    affine*=Rotate(a0);
    affine*=Translate(pos);

    Path path;
    unit_arc_path(path,affine);
    path_out.push_back(path);

    if(gen_isometric_frame.get_value()) {
        Path rect;
        SPCurve curve;
        // unit rectangle
        curve.moveto(-1, -1);
        curve.lineto(1, -1);
        curve.lineto(1, 1);
        curve.lineto(-1, 1);
        curve.transform(Rotate(-rot_angle)*affine);
        rect.append(*curve.first_path());
        rect.close(true);
        path_out.push_back(rect);
    }

    if(draw_axes.get_value()) {
        gen_axes_paths(path_out,affine);
    }

    return 0;
}

// void
// LPEPts2Ellipse::addCanvasIndicators(const SPLPEItem */*lpeitem*/, std::vector<Geom::PathVector> &hp_vec)
// {
//   printf("Hallo LPEPts2Ellipse::addCanvasIndicators!\n");
//   SPCurve curve;
//   curve.moveto(points.front());
//   const size_t n=points.size();
//   for(size_t i;i<n;i++)
//     curve.lineto(points[i]);
//   Path path;
//   path.append(*curve.first_path());
//   PathVector pathv;
//   pathv.push_back(path);
//   hp_vec.push_back(pathv);
// }


/* ######################## */

} //namespace LivePathEffect
} /* namespace Inkscape */

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
