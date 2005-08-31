/*
 * A class for handling shape interaction with libavoid.
 *
 * Authors:
 *   Michael Wybrow <mjwybrow@users.sourceforge.net>
 *
 * Copyright (C) 2005 Michael Wybrow
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */


#include <vector>

#include "sp-item.h"
#include "libnr/nr-point.h"
#include "libnr/nr-rect.h"
#include "libnr/nr-convex-hull.h"
#include "conn-avoid-ref.h"
#include "libnr/nr-point-matrix-ops.h"
#include "libnr/nr-rect-ops.h"
#include "libavoid/polyutil.h"
#include "libavoid/incremental.h"


static Avoid::Polygn avoid_item_poly(SPItem const *item);
static void avoid_item_move(NR::Matrix const *mp, SPItem *moved_item);


SPAvoidRef::SPAvoidRef(SPItem *spitem)
    : shapeRef(NULL)
    , item(spitem)
    , setting(false)
    , _transformed_connection()
{
}


SPAvoidRef::~SPAvoidRef()
{
    _transformed_connection.disconnect();
    if (shapeRef) {
        // shapeRef is finalised by delShape,
        // so no memory is lost here.
        Avoid::delShape(shapeRef);
        shapeRef = NULL;
    }
}


void SPAvoidRef::setAvoid(char const *value)
{
    new_setting = false;
    if (value && (strcmp(value, "true") == 0)) {
        new_setting = true;
    }
}

void SPAvoidRef::handleSettingChange(void)
{
    if (new_setting == setting) {
        // Don't need to make any changes
        return;
    }

    _transformed_connection.disconnect();
    if (new_setting) {
        _transformed_connection = item->connectTransformed(
                sigc::ptr_fun(&avoid_item_move));

        Avoid::Polygn poly = avoid_item_poly(item);
        if (poly.pn > 0) {
            // Get a unique ID for the item.
            GQuark itemID = g_quark_from_string(SP_OBJECT(item)->id);

            shapeRef = new Avoid::ShapeRef(itemID, poly);
            Avoid::freePoly(poly);
        
            Avoid::addShape(shapeRef);
        }
    }
    else
    {
        g_assert(shapeRef);
        
        // shapeRef is finalised by delShape,
        // so no memory is lost here.
        Avoid::delShape(shapeRef);
        shapeRef = NULL;
    }
    setting = new_setting;
}


static Avoid::Polygn avoid_item_poly(SPItem const *item)
{
    Avoid::Polygn poly;

    std::vector<NR::Point> p;
    sp_item_snappoints(item, SnapPointsIter(p));
   
    NR::Matrix i2d = sp_item_i2d_affine(item);
    std::vector<NR::Point>::iterator i = p.begin(); 
    NR::ConvexHull cvh(*i * i2d);
    for (++i; i != p.end(); ++i) {
        cvh.add(*i * i2d); 
    }
   
    // TODO: NR::ConvexHull just keeps a bounding box for the convex
    //       hull, rather than a real convex hull.  So we're using
    //       a reactangle for each shape but we'll want to switch to
    //       using the convex hull of individual shapes.
    
    NR::Rect rHull = cvh.bounds();
    // Add a little buffer around the edge of each object.
    NR::Rect rExpandedHull = NR::expand(rHull, -10.0); 
    poly = Avoid::newPoly(4);
    
    for (int n = 0; n < 4; ++n) {
        // TODO: I think the winding order in libavoid or inkscape might
        //       be backwards, probably due to the inverse y co-ordinates
        //       used for the screen.  The '3 - n' reverses the order.
        poly.ps[n].x = rExpandedHull.corner(3 - n)[NR::X];
        poly.ps[n].y = rExpandedHull.corner(3 - n)[NR::Y];
    }
    
    return poly;
}


static void avoid_item_move(NR::Matrix const *mp, SPItem *moved_item)
{
    Avoid::ShapeRef *shapeRef = moved_item->avoidRef->shapeRef;
    g_assert(shapeRef);

    Avoid::Polygn poly = avoid_item_poly(moved_item);
    if (poly.pn > 0) {
        shapeRef = Avoid::moveShape(shapeRef, &poly);
        Avoid::freePoly(poly);
    }
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
