#include "livarot/path-description.h"

PathDescr *PathDescrMoveTo::clone() const
{
    return new PathDescrMoveTo(*this);
}

void PathDescrMoveTo::dumpSVG(Inkscape::SVGOStringStream& s, NR::Point const &last) const
{
    s << "M " << p[0] << " " << p[1] << " ";
}

void PathDescrLineTo::dumpSVG(Inkscape::SVGOStringStream& s, NR::Point const &last) const
{
    s << "L " << p[0] << " " << p[1] << " ";
}

PathDescr *PathDescrLineTo::clone() const
{
    return new PathDescrLineTo(*this);
}

PathDescr *PathDescrBezierTo::clone() const
{
    return new PathDescrBezierTo(*this);
}

PathDescr *PathDescrIntermBezierTo::clone() const
{
    return new PathDescrIntermBezierTo(*this);
}

void PathDescrCubicTo::dumpSVG(Inkscape::SVGOStringStream& s, NR::Point const &last) const
{
    s << "C "
      << last[NR::X] + start[0] / 3 << " "
      << last[NR::Y] + start[1] / 3 << " "
      << p[0] - end[0] / 3 << " "
      << p[1] - end[1] / 3 << " "
      << p[0] << " "
      << p[1] << " ";
}

PathDescr *PathDescrCubicTo::clone() const
{
    return new PathDescrCubicTo(*this);
}

void PathDescrArcTo::dumpSVG(Inkscape::SVGOStringStream& s, NR::Point const &last) const
{
    s << "A "
      << rx << " "
      << ry << " "
      << angle << " "
      << (large ? "1" : "0") << " "
      << (clockwise ? "0" : "1") << " "
      << p[NR::X] << " "
      << p[NR::Y] << " ";
}

PathDescr *PathDescrArcTo::clone() const
{
    return new PathDescrArcTo(*this);
}


PathDescr *PathDescrForced::clone() const
{
    return new PathDescrForced(*this);
}

void PathDescrClose::dumpSVG(Inkscape::SVGOStringStream& s, NR::Point const &last) const
{
    s << "z ";
}

PathDescr *PathDescrClose::clone() const
{
    return new PathDescrClose(*this);
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
