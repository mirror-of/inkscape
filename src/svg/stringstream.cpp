#include "svg/stringstream.h"
#include <glib/gmessages.h>
#include <string>

Inkscape::SVGOStringStream::SVGOStringStream()
{
    /* These two are probably unnecessary now that we provide our own operator<< for float and
     * double. */
    ostr.imbue(std::locale::classic());
    ostr.setf(std::ios::showpoint);

    /* This one is (currently) needed though, as we currently use ostr.precision as a sort of
       variable for storing the desired precision: see our two precision methods and our operator<<
       methods for float and double. */
    ostr.precision(8);
}

static void
write_without_trailing_zeros(Inkscape::SVGOStringStream &os, std::ostringstream &s)
{
    std::string str(s.str());
    std::string::size_type p_ix = str.find('.');
    if (p_ix != std::string::npos) {
        std::string::size_type e_ix = str.find('e', p_ix);
        /* N.B. In some contexts (e.g. CSS) it is an error for a number to contain `e'.  fixme:
         * Default to avoiding `e', e.g. using sprintf(str, "%17f", d).  Add a new function that
         * allows use of `e' and use that function only where the spec allows it.
         */
        std::string::size_type nz_ix = str.find_last_not_of('0', (e_ix == std::string::npos
                                                                  ? e_ix
                                                                  : e_ix - 1));
        if (nz_ix == std::string::npos || nz_ix < p_ix || nz_ix >= e_ix) {
            g_error("have `.' but couldn't find non-0");
        } else {
            str.erase(str.begin() + (nz_ix == p_ix
                                     ? p_ix
                                     : nz_ix + 1),
                      (e_ix == std::string::npos
                       ? str.end()
                       : str.begin() + e_ix));
        }
    }
    os << str;
}

Inkscape::SVGOStringStream &
operator<<(Inkscape::SVGOStringStream &os, float d)
{
    /* Try as integer first. */
    {
        long const n = long(d);
        if (d == n) {
            os << n;
            return os;
        }
    }

    std::ostringstream s;
    s.imbue(std::locale::classic());
    s.setf(std::ios::showpoint);
    s.precision(os.precision());
    s << d;
    write_without_trailing_zeros(os, s);
    return os;
}

Inkscape::SVGOStringStream &
operator<<(Inkscape::SVGOStringStream &os, double d)
{
    /* Try as integer first. */
    {
        long const n = long(d);
        if (d == n) {
            os << n;
            return os;
        }
    }

    std::ostringstream s;
    s.imbue(std::locale::classic());
    s.setf(std::ios::showpoint);
    s.precision(os.precision());
    s << d;
    write_without_trailing_zeros(os, s);
    return os;
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=99 :
