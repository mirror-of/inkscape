#ifndef CANVAS_NGONGRID_H
#define CANVAS_NGONGRID_H

/*
 * Authors:
 *    Matthew Woehlke <mw_triad@users.sourceforge.net>
 *    Johan Engelen <j.b.c.engelen@alumnus.utwente.nl>
 *
 * Copyright (C) 2006-2012 Authors
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "line-snapper.h"
#include "canvas-grid.h"

class  SPCanvasBuf;
class  SPDesktop;
struct SPNamedView;

namespace Inkscape {
namespace XML {
    class Node;
};

class CanvasNGonGrid : public CanvasGrid {
public:
    CanvasNGonGrid(SPNamedView * nv, Inkscape::XML::Node * in_repr, SPDocument * in_doc);
    virtual ~CanvasNGonGrid();

    void Update (Geom::Affine const &affine, unsigned int flags);
    void Render (SPCanvasBuf *buf);

    void readRepr();
    void onReprAttrChanged (Inkscape::XML::Node * repr, const gchar *key, const gchar *oldval, const gchar *newval, bool is_interactive);

    int sections;         /**< Number of grid sections */
    double lengthx;       /**< Step size along concentric polygons */
    double lengthy;       /**< Step size along semi-radius lines */
    double angle_deg;     /**< Angle of rotation (degrees) */
    double angle_rad;     /**< Angle of rotation (radians) */
    double se_angle_deg;  /**< Half of section arc (degrees) */
    double se_angle_rad;  /**< Half of section arc (radians) */
    double se_tan;        /**< tan(se_angle) */

    bool scaled;          /**< Whether the grid is in scaled mode */

protected:
    friend class CanvasNGonGridSnapper;

    Geom::Point ow;         /**< Transformed origin by the affine for the zoom */
    double lxw;             /**< Transformed length x by the affine for the zoom */
    double lyw;             /**< Transformed length y by the affine for the zoom */

    Geom::Point sw;          /**< the scaling factors of the affine transform */

    virtual Gtk::Widget * newSpecificWidget();

private:
    CanvasNGonGrid(const CanvasNGonGrid&);
    CanvasNGonGrid& operator=(const CanvasNGonGrid&);

    void updateWidgets();

    void renderSection (SPCanvasBuf *buf, double section_angle_deg, guint32 _empcolor);
};



class CanvasNGonGridSnapper : public LineSnapper
{
public:
    CanvasNGonGridSnapper(CanvasNGonGrid *grid, SnapManager *sm, Geom::Coord const d);
    bool ThisSnapperMightSnap() const;

    Geom::Coord getSnapperTolerance() const; //returns the tolerance of the snapper in screen pixels (i.e. independent of zoom)
    bool getSnapperAlwaysSnap() const; //if true, then the snapper will always snap, regardless of its tolerance

private:
    LineList _getSnapLines(Geom::Point const &p) const;
    void _addSnappedLine(IntermSnapResults &isr, Geom::Point const snapped_point, Geom::Coord const snapped_distance, SnapSourceType const &source, long source_num, Geom::Point const normal_to_line, const Geom::Point point_on_line) const;
    void _addSnappedPoint(IntermSnapResults &isr, Geom::Point const snapped_point, Geom::Coord const snapped_distance, SnapSourceType const &source, long source_num, bool constrained_snap) const;
    void _addSnappedLinePerpendicularly(IntermSnapResults &isr, Geom::Point const snapped_point, Geom::Coord const snapped_distance, SnapSourceType const &source, long source_num, bool constrained_snap) const;

    CanvasNGonGrid *grid;
};


}; //namespace Inkscape



#endif


