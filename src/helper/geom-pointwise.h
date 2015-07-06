/**
 * \file
 * \brief Pointwise a class to manage a vector of satellites per piecewise curve
 */ /*
    * Authors:
    * Jabiertxof
    * Johan Engelen
    * Josh Andler
    * suv
    * Mc-
    * Liam P. White
    * Nathan Hurst
    * Krzysztof Kosiński
    * This code is in public domain
    */

#ifndef SEEN_POINTWISE_H
#define SEEN_POINTWISE_H

#include <helper/geom-satellite.h>
#include <2geom/sbasis.h>
#include <2geom/sbasis-2d.h>
#include <2geom/piecewise.h>
#include <2geom/sbasis-to-bezier.h>
#include <2geom/path.h>
#include <boost/optional.hpp>

/**
 * @brief Pointwise a class to manage a vector of satellites per piecewise curve
 *
 * For the moment is a per curve satellite holder not per node. This is ok for
 * much cases but not a real node satellite on open paths
 * To implement this we can:
 * add extra satellite in open paths, and take notice of current open paths
 * or put extra satellites on back for each open subpath
 *
 * Also maybe the vector of satellites become a vector of
 * optional satellites, and remove the active variable in satellites.
 *
 */
typedef Geom::Piecewise<Geom::D2<Geom::SBasis> > pwd2sb;
class Pointwise {
public:
    pwd2sb getPwd2() const;
    void setPwd2(pwd2sb const &pwd2_in);

    std::vector<Satellite> getSatellites() const;
    void setSatellites(std::vector<Satellite> const &sats);
    void setStart();

    void recalculateForNewPwd2(pwd2sb const &A, Geom::PathVector const &B, Satellite const &S);
    void pwd2Subtract(pwd2sb const &A);
    void pwd2Append(pwd2sb const &A, Satellite const &S);
    void subpathToBack(size_t subpath);
    void subpathReverse(size_t start, size_t end);
    void insertDegenerateSatellites(pwd2sb const &A, Geom::PathVector const &B, Satellite const &S);

private:
    pwd2sb _pwd2;
    std::vector<Satellite> _satellites;
};

#endif //SEEN_POINTWISE_H
/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim:
// filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8:textwidth=99
// :
