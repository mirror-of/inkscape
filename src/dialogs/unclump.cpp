#define __UNCLUMP_C__

/*
 * Unclumping objects
 *
 * Authors:
 *   bulia byak
 *
 * Copyright (C) 2005 Authors
 * Released under GNU GPL
 */

#include <glib.h>

#include <map>

#include "selection.h"
#include "sp-object.h"
#include "sp-item.h"

#include "libnr/nr-point.h"
#include "libnr/nr-point-ops.h"
#include "libnr/nr-point-fns.h"
#include "libnr/nr-rect.h"
#include "libnr/nr-matrix.h"
#include "libnr/nr-matrix-ops.h"
#include "libnr/nr-matrix-fns.h"

// Taking bbox of an item is an expensive operation, and we need to do it many times, so here we
// cache the centers, widths, and heights of items

//FIXME: make a class with these cashes as members instead of globals 
std::map<const gchar *, NR::Point> c_cache;
std::map<const gchar *, NR::Point> wh_cache;

/**
Center of bbox of item
*/
NR::Point
unclump_center (SPItem *item)
{
    std::map<const gchar *, NR::Point>::iterator i = c_cache.find(SP_OBJECT_ID(item));
    if ( i != c_cache.end() ) {
        return i->second;
    }

    NRRect r;
    sp_item_invoke_bbox(item, &r, sp_item_i2d_affine(item), TRUE);
    NR::Point c = NR::Point ((r.x0 + r.x1)/2, (r.y0 + r.y1)/2); 
    c_cache[SP_OBJECT_ID(item)] = c;
    return c; 
}


/**
Distance from item1 to the "edge" of item2. "Edge" is defined as that of an equal-area circle with
the same center. May be negative if the center of item1 is between the center and the edge of
item2.
*/
double
unclump_dist (SPItem *item1, SPItem *item2)
{
    NR::Point c1 = unclump_center (item1);
    NR::Point c2 = unclump_center (item2);

    double w;
    double h;
    std::map<const gchar *, NR::Point>::iterator i = wh_cache.find(SP_OBJECT_ID(item2));
    if ( i != wh_cache.end() ) {
        w = i->second[NR::X];
        h = i->second[NR::Y];
    } else {
        NRRect r2;
        sp_item_invoke_bbox(item2, &r2, sp_item_i2d_affine(item2), TRUE);
        w = fabs (r2.x1 - r2.x0);
        h = fabs (r2.y1 - r2.y0);
        wh_cache[SP_OBJECT_ID(item2)] = NR::Point(w, h);
    }

    double r = sqrt (w * h / M_PI); // the radius of the equal-area circle

    return (NR::L2 (c2 - c1) - r);
}

/**
Average unclump_dist from item to others
*/
double unclump_average (SPItem *item, GSList *others)
{
    int n = 0;
    double sum = 0;

    for (GSList *i = others; i != NULL; i = i->next) {
        SPItem *other = SP_ITEM (i->data);

        if (other == item)
            continue;

        n++;
        sum += unclump_dist (item, other);
    }

    if (n != 0)
        return sum/n;
    else
        return 0;
}

/**
Closest to item among others
 */
SPItem *unclump_closest (SPItem *item, GSList *others)
{
    double min = HUGE_VAL;
    SPItem *closest = NULL;

    for (GSList *i = others; i != NULL; i = i->next) {
        SPItem *other = SP_ITEM (i->data);

        if (other == item)
            continue;

        double dist = unclump_dist (item, other);
        if (dist < min && fabs (dist) < 1e6) {
            min = dist;
            closest = other;
        }
    }

    return closest;
}

/**
Most distant from item among others
 */
SPItem *unclump_farest (SPItem *item, GSList *others)
{
    double max = -HUGE_VAL;
    SPItem *farest = NULL;

    for (GSList *i = others; i != NULL; i = i->next) {
        SPItem *other = SP_ITEM (i->data);

        if (other == item)
            continue;

        double dist = unclump_dist (item, other);
        if (dist > max && fabs (dist) < 1e6) {
            max = dist;
            farest = other;
        }
    }

    return farest;
}

/**
Removes from the \a rest list those items that are "behind" \a closest as seen from \a item,
i.e. those on the other side of the line through \a closest perpendicular to the direction from \a
item to \a closest. Returns a newly created list which must be freed.
 */
GSList *
unclump_remove_behind (SPItem *item, SPItem *closest, GSList *rest)
{
    NR::Point it = unclump_center (item);
    NR::Point p1 = unclump_center (closest);

    // perpendicular through closest to the direction to item:
    NR::Point perp = NR::rot90(it - p1);
    NR::Point p2 = p1 + perp;

    // get the standard Ax + By + C = 0 form for p1-p2:
    double A = p1[NR::Y] - p2[NR::Y];
    double B = p2[NR::X] - p1[NR::X];
    double C = p2[NR::Y] * p1[NR::X] - p1[NR::Y] * p2[NR::X];

    // substitute the item into it:
    double val_item = A * it[NR::X] + B * it[NR::Y] + C;

    GSList *out = NULL;

    for (GSList *i = rest; i != NULL; i = i->next) {
        SPItem *other = SP_ITEM (i->data);

        if (other == item)
            continue;

        NR::Point o = unclump_center (other);
        double val_other = A * o[NR::X] + B * o[NR::Y] + C;

        if (val_item * val_other <= 1e-6) {
            // different signs, which means item and other are on the different sides of p1-p2 line; skip
        } else {
            out = g_slist_prepend (out, other);
        }
    }

    return out;
}

/**
Moves \a what away from \a from by \a dist
 */
void
unclump_push (SPItem *from, SPItem *what, double dist)
{
    NR::Point it = unclump_center (what);
    NR::Point p = unclump_center (from);
    NR::Point by = dist * NR::unit_vector (- (p - it));

    NR::Matrix move = NR::Matrix (NR::translate (by));

    std::map<const gchar *, NR::Point>::iterator i = c_cache.find(SP_OBJECT_ID(what));
    if ( i != c_cache.end() ) {
        i->second *= move;
    }

    //g_print ("push %s at %g,%g from %g,%g by %g,%g, dist %g\n", SP_OBJECT_ID(what), it[NR::X],it[NR::Y], p[NR::X],p[NR::Y], by[NR::X],by[NR::Y], dist);

    sp_item_set_i2d_affine(what, sp_item_i2d_affine(what) * move);
    sp_item_write_transform(what, SP_OBJECT_REPR(what), what->transform, NULL);
}

/**
Moves \a what towards \a to by \a dist
 */
void
unclump_pull (SPItem *to, SPItem *what, double dist)
{
    NR::Point it = unclump_center (what);
    NR::Point p = unclump_center (to);
    NR::Point by = dist * NR::unit_vector (p - it);

    NR::Matrix move = NR::Matrix (NR::translate (by));

    std::map<const gchar *, NR::Point>::iterator i = c_cache.find(SP_OBJECT_ID(what));
    if ( i != c_cache.end() ) {
        i->second *= move;
    }

    //g_print ("pull %s at %g,%g to %g,%g by %g,%g, dist %g\n", SP_OBJECT_ID(what), it[NR::X],it[NR::Y], p[NR::X],p[NR::Y], by[NR::X],by[NR::Y], dist);

    sp_item_set_i2d_affine(what, sp_item_i2d_affine(what) * move);
    sp_item_write_transform(what, SP_OBJECT_REPR(what), what->transform, NULL);
}


/**
Unclumps the items in \a items, reducing local unevenness in their distribution. Produces an effect
similar to "engraver dots". The only distribution which is unchanged by unclumping is a hexagonal
grid. May be called repeatedly for stronger effect. 
 */
void
unclump (GSList *items)
{
    c_cache.clear();
    wh_cache.clear();

    for (GSList *i = items; i != NULL; i = i->next) { //  for each original/clone x: 
        SPItem *item = SP_ITEM (i->data);

        GSList *nei = NULL;

        GSList *rest = g_slist_copy (items);
        rest = g_slist_remove (rest, item);

        while (rest != NULL) {
            SPItem *closest = unclump_closest (item, rest);
            if (closest) {
                nei = g_slist_prepend (nei, closest);
                rest = g_slist_remove (rest, closest);
                GSList *new_rest = unclump_remove_behind (item, closest, rest);
                g_slist_free (rest);
                rest = new_rest;
            } else {
                g_slist_free (rest);
                break;
            }
        } 

        if (g_slist_length (nei) >= 2) {
            double ave = unclump_average (item, nei);

            SPItem *closest = unclump_closest (item, nei);
            SPItem *farest = unclump_farest (item, nei);

            double dist_closest = unclump_dist (closest, item);
            double dist_farest = unclump_dist (farest, item);

            //g_print ("NEI %d for item %s    closest %s at %g  farest %s at %g  ave %g\n", g_slist_length(nei), SP_OBJECT_ID(item), SP_OBJECT_ID(closest), dist_closest, SP_OBJECT_ID(farest), dist_farest, ave);

            if (fabs (ave) < 1e6 && fabs (dist_closest) < 1e6 && fabs (dist_farest) < 1e6) { // otherwise the items are bogus
                // increase these coefficients to make unclumping more aggressive and less stable
                // the pull coefficient is a bit bigger to counteract the long-term expansion trend
                unclump_push (closest, item, 0.3 * (ave - dist_closest)); 
                unclump_pull (farest, item, 0.35 * (dist_farest - ave));
            }
        }
    }
}
